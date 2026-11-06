#!/usr/bin/env python3
"""build.py - compile the reconstruction with Visual C++ 2005, one object per TU.

    build.py [-j N] [--force] [--with-data] [PATTERN ...]

Mirrors the tree through vc8_translate.py into build/src, then compiles every
translation unit (or those whose path contains one of the PATTERNs) to
build/obj/<path>.obj under wine, N at a time. Incremental by content, not by
time: every compile is stored in a content-addressed cache (objcache.py) under
the hash of what produced it - the compiler, the command line, the translated
source and every header /showIncludes reported (recorded in <path>.d) - and a
TU is compiled only when no stored compile matches. An object already in
place for the same inputs is left untouched (its <path>.key says which), so
nothing downstream sees a change either. --force (or ORACLE_NOCACHE=1)
compiles every TU regardless: a full build.

Sources the tree no longer has are removed from build/src and their objects
from build/obj, so an incremental build holds what a clean one would.

Several TUs of one object directory go through one cl.exe (--batch, at most
16 by default): starting the compiler under wine costs as much as compiling
a small TU. Each TU's object, log and status come out exactly as from a
compile of its own; a TU that fails goes on through the quarantine alone.
Each object the compile finishes is decoded (ours.load_object) in a few
processes while the rest compile (--decode-jobs), so the map and the audit
that follow find it decoded.

Per TU it leaves <path>.log (the compiler's own output) and appends a row to
build/compile_status.tsv: path, status (ok / FAIL), error count, first error.

Source/globals/*.cpp define data only - no function - and are the largest units in
the tree, so they are skipped unless --with-data asks for them (a whole-image
link would need them; the per-function comparison does not).

--extra DIR compiles what the whole-image build adds (recon/runs/build-vc8.sh)
with the same command line and precompiled header, into DIR/obj instead of
build/obj, where the oracle never sees them: Source/globals/ (with --with-data) and
the hand-written Redivivus/ TUs named by --support, translated into DIR/src. The
TUs the oracle compares are then compiled once, for both.
Exit status is 0 when every requested TU compiled.

Flags come from ORACLE_CFLAGS (env.sh default). Anything but the default marks
the build as an experiment, and status rows say so.

The shared prefix every TU parses - vc8_compat.h, Source/D2Headers/d2_platform.h and
Source/globals/globals.h - is compiled once into a precompiled header (build/pch/),
cutting each TU's compile time about threefold. A precompiled header is parse
state, not code: objdiff.py over 19 objects from nine modules found all 741
functions identical with and without it. ORACLE_PCH=0 turns it off.
"""
import argparse
import fcntl
import glob
import concurrent.futures as cf
import json
import pickle
import os
import re
import subprocess
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import coffobj        # noqa: E402
import objcache       # noqa: E402
import vc8_translate  # noqa: E402

from oracle_config import ORACLE, REPO, BUILD, CFLAGS, DEFAULT_CFLAGS, EXPERIMENT, VC, TOOL_CFLAGS  # noqa: E402
SRC = os.path.join(BUILD, 'src')
SUPPORT = 'Redivivus'       # the hand-written tree: headers mirrored, TUs only through --support
OBJ = os.path.join(BUILD, 'obj')
USE_PCH = os.environ.get('ORACLE_PCH', '1') == '1'
PCHDIR = os.path.join(BUILD, 'pch')
PCH_HEADER = 'oracle_pch.h'
PCH_FILE = os.path.join(PCHDIR, 'oracle.pch')
DRIVE = os.environ.get('ORACLE_DRIVE', 'r').upper() + ':'
COMPAT = [os.path.join(ORACLE, 'vc8_compat.h')] + \
    [os.path.join(ORACLE, 'shim_include', f) for f in os.listdir(os.path.join(ORACLE, 'shim_include'))]
INCLUDING = re.compile(r'^Note: including file:\s*(.*?)\s*$')
ERROR = re.compile(r'\b(error|fatal error) C\d{4}\b')
FLAGS_TSV = os.path.join(ORACLE, 'flags.tsv')
ERROR_AT = re.compile(r'^(?P<file>.+?)\((?P<line>\d+)\) : (?:fatal )?error (?P<code>C\d{4}): (?P<msg>.*)$')
QUARANTINE_TSV = os.path.join(BUILD, 'quarantine.tsv')
MAX_QUARANTINE_ROUNDS = 40   # an ICE is fatal, so each round finds one; every round adds one
COMPILE_TIMEOUT = 300       # seconds; the largest TU takes well under a minute
_qlock = threading.Lock()
CACHE_VERSION = 'build-1'   # bump when what a cached compile holds, or how it is keyed, changes
BATCH = 16                  # TUs per cl.exe at most (compile_batch)
# cl.exe processes at a time. With batches, more than about six only contend:
# 448 TUs compiled in 107s at -j6 against 123s at -j8, 143s at -j12 and 165s
# at -j16 (batch 16, a 16-core M3 Max under load).
JOBS = min(os.cpu_count() or 4, 6)


class TU:
    """One translation unit: its path in the tree (rel), its translation (src),
    its object without .obj (base), and which status file it reports to."""
    __slots__ = ('rel', 'src', 'objdir', 'base', 'statusdir')

    def __init__(self, rel, src, objdir, statusdir):
        self.rel = rel
        self.src = src
        self.objdir = objdir
        self.base = os.path.join(objdir, rel[:-4])
        self.statusdir = statusdir


def win(path):
    rel = os.path.relpath(os.path.realpath(path), os.path.realpath(REPO))
    if rel.startswith('..'):
        raise ValueError('%s is outside the repository' % path)
    return DRIVE + '\\' + rel.replace('/', '\\')


def linux(winpath):
    """A path from /showIncludes back to the host: only repo paths matter for
    dependencies; toolchain headers never change between builds."""
    p = winpath.replace('\\', '/')
    if p[:2].upper() == DRIVE:
        return os.path.normpath(os.path.join(REPO, p[3:]))
    return None


def load_flags_table():
    rows = []
    if os.path.exists(FLAGS_TSV):
        for line in open(FLAGS_TSV):
            if line.startswith('#') or not line.strip():
                continue
            parts = line.rstrip('\n').split('\t')
            rows.append((parts[0], parts[1] if len(parts) > 1 else ''))
    return sorted(rows, key=lambda r: -len(r[0]))


FLAGS_TABLE = load_flags_table()


def extra_flags(tu):
    """The flags.tsv row with the longest prefix of `tu`, or ''."""
    for prefix, flags in FLAGS_TABLE:
        if tu.startswith(prefix):
            return flags
    return ''


def write_if_changed(path, text):
    old = open(path).read() if os.path.exists(path) else None
    if old != text:
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, 'w') as f:
            f.write(text)


def write_rsp():
    dirs = set()
    for dirpath, _dirs, files in os.walk(SRC):
        if any(f.endswith('.h') for f in files):
            dirs.add(dirpath)
    lines = ['/nologo /c ' + CFLAGS + ' ' + TOOL_CFLAGS,
             '/FI' + win(os.path.join(ORACLE, 'vc8_compat.h'))]
    if USE_PCH:
        write_if_changed(os.path.join(PCHDIR, PCH_HEADER),
                         '// The prefix every TU parses, precompiled once (build.py, ORACLE_PCH=1).\n'
                         '#include "%s"\n#include "Source/globals/globals.h"\n'
                         % vc8_translate.PLATFORM_H.replace(os.sep, '/'))
        write_if_changed(os.path.join(PCHDIR, 'oracle_pch.cpp'), '// precompiled-header creation unit\n')
        lines += ['/FI' + PCH_HEADER, '/I' + win(PCHDIR), '/Fp' + win(PCH_FILE)]
    else:
        lines += ['/FI' + win(os.path.join(SRC, vc8_translate.PLATFORM_H))]
    lines += ['/I' + win(os.path.join(ORACLE, 'shim_include')),
              '/I' + win(SRC)]
    lines += ['/I' + win(d) for d in sorted(dirs) if d != SRC]
    rsp = os.path.join(BUILD, 'cl.rsp')
    write_if_changed(rsp, '\n'.join(lines) + '\n')
    return rsp


def build_pch(rsp, start_wine=lambda: None):
    """(Re)create the precompiled header when it is older than anything it
    was made from. -> (an error string or None, what was done: 'current',
    'from cache' or 'compiled')."""
    dep = os.path.join(PCHDIR, 'oracle.d')
    pobj = os.path.join(PCHDIR, 'oracle_pch.obj')
    sidecar = os.path.join(PCHDIR, 'oracle.key')
    base = objcache.sha_bytes(CACHE_VERSION, 'pch', compiler_id(), open(rsp).read(), compat_id(),
                              open(os.path.join(PCHDIR, PCH_HEADER)).read(),
                              open(os.path.join(PCHDIR, 'oracle_pch.cpp')).read(), win(PCH_FILE), win(pobj))
    key = objcache.lookup(base, hash_dep)
    if key is not None:
        if read_key(sidecar) == key and os.path.exists(PCH_FILE) and os.path.exists(pobj):
            return None, 'current'
        d = objcache.get('pch', key)
        if d is not None:
            objcache.install(os.path.join(d, 'pch'), PCH_FILE)
            objcache.install(os.path.join(d, 'obj'), pobj)
            write_text(dep, ''.join(os.path.join(REPO, r) + '\n' for r in objcache.meta(d)['deps']))
            write_text(sidecar, key)
            return None, 'from cache'
    for f in (PCH_FILE, dep, sidecar):
        if os.path.exists(f):
            os.remove(f)
    start_wine()
    cmd = [os.path.join(HERE, 'vc8.sh'), 'cl', '@' + win(rsp), '/showIncludes', '/Yc' + PCH_HEADER,
           '/Fo' + win(os.path.join(PCHDIR, 'oracle_pch.obj')), win(os.path.join(PCHDIR, 'oracle_pch.cpp'))]
    p = subprocess.run(cmd, capture_output=True, text=True, encoding='latin1')
    deps = [linux(m.group(1)) for m in map(INCLUDING.match, (p.stdout + p.stderr).splitlines()) if m]
    errors = [l for l in (p.stdout + p.stderr).splitlines() if ERROR.search(l)]
    if errors or not os.path.exists(PCH_FILE):
        return (errors[0] if errors else 'no precompiled header produced'), 'compiled'
    with open(dep, 'w') as f:
        f.write('\n'.join(sorted(set(d for d in deps if d))) + '\n')
    sdeps = store_deps(deps_of(dep))
    key = objcache.sha_bytes(base, json.dumps(sdeps))
    objcache.put('pch', key, {'pch': PCH_FILE, 'obj': pobj}, {'deps': [r for r, _h in sdeps]})
    objcache.record(base, sdeps, key)
    write_text(sidecar, key)
    return None, 'compiled'


_compiler_id = None


def compiler_id():
    """Hash of the compiler's own binaries: a service pack or a patched c2.dll
    is another compiler. Toolchain headers are not hashed; they come with it."""
    global _compiler_id
    if _compiler_id is None:
        prefix = os.environ.get('WINEPREFIX') or os.environ.get('VC8_WINEPREFIX', '')
        bindir = os.path.join(prefix, 'drive_c', 'vc8', VC, 'bin')
        names = sorted(n for n in os.listdir(bindir) if n.lower().endswith(('.exe', '.dll')))
        _compiler_id = objcache.sha_bytes(VC, *[n + objcache.sha_file(os.path.join(bindir, n)) for n in names])
    return _compiler_id


def repo_rel(path):
    return os.path.relpath(path, REPO)


def hash_dep(rel):
    return objcache.sha_file(os.path.join(REPO, rel))


def deps_of(dfile):
    return [l.rstrip('\n') for l in open(dfile) if l.strip()] if os.path.exists(dfile) else []


def store_deps(deps):
    """Dependencies as a manifest records them: repo-relative, with their hashes."""
    rels = sorted(set(repo_rel(d) for d in deps))
    return [[r, hash_dep(r)] for r in rels]


def compat_id():
    files = []
    for p in COMPAT:
        if os.path.isdir(p):
            files += [os.path.join(d, f) for d, _ds, fs in os.walk(p) for f in fs]
        else:
            files.append(p)
    return objcache.sha_bytes(*[repo_rel(p) + objcache.sha_file(p) for p in sorted(files)])


def read_key(path):
    return open(path).read().strip() if os.path.exists(path) else None


def write_text(path, text):
    tmp = '%s.%d.tmp' % (path, os.getpid())
    with open(tmp, 'w') as f:
        f.write(text)
    os.replace(tmp, path)


def unquarantined_text(t):
    """The TU's translation with none of its functions blanked: what a compile
    starts from (compile_one lifts the TU's quarantine first)."""
    if t.rel in vc8_translate.QUARANTINE:
        saved = vc8_translate.QUARANTINE.pop(t.rel)
        try:
            orig = open(os.path.join(REPO, t.rel), encoding='utf-8', errors='surrogateescape').read()
            return vc8_translate.translate(orig, t.rel)
        finally:
            vc8_translate.QUARANTINE[t.rel] = saved
    return open(t.src, encoding='utf-8', errors='surrogateescape').read()


def tu_base_key(t, rsp_text, pch_id):
    return objcache.sha_bytes(CACHE_VERSION, compiler_id(), rsp_text, extra_flags(t.rel), pch_id,
                              win(t.src), win(t.base + '.obj'), compat_id(), unquarantined_text(t))


def quarantine_rows(rel):
    if not os.path.exists(QUARANTINE_TSV):
        return []
    return [l.rstrip('\n') for l in open(QUARANTINE_TSV) if l.split('\t', 1)[0] == rel]


def write_quarantine(lines):
    """quarantine.tsv as `lines` (its header first), or no file when no row is
    left: an incremental build then holds what a clean one would."""
    if any(l.split('\t', 1)[0] != 'file' for l in lines):
        with open(QUARANTINE_TSV, 'w') as f:
            f.writelines(lines)
    elif os.path.exists(QUARANTINE_TSV):
        os.remove(QUARANTINE_TSV)


def set_quarantine(t, rows):
    """Make the TU's rows in quarantine.tsv exactly `rows`, and its
    translation follow them."""
    with _qlock:
        lines = open(QUARANTINE_TSV).readlines() if os.path.exists(QUARANTINE_TSV) else []
        if [l.rstrip('\n') for l in lines if l.split('\t', 1)[0] == t.rel] == list(rows):
            return                      # the mirror translated it under these rows already
        keep = [l for l in lines if l.split('\t', 1)[0] != t.rel]
        if not keep and rows:
            keep = ['file\tline\tfunction_line\tfunction\terror\n']
        new = keep + [r + '\n' for r in rows]
        if new != lines:
            write_quarantine(new)
        vc8_translate.load_quarantine(QUARANTINE_TSV)
        orig = open(os.path.join(REPO, t.rel), encoding='utf-8', errors='surrogateescape').read()
        out = vc8_translate.translate(orig, t.rel).encode('utf-8', errors='surrogateescape')
        if not os.path.exists(t.src) or open(t.src, 'rb').read() != out:
            with open(t.src, 'wb') as f:
                f.write(out)


def restore(t, key):
    """Put the cached compile `key` in place for TU t. Returns True when it did."""
    d = objcache.get('obj', key)
    if d is None:
        return False
    info = objcache.meta(d)
    set_quarantine(t, info.get('quarantine', []))
    os.makedirs(os.path.dirname(t.base), exist_ok=True)
    objcache.install(os.path.join(d, 'obj'), t.base + '.obj')
    objcache.install(os.path.join(d, 'log'), t.base + '.log')
    write_text(t.base + '.d', ''.join(os.path.join(REPO, r) + '\n' for r in info['deps']))
    write_text(t.base + '.flags', extra_flags(t.rel))
    write_text(t.base + '.key', key)
    return True


def store(t, base):
    """Record a successful compile of TU t in the cache, and its key beside the object."""
    deps = store_deps(deps_of(t.base + '.d'))
    key = objcache.sha_bytes(base, json.dumps(deps))
    objcache.put('obj', key, {'obj': t.base + '.obj', 'log': t.base + '.log'},
                 {'tu': t.rel, 'deps': [r for r, _h in deps], 'quarantine': quarantine_rows(t.rel)})
    objcache.record(base, deps, key)
    write_text(t.base + '.key', key)


def valid_object(path):
    """True if `path` is a complete i386 COFF object: every section's data and
    relocations, the symbol table and the string table lie inside the file."""
    if not os.path.exists(path):
        return False
    try:
        o = coffobj.CoffObject(open(path, 'rb').read(), path)
        return o.comp_id() is not None
    except Exception:
        return False


def quarantine(t, errors_log):
    """Record the functions holding the TU's own compile errors in
    quarantine.tsv and re-translate the TU with their bodies blanked. Returns
    the number of functions newly quarantined (0: nothing that can be)."""
    tu, src = t.rel, t.src
    own = []
    for line in errors_log:
        m = ERROR_AT.match(line.strip())
        if m and linux(m.group('file')) and os.path.normcase(linux(m.group('file'))).lower() == src.lower():
            own.append((int(m.group('line')), m.group('code') + ' ' + m.group('msg')))
    if not own:
        return 0
    text = open(src, encoding='utf-8', errors='surrogateescape').read()
    rows = []
    for ln, msg in own:
        b = vc8_translate.function_body_at(text, ln)
        if b is None:
            return 0                      # an error outside any function: cannot quarantine
        head_line = text.count('\n', 0, b[0]) + 1
        _, done = vc8_translate.quarantine_bodies(text, [ln])
        name = done[0][1] if done else '?'
        rows.append((tu, ln, head_line, name, msg))
    with _qlock:
        known = set()
        if os.path.exists(QUARANTINE_TSV):
            known = {tuple(l.split('\t')[:2]) for l in open(QUARANTINE_TSV)}
        new = [r for r in rows if (r[0], str(r[1])) not in known]
        with open(QUARANTINE_TSV, 'a') as f:
            if not known:
                f.write('file\tline\tfunction_line\tfunction\terror\n')
            for r in new:
                f.write('%s\t%d\t%d\t%s\t%s\n' % r)
        vc8_translate.load_quarantine(QUARANTINE_TSV)
        orig = open(os.path.join(REPO, tu), encoding='utf-8', errors='surrogateescape').read()
        out = vc8_translate.translate(orig, tu)
        with open(src, 'w', encoding='utf-8', errors='surrogateescape') as f:
            f.write(out)
    return len(new)


def unquarantine(t):
    """Forget the TU's quarantined functions and re-translate it without them:
    every recompile first tries the real bodies, so a quarantine whose cause
    has since been fixed (in the tree or in the oracle) lifts itself."""
    tu = t.rel
    with _qlock:
        if not os.path.exists(QUARANTINE_TSV):
            return
        lines = open(QUARANTINE_TSV).readlines()
        keep = [l for l in lines if l.split('\t', 1)[0] != tu]
        if len(keep) == len(lines):
            return
        write_quarantine(keep)
        vc8_translate.load_quarantine(QUARANTINE_TSV)
        orig = open(os.path.join(REPO, tu), encoding='utf-8', errors='surrogateescape').read()
        with open(t.src, 'w', encoding='utf-8', errors='surrogateescape') as f:
            f.write(vc8_translate.translate(orig, tu))


def compile_one(t, rsp, base_key, first=None):
    """Compile TU t, quarantining what VC8 cannot compile, round by round.
    first: the result of a compile already made (compile_batch), as
    compile_once returns it; the rounds then go on from there."""
    if first is None:
        unquarantine(t)
    for _round in range(MAX_QUARANTINE_ROUNDS + 1):
        if first is not None:
            _t, ok, errors, secs, log = first
            first = None
        else:
            _t, ok, errors, secs, log = compile_once(t, rsp)
        if ok or not quarantine(t, log):
            break
    if ok:
        store(t, base_key)
    return t, ok, errors, secs


def clear_outputs(t):
    os.makedirs(os.path.dirname(t.base), exist_ok=True)
    for ext in ('.obj', '.d', '.key'):
        if os.path.exists(t.base + ext):
            os.remove(t.base + ext)


def cl_command(rsp, tu, out, sources):
    return [os.path.join(HERE, 'vc8.sh'), 'cl', '@' + win(rsp), '/showIncludes'] + \
        extra_flags(tu).split() + \
        (['/Yu' + PCH_HEADER] if USE_PCH else []) + \
        ['/Fo' + out] + [win(s) for s in sources]


def compile_once(t, rsp):
    tu = t.rel
    clear_outputs(t)
    cmd = cl_command(rsp, tu, win(t.base + '.obj'), [t.src])
    t0 = time.time()
    for attempt in (1, 2):
        try:
            p = subprocess.run(cmd, capture_output=True, text=True, encoding='latin1', timeout=COMPILE_TIMEOUT)
            break
        except subprocess.TimeoutExpired:
            # A compile under wine can block indefinitely (seen on the first
            # processes of a fresh wineserver); one retry, then give up.
            if attempt == 2:
                return tu, False, ['timed out twice after %ds' % COMPILE_TIMEOUT], time.time() - t0, []
    return finish(t, p.stdout + p.stderr, p.returncode, time.time() - t0)


def finish(t, out, returncode, secs):
    """Judge one TU's compile from the compiler's output for it: write its
    log, and on success its dependencies. -> as compile_once."""
    tu, base = t.rel, t.base
    deps, log = [], []
    for line in out.splitlines():
        m = INCLUDING.match(line)
        if m:
            path = linux(m.group(1))
            if path:
                deps.append(path)
        else:
            log.append(line)
    with open(base + '.log', 'w') as f:
        f.write('\n'.join(log) + '\n')
    errors = [l for l in log if ERROR.search(l)]
    # The exit status is not trusted: under wine on box64 a compile that has
    # written its object can still be SIGKILLed on its way out (-9 here, 137
    # in a shell) when the wineserver tears the process down before its threads
    # finish exiting. cl.exe deletes the object on any real error, so success
    # is: an object that parses completely as COFF, and no error in the log.
    ok = not errors and valid_object(base + '.obj')
    if returncode not in (0, 137, -9) and ok:
        ok = False
        errors = ['cl.exe exit status %d' % returncode]
    if ok:
        with open(base + '.d', 'w') as f:
            f.write('\n'.join(sorted(set(deps))) + '\n')
        with open(base + '.flags', 'w') as f:
            f.write(extra_flags(tu))
    return tu, ok, errors, secs, log


def batches(todo, size):
    """Split the TUs to compile, (TU, base key) pairs, into lists that one
    cl.exe can compile together: one object directory - an object's path is
    written into its .debug$S (S_OBJNAME), so it must come out exactly where
    a compile of its own would put it, and one /Fo directory serves every
    source - and one set of flags. Each group goes into batches of at most
    `size`, of near-equal length; the largest batches (by source size) come
    first, so the longest compiles start early."""
    groups = {}
    for t, base in todo:
        groups.setdefault((os.path.dirname(t.base), extra_flags(t.rel)), []).append((t, base))
    out = []
    for _k, items in sorted(groups.items()):
        items.sort(key=lambda x: (-os.path.getsize(x[0].src), x[0].rel))
        n = -(-len(items) // max(1, size))
        parts = [items[k::n] for k in range(n)]
        for part in parts:
            # Two sources one case apart would write the same object on a
            # case-insensitive drive: each gets a batch of its own.
            names = [os.path.basename(t.src).lower() for t, _b in part]
            if len(set(names)) == len(names):
                out.append(part)
            else:
                out += [[x] for x in part]
    out.sort(key=lambda b: (-sum(os.path.getsize(t.src) for t, _b in b), b[0][0].rel))
    return out


BACKEND = 'Generating Code...'
MESSAGE_FILE = re.compile(r'^(\S.*?)(?:\(\d+\))? : ')


def split_output(stdout, stderr, sources):
    """The compiler's output for several sources, cut into what a compile of
    each alone prints, or None where it cannot be. On stdout the front end
    goes through the sources in the order given, each starting with a line
    holding its file name alone, followed by its /showIncludes notes,
    warnings and errors. Then, after 'Generating Code...', the back end's
    messages (C4700, an internal error) for all of them, in an order of its
    own, each naming its source; an indented line continues the one above.
    stderr has the driver's command-line warnings (D9025), once for the run:
    each source's own compile would print them, last."""
    names = [os.path.basename(s) for s in sources]
    paths = {win(s).lower(): k for k, s in enumerate(sources)}
    parts, k, backend = [[] for _ in names], -1, False
    for line in stdout.splitlines():
        if backend:
            m = MESSAGE_FILE.match(line)
            if m:
                k = paths.get(m.group(1).lower())
                if k is None:
                    return None         # a message about no source of the batch
            elif line.strip() and (k is None or not line[:1].isspace()):
                return None
            if line.strip():
                parts[k].append(line)
            continue
        if k == len(names) - 1 and line == BACKEND:
            backend, k = True, None
            continue
        if k + 1 < len(names) and line.strip() == names[k + 1]:
            k += 1
        if k < 0:
            if line.strip():
                return None             # output before the first source: whose is it?
            continue
        parts[k].append(line)
    if not backend and k != len(names) - 1:
        return None
    return ['\n'.join(p + stderr.splitlines()) + '\n' for p in parts]


def compile_batch(batch, rsp):
    """Compile a batch (batches()) in one cl.exe: starting the compiler under
    wine costs as much as compiling a small TU. Each TU is then judged on its
    own output exactly as compile_once judges it, and one that failed goes on
    through compile_one's quarantine rounds alone. When the output cannot be
    attributed to its sources, or the compiler failed without saying where,
    every TU of the batch is compiled again on its own. -> compile_one's
    result for each TU."""
    if len(batch) == 1:
        t, base = batch[0]
        return [compile_one(t, rsp, base)]
    for t, _b in batch:
        unquarantine(t)
        clear_outputs(t)
    first = batch[0][0]
    cmd = cl_command(rsp, first.rel, win(os.path.dirname(first.base)) + '\\', [t.src for t, _b in batch])
    t0 = time.time()
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, encoding='latin1',
                           timeout=COMPILE_TIMEOUT * len(batch))
    except subprocess.TimeoutExpired:
        p = None
    secs = time.time() - t0
    parts = split_output(p.stdout, p.stderr, [t.src for t, _b in batch]) if p is not None else None
    if parts is not None:
        results = [finish(t, part, 0, secs / len(batch)) for (t, _b), part in zip(batch, parts)]
        if p.returncode not in (0, 137, -9) and all(r[1] for r in results):
            parts = None                # it failed, and no source says why
    if parts is None:
        return [compile_one(t, rsp, base) for t, base in batch]
    # A TU with no object and no error of its own may have been cut short by
    # another's: it is compiled again alone.
    return [compile_one(t, rsp, base, first=r) if r[1] or r[2] else compile_one(t, rsp, base)
            for (t, base), r in zip(batch, results)]


def sweep_box64_cache():
    """Delete box64's orphaned dynarec cache files. box64 writes a process's
    cache to <module>.box64.tmp.<pid>.N and renames it when the process
    exits; a cl.exe SIGKILLed on its way out under wine never gets there, so
    every compile leaves its c1xx.dll cache behind (about 12 MB, never read
    again) - thousands of them filled the disk once. A file whose process is
    gone is garbage."""
    folder = os.environ.get('BOX64_DYNACACHE_FOLDER') or os.path.expanduser('~/.cache/box64')
    freed = 0
    for path in glob.glob(os.path.join(folder, '*.box64.tmp.*')):
        m = re.search(r'\.tmp\.(\d+)\.\d+$', path)
        if m and not os.path.exists('/proc/' + m.group(1)):
            try:
                freed += os.path.getsize(path)
                os.remove(path)
            except OSError:
                pass
    return freed


def translate_support(extra_src, names):
    """Translate the hand-written support TUs `names` into extra_src/Redivivus,
    with the same per-tree state as the mirror; drop any no longer named."""
    out = os.path.join(extra_src, SUPPORT)
    os.makedirs(out, exist_ok=True)
    n = 0
    for name in names:
        rel = os.path.join(SUPPORT, name)
        with open(os.path.join(REPO, rel), encoding='utf-8', errors='surrogateescape') as f:
            data = vc8_translate.translate(f.read(), rel).encode('utf-8', errors='surrogateescape')
        dst = os.path.join(out, name)
        if not os.path.exists(dst) or open(dst, 'rb').read() != data:
            with open(dst, 'wb') as f:
                f.write(data)
            n += 1
    for f in os.listdir(out):
        if f.endswith('.cpp') and f not in names:
            os.remove(os.path.join(out, f))
    return n


OBJ_EXTS = ('.obj', '.d', '.log', '.flags', '.key', '.fn.pkl', '.fn.sum')


def prune(expected_src, keep_obj):
    """Remove translations of files the tree no longer has, and objects of TUs
    no longer compiled into a directory: a clean build would not have them.
    keep_obj: object directory -> predicate on an object's path without
    extension, relative to that directory. Existence is asked of the file
    system, not compared as strings, so a case-insensitive one agrees."""
    removed = 0
    for dirpath, _dirs, files in os.walk(SRC):
        for f in files:
            p = os.path.join(dirpath, f)
            if f.endswith(vc8_translate.EXTS) and p not in expected_src and \
                    os.path.normcase(p) not in expected_src:
                os.remove(p)
                removed += 1
    for objdir, keep in keep_obj.items():
        for dirpath, _dirs, files in os.walk(objdir):
            for f in files:
                ext = next((e for e in OBJ_EXTS if f.endswith(e)), None)
                if ext and not keep(os.path.relpath(os.path.join(dirpath, f[:-len(ext)]), objdir)):
                    os.remove(os.path.join(dirpath, f))
                    removed += 1
    return removed


def _decode(objdir, rel):
    import ours
    ours.load_object(objdir, rel)


class Decoder:
    """Decode each object the compile finishes (ours.load_object) in a pool
    of processes while the next batches compile, so funcmap.py, libmap.py and
    audit.py find every object decoded. It warms a cache keyed by content
    and changes no result: a decode that fails here is redone by the reader.
    jobs 0 decodes nothing here."""

    def __init__(self, jobs):
        self.pool = None
        self.futs = []
        if jobs > 0:
            import multiprocessing
            self.pool = cf.ProcessPoolExecutor(jobs, mp_context=multiprocessing.get_context('spawn'))

    def submit(self, t):
        if self.pool is not None:
            self.futs.append(self.pool.submit(_decode, t.objdir, t.rel[:-4] + '.obj'))

    def finish(self):
        if self.pool is None:
            return 0
        done = 0
        for f in self.futs:
            try:
                f.result()
                done += 1
            except Exception:
                pass
        self.pool.shutdown()
        return done


def write_status(statusdir, tus, status):
    path = os.path.join(statusdir, 'compile_status.tsv')
    with open(path, 'w') as f:
        f.write('tu\tstatus\terrors\tfirst_error\n')
        for t in sorted(tus, key=lambda t: t.rel):
            if t.rel in status:
                f.write('\t'.join(status[t.rel]) + '\n')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('-j', '--jobs', type=int, default=JOBS, help='cl.exe processes at a time (default %d)' % JOBS)
    ap.add_argument('--batch', type=int, default=int(os.environ.get('ORACLE_BATCH', BATCH)),
                    help='TUs one cl.exe compiles at most (default %d, or ORACLE_BATCH); 1: one each' % BATCH)
    ap.add_argument('--decode-jobs', type=int, default=int(os.environ.get('ORACLE_DECODE_JOBS', '2')),
                    help='processes decoding finished objects during the compile (default 2, or '
                         'ORACLE_DECODE_JOBS); 0: leave decoding to the readers')
    ap.add_argument('--force', action='store_true', help='recompile every TU, reading nothing from the cache')
    ap.add_argument('--with-data', action='store_true', help='also compile Source/globals/ (data only)')
    ap.add_argument('--extra', metavar='DIR', help='compile Source/globals/ and --support TUs into DIR/obj (build-vc8.sh)')
    ap.add_argument('--support', default='', help='Redivivus/ TUs to compile into --extra, space-separated')
    ap.add_argument('patterns', nargs='*')
    args = ap.parse_args()
    if args.force:
        objcache.READ = False
    if args.support and not args.extra:
        ap.error('--support needs --extra')

    os.makedirs(BUILD, exist_ok=True)
    lockf = open(os.path.join(BUILD, '.build.lock'), 'w')
    fcntl.flock(lockf, fcntl.LOCK_EX)       # one build of this directory at a time

    if EXPERIMENT:
        print('*** EXPERIMENT: ORACLE_VC=%s ORACLE_CFLAGS=%r -> %s' % (VC, CFLAGS, BUILD), file=sys.stderr)
        print('*** evidence from this build must not be committed', file=sys.stderr)
    # The translation is a function of the tree's sources, the translator and
    # the quarantine: skipped when none of them changed since it last ran.
    t_start = time.time()
    files = list(vc8_translate.sources(REPO))
    mkey = objcache.sha_bytes(CACHE_VERSION, objcache.sha_file(os.path.join(HERE, 'vc8_translate.py')),
                              objcache.sha_file(QUARANTINE_TSV),
                              *[os.path.join(r, n) + objcache.sha_file(os.path.join(REPO, r, n)) for r, n in files])
    # The per-tree state translate() reads (thiscall names, constructor-
    # stripped structs) is kept with the key, for what is translated later
    # (support TUs, a quarantine); and with it each source's hash, so that
    # when the state comes out the same only changed sources are translated.
    mstamp = os.path.join(BUILD, 'mirror.key')
    mstate = os.path.join(BUILD, 'mirror.state.pkl')
    names = ('THISCALL_FUNCS', 'THISCALL_PTRS', 'STRIP_CTORS', 'CONSTRUCT', 'THISCALL_DIRECT')
    prev = None
    if objcache.READ and os.path.exists(mstate):
        try:
            with open(mstate, 'rb') as fh:
                prev = pickle.load(fh)          # (key, state, translator hash, {path: (hash, quarantine)})
        except Exception:
            prev = None
    if prev is not None and read_key(mstamp) == mkey and prev[0] == mkey and \
            all(os.path.exists(os.path.join(SRC, r, n)) for r, n in files):
        vc8_translate.load_quarantine(QUARANTINE_TSV)
        for name, value in zip(names, prev[1]):
            getattr(vc8_translate, name).clear()
            getattr(vc8_translate, name).update(value)
        hashes = prev[3]
        print('translate: tree unchanged, %d kept (%.1fs)' % (len(files), time.time() - t_start))
    else:
        # A file's translation is a function of the translator, the per-tree
        # state, its source and its own quarantined lines: a changed
        # translator retranslates every file, a changed quarantine only the
        # files it names.
        thash = objcache.sha_file(os.path.join(HERE, 'vc8_translate.py'))
        sha = {os.path.join(r, n): objcache.sha_file(os.path.join(REPO, r, n)) for r, n in files}

        def file_key(path):
            return sha[path], tuple(sorted(vc8_translate.QUARANTINE.get(os.path.normpath(path), ())))

        def unchanged(r, n, _text):
            # Same state, same quarantine, same source: the same translation.
            return prev[1] == state[0] and prev[2] == thash and \
                prev[3].get(os.path.join(r, n)) == file_key(os.path.join(r, n))
        state = []
        if prev is not None:
            def unchanged_once(r, n, text):
                if not state:
                    state.append(vc8_translate.mirror_state())
                return unchanged(r, n, text)
        # What the pre-pass found in each file, by content, kept for the
        # next run: only the files that changed are read for it again.
        mfacts = os.path.join(BUILD, 'mirror.facts.pkl')
        facts = {}
        if objcache.READ and os.path.exists(mfacts):
            try:
                with open(mfacts, 'rb') as fh:
                    saved = pickle.load(fh)         # (translator hash, facts)
                if saved[0] == thash:
                    facts = saved[1]
            except Exception:
                facts = {}
        w, k = vc8_translate.mirror(REPO, SRC, QUARANTINE_TSV, unchanged_once if prev is not None else None, facts)
        hashes = {path: file_key(path) for path in sha}
        for path, obj in ((mfacts, (thash, facts)), (mstate, (mkey, vc8_translate.mirror_state(), thash, hashes))):
            tmp = '%s.%d.tmp' % (path, os.getpid())
            with open(tmp, 'wb') as fh:
                pickle.dump(obj, fh)
            os.replace(tmp, path)
        write_text(mstamp, mkey)
        print('translate: %d written, %d unchanged (%.1fs)' % (w, k, time.time() - t_start))
    extra_src = extra_obj = None
    if args.extra:
        extra = os.path.abspath(args.extra)
        extra_src, extra_obj = os.path.join(extra, 'src'), os.path.join(extra, 'obj')
        n = translate_support(extra_src, args.support.split())
        print('support: %d TUs, %d written' % (len(args.support.split()), n))
    rsp = write_rsp()
    rsp_text = open(rsp).read()
    # Start the wine session before the parallel phase, with no pipes attached.
    # The first wine process of a session spawns wine's background services
    # (explorer, winedevice, ...), which inherit its stdout and stderr: a
    # compile captured through pipes would then wait for an end-of-file that
    # only comes when those services exit - at the end of the build. A
    # persistent wineserver (-p) keeps the session, and the services, up.
    env = dict(os.environ, WINEPREFIX=os.environ.get('VC8_WINEPREFIX', os.path.expanduser(
        '~/q2-dev/asm-matching-toolchain/wine-vc8')))
    quiet = dict(stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    wine_up = []

    def start_wine():
        if not wine_up:
            subprocess.run(['wineserver', '-p'], env=env, **quiet)
            subprocess.run([os.path.join(HERE, 'vc8.sh'), 'cl', '/nologo'], timeout=300, **quiet)
            wine_up.append(1)

    pch_id = ''
    if USE_PCH:
        t_pch = time.time()
        err, how = build_pch(rsp, start_wine)
        if err:
            print('precompiled header failed: %s' % err, file=sys.stderr)
            return 1
        print('precompiled header: %s (%.1fs)' % (how, time.time() - t_pch))
        pch_id = objcache.sha_file(PCH_FILE) + objcache.sha_file(os.path.join(PCHDIR, 'oracle_pch.obj'))

    expected_src = set()
    tus = []
    for rel, name in vc8_translate.sources(REPO):
        relpath = os.path.normpath(os.path.join(rel, name))
        expected_src.add(os.path.join(SRC, relpath))
        if not name.endswith('.cpp'):
            continue
        if relpath.startswith(os.path.join('Source', 'globals') + os.sep):
            if args.with_data:
                tus.append(TU(relpath, os.path.join(SRC, relpath), extra_obj or OBJ, args.extra or BUILD))
        else:
            tus.append(TU(relpath, os.path.join(SRC, relpath), OBJ, BUILD))
    for name in args.support.split():
        relpath = os.path.join(SUPPORT, name)
        tus.append(TU(relpath, os.path.join(extra_src, relpath), extra_obj, args.extra))
    tus.sort(key=lambda t: t.rel)
    globals_dir = (extra_obj or OBJ) if args.with_data else extra_obj
    data = os.path.join('Source', 'globals') + os.sep
    keep = {OBJ: lambda r: not r.startswith(data) and os.path.exists(os.path.join(SRC, r + '.cpp'))
            or (globals_dir == OBJ and r.startswith(data) and os.path.exists(os.path.join(SRC, r + '.cpp')))}
    if extra_obj:
        support = {os.path.join(SUPPORT, n[:-4]) for n in args.support.split()}
        keep[extra_obj] = lambda r: r in support or (r.startswith(data) and os.path.exists(os.path.join(SRC, r + '.cpp')))
    pruned = prune(expected_src, keep)
    if args.patterns:
        tus = [t for t in tus if any(p in t.rel for p in args.patterns)]

    # Which TUs need a compile: none whose inputs a stored compile matches.
    t0 = time.time()
    todo, ready, restored, current = [], [], 0, 0
    for t in tus:
        base = tu_base_key(t, rsp_text, pch_id)
        key = objcache.lookup(base, hash_dep)
        if key is not None and read_key(t.base + '.key') == key and os.path.exists(t.base + '.obj') \
                and os.path.exists(t.base + '.d'):
            current += 1
            ready.append(t)
        elif key is not None and restore(t, key):
            restored += 1
            ready.append(t)
        else:
            todo.append((t, base))
    work = batches(todo, args.batch)
    print('%d TUs: %d unchanged, %d from cache, %d to compile in %d cl.exe, %d jobs%s (%.1fs)' % (
        len(tus), current, restored, len(todo), len(work), args.jobs,
        ', %d stale files removed' % pruned if pruned else '', time.time() - t0))

    status = {}
    for d in {t.statusdir for t in tus}:
        status_file = os.path.join(d, 'compile_status.tsv')
        if os.path.exists(status_file):
            for line in open(status_file):
                parts = line.rstrip('\n').split('\t')
                if len(parts) >= 2 and parts[0] != 'tu':
                    status[parts[0]] = parts
    # Only a compile that succeeded is stored, so an object in place or
    # restored is one: a row a failed compile left for it is stale.
    for t in ready:
        status[t.rel] = [t.rel, 'ok', '0', '']
    failed = 0
    if todo:
        start_wine()
    decoder = Decoder(args.decode_jobs)
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futs = [pool.submit(compile_batch, b, rsp) for b in work]
        n = 0
        for fut in cf.as_completed(futs):
            for t, ok, errors, secs in fut.result():
                n += 1
                failed += not ok
                first = errors[0].strip() if errors else ''
                status[t.rel] = [t.rel, 'ok' if ok else 'FAIL', str(len(errors)), first[:300]]
                print('[%d/%d] %-4s %5.1fs %s%s' % (n, len(todo), 'ok' if ok else 'FAIL', secs, t.rel,
                                                    '' if ok else '  ' + first[:160]))
                if ok and t.objdir == OBJ:
                    decoder.submit(t)
    t1 = time.time()
    decoded = decoder.finish()
    if decoded:
        print('decoded %d objects alongside the compile (%.1fs after it)' % (decoded, time.time() - t1))
    for t in tus:
        if t.rel not in status and os.path.exists(t.base + '.obj'):
            status[t.rel] = [t.rel, 'ok', '0', '']
    for d in {t.statusdir for t in tus}:
        group = [t for t in tus if t.statusdir == d]
        if args.patterns:
            # Keep the rows of the TUs this run did not look at.
            path = os.path.join(d, 'compile_status.tsv')
            seen = {t.rel for t in group}
            if os.path.exists(path):
                for line in open(path):
                    parts = line.rstrip('\n').split('\t')
                    if parts[0] != 'tu' and parts[0] not in seen and os.path.exists(os.path.join(SRC, parts[0])):
                        group.append(TU(parts[0], os.path.join(SRC, parts[0]), OBJ, d))
        write_status(d, group, status)
    nok = sum(1 for t in tus if status.get(t.rel, [None, ''])[1] == 'ok')
    sweep_box64_cache()
    print('compiled %d/%d TUs OK (%d failed this run) in %.0fs' % (nok, len(tus), failed, time.time() - t0))
    return 0 if nok == len(tus) else 1


if __name__ == '__main__':
    sys.exit(main())
