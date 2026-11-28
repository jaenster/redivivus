// raveDebug.cpp - the debug text slot, the empty light slot and four empty hooks.
//
// File name inferred and unattested: a placeholder for the unit that runs 102b4798-102b4878 and
// ends with __sinit_raveDebug_cpp at 102b486c. The Windows renderers keep DrawString and
// DrawLight in their engine units. See Source/D2Rave/README.md.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

namespace D2Rave {

// Windows slot 51. Empty on RAVE; asm only in the database.
// mac ppc classic/1.09d: 102b4798 (1.00: 1007a8f8)
void RaveDrawLight(uint32_t * pLight, uint32_t * pColor, int nX, int nY) {
    (void)pLight;
    (void)pColor;
    (void)nX;
    (void)nY;
}

// Windows slot 50: printf-style debug text in font 13, only inside a frame.
// mac ppc classic/1.09d: 102b479c (1.00: 1007a8fc)
void RaveDrawString(int nX, int nY, const char * szFormat, va_list args) {
    if (gbRaveInDraw == 0) {
        return;
    }
    char szText[524];
    Unicode wszText[0x200];
    vsprintf(szText, szFormat, args);
    Unicode::toUnicode(wszText, szText, 0x200);
    int nOldFont = D2Win_SetFont(13);
    D2Win_DrawText(wszText, nX, nY + 12, 0, 0);
    D2Win_SetFont(nOldFont);
}

// The four empty hooks sInitRaveContext and sCloseRaveContext call when gbRaveModeFlag is set
// (in place of the Smacker texture setup and teardown).
// mac ppc classic/1.09d: 102b485c (1.00: 1007a9bc)
void sRaveHookStub0() {
}

// mac ppc classic/1.09d: 102b4860 (1.00: 1007a9c0)
void sRaveHookStub1() {
}

// mac ppc classic/1.09d: 102b4864 (1.00: 1007a9c4)
void sRaveHookStub2() {
}

// mac ppc classic/1.09d: 102b4868 (1.00: 1007a9c8)
void sRaveHookStub3() {
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
