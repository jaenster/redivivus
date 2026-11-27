#!/usr/bin/env python3
"""
Build the per-function spec the difftest host runs.

For each candidate 1.14d function it joins four sources on the 1.14d address:

  d2fn snapshot   the Ghidra bundle (read-only cache, `d2fn show`): parameter
                  storage, callees, globals, flags and the original asm
  the tree        the header declaration (`// 1.14d: <addr>`): the convention and
                  the types OUR build was compiled with, and the source map that
                  names the function's namespace
  nm              where the function and the globals sit in our exe
  objdump         our function's asm, for its `ret N` and its data references

The original's ABI comes from Ghidra's storage and is checked against the asm:
the `ret N` must pop exactly the stack parameters, and a register the asm reads
before writing must be a parameter. Ours comes from the header and is checked
against our `ret N`. Anything that cannot be placed exactly is reported as
untestable with the reason, never guessed.

  mkspec.py --ours <d2-difftest.exe> --out <spec.txt> [--pick auto|<file>] [--n 50]

Writes <spec.txt> for the host and <spec.txt>.tsv listing every candidate
considered with its verdict.
"""
import argparse, bisect, glob, json, os, re, subprocess, sys
from collections import defaultdict

HOME = os.path.expanduser('~')
TREE = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
SNAP = os.environ.get('D2FN_SNAPSHOT', f'{HOME}/.cache/d2fn-v2/snapshot') + '/fn'
AUDIT = f'{TREE}/asm_matching/msvc8/evidence/audit.tsv'
NM = 'i686-w64-mingw32-nm'
OBJDUMP = 'i686-w64-mingw32-objdump'

REGS = ['eax', 'ecx', 'edx', 'ebx', 'esi', 'edi']
GLOB_MAX = 0x8000     # must match host/difftest.c


# ------------------------------------------------------------------ types

def split_params(s):
    out, depth, cur = [], 0, ''
    for ch in s:
        if ch in '(<[':
            depth += 1
        elif ch in ')>]':
            depth -= 1
        if ch == ',' and depth == 0:
            out.append(cur.strip()); cur = ''
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return [p for p in out if p != 'void']


ONE = {'char', 'int8_t', 'uint8_t', 'byte', 'BYTE', 'bool', 'undefined1', 'uchar', 'unsigned char',
       'signed char', 'CHAR', 'UCHAR', 'BOOLEAN'}
TWO = {'short', 'int16_t', 'uint16_t', 'WORD', 'ushort', 'undefined2', 'unsigned short', 'wchar_t',
       'USHORT', 'SHORT'}
EIGHT = {'int64_t', 'uint64_t', 'longlong', 'ulonglong', '__int64', 'long long', 'unsigned long long',
         'undefined8', 'LONGLONG', 'ULONGLONG', 'QWORD', '__time64_t'}
UNSIGNED = re.compile(r'\b(u|unsigned|uint\w*|DWORD|WORD|BYTE|UINT|ULONG|size_t|uchar|ushort|uint)\b')


def classify(t, structs):
    """(gen, size) for a C type string, or (None, reason)."""
    t = re.sub(r'\b(const|volatile|struct|enum|union|class)\b', ' ', t)
    t = re.sub(r'\s+', ' ', t).strip()
    if '(' in t:
        return 'ptr', 4                     # function pointer
    if '*' in t or '&' in t or '[' in t:
        base = t.replace('*', '').replace('&', '').strip()
        if base in ('char', 'CHAR', 'uchar', 'unsigned char', 'int8_t', 'uint8_t', 'BYTE', 'byte') and t.count('*') == 1:
            return 'str', 4
        return 'ptr', 4
    if t in ('float', 'FLOAT'):
        return 'f32', 4
    if t in ('double', 'long double'):
        return 'f64', 8
    if t in EIGHT:
        return None, f'64-bit parameter {t}'
    if t in ('bool',):
        return 'bool', 1
    if t in ONE:
        return ('uint' if UNSIGNED.search(t) or t in ('byte', 'BYTE', 'undefined1') else 'int'), 1
    if t in TWO:
        return ('uint' if UNSIGNED.search(t) or t in ('undefined2', 'wchar_t') else 'int'), 2
    if t in structs:
        return None, f'struct {t} by value'
    return ('uint' if UNSIGNED.search(t) else 'int'), 4


def ret_kind(t, structs):
    t = re.sub(r'\b(const|volatile|struct|enum|union|class|static|inline)\b', ' ', t)
    t = re.sub(r'\s+', ' ', t).strip()
    if t == 'void':
        return 'v', 0
    if '*' in t:
        return 'p', 4
    if t == 'bool':
        return 'b', 1
    if t in ('float',):
        return 'f', 4
    if t in ('double', 'long double'):
        return 'd', 8
    if t in EIGHT:
        return 'q', 8
    if t in ONE:
        return ('u' if UNSIGNED.search(t) or t in ('byte', 'BYTE') else 'i'), 1
    if t in TWO:
        return ('u' if UNSIGNED.search(t) else 'i'), 2
    if t in structs:
        return None, 0
    return 'i', 4


# ------------------------------------------------------------------ the tree

DECL = re.compile(r'^\s*(?:static\s+|extern\s+|inline\s+)*(.+?)\s*\b(__fastcall|__stdcall|__thiscall|__cdecl)?\s*'
                  r'([A-Za-z_]\w*)\s*\((.*)\)\s*;\s*//\s*1\.14d:\s*([0-9a-f]{8})')
# The same declaration without the address comment, which the published tree leaves out;
# such a declaration is joined to its address through the .cpp.map entry's name.
DECL_BARE = re.compile(r'^(?:static\s+|extern\s+|inline\s+)*(.+?)\s*\b(__fastcall|__stdcall|__thiscall|__cdecl)?\s*'
                       r'([A-Za-z_]\w*)\s*\((.*)\)\s*;\s*(?://.*)?$')


def read_tree_decls(tree):
    """1.14d addr -> header declaration, name -> declarations without an address, and the
    set of struct names."""
    decls, byname, structs = {}, defaultdict(list), set()
    for path in glob.glob(f'{tree}/**/*.h', recursive=True):
        if '/asm_matching/' in path or '/thirdparty/' in path:
            continue
        try:
            text = open(path, encoding='utf-8', errors='replace').read()
        except OSError:
            continue
        for m in re.finditer(r'\b(?:struct|union|class)\s+([A-Za-z_]\w*)\s*(?::[^{;]*)?\{', text):
            structs.add(m.group(1))
        for m in re.finditer(r'typedef\s+(?:struct|union)\s+\w*\s*\{[^}]*\}\s*(\w+)', text):
            structs.add(m.group(1))
        for line in text.splitlines():
            if '(' not in line or line[:1].isspace() or line.startswith(('#', '//', 'return', 'typedef')):
                continue
            m = DECL.match(line) if '// 1.14d:' in line else None
            if m:
                ret, cc, name, params, addr = m.groups()
            else:
                m = DECL_BARE.match(line)
                if not m:
                    continue
                (ret, cc, name, params), addr = m.groups(), None
            d = {'ret': ret.strip(), 'cc': (cc or '__cdecl').strip('_'), 'name': name,
                 'params': split_params(params), 'header': os.path.relpath(path, tree)}
            if addr:
                decls.setdefault(addr, []).append(d)
            else:
                byname[name].append(d)
    return decls, byname, structs


def decl_by_name(byname, name, cpp):
    """The one declaration of `name`, preferring the header beside its .cpp."""
    ds = byname.get(name, [])
    beside = [d for d in ds if os.path.splitext(d['header'])[0] == os.path.splitext(cpp)[0]]
    if len(beside) == 1:
        return beside
    if ds and len({(d['ret'], d['cc'], tuple(d['params'])) for d in ds}) == 1:
        return ds[:1]
    return ds


def read_source_maps(tree):
    fn = {}
    for path in glob.glob(f'{tree}/**/*.cpp.map', recursive=True):
        if '/asm_matching/' in path:
            continue
        try:
            doc = json.load(open(path))
        except (ValueError, OSError):
            continue
        for f in doc.get('functions', []):
            if f.get('address'):
                fn[f['address'].lower()] = (f.get('namespace', ''), f['name'], doc.get('file', ''))
    return fn


# ------------------------------------------------------------------ our exe

def qualified_name(sym):
    if sym.startswith('@'):
        sym = sym[1:]                   # GCC's __fastcall decoration: @name@bytes
    if sym.startswith('__'):
        sym = sym[1:]
    if not sym.startswith('_ZN'):
        return None
    i, comps = 3, []
    while i < len(sym):
        while i < len(sym) and sym[i] in 'LK':
            i += 1
        j = i
        while j < len(sym) and sym[j].isdigit():
            j += 1
        if j == i:
            break
        n = int(sym[i:j])
        comps.append(sym[j:j + n])
        i = j + n
    return '::'.join(comps) if comps else None


def collapse(q):
    out = []
    for part in q.split('::'):
        if not out or out[-1] != part:
            out.append(part)
    return '::'.join(out)


def read_nm(exe):
    out = subprocess.run([NM, '--defined-only', exe], capture_output=True, text=True, check=True).stdout
    qual, plain, addrs = defaultdict(list), defaultdict(list), []
    for line in out.splitlines():
        bits = line.split(None, 2)
        if len(bits) != 3:
            continue
        try:
            va = int(bits[0], 16)
        except ValueError:
            continue
        kind, sym = bits[1], bits[2]
        addrs.append((va, sym, kind))
        q = qualified_name(sym)
        if q:
            qual[q].append((sym, va, kind))
            c = collapse(q)
            if c != q:
                qual[c].append((sym, va, kind))
        elif sym.startswith('_'):
            plain[sym[1:].split('@')[0]].append((sym, va, kind))
    addrs.sort()
    return qual, plain, addrs


_ADDR_KEYS = []


def sym_extent(addrs, va):
    if len(_ADDR_KEYS) != len(addrs):
        _ADDR_KEYS[:] = [a for a, _s, _k in addrs]
    i = bisect.bisect_right(_ADDR_KEYS, va)
    while i < len(addrs) and addrs[i][0] == va:
        i += 1
    return addrs[i][0] if i < len(addrs) else va + 0x1000


class OurText:
    """Our exe's .text disassembled once (objdump over a DWARF-laden exe is
    slow per call), indexed by address."""
    def __init__(self, exe):
        cache = exe + '.difftest-asm.txt'
        if not os.path.exists(cache) or os.path.getmtime(cache) < os.path.getmtime(exe):
            with open(cache + '.tmp', 'w') as f:
                subprocess.run([OBJDUMP, '-d', '-M', 'intel', '--no-show-raw-insn', '-j', '.text', exe],
                               stdout=f, check=True)
            os.replace(cache + '.tmp', cache)
        self.addr, self.text = [], []
        pat = re.compile(r'^\s*([0-9a-f]+):\s+(.*)$')
        for line in open(cache):
            m = pat.match(line)
            if m:
                self.addr.append(int(m.group(1), 16)); self.text.append(m.group(2).strip())

    def range(self, start, end):
        i = bisect.bisect_left(self.addr, start)
        j = bisect.bisect_left(self.addr, end)
        return list(zip(self.addr[i:j], self.text[i:j]))


def read_datamap(path):
    """Global name -> 1.14d address from the VC8 oracle's datamap (MSVC names)."""
    out = {}
    if not os.path.exists(path):
        return out
    for line in open(path):
        c = line.rstrip('\n').split('\t')
        if len(c) < 2 or not c[0].startswith('?'):
            continue
        name = c[0][1:].split('@', 1)[0]
        try:
            out.setdefault(name, int(c[1], 16))
        except ValueError:
            pass
    return out


# ------------------------------------------------------------------ original asm

def insn(line):
    """The instruction text of a snapshot asm line: `addr  bytes  INSN`."""
    f = re.split(r'\s{2,}', line.strip(), maxsplit=2)
    return f[2] if len(f) == 3 else f[-1]


def orig_ret_pop(asm):
    pops = set()
    for line in asm:
        m = re.match(r'RET\b\s*(0x[0-9a-f]+)?', insn(line))
        if m:
            pops.add(int(m.group(1), 16) if m.group(1) else 0)
    return pops


R32 = {'EAX': 'eax', 'AX': 'eax', 'AL': 'eax', 'AH': 'eax', 'ECX': 'ecx', 'CX': 'ecx', 'CL': 'ecx', 'CH': 'ecx',
       'EDX': 'edx', 'DX': 'edx', 'DL': 'edx', 'DH': 'edx', 'EBX': 'ebx', 'BX': 'ebx', 'BL': 'ebx', 'BH': 'ebx',
       'ESI': 'esi', 'SI': 'esi', 'EDI': 'edi', 'DI': 'edi'}


def orig_reg_inputs(asm):
    """Registers the first basic block reads before it writes them: the
    original's register inputs, on the straight-line path from the entry."""
    written, read = set(), set()
    for line in asm:
        m = re.match(r'([A-Z]+)\s*(.*)', insn(line))
        if not m:
            continue
        op, ops = m.group(1), m.group(2)
        regs_in = lambda s: {R32[r] for r in re.findall(r'\b(E?[ABCD]X|[ABCD][LH]|E?[SD]I)\b', s)}
        if op == 'PUSH' and '[' not in ops:
            continue                    # a leaf pushes only to save a register
        if op in ('RET', 'JMP') or op.startswith('J'):
            # stop at the first branch: past it the straight-line view is wrong
            src = regs_in(ops)
            read |= src - written
            break
        dst, _, src = ops.partition(',')
        if op in ('XOR', 'SUB', 'SBB') and dst.strip() == src.strip() and '[' not in dst:
            written |= regs_in(dst)
            continue
        mem_regs = set()
        for mm in re.findall(r'\[([^\]]*)\]', ops):
            mem_regs |= regs_in(mm)
        read |= (mem_regs | regs_in(src)) - written
        if op not in ('MOV', 'LEA', 'MOVZX', 'MOVSX', 'POP', 'SETZ', 'SETNZ', 'SETL', 'SETG', 'SETLE', 'SETGE',
                      'SETA', 'SETB', 'SETBE', 'SETAE', 'SETC', 'SETNC', 'SETS', 'SETNS'):
            read |= regs_in(dst) - written - mem_regs
        if '[' not in dst:
            written |= regs_in(dst)
        if op in ('CDQ',):
            written.add('edx'); read |= {'eax'} - written
        if op in ('MUL', 'IMUL', 'DIV', 'IDIV') and ',' not in ops:
            read |= {'eax'} - written
            written |= {'eax', 'edx'}
    return read


BAD_ORIG = re.compile(r'\b(CALL|RDTSC|CPUID|INT|IN|OUT|HLT|LOCK|CMPXCHG|XCHG|FS:|SYSENTER)\b|FS:', re.I)


# ------------------------------------------------------------------ assembling

SUBREG = {'AX': 'eax', 'AL': 'eax', 'CX': 'ecx', 'CL': 'ecx', 'DX': 'edx', 'DL': 'edx', 'BX': 'ebx',
          'BL': 'ebx', 'SI': 'esi', 'DI': 'edi'}


def storage_to_loc(st):
    m = re.match(r'(E[A-Z]{2}):(\d+)', st)
    if m:
        return m.group(1).lower(), int(m.group(2))
    m = re.match(r'([A-Z]{2}):(\d+)$', st)
    if m and m.group(1) in SUBREG:
        return SUBREG[m.group(1)], int(m.group(2))
    m = re.match(r'Stack\[0x([0-9a-f]+)\]:(\d+)', st)
    if m:
        return 'stack', int(m.group(2)), int(m.group(1), 16)
    return None


def our_locs(cc, gens):
    """Where OUR build passes each parameter, per the convention GCC honours."""
    locs, regs = [], []
    if cc == 'fastcall':
        regs = ['ecx', 'edx']
    elif cc == 'thiscall':
        regs = ['ecx']
    stack = 0
    for gen, size in gens:
        if regs and gen not in ('f32', 'f64') and size <= 4:
            locs.append(regs.pop(0))
        else:
            locs.append('stack'); stack += 8 if gen == 'f64' else 4
        if cc == 'fastcall' and gen in ('f32', 'f64'):
            pass
    pop = stack if cc in ('stdcall', 'fastcall', 'thiscall') else 0
    return locs, pop


def build(args):
    decls, byname, structs = read_tree_decls(args.tree)
    smap = read_source_maps(args.tree)
    qual, plain, addrs = read_nm(args.ours)
    text = OurText(args.ours)
    datamap = read_datamap(os.path.join(os.path.dirname(args.audit), 'datamap.tsv'))
    last = defaultdict(list)
    for k, v in qual.items():
        last[k.rsplit('::', 1)[-1]].extend(v)
    audit = {}
    for line in open(args.audit):
        c = line.rstrip('\n').split('\t')
        if c[0] == 'start':
            continue
        audit[c[0]] = {'end': c[1], 'name': c[2], 'status': c[9], 'sim': c[8]}

    rows, specs = [], []
    for fname in sorted(os.listdir(SNAP)):
        if not fname.endswith('.json'):
            continue
        addr = fname[:8]
        try:
            b = json.load(open(os.path.join(SNAP, fname)))
        except ValueError:
            continue
        a = audit.get(addr, {})
        row = {'addr': addr, 'name': f"{b.get('ns') or ''}::{b['name']}".lstrip(':'), 'status': a.get('status', '-'),
               'size': b.get('size', 0)}
        rows.append(row)

        def no(reason):
            row['verdict'] = 'untestable'; row['why'] = reason

        if b.get('class') != 'game':
            no(f"class {b.get('class')}"); continue
        asm = b.get('asm') or []
        if b.get('callees') or any(re.search(r'\sCALL\s', l) for l in asm):
            no('not a leaf: calls'); continue
        f = b.get('flags') or {}
        if f.get('seh') or f.get('indirectCalls') or f.get('sharedTail') or f.get('interiorEntry') or f.get('tailTargets'):
            no('flags: ' + ','.join(k for k in ('seh', 'indirectCalls', 'sharedTail', 'interiorEntry', 'tailTargets') if f.get(k)))
            continue
        if f.get('varargs'):
            no('varargs'); continue
        bad = [insn(l) for l in asm if BAD_ORIG.search(insn(l))]
        if bad:
            no('instruction: ' + bad[0][:40]); continue
        if not 6 <= b.get('size', 0) <= args.max_size:
            no(f"size {b.get('size')}"); continue
        d = decls.get(addr)
        if not d and addr in smap:
            d = decl_by_name(byname, smap[addr][1], smap[addr][2])
        if not d:
            no('no header declaration in the tree'); continue
        if len(d) > 1:
            no('several header declarations'); continue
        d = d[0]
        sm = smap.get(addr)
        if not sm:
            no('no source map entry'); continue
        ns, name, _file = sm
        key = f'{ns}::{name}' if ns else name
        hits = qual.get(key) or qual.get(collapse(key)) or ([] if ns else plain.get(name, []))
        hits = [h for h in hits if h[2] in 'Tt']
        if len({h[1] for h in hits}) != 1:
            no(f'our symbol: {len(hits)} matches for {key}'); continue
        usym, uva, _k = hits[0]
        gparams = b.get('params') or []
        if len(gparams) != len(d['params']):
            no(f"signature drift: Ghidra v1054 has {len(gparams)} params, tree {len(d['params'])}"); continue

        # Pair Ghidra's parameters with the header's. The snapshot is older than the
        # tree, so a parameter may have been renamed or the list reordered: pair by
        # name when every name matches, positionally when no shared name moved and
        # the pointer-ness of each pair agrees (a rename), and refuse a reorder.
        hnames = [(re.search(r'([A-Za-z_]\w*)\s*$', tp) or [None, ''])[1] for tp in d['params']]
        gnames = [gp.get('name') or '' for gp in gparams]
        if sorted(hnames) == sorted(gnames) and hnames != gnames and len(set(hnames)) == len(hnames):
            gparams = [gparams[gnames.index(n)] for n in hnames]
        elif any(n in gnames and gnames.index(n) != i for i, n in enumerate(hnames)):
            # a name both lists share sits at different positions: reordered
            no(f"signature drift: Ghidra v1054 ({','.join(gnames)}) vs tree ({','.join(hnames)})"); continue
        if any(('*' in (gp.get('type') or '')) != ('*' in tp or '(' in tp) for gp, tp in zip(gparams, d['params'])):
            no('signature drift: pointer-ness differs between Ghidra v1054 and the tree'); continue

        # parameter classes from OUR declared types; sizes must agree with Ghidra's storage
        ps, why = [], None
        for gp, tp in zip(gparams, d['params']):
            tname = re.sub(r'\s*\b[A-Za-z_]\w*\s*$', '', tp) if re.search(r'[\w*&]\s+[A-Za-z_]\w*$', tp) or re.search(r'\*\s*[A-Za-z_]\w*$', tp) else tp
            gen, size = classify(tname, structs)
            if gen is None:
                why = size; break
            loc = storage_to_loc(gp.get('storage', ''))
            if not loc:
                why = f"storage {gp.get('storage')}"; break
            ps.append({'name': gp['name'] or 'p', 'gen': gen, 'size': size, 'oloc': loc[0],
                       'ostack': loc[2] if loc[0] == 'stack' else None, 'gsize': loc[1]})
        if why:
            no(why); continue
        rk, rs = ret_kind(d['ret'], structs)
        if rk is None:
            no(f"returns struct {d['ret']}"); continue

        # original: stack parameters in Ghidra's order must be contiguous from +4
        st = sorted([p for p in ps if p['oloc'] == 'stack'], key=lambda p: p['ostack'])
        off, ok = 4, True
        for p in st:
            if p['ostack'] != off:
                ok = False; break
            off += 8 if p['gen'] == 'f64' else 4
        if not ok:
            no('original stack parameters not contiguous'); continue
        opop = off - 4
        pops = orig_ret_pop(asm)
        conv = b.get('convention', '')
        if pops and pops != {opop} and pops != {0}:
            no(f'original ret pops {sorted(pops)} but stack params are {opop} bytes'); continue
        if pops == {0} and opop and conv not in ('__cdecl', 'unknown', ''):
            no(f'original ret 0 with {opop} stack bytes under {conv}'); continue
        ocleanup = list(pops)[0] if len(pops) == 1 else -1
        # a register the original reads first must be one Ghidra places a parameter in
        inputs = orig_reg_inputs(asm)
        placed = {p['oloc'] for p in ps if p['oloc'] != 'stack'}
        stray = inputs - placed
        if stray:
            no(f"original reads {','.join(sorted(stray))} before writing it, Ghidra has no parameter there"); continue

        ulocs, upop = our_locs(d['cc'], [(p['gen'], p['size']) for p in ps])
        uend = sym_extent(addrs, uva)
        uasm = text.range(uva, min(uend, uva + 0x4000))
        uret = {int(m.group(1), 16) if m.group(1) else 0
                for _a, t in uasm for m in [re.match(r'ret\s*(?:0x([0-9a-f]+))?$', t)] if m}
        if uret and uret != {upop}:
            no(f"ours ret pops {sorted(uret)}, {d['cc']} with these params pops {upop}"); continue
        ucalls = sorted({t.split('<', 1)[-1].rstrip('>') for _a, t in uasm if t.startswith('call')})

        # globals: every data item the original touches, paired by name with ours
        globs, gwhy = [], None
        items = {}
        for g in b.get('globals') or []:
            items[g['addr']] = {'addr': g['addr'], 'label': g.get('name'), 'size': g.get('size')}
        for g in b.get('data') or []:
            items.setdefault(g['addr'], {}).update({'addr': g['addr'], 'label': g.get('label'), 'size': g.get('size')})
        for g in items.values():
            g['writable'] = 0x705000 <= int(g['addr'], 16) < 0x995000
        for g in sorted(items.values(), key=lambda g: g['addr']):
            label = g.get('label') or ''
            if not label or label.startswith(('s_', 'u_', 'DAT_', 'PTR_')) and not g.get('writable'):
                # a string or constant read through its address: both images hold their own copy
                continue
            hits = plain.get(label) or last.get(label, [])
            hits = [h for h in hits if h[2] in 'DdBbRr']
            field_off = 0
            if not hits and '.' in label:
                # a field of a struct global: the emitter may have split it out
                # (`base_field`), otherwise it is base + the field's offset
                base, field = label.split('.', 1)
                hits = [h for h in (plain.get(f"{base}_{field.replace('.', '_')}") or []) if h[2] in 'DdBbRr']
                if not hits and base in datamap:
                    hits = [h for h in (plain.get(base) or last.get(base, [])) if h[2] in 'DdBbRr']
                    field_off = int(g['addr'], 16) - datamap[base]
            if field_off:
                hits = [(h[0], h[1] + field_off, h[2]) for h in hits]
            if len({h[1] for h in hits}) != 1:
                if g.get('writable'):
                    gwhy = f'writable global {label} has {len(hits)} counterparts in ours'
                    break
                continue
            gva = hits[0][1]
            size = min(int(g.get('size') or 4), (sym_extent(addrs, gva - field_off) - gva) if field_off
                       else sym_extent(addrs, gva) - gva, GLOB_MAX)
            globs.append({'ova': int(g['addr'], 16), 'uva': gva, 'size': max(size, 1),
                          'rand': 1 if g.get('writable') else 0, 'name': label})
        if gwhy:
            no(gwhy); continue
        # A label inside another global's extent (Ghidra's stray interior labels:
        # gLightmapCellData at gaLightMapCells+4) is the same bytes on the original
        # side; filling it separately would desynchronise the two images.
        globs.sort(key=lambda g: (g['ova'], -g['size']))
        kept = []
        for g in globs:
            if any(k['ova'] <= g['ova'] < k['ova'] + k['size'] for k in kept):
                continue
            kept.append(g)
        globs = kept

        row['verdict'] = 'testable'
        row['ours'] = f'{uva:08x}'
        row['cc'] = f"{conv or '?'} -> {d['cc']}"
        row['ucalls'] = ','.join(ucalls)
        specs.append({'addr': addr, 'name': key, 'ova': int(addr, 16), 'uva': uva, 'rk': rk, 'rs': rs,
                      'ocleanup': ocleanup, 'ucleanup': upop if uret else -1, 'params': ps, 'ulocs': ulocs,
                      'globs': globs, 'status': row['status'], 'ucalls': ucalls, 'size': b.get('size', 0)})
    return rows, specs


INTEREST = re.compile(r'SEED|RAND|Rand|Seed|DRLG|Drlg|Str|str|Bit|bit|Math|Calc|Txt|TXT|Hash|Crc|CRC|Coord|Abs|'
                      r'Clamp|Min|Max|Sqrt|Div|Mul|Dist|Pos|Get|Is|Has|Count|Index', re.I)


def pick(specs, n, controls):
    """Up to `controls` VC8 MATCH functions as the control group, the rest not
    MATCH, preferring pure work: no globals, our build calls nothing."""
    def score(s):
        k = 0
        if not s['globs']:
            k += 4
        if not s['ucalls']:
            k += 2
        if INTEREST.search(s['name']):
            k += 2
        if any(p['gen'] in ('ptr', 'str') for p in s['params']):
            k += 1                      # memory effects are where silent bugs show
        return -k, -s['size'], s['addr']
    match = sorted([s for s in specs if s['status'] == 'MATCH'], key=score)
    other = sorted([s for s in specs if s['status'] != 'MATCH'], key=score)
    # spread over modules rather than taking the first run of one file
    def spread(lst, k):
        seen, out, rest = defaultdict(int), [], []
        for s in lst:
            mod = s['name'].split('::')[0]
            (out if seen[mod] < max(2, k // 6) else rest).append(s)
            seen[mod] += 1
        return (out + rest)[:k]
    return spread(match, controls) + spread(other, n - min(controls, len(match)))


def write_spec(path, chosen):
    with open(path, 'w') as f:
        f.write('# difftest spec, generated by tools/mkspec.py\n')
        for i, s in enumerate(chosen, 1):
            f.write(f"F {i} {s['name']}\n")
            f.write(f"A {s['ova']:08x} {s['uva']:08x} {s['rk']} {s['rs']} {s['ocleanup']} {s['ucleanup']}\n")
            for p, ul in zip(s['params'], s['ulocs']):
                f.write(f"P {p['name']} {p['gen']} {p['size']} {p['oloc']} {ul}\n")
            for g in s['globs']:
                f.write(f"G {g['ova']:08x} {g['uva']:08x} {g['size']} {g['rand']} {g['name']}\n")
            f.write('E\n')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--ours', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--tree', default=TREE)
    ap.add_argument('--audit', default=AUDIT)
    ap.add_argument('--n', type=int, default=50)
    ap.add_argument('--controls', type=int, default=15)
    ap.add_argument('--max-size', type=int, default=400)
    ap.add_argument('--pick', default='auto', help="auto, or a file of 1.14d addresses, one per line")
    args = ap.parse_args()
    rows, specs = build(args)
    if args.pick == 'auto':
        chosen = pick(specs, args.n, args.controls)
    else:
        lines = [l.split() for l in open(args.pick) if l.strip() and not l.startswith('#')]
        norm = lambda a: a.lower().replace('0x', '').zfill(8)
        want = [norm(l[0]) for l in lines]
        by = {s['addr']: s for s in specs}
        chosen = []
        for l in lines:
            w = norm(l[0])
            if w not in by:
                continue
            s = dict(by[w])
            if len(l) >= 3 and l[1] == 'vs' and norm(l[2]) in by:
                # negative control: the original of one function against our build
                # of another with the same signature; the harness must call it different
                other = by[norm(l[2])]
                s.update(uva=other['uva'], ulocs=other['ulocs'], ucleanup=other['ucleanup'],
                         name=f"{s['name']}__vs__{other['name'].rsplit('::', 1)[-1]}")
            chosen.append(s)
        missing = [w for w in want if w not in by]
        if missing:
            why = {r['addr']: r.get('why', '') for r in rows}
            for w in missing:
                print(f'not testable: {w} {why.get(w, "not in snapshot")}', file=sys.stderr)
    write_spec(args.out, chosen)
    with open(args.out + '.tsv', 'w') as f:
        f.write('addr\tname\tvc8\tsize\tverdict\twhy\tours\tconvention\tours_calls\tchosen\n')
        ch = {s['addr'] for s in chosen}
        for r in rows:
            f.write('\t'.join([r['addr'], r['name'], r['status'], str(r['size']), r.get('verdict', ''), r.get('why', ''),
                               r.get('ours', ''), r.get('cc', ''), r.get('ucalls', ''), 'yes' if r['addr'] in ch else '']) + '\n')
    n_test = sum(1 for r in rows if r.get('verdict') == 'testable')
    print(f'{len(rows)} functions in the snapshot, {n_test} testable, {len(chosen)} chosen '
          f'({sum(1 for s in chosen if s["status"] == "MATCH")} VC8 MATCH)')


if __name__ == '__main__':
    main()
