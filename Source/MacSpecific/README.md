# MacSpecific

The game's Mac-only module, reconstructed by hand from the 1.14d Intel Mac build
(`DiabloII_macho`). It sits beside D2Common and the other modules because that is where the
original put it: the binary's source paths read
`Diablo2/Source/DiabloAll/../MacSpecific/Source/...`.

The Mac shell (`Source/MacShell/`) is Mac-only and not part of the Windows game: every file
compiles to nothing unless `D2_MAC_CARBON`, with the guard spelled `#if !defined(_WIN32) && D2_MAC_CARBON`
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

The 1.14d Mac shell run 0019a59a..0019f900 is eight compilation units plus Game/Main.cpp, cut
from the binary itself. The evidence, in order of strength:

- **Direct versus non-lazy-pointer access.** The binary is PIC: a global defined in the unit is
  addressed off the PIC base, one defined in another unit through `__nl_symbol_ptr`, and the
  same holds for taking a function's address. So whoever addresses a global directly defines it,
  and a function that goes through a non-lazy pointer is in another unit.
- **File-local functions.** A static gets a register argument (ecx) when all its callers are
  known, and clang places it right after the function that first refers to it; a static's
  callers are in its unit.
- **Out-of-line one-liners.** clang-425 (`-Os`) inlines a few-instruction callee in its own
  unit, and drops the argument stores a known empty body does not read. A trivial callee
  called out of line, or an unused argument still passed, means the callee is in another unit.
- **Padding.** ld64 keeps each object's `__text` alignment, which is 4 when the unit has a jump
  table and 1 when it has none. Nops before a 4-aligned function mark an object start.
- **The order of `__common`, `__data` and `__bss`.** The linker lays each object's data out in
  object order; the shell's data follows the units' text order (commons 005c865c..005c8a30
  unknown_startup, 005c8a34..36 unknown_input, 005c8a38 unknown_app, 005c8a3c..44 unknown_main,
  005c8a48..4c Game/Main.cpp).

|file|1.14d|functions|pinned by|
|-|-|-|-|
|`Source/MacShell/unknown_startup.cpp`|0019a59a..0019b290|10|defines the configuration 005c8660, the instance 005c865c and the mode 005c8a30 (MAC_ParseCommandLineAndRun addresses them directly; FUN_0019be53/be79, FUN_0019bed3, FUN_0019c96e and ApplicationMain go through non-lazy pointers), the modal state 00536af4.. and the tokenizer state 00536b1c..; ends with the file-local tokenizer 0019b1ec (ecx argument), then three nops to 0019b290, a 16-byte boundary. No evidence splits the dialogs from the command line|
|`Source/MacShell/unknown_input.cpp`|0019b290..0019bbe8|23|padding on both sides (three nops to 0019b290, two to 0019bbe8; the unit has a jump table); the statics 00536c28..00536c3d are addressed directly by its functions and by none outside; defines 005c8a34..36 and 003b8d78. Its one-liners FUN_0019b290/b2a2/b2b4, MAC_HandleMouseDown/Up/Moved, MAC_HandleUpdateEvent, MAC_PostKeyDown/Up, FUN_0019bac5, FUN_0019bbcd and MAC_OnApplicationActivate/Deactivate are called out of line (and with unused arguments) from unknown_app.cpp and unknown_main.cpp|
|`Source/MacShell/unknown_app.cpp`|0019bbe8..0019bed3|6|starts at the padding to 0019bbe8 (its jump table makes it 4-aligned); MAC_HandleMenuCommand stores 005c8a38 directly, unknown_display.cpp reaches it through a non-lazy pointer; the statics 00536c40/48. MAC_HandleUpdateEvent calls its FUN_0019be2c, FUN_0019bed3 and FUN_0019cc81 its MAC_GetDefaultDisplayMode, all out of line|
|`Source/MacShell/GameSetup.cpp`|0019bed3..0019c7e4|4|1.14d `__FILE__` `.../Mac Shell/GameSetup.cpp` (0031a4da) in GAMESETUP_Alloc/Free (lines 463, 486); FUN_0019bed3 takes the addresses of GAMESETUP_Alloc, GAMESETUP_Free and FUN_0019c7d9 directly, so it is in this unit, and it calls the 44-byte MAC_GetDefaultDisplayMode out of line, so that is not; the unit has no jump table (alignment 1), so no padding marks its start. One nop to 0019c7e4 ends it|
|`Source/MacShell/unknown_display.cpp`|0019c7e4..0019cf88|12|padding on both sides (one nop to 0019c7e4, three to 0019cf88; jump table in MAC_ApplyDisplayModuleSettings); defines the video mode 003b8d7c and the statics 00536c50/54; FUN_0019cc81 takes FUN_0019cb40, MAC_SetCursorPos, MAC_GetCursorPos and FUN_0019cf50 directly and the StormMac cursor functions through non-lazy pointers; 005c8a38 and 005c8660 come through non-lazy pointers|
|`Source/MacShell/unknown_main.cpp`|0019cf88..0019dd07|14|starts at the padding to 0019cf88; the file-local handlers MAC_ProcessEvents refers to first follow it (MAC_TrackMouseAndModifierChanges, MAC_HandleSuspendResumeEvent, MAC_HandleKeyDownEvent: ecx arguments), MAC_Main's follow MAC_Main, and the ones first referred to from those bodies close the unit (FUN_0019db65, MAC_HandleMenuSelection, MAC_SynthesizeKeyEvent at 0019dcd4); defines 005c8a3c..44, 00536c58..5e, 00536ca4, 003b8d80. MAC_HandleSuspendResumeEvent calls the 35-byte TSM activate/deactivate out of line|
|`Source/MacShell/unknown_tsm.cpp`|0019dd07..0019e8a1|15|after unknown_main.cpp's last file-local function, so not in it; owns 003b8d84..8c and 00536ca8..00536cf0; no jump table (alignment 1), so it can start at the odd 0019dd07|
|`Source/Game/Main.cpp` (shared)|0019e8a1, 0019ebbe|2|0019e8a1 is MISC_InitD2IniAndIgnorelist (Windows 004059a0) with MISC_ParseCommandLineAndPutIntoD2IniData inlined: it reads Main.cpp's switch table (D2IniConfigValues, the `__data` at 003b8d90: DEBUG, CRASHAPP, crash, VIDEO, WINDOW ...) and Fog's assert; ApplicationMain follows. Both are covered by the shared body; the Mac-compiled reconstruction of 0019e8a1 that sat in GameSetup.cpp is kept at dd35cd3d:Source/MacSpecific/Source/MacShell/GameSetup.cpp (`MAC_ParseCommandLine`) as the reference for Main.cpp's Mac compile|
|`Source/MacShell/unknown_util.cpp`|0019f18a..0019f2b6|2|between Main.cpp and the logo; neither function names a file and nothing decides whether they close Main.cpp, open D2FlamingLogo.cpp or are a unit of their own|
|`Source/MacShell/D2FlamingLogo.cpp`|0019f2b6..0019f900|3|1.14d `__FILE__` `.../Mac Shell/D2FlamingLogo.cpp` (0031a9d3) in FLAMINGLOGO_Init/Shutdown; the Baal AI starts at 0019f900|
|`Source/MacShell/QuickDrawMac.h`|-|-|not original: the QuickDraw prototypes the 10.7 SDK headers dropped|

The 1.14d `__mod_init_func` has no initialiser inside the Mac shell run, so static initialisers
mark no boundary here. Two `noinline`s remain, each on a file-local function (ecx argument, so
its caller is in the same unit) that the original calls out of line from its single caller:
FUN_0019b1ec and MAC_HandleKeyDownEvent; clang-425 inlines a one-caller static otherwise.
The modal state's 16-bit item stores stay separate because the record is 4-byte aligned (it
is Storm's DlgMenuState, with a DWORD and a WindowRef); with 2-byte alignment clang merges the
first pair.
