# D2gfx Mac glue

The Mac-only functions of D2gfx in the 1.14d Intel Mac build (`DiabloII_macho`): the surface
and palette slots of the helper table, the software tile blitters, the renderer switch, the
forwarders only the Mac renderer table has, the display-mode enumeration (MACGFX) and the
code-signature check (MACSEC). Built by `cmake/mac.cmake` (compile only), measured by
`asm_matching/mac/cmp.ts`. Every file compiles to nothing unless `D2_OS_MAC`; the files that call the Carbon-era layer (Win32Mac, StormMac, the Mac renderer forwarders) only under `D2_MAC_CARBON`.

No `__FILE__` names these units. The binary files them among D2gfx's shared functions
(`D2GFX.cpp`), so the split here follows how each unit reaches its globals: a global the
translation unit defines is addressed directly, any other through its PIC pointer cell.

|file|1.14d|split decided by|
|-|-|-|
|`D2GFXMacHelper.cpp`|002b18a8..002b1983|defines the surface (005c8a6c) and the palette copies; the tile code reaches the surface indirectly|
|`D2GFXMacTiles.cpp`|002b1a44..002b4696|defines the clip rectangles, row table and light grid; reaches the surface and shade table indirectly|
|`D2GFXMacRenderer.cpp`|002b4b72, 002b4d60|defines the renderer pointer and state directly|
|`D2GFXMacForward.cpp`|002b5122..002b6035|reaches the renderer pointer indirectly|
