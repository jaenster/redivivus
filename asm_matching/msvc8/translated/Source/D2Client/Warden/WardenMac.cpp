// WardenMac.cpp - what the 1.14d Intel Mac build has in place of the Warden client.
//
// The Mac binary carries no Warden: no module loader, no RC4, no "warden" string. The game
// calls four empty functions where the Windows build calls Warden (the frame tick, the
// game-exit cleanup, the join-game init and the server's 0xAE packet), 0012b628..0012b640,
// out of line in one unit beside the other stubs. WardenClient.cpp is the Windows client and is
// not part of the Mac target (cmake/mac.cmake).
//
// Mac-only: compiles to nothing elsewhere.
#include "Source/D2Client/Warden/WardenClient.h"

#if !defined(_WIN32) && D2_OS_MAC

namespace D2Client { namespace Warden {

// 1.14d mac: 0012b628
void __fastcall WARDEN_Tick(uint32_t dwTickArg) {
}

// 1.14d mac: 0012b62d
void __stdcall WARDEN_Cleanup() {
}

// 1.14d mac: 0012b632
BOOL __fastcall WARDEN_Initialize(uint32_t dwModuleParam) {
    return 1;
}

// 1.14d mac: 0012b63c
void __fastcall WARDEN_HandleServerPacket(void * pPacket, uint32_t nPacketLen) {
}

}} // namespace D2Client::Warden

#endif
