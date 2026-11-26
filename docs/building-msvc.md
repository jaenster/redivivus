# Building with Visual Studio 2026 / 2022

The reference builds are the mingw-w64 g++ one (the game everybody runs) and the VC8
oracle (byte matching). Visual Studio builds the same tree for one more reason: to
step through it in the Visual Studio debugger, with breakpoints, watches and the
call stack on the recovered source.

Supported: Visual Studio 2026 (MSVC 14.5x) and Visual Studio 2022 17.8 or later
(MSVC 14.4x), the IDE or the Build Tools, with cl or clang-cl, for x86 and x64. You
need the "Desktop development with C++" workload, a Windows 11 SDK, and for clang-cl
the "C++ Clang tools for Windows" component. CMake and Ninja come with the workload.

## In the IDE

1. File > Open > Folder..., the checkout's root. Visual Studio reads `CMakePresets.json`.
2. Pick a configuration in the toolbar, for example **MSVC x86 Debug (game)** or
   **MSVC x86 Debug (d2host)**.
3. Pick the startup item (`d2.exe` or `d2host.exe`) and press F5.

The game needs the Diablo II 1.14d data next to it or as its working directory, like
the mingw build. Set it in the launch configuration (Debug > Debug and Launch Settings
for the target, `"currentDir"` and `"args"`), for example:

```json
{ "type": "default", "project": "CMakeLists.txt", "projectTarget": "d2host.exe",
  "currentDir": "D:\\Games\\Diablo II", "args": [ "-exitafter", "60" ] }
```

## On the command line

From a "Developer Command Prompt" (or after `vcvarsall.bat x86` / `amd64`):

```bat
cmake --preset msvc-x86-debug
cmake --build --preset msvc-x86-debug
```

The build lands in `out/build/<preset>/`.

|Preset|Compiler|Arch|Builds|
|-|-|-|-|
|`msvc-x86-debug`, `msvc-x86-release`|cl|x86|d2.exe|
|`msvc-x64-debug`, `msvc-x64-release`|cl|x64|d2.exe (the 64-bit experiment)|
|`clangcl-x86`, `clangcl-x64`|clang-cl|x86, x64|d2.exe|
|`msvc-x86-server-debug`, `msvc-x64-server-debug`|cl|x86, x64|d2host.exe|
|`clangcl-x86-server`, `clangcl-x64-server`|clang-cl|x86, x64|d2host.exe|

The x64 game executable needs FFmpeg's headers, as the mingw x64 build does
([building-x64.md](building-x64.md)): set `REDIVIVUS_FFMPEG_INCLUDE` in the
environment (or as a cache variable) to the `include` directory of an FFmpeg shared
build. The server presets do not need it.

## What differs from the mingw build

- **Debug information is a PDB**, in every configuration, Release included
  (`/Zi`, `/DEBUG:FULL`). The crash report (`Redivivus/d2_crash.cpp`) symbolises with
  DbgHelp from that PDB instead of libbacktrace from DWARF; keep the `.pdb` beside the
  `.exe`.
- **The static CRT** (`/MT`, `/MTd` in Debug), as the mingw build links `-static`.
- **No `/RTC1`** in Debug: decompiled code reads locals before storing them where the
  original did, and calls through casts that move ESP. `-DREDIVIVUS_MSVC_RTC=ON` turns
  the checks on, which is useful to find exactly those places.
- **The Bink, Smacker and IJL import libraries** (x86) are made with `lib.exe` at
  configure time from the `.def` files in `3rdparty/implibs/`, translated from dlltool's
  `==` to lib.exe's `EXPORTAS` (hence Visual Studio 2022 17.8 or later).
- **cl (not clang-cl) maps two GCC spellings** in `Source/D2Headers/d2_platform.h`:
  `__attribute__((...))` to `__declspec` or nothing, with every packed struct wrapped in
  `D2_PACKED_BEGIN`/`D2_PACKED_END` (`#pragma pack(1)`, the same layout: every member
  offset of all packed structs is checked equal to the mingw build's), and `__thiscall`
  on free functions, which cl allows on members only, to `__fastcall` on x86. The
  receiver stays in ECX; the second argument of those functions moves from the stack to
  EDX. Every caller and callee is compiled the same way, so the program agrees with
  itself, but in a cl x86 build those functions do not have the original's calling
  convention - keep that in mind when reading their disassembly in the debugger.
  clang-cl accepts both GCC spellings as they are and keeps `__thiscall`.
- `/permissive-` with `/Zc:strictStrings-`: string literals bind to `char*`, as G++
  allows, because the recovered signatures take `char*` where the original passed
  literals.
- `REDIVIVUS_LOUD` is GCC-only (sanitizers, guard sections).

Neither the mingw build nor the VC8 oracle is affected: the MSVC branches are under
`_MSC_VER >= 1900`, and VC8 is 1400.
