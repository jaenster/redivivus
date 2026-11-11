#!/usr/bin/env python3
"""fix_all.py - run every fixer over the tree, in order, until none gains.

    fix_all.py [--rounds N] [-j N] [PATTERN ...]

The fixers' rules interact: a function made static changes its callers, a
comparison written back lets an if/else line up. So they run in a fixed order
- files and conventions first (fix_refile, fix_paramtype), then the local rewrites - and the cycle repeats while a round
still keeps edits, at most N rounds.
This is also what a regenerated tree is run through (UPSTREAM.md). Each
fixer records its run in evidence/fixes.tsv; the audit at the end is the
measure.
"""
import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
FIXERS = [['fix_refile.py'], ['fix_paramtype.py'], ['fix_compare.py'],
          ['fix_assert.py'], ['fix_ifelse.py'], ['fix_switch.py'], ['fix_switch.py', '--single'], ['fix_caseorder.py'], ['fix_forloop.py'],
          ['fix_idioms.py'], ['fix_bitmask.py'], ['fix_bitmask.py', '--inverse'], ['fix_shiftbit.py'], ['fix_localtype.py'], ['fix_return.py'],
          ['fix_gsbuffer.py'], ['fix_stringops.py'], ['fix_nestcall.py'], ['fix_inplace.py'], ['fix_boolarith.py'], ['fix_order.py'],
          ['fix_order.py', '--by', 'store'], ['fix_permute.py']]


def run(args):
    p = subprocess.run([sys.executable, '-I', os.path.join(HERE, args[0])] + args[1:],
                       capture_output=True, text=True)
    lines = [l for l in p.stdout.splitlines() if l.strip()]
    first, last = lines[0] if lines else '', lines[-1] if lines else ''
    if p.returncode:                             # a fixer that died says so, not its last progress line
        err = p.stderr.strip().splitlines()
        last = 'FAILED (exit %d): %s' % (p.returncode, err[-1] if err and not last.startswith('fix_') else last)
    return first, last


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--rounds', type=int, default=3)
    ap.add_argument('-j', '--jobs', type=int, default=4, help='for the final build (the fixers build four at a time)')
    ap.add_argument('patterns', nargs='*')
    args = ap.parse_args()
    for rnd in range(1, args.rounds + 1):
        any_kept = False
        for fx in FIXERS:
            first, last = run(fx + ['--apply'] + args.patterns)
            print('round %d %-28s %s | %s' % (rnd, ' '.join(fx), first, last), flush=True)
            if last.startswith('kept ') and not last.startswith('kept 0 '):
                any_kept = True
        if not any_kept:
            break
    oracle = os.path.join(os.path.dirname(HERE), 'oracle')
    subprocess.run([oracle, 'build', '-j%d' % args.jobs], stdout=subprocess.DEVNULL)
    subprocess.run([oracle, 'audit'])
    return 0


if __name__ == '__main__':
    sys.exit(main())
