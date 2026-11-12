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
| make the source compile | `msvc6_compat.h`, shim headers | `vc8_compat.h`, shim headers, and ghidra-reconstruct's VC8 translation for what a macro cannot express |
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
asm_matching/msvc8/oracle build Source/D2Common/Environment   # recompiles what the edit touched
asm_matching/msvc8/oracle diff DRLGENV_UpdateLightIntensity
```

`oracle audit` re-judges every mapped function (two seconds with zig, else
well under a minute);
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
content-addressed cache (`zig/src/objcache.zig`, `ORACLE_CACHE`, default
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
`7z`, `cabextract`, `msitools`, `curl`, and `zig` (0.17; `zig/`, built on
first use). Every step runs oracle-zig, and stops when zig is missing. Nothing
is Python. `oracle build` translates the tree with a ghidra-reconstruct
checkout; a checkout without one (the public repository, its CI) compiles the
translation the export ships in `translated/` instead.

* **The reference.** `scripts/fetch_reference.sh` downloads Blizzard's
  `LODPatch_114d.exe` (6 MB) and extracts `Game.exe` with a self-contained
  MPQ reader (`oracle-zig mpq-extract`);
  both are SHA-256 checked. It lands in
  `reference/`, which is gitignored: **no Blizzard binary, and nothing
  derived from one's bytes, is ever committed.** `D2_GAME_EXE=/path` uses a
  copy you already have instead.
* **The toolchain.** `scripts/setup_toolchain.sh` builds it from Microsoft's
  packages without running an installer: Visual C++ 2005 Express (RTM
  compiler), its SP1 patch (whose cabinet carries whole compiler binaries) and
  VS2005 Professional (only for its Platform SDK). Cabinet members are named by
  MSI file key; `oracle-zig msi-paths` resolves each to
  its install path, and `vc8-layout` and `psdk-layout` lay the
  trees out. The
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

1. **Mirrors** the tree into `build/src` through ghidra-reconstruct's VC8
   output mode (`packages/reconstruct/src/vc8`). Its rewrites are
   line-preserving (so `__LINE__` and every diagnostic map back 1:1) and each
   has an exact C++03 meaning: nested namespace definitions, GCC's packed
   attribute as `#pragma pack`, defaulted constructors, list-initialised
   declarations, calling conventions placed after the return type, with
   `__thiscall` functions keyed on their qualified name. The generator gives
   small by-value structs converting constructors (C++11 conveniences): where
   such a struct is a union member they are dropped (C++03 forbids them, and
   the original's C structs had none), where it is only list-initialised its
   braced initialisers become constructor calls. `FILE_PATCHES` there holds
   the few edits tied to one file, all in `Source/D2Headers/d2_platform.h`'s
   toolchain glue; each anchor must match exactly once, so a regenerated
   header fails loudly rather than compiling unpatched.

   `zig/src/ts_translate.zig` picks where the translation comes from:
   `ORACLE_TRANSLATED=<dir>` names a shipped tree; else `GR=<checkout>` names a
   translator (an error if it has no VC8 mode); else the live daemon's
   checkout `~/code/ts/gr-daemon` when it has `src/vc8/cli.ts` and a built
   cpp-parser; else `translated/` beside the oracle, the tree the export
   ships to the public repository (`oracle translate <out>` writes one, with
   `sources.tsv`: the SHA-256 of each source it was translated from). A
   shipped tree is checked against the checkout first, and a source changed
   since it was translated stops the build with the list. The build log's
   `translate (...)` line names the one used. A shipped tree cannot blank a
   function, so there a TU VC8 cannot compile fails rather than being
   quarantined; the translator quarantines as below.
2. **Forces in** `vc8_compat.h` (nullptr, constexpr, static_assert and the like
   as macros) ahead of `d2_platform.h`, and puts `shim_include/` first on the
   include path (`<cstdint>` for a compiler without one; a hook that runs
   right after the Windows headers; and the oracle's own
   `Redivivus/d2_trace.h`, in which the tree's hand-added tracing calls compile
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

`Redivivus/` (but for the generated `ghidra_version.cpp`), `3rdparty/libbacktrace/`,
`3rdparty/implibs/` and the tooling directories are not reconstruction and are
not compiled; only the headers of `Redivivus/` are translated, since the tree
includes them. `Source/globals/*.cpp` hold data only, no function, and are
skipped unless `oracle build --with-data` asks for them.

## Which original function is which

The tree carries no addresses and the original no symbols, so
`zig/src/funcmap.zig` derives the map from evidence, strongest tier first, and
records the tier on every row of `evidence/funcmap.tsv`:

| tier | evidence |
|-|-|
| `sourcemap` | the generator's `.cpp.map` names its address (exact; see below) |
| `name` | its generated name ends in the address Ghidra knew it by (`FOO_00512340`) |
| `export` | the original exports it under this mangled name |
| `exact` | its code is identical to one original function's (references masked, strings, imports and switch offsets compared) and unique on both sides; within a bracket gap also shorter code, unique in the gap - in a gap with another file's mapped code inside, which is not one object's, only while the gap is narrow (`FOREIGN_GAP`), else the code must also be unique overall |
| `located` | the same, but for source locations: where ours pushes its own line and loads a file name of the oracle's tree (`__LINE__`, `__FILE__`: upstream's allocator macros), the original pushes and loads its own; jumps compared by the instruction they reach. |
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

Our side is the functions the link keeps, not every function of every object
(`ours.link_kept`). An external COMDAT that several objects define - an inline
function of a header, a template instantiation, all `selectany` - is the copy
of the first object in link order, as link.exe keeps the first definition it
reads and discards the rest; the original's one copy came about the same way.
Link order is `recon/runs/build-vc8.sh`'s, which is the CMake build's: the
module libraries of `Source/modules.cmake` in the executable's (1.14d) link
order, each library's TUs in the original's order (`D2_MODULE_<m>_UNITS`,
below) and its data unit last, what no module owns last of all
(`ours.linkKept`, `zig/src/modules.zig`). Against the plain path order it
moves 7 pairings to another TU's copy and no MATCH (v2013); the original's TU
order moves 22 more and no MATCH (v2015). The copies are not interchangeable,
since a directory can compile with other flags (`/GS-`, `/O1`), so it is that
copy that is paired and judged, and the discarded ones are neither paired nor
listed as unmapped. A map entry whose own TU's object holds no kept copy - the
generator's `inline-host` functions, defined in a `<unit>.inline.h` (the
entry's `definedIn`) and emitted only into the objects that call them - is
found among all objects' external functions by its qualified name, or by its
bare name where our qualifier is a prefix of the map's namespace, if that
names one function; the row's `tu` is then the object the copy came from.

Two checks keep the map honest. Functions paired by order-blind evidence
(strings, imports, calls, refs) should also fall in address order within their
TU, as the original lays them out; and every generated name that carries its own
Ghidra address (`eD2GSGameType_007a0610`) must land on that address - the
audit summary's self-check line reports both kinds of disagreement.

## How a function is judged

`oracle ref` (`zig/src/refimage.zig`) carves the original into functions (16-aligned starts
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

`zig/src/compare.zig` reduces both to the same view: per instruction, the bytes
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

`oracle crt` (`zig/src/crtmap.zig`) compares every function of VC8's `libcmt.lib` and
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
in. It leaves out a prebuilt third-party library as well, listed in
`evidence/prebuilt.tsv`: the 16 functions of the PKWARE Data Compression Library
(006b01b0..006b1343, 4,397 bytes) are byte-identical, relocations masked, to the
`explode.obj` and `implode.obj` of the 1997 `IMPLODE.LIB`, objects with no
`@comp.id` (the Rich header's prodid 0 row). VC8 never compiled them, so no source
or flag can match them; only linking that library can. The whole-binary figures
follow it.

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
`oracle svg` draws the block as `evidence/byte-match-light.svg`
and `-dark.svg`, for a README's `<picture>`.

The runtime is code we link, not code the tree holds, and it is measured like
the rest. `zig/src/libmap.zig` (`oracle libmap`, run by `oracle all` after
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

## How data is judged

The function figure cannot see a table: a global is only "unverified" there.
`oracle data` (`zig/src/datamatch.zig`, or `oracle audit --data` /
`ORACLE_DATA=1` to run it after the function audit) compares every data symbol
of our objects with the bytes 1.14d holds at that symbol's address, writing
`evidence/data-audit.tsv`, its `.summary`, `evidence/datatrend.tsv` (MATCH rate
by size) and `evidence/data-fixes.tsv` (the largest defects with a likely root
cause and whether they look like runtime faults).

Ghidra is the truth for an extent, so the denominator is
`evidence/dataextent.tsv`: every item Ghidra has in `.rdata`/`.data`, read by
`oracle data extents` from a ghidra-reconstruct codegen snapshot (the export a
regen emits the tree from; `--snapshot DIR`, default the daemon's) and committed,
so the audit needs neither Ghidra nor a daemon. A label inside another item
(`gnHashHighTable` four bytes into `gaHashSeedTable`, `gaTable[3][0]`, which the
generator writes `gaTable_3__0_`) is an `interior` row: its bytes are its
container's, so it is judged (is ours laid out inside the container, or an object
of its own?) but kept out of the byte totals.

Our side is the objects: `build/obj` and the `Source/globals/` objects the whole-image
build compiles into `build-exe/obj/globals` with the same command line. A symbol
pairs with an item by name (narrowed by an address the name carries, the owner
function of a static local, or the namespace), else by the address
`evidence/datamap.tsv` implies from aligned code, else by the address a generated
name carries. Relocations are masked as the function compare masks them: a pointer
compares by what it names (our target symbol's original address, through a jump
thunk where the original's table names the thunk), a pointer to a literal of ours
by the literal's content.

A read-only string the generator spells as a literal at its uses (`s_Russian_006cc7a4`
becomes `"Russian"` in the bodies and tables) has no symbol of ours to pair by name.
Those are found by the generator's byte rule over `dataextent.tsv` and
`evidence/datastrings.tsv` (Ghidra's untyped strings, written by `oracle data strings`
from the snapshot), or taken from the generator's own compiler-data report
(`--compiler-data`), and judged through every relocation of the original that points
at one: the function of ours the funcmap pairs with a code use, the pointer field of
ours paired with a data use. Content and terminator are compared, placement is not.
They are kind `strlit`, a summary row of their own; the headline stays the named data.

| status | meaning |
|-|-|
| `MATCH` | every byte equal, every pointer names the same target |
| `ZERO` | ours is all zero where the original has data: an initialiser never written, or one VC8 builds at run time (a constructor call; the note says so) |
| `SPLIT` | an interior label emitted as an object of its own |
| `SIZE_SMALL` / `SIZE_LARGE` | our extent shorter than Ghidra's / longer than it plus the padding the next symbol's alignment explains |
| `RELOC` | a pointer names another target, or ours holds a number where the original relocates (`raw` in the notes when it is the original's own address) |
| `STRING` | a string, or a literal a pointer names, differs in content |
| `VALUE` | other bytes differ (`FLOAT` in the notes for a float or double) |
| `BSS_INIT`, `ALIGN` | for an original `.bss` item only size and alignment compare: ours carries bytes / ours is less aligned than an 8-byte scalar needs |

A floating-point constant of `.rdata` that our code reads as a compiler literal
rather than by name is judged by the literals of its readers' objects (kind
`const`): a miss means our code computes with another value. `fld1`/`fldz` serve
1.0 and 0.0 without a literal, so those two never count as a miss. Each defect
says whether code or data of ours references the symbol: a wrong copy nothing
references is dead weight, not a fault.

The headline counts the initialised game data: items no symbol of ours pairs with
count as uncompared when a game function reads them; items only the runtime reads,
import slots and compiler EH/RTTI data are left out, as the function figure leaves
the runtime out. Never quote its counts in prose; read the file.

## Per-module figures

`recon/runs/build-vc8.sh` links the exe from one static library per module,
as the CMake build does (`--flat` links the objects instead, in the same
order), so the linker map says which module library every function of ours
came from. `oracle permodule --map <build>/d2recon-vc8.map` turns that, the
audit and the module ground truth into `evidence/permodule.tsv` and
`evidence/permodule.md`, a module per row:

| Column | What it counts |
|-|-|
| `functions`, `bytes` | M's functions: the original functions of the headline's universe (no C runtime, prebuilt library or compiler EH code) that `docs/data/attribution-v2.tsv` gives to M, else the `runs-v2.tsv` band they lie in. attribution-v2 has no crashy, Game or D2Hell (it counts them as Fog, Fog, Bnclient): a function whose counterpart is in one of those three takes that module. The rows add up to the headline |
| `match`, `match_bytes` | M's functions that MATCH, and their bytes |
| `ident_bytes` | the audit's `ident_bytes` summed: bytes in instructions identical to ours, the whole function for a MATCH |
| `bands` | M's 1.14d address bands: its `runs-v2.tsv` rows; for crashy, Game and D2Hell each run of two or more of their functions |
| `in_lib`, `in_other_lib`, `unpaired`, `not_linked` | where M's functions' counterparts are: in the M library of the exe, in another module's library (filed in the wrong module), without a counterpart, or paired but not in the exe (dropped by `/OPT:REF`; 0 to a few with the default `/OPT:NOREF`) |
| `lib_functions`, `lib_foreign` | the functions the M library puts in the exe, and how many of them are no counterpart of one of M's |
| `order_lcs` | the longest common subsequence of M's functions in the original's address order and in ours: the longest run of them, not necessarily adjacent, that both images lay out in the same order (a longest increasing subsequence of our addresses). `order_lcs / functions` is the ordered-match ratio |
| `same_offset` | M's functions at the same distance from the start of their module as in the original. Ours: from the lowest address of any function of the M library. The original's: from the first function of M's band, a later band counting on from where the one before ends (its last function's end, rounded up to 16), as if M's bands were one. Only a counterpart inside the M library counts. Padding counts: one function of another size, or one dropped, moves every later one |
| `longest_match_run`, `run_start` | the longest stretch of M's functions, consecutive in the original's address order, that all MATCH, and where it starts |
| `identical` | `yes` when every one of M's functions MATCHes and sits at its offset and the library holds nothing else: the whole module is the original's code laid out the original's way, the target a module can reach |

`order_lcs` measures the order of objects and of functions inside them,
`same_offset` adds every size and every gap, so it stays near zero until a
module's leading functions all match in order and size. The original keeps
functions nothing calls and folds no identical ones, so build-vc8.sh links
with the original's options, `/OPT:NOREF /OPT:NOICF /DEBUG /RELEASE
/SAFESEH:NO` (its header gives the evidence; `--opt-ref` is the earlier
`/OPT:REF /OPT:ICF` link, which dropped 2319 of them as `not_linked`). On
v2015 that took order from 4211 to 5076 and same-offset from 4 to 34 at the
same MATCH. What separated the rest was mostly object order: inside a TU 94%
of the functions are in the original's order, but our objects sat in each
library in path order, of which only 127 of 438 TUs follow the original's.
Each module's units are now in the original's order: project.json
`modules.<m>.unitOrder: "original-address"` has the generator place each unit
by the 1.14d address of its first function inside the module's runs
(`moduleRuns`), and write the list to `Source/modules.cmake` as
`D2_MODULE_<m>_UNITS`, each unit with that address and the number of runs its
functions form. The CMake module libraries list their sources in it (GNU ld
links an archive's members whole, in member order), and `zig/src/modules.zig`
reads it for build-vc8.sh's libraries and for `ours.link_order`, so the link
and the COMDAT pairing agree. On v2015 that took order from 5076 to 10827 and
same-offset from 34 to 67 at the same MATCH. What limits it now: 245 of 438
units are split over several runs in the original (an inlined helper kept in
another object, a namespace that was several source files), and a unit takes
one place. The original's bands also show how it was linked: Fog, Storm and D2Launch each
have a second band after the C runtime (and D2Direct3D sits there too), which
is where a library member lands that only a later pass of the linker pulls in.

## Flags

The default (`DEFAULT_CFLAGS` in `zig/src/compile.zig`; `audit.zig`'s `Config` reads the same settings):

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
asm_matching/msvc8/oracle abcompare asm_matching/msvc8/evidence/audit.tsv \
    asm_matching/msvc8/build/experiment-sp1-<hash>/audit.tsv
```

A flag that a module was built with goes into `flags.tsv` (path prefix, extra
flags, the evidence), never into the defaults; changing a row recompiles
exactly the TUs it covers.

An experiment builds in `build/experiment-<compiler>-<hash>/`, is audited with
the baseline's function map (same pairs, other code) and writes its audit
there. **Evidence from an experiment is never committed.** `ORACLE_VC=rtm`
selects the RTM compiler the same way. The rule lives in
oracle-zig (`compile.configure`, `audit.Config`), so it holds however a stage is run: an
experiment aimed at the baseline directory, or writing into `evidence/`, is
refused.

A precompiled header (default; `ORACLE_PCH=0` turns it off) covers the
prefix every TU parses (`d2_platform.h`, the Windows headers,
`Source/globals/globals.h`), which is most of each compile. `oracle objdiff`
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
asm_matching/msvc8/oracle build Source/D2Common/Units       # recompile what the edit touched
asm_matching/msvc8/oracle check Source/D2Common/Units      # re-judge those functions against evidence/audit.tsv
asm_matching/msvc8/oracle diff InitRightSkill        # what still differs, with the source lines it comes from
```

`oracle check` prints new matches and regressions and exits 1 on a regression;
`--update` writes the new rows into `evidence/audit.tsv` (the summary is
rewritten only by a full `oracle audit`). The diff's source lines come from
CodeView line tables: every TU is compiled with `/Z7`, a tool flag that adds
debug sections and leaves the code alone (checked with `oracle objdiff` over the
whole tree).

A hypothesis about the source - "was it `x >= 0`?", "was this an assertion?"
- is tested on the bench before it touches the tree:

```sh
cp asm_matching/msvc8/build/src/Source/Fog/Src/AsyncData.cpp asm_matching/msvc8/build/scratch/try.cpp   # edit it
asm_matching/msvc8/oracle snippet asm_matching/msvc8/build/scratch/try.cpp --func ASYNCDATA_GetReadSize
```

`oracle snippet` compiles the file with the baseline compiler, flags, response
file and (a private copy of) the precompiled header into `build/scratch/`,
and prints the function beside the original with the status the audit would
give it; without `--func`, every function of the object. The function's
module flags (`flags.tsv`, e.g. D2Client's `/GS-`) apply as in the build:
those of the TU the map files the function under, or of `--tu`.

Finding what to fix is statistics over all pairs, not reading diffs one by
one:

| | |
|-|-|
| `oracle diffgrams` | the most frequent differing stretches, by shape (`cmp R, -N ; jle N => test R, R ; jl N`): a construct written another way than Blizzard did, a flag, a convention |
| `oracle classify` | each non-matching function tagged by kind of difference: `cleanup` (`ret N`), `regargs` (arguments in other registers), `frame`, `saved`, `calls`, `inverted`, `signedness`, `regalloc`, `displacement`, `immediate`, `shape` |
| `oracle tulayout` | does each TU correspond to one original object: its functions in one range, in order, nothing foreign inside |

A difference that the generator produces the same way everywhere is fixed
where it is produced: a ghidra-reconstruct pass, or the Ghidra database
([UPSTREAM.md](UPSTREAM.md) states each rule with its evidence;
`evidence/fixes.tsv` records the runs of the Python fixers that found them,
since retired). A form that only trial compilation finds - a statement order,
a comparison's spelling - is the vc8form permuter's (ghidra-reconstruct's
`scripts/vc8form-permute/`, run by `recon/runs/vc8form-permute.sh` against
this checkout's `oracle-zig trial`/`judge`), which records the winning form as
a Ghidra tag the generator reads.

Our objects lay a TU's functions out in an order of their own: VC8 compiles
a caller only after the same-TU functions it calls, and emits a function
calling one defined further down (or an inline helper) at the end of the
object. The generator writes a file's functions in address order, so the
map (`bracket`, `align`) and `oracle tulayout` order our functions by source line
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
| `zig/` (`oracle-zig`) | the oracle in Zig: every stage and tool - setup helpers, carve, build, map, libmap, crt, audit, data, classify, permodule, check, snippet, diff, the analyses, svg, rich/compid, commit (`zig/README.md`) |
| `scripts/env.sh` | shared settings, drive mapping, `have_zig`/`oracle_zig` |
| `scripts/fetch_reference.sh` | Game.exe 1.14d from Blizzard's patch |
| `scripts/setup_toolchain.sh`, `setup_psdk.sh`, `smoke_test.sh` | the toolchain from Microsoft's packages |
| `scripts/vc8.sh` | run one VC8 tool under wine with the oracle's INCLUDE/LIB |
| `vc8_compat.h`, `shim_include/` | the source the compiler sees, with the VC8 translation |
| `translated/` (public repository only) | the tree's VC8 translation as the export wrote it, with `sources.tsv`: what `oracle build` compiles where no ghidra-reconstruct checkout translates (the public CI) |
