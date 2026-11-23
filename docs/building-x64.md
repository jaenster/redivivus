# Building: the licensed DLLs and the 64-bit experiment

The target is 32-bit Windows: the decompiled code assumes the MSVC x86 ABI. The build itself is
in [building.md](building.md); this page covers the dependencies the executable has beyond the
Windows SDK, and the 64-bit experiment.

## The three licensed DLLs (32-bit)

Game.exe used three DLLs it did not contain: Bink (`binkw32.dll`), Smacker
(`smackw32.dll`) and the Intel JPEG Library (`ijl11.dll`, screenshots). The 32-bit build
links import libraries generated from the checked-in `.def` files in `3rdparty/implibs/`
(derived from the shipped DLLs' export tables, see `PROVENANCE` there) and runs beside the
original DLLs from a Diablo II install. Nothing of them is reimplemented for that target.

## The 64-bit build

```sh
cmake -S . -B build64 -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-x86_64.cmake \
      -DREDIVIVUS_PERMISSIVE=ON -DREDIVIVUS_FFMPEG_INCLUDE=<ffmpeg-shared>/include
cmake --build build64 -j            # d2.exe
cmake --build build64 -t x64deps_test
```

Needs `x86_64-w64-mingw32-g++` (Homebrew `mingw-w64`, Debian `g++-mingw-w64-x86-64`).
`REDIVIVUS_PERMISSIVE=ON` is required: the 32-bit ABI assumptions in decompiled code are
pointer-truncating casts on x86-64, which only `-fpermissive` lets through. The executable
links with no undefined symbol and no import of the three 32-bit DLLs, and is an experiment:
at the time of writing it faults during startup in Storm's heap (pointer width), long before
any renderer or video. What the build proves is the dependency story below.

### What replaces the DLLs

No 64-bit binkw32, smackw32 or ijl11 exists. `Redivivus/x64/` defines the same entry
points, selected by CMake for the 64-bit target only (the 32-bit build does not compile
them):

|Original|Shim|Backend|
|-|-|-|
|binkw32.dll (BinkOpen, BinkClose, BinkWait, BinkDoFrame, BinkNextFrame, BinkCopyToBuffer, BinkSetSoundSystem, BinkOpenDirectSound, BinkDDSurfaceType)|`d2_bink.cpp`|FFmpeg bink/binkaudio through `d2_media.cpp`|
|smackw32.dll (SmackOpen, SmackClose, SmackWait, SmackDoFrame, SmackNextFrame, SmackToBuffer)|`d2_smack.cpp`|FFmpeg smackvid/smackaud through `d2_media.cpp`|
|ijl11.dll (ijlInit, ijlWrite, ijlFree)|`d2_ijl.cpp`|`3rdparty/stb/stb_image_write.h` (MIT / public domain)|

The BINK, Smack and JPEG_CORE_PROPERTIES layouts are the ones the callers read
(`Redivivus/x64/d2_x64deps.h` documents each field's evidence). Video audio plays through
waveOut (`d2_pcmout.cpp`); the originals used DirectSound, but the game only observes that
the sound keeps pace with the frames, and waveOut provides the sample clock the pacing
(`BinkWait`/`SmackWait`) follows.

### FFmpeg at run time

FFmpeg is **not linked**. `d2_ffmpeg.cpp` loads the shared libraries with `LoadLibrary` and
resolves every entry point with `GetProcAddress`, so the tree stays MIT and FFmpeg stays
the separate LGPL library its licence asks for. The headers are needed to compile
(`REDIVIVUS_FFMPEG_INCLUDE`) because struct layouts are read through them; they must come
from the same major versions as the DLLs placed beside the executable.

Tested with the BtbN build `ffmpeg-n8.1.3-39-g660b5aa7db-win64-lgpl-shared-8.1`
(release tag `autobuild-2026-11-22-12-36` of github.com/BtbN/FFmpeg-Builds, LGPL v3). Put
these five from its `bin/` next to `d2.exe`:

```
avutil-60.dll  swresample-6.dll  swscale-9.dll  avcodec-62.dll  avformat-62.dll
```

(avdevice, avfilter and the ffmpeg executables are not needed.) The names carry each
library's major version; a build against other headers wants the matching numbers, which
`d2_ffmpeg.cpp` derives from the headers it was compiled with. Without the DLLs
`BinkOpen`/`SmackOpen` return NULL and the game skips the video, exactly as it does for
a missing file; one line in the debug output says which DLL was missing.

### Testing the shims without the game

`x64deps_test.exe` (target `x64deps_test`, not built by default) drives the three shims
the way the game does: a `.bik` through a file handle opened with `BINKFILEHANDLE` and
`BINKCOPY2XH` (the game's way of showing the 640x240 logo clips at 640x480), a `.smk`
through the 8-bit and the 16-bit `SmackToBuffer` paths, and a JPEG through `ijlWrite`.

```
x64deps_test <out-dir> --bik BlizNorth640x240.bik --smk some.smk [--paced] [--frames 1,30,60]
```

It writes the chosen frames as PNG and `ijl-screenshot.jpg`, and exits non-zero when a
step produced nothing. `--paced` honours `BinkWait`/`SmackWait` and so plays the audio;
without it the loop decodes as fast as it can. The clips come out of `d2video.mpq`
(`data\local\video\BlizNorth640x240.bik`, 736 KB, is a good one); the game ships no
`.smk`, so bring one from another title. The exe runs under Wine (x86_64) as well as on
Windows.

### Glide on 64-bit

`glide3x.dll` is loaded by name (`Source/D2Glide/Renderer/Glide.cpp`), and no 64-bit
Glide exists from 3dfx, so a missing DLL fails the Glide mode the same way on both
targets. For a working 64-bit Glide mode use **gl2d9** (github.com/PlayNeth/gl2d9, MIT,
Glide 2.1/2.x/3.x on Direct3D 9). Its sources are plain C++ and build with the
x86_64 mingw compiler unchanged; only its export table has to lose the `@N` on the
internal names, because x86-64 has no stdcall decoration while the game still asks
`GetProcAddress` for `_grSstWinOpen@28` and the other 44 decorated names:

```sh
git clone https://github.com/PlayNeth/gl2d9 && cd gl2d9        # tested at 1.2.0, fc2714c
sed -E 's/^( +_[A-Za-z0-9_]+@[0-9]+) = ([A-Za-z0-9_]+)@[0-9]+$/\1 = \2/' src/glide3x.def > glide3x64.def
for s in main aa api_common core display draw gu gu3df gump gutex hook lfb present render \
         renderlfb renderstate rendertex shader shadercache state stipple targets texmem util window api3; do
  x86_64-w64-mingw32-g++ -std=c++17 -O2 -DNDEBUG -msse2 -mfpmath=sse -w -c src/$s.cpp -o $s.o
done
x86_64-w64-mingw32-g++ -shared -o glide3x.dll *.o glide3x64.def -static -static-libgcc -static-libstdc++ -lgdi32 -luser32
```

The result exports all 45 entry points the tree calls under their decorated names and
loads Direct3D 9 at run time. gl2d9 is a drop-in DLL and patches nothing in the game, so it
is the candidate that fits this executable; it ships beside `d2.exe` as a separate MIT
component, not in the tree. `glidetest` (target of the same name, `Redivivus/x64/test/glidetest.cpp`)
is the smoke test: a 64-bit exe that resolves the decorated names as Glide.cpp does, opens a
640x480 context on its window and draws through `grBufferClear` and the LFB. On the Windows VM
with this DLL it opened the context and rendered the LFB gradient. The other wrappers looked at do not fit: OpenGlide and
psVoodoo implement Glide 2.x only (psVoodoo is also 32-bit only by construction), D2DX is
GPL-3 and hooks Game.exe's internals per game version through Detours, and nGlide and
dgVoodoo2 are closed source and 32-bit. In-game verification of the Glide mode waits on the
64-bit executable reaching its renderer at all (above).

### What differs between the targets in hand-written code

`Source/D2Headers/D2Arch.h` defines exactly one of `D2_ARCH_X86` / `D2_ARCH_X64`. The
hand-maintained files that must differ test it: the crash reporter (`Redivivus/d2_crash.cpp`
walks the x64 unwind tables with `RtlVirtualUnwind` instead of the EBP chain) and the
tracing handler (`Redivivus/d2_trace.cpp`). The x86 branches are the pre-existing code; their
32-bit object files are byte-identical to before the split. `Redivivus/d2_link_support.s`
(i686 assembly) is left out of the 64-bit build and `Redivivus/x64/d2_crt_x64.cpp` carries
its one definition in C++ (plus `_rotl16`, which mingw's x64 `winnt.h` declares as a function).
Generated code never tests the architecture.
