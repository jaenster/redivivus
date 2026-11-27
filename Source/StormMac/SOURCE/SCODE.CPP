// StormMac Storm Mac/SOURCE/SCODE.CPP: the code-stream objects (SCodeDelete, SCodeDestroy)
// and the TSList instantiations the file's object holds, 1.14d Intel Mac
// 0000e610..0000eab0 (the static initialiser closes it).
//
// __FILE__ is ".../StormMac/Storm Mac/SOURCE/SCODE.CPP". The list helpers occur once per
// list type the file instantiates (the streams, the blocks); the instantiations are
// tagged at the file's end.
#include "../Win32Mac/Source/Win32/Win32Mac.h"
#pragma GCC visibility push(hidden)
#include "../H/stpl.h"
#pragma GCC visibility pop

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SMemZero(void* pDest, size_t nSize);
void SErrSetLastError(DWORD dwErrCode);
void SErrReportResourceLeak(const char* szType);

#define SCODE_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SCODE.CPP"

// A code stream: the list link, then the three buffers the stream owns.
struct SCodeStream : TSLinkedNode<SCodeStream> {
    uint8_t abyHead[0x14];
    void* pBuffer1;  // 0x1c
    uint8_t abyMid1[0x34 - 0x20];
    void* pBuffer2;  // 0x34
    uint8_t abyMid2[0x4c - 0x38];
    void* pBuffer3;  // 0x4c
};

// A block record of the block list.
struct SCodeBlock : TSLinkedNode<SCodeBlock> {};

// A buffer descriptor (pointer, size, capacity), freed at shutdown.
struct SCodeBuffer {
    void* pData;
    uint32_t nSize;
    uint32_t nCapacity;
};

// 1.14d mac: 003f0270
static TSExplicitList<SCodeBlock, 0> s_tBlockList;
// 1.14d mac: 003f027c
// The live code streams.
static TSExplicitList<SCodeStream, 0> s_tStreamList;
// 1.14d mac: 003f0288
static SCodeBuffer s_tBuffer1;
// 1.14d mac: 003f0294
static SCodeBuffer s_tBuffer2;

// 1.14d mac: 0000e610
// Frees the three buffers of a code stream, then the stream.
BOOL SCodeDelete(SCodeStream* pCodeStream) {
    if (pCodeStream == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    if (pCodeStream->pBuffer1 != NULL) {
        SMemFree(pCodeStream->pBuffer1, SCODE_FILE, 0x454, 0);
        pCodeStream->pBuffer1 = NULL;
    }
    if (pCodeStream->pBuffer2 != NULL) {
        SMemFree(pCodeStream->pBuffer2, SCODE_FILE, 0x458, 0);
        pCodeStream->pBuffer2 = NULL;
    }
    if (pCodeStream->pBuffer3 != NULL) {
        SMemFree(pCodeStream->pBuffer3, SCODE_FILE, 0x45c, 0);
        pCodeStream->pBuffer3 = NULL;
    }
    s_tStreamList.DeleteNode(pCodeStream);
    return 1;
}

// 1.14d mac: 0000e6e7
// Every stream still alive is a leak, then deleted.
BOOL SCodeDestroy(void) {
    while (!TSLink<SCodeStream>::IsTerminator(s_tStreamList.m_terminator.m_next)) {
        SErrReportResourceLeak("HSCODESTREAM");
        SCodeDelete(s_tStreamList.Head());
    }
    if (s_tBuffer1.pData != NULL) {
        SMemFree(s_tBuffer1.pData, SCODE_FILE, 0x473, 0);
    }
    SMemZero(&s_tBuffer1, sizeof(s_tBuffer1));
    if (s_tBuffer2.pData != NULL) {
        SMemFree(s_tBuffer2.pData, SCODE_FILE, 0x476, 0);
    }
    SMemZero(&s_tBuffer2, sizeof(s_tBuffer2));
    s_tBlockList.DeleteAll();
    return 1;
}

template void TSLink<SCodeStream>::Unlink();
template TSLink<SCodeStream>* TSLink<SCodeStream>::NextLink(int);
template void TSExplicitList<SCodeStream, 0>::UnlinkAll();
template SCodeStream* TSExplicitList<SCodeStream, 0>::DeleteNode(SCodeStream*);
template TSExplicitList<SCodeStream, 0>::~TSExplicitList();

template void TSLink<SCodeBlock>::Unlink();
template TSLink<SCodeBlock>* TSLink<SCodeBlock>::NextLink(int);
template void TSExplicitList<SCodeBlock, 0>::UnlinkAll();
template void TSExplicitList<SCodeBlock, 0>::DeleteAll();
template TSExplicitList<SCodeBlock, 0>::~TSExplicitList();

// 1.14d mac: 0000e7f4 TSExplicitList<SCodeBlock,0>::~TSExplicitList[D2]
// 1.14d mac: 0000e816 TSExplicitList<SCodeStream,0>::~TSExplicitList[D2]
// 1.14d mac: 0000e838 TSExplicitList<SCodeStream,0>::DeleteNode
// 1.14d mac: 0000e8a8 TSLink<SCodeStream>::Unlink
// 1.14d mac: 0000e8e8 TSLink<SCodeStream>::NextLink
// 1.14d mac: 0000e91e TSExplicitList<SCodeStream,0>::UnlinkAll
// 1.14d mac: 0000e966 TSExplicitList<SCodeBlock,0>::UnlinkAll
// 1.14d mac: 0000e9ae TSLink<SCodeBlock>::Unlink
// 1.14d mac: 0000e9ee TSLink<SCodeBlock>::NextLink
// 1.14d mac: 0000ea24 TSExplicitList<SCodeBlock,0>::DeleteAll
// 1.14d mac: 0000eab0 _GLOBAL__I_a

#endif  // D2_OS_MAC
