// Storm Mac: SNET.CPP, named by the __FILE__ literal its allocations carry (1.14d Intel Mac
// 00019b0b's SMemAlloc, line 0x52f). Storm's network layer: the connection records and their
// packet queues, the packet header and its checksum, the packet dump log, and the leave/destroy
// paths. 1.14d Mac band 00019532..0001b086: the file's functions and the container
// instantiations its object holds, then its static initialiser (0001ae80). What follows the
// initialiser (0001b087 on) is the region table, SRGN, which has its own file.
//
// Every list here keeps its link as the first member of the element, so a list is walked by the
// element address itself: Link() is the identity, and the link offset the list stores (0) is
// only read by LinkNode's NextLink call.
#include "Source/StormMac/H/stpl.h"
#include "Source/StormMac/Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include "Source/StormMac/H/StormApi.h"

#include <pthread.h>
#include <string.h>

// ---- what the file calls outside itself ----

// 1.14d mac: 000115a5
int SEvtUnregisterType(uint32_t nEventId, uint32_t nSubId);
// 1.14d mac: 0002c035
void Sleep(uint32_t dwMilliseconds);
// 1.14d mac: 000217de
BOOL SetEvent(HANDLE hEvent);
// 1.14d mac: 00029fec
BOOL FreeLibrary(HANDLE hModule);
// 1.14d mac: 00028d67
BOOL VirtualUnlock(void* pAddress, unsigned long dwSize);
// 1.14d mac: 00028d2e
BOOL VirtualFree(void* pAddress, unsigned long dwSize, uint32_t dwFreeType);
// 1.14d mac: 0001839a
void SLogDump(uint32_t nHandle, const uint8_t* pData, uint32_t nLength);
// The log's indent: set to a column, or moved by a delta (named for what dump_packet does with them).
// 1.14d mac: 00018872
uint32_t SLogSetAbsIndent(uint32_t nHandle, uint32_t nIndent);
// 1.14d mac: 0001891e
void SLogSetTimestamp(uint32_t nHandle, uint32_t nValue);
// 1.14d mac: 000176a0
void CRITSECT_Init(pthread_mutex_t* pMutex);
// 1.14d mac: 00016a72
// The destructor SFileCritSect::~SFileCritSect[D2] (SFILE.CPP, 00016a72), which the statics register with __cxa_atexit.
void CRITSECT_Destroy(pthread_mutex_t* pMutex) __attribute__((visibility("hidden"))) __asm__("__ZN13SFileCritSectD2Ev");
// 1.14d mac: 00018860
BOOL SLogIsInitialized(void);

#define SNET_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SNET.CPP"

#define SNET_TYPE_SYSTEM 0
#define SNET_TYPE_DATAGRAM 3
#define SNET_SUB_INITIALCONTACT 1
#define SNET_SUB_PLAYERLEAVE 11
#define SNET_NOPLAYER 0xFF
#define SNET_QUEUE_COUNT 4

// ---- the list ----

// A list whose elements hold their TSLink as the first member (TSLinkedNode).
template<class T>
class TSList {
public:
    int m_linkoffset;        // 0x00
    TSLink<T> m_terminator;  // 0x04

    TSList() : m_linkoffset(0) {
        m_terminator.m_prevlink = &m_terminator;
        m_terminator.m_next = (T*)((intptr_t)&m_terminator | 1);
    }
    ~TSList() { UnlinkAll(); }

    TSLink<T>* Link(T* pNode) { return pNode ? (TSLink<T>*)pNode : &m_terminator; }
    T* Head() { return m_terminator.Next(); }
    void UnlinkAll() {
        T* pNode;
        while ((pNode = Head()) != NULL) {
            Link(pNode)->Unlink();
        }
    }
    void LinkNode(T* pNode, uint32_t nLinkType, T* pExisting);
    T* NewNode(uint32_t nLinkType, uint32_t nExtraBytes, uint32_t dwFlags);
    T* DeleteNode(T* pNode);
    void Clear();
};

template<class T>
void TSList<T>::LinkNode(T* pNode, uint32_t nLinkType, T* pExisting) {
    TSLink<T>* pLink = Link(pNode);
    if (pLink->m_prevlink != NULL) {
        pLink->Unlink();
    }
    TSLink<T>* pExistingLink = Link(pExisting);
    if (nLinkType == STORM_LIST_LINK_AFTER) {
        pLink->m_prevlink = pExistingLink;
        pLink->m_next = pExistingLink->m_next;
        pExistingLink->NextLink(m_linkoffset)->m_prevlink = pLink;
        pExistingLink->m_next = pNode;
    } else if (nLinkType == STORM_LIST_LINK_BEFORE) {
        TSLink<T>* pPrev = pExistingLink->m_prevlink;
        pLink->m_prevlink = pPrev;
        pLink->m_next = (T*)pPrev->m_next;
        pPrev->m_next = pNode;
        pExistingLink->m_prevlink = pLink;
    }
}

template<class T>
T* TSList<T>::NewNode(uint32_t nLinkType, uint32_t nExtraBytes, uint32_t dwFlags) {
    void* pMem = SMemAlloc(sizeof(T) + nExtraBytes, "", -2, dwFlags | SMEM_FLAG_ZEROMEMORY);
    T* pNode = pMem ? new (pMem) T : NULL;
    if (nLinkType) {
        LinkNode(pNode, nLinkType, NULL);
    }
    return pNode;
}

// Unlinks and frees the node, and returns the one after it.
template<class T>
T* TSList<T>::DeleteNode(T* pNode) {
    T* pNextRaw = Link(pNode)->m_next;
    pNode->~T();
    T* pNext = TSLink<T>::IsTerminator(pNextRaw) ? NULL : pNextRaw;
    SMemFree(pNode, "", -2, 0);
    return pNext;
}

template<class T>
void TSList<T>::Clear() {
    T* pNode = Head();
    while (pNode != NULL) {
        pNode->~T();
        SMemFree(pNode, "", -2, 0);
        pNode = Head();
    }
}

// ---- the records ----

// A network address: 16 bytes, opaque here.
struct SNetAddress {
    uint8_t abData[0x10];
};

// The provider's send buffer (0x2000 bytes from VirtualAlloc): a pointer to the address
// copy, the address, then the packet.
struct SNetSendBuffer {
    SNetAddress* pAddress;      // 0x000: &tAddress
    uint8_t abUnknown04[0x3C];  // 0x004
    SNetAddress tAddress;       // 0x040
    uint8_t abUnknown50[0xF0];  // 0x050
    uint8_t abData[0x1F00];     // 0x140
};

// A packet on the wire: the header, then the payload.
struct SNetPacketHdr {
    uint16_t wChecksum;  // 0x00
    uint16_t wSize;      // 0x02: header and payload
    uint16_t wSeq;       // 0x04
    uint16_t wAck;       // 0x06
    uint8_t byType;      // 0x08
    uint8_t bySubType;   // 0x09
    uint8_t byPlayerId;  // 0x0A
    uint8_t byFlags;     // 0x0B
    uint8_t abData[1];   // 0x0C
};

// A queued message: where it goes, the packet, and its timing.
struct SNetPacket : TSLinkedNode<SNetPacket> {
    SNetAddress* pAddress;  // 0x08
    SNetPacketHdr* pData;   // 0x0C
    uint32_t nBytes;        // 0x10
    int bLocal;             // 0x14
    uint32_t dwSendTime;    // 0x18
    uint32_t dwResendTime;  // 0x1C
};

typedef TSList<SNetPacket> SNetQueue;

// A connection: the remote's address, one outgoing, local and incoming queue per packet type
// (system, message, turn, datagram), the turns already seen, and the sequence state.
struct SNetConn : TSLinkedNode<SNetConn> {
    uint8_t pad08[0x100];          // 0x008
    SNetAddress tAddress;          // 0x108
    uint8_t pad118[4];             // 0x118
    uint32_t dwLastReceiveTime;    // 0x11C
    uint8_t pad120[0x10];          // 0x120
    SNetQueue aOutgoing[SNET_QUEUE_COUNT];  // 0x130
    SNetQueue aLocal[SNET_QUEUE_COUNT];     // 0x160
    SNetQueue aIncoming[SNET_QUEUE_COUNT];  // 0x190
    SNetQueue tOldTurns;           // 0x1C0
    uint16_t aSendSeq[SNET_QUEUE_COUNT];    // 0x1CC
    uint8_t pad1d4[10];            // 0x1D4
    uint16_t aAckSeq[SNET_QUEUE_COUNT];     // 0x1DE
    uint16_t aLastAck[SNET_QUEUE_COUNT];    // 0x1E6
    uint8_t pad1ee[2];             // 0x1EE
    uint32_t aLastAckTime[SNET_QUEUE_COUNT];  // 0x1F0
    int bNotifyProvider;           // 0x200
    uint8_t pad204[0xE];           // 0x204
    uint8_t byPlayerId;            // 0x212
    uint8_t pad213;                // 0x213
};

// A provider's address in the provider list.
struct SNetSpiNode : TSLinkedNode<SNetSpiNode> {
};

// A pending receive buffer.
struct SNetPendingNode : TSLinkedNode<SNetPendingNode> {
    uint32_t dw08;     // 0x08
    uint32_t dw0C;     // 0x0C
    void* pBuffer;     // 0x10
};

// The provider interface: the entries this file calls.
struct SNetSpi {
    uint8_t pad00[8];
    void (*pfnDestroy)(void);                                      // 0x08
    void (*pfnFreePacket)(void* pAddress, void* pData, uint32_t nBytes);  // 0x0C
    uint8_t pad10[0x38 - 0x10];
    void (*pfnSend)(int nCount, SNetAddress** ppAddress, void* pData, uint32_t nBytes);  // 0x38
    uint8_t pad3c[0x44 - 0x3C];
    void (*pfnLeave)(void);                                        // 0x44
};

// The provider's capabilities: the send bandwidth is all that is read here.
struct SNetCaps {
    uint8_t pad00[0x228];
    uint32_t dwBandwidth;  // 0x228
};

// A packet-dump log: created on first use, directly or from the registry.
struct SNetLogInfo {
    int bCreated;          // 0x00
    uint32_t hLog;         // 0x04
    int nFlags;            // 0x08
    int bFromRegistry;     // 0x0C
    char szName[0x104];    // 0x10: the log's file name, or the registry key
    char szValue[0x104];   // 0x114: the registry value

    SNetLogInfo() : bCreated(0), hLog(0), nFlags(1), bFromRegistry(1) {
        strcpy(szName, "Internal");
        szValue[0] = 0;
        strcpy(szValue, "SNet Packet Dump");
    }
    ~SNetLogInfo();
};

// ---- the file's state ----

extern "C" int __cxa_atexit(void (*pfnDestructor)(void*), void* pObject, void* pDsoHandle);
extern "C" char __dso_handle;

// The file's critical section: made first by the static initialiser, which registers its
// destructor straight from Storm's own CRITSECT_Destroy.
class SNetCritSect : public pthread_mutex_t {
public:
    SNetCritSect() {
        CRITSECT_Init(this);
        __cxa_atexit((void (*)(void*))CRITSECT_Destroy, this, &__dso_handle);
    }
};

// 1.14d mac: 003f0970
SNetCritSect g_tSNetCritSect;
// 1.14d mac: 003f099c
SNetLogInfo g_tSNetLog;
// 1.14d mac: 003f0bb4
TSList<SNetSpiNode> g_tSpiList;
// 1.14d mac: 003f0bc0
TSList<SNetConn> g_tConnList;
// 1.14d mac: 003f0bcc
TSList<SNetConn> g_tLocalConnList;
// 1.14d mac: 003f0bd8
TSGrowableArray<uint32_t> g_tSNetBuffer;
// 1.14d mac: 003f0be8
TSList<SNetPendingNode> g_tPendingList;
// 1.14d mac: 003f0bf4
SNetSpi* g_pSpi;
// 1.14d mac: 003f0bfc
char g_szSNetState0[0x80];  // a string: the next global is 0x80 bytes on
// 1.14d mac: 003f0bf8
int g_nSNetState1;
// 1.14d mac: 003f0c7c
char g_szSNetState2[0x80];  // a string: the next global is 0x80 bytes on
// 1.14d mac: 003f0cfc
char g_szSNetState3[0x80];  // a string: the next global is 0x80 bytes on
// 1.14d mac: 003f0d7c
int g_nSNetState4;
// 1.14d mac: 003f0d80
int g_nSNetState5;
// 1.14d mac: 003f0d84
void* g_pGameData;
// 1.14d mac: 003f0d88
int g_nGameDataSize;
// 1.14d mac: 003f0d8c
int g_nSNetFlag0;
// 1.14d mac: 003f0d90
int g_nSNetFlag1;
// 1.14d mac: 003f0d94
int g_nSpiCount;
// 1.14d mac: 003f0d98
SNetCaps* g_pSpiCaps;
// 1.14d mac: 003f0d9c
HANDLE g_hRecvEvent;
// 1.14d mac: 003f0da0
uint32_t g_dwNextSendTime;
// 1.14d mac: 003f0da4
SNetSendBuffer* g_pSendBuffer;
// 1.14d mac: 003f0da8
HANDLE g_hSpiLibrary;
// 1.14d mac: 003f0dac
HANDLE g_hRecvThread;
// 1.14d mac: 003f0db0
uint8_t g_bStopRecvThread;
// 1.14d mac: 003970d4
int g_bSNetLogEnabled = 1;
// 1.14d mac: 003970d8
uint8_t g_byLocalPlayerId = SNET_NOPLAYER;
// 1.14d mac: 003971dc
uint32_t g_dwLeaveSleepMs = 50;
// 1.14d mac: 0039715c
uint32_t g_nBytesQueued;
// 1.14d mac: 0039717c
uint32_t g_nBytesSent;
// 1.14d mac: 003cf76c
const char* g_aszPacketTypes[4] = {"TYPE_SYSTEM", "TYPE_MESSAGE", "TYPE_TURN", "TYPE_DATAGRAM"};
// 1.14d mac: 003cf780
const char* g_aszPacketSubTypes[16] = {
    "SYS_UNUSED",       "SYS_INITIALCONTACT",       "SYS_CIRCUITCHECK",       "SYS_CIRCUITCHECKRESPONSE",
    "SYS_PING",         "SYS_PINGRESPONSE",         "SYS_PLAYERINFO",         "SYS_PLAYERJOIN",
    "SYS_PLAYERJOIN_ACCEPTSTART", "SYS_PLAYERJOIN_ACCEPTDONE", "SYS_PLAYERJOIN_REJECT", "SYS_PLAYERLEAVE",
    "SYS_DROPPLAYER",   "SYS_NEWGAMEOWNER",         "SYS_NEWGAMEMODE",        "SYS_NEWLADDERID",
};

// The connection kept for this machine itself: the local list's first record.
#define SNET_LOCALCONN() (g_tLocalConnList.Head())

// The container instantiations and compiler-made functions this file's object holds, by the
// qualified name the compare tool pairs them with:
// 1.14d mac: 0001a4a2 SNetLogInfo::~SNetLogInfo[D1]
// 1.14d mac: 0001a4c4 TSList<SNetSpiNode>::~TSList[D1]
// 1.14d mac: 0001a4e6 TSList<SNetConn>::~TSList[D1]
// 1.14d mac: 0001a508 TSGrowableArray<unsignedint>::~TSGrowableArray[D1]
// 1.14d mac: 0001a54a TSList<SNetPendingNode>::~TSList[D1]
// 1.14d mac: 0001a56c TSList<SNetConn>::DeleteNode
// 1.14d mac: 0001a5d4 TSList<SNetPacket>::DeleteNode
// 1.14d mac: 0001a644 TSList<SNetPendingNode>::UnlinkAll
// 1.14d mac: 0001a68c TSLink<SNetPendingNode>::Unlink
// 1.14d mac: 0001a6cc TSLink<SNetPendingNode>::NextLink
// 1.14d mac: 0001a702 TSLink<SNetPacket>::Unlink
// 1.14d mac: 0001a742 TSLink<SNetPacket>::NextLink
// 1.14d mac: 0001a778 SNetConn::~SNetConn[D2]
// 1.14d mac: 0001a834 TSLink<SNetConn>::Unlink
// 1.14d mac: 0001a874 TSLink<SNetConn>::NextLink
// 1.14d mac: 0001a8aa TSList<SNetPacket>::UnlinkAll
// 1.14d mac: 0001a8f2 TSList<SNetConn>::UnlinkAll
// 1.14d mac: 0001a93a TSList<SNetSpiNode>::UnlinkAll
// 1.14d mac: 0001a982 TSLink<SNetSpiNode>::Unlink
// 1.14d mac: 0001a9c2 TSLink<SNetSpiNode>::NextLink
// 1.14d mac: 0001ab04 TSList<SNetPacket>::LinkNode
// 1.14d mac: 0001ab78 TSList<SNetPendingNode>::DeleteNode
// 1.14d mac: 0001abe8 TSList<SNetPacket>::NewNode
// 1.14d mac: 0001ac5e TSList<SNetConn>::NewNode
// 1.14d mac: 0001ad80 TSList<SNetConn>::LinkNode
// 1.14d mac: 0001adf4 TSList<SNetSpiNode>::Clear
// 1.14d mac: 0001ae80 _GLOBAL__I_a

SNetLogInfo::~SNetLogInfo() {
    if (hLog != 0) {
        SLogClose(hLog);
    }
}

void SLOG_CreateFromRegistry(SNetLogInfo* pLogInfo, const char* szKey, const char* szValue, int nFlags);
// Kept out of line: the binary calls each of these, and the original keeps their arguments in
// registers unfolded (dump_packet, SNET_ConnSendMessage, SNET_CreateConnection), which shows the
// original file had further callers (SNet's send and receive API, dead-stripped at link) that
// this file does not carry; with only the surviving callers the inliner would take them in.
#define SNET_NOINLINE __attribute__((noinline))
static SNetConn* SNET_CreateConnection(TSList<SNetConn>* pConnList, SNetAddress* pAddress);

// 1.14d mac: 0001a9f8
// External: Bnclient's log (0005ab50, 0005abdf) calls it, with its argument on the stack.
void SLOG_EnsureLogCreated(SNetLogInfo* pLogInfo) {
    if (pLogInfo->bCreated == 0) {
        if (SLogIsInitialized() && pLogInfo->szName[0] != '\0') {
            pLogInfo->bCreated = 1;
            if (pLogInfo->bFromRegistry == 0) {
                SLogCreate(pLogInfo->szName, 0, (int*)&pLogInfo->hLog);
            } else {
                SLOG_CreateFromRegistry(pLogInfo, pLogInfo->szName, pLogInfo->szValue, pLogInfo->nFlags);
            }
        }
    }
}

// 1.14d mac: 0001aa62
// External as well: its four arguments come on the stack, not in the registers a file-local gets.
void SLOG_CreateFromRegistry(SNetLogInfo* pLogInfo, const char* szKey, const char* szValue, int nFlags) {
    char szPath[260];
    szPath[0] = '\0';
    SRegLoadString(szKey, szValue, 0, szPath, sizeof(szPath));
    if (szPath[0] != '\0') {
        SLogCreate(szPath, 0, (int*)&pLogInfo->hLog);
        if (nFlags == 0) {
            SLogSetTimestamp(pLogInfo->hLog, 0);
        }
    }
}

// ---- the packet dump ----

// When network logging is on: a header, the decoded fields and a hex dump of the packet.
// 1.14d mac: 0001a05f
static void dump_packet(const char* szLabel, int bReceive, SNetPacketHdr* pPacket) {
    if (g_bSNetLogEnabled) {
        SLOG_EnsureLogCreated(&g_tSNetLog);
        uint32_t hLog = g_tSNetLog.hLog;
        if (hLog != 0) {
            SLogWrite(hLog, "------------------------------------------------------------");
            uint16_t wSeq = pPacket->wSeq;
            uint16_t wAck = pPacket->wAck;
            uint16_t wChecksum = pPacket->wChecksum;
            uint16_t wSize = pPacket->wSize;
            SLogSetAbsIndent(hLog, (bReceive != 0) << 4);
            SLogWrite(hLog, "%s: (%d bytes)", szLabel, (uint32_t)wSize);
            SLogSetIndent(hLog, 2);
            SLogWrite(hLog, "Checksum: 0x%04X", (uint32_t)wChecksum);
            SLogWrite(hLog, "Seq/Ack : %d, %d", (uint32_t)wSeq, (uint32_t)wAck);
            uint32_t nType = pPacket->byType;
            uint32_t nSubType = pPacket->bySubType;
            uint32_t nSubIndex = nSubType < 15 ? nSubType : 15;
            uint32_t nTypeIndex = nType < 3 ? nType : 3;
            SLogWrite(hLog, "Type/Sub: %s=%d %s=%d", g_aszPacketTypes[nTypeIndex], nType,
                      g_aszPacketSubTypes[nSubIndex], nSubType);
            SLogWrite(hLog, "PlayerID: %d", (uint32_t)pPacket->byPlayerId);
            uint8_t byFlags = pPacket->byFlags;
            const char* szNone = "";
            SLogWrite(hLog, "Flags:    0x%02X%s%s%s%s%s%s", (uint32_t)byFlags, byFlags ? " =" : szNone,
                      (byFlags & 1) ? " MF_ACK" : szNone, (byFlags & 2) ? " MF_RESENDREQUEST" : szNone,
                      (byFlags & 4) ? " MF_FORWARDED" : szNone, (byFlags & 4) ? " PF_JOINING" : szNone,
                      (byFlags & 8) ? " PF_LEAVING" : szNone);
            SLogDump(hLog, (const uint8_t*)pPacket, wSize);
            SLogSetAbsIndent(hLog, 0);
        }
    }
}

// ---- packets ----

// A Fletcher-style mod-255 checksum, computed backwards over the data.
// 1.14d mac: 0001a27c
SNET_NOINLINE static uint16_t SNET_ComputePacketChecksum(const uint8_t* pData, uint32_t nLength) {
    uint32_t nSum2 = 0;
    uint32_t nSum1 = 0;
    if (nLength != 0) {
        uint32_t nTotal = 0;
        uint32_t nRunning = 0;
        for (; nLength != 0; nLength--) {
            nRunning += pData[nLength - 1];
            if (nRunning > 0xFE) {
                nRunning -= 0xFF;
            }
            nTotal += nRunning;
        }
        nSum2 = nTotal % 0xFF;
        nSum1 = nRunning << 8;
    }
    return (uint16_t)(nSum2 | nSum1);
}

// The packet's check word: over everything after the word itself.
static inline void SNET_StampChecksum(SNetPacketHdr* pPacket) {
    uint16_t wSum = SNET_ComputePacketChecksum((const uint8_t*)&pPacket->wSize, pPacket->wSize - 2);
    uint32_t nLow = ((wSum & 0xFF) + (uint32_t)(wSum >> 8)) % 0xFF ^ 0xFF;
    uint32_t nHigh = (wSum >> 8) + nLow;
    pPacket->wChecksum = ((uint16_t)(nLow << 8) | (uint16_t)(nHigh % 0xFF)) ^ 0xFF;
}

// Frees a locally queued message: its address object and its packet, each when present.
// 1.14d mac: 0001a2d0
static void SNET_FreeLocalMessage(SNetAddress* pAddress, SNetPacketHdr* pData) {
    if (pAddress != NULL) {
        operator delete(pAddress);
    }
    if (pData != NULL) {
        SMemFree(pData, SNET_FILE, 0x54C, 0);
    }
}

// Rejects a bad packet, sends it through the provider, and records the ack sequence and time.
// 1.14d mac: 00019eb2
static void SNET_ConnSendPacket(SNetConn* pConn, SNetPacketHdr* pPacket) {
    uint32_t nSize = pPacket->wSize;
    if (nSize < 12) {
        if (g_bSNetLogEnabled) {
            SLOG_EnsureLogCreated(&g_tSNetLog);
            uint32_t hLog = g_tSNetLog.hLog;
            if (hLog != 0) {
                SLogWrite(hLog, "************************************************************");
                SLogWrite(hLog, "******   !!!! ConnSendPacket: Bad packet send !!!!    ******");
                dump_packet("ConnSendPacket", 0, pPacket);
            }
        }
        return;
    }
    uint16_t wAck = pPacket->wAck;
    dump_packet("ConnSendPacket", 0, pPacket);
    SNetAddress* pAddress = &pConn->tAddress;
    if (g_pSpi != NULL) {
        uint32_t dwNow = GetTickCount();
        if (dwNow - g_dwNextSendTime < 0x7FFFFFFF) {
            g_dwNextSendTime = dwNow;
        }
        g_dwNextSendTime = (nSize * 1000 + 64000) / g_pSpiCaps->dwBandwidth + g_dwNextSendTime;
        SNetPacketHdr* pSendData = pPacket;
        if (g_pSendBuffer != NULL) {
            if (nSize > 0x1F00) {
                goto done;
            }
            g_pSendBuffer->pAddress = &g_pSendBuffer->tAddress;
            SMemCopy(&g_pSendBuffer->tAddress, pAddress, sizeof(SNetAddress));
            SMemCopy(g_pSendBuffer->abData, pPacket, nSize);
            pSendData = (SNetPacketHdr*)g_pSendBuffer->abData;
        }
        g_nBytesSent = g_nBytesSent + nSize;
        g_pSpi->pfnSend(1, &pAddress, pSendData, nSize);
    }
done:
    pConn->aLastAck[pPacket->byType] = wAck;
    pConn->aLastAckTime[pPacket->byType] = 0;
}

// Builds a packet for a connection and queues or sends it; returns the queue node, or 0.
// A packet for the local connection goes on its local queue; one for a remote goes on its
// outgoing queue unless it is a datagram, and is then checksummed and sent.
// 1.14d mac: 00019b0b
SNET_NOINLINE static SNetPacket* SNET_ConnSendMessage(SNetConn* pConn, uint32_t nType, uint32_t nSubType, const void* pPayload,
                                        uint32_t nPayloadSize) {
    SNetConn* pLocalConn = SNET_LOCALCONN();
    if (pLocalConn == NULL) {
        SNetAddress tZero;
        SMemZero(&tZero, sizeof(tZero));
        pLocalConn = SNET_CreateConnection(&g_tLocalConnList, &tZero);
    }
    SNetPacket* pNode = NULL;
    if (pLocalConn == NULL) {
        return NULL;
    }
    uint32_t nBytes = nPayloadSize + 12;
    void* pMem = SMemAlloc(sizeof(SNetAddress), SNET_FILE, 0x52F, 0);
    SNetAddress* pAddress = pMem ? (SNetAddress*)pMem : NULL;
    size_t nAlloc = (4 - (nBytes & 3)) + nBytes;
    if (nAlloc < 5) {
        nAlloc = 4;
    }
    SNetPacketHdr* pPacket = (SNetPacketHdr*)SMemAlloc(nAlloc, SNET_FILE, 0x531, 0);
    SMemCopy(pAddress, &pConn->tAddress, sizeof(SNetAddress));
    pPacket->wSize = (uint16_t)nBytes;
    pPacket->wAck = pConn->aAckSeq[nType];
    pPacket->byType = (uint8_t)nType;
    pPacket->bySubType = (uint8_t)nSubType;
    pPacket->byPlayerId = pLocalConn->byPlayerId;
    pPacket->byFlags = 0;
    if (nType == SNET_TYPE_SYSTEM && nSubType == SNET_SUB_INITIALCONTACT) {
        pPacket->wSeq = 0;
    } else {
        uint16_t wSeq = pConn->aSendSeq[nType];
        pConn->aSendSeq[nType] = wSeq + 1;
        pPacket->wSeq = wSeq;
    }
    if (pPayload != NULL && nPayloadSize != 0) {
        SMemCopy(pPacket->abData, (void*)pPayload, nPayloadSize);
    }
    bool bQueued;
    if (pLocalConn == pConn || nType != SNET_TYPE_DATAGRAM) {
        SNetQueue* pQueue = pLocalConn == pConn ? &pConn->aLocal[nType] : &pConn->aOutgoing[nType];
        pNode = pQueue->NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
        if (pNode == NULL) {
            pNode = NULL;
        } else {
            pNode->pAddress = pAddress;
            pNode->pData = pPacket;
            pNode->nBytes = nBytes;
            pNode->bLocal = 1;
            pNode->dwSendTime = GetTickCount();
        }
        bQueued = pNode != NULL;
        if (pLocalConn == pConn) {
            if (bQueued) {
                pNode->dwResendTime = g_dwNextSendTime;
                return pNode;
            }
            SNET_FreeLocalMessage(pAddress, pPacket);
            return pNode;
        }
    } else {
        pNode = NULL;
        bQueued = false;
    }
    SNET_StampChecksum(pPacket);
    SNET_ConnSendPacket(pConn, pPacket);
    g_nBytesQueued = g_nBytesQueued + nPayloadSize;
    if (bQueued) {
        pNode->dwResendTime = g_dwNextSendTime;
        return pNode;
    }
    SNET_FreeLocalMessage(pAddress, pPacket);
    return pNode;
}

// Resends a stored packet: a copy with the ack refreshed from the connection, sent and freed.
// 1.14d mac: 00019dc6
static void SNET_ConnResendMessage(SNetConn* pConn, SNetPacketHdr* pData, uint32_t nSize) {
    SNetPacketHdr* pCopy = (SNetPacketHdr*)SMemAlloc(nSize, SNET_FILE, 0x700, 0);
    SMemCopy(pCopy, pData, nSize);
    pCopy->wAck = pConn->aAckSeq[pCopy->byType];
    SNET_StampChecksum(pCopy);
    SNET_ConnSendPacket(pConn, pCopy);
    SMemFree(pCopy, SNET_FILE, 0x70E, 0);
}

// ---- connections ----

// A new connection record on the list, at this address, with no player id yet.
// 1.14d mac: 0001a43f
static SNetConn* SNET_CreateConnection(TSList<SNetConn>* pConnList, SNetAddress* pAddress) {
    SNetConn* pConn = pConnList->NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
    SMemCopy(&pConn->tAddress, pAddress, sizeof(SNetAddress));
    *(uint16_t*)&pConn->byPlayerId = 0xFFFF;
    pConn->dwLastReceiveTime = GetTickCount();
    return pConn;
}

// Frees each message of a queue: a local one directly, another through the provider.
// 1.14d mac: 0001a3a7
static void SNET_ClearPacketQueue(SNetQueue* pQueue) {
    SNetPacket* pNode = pQueue->Head();
    while (pNode != NULL) {
        if (pNode->bLocal == 0) {
            if (g_pSpi != NULL) {
                g_pSpi->pfnFreePacket(pNode->pAddress, pNode->pData, pNode->nBytes);
            }
        } else {
            SNET_FreeLocalMessage(pNode->pAddress, pNode->pData);
        }
        pQueue->DeleteNode(pNode);
        pNode = pQueue->Head();
    }
}

// Destroys every queue and the old turns of a connection, then deletes it from its list.
// 1.14d mac: 0001a319
SNET_NOINLINE static void SNET_DestroyConnection(SNetConn* pConn) {
    int i = 0;
    do {
        SNET_ClearPacketQueue(&pConn->aOutgoing[i]);
        SNET_ClearPacketQueue(&pConn->aLocal[i]);
        SNET_ClearPacketQueue(&pConn->aIncoming[i]);
        i++;
    } while (i != SNET_QUEUE_COUNT);
    SNET_ClearPacketQueue(&pConn->tOldTurns);
    if (SNET_LOCALCONN() == pConn) {
        g_tLocalConnList.DeleteNode(pConn);
    } else {
        g_tConnList.DeleteNode(pConn);
    }
}

// Frees every connection of both lists.
// 1.14d mac: 0001983b
static void SNET_DestroyAllConnections(void) {
    int nList = 0;
    do {
        TSList<SNetConn>* pList = nList != 0 ? &g_tLocalConnList : &g_tConnList;
        SNetConn* pConn;
        while ((pConn = pList->Head()) != NULL) {
            SNET_DestroyConnection(pConn);
        }
        nList++;
    } while (nList != 2);
}

// ---- leaving and destroying ----

// Unloads the provider: its destroy entry, the library, the provider object and the send
// buffer; optionally the provider list.
// 1.14d mac: 00019a47
SNET_NOINLINE static void SpiDestroy(int bClearList) {
    if (g_hSpiLibrary != 0 && g_pSpi != NULL) {
        g_pSpi->pfnDestroy();
    }
    if (g_hSpiLibrary != 0) {
        FreeLibrary(g_hSpiLibrary);
        g_hSpiLibrary = 0;
    }
    if (g_pSpi != NULL) {
        operator delete(g_pSpi);
        g_pSpi = NULL;
    }
    if (g_pSendBuffer != NULL) {
        VirtualUnlock(g_pSendBuffer, 0x2000);
        VirtualFree(g_pSendBuffer, 0, MEM_RELEASE);
        g_pSendBuffer = NULL;
    }
    if (bClearList) {
        g_tSpiList.Clear();
        g_nSpiCount = 0;
    }
}

// Leaves the game: tells the provider, sends a leave message with the exit code to every
// connection that has a player id, resends what is queued, frees the game data and resets the
// state. Returns 0 with an error set when there is no provider or the local connection cannot
// be made.
// 1.14d mac: 00019532
BOOL SNetLeaveGame(DWORD dwExitCode) {
    pthread_mutex_lock(&g_tSNetCritSect);
    DWORD dwErrCode;
    if (g_pSpi == NULL) {
        dwErrCode = 0x4B4;
    } else {
        SNetConn* pLocalConn = SNET_LOCALCONN();
        if (pLocalConn == NULL) {
            SNetAddress tZero;
            SMemZero(&tZero, sizeof(tZero));
            pLocalConn = SNET_CreateConnection(&g_tLocalConnList, &tZero);
        }
        if (pLocalConn != NULL) {
            if (pLocalConn->bNotifyProvider != 0) {
                g_pSpi->pfnLeave();
            }
            for (SNetConn* pConn = g_tConnList.Head(); pConn != NULL; pConn = g_tConnList.Link(pConn)->Next()) {
                if (pConn->byPlayerId != SNET_NOPLAYER) {
                    uint32_t aMessage[2];
                    aMessage[0] = pConn->aSendSeq[2];
                    aMessage[1] = dwExitCode;
                    SNetPacket* pNode = SNET_ConnSendMessage(pConn, 0, SNET_SUB_PLAYERLEAVE, aMessage, 8);
                    if (pNode != NULL) {
                        SNET_ConnResendMessage(pConn, pNode->pData, pNode->nBytes);
                        SNET_ConnResendMessage(pConn, pNode->pData, pNode->nBytes);
                    }
                }
            }
            for (SNetConn* pConn = g_tConnList.Head(); pConn != NULL; pConn = g_tConnList.Link(pConn)->Next()) {
                if (pConn->byPlayerId != SNET_NOPLAYER) {
                    for (int i = 0; i != SNET_QUEUE_COUNT; i++) {
                        SNetQueue* pQueue = &pConn->aOutgoing[i];
                        for (SNetPacket* pNode = pQueue->Head(); pNode != NULL; pNode = pQueue->Link(pNode)->Next()) {
                            SNET_ConnResendMessage(pConn, pNode->pData, pNode->nBytes);
                        }
                    }
                }
            }
            if (g_pGameData != NULL) {
                SMemFree(g_pGameData, SNET_FILE, 0x81B, 0);
                g_pGameData = NULL;
                g_nGameDataSize = 0;
            }
            g_szSNetState0[0] = '\0';
            g_szSNetState3[0] = '\0';
            g_szSNetState2[0] = '\0';
            g_nSNetState4 = 0;
            g_nSNetState5 = 0;
            g_byLocalPlayerId = SNET_NOPLAYER;
            g_nSNetState1 = 0;
            SNET_DestroyAllConnections();
            pthread_mutex_unlock(&g_tSNetCritSect);
            Sleep(g_dwLeaveSleepMs);
            return 1;
        }
        dwErrCode = 8;
    }
    SErrSetLastError(dwErrCode);
    pthread_mutex_unlock(&g_tSNetCritSect);
    return 0;
}

// Shuts networking down: leaves any game, stops and closes the receive thread, frees the
// pending entries, unregisters the SNET event types, destroys the connections and the provider,
// and closes the event handle.
// 1.14d mac: 00019897
BOOL SNetDestroy(void) {
    pthread_mutex_lock(&g_tSNetCritSect);
    if (g_byLocalPlayerId != SNET_NOPLAYER) {
        SNetLeaveGame(3);
    }
    pthread_mutex_unlock(&g_tSNetCritSect);
    if (g_hRecvEvent != 0 && g_hRecvThread != 0) {
        g_bStopRecvThread = 1;
        SetEvent(g_hRecvEvent);
        WaitForSingleObject(g_hRecvThread, INFINITE);
        g_bStopRecvThread = 0;
        CloseHandle(g_hRecvThread);
        g_hRecvThread = 0;
    }
    pthread_mutex_lock(&g_tSNetCritSect);
    SNetPendingNode* pNode = g_tPendingList.Head();
    while (pNode != NULL) {
        if (pNode->pBuffer != NULL) {
            SMemFree(pNode->pBuffer, SNET_FILE, 0x8F3, 0);
        }
        g_tPendingList.DeleteNode(pNode);
        pNode = g_tPendingList.Head();
    }
    SEvtUnregisterType(0x534E4554, 1);
    SEvtUnregisterType(0x534E4554, 2);
    SNET_DestroyAllConnections();
    g_tSNetBuffer.m_count = 0;
    SpiDestroy(1);
    pthread_mutex_unlock(&g_tSNetCritSect);
    if (g_hRecvEvent != 0) {
        CloseHandle(g_hRecvEvent);
        g_hRecvEvent = 0;
    }
    pthread_mutex_lock(&g_tSNetCritSect);
    g_nSNetFlag0 = 0;
    g_nSNetFlag1 = 0;
    pthread_mutex_unlock(&g_tSNetCritSect);
    return 1;
}

#endif  // D2_OS_MAC
