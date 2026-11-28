# Where the file-name evidence for the Mac-only code was looked for

`coverage.tsv` grades a `blizzard-mac` body `dir` or `range` where the 1.14d Intel binary has no
`__FILE__` for its function, and `exception` where the body sits at none of the paths the evidence
allows. This page records, per unit, which other sources were searched for the file name and what they
gave, so that "no binary names this file" is a checked statement. Hits are rows of
`file-evidence.tsv`, which `coverage.ts` reads as anchors.

## The sources

|source|what it is|how it was read|what it gave|
|-|-|-|-|
|1.14d `DiabloII_macho`|the binary itself|`nm -ap`: only `start` and the `radr://5614542` OPT entry; no `N_SO`/`N_OSO`/`N_SOL` stabs, no debug map. Strings: 274 source paths|nothing new (already the units' anchors)|
|1.14b classic, 1.14b LoD, 1.14c classic `Diablo II` (Intel Mach-O)|`d2-surveys/ghidra-archive/Diablo2Lod/mac/intel/`|`nm -ap` as above (stripped); source-path strings compared with 1.14d after cutting the build-host prefix|the same 272 paths; 1.14d adds only `mach_override.c`|
|1.14c `BNUpdate` (Intel Mach-O)|the patcher, built from the same `StormMac/Storm Mac/` tree|`LC_FUNCTION_STARTS`, capstone with PIC tracking: the functions that load each path, matched to 1.14d by a normalised instruction fingerprint|names `Win32 Mac/Source/Win32/WinCompat.cpp` and `SOURCE/IOUnit/IO{Align,ARC4,Verify}Unit.cpp`, but only in functions the game does not have (WinCompat's "Unimplemented Win32 function: %s" reporter, the IOUnit streams); its other path users pair with 1.14d functions already graded `yes`|
|1.14d `BNUpdate`, `Blizzard Error`, `SystemSurvey`|`ghidra-archive/.../mac/intel/1.14d/`|strings|no StormMac or game path (BNUpdate carries none; the other two are curl and protobuf)|
|PPC Storm, 1.13d classic and 1.10 Beta Carbon|CodeWarrior PEF shared libraries with traceback tables (2014 and 2008 names)|`pef_unpack.py`, `pef_syms.py`; per function the paths it loads through the TOC; the 1.14d function of the same name|`ppc-storm` rows. The `__sinit_<file>` names: SCMD, SCODE, SDLG, SERR, SEVT, SFILE, SGDI, SMSG, SNET, SRGN, STRANS, SVID, SCOMP `_CPP`, plus in the Beta FPMem, fp_assert, fp_assert_impl, Allocation, MessageBox, Utils, MousePosition, VolumeIsCD, ThemeText, Tracer, CG_DirectDisplayGlue, FileTypeForPath, SaveFileFromMem, StartupStorm, SMem_Mac, Win32_Files/Process/Registry/utils/TimeAndDate, WinSock_*. A file without a path literal has one anchor there (its `__sinit`), so its other functions are only ever between two files|
|PPC game, 1.13d classic and Carbon, 1.10 Beta Carbon|PEF, no traceback names|function starts from `bl` targets and TOC code pointers; strings per function; paired to 1.14d only through a string unique on both sides|`ppc-shell` rows (OpenGLMacSetRenderer, the Apple texture-extension test). 1.10 Beta Carbon also names TSMGame, MainApp, Restart, D2AboutBox, D2Dialogs, MovableModalDialog, MouseControl, Cursor, Gamma, MacGamma, GameVideo, Video, VideoWizard, system, MemoryPartitions, LISTBOX, WNDPROC, Window, oglCore, oglEngine, oglMac, oglWall, COGLTextures; 1.13d Carbon only CFMLoader, GameSetup, MainMenus, D2FlamingLogo, CD2Textures and the six 1.14d renderer files. No 1.14d Mac-only function shares a unique string with a function that loads one of these, apart from the two rows|
|Windows 1.07 Beta and 1.07 JP `D2OpenGL.dll`|the 1999 renderer with a full COFF symbol table (values are RVAs) and `C:\D2\Source\D2OpenGL\Src\*.cpp` literals (JP: oglBlocks, oglCore, oglDebugFont, oglEngine, oglFloorCache, oglPerspective, oglSmack, oglSprite, oglSubtileCache, oglTextureCache, oglWall)|each COFF function's own pushes of a path, then bracketing in address order; the Mac function by its interface slot (`docs/data/mac-only-proposals.tsv` kind `fossil-name`, `high` only)|`fossil` rows|
|Windows 1.14d `Game.exe` and `docs/data/win-assert-oracle.tsv`|the Windows twins|Storm literals in Game.exe: only `.\SOURCE\SBig.cpp`; the oracle has no Storm module|nothing for the Mac-only units; Mac asserts carry no file (`docs/analysis/mac-assert-oracle.md`)|
|Metrowerks `.SYM`/`xSYM`, `.dSYM`, any PDB of a Mac build|searched the workspace (outside `recon/`) and the disk for `*.SYM`, `*.xSYM`, `*.dSYM`|`find`|none exist|
|`refs/hellfire` 1997 Storm|Windows Storm source, corroboration only|file list|`SLOG.CPP`, `SMEM.CPP`, `SMSG.CPP`, `SRGN.CPP` exist there in upper case; not used as evidence|

## Per unit: what remains and why

`dir`: no binary names the file; the folder is known.

|unit (body file)|rows|searched, none names the file|
|-|-|-|
|`Win32Mac/Source/Win32/Win32_*` (all but Win32_utils, Win32_Files, Win32_Process, Win32_TimeAndDate)|348|1.14d, 1.14b, 1.14c, BNUpdate 1.14c (its only Win32 path besides Win32_utils is WinCompat.cpp, in a function the game lacks), both PPC Storms (no path literal in that region; only `__sinit` anchors, one per file, so no function of it is bracketed by one file). The Beta's `__sinit` names (FPMem, fp_assert, MessageBox, Utils, Allocation, MousePosition, VolumeIsCD, ...) are candidates for these units, not evidence for any function|
|`StormMac/MacDisplayDriver.cpp`, `unknown_appinfo.cpp`, `unknown_dialog.cpp`|79|as above; the PPC Storm's `__sinit_SaveFileFromMem_cpp` names one file of this band, but no 1.14d function pairs with a function bracketed by it|
|`StormMac/SOURCE/unknown_sha1.cpp`|4|PPC Storm and BNUpdate: no literal in the SHA-1 class|
|`MacSpecific/Source/MacShell/unknown_*` (app, display, input, main, startup, tsm, util)|82|1.14b/c Intel (only GameSetup and D2FlamingLogo), PPC 1.13d Carbon (CFMLoader, GameSetup, MainMenus), PPC 1.10 Beta Carbon (TSMGame, MainApp, ... above): no unique string shared with a function that loads one of them|

`range`: the file is one of two (or one between them in link order); the anchors on both sides differ.

|unit|rows|note|
|-|-|-|
|Storm `SOURCE` between two literal files (SBig, SCMD, SCODE, SCOMP, SERR, SEVT, SFILE, SGDI, SSTR, STORM, STRANS, SVID, SMSG)|149|the PPC Storm brackets only the six `ppc-storm` rows; the rest have a different name on the Mac or lie between two PPC files too|
|`SLOG.cpp`, `SMEM.cpp`|26|no Mac literal and no PPC `__sinit`; the names are the Windows twins' (file map); case stays `.cpp`|
|`SRGN.CPP`|48|the PPC Storm names the file (`__sinit_SRGN_CPP`, which gives its case) but has no path literal in it, so its functions lie between SNET's and SRGN's anchors|
|`SRegMac.mm`, `unknown_macfile.cpp`|45|after SRegMac's last literal user; no PPC counterpart (the PPC registry is Win32_Registry.cpp)|
|D2OpenGL (`oglCore`, `oglMac`, `oglSprite`, `oglVertex`, `COGLTextures`, `BinkCutscene`, ...)|108|the fossil brackets only its `own`/`bracket` rows; slot pairs at `medium` confidence (OpenGLClose, OpenGLUtilDiamond, OpenGLCelFlatSpriteDraw, ...) are not used; the Mac-only functions (cutscene, gamma, AGL window code, COGLTextures) have no Windows twin and share no unique string with a PPC path user|
|shared modules (D2Client Engine/Game/Sound/EscMenu/..., D2Game MonsterMode, D2Win, Fog)|17|Mac-only functions with no Windows twin; Mac asserts carry no file; placed by their paired neighbours' tree files|

`exception` (`path-exceptions.tsv`), each re-checked:

|body|rows|searched|
|-|-|-|
|`D2gfx/Mac/*` (Forward, Helper, Renderer, Tiles, MACGFX)|32|no 1.14d, 1.14b/c, PPC or fossil literal names a D2gfx file; the PPC game has no D2gfx path at all|
|`D2gfx/Mac/MACDISPLAYGamma.cpp`|3|PPC 1.10 Beta Carbon has `MacGamma.cpp` and `Gamma.cpp`, but no gamma function there shares a unique string with these; the 1.13d Carbon and Intel builds have neither|
|`D2gfx/Mac/MacSupport.mm`|2|no literal in any build|
|`D2Hell/Mac/MACSEC.cpp`|8|the only D2Hell literal is `SRC/Archive.cpp` (1.14b/c/d); PPC has only the 'D2Hell Fault.cpp error.' message|
|`D2Client/Warden/WardenMac.cpp`|4|PPC 1.13a/c/d have `WardenClient.cpp`, the Intel builds do not; the four empty hooks share nothing with the PPC Warden code|

The 20 SMSG rows that were an exception (the case of SMSG.CPP was unconfirmed) are `yes` now: the
PPC Storm's `__sinit_SMSG_CPP` gives the case, and the body moved to `SMSG.CPP`.

## Re-running

The binaries are under `d2-surveys/ghidra-archive/Diablo2Lod/` (Intel, the 1.07 DLLs),
`d2-surveys/ghidra-import/files/Diablo2Lod/mac/powerpc/` and `d2-surveys/ghidra-gap-work/ref/files/mac/powerpc/`
(PPC). `docs/tools/pef_unpack.py` and `pef_syms.py` give the PPC sections and traceback names; strings in
these PEFs sit in the code section and are reached through `lwz rX, d(r2)` with the TOC at pdata + 0x8000.
Every address in `file-evidence.tsv` can be checked against those files.
