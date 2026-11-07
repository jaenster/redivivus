#!/usr/bin/env python3
"""permodule.py - the byte-match figures per module, and how far each module's
layout is from the original's.

    permodule.py --map <build>/d2recon-vc8.map [--audit evidence/audit.tsv]
                 [--data <workspace>/docs/data] [--out evidence/permodule.tsv]

The map is the linker map of recon/runs/build-vc8.sh's per-module link (its
default): every function of ours with its address and the module library it
came from. Writes <out> (one row per module, a TOTAL row last) and <out
without .tsv>.md (a short table). README.md, "Per-module figures", defines
every column; in short, for module M:

  The original's side. M's functions are the original functions of the
  headline's universe (the game without the C runtime, the prebuilt library
  and the compiler's EH code) that docs/data/attribution-v2.tsv gives to M;
  where it has no row, the band of docs/data/runs-v2.tsv the function lies
  in. attribution-v2 predates three modules of the module table - crashy,
  Game and D2Hell, which it counts as Fog, Fog and Bnclient - so a function
  whose counterpart sits in one of those three takes that module. M's bands
  are its runs-v2 rows (for those three: each run of two or more of their
  functions, consecutive in the original's order).

  Bytes. MATCH functions / M's functions; MATCH bytes and identical bytes
  (audit.tsv ident_bytes: the bytes in instructions identical to ours, the
  whole function for a MATCH) / M's bytes.

  Layout, against our M library's functions in the map. order = the longest
  common subsequence of M's functions in the original's address order and in
  ours (a longest increasing subsequence of our addresses), over M's function
  count. offset = how many of M's functions sit at the same distance from
  the start of their module as in the original: ours from the lowest address
  of any function of the M library, the original's from the first function
  of M's band, a later band of M counting on from where the previous one ends
  (its last function's end rounded up to 16), as if M's bands were one. A
  function counts only when its counterpart is in the M library. run = the
  longest stretch of M's functions, consecutive in the original's order, that
  all MATCH. A module is whole-module identical when every one of its
  functions MATCHes, sits at its offset, and the library holds nothing else.
"""
import argparse
import bisect
import collections
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import audit as auditlib   # noqa: E402
import modules             # noqa: E402
from oracle_config import ORACLE, REPO  # noqa: E402

NEW_MODULES = ('crashy', 'Game', 'D2Hell')   # in the module table, not in attribution-v2


def load_map(path):
    """[(address, symbol, library, object)] of every function in a VC8 .map
    ('f' entries of 'Publics by Value' and 'Static symbols')."""
    out = []
    pat = re.compile(r'^\s*[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s+(f\s+)?(?:i\s+)?(\S+)\s*$')
    for line in open(path, errors='replace'):
        m = pat.match(line)
        if not m or not m.group(3):
            continue
        sym, addr, where = m.group(1), int(m.group(2), 16), m.group(4)
        lib, _, obj = where.rpartition(':')
        out.append((addr, sym, lib, obj))
    return out


def lis_length(seq):
    tails = []
    for x in seq:
        i = bisect.bisect_left(tails, x)
        if i == len(tails):
            tails.append(x)
        else:
            tails[i] = x
    return len(tails)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--map', required=True, help='the per-module VC8 link map (build-vc8.sh)')
    ap.add_argument('--audit', default=os.path.join(ORACLE, 'evidence', 'audit.tsv'))
    ap.add_argument('--funcmap', default=os.path.join(ORACLE, 'evidence', 'funcmap.tsv'))
    ap.add_argument('--libmap', default=os.path.join(ORACLE, 'evidence', 'libmap.tsv'))
    ap.add_argument('--data', default=os.environ.get('D2_DATA') or os.path.join(REPO, '..', '..', 'docs', 'data'),
                    help='directory of attribution-v2.tsv and runs-v2.tsv')
    ap.add_argument('--out', default=os.path.join(ORACLE, 'evidence', 'permodule.tsv'))
    args = ap.parse_args()

    ref, model = auditlib.load_context()
    librows = auditlib.load_funcmap(args.libmap) if os.path.exists(args.libmap) else []
    runtime, _rng = auditlib.runtime_starts(model, librows)
    runtime |= auditlib.prebuilt_starts(model)
    eh = auditlib.eh_starts(ref, model) - runtime
    game = sorted(s for s in model if s not in runtime and s not in eh)
    size = {s: model[s][0].end - s for s in game}

    names = {n.lower(): n for n in modules.linked(REPO)}
    canon = lambda n: names.get(n.lower(), n)  # noqa: E731

    gt = {}
    for line in open(os.path.join(args.data, 'attribution-v2.tsv')):
        p = line.rstrip('\n').split('\t')
        if p[0] != 'addr' and len(p) > 2 and p[2]:
            gt[int(p[0], 16)] = canon(p[2])
    bands = collections.defaultdict(list)     # module -> [(lo, hi)] in address order
    allbands = []
    for line in open(os.path.join(args.data, 'runs-v2.tsv')):
        p = line.rstrip('\n').split('\t')
        if p[0] == 'module':
            continue
        m, lo, hi, status = canon(p[0]), int(p[1], 16), int(p[2], 16), p[8]
        allbands.append((lo, hi, m, status))
        if status != 'shared':                # the C runtime's run, out of the universe
            bands[m].append((lo, hi))

    rows = {}
    for line in open(args.audit):
        p = line.rstrip('\n').split('\t')
        if p[0] == 'start':
            continue
        rows[int(p[0], 16)] = (p[3], p[9], int(p[11] or 0))
    pair = {}                                  # original start -> (tu, symbol)
    for s, _e, _name, tu, sym, _method, _d in auditlib.load_funcmap(args.funcmap):
        pair[s] = (tu, sym)

    def module_of(s):
        tu = (rows.get(s) or pair.get(s) or ('',))[0]
        ours = modules.module_of(tu, REPO) if tu else None
        if ours in NEW_MODULES:
            return ours
        if s in gt:
            return gt[s]
        for lo, hi, m, status in allbands:
            if lo <= s < hi and status != 'shared':
                return m
        return 'unattributed'

    of = {s: module_of(s) for s in game}
    # The three modules attribution-v2 lacks have no runs-v2 band: theirs are
    # the runs of at least two of their functions, consecutive in the game's order.
    for m in NEW_MODULES:
        runs, cur = [], []
        for s in game:
            if of[s] == m:
                cur.append(s)
            elif cur:
                runs.append(cur)
                cur = []
        if cur:
            runs.append(cur)
        bands[m] = [(r[0], r[-1] + size[r[-1]]) for r in runs if len(r) >= 2]

    # Ours: every function of the map, by module library.
    libname = {('d2_root' if n == modules.ROOT_LIB else n).lower(): n for n in modules.linked(REPO)}
    mapped = load_map(args.map)
    ours_by_lib = collections.defaultdict(list)
    where = {}                                 # (module, symbol, obj) / (module, symbol) -> address
    for addr, sym, lib, obj in mapped:
        m = libname.get(lib.lower())
        if m is None:
            continue
        ours_by_lib[m].append((addr, sym))
        where.setdefault((m, sym, obj.lower()), addr)
        where.setdefault((m, sym), addr)
    if not ours_by_lib:
        sys.exit('%s: no function from a module library - is it the per-module link\'s map?' % args.map)

    def our_addr(s):
        """(module library, address) of the counterpart of original function s."""
        if s not in pair:
            return None, None
        tu, sym = pair[s]
        m = modules.module_of(tu, REPO)
        a = where.get((m, sym, os.path.basename(tu).lower() + '.obj'))
        if a is None:
            a = where.get((m, sym))
        return (m, a) if a is not None else (m, None)

    # The original's offset of each function: inside its band, M's bands one after another.
    def offsets(m, fs):
        out, base = {}, 0
        for lo, hi in sorted(bands.get(m, [])):
            inb = [s for s in fs if lo <= s < hi]
            if not inb:
                continue
            for s in inb:
                out[s] = base + (s - inb[0])
            base += (inb[-1] + size[inb[-1]] - inb[0] + 15) & ~15
        return out

    order = modules.linked(REPO)
    present = sorted(set(of.values()), key=lambda m: (order.index(m) if m in order else len(order), m))
    out = []
    tot = collections.Counter()
    for m in present:
        fs = [s for s in game if of[s] == m]
        n, nbytes = len(fs), sum(size[s] for s in fs)
        match = [s for s in fs if rows.get(s, ('', '', 0))[1] == 'MATCH']
        mbytes = sum(size[s] for s in match)
        ibytes = sum(min(rows[s][2], size[s]) for s in fs if s in rows)
        # longest run of consecutive MATCH functions in the original's order
        best = cur = 0
        best_at = cur_at = None
        for s in fs:
            if s in rows and rows[s][1] == 'MATCH':
                if cur == 0:
                    cur_at = s
                cur += 1
                if cur > best:
                    best, best_at = cur, cur_at
            else:
                cur = 0
        # layout
        lib = sorted(ours_by_lib.get(m, []))
        lib_start = lib[0][0] if lib else None
        here, elsewhere, missing, unpaired = [], 0, 0, 0
        for s in fs:
            lm, a = our_addr(s)
            if s not in pair:
                unpaired += 1
            elif a is None:
                missing += 1
            elif lm != m:
                elsewhere += 1
            else:
                here.append((s, a))
        lcs = lis_length([a for _s, a in here])
        off = offsets(m, fs)
        same = sum(1 for s, a in here if s in off and a - lib_start == off[s])
        claimed = {a for _s, a in here}
        foreign = sum(1 for a, _sym in lib if a not in claimed)
        whole = n > 0 and len(match) == n and same == n and foreign == 0
        bandtxt = ' '.join('%08x-%08x' % b for b in sorted(bands.get(m, [])))
        out.append((m, n, len(match), nbytes, mbytes, ibytes, bandtxt, len(here), elsewhere, unpaired, missing,
                    len(lib), foreign, lcs, same, best, '%08x' % best_at if best_at else '-',
                    'yes' if whole else 'no'))
        for k, v in (('n', n), ('match', len(match)), ('bytes', nbytes), ('mbytes', mbytes), ('ibytes', ibytes),
                     ('here', len(here)), ('elsewhere', elsewhere), ('unpaired', unpaired), ('missing', missing), ('lib', len(lib)),
                     ('foreign', foreign), ('lcs', lcs), ('same', same)):
            tot[k] += v

    hdr = ('module', 'functions', 'match', 'bytes', 'match_bytes', 'ident_bytes', 'bands',
           'in_lib', 'in_other_lib', 'unpaired', 'not_linked', 'lib_functions', 'lib_foreign', 'order_lcs',
           'same_offset', 'longest_match_run', 'run_start', 'identical')
    pct = lambda a, b: 100.0 * a / b if b else 0.0  # noqa: E731
    import subprocess
    tree = subprocess.run(['git', '-C', REPO, 'describe', '--always', '--dirty', '--tags'],
                          capture_output=True, text=True).stdout.strip() or 'unknown'
    with open(args.out, 'w') as fh:
        fh.write('# tree %s; %s; map %s\n' % (tree, os.path.basename(args.audit), os.path.basename(args.map)))
        fh.write('\t'.join(hdr) + '\n')
        for r in out:
            fh.write('\t'.join(str(x) for x in r) + '\n')
        fh.write('\t'.join(str(x) for x in ('TOTAL', tot['n'], tot['match'], tot['bytes'], tot['mbytes'],
                                             tot['ibytes'], '', tot['here'], tot['elsewhere'], tot['unpaired'], tot['missing'],
                                             tot['lib'], tot['foreign'], tot['lcs'], tot['same'], '', '', ''))
                 + '\n')
    md = os.path.splitext(args.out)[0] + '.md'
    with open(md, 'w') as fh:
        fh.write('Per-module figures, tree %s (definitions: README.md, "Per-module figures").\n\n' % tree)
        fh.write('|Module|Functions MATCH|Bytes MATCH|Bytes identical|Order (LCS)|Same offset|'
                 'Longest MATCH run|Band(s)|\n|-|-|-|-|-|-|-|-|\n')
        for r in out:
            (m, n, mt, nb, mb, ib, bandtxt, _h, _e, _u, _x, _l, _f, lcs, same, run, _ra, whole) = r
            fh.write('|%s%s|%d/%d (%.1f%%)|%.1f%%|%.1f%%|%.1f%%|%d|%d|%s|\n' % (
                m, ' (identical)' if whole == 'yes' else '', mt, n, pct(mt, n), pct(mb, nb), pct(ib, nb),
                pct(lcs, n), same, run, bandtxt.replace(' ', ', ') or '-'))
        fh.write('|Total|%d/%d (%.1f%%)|%.1f%%|%.1f%%|%.1f%%|%d|||\n' % (
            tot['match'], tot['n'], pct(tot['match'], tot['n']), pct(tot['mbytes'], tot['bytes']),
            pct(tot['ibytes'], tot['bytes']), pct(tot['lcs'], tot['n']), tot['same']))
    print('%d modules; %d of %d functions MATCH, %d of %d bytes; order %d, same offset %d -> %s, %s' % (
        len(out), tot['match'], tot['n'], tot['mbytes'], tot['bytes'], tot['lcs'], tot['same'], args.out, md))


if __name__ == '__main__':
    main()
