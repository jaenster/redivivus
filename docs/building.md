# Building and running Redivivus

This is the build that delivers a runnable `d2.exe` today, with a modern compiler. To compile
with the compilers that built the original, Visual C++ 2005 SP1 and Apple LLVM 4.2, with their
flags, see [original-compilers.md](original-compilers.md).

## You need

- Your own installation of Diablo II: Lord of Destruction, version 1.14. This repository
  contains no game data and no Blizzard binaries; the executable reads the MPQ archives
  from your installation. If you have no installation at hand,
  [blizzard-legacy-dl](https://github.com/jaenster/blizzard-legacy-dl) downloads the
  installers from Blizzard's own servers:

  ```sh
  blizzard-legacy-dl fetch D2DV -o ./d2     # Diablo II
  blizzard-legacy-dl fetch D2XP -o ./d2     # Lord of Destruction
  ```

  You still need your own CD keys to install.
- To build: a mingw-w64 cross compiler for 32-bit Windows (`i686-w64-mingw32-g++`), CMake
  and Ninja.
  - macOS: `brew install mingw-w64 cmake ninja`
  - Debian/Ubuntu: `apt install g++-mingw-w64-i686 cmake ninja-build`
  - Windows: see [Building on Windows](#building-on-windows)

## Build

```sh
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake
cmake --build build
```

That produces `build/d2.exe`, a 32-bit Windows executable. `tools/check-build.sh` does
the same from a clean clone and checks the result; CI runs it on every push.

### Building on Windows

The tree builds with GCC 13 on mingw-w64 11 headers, which is what CI uses (Ubuntu 24.04,
`g++-mingw-w64-i686`). The current MSYS2 MinGW32 packages do not build it: their mingw-w64
v15 headers have no `threadlocinfo` and declare `_EXCEPTION_DISPOSITION` as an enum, which
clashes with the generated headers.

Two routes work:

- **The CI toolchain in Docker or WSL.** Use Ubuntu 24.04 and the Debian/Ubuntu line above,
  then the commands under [Build](#build). In Docker, a bind mount from a Windows drive makes
  the compiler's reads slow. Copying the tree into the container or a named volume first makes
  the build several times faster.
- **Natively, with a [WinLibs](https://winlibs.com) GCC 13 build for i686 on mingw-w64 11.**
  Tested: `13.3.0posix-11.0.1-msvcrt-r1`, which also ships `ninja`. You also need CMake 3.24
  or later; the one bundled with Visual Studio works. Put the toolchain's `bin` on `PATH`, then:

  ```powershell
  cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake `
    "-DCMAKE_C_FLAGS=-D_WIN32_WINNT=0x0A00" "-DCMAKE_CXX_FLAGS=-D_WIN32_WINNT=0x0A00"
  cmake --build build
  ```

  WinLibs' headers default `_WIN32_WINNT` to Windows XP, which leaves
  `GetModuleFileNameExA` and `GetModuleInformation` unresolved at link time. The define gives
  the same headers as the Ubuntu package.

  Use `PATH` rather than a full path in `D2_TOOL_PREFIX`: the toolchain file appends `gcc` and
  `g++` without `.exe`, and CMake rejects the result.

  This is not the CI compiler (GCC 13.3 with posix threads, against 13.2 with win32 threads),
  so use the CI toolchain when a build has to match CI's.

Each module under `Source/` and `3rdparty/` builds into its own static library, and the
executable links them whole in the order of the 1.14d link. `-DREDIVIVUS_MODULE_LIBRARIES=OFF`
compiles the tree into the executable directly instead.

The module table is `project.json` (`modules`): each module's directory, kind, dependencies
with their evidence, documented exceptions, link order and strictness. `Source/modules.cmake` is
generated from it, and the configure step fails when the two disagree, so a change to the table
belongs in `project.json`.

The build also writes a module gate report. `build/module-gates.tsv` has one row per module
pair: the includes of a module that the includer could not see under the original 1.13d DLL
import graph, the documented exceptions, and what reaches it through `Source/globals/globals.h`
(`via-root`). `build/module-gates-summary.tsv` sums it per module. The report does not fail
the build, except for a module marked strict in the module table. `tools/check-build.sh`
prints the summary, and CI uploads the report as the artifact `module-gates`.

## Run

Copy `d2.exe` into your Diablo II folder, next to the original `Game.exe`, and
start it from there:

```
d2.exe -w
```

`-w` runs it in a window.
