#!/usr/bin/env python3
"""libmap.py - pair the original's statically linked library code with the libraries we link.

    libmap.py [--exe-map FILE] [--out evidence/libmap.tsv]

The original links VC8's static runtime (/MT: libcmt.lib, libcpmt.lib; SP1,
evidence/toolchain.md), and so does our image. That code is not in the tree,
so funcmap.py leaves it unpaired; here every original function the funcmap did
not pair is looked up among the functions of those two libraries, as shipped
with the toolchain, and the pairs go to evidence/libmap.tsv in the funcmap's
format (tu = <library>/<member>). The audit judges them with the same
comparer, relocations masked the same way, and counts them in its totals: a
library function MATCHes only if its masked bytes and every comparable
reference agree, as any other.

Tiers, in the order they run:

  lib-exact   identical masked code, unique among the original's unpaired
              functions (three instructions or more) and among the library
              functions. Code the libraries hold more than once (a stub in
              seven members, printf's write_char in every printf variant) is
              taken, inside the run of library code, as the one copy our image
              links (detail: "one of N identical"), else left to the others.
  lib-order   the linker keeps a member's functions in member order, minus
              the ones nobody references: next to a paired function of a
              member, the original function right after (or before) it is
              paired with the nearest unpaired functions of that member, the
              most similar one, if clearly more similar than the others and
              above LIB_ORDER_THRESHOLD.
  lib-similar inside the run of library code (first to last lib-exact pair),
              an original whose most similar library function is clearly the
              most similar (LIB_SIMILAR_*): the same function from another
              revision of the runtime's source, which the audit then shows as
              not matching.
  lib-votes   code that cannot tell twins apart (memcpy and memmove are one
              source): at the aligned call sites of everything already paired,
              a call of our symbol lands on one original address; paired there
              when the sites agree (VOTES_*) and the code bears it out.

None of this decides a status: the audit judges each pair as any other, so a
wrong pair costs a MATCH, it cannot make one.

A pair may cover several carved functions of the original: refimage starts a
function at every call target, so a CRT function's __finally funclet, which
its own body calls, is carved apart, while the library's object holds it in
the one function. A pair's end is then the end of the last piece, and the
audit compares over the joined span (compare.ref_view_span). Pieces are
joined only while they tile the span exactly and are unpaired.

`linked` in the detail says our image (the map of build-exe/d2recon-vc8.exe,
written by recon/runs/build-vc8.sh) links that function; a pair it does not
link is still the same library object's code, so it is judged the same way.
Without that map lib-votes and the linked-copy rule of lib-exact pair nothing.

A summary goes to <out>.summary: pairs per tier, and what stays unpaired.
"""
import argparse
import collections
import difflib
import os
import pickle
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import audit      # noqa: E402
import coffobj    # noqa: E402
import compare    # noqa: E402
import objfuncs   # noqa: E402
import ours as ourslib  # noqa: E402

from oracle_config import BUILD, EXPERIMENT, ORACLE, REFCACHE  # noqa: E402

LIBS = ('libcmt.lib', 'libcpmt.lib')
LIBVC = 'sp1'                   # the runtime the original links: evidence/toolchain.md
DECODER = 2                     # bump when how library functions are decoded changes
EXACT_MIN_INSNS = 3
LIB_ORDER_THRESHOLD = 0.60      # minimum similarity of the neutral listings for a lib-order pair
LIB_ORDER_MARGIN = 0.10         # ... and its lead over the next best candidate
MAX_PIECES = 4                  # carved functions one library function may span
LIB_SIMILAR_THRESHOLD = 0.80    # lib-similar: minimum similarity ...
LIB_SIMILAR_MARGIN = 0.05       # ... and its lead over the most similar other function
VOTES_MIN = 2                   # lib-votes: aligned call sites that must agree ...
VOTES_SHARE = 0.75              # ... and their share of all
NEIGHBOURS = 3                  # unpaired member functions looked at on each side
PAD = 16                        # alignment padding allowed between two members' functions


def lib_dir():
    prefix = os.environ.get('VC8_WINEPREFIX') or os.path.join(
        os.environ.get('VC8_TOOLCHAIN', os.path.expanduser('~/q2-dev/asm-matching-toolchain')), 'wine-vc8')
    return os.path.join(prefix, 'drive_c', 'vc8', LIBVC, 'lib')


def _member_ids(names):
    """Archive member path -> short id: the base name without .obj, or the
    last two path components where two members share a base name."""
    base = collections.Counter(n.replace('\\', '/').rsplit('/', 1)[-1].lower() for n in names)
    out = {}
    for n in names:
        parts = n.replace('\\', '/').split('/')
        b = parts[-1]
        short = '/'.join(parts[-2:]) if base[b.lower()] > 1 else b
        out[n] = short[:-4] if short.lower().endswith('.obj') else short
    return out


def load_lib_functions():
    """Every function of the libraries, as ours.OurFunc with tu '<lib>/<member id>'.
    Cached in the reference cache, keyed by the libraries' mtimes."""
    d = lib_dir()
    paths = [os.path.join(d, lib) for lib in LIBS]
    stamp = (tuple(os.path.getmtime(p) for p in paths), DECODER, ourslib.DECODER)
    cache = os.path.join(REFCACHE, 'libfuncs-%s.pkl' % LIBVC)
    if os.path.exists(cache):
        try:
            with open(cache, 'rb') as fh:
                st, fns = pickle.load(fh)
            if st == stamp:
                return fns
        except Exception:
            pass
    fns = []
    for lib, path in zip(LIBS, paths):
        members = [(n, b) for n, b in coffobj.read_archive(open(path, 'rb').read())
                   if b[:4] != b'\0\0\xff\xff' and len(b) >= 20]
        ids = _member_ids([n for n, _b in members])
        for name, body in members:
            try:
                fl = objfuncs.functions(name, body, labels=True)
            except ValueError:
                continue                # not an i386 COFF object (an /GL or anonymous member)
            for k, f in enumerate(fl):
                fns.append(ourslib.OurFunc('%s/%s' % (lib, ids[name]), k, f))
    os.makedirs(REFCACHE, exist_ok=True)
    tmp = '%s.%d.tmp' % (cache, os.getpid())
    with open(tmp, 'wb') as fh:
        pickle.dump((stamp, fns), fh, protocol=pickle.HIGHEST_PROTOCOL)
    os.replace(tmp, cache)
    return fns


_MAPLINE = re.compile(r'^\s*[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s+(?:f\s+)?(?:i\s+)?(\S+):(\S+)$')


def linked_symbols(path):
    """(member base name lowercased, symbol) of every library symbol the
    linker map lists. Empty when there is no map."""
    out = set()
    if path and os.path.exists(path):
        for line in open(path, errors='replace'):
            m = _MAPLINE.match(line.rstrip())
            if m:
                out.add((m.group(4).lower(), m.group(1)))
    return out


def is_linked(f, linked):
    member = f.tu.split('/', 1)[1].rsplit('/', 1)[-1].lower() + '.obj'
    return (member, f.symbol) in linked


def key_of(view):
    return (len(view), b''.join(i.masked for i in view))


class LibMatcher:
    def __init__(self, model, paired, fns, linked):
        self.model = model
        self.starts = sorted(model)
        self.memo = ourslib.PairMemo('libmap-votes', BUILD)
        self.index = {s: k for k, s in enumerate(self.starts)}
        self.taken = set(paired)            # original starts paired by the funcmap, or here
        self.fns = fns
        self.linked = linked
        self.views = {}
        self.map = {}                       # f.key -> (start, end, method, detail)
        self.by_member = collections.defaultdict(list)
        for f in fns:
            self.by_member[f.tu].append(f)
        for v in self.by_member.values():
            v.sort(key=lambda f: f.order)
        self.pos = {f.key: k for v in self.by_member.values() for k, f in enumerate(v)}
        self.byname = {f.key: f for f in fns}

    def our(self, f):
        v = self.views.get(f.key)
        if v is None:
            v = self.views[f.key] = compare.our_view(f, {})
        return v

    def spans(self, s, limit=None):
        """Ends of the runs of 1..MAX_PIECES unpaired carved functions starting at s."""
        if s in self.taken:
            return
        e = s
        for n in range(MAX_PIECES):
            if n and (e not in self.model or e in self.taken):
                return
            e = self.model[e][0].end
            if limit is not None and e - s > limit:
                return
            yield e

    def take(self, f, s, e, method, detail):
        pieces = compare.span_pieces(self.model, s, e)
        if any(p in self.taken for p in pieces) or f.key in self.map:
            return False
        self.taken.update(pieces)
        if len(pieces) > 1:
            detail = (detail + '; ' if detail else '') + '%d pieces' % len(pieces)
        if is_linked(f, self.linked):
            detail = (detail + '; ' if detail else '') + 'linked'
        self.map[f.key] = (s, e, method, detail)
        return True

    def by_exact(self, extent=None):
        lib = collections.defaultdict(list)
        for f in self.fns:
            v = self.our(f)
            if len(v) >= EXACT_MIN_INSNS:
                lib[key_of(v)].append(f)
        # Only spans whose instruction count and length some library function
        # has are worth rendering.
        shapes = {(k[0], len(k[1])) for k in lib}
        limit = max(f.size for f in self.fns)
        ref = collections.defaultdict(list)
        for s in self.starts:
            for e in self.spans(s, limit=limit):
                pieces = compare.span_pieces(self.model, s, e)
                insns = [i for p in pieces for i in self.model[p][0].insns]
                if (len(insns), sum(i.size for i in insns)) in shapes:
                    ref[key_of(compare.ref_view_span(s, e, self.model))].append((s, e))
        n = 0
        # Longest code first, so a function and the funclet carved off it pair
        # as one. Code the libraries hold more than once (a stub such as
        # _invoke_watson, in seven members; printf's write_char in each
        # printf variant) names no member by itself: with `extent`, inside the
        # run of library code, the one copy our image links is taken; else it
        # is left to lib-order.
        for k in sorted(set(ref) & set(lib), key=lambda k: -len(k[1])):
            if len(ref[k]) != 1:
                continue
            s, e = ref[k][0]
            if len(lib[k]) == 1:
                n += self.take(lib[k][0], s, e, 'lib-exact', '')
            elif extent and extent[0] <= s < extent[1]:
                ours = [f for f in lib[k] if is_linked(f, self.linked) and f.key not in self.map]
                if len(ours) == 1:
                    n += self.take(ours[0], s, e, 'lib-exact', 'one of %d identical' % len(lib[k]))
        return n

    def by_votes(self, pairs):
        """Where the libraries hold the same code twice (memcpy and memmove
        are one source), or two near-twins (sin and cos), code cannot tell
        them apart; their callers can. pairs: (original start, end, our
        function) of everything paired so far. At their aligned call sites a
        call of our symbol X lands on one original address; X is paired there
        when at least VOTES_MIN sites agree, VOTES_SHARE of them. Our symbol
        is the library function our image links under that name."""
        by_sym = {}
        for f in self.fns:
            if f.key not in self.map and is_linked(f, self.linked):
                by_sym.setdefault(f.symbol, []).append(f)
        by_sym = {k: v[0] for k, v in by_sym.items() if len(v) == 1}
        # A call binds to a definition in the caller's own object first: the
        # static write_char of outputs.obj is not output.obj's, though both
        # are `_write_char`, so _output_s_l's calls do not vote for the one
        # _output_l calls.
        defined = collections.defaultdict(set)
        for f in self.fns:
            defined[f.tu].add(f.symbol)
        votes = collections.defaultdict(collections.Counter)
        for s, e, f in pairs:
            sites = self.memo.get(f, s, (e,))
            if sites is None:
                if not any(k in ('call', 'fptr') and t in by_sym for _o, k, t in f.refs):
                    continue                    # no call that could vote: nothing to align
                sites = self.memo.put(f, s, self.aligned_calls(f, s, e), (e,))
            for sym, target in sites:
                if sym in by_sym and (sym not in defined[f.tu] or by_sym[sym].tu == f.tu):
                    votes[sym][target] += 1
        n = 0
        for sym, c in sorted(votes.items()):
            (t, k), total = c.most_common(1)[0], sum(c.values())
            if k < VOTES_MIN or k < VOTES_SHARE * total or t in self.taken or t not in self.model:
                continue
            f = by_sym[sym]
            ends = [(self.similarity(f, t, e), e) for e in self.spans(t, limit=f.size)]
            if not ends:
                continue
            # What our code calls may itself be wrong (the tree calling atol
            # where the original calls atoi): the code must bear it out.
            sim, e = max(ends)
            if sim < LIB_ORDER_THRESHOLD:
                continue
            n += self.take(f, t, e, 'lib-votes', '%d of %d call sites, %.2f' % (k, total, sim))
        return n

    def aligned_calls(self, f, s, e):
        """(our callee symbol, original target) of every call of an unmapped
        symbol in the stretches f and the original's s..e align on: a
        function of the two functions' code alone, so ours.PairMemo keeps it."""
        ov = compare.our_view(f, {})
        rv = compare.ref_view_span(s, e, self.model)
        sm = difflib.SequenceMatcher(None, [i.key for i in ov], [i.key for i in rv], autojunk=False)
        out = []
        for a, b, size in sm.get_matching_blocks():
            for x, y in zip(ov[a:a + size], rv[b:b + size]):
                if x.mnemonic != 'call':
                    continue
                for ta, tb in zip(x.tokens, y.tokens):
                    if ta[0] == 'fn?' and tb[0] == 'fn':
                        out.append((ta[1], tb[1]))
        return out

    def run_extent(self):
        """The original's run of library code: from the first to the last
        exactly paired library function."""
        exact = sorted(v[:2] for v in self.map.values() if v[2] == 'lib-exact')
        return (exact[0][0], exact[-1][1]) if exact else (0, 0)

    def by_similar(self, lo, hi):
        """Inside the run of library code, an unpaired original whose most
        similar library function (over every member) is clearly the most
        similar: the same function compiled from another revision of the
        runtime's source, or carved differently."""
        bylen = collections.defaultdict(list)
        for f in self.fns:
            if f.key not in self.map:
                bylen[len(self.our(f))].append(f)
        n = 0
        for s in self.starts:
            if not lo <= s < hi or s in self.taken:
                continue
            scored = []
            for e in self.spans(s):
                rv = compare.ref_view_span(s, e, self.model)
                b = [i.key for i in rv]
                if len(b) < EXACT_MIN_INSNS:
                    continue
                for m in range(int(len(b) * 0.8), int(len(b) * 1.25) + 1):
                    for g in bylen.get(m, ()):
                        sm = difflib.SequenceMatcher(None, [i.key for i in self.our(g)], b, autojunk=False)
                        floor = scored[1][0] if len(scored) > 1 else LIB_SIMILAR_THRESHOLD - LIB_SIMILAR_MARGIN
                        if sm.real_quick_ratio() < floor or sm.quick_ratio() < floor:
                            continue
                        scored.append((sm.ratio(), g, e))
                        scored.sort(key=lambda t: -t[0])
                        # Keep the best per function, and the best of any other.
                        seen, kept = set(), []
                        for t in scored:
                            if t[1].key not in seen:
                                seen.add(t[1].key)
                                kept.append(t)
                        scored = kept[:2]
            if not scored:
                continue
            top = scored[0]
            rival = scored[1][0] if len(scored) > 1 else 0.0
            if top[0] >= LIB_SIMILAR_THRESHOLD and top[0] - rival >= LIB_SIMILAR_MARGIN:
                n += self.take(top[1], s, top[2], 'lib-similar', '%.2f, next %.2f' % (top[0], rival))
        return n

    def similarity(self, f, s, e):
        a = [i.key for i in self.our(f)]
        b = [i.key for i in compare.ref_view_span(s, e, self.model)]
        sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
        return sm.ratio()

    def neighbours(self, f, step):
        """The unpaired functions of f's member next to it, up to the next paired one."""
        member = self.by_member[f.tu]
        k = self.pos[f.key] + step
        out = []
        while 0 <= k < len(member) and len(out) < NEIGHBOURS:
            g = member[k]
            if g.key in self.map:
                break
            out.append(g)
            k += step
        return out

    def best(self, cands, spans):
        scored = []
        for g in cands:
            for s, e in spans:
                scored.append((self.similarity(g, s, e), g, s, e))
        if not scored:
            return None
        scored.sort(key=lambda t: -t[0])
        top = scored[0]
        rival = next((t[0] for t in scored[1:] if t[1].key != top[1].key), 0.0)
        if top[0] >= LIB_ORDER_THRESHOLD and top[0] - rival >= LIB_ORDER_MARGIN:
            return top
        return None

    def by_order(self):
        n = 0
        changed = True
        while changed:
            changed = False
            for key, (s, e, _m, _d) in sorted(self.map.items(), key=lambda kv: kv[1][0]):
                f = self.byname[key]
                # After it: the next carved function within padding of its end.
                k = self.index.get(e)
                if k is None:
                    k = next((j for j in range(self.index[s] + 1, len(self.starts))
                              if self.starts[j] >= e), None)
                if k is not None and k < len(self.starts) and self.starts[k] - e < PAD:
                    t = self.starts[k]
                    hit = self.best(self.neighbours(f, +1), [(t, x) for x in self.spans(t)])
                    if hit and self.take(hit[1], hit[2], hit[3], 'lib-order', 'after %s %.2f' % (f.name, hit[0])):
                        n += 1
                        changed = True
                # Before it: runs of carved functions ending within padding of its start.
                j = self.index[s]
                runs = []
                for back in range(1, MAX_PIECES + 1):
                    if j - back < 0:
                        break
                    t = self.starts[j - back]
                    for x in self.spans(t):
                        if 0 <= s - x < PAD:
                            runs.append((t, x))
                hit = self.best(self.neighbours(f, -1), runs)
                if hit and self.take(hit[1], hit[2], hit[3], 'lib-order', 'before %s %.2f' % (f.name, hit[0])):
                    n += 1
                    changed = True
        return n


def is_import_thunk(entry):
    dec, refs = entry
    return len(dec.insns) == 1 and dec.insns[0].bytes[:2] == b'\xff\x25' and any(r[3] == 'import' for r in refs)


def load_libmap(path):
    """evidence/libmap.tsv rows, in funcmap.tsv's shape."""
    return audit.load_funcmap(path) if os.path.exists(path) else []


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe-map', default=os.path.join(ORACLE, 'build-exe', 'd2recon-vc8.map'),
                    help="our linked image's map (recon/runs/build-vc8.sh); says which pairs our image links")
    ap.add_argument('--funcmap', default=os.path.join(ORACLE, 'evidence', 'funcmap.tsv'))
    ap.add_argument('--out', default=os.path.join(ORACLE, 'evidence', 'libmap.tsv'))
    args = ap.parse_args()
    if EXPERIMENT and os.path.realpath(os.path.dirname(args.out)) == os.path.realpath(os.path.join(ORACLE, 'evidence')):
        sys.exit('libmap.py: not in an experiment - an A/B audit uses the baseline map (evidence/libmap.tsv)')
    import objcache
    linked = linked_symbols(args.exe_map)
    # Everything the pairing reads: the funcmap, which library functions our
    # image links, the libraries, our objects (and the reference, the scripts).
    key = ourslib.stage_key('libmap', objcache.sha_file(args.funcmap), repr(sorted(linked)),
                            repr([objcache.sha_file(os.path.join(lib_dir(), lib)) for lib in LIBS]),
                            repr(ourslib.object_keys(os.path.join(BUILD, 'obj'))), args.exe_map)
    outputs = [args.out, args.out + '.summary']
    if ourslib.stage_reuse('libmap', BUILD, key, outputs):
        print(open(args.out + '.summary').read(), end='')
        return
    _ref, model = audit.load_context()
    paired = {r[0] for r in audit.load_funcmap(args.funcmap)}
    fns = load_lib_functions()
    m = LibMatcher(model, paired, fns, linked)
    m.by_exact()
    m.by_order()
    lo, hi = m.run_extent()
    m.by_exact((lo, hi))
    m.by_similar(lo, hi)
    m.by_order()
    objs = {f.key: f for f in ourslib.load_all(os.path.join(BUILD, 'obj'))}
    game = [(r[0], r[1], objs[r[3] + '|' + r[4]]) for r in audit.load_funcmap(args.funcmap)
            if r[3] + '|' + r[4] in objs and r[0] in model]
    for _round in range(3):
        lib = [(s, e, m.byname[k]) for k, (s, e, _m, _d) in m.map.items()]
        got = m.by_votes(game + lib)
        got += m.by_order()
        if not got:
            break
    m.memo.save()
    rows = sorted((s, e, f.name, f.tu, f.symbol, method, detail)
                  for f in fns if f.key in m.map for (s, e, method, detail) in [m.map[f.key]])
    with open(args.out, 'w') as fh:
        fh.write('start\tend\tname\ttu\tsymbol\tmethod\tdetail\n')
        for r in rows:
            fh.write('%08x\t%08x\t%s\t%s\t%s\t%s\t%s\n' % r)
    size = lambda s: model[s][0].end - s   # noqa: E731
    lines = ['%s functions (%s): %d, of which our image links %d%s' % (
        ' + '.join(LIBS), LIBVC, len(fns), sum(1 for f in fns if is_linked(f, linked)),
        '' if linked else ' (no linker map: %s)' % args.exe_map)]
    lines.append('original functions the funcmap leaves unpaired: %d (%d bytes)' % (
        len(set(model) - paired), sum(size(s) for s in set(model) - paired)))
    lines.append('paired with a library function: %d (%d bytes), of which our image links %d' % (
        len(rows), sum(r[1] - r[0] for r in rows), sum(1 for r in rows if 'linked' in r[6])))
    by = collections.Counter(r[5] for r in rows)
    for t in ('lib-exact', 'lib-order', 'lib-similar', 'lib-votes'):
        lines.append('  %-11s %5d' % (t, by.get(t, 0)))
    left = sorted(set(model) - m.taken)
    thunks = [s for s in left if is_import_thunk(model[s])]
    inside = [s for s in left if lo <= s < hi and s not in thunks]
    outside = [s for s in left if not lo <= s < hi and s not in thunks]
    lines.append('still unpaired: %d (%d bytes)' % (len(left), sum(size(s) for s in left)))
    lines.append('  among the library code (%08x..%08x): %d (%d bytes)' % (
        lo, hi, len(inside), sum(size(s) for s in inside)))
    lines.append('  import thunks (jmp [__imp_X], made by the linker): %d (%d bytes)' % (
        len(thunks), sum(size(s) for s in thunks)))
    lines.append('  elsewhere: %d (%d bytes)' % (len(outside), sum(size(s) for s in outside)))
    with open(args.out + '.summary', 'w') as fh:
        fh.write('\n'.join(lines) + '\n')
    print('\n'.join(lines))
    ourslib.stage_done('libmap', BUILD, key, outputs)


if __name__ == '__main__':
    main()
