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
|`Source/StormMac/SOURCE/`|`Storm Mac/SOURCE/` (Storm proper: SBig.cpp, SBLT.CPP, SCMD.CPP, SFILE.CPP, SRegMac.mm, IOUnit/... - nothing reconstructed yet)|
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
|`Win32_Process.cpp`|00029b68..0002b1fe|31 of 43|beta `__sinit_Win32_Process_cpp`, same order; SetLastError inlined into GetModuleFileName|
|`Win32_TimeAndDate.cpp`|0002c0ee..0002c670|16 of 16|beta `__sinit_Win32_TimeAndDate_cpp`, same order|
|`Win32_Window.cpp`|0002eae0..00031b0c|65|window manager (classes, window tree, focus, paint, props, key maps); no binary records the file name, `Win32_Window` follows the Win32_<Family> naming. Key tables 00031a84/00031af6 end the run before the region code (00031b0d, Win32_Region) |
|`unknown_events.cpp`|000216b0..00021a94|7|beta: a separate unit linked before Win32_Files.cpp|
|`unknown_closehandle.cpp`|00025660..000257bc|1|separate from the handle table: it calls GetWin32HandlePtr out of line, inlined if merged|
|`unknown_handles.cpp`|000257bc..00026d14|3|ends with the static initialiser 00026c70|
|`unknown_memory.cpp`|000289b8..00028e09|19|beta: a unit of its own (SMemZero/Fill/Copy are SMem_Mac.cpp in the beta, but sit inside this run in 1.14d)|
|`unknown_messages.cpp`|00028e09..00029968|19|beta: a unit of its own|
|`unknown_priv.cpp`|00029968..00029b68|6|beta: Priv_ functions linked before Win32_Process.cpp; not in it in 1.14d (ExitProcess's body changes Priv_Alloc if merged)|
|`unknown_threads.cpp`|0002bdea..0002c0ee|11|beta: a unit linked before Win32_utils.cpp|
|`MacDisplayDriver.cpp`|00036830..000371f9 (less the unnamed 00036dc7..00036f91)|46|name unrecorded: one driver table (default display driver: cursor counter, palette, bounds) with its registration calls, then Accel_DeleteAcceleratorList and MACDISPLAY_FindBestDisplayMode by address order; D2OpenGL imports the MACCURSOR_*/MACDISPLAY_* names. Palette get/set stay DIFF: the original's indexed displacement is a link-time address|
|`Win32_Region.cpp`|00031b0d..00032b50|21|name unrecorded: the strip-region run between the window manager's key-code helpers and the DirectX shims; one heap-block region type, so one unit|
|`Win32_DirectSound.cpp`|00032b50..00033a24|31|name unrecorded: DirectDrawCreate, DirectSoundCreate and the IDirectSound/IDirectSoundBuffer/IKsPropertySet bodies share the sound-buffer globals 003f481c..003f4828|
|`Win32_String.cpp`|00033a24..00033cef|11|name unrecorded: the CRT string and number helpers (_stricmp .. srand) before StormMac's own SOURCE run begins at 00033cef; several have Windows twins|

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
|`SNET.cpp`|00019532..0001ae80|ends with `__sinit_SNET_CPP`|
|`SRGN.cpp`|0001b087..0001c6f0|region code only, no literal names the file; ends with `SRGN_InitStatics`|
|`SSTR.cpp`|0001c730..0001d2f9|SStrLower (0001d2f9) included|
|`SSignature.cpp`|0001d302..0001d69e||
|`STRANS.cpp`|0001d69e..0001de67|the UTF-8/UTF-16/Latin-1 helpers have no file evidence; placed here with their tables|
|`SVID.cpp`|0001de67..0001e2fd|includes STORM_LaunchURL and its callback|
|`unknown_allocators.cpp`|0001e2fd|`Mem_SetDefaultAllocators`, no file name recorded|
|`SDRAW_VidDriver.cpp`|00036035|`SDrawDestroy`|

Extensions are lowercase `.cpp` because the compare and coverage tools scan only that.
