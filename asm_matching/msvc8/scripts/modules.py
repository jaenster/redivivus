"""modules.py - the tree's modules, as the CMake build links them.

Source/modules.cmake (generated from project.json) names every module, its
directory and its data unit, in the executable's link order (the 1.14d one).
The CMake build makes one static library per module from the TUs under its
directory plus its data unit (Source/globals/<m>.cpp), a root library
(d2_root) from every unit no module owns, and links them whole in that order.
recon/runs/build-vc8.sh links the VC8 objects the same way, and the oracle
models which copy of a COMDAT the link keeps (ours.link_kept) by the same
order, so the three agree.

    modules.py order  < tus       module<TAB>tu, in link order (tu = path without .cpp)
    modules.py includes <list>    one /INCLUDE symbol per object (list: module<TAB>tu<TAB>obj)

The order inside a library is the TUs' paths through sort(1), the order a
directory glob hands CMake, then the data unit, which CMake appends.
"""
import collections
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

ROOT_LIB = 'root'          # CMake's d2_root: what no module owns
SKIP_KINDS = ('headers-only',)
SKIP_MODULES = ('libbacktrace',)   # GCC only; CMake leaves it out of the module libraries too

_cache = {}


def load(repo):
    """[(name, dir, data, kind)] in link order, from <repo>/Source/modules.cmake."""
    repo = os.path.realpath(repo)
    if repo in _cache:
        return _cache[repo]
    path = os.path.join(repo, 'Source', 'modules.cmake')
    order, attrs = [], collections.defaultdict(dict)
    with open(path) as fh:
        for line in fh:
            m = re.match(r'\s*set\(D2_MODULES\s+(.*)\)\s*$', line)
            if m:
                order = m.group(1).split()
                continue
            m = re.match(r'\s*set\(D2_MODULE_(\w+?)_(DIR|DATA|KIND)\s+"(.*)"\)\s*$', line)
            if m:
                attrs[m.group(1)][m.group(2)] = m.group(3)
    if not order:
        raise SystemExit('%s: no D2_MODULES' % path)
    mods = [(n, attrs[n].get('DIR', ''), attrs[n].get('DATA', ''), attrs[n].get('KIND', '')) for n in order]
    _cache[repo] = mods
    return mods


def linked(repo):
    """The module libraries the executable links, in order, the root library last."""
    return [n for n, _d, _data, kind in load(repo)
            if kind not in SKIP_KINDS and n not in SKIP_MODULES] + [ROOT_LIB]


def module_of(tu, repo):
    """The library a TU goes into: the module whose data unit it is, else the
    module with the longest directory above it, else the root library."""
    mods = load(repo)
    for n, _d, data, _k in mods:
        if data and tu + '.cpp' == data and n not in SKIP_MODULES:
            return n
    best, blen = ROOT_LIB, -1
    for n, d, _data, kind in mods:
        if d and tu.startswith(d + '/') and len(d) > blen and kind not in SKIP_KINDS and n not in SKIP_MODULES:
            best, blen = n, len(d)
    return best


def sort1(tus):
    """The TUs through sort(1), as the build scripts sort paths (the locale's
    collation, case-blind in en_* locales); plain sorted() if sort(1) fails."""
    tus = sorted(set(tus))
    r = subprocess.run(['sort'], input=''.join(t + '\n' for t in tus), capture_output=True, text=True)
    out = [t for t in r.stdout.split('\n') if t]
    if r.returncode != 0 or sorted(out) != tus:
        return tus
    return out


def libraries(tus, repo):
    """[(module, [tu, ...])] in link order; empty libraries left out."""
    data = {d[:-4]: n for n, _dir, d, _k in load(repo) if d.endswith('.cpp')}
    groups = collections.defaultdict(list)
    for tu in tus:
        groups[module_of(tu, repo)].append(tu)
    out = []
    for n in linked(repo):
        if not groups.get(n):
            continue
        body = [t for t in sort1(groups[n]) if data.get(t) != n]
        tail = [t for t in groups[n] if data.get(t) == n]
        out.append((n, body + tail))
    return out


def link_order(tus, repo):
    """Every TU in the order its object reaches the linker."""
    return [t for _n, group in libraries(tus, repo) for t in group]


SCN_LNK_INFO, SCN_LNK_REMOVE, SCN_MEM_DISCARDABLE = 0x200, 0x800, 0x02000000
ENTRY_ROOTS = ('_WinMain@16',)     # what the C runtime's entry calls into ours


def kept_sections(objs, roots=ENTRY_ROOTS):
    """{(obj index, section number)} - the sections /OPT:REF keeps when the
    objects are linked whole in this order: every non-COMDAT section that
    reaches the image, the definitions of `roots`, and whatever those refer
    to, a COMDAT that several objects define resolving to the first object's
    copy (link.exe keeps the first definition it reads), an associative
    COMDAT kept with the section it belongs to."""
    import coffobj
    first = {}                                  # external symbol -> (obj, section)
    for i, o in enumerate(objs):
        for sym in o.symbols:
            if sym is not None and sym.storage == coffobj.C_EXTERNAL and sym.section > 0:
                first.setdefault(sym.name, (i, sym.section))
    assoc = collections.defaultdict(list)
    work = []
    for i, o in enumerate(objs):
        for sec in o.sections[1:]:
            if sec.comdat == 5 and sec.comdat_assoc:
                assoc[(i, sec.comdat_assoc)].append((i, sec.number))
            elif not sec.flags & coffobj.SCN_LNK_COMDAT and sec.size and \
                    not sec.flags & (SCN_LNK_INFO | SCN_LNK_REMOVE | SCN_MEM_DISCARDABLE):
                work.append((i, sec.number))
    work.extend(first[r] for r in roots if r in first)
    kept = set()
    while work:
        node = work.pop()
        if node in kept:
            continue
        kept.add(node)
        i, n = node
        o = objs[i]
        work.extend(assoc.get(node, ()))
        for r in o.sections[n].relocs:
            sym = o.symbols[r.symbol] if r.symbol < len(o.symbols) else None
            if sym is None:
                continue
            if sym.storage == coffobj.C_EXTERNAL:
                t = first.get(sym.name)
                if t is not None:
                    work.append(t)
            elif sym.section > 0:
                work.append((i, sym.section))
    return kept


def include_symbols(objects, libs=None):
    """{obj path: (symbol, how)} - for each object, an external symbol that no
    object before it defines, for /INCLUDE:<symbol>, which makes link.exe load that library
    member as if the object were named on the command line. VC8's link.exe has
    no /WHOLEARCHIVE; this is its equivalent, provided the symbol forces nothing
    the whole-object link would not keep. The symbol is one in a section that
    link keeps when it links the objects whole (kept_sections):
      data      in a non-COMDAT section, which a loaded object always keeps
      comdat    else in a COMDAT section (/Gy puts every function in one)
                reachable from the entry or from such data, one /OPT:ICF
                cannot fold into another, a constant that refers to nothing
                (a string literal) before code: a forced symbol is a root of
                /OPT:REF's walk, and the walk's order decides which copy of a
                folded group survives
      foldable  else a COMDAT ICF may fold: forcing it can move the folded
                group to this object's place; the .map comparison shows it
    An object of which the whole-object link keeps nothing - (None, 'empty'),
    e.g. a data unit with no data - is not forced: left out it changes nothing.
    One that contributes kept sections but defines no symbol of its own gets
    (None, 'none') and must be named as an object."""
    import coffobj
    objs = []
    for path in objects:
        with open(path, 'rb') as fh:
            objs.append(coffobj.CoffObject(fh.read(), path))
    kept = kept_sections(objs)
    libs = libs or ['' for _ in objects]
    first = {}
    inlib = collections.Counter()               # (library, symbol) -> its members defining it
    for i, o in enumerate(objs):
        for sym in o.symbols:
            if sym is not None and sym.storage == coffobj.C_EXTERNAL and sym.section > 0:
                first.setdefault(sym.name, i)
                inlib[(libs[i], sym.name)] += 1
    # /OPT:ICF folds identical COMDATs, and which copy survives follows what
    # reached it first: forcing one of a foldable group moves the group. So a
    # COMDAT symbol is chosen from a section no other kept section equals
    # (same bytes, flags and relocation targets).
    def shape(i, n):
        o = objs[i]
        sec = o.sections[n]
        tgt = []
        for r in sec.relocs:
            sym = o.symbols[r.symbol] if r.symbol < len(o.symbols) else None
            if sym is None:
                tgt.append((r.offset, r.type, None))
            elif sym.storage == coffobj.C_EXTERNAL:
                tgt.append((r.offset, r.type, sym.name))
            else:
                tgt.append((r.offset, r.type, (i, sym.section, sym.value)))
        return (sec.flags, sec.data, tuple(tgt))
    shapes = collections.Counter()
    for i, n in kept:
        if objs[i].sections[n].flags & coffobj.SCN_LNK_COMDAT:
            shapes[shape(i, n)] += 1
    out = {}
    for i, (path, o) in enumerate(zip(objects, objs)):
        cands = []
        for sym in o.symbols:
            if sym is None or sym.storage != coffobj.C_EXTERNAL or sym.section <= 0:
                continue
            # No earlier object defines it (else the symbol is resolved before
            # this member is searched), and no other member of its library
            # (lib.exe indexes one member per symbol).
            if first[sym.name] != i or inlib[(libs[i], sym.name)] != 1 or (i, sym.section) not in kept:
                continue
            comdat = bool(o.sections[sym.section].flags & coffobj.SCN_LNK_COMDAT)
            sec = o.sections[sym.section]
            leaf = not (sec.flags & coffobj.SCN_CNT_CODE) and not sec.relocs
            foldable = comdat and shapes[shape(i, sym.section)] > 1
            cands.append((comdat, not leaf, foldable, sym.name))
        if cands:
            comdat, _code, foldable, name = min(cands)
            if foldable:
                out[path] = (name, 'foldable')
                continue
            out[path] = (name, 'comdat' if comdat else 'data')
        elif any(k[0] == i for k in kept):
            out[path] = (None, 'none')
        else:
            out[path] = (None, 'empty')
    return out


def main(argv):
    repo = os.environ.get('D2_TREE') or os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.dirname(os.path.abspath(__file__)))))
    if len(argv) >= 1 and argv[0] == 'order':
        tus = [l.strip() for l in sys.stdin if l.strip()]
        for n, group in libraries(tus, repo):
            for t in group:
                print('%s\t%s' % (n, t))
    elif len(argv) == 2 and argv[0] == 'includes':
        rows = [l.rstrip('\n').split('\t') for l in open(argv[1]) if l.strip()]
        syms = include_symbols([r[2] for r in rows], [r[0] for r in rows])
        for r in rows:
            sym, how = syms[r[2]]
            print('%s\t%s\t%s\t%s' % (r[0], r[1], sym or '-', how))
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main(sys.argv[1:])
