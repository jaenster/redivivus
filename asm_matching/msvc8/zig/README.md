# oracle-zig

The oracle in Zig: one executable, a subcommand per stage or tool. Each replaced its Python
script once its output equalled the script's byte for byte, and the script was then deleted;
`oracle` and the setup scripts need zig and stop without it. Nothing is Python any more: the
last scripts (`build.py`, `vc8_translate.py` and what they imported) went once the public
repository shipped its VC8 translation (`translated/`), so no checkout needs them. The
Python fixers are retired: their rules are ghidra-reconstruct's.

```sh
zig build --release=fast                   # zig-out/bin/oracle-zig (capstone 5.0.7 from source)
oracle-zig carve [Game.exe] [functions.tsv] [confirmed-starts.tsv]
oracle-zig audit --root <asm_matching/msvc8> [--funcmap F] [--libmap L] [--out O]
oracle-zig crt --root <asm_matching/msvc8> [--out F]
oracle-zig map --root <asm_matching/msvc8> [--out F]
oracle-zig libmap --root <asm_matching/msvc8> [--exe-map F] [--funcmap F] [--out F]
oracle-zig objects --root <asm_matching/msvc8> [--funcmap F] [--out F]
oracle-zig modules order --repo <tree> < tus
oracle-zig modules includes [--noref] <list>   # recon/runs/build-vc8.sh's /INCLUDE symbols
oracle-zig data --root <asm_matching/msvc8> [extents|strings|audit] [options as datamatch.py]
oracle-zig classify --root <asm_matching/msvc8> [--out O] [--tag T] [--single]
oracle-zig permodule --root <asm_matching/msvc8> --map M [--audit A] [--data D] [--out O]
zig build test                             # the unit tests (datamatch's literal cases among them)
oracle-zig build --root <asm_matching/msvc8> [-j N] [--batch N] [--force] [--with-data]
                 [--extra DIR --support "A.cpp B.cpp"] [PATTERN ...]
oracle-zig translate --repo <tree> <out>        # the VC8 translation with sources.tsv (the export's translated/)
oracle-zig tucache lookup|store|prune [...]   # recon/runs/link.sh and measure.sh
oracle-zig trial --root <asm_matching/msvc8> --state | --out DIR [-j N] [--batch N] [--expect KEY] < trials
oracle-zig judge --root <asm_matching/msvc8> < requests
oracle-zig try-source [--root R] <address> <body file> [--helpers F] [--expect STATUS] [--context N]
oracle-zig try-serve [--root R] [--idle SECONDS] [--max-requests N]
```

The inspection tools and helpers keep their scripts' command lines; `--root` defaults to
`$ORACLE_ROOT`, else the oracle this executable was built in (`zig/zig-out/bin/..`):

```sh
oracle-zig quick [-j N] [--verify] [--exe-map F]          # quick.py: build, map, libmap, audit
oracle-zig check [--update] [PATTERN ...]                 # check.py: the regression gate
oracle-zig diff [--funcmap F] <name|addr>                 # asmdiff.py
oracle-zig snippet FILE [--func NAME [--ref ADDR]] [--cflags FLAGS] [--tu TU]   # snippet.py
oracle-zig objdiff <objdir A> <objdir B> [PATTERN ...]    # objdiff.py
oracle-zig diffgrams [--top N] [--max-len K] [--examples] # diffgrams.py
oracle-zig aligndump [--audit A] [--out FILE] [ADDR ...]   # every non-MATCH function aligned, JSON lines
oracle-zig soleblock [--funcmap F] [--libmap L] [--ratchet DIR --ratchet-report T] align.jsonl OUT   # soleblock.py
oracle-zig sizetrend [--audit A] [--funcmap F] [--out O] [--label T]   # sizetrend.py
oracle-zig abcompare <baseline> <experiment> [--depth N] [--list]      # abcompare.py
oracle-zig tulayout [--tu PATTERN] [--intruders]          # tulayout.py
oracle-zig svg [--summary S] [--out O]                    # byte_match_svg.py
oracle-zig rich <image>                                   # decode_rich_header.py
oracle-zig compid <obj|lib> ...                           # compid.py
oracle-zig commit [--base B] [--tooling LINE] [-m MESSAGE]   # commit.py
oracle-zig mpq-extract <LODPatch_114d.exe> <member> <out>    # mpq_extract.py (fetch_reference.sh)
oracle-zig msi-paths [--cabs] <package.msi>               # msi_paths.py (setup_toolchain.sh)
oracle-zig vc8-layout <paths.tsv> <keyed-dir> <dest>      # vc8_layout.py (setup_toolchain.sh)
oracle-zig psdk-layout <psdk.tsv> <keyed-dir> <dest> <prefix>   # setup_psdk.sh's layout
oracle-zig project-json <recon/project.json> <out>        # publish-kit public-project-json.py
oracle-zig x64 compile <src> <out> --oracle <dir> [--compat F] [--zs] [--jobs N] [--batch N] [PATTERN ...]
                                                          # x64/count's compiler driver (compile64.py)
oracle-zig x64 compat <vc8_compat.h> <out>                # x64/count's x64 copy of the compat header
oracle-zig x64 json <out-dir> <x64.json> <sha> <ghidra> <banner>   # x64/count's evidence file
oracle-zig x64 layout <x64.tsv> <keyed-dir> <dest> <prefix>       # x64/setup.sh's layout
```

`quick` runs `oracle build` for its build step and itself for map, libmap and audit. `mpq-extract` inflates zlib sectors and
explodes PKWARE ones like the script; a bzip2 sector, which no member of the 1.14d patch
has, it refuses (the script decoded it). `model.pkl` is no longer written by anything but
the Python modules the fixers import (`audit.load_context`); every tool here decodes the
original itself, in about a second.

`oracle crt`, `oracle map` and `oracle libmap` run these when zig is installed. `map` and
`libmap` skip the derivation when nothing they read changed (`stage.zig`: the inputs, this
executable among them, hash to the key `<build>/<stage>.zig-stage` recorded, and the outputs
are the ones that run wrote); the Python's pair memos and library pickle have no counterpart,
since a cold run takes a few seconds. `ORACLE_NOCACHE=1` runs them afresh.

`audit` reads its settings as `compile.zig` does (`ORACLE_VC`, `ORACLE_CFLAGS`,
`ORACLE_BUILD`, and `VC8_WINEPREFIX`/`VC8_TOOLCHAIN` for the runtime libraries), and writes
`audit.tsv`, `audit.tsv.summary` and `datamap.tsv` beside it. It caches nothing: decoding
the original and every object takes about as long as reading the Python's pickles did.

`build` was `build.py`. The translation stays ghidra-reconstruct's TypeScript CLI (`GR`,
else `~/code/ts/gr-daemon`), driven over a pipe; with no such checkout it copies the
translated tree an export ships (`translated/`, or `ORACLE_TRANSLATED`) after checking its
`sources.tsv` against the checkout, and with neither it stops (`oracle-zig build --probe`
exits 0 when there is one). `translate` writes such a tree: the translator's mirror and the
SHA-256 of each source. A shipped tree blanks nothing, so a TU VC8 cannot compile fails
there instead of being quarantined, and a cached compile that was quarantined is not
restored from it. build/src, the objects, their `.d`/`.log`/`.flags`/`.key`,
`quarantine.tsv`, `compile_status.tsv` and the cache entries (`ORACLE_CACHE`, manifests and
`meta.json` in `json.dumps` form) are laid out as the Python's were, so a cache it wrote
still serves. `--decode-jobs` (the Python's decoder pool) is accepted and ignored.
cl.exe stamps each object with the time and the precompiled header with a fresh signature,
so two clean builds differ in those bytes.

`tucache` is `recon/runs/tucache.py` (the per-TU gcc cache of link.sh and measure.sh), same
keys, manifests and objects; `tucache probe` exits 0 so a script can tell it is there.

`trial` and `judge` are the oracle side of ghidra-reconstruct's vc8form form permuter
(`scripts/vc8form-permute/permute.ts`); they have no Python original of their own (permute.py
imported build.py and audit.py for them). `trial` compiles translated trial sources written
beside their TU in build/src, with the build's command line, flags.tsv flags and precompiled
header, into a scratch directory, under the build lock and only while `--state`'s key still
holds. `judge` stays up and judges one object per request against the original as `audit`
does (same views, same `compare`), with the INSN_MATCH register-only test and the masked
original bytes (`bh`) a vc8form tag records.

`try-source` judges one hand-written function body: it replaces the definition after the
function's `// 1.14d:` anchor in a copy of its TU, has the translator translate the copy, and
compiles and judges it as `trial` and `judge` do, beside the TU's unchanged copy for the
`baseline` and `collateral` fields. Its reply is one JSON line: the status, the counts, the
frames, the differing lines of `diff`'s side-by-side view and every other function of the TU whose
status moved. Callers share `try-serve`, which the first one starts (POSIX hosts): one queue on
`build/try-source.sock` that batches whatever arrives together into one trial round, under the
build's lock and `WINE_BENCH_LOCK` when set, `D2_VC8_JOBS` (6) cl.exe at a time.

## Modules

One file per Python module (most of them since deleted), named after it; the functions keep the Python names in
camelCase, so a port reads side by side with its original.

|File|Python|What|
|-|-|-|
|`main.zig`| |subcommand dispatch|
|`util.zig`| |the process arena (`util.gpa`; nothing is freed), files via libc stdio, tab-separated lines, `%.Nf` via snprintf|
|`pe.zig`|`peimage.py`|PE32 image: sections, base relocations, imports, exports|
|`decode.zig`|`funcdecode.py`|one function by recursive descent; the caller's context answers `relocAt`/`externalAt`; capstone text kept on request (`Decoder.init(allocator)`)|
|`refimage.zig`|`refimage.py`|the carve, `functions.tsv` load/write, `refsOf`, `stringAt`, and `model()` (what `model.pkl` holds)|
|`coff.zig`|`coffobj.py`|COFF objects and archives|
|`objfuncs.zig`|`objfuncs.py`|the functions of one object, relocations resolved; `qualified`, `lineTables`|
|`ours.zig`|`ours.py`|every function of the completed objects (os.walk order), `layoutKey`, `linkKept`|
|`libmap.zig`|`libmap.py`|the runtime libraries' functions, `isImportThunk`, and the pairing: lib-exact, lib-order, lib-similar, lib-votes|
|`crtmap.zig`|`crtmap.py`|the linked CRT, by bytes (RTM and SP1)|
|`funcmap.zig`|`funcmap.py`|every tier of the function map, the vote pass and the drop of contradicted pairs|
|`objects.zig`|`objects.py`|functions filed apart from their callers (liveness: `classify.liveIn`)|
|`modules.zig`|`modules.py`|`Source/modules.cmake`: `linked`, `moduleOf` (permodule) and the link order (`ours.linkKept` keeps by it); `includes`, the /INCLUDE symbol that forces each library member into the link (`keptSections`, `includeSymbols`)|
|`stage.zig`|`ours.stage_*`|whole-stage reuse by a hash of the inputs|
|`difflib.zig`|`difflib`|`SequenceMatcher(None, a, b, autojunk=False)` over interned ids: same blocks, same ties|
|`compare.zig`|`compare.py`|the masked views (`ourView`, `refViewSpan`), `compare`, the aligned byte counts|
|`audit.zig`|`audit.py`|the audit, the summary, the datamap; `loadFuncmap`, `Config` (`oracle_config.py`)|
|`datamatch.zig`|`datamatch.py`|the data audit (`extents`, `strings`, `audit`); `test_datamatch_literals.py` as its tests|
|`classify.zig`|`classify.py`|the tags of every mapped function that differs; liveness over the control flow (`liveIn`)|
|`permodule.zig`|`permodule.py`|`permodule.tsv`/`.md`: the figures and the layout agreement per module|
|`compile.zig`|`build.py`, `oracle_config.py`|mirror, response file, precompiled header, batches, quarantine rounds, prune, status|
|`objcache.zig`|`objcache.py`|the compile cache; `json.dumps`-compatible writer|
|`ts_translate.zig`|`ts_translate.py`|the ghidra-reconstruct translator process, or the shipped translated tree and its `sources.tsv`|
|`proc.zig`| |child processes (posix_spawn, both pipes, timeout), mutex, flock, os.walk/os.path, Python str rules over latin1|
|`tucache.zig`|`recon/runs/tucache.py`|the per-TU gcc result cache|
|`permute.zig`|(permute.py's Compiler and Oracle)|`trial` and `judge`: the form permuter's compiles and judging|
|`trysource.zig`| |`try-source` and its queue `try-serve`: one hand-written body spliced, compiled and judged|
|`cli.zig`| |argparse-style command lines, where the oracle is, stdout as Python prints Latin-1 and UTF-8 text|
|`context.zig`|`audit.load_context`|the original carved and decoded, the symbol map, one object's functions|
|`quick.zig`|`quick.py`|the incremental measurement, gained/lost MATCH, `--verify`|
|`check.zig`|`check.py`|the carve's known extents, re-judging some TUs, `--update`|
|`asmdiff.zig`|`asmdiff.py`|one function side by side (`compare.sideBySide`, `differingLines`), its source lines|
|`snippet.zig`|`snippet.py`|compile a variant with the tree's response file and a copy of the precompiled header|
|`tools.zig`|`objdiff.py`, `diffgrams.py`, `sizetrend.py`, `abcompare.py`, `tulayout.py`|the analyses over the audit|
|`aligndump.zig`| |the audit's alignment of every function that does not match, as JSON lines|
|`soleblock.zig`|`soleblock.py`|the sole-blocker index over aligndump's output: causes per hunk, ranked by the functions each alone blocks|
|`stamps.zig`|`decode_rich_header.py`, `compid.py`|the Rich header and @comp.id stamps|
|`svg.zig`|`byte_match_svg.py`|the byte kinds bar chart|
|`commit.zig`|`commit.py`|the branch's commit split|
|`x64.zig`|`x64/compile64.py`, `x64/count`, `x64/setup.sh`|the x64 /Wp64 count: compile, compat header, x64.json, the toolchain layout|
|`mpq.zig`|`mpq_extract.py`|the MPQ reader and PKWARE DCL explode|
|`layout.zig`|`msi_paths.py`, `vc8_layout.py`, `setup_psdk.sh`|the toolchain trees from the installer cabinets|
|`projectjson.zig`|`publish-kit/tools/public-project-json.py`|the public module table, as `json.dumps(indent=2)`|

## Windows host inputs (experimental)

The native Windows compile driver requires `VC8_NATIVE_CL` to name an absolute VC8
`cl.exe`. Set `INCLUDE` and `LIB` for that installation as you would for a VC8 command
prompt. `VC8_NATIVE_ROOT` names a VC8 runtime tree with `rtm/lib` and `sp1/lib`;
`crt` and `libmap` require it on Windows. Provide `reference/Game.exe` and use the
checked-in `asm_matching/msvc8/translated` tree. Set `ORACLE_TRANSLATED` to its absolute
path and leave `GR` unset for the published pipeline. The translation's source hashes
are checked before compilation; neither a translator checkout nor Node is needed.
`oracle-zig all --root <asm_matching/msvc8>` runs native build, map, libmap, and audit
in order. `oracle-zig quick --root <asm_matching/msvc8>` runs the incremental pipeline;
both require the VC8 inputs above. On Windows, `all`, `quick`, and `map` carve
`build/ref/functions.tsv` from the local Game.exe when needed; they do not download the image.
These commands check Game.exe against the oracle's pinned 1.14d SHA-256 before use.
Confirmed starts come from `evidence/confirmed-starts.tsv`. The cache's
`functions.tsv.inputs` records the image, evidence and carver-source SHA-256 values;
a cache without that record, or with any of them changed, is carved again
(`oracle-zig ref --root <dir> [--force]`, which the Unix `oracle ref` also runs).
Invalid evidence or a wrong image leaves the existing reference cache intact.
The Unix `oracle` Bash wrapper and its build.py fallback are not used by these native
commands. `all` and `quick` require baseline settings (default `ORACLE_VC=sp1` and no
custom `ORACLE_CFLAGS`) because they write the baseline evidence directory.
Any existing `evidence/crtmap.tsv` is an optional input to this pipeline: `all` does
not run `crt` or replace that input. A clean main checkout has no CRT map; the paired
workflow preserves that absence instead of inserting a different preprocessing step.

The `commit`, `x64` helper, and Unix GCC `tucache` commands require the Unix oracle
environment and report an unsupported-command error on Windows. Native x64
compilation and Unix shell setup scripts are outside this Windows host port.

Windows module ordering uses bytewise lexical sorting, equivalent to Unix `sort(1)`
with `LC_ALL=C`. A unit fixture covers case, punctuation, numeric suffixes, UTF-8 and
duplicates. The paired CI additionally compares the complete current tree's module
order from native Windows with Linux in the C locale.

`oracle-windows.yml` compiles the shipped translation on Linux and Windows at the
same candidate commit, and runs the unmodified main Linux pipeline as a control.
The comparison script requires matching input fingerprints, headline figures,
total MATCH, library figures, module order, and all five evidence TSV digests across
hosts. It also compares main when its shipped inputs are identical, checks that
the CRT input was preserved, and requires zero MISSING/QUARANTINE and a clean address
self-check. Counts and output digests are measured within each run rather than
pinned to one source export. The Windows PR remains experimental until that full
comparison passes; local compilation alone is not a parity claim.

## Porting the next stage

- Read the Python whole and port the algorithm, including its orderings: dict iteration is
  insertion order (use `ArrayHashMap`), `sorted` and `list.sort` are stable (`std.mem.sort`
  is too), `Counter.most_common` breaks ties by first insertion, tuple sorts compare every
  field. Each of these decides the order of some output row or line.
- Text the Python formats with `%.3f`/`%6.2f` goes through `util.fixed` (C's printf, which
  rounds as Python does); hex and integers through `std.fmt`.
- Floats must come out bit for bit: `sum()` of floats is Neumaier-compensated since Python
  3.12 (`funcmap.FSum`), `math.log` is libm's (call it, not `std.math`), and a `set` of
  bytes or strings iterates in a per-process order - where Python's result depends on that
  order it is itself not deterministic (libmap's lib-exact ties of equal length).
- The gate is a byte comparison against the Python on a scratch copy of a real build (the
  objects, `build/ref/functions.tsv`, `evidence/*`): never against the measurement checkout
  itself. Run the Python with `ORACLE_NOCACHE=1` and `ORACLE_CACHE` pointing at scratch.
- Wire the stage in `oracle` (`zigtool`/`zigtool_noroot` there), delete the script, and say
  so in the table.
- Python decodes what it reads from a binary as Latin-1 and text files as UTF-8, and prints
  both as UTF-8: `cli.Out.lat` for the first (names and strings from objects, the listings),
  as is for the second.

`aligndump` writes the audit's own alignment of every function that does not match (ours and
the original, masked bytes, comparable references, our source line or header) plus each unit's
function definition lines and linkage. `soleblock` reads it and ranks diff causes by how many
functions each is the only remaining blocker of (the sole-blocker index): index.tsv, callees.tsv,
functions.tsv, and with `--ratchet` frozen.tsv (ratchet members kept at a body that does not
match):

```sh
oracle-zig aligndump --out align.jsonl
oracle-zig soleblock --funcmap ../evidence/funcmap.tsv --libmap ../evidence/libmap.tsv \
    [--ratchet <recon/modified/ratchet> --ratchet-report <project>/.ghidra-mcp/ratchet-report.tsv] align.jsonl <outdir>
```

