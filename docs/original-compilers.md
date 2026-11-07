# Building with the original compilers

Diablo II 1.14d shipped as two builds of the same source, each from its own compiler:

|Build|Compiler|Identified by|
|-|-|-|
|Windows `Game.exe`|Visual C++ 2005 SP1 (cl 14.00.50727.762), CRT linked statically|Rich header, then the linked CRT read function by function|
|Intel Mac `DiabloII_macho`|Apple LLVM 4.2 (clang-425.0.28, Xcode 4.6.2), i386, `-Os`, libc++|Load commands and the SDK, then reproduced: recompiled functions come out byte-identical|

The goal is that the tree builds with those, the original compilers, and with modern ones.
Where that stands:

|Compiler|What it does with the tree today|
|-|-|
|mingw-w64 g++ (C++17)|Builds and links the whole tree into a runnable `d2.exe`. This is the build below.|
|Visual C++ 2005 SP1|Compiles the tree translation unit by translation unit, and every function is compared byte for byte with the shipped `Game.exe`: [asm_matching/msvc8](../asm_matching/msvc8/README.md). It does not link an executable yet.|
|Apple LLVM 4.2|Identified and reproduced on single functions; the tree does not build for the Mac yet.|
|Visual C++ 2005 x64 (RTM)|Not an original compiler: the x64 cross compiler of the same release, used to count what stands between the tree and a 64-bit build ([below](#count-the-way-to-64-bit-with-visual-c-2005-x64)).|

## Build with Visual C++ 2005 SP1 (Windows, the original compiler)

**Compiler:** `cl.exe` 14.00.50727.762, Visual C++ 2005 with SP1. RTM (.42) matches less.
**Linker:** `link.exe` 8.00.50727.

**Flags** (each one measured, see [asm_matching/msvc8/evidence/flags.md](../asm_matching/msvc8/evidence/flags.md)):

```
cl /nologo /c /O2 /Oy- /Ob1 /EHsc /GS /MT /W1 /D WIN32 /D NDEBUG /D _WINDOWS <file>.cpp
```

|Flag|Why|
|-|-|
|`/O2`|optimised release build; `/O1` matches less everywhere|
|`/Oy-`|frame pointer kept: functions open with `push ebp; mov ebp,esp`|
|`/Ob1`|only functions marked `inline` are inlined|
|`/GS`|stack cookies. The client-side modules (`D2Client`, `D2Win`, `D2gfx`, `D2Sound`, the renderers, `D2Launch`, `D2Multi`, `D2MCPClient`) were built with `/GS-`; [flags.tsv](../asm_matching/msvc8/flags.tsv) has the list|
|`/MT`|the VC8 CRT linked statically; `Game.exe` imports no `MSVCR80.dll`|
|`/EHsc`|C++ exceptions|

The original was linked without identical-code folding (`/OPT:NOICF`); identical small
functions exist several times over in `Game.exe`.

The tree is C++17 and VC8 is a C++03 compiler, so the sources go through a line-preserving
translation first (nested namespaces, `[[gnu::packed]]`, defaulted constructors and the like),
with `vc8_compat.h` forced in and the oracle's shim headers ahead on the include path. The
oracle does all of that, and runs the compiler under wine on Linux and macOS:

```sh
asm_matching/msvc8/oracle setup     # once: Game.exe 1.14d from Blizzard's patch, VC8 from Microsoft's packages (~4 GB)
asm_matching/msvc8/oracle all       # compile every translation unit, pair the functions, compare
asm_matching/msvc8/oracle build Source/D2Common/Environment   # recompile one source file after an edit
asm_matching/msvc8/oracle diff <function>              # where one function differs from 1.14d
```

The objects land in `asm_matching/msvc8/build/obj/`. The full guide:
[asm_matching/msvc8/README.md](../asm_matching/msvc8/README.md).

## Build with Apple LLVM 4.2 (Mac, the original compiler)

**Compiler:** Apple LLVM 4.2, `clang-425.0.28`, from the *Command Line Tools (OS X Lion) for
Xcode 4.6.2*. GCC 4.2, the other compiler Apple shipped then, is ruled out: the Mac build uses
libc++, which GCC 4.2 cannot.
**SDK:** Mac OS X 10.7 (Lion). **Linker:** Apple `ld64`, with libc++ linked dynamically.

**Flags:**

```
clang -c -arch i386 -Os -mmacosx-version-min=10.7 -isysroot <Lion SDK> \
      -fstack-protector -stdlib=libc++ <file>.cpp
```

|Flag|Why|
|-|-|
|`-arch i386`|the Mac build is 32-bit Intel|
|`-Os`|size-optimised. On recompiled functions `-Os` reproduces the bytes where `-O2` does not|
|`-mmacosx-version-min=10.7`|the minimum OS the binary's load commands carry|
|`-fstack-protector`|stack guards, as in the shipped binary|
|`-stdlib=libc++`|its C++ runtime is libc++ (`std::__1`), not libstdc++|

Getting the compiler on a current Mac: it is in Apple's `xcode462_cltools_10_76938260a.dmg`.
Expand the packages with `pkgutil --expand-full`; nothing needs installing. `DeveloperToolsCLI.pkg`
holds `usr/bin/clang`, and `DevSDK.pkg` holds the Lion headers to pass as `-isysroot`. On Apple
silicon it runs as x86_64 under Rosetta: `arch -x86_64 <path>/usr/bin/clang ...`.

This compiler takes C++11 at most (`-std=c++11`), and the tree is C++17. It also has no Mac
platform layer yet, so today it compiles single functions, not the tree.

## Count the way to 64-bit with Visual C++ 2005 x64

The original is a 32-bit program, and the tree still keeps pointers in `int` where the original,
or the decompiler, did. On x86 both are 32 bits; on x64 that cast truncates. Visual Studio 2005
has an x64 compiler of the same generation as the one that built 1.14d, and with `/Wp64` it
reports each such cast:

|Warning|Meaning|Example|
|-|-|-|
|C4311|a pointer converted to an integer narrower than a pointer on x64|`(int)pUnit`|
|C4312|an integer converted to a pointer larger than it|`(D2UnitStrc*)nValue`|

Each one is a value typed as an integer that carries a pointer. The fix is the type, made in the
Ghidra database the tree is generated from: the real pointer type, or `intptr_t` where the original
really computes with the bits (a pointer tagged in its sign bit, for one).

**Compiler:** `cl.exe` 14.00.50727.42 for x64, the x86-hosted cross compiler (`x86_amd64`) of
Visual Studio 2005 **Professional**, RTM. Express has no x64 compiler. **Libraries:** its
`VC\lib\amd64` and the bundled Platform SDK's `Lib\AMD64`, used only to link test programs; the
count does not link.

**Getting it,** from your own copy of the Visual Studio 2005 Professional DVD image
(`en_vs_2005_pro_dvd.iso`, MD5 `d8391b57b355def0530eceb03dc41ba4`, the image the oracle's
setup already uses for the Platform SDK). Nothing is installed and no installer runs; no Microsoft
file is part of this repository. With the oracle set up (`asm_matching/msvc8/oracle setup`):

```sh
asm_matching/msvc8/x64/setup.sh
```

It reads `vs/vs_setup.msi` on the image, takes the 231 files under `VC\bin\x86_amd64`,
`VC\lib\amd64` and `VC\PlatformSDK\Lib\AMD64` out of their cabinets by MSI file key, and lays
them out in the oracle's wine prefix next to the x86 compilers, which it does not touch:

```
C:\vc8\rtm-x64\bin      cl, c1, c1xx, c2, link, lib, dumpbin, editbin, ml64 (+ 1033\ messages)
                        and the x86 mspdb80.dll, msobj80.dll, mspdbcore.dll, mspdbsrv.exe, cvtres.exe
C:\vc8\rtm-x64\include  the RTM headers (they serve both targets)
C:\vc8\rtm-x64\lib      VC\lib\amd64
C:\vc8\psdk-x64\lib     VC\PlatformSDK\Lib\AMD64 (headers: the x86 C:\vc8\psdk\include)
```

On Windows with Visual Studio 2005 Professional installed, the same compiler is
`VC\bin\x86_amd64\cl.exe` (the "x64 Cross Tools" command prompt).

**Running the count:**

```sh
asm_matching/msvc8/x64/count     # about 9 minutes; writes asm_matching/msvc8/evidence/x64.json
```

It translates the tree for VC8 as the oracle does and compiles every translation unit with

```
cl /nologo /c /Zs /O2 /Oy- /Ob1 /EHsc /GS /MT /W3 /Wp64 /D WIN32 /D NDEBUG /D _WINDOWS <file>.cpp
```

`/Zs` stops after semantic analysis, where both warnings are issued, so no code is generated.
`x64.json` holds the source lines with C4311, with C4312 and with either (`lines`, the figure
the README shows), and the translation units that do not compile for x64 at all. The full list
of diagnostics is in `asm_matching/msvc8/build/x64/out/diag.tsv`. Details:
[asm_matching/msvc8/x64/README.md](../asm_matching/msvc8/x64/README.md).

Things to know:

* C4311 and C4312 need `/Wp64` on this compiler; without it `(int)p` is silent, even at `/W4`.
* x64 has no inline assembly: `__asm` is error C4235. The count gives the two `__asm` helpers in
  `vc8_compat.h` x64 bodies.
* `__stdcall`, `__fastcall` and `__thiscall` are accepted on x64 and ignored.
* A Win32 `CONTEXT` has other registers on x64 (`Rip`, `Rsp`, `FltSave` for `Eip`, `Esp`,
  `FloatSave`). The tree reads them through `D2_CONTEXT_FIELD(context, Eip)` and
  `D2_CONTEXT_X87_AREA(context)` from `Source/D2Headers/d2_platform.h`, which name the right
  member on either target, so the crash handler and the debug dumps compile for both.
* Sizes that are in the original's data formats (packets, saves, `.bin` tables) do not change
  with the pointer size. The tree's size checks on in-memory structures are written
  `sizeof(void*) != 4 || ...`, so they hold the 32-bit layout without blocking a 64-bit build.
