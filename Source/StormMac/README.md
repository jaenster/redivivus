# StormMac

Storm's Mac project, reconstructed by hand from the 1.14d Intel Mac build
(`DiabloII_macho`). It is its own project next to the game in the original:
the binary's source paths read `Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/...`
(the 1.14d BNUpdate patcher uses the same tree). It is not part of Storm (`Source/Storm`);
it sits beside it under `Source/` because both are Blizzard's own code.

Mac-only: every file compiles to nothing unless `D2_OS_MAC`, with the guard spelled
`#if !defined(_WIN32) && D2_OS_MAC` (see `Win32Mac/Source/Win32/Win32Mac.h`).
Built by `cmake/mac.cmake` (compile only), measured by `asm_matching/mac/cmp.ts`.

## Folders

The original folder names contain spaces. They are kept without them:

|here|original|
|-|-|
|`Source/StormMac/`|`StormMac/Storm Mac/`|
|`Source/StormMac/SOURCE/`|`Storm Mac/SOURCE/` (Storm proper: STORM.CPP, SBig.cpp, SBLT.CPP, SBMP.CPP, SCMD.CPP, SCODE.CPP, SCOMP.CPP, SERR.CPP, SEVT.CPP reconstructed; SFILE.CPP reconstructed; SRegMac.mm, IOUnit/... not yet)|
|`Source/StormMac/H/`|`Storm Mac/H/` (stpl.h)|
|`Source/StormMac/Win32Mac/Source/Win32/`|`Storm Mac/Win32 Mac/Source/Win32/` (the Win32-on-Mac layer; Win32_utils.cpp, WinCompat.cpp)|

## How each file was pinned

Two sources of evidence:

- the 1.14d binary itself: `__FILE__` strings, and the static initialisers in
  `__mod_init_func`, each of which ends a translation unit (00026c70, 0002ea70 in the
  layer); a function another one calls out of line, where one file would inline it,
  marks a boundary;
- the PPC Storm 1.10 Beta (`mac/powerpc/lod/1.10_Beta/Storm_(Carbon)`, md5
  e69b1e32...), whose traceback tables name every function and whose
  `__sinit_<file>` functions name the files that have static initialisers. MWCC emits a
  file's functions last to first and ends it with its `__sinit`; files without one fold
  into the next named one. The Win32 layer's source order has barely moved since: the
  1.14d runs below are the beta files read backwards.

|file|1.14d range|functions|pinned by|
|-|-|-|-|
|`H/stpl.h`|templates|-|1.14d `__FILE__` (`Storm Mac/H/stpl.h`)|
|`Win32_utils.cpp`|0002c670..0002eae0|55|1.14d `__FILE__` + its static initialiser 0002ea70; beta `__sinit_Win32_utils_cpp` has the same path-cache, kernel-pool and FilepathToFSSpec code|
|`Win32_Files.cpp`|00021a94..00022ca8|12 of 16|beta `__sinit_Win32_Files_cpp`, same functions in the same order|
|`Win32_CommandLine.mm`|0002ab8e|1|GetCommandLine, the one Objective-C function of the Win32_Process run (NSProcessInfo), so a unit of its own beside Win32_Process.cpp; Win32Mac.h is not included because its BOOL is a 32-bit integer and Objective-C's is not|
|`Win32_Process.cpp`|00029b68..0002b1fe|31 of 43|beta `__sinit_Win32_Process_cpp`, same order; SetLastError inlined into GetModuleFileName|
|`Win32_TimeAndDate.cpp`|0002c0ee..0002c670|16 of 16|beta `__sinit_Win32_TimeAndDate_cpp`, same order|
|`Win32_Events.cpp`|000216b0..00021a94|7|beta: a separate unit linked before Win32_Files.cpp|
|`Win32_CloseHandle.cpp`|00025660..000257bc|1|separate from the handle table: it calls GetWin32HandlePtr out of line, inlined if merged|
|`Win32_Handles.cpp`|000257bc..00026d14|23|handle table, libc++ deque instantiations; ends with the static initialiser 00026c70|
|`Win32_Memory.cpp`|000289b8..00028e09|19|beta: a unit of its own (SMemZero/Fill/Copy are SMem_Mac.cpp in the beta, but sit inside this run in 1.14d)|
|`Win32_Messages.cpp`|00028e09..00029968|19|beta: a unit of its own|
|`Win32_Priv.cpp`|00029968..00029b68|6|beta: Priv_ functions linked before Win32_Process.cpp; not in it in 1.14d (ExitProcess's body changes Priv_Alloc if merged)|
|`Win32_Threads.cpp`|0002bdea..0002c0ee|11|beta: a unit linked before Win32_utils.cpp|
|`Win32_WndProcs.cpp`|0002eae0..0002f888|5|DefWindowProc and the built-in class procedures (desktop, Button, Static) with Proc_RegisterBuiltInClasses: RegisterClassEx (00030077) loads &DefWindowProc through a non-lazy pointer, so DefWindowProc is not in its unit, while Proc_RegisterBuiltInClasses takes the three procedures' addresses directly; Proc_ButtonWndProc calls GetCapture, GetClientRect and GetFocus out of line. Name unrecorded|
|`Win32_Window.cpp`|0002f888..00031b0c|60|window manager (classes, window tree, focus, paint, props, key maps); g_hDesktopWnd (003f2504) is addressed directly from Wnd_InitWnd through InvalidateRect, one unit. No binary records the file name, `Win32_Window` follows the Win32_<Family> naming. Key tables 00031a84/00031af6 end the run before the region code (00031b0d, Win32_Region) |
|`Win32_MacInit.cpp`|0001ec2e..0001ed6c|4|the two clock thunks, WIN32MAC_Initialize and WinDestroy: MAC_GetMicroseconds64 and FPMilliseconds (Win32_MacMisc.cpp) call the one-jump thunks out of line, which clang -Os inlines within a unit; the boundary could sit anywhere from 0001eafe to 0001ec2e, it is put where the thunks begin. Name unrecorded|
|`MacDisplayDriver.cpp`|00036830..000371f9 (less the unnamed 00036dc7..00036f91)|46|name unrecorded: one driver table (default display driver: cursor counter, palette, bounds) with its registration calls, then Accel_DeleteAcceleratorList and MACDISPLAY_FindBestDisplayMode by address order; D2OpenGL imports the MACCURSOR_*/MACDISPLAY_* names. Palette get/set stay DIFF: the original's indexed displacement is a link-time address|
|`Win32_Region.cpp`|00031b0d..00032b50|21|name unrecorded: the strip-region run between the window manager's key-code helpers and the DirectX shims; one heap-block region type, so one unit|
|`Win32_DirectSound.cpp`|00032b50..00033a24|31|name unrecorded: DirectDrawCreate, DirectSoundCreate and the IDirectSound/IDirectSoundBuffer/IKsPropertySet bodies share the sound-buffer globals 003f481c..003f4828|
|`Win32_String.cpp`|00033a24..00033cef|11|name unrecorded: the CRT string and number helpers (_stricmp .. srand) before StormMac's own SOURCE run begins at 00033cef; several have Windows twins|
|`SOURCE/SRegMac.mm`|0003a696..0003e21d|88 (18 functions, 70 stpl.h instantiations and the static initialiser)|1.14d `__FILE__` (0030f7d6) in SRegSaveData, SRegSaveString, SREG_ReadEscapedData, SREG_FreeValueData, SREG_DestroyEntry; Objective-C++ (NSFileManager through objc_msgSend); ends with its static initialiser 0003e0a0; the three hash tables it instantiates (RegistryEntry, RegistryFileEntry, WriteRegistryEntry) keep their weak members here only, the linker kept the first copy of the ones shared with Win32_utils.cpp (HASHKEY_STRI::operator=)|
|`SOURCE/unknown_macfile.cpp`|0003e21d..0003e30a|4|the unit after SRegMac.mm's static initialiser: MACFILE_FSSpecToPath, a counter and the static object calling StormMacInit / MyRuntimeTerminate; no binary records its name|

The `unknown_*` names are descriptive: no binary records those files' names. The folder
of the `Win32_*` files other than Win32_utils.cpp is inferred from it, and the header
`Win32Mac.h` has no known original name. The beta also names files 1.14d may still have
(WinCompat.cpp, Win32_Registry.cpp, Allocation.cpp, FPMem.cpp, fp_assert.cpp,
MessageBox.cpp, Utils.cpp, MousePosition.cpp, VolumeIsCD.cpp, ...); code from them goes
under those names when it is reconstructed.

## Storm Mac misc and Win32 layer tail (1.14d 00033cef..00036830)

No binary records the file names of this band, so the files are named for what they hold:

|file|1.14d range|functions|decision|
|-|-|-|-|
|`unknown_appinfo.cpp`|00033cef..00035545|26|application info, Internet Config, MoreFiles helpers, user/machine names, file saving, text state, start-up/terminate: one run of Storm Mac code without a recorded `__FILE__`; the static initialiser 00035290 ends the part that owns the app-location global|
|`unknown_dialog.cpp`|0003555c..0003602b|7|modal dialog loop and its menu handling; the Dlg_ functions call each other and the two helpers are file-local (register-passed arguments)|
|`Win32Mac/Source/Win32/Win32_Paths.cpp`|000360c2..00036440|2|path walking and fopen, after the Win32 layer's last named file|
|`Win32Mac/Source/Win32/Win32_MemFile.cpp`|00036440..00036830|5|the MemFile2 family, which shares the statics 003f492c..003f4940|

## StormMac/SOURCE file boundaries

The Storm `SOURCE` files, cut from the binary's address order; where no `__FILE__` literal
decides, the boundary is chosen as follows.

|File|Range|Note|
|-|-|-|
|`SFILE.CPP`|0001204e..00017700|ends with its static initialiser (00017700); the run 00016a2e..0001719c and 000175b6..0001762c is the instantiations of the lists of its five record types (TSLink, TSExplicitList), and its tail 00017484..000176a0 is the stream and request destructors and the lock. The records' destructors are the `SFILE_Free...Record` functions (the callers delete the record themselves); the completion-event list has a node type of its own (SFileEventNode). The instantiations are tagged on explicit instantiations at the file's end (SFILE_ReleaseFileHandle 00017210 is the request list's DeleteNode, CRITSECT_Init 000176a0 sits here: the lock's constructor calls it, SRGN and SNET call it too); the calls inside the file inline them, where the original calls the weak copies, and the linker's hidden visibility of those copies is why the static initialiser reaches the destructors with `lea` rather than through a non-lazy pointer|
|`SGDI.CPP`|00017886..00017c32|`__FILE__` (SGDI.CPP, 0x118/0x136 in SGdiDestroy's frees); ends with its static initialiser (00017be0)|
|`SLOG.cpp`|00017c32..00018b0b|SLogClose is its first function (the Windows build's asserts name SLOG.CPP); no static initialiser, so the end is the first function of the next file, SMemAlloc|
|`SMEM.cpp`|00018b0b..00018cdb|SMemAlloc .. SMemInitialize; SMsgBreakHandlerChain (00018cdb) is the first of SMSG|
|`SMSG.cpp`|00018cdb..00019532|ends with its static initialiser (000194e0); WND_RemoveAndFreeRecord (00019464) is the list's DeleteNode instantiation|
|`SNET.CPP`|00019532..0001ae80|ends with `__sinit_SNET_CPP`|
|`SRGN.cpp`|0001b087..0001c6f0|region code only, no literal names the file; ends with `SRGN_InitStatics`|
|`SSTR.CPP`|0001c730..0001d2f9|SStrLower (0001d2f9) included|
|`SSignature.cpp`|0001d302..0001d69e||
|`STRANS.CPP`|0001d69e..0001de67|the UTF-8/UTF-16/Latin-1 helpers have no file evidence; placed here with their tables|
|`SVID.CPP`|0001de67..0001e2fd|includes STORM_LaunchURL and its callback|
|`SDRAW_VidDriver.CPP`|00036035|`SDrawDestroy`|

Files keep the case of their `__FILE__` literal where one exists (STORM.CPP, SBLT.CPP, SGDI.CPP, SNET.CPP, SSTR.CPP, SDRAW_VidDriver.CPP); the compare and coverage tools and `cmake/mac.cmake` accept both `.cpp` and `.CPP`. SERR, SLOG, SMEM, SMSG and SRGN have no `__FILE__` in the Mac binary: their names come from their Windows twins' files (the file map; SRGN from the address order only). SERR.CPP is spelled as the file map spells it; SLOG, SMEM, SMSG and SRGN stay lowercase `.cpp` because no Mac literal fixes their case (`asm_matching/mac/coverage/path-exceptions.tsv` records SMSG, the one the file map spells `SMSG.CPP`).

## Storm SOURCE, STORM.CPP through SEVT.CPP (1.14d 000095fc..00011fdb)

File names keep the case of their `__FILE__` literals (`SBig.cpp`, `SBLT.CPP`, `SCMD.CPP`). Each
file ends at its static initialiser (SCMD_InitStaticLists, SCODE_InitStaticLists,
SCOMP_InitStatics, SERR_InitStatics, SEVT_InitStatics), which closes a translation unit, and
the list-template instantiations before it belong to that file.

|file|1.14d range|boundary decision|
|-|-|-|
|`STORM.CPP`|000095fc..00009d29|operators and STORM_Alloc carry its `__FILE__`; StormDestroy, the system options, the CRC32, the RSA key calls and the log header have no literal and sit between its last and SBig.cpp's first function|
|`unknown_sha1.cpp`|00009d2e..0000a56e|the SHA-1 class is a unit of its own in the PPC Storm; no 1.14d literal names it|
|`SBig.cpp`|0000a629..0000bf46|the exports in alphabetical order up to SBigNew's `__FILE__` (line 0x51e), then the BigBuffer routines and the array templates|
|`SBLT.CPP`|0000c000|SBltDestroy only|
|`SBMP.CPP`|0000c0ae..0000d43d|the row copy, the TGA helpers and the PCX decoder have no literal and are called from SBMP's functions only|
|`SCMD.CPP`|0000d43e..0000e5cf|ends with SCMD_InitStaticLists|
|`SCODE.CPP`|0000e610..0000eb41|ends with SCODE_InitStaticLists|
|`SCOMP.CPP`, `SERR.CPP`, `SEVT.CPP`|0000eb42..00011f70+|each ends with its static initialiser|

## Boundaries recorded by the last functions of the port

- `Fog/Src/ErrorManager.cpp` (shared, generated): the Mac-only `FOG_MacGetBlizzardSupportPath` (002ef7cd) is a `D2_OS_MAC` block at the end of the file. The Mac build does not compile that file yet (the shared code is not Mac-clean), so the block was checked in isolation with the clang-425 toolchain; it calls `Source::SFile::SafeCreateDirectoryTree`, which the Windows header declares `__fastcall` and the Mac one is not.
- 00002e4d and 00002e52 (`ErrorReportingSetSupportPath`) are not Blizzard code: they belong to crashy's error-reporting API, whose other entry point is `ErrorReportingLaunch` (00002e90; `crashy::ErrorReportingLaunch` on Windows). 2e52 fills the buffer 003e2cf0 that Launch reads, both are reached through the data table at 0061610b..e and called only from FOG_InitErrorManager. `coverage/units.tsv` classes them third-party.
- `Win32MacWindow.h` holds the message state (`Win32MacMsgState`, the rings, MSG, timers) as well as the window; Win32_Messages.cpp used to keep its own copy of the window struct with the child links at 0x54/0x58, where the window manager has 0x50 first child, 0x54 last child, 0x58 previous sibling, 0x5c next sibling.
- `H/StormApi.h` declares the Storm entry points the shared code calls, in the namespaces the binary's symbols and `Source/Storm/Storm.h` give them (`Storm::Source::SSTR`, `SFile`, `SMem`, `SReg`, `SLog`, `SBig`, `SBMP`, `Storm::WindowHandle`, ...), and `using`s them for the StormMac units. The definitions in `SOURCE` carry the qualified names and the parameter types of that declaration; the Mac units must not declare these functions again. Where Storm.h's type is wrong for the binary and the shared callers can take the right one (const pointers, the registry's `int` flags), `Storm.h` and `Storm/Source/*.h` have a `D2_OS_MAC` branch with the Mac type; the shared callers' casts decide the rest (`SFileOpenArchive` keeps Windows' `int*` priority, the Mac units reach it through the `int` overload in StormApi.h).
