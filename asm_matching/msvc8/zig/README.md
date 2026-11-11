# oracle-zig

The oracle in Zig: one executable, a subcommand per stage or tool. Each replaced its Python
script once its output equalled the script's byte for byte, and the script was then deleted;
`oracle` and the setup scripts need zig and stop without it. What is still Python: `build.py`
with `vc8_translate.py` and the modules it imports, for a machine with no ghidra-reconstruct
checkout. The Python fixers are retired: their rules are ghidra-reconstruct's.

```sh
zig build --release=fast                   # zig-out/bin/oracle-zig (capstone 5.0.7 from source)
oracle-zig carve [Game.exe] [functions.tsv]
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
oracle-zig tucache lookup|store|prune [...]   # recon/runs/link.sh and measure.sh
oracle-zig trial --root <asm_matching/msvc8> --state | --out DIR [-j N] [--batch N] [--expect KEY] < trials
oracle-zig judge --root <asm_matching/msvc8> < requests
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

`quick` runs `oracle build` for its build step (oracle-zig's or build.py, whichever that
picks) and itself for map, libmap and audit. `mpq-extract` inflates zlib sectors and
explodes PKWARE ones like the script; a bzip2 sector, which no member of the 1.14d patch
has, it refuses (the script decoded it). `model.pkl` is no longer written by anything but
the Python modules the fixers import (`audit.load_context`); every tool here decodes the
original itself, in about a second.

`oracle crt`, `oracle map` and `oracle libmap` run these when zig is installed. `map` and
`libmap` skip the derivation when nothing they read changed (`stage.zig`: the inputs, this
executable among them, hash to the key `<build>/<stage>.zig-stage` recorded, and the outputs
are the ones that run wrote); the Python's pair memos and library pickle have no counterpart,
since a cold run takes a few seconds. `ORACLE_NOCACHE=1` runs them afresh.

`audit` reads its settings as `scripts/oracle_config.py` does (`ORACLE_VC`, `ORACLE_CFLAGS`,
`ORACLE_BUILD`, and `VC8_WINEPREFIX`/`VC8_TOOLCHAIN` for the runtime libraries), and writes
`audit.tsv`, `audit.tsv.summary` and `datamap.tsv` beside it. It caches nothing: decoding
the original and every object takes about as long as reading the Python's pickles did.

`build` is `build.py`: the translation stays ghidra-reconstruct's TypeScript CLI (`GR`, else
`~/code/ts/gr-daemon`), driven over the same pipe; with no such checkout `oracle build` runs
build.py, whose vc8_translate.py needs none (`oracle-zig build --probe` decides). build/src,
the objects, their `.d`/`.log`/`.flags`/`.key`, `quarantine.tsv`, `compile_status.tsv` and the
cache entries (`ORACLE_CACHE`, manifests and `meta.json` in `json.dumps` form) are the
Python's, so the two read each other's cache. What it leaves out is build.py's decoder pool
(`--decode-jobs`, accepted and ignored): it only warmed the Python readers' pickles.
cl.exe stamps each object with the time and the precompiled header with a fresh signature,
so two clean builds differ in those bytes whichever driver ran them.

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
|`ts_translate.zig`|`ts_translate.py`|the ghidra-reconstruct translator process|
|`proc.zig`| |child processes (posix_spawn, both pipes, timeout), mutex, flock, os.walk/os.path, Python str rules over latin1|
|`tucache.zig`|`recon/runs/tucache.py`|the per-TU gcc result cache|
|`permute.zig`|(permute.py's Compiler and Oracle)|`trial` and `judge`: the form permuter's compiles and judging|
|`cli.zig`| |argparse-style command lines, where the oracle is, stdout as Python prints Latin-1 and UTF-8 text|
|`context.zig`|`audit.load_context`|the original carved and decoded, the symbol map, one object's functions|
|`quick.zig`|`quick.py`|the incremental measurement, gained/lost MATCH, `--verify`|
|`check.zig`|`check.py`|the carve's known extents, re-judging some TUs, `--update`|
|`asmdiff.zig`|`asmdiff.py`|one function side by side (`compare.sideBySide`, `differingLines`), its source lines|
|`snippet.zig`|`snippet.py`|compile a variant with the tree's response file and a copy of the precompiled header|
|`tools.zig`|`objdiff.py`, `diffgrams.py`, `sizetrend.py`, `abcompare.py`, `tulayout.py`|the analyses over the audit|
|`stamps.zig`|`decode_rich_header.py`, `compid.py`|the Rich header and @comp.id stamps|
|`svg.zig`|`byte_match_svg.py`|the byte kinds bar chart|
|`commit.zig`|`commit.py`|the branch's commit split|
|`x64.zig`|`x64/compile64.py`, `x64/count`, `x64/setup.sh`|the x64 /Wp64 count: compile, compat header, x64.json, the toolchain layout|
|`mpq.zig`|`mpq_extract.py`|the MPQ reader and PKWARE DCL explode|
|`layout.zig`|`msi_paths.py`, `vc8_layout.py`, `setup_psdk.sh`|the toolchain trees from the installer cabinets|
|`projectjson.zig`|`publish-kit/tools/public-project-json.py`|the public module table, as `json.dumps(indent=2)`|

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
