# mac/ - the 1.14d Intel Mac oracle, compare stage

The Mac build of 1.14d carries code the Windows executable does not: Storm's Mac
layer (a Win32 API on pthreads and Carbon), the OpenGL renderer, the Carbon
front end. That code is reconstructed by hand under `D2_OS_MAC`, and this tool
measures it against the shipped binary the way `msvc8/` measures the Windows
tree, function by function.

It is the compare stage only. The compile stage is the Mac build itself
(`cmake/mac.cmake`, toolchain `cmake/mac-i386-clang425.cmake`: Apple
clang-425.0.28 from Xcode 4.6.2, the compiler that built the original, with the
10.7 SDK). Linking is not needed: the comparison reads relocations.

## Running it

```
cmake -S . -B build-mac -DCMAKE_TOOLCHAIN_FILE=cmake/mac-i386-clang425.cmake
cmake --build build-mac --target d2_mac_objs
node asm_matching/mac/cmp.ts --build build-mac --binary <DiabloII_macho> [-v] [--tsv out.tsv] [filter...]
```

Node 22.6 or later (it runs the TypeScript directly), and Homebrew `llvm` for
`llvm-objdump`, `llvm-nm` and `llvm-cxxfilt`. The binary is the Intel Mac
`DiabloII_macho` of 1.14d, sha256 `1e8dd6dd...0d640d177`; the tool refuses any
other. `D2_MAC_BINARY` may name it instead of `--binary`. The binary's
disassembly is cached under `asm_matching/mac/build/`. A filter keeps the files
whose path contains it, or the function of that name.

## What it compares

A function or global is paired with the original by the comment on the line
before its definition, `// 1.14d mac: <addr>`, in any source under `Source/`
or `Redivivus/`: StormMac, MacSpecific, the Mac renderer in `Source/D2OpenGL/Src`
and the `D2_OS_MAC` blocks of the shared code. A declaration in a header may carry
the comment too, which names a function or global that lives outside the
reconstructed code. A tagged file `d2_mac_objs` does not build has no object to
compare: its functions are listed as `NOOBJ` in the `--tsv` output and named in
one summary line (`-v` lists the files), and its tags still resolve references.

For each function our object's instructions are compared with the original's,
over the original's whole extent, which the binary's `LC_FUNCTION_STARTS`
bounds. Trailing padding is ignored. Before comparing, both sides are
normalised:

- a branch target becomes the index of the instruction it lands on, so a
  branch to the wrong place is a difference while a shift of addresses is not;
- a call becomes its callee: an imported symbol by name, a reconstructed
  function by name (through its address comment), `call $+5` for the PIC
  thunk;
- a PIC reference, `[reg + disp]` off the register that popped the PIC base,
  becomes what it points at: an imported symbol (`&_name`, through the
  indirect symbol table), a string literal by its text, a paired global plus
  an offset, or a paired function.

A reference that one side can name and the other cannot is counted as
unverified, not as a difference: it is reported beside the result. `SAME`
with no unverified references is instruction-identical including every
callee and data reference. With `-v` each unverified reference is listed
under its function, both sides shown.

## Output

One line per function (`SAME` or `DIFF` with the first differing
instruction), then per source file and for the whole run the count and share
of functions that are instruction-identical:

```
== Source/StormMac/Win32Mac/Source/Win32/Win32_utils.cpp: 40/55 same (72.7%)
== total 77/92 instruction-identical (83.7%)
```

The share is over the functions reconstructed so far, not over the Mac-only
code as a whole. `--tsv` writes the per-function rows for tracking.

## What it does not prove

Instruction identity is not byte identity: an encoding choice (a short versus a
long jump) that prints the same is not seen, and neither is data. Function
order and alignment inside the TU are not compared, and neither is which TU a
function sits in, since that is settled from other evidence (Source/StormMac/README.md).

## Coverage

`cmp.ts` measures the functions that have source. `coverage.ts` measures how much
of the Mac build's own code has source at all:

```
node asm_matching/mac/cmp.ts --build build-mac --binary <DiabloII_macho> --tsv asm_matching/mac/build/cmp.tsv
node asm_matching/mac/coverage.ts [-v]
```

It writes `coverage.tsv`, one row per function of the Mac-only regions:

|column|what|
|-|-|
|`addr`, `name`, `bytes`|the function in the clean Ghidra program|
|`orig_file`|its original file below `Diablo2/Source`, with the Mac folder names' spaces; `?` where nothing names it; `A\|B` where it lies between functions of two files and the boundary is not known|
|`file_evidence`|`__FILE__` (its own literal), `file-map` (`docs/data/mac-file-map.tsv`, for a paired function its Windows twin's file), `bracketed` (between two functions of one file), `between`, `unit` (the unit's file in `coverage/units.tsv`), `override`|
|`redivivus`|where that file lives here: spaces dropped, `StormMac/Storm Mac/` is `Source/StormMac/`|
|`class`|`blizzard-mac` (the goal), `third-party` (crashy, `mach_override`, MoreFiles, zlib, blast, RAD Bink, the CRT start), `shared` (paired with a Windows function, the shared source covers it), `shared-unpaired` (in a shared file, unpaired, but the pairer found a Windows body candidate)|
|`status`|`missing`; `body` (a definition tagged `// 1.14d mac: <addr>` exists); `match` / `nonmatch` (`cmp.ts` SAME / DIFF)|
|`body_file`, `at_path`|where the tagged body is, and whether that is the mapped path|
|`win_twin`|the paired `Game.exe` address|

The summary is the coverage, `blizzard-mac` rows with a body over all `blizzard-mac`
rows, per original file, and the match count. Nothing is read from Ghidra when it
runs. The inputs are checked in under `coverage/`:

- `units.tsv`: the Mac-only regions cut into units, each with its class and file
  or folder and the evidence. A unit of kind `anchors` takes each function's file
  from its own `__FILE__` literal or the file map and brackets the rest.
- `functions.tsv`: the clean program's functions (address, extent, name, the
  `__FILE__` literals each references) with the Windows twin, the pairer's
  candidate count and the file map's path. Rebuild it after a Ghidra naming pass:
  run `coverage/dump.java` read-only through `execute_script` in a session on
  `Diablo2Lod/mac/intel/1.14d/DiabloII_macho` (it puts its output into a daemon
  upload slot), fetch it, then
  `node asm_matching/mac/coverage.ts --import <dump> --docs <workspace>/docs/data`.

A shared file's `D2_OS_MAC` addition counts as `blizzard-mac` when the pairer found
no Windows candidate for it. Unpaired shared code outside the Mac-only regions
(about 785 functions, a pairing job) is not counted.

### Baseline, 2026-11-27

Program `DiabloII_macho` v218. `cmake --build build-mac --target d2_mac_objs`
builds with 0 errors (74 warnings); D2OpenGL and CD2Textures are not in that
target and do not compile under the Mac flags yet (the shared headers clash with
the SDK's `FILE` and lack `errno_t`, and `MacSdkOnWin32.h` wants `windows.h`).
They compile in `d2_mac_objs` since: `MacSdkOnWin32.h` takes the 10.7 SDK on the Mac,
and `cmake/mac-cxx.ts` (the target's compiler launcher) rewrites the shared tree's C++17
nested namespace definitions, which clang-425 cannot parse, for the units that reach them.

The rest of the shared tree (D2Common, D2Game, D2Client, Fog, Storm ...) is in the Mac
binary but not in `d2_mac_objs`. Through the launcher it stops on the Win32 surface, not
on the language: the module headers' `#ifndef _WIN32` type stubs are Windows layouts
declared after their first use (Fog.h needs `in_addr`, `_SYSTEM_INFO`, `__time32_t`
before Safesock.h and SFile.h define them) and clash with the Mac C library (`timeval`,
`fd_set`), and the bodies call Win32 API that neither `d2_platform.h`'s stub nor
`Win32Mac.h` declares (registry, WinSock, SEH `CONTEXT`, IME). Measured 2026-11-27 over
the 453 shared units: 1935 distinct error sites, 68 of them in headers.

|class|functions|bytes|with a body|
|-|-|-|-|
|blizzard-mac|1382|274043|358|
|shared|75|10165|0|
|shared-unpaired|15|610|0|
|third-party|221|108746|0|

Coverage **358/1382 (25.9%)**. `cmp.ts`: **157/193** instruction-identical (the
193 tagged functions `d2_mac_objs` compiles; D2OpenGL's 147 and CD2Textures' 18
bodies have no Mac object yet). Done: D2OpenGL, CD2Textures, `Win32_utils.cpp`,
`Win32_TimeAndDate.cpp`. Largest gaps: the Win32 Mac files with no recorded name
(281), the Storm `SOURCE` files (432 together, `SFILE.CPP` 82), `SRegMac.mm` and
its templates (92), the Mac shell (77).
