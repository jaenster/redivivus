#!/usr/bin/env python3
"""abcompare.py - which configuration matches better, module by module.

    abcompare.py <baseline audit.tsv> <experiment audit.tsv> [--depth N] [--list]

Both audits must come from the same function map (an experiment's audit does:
`oracle audit` uses evidence/funcmap.tsv). Per module - the first N path
components of the TU, default 1 - it prints the MATCH count on each side, how
many functions only one side matches, and the mean similarity, so a flag is
adopted for a module only where it wins there (flags.tsv). --list names the
functions that changed status.
"""
import argparse
import collections


def load(path):
    rows = {}
    for line in open(path):
        p = line.rstrip('\n').split('\t')
        if p[0] == 'start':
            continue
        rows[p[0]] = (p[2], p[3], float(p[8]), p[9])   # name, tu, similarity, status
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('baseline')
    ap.add_argument('experiment')
    ap.add_argument('--depth', type=int, default=1)
    ap.add_argument('--list', action='store_true')
    args = ap.parse_args()
    a, b = load(args.baseline), load(args.experiment)
    mods = collections.defaultdict(lambda: collections.Counter())
    sims = collections.defaultdict(lambda: [0.0, 0.0, 0])
    changed = []
    for k in sorted(set(a) & set(b)):
        name, tu, sa, xa = a[k]
        _n, _t, sb, xb = b[k]
        if xa == 'QUARANTINE' or xb == 'QUARANTINE' or xa == 'MISSING' or xb == 'MISSING':
            continue
        parts = tu.split('/')
        if parts[0] in ('Source', '3rdparty') and len(parts) > 1:
            parts = parts[1:]           # the module is the directory under the layout root
        mod = '/'.join(parts[:args.depth])
        c = mods[mod]
        c['n'] += 1
        c['a'] += xa == 'MATCH'
        c['b'] += xb == 'MATCH'
        c['only_a'] += xa == 'MATCH' and xb != 'MATCH'
        c['only_b'] += xb == 'MATCH' and xa != 'MATCH'
        s = sims[mod]
        s[0] += sa
        s[1] += sb
        s[2] += 1
        if (xa == 'MATCH') != (xb == 'MATCH'):
            changed.append((mod, k, name, xa, xb))
    print('%-28s %6s %8s %8s %7s %7s %8s %8s' % ('module', 'n', 'MATCH a', 'MATCH b', 'only a', 'only b',
                                                 'sim a', 'sim b'))
    tot = collections.Counter()
    for mod in sorted(mods):
        c, s = mods[mod], sims[mod]
        tot.update(c)
        print('%-28s %6d %8d %8d %7d %7d %8.3f %8.3f' % (mod, c['n'], c['a'], c['b'], c['only_a'], c['only_b'],
                                                         s[0] / s[2], s[1] / s[2]))
    print('%-28s %6d %8d %8d %7d %7d' % ('total', tot['n'], tot['a'], tot['b'], tot['only_a'], tot['only_b']))
    if args.list:
        for mod, k, name, xa, xb in changed:
            print('  %s %s %-60s %s -> %s' % (mod, k, name[:60], xa, xb))


if __name__ == '__main__':
    main()
