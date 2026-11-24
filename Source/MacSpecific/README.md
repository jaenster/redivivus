# MacSpecific

The game's Mac-only module, reconstructed by hand from the 1.14d Intel Mac build
(`DiabloII_macho`). It sits beside D2Common and the other modules because that is where the
original put it: the binary's source paths read
`Diablo2/Source/DiabloAll/../MacSpecific/Source/...`.

The Mac shell (`Source/MacShell/`) is Mac-only and not part of the Windows game: every file
compiles to nothing unless `D2_OS_MAC`, with the guard spelled `#if !defined(_WIN32) && D2_OS_MAC`
(the VC8 oracle's translator reads every source in the checkout and skips only `!defined(_WIN32)`
blocks). `Source/CD2Textures/` is the D2OpenGL texture base with the Mac system calls restored on
Win32/WGL, compiled with D2OpenGL by the CMake target `d2opengl_objs` and never linked into the
game. CMake's game build, the oracle (`NOT_GAME` in `compile.zig`) and `recon/runs/link.sh` leave
the folder out. The Mac shell is built by `cmake/mac.cmake` (compile only); both are measured by
`asm_matching/mac/cmp.ts`.

## Folders

Original folder names with spaces are kept without them:

|here|original|
|-|-|
|`Source/MacShell/`|`MacSpecific/Source/Mac Shell/`|
|`Source/CD2Textures/`|`MacSpecific/Source/CD2Textures/`|

The PowerPC builds name more Mac Shell files (MainApp, VideoWizard, VideoDetect, D2Dialogs,
D2AboutBox, FileManager, GameInterface, MouseControl.c, MovableModalDialog.c, ...). Only the
files the 1.14d Intel binary itself names are used here; code from the others goes under
those names once a 1.14d unit is tied to one.

## How each file was pinned

|file|1.14d|functions|pinned by|
|-|-|-|-|
|`Source/MacShell/GameSetup.cpp`|0019c760..0019cc81, 0019cf50|11|1.14d `__FILE__` `.../Mac Shell/GameSetup.cpp` (0031a4da) in GAMESETUP_Alloc/Free (lines 463, 486); the globals 003b8d7c and 00536c50 are addressed directly, so they are defined in this unit, and 0019cf50 writes 00536c50|
|`Source/MacShell/D2FlamingLogo.cpp`|0019f2b6..0019f900|3|1.14d `__FILE__` `.../Mac Shell/D2FlamingLogo.cpp` (0031a9d3) in FLAMINGLOGO_Init/Shutdown; the Baal AI starts at 0019f900|
|`Source/MacShell/QuickDrawMac.h`|-|-|not original: the QuickDraw prototypes the 10.7 SDK headers dropped|

The 1.14d `__mod_init_func` has no initialiser inside the Mac shell run
(0019a59a..0019f900), so the unit boundaries there come from `__FILE__` users and
direct-versus-indirect global access only.
