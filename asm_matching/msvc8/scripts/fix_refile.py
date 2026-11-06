#!/usr/bin/env python3
"""fix_refile.py - file a function with its callers where the original compiled them together.

    fix_refile.py [--apply] [--max-span N] [PATTERN ...]

VC8 passes arguments in registers of its own choosing only to a static
function it compiles together with every call to it (objects.py). Where the
original passes such arguments to a function the tree files in another TU
than its callers, the generator split one of Blizzard's objects - it files by
Ghidra namespace - and neither the function nor its callers can match:
Bnclient::CACHE_ComputeCRC32 (00518de0) takes its arguments in EAX and ECX
from Bnclient/Cache, and lies right ahead of Cache's functions in the
original, in the object CACHE.CPP made.

A function the original passes registers beyond ECX and EDX that its
declared convention does not pass (kind `object` in evidence/objects.tsv)
shares a TU with every caller; these relations make groups. A group the
tree spreads over several TUs goes to one TU: the one owning the original
file the annotations in and around its range name (an annotation names the
object around its function), else the one holding most of it - on a tie
the callers', and not a catch-all such as Bnclient/Bnclient. Name prefixes
are no evidence, they are the database's. It goes only if one object can
hold it: every function that moves, and every caller the map lacks, lies at
most --max-span functions of the original from a function of the group in
that TU, with at most MAX_TUS other TUs between (callers far away mean a /GL
object - the Rich header counts 12 - or a misreading: refiling cannot fix
those). What the original gives a convention of its own becomes static
there if the tree names it nowhere else - not in its old .cpp after the
move, not in another file but its header (a function of that name of
another file's own does not count).

The edit moves each definition, with the comment block above it, into
X.cpp at its place among the functions there in the original's address
order, inside a block of its own namespace: the namespace is the function's
name in the database, so every reference to it stays as it is and only the
file changes. What becomes static loses its header declaration (fix_static's
rule), and a static declaration goes ahead of its first use if the
definition comes after it; what stays extern keeps its declaration. X.cpp
gets the macros of the old files the moved code uses, and the old files'
includes it lacks - those declaring a name the code uses that X.cpp does
not see yet, or all of them if the mingw compiler needs more, less each
the compiler then does without. A static
function of an old file that only moving code calls moves along. The seams
are closed - a namespace block reopened right after it closed is one block,
an empty one goes - and every file keeps its ending. The parts of a group
that do not depend on each other - a function made static goes with the
callers that move to it - are separate edits.

Each edit is compiled alone first, every .cpp it touches, by the mingw
compiler the game builds with (a syntax check): what the game's build would
reject is never tried. Then all edits are made at once and the tree rebuilt
(most headers are in the precompiled header), and every TU an edit touched
is judged with each moved function in its new TU. An edit is dropped when a
function it affects stops matching or vanishes - or another function of a
TU it touched does and no edit's own functions explain it - when a TU it
touched fails to compile, when the functions it affects come out less
similar in sum and none of them comes to match (the register evidence is
for the group, not for each function), when a moved function refers to
anything else than before (a name its new file binds differently), or when
funcmap.py, run over the rebuilt tree, no longer pairs a function of a TU
it touched that it paired before (the map pairs by neighbours in a TU: a
move into Fog/Debug once left ILT_strrchr unpaired). The rest
is tried again, a few rounds at most; a TU no edit touched that fails to
compile drops the edits making static a function it names. A loss in a TU
no edit touched comes from a header's declaration count, the symbol phase:
it is listed for the bench test (evidence/flags.md), not laid on an edit.
Kept moves are written into funcmap.tsv and the audit rows; a second run
retries what the first dropped.
"""
import argparse
import collections
import json
import os
import re
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import audit            # noqa: E402
import check            # noqa: E402
import classify         # noqa: E402
import fixlib           # noqa: E402
import ours as ourslib  # noqa: E402
from fix_ifelse import match_close  # noqa: E402
from fix_static import COMMENTS, Tree, def_pattern  # noqa: E402
from objects import SourceFiles, convention_args  # noqa: E402
from oracle_config import BUILD, ORACLE, REPO  # noqa: E402

MAX_SPAN = 100          # original functions between a function that moves and its new file's
MAX_TUS = 12            # other TUs of the tree's between them
ROUNDS = 5
NS_LINE = re.compile(r'^namespace\s+([A-Za-z_][\w:]*)\s*\{[ \t]*$|^\}\s*//\s*namespace\b[^\n]*$', re.M)
INCLUDE = re.compile(r'^#include\s+[<"][^>"\n]+[>"][^\n]*$', re.M)
MACRO = re.compile(r'^#ifndef\s+(\w+)[ \t]*\n#define\s+\1\b(?:[^\n]*\\\n)*[^\n]*\n#endif[^\n]*$'
                   r'|^#define\s+(\w+)\b(?:[^\n]*\\\n)*[^\n]*$', re.M)
IDENT = re.compile(r'[A-Za-z_]\w*')
FUNCMAP = os.path.join(ORACLE, 'evidence', 'funcmap.tsv')


def blank_comments(text):
    """The text with its comments blanked, every offset where it was."""
    return COMMENTS.sub(lambda m: re.sub(r'[^\n]', ' ', m.group(0)), text)


def definition(text, leaf):
    """(start, end, head) of the one definition of `leaf`: start at the
    comment block above it, end past its closing brace's line. None unless
    there is exactly one."""
    ms = list(re.finditer(def_pattern(leaf), text, re.M))
    if len(ms) != 1:
        return None
    m = ms[0]
    close = match_close(text, m.end() - 1, '{', '}')
    if close is None:
        return None
    end = text.find('\n', close)
    end = len(text) if end < 0 else end + 1
    start = m.start()
    while start > 0:
        prev = text.rfind('\n', 0, start - 1) + 1
        if not text.startswith('//', prev):
            break
        start = prev
    return start, end, m.group('head').rstrip()


HEAD = re.compile(r'^[A-Za-z_][^;{}()\n]*?\b(\w+)\s*\([^;{}]*\)[ \t]*\{', re.M)


def definitions(text):
    """leaf -> [(start, end)] of every function defined at the start of a
    line, as definition() delimits them."""
    out = collections.defaultdict(list)
    for m in HEAD.finditer(text):
        close = match_close(text, m.end() - 1, '{', '}')
        if close is None:
            continue
        end = text.find('\n', close)
        end = len(text) if end < 0 else end + 1
        start = m.start()
        while start > 0:
            prev = text.rfind('\n', 0, start - 1) + 1
            if not text.startswith('//', prev):
                break
            start = prev
        out[m.group(1)].append((start, end))
    return out


def namespace_at(text, pos):
    """The namespace open at `pos` ('' for the global one), and where the
    line opening its block starts."""
    stack = []
    for m in NS_LINE.finditer(text, 0, pos):
        if m.group(1):
            stack.append((m.group(1), m.start()))
        elif stack:
            stack.pop()
    return stack[-1] if stack else ('', None)


def wrap(block, ns, ctx):
    """`block` (definitions, a blank line between them) for a place where
    namespace `ctx` is open, inside namespace `ns`."""
    if ns == ctx:
        return block
    out = ''
    if ctx:
        out += '} // namespace %s\n\n' % ctx
    if ns:
        out += 'namespace %s {\n\n' % ns
    out += block
    if ns:
        out += '\n} // namespace %s\n' % ns
    if ctx:
        out += '\nnamespace %s {\n' % ctx
    return out


DECLARES = [
    re.compile(r'^[ \t]*(?:typedef[ \t]+)?(?:struct|union|class|enum)[ \t]+(\w+)[ \t]*(?::[^{;\n]*)?\{', re.M),
    re.compile(r'^[ \t]*typedef\b[^;]*?\b(\w+)[ \t]*(?:\[[^\]]*\])?[ \t]*;', re.M),
    re.compile(r'^[ \t]*typedef\b[^;]*?\([ \t]*(?:__\w+[ \t]+)?\*[ \t]*(\w+)[ \t]*\)', re.M),
    re.compile(r'\b(\w+)[ \t]*\([^;{}()]*(?:\([^;{}()]*\)[^;{}()]*)*\)[ \t]*;'),
    re.compile(r'^#define[ \t]+(\w+)', re.M),
    re.compile(r'^[ \t]*extern\b[^;(]*?\b(\w+)[ \t]*(?:\[[^\]]*\])*[ \t]*;', re.M),
    re.compile(r'^[ \t]*(\w+)[ \t]*(?:=[^,\n]*)?,?[ \t]*$', re.M),
]


_DECLARED = {}


def declared_names(path):
    """The names a header declares: types it defines, typedefs, functions,
    extern data, macros, enumerators."""
    if path not in _DECLARED:
        text = blank_comments(fixlib.read(path))
        _DECLARED[path] = {n for rx in DECLARES for n in rx.findall(text)}
    return _DECLARED[path]


def visible_names(text):
    """The names the headers `text` includes declare, as far as they include
    each other, with the header every TU is compiled with."""
    seen, todo = set(), [os.path.join(REPO, 'Source', 'D2Headers', 'd2_platform.h')]
    todo += [os.path.join(REPO, n) for n in re.findall(r'^#include\s+"([^"]+)"', text, re.M)]
    names = set()
    while todo:
        p = todo.pop()
        if p in seen or not os.path.exists(p):
            continue
        seen.add(p)
        names |= declared_names(p)
        todo += [os.path.join(REPO, n) for n in re.findall(r'^#include\s+"([^"]+)"', fixlib.read(p), re.M)]
    return names


class Move:
    """One group of functions that must share a TU, filed in TU `x`: the
    movers [(original start, leaf, namespace, TU now, static after)], and
    the functions already in X that become static [(original start, leaf)]."""

    def __init__(self, x, movers, promote, addr):
        self.x = x
        self.movers = sorted(movers)
        self.promote = sorted(promote)
        self.addr = addr                        # TU -> leaf -> original start
        self.addr_x = addr[x]
        self.xcpp = os.path.join(REPO, x + '.cpp')
        self.xh = os.path.join(REPO, x + '.h')
        self.ys = sorted({m[3] for m in self.movers})
        self.includes = 'used'                  # or 'all': which of the old files' includes X gets
        self.unneeded = set()                   # of those, the include lines the compiler does without
        self.helpers = []                       # [(original start, leaf, TU)]: static helpers moving along

    @property
    def owners(self):
        return {m[0] for m in self.movers} | {p[0] for p in self.promote} | {h[0] for h in self.helpers}

    @property
    def label(self):
        parts = []
        for y in self.ys:
            parts.append('%s: %s' % (y, ', '.join([m[1] + ('' if m[4] else ' (extern)')
                                                for m in self.movers if m[3] == y] +
                                               ['%s (helper)' % h[1] for h in self.helpers if h[2] == y])))
        if self.promote:
            parts.append('static here: %s' % ', '.join(p[1] for p in self.promote))
        return '%s <- %s' % (self.x, '; '.join(parts))

    def paths(self):
        out = [self.xcpp, self.xh]
        for y in self.ys:
            out += [os.path.join(REPO, y + '.cpp'), os.path.join(REPO, y + '.h')]
        return out

    def apply(self, texts, placed):
        """The texts with the functions moved, or None if it cannot be done.
        `placed`: leaf -> original start of every function a move files in
        X.cpp, so that moves into one file keep the address order among them.
        A static function of an old file that only moving code calls moves
        along, still static (a helper; self.helpers lists the mapped ones)."""
        self.why = None
        out = self._apply(texts, placed)
        return out

    def no(self, why):
        self.why = why
        return None

    def _apply(self, texts, placed):
        out = {}
        xsrc = texts[self.xcpp]
        xh = texts.get(self.xh)
        statics = {m[1] for m in self.movers if m[4]} | {p[1] for p in self.promote}
        moved = []                               # (start, leaf, namespace, kind, text, head)
        self.helpers = []
        for y in self.ys:
            ycpp, yh = os.path.join(REPO, y + '.cpp'), os.path.join(REPO, y + '.h')
            ysrc = texts[ycpp]
            spans = {}                           # leaf -> (start, end, original start, namespace, kind, head)
            for s, leaf, ns, _y, st in (m for m in self.movers if m[3] == y):
                d = definition(ysrc, leaf)
                if d is None:
                    return self.no('%s: no one definition in %s.cpp' % (leaf, y))
                if definition(xsrc, leaf) is not None:
                    return self.no('%s: %s.cpp defines that name too' % (leaf, self.x))
                if d[2].startswith(('extern ', 'inline ')) or ' inline ' in d[2]:
                    return self.no('%s: extern or inline' % leaf)
                if d[2].startswith('static '):
                    spans[leaf] = (d[0], d[1], s, ns, 'helper', d[2])     # static already
                    statics.add(leaf)
                else:
                    spans[leaf] = (d[0], d[1], s, ns, 'static' if st else 'extern', d[2])
            index = definitions(ysrc)
            changed = True
            while changed:
                changed = False
                used = set(IDENT.findall(blank_comments(''.join(ysrc[v[0]:v[1]] for v in spans.values()))))
                for leaf in sorted(used - set(spans)):
                    if len(index.get(leaf, ())) != 1:
                        continue
                    d = definition(ysrc, leaf)
                    if d is None or not d[2].startswith('static '):
                        continue
                    stay = ysrc
                    for v in sorted(list(spans.values()) + [d], key=lambda v: -v[0]):
                        stay = stay[:v[0]] + stay[v[1]:]
                    stay = forward_declaration(leaf).sub('', stay)
                    if re.search(r'\b%s\b' % re.escape(leaf), blank_comments(stay)):
                        continue                 # code that stays calls it too
                    if definition(xsrc, leaf) is not None:
                        return self.no('helper %s: %s.cpp defines that name too' % (leaf, self.x))
                    spans[leaf] = (d[0], d[1], self.addr.get(y, {}).get(leaf), namespace_at(ysrc, d[0])[0],
                                   'helper', d[2])
                    changed = True
            # A helper the map lacks goes right ahead of the first function calling it.
            movers = {m[1] for m in self.movers}
            for leaf, v in list(spans.items()):
                if leaf in movers:
                    continue
                if v[2] is None:
                    users = [u[2] for u in spans.values() if u[2] is not None and
                             re.search(r'\b%s\b' % re.escape(leaf), blank_comments(ysrc[u[0]:u[1]]))]
                    if not users:
                        return self.no('helper %s: no place for it' % leaf)
                    spans[leaf] = v[:2] + (min(users) - 0.5,) + v[3:]
                elif v[4] == 'helper':
                    self.helpers.append((v[2], leaf, y))
            for leaf, (a, b, s, ns, kind, head) in sorted(spans.items(), key=lambda kv: -kv[1][0]):
                moved.append((s, leaf, ns, kind, ysrc[a:b], head))
                ysrc = ysrc[:a] + ysrc[b + 1 if ysrc.startswith('\n', b) else b:]
            for leaf, v in spans.items():
                if v[4] == 'helper':
                    ysrc = forward_declaration(leaf).sub('', ysrc)
            out[ycpp] = ysrc
            if yh in texts:
                h = drop_declarations(texts[yh], [(leaf, texts[ycpp][v[0]:v[1]]) for leaf, v in spans.items()
                                                  if v[4] == 'static'])
                if h is None:
                    return self.no('%s.h declares a function twice' % y)
                out[yh] = h
        # A static function must be named nowhere else now: not in what stays.
        for y in self.ys:
            rest = blank_comments(out[os.path.join(REPO, y + '.cpp')])
            named = [leaf for leaf in statics if re.search(r'\b%s\b' % re.escape(leaf), rest)]
            if named:
                return self.no('%s.cpp still names %s' % (y, ', '.join(named)))
        # Static in place, the functions already in X.
        for _s, leaf in self.promote:
            d = definition(xsrc, leaf)
            if d is None or d[2].startswith('static '):
                return self.no('%s: no one extern definition in %s.cpp' % (leaf, self.x))
            k = xsrc.index(d[2], d[0])
            if xh is not None:
                xh = drop_declarations(xh, [(leaf, xsrc[d[0]:d[1]])])
                if xh is None:
                    return self.no('%s.h declares a function twice' % self.x)
            xsrc = xsrc[:k] + 'static ' + xsrc[k:]
        if xh is not None and self.xh in texts and xh != texts[self.xh]:
            out[self.xh] = xh
        # Into X.cpp, one at a time, in address order.
        here = dict(placed)
        for s, leaf, ns, kind, src, head in sorted(moved):
            if kind == 'static' and re.search(r'^[A-Za-z_][^\n]*\b%s\s*\([^;{}]*\)\s*;' % re.escape(leaf), xsrc, re.M):
                return self.no('%s.cpp declares %s extern itself' % (self.x, leaf))
            k = src.index(head)
            block = src[:k] + ('static ' if kind == 'static' else '') + src[k:]
            xsrc = self.insert(xsrc, s, ns, block, here)
            if xsrc is None:
                return self.no('no function of known address in %s.cpp' % self.x)
            here[leaf] = s
        # A static function used ahead of its definition needs a declaration
        # ahead of the namespace block holding the first use.
        heads = {leaf: (ns, head[len('static '):] if kind == 'helper' else head)
                 for _s, leaf, ns, kind, _t, head in moved if kind != 'extern'}
        for _s, leaf in self.promote:
            d = definition(xsrc, leaf)
            if d is None:
                return self.no('%s: two definitions in %s.cpp' % (leaf, self.x))
            heads[leaf] = (namespace_at(xsrc, d[0])[0], d[2][len('static '):])
        for leaf, (ns, head) in sorted(heads.items()):
            d = definition(xsrc, leaf)
            if d is None:
                return self.no('%s: two definitions in %s.cpp' % (leaf, self.x))
            at = d[0]
            use = re.search(r'\b%s\b' % re.escape(leaf), blank_comments(xsrc[:at]))
            if use:
                _ctx, opened = namespace_at(xsrc, use.start())
                if opened is None:
                    return self.no('%s: used at global scope ahead of it' % leaf)
                xsrc = xsrc[:opened] + wrap('static %s;\n' % head, ns, '') + '\n' + xsrc[opened:]
        xsrc = self.add_preamble(texts, xsrc, moved)
        if xsrc is None:
            return self.no('%s.cpp has no include' % self.x)
        out[self.xcpp] = xsrc
        return {p: same_ending(tidy_namespaces(t) if p.endswith('.cpp') else t, texts[p]) for p, t in out.items()}

    def insert(self, xsrc, s, ns, block, placed):
        """`xsrc` with `block` at the place of original start `s` among the
        functions defined there whose original start is known."""
        index = definitions(xsrc)
        known = []
        for leaf, addr in list(self.addr_x.items()) + list(placed.items()):
            d = index.get(leaf, ())
            if len(d) == 1:
                known.append((addr, d[0][0], d[0][1]))
        if not known:
            return None
        before = [k for k in known if k[0] < s]
        if before:
            _addr, _a, b = max(before)
            ctx, _o = namespace_at(xsrc, b)
            # Past the end of the block the predecessor ends, while it is not
            # the function's own namespace: no empty block left behind.
            while ctx and ctx != ns:
                m = re.compile(r'(?:[ \t]*\n)*\}\s*//\s*namespace\b[^\n]*\n').match(xsrc, b)
                if not m:
                    break
                b = m.end()
                ctx, _o = namespace_at(xsrc, b)
            return xsrc[:b] + '\n' + wrap(block, ns, ctx) + xsrc[b:]
        _addr, a, _b = min(known)
        ctx, opened = namespace_at(xsrc, a)
        if ctx == ns:
            return xsrc[:a] + block + '\n' + xsrc[a:]
        pos = opened if opened is not None else a
        ctx, _o = namespace_at(xsrc, pos)
        return xsrc[:pos] + wrap(block, ns, ctx) + '\n' + xsrc[pos:]

    def add_preamble(self, texts, xsrc, moved):
        """The old files' macros the moved code uses and X lacks, and their
        includes X lacks (those declaring a name the code uses and X does
        not see yet, or all of them), after X's last include."""
        used = set(IDENT.findall(blank_comments(''.join(m[4] for m in moved))))
        have = set(re.findall(r'^#define\s+(\w+)', xsrc, re.M))
        olds = [texts[os.path.join(REPO, y + '.cpp')] for y in self.ys]
        add = []
        changed = True
        while changed:                           # macros using macros
            changed = False
            for ysrc in olds:
                for m in MACRO.finditer(ysrc):
                    name = m.group(1) or m.group(2)
                    if name in used and name not in have:
                        add.append(m.group(0))
                        have.add(name)
                        used |= set(IDENT.findall(m.group(0)))
                        changed = True
        xinc = set(INCLUDE.findall(xsrc))
        missing = []
        for ysrc in olds:
            missing += [l for l in INCLUDE.findall(ysrc) if l not in xinc and l not in missing]
        if self.includes == 'used':
            need = used - visible_names(xsrc)
            keep = []
            for line in missing:
                p = os.path.join(REPO, line.split('"')[1]) if '"' in line else None
                if p and os.path.exists(p) and declared_names(p) & need:
                    keep.append(line)
                    need -= declared_names(p)
            missing = keep
        missing = [l for l in missing if l not in self.unneeded]
        last = list(INCLUDE.finditer(xsrc))
        if not last:
            return None
        at = last[-1].end()
        text = ''.join('\n' + l for l in missing)
        if add:
            text += '\n\n' + '\n\n'.join(add)
        return xsrc[:at] + text + xsrc[at:]


def tidy_namespaces(text):
    """`text` without the seams moves leave: a namespace block closed and
    the same one opened again right after is one block, an empty block is
    none. Neither creates a symbol, so the code stays as it is."""
    while True:
        new = re.sub(r'\n\} // namespace ([\w:]+)\n(?:[ \t]*\n)*namespace \1 \{\n(?:[ \t]*\n)*', '\n', text)
        new = re.sub(r'^namespace ([\w:]+) \{\n(?:[ \t]*\n)*\} // namespace \1(?:\n(?:[ \t]*\n)*|\Z)', '',
                     new, flags=re.M)
        if new == text:
            return text
        text = new


def same_ending(text, old):
    """`text` ending as `old` did: the generator ends a file without a newline."""
    return text.rstrip('\n') + old[len(old.rstrip('\n')):]


def forward_declaration(leaf):
    """The line declaring static function `leaf` ahead of its definition."""
    return re.compile(r'^static [^\n;{}]*\b%s\s*\([^;{}]*\)\s*;[ \t]*\n' % re.escape(leaf), re.M)


def drop_declarations(h, leaves):
    """Header text `h` without the declarations of [(leaf, its definition's
    text)], each with the comment block above it if the definition repeats
    it (annotations such as // @sourcefile must never be lost); None when a
    function is declared twice."""
    for leaf, text in leaves:
        decl = re.compile(r'(?P<comments>(?:^//[^\n]*\n)*)^[^\n]*\b%s\s*\([^;{}]*\);[ \t]*\n' % re.escape(leaf), re.M)
        hm = list(decl.finditer(h))
        if len(hm) > 1:
            return None
        if hm:
            lo, hi = hm[0].start(), hm[0].end()
            block = hm[0].group('comments')
            if block and block not in text:
                lo += len(block)
            h = h[:lo] + h[hi:]
    return h


def trim_includes(m, new, pristine, placed, mingw):
    """Of the includes move `m` gives X.cpp, drop each the mingw compiler
    does without (`new` is written; the files are left as m makes them)."""
    if mingw.ok(m.xcpp) is not True:
        return
    before = set(INCLUDE.findall(pristine[m.xcpp]))
    for line in reversed([l for l in INCLUDE.findall(new[m.xcpp]) if l not in before]):
        m.unneeded.add(line)
        trial = m.apply(dict(pristine), placed)
        if trial is None:
            m.unneeded.discard(line)
            continue
        fixlib.write(m.xcpp, trial[m.xcpp])
        if not mingw.ok(m.xcpp):
            m.unneeded.discard(line)
    fixlib.write(m.xcpp, m.apply(dict(pristine), placed)[m.xcpp])


def build_tree():
    """Rebuild whatever the edits made stale: (TUs compiled, TUs failed)."""
    p = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'build.py'), '-j4'],
                       capture_output=True, text=True)
    if 'compiled ' not in p.stdout:
        raise SystemExit('build.py did not run: %s' % (p.stderr.strip()[-500:] or p.stdout.strip()[-500:]))
    built, failed = set(), set()
    for line in p.stdout.splitlines():
        m = re.match(r'^\[\d+/\d+\] (ok|FAIL)\s+[\d.]+s (\S+)\.cpp', line)
        if m:
            built.add(m.group(2))
            if m.group(1) == 'FAIL':
                failed.add(m.group(2))
    return built, failed


def fail(why):
    """Stop, the reason as the last line of stdout (what fix_all reports)."""
    print('fix_refile cannot run: ' + ' '.join(why.split()), flush=True)
    raise SystemExit(2)


class Mingw:
    """Syntax checks with the game's own compiler, its command lines from a
    configure-only CMake run in build/mingw. Without them no move is tried:
    the check is what keeps a move the game's build rejects out of the tree,
    so its absence stops the fixer, saying why, rather than passing all."""

    def __init__(self):
        self.cmds = {}
        db = os.path.join(BUILD, 'mingw', 'compile_commands.json')
        if not os.path.exists(db):
            try:
                p = subprocess.run(['cmake', '-S', REPO, '-B', os.path.dirname(db), '-G', 'Ninja',
                                    '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                                    '-DCMAKE_TOOLCHAIN_FILE=' + os.path.join(REPO, 'cmake', 'mingw-i686.cmake')],
                                   capture_output=True, text=True)
                err = (p.stderr.strip() or p.stdout.strip())[-300:]
            except FileNotFoundError:
                err = 'cmake is not installed'
            if not os.path.exists(db):
                fail('no mingw syntax check (cmake, ninja and i686-w64-mingw32-g++ are needed): ' + err)
        for e in json.load(open(db)):
            self.cmds[os.path.realpath(e['file'])] = (e['directory'], e.get('arguments') or shlex.split(e['command']))
        if not self.cmds:
            fail('no mingw syntax check: %s lists no command' % db)

    def ok(self, path):
        """None when there is no command for `path`, else whether it compiles."""
        c = self.cmds.get(os.path.realpath(path))
        if c is None:
            return None
        cwd, args = c
        out, skip = [], False
        for a in args:
            if skip:
                skip = False
                continue
            if a == '-o':
                skip = True
                continue
            if a != '-c':
                out.append(a)
        out.insert(-1, '-fsyntax-only')
        return subprocess.run(out, cwd=cwd, capture_output=True).returncode == 0


def own_name(path, leaf, ns):
    """True if every `leaf` in the file is a function of its own by that
    name - it declares one, and never qualifies the name with a component of
    namespace `ns` - rather than a use of the one in `ns`."""
    text = blank_comments(fixlib.read(path))
    if not re.search(r'^[A-Za-z_][^\n;{}]*\b%s\s*\(' % re.escape(leaf), text, re.M):
        return False
    parts = [p for p in ns.split('::') if p]
    return not any(re.search(r'\b%s\s*::\s*%s\b' % (re.escape(p), re.escape(leaf)), text) for p in parts)


class Annotations:
    """The original file a function's __FILE__ strings name - the
    generator's @sourcefile annotation, where it wrote one - and the TU whose
    functions name a file most."""

    def __init__(self, rows, fns):
        sources = SourceFiles()
        self.of = {}
        per_tu = collections.defaultdict(collections.Counter)
        for s, _e, _name, tu, sym, _m, _d in rows:
            f = fns.get(tu + '|' + sym)
            if f is None or f.source_line is None:
                continue
            a = sources.at(tu, f.source_line).lower()
            if a:
                self.of[s] = a
                per_tu[tu][a] += 1
        # A file belongs to the TU whose functions name it most; among
        # several, to the one named like it (GfxInfo's name .\ENGINE\Gfx.cpp).
        claims = collections.defaultdict(list)
        for tu, c in per_tu.items():
            claims[c.most_common(1)[0][0]].append(tu)
        self.tu_of_file = {}
        for a, tus in claims.items():
            base = re.split(r'[\\/]', a)[-1].rsplit('.', 1)[0]
            named = [t for t in tus if t.rsplit('/', 1)[-1].lower() == base]
            self.tu_of_file[a] = (named or sorted(tus, key=lambda t: -per_tu[t][a]))[0]


def catch_all(tu):
    """A TU of the functions Ghidra left in a module's or a directory's own
    namespace (D2Client/D2Client, D2Common/Drlg/Drlg, Storm/_unnamespaced)."""
    parts = tu.split('/')
    return parts[-1] == '_unnamespaced' or (len(parts) > 1 and parts[-1] == parts[-2])


REGION = 20            # original functions around a group whose annotations speak for its object


def destination(members, tu_of, count, bound, files, order, idx):
    """The TU a group goes to. An annotation names the original file of the
    object around its function: the TU owning the file the annotations in
    and around the group's range name most. Else the one holding most of it;
    on a tie, the callers' and not a catch-all. (Name prefixes are the
    database's, not the files': DRLGPATH_SetPathPosition lies among
    D2Common/PATH/Path's functions.)"""
    lo = max(0, min(idx[m] for m in members) - REGION)
    hi = max(idx[m] for m in members) + REGION
    votes = collections.Counter()
    for t in order[lo:hi + 1]:
        owner = files.tu_of_file.get(files.of.get(t, ''))
        if owner in count:
            votes[owner] += 1
    if votes:
        return max(votes, key=lambda t: (votes[t], count[t], t))
    return max(count, key=lambda t: (count[t], -sum(1 for m in members if m in bound and tu_of[m] == t),
                                     not catch_all(t), t))


def candidates(rows, model, fns, tree, pats, max_span):
    """([Move], {original start: why its group is no candidate}, callers).

    A function the original passes registers beyond ECX and EDX that its
    declared convention does not pass (kind `object` in objects.tsv) shares
    a TU with every caller; those relations make groups, and a group the
    tree spreads over several TUs is filed together, in the TU holding most
    of it (on a tie, the one holding the callers)."""
    by_start = {r[0]: r for r in rows}
    callers = collections.defaultdict(set)
    for s, (_dec, refs) in model.items():
        for _a, _o, _n, k, t in refs:
            if k == 'call':
                callers[t].add(s)
    order = sorted(model)
    idx = {s: i for i, s in enumerate(order)}
    bound = set()
    for s, _e, _name, tu, sym, _m, _d in rows:
        f = fns.get(tu + '|' + sym)
        args_in = convention_args(sym)
        if f is None or '@@Y' not in sym or args_in is None:
            continue
        if (classify.live_in(model[s][0]) - args_in) - {'ecx', 'edx'}:
            bound.add(s)
    parent = {}

    def find(x):
        parent.setdefault(x, x)
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    for g in bound:
        find(g)
        for c in callers.get(g, ()):
            if c in by_start:
                parent[find(c)] = find(g)
    groups = collections.defaultdict(list)
    for x in list(parent):
        groups[find(x)].append(x)
    addr = collections.defaultdict(dict)
    for s, _e, name, tu, _sym, _m, _d in rows:
        addr[tu][name.rsplit('::', 1)[-1]] = s
    files = Annotations(rows, fns)
    moves, why = [], {}
    for members in groups.values():
        members.sort()
        tu_of = {m: by_start[m][3] for m in members}
        count = collections.Counter(tu_of.values())
        if len(count) < 2:
            continue                             # one TU: static is enough (fix_static)
        if pats and not any(p in t for p in pats for t in count):
            continue
        x = destination(members, tu_of, count, bound, files, order, idx)
        movers = [m for m in members if tu_of[m] != x]
        # One object holds them: whatever moves, and every caller the tree
        # lacks a map for, lies near the functions of X's among them.
        home = sorted(idx[m] for m in members if tu_of[m] == x)
        far = None
        for p in movers + [c for g in members if g in bound for c in callers[g] if c not in by_start]:
            k = idx[p]
            near = min(home, key=lambda h: abs(h - k))
            lo, hi = sorted((near, k))
            between = {by_start[t][3] for t in order[lo + 1:hi] if t in by_start} - {x, tu_of.get(p)}
            if hi - lo > max_span or len(between) > MAX_TUS:
                far = (p, hi - lo, len(between))
                break
        if far:
            for m in members:
                why[m] = '%08x lies %d functions from %s, %d other TUs between: not one object' % (
                    far[0], far[1], x, far[2])
            continue
        bad = [m for m in movers if '@@Y' not in by_start[m][4]]
        if bad:
            for m in members:
                why[m] = 'its group would move %s, not a free function' % by_start[bad[0]][2]
            continue
        stale = [m for m in members if tu_of[m] + '|' + by_start[m][4] not in fns]
        if stale:
            for m in members:
                why[m] = 'the build lacks %s under its mapped symbol: oracle map' % by_start[stale[0]][2]
            continue
        sources = {os.path.join(REPO, tu_of[m] + '.cpp') for m in movers}
        spec, promote, extern = [], [], []
        for m in members:
            name, tu = by_start[m][2], tu_of[m]
            leaf = name.rsplit('::', 1)[-1]
            ns = name.rsplit('::', 1)[0] if '::' in name else ''
            st = fns[tu + '|' + by_start[m][4]].static     # moves on as it is
            if m in bound and not st:
                allowed = sources | {os.path.join(REPO, p) for p in (tu + '.cpp', tu + '.h', x + '.cpp')}
                named = sorted(q for q in tree.where.get(leaf, set()) - allowed if not own_name(q, leaf, ns))
                if named:
                    extern.append('%s (named in %s)' % (leaf, os.path.relpath(named[0], REPO)))
                else:
                    st = True
            if tu != x:
                spec.append((m, leaf, ns, tu, st))
            elif st and not fns[tu + '|' + by_start[m][4]].static:
                promote.append((m, leaf))
        # Independent parts of the group are separate edits: a function
        # made static goes with its callers that move, nothing else does.
        elems = {sp[0] for sp in spec} | {p[0] for p in promote}
        part = {e: e for e in elems}

        def root(e):
            while part[e] != e:
                part[e] = part[part[e]]
                e = part[e]
            return e

        for g in members:
            if g in bound and g in elems:
                for c in callers[g]:
                    if c in elems:
                        part[root(c)] = root(g)
        pieces = collections.defaultdict(lambda: ([], []))
        for sp in spec:
            pieces[root(sp[0])][0].append(sp)
        for pr in promote:
            pieces[root(pr[0])][1].append(pr)
        made = False
        for sub_spec, sub_promote in pieces.values():
            if not sub_spec:
                continue                         # nothing moves: fix_static's business
            # Made static in place only if named nowhere but in what this
            # piece moves, its own files and X.cpp.
            mine = {os.path.join(REPO, sp[3] + '.cpp') for sp in sub_spec}
            allowed = mine | {os.path.join(REPO, p) for p in (x + '.cpp', x + '.h')}
            sub_promote = [(m, leaf) for m, leaf in sub_promote
                           if not [q for q in tree.where.get(leaf, set()) - allowed
                                   if not own_name(q, leaf, by_start[m][2].rsplit('::', 1)[0])]]
            sub_spec = [sp[:4] + (sp[4] and not [q for q in tree.where.get(sp[1], set()) - allowed -
                                                 {os.path.join(REPO, sp[3] + '.h')}
                                                 if not own_name(q, sp[1], sp[2])],)
                        for sp in sub_spec]
            if sub_promote or any(sp[4] and sp[0] in bound for sp in sub_spec):
                moves.append(Move(x, sub_spec, sub_promote, addr))
                made = True
        if not made:
            for m in members:
                why[m] = 'no function of its group could be static: %s' % '; '.join(extern)
    moves.sort(key=lambda mv: (mv.x, mv.movers))
    return moves, why, callers


def refresh(rows, fns):
    """The funcmap's rows, a symbol the build no longer has - a fixer since
    the map changed its mangling (fix_fastcall's convention, fix_return's
    type) - replaced by the one function of that name in its TU. What has
    none or several stays as it is, and candidates() leaves its group alone."""
    by_name = collections.defaultdict(list)
    for f in fns.values():
        by_name[(f.tu, f.name)].append(f.symbol)
    out = []
    for s, e, name, tu, sym, mt, d in rows:
        if tu + '|' + sym not in fns and len(by_name.get((tu, name), ())) == 1:
            sym = by_name[(tu, name)][0]
        out.append((s, e, name, tu, sym, mt, d))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--apply', action='store_true')
    ap.add_argument('--max-span', type=int, default=MAX_SPAN)
    ap.add_argument('patterns', nargs='*')
    args = ap.parse_args()
    rows = audit.load_funcmap(FUNCMAP)
    _ref, model = audit.load_context()
    fns = {f.key: f for f in ourslib.load_all(os.path.join(BUILD, 'obj'))}
    rows = refresh(rows, fns)
    tree = Tree()
    moves, why, callers = candidates(rows, model, fns, tree, args.patterns, args.max_span)
    print('%d functions in %d moves' % (sum(len(m.movers) for m in moves), len(moves)))
    if not args.apply:
        for m in moves:
            print('  ' + m.label)
        for s in sorted(why):
            print('  no: %08x %s: %s' % (s, next(r[2] for r in rows if r[0] == s), why[s]))
        return 0

    # Each move alone, through the mingw compiler: the files it touches must compile.
    mingw = Mingw()
    placed = collections.defaultdict(dict)       # X -> leaf -> original start, of what moves there
    for m in moves:
        for s, leaf, _n, _y, _st in m.movers:
            placed[m.x][leaf] = s
    paths = sorted({p for m in moves for p in m.paths() if os.path.exists(p)})
    pristine = {p: fixlib.read(p) for p in paths}
    tried = []
    for m in moves:
        for inc in ('used', 'all'):
            m.includes = inc
            new = m.apply(dict(pristine), placed[m.x])
            if new is None:
                print('  cannot apply (%s): %s' % (m.why, m.label), flush=True)
                break
            for p, t in new.items():
                fixlib.write(p, t)
            ok = [mingw.ok(p) for p in new if p.endswith('.cpp')]
            if all(o is not False for o in ok):
                trim_includes(m, new, pristine, placed[m.x], mingw)
                for p in new:
                    fixlib.write(p, pristine[p])
                tried.append(m)
                break
            for p in new:
                fixlib.write(p, pristine[p])
        else:
            print('  mingw rejects: %s' % m.label, flush=True)
    print('%d of %d moves compile with mingw' % (len(tried), len(moves)), flush=True)
    if not tried:
        fixlib.record('fix_refile', sum(len(m.movers) for m in moves), 0, 0)
        print('kept 0 of %d moves' % len(moves))
        return 0

    edits = []
    for m in tried:
        owners = m.owners
        affects = owners | {c for s in owners for c in callers[s]}
        edits.append(fixlib.Edit(m.label, owners, affects, lambda texts, m=m: m.apply(texts, placed[m.x]),
                                 excused_by_match=True))
    move_of = {e.label: m for e, m in zip(edits, tried)}
    syms = audit.sym_map(rows)
    quarantined = audit.load_quarantine(os.path.join(BUILD, 'quarantine.tsv'))
    sym_of = {r[0]: r[4] for r in rows}
    tu_of = {r[0]: r[3] for r in rows}
    before = {s: refset(fns.get(tu_of[s] + '|' + sym_of[s])) for m in tried for s in m.owners}

    def rows_for(active):
        moved = {mv[0]: move_of[e.label].x for e in active
                 for mv in move_of[e.label].movers + move_of[e.label].helpers}
        return [(s, e, n, moved.get(s, tu), sym, mt, d) for s, e, n, tu, sym, mt, d in rows]

    pats = sorted({t for m in tried for t in [m.x] + m.ys})
    _built, failed_before = build_tree()         # the baseline must be current
    base = check.judge(rows, pats, model, syms, quarantined)
    base_map = derived_map()
    active = list(edits)
    state, built = None, set()
    for rnd in range(ROUNDS):
        active = fixlib.materialise(pristine, active)
        if not active:
            break
        built, failed = build_tree()
        failed -= failed_before
        quarantined = audit.load_quarantine(os.path.join(BUILD, 'quarantine.tsv'))
        orphans = failed - {t for e in active for t in [move_of[e.label].x] + move_of[e.label].ys}
        if orphans:
            # A TU no move touched names a function a move made static.
            blamed = set()
            for t in orphans:
                text = blank_comments(fixlib.read(os.path.join(REPO, t + '.cpp')))
                for e in active:
                    m = move_of[e.label]
                    leaves = [mv[1] for mv in m.movers if mv[4]] + [pr[1] for pr in m.promote]
                    if any(re.search(r'\b%s\b' % re.escape(l), text) for l in leaves):
                        blamed.add(e.label)
            print('  failed to compile, though no move touched them: %s; dropping %d moves naming them'
                  % (', '.join(sorted(orphans)), len(blamed)), flush=True)
            if not blamed:
                active, state = [], None
                break
            active = [e for e in active if e.label not in blamed]
            state = None
            continue
        cur = rows_for(active)
        state = check.judge(cur, pats, model, syms, quarantined)
        lost = (fixlib.matches(base) - fixlib.matches(state)) | fixlib.vanished(base, state)
        won = fixlib.matches(state) - fixlib.matches(base)
        tu_now = {r[0]: r[3] for r in cur}
        # A loss among an edit's own functions is that edit's; any other loss
        # in a TU is laid on every edit touching the TU.
        explained = {s for s in lost if any(s in e.affects for e in active)}
        bad = {}
        for e in active:
            m = move_of[e.label]
            mine = {m.x} | set(m.ys)
            if mine & failed:
                bad[e.label] = 'did not compile'
            elif e.affects & lost or any(tu_now.get(s) in mine for s in lost - explained):
                bad[e.label] = 'a match lost: %s' % ', '.join(
                    '%08x' % s for s in sorted(lost) if s in e.affects or
                    (s not in explained and tu_now.get(s) in mine))
            elif sum(state[s][1] - base[s][1] for s in e.affects if s in state and s in base) < -fixlib.SIM_EPS \
                    and not e.affects & won:
                bad[e.label] = 'less similar'
            elif any(refset(our_function(tu_now[s], sym_of[s])) != before[s] for s in e.owners):
                bad[e.label] = 'refers to something else'     # its new file binds a name differently
        if not bad:
            # The map pairs a function by its neighbours in its TU (bracket,
            # align): a move can leave one the next `oracle map` no longer
            # pairs, which no judgement by the old map sees (DebugThread
            # into Fog/Debug unpaired ILT_strrchr). A function the map paired
            # before and pairs no longer, or with another, is laid on every
            # edit touching its TU.
            now_map = derived_map()
            unmapped = {s for s, (_t, sym) in base_map.items() if now_map.get(s, (None, None))[1] != sym}
            for e in active:
                m = move_of[e.label]
                mine = {m.x} | set(m.ys)
                hit = sorted(s for s in unmapped if base_map[s][0] in mine or s in e.affects)
                if hit:
                    bad[e.label] = 'unmaps %s' % ', '.join('%08x' % s for s in hit)
            for s in sorted(unmapped - {s for e in active for s in e.affects} -
                            {s for e in active for s in unmapped
                             if base_map[s][0] in {move_of[e.label].x} | set(move_of[e.label].ys)}):
                print('  unmapped, in a TU no move touched: %08x %s' % (s, base_map[s][1]), flush=True)
        for label, reason in sorted(bad.items()):
            print('  drop (%s): %s' % (reason, label), flush=True)
        claimed = {s for e in active for s in lost
                   if s in e.affects or tu_now.get(s) in {move_of[e.label].x} | set(move_of[e.label].ys)}
        for s in sorted(lost - claimed):
            print('  lost, in a TU no move touched (symbol phase?): %08x %s' % (s, state[s][7]), flush=True)
        print('round %d: %d moves, %d TUs failed, %d lost, %d won, %d to drop' % (
            rnd + 1, len(active), len(failed), len(lost), len(won), len(bad)), flush=True)
        if not bad:
            break
        active = [e for e in active if e.label not in bad]
        state = None
    if state is None:
        fixlib.materialise(pristine, [])
        build_tree()
        print('kept 0 of %d moves' % len(moves))
        fixlib.record('fix_refile', sum(len(m.movers) for m in moves), 0, 0)
        return 0
    gained = len(fixlib.matches(state) - fixlib.matches(base))
    others = sorted(set(built) - set(pats))
    if others:
        # TUs no move touched, rebuilt for a header (most are in the
        # precompiled header): judged for the audit rows and the record.
        rest = {s: v for s, v in check.judge(rows_for(active), others, model, syms, quarantined).items()
                if v[8] in others}             # judge() takes patterns: Sound/Sound is in Sound/SoundHdr
        old = {}
        for line in open(check.AUDIT):
            p = line.rstrip('\n').split('\t')
            if p[0] != 'start' and len(p) > 9:
                old[int(p[0], 16)] = p[9]
        for s, v in sorted(rest.items()):
            if (old.get(s) == 'MATCH') != (v[0] == 'MATCH'):
                print('  symbol phase? %08x %s (%s): %s -> %s, its TU untouched - bench test it'
                      % (s, v[7], v[8], old.get(s), v[0]))
        state.update(rest)
    check.update_rows(state)
    kept = [move_of[e.label] for e in active]
    refile_funcmap({mv[0]: m.x for m in kept for mv in m.movers + m.helpers})
    for m in kept:
        print('  kept %s' % m.label)
    nf = sum(len(m.movers) + len(m.helpers) for m in kept)
    print('kept %d of %d moves (%d functions), MATCH %+d' % (len(kept), len(moves), nf, gained))
    fixlib.record('fix_refile', sum(len(m.movers) for m in moves), nf, gained)
    return 0


def derived_map():
    """original start -> (TU, symbol), as funcmap.py pairs the current build."""
    out = os.path.join(BUILD, 'refile-map.tsv')
    p = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'funcmap.py'), '--out', out],
                       capture_output=True, text=True)
    if p.returncode != 0:
        fail('funcmap.py failed:\n' + p.stderr[-2000:])
    return {r[0]: (r[3], r[4]) for r in audit.load_funcmap(out)}


def refset(f):
    """What a function refers to, but for its own code's offsets and the
    source file names __FILE__ puts in: the names a move must not rebind."""
    if f is None:
        return None
    out = set()
    for _o, k, t in f.refs:
        if k == 'self':
            continue
        if k == 'string' and b'.cpp' in t.lower():
            continue
        # A function's address is `fptr` when the function is in the same
        # object, `data` when it is not: the same name either way.
        out.add(('data' if k == 'fptr' else k, t))
    return out


def our_function(tu, sym):
    path = os.path.join(BUILD, 'obj', tu + '.obj')
    if not os.path.exists(path):
        return None
    for f in ourslib.load_object(os.path.join(BUILD, 'obj'), tu + '.obj'):
        if f.symbol == sym:
            return f
    return None


def refile_funcmap(new_tu):
    """funcmap.tsv with the moved functions' rows in their new TUs."""
    lines = open(FUNCMAP).read().split('\n')
    out = []
    for line in lines:
        p = line.split('\t')
        if len(p) > 3 and p[0] != 'start' and int(p[0], 16) in new_tu:
            p[3] = new_tu[int(p[0], 16)]
            line = '\t'.join(p)
        out.append(line)
    open(FUNCMAP, 'w').write('\n'.join(out))


if __name__ == '__main__':
    sys.exit(main())
