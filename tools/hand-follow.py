#!/usr/bin/env python3
"""Carry the hand-written delta of source/modified through file and TU moves.

source/modified is the generator output (source/regen) plus a small hand delta. Every
generated function carries a `// 1.14d: <addr>` line, so a hand hunk can be keyed by the
function it sits in instead of by path and line number. That key survives a TU move or a
layout rename; a path does not. A plain merge of a regen that moves functions turns every
hand hunk inside a moved function into a modify/delete conflict, and `--theirs` then drops
it without a trace.

    hand-follow.py inventory [--base source/regen] [--head HEAD] [--tree DIR] -o OUT
        Every hunk of `git diff -U0 base head` over generated files, attributed to the
        function it sits in (plate comment + marker + body), or file-level, or
        multi-function. Writes OUT/hand-inventory.tsv and OUT/hand-inventory.json.

    hand-follow.py plan --inventory J [--clusters TSV] [--fn-map TSV] [--fn-map-tree DIR]
                        [--layout TSV] [--path-map TSV] -o OUT
        Where every hunk goes. Writes OUT/move-plan.tsv, OUT/review.tsv,
        OUT/git-mv.sh (pure renames, one commit, no content), OUT/lift.patch (the hand
        hunks of functions that change TU, taken out of their old file; applies after
        git-mv.sh and before the regen merge) and OUT/summary.txt.

    hand-follow.py reapply --inventory J --tree DIR [--plan OUT] [--apply] -o OUT
        After the merge: every function-level hunk that is no longer in its function is put
        back at the same spot of that function, wherever the function now lives. Writes
        OUT/drop.patch and OUT/reapply.tsv; --apply also writes the files.

    hand-follow.py audit --inventory J --tree DIR [--plan OUT] -o OUT
        Per hunk: present / missing / reverted / duplicated, checked inside the function it
        belongs to, by content. Exit 1 when anything is not present. Replaces counting
        hand-delta lines after a merge.

    hand-follow.py simulate --inventory J --plan OUT --worktree WT [maps] -o OUT
        Rehearsal on a scratch worktree: fakes the codegen side of the moves on a branch
        cut from the inventory base, runs git-mv.sh and lift.patch on a branch cut from the
        inventory head, merges, reapplies and audits; and the same merge without the tool
        as the control. Refuses a worktree on a source/* or master branch.

Deploy order with a real regen that carries moves:
    inventory, plan, git-mv.sh, git apply --index lift.patch + commit, merge the regen,
    reapply --apply + commit, audit.

Matching is on whitespace-stripped lines. Context is up to 3 unchanged base lines either
side, clipped to the function, so it travels with the function.
"""
import argparse
import collections
import difflib
import hashlib
import json
import os
import re
import subprocess
import sys

MARK = re.compile(r'^// 1\.14d: (?:0x)?([0-9a-fA-F]{6,8})\b')
MARKS = re.compile(r'^// 1\.14d: (?:0x)?([0-9a-fA-F]{6,8})\b', re.M)
# Hand-owned by rsync exclude, not generated; their delta is not a hand patch over code.
DEFAULT_EXCLUDES = ['asm_matching', 'support', 'thirdparty', 'cmake', 'tools',
                    'CMakeLists.txt', 'README.md', '.gitignore', '*.map']
SRC_EXT = ('.cpp', '.h', '.c', '.hpp', '.inc', '.CPP', '.H', '.C')
CTX = 3
TRIVIAL = {'', '{', '}', '};', 'return;', 'break;', 'continue;', '} else {', 'else', '#endif'}
COMPANIONS = ('.h', '.data.h', '.cpp.map')


# ---------------------------------------------------------------- git and files

def git(repo, *args, check=True):
    r = subprocess.run(['git', '-C', repo, *args], capture_output=True, text=True,
                       errors='surrogateescape')
    if check and r.returncode != 0:
        sys.exit(f'git {" ".join(args[:6])}: {r.stderr.strip()}')
    return r.stdout


def show(repo, rev, path):
    r = subprocess.run(['git', '-C', repo, 'show', f'{rev}:{path}'], capture_output=True,
                       errors='surrogateescape', text=True)
    return r.stdout if r.returncode == 0 else None


def read(path):
    try:
        with open(path, encoding='utf-8', errors='surrogateescape', newline='') as f:
            return f.read()
    except (FileNotFoundError, IsADirectoryError):
        return None


def write(path, text):
    os.makedirs(os.path.dirname(path) or '.', exist_ok=True)
    with open(path, 'w', encoding='utf-8', errors='surrogateescape', newline='') as f:
        f.write(text)


def lines_of(text):
    return text.splitlines(keepends=True) if text else []


def norm(lines):
    return [l.strip() for l in lines]


def h12(*parts):
    m = hashlib.sha1()
    for p in parts:
        m.update('\n'.join(norm(p)).encode('utf-8', 'surrogateescape'))
        m.update(b'\x1f')
    return m.hexdigest()[:12]


def weak(block):
    s = [l for l in norm(block) if l not in TRIVIAL]
    return sum(len(l) for l in s) < 8


# ---------------------------------------------------------------- the scanner

class Fn:
    __slots__ = ('addr', 'start', 'marker', 'body', 'end', 'ns', 'name')

    def __init__(self, addr, marker, body, end, ns, name):
        self.addr, self.start, self.marker, self.body = addr, marker, marker, body
        self.end, self.ns, self.name = end, ns, name

    def contains(self, li):
        return self.start <= li <= self.end


IDENT = re.compile(r'[A-Za-z_]\w*')


def _sig_name(pre):
    ids = IDENT.findall(pre.split('(')[0])
    return ids[-1] if ids else ''


def scan(lines):
    """Function spans of one file, 0-based inclusive line indices.

    A marker counts only at namespace level. The body is the first `{` after it at that
    level; a `;` first means a declaration, whose span ends there. Braces in strings,
    character literals, comments and preprocessor lines do not count; at #else/#elif the
    brace state of the #if is restored and the first branch's result kept at #endif.
    """
    events = []           # (line, kind, text before '{')
    in_block = False
    cont = False
    pending = []
    for li, line in enumerate(lines):
        if not in_block and (cont or line.lstrip().startswith('#')):
            if not cont:
                d = IDENT.match(line.lstrip()[1:].lstrip())
                d = d.group(0) if d else ''
                if d in ('if', 'ifdef', 'ifndef'):
                    events.append((li, '#if', None))
                elif d in ('else', 'elif'):
                    events.append((li, '#else', None))
                elif d == 'endif':
                    events.append((li, '#endif', None))
            cont = line.rstrip('\r\n').endswith('\\')
            continue
        i, n = 0, len(line)
        while i < n:
            c = line[i]
            if in_block:
                j = line.find('*/', i)
                if j < 0:
                    break
                in_block = False
                i = j + 2
                continue
            if c == '/' and line.startswith('//', i):
                break
            if c == '/' and line.startswith('/*', i):
                in_block = True
                i += 2
                continue
            if c == '"' and i > 0 and line[i - 1] == 'R' and '(' in line[i:]:
                p = line.index('(', i)
                delim = ')' + line[i + 1:p] + '"'
                j = line.find(delim, p)
                i = (j + len(delim)) if j >= 0 else n
                pending.append(' ')
                continue
            if c in '"\'':
                j = i + 1
                while j < n:
                    if line[j] == '\\':
                        j += 2
                        continue
                    if line[j] == c:
                        break
                    j += 1
                pending.append(' ')
                i = j + 1
                continue
            if c == '{':
                events.append((li, '{', ''.join(pending)[-400:]))
                pending = []
            elif c in '};':
                events.append((li, c, None))
                pending = []
            else:
                pending.append(c)
            i += 1
        pending.append('\n')

    markers = [(li, m.group(1).lower().zfill(8))
               for li, m in ((li, MARK.match(l)) for li, l in enumerate(lines)) if m]
    fns = []
    stack = []            # (kind, name)
    pp = []               # [stack at #if, stack after the first branch or None]
    mi = 0
    cur = None            # [addr, marker line, depth, body line, name, ns]
    for li, kind, pre in events:
        while mi < len(markers) and markers[mi][0] < li:
            mli, addr = markers[mi]
            mi += 1
            if cur is None and all(k == 'ns' for k, _ in stack):
                cur = [addr, mli, len(stack), None, '', '::'.join(n for _, n in stack if n)]
        if kind == '#if':
            pp.append([list(stack), None])
            continue
        if kind == '#else':
            if pp:
                if pp[-1][1] is None:
                    pp[-1][1] = list(stack)
                stack = list(pp[-1][0])
            continue
        if kind == '#endif':
            if pp:
                top = pp.pop()
                if top[1] is not None:
                    stack = top[1]
            continue
        if kind == '{':
            t = pre.strip()
            if re.match(r'(?:inline\s+)?namespace\b', t):
                stack.append(('ns', t.split('namespace', 1)[1].strip()))
            elif re.fullmatch(r'extern\s*', t):
                stack.append(('ns', ''))
            else:
                stack.append(('blk', ''))
                if cur is not None and cur[3] is None and len(stack) == cur[2] + 1:
                    cur[3] = li
                    cur[4] = _sig_name(pre)
        elif kind == '}':
            if stack:
                stack.pop()
            if cur is not None and cur[3] is not None and len(stack) == cur[2]:
                fns.append(Fn(cur[0], cur[1], cur[3], li, cur[5], cur[4]))
                cur = None
        elif kind == ';':
            if cur is not None and cur[3] is None and len(stack) == cur[2]:
                fns.append(Fn(cur[0], cur[1], None, li, cur[5], ''))
                cur = None
    # The plate comment is the `//` block directly above the marker.
    prev_end = -1
    for f in fns:
        s = f.marker
        while s - 1 > prev_end and lines[s - 1].lstrip().startswith('//') and \
                not MARK.match(lines[s - 1]):
            s -= 1
        f.start = s
        prev_end = f.end
    return fns


def fn_at(fns, li):
    for f in fns:
        if f.contains(li):
            return f
    return None


def addr_at(fns, li):
    f = fn_at(fns, li)
    return f.addr if f else None


def point(fns, i, j):
    """The function a gap between lines i and j sits in, or None between functions."""
    a, b = fn_at(fns, i), fn_at(fns, j)
    return a.addr if a is not None and a is b else None


def file_statics(lines, fns):
    """Names of namespace-level `static` objects and functions outside any marked span."""
    inside = set()
    for f in fns:
        inside.update(range(f.start, f.end + 1))
    out = set()
    for li, l in enumerate(lines):
        s = l.strip()
        if li in inside or not s.startswith('static '):
            continue
        cut = min([p for p in (s.find('='), s.find(';'), s.find('['), s.find('(')) if p >= 0]
                  or [len(s)])
        ids = IDENT.findall(s[:cut])
        if ids:
            out.add(ids[-1])
    return out


def tokens(lines):
    out = set()
    for l in lines:
        out.update(IDENT.findall(l.split('//', 1)[0]))
    return out


def rel_qualifiers(lines, tops):
    """`A::B` spellings whose first component is neither `::` nor a top-level namespace."""
    out = set()
    for l in lines:
        for m in re.finditer(r'(?<![\w:])([A-Za-z_]\w*)::', l.split('//', 1)[0]):
            if m.group(1) not in tops and m.group(1) != 'std':
                out.add(m.group(1))
    return out


def tracked_sources(root):
    if os.path.exists(os.path.join(root, '.git')):
        out = git(root, 'ls-files', '-z', '--', *[f'*{e}' for e in SRC_EXT])
        return [p for p in out.split('\0') if p]
    res = []
    for dp, dn, fnames in os.walk(root):
        dn[:] = [d for d in dn if d != '.git']
        res += [os.path.relpath(os.path.join(dp, f), root) for f in fnames if f.endswith(SRC_EXT)]
    return res


def index_tree(root, want=None):
    """rel path -> [Fn] for source files under root; with `want`, only files naming one."""
    out = {}
    for rel in tracked_sources(root):
        t = read(os.path.join(root, rel))
        if t is None or '// 1.14d: ' not in t:
            continue
        if want is not None and not want & {m.lower().zfill(8) for m in MARKS.findall(t)}:
            continue
        out[rel] = scan(lines_of(t))
    return out


# ---------------------------------------------------------------- inventory

HUNK = re.compile(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@')


def parse_diff(text):
    files = collections.OrderedDict()
    cur = None
    for line in text.splitlines(keepends=True):
        if line.startswith('diff --git '):
            cur = None
        elif cur is None and line.startswith('+++ '):
            p = line[4:].rstrip('\n')
            cur = p[2:] if p.startswith('b/') else None
            if cur is not None:
                files[cur] = []
        elif line.startswith('@@') and cur is not None:
            m = HUNK.match(line)
            a, b, c, d = m.group(1), m.group(2), m.group(3), m.group(4)
            files[cur].append({'old_start': int(a), 'old_len': 1 if b is None else int(b),
                               'new_start': int(c), 'new_len': 1 if d is None else int(d),
                               'removed': [], 'added': []})
        elif cur is not None and files[cur]:
            if line.startswith('-'):
                files[cur][-1]['removed'].append(line[1:])
            elif line.startswith('+'):
                files[cur][-1]['added'].append(line[1:])
    return files


def hunk_kind(h):
    body = [l for l in norm(h['removed'] + h['added']) if l]
    if body and all(l.startswith('#include') for l in body):
        return 'include'
    if body and all(l.startswith('#') for l in body):
        return 'preprocessor'
    return 'code'


def cmd_inventory(a):
    repo = os.path.abspath(a.tree)
    base = git(repo, 'rev-parse', a.base).strip()
    head = git(repo, 'rev-parse', a.head).strip()
    spec = ['--', '.'] + [f':!{x}' for x in DEFAULT_EXCLUDES + (a.exclude or [])]
    added_files = git(repo, 'diff', '--name-only', '--diff-filter=A', base, head, *spec).split()
    diff = git(repo, 'diff', '-U0', '--no-color', '--no-ext-diff', '--no-renames',
               '--diff-algorithm=histogram', '--diff-filter=M', base, head, *spec)
    files = parse_diff(diff)
    hunks = []
    tops = set()
    for path, hs in files.items():
        bl, hl = lines_of(show(repo, base, path)), lines_of(show(repo, head, path))
        bf, hf = scan(bl), scan(hl)
        # Base lines another hunk replaces, and base lines another hunk inserts in front
        # of: context may not cross either, or it would not be contiguous in the result.
        changed_old, gaps = set(), set()
        for h in hs:
            changed_old.update(range(h['old_start'] - 1, h['old_start'] - 1 + h['old_len']))
            if not h['old_len']:
                gaps.add(h['old_start'])
        for f in hf:
            if f.ns:
                tops.add(f.ns.split('::')[0])
        for h in hs:
            ol, nl = h['old_len'], h['new_len']
            # -U0: a zero-length side is a gap after line N (1-based) of that side
            first = h['old_start'] - 1 if ol else h['old_start']
            if ol:
                fo = {addr_at(bf, i) for i in range(first, first + ol)}
            else:
                fo = {point(bf, first - 1, first)}
            n0 = h['new_start'] - 1 if nl else h['new_start']
            if nl:
                fw = {addr_at(hf, i) for i in range(n0, n0 + nl)}
            else:
                fw = {point(hf, n0 - 1, n0)}
            addrs = fo | fw
            named = sorted(x for x in addrs if x)
            if len(addrs) == 1 and named:
                scope = 'function'
            elif not named:
                scope = 'file'
            else:
                scope = 'multi'
            rec = {'file': path, **h, 'scope': scope, 'kind': hunk_kind(h), 'addrs': named,
                   'addr': None, 'fn': '', 'ns': '', 'off_old': None, 'off_new': None}
            anchor = None
            if scope == 'function':
                addr = named[0]
                anchor = next((f for f in bf if f.addr == addr), None)
                nf = next((f for f in hf if f.addr == addr), None)
                src = nf or anchor
                rec.update(addr=addr, fn=src.name if src else '', ns=src.ns if src else '')
                if anchor is None:
                    rec['scope'] = 'hand-function'
                else:
                    rec['off_old'] = first - anchor.start
                if nf is not None:
                    rec['off_new'] = n0 - nf.start
            # Context: unchanged base lines next to the hunk, clipped to the function.
            lo, hi = (anchor.start, anchor.end) if anchor else (0, len(bl) - 1)
            cb, i = [], first - 1
            while i >= lo and len(cb) < CTX and i not in changed_old and \
                    (i + 1 == first or i + 1 not in gaps):
                cb.insert(0, bl[i])
                i -= 1
            ca, i = [], first + ol
            while i <= hi and len(ca) < CTX and i not in changed_old and \
                    (i == first + ol or i not in gaps):
                ca.append(bl[i])
                i += 1
            rec['ctx_before'], rec['ctx_after'] = cb, ca
            rec['hash'] = h12(rec['removed'], rec['added'])
            rec['weak'] = weak(rec['added']) and weak(rec['removed'])
            hunks.append(rec)
    for i, r in enumerate(hunks):
        r['id'] = f'h{i + 1:03d}'
    inv = {'base': base, 'head': head, 'base_ref': a.base, 'head_ref': a.head, 'tree': repo,
           'top_namespaces': sorted(tops), 'hand_files': added_files, 'hunks': hunks}
    os.makedirs(a.out, exist_ok=True)
    with open(os.path.join(a.out, 'hand-inventory.json'), 'w') as f:
        json.dump(inv, f, indent=1)
    cols = ['id', 'hash', 'file', 'scope', 'kind', 'addr', 'fn', 'ns', 'old_start', 'old_len',
            'new_start', 'new_len', 'off_in_fn', 'n_added', 'n_removed', 'weak', 'first_line']
    with open(os.path.join(a.out, 'hand-inventory.tsv'), 'w') as f:
        f.write('\t'.join(cols) + '\n')
        for r in hunks:
            first = next((l.strip() for l in r['added'] + r['removed'] if l.strip()), '')
            f.write('\t'.join(str(x) for x in [
                r['id'], r['hash'], r['file'], r['scope'], r['kind'],
                r['addr'] or ','.join(r['addrs']), r['fn'], r['ns'], r['old_start'],
                r['old_len'], r['new_start'], r['new_len'],
                '' if r['off_new'] is None else r['off_new'], len(r['added']),
                len(r['removed']), int(r['weak']), first[:100].replace('\t', ' ')]) + '\n')
        for p in added_files:
            f.write('\t'.join(['-', '-', p, 'hand-file', '-'] + [''] * 10 +
                              ['0', 'whole file absent from base']) + '\n')
    c = collections.Counter(r['scope'] for r in hunks)
    print(f'{len(hunks)} hunks in {len(files)} files (+{len(added_files)} hand-owned files): '
          + ', '.join(f'{k} {v}' for k, v in sorted(c.items())))
    print(f'base {base[:10]} ({a.base})  head {head[:10]} ({a.head})')


# ---------------------------------------------------------------- maps

def read_tsv(path):
    with open(path, encoding='utf-8') as f:
        rows = [l.rstrip('\n').split('\t') for l in f if l.strip()]
    return [dict(zip(rows[0], r)) for r in rows[1:]]


def load_fn_map(a, inv):
    """addr -> (new file, where the claim came from, the file the claim says it leaves)"""
    m = {}
    if a.clusters:
        for r in read_tsv(a.clusters):
            for item in (r.get('functions') or '').split(';'):
                if ':' in item:
                    addr = item.split(':', 1)[0].lower().zfill(8)
                    m[addr] = (r['to_tu'], r.get('cluster', ''), r.get('from_tu', ''))
    if a.fn_map:
        with open(a.fn_map) as f:
            for l in f:
                p = l.rstrip('\n').split('\t')
                if len(p) >= 2 and re.fullmatch(r'(0x)?[0-9a-fA-F]{6,8}', p[0]):
                    m[p[0].lower().replace('0x', '').zfill(8)] = (p[1], 'fn-map', '')
    if getattr(a, 'fn_map_tree', None):
        # every function of a file with hunks, so file-level hunks see what leaves
        want = set()
        for p in {r['file'] for r in inv['hunks']}:
            want.update(f.addr for f in scan(lines_of(show(inv['tree'], inv['head'], p))))
        for rel, fns in index_tree(a.fn_map_tree, want).items():
            for f in fns:
                m[f.addr] = (rel, 'tree', '')
    return m


def load_path_map(a, tracked):
    """(renames old -> new, merged files old -> the file that absorbs them)."""
    pairs = []
    if a.layout:
        for r in read_tsv(a.layout):
            new = (r.get('proposed_path') or '').split()
            new = new[0] if new else ''     # the column may carry a trailing [note]
            if new and new != r['current_path']:
                pairs.append((r['current_path'], new))
    if a.path_map:
        with open(a.path_map) as f:
            for l in f:
                p = l.rstrip('\n').split('\t')
                if len(p) >= 2 and p[0] and p[1] and p[0] != 'old':
                    pairs.append((p[0], p[1]))
    by_new = collections.defaultdict(list)
    for o, n in pairs:
        by_new[n].append(o)
    mv, merge = {}, {}
    for n, olds in by_new.items():
        if len(olds) == 1:
            mv[olds[0]] = n
            continue
        # Several files become one: the one with the target's name is renamed, the
        # others' functions move into it.
        same = [o for o in olds if os.path.basename(o).lower() == os.path.basename(n).lower()]
        keep = same[0] if len(same) == 1 else sorted(olds)[0]
        mv[keep] = n
        for o in olds:
            if o != keep:
                merge[o] = n
    for o, n in list(mv.items()):
        if o.endswith('.cpp') and n.lower().endswith('.cpp'):
            ob, nb = o[:-4], n[:-4]
            for ext in COMPANIONS:
                if ob + ext in tracked and ob + ext not in mv:
                    mv[ob + ext] = nb + ext
    return mv, merge


class Canon:
    """Spell a path the way the tree does. Plans written on a case-insensitive file system
    name `D2Game/Player/PlayerMsg.cpp` for the tracked `D2Game/PLAYER/PlayerMsg.cpp`; git
    treats those as two files, so every path from a map goes through here first."""

    def __init__(self, tracked):
        self.files = {p.lower(): p for p in tracked}
        self.dirs = {}
        for p in tracked:
            parts = p.split('/')
            for i in range(1, len(parts)):
                d = '/'.join(parts[:i])
                self.dirs.setdefault(d.lower(), d)
        self.fixed = {}

    def __call__(self, p):
        if not p:
            return p
        q = self.files.get(p.lower())
        if q is None:
            d, b = os.path.split(p)
            q = (self.dirs.get(d.lower(), d) + '/' + b) if d else b
        if q != p:
            self.fixed[p] = q
        return q


def fold_case(tracked, mv, merge):
    """One spelling per directory across the tree after the moves.

    Two spellings of a directory are one directory on macOS and Windows, so `git mv` into
    the second fails or lands in the first. The spelling the tree already has wins, else
    the one most targets use. Returns the respell map; the caller reports it."""
    final = [mv.get(p, p) for p in tracked if p not in merge] + list(merge.values())
    count = collections.Counter()
    for p in final:
        parts = p.split('/')
        for i in range(1, len(parts)):
            count['/'.join(parts[:i])] += 1
    have = {}
    for p in tracked:
        parts = p.split('/')
        for i in range(1, len(parts)):
            have.setdefault('/'.join(parts[:i]).lower(), '/'.join(parts[:i]))
    for p in final:
        count[p] += 1
    groups = collections.defaultdict(list)
    for d in count:
        groups[d.lower()].append(d)
    pick = {}
    for k, v in groups.items():
        if len(v) > 1:
            win = have.get(k) if have.get(k) in v else max(v, key=lambda d: (count[d], d))
            for d in v:
                if d != win:
                    pick[d] = win
    if not pick:
        return {}

    def fix(p):
        if p in pick:
            return pick[p]
        parts = p.split('/')
        for i in range(len(parts) - 1, 0, -1):
            d = '/'.join(parts[:i])
            if d in pick:
                return fix(pick[d] + '/' + '/'.join(parts[i:]))
        return p
    return {p: fix(p) for p in set(mv.values()) | set(merge.values()) if fix(p) != p}


def canon_maps(c, fnmap, mv, merge, tracked=None):
    fnmap = {k: (c(t), s, c(f)) for k, (t, s, f) in fnmap.items()}
    mv = {c(o): c(n) for o, n in mv.items()}
    merge = {c(o): c(n) for o, n in merge.items()}
    folded = fold_case(tracked, mv, merge) if tracked is not None else {}
    if folded:
        mv = {o: folded.get(n, n) for o, n in mv.items()}
        merge = {o: folded.get(n, n) for o, n in merge.items()}
        fnmap = {k: (folded.get(t, t), s, f) for k, (t, s, f) in fnmap.items()}
        # two files folded onto one name: the second is merged into the first
        seen = {}
        for o in sorted(mv):
            n = mv[o]
            if n in seen:
                merge[o] = n
                del mv[o]
            else:
                seen[n] = o
    return fnmap, mv, merge, folded


# ---------------------------------------------------------------- plan

def cmd_plan(a):
    inv = json.load(open(a.inventory))
    repo, head = inv['tree'], inv['head']
    tracked = set(git(repo, 'ls-tree', '-r', '--name-only', head).splitlines())
    fnmap = load_fn_map(a, inv)
    mv, merge = load_path_map(a, tracked)
    canon = Canon(tracked)
    fnmap, mv, merge, folded = canon_maps(canon, fnmap, mv, merge, tracked)
    tops = set(inv['top_namespaces'])
    hunks = inv['hunks']
    byid = {r['id']: r for r in hunks}
    cache = {}

    def hfile(p):
        if ('h', p) not in cache:
            cache[('h', p)] = lines_of(show(repo, head, p))
        return cache[('h', p)]

    def bfile(p):
        if ('b', p) not in cache:
            ls = lines_of(show(repo, inv['base'], p))
            cache[('b', p)] = ls
            cache[('bs', p)] = scan(ls)
        return cache[('b', p)]

    def bscan(p):
        bfile(p)
        return cache[('bs', p)]

    head_fns = {p: scan(hfile(p)) for p in {r['file'] for r in hunks}}
    for o, n in merge.items():
        if o in tracked:
            for f in scan(hfile(o)):
                fnmap.setdefault(f.addr, (n, 'layout-merge', o))

    def pmap(p):
        return mv.get(p, merge.get(p, p))

    def ren(p):     # where the file itself goes; a merged file is not renamed
        return mv.get(p, p)

    moved_from = collections.defaultdict(set)     # file -> addrs that leave it
    for p, fns in head_fns.items():
        for f in fns:
            t = fnmap.get(f.addr)
            if t and pmap(t[0]) != ren(p):
                moved_from[p].add(f.addr)

    # Identifiers a file-level hunk adds that the base file never had.
    introduced = {r['id']: tokens(r['added']) - tokens(bfile(r['file']))
                  for r in hunks if r['scope'] in ('file', 'multi', 'hand-function')}
    statics = {}
    inv_mv = {n: o for o, n in mv.items()}

    plan, review = [], []
    lift = collections.defaultdict(list)
    for r in hunks:
        p = r['file']
        np_ = ren(p)
        row = {'id': r['id'], 'hash': r['hash'], 'file': p, 'scope': r['scope'],
               'kind': r['kind'], 'addr': r['addr'] or ','.join(r['addrs']), 'fn': r['fn'],
               'action': 'stay', 'new_file': np_, 'notes': []}
        if r['scope'] == 'function':
            t = fnmap.get(r['addr'])
            target = pmap(t[0]) if t else np_
            row['new_file'] = target
            if t and t[2] and t[2] != p and t[1] != 'tree':
                row['notes'].append(f'map says it leaves {t[2]}, head has it in {p}')
            if target != np_:
                row['action'] = 'move-fn'
                lift[p].append(r)
                if p not in statics:
                    statics[p] = file_statics(bfile(p), bscan(p))
                bfn = next((f for f in bscan(p) if f.addr == r['addr']), None)
                body = tokens(bfile(p)[bfn.start:bfn.end + 1]) if bfn else set()
                new = tokens(r['added'])
                dep = sorted((new & statics[p]) - body)
                if dep:
                    row['notes'].append('uses file statics of the old TU the generated body '
                                        'does not: ' + ','.join(dep))
                for oid, names in introduced.items():
                    if byid[oid]['file'] == p and new & names:
                        row['notes'].append(f'uses {",".join(sorted(new & names)[:6])} '
                                            f'introduced by file-level {oid}')
                # Includes the old TU's hand code added that the target does not have.
                cur = inv_mv.get(target, target)
                have = set(norm(hfile(cur))) if cur in tracked else set()
                for x in hunks:
                    if x['file'] == p and x['scope'] == 'file' and x['kind'] == 'include':
                        need = [l.strip() for l in x['added'] if l.strip() and l.strip() not in have]
                        if need:
                            row['notes'].append(f'target lacks {"; ".join(need)} (file-level '
                                                f'{x["id"]} of the old TU)')
                q = sorted(rel_qualifiers(r['added'], tops))
                if q:
                    row['notes'].append('namespace-relative qualifiers, re-check in the new '
                                        'namespace: ' + ','.join(q))
            elif np_ != p:
                row['action'] = 'follow-file'
        else:
            if p in merge:
                row['new_file'] = merge[p]
                row['action'] = 'review'
                row['notes'].append(f'file is merged into {merge[p]}; its file-level hand '
                                    'code has no home')
            elif p in moved_from:
                row['action'] = 'review'
                row['notes'].append(f'{len(moved_from[p])} function(s) leave this TU')
                users = [x['id'] for x in hunks if x['file'] == p and x['scope'] == 'function'
                         and x['addr'] in moved_from[p]
                         and tokens(x['added']) & introduced.get(r['id'], set())]
                if users:
                    row['notes'].append('needed by moving hunk(s) ' + ','.join(users))
            elif np_ != p:
                row['action'] = 'follow-file'
            if r['scope'] == 'multi' and (p in merge or any(
                    fnmap.get(x) and pmap(fnmap[x][0]) != np_ for x in r['addrs'])):
                row['action'] = 'review'
                row['notes'].append('hunk spans functions ' + ','.join(r['addrs']) +
                                    ' and one of them changes TU')
        for l in r['added']:
            m = re.match(r'\s*#\s*include\s+"([^"]+)"', l)
            if m and m.group(1) in mv:
                row['notes'].append(f'#include "{m.group(1)}" moves to {mv[m.group(1)]}')
                if row['action'] in ('stay', 'follow-file'):
                    row['action'] = 'review'
        if row['action'] == 'review' or (row['action'] == 'move-fn' and row['notes']):
            review.append(row)
        plan.append(row)

    os.makedirs(a.out, exist_ok=True)
    cols = ['id', 'hash', 'file', 'scope', 'kind', 'addr', 'fn', 'action', 'new_file', 'notes']
    for name, rows in (('move-plan.tsv', plan), ('review.tsv', review)):
        with open(os.path.join(a.out, name), 'w') as f:
            f.write('\t'.join(cols) + '\n')
            for r in rows:
                f.write('\t'.join('; '.join(r['notes']) if c == 'notes' else str(r[c])
                                  for c in cols) + '\n')

    conflicts = []
    if folded:
        dirs = sorted({(os.path.dirname(k), os.path.dirname(v)) for k, v in folded.items()
                       if os.path.dirname(k) != os.path.dirname(v)})
        conflicts.append(f'case collision: {len(folded)} target(s) respelled so each directory '
                         'has one spelling (fix the map): ' +
                         ', '.join(f'{a} -> {b}' for a, b in dirs[:8]))
    if canon.fixed:
        conflicts.append(f'{len(canon.fixed)} map path(s) differ from the tree only in case, '
                         'respelled: ' + ', '.join(f'{k} -> {v}' for k, v in
                                                   sorted(canon.fixed.items())[:8]))
    for o in sorted(mv):
        if o not in tracked:
            conflicts.append(f'path map names {o}, which is not tracked at head')
    # Renames ordered so that every target is free when it is used.
    todo = {o: n for o, n in mv.items() if o in tracked}
    exists = set(tracked)
    order = []
    while todo:
        ready = sorted(o for o, n in todo.items() if n not in exists)
        if not ready:
            conflicts.append('occupied targets or a rename cycle: ' +
                             ', '.join(f'{o}->{n}' for o, n in sorted(todo.items())[:5]))
            break
        for o in ready:
            n = todo.pop(o)
            order.append((o, n))
            exists.discard(o)
            exists.add(n)
    final = {pmap(p) for p in tracked if p not in merge}
    spell = collections.defaultdict(set)
    for p in final:
        parts = p.split('/')
        for i in range(1, len(parts)):
            spell['/'.join(parts[:i]).lower()].add('/'.join(parts[:i]))
        spell[p.lower()].add(p)
    for k, v in sorted(spell.items()):
        if len(v) > 1:
            conflicts.append('case collision, one path on macOS and Windows: ' +
                             ' / '.join(sorted(v)))
    owned = [p for p in tracked if p == 'CMakeLists.txt' or p.startswith(('support/', 'cmake/'))
             or p in inv['hand_files']]
    moving = set(mv) | set(merge)
    for p in sorted(owned):
        if not p.endswith(SRC_EXT + ('.txt', '.cmake')):
            continue
        t = show(repo, head, p) or ''
        c = sum(1 for o in moving if re.search(r'(?<![\w./-])' + re.escape(o) + r'(?![\w.-])', t))
        if c:
            conflicts.append(f'hand-owned {p} names {c} moving path(s)')

    with open(os.path.join(a.out, 'git-mv.sh'), 'w') as f:
        f.write('#!/bin/sh\n# Pure renames, no content change. Run on the deploy tree before '
                'merging the regen.\nset -eu\nT="${1:?usage: git-mv.sh <tree>}"\n')
        for d in sorted({os.path.dirname(n) for _, n in order if os.path.dirname(n)}):
            f.write(f'mkdir -p "$T/{d}"\n')
        for o, n in order:
            f.write(f'git -C "$T" mv -- "{o}" "{n}"\n')
        if order:
            f.write('git -C "$T" commit -q -m "chore(layout): move files to the new layout"\n')
    os.chmod(os.path.join(a.out, 'git-mv.sh'), 0o755)

    # The lift patch takes the hand hunks of TU-changing functions out of their old file,
    # on the path the file has after git-mv.sh.
    out = []
    for p, rs in sorted(lift.items()):
        old = hfile(p)
        new = list(old)
        for r in sorted(rs, key=lambda r: -r['new_start']):
            n0 = r['new_start'] - 1 if r['new_len'] else r['new_start']
            new[n0:n0 + r['new_len']] = r['removed']
        out.append(udiff(old, new, ren(p)))
    write(os.path.join(a.out, 'lift.patch'), ''.join(out))

    c = collections.Counter(r['action'] for r in plan)
    with open(os.path.join(a.out, 'summary.txt'), 'w') as f:
        f.write(f'inventory {inv["head"][:10]} over {inv["base"][:10]}: {len(hunks)} hunks\n')
        f.write(f'maps: {len(fnmap)} function placements, {len(mv)} renames '
                f'({len(order)} tracked), {len(merge)} merged files\n')
        f.write('actions: ' + ', '.join(f'{k} {v}' for k, v in sorted(c.items())) + '\n')
        f.write(f'git-mv.sh: {len(order)} renames\n')
        f.write(f'lift.patch: {sum(len(v) for v in lift.values())} hunks in {len(lift)} files\n')
        f.write(f'review.tsv: {len(review)} rows\n')
        f.write(f'conflicts: {len(conflicts)}\n')
        for x in conflicts:
            f.write(f'  {x}\n')
    print(open(os.path.join(a.out, 'summary.txt')).read(), end='')


def udiff(old, new, path):
    if old == new:
        return ''
    fix = lambda ls: [l if l.endswith('\n') else l + '\n' for l in ls]
    body = ''.join(difflib.unified_diff(fix(old), fix(new), f'a/{path}', f'b/{path}', n=3))
    return f'diff --git a/{path} b/{path}\n' + body


# ---------------------------------------------------------------- locate, audit, reapply

def find_all(hay, needle):
    if not needle:
        return []
    n, first = len(needle), needle[0]
    return [i for i in range(len(hay) - n + 1) if hay[i] == first and hay[i:i + n] == needle]


class Tree:
    def __init__(self, root):
        self.root = root
        self.text = {}
        self.byaddr = None
        self.files = {}

    def lines(self, rel):
        if rel not in self.text:
            self.text[rel] = lines_of(read(os.path.join(self.root, rel)) or '')
        return self.text[rel]

    def index(self, want):
        if self.byaddr is None:
            self.byaddr = collections.defaultdict(list)
            for rel, fns in index_tree(self.root, want).items():
                self.files[rel] = fns
                for f in fns:
                    self.byaddr[f.addr].append(rel)
        return self.byaddr

    def put(self, rel, ls):
        self.text[rel] = ls
        self.files[rel] = scan(ls)
        for k in list(self.byaddr):
            self.byaddr[k] = [r for r in self.byaddr[k] if r != rel]
        for f in self.files[rel]:
            self.byaddr[f.addr].append(rel)

    def fn(self, rel, addr):
        fns = self.files.get(rel)
        if fns is None:
            fns = self.files[rel] = scan(self.lines(rel))
        return next((f for f in fns if f.addr == addr), None)


def status_in(hay, r):
    """(status, hit offsets, 'ctx' or 'bare') of one hunk inside the normalised lines hay."""
    cb, ca = norm(r['ctx_before']), norm(r['ctx_after'])
    add, rem = norm(r['added']), norm(r['removed'])
    new_hits = find_all(hay, cb + add + ca)
    old_hits = find_all(hay, cb + rem + ca)
    if not add:                                     # a pure deletion
        if rem and old_hits:
            return 'reverted', old_hits, 'ctx'
        if new_hits:
            return 'present', new_hits, 'ctx'
        if rem and not weak(rem):
            h = find_all(hay, rem)
            return ('reverted', h, 'bare') if h else ('present', [], 'bare')
        return 'missing', [], ''
    if new_hits:
        return ('duplicated' if len(new_hits) > 1 else 'present'), new_hits, 'ctx'
    if old_hits:
        return 'reverted', old_hits, 'ctx'
    if not weak(add):
        h = find_all(hay, add)
        if h:
            return ('duplicated' if len(h) > 1 else 'present'), h, 'bare'
    if rem and not weak(rem):
        h = find_all(hay, rem)
        if h:
            return 'reverted', h, 'bare'
    return 'missing', [], ''


def expected_files(a):
    if not a.plan:
        return {}
    return {row['id']: row['new_file'] for row in read_tsv(os.path.join(a.plan, 'move-plan.tsv'))}


def check(inv, tree, exp):
    byaddr = tree.index({r['addr'] for r in inv['hunks'] if r.get('addr')})
    res = []
    for r in inv['hunks']:
        e = exp.get(r['id'], r['file'])
        out = {'id': r['id'], 'hash': r['hash'], 'scope': r['scope'], 'addr': r['addr'] or '',
               'fn': r['fn'], 'expected': e, 'found_in': '', 'status': '', 'how': '', 'note': ''}
        notes = []
        if r['scope'] in ('function', 'hand-function') and r['addr']:
            where = sorted(set(byaddr.get(r['addr'], [])))
            if not where:
                out['status'] = 'fn-missing'
                res.append((r, out, None))
                continue
            rel = e if e in where else where[0]
            if len(where) > 1:
                notes.append('function defined in ' + ','.join(where))
            f = tree.fn(rel, r['addr'])
            st, hits, how = status_in(norm(tree.lines(rel)[f.start:f.end + 1]), r)
            if len(where) > 1 and st == 'present':
                st = 'duplicated'
            if rel != e:
                notes.append(f'expected in {e}')
            # a move that left the hand text behind in the old file
            if r['file'] not in where and not weak(r['added']):
                ol = norm(tree.lines(r['file']))
                if find_all(ol, norm(r['ctx_before'] + r['added'] + r['ctx_after'])):
                    st = 'duplicated'
                    notes.append(f'copy left in {r["file"]}')
            out.update(found_in=rel, status=st, how=how)
        else:
            rel = e if os.path.exists(os.path.join(tree.root, e)) else r['file']
            hay = norm(tree.lines(rel))
            st, hits, how = status_in(hay, r)
            out.update(found_in=rel if hay else '', status=st if hay else 'file-missing', how=how)
        out['note'] = '; '.join(notes)
        res.append((r, out, out['found_in'] or None))
    return res


def write_report(path, rows):
    cols = ['id', 'hash', 'scope', 'addr', 'fn', 'status', 'how', 'expected', 'found_in', 'note']
    with open(path, 'w') as f:
        f.write('\t'.join(cols) + '\n')
        for o in rows:
            f.write('\t'.join(str(o[c]) for c in cols) + '\n')


def cmd_audit(a):
    inv = json.load(open(a.inventory))
    rows = [o for _, o, _ in check(inv, Tree(a.tree), expected_files(a))]
    os.makedirs(a.out, exist_ok=True)
    write_report(os.path.join(a.out, a.name), rows)
    c = collections.Counter(o['status'] for o in rows)
    bad = [o for o in rows if o['status'] != 'present']
    print(f'audit {a.tree}: {len(rows)} hunks: ' + ', '.join(f'{k} {v}' for k, v in sorted(c.items())))
    for o in bad[:40]:
        print(f'  {o["id"]} {o["status"]:<11} {o["scope"]:<8} {o["addr"]:<9} {o["fn"][:40]:<40} '
              f'{o["found_in"] or o["expected"]} {o["note"]}')
    return 1 if bad else 0


def cmd_reapply(a):
    inv = json.load(open(a.inventory))
    tree = Tree(a.tree)
    res = check(inv, tree, expected_files(a))
    orig = {}
    done, skipped = [], []
    for r, o, rel in res:
        if o['status'] == 'present':
            continue
        if r['scope'] != 'function' or o['status'] not in ('reverted', 'missing'):
            skipped.append((o, o['status'] if r['scope'] == 'function'
                            else f'{r["scope"]}-level, {o["status"]}'))
            continue
        ls = tree.lines(rel)
        f = tree.fn(rel, r['addr'])
        hay = norm(ls[f.start:f.end + 1])
        st, hits, how = status_in(hay, r)
        cb, rem = norm(r['ctx_before']), norm(r['removed'])
        if st == 'reverted' and len(hits) == 1:
            k = hits[0] + (len(cb) if how == 'ctx' else 0)
        elif st == 'missing' and not rem and r['off_old'] is not None and cb and \
                0 < r['off_old'] <= len(hay) and hay[r['off_old'] - 1] == cb[-1]:
            k = r['off_old']    # an insertion whose far context drifted; the near line holds
        else:
            skipped.append((o, f'{st}, {len(hits)} candidate(s), no unique anchor'))
            continue
        orig.setdefault(rel, list(ls))
        s = f.start + k
        tree.put(rel, ls[:s] + r['added'] + ls[s + len(rem):])
        done.append((o, rel))
    patch = ''.join(udiff(orig[p], tree.text[p], p) for p in sorted(orig))
    os.makedirs(a.out, exist_ok=True)
    write(os.path.join(a.out, 'drop.patch'), patch)
    with open(os.path.join(a.out, 'reapply.tsv'), 'w') as f:
        f.write('id\taddr\tfn\tresult\tfile\n')
        for o, rel in done:
            f.write(f'{o["id"]}\t{o["addr"]}\t{o["fn"]}\treapplied\t{rel}\n')
        for o, why in skipped:
            f.write(f'{o["id"]}\t{o["addr"]}\t{o["fn"]}\tleft for review: {why}\t'
                    f'{o["found_in"]}\n')
    if a.apply:
        for p in orig:
            write(os.path.join(a.tree, p), ''.join(tree.text[p]))
    print(f'reapply: {len(done)} hunk(s) put back in {len(orig)} file(s), {len(skipped)} left '
          f'for review{" (written)" if a.apply else " (drop.patch only)"}')
    for o, why in skipped:
        print(f'  {o["id"]} {o["scope"]:<8} {o["addr"]:<9} {o["fn"][:40]:<40} {why}')
    return 0


# ---------------------------------------------------------------- simulate

def cmd_simulate(a):
    inv = json.load(open(a.inventory))
    wt = os.path.abspath(a.worktree)
    br = git(wt, 'rev-parse', '--abbrev-ref', 'HEAD').strip()
    if br.startswith('source/') or br in ('master', 'main'):
        sys.exit(f'refusing: {wt} is on {br}; simulate needs a scratch worktree')
    if git(wt, 'status', '--porcelain', '--untracked-files=no').strip():
        sys.exit(f'refusing: {wt} has uncommitted changes')
    tag, out = a.tag, os.path.abspath(a.out)
    os.makedirs(out, exist_ok=True)
    log = open(os.path.join(out, 'simulate.log'), 'w')

    def say(s):
        print(s)
        log.write(s + '\n')
        log.flush()

    # Every function the maps place moves, not only hand-touched ones: the merge has to
    # see the whole TU change the codegen would make.
    tracked = set(git(wt, 'ls-tree', '-r', '--name-only', inv['base']).splitlines())
    fnmap = load_fn_map(a, inv)
    mv, merge = load_path_map(a, tracked)
    fnmap, mv, merge, _ = canon_maps(Canon(tracked), fnmap, mv, merge, tracked)

    git(wt, 'checkout', '-q', '-B', f'scratch/hf-{tag}-regen', inv['base'])
    touched = set()
    for o, n in merge.items():
        if o in tracked:
            for f in scan(lines_of(read(os.path.join(wt, o)))):
                fnmap.setdefault(f.addr, (n, 'layout-merge', o))
    # Renames first, through temporary names so a chain cannot clobber itself.
    live = [(o, n) for o, n in sorted(mv.items()) if os.path.exists(os.path.join(wt, o))]
    for i, (o, _) in enumerate(live):
        os.rename(os.path.join(wt, o), os.path.join(wt, f'.hf-tmp-{i}'))
    for i, (o, n) in enumerate(live):
        os.makedirs(os.path.dirname(os.path.join(wt, n)), exist_ok=True)
        os.rename(os.path.join(wt, f'.hf-tmp-{i}'), os.path.join(wt, n))
        touched.update((o, n))
    if mv:
        say(f'[regen] renamed {len(live)} file(s) the way rsync does: delete + add')
    paths = sorted(touched)
    for i in range(0, len(paths), 200):
        git(wt, 'add', '--', *paths[i:i + 200])
    if fnmap:
        tree = Tree(wt)
        byaddr = tree.index(set(fnmap))
        moved, written = 0, set()
        for addr, (target, _, _) in sorted(fnmap.items()):
            where = byaddr.get(addr, [])
            if len(where) != 1 or where[0] == target:
                continue
            src = where[0]
            ls = tree.lines(src)
            f = tree.fn(src, addr)
            if f is None:
                continue
            body = ls[f.start:f.end + 1]
            tree.put(src, ls[:f.start] + ls[f.end + 1:])
            tree.put(target, list(tree.lines(target)) + ['\n'] + body)
            written.update((src, target))
            moved += 1
        for p in written:
            write(os.path.join(wt, p), ''.join(tree.text[p]))
        say(f'[regen] moved {moved} function(s) across {len(written)} file(s)')
        paths = sorted(written)
        for i in range(0, len(paths), 200):
            git(wt, 'add', '--', *paths[i:i + 200])
    git(wt, 'commit', '-q', '-m', f'test(hand-follow): simulated codegen for {tag}')
    regen = git(wt, 'rev-parse', 'HEAD').strip()

    def merge_into(label, prep):
        git(wt, 'checkout', '-q', '-B', f'scratch/hf-{tag}-{label}', inv['head'])
        prep()
        r = subprocess.run(['git', '-C', wt, 'merge', '-q', '--no-edit', regen],
                           capture_output=True, text=True)
        un = git(wt, 'diff', '--name-only', '--diff-filter=U').split()
        say(f'[{label}] merge exit {r.returncode}, {len(un)} conflicted file(s)')
        for p in un[:30]:
            say(f'    conflict: {p}')
        if un:
            # the resolution that has already lost hand fixes: take the generator's side
            for p in un:
                if subprocess.run(['git', '-C', wt, 'checkout', '--theirs', '--', p],
                                  capture_output=True).returncode == 0:
                    git(wt, 'add', '--', p)
                else:
                    git(wt, 'rm', '-q', '--', p, check=False)
            git(wt, 'commit', '-q', '--no-edit')
            say(f'[{label}] conflicts resolved with --theirs')
        return un

    def prep_tool():
        sh = os.path.join(a.plan, 'git-mv.sh')
        if mv:
            r = subprocess.run(['sh', sh, wt], capture_output=True, text=True)
            say(f'[tool] git-mv.sh exit {r.returncode} {r.stderr.strip()[:300]}')
        lp = os.path.join(a.plan, 'lift.patch')
        if os.path.getsize(lp):
            r = subprocess.run(['git', '-C', wt, 'apply', '--index', lp], capture_output=True,
                               text=True)
            say(f'[tool] lift.patch exit {r.returncode} {r.stderr.strip()[:300]}')
            git(wt, 'commit', '-q', '-m', f'test(hand-follow): lift hand hunks for {tag}')

    results = {}
    for label, prep in (('control', lambda: None), ('tool', prep_tool)):
        un = merge_into(label, prep)
        if label == 'tool':
            cmd_reapply(argparse.Namespace(inventory=a.inventory, tree=wt, plan=a.plan,
                                           apply=True, out=os.path.join(out, 'reapply')))
            changed = git(wt, 'diff', '--name-only').split()
            if changed:
                git(wt, 'add', '--', *changed)
                git(wt, 'commit', '-q', '-m', f'test(hand-follow): reapply hand hunks for {tag}')
        cmd_audit(argparse.Namespace(inventory=a.inventory, tree=wt, plan=a.plan, out=out,
                                     name=f'audit-{label}.tsv'))
        rows = read_tsv(os.path.join(out, f'audit-{label}.tsv'))
        results[label] = (len(un), collections.Counter(r['status'] for r in rows))
    for k, (u, c) in results.items():
        say(f'RESULT {tag} {k}: {u} conflicted file(s); ' +
            ', '.join(f'{s} {n}' for s, n in sorted(c.items())))
    git(wt, 'checkout', '-q', '--detach', inv['head'])
    return 0


# ---------------------------------------------------------------- main

def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    sub = ap.add_subparsers(dest='cmd', required=True)
    p = sub.add_parser('inventory')
    here = os.path.dirname(os.path.abspath(__file__))
    tree = os.path.join(here, '..', 'modified')      # recon/runs -> recon/modified
    p.add_argument('--tree', default=tree if os.path.isdir(tree) else os.path.join(here, '..'))
    p.add_argument('--base', default='source/regen')
    p.add_argument('--head', default='HEAD')
    p.add_argument('--exclude', action='append', help='extra pathspec to leave out')
    p.add_argument('-o', '--out', required=True)
    for name in ('plan', 'simulate'):
        p = sub.add_parser(name)
        p.add_argument('--inventory', required=True)
        p.add_argument('--clusters', help='TU cluster plan (tumoves clusters.tsv)')
        p.add_argument('--fn-map', help='TSV: addr <tab> new file')
        p.add_argument('--layout', help='layout current-vs-original.tsv (proposed_path)')
        p.add_argument('--path-map', help='TSV: old path <tab> new path')
        p.add_argument('-o', '--out', required=True)
        if name == 'plan':
            p.add_argument('--fn-map-tree', help='a generated tree to read placements from')
        else:
            p.add_argument('--plan', required=True)
            p.add_argument('--worktree', required=True)
            p.add_argument('--tag', default='sim')
    p = sub.add_parser('reapply')
    p.add_argument('--inventory', required=True)
    p.add_argument('--tree', required=True)
    p.add_argument('--plan')
    p.add_argument('--apply', action='store_true')
    p.add_argument('-o', '--out', required=True)
    p = sub.add_parser('audit')
    p.add_argument('--inventory', required=True)
    p.add_argument('--tree', required=True)
    p.add_argument('--plan')
    p.add_argument('--name', default='audit.tsv')
    p.add_argument('-o', '--out', required=True)
    a = ap.parse_args()
    return {'inventory': cmd_inventory, 'plan': cmd_plan, 'reapply': cmd_reapply,
            'audit': cmd_audit, 'simulate': cmd_simulate}[a.cmd](a) or 0


if __name__ == '__main__':
    sys.exit(main())
