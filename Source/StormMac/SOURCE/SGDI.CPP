// Storm's SGdi, 1.14d Intel Mac build (DiabloII_macho): the font and drawing-object
// bookkeeping SGdiDestroy tears down. Band 00017886..00017c32, ended by the file's
// static initialiser (00017be0). Mac-only, like the rest of StormMac.
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <stdint.h>

// 1.14d mac: 00010bb3
void SErrReportResourceLeak(const char* szResource);

#define SGDI_OBJECT_SIGNATURE 0x4F4D  // 'MO'

// An SGdi font or drawing object, linked into the object list by its first member.
struct SGdiObject {
    TSLink<SGdiObject> m_link;  // 0x00
    uint16_t wSignature;        // 0x08  SGDI_OBJECT_SIGNATURE
    uint16_t wKind;             // 0x0A  0: font (HSGDIFONT), else object (HSGDIOBJ)
    void* pData;                // 0x0C
};

// 1.14d mac: 003f06d8
static TSExplicitList<SGdiObject, 0> s_tObjectList;
// 1.14d mac: 003f06e4
static SGdiObject* s_pCurrentObject;
// 1.14d mac: 003f06e8
static void* s_pBuffer;
// 1.14d mac: 003f06ec
static int s_nBufferSize;
// 1.14d mac: 003f06f0
static int s_nBufferUsed;
// 1.14d mac: 003f06f4
static void* s_pScratchBuffer;
// 1.14d mac: 003f06f8
static int s_nScratchBufferSize;

static int SGDI_ResizeScratchBuffer(int nSize);

// The instantiations clang-425 emitted into this file's object (stpl.h):
// 1.14d mac: 00017a82 TSExplicitList<SGdiObject,0>::~TSExplicitList[D1]
// 1.14d mac: 00017aa4 TSExplicitList<SGdiObject,0>::DeleteNode
// 1.14d mac: 00017b14 TSLink<SGdiObject>::Unlink
// 1.14d mac: 00017b54 TSLink<SGdiObject>::NextLink
// 1.14d mac: 00017b8a TSExplicitList<SGdiObject,0>::UnlinkAll
// 1.14d mac: 00017be0 _GLOBAL__I_a

// 1.14d mac: 00017886
int SGDI_DeleteObject(SGdiObject* pObject) {
    if (pObject && pObject->wSignature == SGDI_OBJECT_SIGNATURE && pObject->wKind == 0) {
        if (s_pCurrentObject == pObject) {
            s_pCurrentObject = NULL;
        }
        if (pObject->pData) {
            SMemFree(pObject->pData, __FILE__, 280, 0);
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
    if (s_pBuffer) {
        SMemFree(s_pBuffer, __FILE__, 310, 0);
        s_pBuffer = NULL;
        s_nBufferSize = 0;
        s_nBufferUsed = 0;
    }
    return 1;
}

// 1.14d mac: 000179e0
static int SGDI_ResizeScratchBuffer(int nSize) {
    if (s_nScratchBufferSize < nSize || nSize == 0) {
        if (s_pScratchBuffer) {
            SMemFree(s_pScratchBuffer, __FILE__, 79, 0);
            s_pScratchBuffer = NULL;
            s_nScratchBufferSize = 0;
        }
        if (nSize == 0) {
            return 0;
        }
        s_pScratchBuffer = SMemAlloc(nSize, __FILE__, 86, 0);
        s_nScratchBufferSize = nSize;
    }
    return 1;
}

#endif  // D2_OS_MAC
