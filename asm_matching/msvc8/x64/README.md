# x64: how far the tree is from a 64-bit build

Diablo II 1.14d is a 32-bit program, and the reconstruction inherits every place where the
original, or the decompiler, kept a pointer in an `int`. On x86 the two are the same 32 bits;
on x64 such a cast truncates. Visual C++ 2005 has an x64 compiler of the same generation as
the original's, and with `/Wp64` it reports each of those casts. This directory counts them.

```sh
asm_matching/msvc8/x64/setup.sh     # once, after `oracle setup`: lay out the x64 compiler
asm_matching/msvc8/x64/count        # compile every TU for x64 (/Zs), count, write evidence/x64.json
```

**The numbers are in [../evidence/x64.json](../evidence/x64.json)**, measured at the commit it
names:

|Field|What it counts|
|-|-|
|`c4311`|source lines with C4311: a pointer converted to a narrower integer, `(int)p`|
|`c4312`|source lines with C4312: an integer converted to a larger pointer, `(T*)n`|
|`lines`|source lines with either (a line with both counts once)|
|`tus_failing` / `tus_total`|translation units that do not compile for x64 at all, of all; `failing` names them|
|`commit`, `ghidra`|the commit measured and the Ghidra database version it was generated from|

Each counted line is a value typed as an integer that carries a pointer. The fix is the type,
in the Ghidra database the tree is generated from, not the cast in the output: the right
pointer type when the value is one object, `intptr_t`/`uintptr_t` only where the original
really does arithmetic or tagging on the bits. Some casts stay because they are the original's:
a pointer tagged in its low or sign bit, an address sent over the wire.

## What it runs

`count` translates the checkout for VC8 the way the oracle does (`scripts/vc8_translate.py`),
then compiles each translation unit (`oracle-zig x64 compile`, which needs zig 0.17) with

```
cl /nologo /c /Zs /O2 /Oy- /Ob1 /EHsc /GS /MT /W3 /Wp64 /D WIN32 /D NDEBUG /D _WINDOWS
```

plus the per-module flags of `../flags.tsv`, `../vc8_compat.h` forced in, and
`../shim_include/` first on the include path. `/Zs` stops after semantic analysis, which is
where both warnings come from, so no code is generated. The compiler is
`cl.exe 14.00.50727.42 for x64` (RTM): no Visual Studio 2005 SP1 package with x64 binaries was
used. Settings are the oracle's (`scripts/env.sh`): `VC8_TOOLCHAIN`, `VC8_WINEPREFIX`;
`X64_WORK` moves the work directory (default `build/x64`).

The full diagnostics stay in `build/x64/out/diag.tsv` (tu, file, line, kind, code, message)
and `status.tsv`. A checkout with uncommitted changes is measured too, but written to
`build/x64/x64.json`: the evidence file only ever describes a commit.

Traps:

* C4311 and C4312 need `/Wp64` on this compiler; without it `(int)p` is silent even at `/W4`.
* This compiler has no inline assembly (`__asm` is error C4235). `count` gives the two
  `__asm` helpers of `vc8_compat.h` x64 bodies in a copy it forces in instead.
* `__stdcall`, `__fastcall` and `__thiscall` are accepted and ignored, with no diagnostic.
* The x86-only registers of a Win32 `CONTEXT` (`Eip`, `Esp`, `FloatSave`...) do not exist on
  x64; the tree reads them through `D2_CONTEXT_FIELD(c, Eip)` and `D2_CONTEXT_X87_AREA(c)`
  (`Source/D2Headers/d2_platform.h`), which name `Rip`, `Rsp`, `FltSave` there.

## What `setup.sh` lays out

From the Visual Studio 2005 Professional DVD image the oracle's `setup` already reads for the
Platform SDK (`$VC8_SRC/pro-iso/en_vs_2005_pro_dvd.iso`, MD5-checked), by MSI file key, without
running an installer (`oracle-zig msi-paths` resolves each cabinet member to its path, `oracle-zig x64 layout` places it):

```
C:\vc8\rtm-x64\bin      VC\bin\x86_amd64: cl, c1, c1xx, c2, link, lib, dumpbin, editbin, ml64, 1033\
                        + mspdb80.dll, msobj80.dll, mspdbcore.dll, mspdbsrv.exe, cvtres.exe from C:\vc8\rtm\bin
C:\vc8\rtm-x64\include  a copy of C:\vc8\rtm\include (the headers serve both targets)
C:\vc8\rtm-x64\lib      VC\lib\amd64
C:\vc8\psdk-x64\lib     VC\PlatformSDK\Lib\AMD64
                        (headers: C:\vc8\psdk\include, shared with x86)
```

231 files, from the cabinets `vs/_18273`..`_18275_RTL_x86_enu_AMD64_*.cab` on the image and the
Platform SDK's `*_AMD64*.msm` streams inside `vs/vs_setup.msi`. The x86 oracle's compilers are
not touched. `x64/vc8x64.sh <tool> args` runs one of the tools with `INCLUDE` and `LIB` set:

```sh
asm_matching/msvc8/x64/vc8x64.sh cl /nologo /c /W3 /Wp64 'Z:\path\to\file.cpp' '/FoZ:\path\to\file.obj'
asm_matching/msvc8/x64/vc8x64.sh link /nologo /MACHINE:X64 '/OUT:Z:\path\to\t.exe' 'Z:\path\to\t.obj'
```
