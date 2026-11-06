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
