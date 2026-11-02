"""funcdecode.py - decode one function by recursive descent, the same way on
both sides of a comparison.

    decode_function(code, base, start, limit, reloc_at, noreturn, external_at)

`code` holds the bytes of [base, base + len(code)); the function starts at
`start` and may not extend past `limit` (the next function's start, or the end
of its COMDAT section). `reloc_at(addr)` says whether the four bytes at `addr`
are an absolute address (a base relocation in the image, a DIR32 in an object)
and returns the address they hold, else None. `external_at(addr)` says
whether the rel32 field at `addr` is relocated against another symbol: in an
object a jmp to another function holds 0, which reads as a jump to the next
instruction, so without it a tail jump makes the descent run on into whatever
follows (a switch table).

Following every branch inside the function, rather than sweeping linearly, is
what keeps MSVC's in-.text switch tables (a dword table of case addresses,
optionally preceded by a byte index table) and the int3 padding out of the
instruction stream. Bytes the descent never reaches are reported as data
ranges: tables, padding, or unreachable code.
"""
import capstone
from capstone import x86 as cx

_MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
_MD.detail = True

_COND = {'jo', 'jno', 'jb', 'jae', 'je', 'jne', 'jbe', 'ja', 'js', 'jns', 'jp', 'jnp',
         'jl', 'jge', 'jle', 'jg', 'jecxz', 'jcxz', 'loop', 'loope', 'loopne'}


class DecodedInsn:
    __slots__ = ('addr', 'size', 'bytes', 'mnemonic', 'op_str', 'target', 'table')

    def __init__(self, ins):
        self.addr, self.size = ins.address, ins.size
        self.bytes = bytes(ins.bytes)
        self.mnemonic, self.op_str = ins.mnemonic, ins.op_str
        self.target = None   # direct branch/call destination
        self.table = None    # jump table address, for jmp [table + reg*4]


class DecodedFunction:
    def __init__(self, start):
        self.start = start
        self.insns = []        # in address order
        self.tables = []       # (addr, [entry targets]) dword jump tables
        self.index_tables = [] # (addr, length) byte index tables
        self.end = start       # one past the last instruction or table byte
        self.calls = []        # (insn addr, target) for direct calls
        self.exits = []        # (insn addr, target) for jmp/jcc leaving [start, end)
        self.overrun = False   # descent wanted to run past `limit`


def _decode_at(code, base, addr):
    off = addr - base
    for ins in _MD.disasm(code[off:off + 16], addr, 1):
        return ins
    return None


def decode_function(code, base, start, limit, reloc_at, noreturn=frozenset(), external_at=None):
    f = DecodedFunction(start)
    seen = {}
    work = [start]
    table_addrs = set()
    while work:
        a = work.pop()
        while start <= a < limit and a not in seen:
            ins = _decode_at(code, base, a)
            if ins is None:
                f.overrun = True
                break
            d = DecodedInsn(ins)
            seen[a] = d
            m = ins.mnemonic
            nxt = a + ins.size
            if nxt > limit:
                f.overrun = True
            if m.startswith('ret') or m in ('int3', 'ud2', 'hlt'):
                break
            ops = ins.operands
            if m == 'call':
                if ops and ops[0].type == cx.X86_OP_IMM:
                    d.target = ops[0].imm
                    f.calls.append((a, d.target))
                    if d.target in noreturn:
                        break
                a = nxt
                continue
            if m == 'jmp' or m in _COND:
                if ops and ops[0].type == cx.X86_OP_IMM:
                    t = ops[0].imm
                    d.target = t
                    if external_at is not None and ins.size >= 5 and external_at(nxt - 4):
                        f.exits.append((a, None))
                    elif start <= t < limit:
                        work.append(t)
                    else:
                        f.exits.append((a, t))
                elif m == 'jmp' and ops and ops[0].type == cx.X86_OP_MEM:
                    mem = ops[0].mem
                    # The table's address through the relocation on the
                    # displacement: in an object the field holds only the
                    # addend when VC8 relocates against a $LN label.
                    tbl = reloc_at(a + ins.disp_offset)
                    if tbl is None:
                        tbl = mem.disp & 0xffffffff
                    if mem.index != 0 and mem.scale == 4 and start <= tbl < limit:
                        d.table = tbl
                        entries = []
                        p = tbl
                        while p + 4 <= limit and p not in seen:
                            t = reloc_at(p)
                            if t is None or not (start <= t < limit):
                                break
                            entries.append(t)
                            work.append(t)
                            p += 4
                        f.tables.append((tbl, entries))
                        table_addrs.update(range(tbl, tbl + 4 * len(entries)))
                if m == 'jmp':
                    break
                a = nxt
                continue
            # movzx reg, byte ptr [reg + idx_table]: the byte index of a sparse switch
            if m == 'movzx' and len(ops) == 2 and ops[1].type == cx.X86_OP_MEM and ops[1].size == 1:
                disp = reloc_at(a + ins.disp_offset)
                if disp is not None and start <= disp < limit and ops[1].mem.base != 0:
                    f.index_tables.append((disp, None))
            a = nxt
    f.insns = [seen[k] for k in sorted(seen)]
    if f.insns:
        f.end = max(i.addr + i.size for i in f.insns)
    for tbl, entries in f.tables:
        f.end = max(f.end, tbl + 4 * len(entries))
    # A byte index table runs from its start to the next instruction, table, or
    # the limit; its entries index the dword table and so stay below 256.
    fixed = []
    for disp, _ in f.index_tables:
        stops = [i.addr for i in f.insns if i.addr > disp] + [t for t, _ in f.tables if t > disp] + [limit]
        stop = min(stops)
        n = 0
        while disp + n < stop and code[disp + n - base] != 0xCC:
            n += 1
        fixed.append((disp, n))
        f.end = max(f.end, disp + n)
    f.index_tables = fixed
    return f
