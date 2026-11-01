"""objfuncs.py - our side of the comparison: the functions in one VC8 object.

Each function is a COMDAT code section (/O2 implies /Gy), so its extent is
exact. Relocations are resolved to what they name:

    ('call', sym)    REL32 to a function            ('import', 'KERNEL32-free name')
    ('data', sym)    DIR32 to a data symbol          ('string', bytes) for a literal
    ('self', off)    DIR32 into this same section (switch tables, handler blocks)

so a function can be compared and matched by what it refers to, not by where.
"""
import re
import struct

import coffobj
import funcdecode

_IMP = re.compile(r'^__imp__(.+?)(@\d+)?$')


def qualified(mangled):
    """`?Name@Ns2@Ns1@@...` -> 'Ns1::Ns2::Name' (enough to match by name; the
    full signature is not needed). C names lose their leading underscore and
    stdcall/fastcall decoration."""
    if not mangled.startswith('?'):
        n = mangled[1:] if mangled[:1] in '_@' else mangled
        return re.sub(r'@\d+$', '', n)
    body = mangled[1:]
    special = ''
    if body.startswith('?'):
        # ??0 ctor, ??1 dtor, ??_G scalar deleting dtor, ??4 operator=, ...
        m = re.match(r'\?(_[A-Z0-9]|[A-Z0-9])', body)
        special = m.group(0) if m else '?'
        body = body[len(special):]
    parts = []
    i = 0
    while i < len(body) and not body.startswith('@', i):
        if body.startswith('?$', i):             # template name: ?$Name@args@
            j = body.find('@', i + 2)
            parts.append(body[i + 2:j] + '<>')
            depth, k = 0, j + 1
            while k < len(body):
                if body[k] == '@' and depth == 0:
                    break
                k += 1
            i = k + 1
            continue
        j = body.find('@', i)
        if j < 0:
            break
        seg = body[i:j]
        if seg and seg[0].isdigit():             # back-reference: keep as-is
            seg = '#' + seg
        parts.append(seg)
        i = j + 1
    parts.reverse()
    if special:
        cls = parts[-1] if parts else '?'
        name = {'?0': cls, '?1': '~' + cls, '?_G': '`scalar deleting dtor`', '?_E': '`vector deleting dtor`',
                '?4': 'operator='}.get(special, 'operator' + special)
        parts.append(name)
    return '::'.join(p for p in parts if p)


def line_tables(o):
    """symbol -> [(function-relative offset, source file, line)] from the
    CodeView C13 line tables /Z7 writes into .debug$S (subsection 0xF2, tied
    to its function by a SECREL relocation; file names via 0xF4 -> 0xF3)."""
    strings, checks, raw = b'', b'', {}
    for sec in o.sections[1:]:
        if not sec.name.startswith('.debug$S'):
            continue
        d = sec.data
        if len(d) < 4 or struct.unpack_from('<I', d, 0)[0] != 4:
            continue
        relocs = {r.offset: r for r in sec.relocs}
        p = 4
        while p + 8 <= len(d):
            t, n = struct.unpack_from('<II', d, p)
            body = p + 8
            if t == 0xF3:
                strings = d[body:body + n]
            elif t == 0xF4:
                checks = d[body:body + n]
            elif t == 0xF2 and body in relocs:
                sym = o.symbol(relocs[body].symbol).name
                off = struct.unpack_from('<I', d, body)[0]
                q, end = body + 12, body + n
                while q + 12 <= end:
                    fid, nl, cbb = struct.unpack_from('<III', d, q)
                    for i in range(nl):
                        lo, ln = struct.unpack_from('<II', d, q + 12 + 8 * i)
                        raw.setdefault(sym, []).append((off + lo, fid, ln & 0xffffff))
                    q += max(cbb, 12)
            p = (body + n + 3) & ~3

    def fname(fid):
        if fid + 4 > len(checks):
            return '?'
        no = struct.unpack_from('<I', checks, fid)[0]
        e = strings.find(b'\0', no)
        return strings[no:e].decode('latin1') if e >= 0 else '?'
    return {sym: sorted((off, fname(fid), ln) for off, fid, ln in v) for sym, v in raw.items()}


class Function:
    __slots__ = ('obj', 'symbol', 'name', 'section', 'code', 'relocs', 'refs', 'decoded', 'static', 'lines')

    def __init__(self, obj, sym, sec, start, end):
        self.obj = obj
        self.symbol = sym.name
        self.name = qualified(sym.name)
        self.static = sym.storage == coffobj.C_STATIC
        self.section = sec
        self.code = sec.data[start:end]
        self.relocs = {}      # offset -> (kind, target)
        self.refs = []        # (offset, kind, target) in code order
        self.decoded = None
        self.lines = []       # (offset, source file, line), when built with /Z7


def _string_of(o, sym):
    """Bytes of a string-literal COMDAT (??_C@...), else None."""
    if sym.section <= 0 or not sym.name.startswith('??_C@'):
        return None
    sec = o.sections[sym.section]
    data = sec.data[sym.value:]
    if sym.name.startswith('??_C@_1'):          # wide string: up to the first 0x0000 unit
        for k in range(0, len(data) - 1, 2):
            if data[k] == 0 and data[k + 1] == 0:
                return data[:k + 2]
        return data
    k = data.find(b'\0')
    return data[:k + 1] if k >= 0 else data


def functions(path, data=None, labels=False):
    """Functions of the object at `path`, or of `data` (an archive member) named `path`.
    labels: see coffobj.CoffObject.functions (assembler objects of a library)."""
    if data is None:
        data = open(path, 'rb').read()
    o = coffobj.CoffObject(data, path)
    out = []
    lines = line_tables(o)
    # Code addresses inside a function's section that other sections hold
    # (EH unwind/catch maps, SEH scope tables): extra descent roots.
    roots = {}
    for sec in o.sections[1:]:
        for r in sec.relocs:
            s = o.symbol(r.symbol)
            if r.type == coffobj.DIR32 and s.section > 0 and o.sections[s.section].is_code and s.section != sec.number:
                addend = struct.unpack_from('<I', sec.data, r.offset)[0]
                roots.setdefault(s.section, set()).add(s.value + addend)
    funcs = o.functions(labels)
    # Function starts by (section, offset): a section of an assembler or C
    # library object holds several functions, so an address taken in the
    # same section is not necessarily the function's own.
    starts = {(sec.number, start): sym.name for sym, sec, start, _end in funcs}
    for sym, sec, start, end in funcs:
        if not sec.is_code:
            continue
        f = Function(path, sym, sec, start, end)
        f.lines = lines.get(sym.name, [])
        for r in sec.relocs:
            if not start <= r.offset < end:
                continue
            s = o.symbol(r.symbol)
            off = r.offset - start
            if r.type == coffobj.REL32:
                m = _IMP.match(s.name)
                kind = ('import', m.group(1)) if m else ('call', s.name)
            elif r.type == coffobj.DIR32:
                m = _IMP.match(s.name)
                st = _string_of(o, s)
                if m:
                    kind = ('import', m.group(1))
                elif st is not None:
                    kind = ('string', st)
                elif s.section == sec.number:
                    # Modulo 2**32: against a label after the target the addend is negative.
                    at = (s.value + struct.unpack_from('<I', sec.data, r.offset)[0]) & 0xffffffff
                    other = starts.get((sec.number, at)) if not start <= at < end else None
                    kind = ('fptr', other) if other else ('self', (at - start) & 0xffffffff)
                elif s.section > 0 and o.sections[s.section].is_code and s.is_function:
                    kind = ('fptr', s.name)
                else:
                    kind = ('data', s.name)
            else:
                kind = ('other', s.name)
            f.relocs[off] = kind
            f.refs.append((off, kind[0], kind[1]))
        f.refs.sort()

        def reloc_at(addr, f=f):
            k = f.relocs.get(addr)
            return k[1] if k and k[0] == 'self' else None
        def external_at(addr, f=f):
            k = f.relocs.get(addr)
            return k is not None and k[0] in ('call', 'import')
        f.decoded = funcdecode.decode_function(f.code, 0, 0, len(f.code), reloc_at, external_at=external_at)
        extra = sorted(x - start for x in roots.get(sec.number, ()) if start < x < end)
        for r in extra:
            g = funcdecode.decode_function(f.code, 0, r, len(f.code), reloc_at, external_at=external_at)
            have = {i.addr for i in f.decoded.insns}
            f.decoded.insns += [i for i in g.insns if i.addr not in have]
            f.decoded.tables += g.tables
            f.decoded.end = max(f.decoded.end, g.end)
        f.decoded.insns.sort(key=lambda i: i.addr)
        out.append(f)
    return out
