/**
 * Data defined in D2MCPClient/McpConnect.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2MCPClient)
#error "D2MCPClient header included by a module that may not use D2MCPClient (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2McpGameInfoStrc;
struct D2QSSocketStrc;
typedef void (__fastcall *pfnMCPPacketCallback)(uint16_t wPacketField, char * pPayload);

extern D2McpGameInfoStrc g_CharSelectBuffer;
extern HANDLE MCPThread;
extern pfnMCPPacketCallback g_pfnMCPCallback1;
extern pfnMCPPacketCallback g_pfnMCPCallback2;
// Former interior labels: gInterfaceConfig6=+0x4 gInterfaceConfig5=+0x8 gInterfaceConfig4=+0xc gInterfaceConfig3=+0x10 gInterfaceConfig2=+0x14
extern uint8_t gMCPGameListBuffer[28680];

namespace D2MCPClient::McpConnect {
extern uint16_t D2GSRequestId;
extern uint32_t D2GSGameHash;
extern uint32_t D2GSGameToken;
extern D2QSSocketStrc * MCPD2QSSocket;
extern uint32_t D2GSJoinResult;
extern char D2GSConnectIp[18];
extern void * CharListPacket;
} // namespace D2MCPClient::McpConnect
