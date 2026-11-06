#!/usr/bin/env python3
"""fix_uninline.py - call the helpers the decompiler pasted into their callers.

    fix_uninline.py [--apply] [PATTERN ...]

A function marked inline in the Ghidra database is decompiled into its
callers' bodies, which reads well and compiles to something else: the
original calls it. InitSeed_Param_666 (D2Common/Seed, 00650e40) sets a seed's
low dword and its high dword to 666; the tree has the two assignments where
the original has `lea ecx, [seed]; call 00650e40` (InitCommonData,
UNIT_CreatePlayer, ...) - also through two names for the one seed, where
the names are shown to be the same pointer (_alias_seed_call). STRING_ZeroOneWCHAR (D2Launch/MainMenus, 0042f530)
clears one WCHAR. Each helper below comes with the text its body becomes in a
caller; in a function that does not match yet and whose original calls the
helper, that text is replaced by the call - the helper's header included if the TU does not yet - and kept
through fixlib's batch loop only where the oracle confirms it. The durable
fix is the database's: these functions are not inline.
"""
import argparse
import collections
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import audit            # noqa: E402
import fixlib           # noqa: E402
import ours as ourslib  # noqa: E402
from oracle_config import BUILD, ORACLE, REPO  # noqa: E402

LV = r'[A-Za-z_][\w]*(?:(?:\.|->)[A-Za-z_]\w*|\[[^\[\]]*\])*'


def _seed_call(m):
    ind, obj, sep, value = m.group('ind'), m.group('obj'), m.group('sep'), m.group('v')
    ptr = obj if sep == '->' else '&' + obj
    return '%sD2Common::Seed::InitSeed_Param_666(%s, %s);' % (ind, ptr, value)


IDENT = re.compile(r'[A-Za-z_]\w*')


def _declared(head_and_body, name):
    """Whether `name` is a parameter or a local of the function text."""
    return re.search(r'(?:^[ \t]*|[(,]\s*)(?!(?:return|else|case|goto|delete|throw|new|sizeof)\b)(?:const\s+)?'
                     r'[\w:]+(?:\s*[*&]+\s*|\s+)%s\s*[;=,)\[]' % re.escape(name), head_and_body, re.M) is not None


def _alias_seed_call(m):
    """InitSeed_Param_666 where its two stores go through two names for one
    seed: `pSeed = &pUnit->sSeed; ... pSeed->nSeedLow = v; pUnit->sSeed.nSeedHigh
    = 0x29a;` (MISSILE_CltDoFn50_Volcano, 004d5c10). The names are the same
    pointer when one was assigned the other a few lines up and nothing they
    are made of was assigned since - nor, for a global among them, anything
    called. The call goes through the pointer variable. None if unproven."""
    text, ind = m.string, m.group('ind')
    pa = m.group('a') if m.group('sa') == '->' else '&' + m.group('a')
    pb = m.group('b') if m.group('sb') == '->' else '&' + m.group('b')
    if pa == pb:
        return None
    func = text[max(text.rfind('\n}', 0, m.start()) + 2, 0):m.start()]   # this function, up to the stores
    lines = func.split('\n')
    if lines and lines[-1] == '':
        lines.pop()
    for back in range(1, 7):
        if back > len(lines):
            break
        d = re.match(r'^[ \t]*(?:[\w:]+\s*\*\s*)?(?P<x>[A-Za-z_]\w*) = (?P<y>[^;]+);[ \t]*$', lines[-back])
        if not d:
            continue
        x, y = d.group('x'), d.group('y').strip()
        if {x, y} != {pa, pb}:
            continue
        between = '\n'.join(lines[len(lines) - back + 1:])
        names = set(re.findall(r'(?<![.>\w])[A-Za-z_]\w*', y)) | {x}      # not the members
        for n in names:
            if re.search(r'(?:\b%s\s*(?:[-+*/|&^]?=(?!=)|\+\+|--)|(?:\+\+|--|&)\s*%s\b)' % (re.escape(n), re.escape(n)), between):
                return None
        if any(not _declared(func, n) for n in names) and re.search(r'\w\s*\(', between):
            return None                       # a call could move a global
        return '%sD2Common::Seed::InitSeed_Param_666(%s, %s);' % (ind, x, m.group('v'))
    return None


HELPERS = [
    # (name, header, regex over consecutive lines, replacement(match) -> one line)
    ('InitSeed_Param_666', 'Source/D2Common/Seed.h',
     re.compile(r'^(?P<ind>[ \t]*)(?P<obj>' + LV + r')(?P<sep>\.|->)nSeedLow = (?P<v>[^;\n]+);\n'
                r'(?P=ind)(?P=obj)(?P=sep)nSeedHigh = (?:0x29a|666);$', re.M),
     _seed_call),
    ('InitSeed_Param_666', 'Source/D2Common/Seed.h',
     re.compile(r'^(?P<ind>[ \t]*)(?P<a>' + LV + r')(?P<sa>\.|->)nSeedLow = (?P<v>[^;\n]+);\n'
                r'(?P=ind)(?P<b>' + LV + r')(?P<sb>\.|->)nSeedHigh = (?:0x29a|666);$', re.M),
     _alias_seed_call),
    ('STRING_ZeroOneWCHAR', 'Source/D2Launch/MainMenus.h',
     re.compile(r'^(?P<ind>[ \t]*)\*\(uint16_t\s*\*\)(?P<p>' + LV + r'|\([^()\n]*\)) = 0;$', re.M),
     lambda m: '%sD2Launch::MainMenus::STRING_ZeroOneWCHAR(%s);' % (m.group('ind'), m.group('p'))),
]


def candidates(rows, status, model, pats):
    """tu -> [(original start, helper, line number of the match's first line, old text, new text)],
    where the original of the function the text is in calls the helper."""
    at = {r[2].rsplit('::', 1)[-1]: r[0] for r in rows}
    calls = {s: {t for _a, _o, _n, k, t in model[s][1] if k == 'call'} for s in model}
    out = collections.defaultdict(list)
    for tu in sorted({r[3] for r in rows}):
        if pats and not any(p in tu for p in pats):
            continue
        cpp = os.path.join(REPO, tu + '.cpp')
        obj = os.path.join(BUILD, 'obj', tu + '.obj')
        if not os.path.exists(cpp) or not os.path.exists(obj):
            continue
        text = fixlib.read(cpp)
        found = [(h, m) for h in HELPERS for m in h[2].finditer(text)]
        if not found:
            continue
        own = tu.lower() + '.cpp'
        by_sym = {r[4]: r[0] for r in rows if r[3] == tu}
        spans = []
        for f in ourslib.load_object(os.path.join(BUILD, 'obj'), tu + '.obj'):
            s = by_sym.get(f.symbol)
            mine = [ln for _o, fn, ln in f.lines if fn.lower().replace('\\', '/').endswith(own)]
            if s is not None and mine:
                spans.append((min(mine), max(mine), s))
        for h, m in found:
            ln = text.count('\n', 0, m.start()) + 1
            owner = [s for lo, hi, s in spans if lo <= ln <= hi]
            if len(owner) != 1 or status.get(owner[0]) in (None, 'MATCH', 'QUARANTINE', 'MISSING'):
                continue
            if at.get(h[0]) not in calls.get(owner[0], ()):
                continue                     # the original does not call it here: a store of its own
            if h[0] == 'STRING_ZeroOneWCHAR' and tu == 'Source/D2Launch/MainMenus':
                body = text[text.rfind('\n', 0, m.start()) - 200:m.start()]
                if 'STRING_ZeroOneWCHAR(void * pBuffer)' in body:
                    continue                 # the helper's own body
            new = h[3](m)
            if new is not None:
                out[tu].append((owner[0], h, ln, m.group(0), new))
    return out


def site_edit(cpp, ln, old, new, header):
    def fn(texts):
        t = texts[cpp]
        # Near its line, not at it: an #include added by another site's edit
        # moves everything below it down by one.
        starts = [0] + [k + 1 for k, c in enumerate(t) if c == '\n']
        lo = starts[max(0, min(ln - 6, len(starts) - 1))]
        hi = starts[min(ln + 5, len(starts) - 1)]
        hits = [k for k in range(lo, hi) if t.startswith(old, k) and (k == 0 or t[k - 1] == '\n')]
        if len(hits) != 1:
            return None
        at = hits[0]
        t = t[:at] + new + t[at + len(old):]
        inc = '#include "%s"' % header
        if inc not in t:
            # After the last #include at the top of the file.
            incs = list(re.finditer(r'^#include [^\n]*\n', t, re.M))
            if not incs:
                return None
            t = t[:incs[-1].end()] + inc + '\n' + t[incs[-1].end():]
        return {cpp: t}
    return fn


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--apply', action='store_true')
    ap.add_argument('-j', '--jobs', type=int, default=4)
    ap.add_argument('patterns', nargs='*')
    args = ap.parse_args()
    rows = audit.load_funcmap(os.path.join(ORACLE, 'evidence', 'funcmap.tsv'))
    status = {}
    for line in open(os.path.join(ORACLE, 'evidence', 'audit.tsv')):
        p = line.rstrip('\n').split('\t')
        if p[0] != 'start' and len(p) > 9:
            status[int(p[0], 16)] = p[9]
    _ref, model = audit.load_context()
    cands = candidates(rows, status, model, args.patterns)
    total = sum(len(v) for v in cands.values())
    print('%d sites in %d TUs: %s' % (total, len(cands), ', '.join('%s %d' % kv for kv in sorted(
        collections.Counter(c[1][0] for v in cands.values() for c in v).items()))))
    if not args.apply:
        for tu in sorted(cands):
            for s, h, ln, old, new in cands[tu]:
                print('%s:%d  %08x\n    - %s\n    + %s' % (tu, ln, s, old.strip().replace('\n', ' / '), new.strip()))
        return 0
    syms = audit.sym_map(rows)
    quarantined = audit.load_quarantine(os.path.join(BUILD, 'quarantine.tsv'))
    groups = {}
    for tu in sorted(cands):
        cpp = os.path.join(REPO, tu + '.cpp')
        # Bottom-up, so a site that loses a line never moves another's.
        order = sorted(cands[tu], key=lambda c: -c[2])
        groups[tu] = ([fixlib.Edit('%s:%d' % (tu, ln), [s], [s], site_edit(cpp, ln, old, new, h[1]))
                       for s, h, ln, old, new in order], [cpp])
    kept, gained = fixlib.apply_batch(groups, (rows, model, syms, quarantined), jobs=args.jobs,
                                      log=lambda m: print(m, flush=True))
    print('kept %d of %d edits, MATCH %+d' % (kept, total, gained))
    fixlib.record('fix_uninline', total, kept, gained)
    return 0


if __name__ == '__main__':
    sys.exit(main())
