# msvc8: the Visual C++ 2005 ASM-matching oracle

Recompile the tree with the compiler that built Diablo II 1.14d, and compare
every function, byte for byte, with the shipped `Game.exe`. A function whose
code is identical is provably the original code, without running the game.
A function that is not shows exactly where it differs.

This is the method of the sibling Quake II reconstructions
(`gladiator-bot-restored`, `rocketarena2`, `osp-tourney`, each with a VC6
oracle), carried over to a C++17 tree and a 2016 binary:

| step | there | here |
|-|-|-|
| identify the compiler | Rich header, then an A/B for the service pack | Rich header, then the linked CRT read function by function ([evidence/toolchain.md](evidence/toolchain.md)) |
| run it | VC6 under wine/box64 | VC8 SP1 under wine/box64, built from Microsoft's own packages |
| make the source compile | `msvc6_compat.h`, shim headers | `vc8_compat.h`, shim headers, and `vc8_translate.py` for what a macro cannot express |
| know which function is which | `// gladiator.dll:` docblocks | derived from evidence: `evidence/funcmap.tsv` |
| compare | masked objdump text | relocation-exact masking, and every reference checked for *what* it refers to |
| the measurement | `evidence/dll_audit.tsv` | `evidence/audit.tsv` |

**The numbers are in the evidence files, not in this README.** A count in
prose is stale the moment the next fix lands. Read `evidence/audit.tsv.summary`.

## Quick start

```sh
asm_matching/msvc8/oracle setup     # once: Game.exe 1.14d + the VC8 toolchain (~4 GB download)
asm_matching/msvc8/oracle all       # build every TU, map, libmap, audit
asm_matching/msvc8/oracle diff DRLGENV_UpdateLightIntensity
```

After editing one source file:

```sh
asm_matching/msvc8/oracle build D2Common/Environment   # recompiles what the edit touched
asm_matching/msvc8/oracle diff DRLGENV_UpdateLightIntensity
```

`oracle audit` re-judges every mapped function (well under a minute);
`oracle map` is needed when the set of functions changed (a TU newly
compiling, renames) and is worth re-running after larger changes, since some
of its tiers weigh code similarity. A full `oracle build` takes about six
minutes on a Raspberry Pi 5; after an edit only the touched TUs recompile.

After a tree change (a regen, a fix), the whole measurement in one step,
doing only what changed:

```sh
asm_matching/msvc8/oracle quick            # build, map, libmap, audit; gained/lost MATCH
asm_matching/msvc8/oracle quick --verify   # then the full run too, and diff the two
```

Each step is incremental without being approximate. A compile is stored in a
content-addressed cache (`scripts/objcache.py`, `ORACLE_CACHE`, default
`~/.cache/d2-msvc8-oracle`) under the hash of the compiler, the command line,
the translated source and every header it included, so a TU compiles only
when one of those changed - whichever build or checkout compiled it before
(`recon/runs/build-vc8.sh` compiles into the same `build/`, so the exe and
the oracle share one compile). Objects are decoded once per content. The map
and the libmap are global - pairing reads neighbours, votes and the whole
link - so they are derived whole, but only when an input changed, and what
one pair of functions decides alone (aligned call votes, an audit verdict
under the same callee addresses) is kept per pair (`ours.PairMemo`).
`ORACLE_NOCACHE=1` reads none of it: the full run `--verify` diffs against.

## What `setup` does, and needs

Needs: wine (here wine 9.17 run by box64 on aarch64; any x86 wine works),
`7z`, `cabextract`, `msitools`, `curl`, `python3` with `venv`. A private
venv with capstone is created on first use (`.venv/`). `oracle fix` also
needs what the game builds with - `cmake`, `ninja` and the mingw-w64 i686
cross compiler - for `fix_refile.py`'s syntax check, without which it
stops, saying so.

* **The reference.** `scripts/fetch_reference.sh` downloads Blizzard's
  `LODPatch_114d.exe` (6 MB) and extracts `Game.exe` with a standard-library
  MPQ reader (`scripts/mpq_extract.py`); both are SHA-256 checked. It lands in
  `reference/`, which is gitignored: **no Blizzard binary, and nothing
  derived from one's bytes, is ever committed.** `D2_GAME_EXE=/path` uses a
  copy you already have instead.
* **The toolchain.** `scripts/setup_toolchain.sh` builds it from Microsoft's
  packages without running an installer: Visual C++ 2005 Express (RTM
  compiler), its SP1 patch (whose cabinet carries whole compiler binaries) and
  VS2005 Professional (only for its Platform SDK). Cabinet members are named by
  MSI file key; `scripts/msi_paths.py` resolves each to its install path. The
  result, in its own wine prefix (`~/q2-dev/asm-matching-toolchain/wine-vc8`,
  override with `VC8_TOOLCHAIN` / `VC8_WINEPREFIX`):

  ```
  C:\vc8\rtm\{bin,include,lib}   cl.exe 14.00.50727.42
  C:\vc8\sp1\{bin,include,lib}   cl.exe 14.00.50727.762   <- the default
  C:\vc8\psdk\{include,lib}      Platform SDK (Server 2003 SP1, as bundled)
  ```

  The compilers' own runtime is Microsoft's MSVCR80 SP1, installed as a
  side-by-side assembly with wine's placeholder set aside, and loaded
  native-only: a compile that runs at all ran on Microsoft's CRT.
  `scripts/smoke_test.sh` proves each compiler runs and stamps its objects
  `Utc1400_CPP/50727`, the record the original's game objects carry.

## How a source file reaches the compiler

The tree is C++17 for mingw; VC8 is a C++03 compiler. Nothing in the tree is
edited for it. Instead `oracle build`:

1. **Mirrors** the tree into `build/src` through `scripts/vc8_translate.py`.
   Its rewrites are line-preserving (so `__LINE__` and every diagnostic map
   back 1:1) and each has an exact C++03 meaning: nested namespace
   definitions, GCC's packed attribute as `#pragma pack`, defaulted
   constructors, list-initialised declarations, calling conventions placed
   after the return type. The generator gives small by-value structs
   converting constructors (C++11 conveniences): where such a struct is a
   union member they are dropped (C++03 forbids them, and the original's C
   structs had none), where it is only list-initialised its braced
   initialisers become constructor calls. The docstring is the full list.
   `FILE_PATCHES` holds the few edits tied to one file, all in
   `d2_platform.h`'s toolchain glue; each anchor must match exactly once, so a
   regenerated header fails loudly rather than compiling unpatched.
2. **Forces in** `vc8_compat.h` (nullptr, constexpr, static_assert and the like
   as macros) ahead of `d2_platform.h`, and puts `shim_include/` first on the
   include path (`<cstdint>` for a compiler without one; a hook that runs
   right after the Windows headers; and the oracle's own
   `support/d2_trace.h`, in which the tree's hand-added tracing calls compile
   to nothing - the original has none).
3. **Compiles** each TU to `build/obj/<path>.obj`, four at a time,
   incrementally (by content: the compile cache above, header dependencies
   from `/showIncludes`). Per TU it keeps
   the compiler's log and a row in `build/compile_status.tsv`.
4. **Quarantines** what VC8 cannot compile at all - an internal compiler
   error, a GCC extension such as arithmetic on `void*` - one function at a
   time instead of losing the TU: the function holding the error is recorded
   in `build/quarantine.tsv` with the compiler's message, its body is blanked
   (lines kept), and the TU is compiled again. The audit reports such a
   function as `QUARANTINE`. An error outside any function (a header, a
   global initialiser) still fails the TU.

   Two wine facts shape this step. A compile that has written its object can
   be SIGKILLed on its way out, so the exit status is not trusted: success is
   an object that parses completely as COFF and a log without errors
   (`cl.exe` deletes its object on any real error). And the first wine process
   of a session starts wine's background services, which inherit its output
   pipes; so the session is started once, with no pipes, under a persistent
   wineserver, before any compile is captured (otherwise the first compile's
   pipes stay open until the services exit, and it waits for the whole build).

`support/`, `thirdparty/` and the tooling directories are not reconstruction
and are not compiled. `globals/*.cpp` hold data only, no function, and are
skipped unless `build.py --with-data` asks for them.

## Which original function is which

The tree carries no addresses and the original no symbols, so
`scripts/funcmap.py` derives the map from evidence, strongest tier first, and
records the tier on every row of `evidence/funcmap.tsv`:

| tier | evidence |
|-|-|
| `sourcemap` | the generator's `.cpp.map` names its address (exact; see below) |
| `name` | its generated name ends in the address Ghidra knew it by (`FOO_00512340`) |
| `export` | the original exports it under this mangled name |
| `exact` | its code is identical to one original function's (references masked, strings, imports and switch offsets compared) and unique on both sides; within a bracket gap also shorter code, unique in the gap - in a gap with another file's mapped code inside, which is not one object's, only while the gap is narrow (`FOREIGN_GAP`), else the code must also be unique overall |
| `located` | the same, but for source locations: where ours pushes its own line and loads a file name of the oracle's tree (`__LINE__`, `__FILE__`: upstream's allocator macros), the original pushes and loads its own; jumps compared by the instruction they reach. Pairs what `fix_allocargs.py` needs to read the original's values off |
| `votes` | under the map of a first pass, the aligned call sites of all paired functions agree where our callee sits in the original; pinned, and the map derived again around the pins |
| `string` | string literals that occur in at most a few original functions, nearly all held by one |
| `import` | the same sequence of imported-API calls, unique on both sides |
| `calls` | an already-paired caller calls it at the same place of an aligned call sequence (a single unknown between agreeing neighbours) |
| `bracket` | in its TU it lies between two paired functions whose originals have exactly as many unpaired functions between them (the linker keeps an object's functions in order, the generator writes them in address order) |
| `align` | the same with unequal counts, aligned by instruction similarity above a threshold |
| `refs` | what it refers to that is known on both sides (mapped callees, CRT functions, imports, strings), weighted by rarity, matches one original function clearly better than any other |

Pairs are one-to-one; a function claimed twice at the same tier keeps
neither. An equal-count gap that an alignment by code similarity pairs
clearly better than plain order is left to `align` (its two sides hold
different functions), and at the end a pair made by position or similarity
whose callees, all mapped, are not the original's is dropped. Callees that
identical code calls are pinned like voted ones. Votes do not move a function
away from an original its code matches, unless they are overwhelming (20 call
sites or more, 95% agreeing): those are pinned before identical code is even
looked at, since an edit can make one of two sibling helpers compile exactly
to the other's original. In an aligned call sequence the stack-cookie check
does not count as a neighbour: VC8 places its own `__security_check_cookie`
call where the epilogue lands. Calls into the statically linked CRT are known on both sides from
`evidence/crtmap.tsv` (below), so they anchor call sequences as imports do.
`evidence/funcmap.tsv.unmapped` lists our functions without a counterpart.

If the generator's `.cpp.map` source maps (gitignored in this repo) are
present next to the sources, the `sourcemap` tier reads them and everything
else only fills gaps; the other tiers need nothing but the tree and the image.

Two checks keep the map honest. Functions paired by order-blind evidence
(strings, imports, calls, refs) should also fall in address order within their
TU, as the original lays them out; and every generated name that carries its own
Ghidra address (`eD2GSGameType_007a0610`) must land on that address - the
audit summary's self-check line reports both kinds of disagreement.

## How a function is judged

`scripts/refimage.py` carves the original into functions (16-aligned starts
after int3 padding, call targets to a fixpoint, vtables, and every address
code takes - `push offset Callback` - that no function's decoding reaches;
everything else in data that points at code is an entry *into* a function -
SEH filters, handlers, C++ EH funclets) and decodes each by recursive
descent, so switch tables and padding are never taken for code. Our side is the COFF object:
each function is a COMDAT section (`/O2` implies `/Gy`) with exact relocations,
decoded the same way; a switch table is found through the relocation on the
jump's displacement, which in an object holds only the addend when VC8
relocates against a `$LN` label. A branch whose rel32 carries a relocation
leaves the function: in an object its field holds 0, which reads as a jump to
the next instruction, and a tail jump followed by a switch table would
otherwise decode the table as code.

`scripts/compare.py` reduces both to the same view: per instruction, the bytes
with every reference field zeroed, and per reference a token for what it
names - a function by its original address, an import by name, a string by
content (from the referenced byte on: ours applies the addend, so a reference
into a merged literal, `"Amazon"+4`, is `"on"` as the original's is), a switch
table by its offset in the function, a global (ours: symbol + addend, theirs:
address). Base relocations make the masking exact rather
than guessed: at image base 0x400000 an address looks like any constant. A
call of a function's own entry (recursion) is a reference like any other
call: our object relocates it against our own symbol.

| status | meaning |
|-|-|
| `MATCH` | same instructions, identical masked bytes, every comparable reference agrees |
| `REF_DIFF` | identical bytes, but some reference names a different function/string/import |
| `INSN_MATCH` | same instruction sequence, different bytes (registers, displacements) |
| `INSN_COUNT_MATCH` | same number of instructions, different instructions |
| `OUR+N` / `OUR-N` | N more / fewer instructions than the original |
| `QUARANTINE` | VC8 could not compile it; its body was blanked so its TU compiles (the note has the error) |
| `MISSING` | mapped, but its object is not built (its TU failed) |

`similarity` (0..1) is the difflib ratio of the two listings with functions
and globals neutralised: the progress measure for a function that does not
match yet. A global cannot be checked function by function, so it is counted
as *unverified*; instead, every aligned use of one of our globals implies an
original address (that use's address minus our offset), and the audit writes
them to `evidence/datamap.tsv`: one consistent address per global, or the
conflicting ones, which point at a wrong global or offset in the tree.

| marker in `oracle diff` | |
|-|-|
| (blank) | identical, every comparable reference agrees |
| `!` | identical bytes, but a reference names a different function, string or import |
| `~` | same instruction, different bytes |
| `\|` | different instruction |
| `<` / `>` | only ours / only the original's |

## The statically linked CRT

`scripts/crtmap.py` compares every function of VC8's `libcmt.lib` and
`libcpmt.lib` (RTM and SP1) with every original function. The hits name the
original's CRT (`evidence/crtmap.tsv`), and the per-version split is what
settled SP1 (`evidence/toolchain.md`).

The audit summary's headline counts the game **without the C runtime**: it leaves out of
both the matched and the total every original function that is Microsoft's
runtime - the libmap pairs, the run of library code they span, the linker's
import thunks, and `evidence/runtime.tsv` (the functions Ghidra tags `runtime`,
named there from this evidence). It leaves out the compiler's exception-handling
code too: each `__ehhandler` stub (`mov eax, offset FuncInfo; jmp
__CxxFrameHandler3`) and each unwind funclet a FuncInfo's unwind map names
(`audit.eh_starts`). VC8 writes them into `.text$x`, after all the code, where
the carve makes them functions of their own, while our objects keep them beside
the function they serve; they have no source of their own. Catch handlers stay
in. The whole-binary figures follow it.

The summary's `byte kinds` block splits the headline's total into six kinds,
one line each (`  kind <key> <bytes> <percent>% <meaning>`), summing to the total
exactly (asserted): `whole` (MATCH functions), and for every other compared
function `ident` (original bytes in instructions identical to the aligned one of
ours, references judged as MATCH judges them), `operand` (aligned, same
mnemonic, other registers or displacements), `differ` (instructions with no
such partner) and `noncode` (switch tables and padding inside the extent);
`uncompared` is the original functions no row compares. The alignment is the
similarity's, on the neutral keys, with what it leaves unpaired aligned again on
mnemonics (`compare.aligned_bytes`); `audit.tsv` carries the per-function
`ident_bytes` and `operand_bytes` as its last two columns.
`scripts/byte_match_svg.py` draws the block as `evidence/byte-match-light.svg`
and `-dark.svg`, for a README's `<picture>`.

The runtime is code we link, not code the tree holds, and it is measured like
the rest. `scripts/libmap.py` (`oracle libmap`, run by `oracle all` after
`map`) pairs every original function the funcmap leaves unpaired with a
function of the SP1 libraries, and writes `evidence/libmap.tsv` in the
funcmap's format, `tu` being `<library>/<member>`:

| tier | evidence |
|-|-|
| `lib-exact` | identical masked code, unique on both sides; code the libraries hold in several members is taken, inside the run of library code, as the copy our image links |
| `lib-order` | the linker keeps a member's functions in member order: beside a paired one, the most similar of its unpaired neighbours, clearly |
| `lib-similar` | inside the run of library code, the clearly most similar library function: another revision of the same function |
| `lib-votes` | twins code cannot tell apart (memcpy, memmove): where the aligned call sites of everything paired send our call |

`oracle audit` judges these pairs with the same comparer, relocations masked
the same way, and counts them in its totals; the summary's next line says how
much of them is the runtime. Nothing about a library pair decides its status:
a wrong pair costs a MATCH, it cannot make one. `oracle diff` shows them too.

Two facts of the original's carving matter here. refimage starts a function
at every call target, so a CRT function's `__finally` funclet, called from its
own body, is carved apart, while the library object holds it in one function:
a pair may span several carved functions that tile it exactly
(`compare.ref_view_span`), and over the span a branch into it is compared by
its raw bytes and an address taken in it as `$+offset`, as in our object. And
an assembler object's PROCs need not carry the function type, so a library's
code section without function symbols is split at its public labels
(`objfuncs.functions(..., labels=True)`).

Which pairs our image links (`linked` in the detail) comes from the map of
`build-exe/d2recon-vc8.exe`, which `recon/runs/build-vc8.sh` writes
(`--exe-map` to point elsewhere). `evidence/libmap.tsv.summary` has the pairs
per tier and what stays unpaired: inside the run of library code, the import
thunks the linker makes (`jmp [__imp_X]`, no code of ours to compare), and
elsewhere - the game's own functions the funcmap does not pair.

## Flags

The default (`scripts/oracle_config.py`):

```
/O2 /Oy- /Ob1 /EHsc /GS /MT /W1 /D WIN32 /D NDEBUG /D _WINDOWS
```

Every flag in it is either established (`/O2`: byte-identical functions;
`/MT`: the static CRT) or measured: [evidence/flags.md](evidence/flags.md)
records each A/B, the per-module tables, and which questions are settled.
`/Oy-` (keep the frame pointer) and `/Ob1` (no automatic inlining) each win in every
module. Before 1.14 every module was its own project with its own settings,
so a flag may also apply to some modules only. An A/B:

```sh
X='/O2 /Oy- /Ob1 /EHsc /GS- /MT /W1 /D WIN32 /D NDEBUG /D _WINDOWS'   # e.g. no /GS
ORACLE_CFLAGS="$X" asm_matching/msvc8/oracle build    # into build/experiment-sp1-<hash>/
ORACLE_CFLAGS="$X" asm_matching/msvc8/oracle audit    # same map, that build's code
asm_matching/msvc8/scripts/abcompare.py asm_matching/msvc8/evidence/audit.tsv \
    asm_matching/msvc8/build/experiment-sp1-<hash>/audit.tsv
```

A flag that a module was built with goes into `flags.tsv` (path prefix, extra
flags, the evidence), never into the defaults; changing a row recompiles
exactly the TUs it covers.

An experiment builds in `build/experiment-<compiler>-<hash>/`, is audited with
the baseline's function map (same pairs, other code) and writes its audit
there. **Evidence from an experiment is never committed.** `ORACLE_VC=rtm`
selects the RTM compiler the same way. The rule lives in
`scripts/oracle_config.py`, so it holds however a script is run: an
experiment aimed at the baseline directory, or writing into `evidence/`, is
refused.

A precompiled header (default; `ORACLE_PCH=0` turns it off) covers the
prefix every TU parses (`d2_platform.h`, the Windows headers,
`globals/globals.h`), which is most of each compile. `scripts/objdiff.py`
compares two builds function by function. A PCH build comes out identical to
a plain one but for the rare function whose code depends on the *symbol
phase*: VC8 breaks some ties by the numbers it gives symbols in declaration
order, modulo 64, and the PCH numbers them differently - as does any
declaration added to or removed from a header it includes. Neither phase is
the original's; [evidence/flags.md](evidence/flags.md) ("Symbol phase") has
the measurement and the bench test that tells such a flip from a real
regression.

## Working toward identity

The inner loop for an edit to the tree:

```sh
asm_matching/msvc8/oracle build D2Common/Units       # recompile what the edit touched
asm_matching/msvc8/scripts/check.py D2Common/Units   # re-judge those functions against evidence/audit.tsv
asm_matching/msvc8/oracle diff InitRightSkill        # what still differs, with the source lines it comes from
```

`check.py` prints new matches and regressions and exits 1 on a regression;
`--update` writes the new rows into `evidence/audit.tsv` (the summary is
rewritten only by a full `oracle audit`). The diff's source lines come from
CodeView line tables: every TU is compiled with `/Z7`, a tool flag that adds
debug sections and leaves the code alone (checked with `objdiff.py` over the
whole tree).

A hypothesis about the source - "was it `x >= 0`?", "was this an assertion?"
- is tested on the bench before it touches the tree:

```sh
cp asm_matching/msvc8/build/src/Fog/Src/AsyncData.cpp asm_matching/msvc8/build/scratch/try.cpp   # edit it
asm_matching/msvc8/scripts/snippet.py asm_matching/msvc8/build/scratch/try.cpp --func ASYNCDATA_GetReadSize
```

`snippet.py` compiles the file with the baseline compiler, flags, response
file and (a private copy of) the precompiled header into `build/scratch/`,
and prints the function beside the original with the status the audit would
give it; without `--func`, every function of the object. The function's
module flags (`flags.tsv`, e.g. D2Client's `/GS-`) apply as in the build:
those of the TU the map files the function under, or of `--tu`.

Finding what to fix is statistics over all pairs, not reading diffs one by
one:

| | |
|-|-|
| `scripts/diffgrams.py` | the most frequent differing stretches, by shape (`cmp R, -N ; jle N => test R, R ; jl N`): a construct written another way than Blizzard did, a flag, a convention |
| `scripts/classify.py` | each non-matching function tagged by kind of difference: `cleanup` (`ret N`), `regargs` (arguments in other registers), `frame`, `saved`, `calls`, `inverted`, `signedness`, `regalloc`, `displacement`, `immediate`, `shape` |
| `scripts/tulayout.py` | does each TU correspond to one original object: its functions in one range, in order, nothing foreign inside |

A difference that the generator produces the same way everywhere is fixed
the same way everywhere, by a fixer: a script that finds the sites in the
aligned code, edits the tree, rebuilds, re-judges, and keeps an edit only if
nothing that matched stops matching and the function it is for gets no less
similar (`scripts/fixlib.py`; without `--apply` each lists what it would do).
An edit that only removes what the original provably lacks (a statement
standing for code the compiler emits by itself) is *neutral*: kept even if
the registers shuffle and the similarity dips, unless a function stops
matching. An edit that other functions prove - a static promotion, whose
convention its callers' code depends on - may leave its function less
similar when one of those callers comes to match. Each fixer's docstring
names the evidence its rule rests on.

| fixer | the generator wrote | the original has |
|-|-|-|
| `fix_static.py` | every function extern | a file-local function `static`: VC8 then gives it a register convention of its own (an argument in EAX, ESI, ..., or a `__fastcall` that leaves EDX or ECX unused), which the original shows - a register it reads, or pushes on and never pops, at entry |
| `fix_refile.py` | a function in the file of its Ghidra namespace, though the original passes it arguments in registers from callers filed elsewhere (`evidence/objects.tsv`, kind `object`) | the function and its callers in one file - the one the annotations around them name, else the one holding most of them - at their addresses, `static` where only that file calls them, each in a block of its own namespace: names stay, files change |
| `fix_compare.py` | Ghidra's canonical comparison: `-1 < x`, `x < 5`, `b <= a`; `x != 0` where the original's unsigned jump shows `x > 0` (with the if/else swapped where the jump is inverted too) | the form the instruction shows: `x >= 0`, `x <= 4`, `a >= b`, `(uint32_t)x > 0` |
| `fix_assert.py` | `if (ok) { return x; } halt(); exit(-1);` | the assertion first: `if (!ok) { halt(); exit(-1); } return x;` |
| `fix_ifelse.py` | `if (!c) { B } else { A }`, where a jump shows it - or, kept only where its function gets more similar, any `if (!x)` or `if (x == 0)` of one operand | the branches in the order VC8 lays out: `if (c) { A } else { B }` |
| `fix_switch.py` | an if-chain on one variable, `if (v == A) {…} else if (v == B) {…}`, or consecutive `if (v == A) {… return;}` blocks | the `switch (v)` the original lowers to subtractions; `--single` also a one-case chain where the original subtracts the case value (`sub; je`) |
| `fix_guardstack.py` | `Fog::Debug::GuardStack();` where the original checks its stack cookie | nothing: under `/GS` VC8 emits the check itself |
| `fix_uninline.py` | the body of a helper marked inline in the Ghidra database, e.g. `seed.nSeedLow = v; seed.nSeedHigh = 0x29a;` | the call: `InitSeed_Param_666(&seed, v)` |
| `fix_return.py` | `void` where the original returns one constant on every path (EAX at each `ret`, by constant propagation over its control flow: `classify.eax_at_returns`), or one argument (`arg_at_returns`) | `int` returning that constant; `bool` where only AL holds it; the parameter, as its type |
| `fix_gsbuffer.py` | `uint8_t aPacket[9]` in a function the original gives no stack cookie (`/GS` protects byte arrays); with `--members` a byte array inside a local's struct type, such as D2DamageStrc's padding `uint8_t nReserved66[2]` | a struct of scalar fields, `struct { uint8_t e0, …, e8; }`, addressed through a byte pointer |
| `fix_order.py` | independent stores in an order of its own, `p[2] = x; p[1] = y; p[0] = 0x0c;` | the order VC8 schedules the original's from: fields in member order (`--by offset`), or the original's store order (`--by store`) |
| `fix_idioms.py` | operators Ghidra could not fold back, e.g. `s.nSeedLow + (int)(((uint64_t)d2_bits_of(s) & 0xffffffff) / 100) * -100` | the operator: `s.nSeedLow % 100`; `__alldiv((uint32_t)E, (uint32_t)((uint64_t)E >> 0x20), 100, 0)` is `(int64_t)E / 100`; `(int)(x + (x >> 0x1f & 3U)) >> 2` is `x / 4`; `x & 0xff` is `(uint8_t)x` (each idiom documented with why the value is the same) |
| `fix_localtype.py` | a `uint8_t` or `uint16_t` local the original keeps whole: VC8 loads and stores it as a byte (`mov al, byte ptr [edi + 3]`, `mov byte ptr [ebp - 0xcd], 0`) where the original zero-extends and stores a dword | `uint32_t` (`int32_t` if signed), kept where the function comes out more similar |
| `fix_bitmask.py` | a flag tested or set with a constant mask, `dwState & 4`, or read through a one-bit bitfield, `if (pMonStats2->isSmall)` | the mask read from Blizzard's table, as the original loads it: `dwState & gadwBitMasks[2]`, `(*((uint8_t *)pMonStats2 + 0xOFF) & gadwBitMasks[n])` - as many sites per bit as the original reads the table at that bit; with `--inverse` a flag cleared, `dwState &= -2`, as `dwState &= gadwInvBitMasks[0]`, each function with its gadwBitMasks sites in one edit |
| `fix_shiftbit.py` | a flag test as a mask, `f & 8` or `f >> 3 & 1`, which VC8 compiles to `test byte ptr [R + N], 8` | the bool the original makes of the shifted bit, `((bool)(f >> 3 & 1))`: `shr eax, 3; test al, 1` |
| `fix_stringops.py` | the string instructions as library calls or pointer loops: `memset(a, 0, 32 * sizeof(a[0]))`, `for (n = 25; n; n--) { p[0] = 0; p[1] = 0; p += 2; }`, `for (n = K; n; n--) { *d = *s; s++; d++; }` | what VC8 compiles to those instructions: `for (int i = 0; i < 32; i++) a[i] = 0;` (VC8 expands no memset beyond 32 bytes), `memcpy(d, s, K * sizeof(*d))` |
| `fix_nestcall.py` | a call's result in a temporary, `pGame = GetGamePointer(pUnit); HasQuestItem(pGame, pUnit, code);` (with `--runs`, several temporaries feeding one call) | the call nested in the argument list, which VC8 evaluates after the arguments to its right: `HasQuestItem(GetGamePointer(pUnit), pUnit, code)` |
| `fix_inplace.py` | a read-modify-write through a temporary pointer, `p = &pUnit->nCount; *p = *p + 1;` | the update of the field itself, `pUnit->nCount++` (`+= v`, `|= v`, ...) |
| `fix_boolarith.py` | arithmetic on a comparison, `(uint32_t)(c != 0) * 2 + 1` | the choice VC8 compiled without a branch, `(uint32_t)((c != 0) ? 3 : 1)` |
| `fix_allocargs.py` | Fog allocator calls as upstream's macros, `D2_FREE(p)`, `D2_NEW(T)`, `D2_REALLOC_POOL(m, p, n)`, which pass the reconstruction's own `__FILE__` and `__LINE__` | the call with the file and line the original passes, read off the original's code: `FreeClientMemory(p, ".\\Src\\Exceptions.cpp", 0x1f7, 0)` |
| `fix_permute.py` | a run of independent assignments in the order Ghidra listed the original's stores, which is VC8's schedule, not the source order | the order that compiles to the original's: searched, every order of each run tried, one trial per function per round |
| `fix_caseorder.py` | a switch's cases in order of their values | the order Blizzard wrote them in, read off the original's jump table: VC8 lays case bodies out in source order, so the same case values reach the same bodies at other addresses |
| `fix_forloop.py` | a rotated loop, `if (0 < n) { int i = 0; do { B; i++; } while (i < n); }` | the for loop it was, `for (int i = 0; i < n; i++) { B }`: VC8 then initialises the counter before the test, as the original does |
| `fix_fastcall.py` | `__cdecl`/`__stdcall` on a function whose original takes its first arguments in ECX (and EDX) and pops what `__fastcall` leaves on the stack | `__fastcall`, judged on the function and every caller; `--thiscall`: a function taking `this` in ECX that no original code calls directly becomes `__fastcall(this, int32_t nEdxUnused, ...)` |
| `fix_paramtype.py` | a `uint8_t` or `uint16_t` parameter of a `__fastcall` function, where the original's callers load the whole of ECX or EDX (`mov edx, 5` against our `mov dl, 5`) | `int32_t`, judged on the function and every caller |
| `fix_globalwidth.py` | a global or static typed `bool`, `uint8_t`, `uint16_t`, ..., by the accesses Ghidra saw: `mov byte ptr [g], 1` | the dword the original reads and writes, `mov dword ptr [g], 1`: `BOOL`, `int32_t` or `uint32_t` in every declaration, judged on the whole tree |
| `fix_restore.py` | after a regeneration, the generator's body again for a function that matched before it | the body it matched with, from the old tree's commit (`fix_restore.py REV OLD_AUDIT`), kept only where the function then matches again |

`oracle fix` runs them all (`scripts/fix_all.py`): conventions first
(`fix_refile`, `fix_static`, then `--demote`, then `fix_fastcall`, `fix_paramtype`, `fix_globalwidth`), the local rewrites after, the whole cycle
again while a round still keeps edits - their rules interact (a static
function changes its callers; a comparison written back lets an if/else line
up). It is also what a regenerated tree is put through, and what
the fixers do not find again `fix_restore.py` takes from the tree before it,
where the oracle confirms it. Fixers keep a file's
line count while they work (a removed statement becomes an empty line, so
the line tables of everything after it stay valid between builds);
`scripts/tidy_blanks.py --apply` drops those lines once the fixing is done.

They edit generated files. The durable home of each rule is the generator
or the Ghidra database the tree is generated from - [UPSTREAM.md](UPSTREAM.md)
states each for its maintainers, `evidence/fixes.tsv` records every run - and
the fixers are what the next regeneration is re-run through until then.

Our objects lay a TU's functions out in an order of their own: VC8 compiles
a caller only after the same-TU functions it calls, and emits a function
calling one defined further down (or an inline helper) at the end of the
object. The generator writes a file's functions in address order, so the
map (`bracket`, `align`) and `tulayout.py` order our functions by source line
(`ours.OurFunc.layout_key`), not by COMDAT order.

## Rules

* Numbers live in `evidence/`. Do not quote a MATCH count in prose.
* `evidence/` holds the baseline (SP1, default flags) only.
* Blizzard's binaries, their disassembly and anything else carrying their
  bytes stay out of git: `reference/` and `build/` are ignored.
* One build per checkout at a time: two concurrent `oracle build`s write the
  same objects. Two checkouts sharing the wine prefix need different drive
  letters (`ORACLE_DRIVE=s`); `scripts/env.sh` refuses to re-point a letter
  that maps another checkout.
* A function that matched and stops matching after an edit is a regression,
  whatever the runtime says: diff it before committing. If the edit did not
  touch its TU (a header declaration), shift its symbol phase on the bench
  first (evidence/flags.md, "Symbol phase").

## Scripts

| | |
|-|-|
| `oracle` | the entry point; `oracle help` |
| `scripts/env.sh` | shared settings, drive mapping, the venv |
| `scripts/fetch_reference.sh`, `mpq_extract.py` | Game.exe 1.14d from Blizzard's patch |
| `scripts/setup_toolchain.sh`, `setup_psdk.sh`, `msi_paths.py`, `vc8_layout.py`, `smoke_test.sh` | the toolchain from Microsoft's packages |
| `scripts/vc8.sh` | run one VC8 tool under wine with the oracle's INCLUDE/LIB |
| `scripts/vc8_translate.py`, `vc8_compat.h`, `shim_include/` | the source the compiler sees |
| `scripts/build.py` | compile, incremental and parallel |
| `scripts/peimage.py`, `refimage.py`, `funcdecode.py` | the original, carved and decoded |
| `scripts/coffobj.py`, `objfuncs.py`, `ours.py` | our objects |
| `scripts/funcmap.py` | which original function is which |
| `scripts/compare.py`, `audit.py`, `asmdiff.py` | the judgement |
| `scripts/byte_match_svg.py` | the summary's byte kinds as a bar chart |
| `scripts/check.py`, `snippet.py` | the inner loop: re-judge a TU, try a variant |
| `scripts/diffgrams.py`, `classify.py`, `tulayout.py` | what differs, over all pairs |
| `scripts/commit.py` | commit the branch's way (CLAUDE.md, "Commits"): tooling amended into the first commit, source changes as a new commit |
| `scripts/objects.py` | `evidence/objects.tsv`: functions the tree files apart from the callers they share a register convention with, and declared conventions the original contradicts - for upstream |
| `scripts/fixlib.py`, `fix_static.py`, `fix_refile.py`, `fix_compare.py`, `fix_assert.py`, `fix_ifelse.py`, `fix_switch.py`, `fix_guardstack.py`, `fix_idioms.py`, `fix_bitmask.py`, `fix_shiftbit.py`, `fix_localtype.py`, `fix_uninline.py`, `fix_return.py`, `fix_gsbuffer.py`, `fix_stringops.py`, `fix_nestcall.py`, `fix_inplace.py`, `fix_boolarith.py`, `fix_allocargs.py`, `fix_order.py`, `fix_permute.py`, `fix_caseorder.py`, `fix_forloop.py`, `fix_fastcall.py`, `fix_paramtype.py`, `fix_globalwidth.py`, `fix_restore.py`, `fix_all.py`, `tidy_blanks.py` | systematic fixes, kept only where confirmed; `oracle fix` runs them all |
| `scripts/oracle_config.py` | which build directory a configuration uses |
| `scripts/objdiff.py` | do two builds contain the same code |
| `scripts/abcompare.py` | baseline vs experiment, module by module |
| `scripts/crtmap.py` | the linked CRT, by bytes |
| `scripts/libmap.py` | the linked runtime's functions paired with the original's, judged by the audit |
| `scripts/decode_rich_header.py`, `compid.py` | tool stamps in images and objects |
