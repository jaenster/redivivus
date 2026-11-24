// Redivivus' own code, not recovered from any Blizzard binary.
//
// What the server needs from Game.exe's own module (Source/Game), which it does not link:
// that module is the client's WinMain and app-mode dispatch.
#include <windows.h>

#include <cstdint>

#include "3rdparty/Storm/Source/SMem.h"
#include "Redivivus/d2_trace.h"

// The per-opcode packet counters the engine's packet handlers bump (declared in
// Redivivus/d2_trace.h). A development build defines and reports them in
// Redivivus/d2_control.cpp; otherwise Redivivus/d2_trace.cpp holds them, and the server
// leaves that file out (it is the development tracer, and it probes D2gfx state), so
// they live here: counted, never read.
#if !defined(REDIVIVUS_DEV)
unsigned g_anClientPktCount[256];
unsigned g_anServerPktCount[256];
#endif

namespace Game::Main {

// The program's global operator new. D2Hell's WinTrust code (D2FILE_VerifySignatureWinTrust,
// D2FILE_GetSignerOrganization) allocates through it. In Game.exe it is 00405410 in
// Game/Main.cpp, a single call into Storm's allocator with the "new" source tag; the
// server's is the same call.
void * MISC_OperatorNewShim(uint32_t nSize) {
    return Storm::Source::SMem::SMemAlloc(nSize, (char *)"new", (uint32_t)-1, 0);
}

}  // namespace Game::Main
