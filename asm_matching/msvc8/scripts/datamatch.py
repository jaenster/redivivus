#!/usr/bin/env python3
"""datamatch.py - byte-compare our initialised data with the original's.

    datamatch.py extents [--snapshot DIR] [--out evidence/dataextent.tsv]
    datamatch.py audit [--obj DIR ...] [--out evidence/data-audit.tsv] [--label TEXT]

The function audit (audit.py) judges code; this judges what the code reads:
every global and constant table of ours against the bytes 1.14d's Game.exe
holds at that global's address, relocations masked as the function compare
masks them - a pointer compares by the symbol it names, not by its address.

`extents` reads a ghidra-reconstruct codegen snapshot (globals.ndjson, the
same export a regen emits the tree from) into evidence/dataextent.tsv: one
row per data item Ghidra has in .rdata/.data, with its extent. Ghidra is the
truth for an extent, so this file is the denominator; it is written once per
Ghidra version and committed, so the audit needs neither Ghidra nor a daemon.
A label Ghidra places inside another item (`gnHashHighTable` four bytes into
`gaHashSeedTable`, or `gaTable[3][0]`, which the generator writes as
`gaTable_3__0_`) is reported with the size of the item it sits in; its row
is marked `interior` and its extent runs to that item's end.

`audit` pairs every data symbol of our objects with one of those rows - by
name (namespace, or owner function for a static local), else by the address
evidence/datamap.tsv implies from aligned code - and judges it:

  MATCH        every byte equal, every pointer names the same target
  ZERO         ours is all zero (or .bss) where the original has data:
               an initialiser the generator never wrote, or one VC8 builds
               at run time (a constructor call)
  SPLIT        an interior label emitted as an object of its own
  SIZE_SMALL   our extent is shorter than Ghidra's: a truncated table
  SIZE_LARGE   ours is longer than Ghidra's plus any alignment padding
  RELOC        a pointer names another target than the original's (or ours
               holds a constant where the original holds a pointer: RAW
               when that constant is the original's own unrelocated address)
  STRING       a string (or a pointer to one) differs in content
  VALUE        other bytes differ; FLOAT in the notes when the type is one
  BSS_INIT     the original is uninitialised (.bss), ours carries bytes
  ALIGN        .bss only: our alignment is weaker than the original's

For an original .bss item only size and alignment are compared. Our extent
is a symbol's offset to the next symbol of its section (or the section end),
so it includes padding; SIZE_LARGE allows for the padding the next symbol's
alignment can explain. The status is the first of ZERO, SPLIT, SIZE_SMALL,
RELOC, STRING, VALUE, SIZE_LARGE that applies; `flags` lists every one that
applies. A defect's notes say whether code or data of ours references the
symbol: a wrong copy nothing references is dead weight, not a fault.

A floating-point constant our code reads as a compiler literal rather than
by name is judged by the literals of its readers' objects (kind `const`).
Besides the audit (and its .summary) it writes datatrend.tsv (MATCH rate by
size) and data-fixes.tsv (the largest defects, a likely root cause each).

Compiler literals our code references (`__real@`, `??_C@` strings, `??_7`
vftables) have no Ghidra name of their own; the datamap's address pairs them,
and they are judged the same way but reported apart (kind `literal`).

Never quote counts from this file in prose; they go stale. Read the file.
"""
import argparse
import bisect
import collections
import glob
import json
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import coffobj      # noqa: E402
import peimage      # noqa: E402
from oracle_config import ORACLE, REPO, BUILD, EXPERIMENT  # noqa: E402

EVIDENCE = os.path.join(ORACLE, 'evidence')
EXTENTS = os.path.join(EVIDENCE, 'dataextent.tsv')
REFERENCE = os.path.join(ORACLE, 'reference', 'Game.exe')
DATA_LO, DATA_HI = 0x6cc000, 0x995000        # .rdata and .data (with its zero-fill tail)

# Compiler- and linker-written data: no source of its own, so out of the
# headline as the compiler's EH code is out of the function figure.
EH_TYPES = ('FuncInfo', 'UnwindMapEntry', 'TryBlockMapEntry', 'HandlerType', 'ScopeTableRecord',
            'ESTypeList', '_s_FuncInfo', '_s_UnwindMapEntry', '_s_TryBlockMapEntry', '_s_HandlerType',
            'RTTICompleteObjectLocator', 'RTTIClassHierarchyDescriptor', 'RTTIBaseClassDescriptor',
            'RTTIBaseClassArray', 'TypeDescriptor', 'IMAGE_LOAD_CONFIG_DIRECTORY32',
            'IMAGE_IMPORT_DESCRIPTOR', 'IMAGE_THUNK_DATA32')
FLOAT_TYPES = re.compile(r'^(float|double|long double|float10|real)(\[|$)')
STRING_TYPES = re.compile(r'^(char|wchar_t|WCHAR|TCHAR|string|unicode|TerminatedCString|TerminatedUnicode)'
                          r'(\[|$)|^(ds|string|unicode)$')

EIGHT_BYTE_TYPES = re.compile(r'^(double|longlong|ulonglong|__int64|qword|undefined8|LARGE_INTEGER|ULARGE_INTEGER|'
                              r'FILETIME|int64_t|uint64_t)(\[|$)')
EXT_HEADER = 'address\tsize\textent\tname\taliases\tscope\tnamespace\towner\ttype\tkind\treferencing'


# -- the original's extents ----------------------------------------------------

def is_label_name(name):
    """A label Ghidra derives from another item: `gaTable[5].field` (but not
    a string label, `s_.?AVfoo@@_0070f5f4`)."""
    if name.startswith(('s_', 'u_')):
        return False
    return '[' in name or bool(re.search(r'[A-Za-z0-9_\]]\.[A-Za-z_]', name))


def extents_main(args):
    snap = args.snapshot
    manifest = json.load(open(os.path.join(snap, 'manifest.json')))
    prov = manifest.get('provenance', {})
    rows = [json.loads(l) for l in open(os.path.join(snap, 'globals.ndjson')) if l.strip()]
    by_addr = collections.defaultdict(list)
    for r in rows:
        a = int(r['address'], 16)
        if DATA_LO <= a < DATA_HI:
            if is_label_name(r['name']):
                # The generator writes `gaTable[3][0]` as `gaTable_3__0_`.
                r = dict(r, name=re.sub(r'[\[\].]', '_', r['name']), label=True)
            by_addr[a].append(r)
    heads = []
    for a in sorted(by_addr):
        rs = by_addr[a]
        # The primary symbol at an address names it; the rest are aliases.
        rs.sort(key=lambda r: (r.get('label', False), r.get('primary') is not True, r.get('primary') is False,
                               r['name'].startswith(('DAT_', 'PTR_', 'LAB_'))))
        heads.append((a, rs[0], [r['name'] for r in rs[1:]]))
    out = []
    container = None          # (address, end, size, type) of the last item that is no interior label
    for a, r, aliases in heads:
        size = int(r.get('size') or 1)
        typ = r.get('dataType') or ''
        kind = 'data'
        extent = size
        if container and container[0] < a < container[1]:
            kind = 'interior'
            if size == container[2] and typ == container[3]:
                extent = container[1] - a          # it reports the item it sits in
            else:
                extent = min(size, container[1] - a)
        else:
            container = (a, a + size, size, typ)
        if r.get('importSlot'):
            kind = 'import'
        elif typ.split('[')[0].strip(' *') in EH_TYPES:
            kind = 'compiler'
        out.append((a, size, extent, r['name'], ','.join(aliases), r.get('scope') or '',
                    r.get('namespace') or '', r.get('ownerFunction') or '', typ, kind,
                    ','.join(r.get('referencingFunctions') or [])))
    with open(args.out, 'w') as fh:
        fh.write('# Ghidra %s v%s, snapshot written %s\n' % (
            prov.get('programPath', '?'), prov.get('programVersion', '?'), prov.get('writtenAt', '?')))
        fh.write(EXT_HEADER + '\n')
        for row in out:
            fh.write('%08x\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' % row)
    print('%d data items (%d interior labels) from %s v%s -> %s' % (
        len(out), sum(1 for r in out if r[9] == 'interior'), prov.get('programPath'),
        prov.get('programVersion'), args.out))


class Item:
    __slots__ = ('addr', 'size', 'extent', 'name', 'aliases', 'scope', 'ns', 'owner', 'type', 'kind', 'refs', 'parent')


def load_extents(path):
    items, version = [], '?'
    for line in open(path):
        if line.startswith('# Ghidra'):
            m = re.search(r' v(\d+)', line)
            version = m.group(1) if m else '?'
            continue
        if line.startswith('address\t') or not line.strip():
            continue
        c = line.rstrip('\n').split('\t')
        it = Item()
        it.addr, it.size, it.extent = int(c[0], 16), int(c[1]), int(c[2])
        it.name, it.aliases, it.scope, it.ns, it.owner, it.type, it.kind = c[3], c[4], c[5], c[6], c[7], c[8], c[9]
        it.refs = [x for x in c[10].split(',') if x]
        it.parent = None
        items.append(it)
    # An interior label's container: the item before it whose extent covers it.
    head = None
    for it in items:
        if it.kind == 'interior':
            if head is not None and head.addr < it.addr < head.addr + head.extent:
                it.parent = head
        else:
            head = it
    return items, version


# -- our side ----------------------------------------------------------------

class Ours:
    __slots__ = ('uid', 'symbol', 'tu', 'obj', 'sec', 'off', 'extent', 'align', 'next_align', 'bss', 'common')


def our_uid(tu, s):
    """A static is its object's own: two TUs may each define one of a name."""
    return '%s|%s' % (tu, s.name) if s.storage == coffobj.C_STATIC else s.name


def sec_align(flags):
    n = (flags >> 20) & 0xf
    return 1 << (n - 1) if n else 16


def lowbit(v, cap=16):
    return min(cap, v & -v) if v else cap


SKIP_SECTIONS = ('.debug', '.drectve', '.CRT', '.xdata', '.sxdata', '.tls', '.text', '.idata', '.rsrc')


def load_ours(objdirs):
    """Every data symbol our objects define: symbol -> Ours (the first
    definition; a COMDAT or selectany twin counts once)."""
    out, dups = {}, collections.Counter()
    objs = {}
    for root, sub in objdirs:
        for p in sorted(glob.glob(os.path.join(root, '**', '*.obj'), recursive=True)):
            rel = os.path.relpath(p, root)[:-4]
            tu = (sub + '/' + rel) if sub else rel
            try:
                o = coffobj.CoffObject(open(p, 'rb').read(), p)
            except ValueError:
                continue
            objs[tu] = o
            o.tu = tu
            o.data_index = {}       # section -> sorted (offset, uid): a section-relative reference's target
            per_sec = collections.defaultdict(list)
            for s in o.symbols:
                if s is None:
                    continue
                if s.section == 0 and s.storage == coffobj.C_EXTERNAL and s.value > 0 and not s.is_function:
                    u = Ours()
                    u.symbol, u.tu, u.obj, u.sec, u.off, u.extent = s.name, tu, o, None, 0, s.value
                    u.align, u.next_align, u.bss, u.common = lowbit(s.value, 8), 16, True, True
                    u.uid = s.name
                    if u.uid in out:
                        dups[u.uid] += 1
                    else:
                        out[u.uid] = u
                    continue
                if s.section <= 0 or s.storage not in (coffobj.C_EXTERNAL, coffobj.C_STATIC):
                    continue
                sec = o.sections[s.section]
                if sec.is_code or sec.name.startswith(SKIP_SECTIONS) or s.is_function:
                    continue
                if s.storage == coffobj.C_STATIC and s.name == sec.name:
                    continue
                per_sec[s.section].append(s)
            for secnum, syms in per_sec.items():
                sec = o.sections[secnum]
                # Extent: to the next symbol of the section at a higher offset.
                offs = sorted({s.value for s in syms})
                for s in syms:
                    k = bisect.bisect_right(offs, s.value)
                    end = offs[k] if k < len(offs) else sec.size
                    if s.name.startswith('$') or '$' in s.name.split('@')[0][1:]:
                        continue
                    u = Ours()
                    u.symbol, u.tu, u.obj, u.sec, u.off, u.extent = s.name, tu, o, sec, s.value, end - s.value
                    u.align = min(sec_align(sec.flags), lowbit(s.value))
                    u.next_align = lowbit(end) if k < len(offs) else sec_align(sec.flags)
                    u.bss = sec.name.startswith('.bss') or not (sec.flags & 0x40)   # no initialised data
                    u.common = False
                    u.uid = our_uid(tu, s)
                    o.data_index.setdefault(secnum, []).append((s.value, u.uid))
                    if u.uid in out:
                        dups[u.uid] += 1
                    else:
                        out[u.uid] = u
                o.data_index.get(secnum, []).sort()
    return out, dups, objs


MANGLED_STATIC_LOCAL = re.compile(r'^\?([^@?]+)@\?(?:\d|[A-P]+@)\?\?([^@]+)@((?:[^@?]+@)*)@')
MANGLED_GLOBAL = re.compile(r'^\?([^@?]+)@((?:[^@?$]+@)*)@[0-9]')


def our_identity(sym):
    """(name, scope path, owner function) of a data symbol, from its mangled
    name: `?x@Ns2@Ns1@@3HA` is ('x', 'Ns1::Ns2', ''), a static local
    `?x@?1??Func@Ns@@YGXXZ@4HA` is ('x', 'Ns', 'Func'). None for compiler
    literals and templates."""
    if not sym.startswith('?'):
        if sym.startswith(('__real@', '__xmm@', '__imp_')):
            return None
        return (sym[1:] if sym.startswith('_') else sym, '', '')
    if sym.startswith('??'):
        return None
    m = MANGLED_STATIC_LOCAL.match(sym)
    if m:
        return (m.group(1), '::'.join(reversed([p for p in m.group(3).split('@') if p])), m.group(2))
    m = MANGLED_GLOBAL.match(sym)
    if m:
        return (m.group(1), '::'.join(reversed([p for p in m.group(2).split('@') if p])), '')
    return None


def is_literal(sym):
    return sym.startswith(('__real@', '__xmm@', '??_C@', '??_7', '??_R'))


def embedded_address(name):
    m = re.search(r'_(00[67][0-9a-f]{5}|[67][0-9a-f]{5}|00[89][0-9a-f]{5}|[89][0-9a-f]{5})(?:$|_)', name)
    return int(m.group(1), 16) if m else None


def load_datamap(path):
    out = {}
    if os.path.exists(path):
        for line in open(path):
            if line.startswith('symbol\t') or not line.strip():
                continue
            c = line.rstrip('\n').split('\t')
            out[c[0]] = (int(c[1], 16), bool(c[3]) if len(c) > 3 else False)
    return out


def pair(ours, items, datamap):
    """Our uid -> (Item, method, note). Name first (unique, or narrowed by
    an address the name carries / owner function / namespace), then the
    datamap's address, then the address a generated name carries."""
    by_name = collections.defaultdict(list)
    by_addr = {}
    for it in items:
        by_addr.setdefault(it.addr, it)
        by_name[it.name].append(it)
        for a in it.aliases.split(','):
            if a:
                by_name[a].append(it)
    pairs = {}
    for uid, u in ours.items():
        sym = u.symbol
        ident = our_identity(sym)
        dm = datamap.get(sym)
        hit, method = None, None
        if ident:
            name, scope, owner = ident
            cands = by_name.get(name, [])
            if len(cands) > 1:
                ea = embedded_address(name)
                narrowed = ([c for c in cands if c.addr == ea] if ea else []) or \
                           ([c for c in cands if c.owner == owner] if owner else []) or \
                           [c for c in cands if c.ns == scope]
                if len(narrowed) > 1 and dm:
                    narrowed = [c for c in narrowed if c.addr == dm[0]] or narrowed
                cands = narrowed
            if len(cands) == 1:
                hit, method = cands[0], 'name'
            elif len(cands) > 1:
                method = 'ambiguous'
        where, how = None, None
        if hit is None and dm and not dm[1]:
            where, how = dm[0], 'datamap'
        elif hit is None and ident and embedded_address(ident[0]):
            where, how = embedded_address(ident[0]), 'name-address'
        if where is not None:
            it = by_addr.get(where)
            if it is not None:
                hit, method = it, how
            elif DATA_LO <= where < DATA_HI:
                # Ghidra has no item there (a literal, or a label it renamed):
                # judge it over our extent.
                it = Item()
                it.addr, it.size, it.extent, it.name, it.aliases = where, u.extent, u.extent, sym, ''
                it.scope = it.ns = it.owner = ''
                it.type, it.kind, it.refs = '', 'literal' if is_literal(sym) else 'noitem', []
                it.parent = None
                hit, method = it, how + '-noitem'
        if hit is None:
            continue
        note = ''
        if dm and dm[0] != hit.addr and method == 'name':
            note = 'datamap implies %08x' % dm[0]
        pairs[uid] = (hit, method, note)
    return pairs


# -- the comparison -----------------------------------------------------------

class Resolver:
    """Where a relocation of ours points, in the original's address space."""

    def __init__(self, pe, pairs, ours, funcsyms):
        self.pe = pe
        self.data = {s: p[0].addr for s, p in pairs.items()}
        self.ours = ours
        self.funcs = funcsyms
        self.imports = {}
        for va, nm in pe.imports.items():
            self.imports[norm_import(nm.split('!', 1)[1])] = va

    def target(self, obj, symidx, addend):
        """(kind, original VA or None, our symbol)"""
        s = obj.symbol(symidx)
        name = s.name
        if s.storage == coffobj.C_STATIC and s.section > 0:
            if name == obj.sections[s.section].name:
                # Section-relative: the data symbol of ours the addend lands in.
                idx = obj.data_index.get(s.section, [])
                k = bisect.bisect_right(idx, (addend, '\uffff')) - 1
                if k < 0:
                    return ('section', None, name)
                off, uid = idx[k]
                if uid in self.data:
                    return ('data', self.data[uid] + addend - off, uid)
                return ('section', None, uid)
            uid = '%s|%s' % (obj.tu, name)
            if uid in self.data:
                return ('data', self.data[uid] + addend, name)
        if name in self.data:
            return ('data', self.data[name] + addend, name)
        f = self.funcs.get(name)
        if f is not None:
            return ('func', f + addend, name)
        if name.startswith('__imp_'):
            va = self.imports.get(norm_import(name[6:]))
            return ('import', va, name)
        return ('unknown', None, name)

    def through_thunk(self, va):
        """A jump thunk's destination (the original's tables may point at the
        `jmp` stub where ours names the function), else va itself."""
        try:
            s = self.pe.section_at(va)
            if s is not None and s.name == '.text':
                b = self.pe.read(va, 5)
                if b[0] == 0xe9:
                    return (va + 5 + struct.unpack_from('<i', b, 1)[0]) & 0xffffffff
        except ValueError:
            pass
        return va


def norm_import(n):
    n = n[1:] if n.startswith(('_', '@')) else n
    return re.sub(r'@\d+$', '', n).lower()


def literal_bytes(obj, symidx, addend, n):
    """Our bytes at symbol+addend (a string or float literal's content)."""
    s = obj.symbol(symidx)
    if s.section <= 0:
        return None
    sec = obj.sections[s.section]
    o = s.value + addend
    return sec.data[o:o + n]


PTR_STATS = collections.Counter()      # (target kind, verdict) over every pointer field compared


def pointer_verdict(pe, res, obj, r, ov, uv):
    """(target kind, verdict, our symbol) for a field both sides relocate.
    verdict: agree, thunk (ours names the function, the original its jump
    thunk), bad (another target), string (a literal of other content),
    unverified (our target has no original address to compare)."""
    kind, va, sym = res.target(obj, r.symbol, uv)
    if sym.startswith('??_C@'):
        kind = 'string'
    elif sym.startswith('__real@'):
        kind = 'float'
    if va is None:
        # A literal of ours with no pair: compare its content.
        if sym.startswith('??_C@_0'):
            want = pe.cstring(ov)
            have = literal_bytes(obj, r.symbol, uv, len(want) + 1) if want is not None else None
            if have is not None:
                return kind, ('agree' if have[:len(want) + 1] == want + b'\0' else 'string'), sym
        if sym.startswith('__real@'):
            have = literal_bytes(obj, r.symbol, uv, 8)
            if have:
                return kind, ('agree' if pe.read(ov, len(have)) == have else 'bad'), sym
        return kind, 'unverified', sym
    if va == ov:
        return kind, 'agree', sym
    if res.through_thunk(va) == res.through_thunk(ov):
        return kind, 'thunk', sym
    return kind, 'bad', sym


def judge(pe, relocs, it, u, res):
    """(status, flags, notes, first_diff, diff_bytes, matched_bytes)"""
    flags, notes = [], []
    n_orig = it.extent
    rawend = pe.base + pe.section('.data').va + pe.section('.data').rawsize
    orig_bss = it.addr >= rawend
    ours_ext = u.extent
    n = min(n_orig, ours_ext)
    ob = pe.read(it.addr, n_orig)
    ub = (bytes(u.sec.data[u.off:u.off + ours_ext]) if u.sec is not None else b'\0' * ours_ext)
    is_float = bool(FLOAT_TYPES.match(it.type))
    is_string = bool(STRING_TYPES.match(it.type)) or it.name.startswith(('s_', 'u_')) or u.symbol.startswith('??_C@')
    # Size: ours may carry the padding the next symbol's alignment explains.
    if ours_ext < n_orig:
        flags.append('SIZE_SMALL')
        notes.append('ours %d < ghidra %d' % (ours_ext, n_orig))
    elif ours_ext - (u.next_align - 1) > n_orig and any(ub[n_orig:]):
        flags.append('SIZE_LARGE')
        notes.append('ours %d > ghidra %d' % (ours_ext, n_orig))
    oa = lowbit(it.addr)
    if orig_bss:
        if any(ub):
            flags.append('BSS_INIT')
        # Only an 8-byte scalar has a natural alignment the original's
        # address proves; an int array at a 16-aligned address is incidental.
        natural = 8 if EIGHT_BYTE_TYPES.match(it.type) else 1
        if u.align < min(oa, natural):
            flags.append('ALIGN')
            notes.append('align ours %d, original %d' % (u.align, oa))
        status = flags[0] if flags else 'MATCH'
        return status, flags, notes, -1, 0, (it.size if status == 'MATCH' else 0)
    # Relocation fields on both sides.
    orel = {va - it.addr for va in relocs_in(relocs, it.addr, it.addr + n_orig)}
    urel = {}
    if u.sec is not None:
        for r in u.sec.relocs:
            if u.off <= r.offset < u.off + n and r.type == coffobj.DIR32:
                urel[r.offset - u.off] = r
    masked = set()
    reloc_bad = raw = nulls = extra = unverified = strdiff = thunked = 0
    first_reloc = None
    for p in sorted(orel | set(urel)):
        if p + 4 > n:
            continue
        masked.update(range(p, p + 4))
        ov = struct.unpack_from('<I', ob, p)[0]
        uv = struct.unpack_from('<I', ub, p)[0]
        if p in orel and p in urel:
            kind, verdict, sym = pointer_verdict(pe, res, u.obj, urel[p], ov, uv)
        elif p in orel:
            kind, sym = 'none', '(no relocation)'
            verdict = 'raw' if uv == ov else 'null' if uv == 0 else 'bad'
        else:
            kind, verdict, sym = 'none', 'extra', '(ours only)'
        PTR_STATS[(kind, verdict)] += 1
        if verdict == 'bad':
            reloc_bad += 1
        elif verdict == 'string':
            strdiff += 1
        elif verdict == 'raw':
            raw += 1
        elif verdict == 'null':
            nulls += 1
        elif verdict == 'extra':
            extra += 1
        elif verdict == 'unverified':
            unverified += 1
        elif verdict == 'thunk':
            thunked += 1
        if verdict not in ('agree', 'thunk', 'unverified'):
            first_reloc = first_reloc or (p, sym)
    diffs = [i for i in range(n) if i not in masked and ob[i] != ub[i]]
    tail = [i for i in range(n, n_orig) if ob[i]]          # the part ours lacks
    all_zero = not any(ub[:n]) and not urel
    if all_zero and (any(ob) or orel):
        flags.insert(0, 'ZERO')
    if raw:
        notes.append('%d raw original addresses' % raw)
    if nulls:
        notes.append('%d null where the original points' % nulls)
    if extra:
        notes.append('%d pointers only ours has' % extra)
    if unverified:
        notes.append('%d pointers unverified' % unverified)
    if thunked:
        notes.append('%d pointers name the function where the original names its jump thunk' % thunked)
    if (reloc_bad or raw or nulls or extra) and 'ZERO' not in flags:
        flags.append('RELOC')
        if first_reloc:
            notes.append('first at +%d %s' % first_reloc)
    if strdiff or (is_string and diffs):
        flags.append('STRING')
    if diffs and not is_string:
        flags.append('VALUE')
        if is_float:
            notes.append('FLOAT')
    if diffs:
        notes.append('%d bytes differ' % len(diffs))
    first = diffs[0] if diffs else (n if tail else -1)
    order = ('ZERO', 'SPLIT', 'SIZE_SMALL', 'RELOC', 'STRING', 'VALUE', 'SIZE_LARGE', 'BSS_INIT', 'ALIGN')
    status = 'MATCH'
    for k in order:
        if k in flags:
            status = k
            break
    diff_bytes = len(diffs) + len(tail) + 4 * (reloc_bad + raw + nulls + extra + strdiff)
    if status == 'ZERO':
        diff_bytes = sum(1 for b in ob if b) + 4 * len(orel)
    matched = n_orig if status == 'MATCH' else max(0, n - len(diffs) - 4 * (reloc_bad + raw + nulls + extra + strdiff))
    return status, flags, notes, first, diff_bytes, matched


def relocs_in(sorted_relocs, lo, hi):
    i = bisect.bisect_left(sorted_relocs, lo)
    out = []
    while i < len(sorted_relocs) and sorted_relocs[i] < hi:
        out.append(sorted_relocs[i])
        i += 1
    return out


def module_of(tu):
    if tu.startswith('3rdparty/crashy/'):
        return 'Fog'                    # its namespace, Fog::_3rdparty::crashy (module_of_item)
    if tu.startswith(('Source/', '3rdparty/')):
        tu = tu.split('/', 1)[1]
    top = tu.split('/', 1)[0]
    if top == 'globals' and '/' in tu:
        top = tu.split('/', 1)[1]
        return 'Global' if top == 'globals' else top
    return top


def module_of_item(it):
    if it.ns:
        return it.ns.split('::')[0]
    tops = collections.Counter(r.split('::')[0] for r in it.refs if '::' in r)
    return tops.most_common(1)[0][0] if tops else '?'


def section_of(pe, va):
    s = pe.section_at(va)
    if s is None:
        return '?'
    if s.name == '.data' and va - pe.base - s.va >= s.rawsize:
        return '.bss'
    return s.name


def load_funcsyms():
    """Our function symbol -> original start (funcmap, then libmap), as audit.sym_map."""
    out, seen = {}, collections.defaultdict(set)
    names = {}          # game function name -> the TUs of ours that define it
    for path in (os.path.join(EVIDENCE, 'funcmap.tsv'), os.path.join(EVIDENCE, 'libmap.tsv')):
        if not os.path.exists(path):
            continue
        for line in open(path):
            if line.startswith('start\t'):
                continue
            c = line.rstrip('\n').split('\t')
            seen[c[4]].add(int(c[0], 16))
            if path.endswith('funcmap.tsv'):
                names.setdefault(c[2], set()).add(c[3])
    for k, v in seen.items():
        if len(v) == 1:
            out[k] = next(iter(v))
    return out, names


def dynamic_init(u):
    """Whether our object initialises the symbol at run time: a static
    local's `?$S<n>@` once-guard in the same function, or a global's
    `??__E` dynamic initialiser. The image then holds zero where the
    original's holds the table; the values arrive on first use."""
    m = MANGLED_STATIC_LOCAL.match(u.symbol)
    if m:
        tail = u.symbol[u.symbol.index('@') + 1:].rsplit('@4', 1)[0]
        return any(s is not None and s.name.startswith('?$S') and tail in s.name for s in u.obj.symbols)
    ident = our_identity(u.symbol)
    return bool(ident) and any(s is not None and s.name.startswith('??__E') and ident[0] in s.name
                               for s in u.obj.symbols)


_USED = None


def used_by_code(objs, u):
    """Whether code or data of ours relocates against this data symbol (a
    copy nothing references is dead weight, not a runtime fault)."""
    global _USED
    if _USED is None:
        _USED = set()
        for tu, o in objs.items():
            for sec in o.sections[1:]:
                if sec.name.startswith('.debug'):
                    continue
                for r in sec.relocs:
                    s = o.symbol(r.symbol)
                    _USED.add('%s|%s' % (tu, s.name) if s.storage == coffobj.C_STATIC else s.name)
    return u.uid in _USED


def real_literal_bytes(sym):
    """`__real@3ff0000000000000` -> its little-endian bytes."""
    h = sym[len('__real@'):]
    try:
        return bytes.fromhex(h)[::-1]
    except ValueError:
        return None


def float_constants(pe, items, paired_addr, objs, game_fns, dead):
    """Audit rows for the floating-point constants of .rdata no data symbol
    of ours that our code reads names (`dead`: those a symbol of ours
    names that nothing reads): our code reads them as compiler literals (`__real@`), so
    the check is whether the objects of the functions that read one hold a
    literal of exactly its bytes. A miss means our code computes with
    another constant: the float/double encoding class (a double emitted as
    a float, a rounded or truncated value). Only constants whose readers
    are compiled TUs of ours are judged; fld1/fldz serve 1.0 and 0.0
    without a literal, so those two never count as a miss."""
    lits = collections.defaultdict(set)
    for tu, o in objs.items():
        for s in o.symbols:
            if s is not None and s.name.startswith('__real@'):
                b = real_literal_bytes(s.name)
                if b:
                    lits[tu].add(b)
    out = []
    for it in items:
        if it.addr in paired_addr or it.kind != 'data' or section_of(pe, it.addr) != '.rdata':
            continue
        if not FLOAT_TYPES.match(it.type) or it.extent not in (4, 8):
            continue
        tus = set()
        for r in it.refs:
            tus |= game_fns.get(r, set())
        tus = {t for t in tus if t in objs}
        if not tus:
            continue
        want = pe.read(it.addr, it.extent)
        have = set().union(*(lits[t] for t in tus))
        value = struct.unpack('<d' if it.extent == 8 else '<f', want)[0]
        notes = ['%s %r read by %s' % ('double' if it.extent == 8 else 'float', value, ','.join(sorted(tus)))]
        if want in have or value in (0.0, 1.0):
            status, flags, matched, diff = 'MATCH', '', it.extent, 0
        else:
            status, flags, matched, diff = 'VALUE', 'VALUE', 0, it.extent
            alt = struct.pack('<f', value) if it.extent == 8 else struct.pack('<d', value)
            near = [b for b in have if len(b) == it.extent and
                    abs(struct.unpack('<d' if it.extent == 8 else '<f', b)[0] - value) <= abs(value) * 1e-6]
            if alt in have:
                notes.append('ours has it as a %s' % ('float' if it.extent == 8 else 'double'))
            elif near:
                notes.append('ours has a rounded %r' % struct.unpack('<d' if it.extent == 8 else '<f', near[0])[0])
            else:
                notes.append('no literal of that value in those objects')
            notes.append('FLOAT')
        if it.addr in dead:
            notes.append('ours also defines %s, which nothing reads' % dead[it.addr])
        out.append((it.addr, it.extent, it.extent, '.rdata', module_of_item(it), it.name, '', ','.join(sorted(tus)),
                    'const', 'tu-literal', it.type, status, flags, -1 if status == 'MATCH' else 0, diff, matched,
                    '; '.join(notes)))
    return out


def runtime_names():
    out = set()
    p = os.path.join(EVIDENCE, 'runtime.tsv')
    if os.path.exists(p):
        for line in open(p):
            c = line.rstrip('\n').split('\t')
            if len(c) > 1 and c[0] != 'start':
                out.add(c[1])
    return out


POINTERISH = re.compile(r'\*|pointer|PTR|FARPROC|pfn|fn', re.I)


def root_cause(r):
    """(likely root cause, runtime risk, why) for a row that does not MATCH.
    Root causes: `generator` (initialiser emission), `ghidra-extent`,
    `ghidra-type`. Runtime risk: yes when our code references the symbol
    and it holds wrong bytes; no for a copy nothing references."""
    status, typ, notes, kind = r[11], r[10], r[16], r[8]
    referenced = 'referenced by our code' in notes
    risk = 'yes' if referenced else 'no'
    if kind == 'interior':
        return ('generator', 'yes' if referenced else 'no',
                'interior label emitted as an object of its own; the original\'s name is an alias into its '
                'container, so a reader of ours sees a separate copy' + ('' if referenced else ' (nothing reads it)'))
    if 'address claimed by' in notes and 'COOKIE' in r[5]:
        return ('generator', 'no', 'a runtime (CRT) global the library defines too; ours shadows it')
    if status == 'RELOC':
        if 'raw original addresses' in notes:
            if POINTERISH.search(typ):
                return ('generator', risk, 'pointer initialiser to an address with no symbol emitted as a raw '
                        '1.14d address cast; label the target in Ghidra or emit &symbol')
            return ('ghidra-type', risk, 'a pointer slot of %s is typed as an integer or bytes, so the generator '
                    'emits the 1.14d address as a number; type that slot as a pointer' % typ)
        if 'null where the original points' in notes:
            return ('generator', risk, 'pointer initialiser dropped (null where the original points)')
        return ('generator', risk, 'pointer names another target than the original')
    if status == 'ZERO':
        if 'dynamic initialisation' in notes:
            return ('generator', 'low', 'the initialiser calls a constructor, so VC8 builds the table at run time '
                    '(zero in the image, a once-guard in the function); emit it as aggregate initialisation')
        if typ in ('undefined', 'undefined1') or r[1] == 1:
            return ('ghidra-extent', 'no', 'a one-byte undefined item where the original holds a wider value')
        return ('generator', risk, 'initialiser not emitted as static data (zero in our image): a constructor-'
                'style initialiser compiles to dynamic initialisation, or none was written')
    if kind == 'const' and 'no literal of that value' in notes:
        return ('generator', 'maybe', 'the readers\' objects hold no literal of this value: the constant is '
                'computed another way or another one is used; read the function')
    if kind == 'const' or (status == 'VALUE' and 'FLOAT' in notes):
        return ('generator', 'yes' if (kind == 'const' or referenced) else 'no',
                'floating-point value printed with too few digits (float needs %.9g, double %.17g); ' +
                ('the literal our code computes with differs' if kind == 'const' else 'the table differs'))
    if status in ('VALUE', 'STRING') and STRING_TYPES.match(typ or 'char') and r[13] >= 0:
        return ('ghidra-extent', 'no', 'string item runs past its terminator into bytes ours pads with zero')
    return ('generator', risk, 'bytes differ')


def write_fixes(rows, path, top=50):
    """The defects, largest first by original bytes, with a likely root cause:
    the `top` largest, then every smaller one that looks like a runtime fault.
    An interior label emitted as a copy nothing of ours references is dead
    weight, and its extent is its container's tail, not a size of its own:
    those are one summary row at the end instead of crowding the list."""
    bad = [r for r in rows if r[11] != 'MATCH']
    dead = [r for r in bad if r[8] == 'interior' and 'referenced by our code' not in r[16]]
    bad = [r for r in bad if r not in dead]
    bad.sort(key=lambda r: (-r[1], r[0]))
    with open(path, 'w') as fh:
        fh.write('rank\taddress\tbytes\tsection\tname\ttu\tstatus\troot_cause\truntime_risk\twhy\tnotes\n')
        # The largest by bytes, then every smaller one that looks like a runtime fault.
        listed = [r for k, r in enumerate(bad) if k < top or root_cause(r)[1] in ('yes', 'low', 'maybe')]
        for k, r in enumerate(listed, 1):
            cause, risk, why = root_cause(r)
            fh.write('%d\t%08x\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' % (
                k, r[0], r[1], r[3], r[5], r[7], r[11], cause, risk, why, r[16]))
        if dead:
            fh.write('-\t-\t-\t-\t(%d interior labels)\t-\tSPLIT\tgenerator\tno\t'
                     'emitted as objects of their own that nothing of ours references (%d .bss, %d '
                     'initialised): dead copies, see data-audit.tsv kind interior\t\n' % (
                         len(dead), sum(1 for r in dead if r[3] == '.bss'), sum(1 for r in dead if r[3] != '.bss')))


AUDIT_HEADER = ('address\tsize\tours_size\tsection\tmodule\tname\tsymbol\ttu\tkind\tmethod\ttype'
                '\tstatus\tflags\tfirst_diff\tdiff_bytes\tmatched_bytes\tnotes')
TREND_BUCKETS = [(1, 4, '1-4'), (5, 16, '5-16'), (17, 64, '17-64'), (65, 256, '65-256'),
                 (257, 1024, '257-1K'), (1025, 4096, '1K-4K'), (4097, 10 ** 12, '4K+')]


def audit_main(args):
    import subprocess
    pe = peimage.PEImage(REFERENCE)
    relocs = sorted(pe.relocs)
    items, version = load_extents(args.extents)
    objdirs = []
    for spec in args.obj:
        root, _, sub = spec.partition('=')
        if os.path.isdir(root):
            objdirs.append((root, sub))
    ours, dups, objs = load_ours(objdirs)
    datamap = load_datamap(args.datamap)
    pairs = pair(ours, items, datamap)
    funcsyms, game_fns = load_funcsyms()
    for n in runtime_names():
        game_fns.pop(n, None)
    res = Resolver(pe, pairs, ours, funcsyms)

    rows = []
    paired_addr = collections.Counter()
    by_item = {}
    dead_float = {}
    for uid, (it, _m, _n) in pairs.items():
        by_item.setdefault(id(it), ours[uid])
    for uid, (it, method, pnote) in sorted(pairs.items(), key=lambda kv: (kv[1][0].addr, kv[0])):
        u = ours[uid]
        sym = u.symbol
        if (it.kind == 'data' and FLOAT_TYPES.match(it.type) and it.extent in (4, 8)
                and section_of(pe, it.addr) == '.rdata' and not used_by_code(objs, u)):
            # A constant our code never reads by name: it reads a literal of
            # its own (float_constants judges that one); this copy is dead.
            dead_float[it.addr] = sym
            continue
        paired_addr[it.addr] += 1
        status, flags, notes, first, diff_bytes, matched = judge(pe, relocs, it, u, res)
        if pnote:
            notes.append(pnote)
        if it.kind == 'interior':
            # Ours is an object of its own unless it sits where the
            # container's symbol of ours puts that offset.
            cu = by_item.get(id(it.parent)) if it.parent is not None else None
            cont = cu is not None and cu.sec is u.sec and cu.off + (it.addr - it.parent.addr) == u.off
            if not cont:
                flags = ['SPLIT'] + flags
                if status != 'ZERO':
                    status = 'SPLIT'
                matched = 0
            notes.append('interior label of %s +%d%s' % (
                it.parent.name if it.parent is not None else '?', it.addr - it.parent.addr if it.parent is not None else 0,
                ', laid out inside it' if cont else ', emitted as an object of its own'))

        if status == 'ZERO' and u.bss and dynamic_init(u):
            notes.append('dynamic initialisation: our code builds it at run time')
        if status != 'MATCH':
            notes.append('referenced by our code' if used_by_code(objs, u) else 'nothing of ours references it')
        kind = 'literal' if (is_literal(sym) or it.kind == 'literal') else it.kind
        rows.append((it.addr, it.extent, u.extent, section_of(pe, it.addr), module_of(u.tu), it.name, sym, u.tu,
                     kind, method, it.type, status, ','.join(flags), first, diff_bytes, matched, '; '.join(notes)))
    consts = float_constants(pe, items, paired_addr, objs, game_fns, dead_float)
    for r in consts:
        paired_addr[r[0]] += 1
    rows += consts
    rows.sort(key=lambda r: (r[0], r[6]))
    # Twice-paired addresses: two of ours claim one original item.
    twice = {a for a, k in paired_addr.items() if k > 1}
    rows = [r[:16] + ((r[16] + '; ' if r[16] else '') + 'address claimed by %d symbols' % paired_addr[r[0]],)
            if r[0] in twice else r for r in rows]

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    with open(args.out, 'w') as fh:
        fh.write(AUDIT_HEADER + '\n')
        for r in rows:
            fh.write('%08x\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%d\t%s\n' % r)

    # The headline: data the game's own code reads. An item no paired symbol
    # covers counts as uncompared when a game function references it; one
    # only the runtime references is the runtime's, out as in the function
    # figure; imports and compiler EH/RTTI data are the linker's and compiler's.
    covered = set(paired_addr)
    uncompared = []
    runtime_items = []
    excluded = collections.Counter()
    for it in items:
        if it.addr in covered:
            continue
        if it.kind in ('import', 'compiler'):
            excluded[it.kind] += 1
            continue
        if it.kind == 'interior':
            continue                 # its bytes are its container's
        if any(r in game_fns for r in it.refs):
            uncompared.append(it)
        else:
            runtime_items.append(it)
    # An interior label's bytes are its container's: it is judged (is ours
    # laid out inside the container, or an object of its own?) but its
    # bytes stay out of the totals, which would count them twice.
    named = [r for r in rows if r[8] not in ('literal', 'import', 'compiler', 'interior')]
    interior = [r for r in rows if r[8] == 'interior']
    lit = [r for r in rows if r[8] == 'literal']
    tree = subprocess.run(['git', '-C', REPO, 'describe', '--always', '--dirty', '--tags'],
                          capture_output=True, text=True).stdout.strip() or 'unknown'

    def init(rs):
        return [r for r in rs if r[3] != '.bss']

    def fig(rs, unc=()):
        ri = init(rs)
        m = [r for r in ri if r[11] == 'MATCH']
        total_b = sum(r[1] for r in ri) + sum(i.extent for i in unc if section_of(pe, i.addr) != '.bss')
        total_n = len(ri) + sum(1 for i in unc if section_of(pe, i.addr) != '.bss')
        return len(m), total_n, sum(r[1] for r in m), total_b, sum(r[15] for r in ri)

    lines = ['tree %s; Ghidra v%s extents (%s); %d objects (%s)' % (
        tree, version, os.path.basename(args.extents), len(objs),
        ' + '.join('%s/' % (s or 'build/obj') for _r, s in objdirs))]
    mn, tn, mb, tb, ib = fig(named, uncompared)
    lines.append('Initialised game data: %d of %d symbols MATCH (%.2f%%), %d of %d bytes (%.2f%%); '
                 'bytes equal in place %d (%.2f%%)' % (
                     mn, tn, 100.0 * mn / max(1, tn), mb, tb, 100.0 * mb / max(1, tb), ib, 100.0 * ib / max(1, tb)))
    unc_init = [i for i in uncompared if section_of(pe, i.addr) != '.bss']
    lines.append('  uncompared: %d game items (%d bytes) that no data symbol of ours pairs with' % (
        len(unc_init), sum(i.extent for i in unc_init)))
    lines.append('  left out: %d runtime-only items (%d bytes), %d import slots, %d compiler EH/RTTI items' % (
        len(runtime_items), sum(i.extent for i in runtime_items), excluded['import'], excluded['compiler']))
    bss = [r for r in named if r[3] == '.bss']
    bm = [r for r in bss if r[11] == 'MATCH']
    lines.append('Uninitialised (.bss), size and alignment only: %d of %d symbols agree, %d of %d bytes' % (
        len(bm), len(bss), sum(r[1] for r in bm), sum(r[1] for r in bss)))
    sp = [r for r in interior if r[11] != 'MATCH']
    lines.append('Interior labels (a name inside a larger Ghidra item): %d paired, %d emitted as an object of '
                 'their own (%d .bss, %d initialised, %d of those all zero)' % (
                     len(interior), len(sp), sum(1 for r in sp if r[3] == '.bss'),
                     sum(1 for r in sp if r[3] != '.bss'), sum(1 for r in sp if r[11] == 'ZERO')))
    lm = [r for r in lit if r[11] == 'MATCH']
    lines.append('Compiler literals (__real@, ??_C@, ??_7) paired by datamap: %d of %d MATCH, %d of %d bytes' % (
        len(lm), len(lit), sum(r[1] for r in lm), sum(r[1] for r in lit)))
    lines.append('status of the initialised game data compared (symbols, original bytes):')
    st = collections.Counter()
    stb = collections.Counter()
    for r in init(named):
        st[r[11]] += 1
        stb[r[11]] += r[1]
    for k in ('MATCH', 'ZERO', 'SIZE_SMALL', 'RELOC', 'STRING', 'VALUE', 'SIZE_LARGE', 'BSS_INIT', 'ALIGN'):
        lines.append('  %-11s %6d %9d' % (k, st.get(k, 0), stb.get(k, 0)))
    fl = collections.Counter(f for r in init(named) for f in r[12].split(',') if f)
    lines.append('flags (a symbol may carry several): ' + ', '.join('%s %d' % kv for kv in fl.most_common()))
    raw = sum(1 for r in named if 'raw original address' in r[16])
    lines.append('symbols holding a raw 1.14d address where the original relocates: %d' % raw)
    kinds = sorted({k for k, _v in PTR_STATS})
    lines.append('pointer fields compared, by what ours names: ' + '; '.join(
        '%s %s' % (k, ', '.join('%s %d' % (v, n) for (kk, v), n in sorted(PTR_STATS.items()) if kk == k))
        for k in kinds))
    lines.append('by module (initialised; symbols MATCH/total, bytes MATCH/total):')
    for mod in sorted({r[4] for r in named}):
        rs = init([r for r in named if r[4] == mod])
        if not rs:
            continue
        m = [r for r in rs if r[11] == 'MATCH']
        lines.append('  %-12s %5d/%-5d %8d/%-8d %6.2f%%' % (mod, len(m), len(rs), sum(r[1] for r in m),
                                                           sum(r[1] for r in rs),
                                                           100.0 * sum(r[1] for r in m) / max(1, sum(r[1] for r in rs))))
    lines.append('by section (symbols MATCH/total, bytes MATCH/total):')
    for sec in ('.rdata', '.data', '.bss'):
        rs = [r for r in named if r[3] == sec]
        m = [r for r in rs if r[11] == 'MATCH']
        lines.append('  %-7s %5d/%-5d %8d/%-8d' % (sec, len(m), len(rs), sum(r[1] for r in m), sum(r[1] for r in rs)))
    pm = collections.Counter(r[9] for r in rows)
    lines.append('floating-point constants judged by the literals of their readers\' objects: %d, %d MATCH' % (
        len(consts), sum(1 for r in consts if r[11] == 'MATCH')))
    lines.append('pairings: ' + ', '.join('%s %d' % kv for kv in pm.most_common()) +
                 '; data symbols of ours unpaired %d (%d of them compiler literals); defined twice %d' % (
                     len(ours) - len(pairs), sum(1 for k, u in ours.items() if k not in pairs and is_literal(u.symbol)),
                     len(dups)))
    dis = sum(1 for r in rows if 'datamap implies' in r[16])
    lines.append('name pair disagrees with the datamap address: %d' % dis)
    with open(args.out + '.summary', 'w') as fh:
        fh.write('\n'.join(lines) + '\n')
    print('\n'.join(lines))

    write_fixes(rows, args.fixes)

    # datatrend: MATCH rate by Ghidra size.
    trend = ['# datatrend %s' % args.label if args.label else '# datatrend',
             'bucket\tsymbols\tmatch_pct\tbytes_pct\tzero\tsize_small\treloc\tvalue']
    ri = init(named)
    tot = sum(r[1] for r in ri)
    for lo, hi, name in TREND_BUCKETS:
        rs = [r for r in ri if lo <= r[1] <= hi]
        c = collections.Counter(r[11] for r in rs)
        trend.append('%s\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t%d' % (
            name, len(rs), 100.0 * c['MATCH'] / max(1, len(rs)), 100.0 * sum(r[1] for r in rs) / max(1, tot),
            c['ZERO'], c['SIZE_SMALL'], c['RELOC'], c['VALUE'] + c['STRING']))
    with open(args.trend, 'w') as fh:
        fh.write('\n'.join(trend) + '\n')


def default_snapshot():
    for p in (os.environ.get('ORACLE_GHIDRA_SNAPSHOT'),
              os.path.expanduser('~/code/ts/gr-daemon/project/.ghidra-mcp/codegen-snapshot'),
              os.path.expanduser('~/code/ts/ghidra-reconstruct/project/.ghidra-mcp/codegen-snapshot')):
        if p and os.path.exists(os.path.join(p, 'globals.ndjson')):
            return p
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest='cmd')
    e = sub.add_parser('extents')
    e.add_argument('--snapshot', default=default_snapshot())
    e.add_argument('--out', default=EXTENTS)
    a = sub.add_parser('audit')
    a.add_argument('--extents', default=EXTENTS)
    a.add_argument('--obj', action='append', default=None,
                   help='object root, optionally ROOT=PREFIX for TU names (repeatable); default build/obj '
                        'and the whole-image build\'s globals/')
    a.add_argument('--datamap', default=os.path.join(EVIDENCE, 'datamap.tsv'))
    a.add_argument('--out', default=None, help='default evidence/data-audit.tsv (an experiment: beside its objects)')
    a.add_argument('--trend', default=None)
    a.add_argument('--fixes', default=None)
    a.add_argument('--label', default='')
    args = ap.parse_args()
    if args.cmd == 'extents':
        if not args.snapshot:
            sys.exit('no codegen snapshot found; pass --snapshot DIR (holding globals.ndjson)')
        extents_main(args)
    elif args.cmd in (None, 'audit'):
        if args.cmd is None:
            args = a.parse_args([])
        if args.out is None:
            args.out = os.path.join(BUILD if EXPERIMENT else EVIDENCE, 'data-audit.tsv')
        elif EXPERIMENT and os.path.realpath(os.path.dirname(args.out)) == os.path.realpath(EVIDENCE):
            sys.exit('refusing to write an experiment data audit into evidence/')
        if args.obj is None:
            # Source/globals/ holds most of the data and is not compiled into build/obj:
            # the whole-image build (recon/runs/build-vc8.sh) compiles it with
            # the same command line into build-exe/obj/Source/globals.
            args.obj = [os.path.join(BUILD, 'obj'),
                        os.path.join(ORACLE, 'build-exe', 'obj', 'Source', 'globals') + '=Source/globals']
            for spec in args.obj:
                if not os.path.isdir(spec.partition('=')[0]):
                    print('warning: %s missing; its data is not compared' % spec.partition('=')[0], file=sys.stderr)
        if args.trend is None:
            args.trend = os.path.join(os.path.dirname(args.out), 'datatrend.tsv')
        if args.fixes is None:
            args.fixes = os.path.join(os.path.dirname(args.out), 'data-fixes.tsv')
        if not os.path.exists(args.extents):
            sys.exit('%s missing: run `oracle data extents` first' % args.extents)
        audit_main(args)


if __name__ == '__main__':
    main()
