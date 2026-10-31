/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "Bnclient/BNGatewayAccess.h"
#include "Bnclient/BnDownload.h"
#include "Bnclient/BnMessQueue.h"
#include "Bnclient/BnSend.h"
#include "Bnclient/Bnet.h"
#include "Bnclient/Debug.h"
#include "Bnclient/SRP.h"
#include "Bnclient/net_sid.h"
#include "D2Client/D2BNetGatewayAccessStr.h"
#include "D2Client/D2IniConfigStrc.h"
#include "Storm/_unnamespaced.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

uint8_t gaBnSendRawBytesHeader[4] = {}; D2_GLOBAL_GUARD(gaBnSendRawBytesHeader, "gaBnSendRawBytesHeader");

char gaCDKeyAuthData[4] = { ' ', ',', ';', '\x00' }; D2_GLOBAL_GUARD(gaCDKeyAuthData, "gaCDKeyAuthData");

char s_ColonSeparator[2] = { ':', '\x00' }; D2_GLOBAL_GUARD(s_ColonSeparator, "s_ColonSeparator");

D2BNetClientCallbacksStrc sgBNetClientCallbacks = {
    /* .pfnSetLoggingFlag = */ (fnSetLoggingFlag)&Bnclient::Debug::DEBUG_SetLoggingFlag,
    /* .pfnConnectToBattleNet = */ (fnConnectToBattleNet)&Bnclient::BnSend::ConnectingToBattleNet,
    /* .pfnSendLogonRequest = */ &Bnclient::BnSend::BNCLIENT_SendLogonRequest,
    /* .pfnWaitForLogonResponse = */ &Bnclient::BnSend::BNCLIENT_WaitForLogonResponse,
    /* .pfnDisconnect = */ &Bnclient::BnSend::BNSEND_Disconnect,
    /* .pfnSendChatCommand = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x0E_ChatCommand,
    /* .pfnGetChannelNamePtr = */ &Bnclient::_net_sid::NET_SID_CLIENT_GetChannelNamePtr,
    /* .pfnSendJoinChannel = */ (fnSendJoinChannel)&Bnclient::BnSend::NET_SID_CLIENT_Send_0x0C_JoinChannel,
    /* .pfnSendGetChannelList = */ (fnSendGetChannelList)&Bnclient::BnSend::NET_SID_CLIENT_Send_0x0B_GetChannelList,
    /* .pfnDequeueMessage = */ (fnDequeueMessage)&Bnclient::BNMESSQUEUE_DequeueMessage,
    /* .pfnGetStateAndClear = */ &Bnclient::_net_sid::NET_SID_CLIENT_GetStateAndClear,
    /* .pfnSendGetFileTime = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x33_GetFileTime,
    /* .pfnSendCheckAd = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x15_CheckAd,
    /* .pfnSendStartAdvEx3 = */ (fnSendStartAdvEx3)&Bnclient::BnSend::NET_SID_CLIENT_Send_0x1C_StartAdVex3,
    /* .pfnSendGetAdvListEx = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x09_GetAdvListEx,
    /* .pfnSendNotifyJoin = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x22_NotifyJoin,
    /* .pfnSendQueryRealms2 = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x40_QueryRealms2,
    /* .pfnSendLogonRealmEx = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x3E_LogonRealmEx,
    /* .pfnApplyPrepatch = */ (fnApplyPrepatch)&Bnclient::BnDownload::BNDOWNLOAD_ApplyPrepatch,
    /* .pfnGetDownloadProgress = */ &Bnclient::BnDownload::BNDOWNLOAD_GetProgress,
    /* .pfnInitiatePatchDownload = */ (fnInitiatePatchDownload)&Bnclient::BnDownload::BNDOWNLOAD_InitiatePatchDownload,
    /* .pfnEnterChatAndJoinChannel = */ &Bnclient::BnSend::BNCLIENT_EnterChatAndJoinChannel,
    /* .pfnSendLeaveChat = */ &Bnclient::BnSend::thunk_NET_SID_CLIENT_Send_0x10_LeaveChat,
    /* .pfnGetGameVersion = */ (fnGetGameVersion)&Bnclient::BnSend::GET_GameVersion,
    /* .pfnSendCreateAccountRequest = */ &Bnclient::BnSend::BNCLIENT_SendCreateAccountRequest,
    /* .pfnWaitForCreateAccountResponse = */ &Bnclient::BnSend::BNCLIENT_WaitForCreateAccountResponse,
    /* .pfnIsSocketConnected = */ &Bnclient::BnSend::BNCLIENT_IsSocketConnected,
    /* .pfnSendClickAd = */ (fnSendClickAd)&Bnclient::BnSend::NET_SID_CLIENT_Send_0x16_ClickAd,
    /* .pfnSetDownloadCancelFlag = */ &Bnclient::BnDownload::BNDOWNLOAD_SetCancelFlag,
    /* .pfnSendWriteUserData = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x27_WriteUserData,
    /* .pfnSendReadUserData = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x26_ReadUserData,
    /* .pfnGetStoredReadUserData = */ (fnGetStoredReadUserData)&Bnclient::BnSend::BNSEND_GetStoredReadUserData,
    /* .pfnGetConnectionState = */ (fnGetConnectionState)&Bnclient::BnSend::BNSEND_GetConnectionState,
    /* .pfnLogWriteLine = */ &Bnclient::Bnet::BnetLog_WriteLine,
    /* .pfnLogIndent = */ (fnLogIndent)&Bnclient::Bnet::BNET_LogIndent,
    /* .pfnGetEnterChatResponseFlags = */ &Bnclient::BnSend::BNCLIENT_GetEnterChatResponseFlags,
    /* .pfnSendKeepAlive = */ &Bnclient::BnSend::BNCLIENT_SendKeepAlive,
    /* .pfnDecodeCDKeys = */ &Bnclient::BnSend::BNNEWS_DecodeCDKeys,
    /* .pfnBnetLoadAndReturn = */ (fnBnetLoadAndReturn)&Bnclient::BnSend::BnetLoadAndReturn,
    /* .pfnSendChangePasswordRequest = */ &Bnclient::BnSend::BNCLIENT_SendChangePasswordRequest,
    /* .pfnWaitForChangePasswordResponse = */ &Bnclient::BnSend::BNCLIENT_WaitForChangePasswordResponse,
    /* .pfnSendQueryAdURL = */ (fnSendQueryAdURL)&Bnclient::BnSend::NET_SID_CLIENT_Send_0x41_QueryAdURL,
    /* .pfnSendNewsInfo = */ (fnSendNewsInfo)&Bnclient::BnSend::NET_SID_CLIENT_Send_0x46_NewsInfo,
    /* .pfnNewsShutdown = */ &Bnclient::BnSend::BNNEWS_Shutdown,
    /* .pfnGetProtocolVersionChar = */ &Bnclient::BnSend::BNCLIENT_GetProtocolVersionChar,
    /* .pfnIsEmailRequested = */ &Bnclient::BnSend::BNSEND_GetBNetThreadInitFlag,
    /* .pfnSendSetEmail = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x59_SetEmail,
    /* .pfnSendResetPassword = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x5A_ResetPassword,
    /* .pfnSendChangeEmail = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x5B_ChangeEmail,
    /* .pfnSendLeaveGame = */ &Bnclient::BnSend::NET_SID_CLIENT_Send_0x1F_LeaveGame,
    /* .pfnCacheFreeMem = */ (fnCacheFreeMem)&Bnclient::BNGatewayAccess::CACHE_FreeMem,
    /* .pfnCacheReadData = */ (fnCacheReadData)&Bnclient::BNGatewayAccess::CACHE_ReadData,
    /* .pfnCacheWriteData = */ (fnCacheWriteData)&Bnclient::BNGatewayAccess::CACHE_WriteData,
    /* .pfnReserved = */ nullptr
}; D2_GLOBAL_GUARD(sgBNetClientCallbacks, "sgBNetClientCallbacks");

// also labelled: gdwFogCacheMax
uint32_t gaBnCacheCrc32Table[256] = {
    0x0,
    0x77073096,
    0xee0e612c,
    0x990951ba,
    0x76dc419,
    0x706af48f,
    0xe963a535,
    0x9e6495a3,
    0xedb8832,
    0x79dcb8a4,
    0xe0d5e91e,
    0x97d2d988,
    0x9b64c2b,
    0x7eb17cbd,
    0xe7b82d07,
    0x90bf1d91,
    0x1db71064,
    0x6ab020f2,
    0xf3b97148,
    0x84be41de,
    0x1adad47d,
    0x6ddde4eb,
    0xf4d4b551,
    0x83d385c7,
    0x136c9856,
    0x646ba8c0,
    0xfd62f97a,
    0x8a65c9ec,
    0x14015c4f,
    0x63066cd9,
    0xfa0f3d63,
    0x8d080df5,
    0x3b6e20c8,
    0x4c69105e,
    0xd56041e4,
    0xa2677172,
    0x3c03e4d1,
    0x4b04d447,
    0xd20d85fd,
    0xa50ab56b,
    0x35b5a8fa,
    0x42b2986c,
    0xdbbbc9d6,
    0xacbcf940,
    0x32d86ce3,
    0x45df5c75,
    0xdcd60dcf,
    0xabd13d59,
    0x26d930ac,
    0x51de003a,
    0xc8d75180,
    0xbfd06116,
    0x21b4f4b5,
    0x56b3c423,
    0xcfba9599,
    0xb8bda50f,
    0x2802b89e,
    0x5f058808,
    0xc60cd9b2,
    0xb10be924,
    0x2f6f7c87,
    0x58684c11,
    0xc1611dab,
    0xb6662d3d,
    0x76dc4190,
    0x1db7106,
    0x98d220bc,
    0xefd5102a,
    0x71b18589,
    0x6b6b51f,
    0x9fbfe4a5,
    0xe8b8d433,
    0x7807c9a2,
    0xf00f934,
    0x9609a88e,
    0xe10e9818,
    0x7f6a0dbb,
    0x86d3d2d,
    0x91646c97,
    0xe6635c01,
    0x6b6b51f4,
    0x1c6c6162,
    0x856530d8,
    0xf262004e,
    0x6c0695ed,
    0x1b01a57b,
    0x8208f4c1,
    0xf50fc457,
    0x65b0d9c6,
    0x12b7e950,
    0x8bbeb8ea,
    0xfcb9887c,
    0x62dd1ddf,
    0x15da2d49,
    0x8cd37cf3,
    0xfbd44c65,
    0x4db26158,
    0x3ab551ce,
    0xa3bc0074,
    0xd4bb30e2,
    0x4adfa541,
    0x3dd895d7,
    0xa4d1c46d,
    0xd3d6f4fb,
    0x4369e96a,
    0x346ed9fc,
    0xad678846,
    0xda60b8d0,
    0x44042d73,
    0x33031de5,
    0xaa0a4c5f,
    0xdd0d7cc9,
    0x5005713c,
    0x270241aa,
    0xbe0b1010,
    0xc90c2086,
    0x5768b525,
    0x206f85b3,
    0xb966d409,
    0xce61e49f,
    0x5edef90e,
    0x29d9c998,
    0xb0d09822,
    0xc7d7a8b4,
    0x59b33d17,
    0x2eb40d81,
    0xb7bd5c3b,
    0xc0ba6cad,
    0xedb88320,
    0x9abfb3b6,
    0x3b6e20c,
    0x74b1d29a,
    0xead54739,
    0x9dd277af,
    0x4db2615,
    0x73dc1683,
    0xe3630b12,
    0x94643b84,
    0xd6d6a3e,
    0x7a6a5aa8,
    0xe40ecf0b,
    0x9309ff9d,
    0xa00ae27,
    0x7d079eb1,
    0xf00f9344,
    0x8708a3d2,
    0x1e01f268,
    0x6906c2fe,
    0xf762575d,
    0x806567cb,
    0x196c3671,
    0x6e6b06e7,
    0xfed41b76,
    0x89d32be0,
    0x10da7a5a,
    0x67dd4acc,
    0xf9b9df6f,
    0x8ebeeff9,
    0x17b7be43,
    0x60b08ed5,
    0xd6d6a3e8,
    0xa1d1937e,
    0x38d8c2c4,
    0x4fdff252,
    0xd1bb67f1,
    0xa6bc5767,
    0x3fb506dd,
    0x48b2364b,
    0xd80d2bda,
    0xaf0a1b4c,
    0x36034af6,
    0x41047a60,
    0xdf60efc3,
    0xa867df55,
    0x316e8eef,
    0x4669be79,
    0xcb61b38c,
    0xbc66831a,
    0x256fd2a0,
    0x5268e236,
    0xcc0c7795,
    0xbb0b4703,
    0x220216b9,
    0x5505262f,
    0xc5ba3bbe,
    0xb2bd0b28,
    0x2bb45a92,
    0x5cb36a04,
    0xc2d7ffa7,
    0xb5d0cf31,
    0x2cd99e8b,
    0x5bdeae1d,
    0x9b64c2b0,
    0xec63f226,
    0x756aa39c,
    0x26d930a,
    0x9c0906a9,
    0xeb0e363f,
    0x72076785,
    0x5005713,
    0x95bf4a82,
    0xe2b87a14,
    0x7bb12bae,
    0xcb61b38,
    0x92d28e9b,
    0xe5d5be0d,
    0x7cdcefb7,
    0xbdbdf21,
    0x86d3d2d4,
    0xf1d4e242,
    0x68ddb3f8,
    0x1fda836e,
    0x81be16cd,
    0xf6b9265b,
    0x6fb077e1,
    0x18b74777,
    0x88085ae6,
    0xff0f6a70,
    0x66063bca,
    0x11010b5c,
    0x8f659eff,
    0xf862ae69,
    0x616bffd3,
    0x166ccf45,
    0xa00ae278,
    0xd70dd2ee,
    0x4e048354,
    0x3903b3c2,
    0xa7672661,
    0xd06016f7,
    0x4969474d,
    0x3e6e77db,
    0xaed16a4a,
    0xd9d65adc,
    0x40df0b66,
    0x37d83bf0,
    0xa9bcae53,
    0xdebb9ec5,
    0x47b2cf7f,
    0x30b5ffe9,
    0xbdbdf21c,
    0xcabac28a,
    0x53b39330,
    0x24b4a3a6,
    0xbad03605,
    0xcdd70693,
    0x54de5729,
    0x23d967bf,
    0xb3667a2e,
    0xc4614ab8,
    0x5d681b02,
    0x2a6f2b94,
    0xb40bbe37,
    0xc30c8ea1,
    0x5a05df1b,
    0x2d02ef8d
}; D2_GLOBAL_GUARD(gaBnCacheCrc32Table, "gaBnCacheCrc32Table");

char s___AUItem_NewsTable___0072ff8c[24] = {
    '.',
    '?',
    'A',
    'U',
    'I',
    't',
    'e',
    'm',
    '@',
    'N',
    'e',
    'w',
    's',
    'T',
    'a',
    'b',
    'l',
    'e',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUItem_NewsTable___0072ff8c, "s___AUItem_NewsTable___0072ff8c");

D2BNetGatewayAccessStr ActualRealmInfo = {}; D2_GLOBAL_GUARD(ActualRealmInfo, "ActualRealmInfo");

D2BnNewsTableStrc g_nNewsMaxCacheSize = {}; D2_GLOBAL_GUARD(g_nNewsMaxCacheSize, "g_nNewsMaxCacheSize");

char lpFileName_00881c58[260] = {}; D2_GLOBAL_GUARD(lpFileName_00881c58, "lpFileName_00881c58");

char gszBNetChatInputBuffer[260] = {}; D2_GLOBAL_GUARD(gszBNetChatInputBuffer, "gszBNetChatInputBuffer");

char g_szBnetConnectionStateString[128] = {}; D2_GLOBAL_GUARD(g_szBnetConnectionStateString, "g_szBnetConnectionStateString");

_RTL_CRITICAL_SECTION lpCriticalSection_00882004 = {}; D2_GLOBAL_GUARD(lpCriticalSection_00882004, "lpCriticalSection_00882004");

char g_szCurrentChannelName[256] = {}; D2_GLOBAL_GUARD(g_szCurrentChannelName, "g_szCurrentChannelName");

BOOL NET_SID_INCOMING_CLIENT_PacketRequireAnswer[94] = {}; D2_GLOBAL_GUARD(NET_SID_INCOMING_CLIENT_PacketRequireAnswer, "NET_SID_INCOMING_CLIENT_PacketRequireAnswer");

uint32_t g_SIDPacketResponseFlags[94] = {}; D2_GLOBAL_GUARD(g_SIDPacketResponseFlags, "g_SIDPacketResponseFlags");

char gszCDKeyInUseByUsername[256] = {}; D2_GLOBAL_GUARD(gszCDKeyInUseByUsername, "gszCDKeyInUseByUsername");

char g_szStoredUsername[256] = {}; D2_GLOBAL_GUARD(g_szStoredUsername, "g_szStoredUsername");

D2SrpSaltVerifierStrc g_srpSaltVerifier = {}; D2_GLOBAL_GUARD(g_srpSaltVerifier, "g_srpSaltVerifier");

D2SrpCredentialsStrc g_srpCredentials = {}; D2_GLOBAL_GUARD(g_srpCredentials, "g_srpCredentials");

uint8_t g_SHA1Contexts[276] = {}; D2_GLOBAL_GUARD(g_SHA1Contexts, "g_SHA1Contexts");

uint8_t g_IDEAKeySchedules[312] = {}; D2_GLOBAL_GUARD(g_IDEAKeySchedules, "g_IDEAKeySchedules");

// =============================================================================
// Initialized scalars
// =============================================================================

HANDLE ghCacheFileHandle = INVALID_HANDLE_VALUE; D2_GLOBAL_GUARD(ghCacheFileHandle, "ghCacheFileHandle");
char * szBnNewsChannelName = s_BNNews_D2DV_006df210; D2_GLOBAL_GUARD(szBnNewsChannelName, "szBnNewsChannelName");
uint32_t g_nRealmLogonResult = 0xff; D2_GLOBAL_GUARD(g_nRealmLogonResult, "g_nRealmLogonResult");
uint32_t g_dwProductId = 0x1; D2_GLOBAL_GUARD(g_dwProductId, "g_dwProductId");
int gnBnetLogFileHandle = 0; D2_GLOBAL_GUARD(gnBnetLogFileHandle, "gnBnetLogFileHandle");
unsigned int gbBnetLogInitialized = {}; D2_GLOBAL_GUARD(gbBnetLogInitialized, "gbBnetLogInitialized");
D2BnCacheFileHeaderStrc * gpCacheFileHeader = 0; D2_GLOBAL_GUARD(gpCacheFileHeader, "gpCacheFileHeader");
void * gpCacheBlockTable = 0; D2_GLOBAL_GUARD(gpCacheBlockTable, "gpCacheBlockTable");
void * g_pCacheCryptTable = 0; D2_GLOBAL_GUARD(g_pCacheCryptTable, "g_pCacheCryptTable");
uint8_t * gpCacheBufferPoolFirst = 0; D2_GLOBAL_GUARD(gpCacheBufferPoolFirst, "gpCacheBufferPoolFirst");
uint32_t gbCacheIsOpen = 0; D2_GLOBAL_GUARD(gbCacheIsOpen, "gbCacheIsOpen");
uint32_t g_nBNetServerToken = 0; D2_GLOBAL_GUARD(g_nBNetServerToken, "g_nBNetServerToken");
uint32_t g_nBNetClientToken = 0; D2_GLOBAL_GUARD(g_nBNetClientToken, "g_nBNetClientToken");
void * g_pBNetChannelListData = 0; D2_GLOBAL_GUARD(g_pBNetChannelListData, "g_pBNetChannelListData");
void * gpRealmQueryData = 0; D2_GLOBAL_GUARD(gpRealmQueryData, "gpRealmQueryData");
void * g_pBNetGameListData = 0; D2_GLOBAL_GUARD(g_pBNetGameListData, "g_pBNetGameListData");
void * gpBNetWriteUserData = 0; D2_GLOBAL_GUARD(gpBNetWriteUserData, "gpBNetWriteUserData");
void * g_pBNetReadUserData = 0; D2_GLOBAL_GUARD(g_pBNetReadUserData, "g_pBNetReadUserData");
void * g_pBNetCDKeyHashData = 0; D2_GLOBAL_GUARD(g_pBNetCDKeyHashData, "g_pBNetCDKeyHashData");
uint32_t g_nBNetConnectionState = 0; D2_GLOBAL_GUARD(g_nBNetConnectionState, "g_nBNetConnectionState");
LONG * g_pBNetSocketData = 0; D2_GLOBAL_GUARD(g_pBNetSocketData, "g_pBNetSocketData");
// also labelled: lpAddend_00881bf4
LONG * Destination_00881bf4 = 0; D2_GLOBAL_GUARD(Destination_00881bf4, "Destination_00881bf4");
uint32_t gbBNetSocketConnected = 0; D2_GLOBAL_GUARD(gbBNetSocketConnected, "gbBNetSocketConnected");
long Target_00881bfc = 0; D2_GLOBAL_GUARD(Target_00881bfc, "Target_00881bfc");
// also labelled: lpAddend_00881c00
LONG * Destination_00881c00 = 0; D2_GLOBAL_GUARD(Destination_00881c00, "Destination_00881c00");
uint32_t g_nBNetThreadInitFlag = 0; D2_GLOBAL_GUARD(g_nBNetThreadInitFlag, "g_nBNetThreadInitFlag");
uint32_t g_pfnNewsChannelCallback = 0; D2_GLOBAL_GUARD(g_pfnNewsChannelCallback, "g_pfnNewsChannelCallback");
pfnNewsEntryCallback g_pfnMotdCallback = (pfnNewsEntryCallback)0; D2_GLOBAL_GUARD(g_pfnMotdCallback, "g_pfnMotdCallback");
pfnVoidNoArgCallback g_pfnNewsCallback = (pfnVoidNoArgCallback)0; D2_GLOBAL_GUARD(g_pfnNewsCallback, "g_pfnNewsCallback");
uint32_t g_nFirstUnreadNewsIndex = 0; D2_GLOBAL_GUARD(g_nFirstUnreadNewsIndex, "g_nFirstUnreadNewsIndex");
uint32_t g_dwNewsTimestamp = 0; D2_GLOBAL_GUARD(g_dwNewsTimestamp, "g_dwNewsTimestamp");
unsigned int gnNetworkSendState = {}; D2_GLOBAL_GUARD(gnNetworkSendState, "gnNetworkSendState");
unsigned int gbBnetDownloadCancelFlag = {}; D2_GLOBAL_GUARD(gbBnetDownloadCancelFlag, "gbBnetDownloadCancelFlag");
LONG * lpAddend_00881d5c = 0; D2_GLOBAL_GUARD(lpAddend_00881d5c, "lpAddend_00881d5c");
uint8_t gbBnetFtpFileTransferActive = 0; D2_GLOBAL_GUARD(gbBnetFtpFileTransferActive, "gbBnetFtpFileTransferActive");
uint8_t bBnetFtpDownloadActive = 0; D2_GLOBAL_GUARD(bBnetFtpDownloadActive, "bBnetFtpDownloadActive");
uint32_t g_nBattleNetConnectionData = 0; D2_GLOBAL_GUARD(g_nBattleNetConnectionData, "g_nBattleNetConnectionData");
uint32_t g_nBattleNetConnectionState = 0; D2_GLOBAL_GUARD(g_nBattleNetConnectionState, "g_nBattleNetConnectionState");
BnMessQueue * nLobbyChatCurrentMessage = 0; D2_GLOBAL_GUARD(nLobbyChatCurrentMessage, "nLobbyChatCurrentMessage");
BnMessQueue * nLobbyChatQueueMessages = 0; D2_GLOBAL_GUARD(nLobbyChatQueueMessages, "nLobbyChatQueueMessages");
uint32_t g_nSIDResponseCount1 = 0; D2_GLOBAL_GUARD(g_nSIDResponseCount1, "g_nSIDResponseCount1");
uint32_t g_nSIDResponseCount2 = 0; D2_GLOBAL_GUARD(g_nSIDResponseCount2, "g_nSIDResponseCount2");
uint32_t g_bSIDProtocolReady = 0; D2_GLOBAL_GUARD(g_bSIDProtocolReady, "g_bSIDProtocolReady");
uint32_t g_dwLastAdId = 0; D2_GLOBAL_GUARD(g_dwLastAdId, "g_dwLastAdId");
uint32_t g_bSIDFlag2 = 0; D2_GLOBAL_GUARD(g_bSIDFlag2, "g_bSIDFlag2");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace Fog::Src::Excel {

} // namespace Fog::Src::Excel

namespace D2Client::Warden::NWardenClient {

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

} // namespace D2Client::Warden

namespace D2Client::UI {

} // namespace D2Client::UI

namespace D2Client::UI::Button::ButtonImplementation {

} // namespace D2Client::UI::Button::ButtonImplementation

namespace D2Client::UI::Button::ButtonWrapper {

} // namespace D2Client::UI::Button::ButtonWrapper

namespace D2Client::UI::Editbox::EditBoxImplementation {

} // namespace D2Client::UI::Editbox::EditBoxImplementation

namespace D2Client::UI::Editbox::EditBoxWrapper {

} // namespace D2Client::UI::Editbox::EditBoxWrapper

namespace D2Client::UI::Spinner::SpinnerImplementation {

} // namespace D2Client::UI::Spinner::SpinnerImplementation

namespace D2Client::UI::Spinner::SpinnerWrapper {

} // namespace D2Client::UI::Spinner::SpinnerWrapper

namespace D2Client::UNIT::Object {

} // namespace D2Client::UNIT::Object

namespace D2Client::UI::ListBox::ListBoxWrapper {

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

} // namespace D2Client::UI::ListBox::ListBoxImplementation

namespace D2Win::_FONT {

} // namespace D2Win::_FONT

namespace D2Game::GAME::Event::Dispatch {

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::GAME::Event::Dispatch::Player {

} // namespace D2Game::GAME::Event::Dispatch::Player

namespace D2Game::GAME::Event::Dispatch::Object {

} // namespace D2Game::GAME::Event::Dispatch::Object

namespace D2Game::GAME::Event::Dispatch::Monster {

} // namespace D2Game::GAME::Event::Dispatch::Monster

namespace D2Common::Chat::Ignorelist::ChatIgnoreList {

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common::Drlg::Outdoors {

} // namespace D2Common::Drlg::Outdoors

namespace Define_the_symbol__ATL_MIXED {

} // namespace Define_the_symbol__ATL_MIXED

namespace Fog::_3rdparty::crashy {

} // namespace Fog::_3rdparty::crashy

namespace Storm::SFile {

} // namespace Storm::SFile

namespace Class::FILEREC {

} // namespace Class::FILEREC

namespace Class::EVENTREC {

} // namespace Class::EVENTREC

namespace Class::REQUEST {

} // namespace Class::REQUEST

namespace Class::AUDIOSTREAM {

} // namespace Class::AUDIOSTREAM

namespace Class::HANDLER {

} // namespace Class::HANDLER

namespace D2Client::CharScreen {

} // namespace D2Client::CharScreen

namespace D2Client::UI::Automap {

} // namespace D2Client::UI::Automap

namespace D2Game::GAME::Rendering {

} // namespace D2Game::GAME::Rendering

namespace D2Client::UI::Help {

} // namespace D2Client::UI::Help

namespace D2Client::UI::npcmenu {

} // namespace D2Client::UI::npcmenu

namespace D2Client::NPC {

} // namespace D2Client::NPC

namespace D2Client::UNIT::UnitSnd {

} // namespace D2Client::UNIT::UnitSnd

namespace D2Client::UI::ui {

} // namespace D2Client::UI::ui

namespace D2Game::Quests::QUESTS::Quest40 {

} // namespace D2Game::Quests::QUESTS::Quest40

namespace D2Game::Quests::QUESTS::A5Q1 {

} // namespace D2Game::Quests::QUESTS::A5Q1

namespace D2Game::Quests::QUESTS::A5Q2 {

} // namespace D2Game::Quests::QUESTS::A5Q2

namespace D2Game::Quests::QUESTS::A5Q3 {

} // namespace D2Game::Quests::QUESTS::A5Q3

namespace D2Game::Quests::QUESTS::A5Q4 {

} // namespace D2Game::Quests::QUESTS::A5Q4

namespace D2Game::Quests::QUESTS::A5Q5 {

} // namespace D2Game::Quests::QUESTS::A5Q5

namespace D2Game::Quests::QUESTS::A5Q6 {

} // namespace D2Game::Quests::QUESTS::A5Q6

namespace D2Game::Quests::QUESTS::Quest37 {

} // namespace D2Game::Quests::QUESTS::Quest37

namespace D2Game::Quests::QUESTS::A1Q0 {

} // namespace D2Game::Quests::QUESTS::A1Q0

namespace D2Game::Quests::QUESTS::A1Q1 {

} // namespace D2Game::Quests::QUESTS::A1Q1

namespace D2Game::Quests::QUESTS::A1Q2 {

} // namespace D2Game::Quests::QUESTS::A1Q2

namespace D2Game::Quests::QUESTS::A1Q3 {

} // namespace D2Game::Quests::QUESTS::A1Q3

namespace D2Game::Quests::QUESTS::A1Q4 {

} // namespace D2Game::Quests::QUESTS::A1Q4

namespace D2Game::Quests::QUESTS::A1Q5 {

} // namespace D2Game::Quests::QUESTS::A1Q5

namespace D2Game::Quests::QUESTS::A1Q6 {

} // namespace D2Game::Quests::QUESTS::A1Q6

namespace D2Game::Quests::QUESTS::A1Q7 {

} // namespace D2Game::Quests::QUESTS::A1Q7

namespace D2Game::Quests::QUESTS::Quest38 {

} // namespace D2Game::Quests::QUESTS::Quest38

namespace D2Game::Quests::QUESTS::A2Q0 {

} // namespace D2Game::Quests::QUESTS::A2Q0

namespace D2Game::Quests::QUESTS::A2Q1 {

} // namespace D2Game::Quests::QUESTS::A2Q1

namespace D2Game::Quests::QUESTS::A2Q2 {

} // namespace D2Game::Quests::QUESTS::A2Q2

namespace D2Game::Quests::QUESTS::A2Q3 {

} // namespace D2Game::Quests::QUESTS::A2Q3

namespace D2Game::Quests::QUESTS::A2Q4 {

} // namespace D2Game::Quests::QUESTS::A2Q4

namespace D2Game::Quests::QUESTS::A2Q5 {

} // namespace D2Game::Quests::QUESTS::A2Q5

namespace D2Game::Quests::QUESTS::A2Q6 {

} // namespace D2Game::Quests::QUESTS::A2Q6

namespace D2Game::Quests::QUESTS::A2Q7 {

} // namespace D2Game::Quests::QUESTS::A2Q7

namespace D2Game::Quests::QUESTS::A2Q8 {

} // namespace D2Game::Quests::QUESTS::A2Q8

namespace D2Common::Monsters::MONSTERS::Umod {

} // namespace D2Common::Monsters::MONSTERS::Umod

namespace D2Game::Quests::QUESTS::A4Q0 {

} // namespace D2Game::Quests::QUESTS::A4Q0

namespace D2Game::Quests::QUESTS::A4Q1 {

} // namespace D2Game::Quests::QUESTS::A4Q1

namespace D2Game::Quests::QUESTS::A4Q2 {

} // namespace D2Game::Quests::QUESTS::A4Q2

namespace D2Game::Quests::QUESTS::A4Q3 {

} // namespace D2Game::Quests::QUESTS::A4Q3

namespace D2Game::Quests::QUESTS::A4Q4 {

} // namespace D2Game::Quests::QUESTS::A4Q4

namespace D2Game::Quests::QUESTS::Quest39 {

} // namespace D2Game::Quests::QUESTS::Quest39

namespace D2Game::Quests::QUESTS::A3Q0 {

} // namespace D2Game::Quests::QUESTS::A3Q0

namespace D2Game::Quests::QUESTS::A3Q1 {

} // namespace D2Game::Quests::QUESTS::A3Q1

namespace D2Game::Quests::QUESTS::A3Q2 {

} // namespace D2Game::Quests::QUESTS::A3Q2

namespace D2Game::Quests::QUESTS::A3Q3 {

} // namespace D2Game::Quests::QUESTS::A3Q3

namespace D2Game::Quests::QUESTS::A3Q4 {

} // namespace D2Game::Quests::QUESTS::A3Q4

namespace D2Game::Quests::QUESTS::A3Q5 {

} // namespace D2Game::Quests::QUESTS::A3Q5

namespace D2Game::Quests::QUESTS::A3Q6 {

} // namespace D2Game::Quests::QUESTS::A3Q6

namespace D2Client::Waypoint {

} // namespace D2Client::Waypoint

namespace D2Common::Drlg::DrlgGrid {

} // namespace D2Common::Drlg::DrlgGrid

namespace Fog::Memory {

} // namespace Fog::Memory

namespace Fog::Debug {

} // namespace Fog::Debug

namespace D2Client::MCP {

} // namespace D2Client::MCP

namespace D2Client::Bnet {

} // namespace D2Client::Bnet

namespace D2Game::Save {

} // namespace D2Game::Save

namespace D2Game::GameData {

} // namespace D2Game::GameData

namespace D2Client::GAME::Record {

} // namespace D2Client::GAME::Record

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::_Selection {

} // namespace D2Client::_Selection

namespace D2Client::Cursor {

} // namespace D2Client::Cursor

namespace D2Client::Draw::Weather {

} // namespace D2Client::Draw::Weather

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::View {

} // namespace D2Client::GAME::View

namespace D2Client::GAME::RosterPets {

} // namespace D2Client::GAME::RosterPets

namespace D2Client::GAME::Roster {

} // namespace D2Client::GAME::Roster

namespace D2Client::UI::Inv {

} // namespace D2Client::UI::Inv

namespace D2Client::UI::Hireables {

} // namespace D2Client::UI::Hireables

namespace D2Client::UI::Panel {

} // namespace D2Client::UI::Panel

namespace D2Client::UI::Party {

} // namespace D2Client::UI::Party

namespace D2Client::_Interaction {

} // namespace D2Client::_Interaction

namespace D2Client::Merc {

} // namespace D2Client::Merc

namespace D2Client::Mouse {

} // namespace D2Client::Mouse

namespace D2Client::Forms {

} // namespace D2Client::Forms

namespace D2Client::_CdKey {

// =============================================================================
// Initialized scalars
// =============================================================================

char * KeyClassic = 0; D2_GLOBAL_GUARD(KeyClassic, "D2Client::_CdKey::KeyClassic");
char * KeyExpansion = 0; D2_GLOBAL_GUARD(KeyExpansion, "D2Client::_CdKey::KeyExpansion");
char * KeyOwner = 0; D2_GLOBAL_GUARD(KeyOwner, "D2Client::_CdKey::KeyOwner");

} // namespace D2Client::_CdKey

namespace D2Direct3D::Renderer {

} // namespace D2Direct3D::Renderer
