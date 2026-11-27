// D2GFXMacForward.cpp - the forwarders to the Mac renderer's function table that D2gfx has
// no Windows twin for.
//
// 1.14d Intel Mac (DiabloII_macho). No __FILE__ names the unit; the renderer pointer is
// reached through its PIC pointer cell, so these sit in a unit other than the one that
// defines it (D2GFXMacRenderer.cpp). See Source/D2gfx/Mac/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`.
#include "D2GFXMac.h"

#if !defined(_WIN32) && D2_OS_MAC

struct EventRecord;
struct Point;

// 1.14d mac: 002b5122
void D2GFX_DrawLine(int nXStart, int nYStart, int nXEnd, int nYEnd, uint32_t nColor, uint32_t nAlpha) {
    RENDERER_CurrentRenderedFunctions->pfnDrawLine(nXStart, nYStart, nXEnd, nYEnd, nColor, nAlpha);
}

// 1.14d mac: 002b5c1c
// A renderer is installed and the display is windowed: the state's fullscreen flag
// (005cbb90) is the dword at 0xC from 005cbb84, the cell the binary addresses it by (the state
// starts with the init option, g_nRendererInitOption).
int FUN_002b5c1c(void) {
    int bResult = 0;
    if (RENDERER_CurrentRenderedFunctions != 0) {
        bResult = (&g_nRendererInitOption)[3] == 0;
    }
    return bResult;
}

// 1.14d mac: 002b5d9a
int FUN_002b5d9a(void) {
    return 1;
}

// 1.14d mac: 002b5da4
// This and the next two forward their one argument to the renderer slot.
int FUN_002b5da4(EventRecord* pEvent) {
    if (RENDERER_CurrentRenderedFunctions != 0) {
        return RENDERER_CurrentRenderedFunctions->pfnSlot2C(pEvent);
    }
    return 0;
}

// 1.14d mac: 002b5dc3
int FUN_002b5dc3(Point* pPoint) {
    if (RENDERER_CurrentRenderedFunctions != 0) {
        return RENDERER_CurrentRenderedFunctions->pfnSlot30(pPoint);
    }
    return 0;
}

// 1.14d mac: 002b5de2
int FUN_002b5de2(Point* pPoint) {
    if (RENDERER_CurrentRenderedFunctions != 0) {
        return RENDERER_CurrentRenderedFunctions->pfnSlot34(pPoint);
    }
    return 0;
}

// 1.14d mac: 002b6035
void FUN_002b6035(void) {
}

#endif
