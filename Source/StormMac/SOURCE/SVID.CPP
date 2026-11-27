// Storm Mac: SVID.CPP, named by the __FILE__ literal of its frees (1.14d Intel Mac 0001de67).
// Video playback: the list of open videos, its teardown, and the list's instantiations.
// 1.14d band 0001de67..0001e292. The launch-URL helper after it (0001e292) is not named by
// any literal; it follows this file in address order and is kept here (see
// Source/StormMac/README.md).
#include "Source/StormMac/H/stpl.h"
#include <string.h>

#if !defined(_WIN32) && D2_OS_MAC

// 1.14d mac: 00010b98
void SErrSetLastError(uint32_t dwErrCode);
// 1.14d mac: 00010bb3
void SErrReportResourceLeak(char* szMessage);
// 1.14d mac: 0000e610
int SCodeDelete(int hCodeStream);
// 1.14d mac: 0001b087
void SRgnDelete(int hRgn);
// 1.14d mac: 00029fec
int FreeLibrary(int hModule);
// 1.14d mac: 00029e4b
int CloseHandle(int hObject);
// 1.14d mac: 0003411f
int LaunchURL(char* szURL, int nLength, int (*pfnConfirm)(void), void* pArg);
// 1.14d mac: 00036d5e
bool STORM_NotifyCallback(bool bRequested);

#define ERROR_INVALID_PARAMETER 0x57

// An open video. The size of the record is not known: the code that allocates it is outside
// this file's band.
class SVid : public TSLinkedNode<SVid> {
public:
    int m_hFile;           // 0x08
    uint8_t m_aPad0C[0x20];
    uint32_t m_dwBufSize;  // 0x2C
    uint8_t m_aPad30[0x1C];
    void* m_pBuffer;       // 0x4C
    uint8_t m_aPad50[0x8];
    void* m_pFrame;        // 0x58
    uint8_t m_aPad5C[0x34];
    int m_ahRgn[2];        // 0x90
    uint8_t m_aPad98[0x8];
    void* m_pPalette;      // 0xA0
};

// Out-of-line destructor: see STransList in STRANS.cpp.
class SVidList : public TSExplicitList<SVid, 0> {
public:
    ~SVidList();
};
SVidList::~SVidList() {}

// The list's instantiations, by the qualified name the compare tool pairs them with:
// 1.14d mac: 0001e0ce SVidList::~SVidList[D1]
// 1.14d mac: 0001e182 TSLink<SVid>::Unlink
// 1.14d mac: 0001e1c2 TSLink<SVid>::NextLink
// 1.14d mac: 0001e1f8 TSExplicitList<SVid,0>::UnlinkAll
// 1.14d mac: 0001e240 _GLOBAL__I_a

// 1.14d mac: 003f0e48
SVidList g_tVideoList;
// The four code streams the player keeps.
// 1.14d mac: 003f0e54
int* g_pCodeStreams;
// 1.14d mac: 003f0e58
int g_hVideoLibrary;

static int SVID_IsVideoInList(SVid* pVideo) __attribute__((noinline));
// 1.14d mac: 0001e067
static int SVID_IsVideoInList(SVid* pVideo) {
    for (SVid* pNode = g_tVideoList.Head(); pNode != NULL; pNode = g_tVideoList.Next(pNode)) {
        if (pNode == pVideo) {
            return 1;
        }
    }
    return 0;
}

// 1.14d mac: 0001e0f0
// Deletes the node's two regions and the node, and returns the one after it.
SVid* SVID_DeleteVideoNode(SVidList* pList, SVid* pNode) {
    SVid* pNext = pList->Next(pNode);
    for (int i = 0; i < 2; i++) {
        SRgnDelete(pNode->m_ahRgn[1 - i]);
    }
    pNode->m_link.Unlink();
    pNode->~SVid();
    SMemFree(pNode, "", -2, 0);
    return pNext;
}

// 1.14d mac: 0001df6a
int SVidPlayEnd(SVid* hVideo) {
    if (hVideo == NULL) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (SVID_IsVideoInList(hVideo) && hVideo->m_pFrame != NULL) {
        SMemFree(hVideo->m_pFrame, __FILE__, 0x51b, 0);
        hVideo->m_pFrame = NULL;
        if (hVideo->m_hFile != 0) {
            CloseHandle(hVideo->m_hFile);
            hVideo->m_hFile = 0;
        }
        if (hVideo->m_pBuffer != NULL) {
            SMemFree(hVideo->m_pBuffer, __FILE__, 0x524, 0);
            hVideo->m_pBuffer = NULL;
            hVideo->m_dwBufSize = 0;
        }
        if (hVideo->m_pPalette != NULL) {
            SMemFree(hVideo->m_pPalette, __FILE__, 0x52a, 0);
        }
        SVID_DeleteVideoNode(&g_tVideoList, hVideo);
        return 1;
    }
    return 0;
}

// 1.14d mac: 0001de67
int SVidDestroy(void) {
    SVid* pVideo;
    while ((pVideo = g_tVideoList.Head()) != NULL) {
        SErrReportResourceLeak("HSVIDEO");
        SVidPlayEnd(pVideo);
    }
    if (g_pCodeStreams != NULL) {
        SCodeDelete(g_pCodeStreams[0]);
        SCodeDelete(g_pCodeStreams[1]);
        SCodeDelete(g_pCodeStreams[2]);
        SCodeDelete(g_pCodeStreams[3]);
        SMemFree(g_pCodeStreams, __FILE__, 0x272, 0);
        g_pCodeStreams = NULL;
    }
    if (g_hVideoLibrary != 0) {
        FreeLibrary(g_hVideoLibrary);
        g_hVideoLibrary = 0;
    }
    return 1;
}

// 1.14d mac: 0001e2d0
// LaunchURL's confirmation callback: asks the registered callback, then declines.
int STORM_LaunchURLCallback(void) {
    STORM_NotifyCallback(true);
    return 0;
}

// 1.14d mac: 0001e292
void STORM_LaunchURL(char* szURL) {
    LaunchURL(szURL, strlen(szURL), STORM_LaunchURLCallback, NULL);
}

// 1.14d mac: 003f0e5c
uint32_t g_dwStoredValue;

// 1.14d mac: 0001e2e9
void STORM_StoreValue(uint32_t dwValue) {
    g_dwStoredValue = dwValue;
}

#endif  // D2_OS_MAC
