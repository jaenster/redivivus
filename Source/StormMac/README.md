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
|`unknown_events.cpp`|000216b0..00021a94|7|beta: a separate unit linked before Win32_Files.cpp|
|`unknown_closehandle.cpp`|00025660..000257bc|1|separate from the handle table: it calls GetWin32HandlePtr out of line, inlined if merged|
|`unknown_handles.cpp`|000257bc..00026d14|3|ends with the static initialiser 00026c70|
|`unknown_memory.cpp`|000289b8..00028e09|19|beta: a unit of its own (SMemZero/Fill/Copy are SMem_Mac.cpp in the beta, but sit inside this run in 1.14d)|
|`unknown_messages.cpp`|00028e09..00029968|19|beta: a unit of its own|
|`unknown_priv.cpp`|00029968..00029b68|6|beta: Priv_ functions linked before Win32_Process.cpp; not in it in 1.14d (ExitProcess's body changes Priv_Alloc if merged)|
|`unknown_threads.cpp`|0002bdea..0002c0ee|11|beta: a unit linked before Win32_utils.cpp|

The `unknown_*` names are descriptive: no binary records those files' names. The folder
of the `Win32_*` files other than Win32_utils.cpp is inferred from it, and the header
`Win32Mac.h` has no known original name. The beta also names files 1.14d may still have
(WinCompat.cpp, Win32_Registry.cpp, Allocation.cpp, FPMem.cpp, fp_assert.cpp,
MessageBox.cpp, Utils.cpp, MousePosition.cpp, VolumeIsCD.cpp, ...); code from them goes
under those names when it is reconstructed.
