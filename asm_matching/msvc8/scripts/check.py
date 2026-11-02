#!/usr/bin/env python3
"""check.py - re-judge some TUs' functions against the committed audit.

    check.py [--update] PATTERN ...      (PATTERN: a path substring, e.g. D2Game/PLAYER)
    check.py                             (the carve check alone)

The inner loop and the regression gate. For every mapped function whose TU
path contains a PATTERN, compares the object on disk (rebuild it first) with
the original and with that function's row in evidence/audit.tsv, and prints
what changed: new MATCHes, regressions (a MATCH that no longer is), and
similarity moves. Exit status 1 if anything regressed.

--update writes the new rows into evidence/audit.tsv (the summary is only
rewritten by a full `oracle audit`).

Every run first checks the carve of the original (build/ref/functions.tsv)
against CARVE: extents that the carving has got wrong before, each confirmed
against Ghidra's body and the disassembly. Exit status 1 if one differs.
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import audit            # noqa: E402
import compare          # noqa: E402
import ours as ourslib  # noqa: E402
import refimage         # noqa: E402
from oracle_config import BUILD, ORACLE, REFCACHE  # noqa: E402

AUDIT = os.path.join(ORACLE, 'evidence', 'audit.tsv')

# start, end (exclusive), roots, name: what refimage.py must carve.
CARVE = [
    # A 16-aligned 0xCC that is an operand byte of the last instruction
    # (call rel32 at 0066a15d) is not a padded start.
    (0x00669130, 0x0066a19f, (), 'CUBE_LoadCubeMainTable'),
    (0x00401f10, 0x004020c2, (), 'GetSymbolNameForAddress'),
    (0x00409dd0, 0x00409fea, (), 'InitializePoolSystem'),
    # A branch past a padded start into its body: the switch table lies beyond.
    (0x00626200, 0x006266bc, (), 'STATLIST_CalcCombinedStatValue'),
    # An SEH prolog starts a function nothing calls (_tsopen_helper); its
    # __finally funclet is its root, not _tsopen_nolock's.
    (0x0069b08a, 0x0069b7e3, (), '_tsopen_nolock'),
    (0x0069b7e3, 0x0069b85f, (0x0069b85d,), '_tsopen_helper'),
    (0x006a8b91, 0x006a92eb, (), '_tsopen_nolock (wopen)'),
    (0x006a92eb, 0x006a9367, (0x006a9365,), '_tsopen_helper (wopen)'),
    # A callback held in data, right after its predecessor's ret with no padding.
    (0x005ee040, 0x005ee260, (), 'AITHINK_Fn1_CatapultSpotter'),
    (0x005ee260, 0x005ee3b6, (), 'AITHINK_Fn1_DeathMauler'),
    (0x0046a840, 0x0046a940, (), 'WindowsKeyPressedEvent'),
    (0x0046a940, 0x0046aa1b, (), 'INPUT_DispatchKeyCommand'),
    # Funclets the exception tables name stay roots, even on the 16-byte grid.
    (0x0069ac98, 0x0069ad85, (0x0069ad80,), 'SEH function, __finally on the grid'),
    (0x006b42f0, 0x006b4c86, (0x006b4c42, 0x006b4c73), 'Direct3DCallBack catch funclets'),
]


def check_carve(functions):
    """Lines describing every CARVE entry the carve gets wrong."""
    bad = []
    for s, e, roots, name in CARVE:
        got = functions.get(s)
        if got is None:
            bad.append('carve: %08x %s is not a function start' % (s, name))
        elif got[0] != e or tuple(got[3]) != roots:
            bad.append('carve: %08x %s is %08x..%08x roots %s, expected ..%08x roots %s' % (
                s, name, s, got[0], ','.join('%08x' % r for r in got[3]) or '-', e,
                ','.join('%08x' % r for r in roots) or '-'))
    return bad


def judge(rows, pats, model, syms, quarantined):
    """start -> (status, similarity, ref_insns, our_insns, byte_diffs, notes, end, name, tu, method,
    ident_bytes, operand_bytes) for the selected TUs."""
    tus = sorted({r[3] for r in rows if any(p in r[3] for p in pats)})
    fns, by_name = {}, {}
    for tu in tus:
        path = os.path.join(BUILD, 'obj', tu + '.obj')
        if os.path.exists(path) and os.path.exists(path[:-4] + '.d'):
            for f in ourslib.load_object(os.path.join(BUILD, 'obj'), tu + '.obj'):
                fns[f.key] = f
                by_name.setdefault((tu, f.name), []).append(f)
    out = {}
    for s, e, name, tu, sym, method, _d in rows:
        if tu not in tus:
            continue
        f = fns.get(tu + '|' + sym)
        if f is None and len(by_name.get((tu, name), ())) == 1:
            f = by_name[(tu, name)][0]       # its signature changed since the map: same function, other mangling
        q = quarantined.get((tu, name.rsplit('::', 1)[-1]))
        if f is None:
            out[s] = ('MISSING', 0.0, 0, 0, 0, 'object not compiled', e, name, tu, method, 0, 0)
        elif q is not None:
            out[s] = ('QUARANTINE', 0.0, len(model[s][0].insns), 0, 0, q, e, name, tu, method, 0, 0)
        else:
            r = compare.compare(compare.our_view(f, syms), compare.ref_view(s, model[s]))
            notes = '; '.join(x for x in (('%d refs differ' % r.ref_diffs) if r.ref_diffs else '',
                                          ('%d refs unverified' % r.unverified) if r.unverified else '') if x)
            out[s] = (r.status, r.similarity, r.ref_insns, r.our_insns, r.byte_diffs, notes, e, name, tu, method,
                      r.ident_bytes, r.operand_bytes)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--update', action='store_true')
    ap.add_argument('patterns', nargs='*')
    args = ap.parse_args()
    ref = refimage.Reference(os.path.join(ORACLE, 'reference', 'Game.exe'))
    bad = check_carve(ref.load(os.path.join(REFCACHE, 'functions.tsv')))
    for line in bad:
        print(line)
    print('carve: %d of %d checked extents as expected' % (len(CARVE) - len(bad), len(CARVE)))
    if not args.patterns:
        return 1 if bad else 0
    rows = audit.load_funcmap(os.path.join(ORACLE, 'evidence', 'funcmap.tsv'))
    _ref, model = audit.load_context()
    syms = audit.sym_map(rows)
    quarantined = audit.load_quarantine(os.path.join(BUILD, 'quarantine.tsv'))
    old = {}
    lines = open(AUDIT).read().split('\n')
    for line in lines[1:]:
        p = line.split('\t')
        if len(p) > 9:
            old[int(p[0], 16)] = (p[9], float(p[8]))
    new = judge(rows, args.patterns, model, syms, quarantined)
    gained = lost = 0
    up = down = 0.0
    for s in sorted(new):
        st, sim = new[s][0], new[s][1]
        ost, osim = old.get(s, ('?', 0.0))
        if st == 'MATCH' and ost != 'MATCH':
            gained += 1
            print('  + MATCH  %08x %s' % (s, new[s][7]))
        elif ost == 'MATCH' and st != 'MATCH':
            lost += 1
            print('  - LOST   %08x %s (%s)' % (s, new[s][7], st))
        elif abs(sim - osim) >= 0.001:
            up += max(0.0, sim - osim)
            down += max(0.0, osim - sim)
    match = sum(1 for v in new.values() if v[0] == 'MATCH')
    print('%d functions: %d MATCH (%+d, -%d), similarity +%.2f / -%.2f' % (len(new), match, gained, lost, up, down))
    if args.update:
        update_rows(new)
    return 1 if lost or bad else 0


def update_rows(state):
    """Write the judged functions' rows into evidence/audit.tsv."""
    lines = open(AUDIT).read().split('\n')
    out = [lines[0]]
    for line in lines[1:]:
        p = line.split('\t')
        if len(p) > 9 and int(p[0], 16) in state:
            s = int(p[0], 16)
            st, sim, ri, oi, bd, notes, e, name, tu, method, ident, operand = state[s]
            line = audit.ROW % (s, e, name, tu, method, ri, oi, bd, sim, st, notes, ident, operand)
        out.append(line)
    open(AUDIT, 'w').write('\n'.join(out))


if __name__ == '__main__':
    sys.exit(main())
