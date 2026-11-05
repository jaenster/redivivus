#!/usr/bin/env python3
"""vc8_translate.py - mirror the source tree into a form Visual C++ 2005 compiles.

    vc8_translate.py <repo-root> <out-dir>

Writes every game header and source file under <out-dir>, rewritten where the
C++17 spelling has no VC8 equivalent. Rewrites are LINE-PRESERVING: no line is
added or removed, so __LINE__ (which assert sites embed) and every compiler
diagnostic map 1:1 back to the real file. Unchanged inputs are not rewritten,
so their mtimes stay put and the compile step can skip them.

What is rewritten, and why each is exact:

  namespace A::B {          ->  namespace A { namespace B {
  } // namespace A::B       ->  }} // namespace A::B
      C++17 nested-namespace definitions; identical meaning.
  struct __attribute__((packed)) S { ... };
                            ->  __pragma(pack(push, 1)) struct S { ... }; __pragma(pack(pop))
      GCC's packed attribute is MSVC's pack(1) around the definition.
  T() = default;            ->  (blanked), or T() {} if T has other constructors
      A defaulted default constructor keeps a C++11 struct an aggregate;
      dropping it does the same in C++03. Only a struct with other
      constructors needs it spelled, and is not an aggregate either way.
  x = {};                   ->  memset(&(x), 0, sizeof(x));
      Assigning an empty braced list zero-initialises a POD; VC8 needs it
      spelled, and the translator knows no types to write `x = T()`.
  T x = {};                 ->  T x = {0};    (after the constructor rewrite below)
      An empty braced initialiser zero-initialises; C++03 accepts it only
      with one element, for a scalar and a plain aggregate alike.
  T x{args};                ->  T x(args);
      Direct-list-initialization of a declared object; same constructor call.
  template<> T C<A>::m;     ->  (blanked, in headers)
      Declaring an explicit specialization of a static data member: VC8 takes
      it for a definition and then rejects the real one as a redefinition.
  0b1010                    ->  0xa
  [[attribute]]             ->  (removed; no codegen meaning)
  __attribute__((noinline)) ->  __declspec(noinline)
  extern "C" __stdcall T f( ->  extern "C" T __stdcall f(
      GCC takes a calling convention before the return type; MSVC only after.
  T __thiscall f(A a, B b)  ->  T __fastcall f(A a, int, B b)
  T (__thiscall *p)(A, B)   ->  T (__fastcall *p)(A, int, B)
  f(a, b)  /  x->p(a, b)    ->  vc8_thiscall(f)(a, b)  /  vc8_thiscall(x->p)(a, b)
      The tree spells __thiscall on free functions that take the receiver as
      their first parameter, and on the pointer types that hold them; gcc
      passes it in ECX, VC8 accepts the keyword only on member functions
      (C3865). __fastcall with an unused int second parameter is the same
      callee: receiver in ECX, the rest on the stack, popped by the callee.
      A call casts the callee back to the __thiscall pointer type, which VC8
      does accept (vc8_thiscall in vc8_compat.h: a plain cast, its type
      deduced by the compiler), so the caller loads no EDX for the unused
      int; the cast folds into a direct `call f`. A callee
      expression the walk back cannot delimit gets the unused int (`f(a, 0,
      b)`, one `xor edx, edx`) instead, and so does a call to a function
      declared static or inline (file_facts): through the cast VC8
      would neither inline it nor give a static one its own register
      convention, and for those the unused int costs nothing.
      (VC8 SP1 does take __thiscall on a static member function, and a
      caller compiles the same as through the cast; but it would need a
      class around every function, declared once across the headers, and
      would change every symbol, so the definitions stay __fastcall.)
      Calls are rewritten by name (merge_facts); a name also declared
      without __thiscall with the same parameter count is left stripped, as
      __thiscall was before: compiled as __cdecl.
  constructors of a struct that is a union member  ->  (blanked)
      C++03 forbids them (C2620). The generator adds converting constructors
      to small by-value structs as decompilation conveniences; the original's
      C-style structs had none.
  T x[] = {{a, b}, {}}      ->  T x[] = {T(a, b), T()}     (for a T with constructors)
      C++11 list-initialisation of such a struct is a constructor call; C++03
      needs it spelled. Function code is unchanged; a table so initialised is
      filled at startup instead of being static data, which the per-function
      comparison does not see.
  Both sets come from a whole-tree pre-pass (file_facts, merge_facts).

Everything the tree spells that VC8 lacks but a macro can express lives in
../vc8_compat.h instead (nullptr, constexpr, static_assert, thread_local).

A function the compiler cannot build at all (an internal compiler error, a
GCC extension such as arithmetic on void*) is QUARANTINED rather than losing
its whole translation unit: build.py records it in build/quarantine.tsv, and
its body is blanked here, keeping its lines. The audit reports it as such.

FILE_PATCHES holds the few edits tied to one generated file, all of them in
the platform header's toolchain glue, never in game code. Each anchor must
match exactly once, so a regenerated header that moved on fails here loudly
instead of compiling unpatched. Only a header without __LINE__ may take a patch
that adds a line.
The vendored thirdparty/ and the build and tooling directories are not part
of the reconstruction and are not mirrored; of the hand-written support/ tree
only the headers are, because some game sources include them.
"""
import hashlib
import os
import re
import sys

SKIP_DIRS = {'.git', '.github', '.idea', 'asm_matching', 'build', 'cmake',
             'thirdparty', 'tools', 'metrics'}
HEADERS_ONLY = {'support'}   # hand-written scaffolding: its headers are included, its code is not ours to diff
EXTS = ('.h', '.hpp', '.inl', '.cpp', '.c')

NS_OPEN = re.compile(r'^(\s*)namespace\s+([A-Za-z_]\w*(?:::[A-Za-z_]\w*)+)\s*\{', re.M)
NS_CLOSE = re.compile(r'^(\s*)\}(\s*//\s*namespace\s+([A-Za-z_]\w*(?:::[A-Za-z_]\w*)+)\b)', re.M)
PACKED = re.compile(r'\b(struct|union|class)\s+__attribute__\s*\(\(\s*packed\s*\)\)\s*')
BRACE_INIT = re.compile(r'^([ \t]*)((?!(?:struct|class|union|enum|namespace|typedef|return|else|do)\b)'
                        r'[A-Za-z_][\w:]*(?:<[^;{}()]*>)?[\s*&]+[A-Za-z_]\w*)\{([^{};]*)\};', re.M)
EMPTY_INIT = re.compile(r'(=\s*)\{\s*\}(\s*;)')
ZERO_ASSIGN = re.compile(r'^([ \t]*)([A-Za-z_][\w.\[\]]*(?:->[\w.\[\]]+)*)\s*=\s*\{\s*\};', re.M)
TEMPLATE_STATIC_DECL = re.compile(r'^template\s*<>\s*[^;=(){}]*::\w+\s*(?:\[[^\]]*\])?\s*;[ \t]*$', re.M)
BINARY = re.compile(r'\b0[bB]([01]+)([uUlL]*)\b')
CXX_ATTR = re.compile(r'\[\[\s*[A-Za-z_][\w:]*(?:\([^)]*\))?\s*\]\]\s*')
THISCALL = re.compile(r'\b__thiscall\b[ \t]*')
QUAL = r'(?:[A-Za-z_]\w*(?:<[^;{}()<>]*>)?::)*'
THISCALL_FN = re.compile(r'\b__thiscall(\s+' + QUAL + r'([A-Za-z_]\w*)\s*)\(')
THISCALL_PTR = re.compile(r'\(\s*__thiscall(\s*\*\s*([A-Za-z_]\w*)?\s*\)\s*)\(')
OTHER_FN = re.compile(r'^[ \t]*(?:template\s*<>\s*)?(?!(?:return|else|new|delete|case|throw|goto|sizeof)\b)'
                      r'(?:::)?[A-Za-z_][\w:<>, ]*?[\s*&]+(?:__(?:cdecl|stdcall|fastcall)\s+)?' + QUAL +
                      r'([A-Za-z_]\w*)\s*\(', re.M)
OTHER_PTR = re.compile(r'[\w*&>][ \t]*\(\s*(?:__(?:cdecl|stdcall|fastcall)\s*)?\*\s*([A-Za-z_]\w*)\s*\)\s*\(')
THISCALL_TYPEDEF = re.compile(r'\btypedef\b[^;]*?\(\s*__thiscall\s*\*\s*([A-Za-z_]\w*)\s*\)\s*\(')
NOINLINE = re.compile(r'__attribute__\s*\(\(\s*noinline\s*\)\)')
CONV_FIRST = re.compile(r'\b(__stdcall|__cdecl|__fastcall|__thiscall)\s+'
                        r'((?:const\s+|struct\s+|unsigned\s+|signed\s+)*[A-Za-z_][\w:]*(?:\s*\*+)?)\s+'
                        r'([A-Za-z_]\w*)\s*\(')

FILE_PATCHES = {
    'd2_platform.h': [
        # The hook that runs right after the Windows headers: see vc8_after_windows.h.
        (r'^#  include <wintrust\.h>$', '#  include <wintrust.h>\n#  include <vc8_after_windows.h>'),
        # mingw lacks the MSVC names and maps them to C99's; VC8 has them, and the
        # reverse mapping in vc8_after_windows.h would make these loop.
        (r'^#define __snprintf snprintf$', '#define __snprintf _snprintf'),
        (r'^#define _snprintf snprintf$', '/* VC8 has _snprintf */'),
        (r'^#define _vsnprintf vsnprintf$', '/* VC8 has _vsnprintf */'),
        # VC8 SP1's back end crashes (C1001, P2 main.c line 182) inlining
        # memcpy(p, &v, 3) when v is the by-value parameter and the call passes
        # a computed value; a local copy avoids it and compiles to the same
        # word + byte stores.
        (r'^static inline void d2_store24\(void\* p, uint32_t v\) \{ std::memcpy\(p, &v, 3\); \}$',
         'static inline void d2_store24(void* p, uint32_t v) { uint32_t t = v; std::memcpy(p, &t, 3); }'),
        (r'^#define __stricmp strcasecmp$', '#define __stricmp _stricmp'),
        (r'^#define _stricmp strcasecmp$', '/* VC8 has _stricmp */'),
        (r'^#define _strnicmp strncasecmp$', '/* VC8 has _strnicmp */'),
    ],
}


# Game.exe calls binkw32, smackw32 and ijl11 through the IAT
# (`call [__imp__SmackClose@4]`): their own headers declare them dllimport, and
# d2_platform.h (written for mingw) does not.
RAD_IJL_IMPORTS = ('SmackOpen', 'SmackClose', 'SmackDoFrame', 'SmackNextFrame', 'SmackWait',
                   'SmackToBuffer', 'BinkOpen', 'BinkClose', 'BinkDoFrame', 'BinkNextFrame',
                   'BinkWait', 'BinkCopyToBuffer', 'BinkSetSoundSystem', 'BinkOpenDirectSound',
                   'BinkDDSurfaceType', 'ijlInit', 'ijlFree', 'ijlWrite')
FILE_PATCHES['d2_platform.h'] += [
    (r'^extern "C" (?=[^;(]* __stdcall %s\()' % name, 'extern "C" __declspec(dllimport) ')
    for name in RAD_IJL_IMPORTS]

def apply_patches(rel, text):
    for pattern, repl in FILE_PATCHES.get(rel, ()):
        rx = re.compile(pattern, re.M)
        hits = len(rx.findall(text))
        if hits != 1:
            raise ValueError('patch anchor %r matches %d times, expected once' % (pattern, hits))
        text = rx.sub(lambda m: repl, text)
    return text


def _skip_literal(s, i):
    """Index just past the string/char literal or comment starting at s[i],
    or None if s[i] starts none of them."""
    c = s[i]
    if c == '/' and s.startswith('//', i):
        j = s.find('\n', i)
        return len(s) if j < 0 else j
    if c == '/' and s.startswith('/*', i):
        j = s.find('*/', i + 2)
        return len(s) if j < 0 else j + 2
    if c in '"\'':
        j = i + 1
        while j < len(s) and s[j] != c:
            j += 2 if s[j] == '\\' else 1
        return j + 1
    return None


def _matching_brace(s, open_idx):
    depth = 0
    i = open_idx
    while i < len(s):
        k = _skip_literal(s, i)
        if k is not None:
            i = k
            continue
        if s[i] == '{':
            depth += 1
        elif s[i] == '}':
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def rewrite_packed(s):
    out = []
    pos = 0
    for m in PACKED.finditer(s):
        if m.start() < pos:
            continue
        brace = s.find('{', m.end())
        semi = s.find(';', m.end())
        if brace < 0 or (0 <= semi < brace):          # a declaration, not a definition
            out.append(s[pos:m.start()] + m.group(1) + ' ')
            pos = m.end()
            continue
        close = _matching_brace(s, brace)
        end = s.find(';', close)
        if close < 0 or end < 0:
            raise ValueError('unbalanced packed struct at offset %d' % m.start())
        out.append(s[pos:m.start()] + '__pragma(pack(push, 1)) ' + m.group(1) + ' ')
        out.append(s[m.end():end + 1] + ' __pragma(pack(pop))')
        pos = end + 1
    out.append(s[pos:])
    return ''.join(out)


STRUCT_HEAD = re.compile(r'\b(struct|class)\s+(?:__attribute__\s*\(\(\s*packed\s*\)\)\s+)?(\w+)\s*(?::[^{;]*)?\{')
UNION_HEAD = re.compile(r'\bunion\s+(\w+)\s*\{')
MEMBER = re.compile(r'^\s*(?:/\*[^*]*\*/\s*)?(?:struct\s+)?(\w+)\s+\w+\s*(?:\[[^\]]*\])?\s*;', re.M)


def _bodies(s, head):
    for m in head.finditer(s):
        close = _matching_brace(s, m.end() - 1)
        if close > 0:
            yield m, m.end(), close


LIST_INIT = re.compile(r'^[ \t]*(?:static\s+|const\s+|extern\s+)*(?:struct\s+)?([A-Za-z_]\w*)\s+[A-Za-z_][\w:]*'
                       r'\s*(?:\[[^\]]*\]\s*)*=\s*\{', re.M)


def without_non_win32(s):
    """`s` with every `#ifndef _WIN32` block blanked (lines kept): the
    generator's stand-ins for SDK types (tagPALETTEENTRY, ...), which VC8 never
    compiles - it has the SDK's own, without constructors."""
    out, depth = [], 0
    for line in s.split('\n'):
        t = line.strip()
        if depth:
            if re.match(r'#\s*(?:if|ifdef|ifndef)\b', t):
                depth += 1
            elif re.match(r'#\s*endif\b', t):
                depth -= 1
            elif depth == 1 and re.match(r'#\s*(?:else|elif)\b', t):
                depth = 0                         # the _WIN32 branch is compiled
            out.append('')
            continue
        if re.match(r'#\s*(?:ifndef\s+_WIN32|if\s+!\s*defined\s*\(?\s*_WIN32\s*\)?)\b', t):
            depth = 1
            out.append('')
            continue
        out.append(line)
    return '\n'.join(out)


LIST_INIT_DECL = re.compile(r'^[ \t]*(?:static\s+|const\s+|extern\s+)*(?:struct\s+)?([A-Za-z_]\w*)\s+[A-Za-z_][\w:]*'
                            r'\s*((?:\[[^\]]*\]\s*)*)=\s*\{', re.M)


def rewrite_list_inits(s, types):
    """`T x = {a, b};` -> `T x = T(a, b);`, `T x[] = {{a, b}, {}}` -> `{T(a, b), T()}`
    for the named structs: the braces at the struct's own depth (one deeper
    than the declarator's array rank) become a call to its field-wise
    constructor. A struct brace holding braces of its own is left alone."""
    out, pos = [], 0
    for m in LIST_INIT_DECL.finditer(s):
        if m.group(1) not in types or m.start() < pos:
            continue
        rank = m.group(2).count('[')
        open_at = m.end() - 1
        close_at = _matching_brace(s, open_at)
        if close_at < 0:
            continue
        T = m.group(1)
        edits = []               # (index, replacement) of single characters
        depth, i = 0, open_at
        while i <= close_at:
            k = _skip_literal(s, i)
            if k is not None:
                i = k
                continue
            c = s[i]
            if c == '{':
                depth += 1
                if depth == rank + 1:
                    end = _matching_brace(s, i)
                    inner = s[i + 1:end]
                    if '{' not in inner:
                        edits.append((i, T + '('))
                        edits.append((end, ')'))
                        depth -= 1
                        i = end + 1
                        continue
            elif c == '}':
                depth -= 1
            i += 1
        if rank == 0 and edits:
            # The struct itself: `= {..}` -> `= T(..)` (already in edits).
            pass
        out.append(s[pos:open_at])
        seg = list(s[open_at:close_at + 1])
        for idx, rep in sorted(edits, reverse=True):
            seg[idx - open_at] = rep
        out.append(''.join(seg))
        pos = close_at + 1
    out.append(s[pos:])
    return ''.join(out)


def _arg_commas(s, open_idx, angle=False):
    """(top-level comma indices, index of the closing paren) of the
    parenthesised list at s[open_idx]; with `angle`, <> nest too (a
    parameter list, where they are template arguments)."""
    commas, depth, i = [], 0, open_idx
    opens, closes = ('([{<', ')]}>') if angle else ('([{', ')]}')
    while i < len(s):
        k = _skip_literal(s, i)
        if k is not None:
            i = k
            continue
        c = s[i]
        if c in opens:
            depth += 1
        elif c in closes and not (c == '>' and s[i - 1] == '-'):
            depth -= 1
            if depth == 0:
                return commas, i
        elif c == ',' and depth == 1:
            commas.append(i)
        i += 1
    return commas, -1


def _arg_count(s, open_idx, angle=False):
    commas, close = _arg_commas(s, open_idx, angle)
    if close < 0:
        return -1
    inner = s[open_idx + 1:close].strip()
    return 0 if inner in ('', 'void') else len(commas) + 1


def file_facts(text, header):
    """What the whole-tree pre-pass needs from one file, in the order the file
    holds it. A function of the file's text alone, so it can be kept across
    runs per content (mirror's `facts` cache) and merged (merge_facts).
      fn, ptr, typedef  the __thiscall functions, pointer fields and pointer
                        typedefs, with their parameter counts
      other_fn, other_ptr  the same names declared otherwise
      direct            __thiscall functions declared static or inline
      ctor, union       (headers) structs that declare a constructor, and the
                        types of union members
      list_init         types of list-initialised declarations
    Definitions VC8 does not compile (`#ifndef _WIN32`) do not count; the
    list initialisations of a non-header are taken from its full text."""
    s = without_non_win32(text)
    f = {'fn': [], 'ptr': [], 'typedef': [], 'other_fn': [], 'other_ptr': [], 'direct': []}
    for m in THISCALL_FN.finditer(s):
        f['fn'].append((m.group(2), _arg_count(s, m.end() - 1, True)))
        head = s[max(s.rfind('\n', 0, m.start()), s.rfind(';', 0, m.start()), s.rfind('}', 0, m.start())) + 1:m.start()]
        if re.search(r'\b(?:static|inline|__inline|__forceinline)\b', head):
            f['direct'].append(m.group(2))
    for m in THISCALL_PTR.finditer(s):
        n = _arg_count(s, m.end() - 1, True)
        if m.group(2):
            f['ptr'].append((m.group(2), n))
    for m in THISCALL_TYPEDEF.finditer(s):
        f['typedef'].append((m.group(1), _arg_count(s, s.index('(', m.end() - 1), True)))
    for m in OTHER_FN.finditer(s):
        if '__thiscall' not in m.group(0):
            f['other_fn'].append((m.group(1), _arg_count(s, m.end() - 1, True)))
    for m in OTHER_PTR.finditer(s):
        f['other_ptr'].append((m.group(1), _arg_count(s, m.end() - 1, True)))
    f['ctor'], f['union'] = [], []
    if header:
        for m, a, b in _bodies(s, STRUCT_HEAD):
            if re.search(r'^\s*' + re.escape(m.group(2)) + r'\s*\(', s[a:b], re.M):
                f['ctor'].append(m.group(2))
        for m, a, b in _bodies(s, UNION_HEAD):
            f['union'] += [mm.group(1) for mm in MEMBER.finditer(s[a:b])]
    f['list_init'] = [m.group(1) for m in LIST_INIT.finditer(s if header else text)]
    f['fields'] = None              # (typedef names, [(typedef, field)]): merge_facts fills it
    return f


def merge_facts(facts, stripped_text):
    """The whole-tree pre-pass from every file's file_facts, in file order:
    (thiscall functions, thiscall pointers, thiscall direct, strip, construct).
      thiscall functions/pointers  name -> parameter counts of the functions
          and the pointer fields declared __thiscall (a field typed by a
          __thiscall pointer typedef too), less any count the same name also
          has declared otherwise - a call to that one cannot be told apart
      strip, construct  structs that declare a constructor and are used where
          C++03 wants an aggregate. A union member must lose its constructors
          (C2620); a struct that is only list-initialised keeps them, and its
          braced initialisers become constructor calls instead (C2552).
    stripped_text(k) -> file k's text without its non-_WIN32 blocks, asked
    only for a file whose typedef-typed fields are not known yet."""
    funcs, ptrs, other_f, other_p, typedefs = {}, {}, {}, {}, {}
    direct, with_ctor, in_union, list_init = set(), set(), set(), set()
    for f in facts.values():
        for n, c in f['fn']:
            funcs.setdefault(n, set()).add(c)
        for n, c in f['ptr']:
            ptrs.setdefault(n, set()).add(c)
        for n, c in f['typedef']:
            typedefs[n] = c
        for n, c in f['other_fn']:
            other_f.setdefault(n, set()).add(c)
        for n, c in f['other_ptr']:
            other_p.setdefault(n, set()).add(c)
        direct.update(f['direct'])
        with_ctor.update(f['ctor'])
        in_union.update(f['union'])
        list_init.update(f['list_init'])
    if typedefs:
        names = tuple(sorted(typedefs))
        field = re.compile(r'\b(' + '|'.join(map(re.escape, typedefs)) + r')\s+([A-Za-z_]\w*)\s*;')
        for k, f in facts.items():
            if f['fields'] is None or f['fields'][0] != names:
                f['fields'] = (names, [(m.group(1), m.group(2)) for m in field.finditer(stripped_text(k))])
            for t, n in f['fields'][1]:
                ptrs.setdefault(n, set()).add(typedefs[t])
    funcs = {k: v - other_f.get(k, set()) - {0, -1} for k, v in funcs.items()}
    ptrs = {k: v - other_p.get(k, set()) - other_f.get(k, set()) - {0, -1} for k, v in ptrs.items()}
    strip = with_ctor & in_union
    return ({k: v for k, v in funcs.items() if v}, {k: v for k, v in ptrs.items() if v}, direct,
            strip, (with_ctor & list_init) - strip)


def _insert_second(s, open_idx, text, counts, angle):
    """s with `text` added as the second item of the list at s[open_idx] when
    its item count is in `counts`, else None."""
    commas, close = _arg_commas(s, open_idx, angle)
    if close < 0 or _arg_count(s, open_idx, angle) not in counts:
        return None
    at = commas[0] if commas else close
    return s[:at] + ', ' + text + s[at:]


WORD = re.compile(r'\w+')


def rewrite_thiscall(s):
    """__thiscall declarations, pointer types and the calls through them as
    __fastcall with an unused int second parameter (see the docstring)."""
    for rx, names in ((THISCALL_FN, THISCALL_FUNCS), (THISCALL_PTR, THISCALL_PTRS)):
        pos = 0
        while True:
            m = rx.search(s, pos)
            if not m:
                break
            name = m.group(2)
            new = _insert_second(s, m.end() - 1, 'int', names.get(name, ()), True) if name else None
            if new is None and rx is THISCALL_PTR and not name:   # a cast or an unnamed parameter
                new = _insert_second(s, m.end() - 1, 'int', range(1, 64), True)
            if new is None:
                s = s[:m.start()] + s[m.start():m.end()].replace('__thiscall', '', 1).lstrip() + s[m.end():] \
                    if rx is THISCALL_FN else s[:m.start()] + '(' + m.group(1) + '(' + s[m.end():]
                pos = m.start() + 1
                continue
            s = new[:m.start()] + new[m.start():].replace('__thiscall', '__fastcall', 1)
            pos = m.end()
    # Only the names the text holds as a whole word go into the patterns:
    # each pattern matches a name only between non-word characters, and the
    # rewrites below insert no name of the tree (only vc8_thiscall, 0 and
    # int). A pattern of every name in the tree is several times slower.
    words = set(WORD.findall(s))
    funcs = [n for n in THISCALL_FUNCS if n in words]
    ptrs = [n for n in THISCALL_PTRS if n in words]
    calls = [(r'(?<![\w.>])(' + '|'.join(map(re.escape, funcs)) + r')\s*\(', THISCALL_FUNCS)] if funcs else []
    if ptrs:
        ptrs = '|'.join(map(re.escape, ptrs))
        calls.append((r'(?:->|\.)\s*(' + ptrs + r')\s*\(', THISCALL_PTRS))
        calls.append((r'\(\s*\*\s*(' + ptrs + r')\s*\)\s*\(', THISCALL_PTRS))    # (*pfn)(...), pfn a local
    for pat, names in calls:
        rx, pos = re.compile(pat), 0
        while True:
            m = rx.search(s, pos)
            if not m:
                break
            pos = m.end()
            head = re.sub(r'//[^\n]*', '', s[max(0, m.start() - 200):m.start()])   # a line comment is not a type
            head = re.sub(r'(?:::)?' + QUAL + r'$', '', head) if names is THISCALL_FUNCS else ''
            if re.search(r'(?:[\w>]\s*[*&]*|(?<!&)[*&])\s*$', head) and \
                    not re.search(r'\b(?:return|else|case|throw)\s*$', head):
                continue                  # a declaration or definition, not a call
            if _arg_count(s, m.end() - 1) not in names[m.group(1)]:
                continue
            if names is THISCALL_FUNCS and m.group(1) in THISCALL_DIRECT:
                start = None
            elif names is THISCALL_FUNCS:
                start = m.start(1) - len(re.search(r'(?:::)?' + QUAL + r'$', s[max(0, m.start(1) - 200):m.start(1)]).group(0))
            elif m.group(0).startswith('('):
                start = m.start()
            else:
                start = _postfix_start(s, m.start(1))
            if start is None:             # a direct call, or a callee not walked back: pass the unused int
                s = _insert_second(s, m.end() - 1, '0', names[m.group(1)], False)
                continue
            args = s.rindex('(', 0, m.end())
            lead = ' ' if start and (s[start - 1].isalnum() or s[start - 1] == '_') else ''
            head = s[:start] + lead + 'vc8_thiscall(' + s[start:args] + ')'
            s, pos = head + s[args:], len(head) + 1
    return s


def _back_ws(s, i):
    while i > 0 and s[i - 1] in ' \t\n':
        i -= 1
    return i


def _back_match(s, close_idx):
    """Index of the bracket that opens the one at s[close_idx], walking back
    over string and char literals; -1 if there is none."""
    pairs = {')': '(', ']': '['}
    stack, i = [], close_idx
    while i >= 0:
        c = s[i]
        if c in '"\'':
            i -= 1
            while i >= 0 and not (s[i] == c and (i == 0 or s[i - 1] != '\\')):
                i -= 1
        elif c in pairs:
            stack.append(pairs[c])
        elif c in '([':
            if not stack or stack.pop() != c:
                return -1
            if not stack:
                return i
        elif c in ';{}':
            return -1
        i -= 1
    return -1


def _postfix_start(s, i):
    """Where the postfix expression begins whose member name, reached through
    -> or ., starts at s[i] (`a[k].b->p` for p): back over member accesses,
    subscripts, calls and identifiers to a parenthesised primary or the first
    name. None for a shape it does not know."""
    while True:
        j = _back_ws(s, i)
        if s[j - 2:j] in ('->', '::'):
            j -= 2
        elif s[j - 1:j] == '.':
            j -= 1
        else:
            return i
        j = _back_ws(s, j)
        while j > 0 and s[j - 1] in ')]':
            closer, k = s[j - 1], _back_match(s, j - 1)
            if k < 0:
                return None
            j = _back_ws(s, k)
            if closer == ')' and not (j > 0 and (s[j - 1].isalnum() or s[j - 1] in '_)]')):
                j = k                     # a parenthesised primary
                break
        else:
            k = j
            while k > 0 and (s[k - 1].isalnum() or s[k - 1] == '_'):
                k -= 1
            if k == j or s[k].isdigit():
                return None
            j = k
        i = j


def strip_ctors(s, names):
    """Blank every constructor of the named structs, keeping the line count."""
    out, pos = [], 0
    for m, a, b in _bodies(s, STRUCT_HEAD):
        if m.group(2) not in names or a < pos:
            continue
        body = s[a:b]
        ctor = re.compile(r'^[ \t]*' + re.escape(m.group(2)) + r'\s*\(', re.M)
        parts, p = [], 0
        for c in ctor.finditer(body):
            if c.start() < p:
                continue
            semi = body.find(';', c.end())
            brace = body.find('{', c.end())
            end = (semi + 1) if brace < 0 or (0 <= semi < brace) else _matching_brace(body, brace) + 1
            parts.append(body[p:c.start()])
            parts.append('\n' * body.count('\n', c.start(), end))
            p = end
        parts.append(body[p:])
        out.append(s[pos:a] + ''.join(parts))
        pos = b
    out.append(s[pos:])
    return ''.join(out)


STRIP_CTORS = set()   # filled by mirror()'s pre-pass
THISCALL_FUNCS = {}   # likewise: __thiscall function name -> its parameter counts
THISCALL_PTRS = {}    # likewise: __thiscall pointer field name -> its parameter counts
THISCALL_DIRECT = set()   # likewise: __thiscall functions declared static or inline
CONSTRUCT = set()     # likewise: list-initialised structs whose braces become constructor calls


def rewrite_defaulted(s):
    """`T() = default;` -> blank when T declares no other constructor, else `T() {}`."""
    if 'default' not in s:
        return s
    out, pos = [], 0
    for m, a, b in _bodies(s, STRUCT_HEAD):
        if a < pos:
            continue
        body = s[a:b]
        name = re.escape(m.group(2))
        dflt = re.compile(r'^([ \t]*)' + name + r'\s*\(\s*\)\s*=\s*default\s*;', re.M)
        if not dflt.search(body):
            continue
        others = [c for c in re.finditer(r'^[ \t]*' + name + r'\s*\(', body, re.M) if not dflt.match(body, c.start())]
        repl = (lambda mm: mm.group(0)[:mm.group(0).index('=')].rstrip() + ' {}') if others else (lambda mm: mm.group(1))
        out.append(s[pos:a] + dflt.sub(repl, body))
        pos = b
    out.append(s[pos:])
    return ''.join(out)


FUNC_HEAD_BAD = re.compile(r'^\s*(?:namespace|struct|class|union|enum|if|for|while|switch|do|else|try|catch|__try|__except|__finally)\b')


def function_body_at(s, line):
    """(open, close) offsets of the outermost function body enclosing 1-based
    `line`, or None. A block is a function body when the statement before its
    brace ends in a parameter list and is not a control statement, a type or
    an initialiser."""
    target = sum(len(l) + 1 for l in s.split('\n')[:line - 1])
    stack = []
    i = 0
    while i < len(s):
        k = _skip_literal(s, i)
        if k is not None:
            if i <= target < k and stack:
                break
            i = k
            continue
        c = s[i]
        if c == '{':
            stack.append(i)
        elif c == '}':
            if stack:
                o = stack.pop()
                if o <= target <= i:
                    pass
        if i >= target:
            break
        i += 1
    for o in stack:                       # outermost first
        head_start = max(s.rfind(';', 0, o), s.rfind('}', 0, o), s.rfind('{', 0, o)) + 1
        head = re.sub(r'//[^\n]*|/\*.*?\*/', ' ', s[head_start:o], flags=re.S).strip()
        if head.endswith(')') or re.search(r'\)\s*(const)?\s*$', head):
            if not FUNC_HEAD_BAD.match(head) and '=' not in head.split('(')[0]:
                close = _matching_brace(s, o)
                if close > 0:
                    return o, close
    return None


def quarantine_bodies(s, lines):
    """Blank the bodies of the functions enclosing each 1-based line, keeping
    every newline. Returns (text, [(line, function name)])."""
    spans = {}
    for ln in lines:
        b = function_body_at(s, ln)
        if b:
            spans[b] = ln
    done = []
    for (o, c) in sorted(spans, reverse=True):
        head_start = max(s.rfind(';', 0, o), s.rfind('}', 0, o)) + 1
        m = re.search(r'([A-Za-z_~][\w:~]*)\s*\([^()]*(?:\([^()]*\)[^()]*)*\)\s*(?:const\s*)?$',
                      re.sub(r'//[^\n]*|/\*.*?\*/', ' ', s[head_start:o], flags=re.S).rstrip())
        name = m.group(1) if m else '?'
        # __assume(0): an unreachable end, so a non-void function needs no
        # return value (VC8 makes C4716 an error) and no code is emitted.
        s = s[:o + 1] + ' __assume(0);' + '\n' * s.count('\n', o + 1, c) + s[c:]
        done.append((s.count('\n', 0, o) + 1, name))
    return s, sorted(done)


QUARANTINE = {}       # rel path -> set of 1-based lines inside functions to blank


def translate(text, rel=''):
    s = NS_OPEN.sub(lambda m: m.group(1) + ' '.join('namespace %s {' % p for p in m.group(2).split('::')), text)
    s = NS_CLOSE.sub(lambda m: m.group(1) + '}' * len(m.group(3).split('::')) + m.group(2), s)
    if '__attribute__' in s:
        s = rewrite_packed(s)
        s = NOINLINE.sub('__declspec(noinline)', s)
    s = rewrite_defaulted(s)
    s = BRACE_INIT.sub(lambda m: '%s%s(%s);' % (m.group(1), m.group(2), m.group(3)), s)
    s = ZERO_ASSIGN.sub(lambda m: '%smemset(&(%s), 0, sizeof(%s));' % (m.group(1), m.group(2), m.group(2)), s)
    if rel.endswith('.h'):
        s = TEMPLATE_STATIC_DECL.sub('', s)
    s = BINARY.sub(lambda m: hex(int(m.group(1), 2)) + m.group(2), s)
    s = CXX_ATTR.sub('', s)
    if '__thiscall' in s or THISCALL_PTRS or (THISCALL_FUNCS and any(n in s for n in THISCALL_FUNCS)):
        s = rewrite_thiscall(s)
    s = THISCALL.sub('', s)
    s = CONV_FIRST.sub(lambda m: '%s %s %s(' % (m.group(2), m.group(1), m.group(3)), s)
    if STRIP_CTORS and any(n in s for n in STRIP_CTORS):
        s = strip_ctors(s, STRIP_CTORS)
    if CONSTRUCT and any(n in s for n in CONSTRUCT):
        s = rewrite_list_inits(s, CONSTRUCT)
    s = EMPTY_INIT.sub(lambda m: m.group(1) + '{0}' + m.group(2), s)
    if s.count('\n') != text.count('\n'):
        raise AssertionError('rewrite changed the line count')
    if rel in FILE_PATCHES:
        s = apply_patches(rel, s)
        if s.count('\n') != text.count('\n') and '__LINE__' in text:
            raise AssertionError('a line-adding patch on a file that uses __LINE__')
    if rel in QUARANTINE:
        s, _ = quarantine_bodies(s, QUARANTINE[rel])
    return s


def sources(root):
    for dirpath, dirnames, filenames in os.walk(root):
        rel = os.path.relpath(dirpath, root)
        if rel == '.':
            dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS and not d.startswith('build')]
        top = rel.split(os.sep)[0]
        for name in sorted(filenames):
            if name.endswith(EXTS) and (top not in HEADERS_ONLY or name.endswith('.h')):
                yield rel, name


def load_quarantine(path):
    QUARANTINE.clear()
    if path and os.path.exists(path):
        for line in open(path):
            parts = line.rstrip('\n').split('\t')
            if len(parts) >= 2 and parts[0] != 'file':
                QUARANTINE.setdefault(parts[0], set()).add(int(parts[1]))


def mirror_state():
    """The per-tree state the pre-pass of mirror() leaves for translate()."""
    return (dict(THISCALL_FUNCS), dict(THISCALL_PTRS), set(STRIP_CTORS), set(CONSTRUCT), set(THISCALL_DIRECT))


def mirror(root, out, quarantine=None, unchanged=None, facts=None):
    """Translate every source of `root` into `out`. unchanged(rel, name, text)
    -> True skips a file whose translation is known to be in place already;
    it is asked only when the per-tree state came out as the caller expects
    (the caller compares mirror_state() itself, through the callback).
    facts: a dict the caller keeps between runs, (content hash, is header) ->
    file_facts; the pre-pass then reads only the files that changed. It is
    updated in place to hold exactly this tree's files."""
    written = kept = 0
    load_quarantine(quarantine)
    files = list(sources(root))
    texts = {}
    for rel, name in files:
        with open(os.path.join(root, rel, name), encoding='utf-8', errors='surrogateescape') as f:
            texts[(rel, name)] = f.read()
    known = facts if facts is not None else {}
    tree = {}
    for k, t in texts.items():
        h = (hashlib.sha1(t.encode('utf-8', 'surrogateescape')).hexdigest(), k[1].endswith('.h'))
        f = known.get(h)
        if f is None:
            f = file_facts(t, h[1])
        tree[k] = f
        known[h] = f
    funcs, ptrs, direct, strip, construct = merge_facts(tree, lambda k: without_non_win32(texts[k]))
    if facts is not None:
        live = {id(f) for f in tree.values()}
        for h in [h for h, f in facts.items() if id(f) not in live]:
            del facts[h]
    THISCALL_FUNCS.clear()
    THISCALL_FUNCS.update(funcs)
    THISCALL_PTRS.clear()
    THISCALL_PTRS.update(ptrs)
    THISCALL_DIRECT.clear()
    THISCALL_DIRECT.update(direct)
    STRIP_CTORS.clear()
    STRIP_CTORS.update(strip)
    CONSTRUCT.clear()
    CONSTRUCT.update(construct)
    for rel, name in files:
        src = os.path.join(root, rel, name)
        dst = os.path.normpath(os.path.join(out, rel, name))
        if unchanged is not None and os.path.exists(dst) and unchanged(rel, name, texts[(rel, name)]):
            kept += 1
            continue
        try:
            new = translate(texts[(rel, name)], os.path.normpath(os.path.join(rel, name)))
        except Exception as e:
            sys.exit('%s: %s' % (src, e))
        data = new.encode('utf-8', errors='surrogateescape')
        if os.path.exists(dst):
            with open(dst, 'rb') as f:
                if f.read() == data:
                    kept += 1
                    continue
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        with open(dst, 'wb') as f:
            f.write(data)
        written += 1
    return written, kept


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    w, k = mirror(os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2]))
    print('vc8_translate: %d written, %d unchanged' % (w, k))
