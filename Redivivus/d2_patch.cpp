// Runtime patches applied from OUR side.
//
// WHY NOT EDIT THE ENGINE SOURCE. recon/modified's engine tree is generator
// output. A hand edit there is debt that has to be justified, counted and
// retired, and every regen fights it. A patch that lives here fights nothing.
//
// WHY CALL SITES AND NOT FUNCTION ENTRIES. The obvious way to hook is to write a
// five-byte JMP over a function's prologue, which is what D2BS does. That needs a
// trampoline, which needs an instruction-length decoder, and it does not compose:
// two hooks on the same function silently clobber each other - D2BS has exactly
// that bug, where CubeOrgans_Intercept and KillMonster_Hook both patch
// SERVER_KillMonster and only the second survives.
//
// Rewriting the four-byte displacement of a `CALL rel32` instead needs no
// decoder, no trampoline and no copied prologue. Both ends are named symbols the
// linker resolves for us, so nothing in this file contains an address. That is
// the point: D2BS's patch table is a list of hardcoded 1.14d offsets that a
// recompile invalidates, while "every call to function B now goes to our B" stays
// true however far the code moves.
//
// WHY THE WHOLE .text AND NOT ONE CALLER. The first cut bounded the scan to a
// caller's first 0x400 bytes and it ran straight off the end of a 0x193-byte
// function into its neighbour. Bounding by symbol means knowing where a function
// ends, which we do not at runtime. Scanning the section and matching on the
// CALL's resolved target has no such edge: a false positive needs a random
// displacement to land exactly on the callee.
//
// WHY IT REFUSES RATHER THAN GUESSES. The scan counts first and writes only if
// the count is exactly what the patch declared. If a regen changes a function's
// shape the patch reports and does nothing, instead of rewriting a displacement
// that now means something else. A patch that silently half-applies is worse than
// one that never applied - that first overrun is precisely how this was found.

#include "Source/D2Headers/d2_platform.h"
#include "Redivivus/d2_patch.h"
#include "Redivivus/d2_trace.h"

#include "Source/D2CMP/SRC/LRUCache.h"
#include "Source/D2Game/GAME/CCmd.h"
#include "Source/D2Common/D2Common.h"
#include "Source/D2Common/D2Common.globals.h"
#include "Source/D2Common/DATATBLS/LvlTbls.h"
#include "Source/D2Win/D2Win.globals.h"
#include "Source/Fog/Source/SFile.h"

#include <cstdio>

// The Game.exe Ghidra database version this tree was generated from (ghidra_version.cpp).
extern const char g_szGhidraVersion[];

namespace {

bool WriteBytes(void* pDst, const void* pSrc, size_t n) {
    DWORD dwOld = 0;
    if (!VirtualProtect(pDst, n, PAGE_EXECUTE_READWRITE, &dwOld)) return false;
    memcpy(pDst, pSrc, n);
    DWORD dwTmp = 0;
    VirtualProtect(pDst, n, dwOld, &dwTmp);
    FlushInstructionCache(GetCurrentProcess(), pDst, n);
    return true;
}

// The executable section of our own module, from the PE headers. No constants.
bool TextSection(unsigned char** ppStart, size_t* pnSize) {
    unsigned char* pBase = (unsigned char*)GetModuleHandleA(0);
    if (!pBase) return false;
    IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBase;
    if (pDos->e_magic != IMAGE_DOS_SIGNATURE) return false;
    IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pBase + pDos->e_lfanew);
    if (pNt->Signature != IMAGE_NT_SIGNATURE) return false;
    IMAGE_SECTION_HEADER* pSec = IMAGE_FIRST_SECTION(pNt);
    for (unsigned i = 0; i < pNt->FileHeader.NumberOfSections; i++) {
        if (pSec[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) {
            *ppStart = pBase + pSec[i].VirtualAddress;
            *pnSize = pSec[i].Misc.VirtualSize;
            return true;
        }
    }
    return false;
}

// Every readable section of our own module, so a literal can be found by its
// CONTENT rather than by an address. The string this is used for moves whenever
// the linker feels like it; its bytes do not.
bool EachSection(unsigned i, unsigned char** ppStart, size_t* pnSize, unsigned* pnCount) {
    unsigned char* pBase = (unsigned char*)GetModuleHandleA(0);
    if (!pBase) return false;
    IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBase;
    if (pDos->e_magic != IMAGE_DOS_SIGNATURE) return false;
    IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pBase + pDos->e_lfanew);
    if (pNt->Signature != IMAGE_NT_SIGNATURE) return false;
    if (pnCount) *pnCount = pNt->FileHeader.NumberOfSections;
    if (i >= pNt->FileHeader.NumberOfSections) return false;
    IMAGE_SECTION_HEADER* pSec = IMAGE_FIRST_SECTION(pNt);
    if (!(pSec[i].Characteristics & IMAGE_SCN_MEM_READ)) return false;
    *ppStart = pBase + pSec[i].VirtualAddress;
    *pnSize = pSec[i].Misc.VirtualSize;
    return true;
}

volatile long g_bInstalled = 0;

// ---------------------------------------------------------------------------
// Per-opcode counts of the client messages the server dispatches, for d2ctl's `pkt`
// (Redivivus/d2_control.cpp). NET_D2GS_SERVER_HandleAnyIncomingPacket (0052cfe0) hands
// each dequeued message to one of two dispatchers, GameSetup (mode 0) and InGame
// (mode 1), each once; both messages are a 4-byte client id followed by the client's
// packet, opcode first. The system queue (mode 2) carries no client packet. The
// originals are reached through pointers so the redirect cannot rewrite these calls.
typedef void(__fastcall* PfnClientMessageGameSetup)(D2GSClientMsgEnvelopeStrc*, int32_t);
typedef void(__fastcall* PfnClientMessageInGame)(char*, size_t);
PfnClientMessageGameSetup g_pfnClientMessageGameSetup = &D2Game::GAME::CCmd::NET_D2GS_SERVER_ProcessClientMessage_GameSetup;
PfnClientMessageInGame g_pfnClientMessageInGame = &D2Game::GAME::CCmd::NET_D2GS_SERVER_ProcessClientMessage_InGame;

void CountServerPacket(const void* pMessage, size_t nSize) {
    if (pMessage && nSize > 4) g_anServerPktCount[((const unsigned char*)pMessage)[4]]++;
}

void __fastcall D2PATCH_ClientMessageGameSetup(D2GSClientMsgEnvelopeStrc* pPacket, int32_t nPacketSize) {
    CountServerPacket(pPacket, nPacketSize > 0 ? (size_t)nPacketSize : 0);
    g_pfnClientMessageGameSetup(pPacket, nPacketSize);
}

void __fastcall D2PATCH_ClientMessageInGame(char* pBytes, size_t nSize) {
    CountServerPacket(pBytes, nSize);
    g_pfnClientMessageInGame(pBytes, nSize);
}

// The original, reached INDIRECTLY so the redirect below cannot rewrite it and
// turn the wrapper into infinite recursion. A `CALL [mem]` is not a `CALL rel32`.
void (*g_pfnLRUCacheUnlink)(D2LRUCacheNodeStrc*, D2LRUCacheStrc*) =
    &D2CMP::SRC::LRUCache::LRUCACHE_Unlink;

// ---------------------------------------------------------------------------
// LRUCACHE_Unlink writes through a null pointer on a single-entry list:
//
//     if (pEntry == pCache->pLRUTail) {
//         pOldestEntry = pCache->pLRUTail->pPrev;   // null when it is the only node
//         pCache->pLRUTail = pOldestEntry;
//         pOldestEntry->pNext = 0;                  // <- writes to 0x0C
//
// Blizzard's bug, not a reconstruction artifact - D2BS carries the same fix as
// GameCrashFix. Pre-empt the case in a wrapper rather than guarding inside the
// function, because the wrapper is something we can own.
long g_nLRUSingleNode = 0;

void D2PATCH_LRUCACHE_Unlink(D2LRUCacheNodeStrc* pEntry, D2LRUCacheStrc* pCache) {
    if (pEntry && pCache) {
        D2LRUCacheNodeStrc* pNode = pEntry;
        if (pNode == pCache->pLRUTail && !pNode->pPrev) {
            // The only node in the list. Unlink it the way the tail branch would
            // have if it had checked: empty the list, clear the node's links.
            pCache->pLRUHead = 0;
            pCache->pLRUTail = 0;
            pNode->pNext = 0;
            pNode->pPrev = 0;
            if (++g_nLRUSingleNode <= 8 || (g_nLRUSingleNode % 256) == 0) {
                D2TraceFmt("D2PATCH lru-single-node hits=%ld entry=%08X cache=%08X",
                           g_nLRUSingleNode, (unsigned)(uintptr_t)pEntry,
                           (unsigned)(uintptr_t)pCache);
            }
            return;
        }
    }
    g_pfnLRUCacheUnlink(pEntry, pCache);
}

// ---------------------------------------------------------------------------
// Refuse to run on game data older than 1.14d. Levels.txt in d2exp.mpq has 133
// rows; 1.14d's Patch_D2.mpq overrides it with 137. Without the patch the game
// starts normally and only faults when it builds one of the missing levels, e.g.
// the Act V waypoint to the Worldstone Keep. This check is ours, not 1.14d's.
const int kLevelsRows114d = 137;

void (__fastcall* g_pfnAllocTxtLevels)(D2PoolManagerStrc*) =
    &D2Common::DATATBLS::LvlTbls::TXT_AllocTxt_levels;

void __fastcall D2PATCH_TXT_AllocTxt_levels(D2PoolManagerStrc* pMemory) {
    g_pfnAllocTxtLevels(pMemory);

    const int nRows = D2Common::DATATBLS::MonsterTbls::sgtDataTable.nTxtLevelsSize;
    // The MPQs are opened only outside server mode and -direct; d2data.mpq being
    // open says that load ran.
    const bool bPatchMissing = D2Win::Src::D2WinArchive::MPQ_d2data &&
                               !D2Win::Src::D2WinArchive::MPQ_patch_d2 &&
                               !Fog::Source::SFile::GetUseDirectCommand();
    if (nRows >= kLevelsRows114d && !bPatchMissing) return;

    char szText[512];
    snprintf(szText, sizeof(szText),
             "Patch_D2.mpq is missing or from an older version - Diablo II 1.14d data is required.\n\n"
             "The level table has %d rows, 1.14d has %d%s. Update Diablo II to 1.14d and make sure "
             "its Patch_D2.mpq is in the game folder.",
             nRows, kLevelsRows114d, bPatchMissing ? ", and Patch_D2.mpq was not opened" : "");
    D2TraceFmt("D2PATCH game data check failed: levels=%d patch_d2=%s", nRows,
               bPatchMissing ? "missing" : "open");
    MessageBoxA(0, szText, "Diablo II", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
    ExitProcess(1);
}

} // namespace

int D2PATCH_RedirectAllCalls(void* pOldCallee, void* pNewCallee, int nExpectedSites) {
    if (!pOldCallee || !pNewCallee) return -1;
    unsigned char* pText = 0;
    size_t nText = 0;
    if (!TextSection(&pText, &nText)) {
        D2TraceFmt("D2PATCH no .text section");
        return -1;
    }

    // Pass one: count, so a shape change is reported instead of half-applied.
    int nFound = 0;
    for (size_t i = 0; i + 5 <= nText; i++) {
        if (pText[i] != 0xE8) continue;                    // CALL rel32
        int32_t rel = *(int32_t*)(pText + i + 1);
        if (pText + i + 5 + rel == (unsigned char*)pOldCallee) nFound++;
    }
    if (nExpectedSites >= 0 && nFound != nExpectedSites) {
        D2TraceFmt("D2PATCH REFUSED callee=%08X found=%d expected=%d",
                   (unsigned)(uintptr_t)pOldCallee, nFound, nExpectedSites);
        return -1;
    }

    // Pass two: apply.
    int nDone = 0;
    for (size_t i = 0; i + 5 <= nText; i++) {
        if (pText[i] != 0xE8) continue;
        int32_t rel = *(int32_t*)(pText + i + 1);
        if (pText + i + 5 + rel != (unsigned char*)pOldCallee) continue;
        int32_t relNew = (int32_t)((unsigned char*)pNewCallee - (pText + i + 5));
        if (WriteBytes(pText + i + 1, &relNew, sizeof(relNew))) nDone++;
    }
    D2TraceFmt("D2PATCH redirect %08X -> %08X sites=%d",
               (unsigned)(uintptr_t)pOldCallee, (unsigned)(uintptr_t)pNewCallee, nDone);
    return nDone;
}

// ---------------------------------------------------------------------------
// Retarget a format-string operand, by content.
//
// The splash and the automap both render their version line with
// `SStrPrintf(buf, n, "v %d.%d%c", 1, 0xe, 'd')`, pushing the format string's
// address as a PUSH imm32. Neither the string's address nor the push sites are
// knowable ahead of time in a rebuilt binary, so both are found the same way the
// call redirect finds its sites: search the image for the literal's exact bytes
// INCLUDING its terminator, then scan .text for a PUSH of that address.
//
// Matching with the terminator is what keeps the replacement from matching
// itself - "v %d.%d%c Redivivus" contains the original as a prefix, so a match
// that ignored the NUL would find our own string and patch the push to itself.
//
// The operand is matched by VALUE, not by instruction encoding. The first cut
// looked for `PUSH imm32` (0x68) because that is what the 1.14d binary does, and
// it found zero sites: we are built with mingw GCC, which materialises a stack
// argument as `MOV DWORD PTR [ESP+n], imm32` instead. Scanning for the address
// itself covers both, and every other form the compiler might pick next.
//
// The destination buffers are 64 bytes (splash) and 100 (automap), and both
// calls are length-bounded, so lengthening the rendered text cannot overrun.
int D2PATCH_RedirectFormatString(const char* pszOld, const char* pszNew, int nExpectedSites) {
    if (!pszOld || !pszNew) return -1;
    unsigned char* pText = 0;
    size_t nText = 0;
    if (!TextSection(&pText, &nText)) return -1;

    const size_t nOldLen = strlen(pszOld) + 1;   // include the NUL

    // Pass one: every address in the image holding the literal, with terminator.
    unsigned char* apFound[8];
    int nFound = 0;
    unsigned nSections = 0;
    EachSection(0, &pText, &nText, &nSections);
    TextSection(&pText, &nText);
    for (unsigned s = 0; s < nSections && nFound < 8; s++) {
        unsigned char* pSec = 0;
        size_t nSec = 0;
        if (!EachSection(s, &pSec, &nSec, 0)) continue;
        for (size_t i = 0; i + nOldLen <= nSec && nFound < 8; i++) {
            if (memcmp(pSec + i, pszOld, nOldLen) == 0) apFound[nFound++] = pSec + i;
        }
    }
    if (!nFound) {
        D2TraceFmt("D2PATCH fmt REFUSED - literal not found");
        return -1;
    }

    // Pass two: count operands anywhere in .text whose value is one of those
    // addresses, whatever instruction carries them.
    int nSites = 0;
    for (size_t i = 0; i + 4 <= nText; i++) {
        unsigned char* pImm = *(unsigned char**)(pText + i);
        for (int f = 0; f < nFound; f++) if (pImm == apFound[f]) { nSites++; break; }
    }
    if (nExpectedSites >= 0 && nSites != nExpectedSites) {
        D2TraceFmt("D2PATCH fmt REFUSED copies=%d found=%d expected=%d",
                   nFound, nSites, nExpectedSites);
        return -1;
    }

    // Pass three: apply.
    int nDone = 0;
    for (size_t i = 0; i + 4 <= nText; i++) {
        unsigned char* pImm = *(unsigned char**)(pText + i);
        bool bHit = false;
        for (int f = 0; f < nFound; f++) if (pImm == apFound[f]) { bHit = true; break; }
        if (!bHit) continue;
        const char* pRepl = pszNew;
        if (WriteBytes(pText + i, &pRepl, sizeof(pRepl))) nDone++;
    }
    D2TraceFmt("D2PATCH fmt '%s' -> '%s' copies=%d sites=%d", pszOld, pszNew, nFound, nDone);
    return nDone;
}

void D2PATCH_InstallAll(void) {
    if (InterlockedExchange(&g_bInstalled, 1)) return;

    // LRUCACHE_Unlink has exactly two direct callers, LRUCACHE_FreeEntry and
    // LRUCACHE_MoveToFront. If that ever stops being true the patch says so and
    // does nothing.
    D2PATCH_RedirectAllCalls((void*)&D2CMP::SRC::LRUCache::LRUCACHE_Unlink,
                             (void*)&D2PATCH_LRUCACHE_Unlink, 2);

    // One call each, from NET_D2GS_SERVER_HandleAnyIncomingPacket.
    D2PATCH_RedirectAllCalls((void*)&D2Game::GAME::CCmd::NET_D2GS_SERVER_ProcessClientMessage_GameSetup,
                             (void*)&D2PATCH_ClientMessageGameSetup, 1);
    D2PATCH_RedirectAllCalls((void*)&D2Game::GAME::CCmd::NET_D2GS_SERVER_ProcessClientMessage_InGame,
                             (void*)&D2PATCH_ClientMessageInGame, 1);

    // Brand the build: the version line reads "R<Ghidra version>" (ghidra_version.cpp,
    // written by each regen) instead of "v 1.14d". This is a deliberate DIVERGENCE from
    // 1.14d, not a fidelity fix, which is why it lives here and not in Ghidra: the
    // database has to keep describing the binary Blizzard shipped or the assert oracle
    // and the byte-matching targets stop meaning anything.
    // Two sites: UIMENU_MainMenu (splash) and AUTOMAP_DrawVersion. Both still pass
    // (1, 0xe, 'd'); a format without conversions ignores them.
    // The needle "v %d.%d%c" is assembled at RUNTIME: a literal of it here would put a
    // third copy in .rdata, referenced from this very function, and the scan would
    // patch this function's own operand along with the two engine sites.
    static char szVersion[24];
    snprintf(szVersion, sizeof(szVersion), "R%s", g_szGhidraVersion);
    // Stored XOR 0x5a so no copy of the needle's bytes exists in the image either.
    static const unsigned char kNeedleXor[9] = {0x2c, 0x7a, 0x7f, 0x3e, 0x74, 0x7f, 0x3e, 0x7f, 0x39};
    char szNeedle[10];
    for (int i = 0; i < 9; i++) szNeedle[i] = (char)(kNeedleXor[i] ^ 0x5a);
    szNeedle[9] = '\0';
    D2PATCH_RedirectFormatString(szNeedle, szVersion, 2);

    // One call, from DATATBLS_LoadAllTxts.
    D2PATCH_RedirectAllCalls((void*)&D2Common::DATATBLS::LvlTbls::TXT_AllocTxt_levels,
                             (void*)&D2PATCH_TXT_AllocTxt_levels, 1);
}

#ifdef _MSC_VER
// Both builds install these before WinMain runs any game code: Visual C++ from the
// CRT's initialiser table, GCC from a constructor. D2CONTROL_Start calls it too;
// the second call returns at once.
#pragma section(".CRT$XCU", read)
extern "C" __declspec(allocate(".CRT$XCU")) void (__cdecl* const gpfnD2PatchInstallAll)(void) = D2PATCH_InstallAll;
#elif defined(__GNUC__)
// The same for the GCC build: before WinMain, not at the first in-game pump, so the
// splash already shows the R<version> line and every patch is in before the menus run.
__attribute__((constructor)) static void D2PATCH_InstallAtStartup(void) { D2PATCH_InstallAll(); }
#endif
