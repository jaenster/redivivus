"""sizetrend.py - MATCH rate and closeness by function size, from evidence/audit.tsv.

  python -I scripts/sizetrend.py [--audit evidence/audit.tsv] [--out evidence/sizetrend.tsv] [--label TEXT]

One row per size bucket of the headline functions (the audit's "without the C
runtime" set: runtime, prebuilt libraries and compiler EH code left out), bucketed
by the original's instruction count: 1, 2-3, 4-9, 10-29, 30-99, 100-299, 300+.

  functions        headline functions in the bucket that the audit compared
  match_pct        of those, MATCH
  bytes_pct        the bucket's share of the compared headline bytes
  hunks_median     non-MATCH only: diff hunks, a hunk being a run of adjacent
  hunks_mean       differing lines in compare.side_by_side (any marker but ' ')
  sim_mean         non-MATCH only: the audit's similarity

The hunks need the objects of the build the audit judged (build/obj), so run it
right after `oracle all`, before the next build replaces them. Rows the audit marks
MISSING or QUARANTINE have no object to diff and count only in `functions`.
"""
import argparse
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import audit          # noqa: E402
import compare        # noqa: E402
import ours as ourslib  # noqa: E402
from oracle_config import ORACLE, BUILD  # noqa: E402

BUCKETS = [(1, 1, '1'), (2, 3, '2-3'), (4, 9, '4-9'), (10, 29, '10-29'), (30, 99, '30-99'),
           (100, 299, '100-299'), (300, 10 ** 9, '300+')]


def bucket(n):
    for lo, hi, name in BUCKETS:
        if lo <= n <= hi:
            return name
    return None


def hunks(o, rf):
    n, prev = 0, False
    for line in compare.side_by_side(o, rf):
        d = len(line) > 67 and line[67] != ' '
        if d and not prev:
            n += 1
        prev = d
    return n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--audit', default=os.path.join(ORACLE, 'evidence', 'audit.tsv'))
    ap.add_argument('--funcmap', default=os.path.join(ORACLE, 'evidence', 'funcmap.tsv'))
    ap.add_argument('--out', default=None)
    ap.add_argument('--label', default='')
    args = ap.parse_args()

    ref, model = audit.load_context()
    librows = audit.load_funcmap(audit.LIBMAP) if os.path.exists(audit.LIBMAP) else []
    runtime, _span = audit.runtime_starts(model, librows)
    runtime |= audit.prebuilt_starts(model)
    eh = audit.eh_starts(ref, model) - runtime

    rows = audit.load_funcmap(args.funcmap)
    syms = audit.sym_map(rows, librows)
    sym_of = {(r[0], r[3]): r[4] for r in rows}
    fns = {f.key: f for f in ourslib.load_all(os.path.join(BUILD, 'obj'))}

    stats = {name: {'n': 0, 'match': 0, 'bytes': 0, 'hunks': [], 'sim': []} for _lo, _hi, name in BUCKETS}
    total_bytes = 0
    for line in open(args.audit):
        if line.startswith('start\t') or not line.strip():
            continue
        c = line.rstrip('\n').split('\t')
        s, e, name, tu, status = int(c[0], 16), int(c[1], 16), c[2], c[3], c[9]
        if s in runtime or s in eh or s not in model:
            continue
        b = bucket(len(model[s][0].insns))
        if b is None:
            continue
        st = stats[b]
        st['n'] += 1
        st['bytes'] += e - s
        total_bytes += e - s
        if status == 'MATCH':
            st['match'] += 1
            continue
        if status in ('MISSING', 'QUARANTINE'):
            continue
        st['sim'].append(float(c[8]))
        f = fns.get(tu + '|' + (sym_of.get((s, tu)) or ''))
        if f is None:
            continue
        st['hunks'].append(hunks(compare.our_view(f, syms), compare.ref_view_span(s, e, model)))

    out = ['# sizetrend %s' % args.label if args.label else '# sizetrend',
           'bucket\tfunctions\tmatch_pct\tbytes_pct\thunks_median\thunks_mean\tsim_mean']
    for _lo, _hi, name in BUCKETS:
        st = stats[name]
        h, sm = st['hunks'], st['sim']
        out.append('%s\t%d\t%.2f\t%.2f\t%s\t%s\t%s' % (
            name, st['n'], 100.0 * st['match'] / max(1, st['n']), 100.0 * st['bytes'] / max(1, total_bytes),
            ('%g' % statistics.median(h)) if h else '-', ('%.2f' % statistics.mean(h)) if h else '-',
            ('%.4f' % statistics.mean(sm)) if sm else '-'))
    text = '\n'.join(out) + '\n'
    if args.out:
        with open(args.out, 'w') as fh:
            fh.write(text)
    sys.stdout.write(text)


if __name__ == '__main__':
    main()
