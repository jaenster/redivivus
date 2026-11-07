#!/usr/bin/env python3
"""compile64.py - compile the VC8-translated tree with the VC8 x64 cross compiler.

    compile64.py <src> <out> --oracle <asm_matching/msvc8> [--compat <vc8_compat.h>]
                 [--zs] [--jobs N] [--batch N] [patterns...]

<src> is vc8_translate.py output (the tree mirrored for VC8), <out> the result
directory: obj/, log/<tu>.log, diag.tsv (every diagnostic: tu, file, line,
kind, code, message) and status.tsv (tu, ok/FAIL, errors, warnings).
The flags are the oracle's (flags.tsv per module) plus /W3 /Wp64: /Wp64 is what
makes this compiler report C4311 (pointer -> int) and C4312 (int -> pointer).
--zs: syntax check only (/Zs), no objects. Paths reach wine through Z: (= /).
"""
import argparse
import concurrent.futures as cf
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
WRAP = os.path.join(HERE, 'vc8x64.sh')
CFLAGS = '/O2 /Oy- /Ob1 /EHsc /GS /MT /W3 /Wp64 /D WIN32 /D NDEBUG /D _WINDOWS'
DIAG = re.compile(r'^(?P<file>.+?)\((?P<line>\d+)\) : (?P<kind>(?:fatal )?error|warning) (?P<code>C\d{4}): (?P<msg>.*)$')
DIAG_NOLINE = re.compile(r'^(?P<file>.+?) : (?P<kind>(?:fatal )?error|warning) (?P<code>[CD]\d{4}): (?P<msg>.*)$')


def z(path):
    return 'Z:' + os.path.realpath(path).replace('/', '\\')


def host(winpath):
    p = winpath.strip()
    if p[:2].upper() == 'Z:':
        return p[2:].replace('\\', '/')
    return p.replace('\\', '/')


def load_flags(tsv):
    rows = []
    for line in open(tsv):
        if line.startswith('#') or not line.strip():
            continue
        parts = line.rstrip('\n').split('\t')
        rows.append((parts[0], parts[1] if len(parts) > 1 else ''))
    return sorted(rows, key=lambda r: -len(r[0]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('src')
    ap.add_argument('out')
    ap.add_argument('--oracle', required=True, help='asm_matching/msvc8 of the tree (vc8_compat.h, shim_include, flags.tsv)')
    ap.add_argument('--compat', help='the forced-in compat header (default: <oracle>/vc8_compat.h)')
    ap.add_argument('--zs', action='store_true')
    ap.add_argument('--jobs', type=int, default=6)
    ap.add_argument('--batch', type=int, default=16)
    ap.add_argument('patterns', nargs='*')
    a = ap.parse_args()
    src, out = os.path.realpath(a.src), os.path.realpath(a.out)
    flags_table = load_flags(os.path.join(a.oracle, 'flags.tsv'))

    def extra(rel):
        for p, f in flags_table:
            if rel.startswith(p):
                return f
        return ''

    dirs = set()
    tus = []
    for dp, _d, files in os.walk(src):
        if any(f.endswith('.h') for f in files):
            dirs.add(dp)
        for f in files:
            if f.endswith('.cpp'):
                rel = os.path.relpath(os.path.join(dp, f), src)
                if a.patterns and not any(p in rel for p in a.patterns):
                    continue
                tus.append(rel)
    tus.sort()
    os.makedirs(out, exist_ok=True)
    lines = ['/nologo /c ' + CFLAGS + (' /Zs' if a.zs else ''),
             '/FI' + z(a.compat or os.path.join(a.oracle, 'vc8_compat.h')),
             '/FI' + z(os.path.join(src, 'Source', 'D2Headers', 'd2_platform.h')),
             '/I' + z(os.path.join(a.oracle, 'shim_include')), '/I' + z(src)]
    lines += ['/I' + z(d) for d in sorted(dirs) if d != src]
    rsp = os.path.join(out, 'cl.rsp')
    with open(rsp, 'w') as f:
        f.write('\n'.join(lines) + '\n')

    groups = {}
    for rel in tus:
        groups.setdefault((os.path.dirname(rel), extra(rel)), []).append(rel)
    batches = []
    for (d, fl), items in sorted(groups.items()):
        n = -(-len(items) // a.batch)
        for k in range(n):
            part = items[k::n]
            names = [os.path.basename(x).lower() for x in part]
            if len(set(names)) == len(names):
                batches.append((d, fl, part))
            else:
                batches += [(d, fl, [x]) for x in part]
    batches.sort(key=lambda b: -sum(os.path.getsize(os.path.join(src, x)) for x in b[2]))

    def run(b):
        d, fl, part = b
        objdir = os.path.join(out, 'obj', d)
        os.makedirs(objdir, exist_ok=True)
        for x in part:
            o = os.path.join(out, 'obj', x[:-4] + '.obj')
            if os.path.exists(o):
                os.remove(o)
        cmd = [WRAP, 'cl', '@' + z(rsp)] + fl.split() + ['/Fo' + z(objdir) + '\\'] + [z(os.path.join(src, x)) for x in part]
        t0 = time.time()
        try:
            p = subprocess.run(cmd, capture_output=True, text=True, encoding='latin1', timeout=600 * len(part))
            text = p.stdout + p.stderr
        except subprocess.TimeoutExpired:
            text = 'TIMEOUT\n'
        return b, text, time.time() - t0

    srcpaths = {os.path.realpath(os.path.join(src, x)).lower(): x for x in tus}
    diag = open(os.path.join(out, 'diag.tsv'), 'w')
    diag.write('tu\tfile\tline\tkind\tcode\tmsg\n')
    status = {}
    t_start = time.time()
    done = 0
    with cf.ThreadPoolExecutor(a.jobs) as ex:
        for b, text, secs in ex.map(run, batches):
            d, fl, part = b
            names = {os.path.basename(x): x for x in part}
            cur = None
            per = {x: [] for x in part}
            backend = False
            for line in text.splitlines():
                s = line.strip()
                if s == 'Generating Code...':
                    backend = True
                    continue
                if not backend and s in names:
                    cur = names[s]
                    continue
                m = DIAG.match(line) or DIAG_NOLINE.match(line)
                if m:
                    f = host(m.group('file'))
                    tu = srcpaths.get(os.path.realpath(f).lower()) if backend else cur
                    if tu is None:
                        tu = cur if cur else part[0]
                    rel = os.path.relpath(f, src) if f.startswith(src) else f
                    kind = 'error' if 'error' in m.group('kind') else 'warning'
                    ln = m.groupdict().get('line') or ''
                    per.setdefault(tu, []).append(line)
                    diag.write('\t'.join([tu, rel, ln, kind, m.group('code'), m.group('msg').replace('\t', ' ')]) + '\n')
                elif cur:
                    per[cur].append(line)
            for x in part:
                lp = os.path.join(out, 'log', x[:-4] + '.log')
                os.makedirs(os.path.dirname(lp), exist_ok=True)
                with open(lp, 'w') as f:
                    f.write('\n'.join(per.get(x, [])) + '\n')
                errs = sum(1 for l in per.get(x, []) if ' error C' in l)
                warns = sum(1 for l in per.get(x, []) if ' warning C' in l)
                obj = os.path.join(out, 'obj', x[:-4] + '.obj')
                ok = (errs == 0) if a.zs else (os.path.exists(obj) and errs == 0)
                if 'TIMEOUT' in text:
                    ok = False
                status[x] = (ok, errs, warns)
            done += len(part)
            print('%4d/%d  %5.1fs  %s (%d)' % (done, len(tus), secs, d, len(part)), flush=True)
    diag.close()
    with open(os.path.join(out, 'status.tsv'), 'w') as f:
        f.write('tu\tstatus\terrors\twarnings\n')
        for x in tus:
            ok, e, w = status.get(x, (False, 0, 0))
            f.write('%s\t%s\t%d\t%d\n' % (x, 'ok' if ok else 'FAIL', e, w))
    nfail = sum(1 for v in status.values() if not v[0])
    print('TUs %d, failed %d, %.0fs' % (len(tus), nfail, time.time() - t_start))


if __name__ == '__main__':
    main()
