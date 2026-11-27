// StormMac Storm Mac/SOURCE/SBLT.CPP: the blitter's shutdown, 1.14d Intel Mac 0000c000.
//
// The binary's __FILE__ is ".../StormMac/Storm Mac/SOURCE/SBLT.CPP" (SMemFree, line 0x5c).
// It is the only function of the file before SBMP.CPP.
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SMemZero(void* pDest, size_t nSize);
void SCodeDelete(int hCodeStream);

// 1.14d mac: 003f0238
// The blit table: 0x100 entries of 0x10 bytes; the second word is a compiled code stream.
static void* s_pBlitTable;

// 1.14d mac: 003f023c
// Two fixed entries of three words each, the first of which is a code stream.
static int s_atFixedEntries[2][3];

// 1.14d mac: 0000c000
BOOL SBltDestroy(void) {
    if (s_pBlitTable != NULL) {
        int* pEntry = &s_atFixedEntries[0][0];
        for (int i = 2; i != 0; i--) {
            SCodeDelete(*pEntry);
            SMemZero(pEntry, 0xc);
            pEntry += 3;
        }
        for (int nOffset = 0; nOffset != 0x1000; nOffset += 0x10) {
            int hCodeStream = *(int*)((char*)s_pBlitTable + nOffset + 4);
            if (hCodeStream != 0) {
                SCodeDelete(hCodeStream);
            }
        }
        SMemFree(s_pBlitTable, "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SBLT.CPP", 0x5c, 0);
        s_pBlitTable = NULL;
    }
    return 1;
}

#endif  // D2_OS_MAC
