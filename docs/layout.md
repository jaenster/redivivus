# How the tree is laid out

The tree is organised by the module names the game itself uses (`D2Client`, `D2Common`,
`D2Game`, `D2Win`, `D2gfx`, `Fog`, ...), and inside each module by the folders Blizzard's own
source paths show where the binaries record them (for example `D2Game/Quests/`, `D2Win/Src/`).

|Directory|What is in it|
|-|-|
|`Source/<module>/`|Blizzard's modules, one directory each, generated|
|`Source/D2Headers/`|the shared headers `d2_platform.h` and `d2_enums.h`|
|`Source/globals/`|the game's global data, generated|
|`3rdparty/Storm/`, `3rdparty/crashy/`|Storm and the crash-report library, generated|
|`Redivivus/`|hand-written support code: the host glue, tracing, the crash handler|
|`3rdparty/libbacktrace/`, `3rdparty/implibs/`|vendored code and the import-library definitions|
|`project.json`|the module table (below)|

## Modules

Each module builds into its own static library, and the executable links them whole in the
order of the 1.14d link. 1.14d merged the separate DLLs of earlier versions into one
`Game.exe`; the libraries are those DLLs again, linked the way 1.14d links them.

`project.json` holds the module table: each module's directory, kind, the modules it may use
with the evidence for each (the 1.13d DLL import tables), documented exceptions, the link order,
and whether the module is strict. `Source/modules.cmake` is generated from it, and the configure
step fails when the two disagree.

Includes are rooted at the tree: `#include "Source/D2Common/ITEMS/Items.h"`. Every file
includes what it uses and forward-declares types it only points at.

## The module gates

The build writes `build/module-gates.tsv`: every include that crosses a module boundary the
original 1.13d imports did not allow, per pair of modules, and `build/module-gates-summary.tsv`,
one row per module. A strict module fails the build on such an include instead of reporting it,
so once a module has none it cannot gain one again. The goal is every module strict.
