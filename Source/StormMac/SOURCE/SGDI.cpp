// Storm's SGdi, 1.14d Intel Mac build (DiabloII_macho): the font and drawing-object
// bookkeeping SGdiDestroy tears down. Band 00017886..00017c32, ended by the file's
// static initialiser (00017be0). Mac-only, like the rest of StormMac.
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <stdint.h>

// 1.14d mac: 00010bb3
void SErrReportResourceLeak(const char* szResource);

// The path the original's SGDI_FILE names.
#define SGDI_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SGDI.CPP"

#define SGDI_OBJECT_SIGNATURE 0x4F4D  // 'MO'

// An SGdi font or drawing object, linked into the object list by its first member.
struct SGdiObject {
    TSLink<SGdiObject> m_link;  // 0x00
    uint16_t wSignature;        // 0x08  SGDI_OBJECT_SIGNATURE
    uint16_t wKind;             // 0x0A  0: font (HSGDIFONT), else object (HSGDIOBJ)
    void* pData;                // 0x0C
};

// 1.14d mac: 003f06d8
static TSLinkList<SGdiObject> s_tObjectList;
// 1.14d mac: 003f06e4
// The globals below are read by the rest of SGDI.CPP (SGdi's drawing calls, not in this band).
SGdiObject* g_pSGdiCurrentObject;
// 1.14d mac: 003f06e8
void* g_pSGdiBuffer;
// 1.14d mac: 003f06ec
int g_nSGdiBufferSize;
// 1.14d mac: 003f06f0
int g_nSGdiBufferUsed;
// 1.14d mac: 003f06f4
void* g_pSGdiScratchBuffer;
// 1.14d mac: 003f06f8
int g_nSGdiScratchBufferSize;

int SGDI_ResizeScratchBuffer(int nSize);

// The instantiations clang-425 emitted into this file's object (stpl.h):
// 1.14d mac: 00017a82 TSLinkList<SGdiObject>::~TSLinkList[D1]
// 1.14d mac: 00017aa4 TSLinkList<SGdiObject>::DeleteNode
// 1.14d mac: 00017b14 TSLink<SGdiObject>::Unlink
// 1.14d mac: 00017b54 TSLink<SGdiObject>::NextLink
// 1.14d mac: 00017b8a TSLinkList<SGdiObject>::UnlinkAll
// 1.14d mac: 00017be0 _GLOBAL__I_a

// 1.14d mac: 00017886
int SGDI_DeleteObject(SGdiObject* pObject) {
    if (pObject && pObject->wSignature == SGDI_OBJECT_SIGNATURE && pObject->wKind == 0) {
        if (g_pSGdiCurrentObject == pObject) {
            g_pSGdiCurrentObject = NULL;
        }
        if (pObject->pData) {
            SMemFree(pObject->pData, SGDI_FILE, 280, 0);
            pObject->pData = NULL;
        }
        s_tObjectList.DeleteNode(pObject);
        return 1;
    }
    return 0;
}

// 1.14d mac: 0001790c
int SGdiDestroy(void) {
    SGdiObject* pObject;
    while ((pObject = s_tObjectList.Head()) != NULL) {
        const char* szResource;
        if (pObject->wKind == 0) {
            szResource = "HSGDIFONT";
        } else {
            szResource = "HSGDIOBJ";
        }
        SErrReportResourceLeak(szResource);
        SGDI_DeleteObject(pObject);
    }
    SGDI_ResizeScratchBuffer(0);
    if (g_pSGdiBuffer) {
        SMemFree(g_pSGdiBuffer, SGDI_FILE, 310, 0);
        g_pSGdiBuffer = NULL;
        g_nSGdiBufferSize = 0;
        g_nSGdiBufferUsed = 0;
    }
    return 1;
}

// 1.14d mac: 000179e0
// Called with other sizes by the drawing calls of this file that are not in this band.
int SGDI_ResizeScratchBuffer(int nSize) {
    if (g_nSGdiScratchBufferSize < nSize || nSize == 0) {
        if (g_pSGdiScratchBuffer) {
            SMemFree(g_pSGdiScratchBuffer, SGDI_FILE, 79, 0);
            g_pSGdiScratchBuffer = NULL;
            g_nSGdiScratchBufferSize = 0;
        }
        if (nSize == 0) {
            return 0;
        }
        g_pSGdiScratchBuffer = SMemAlloc(nSize, SGDI_FILE, 86, 0);
        g_nSGdiScratchBufferSize = nSize;
    }
    return 1;
}

#endif  // D2_OS_MAC
