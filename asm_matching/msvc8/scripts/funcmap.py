#!/usr/bin/env python3
"""funcmap.py - map every compiled function to its address in the original.

    funcmap.py [--out evidence/funcmap.tsv]

The original carries no symbols and the tree no addresses, so the map is
derived from evidence, strongest first; each row records which tier made it:

  sourcemap the generator's own .cpp.map for its file names its address
            (ghidra-reconstruct writes one per .cpp; this repository ignores
            them, but when a checkout has them they are exact). By qualified
            name, else by a bare name unique in the TU on both sides; last of
            all, a bare name left over once its namesakes are placed. No
            inferred tier pairs a function away from the address its map names
  name      its generated name ends in the address Ghidra knew it by
            (FOO_00512340): taken from the database the tree came from
  export    the original exports it under this mangled name
  exact     its code is identical to one original function's - the bytes with
            every reference masked, and the references known on both sides
            (strings, imports, switch-table offsets) - and no other function
            on either side has that code; at least EXACT_MIN instructions
  located   the same, but for source locations: where ours pushes its own
            source line and loads a file name of the oracle's tree
            (__LINE__, __FILE__ - upstream's allocator macros pass them),
            the original pushes some number and loads some string; jumps
            compared by the instruction they reach, since a line pushed as
            imm8 on one side and imm32 on the other moves the code after it
  string    it references string literals that occur in at most a few original
            functions, and one original function holds (nearly) all of them
  import    the same sequence of imported-API references, unique on both sides
  votes     every call that lines up between a pair of the first pass's map
            (the two instruction streams aligned by mnemonic) says "our callee
            sits where the original calls that"; a callee most of its call
            sites agree on is pinned before anything else, and the map is
            derived again around the pins - so the evidence of many agreeing
            call sites outweighs one ambiguous one
  calls     an already-mapped caller calls it at the same position of its call
            sequence: both sequences (calls and imported-API calls, in code
            order) are aligned on the callees already known on both sides, and
            a single unknown callee between agreeing neighbours is paired
            (repeated to a fixpoint)
  bracket   it lies, in its TU, between two mapped functions whose
            original counterparts have exactly as many unmapped functions
            between them; paired in order - unless an alignment by code
            similarity pairs the gap clearly better, which says the two gaps
            hold different functions (then it is left to `align`). The order
            is the source line's, not the object's: the generator writes
            functions in address order, and VC8 emits a caller of a function
            defined further down at the end of the object
            (ours.OurFunc.layout_key)
  align     the same gap with unequal counts, aligned by instruction-sequence
            similarity, accepted only above a similarity threshold

Within a bracket gap, identical code unique in the gap is paired first
(`exact` too): code without calls by itself, code with calls only when the
functions it calls are mapped and are the ones the original calls. A gap
with another of our files' code mapped inside is not one object's (the
generator split the files differently there): nothing in it is paired by
position, and identical code only while the gap is narrow (FOREIGN_GAP),
else only code unique overall. A pair made by position, similarity or shape
(bracket, align, exact code with calls in it, located) whose callees, all
mapped, contradict the original's is dropped at the end.
  refs      what it refers to that is known on both sides - mapped callees,
            CRT functions, imports, strings - weighted by rarity, matches one
            original function clearly better than any other (repeated: every
            round of mapping makes more references known)

Callees in the statically linked CRT are known on both sides through
evidence/crtmap.tsv (crtmap.py), so they anchor sequences like imports do.

Every pairing is one-to-one; a function claimed twice at the same tier keeps
neither. Output (tab-separated, sorted by address):
  start  end  name  tu  symbol  method  detail
plus <out>.unmapped listing our functions with no counterpart found.
"""
import argparse
import bisect
import collections
import difflib
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ours as ourslib  # noqa: E402
import refimage         # noqa: E402

from oracle_config import ORACLE, BUILD, REFCACHE, EXPERIMENT  # noqa: E402

MAX_STRING_FANOUT = 4      # a string in more original functions than this is no anchor
ALIGN_THRESHOLD = 0.80     # minimum mnemonic-sequence similarity for an 'align' pairing
EXACT_MIN = 6              # instructions; shorter code is too generic to be unique evidence
OVERWHELM_VOTES = 20       # call sites agreeing on a callee ...
OVERWHELM_SHARE = 0.95     # ... and their share: enough to outvote identical code
COOKIE_CHECK = ('fn', 0x00681a48)   # __security_check_cookie, the /GS epilogue's call
BRACKET_MARGIN = 0.5       # total similarity by which an alignment must beat the in-order pairing
FOREIGN_GAP = 8            # unmapped original functions: a gap with another file's code inside
                           # and more than these pairs identical code only if unique overall
OWN_TREE = b'asm_matching'  # in a string of ours: a __FILE__ of the oracle's own build
PUSH_IMM = re.compile(r'^push (0x[0-9a-f]+|\d+)$')
JUMP = re.compile(r'^(?:j[a-z]+|loop[a-z]*) (0x[0-9a-f]+)$')


def mnemonics(decoded, cap=600):
    return [i.mnemonic for i in decoded.insns[:cap]]


class Matcher:
    def __init__(self, ref, model, fns, crt=None):
        self.ref = ref
        self.crt = crt or {}           # qualified CRT name -> original start
        self.crt_starts = set(self.crt.values())
        self.model = model
        self.starts = sorted(model)
        self.fns = fns
        self.by_key = {f.key: f for f in fns}
        self.layout = {f.key: f.layout_key for f in fns}   # computed once: it reads every line record
        self.by_symbol = collections.defaultdict(list)
        for f in fns:
            self.by_symbol[f.symbol].append(f)
        self.sourcemap_addr = {}   # our key -> the address its .cpp.map names (by_sourcemaps)
        self.map = {}       # our key -> (start, method, detail)
        self.rmap = {}      # start -> our key
        self.r_strings, self.r_imports, self.r_calls = {}, {}, {}
        self.str_index = collections.defaultdict(set)
        self.r_seq = {}
        for s, (dec, refs) in model.items():
            refs = sorted(refs)
            self.r_strings[s] = [t for _a, _o, _n, k, t in refs if k == 'string']
            self.r_imports[s] = [t.split('!', 1)[1] for _a, _o, _n, k, t in refs if k == 'import']
            self.r_calls[s] = [t for _a, _o, _n, k, t in refs if k == 'call']
            # Calls and import calls in code order: ('imp', name) or ('fn', start)
            self.r_seq[s] = [('imp', t.split('!', 1)[1]) if k == 'import' else ('fn', t)
                             for _a, _o, _n, k, t in refs if k in ('call', 'import')]
            for st in self.r_strings[s]:
                self.str_index[st].add(s)

    # -- bookkeeping ---------------------------------------------------------
    def assign(self, pairs, method):
        """pairs: list of (score, our key, start, detail). One-to-one, highest
        score first; a key or start proposed twice at equal best score is dropped."""
        best_k, best_s = {}, {}
        for sc, k, s, d in pairs:
            if k in self.map or s in self.rmap:
                continue
            if method != 'sourcemap' and not self.sourcemap_allows(k, s):
                continue
            if k not in best_k or sc > best_k[k][0]:
                best_k[k] = (sc, s, d, 1)
            elif sc == best_k[k][0] and s != best_k[k][1]:
                best_k[k] = (sc, s, d, best_k[k][3] + 1)
            if s not in best_s or sc > best_s[s][0]:
                best_s[s] = (sc, k, 1)
            elif sc == best_s[s][0] and k != best_s[s][1]:
                best_s[s] = (sc, k, best_s[s][2] + 1)
        n = 0
        for k, (sc, s, d, ties) in sorted(best_k.items(), key=lambda kv: -kv[1][0]):
            if ties > 1 or best_s.get(s, (0, None, 2))[1] != k or best_s[s][2] > 1:
                continue
            if k in self.map or s in self.rmap:
                continue
            self.map[k] = (s, method, d)
            self.rmap[s] = k
            n += 1
        return n

    def sourcemap_allows(self, k, s):
        """An inferred pair may not contradict the generator's map: when the
        map names an address for our function, the original function it is
        paired with must contain it. The map's address can sit inside the carve
        (a Ghidra entry a few bytes past the carved start) but never outside."""
        a = self.sourcemap_addr.get(k)
        return a is None or s <= a < self.model[s][0].end

    def resolve_callee(self, caller, symbol):
        cands = self.by_symbol.get(symbol, [])
        if len(cands) == 1:
            return cands[0]
        same = [c for c in cands if c.tu == caller.tu]
        return same[0] if len(same) == 1 else None

    # -- tiers ---------------------------------------------------------------
    @staticmethod
    def bare_name(name):
        """The last component of a qualified name, without a parameter list:
        'TSList<>::Link' and the map's 'Link' in namespace
        'TSList<TCELENTRY, TSGetLink<TCELENTRY>>' are both 'Link'."""
        name = name.split('(', 1)[0]
        depth, cut = 0, 0
        for i, ch in enumerate(name):
            if ch == '<':
                depth += 1
            elif ch == '>':
                depth -= 1
            elif ch == ':' and depth == 0 and name.startswith('::', i):
                cut = i + 2
        return name[cut:]

    @staticmethod
    def sourcemap_files(root):
        """Every .cpp.map under root, by its TU path case-folded: the build's
        TU spelling ('D2Client/Game/...') and the checkout's ('D2Client/GAME/...')
        differ in case, and a case-sensitive file system would miss the file."""
        out = {}
        for d, dirs, files in os.walk(root):
            dirs[:] = [x for x in dirs if not x.startswith('.') and x != 'asm_matching']
            for n in files:
                if n.endswith('.cpp.map'):
                    p = os.path.join(d, n)
                    out[os.path.relpath(p, root)[:-len('.cpp.map')].replace(os.sep, '/').lower()] = p
        return out

    def sourcemap_entries(self, root):
        """(our functions of the TU, map path, the map's functions) per TU
        that has a <tu>.cpp.map. Read once: both sourcemap tiers use it."""
        import json
        if getattr(self, '_sourcemaps', None) is not None:
            return self._sourcemaps
        by_tu = collections.defaultdict(list)
        for f in self.fns:
            by_tu[f.tu].append(f)
        maps = self.sourcemap_files(root)
        out = []
        for tu, ours in by_tu.items():
            path = maps.get(tu.lower())
            if path is None:
                continue
            try:
                sm = json.load(open(path))
            except ValueError:
                continue
            entries = []
            for fn in sm.get('functions', []):
                try:
                    a = int(str(fn.get('address', '')), 16)
                except ValueError:
                    continue
                q = (fn.get('namespace', '') + '::' if fn.get('namespace') else '') + fn.get('name', '')
                entries.append((q, fn.get('name', ''), a))
            out.append((ours, path, entries))
        self._sourcemaps = out
        return out

    def by_sourcemaps(self, root):
        """<tu>.cpp.map files next to the sources: {"functions": [{"name",
        "namespace", "address"}, ...]} (ghidra-reconstruct codegen/sourcemap.ts).

        Ours is found by its qualified name, else by its bare name when that is
        unique in the TU on both sides: our demangled name strips template
        arguments ('TSList<>') and can lose a namespace level the map spells
        ('Renderer::Direct3D::CheckCutScene' against ours 'Renderer::CheckCutScene'),
        so the qualified lookup misses functions the map does name."""
        pairs = []
        for ours, path, entries in self.sourcemap_entries(root):
            names = {f.name: f for f in ours}
            ours_bare = collections.Counter(self.bare_name(f.name) for f in ours)
            map_bare = collections.Counter(self.bare_name(n) for _q, n, _a in entries)
            bare = {self.bare_name(f.name): f for f in ours}
            for q, n, a in entries:
                f = names.get(q) or names.get(n)
                detail = 'sourcemap ' + os.path.basename(path)
                if f is None:
                    b = self.bare_name(n)
                    if b and ours_bare[b] == 1 and map_bare[b] == 1:
                        f, detail = bare[b], detail + ' (bare name)'
                if f is not None:
                    self.sourcemap_addr[f.key] = a
                if f is not None and a in self.model:
                    pairs.append((10.0, f.key, a, detail))
        return self.assign(pairs, 'sourcemap')

    def by_sourcemap_rest(self, root):
        """Last: a map entry whose address nothing paired, and one of our
        functions nothing paired, with the same bare name, each the only one of
        that name left in the TU - provided every other entry of that name is
        paired, at its own address, with one of ours of that name. The bare name
        was ambiguous only because of functions already placed where the map
        says ('Destructor' among a TU's template instantiations)."""
        pairs = []
        for ours, path, entries in self.sourcemap_entries(root):
            by_bare = collections.defaultdict(list)
            for f in ours:
                by_bare[self.bare_name(f.name)].append(f)
            map_by_bare = collections.defaultdict(list)
            for _q, n, a in entries:
                map_by_bare[self.bare_name(n)].append(a)
            for b, addrs in map_by_bare.items():
                fs = by_bare.get(b, [])
                if not b or len(addrs) < 2 or len(set(addrs)) != len(addrs):
                    continue
                free_a = [a for a in addrs if a not in self.rmap]
                free_f = [f for f in fs if f.key not in self.map]
                if len(free_a) != 1 or len(free_f) != 1 or free_a[0] not in self.model:
                    continue
                keys = {f.key for f in fs}
                if any(self.rmap[a] not in keys for a in addrs if a != free_a[0]):
                    continue
                pairs.append((10.0, free_f[0].key, free_a[0],
                              'sourcemap %s (bare name, the last of %d)' % (os.path.basename(path), len(addrs))))
        return self.assign(pairs, 'sourcemap')

    def by_names(self):
        import re
        pairs = []
        for f in self.fns:
            m = re.search(r'_(00[4-6][0-9a-f]{5}|[4-6][0-9a-f]{5})$', f.name)
            if m and int(m.group(1), 16) in self.model:
                pairs.append((5.0, f.key, int(m.group(1), 16), 'name ' + f.name.rsplit('::', 1)[-1]))
        return self.assign(pairs, 'name')

    def ref_tokens(self, s):
        dec, refs = self.model[s]
        out = collections.Counter()
        for _a, _o, _n, k, t in refs:
            if k == 'call' and t in self.model:
                out[('fn', t)] += 1
            elif k == 'import':
                out[('imp', t.split('!', 1)[1])] += 1
            elif k == 'string':
                out[('str', t)] += 1
        return out

    def our_tokens(self, f):
        out = collections.Counter()
        for _o, k, t in f.refs:
            if k == 'import':
                out[('imp', t)] += 1
            elif k == 'string':
                out[('str', t)] += 1
            elif k == 'call':
                g = self.resolve_callee(f, t)
                if g is not None and g.key in self.map:
                    out[('fn', self.map[g.key][0])] += 1
                elif g is None and ourslib_qualified(t) in self.crt:
                    out[('fn', self.crt[ourslib_qualified(t)])] += 1
        return out

    def by_refs(self):
        import math
        free = [s for s in self.starts if s not in self.rmap]
        index = collections.defaultdict(list)
        rtok = {}
        for s in free:
            rtok[s] = self.ref_tokens(s)
            for t in rtok[s]:
                index[t].append(s)
        # Rarity over the whole original, so a call to an assertion handler counts for little.
        df = collections.Counter()
        for s in self.starts:
            for t in (rtok.get(s) or self.ref_tokens(s)):
                df[t] += 1
        n_all = len(self.starts)
        pairs = []
        for f in self.fns:
            if f.key in self.map:
                continue
            ot = self.our_tokens(f)
            if sum(ot.values()) < 2:
                continue
            scores = collections.Counter()
            for t, c in ot.items():
                if df[t] > 50:
                    continue
                w = math.log(n_all / (1 + df[t]))
                for s in index.get(t, ()):
                    scores[s] += w * min(c, rtok[s][t])
            if not scores:
                continue
            top = scores.most_common(2)
            best, sc = top[0]
            second = top[1][1] if len(top) > 1 else 0.0
            total = sum(math.log(n_all / (1 + df[t])) * c for t, c in ot.items() if df[t] <= 50)
            # Clearly ahead, and covering most of what f refers to.
            if sc >= 2 * max(second, 1e-9) and sc >= 0.6 * total and sc >= 8.0:
                pairs.append((sc / 100.0, f.key, best, 'refs %.1f (next %.1f)' % (sc, second)))
        return self.assign(pairs, 'refs')

    @staticmethod
    def code_key(view):
        """What identical code shows on both sides: per instruction the masked
        bytes and the references known on both - strings, imports, switch-table
        offsets - with functions and globals only as kinds. None for code
        with an unrecognised table in it."""
        parts = []
        for i in view:
            toks = []
            for t in i.tokens:
                if t[0] == 'straddle':
                    return None
                toks.append(t if t[0] in ('str', 'import', 'self') else ('fn',) if t[0] in ('fn', 'fn?') else (t[0],))
            parts.append((i.masked, tuple(toks)))
        return tuple(parts)

    def code_keys(self):
        """code_key of every function on both sides, computed once."""
        import compare
        if not hasattr(self, '_okey'):
            self._okey = {f.key: self.code_key(compare.our_view(f, {}, render=False)) for f in self.fns}
            self._rkey = {st: self.code_key(compare.ref_view(st, self.model[st], render=False)) for st in self.starts}
        return self._okey, self._rkey

    def by_exact(self):
        if not hasattr(self, '_exact_keys'):
            okey, rkey = self.code_keys()
            ours, theirs = collections.defaultdict(list), collections.defaultdict(list)
            for f in self.fns:
                if f.insns >= EXACT_MIN and okey[f.key] is not None:
                    ours[okey[f.key]].append(f.key)
            for st in self.starts:
                k = rkey[st]
                if k is not None and k in ours and len(self.model[st][0].insns) >= EXACT_MIN:
                    theirs[k].append(st)
            self._exact_keys = [(ours[k], theirs[k]) for k in theirs]
        pairs = []
        for keys, starts in self._exact_keys:
            keys = [k for k in keys if k not in self.map]
            starts = [st for st in starts if st not in self.rmap]
            if len(keys) == 1 and len(starts) == 1:
                pairs.append((6.0, keys[0], starts[0], 'identical code, %d insns' % self.by_key[keys[0]].insns))
        return self.assign(pairs, 'exact')

    @staticmethod
    def coarse_key(view, key):
        """code_key without what a source location changes: pushed numbers,
        string contents, jump distances, offsets into the function itself."""
        out = []
        for x, (masked, toks) in zip(view, key):
            if PUSH_IMM.match(x.text):
                out.append('push')
            elif JUMP.match(x.text):
                out.append(x.mnemonic)
            else:
                out.append((masked, tuple((t[0],) if t[0] in ('str', 'self') else t for t in toks)))
        return tuple(out)

    @staticmethod
    def located_equal(ov, okey, rv, rkey):
        """(lines, files) where our code differs from the original's only by
        its source locations, else None: every instruction the same - a jump
        reaching the same instruction - except where ours pushes its own
        source line (the original: some number) or loads a file name of the
        oracle's tree (the original: some string). Both kinds of site, at
        least one each."""
        if len(ov) != len(rv):
            return None
        oi = {x.addr: k for k, x in enumerate(ov)}
        ri = {y.addr: k for k, y in enumerate(rv)}
        lines = files = 0
        for x, y, (xm, xt), (ym, yt) in zip(ov, rv, okey, rkey):
            jx, jy = JUMP.match(x.text), JUMP.match(y.text)
            if jx or jy:
                if not (jx and jy and x.mnemonic == y.mnemonic):
                    return None
                tx, ty = oi.get(int(jx.group(1), 16)), ri.get(int(jy.group(1), 16))
                if tx is None or tx != ty:
                    return None
                continue
            if (xm, xt) == (ym, yt):
                continue
            m = PUSH_IMM.match(x.text)
            if m and x.line and int(m.group(1), 0) == x.line and PUSH_IMM.match(y.text):
                lines += 1
                continue
            if xm == ym and len(xt) == len(yt) and all(
                    a == b or (a[0] == 'str' and OWN_TREE in a[1] and b[0] == 'str') for a, b in zip(xt, yt)):
                files += 1
                continue
            return None
        return (lines, files) if lines and files else None

    def by_located(self):
        """Pairs of located_equal code, unique on both sides among the
        unmapped functions with the same coarse_key."""
        import compare
        okey, rkey = self.code_keys()
        if not hasattr(self, '_located'):
            groups, views = collections.defaultdict(lambda: ([], [])), {}
            for f in self.fns:
                if f.insns < EXACT_MIN or okey[f.key] is None or not any(
                        kind == 'string' and OWN_TREE in t for _o, kind, t in f.refs):
                    continue
                v = compare.our_view(f, {})
                views[f.key] = v
                groups[self.coarse_key(v, okey[f.key])][0].append(f.key)
            lengths = {len(k) for k in groups}
            for st in self.starts:
                if rkey[st] is None or len(rkey[st]) not in lengths or len(rkey[st]) < EXACT_MIN:
                    continue
                v = compare.ref_view(st, self.model[st])
                k = self.coarse_key(v, rkey[st])
                if k in groups:
                    groups[k][1].append(st)
                    views[st] = v
            self._located = (groups, views)
        groups, views = self._located
        pairs = []
        for keys, starts in groups.values():
            found = []
            for k in keys:
                if k in self.map:
                    continue
                for st in starts:
                    if st not in self.rmap:
                        sites = self.located_equal(views[k], okey[k], views[st], rkey[st])
                        if sites:
                            found.append((k, st, sites))
            nk = collections.Counter(k for k, _s, _n in found)
            ns = collections.Counter(st for _k, st, _n in found)
            for k, st, (lines, files) in found:
                if nk[k] == 1 and ns[st] == 1:
                    pairs.append((5.0, k, st, 'identical code but for %d source lines and %d file names, %d insns'
                                  % (lines, files, self.by_key[k].insns)))
        return self.assign(pairs, 'located')

    def by_exports(self):
        pairs = []
        exports = {refimage_qualified(n): v for n, v in self.ref.pe.exports.items()}
        for f in self.fns:
            for q, v in exports.items():
                if v in self.model and (f.name == q or f.name.endswith('::' + q)):
                    pairs.append((3.0, f.key, v, 'export ' + q))
        return self.assign(pairs, 'export')

    def by_strings(self):
        pairs = []
        for f in self.fns:
            rare = [s for s in set(f.strings) if len(s) >= 4 and 0 < len(self.str_index.get(s, ())) <= MAX_STRING_FANOUT]
            if not rare:
                continue
            votes = collections.Counter()
            for s in rare:
                for a in self.str_index[s]:
                    votes[a] += 1
            top = votes.most_common(2)
            (a, n) = top[0]
            if n * 2 < len(rare) or (len(top) > 1 and top[1][1] == n):
                continue
            pairs.append((1.0 + n / 100.0, f.key, a, '%d/%d strings' % (n, len(rare))))
        return self.assign(pairs, 'string')

    def by_imports(self):
        index = collections.defaultdict(list)
        for s, imps in self.r_imports.items():
            if len(imps) >= 2:
                index[tuple(imps)].append(s)
        pairs = []
        for f in self.fns:
            if len(f.imports) >= 2:
                c = index.get(tuple(f.imports), [])
                if len(c) == 1:
                    pairs.append((1.0, f.key, c[0], '%d imports' % len(f.imports)))
        return self.assign(pairs, 'import')

    def our_seq(self, f):
        """f's calls in code order: ('imp', name), ('fn', start) when the callee
        is mapped or a known CRT function, else ('our', OurFunc or symbol)."""
        out = []
        for _o, k, t in f.refs:
            if k == 'import':
                out.append(('imp', t))
            elif k == 'call':
                g = self.resolve_callee(f, t)
                if g is not None and g.key in self.map:
                    out.append(('fn', self.map[g.key][0]))
                elif g is None and ourslib_qualified(t) in self.crt:
                    out.append(('fn', self.crt[ourslib_qualified(t)]))
                else:
                    out.append(('our', g if g is not None else t))
        return out

    def by_calls(self):
        pairs = []
        for k, (s, _m, _d) in list(self.map.items()):
            f = self.by_key[k]
            a = self.our_seq(f)
            b = self.r_seq.get(s, [])
            if not a or not b:
                continue
            # Align on what both sides know; an unknown never equals anything.
            ka = [x if x[0] != 'our' else ('?', id(x)) for x in a]
            kb = [y if not (y[0] == 'fn' and y[1] not in self.rmap and y[1] not in self.crt_starts)
                  else ('?', -1 - j) for j, y in enumerate(b)]
            sm = difflib.SequenceMatcher(None, ka, kb, autojunk=False)
            for op, i1, i2, j1, j2 in sm.get_opcodes():
                if op != 'replace':
                    continue
                # One unknown between agreeing neighbours on both sides. A longer
                # run is ambiguous: the tree and the original may call in another
                # order (seen: two callees swapped). The stack-cookie check does
                # not count: VC8 places its own __security_check_cookie call where
                # the epilogue lands, which can be inside the gap on one side only
                # (SErrDisplayError's call of __break, 0041c160).
                xa = [x for x in a[i1:i2] if x != COOKIE_CHECK]
                yb = [y for y in b[j1:j2] if y != COOKIE_CHECK]
                if len(xa) != 1 or len(yb) != 1:
                    continue
                for x, y in zip(xa, yb):
                    if x[0] != 'our' or not isinstance(x[1], ourslib.OurFunc) or y[0] != 'fn':
                        continue
                    if y[1] in self.rmap or y[1] in self.crt_starts or y[1] not in self.model:
                        continue
                    pairs.append((1.0, x[1].key, y[1], 'called from ' + f.name))
        # A callee proposed at two different addresses by two callers: drop it.
        seen = collections.defaultdict(set)
        for _sc, k, t, _d in pairs:
            seen[k].add(t)
        pairs = [p for p in pairs if len(seen[p[1]]) == 1]
        return self.assign(pairs, 'calls')

    def by_gap_exact(self):
        """Only the identical-code pairing of by_brackets' gaps: run first in
        every round, so a call position (by_calls) never claims an address
        identical code would take."""
        return self.by_brackets(exact_only=True)

    def by_brackets(self, align=False, exact_only=False):
        per_tu = collections.defaultdict(list)
        for f in self.fns:
            per_tu[f.tu].append(f)
        pairs, exact = [], []
        okey, rkey = self.code_keys()
        for tu, lst in per_tu.items():
            lst.sort(key=lambda f: self.layout[f.key])
            anchored = [(i, self.map[f.key][0]) for i, f in enumerate(lst) if f.key in self.map]
            chain = longest_increasing(anchored)
            for (i1, s1), (i2, s2) in zip(chain, chain[1:]):
                ours_gap = [f for f in lst[i1 + 1:i2] if f.key not in self.map]
                if len(ours_gap) != i2 - i1 - 1:
                    continue                              # an out-of-order anchor sits inside
                lo, hi = bisect.bisect_right(self.starts, s1), bisect.bisect_left(self.starts, s2)
                ref_gap = self.starts[lo:hi]
                foreign = any(r in self.rmap for r in ref_gap)    # another object's code sits inside
                ref_gap = [r for r in ref_gap if r not in self.rmap]
                if not ours_gap or not ref_gap:
                    continue
                # Identical code, unique within the gap on both sides, however
                # short: pinned before anything is paired by position - also
                # where code of another of our files is mapped inside the gap
                # (the generator's file split is not the original's object
                # there), which only rules out pairing by position.
                ofree, rfree = self.free_keys()
                same = self.identical(ours_gap, ref_gap, lambda f: ofree[f.key], rfree.__getitem__)
                with_callees = not same
                if not same:
                    # The same code calling different functions (callbacks that
                    # only forward): told apart by callees already mapped.
                    same = self.identical(ours_gap, ref_gap, lambda f: (okey[f.key], self.our_callees(f)),
                                          lambda r: (rkey[r], self.ref_callees(r)))
                if foreign and len(ref_gap) > FOREIGN_GAP:
                    # A wide gap that is not one object's: its bounds say less,
                    # so the code must also be the only one of its kind overall.
                    same = [(f, r) for f, r in same if self.unique_overall(f, r, with_callees)]
                if same:
                    for f, r in same:
                        exact.append((2.0, f.key, r, 'identical code between %08x and %08x' % (s1, s2)))
                    continue
                if exact_only or foreign:
                    continue
                if len(ours_gap) == len(ref_gap) and not self.shifted(ours_gap, ref_gap):
                    for f, r in zip(ours_gap, ref_gap):
                        pairs.append((1.0, f.key, r, 'between %08x and %08x' % (s1, s2)))
                elif align:
                    for f, r, sc in self.align_gap(ours_gap, ref_gap):
                        pairs.append((sc, f.key, r, 'aligned %.2f between %08x and %08x' % (sc, s1, s2)))
        return self.assign(exact, 'exact') + self.assign(pairs, 'align' if align else 'bracket')

    def unique_overall(self, f, r, with_callees):
        """f and r hold the only code of its kind (and, with_callees, calling
        the same functions) among the unmapped functions of either side."""
        okey, rkey = self.code_keys()
        if not hasattr(self, '_by_code'):
            self._by_code = collections.defaultdict(lambda: ([], []))
            for g in self.fns:
                if okey[g.key] is not None:
                    self._by_code[okey[g.key]][0].append(g.key)
            for st in self.starts:
                if rkey[st] is not None and rkey[st] in self._by_code:
                    self._by_code[rkey[st]][1].append(st)
        ours, theirs = self._by_code.get(okey[f.key], ([], []))
        ours = [k for k in ours if k not in self.map]
        theirs = [st for st in theirs if st not in self.rmap]
        if with_callees:
            callees = self.our_callees(f)
            ours = [k for k in ours if self.our_callees(self.by_key[k]) == callees]
            theirs = [st for st in theirs if self.ref_callees(st) == callees]
        return ours == [f.key] and theirs == [r]

    @staticmethod
    def callfree(key):
        """A code key that identifies by itself: no call or function pointer
        in it (forwarders differ only in what they call) and at least three
        instructions; else None."""
        if key is None or len(key) < 3 or any(('fn',) in toks for _m, toks in key):
            return None
        return key

    def free_keys(self):
        """callfree() of every code key on both sides, computed once."""
        if not hasattr(self, '_ofree'):
            okey, rkey = self.code_keys()
            self._ofree = {k: self.callfree(v) for k, v in okey.items()}
            self._rfree = {k: self.callfree(v) for k, v in rkey.items()}
        return self._ofree, self._rfree

    @staticmethod
    def identical(ours_gap, ref_gap, okey, rkey):
        """Pairs whose keys are equal and unique within the gap on both sides,
        in gap order. Each key is computed once."""
        if not ours_gap or not ref_gap:
            return []
        ok = [(f, okey(f)) for f in ours_gap]
        rk = [(r, rkey(r)) for r in ref_gap]
        ko = collections.Counter(k for _f, k in ok)
        kr = collections.Counter(k for _r, k in rk)
        unique_r = {k: r for r, k in rk if kr[k] == 1}
        return [(f, unique_r[k]) for f, k in ok
                if k is not None and k[0] is not None and ko[k] == 1 and k in unique_r]

    def our_callees(self, f):
        """Original starts of f's callees and function pointers, in code order;
        None if one is not known yet."""
        out = []
        for _o, k, t in f.refs:
            if k in ('call', 'fptr'):
                g = self.resolve_callee(f, t)
                if g is not None and g.key in self.map:
                    out.append(self.map[g.key][0])
                elif g is None and ourslib_qualified(t) in self.crt:
                    out.append(self.crt[ourslib_qualified(t)])
                else:
                    return None
        return tuple(out)

    def ref_callees(self, start):
        return tuple(t for _a, _o, _n, k, t in sorted(self.model[start][1]) if k in ('call', 'fptr'))

    def shifted(self, ours_gap, ref_gap):
        """True if aligning the two equal gaps by code similarity beats pairing
        them in order by more than BRACKET_MARGIN: the gaps hold different
        functions, and the counts are equal by coincidence."""
        if len(ours_gap) < 2:
            return False
        diagonal = sum(self.similarity(f, r) for f, r in zip(ours_gap, ref_gap))
        best = sum(sc for _f, _r, sc in self.align_gap(ours_gap, ref_gap, threshold=0.0))
        return best > diagonal + BRACKET_MARGIN

    def similarity(self, f, start):
        memo = self.__dict__.setdefault('_sim', {})
        k = (f.key, start)
        if k not in memo:
            a = mnemonics(f.decoded)
            b = mnemonics(self.model[start][0])
            if not a or not b or min(len(a), len(b)) * 2 < max(len(a), len(b)):
                memo[k] = 0.0
            else:
                memo[k] = difflib.SequenceMatcher(None, a, b, autojunk=False).ratio()
        return memo[k]

    def align_gap(self, ours_gap, ref_gap, threshold=ALIGN_THRESHOLD):
        """Needleman-Wunsch over the two ordered gaps; a pair scores its
        similarity, a skip scores 0. Pairs below the threshold are dropped."""
        n, m = len(ours_gap), len(ref_gap)
        if n * m > 4000:
            return []
        sim = [[self.similarity(f, r) for r in ref_gap] for f in ours_gap]
        best = [[0.0] * (m + 1) for _ in range(n + 1)]
        for i in range(n - 1, -1, -1):
            for j in range(m - 1, -1, -1):
                best[i][j] = max(best[i + 1][j], best[i][j + 1], sim[i][j] + best[i + 1][j + 1])
        out, i, j = [], 0, 0
        while i < n and j < m:
            if sim[i][j] + best[i + 1][j + 1] >= best[i][j] - 1e-9 and sim[i][j] >= threshold:
                out.append((ours_gap[i], ref_gap[j], sim[i][j]))
                i, j = i + 1, j + 1
            elif best[i + 1][j] >= best[i][j + 1]:
                i += 1
            else:
                j += 1
        return out

    def call_votes(self):
        """our callee symbol -> Counter(original target) over the call sites
        that line up in every mapped pair: the two instruction streams are
        aligned by mnemonic, and a call inside an aligned stretch votes for
        the original's target. Votes from identical code (exact pairs) are
        counted apart as well."""
        import compare
        votes = collections.defaultdict(collections.Counter)
        self.exact_votes = collections.defaultdict(collections.Counter)
        memo = ourslib.PairMemo('funcmap-votes', BUILD)
        for k, (s, method, _d) in self.map.items():
            f = self.by_key[k]
            sites = memo.get(f, s)
            if sites is None:
                sites = memo.put(f, s, self.aligned_calls(f, s))
            for sym, target in sites:
                votes[sym][target] += 1
                if method == 'exact':
                    self.exact_votes[sym][target] += 1
        memo.save()
        return votes

    def aligned_calls(self, f, s):
        """(our callee symbol, original target) of every call that lines up
        between f and the original at s, the two aligned by mnemonic. A
        function of the two functions' code alone, so PairMemo keeps it."""
        import compare
        ours = compare.our_view(f, {}, render=False)
        ref = compare.ref_view(s, self.model[s], render=False)
        if len(ours) == len(ref) and all(x.mnemonic == y.mnemonic for x, y in zip(ours, ref)):
            pairs = list(zip(ours, ref))
        else:
            sm = difflib.SequenceMatcher(None, [x.mnemonic for x in ours], [y.mnemonic for y in ref],
                                         autojunk=False)
            pairs = [(ours[i], ref[j]) for op, i1, i2, j1, j2 in sm.get_opcodes() if op == 'equal'
                     for i, j in zip(range(i1, i2), range(j1, j2))]
        return [(ta[1], tb[1]) for x, y in pairs for ta, tb in zip(x.tokens, y.tokens)
                if ta[0] == 'fn?' and tb[0] == 'fn']

    def pins_from(self, votes, min_votes=2, share=0.75):
        """Callees most call sites agree on; one site is enough when it is in
        identical code (an exact pair) and no site disagrees."""
        pins = []
        self.pin_votes = {}
        exact = getattr(self, 'exact_votes', {})
        for sym, c in votes.items():
            (t, n), = c.most_common(1)
            enough = n >= min_votes or (exact.get(sym, {}).get(t, 0) >= 1 and n == sum(c.values()))
            if enough and n >= share * sum(c.values()) and t in self.model:
                g = self.by_symbol.get(sym, [])
                if len(g) == 1:
                    pins.append((g[0].key, t, '%d/%d call sites' % (n, sum(c.values()))))
                    self.pin_votes[g[0].key] = (n, sum(c.values()))
        return pins

    def overwhelming(self, k):
        """True if the pin of `k` rests on so many agreeing call sites that no
        code identity outweighs it: a source edit can make one of two sibling
        helpers compile exactly to the other's original (RANDOM_RandomNumberSelector
        to 0045c390, which 245 call sites name UNIT_GetModuloFromSeed)."""
        n, total = self.pin_votes.get(k, (0, 0))
        return n >= OVERWHELM_VOTES and n >= OVERWHELM_SHARE * total

    def by_pins(self, pins):
        return self.assign([(4.0, k, t, d) for k, t, d in pins], 'votes')

    def run(self):
        """Derive the map, vote on callees across it, then derive it again
        with the voted callees pinned first; finally drop the pairs made by
        position or shape whose callees contradict the map."""
        first = self.derive([])
        pins = self.keep_matches(self.pins_from(self.call_votes()))
        moved = [(k, t) for k, t, _d in pins if self.map.get(k, (None,))[0] != t]
        self.map, self.rmap = {}, {}
        log = [('first pass', sum(n for _t, n in first)), ('pinned by votes', len(pins)),
               ('of which moved', len(moved))]
        log += self.derive(pins)
        log.append(('dropped: callees disagree', -self.drop_contradicted()))
        log.append(('sourcemap, the rest', self.by_sourcemap_rest(os.path.dirname(os.path.dirname(ORACLE)))))
        return log

    def keep_matches(self, pins):
        """Pins, less those that would break a match of the first pass: move
        a function away from a partner its code matches (identical bytes and
        references), or move a callee a matching caller's reference depends
        on. Votes aligned by mnemonic do not outvote a MATCH, unless they are
        overwhelming (OVERWHELM_VOTES call sites, OVERWHELM_SHARE of them)."""
        import compare
        syms = {self.by_key[k].symbol: s for k, (s, _m, _d) in self.map.items()}

        def matches(f, s, table):
            return compare.compare(compare.our_view(f, table), compare.ref_view(s, self.model[s])).status == 'MATCH'
        callers = collections.defaultdict(list)
        for k, (s, _m, _d) in self.map.items():
            for _o, kind, t in self.by_key[k].refs:
                if kind in ('call', 'fptr'):
                    callers[t].append((k, s))
        out = []
        for k, t, d in pins:
            s0 = self.map.get(k, (None,))[0]
            if s0 is not None and s0 != t and not self.overwhelming(k):
                f = self.by_key[k]
                if matches(f, s0, syms) and not matches(f, t, syms):
                    continue
                moved = dict(syms)
                moved[f.symbol] = t
                if any(matches(self.by_key[g], sg, syms) and not matches(self.by_key[g], sg, moved)
                       for g, sg in callers.get(f.symbol, ())):
                    continue
            out.append((k, t, d))
        return out

    def drop_contradicted(self):
        """A pair made by position, code similarity or code shape (bracket,
        align, exact with calls in it, located) is wrong if the functions it calls,
        all known and as many as the original's, are not the ones the
        original calls: drop it. Forwarders of one shape call different
        functions (thunk_MISSILESMODE_SrvDoFn01_BasicMissile against
        006439f0, which forwards to D2Common's GetSkill)."""
        okey, _rkey = self.code_keys()
        bad = []
        for k, (s, method, _d) in self.map.items():
            if method not in ('bracket', 'align', 'exact', 'located'):
                continue
            if method == 'exact' and self.free_keys()[0][k] is not None:
                continue                  # identical code without calls: nothing to contradict it
            ours = self.our_callees(self.by_key[k])
            theirs = self.ref_callees(s)
            if ours is not None and len(ours) == len(theirs) and sorted(ours) != sorted(theirs):
                bad.append(k)
        for k in bad:
            del self.rmap[self.map.pop(k)[0]]
        return len(bad)

    def derive(self, pins):
        log = []
        # Ground truth first, then the voted callees, then everything inferred.
        log.append(('sourcemap', self.by_sourcemaps(os.path.dirname(os.path.dirname(ORACLE)))))
        log.append(('name', self.by_names()))
        log.append(('export', self.by_exports()))
        # Overwhelming votes come before identical code (overwhelming()).
        strong = [p for p in pins if self.overwhelming(p[0])]
        if strong:
            log.append(('votes', self.by_pins(strong)))
        log.append(('exact', self.by_exact()))
        log.append(('located', self.by_located()))
        if pins:
            log.append(('votes', self.by_pins(pins)))
        log.append(('string', self.by_strings()))
        log.append(('import', self.by_imports()))
        while True:
            e = self.by_gap_exact()
            a = self.by_calls()
            b = self.by_brackets()
            log.append(('calls+bracket', e + a + b))
            if not e + a + b:
                break
        while True:
            r = self.by_refs()
            e = self.by_gap_exact()
            a = self.by_calls()
            b = self.by_brackets()
            log.append(('refs+calls+bracket', r + e + a + b))
            if not r + e + a + b:
                break
        while True:
            e = self.by_gap_exact()
            c = self.by_brackets(align=True)
            r = self.by_refs()
            a = self.by_calls()
            b = self.by_brackets()
            log.append(('align+refs+calls+bracket', e + a + b + c + r))
            if not e + a + b + c + r:
                break
        return log


def ourslib_qualified(symbol):
    import objfuncs
    return objfuncs.qualified(symbol)


def load_crtmap(path):
    """Unambiguous CRT names from crtmap.tsv -> original start."""
    out = {}
    if os.path.exists(path):
        for line in open(path):
            if line.startswith('start\t'):
                continue
            s, _e, name, _which = line.rstrip('\n').split('\t')
            if not name.startswith('?'):
                out[name] = int(s, 16)
    return out


def refimage_qualified(mangled):
    import objfuncs
    return objfuncs.qualified(mangled)


def longest_increasing(pairs):
    """Longest subsequence of (index, start) pairs whose starts increase."""
    if not pairs:
        return []
    tails, prev, idx = [], [None] * len(pairs), []
    for k, (_i, s) in enumerate(pairs):
        pos = bisect.bisect_left([pairs[t][1] for t in idx], s)
        if pos == len(idx):
            idx.append(k)
        else:
            idx[pos] = k
        prev[k] = idx[pos - 1] if pos > 0 else None
        tails = idx
    out, k = [], tails[-1]
    while k is not None:
        out.append(pairs[k])
        k = prev[k]
    return out[::-1]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--out', default=os.path.join(ORACLE, 'evidence', 'funcmap.tsv'))
    args = ap.parse_args()
    if EXPERIMENT and os.path.realpath(os.path.dirname(args.out)) == os.path.realpath(os.path.join(ORACLE, 'evidence')):
        sys.exit('funcmap.py: not in an experiment - an A/B audit uses the baseline map (evidence/funcmap.tsv)')
    import objcache
    objdir = os.path.join(BUILD, 'obj')
    objs = ourslib.object_keys(objdir)
    root = os.path.dirname(os.path.dirname(ORACLE))
    crtmap = os.path.join(ORACLE, 'evidence', 'crtmap.tsv')
    # Everything the map is derived from: our objects, the generator's source
    # maps beside their sources, the CRT map (and the reference, the scripts).
    key = ourslib.stage_key('funcmap', repr(objs), objcache.sha_file(crtmap),
                            repr([objcache.sha_file(os.path.join(root, rel[:-4] + '.cpp.map')) for rel, _h in objs]))
    outputs = [args.out, args.out + '.unmapped']
    if ourslib.stage_reuse('funcmap', BUILD, key, outputs):
        print('funcmap: inputs unchanged, %s kept' % args.out)
        return
    ref = refimage.Reference(os.path.join(ORACLE, 'reference', 'Game.exe'))
    ref.load(os.path.join(REFCACHE, 'functions.tsv'))
    model = ref.model(os.path.join(REFCACHE, 'model.pkl'))
    fns = ourslib.load_all(objdir)
    m = Matcher(ref, model, fns, load_crtmap(os.path.join(ORACLE, 'evidence', 'crtmap.tsv')))
    for tier, n in m.run():
        print('  %-22s +%d' % (tier, n))
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    rows = []
    for k, (s, method, detail) in m.map.items():
        f = m.by_key[k]
        end = model[s][0].end
        rows.append((s, end, f.name, f.tu, f.symbol, method, detail))
    rows.sort()
    with open(args.out, 'w') as fh:
        fh.write('start\tend\tname\ttu\tsymbol\tmethod\tdetail\n')
        for r in rows:
            fh.write('%08x\t%08x\t%s\t%s\t%s\t%s\t%s\n' % r)
    with open(args.out + '.unmapped', 'w') as fh:
        for f in sorted(fns, key=lambda f: (f.tu, f.order)):
            if f.key not in m.map:
                fh.write('%s\t%s\t%d insns\n' % (f.tu, f.name, f.insns))
    ourslib.stage_done('funcmap', BUILD, key, outputs)
    by = collections.Counter(r[5] for r in rows)
    print('mapped %d of %d compiled functions (%d original functions): %s' % (
        len(rows), len(fns), len(model), ', '.join('%s %d' % kv for kv in by.most_common())))


if __name__ == '__main__':
    main()
