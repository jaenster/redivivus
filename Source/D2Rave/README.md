# D2Rave

The RAVE renderer of the classic Mac OS PowerPC builds: the D2gfx renderer for
`DISPLAYTYPE_RAVE` (7), drawing through QuickDraw 3D RAVE with a DrawSprocket full-screen
context. It exists only in the non-Carbon PowerPC builds (1.00 to 1.13d); the Carbon builds and
every Windows and Intel Mac build have no RAVE. Reconstructed by hand from
`mac/powerpc/classic/1.09d/Diablo_II` (CodeWarrior, PEF/CFM, big-endian), with
`classic/1.00` as the earliest form.

Classic Mac PowerPC only. Every file compiles to nothing unless the target is classic Mac
PowerPC (`#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC`, the axes from compiler flags), and
the folder is left out of every build here: CMakeLists.txt filters it out of the game sources and
include path, and the VC8 oracle lists it in `NOT_GAME`. It is checked against the stub headers
in `Redivivus/Platform/MacClassic/` (see the README there for the command).

## What is attested and what is inferred

**`D2Rave` is an inferred module name**, and so is `Src/`: no folder name is attested. The
code calls itself RAVE (`DISPLAYTYPE_RAVE`, `CRaveTextures`, `Rave*` log prefixes); the name and
the `Source/<Module>/Src/<prefix>Core.cpp` shape follow the Windows renderers.

The file split is measured, not guessed: Metrowerks ends every translation unit that has a
dynamic initialiser with a 12-byte `__sinit_<file>`, and D2Rave has 13 of them. Six file names
are attested as the PPC build's `__FILE__` strings (bare basenames under CodeWarrior); the rest
are inferred, chosen to keep the alphabetical order the attested ones show.

|File|1.09d range|Name|
|-|-|-|
|`raveBink.cpp`|102b1478-102b1744|inferred (Windows keeps Bink playback in its renderer units)|
|`raveBlocks.cpp`|102b1744-102b1d1c|**attested** `__FILE__`|
|`raveCore.cpp`|102b1d1c-102b479c|**attested** `__FILE__`|
|`raveDebug.cpp`|102b479c-102b4878|inferred placeholder, unattested|
|`raveEngine.cpp`|102b4878-102b5614|inferred (Windows `glEngine.cpp`/`d3dEngine.cpp` hold `DrawRect*`)|
|`raveFloor.cpp`|102b5614-102b5db8|inferred (Windows `glFloorCache.cpp`/`d3dFloorCache.cpp`)|
|`ravePerspective.cpp`|102b5db8-102b6164|**attested** `__FILE__`|
|`raveSmack.cpp`|102b6164-102b7368|**attested** `__FILE__`|
|`raveSprite.cpp`|102b7368-102b95fc|**attested** `__FILE__`|
|`raveVertex.cpp`|102b95fc-102b98bc|**attested** `__FILE__`|
|`raveWall.cpp`|102b98bc-102bba50|inferred (Windows `glWall.cpp`/`d3dWall.cpp`)|
|`CRaveTextures.cpp`|102bba50-102bc040|file inferred; **class name attested** by its RTTI string, base `CD2Textures` likewise|
|`raveClip.cpp`|102bc040-102bd068|inferred; `QADrawClippedVTexture` is named by its own log string|

Function names: the `Rave*` log prefixes are attested (`RaveDetect`, `RaveInit`,
`RaveCreateSurface`, `RaveCloseSurface`, `RaveStartDraw`, `RaveEndDraw`, `RaveChangeRes`,
`RaveSetFullScreen`, `RavePlayBinkFile`, `RaveCutsceneCreateWindow`, `RaveCheckError`,
`iRaveSwapContext`, `sInitRaveContext`, `sInitRaveDrawSprocket`, `sCloseRaveDrawSprocket`,
`QADrawClippedVTexture`; `RaveScreenGrab` in 1.00). Table slots without a log string take the
`Rave` prefix and the slot's role; helpers are named by behaviour (`s` prefix for file statics).
Each function carries its 1.09d address and its 1.00 pair in a `// mac ppc classic/1.09d:`
comment, and the `SMemAlloc`/`SMemFree` line numbers are the original source lines.

## Files that are ours

|File|What|
|-|-|
|`D2Rave.h`|the public interface: the renderer table and the slot functions, plain types only|
|`Src/raveInternal.h`|shared state, the engine calls D2Rave makes, every function's prototype|
|`Src/CRaveTextures.h`|the class, and the PPC form of its `CD2Textures` base|
|`Src/raveRenderer.cpp`|the table, typed with the shared `D2RendererFunctionsStrc` of `Source/D2gfx/D2gfx.public.h`|

The Mac table has 57 slots (56 in 1.00); the shared struct has the 54 Windows 1.14d slots.
Mac slots 0-10 are Windows 0-10, Mac 14-56 are Windows 11-53, and Mac 11-13
(`RaveProcessEvent`, `RaveGlobalToLocal`, `RaveLocalToGlobal`) have no Windows slot. This is the
same mapping D2OpenGL's `D2gfxRenderer.cpp` uses for its own 57-slot Mac table.

`raveInternal.h` declares the engine functions D2Rave calls (Storm, Fog, D2CMP, D2Win, the Mac
shell, CD2Textures) instead of including their headers, because the shared tree headers do not
compile on a Mac target yet. Where the tree has a name for the same function, that name is used.

## Not reconstructed yet

The large rasterisers: `RaveDrawGroundTile`, the wall and shadow-tile flat and perspective
drawers, `RaveDrawPerspectiveImage`, `RaveDrawClippedImage`,
`RaveDrawVerticalCropImage`, the sprite upload and quad drawers, the three Smacker players and
`QADrawClippedVTexture` with its polygon clipper. They are declared, so the table is complete;
the list with addresses is in `d2-surveys/macppc/d2rave/recon.md`.
