# D2Uni

The classic Mac PowerPC software renderer, reconstructed by hand from the LoD 1.10b classic PEF
(`Diablo2Lod/mac/powerpc/lod/1.10b/Diablo_II`), with the other four PowerPC builds (classic 1.00,
classic 1.13d, Carbon 1.10b, Carbon 1.13d) used to place and cross-check it. D2gfx picks it from
the mode array as mode 2 (`DISPLAYTYPE_SOFTWARE`).

It is one renderer table over two back ends. `SetOption(8, windowed)` picks the back end: full
screen runs DrawSprocket (`dspCore.cpp`), windowed runs QuickDraw (`qdCore.cpp`). That is where
Windows picks a DLL: D2DDraw (full screen) and D2Gdi (windowed). The drawing slots draw into one
8-bit back buffer and are shared by both.

**Mac-only, PowerPC-only.** Every file compiles to nothing unless the Mac PowerPC target's flags
define `D2_OS_MAC` and `D2_ARCH_PPC`; the guard is spelled
`#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC` (the VC8 oracle's translator skips
`!defined(_WIN32)` blocks), placed before any include. Nothing here includes `D2Arch.h`. No CMake
target builds it yet: the default build's glob compiles each file to an empty object. The Carbon
builds' CoreGraphics back end (`cgCore.cpp`) is not reconstructed.

## Names: what is attested, what is inferred

No PowerPC string names a folder, and only one D2Uni file:

|file|evidence|
|-|-|
|`Src/uniSmack.cpp`|**attested**: the `__FILE__` string `uniSmack.cpp` (1.10b `103f5614`) its functions pass to Fog's allocator and Storm's `SMemFree`, with lines 95, 112, 147 and 256|
|`Src/uniEngine.cpp`|inferred: the first object in link order (Bink player, drawing primitives)|
|`Src/uniSprite.cpp`|inferred: read last to first, its functions follow D2DDraw `ddSprite.cpp`|
|`Src/dspCore.cpp`|inferred: the code's own `DSp*:` log prefix, the Windows `<prefix>Core.cpp` pattern|
|`Src/uniCore.cpp`|inferred: the slot layer, in D2Gdi `gdiCore.cpp` order|
|`Src/qdCore.cpp`|inferred: the code's own `QD*:` log prefix|
|`Src/uniInternal.h`, `D2Uni.h`|ours|

The module name follows the code's own `Uni*` log prefix (`UniDetect`, `UniPlayBinkFile`); the
Video Wizard calls it "Software". Function names with a log string of their own (`UniDetect`,
`DSpBlit`, `QDCreateContext`, ...) are evidence; the rest are slot names with the back-end prefix,
or descriptive. `DSpDeactivateSurface`, `DSpCloseContext`, `DSpSwapContext` and `DSpCreateContext`
exist only inlined but log under those names. Each function carries its address in all five
builds (`// ppc 1.10b: ...`). The evidence is in `d2-surveys/macppc/uni/` (`map.tsv`, `files.md`,
`structs.md`, `api.md`).

## Link order and source order

The objects sit in link order uniEngine, uniSmack, uniSprite, dspCore, uniCore, qdCore (cgCore in
Carbon), confirmed by the string pools. MWCC emits each object's functions last to first, so each
file lists its functions from the highest address down; `uniSmack.cpp`'s `__FILE__` line numbers
agree.

## The function table

`UniFunctionTable` is the shared `D2RendererFunctionsStrc` of `Source/D2gfx/D2gfx.public.h`, the
type the Windows renderers fill. The shipped Mac table has 57 slots: the shared 54 with three
Mac-only slots (ProcessEvent, GlobalToLocal, LocalToGlobal) after slot 10 (56 in classic 1.00,
which lacks DrawClippedImage). The three are `UniMacFunctionTable`, a `D2MacRendererSlotsStrc`
declared in `D2Uni.h`, so the shared type is not forked.

## The Mac stub headers

`Redivivus/Platform/MacClassic/MacClassicUni*.h` are **stubs owned by Redivivus**, not Apple or
RAD headers: the Toolbox, DrawSprocket, Bink and Smacker names D2Uni uses (the CFM import lists in
`d2-surveys/macppc/uni/api.md`), written from the public documentation, with only the record
fields D2Uni touches. They exist so the module can be syntax-checked without the old SDKs. Bink
and Smacker calls that `Source/D2Headers/d2_platform.h` already declares for every target are not
redeclared. A parallel D2Rave branch carries its own stub set in the same folder; the two should
be folded into one when both land.

## Checking

Modern clang has no `powerpc-apple` target, so the check is the i386 fallback with the PPC
macros forced:
`clang++ -std=gnu++17 -fsyntax-only -target i386-apple-macos10.13 -DD2_OS_MAC=1 -DD2_ARCH_PPC=1 -I<tree> -I<tree>/Source/D2Headers Source/D2Uni/Src/<file>.cpp`
(0 errors on every file). Without `D2_ARCH_PPC`, and under mingw, each file is an empty object.
