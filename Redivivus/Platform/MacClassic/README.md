# `Redivivus/Platform/MacClassic/` - stub headers for the classic Mac OS PowerPC builds

These are **stubs, written by Redivivus**, not Apple or RAD headers. They declare only the
names the PowerPC renderer modules `Source/D2Rave` and `Source/D2Uni` use, so they can be
syntax-checked without the Apple SDK. Nothing is copied from any vendor header: layouts and signatures come from the
public documentation of each call, and the set of names from the CFM import lists of the
classic 1.09d `Diablo_II` PEF (D2Uni's from the classic LoD 1.10b PEF).

|Header|Stands in for|
|-|-|
|`MacClassicToolbox.h`|QuickDraw, Window, Event and Memory Manager (InterfaceLib)|
|`MacClassicRave.h`|QuickDraw 3D RAVE ("QuickDraw(TM) 3D Accelerator", 8 weak imports, the draw-context methods)|
|`MacClassicDrawSprocket.h`|DrawSprocketLib|
|`MacClassicBink.h`|the Mac "Bink Shared Library"|
|`MacClassicUniToolbox.h`|the Toolbox calls D2Uni's QuickDraw back end uses (InterfaceLib)|
|`MacClassicUniDrawSprocket.h`|the DrawSprocketLib calls of D2Uni's DSp back end|
|`MacClassicUniRad.h`|the three Bink calls D2Uni needs that `d2_platform.h` does not declare|

The two sets were written in parallel and overlap in some type definitions; they are kept apart
until they are folded into one.

Every file compiles to nothing unless the target is classic Mac PowerPC: the guard is
`#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC`, with the axes from compiler flags. The
folder is left out of every CMake build (CMakeLists.txt) and of the VC8 oracle (`NOT_GAME` in
`asm_matching/msvc8/zig/src/compile.zig`).

`cmake/check-macppc.sh` is the check both modules pass. A D2Rave unit that needs only these
stubs is checked as

```
clang++ -target powerpc-unknown-none -ffreestanding -fsyntax-only -std=gnu++17 \
    -DD2_OS_MAC=1 -DD2_ARCH_PPC=1 -I. Source/D2Rave/Src/<file>.cpp
```

`powerpc-unknown-none` is a 32-bit big-endian PowerPC target; current clang has no
`powerpc-apple-macos` triple. D2Uni and D2Rave's table unit include shared tree headers, which
need a hosted C++ library, so they fall back to `-target i386-apple-macos10.13` with the host SDK
and the same two axes.

RAVE tags 16, 31, 32 and 46 are not named here on purpose: D2Rave's behaviour does not pin them,
and they have not been checked against the public header.
