# D2Uni

The classic Mac PowerPC software renderer, reconstructed by hand from the LoD 1.10b classic PEF
(`Diablo2Lod/mac/powerpc/lod/1.10b/Diablo_II`), with the other four PowerPC builds (classic 1.00,
classic 1.13d, Carbon 1.10b, Carbon 1.13d) used to place and cross-check it. D2gfx picks it from
the mode array as mode 2 (`DISPLAYTYPE_SOFTWARE`).

It is one renderer table over two back ends. `SetOption(8, windowed)` picks the back end: full
screen runs DrawSprocket (`dspCore.cpp`), windowed runs QuickDraw (`qdCore.cpp`). That is where
Windows picks a DLL: D2DDraw (full screen) and D2Gdi (windowed). The drawing slots draw into one
8-bit back buffer and are shared by both.

**Mac PowerPC, and the Win32Posix build.** The DrawSprocket and QuickDraw back ends compile to
nothing unless the Mac PowerPC target's flags define `D2_OS_MAC`, `D2_MAC_CARBON` and
`D2_ARCH_PPC` (`#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC`). The shared files -
`D2Uni.h`, `uniInternal.h`, `uniCore.cpp`, `uniEngine.cpp`, `uniSprite.cpp`, `uniSmack.cpp` - also
build on the modern macOS target, `#if !defined(_WIN32) && ((D2_MAC_CARBON && D2_ARCH_PPC) ||
D2_BACKEND_POSIX)`, where the SDL3 back end `sdlCore.cpp` stands in for both (below). The VC8
oracle's translator skips `!defined(_WIN32)` blocks; every guard sits before any include. Nothing
here includes `D2Arch.h`. The Carbon builds' CoreGraphics back end (`cgCore.cpp`) is not
reconstructed.

## The SDL3 back end (Win32Posix build)

`sdlCore.cpp` is ours: no Mac build had an SDL back end. `uniCore.cpp` dispatches to it as gnUniMode
4 (`UNIMODE_SDL`), the way the Carbon builds added CoreGraphics as 3; SetOption(8) always picks it,
since Win32Posix makes the one window full screen or not. It keeps QuickDraw's shape (the 640x480
and 800x600 buffers, the surface teardown, the 32-bit cutscene surface, the palette compare) and
presents by converting the 8-bit back buffer through the palette into an ARGB8888 streaming
texture, scaled into the client area `Win32Posix_GetViewport` gives (aspect kept, nearest), with
vsync when the settings ask. D2gfx picks it as mode 2 on the modern Mac (`D2GFX.cpp`,
`Main.cpp`); D2OpenGL takes mode 5 once it is linked in (`D2_RENDERER_OPENGL`).

Every POSIX difference in the shared files is an `#if D2_BACKEND_POSIX` branch, so the PowerPC
preprocessed output is unchanged byte for byte:

|where|PowerPC|Win32Posix|
|-|-|-|
|`uniInternal.h`|the classic stub headers|`Src/uniPosix.h`: the Toolbox and RAD records the slots read, natively laid out, and the Memory Manager and block calls as inlines|
|`UniGetCel`|the 1.10 `D2GfxDataStrc`, cel in the first word|the 1.14d record's `pDC6Block`|
|the Bink player|refNum open, Sound Manager, OSEventAvail, dirty rectangles|HANDLE open, DirectSound, `SdlInputPending`, the whole frame (the 64-bit Bink keeps only the head: Width, Height, Frames, FrameNum)|
|`UniDecodeSmacker`|SmackOpen on an FSSpec|SmackOpen on the path, as Windows|
|`UniDrawString`|D2Win 1.10b font calls|the 1.14d `SetAndReturnLastFont`, `DRAW_text`, `STRING_ConvertCharToWchar`|
|SetOption 11|`gnUniContrast = nValue`|the value's low 32 bits|

Byte order: the 8-bit buffer and the palette entries are bytes; the converted frame is written as
ARGB words, which is what ARGB8888 means on either byte order. Only the Bink cutscene surface
(B, G, R, A bytes) depends on a little-endian host, and `sdlCore.cpp` asserts it.

The test, `Tests/UniSdlTest.cpp` (`ninja d2_uni_test`, headless on SDL's dummy driver; `--live`
opens a window), drives the table as D2gfx does and checks the back buffer, the screenshot slot
and the presented pixels at 1:1 and scaled.

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
|`Src/sdlCore.cpp`, `Src/uniPosix.h`, `Tests/UniSdlTest.cpp`|ours: the Win32Posix build's back end|

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
`clang++ -std=gnu++17 -fsyntax-only -target i386-apple-macos10.13 -DD2_OS_MAC=1 -DD2_MAC_CARBON=1 -DD2_ARCH_PPC=1 -I<tree> -I<tree>/Source/D2Headers Source/D2Uni/Src/<file>.cpp`
(0 errors on every file); `cmake/check-macppc.sh` runs it over the module. Under mingw each file is
an empty object. On the modern target `ninja d2_uni_objs` builds the shared files and `sdlCore.cpp`.
