#include <windows.h>
#include <winsock2.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>

#include "support/d2_control.h"
#include "support/d2_trace.h"
#include "support/d2_patch.h"
#include "D2Client/DataTbls/MonsterTbls.h"
#include "D2Common/DATATBLS/MonsterTbls.h"
#include "D2Client/UNIT/Monster.h"
#include "globals/globals.h"
#include "D2Win/Src/D2WinMain.h"
#include "D2Client/GAME/View.h"
#include "D2Common/Drlg/D2DrlgFileStrc.h"
#include "D2Common/Drlg/Drlg.h"
#include "D2Common/Drlg/Transform.h"
#include "D2Client/GAME/Wall2.h"
#include "D2Common/DATATBLS/ObjectTbls.h"
#include "D2Game/Objects/Objects.h"
#include "D2gfx/D2GFX.h"
#include "D2Game/GAME/Server.h"
#include "D2Launch/MainMenus.h"
#include "D2Launch/CharSel.h"
#include "D2Client/UI/Path.h"
#include "D2Client/UI/ui.h"
#include "D2Common/ITEMS/Items.h"
#include "D2Common/INVENTORY/Inventory.h"
#include "D2Client/UI/npcmenu.h"
#include "D2Client/GAME/Msg.h"
#include "D2Common/Units.h"
#include "D2Common/D2UnitStrc.h"
#include "D2Common/D2MonStatsTxt.h"
#include "D2Client/UNIT/PlrSkills.h"
#include "D2Common/Txt/Skills.h"
#include "D2Common/Stats/StatsEx.h"
#include "D2Client/UI/Draw.h"
#include "D2Client/UNIT/UnitMode.h"
#include "D2Client/SKILLS/Skills.h"
#include "D2Common/D2SkillsTxt.h"
#include "D2Game/PLAYER/PlayerModes.h"
#include "D2Game/PLAYER/PlayerMsg.h"
#include "D2Common/COMPOSIT/Composit.h"
#include "D2Client/GAME/SCmd.h"
#include "D2Common/D2SkillStrc.h"
#include "D2Lang/StrTable/strtable.h"
#include "D2Common/DUNGEON/Dungeon.h"
#include "D2Common/D2DynamicPathStrc.h"
#include "D2Client/D2AutomapLayerStrc.h"
#include "D2Client/UI/Automap.h"
#include "D2Client/D2GameViewStrc.h"
#include "D2Game/UNIT/Monster.h"
#include "D2Client/UNIT/Player.h"
#include "D2Client/GAME/Select.h"
#include "D2Common/Drlg/Transform.h"
#include "D2Game/Ai/D2AiGeneralStrc.h"

// One line in, one line out, on TCP 7777. Commands:
//   state                -> a JSON line describing where the game actually is
//   units [type]         -> client-side units with their CLIENT-SPACE screen position,
//                           so a test can click an NPC instead of guessing pixels
//   move <x> <y>         -> game-space cursor move (800x600 client)
//   click <x> <y>        -> move, press, release three frames later
//   key <vk>             -> WM_KEYDOWN/WM_KEYUP
//   text <chars>         -> one WM_CHAR per character
//   enter <name>         -> drive the out-of-game menus to load that character
//
// `enter` does NOT synthesise input. Synthetic clicks reach the main menu through
// WndProc but the menu samples real mouse state, so they land only sometimes and a
// run that misses one looks exactly like a run that crashed. It calls the game's own
// out-of-game entry points instead, which is what the Aether tool does against the
// original 1.14d, and what makes a run repeatable:
//
//   UIMENU_MainMenu()                    dismiss the splash
//   OogCurrentCharSelectionMode = 0      single player
//   MAINMENU_CloseAndLaunchCharSelect()  build the list, enumerate the saves
//   SelectedCharBnetSingleTcpIp(...)     enter as that character, difficulty dialog
//                                        included - it sets the state the dialog would
//
// One step per pumped frame, because each one needs the frames after it to run.
// Actions are queued by the socket thread and applied by D2CONTROL_Pump on the game
// thread, because dispatching a window message from another thread while the game is
// mid-frame is exactly the kind of race that produces an unreproducible bug.

#define D2CONTROL_PORT 7777
#define MAXQ 256

// kind 0 = dispatch a window message, kind 1 = take a screenshot of the real back buffer
struct Action { int kind; UINT msg; WPARAM wp; LPARAM lp; int atFrame; int setPos; int x, y; char path[260]; };

static CRITICAL_SECTION g_cs;
static Action g_queue[MAXQ];
static int g_qHead = 0, g_qTail = 0;
static volatile long g_frames = 0;
static volatile long g_started = 0;
static char g_state[512] = "{}";

// Shop inspection and purchase, serviced on the game thread from D2CONTROL_Pump.
static volatile long g_reqShopList = 0;
static volatile long g_reqBuyGuid  = 0;
static volatile long g_reqTxType   = 1;
static volatile long g_reqMyList   = 0;
static volatile long g_reqFreeGuid = 0;
static volatile long g_freeStage   = 0;
static char g_myList[4096] = "{\"items\":[],\"n\":-1}";
static char g_shopList[4096] = "{\"items\":[],\"n\":-1}";

// The `enter` sequence. One step per pumped frame; -1 means idle.
static volatile long g_enterStep = -1;
static char g_enterName[64] = "";
static int  g_enterWait = 0;
static char g_enterNote[96] = "";
// A duplicate of the GAME thread's handle, taken on the first pump. The socket thread
// uses it to suspend the game and walk its stack, which is the only way to see where a
// HANG actually is - a hung game answers no messages and writes no trace.
static HANDLE g_gameThread = 0;
static DWORD  g_imageBase = 0;

static void Enqueue(UINT msg, WPARAM wp, LPARAM lp, int delayFrames, int setPos, int x, int y) {
    EnterCriticalSection(&g_cs);
    int next = (g_qTail + 1) % MAXQ;
    if (next != g_qHead) {
        Action* a = &g_queue[g_qTail];
        a->kind = 0; a->path[0] = 0;
        a->msg = msg; a->wp = wp; a->lp = lp;
        a->atFrame = (int)g_frames + delayFrames;
        a->setPos = setPos; a->x = x; a->y = y;
        g_qTail = next;
    }
    LeaveCriticalSection(&g_cs);
}

static int UnitsReply(char* out, int outMax, int nWantType);

static volatile long g_walkOn = 0;
static int  g_walkTX = 0, g_walkTY = 0;      // world tile target
static int  g_walkX  = 0, g_walkY  = 0;      // last observed position
static int  g_walkStuck = 0, g_walkSteps = 0, g_walkFrames = 0;
static int  g_walkStartX = 0, g_walkStartY = 0;
static int  g_foundCls = -1, g_foundGuid = -1, g_foundX = 0, g_foundY = 0;
static char g_foundName[40] = "";
static int  g_walkJog = 0;
static int  g_aSeen[32];
static int  g_nSeen = 0;

// Depth-limited so a cyclic tree cannot hang the game thread while it is being counted.
static int AmapCount(D2AutomapCellStrc* pCell, int nDepth, int* pnMaxDepth) {
    if (!pCell || nDepth > 4096 || IsBadReadPtr(pCell, sizeof(D2AutomapCellStrc))) return 0;
    if (nDepth > *pnMaxDepth) *pnMaxDepth = nDepth;
    return 1 + AmapCount(pCell->pLess, nDepth + 1, pnMaxDepth)
             + AmapCount(pCell->pMore, nDepth + 1, pnMaxDepth);
}

// Per-opcode counters for the `pkt` command. They live HERE, in harness code, not in a
// generated translation unit: the instrumentation that once incremented them sat in
// D2Client/Game/SCmd.cpp and D2Game/Game/SCmd.cpp, and the first regen merge that touched
// those files took them away and left the harness with two undefined symbols. Anything the
// harness owns has to be defined in support/, which the generator never rewrites.
unsigned g_anClientPktCount[256];
unsigned g_anServerPktCount[256];

static void DumpAllThreads(void);
extern "C" void D2ProbeArmClipHwbp(void);
extern "C" void D2ProbeArmClipHwbpSelf(void);
// The address the hardware write-watch covers; defined with the VEH below.
extern volatile void* g_pHwbpTarget;

// Trampoline over the renderer's Y-buffer fill so every call is logged with its arguments
// and its caller. Installed by swapping the pointer in the dispatch table, so no generated
// code is touched. A caller passing an unexpected height is the one thing the hardware
// watchpoint cannot rule out.
static void (__fastcall *g_pfnRealFillY)(void*, int, int, int) = nullptr;

static void __fastcall FillYTrampoline(void* ppvBits, int nWidth, int nHeight, int a4) {
    void* pRet = __builtin_return_address(0);
    D2TraceFmt("FILLY caller=%08X bits=%08X w=%d h=%d a4=%d  clipYMin=%d clipBottom=%d",
               (unsigned)(uintptr_t)pRet, (unsigned)(uintptr_t)ppvBits, nWidth, nHeight, a4,
               D2gfx::gnTileClipYMin, D2gfx::D2GFX::gnTileClipBottom);
    if (g_pfnRealFillY) g_pfnRealFillY(ppvBits, nWidth, nHeight, a4);
    D2TraceFmt("FILLY done  clipYMin=%d clipBottom=%d", D2gfx::gnTileClipYMin, D2gfx::D2GFX::gnTileClipBottom);
}

static void HandleLine(char* line, char* reply, int replyMax) {
    while (*line == ' ') line++;
    char* nl = strpbrk(line, "\r\n"); if (nl) *nl = 0;

    if (!strncmp(line, "stack", 5)) {
        if (!g_gameThread) { lstrcpynA(reply, "{\"err\":\"no game thread yet\"}", replyMax); return; }
        CONTEXT ctx; memset(&ctx, 0, sizeof(ctx)); ctx.ContextFlags = CONTEXT_CONTROL;
        char out[480]; int used = 0;
        SuspendThread(g_gameThread);
        BOOL got = GetThreadContext(g_gameThread, &ctx);
        DWORD eip = got ? ctx.Eip : 0, ebp = got ? ctx.Ebp : 0;
        // Copy the EBP chain out while suspended; format afterwards so the game is
        // stopped for as short a time as possible.
        DWORD rvas[24]; int n = 0;
        if (got) {
            rvas[n++] = eip - g_imageBase;
            for (int i = 0; i < 20 && ebp; i++) {
                if (IsBadReadPtr((void*)ebp, 8)) break;
                DWORD ret = *(DWORD*)(ebp + 4);
                DWORD next = *(DWORD*)ebp;
                if (ret <= g_imageBase) break;
                rvas[n++] = ret - g_imageBase;
                if (next <= ebp) break;
                ebp = next;
            }
        }
        ResumeThread(g_gameThread);
        used += wsprintfA(out + used, "{\"base\":\"0x%X\",\"rva\":[", (unsigned)g_imageBase);
        for (int i = 0; i < n && used < 400; i++)
            used += wsprintfA(out + used, "%s\"0x%X\"", i ? "," : "", (unsigned)rvas[i]);
        wsprintfA(out + used, "]}");
        lstrcpynA(reply, out, replyMax);
        return;
    }
    // Freeze every thread and walk all of them into the trace. This is served ENTIRELY on
    // the socket thread and takes no lock the game thread can hold, so it answers when
    // `state` cannot - which is the only situation where it matters. The hang watchdog is
    // started from D2CONTROL_Pump, so a hang BEFORE the first pumped frame leaves nothing
    // else that can say where the game thread is.
    if (!strncmp(line, "threads", 7)) {
        DumpAllThreads();
        lstrcpynA(reply, "{\"ok\":\"threads dumped to the trace\"}", replyMax);
        return;
    }
    // `ping` must never touch a lock or game memory: it is the one command that
    // distinguishes "the control channel is wedged" from "the game is gone". Without
    // it both look identical through d2ctl - a missing reply - and a stalled game
    // thread holding g_cs reads as a crash.
    if (!strncmp(line, "ping", 4)) {
        wsprintfA(reply, "{\"ok\":\"ping\",\"frames\":%ld}", (long)g_frames);
        return;
    }
    if (!strncmp(line, "state", 5)) {
        // Deliberately unlocked. g_state has a single writer (the pump, on the game
        // thread) and is only ever read here; the worst case is a torn diagnostic
        // string. Taking g_cs meant that a game thread which stalled while holding it
        // wedged every control command forever, which is exactly what happened - and
        // it made a hung harness indistinguishable from a dead game.
        lstrcpynA(reply, g_state, replyMax);
        return;
    }
    // mon <start> <count> -> raw MonStats rows. If every row reads the same the TABLE is
    // wrong; if the rows differ the table is fine and it is the spawn that is picking one
    // class for everything. One command separates the two.
    if (!strncmp(line, "mon", 3)) {
        int nStart = 0, nCount = 8;
        sscanf(line, "mon %d %d", &nStart, &nCount);
        static char monOut[8192];
        int u = wsprintfA(monOut, "{\"size\":%d,\"rows\":[",
                          D2Common::ITEMS::Items::sgptDataTable ? (int)D2Common::ITEMS::Items::sgptDataTable->nTxtMonStatsSize : -1);
        for (int i = nStart; D2Common::ITEMS::Items::sgptDataTable && i < nStart + nCount &&
             i < (int)D2Common::ITEMS::Items::sgptDataTable->nTxtMonStatsSize && u < (int)sizeof(monOut) - 200; i++) {
            const D2MonStatsTxt* pRow = &D2Common::ITEMS::Items::sgptDataTable->pTxtMonStats[i];
            char szName[40]; szName[0] = 0;
            const uint16_t* pwsz = D2Lang::StrTable::strtable::GetLocaleString(pRow->NameStr);
            if (pwsz) { int k = 0; for (; k < 32 && pwsz[k]; k++) szName[k] = (char)(pwsz[k] & 0x7f); szName[k] = 0; }
            u += wsprintfA(monOut + u,
                "%s{\"i\":%d,\"Id\":%d,\"NameStr\":%d,\"npc\":%d,\"intr\":%d,\"town\":%d,\"n\":\"%s\"}",
                i == nStart ? "" : ",", i, (int)pRow->Id, (int)pRow->NameStr,
                (int)pRow->npc, (int)pRow->interact, (int)pRow->inTown, szName);
        }
        wsprintfA(monOut + u, "]}");
        lstrcpynA(reply, monOut, replyMax);
        return;
    }
    // pos -> the player's path state on BOTH sides. A click that does not move the player is
    // either "no path was produced" (dwPathPoints stays 0) or "the path was produced and the
    // mode never left NEUTRAL"; only the path fields separate those two, and only comparing
    // client against server says which side dropped it.
    if (!strncmp(line, "pos", 3)) {
        static char posOut[2048];
        D2UnitStrc* apPlayers[2] = { nullptr, nullptr };
        void* apTables[2] = { (void*)D2Client::UNIT::CUnit::ClientSideUnitHashTables, (void*)D2Client::UNIT::CUnit::ServerSideUnitHashTables };
        for (int nTable = 0; nTable < 2; nTable++) {
            D2UnitStrc** ppTable = (D2UnitStrc**)((char*)apTables[nTable] + UNIT_PLAYER * 0x200);
            for (int nBucket = 0; nBucket < 128 && !apPlayers[nTable]; nBucket++)
                for (D2UnitStrc* pUnit = ppTable[nBucket]; pUnit; pUnit = pUnit->pListNext)
                    { apPlayers[nTable] = pUnit; break; }
        }
        int u = wsprintfA(posOut, "{\"sides\":[");
        for (int i = 0; i < 2; i++) {
            D2UnitStrc* pUnit = apPlayers[i];
            const D2DynamicPathStrc* pPath = pUnit ? pUnit->pPath.pDynamicPath : nullptr;
            if (!pPath) { u += wsprintfA(posOut + u, "%s{\"tbl\":%d,\"path\":0}", i ? "," : "", i); continue; }
            u += wsprintfA(posOut + u,
                "%s{\"tbl\":%d,\"mode\":%d,\"x\":%d,\"y\":%d,\"tx\":%d,\"ty\":%d,"
                "\"stx\":%d,\"sty\":%d,\"pts\":%d,\"idx\":%d,\"savedpts\":%d,"
                "\"ptype\":%d,\"prevptype\":%d,\"flags\":\"0x%X\",\"collflag\":\"0x%X\","
                "\"collpat\":\"0x%X\",\"collres\":\"0x%X\",\"size\":%d,\"dir\":%d,\"room\":\"0x%X\"}",
                i ? "," : "", i, (int)pUnit->eAnimMode.field_1c,
                (int)pPath->Coords.tSubtile.nPosX, (int)pPath->Coords.tSubtile.nPosY,
                (int)pPath->nTargetX, (int)pPath->nTargetY,
                (int)pPath->nSavedTargetX, (int)pPath->nSavedTargetY,
                (int)pPath->dwPathPoints, (int)pPath->nPathPointIndex, (int)pPath->nSavedPathPoints,
                (int)pPath->ePathType, (int)pPath->dwPrevPathType,
                (unsigned)pPath->nFlags, (unsigned)pPath->nCollisionFlag,
                (unsigned)pPath->nCollisionPattern, (unsigned)(unsigned short)pPath->eCollisionFlag,
                (int)pPath->nUnitSize, (int)pPath->nDirection, (unsigned)(uintptr_t)pPath->pRoom);
        }
        wsprintfA(posOut + u, "]}");
        lstrcpynA(reply, posOut, replyMax);
        return;
    }
    // walk <x> <y> -> drive the player to a world tile from inside the game loop.
    // walkstat     -> where it got to, whether it is stuck, and any shopkeeper it saw.
    if (!strncmp(line, "walkstat", 8)) {
        static char wOut[1024];
        int nX = 0, nY = 0, nMode = -1;
        if (D2Client::UNIT::CUnit::pPlayerUnit && D2Client::UNIT::CUnit::pPlayerUnit->pPath.pDynamicPath) {
            nX = D2Client::UNIT::CUnit::pPlayerUnit->pPath.pDynamicPath->Coords.tSubtile.nPosX;
            nY = D2Client::UNIT::CUnit::pPlayerUnit->pPath.pDynamicPath->Coords.tSubtile.nPosY;
            nMode = (int)D2Client::UNIT::CUnit::pPlayerUnit->eAnimMode.field_1c;
        }
        wsprintfA(wOut,
            "{\"on\":%d,\"x\":%d,\"y\":%d,\"tx\":%d,\"ty\":%d,\"mode\":%d,\"steps\":%d,"
            "\"stuck\":%d,\"from\":[%d,%d],\"found\":{\"cls\":%d,\"guid\":%d,\"x\":%d,\"y\":%d,\"name\":\"%s\"}}",
            (int)g_walkOn, nX, nY, g_walkTX, g_walkTY, nMode, g_walkSteps, g_walkStuck,
            g_walkStartX, g_walkStartY, g_foundCls, g_foundGuid, g_foundX, g_foundY, g_foundName);
        lstrcpynA(reply, wOut, replyMax);
        return;
    }
    if (!strncmp(line, "walkstop", 8)) {
        InterlockedExchange(&g_walkOn, 0);
        lstrcpynA(reply, "{\"ok\":\"walkstop\"}", replyMax);
        return;
    }
    if (!strncmp(line, "walk", 4)) {
        int nTX = 0, nTY = 0;
        if (sscanf(line, "walk %d %d", &nTX, &nTY) != 2) {
            lstrcpynA(reply, "{\"err\":\"walk <x> <y>\"}", replyMax);
            return;
        }
        g_walkTX = nTX; g_walkTY = nTY;
        g_walkSteps = 0; g_walkStuck = 0; g_walkFrames = 0; g_walkJog = 0;
        if (D2Client::UNIT::CUnit::pPlayerUnit && D2Client::UNIT::CUnit::pPlayerUnit->pPath.pDynamicPath) {
            g_walkStartX = g_walkX = D2Client::UNIT::CUnit::pPlayerUnit->pPath.pDynamicPath->Coords.tSubtile.nPosX;
            g_walkStartY = g_walkY = D2Client::UNIT::CUnit::pPlayerUnit->pPath.pDynamicPath->Coords.tSubtile.nPosY;
        }
        InterlockedExchange(&g_walkOn, 1);
        lstrcpynA(reply, "{\"ok\":\"walk\"}", replyMax);
        return;
    }
    // Which UI panels the client thinks are open. "the shop did not open" and "the shop opened
    // and drew nothing" are different bugs and only the flags separate them.
    if (!strncmp(line, "ui", 2)) {
        static const struct { int id; const char* name; } aFlags[] = {
            { UI_INVENTORY, "inventory" }, { UI_NPCSHOP, "npcshop" }, { UI_STASH, "stash" },
            { UI_NPCMENU, "npcmenu" }, { UI_ESCMENU, "escmenu" }, { UI_CONFIG, "config" },
            { UI_AUTOMAP, "automap" }, { UI_WAYPOINT, "waypoint" }, { UI_QUESTSCREEN, "quest" },
        };
        static char uiOut[1024];
        int u = wsprintfA(uiOut, "{\"flags\":{");
        for (int i = 0; i < (int)(sizeof(aFlags)/sizeof(aFlags[0])); i++)
            u += wsprintfA(uiOut + u, "%s\"%s\":%d", i ? "," : "", aFlags[i].name,
                           (int)D2Client::UI::ui::GetUIFlag((eD2UIvars)aFlags[i].id));
        // The named five were the panels a test drives. A panel that is asked to open and
        // does not is indistinguishable from one that opened and drew nothing unless every
        // flag can be read, so print the raw array too - index is the eD2UIvars value.
        u += wsprintfA(uiOut + u, "},\"raw\":\"");
        for (int i = 0; i <= (int)UI_RECIPESCROLL && u < (int)sizeof(uiOut) - 8; i++)
            u += wsprintfA(uiOut + u, "%d", (&D2Client::UI::Draw::GFX_bShowUIFlag.Game)[i] ? 1 : 0);
        wsprintfA(uiOut + u, "\"}");
        lstrcpynA(reply, uiOut, replyMax);
        return;
    }
    // Per-opcode counts, not "seen once". "the shop packet never arrived" and "it arrived and
    // the panel ignored it" are different bugs and only a count separates them.
    if (!strncmp(line, "pkt", 3)) {
        extern unsigned g_anClientPktCount[256];
        extern unsigned g_anServerPktCount[256];
        static char pktOut[8192];
        int u = wsprintfA(pktOut, "{\"client\":{");
        int n = 0;
        for (int i = 0; i < 256; i++)
            if (g_anClientPktCount[i] && u < (int)sizeof(pktOut) - 60)
                u += wsprintfA(pktOut + u, "%s\"%02X\":%u", n++ ? "," : "", i, g_anClientPktCount[i]);
        u += wsprintfA(pktOut + u, "},\"server\":{");
        n = 0;
        for (int i = 0; i < 256; i++)
            if (g_anServerPktCount[i] && u < (int)sizeof(pktOut) - 60)
                u += wsprintfA(pktOut + u, "%s\"%02X\":%u", n++ ? "," : "", i, g_anServerPktCount[i]);
        wsprintfA(pktOut + u, "}}");
        lstrcpynA(reply, pktOut, replyMax);
        return;
    }
    // These touch game state, so they must run on the GAME thread - the walker queues into
    // D2CONTROL_Pump for exactly this reason. Calling CalculateTransactionCost straight from
    // the socket thread tripped a Fog assert inside the game's own pricing code.
    if (!strncmp(line, "shopitems", 9)) {
        InterlockedExchange(&g_reqShopList, 1);
        lstrcpynA(reply, g_shopList, replyMax);
        return;
    }
    // The player's own inventory, so a slot can be freed when the shop refuses a purchase
    // with "not enough free space" - which is the game's own rule firing correctly.
    if (!strncmp(line, "myitems", 7)) {
        InterlockedExchange(&g_reqMyList, 1);
        lstrcpynA(reply, g_myList, replyMax);
        return;
    }
    // freeslot <itemguid>: take an inventory item to the cursor (C->S 0x19) and drop it to the
    // ground (C->S 0x17). Both are [op:1][itemGUID:u32], the same packets the panel would send.
    // The shop refuses a purchase with "not enough free space" when the grid is full, and the
    // panel's own input layer is dead (INV_HandleShopLeftClick has zero xrefs), so this is the
    // only way to open a slot without hand-editing inventory state.
    if (!strncmp(line, "freeslot ", 9)) {
        int nGuid = 0;
        if (sscanf(line, "freeslot %d", &nGuid) != 1) {
            lstrcpynA(reply, "{\"err\":\"freeslot <itemguid>\"}", replyMax);
            return;
        }
        InterlockedExchange(&g_reqFreeGuid, nGuid);
        wsprintfA(reply, "{\"ok\":\"queued\",\"guid\":%d}", nGuid);
        return;
    }
    // shoptx <type> <itemguid>: 1 buy, 2 sell. Sets exactly what the shop panel's click
    // handler would set, then calls the game's own transaction function.
    if (!strncmp(line, "shoptx ", 7) || !strncmp(line, "buy ", 4)) {
        int nType = 1, nGuid = 0;
        if (sscanf(line, "shoptx %d %d", &nType, &nGuid) != 2 &&
            !(sscanf(line, "buy %d", &nGuid) == 1 && (nType = 1))) {
            lstrcpynA(reply, "{\"err\":\"shoptx <1buy|2sell> <itemguid>\"}", replyMax);
            return;
        }
        InterlockedExchange(&g_reqTxType, nType);
        InterlockedExchange(&g_reqBuyGuid, nGuid);
        wsprintfA(reply, "{\"ok\":\"queued\",\"type\":%d,\"guid\":%d}", nType, nGuid);
        return;
    }
    // npc.txt drives every shop price. CalculateTransactionCost falls through to a Fog assert
    // when TXT_npc_GetLine returns null, so the size and the .npc column are what to look at.
    if (!strncmp(line, "npctbl", 6)) {
        static char ntOut[2048];
        int u = wsprintfA(ntOut, "{\"size\":%d,\"ptr\":\"0x%X\",\"npc\":[",
                          D2Common::ITEMS::Items::sgptDataTable ? (int)D2Common::ITEMS::Items::sgptDataTable->nTxtNpcSize : -1,
                          D2Common::ITEMS::Items::sgptDataTable ? (unsigned)(uintptr_t)D2Common::ITEMS::Items::sgptDataTable->pTxtNpc : 0);
        int n = 0;
        if (D2Common::ITEMS::Items::sgptDataTable && D2Common::ITEMS::Items::sgptDataTable->pTxtNpc) {
            for (int i = 0; i < D2Common::ITEMS::Items::sgptDataTable->nTxtNpcSize && i < 48 && u < (int)sizeof(ntOut) - 40; i++)
                u += wsprintfA(ntOut + u, "%s%d", n++ ? "," : "", (int)D2Common::ITEMS::Items::sgptDataTable->pTxtNpc[i].npc);
        }
        u += wsprintfA(ntOut + u, "],\"lookup178\":\"0x%X\"}",
            (unsigned)(uintptr_t)D2Common::DATATBLS::MonsterTbls::TXT_npc_GetLine(178));
        lstrcpynA(reply, ntOut, replyMax);
        return;
    }
    // Raw memory, so the SAME bytes can be read out of the running original through the
    // injected probe and diffed. Pointers differ between the two builds; row counts, table
    // sizes and struct fields do not, and that is what a diff is for.
    //   hex <hexaddr> <len>      arbitrary address
    //   dt <off> <len>           bytes at &sgtDataTable + off  (the whole data-table struct)
    //   sym <name>               address of a named global in THIS build
    if (!strncmp(line, "hex ", 4) || !strncmp(line, "dt ", 3) || !strncmp(line, "dt", 2)) {
        unsigned long addr = 0; int len = 64, off = 0;
        if (!strncmp(line, "hex ", 4)) {
            if (sscanf(line, "hex %lx %d", &addr, &len) < 1) { lstrcpynA(reply, "{\"err\":\"hex <hexaddr> [len]\"}", replyMax); return; }
        } else {
            sscanf(line, "dt %d %d", &off, &len);
            if (len <= 0) len = 64;
            addr = (unsigned long)(uintptr_t)&D2Common::DATATBLS::MonsterTbls::sgtDataTable + off;
        }
        if (len < 1) len = 1;
        if (len > 1024) len = 1024;
        static char hxOut[3200];
        if (IsBadReadPtr((void*)addr, len)) { lstrcpynA(reply, "{\"err\":\"unreadable\"}", replyMax); return; }
        int u = wsprintfA(hxOut, "{\"addr\":\"0x%X\",\"len\":%d,\"hex\":\"", (unsigned)addr, len);
        const unsigned char* p = (const unsigned char*)addr;
        static const char* kHex = "0123456789abcdef";
        for (int i = 0; i < len; i++) { hxOut[u++] = kHex[p[i] >> 4]; hxOut[u++] = kHex[p[i] & 15]; }
        u += wsprintfA(hxOut + u, "\"}");
        hxOut[u] = 0;
        lstrcpynA(reply, hxOut, replyMax);
        return;
    }
    // Every data table's row count in one line. A table Ghidra sized to one element shows up
    // here as a count of 0 or 1 where the original has hundreds.
    if (!strncmp(line, "sizes", 5)) {
        static char szOut[4096];
        int u = wsprintfA(szOut, "{\"base\":\"0x%X\",\"sizes\":{", (unsigned)(uintptr_t)&D2Common::DATATBLS::MonsterTbls::sgtDataTable);
        struct { const char* name; int off; } kSizes[] = {
            {"CompCode",0x084},{"MonAi",0x090},{"Properties",0x0AC},{"States",0x0C4},
            {"Sounds",0x19C},{"Hireling",0x1A4},{"Npc",0x9AC},{"TreasureClass",0x9C0},
            {"MonStats",0xA80},{"MonSounds",0xA8C},{"MonStatsEx",0xA98},{"MonPlace",0xAA4},
            {"SuperUniques",0xADC},{"Missiles",0xB6C},{"MonLvl",0xB74},{"MonSeq",0xB80},
            {"SkillDesc",0xB94},{"Overlay",0xBC0},{"CharStats",0xBC8},{"ItemStatCost",0xBD4},
            {"MonEquip",0xBE4},{"PetType",0xBF0},{"ItemTypes",0xBFC},{"Sets",0xC10},
            {"SetItems",0xC1C},{"UniqueItems",0xC28},{"MonProp",0xC34},{"MonType",0xC40},
            {"MonUmod",0xC54},{"Levels",0xC5C},{"LvlPrest",0xC68},{"DifficultyLevels",0xC80},
        };
        for (int i = 0; i < (int)(sizeof(kSizes)/sizeof(kSizes[0])); i++) {
            int v = *(int*)((char*)&D2Common::DATATBLS::MonsterTbls::sgtDataTable + kSizes[i].off);
            u += wsprintfA(szOut + u, "%s\"%s\":%d", i ? "," : "", kSizes[i].name, v);
        }
        u += wsprintfA(szOut + u, "}}");
        lstrcpynA(reply, szOut, replyMax);
        return;
    }
    // How much the automap is actually asked to draw. The original draws a few hundred cells
    // a frame; if this reports tens of thousands the bug is in what was REVEALED, not in the
    // draw. Counts every layer, so a leaked layer list shows up as a layer count that climbs.
    if (!strncmp(line, "amap", 4)) {
        static char amOut[2048];
        int nLayers = 0, nFloors = 0, nWalls = 0, nObjects = 0, nExtras = 0, nMaxDepth = 0;
        for (D2AutomapLayerStrc* pLayer = D2Client::UI::Automap::sgpAutomapLayer; pLayer && nLayers < 256; pLayer = pLayer->pNext) {
            nLayers++;
            nFloors  += AmapCount(pLayer->pFloors,  1, &nMaxDepth);
            nWalls   += AmapCount(pLayer->pWalls,   1, &nMaxDepth);
            nObjects += AmapCount(pLayer->pObjects, 1, &nMaxDepth);
            nExtras  += AmapCount(pLayer->pExtras,  1, &nMaxDepth);
        }
        wsprintfA(amOut,
            "{\"layers\":%d,\"floors\":%d,\"walls\":%d,\"objects\":%d,\"extras\":%d,\"maxdepth\":%d,"
            "\"head\":\"0x%X\",\"cur\":\"0x%X\",\"fade\":%d,\"zoom\":%d,\"scrollx\":%d,\"scrolly\":%d}",
            nLayers, nFloors, nWalls, nObjects, nExtras, nMaxDepth,
            (unsigned)(uintptr_t)D2Client::UI::Automap::sgpAutomapLayer, (unsigned)(uintptr_t)D2Client::UI::Automap::sgpAutomapLayerEx,
            (int)D2Client::UI::Automap::AUTOMAP_GetFade(), (int)D2Client::UI::Automap::nAutoMapZoomDivisor,
            (int)D2Client::UI::Automap::nAutoMapScrollX, (int)D2Client::UI::Automap::nAutoMapScrollY);
        lstrcpynA(reply, amOut, replyMax);
        return;
    }
    // Nothing AI-driven moves - not monsters, not town NPCs, not the merc. That is one systemic
    // failure, and three different ones look identical on screen: the AI never ran, it ran and
    // produced no path, or it produced a path and the anim mode never left Neutral. Report the
    // mode, the path point count and the target per server-side monster so a single run says which.
    if (!strncmp(line, "ai", 2)) {
        int nStart = 0, nCount = 12;
        sscanf(line, "ai %d %d", &nStart, &nCount);
        static char aiOut[8192];
        int u = wsprintfA(aiOut, "{\"mon\":[");
        int nSeen = 0, nEmitted = 0, nMoving = 0, nWithPath = 0, nWithTarget = 0;
        D2UnitStrc** ppTable = (D2UnitStrc**)((char*)D2Client::UNIT::CUnit::ServerSideUnitHashTables + UNIT_MONSTER * 0x200);
        for (int nBucket = 0; nBucket < 128; nBucket++) {
            for (D2UnitStrc* pUnit = ppTable[nBucket]; pUnit; pUnit = pUnit->pListNext) {
                const D2DynamicPathStrc* pPath = pUnit->pPath.pDynamicPath;
                const int nMode = (int)pUnit->eAnimMode.field_1c;
                const int nPts = pPath ? (int)pPath->dwPathPoints : -1;
                if (nPts > 0) nWithPath++;
                if (pPath && (pPath->nTargetX || pPath->nTargetY)) nWithTarget++;
                if (nMode != 0) nMoving++;
                nSeen++;
                if (nSeen <= nStart || nEmitted >= nCount || u > (int)sizeof(aiOut) - 220) continue;
                char szName[34]; szName[0] = 0;
                if (D2Common::ITEMS::Items::sgptDataTable && pUnit->nClassId >= 0 &&
                    pUnit->nClassId < D2Common::ITEMS::Items::sgptDataTable->nTxtMonStatsSize) {
                    const uint16_t* pwsz = D2Lang::StrTable::strtable::GetLocaleString(
                        D2Common::ITEMS::Items::sgptDataTable->pTxtMonStats[pUnit->nClassId].NameStr);
                    if (pwsz) { int k = 0; for (; k < 26 && pwsz[k]; k++) szName[k] = (char)(pwsz[k] & 0x7f); szName[k] = 0; }
                }
                // fpFunction is the whole question: if it is null, AttachCorrectAiFunctionToMonster
                // never installed an AI, so "no path" is a symptom and not the cause.
                const D2AiGeneralStrc* pAi = D2Game::UNIT::Monster::GetAiGeneral(pUnit);
                u += wsprintfA(aiOut + u,
                    "%s{\"cls\":%d,\"guid\":%d,\"mode\":%d,\"x\":%d,\"y\":%d,\"tx\":%d,\"ty\":%d,"
                    "\"pts\":%d,\"idx\":%d,\"ptype\":%d,\"uf\":\"0x%X\","
                    "\"ai\":\"0x%X\",\"aifn\":\"0x%X\",\"aiid\":%d,\"aicmd\":\"0x%X\",\"n\":\"%s\"}",
                    nEmitted ? "," : "", (int)pUnit->nClassId, (int)pUnit->nUnitGUID, nMode,
                    pPath ? (int)pPath->Coords.tSubtile.nPosX : -1,
                    pPath ? (int)pPath->Coords.tSubtile.nPosY : -1,
                    pPath ? (int)pPath->nTargetX : -1, pPath ? (int)pPath->nTargetY : -1,
                    nPts, pPath ? (int)pPath->nPathPointIndex : -1,
                    pPath ? (int)pPath->ePathType : -1,
                    (unsigned)pUnit->eUnitFlags,
                    (unsigned)(uintptr_t)pAi,
                    (unsigned)(uintptr_t)(pAi ? (void*)pAi->fpFunction : 0),
                    pAi ? (int)pAi->nFunctionId : -1,
                    (unsigned)(uintptr_t)(pAi ? (void*)pAi->pCmdCurrent : 0),
                    szName);
                nEmitted++;
            }
        }
        wsprintfA(aiOut + u, "],\"total\":%d,\"nonneutral\":%d,\"withpath\":%d,\"withtarget\":%d}",
                  nSeen, nMoving, nWithPath, nWithTarget);
        lstrcpynA(reply, aiOut, replyMax);
        return;
    }
    // "walls are not drawn" has two completely different causes and the screen cannot tell them
    // apart: either the collect pass put nothing in the wall2 grid, or it did and the draw pass
    // skipped it. WALL2_IsInDrawGrid once rejected every tile (191097 rejects, 0 inserts) and
    // looked exactly like a broken renderer. Count the grid instead of guessing.
    if (!strncmp(line, "wall2", 5)) {
        static char wOut[1024];
        D2GameViewStrc* pView = D2Client::GAME::Game::D2GameView;
        if (!pView) { lstrcpynA(reply, "{\"err\":\"no game view\"}", replyMax); return; }
        int nCells = (int)pView->sWall2.nWall2GridDim * (int)pView->sWall2.nWall2GridDim;
        if (nCells < 0 || nCells > 65536) nCells = 0;
        int nUsed = 0, aHead[9] = { 0 };
        const unsigned char* pGrid = (const unsigned char*)pView->sWall2.pWall2;
        if (pGrid && !IsBadReadPtr(pGrid, (UINT_PTR)nCells * 0x24)) {
            for (int i = 0; i < nCells; i++) {
                const uint32_t* pCell = (const uint32_t*)(pGrid + (size_t)i * 0x24);
                int bAny = 0;
                for (int w = 0; w < 9; w++) if (pCell[w]) { aHead[w]++; bAny = 1; }
                if (bAny) nUsed++;
            }
        }
        wsprintfA(wOut,
            "{\"alloc\":%u,\"entries\":%u,\"dim\":%u,\"width\":%d,\"originx\":%d,\"originy\":%d,"
            "\"grid\":\"0x%X\",\"cells\":%d,\"cellsused\":%d,\"heads\":[%d,%d,%d,%d,%d,%d,%d,%d,%d]}",
            (unsigned)pView->sWall2.dwWall2PassMask, (unsigned)pView->sWall2.nWall2EntryCount,
            (unsigned)pView->sWall2.nWall2GridDim, (int)pView->sWall2.nWall2GridWidth,
            (int)pView->sWall2.nWall2GridOriginX, (int)pView->sWall2.nWall2GridOriginY,
            (unsigned)(uintptr_t)pGrid, nCells, nUsed,
            aHead[0], aHead[1], aHead[2], aHead[3], aHead[4],
            aHead[5], aHead[6], aHead[7], aHead[8]);
        lstrcpynA(reply, wOut, replyMax);
        return;
    }
    // Histogram of every drawn wall tile's DT1 direction. Direction 0 is the tell: the
    // intensity table's slot 0 is zero, so the light-sample loop is skipped and the sample
    // buffer reaches the blitter uninitialised - black walls. A direction-0 tile is also
    // what GetTileLibraryEntry's miss path substitutes, so a large bucket 0 means the wall
    // tiles are absent from the loaded DT1 set rather than mis-lit.
    if (!strncmp(line, "walldir", 7)) {
        static char dOut[1024];
        D2GameViewStrc* pView = D2Client::GAME::Game::D2GameView;
        if (!pView) { lstrcpynA(reply, "{\"err\":\"no game view\"}", replyMax); return; }
        int nCells = (int)pView->sWall2.nWall2GridDim * (int)pView->sWall2.nWall2GridDim;
        if (nCells < 0 || nCells > 65536) nCells = 0;
        // cell+0x14 is the list dWallPass walks. node+0xc is the D2DrawWallStrc, whose
        // +0x18 is the DT1 library entry; the entry's +0x00 is nDirection and +0x14 the
        // orientation. Reading the node or the wall struct directly gives world coords.
        int kSlot = 0x14;
        sscanf(line, "walldir %x", &kSlot);
        int aDir[17] = { 0 }, aOri[17] = { 0 }, nNodes = 0, nBadTile = 0, nDrawn = 0;
        int nZeroBlk = 0, nMaxBlk = 0, nNoLru = 0;
        const unsigned char* pGrid = (const unsigned char*)pView->sWall2.pWall2;
        if (pGrid && !IsBadReadPtr(pGrid, (UINT_PTR)nCells * 0x24)) {
            for (int i = 0; i < nCells; i++) {
                int nNode = *(const int*)(pGrid + (size_t)i * 0x24 + kSlot);
                for (int guard = 0; nNode && guard < 4096; guard++) {
                    if (IsBadReadPtr((const void*)nNode, 0x14)) break;
                    int nWall = *(const int*)(nNode + 0xc);
                    if (nWall && !IsBadReadPtr((const void*)nWall, 0x30)) {
                        unsigned nFlags = *(const unsigned*)(nWall + 0x14);
                        if (!(nFlags & 0x408) && (nFlags & 0x1c000)) nDrawn++;
                        int nEntry = *(const int*)(nWall + 0x18);
                        if (nEntry && !IsBadReadPtr((const void*)nEntry, 0x60)) {
                            int d = *(const int*)nEntry;
                            int o = *(const int*)(nEntry + 0x14);
                            aDir[(d >= 0 && d < 16) ? d : 16]++;
                            aOri[(o >= 0 && o < 16) ? o : 16]++;
                            // +0x50 is the DT1 block count. DrawWallTile bails before writing
                            // a single pixel when SPRITECACHE_AllocEntry sees it as 0, so a
                            // zero here is a wall that is not black but absent.
                            int nBlocks = *(const int*)(nEntry + 0x50);
                            if (!nBlocks) nZeroBlk++;
                            if (nBlocks > nMaxBlk) nMaxBlk = nBlocks;
                            if (!*(const int*)(nEntry + 0x5c)) nNoLru++;
                        } else {
                            nBadTile++;
                        }
                    } else {
                        nBadTile++;
                    }
                    nNodes++;
                    nNode = *(const int*)(nNode + 0x10);
                }
            }
        }
        wsprintfA(dOut,
            "{\"slot\":\"0x%X\",\"nodes\":%d,\"drawn\":%d,\"badtile\":%d,\"zeroblocks\":%d,\"maxblocks\":%d,\"nolru\":%d,"
            "\"dir\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],\"dirother\":%d,"
            "\"ori\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],\"oriother\":%d}",
            (unsigned)kSlot, nNodes, nDrawn, nBadTile, nZeroBlk, nMaxBlk, nNoLru,
            aDir[0], aDir[1], aDir[2], aDir[3], aDir[4], aDir[5], aDir[6], aDir[7],
            aDir[8], aDir[9], aDir[10], aDir[11], aDir[12], aDir[13], aDir[14], aDir[15],
            aDir[16],
            aOri[0], aOri[1], aOri[2], aOri[3], aOri[4], aOri[5], aOri[6], aOri[7],
            aOri[8], aOri[9], aOri[10], aOri[11], aOri[12], aOri[13], aOri[14], aOri[15],
            aOri[16]);
        lstrcpynA(reply, dOut, replyMax);
        return;
    }
    // Ask the engine for the light sample at each drawn wall tile, exactly the way dWallPass
    // does. The lightmap is fully lit, so if these come back zero the sampling is at fault;
    // if they come back lit, the samples are fine and the defect is inside the blitter.
    if (!strncmp(line, "wlight", 6)) {
        static char lo[1024];
        D2GameViewStrc* pView = D2Client::GAME::Game::D2GameView;
        if (!pView) { lstrcpynA(reply, "{\"err\":\"no game view\"}", replyMax); return; }
        int nCells = (int)pView->sWall2.nWall2GridDim * (int)pView->sWall2.nWall2GridDim;
        if (nCells < 0 || nCells > 65536) nCells = 0;
        const unsigned char* pGrid = (const unsigned char*)pView->sWall2.pWall2;
        int u = wsprintfA(lo, "{\"samples\":[");
        int n = 0, nZero = 0, nLit = 0;
        if (pGrid && !IsBadReadPtr(pGrid, (UINT_PTR)nCells * 0x24)) {
            for (int i = 0; i < nCells && u < (int)sizeof(lo) - 120; i++) {
                int nNode = *(const int*)(pGrid + (size_t)i * 0x24 + 0x14);
                for (int g = 0; nNode && g < 64; g++) {
                    if (IsBadReadPtr((const void*)nNode, 0x14)) break;
                    int wx = *(const int*)(nNode + 4);
                    int wy = *(const int*)(nNode + 8);
                    uint32_t packed = 0; byte r = 0, gg = 0, b = 0;
                    D2Client::GAME::View::DRAW_CoordIsInCoord(wx, wy, (byte*)&packed, &r, &gg, &b);
                    byte inten = (byte)packed;
                    if (inten) nLit++; else nZero++;
                    if (n < 6 && u < (int)sizeof(lo) - 120) {
                        u += wsprintfA(lo + u, "%s{\"wx\":%d,\"wy\":%d,\"i\":%d,\"r\":%d,\"g\":%d,\"b\":%d}",
                                       n ? "," : "", wx, wy, (int)inten, (int)r, (int)gg, (int)b);
                    }
                    n++;
                    nNode = *(const int*)(nNode + 0x10);
                }
            }
        }
        wsprintfA(lo + u, "],\"n\":%d,\"lit\":%d,\"zero\":%d,\"colorEnabled\":%u,\"biasX\":%d,\"biasY\":%d}", n, nLit, nZero,
                  (unsigned)D2Client::Draw::LightMap::gbLightmapColorEnabled,
                  (int)D2Client::Draw::LightMap::gnLightmapWorldRefreshFlag, (int)D2Client::Draw::LightMap::gnLightmapBufferState);
        lstrcpynA(reply, lo, replyMax);
        return;
    }
    // SELECT_UpdateSelectedUnit hit-tests the player's room plus its adjacent rooms, NOT the
    // unit hash tables. A dangling or wrong-typed entry in a room list is invisible to `units`
    // but is exactly what makes HitTestScreen look up an item's class id in Objects.txt.
    if (!strncmp(line, "roomunits", 9)) {
        static char ro[1024];
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        D2RoomStrc* pRoom = pPl ? D2Common::Units::UNITS_GetRoom(pPl) : nullptr;
        if (!pRoom) { lstrcpynA(reply, "{\"err\":\"no room\"}", replyMax); return; }
        int nTotal = 0, nBad = 0, nRooms = 0;
        int aMax[8] = { 0 };
        int badType = -1, badCls = -1;
        D2RoomStrc** ppList = *(D2RoomStrc***)((char*)pRoom + 0x00);
        int nList = *(int*)((char*)pRoom + 0x24);
        if (nList < 0 || nList > 64) nList = 0;
        for (int i = -1; i < nList; i++) {
            D2RoomStrc* pR = (i < 0) ? pRoom : (ppList ? ppList[i] : nullptr);
            if (!pR || IsBadReadPtr(pR, 0x80)) continue;
            nRooms++;
            D2UnitStrc* pU = *(D2UnitStrc**)((char*)pR + 0x74);
            for (int g = 0; pU && g < 4096; g++) {
                if (IsBadReadPtr(pU, 0xEC)) { nBad++; break; }
                int t = *(const int*)pU;
                int c = *(const int*)((char*)pU + 4);
                nTotal++;
                if (t >= 0 && t < 8 && c > aMax[t]) aMax[t] = c;
                if (t == 2 && (c < 0 || c >= D2Common::DATATBLS::ObjectTbls::nTxtObjectsSize)) {
                    nBad++; if (badType < 0) { badType = t; badCls = c; }
                }
                pU = *(D2UnitStrc**)((char*)pU + 0xE8);
            }
        }
        wsprintfA(ro,
            "{\"rooms\":%d,\"units\":%d,\"badobjects\":%d,\"badcls\":%d,\"objtblsize\":%d,"
            "\"maxcls\":[%d,%d,%d,%d,%d,%d,%d,%d]}",
            nRooms, nTotal, nBad, badCls, D2Common::DATATBLS::ObjectTbls::nTxtObjectsSize,
            aMax[0], aMax[1], aMax[2], aMax[3], aMax[4], aMax[5], aMax[6], aMax[7]);
        lstrcpynA(reply, ro, replyMax);
        return;
    }
    // Histogram the RAW DS1 orientation bytes, per wall layer, straight out of the cached
    // file. This is the source data: if no cell anywhere carries orientation 12, the columns
    // were never tiles to begin with and every downstream stage is innocent.
    if (!strncmp(line, "ds1ori", 6)) {
        static char oo[1600];
        char szWant[64] = "Fortress";
        sscanf(line, "ds1ori %63s", szWant);
        int u = wsprintfA(oo, "{\"file\":\"%s\",\"layers\":[", szWant);
        int nEmitted = 0;
        for (D2DrlgFileCacheNodeStrc* pN = D2Common::Drlg::Preset::gpDrlgFileCache;
             pN && !IsBadReadPtr(pN, 0x110); pN = pN->pNext) {
            const char* nm = pN->szName; const char* base = nm;
            for (const char* q = nm; *q; q++) if (*q == '\\' || *q == '/') base = q + 1;
            if (!strstr(base, szWant)) continue;
            D2DrlgFileStrc* pF = pN->pDrlgFile;
            if (!pF || IsBadReadPtr(pF, 0x5c)) continue;
            int nCells = (pF->nHeight + 1) * (pF->nWidth + 1);
            if (nCells < 0 || nCells > 1 << 20) break;
            for (int L = 0; L < pF->nWallLayers && L < 4; L++) {
                const int* p = (const int*)pF->pTileTypeLayer[L];
                if (!p || IsBadReadPtr(p, (UINT_PTR)nCells * 4)) continue;
                int a[24] = { 0 }, other = 0;
                for (int c = 0; c < nCells; c++) {
                    int t = p[c];
                    if (t >= 0 && t < 24) a[t]++; else other++;
                }
                if (u > (int)sizeof(oo) - 300) break;
                u += wsprintfA(oo + u, "%s{\"L\":%d,\"cells\":%d,\"t\":{", nEmitted ? "," : "", L, nCells);
                int first = 1;
                for (int t = 0; t < 24; t++)
                    if (a[t] && u < (int)sizeof(oo) - 60) {
                        u += wsprintfA(oo + u, "%s\"%d\":%d", first ? "" : ",", t, a[t]); first = 0;
                    }
                u += wsprintfA(oo + u, "},\"other\":%d}", other);
                nEmitted++;
            }
            break;
        }
        wsprintfA(oo + u, "],\"n\":%d}", nEmitted);
        lstrcpynA(reply, oo, replyMax);
        return;
    }
    // Walk EVERY room in the level via pRoomEx->pRoomExNext, not just the player's near
    // list, and evaluate the room-level gate that WALL2_CollectRoomTilesAndUnits applies.
    // A room rejected there contributes no tiles at all, which is what a whole missing
    // colonnade looks like.
    // Objects reach the draw list but paint nothing: check whether their graphics are
    // actually loaded. pAnimData +0x50, pGfxInfo +0x54, pGfxDataCopy +0x58.
    if (!strncmp(line, "objgfx", 6)) {
        static char og[1536];
        int u = wsprintfA(og, "{\"objs\":[");
        int n = 0;
        for (int tbl = 0; tbl < 2 && u < (int)sizeof(og) - 120; tbl++) {
            void* pBase = tbl ? (void*)D2Client::UNIT::CUnit::ServerSideUnitHashTables : (void*)D2Client::UNIT::CUnit::ClientSideUnitHashTables;
            // rows are type-major with a 0x200 stride, as UnitsReply does it
            D2UnitStrc** pT = (D2UnitStrc**)((char*)pBase + UNIT_OBJECT * 0x200);
            for (int b = 0; b < 128 && u < (int)sizeof(og) - 120; b++) {
                for (D2UnitStrc* pU = pT[b];
                     pU && !IsBadReadPtr(pU, 0x60); pU = pU->pListNext) {
                    int cls = (int)pU->nClassId;
                    if (cls != 409 && cls != 408) continue;
                    u += wsprintfA(og + u,
                        "%s{\"t\":%d,\"cls\":%d,\"guid\":%d,\"mode\":%d,\"anim\":\"%c\",\"gfx\":\"%c\",\"copy\":\"%c\"}",
                        n ? "," : "", tbl, cls, (int)pU->nUnitGUID, (int)pU->eAnimMode.ePlayerMode,
                        pU->pAnimData ? 'Y' : 'n', pU->pGfxInfo ? 'Y' : 'n', pU->pGfxDataCopy ? 'Y' : 'n');
                    if (++n >= 14) break;
                }
                if (n >= 14) break;
            }
        }
        wsprintfA(og + u, "],\"n\":%d}", n);
        lstrcpynA(reply, og, replyMax);
        return;
    }
    // What IS a given object class? Name straight out of Objects.txt.
    if (!strncmp(line, "objname", 7)) {
        static char on[512];
        int a = 409, b = 408;
        sscanf(line, "objname %d %d", &a, &b);
        D2ObjectsTxt* pA = D2Common::DATATBLS::ObjectTbls::TXT_Objects_GetLine(a);
        D2ObjectsTxt* pB = D2Common::DATATBLS::ObjectTbls::TXT_Objects_GetLine(b);
        wsprintfA(on,
            "{\"%d\":{\"tok\":\"%c%c\",\"draw\":%d,\"pieces\":%d,\"mode0\":%d,"
            "\"frameCnt\":\"%d %d %d %d %d %d %d %d\"},"
            "\"%d\":{\"tok\":\"%c%c\",\"draw\":%d,\"pieces\":%d,\"mode0\":%d,"
            "\"frameCnt\":\"%d %d %d %d %d %d %d %d\"}}",
            a, pA ? pA->Token[0] : '?', pA ? pA->Token[1] : '?',
            pA ? (int)pA->Draw : -1, pA ? (int)pA->TotalPieces : -1, pA ? (int)pA->Mode[0] : -1,
            pA ? pA->FrameCnt[0] : -1, pA ? pA->FrameCnt[1] : -1, pA ? pA->FrameCnt[2] : -1,
            pA ? pA->FrameCnt[3] : -1, pA ? pA->FrameCnt[4] : -1, pA ? pA->FrameCnt[5] : -1,
            pA ? pA->FrameCnt[6] : -1, pA ? pA->FrameCnt[7] : -1,
            b, pB ? pB->Token[0] : '?', pB ? pB->Token[1] : '?',
            pB ? (int)pB->Draw : -1, pB ? (int)pB->TotalPieces : -1, pB ? (int)pB->Mode[0] : -1,
            pB ? pB->FrameCnt[0] : -1, pB ? pB->FrameCnt[1] : -1, pB ? pB->FrameCnt[2] : -1,
            pB ? pB->FrameCnt[3] : -1, pB ? pB->FrameCnt[4] : -1, pB ? pB->FrameCnt[5] : -1,
            pB ? pB->FrameCnt[6] : -1, pB ? pB->FrameCnt[7] : -1);
        lstrcpynA(reply, on, replyMax);
        return;
    }
    if (!strncmp(line, "levelrooms", 10)) {
        static char lr[1024];
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        D2RoomStrc* pRoom = pPl ? D2Common::Units::UNITS_GetRoom(pPl) : nullptr;
        if (!pRoom) { lstrcpynA(reply, "{\"err\":\"no room\"}", replyMax); return; }
        D2GameViewStrc* pGV = D2Client::GAME::Game::D2GameView;
        int L = pGV ? pGV->nCenterX : 0, T = pGV ? pGV->nCenterY : 0;
        int R = pGV ? (pGV->pView.right - pGV->pView.left + pGV->nCenterX) : 0;
        int B = pGV ? (pGV->pView.bottom - pGV->pView.top + pGV->nCenterY) : 0;
        void* pEx0 = *(void**)((char*)pRoom + 0x10);
        int nTotal = 0, nActive = 0, nPass = 0, nTiles = 0, nTilesPass = 0;
        int aT[24] = { 0 }, aPU[8] = { 0 }, aSkip[8] = { 0 }, nPreset = 0, nSkipFlag = 0;
        static char names[512]; names[0] = 0; int un = 0, nNamed = 0;
        // Start from the LEVEL's room-list head (pRoomEx->pLevel->pRoomExFirst), not from
        // the player's room - a forward walk only sees the tail of the list.
        int nLevelRooms = -1;
        if (pEx0 && !IsBadReadPtr(pEx0, 0x60)) {
            void* pLvl = *(void**)((char*)pEx0 + 0x58);
            if (pLvl && !IsBadReadPtr(pLvl, 0x20)) {
                nLevelRooms = *(const int*)((char*)pLvl + 0x08);
                void* pHead = *(void**)((char*)pLvl + 0x10);
                if (pHead && !IsBadReadPtr(pHead, 0x60)) pEx0 = pHead;
            }
        }
        for (void* pEx = pEx0; pEx && !IsBadReadPtr(pEx, 0x60) && nTotal < 4096;
             pEx = *(void**)((char*)pEx + 0x24)) {
            nTotal++;
            D2RoomStrc* pR = *(D2RoomStrc**)((char*)pEx + 0x30);
            if (!pR || IsBadReadPtr(pR, 0x80)) continue;
            nActive++;
            const unsigned char* pTG = *(const unsigned char**)((char*)pEx + 0x54);
            int nW = 0;
            if (pTG && !IsBadReadPtr(pTG, 0x20)) nW = *(const int*)(pTG + 0x08);
            if (nW < 0 || nW > 8192) nW = 0;
            nTiles += nW;
            // Which DS1 is this room built from? pRoomExData +0x08 pMap, pMap +0x0C pDrlgFile,
            // then match that pointer back to a cache node to recover the filename.
            if (nNamed < 12 && un < (int)sizeof(names) - 40) {
                const char* nm = "?";
                int pt = *(const int*)((char*)pEx + 0x48);
                const unsigned char* pData = *(const unsigned char**)((char*)pEx + 0x20);
                if (pt == 2 && pData && !IsBadReadPtr(pData, 0x10)) {
                    const unsigned char* pMap = *(const unsigned char**)(pData + 0x08);
                    if (pMap && !IsBadReadPtr(pMap, 0x20)) {
                        const void* pDF = *(const void**)(pMap + 0x0C);
                        for (D2DrlgFileCacheNodeStrc* pC = D2Common::Drlg::Preset::gpDrlgFileCache;
                             pC && !IsBadReadPtr(pC, 0x110); pC = pC->pNext)
                            if ((const void*)pC->pDrlgFile == pDF) {
                                nm = pC->szName;
                                for (const char* q = pC->szName; *q; q++)
                                    if (*q == '\\' || *q == '/') nm = q + 1;
                                break;
                            }
                    }
                }
                un += wsprintfA(names + un, "%s%s:%d", nNamed ? " " : "", nm, nW);
                nNamed++;
            }
            // Preset units the DS1 places in this room, by unit type. Compare against the
            // objects that actually exist: a shortfall means preset spawn is dropping them.
            for (const unsigned char* pPU = *(const unsigned char**)((char*)pEx + 0x5C);
                 pPU && !IsBadReadPtr(pPU, 0x20) && nPreset < 4096;
                 pPU = *(const unsigned char**)(pPU + 0x0C)) {
                int et = *(const int*)(pPU + 0x14);
                int fl = *(const int*)(pPU + 0x1C);
                nPreset++;
                if (et >= 0 && et < 8) aPU[et]++;
                // SUNIT_SpawnPresetUnits skips any preset with nFlags bit 0 set.
                if (fl & 1) { nSkipFlag++; if (et >= 0 && et < 8) aSkip[et]++; }
            }
            const unsigned char* pWT = (pTG && nW) ? *(const unsigned char**)(pTG + 0x14) : nullptr;
            if (pWT && !IsBadReadPtr(pWT, (UINT_PTR)nW * 0x30))
                for (int k = 0; k < nW; k++) {
                    int tt = *(const int*)(pWT + (size_t)k * 0x30 + 0x1C);
                    if (tt >= 0 && tt < 24) aT[tt]++;
                }
            D2Wall2DrawContextStrc ctx;
            ctx.pGameView = pGV; ctx.pRoom = pR; ctx.nTickCount = 0;
            ctx.rcCull.left = L; ctx.rcCull.top = T; ctx.rcCull.right = R; ctx.rcCull.bottom = B;
            int mm[4] = { 0, 0, 0, 0 };
            D2Common::DUNGEON::Dungeon::DRLGROOM_GetMinimapRect((uint32_t)(uintptr_t)&ctx,
                                                                pR, mm);
            if (mm[0] <= R && mm[1] <= B + 400 && L - 200 <= mm[2] && T <= mm[3]) {
                nPass++; nTilesPass += nW;
            }
        }
        wsprintfA(lr,
            "{\"levelRoomCount\":%d,\"walked\":%d,\"active\":%d,\"passRoomGate\":%d,"
            "\"wallTilesTotal\":%d,\"wallTilesInPassingRooms\":%d,\"types\":\"",
            nLevelRooms, nTotal, nActive, nPass, nTiles, nTilesPass);
        {
            int uu = lstrlenA(lr);
            for (int t = 0; t < 24; t++)
                if (aT[t]) uu += wsprintfA(lr + uu, "%d:%d ", t, aT[t]);
            uu += wsprintfA(lr + uu, "\",\"presetUnits\":%d,\"presetByType\":\"", nPreset);
            for (int t = 0; t < 8; t++)
                if (aPU[t]) uu += wsprintfA(lr + uu, "%d:%d ", t, aPU[t]);
            uu += wsprintfA(lr + uu, "\",\"skippedByFlag1\":%d,\"skipByType\":\"", nSkipFlag);
            for (int t = 0; t < 8; t++)
                if (aSkip[t]) uu += wsprintfA(lr + uu, "%d:%d ", t, aSkip[t]);
            wsprintfA(lr + uu, "\",\"rooms\":\"%s\"}", names);
        }
        if (0) wsprintfA(lr, "%d%d%d%d", L, T, R, B);
        lstrcpynA(reply, lr, replyMax);
        return;
    }
    // Histogram of nTileType over the tiles the ROOM actually created, before the draw grid
    // collects them. Orientation 12 is pillars/columns/standalone objects. If 12 appears here
    // but not in `walldir`, the tiles are created and lost in collection; if it is absent
    // here too, they were never created and the fault is upstream in the grid.
    if (!strncmp(line, "roomtiles", 9)) {
        static char rt[6144];
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        D2RoomStrc* pRoom = pPl ? D2Common::Units::UNITS_GetRoom(pPl) : nullptr;
        if (!pRoom) { lstrcpynA(reply, "{\"err\":\"no room\"}", replyMax); return; }
        int aType[24] = { 0 }, aNeg[24] = { 0 }, aOn[24] = { 0 }, aNoLayer[24] = { 0 }, nRooms = 0, nTiles = 0, nOther = 0;
        // nScreenX/Y on a tile are ABSOLUTE iso coords; subtract the camera origin to get
        // viewport coords, the same offsets the click path uses.
        int nOffX = D2Client::UI::Path::DRAW_GetWorldOffsetX();
        int nOffY = D2Client::UI::Path::DRAW_GetWorldOffsetY();
        int aMinX[24], aMaxX[24], aMinY[24], aMaxY[24], aMinH[24], aMaxH[24];
        int aRejX[24] = { 0 }, aRejY[24] = { 0 }, aRejG[24] = { 0 }, aPass[24] = { 0 };
        int aMinR[24], aMaxR[24];
        for (int z = 0; z < 24; z++) { aMinR[z] = 1<<28; aMaxR[z] = -(1<<28); }
        D2GameViewStrc* pGV = D2Client::GAME::Game::D2GameView;
        int nCullL = pGV ? pGV->nCenterX : 0, nCullT = pGV ? pGV->nCenterY : 0;
        int nCullR = pGV ? (pGV->pView.right - pGV->pView.left + pGV->nCenterX) : 0;
        int nCullB = pGV ? (pGV->pView.bottom - pGV->pView.top + pGV->nCenterY) : 0;
        D2Wall2DrawListStrc* pAllocB = pGV ? &pGV->sWall2 : nullptr;
        for (int z = 0; z < 24; z++) { aMinX[z]=aMinY[z]=aMinH[z]=1<<28; aMaxX[z]=aMaxY[z]=aMaxH[z]=-(1<<28); }
        int nNegRoof = 0, nMinRoof = 32767, nMaxRoof = -32768;
        D2RoomStrc** ppList = *(D2RoomStrc***)((char*)pRoom + 0x00);
        int nList = *(int*)((char*)pRoom + 0x24);
        if (nList < 0 || nList > 64) nList = 0;
        for (int i = -1; i < nList; i++) {
            D2RoomStrc* pR = (i < 0) ? pRoom : (ppList ? ppList[i] : nullptr);
            if (!pR || IsBadReadPtr(pR, 0x80)) continue;
            void* pEx = *(void**)((char*)pR + 0x10);
            if (!pEx || IsBadReadPtr(pEx, 0x60)) continue;
            const unsigned char* pTG = *(const unsigned char**)((char*)pEx + 0x54);
            if (!pTG || IsBadReadPtr(pTG, 0x20)) continue;
            nRooms++;
            int nWalls = *(const int*)(pTG + 0x08);
            const unsigned char* pWT = *(const unsigned char**)(pTG + 0x14);
            if (nWalls < 0 || nWalls > 8192 || !pWT) continue;
            if (IsBadReadPtr(pWT, (UINT_PTR)nWalls * 0x30)) continue;
            for (int k = 0; k < nWalls; k++) {
                const unsigned char* pT = pWT + (size_t)k * 0x30;
                int t = *(const int*)(pT + 0x1C);
                if (t >= 0 && t < 24) aType[t]++; else nOther++;
                // The cull MOVZX-extends roofHeight, so a NEGATIVE value becomes ~65440 and
                // the tile is culled off-screen. Count those, split by tile type.
                // Each tile already carries its computed screen position, so "is it on
                // camera" is a direct question, not something to re-derive from the cull.
                int sx = *(const int*)(pT + 0x00) - nOffX;
                int sy = *(const int*)(pT + 0x04) - nOffY;
                if (t >= 0 && t < 24) {
                    if (sx > -160 && sx < 960 && sy > -160 && sy < 760) aOn[t]++;
                    if (sx < aMinX[t]) aMinX[t] = sx;
                    if (sx > aMaxX[t]) aMaxX[t] = sx;
                    if (sy < aMinY[t]) aMinY[t] = sy;
                    if (sy > aMaxY[t]) aMaxY[t] = sy;
                }
                // Routing in WALL2_CollectRoomTilesAndUnits keys on the render-layer bits:
                // no 0x1c000 -> AddRoofTile -> slot 0x10 -> drawn as a SHADOW tile.
                if (t >= 0 && t < 24 && !(*(const unsigned*)(pT + 0x14) & 0x1c000U)) aNoLayer[t]++;
                const unsigned char* pLib = *(const unsigned char**)(pT + 0x18);
                if (pLib && !IsBadReadPtr(pLib, 0x10) && t >= 0 && t < 24) {
                    // Replicate WALL2_CollectRoomTilesAndUnits' three gates exactly.
                    int rawX = *(const int*)(pT + 0x00), rawY = *(const int*)(pT + 0x04);
                    int nH  = *(const int*)(pLib + 0x08);                 // GetRoofHeightIndex
                    int adjY = rawY - (int)(uint16_t)*(const short*)(pLib + 0x04);
                    if (!(rawX <= nCullR && nCullL <= rawX + 0xa0)) {
                        aRejX[t]++;
                    } else if (!(nH + adjY <= nCullB && nCullT <= adjY - nH)) {
                        aRejY[t]++;
                    } else {
                        int32_t ix = 0, iy = 0;
                        D2Common::Drlg::Transform::DRLG_OrthToIsoCoordsDiv160(rawX, rawY, &ix, &iy);
                        if (!D2Client::GAME::Wall2::WALL2_IsInDrawGrid_Extern(ix, iy - 1, pAllocB))
                            aRejG[t]++;
                        else
                            aPass[t]++;
                    }
                }
                if (pLib && !IsBadReadPtr(pLib, 0x10)) {
                    int nh = *(const int*)(pLib + 0x08);   // nHeight: the Y-gate term
                    if (t >= 0 && t < 24) {
                        if (nh < aMinH[t]) aMinH[t] = nh;
                        if (nh > aMaxH[t]) aMaxH[t] = nh;
                    }
                    short rh = *(const short*)(pLib + 0x04);
                    if (t >= 0 && t < 24) {
                        if (rh < aMinR[t]) aMinR[t] = rh;
                        if (rh > aMaxR[t]) aMaxR[t] = rh;
                    }
                    if (rh < 0) { nNegRoof++; if (t >= 0 && t < 24) aNeg[t]++; }
                    if (rh < nMinRoof) nMinRoof = rh;
                    if (rh > nMaxRoof) nMaxRoof = rh;
                }
                nTiles++;
            }
        }
        int u = wsprintfA(rt,
            "{\"rooms\":%d,\"walltiles\":%d,\"cull\":\"L%d T%d R%d B%d\",\"center\":\"%d,%d\","
            "\"camoff\":\"%d,%d\",\"view\":\"%dx%d\",\"type\":{",
            nRooms, nTiles, nCullL, nCullT, nCullR, nCullB,
            pGV ? pGV->nCenterX : -1, pGV ? pGV->nCenterY : -1, nOffX, nOffY,
            pGV ? (pGV->pView.right - pGV->pView.left) : -1,
            pGV ? (pGV->pView.bottom - pGV->pView.top) : -1);
        for (int t = 0; t < 24; t++)
            if (aType[t] && u < (int)sizeof(rt) - 40)
                u += wsprintfA(rt + u, "%s\"%d\":%d", u > 0 && rt[u-1] != '{' ? "," : "", t, aType[t]);
        u += wsprintfA(rt + u, "},\"negRoof\":%d,\"minRoof\":%d,\"maxRoof\":%d,\"negByType\":{",
                       nNegRoof, nMinRoof, nMaxRoof);
        int f2 = 1;
        for (int t = 0; t < 24; t++)
            if (aNeg[t] && u < (int)sizeof(rt) - 40) {
                u += wsprintfA(rt + u, "%s\"%d\":%d", f2 ? "" : ",", t, aNeg[t]); f2 = 0;
            }
        u += wsprintfA(rt + u, "},\"onscreen\":{");
        int f3 = 1;
        for (int t = 0; t < 24; t++)
            if (aType[t] && u < (int)sizeof(rt) - 80) {
                u += wsprintfA(rt + u, "%s\"%d\":\"%d/%d x[%d..%d] y[%d..%d]\"",
                               f3 ? "" : ",", t, aOn[t], aType[t],
                               aMinX[t], aMaxX[t], aMinY[t], aMaxY[t]); f3 = 0;
            }
        u += wsprintfA(rt + u, "},\"roofH\":{");
        int f7 = 1;
        for (int t = 0; t < 24; t++)
            if (aType[t] && u < (int)sizeof(rt) - 60) {
                u += wsprintfA(rt + u, "%s\"%d\":\"%d..%d\"", f7 ? "" : ",", t, aMinR[t], aMaxR[t]); f7 = 0;
            }
        u += wsprintfA(rt + u, "},\"gates\":{");
        int f6 = 1;
        for (int t = 0; t < 24; t++)
            if (aType[t] && u < (int)sizeof(rt) - 90) {
                u += wsprintfA(rt + u, "%s\"%d\":\"pass %d rejX %d rejY %d rejGrid %d\"",
                               f6 ? "" : ",", t, aPass[t], aRejX[t], aRejY[t], aRejG[t]); f6 = 0;
            }
        u += wsprintfA(rt + u, "},\"nHeight\":{");
        int f5 = 1;
        for (int t = 0; t < 24; t++)
            if (aType[t] && u < (int)sizeof(rt) - 60) {
                u += wsprintfA(rt + u, "%s\"%d\":\"%d..%d\"", f5 ? "" : ",", t, aMinH[t], aMaxH[t]); f5 = 0;
            }
        u += wsprintfA(rt + u, "},\"noLayerBits\":{");
        int f4 = 1;
        for (int t = 0; t < 24; t++)
            if (aType[t] && u < (int)sizeof(rt) - 40) {
                u += wsprintfA(rt + u, "%s\"%d\":\"%d/%d\"", f4 ? "" : ",", t, aNoLayer[t], aType[t]); f4 = 0;
            }
        wsprintfA(rt + u, "}}");
        lstrcpynA(reply, rt, replyMax);
        return;
    }
    // Every loaded DS1 in the global cache with its layer counts. Columns, railings and
    // pedestals are orientation 10/11/12/14 tiles and live in the UPPER wall layers, so a
    // DS1 parsed as nWallLayers==1 silently drops all of them while floors still render.
    if (!strncmp(line, "ds1", 3)) {
        static char do_[2048];
        int u = wsprintfA(do_, "{\"files\":[");
        int n = 0, nOneLayer = 0;
        for (D2DrlgFileCacheNodeStrc* pN = D2Common::Drlg::Preset::gpDrlgFileCache;
             pN && n < 40 && !IsBadReadPtr(pN, 0x110); pN = pN->pNext) {
            D2DrlgFileStrc* pF = pN->pDrlgFile;
            if (!pF || IsBadReadPtr(pF, 0x5c)) continue;
            if (pF->nWallLayers <= 1) nOneLayer++;
            if (u < (int)sizeof(do_) - 160) {
                const char* nm = pN->szName;
                const char* base = nm;
                for (const char* q = nm; *q; q++) if (*q == '\\' || *q == '/') base = q + 1;
                u += wsprintfA(do_ + u, "%s{\"f\":\"%s\",\"w\":%d,\"h\":%d,\"wall\":%d,\"floor\":%d}",
                               n ? "," : "", base, pF->nWidth, pF->nHeight,
                               pF->nWallLayers, pF->nFloorLayers);
            }
            n++;
        }
        wsprintfA(do_ + u, "],\"n\":%d,\"singleWallLayer\":%d}", n, nOneLayer);
        lstrcpynA(reply, do_, replyMax);
        return;
    }
    // The blitters index gaPaletteColorLUT by (intensity>>3 & 0x1f)*0x100 to shade a source
    // pixel. Row 0 is light level 0 = black, so an unfilled row paints black geometry. Our
    // wall samples come back at intensity ~140, i.e. row 17.
    if (!strncmp(line, "pal", 3)) {
        static char po[768];
        const uint8_t* base = (const uint8_t*)D2gfx::D2GFX::gaPaletteColorLUT;
        int u = wsprintfA(po, "{\"base\":\"0x%X\",\"rows\":{", (unsigned)(uintptr_t)base);
        int rows[4] = { 0, 1, 17, 31 };
        int nNonZeroRows = 0;
        for (int r = 0; r < 49; r++) {
            const uint8_t* q = base + (size_t)r * 0x100;
            for (int k = 0; k < 256; k++) if (q[k]) { nNonZeroRows++; break; }
        }
        for (int i = 0; i < 4; i++) {
            const uint8_t* q = base + (size_t)rows[i] * 0x100;
            u += wsprintfA(po + u, "%s\"%d\":\"", i ? "," : "", rows[i]);
            for (int k = 0; k < 12; k++) u += wsprintfA(po + u, "%02X", q[k]);
            u += wsprintfA(po + u, "\"");
        }
        wsprintfA(po + u, "},\"nonzeroRows\":%d}", nNonZeroRows);
        lstrcpynA(reply, po, replyMax);
        return;
    }
    // Wall intensity comes only from gaLightMapCells; indoor floors take a flat act ambient
    // and never touch it. An all-zero grid is therefore lit floors and black walls.
    // Direct test of the clip diagnosis: force gnTileClipYMin to the value the original
    // holds (553 = screenHeight - 0x2f) and see whether the missing geometry appears.
    if (!strncmp(line, "setclip", 7)) {
        int v = D2gfx::D2GFX::gnTileClipBottom;
        sscanf(line, "setclip %d", &v);
        int nOld = D2gfx::gnTileClipYMin;
        D2gfx::gnTileClipYMin = v;
        wsprintfA(reply, "{\"was\":%d,\"now\":%d,\"clipBottom\":%d}", nOld, D2gfx::gnTileClipYMin, D2gfx::D2GFX::gnTileClipBottom);
        return;
    }
    // Re-arm the page guard. Each arm catches exactly one write (the handler logs it and
    // un-protects so execution continues), so arming in a loop walks the write sequence.
    // Catch whoever zeroes gnTileClipYMin: make its page read-only so the write faults,
    // and let the existing vectored handler log the faulting EIP. addr2line then names it.
    // Arm DR0 on every thread in this process as a 4-byte write watch on gnTileClipYMin.
    if (!strncmp(line, "hookfilly", 9)) {
        D2GfxHelperStrc* pF = D2GfxRendereInitFunctionsStrc;
        if (!pF) { lstrcpynA(reply, "{\"err\":\"no table\"}", replyMax); return; }
        if (!g_pfnRealFillY) {
            g_pfnRealFillY = (void (__fastcall *)(void*, int, int, int))pF->nfpFillYBufferTable;
            pF->nfpFillYBufferTable = FillYTrampoline;
        }
        wsprintfA(reply, "{\"hooked\":1,\"real\":\"%08X\",\"tramp\":\"%08X\"}",
                  (unsigned)(uintptr_t)g_pfnRealFillY, (unsigned)(uintptr_t)FillYTrampoline);
        return;
    }
    if (!strncmp(line, "hwbp", 4)) {
        uintptr_t nAddr = (uintptr_t)&D2gfx::gnTileClipYMin;
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        THREADENTRY32 te; te.dwSize = sizeof(te);
        DWORD dwPid = GetCurrentProcessId(), dwSelf = GetCurrentThreadId();
        int nSet = 0, nSeen = 0;
        if (hSnap != INVALID_HANDLE_VALUE && Thread32First(hSnap, &te)) {
            do {
                if (te.th32OwnerProcessID != dwPid) continue;
                nSeen++;
                HANDLE hT = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME,
                                       FALSE, te.th32ThreadID);
                if (!hT) continue;
                BOOL bOther = (te.th32ThreadID != dwSelf);
                if (bOther) SuspendThread(hT);
                CONTEXT ctx; memset(&ctx, 0, sizeof(ctx));
                ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
                if (GetThreadContext(hT, &ctx)) {
                    ctx.Dr0 = (DWORD)nAddr;
                    ctx.Dr7 &= ~(DWORD)0x000F0003;   // clear L0/G0 and RW0/LEN0
                    ctx.Dr7 |= 0x1;                  // L0: local enable
                    ctx.Dr7 |= (0x1u << 16);         // RW0 = 01 -> break on write
                    ctx.Dr7 |= (0x3u << 18);         // LEN0 = 11 -> 4 bytes
                    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
                    if (SetThreadContext(hT, &ctx)) nSet++;
                }
                if (bOther) ResumeThread(hT);
                CloseHandle(hT);
            } while (Thread32Next(hSnap, &te));
        }
        if (hSnap != INVALID_HANDLE_VALUE) CloseHandle(hSnap);
        D2TraceFmt("HWBP armed addr=%08X threads=%d/%d", (unsigned)nAddr, nSet, nSeen);
        wsprintfA(reply, "{\"hwbp\":\"0x%X\",\"armed\":%d,\"threads\":%d,\"value\":%d}",
                  (unsigned)nAddr, nSet, nSeen, D2gfx::gnTileClipYMin);
        return;
    }
    if (!strncmp(line, "watch", 5)) {
        unsigned nAddr = 0;
        if (sscanf(line, "watch %x", &nAddr) == 1 && nAddr) g_pHwbpTarget = (void*)(uintptr_t)nAddr;
        EnterCriticalSection(&g_cs);
        int nextW = (g_qTail + 1) % MAXQ;
        if (nextW != g_qHead) {
            Action* aw = &g_queue[g_qTail];
            memset(aw, 0, sizeof(*aw));
            aw->kind = 3; aw->atFrame = (int)g_frames;
            g_qTail = nextW;
        }
        LeaveCriticalSection(&g_cs);
        wsprintfA(reply, "{\"watch\":\"0x%X\",\"value\":\"0x%X\"}",
                  (unsigned)(uintptr_t)g_pHwbpTarget, *(unsigned*)(uintptr_t)g_pHwbpTarget);
        return;
    }
    if (!strncmp(line, "gaddr", 5)) {
        wsprintfA(reply, "{\"cofStatusTable\":\"0x%X\",\"value\":\"0x%X\",\"gameView\":\"0x%X\"}",
                  (unsigned)(uintptr_t)&D2Client::Engine::Gfx::g_pMonsterSpriteCofStatusTable,
                  (unsigned)(uintptr_t)D2Client::Engine::Gfx::g_pMonsterSpriteCofStatusTable,
                  (unsigned)(uintptr_t)D2Client::GAME::Game::D2GameView);
        return;
    }
    if (!strncmp(line, "guard", 5)) {
        void* pAddr = (void*)g_pHwbpTarget;
        SYSTEM_INFO si; GetSystemInfo(&si);
        uintptr_t nPage = (uintptr_t)pAddr & ~(uintptr_t)(si.dwPageSize - 1);
        DWORD dwOld = 0;
        int bOn = 1;
        sscanf(line, "guard %d", &bOn);
        BOOL bOk = VirtualProtect((void*)nPage, si.dwPageSize,
                                  bOn ? PAGE_READONLY : PAGE_READWRITE, &dwOld);
        D2TraceFmt("GUARD %s addr=%08X page=%08X ok=%d old=%X",
                   bOn ? "ON" : "OFF", (unsigned)(uintptr_t)pAddr, (unsigned)nPage, (int)bOk, (unsigned)dwOld);
        wsprintfA(reply, "{\"guard\":%d,\"addr\":\"0x%X\",\"page\":\"0x%X\",\"ok\":%d,\"oldProt\":\"0x%X\"}",
                  bOn, (unsigned)(uintptr_t)pAddr, (unsigned)nPage, (int)bOk, (unsigned)dwOld);
        return;
    }
    if (!strncmp(line, "lmap", 4)) {
        static char lOut[512];
        int nZero = 0, nNonZero = 0, nMax = 0, nColl = 0, nSum = 0;
        for (int i = 0; i < 2304; i++) {
            int v = D2Client::Draw::LightMap::gaLightMapCells[i].nCellIntensity;
            if (v) { nNonZero++; nSum += v; if (v > nMax) nMax = v; } else nZero++;
            if (D2Client::Draw::LightMap::gaLightMapCells[i].nCollisionGrid) nColl++;
        }
        // The active blitter is reached through a function-pointer table the client hands
        // the renderer. A null entry here draws nothing at all, silently.
        char szFns[160];
        {
            D2GfxHelperStrc* pF = D2GfxRendereInitFunctionsStrc;
            if (!pF) lstrcpynA(szFns, "TABLE NULL", sizeof(szFns));
            else wsprintfA(szFns,
                "fillY=%08X visTile=%08X unk=%08X ground=%08X wall=%08X blend=%08X roof=%08X",
                (unsigned)(uintptr_t)pF->nfpFillYBufferTable, (unsigned)(uintptr_t)pF->nfpDrawVisTile,
                (unsigned)(uintptr_t)pF->nfpDrawVisTileLit, (unsigned)(uintptr_t)pF->nfpDrawGroundTile,
                (unsigned)(uintptr_t)pF->nfpDrawWallTile, (unsigned)(uintptr_t)pF->nfpDrawBlendedVisTile,
                (unsigned)(uintptr_t)pF->nfpDrawRoofTile);
        }
        // nClipBottom for every wall blit is RENDERER_nWindowHeight + 0x20. If that global
        // is 0 the walls clip to the top 32 pixels while floors, which do not use this
        // clip, keep drawing - a thin lit band at the top of the view.
        wsprintfA(lOut,
            "{\"zero\":%d,\"nonzero\":%d,\"max\":%d,\"avg\":%d,\"collision\":%d,"
            "\"lastupdframe\":%u,\"recidx\":%d,\"winw\":%d,\"winh\":%d,\"clipbottom\":%d,"
            "\"rendererMode\":%d,\"eRendererMode\":%d,\"gfxFns\":\"%s\","
            "\"clip\":\"minY=%d clipYMin=%d innerXMin=%d innerYMin=%d innerYMinRow=%d\","
            "\"clip2\":\"clipBottom=%d minTileScreenY=%d overlayMode=%d clipZMax=%d innerYMax=%d\"}",
            nZero, nNonZero, nMax, nNonZero ? nSum / nNonZero : 0, nColl,
            (unsigned)D2Client::GAME::View::gnLastLightmapUpdateFrame,
            (int)D2Client::GAME::Game::gnRecordingIndex(),
            RENDERER_nWindowWidth, RENDERER_nWindowHeight,
            RENDERER_nWindowHeight + 0x20,
            (int)D2Win::Src::D2WinMain::D2GFX_eRendererMode, (int)D2gfx::D2GFX::eRendererMode, szFns,
            D2gfx::gnMinTileDrawY, D2gfx::gnTileClipYMin, D2gfx::gnTileInnerClipXMin,
            D2gfx::gnTileInnerClipYMin, D2gfx::gnTileInnerClipYMinRow,
            D2gfx::D2GFX::gnTileClipBottom, D2gfx::gnMinTileScreenY, D2gfx::D2GFX::gnGfxOverlayMode,
            D2gfx::D2GFX::gnTileClipZMax, D2gfx::gnTileInnerClipYMax);
        lstrcpynA(reply, lOut, replyMax);
        return;
    }
    // Click a unit the way d2bs's ClickMap does: convert the unit's world position to
    // client screen coords, select it, and call the game's own ClickMap. No real mouse, no
    // foreground - so the stash and a shop's tabs can be exercised from a script.
    if (!strncmp(line, "clickunit", 9)) {
        int nType = -1, nGuid = -1, nClickType = 0;
        if (sscanf(line, "clickunit %d %d %d", &nType, &nGuid, &nClickType) < 2) {
            lstrcpynA(reply, "{\"err\":\"clickunit <type> <guid> [clicktype]\"}", replyMax); return;
        }
        EnterCriticalSection(&g_cs);
        int next = (g_qTail + 1) % MAXQ;
        if (next != g_qHead) {
            Action* a = &g_queue[g_qTail];
            memset(a, 0, sizeof(*a));
            a->kind = 2; a->atFrame = (int)g_frames;
            a->wp = (WPARAM)nType; a->lp = (LPARAM)nGuid; a->x = nClickType;
            g_qTail = next;
        }
        LeaveCriticalSection(&g_cs);
        lstrcpynA(reply, "{\"ok\":\"clickunit\"}", replyMax); return;
    }
    if (!strncmp(line, "units", 5)) {
        int nWantType = -1;
        sscanf(line, "units %d", &nWantType);
        static char unitsOut[8192];
        UnitsReply(unitsOut, (int)sizeof(unitsOut), nWantType);
        lstrcpynA(reply, unitsOut, replyMax);
        return;
    }
    // The player's own skill list, so a test can name a skill instead of guessing an id.
    // Casting needs the RIGHT skill set, and nothing in the UI layer can be driven blind.
    if (!strncmp(line, "skills", 6)) {
        static char skOut[4096];
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        D2SkillListStrc* pList = pPl ? pPl->pSkillsList : nullptr;
        if (!pList) { lstrcpynA(reply, "{\"err\":\"no skill list\"}", replyMax); return; }
        int u = wsprintfA(skOut, "{\"left\":%d,\"right\":%d,\"skills\":[",
            pList->pLeftSkill ? (int)D2Common::Skills::GetSkillId(pList->pLeftSkill, "", 0) : -1,
            pList->pRightSkill ? (int)D2Common::Skills::GetSkillId(pList->pRightSkill, "", 0) : -1);
        int n = 0;
        for (D2SkillStrc* pS = pList->pFirstSkill; pS && u < (int)sizeof(skOut) - 80; pS = pS->pNext)
            u += wsprintfA(skOut + u, "%s{\"id\":%d,\"lvl\":%d,\"owner\":%d,\"item\":%d}",
                           n++ ? "," : "", (int)D2Common::Skills::GetSkillId(pS, "", 0),
                           pS->nLevel + pS->nLevelBonus, pS->nOwnerGUID, pS->bItemSkill);
        wsprintfA(skOut + u, "],\"n\":%d}", n);
        lstrcpynA(reply, skOut, replyMax);
        return;
    }
    // Why a cast did nothing. The skill path returns silently from four different places,
    // so the only way to tell "not enough mana" from "the packet never went out" is to ask
    // the game's own usability check and print its code beside the player's resources.
    if (!strncmp(line, "why", 3)) {
        static char whyOut[512];
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        D2SkillListStrc* pList = pPl ? pPl->pSkillsList : nullptr;
        if (!pList) { lstrcpynA(reply, "{\"err\":\"no player\"}", replyMax); return; }
        D2SkillStrc* pRight = pList->pRightSkill;
        D2SkillStrc* pLeft = pList->pLeftSkill;
        int nCodeR = 0, nCodeL = 0;
        int bUsableR = pRight ? (int)D2Client::UNIT::Player::PLAYER_CheckSkillUsability_Extern(&nCodeR, pPl, &pRight) : -1;
        int bUsableL = pLeft  ? (int)D2Client::UNIT::Player::PLAYER_CheckSkillUsability_Extern(&nCodeL, pPl, &pLeft)  : -1;
        D2RoomStrc* pRoom = D2Common::Units::UNITS_GetRoom(pPl);
        wsprintfA(whyOut,
            "{\"life\":%d,\"mana\":%d,\"maxmana\":%d,\"intown\":%d,"
            "\"right\":{\"usable\":%d,\"code\":%d,\"can\":%d},"
            "\"left\":{\"usable\":%d,\"code\":%d,\"can\":%d},"
            "\"mouse\":[%d,%d],\"panelY\":%d}",
            (int)D2Common::Stats::StatsEx::STATSEX_GetCurrentStat(pPl, UNITSTAT_hitpoints, 0) >> 8,
            (int)D2Common::Stats::StatsEx::STATSEX_GetCurrentStat(pPl, UNITSTAT_mana, 0) >> 8,
            (int)D2Common::Stats::StatsEx::STATSEX_GetCurrentStat(pPl, UNITSTAT_maxmana, 0) >> 8,
            (int)D2Common::DUNGEON::Dungeon::DRLGROOM_IsRoomInTown(pRoom),
            bUsableR, nCodeR,
            pRight ? (int)D2Client::UNIT::PlrSkills::PLRSKILLS_CanUseSkillCheck(pPl, pRight) : -1,
            bUsableL, nCodeL,
            pLeft ? (int)D2Client::UNIT::PlrSkills::PLRSKILLS_CanUseSkillCheck(pPl, pLeft) : -1,
            (int)D2Client::Cursor::GetMouseX(), (int)D2Client::Cursor::GetMouseY(),
            (int)D2Client::UI::Draw::DRAW_GetBottomPanelY());
        lstrcpynA(reply, whyOut, replyMax);
        return;
    }
    // cof: the player's composite as the COF loader sees it. Every animation load asks for a
    // path whose weapon-class component is empty or a bare 1, and the table it is read from is
    // correct in memory - so the INDEX, or the struct the index is read from, is what is wrong.
    if (!strncmp(line, "cof", 3) && line[3] != 'n') {
        static char cfOut[900];
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        if (!pPl) { lstrcpynA(reply, "{\"err\":\"no player\"}", replyMax); return; }
        void* apCand[3] = { (void*)pPl->pAnimData, (void*)pPl->pGfxInfo, (void*)pPl->pGfxDataCopy };
        const char* aszName[3] = { "pAnimData", "pGfxInfo", "pGfxDataCopy" };
        int u = wsprintfA(cfOut, "{\"unit\":\"%08X\",\"cand\":[", (unsigned)(uintptr_t)pPl);
        for (int i = 0; i < 3; i++) {
            char* p = (char*)apCand[i];
            int ok = p && !IsBadReadPtr(p, 0xc0);
            u += wsprintfA(cfOut + u, "%s{\"n\":\"%s\",\"p\":\"%08X\"", i ? "," : "",
                           aszName[i], (unsigned)(uintptr_t)p);
            if (ok) {
                int cls = *(int*)(p + 0x1c), mode = *(int*)(p + 0x20), wc = *(int*)(p + 0x24);
                u += wsprintfA(cfOut + u, ",\"cls\":%d,\"mode\":%d,\"wc\":%d,\"cof\":\"%08X\",\"tok\":\"%08X\"",
                               cls, mode, wc, (unsigned)*(uint32_t*)(p + 0xb8),
                               (wc >= 0 && wc < 15) ? (unsigned)D2Win::Src::D2Comp::gaCompWeaponClassTokens[wc] : 0xDEADBEEF);
            }
            u += wsprintfA(cfOut + u, "}");
        }
        u += wsprintfA(cfOut + u, "],\"tbl1\":\"%08X\",\"tbladdr\":\"%08X\"}",
                       (unsigned)D2Win::Src::D2Comp::gaCompWeaponClassTokens[1],
                       (unsigned)(uintptr_t)&D2Win::Src::D2Comp::gaCompWeaponClassTokens[0]);
        lstrcpynA(reply, cfOut, replyMax);
        return;
    }
    // cofname <mode>: the COF token the animation lookup actually builds for the player, and
    // the weapon class it resolved. A blank AnimData record (name all-zero, dwFrames garbage)
    // is what leaves a cast mode running for ~48 s, and the record is keyed on this name.
    if (!strncmp(line, "cofname", 7)) {
        int nMode = 10;
        sscanf(line, "cofname %d", &nMode);
        static char cnOut[700];
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        if (!pPl) { lstrcpynA(reply, "{\"err\":\"no player\"}", replyMax); return; }
        char szCof[16]; memset(szCof, 0, sizeof(szCof));
        int32_t nWc = 0;
        D2Common::COMPOSIT::Composit::COMPOSIT_BuildCofPath(
            pPl, (uint32_t)pPl->nClassId, (uint32_t)nMode, 0, pPl->pInventory,
            szCof, &nWc, 0, 1);
        int32_t nScratch = 0;
        uint32_t nRes = D2Common::COMPOSIT::Composit::COMPOSIT_ResolveWeaponClass(
            pPl, 0, pPl->nClassId, nMode, pPl->pInventory, &nScratch);
        int u = wsprintfA(cnOut, "{\"mode\":%d,\"cls\":%d,\"inv\":\"%08X\",\"cof\":\"",
                          nMode, (int)pPl->nClassId, (unsigned)(uintptr_t)pPl->pInventory);
        for (int i = 0; i < 12 && szCof[i]; i++)
            u += wsprintfA(cnOut + u, "%02X", (unsigned char)szCof[i]);
        D2UnitStrc* pRight = pPl->pInventory
            ? D2Common::INVENTORY::Inventory::INVENTORY_GetItemByGridOffset(pPl->pInventory, 4) : nullptr;
        D2UnitStrc* pLeft = pPl->pInventory
            ? D2Common::INVENTORY::Inventory::INVENTORY_GetItemByGridOffset(pPl->pInventory, 5) : nullptr;
        wsprintfA(cnOut + u, "\",\"wc\":\"%08X\",\"resolve\":\"%08X\",\"scratch\":\"%08X\","
                  "\"r\":\"%08X\",\"rcomp\":%d,\"rwcl\":\"%08X\",\"rwcl2h\":\"%08X\","
                  "\"l\":\"%08X\",\"lcomp\":%d,\"hand\":%d}",
                  (unsigned)nWc, (unsigned)nRes, (unsigned)nScratch,
                  (unsigned)(uintptr_t)pRight,
                  pRight ? (int)D2Common::ITEMS::Items::ITEM_GetItemsTxt_bComponent(pRight) : -1,
                  pRight ? (unsigned)D2Common::ITEMS::Items::ITEM_GetItemsTxt_dwWClass(pRight) : 0,
                  pRight ? (unsigned)D2Common::ITEMS::Items::ITEM_GetItemsTxt_dw2HandedWClass(pRight) : 0,
                  (unsigned)(uintptr_t)pLeft,
                  pLeft ? (int)D2Common::ITEMS::Items::ITEM_GetItemsTxt_bComponent(pLeft) : -1,
                  pPl->pInventory ? (int)D2Common::INVENTORY::Inventory::INVENTORY_GetWeaponHandedness(pPl, pPl->pInventory) : -1);
        lstrcpynA(reply, cnOut, replyMax);
        return;
    }
    // The SERVER unit's own view of the skill. A cast that the client accepts and the server
    // drops looks identical from the UI, and SCMD_0x0C returns BAD_PARAMS silently when the
    // server-side player has no right skill selected.
    if (!strncmp(line, "srv", 3) && line[3] != 'c') {
        static char svOut[640];
        D2UnitStrc* pSrv = nullptr;
        D2UnitStrc** ppTable = (D2UnitStrc**)((char*)D2Client::UNIT::CUnit::ServerSideUnitHashTables + UNIT_PLAYER * 0x200);
        for (int nBucket = 0; nBucket < 128 && !pSrv; nBucket++)
            for (D2UnitStrc* pU = ppTable[nBucket]; pU; pU = pU->pListNext) { pSrv = pU; break; }
        if (!pSrv) { lstrcpynA(reply, "{\"err\":\"no server player\"}", replyMax); return; }
        D2SkillStrc* pR = D2Common::Units::GetRightSkill(pSrv);
        D2SkillStrc* pL = D2Common::Units::GetLeftSkill(pSrv);
        wsprintfA(svOut,
            "{\"unit\":\"%08X\",\"mode\":%d,"
            "\"right\":{\"id\":%d,\"canuse\":%d,\"skillmode\":%d,\"txt\":%d},"
            "\"left\":{\"id\":%d,\"canuse\":%d,\"skillmode\":%d,\"txt\":%d}}",
            (unsigned)(uintptr_t)pSrv, (int)pSrv->eAnimMode.ePlayerMode,
            pR ? (int)D2Common::Skills::GetSkillId(pR, "", 0) : -1,
            pR ? (int)D2Common::Skills::SKILLS_CanUseSkill(pSrv, pR) : -1,
            pR ? (int)D2Common::Skills::GetSkillMode(pSrv, pR) : -1,
            pR ? (D2Common::Skills::GetSkillsTxt(pR) ? 1 : 0) : -1,
            pL ? (int)D2Common::Skills::GetSkillId(pL, "", 0) : -1,
            pL ? (int)D2Common::Skills::SKILLS_CanUseSkill(pSrv, pL) : -1,
            pL ? (int)D2Common::Skills::GetSkillMode(pSrv, pL) : -1,
            pL ? (D2Common::Skills::GetSkillsTxt(pL) ? 1 : 0) : -1);
        lstrcpynA(reply, svOut, replyMax);
        return;
    }
    // srvcast <x> <y>: the SERVER's own cast, called directly with world coords, with every
    // gate between the packet handler and the mode dispatcher reported. The handler returns
    // a status nobody sees, so this is the only way to learn which gate closed.
    if (!strncmp(line, "srvcast ", 8)) {
        int wx = 0, wy = 0;
        if (sscanf(line, "srvcast %d %d", &wx, &wy) != 2) {
            lstrcpynA(reply, "{\"err\":\"srvcast <x> <y>\"}", replyMax); return;
        }
        static char scOut[640];
        D2UnitStrc* pSrv = nullptr;
        D2UnitStrc** ppTable = (D2UnitStrc**)((char*)D2Client::UNIT::CUnit::ServerSideUnitHashTables + UNIT_PLAYER * 0x200);
        for (int nBucket = 0; nBucket < 128 && !pSrv; nBucket++)
            for (D2UnitStrc* pU = ppTable[nBucket]; pU; pU = pU->pListNext) { pSrv = pU; break; }
        ::SGAMEDATA* pGame = D2Game::GAME::Server::QSERVER_GetFirstActiveGame();
        if (!pSrv || !pGame) { lstrcpynA(reply, "{\"err\":\"no server unit or game\"}", replyMax); return; }
        D2SkillStrc* pR = D2Common::Units::GetRightSkill(pSrv);
        int nMode = pR ? (int)D2Common::Skills::GetSkillMode(pSrv, pR) : -1;
        int nCanInt = (int)D2Game::PLAYER::PlayerModes::PLRMODES_CanInterruptCurrentMode_Extern(pSrv, pGame, (uint32_t)nMode);
        wsprintfA(scOut,
            "{\"game\":\"%08X\",\"unit\":\"%08X\",\"skillmode\":%d,\"caninterrupt\":%d,"
            "\"cancel\":%d,\"cast\":%d,\"before\":[%d,%d]}",
            (unsigned)(uintptr_t)pGame, (unsigned)(uintptr_t)pSrv, nMode, nCanInt,
            (int)D2Game::PLAYER::PlayerModes::PLRMODES_TryCancelModeAndMoveToLocation(pGame, pSrv, nMode, 1, pR),
            pR ? (int)D2Game::PLAYER::PlayerMsg::CastSkillOnLocation_Extern(pSrv, pGame, wx, wy, pR) : -1,
            (int)(pSrv->pPath.pDynamicPath ? pSrv->pPath.pDynamicPath->Coords.tSubtile.nPosX : -1),
            (int)(pSrv->pPath.pDynamicPath ? pSrv->pPath.pDynamicPath->Coords.tSubtile.nPosY : -1));
        lstrcpynA(reply, scOut, replyMax);
        return;
    }
    // castdbg <hand> <screenx> <screeny>: every gate on the cast path, reported, then the
    // action itself. The path returns silently from four places, so a cast that does nothing
    // is otherwise indistinguishable from a cast that was never attempted.
    if (!strncmp(line, "castdbg ", 8)) {
        int nHand = 1, sx = 400, sy = 200;
        if (sscanf(line, "castdbg %d %d %d", &nHand, &sx, &sy) != 3) {
            lstrcpynA(reply, "{\"err\":\"castdbg <0left|1right> <sx> <sy>\"}", replyMax); return;
        }
        static char cdOut[1024];
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        if (!pPl) { lstrcpynA(reply, "{\"err\":\"no player\"}", replyMax); return; }
        int wx = sx, wy = sy;
        D2Client::UI::Path::ConvertCoordToPerspectiveView(&wx, &wy);
        D2SkillStrc* pSk = nHand ? D2Common::Units::GetRightSkill(pPl)
                                 : D2Common::Units::GetLeftSkill(pPl);
        wsprintfA(cdOut,
            "{\"screen\":[%d,%d],\"world\":[%d,%d],\"self\":[%d,%d],"
            "\"caninterrupt\":%d,\"skill\":%08X,\"id\":%d,\"owner\":%d}",
            sx, sy, wx, wy,
            (int)D2Common::Units::GetTargetX(pPl),
            (int)D2Common::Units::GetTargetY(pPl),
            (int)D2Client::UNIT::UnitMode::UNITMODE_CanInterruptSkill_Extern(pPl),
            (unsigned)(uintptr_t)pSk,
            pSk ? (int)D2Common::Skills::GetSkillId(pSk, "", 0) : -1,
            pSk ? (int)D2Common::Skills::GetOwnerGUID(pSk) : -1);
        // The packet only goes out if the CLIENT-side cast returns non-zero
        // (UNITMODE_ExecuteSkillAction gates ClickWithSkillOnUnitOrCoord on HandleMessage's
        // result), so a silent cast is decided here, five frames below the click.
        if (pSk) {
            D2ServerMessageHandleStrc msg;
            memset(&msg, 0, sizeof(msg));
            msg.nSkillId = (uint32_t)D2Common::Skills::GetSkillId(pSk, "", 0);
            msg.nItemId = (uint32_t)D2Common::Skills::GetOwnerGUID(pSk);
            msg.nTargetUnitType = (uint32_t)wx;
            msg.nTargetUnitId = (uint32_t)wy;
            int nCode = 0;
            int nValid = (int)D2Client::UNIT::Player::PLAYER_ValidateSkillMessage_Extern(&nCode, pPl, &msg);
            int nCast  = (int)D2Client::SKILLS::Skills::SKILLS_CastSkillClient_Extern(
                                  (eD2Skills)msg.nSkillId, pPl, (int32_t)msg.nItemId, msg.nSkillLvl);
            int u = (int)strlen(cdOut);
            wsprintfA(cdOut + u - 1, ",\"validate\":%d,\"vcode\":%d,\"castclient\":%d,"
                                     "\"los\":%d,\"txtid\":%d}",
                      nValid, nCode, nCast,
                      (int)(D2Common::ITEMS::Items::sgptDataTable && D2Common::ITEMS::Items::sgptDataTable->pTxtSkills
                            && (int)msg.nSkillId < D2Common::ITEMS::Items::sgptDataTable->nTxtSkillsCount
                            ? D2Common::ITEMS::Items::sgptDataTable->pTxtSkills[msg.nSkillId].lineofsight : -1),
                      (int)msg.nSkillId);
        }
        lstrcpynA(reply, cdOut, replyMax);
        // Queue the action itself so it runs on the game thread, bypassing the whole UI layer.
        EnterCriticalSection(&g_cs);
        int next = (g_qTail + 1) % MAXQ;
        if (next != g_qHead) {
            Action* a = &g_queue[g_qTail];
            memset(a, 0, sizeof(*a));
            a->kind = 6; a->atFrame = (int)g_frames;
            a->wp = (WPARAM)(nHand ? 12 : 5); a->x = wx; a->y = wy;
            g_qTail = next;
        }
        LeaveCriticalSection(&g_cs);
        return;
    }
    // clickmap <type> <x> <y>: the game's own ClickMap at a GROUND position in client space.
    // type 0 left click, 3 right click - which is how a skill is cast at a point. `click`
    // only posts window messages, and the in-game left button is polled rather than handled,
    // so a cast could never be driven from a test at all.
    if (!strncmp(line, "clickmap ", 9)) {
        int nType = 0, cx = 0, cy = 0;
        if (sscanf(line, "clickmap %d %d %d", &nType, &cx, &cy) != 3) {
            lstrcpynA(reply, "{\"err\":\"clickmap <type> <x> <y>\"}", replyMax); return;
        }
        EnterCriticalSection(&g_cs);
        int next = (g_qTail + 1) % MAXQ;
        if (next != g_qHead) {
            Action* a = &g_queue[g_qTail];
            memset(a, 0, sizeof(*a));
            a->kind = 4; a->atFrame = (int)g_frames;
            a->wp = (WPARAM)nType; a->x = cx; a->y = cy;
            g_qTail = next;
        }
        LeaveCriticalSection(&g_cs);
        lstrcpynA(reply, "{\"ok\":\"clickmap\"}", replyMax); return;
    }
    // cpkt <hex>: one raw client->server packet through the game's own sender, so a test can
    // reach an opcode the panel layer cannot.
    if (!strncmp(line, "cpkt ", 5)) {
        Action tmp; memset(&tmp, 0, sizeof(tmp));
        const char* p = line + 5;
        int nLen = 0;
        while (p[0] && p[1] && nLen < 255) {
            int hi = -1, lo = -1;
            for (int k = 0; k < 2; k++) {
                char c = p[k];
                int v = (c >= '0' && c <= '9') ? c - '0'
                      : (c >= 'a' && c <= 'f') ? c - 'a' + 10
                      : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
                if (k == 0) hi = v; else lo = v;
            }
            if (hi < 0 || lo < 0) break;
            tmp.path[nLen++] = (char)((hi << 4) | lo);
            p += 2;
            while (*p == ' ') p++;
        }
        if (!nLen) { lstrcpynA(reply, "{\"err\":\"cpkt <hexbytes>\"}", replyMax); return; }
        EnterCriticalSection(&g_cs);
        int next = (g_qTail + 1) % MAXQ;
        if (next != g_qHead) {
            Action* a = &g_queue[g_qTail];
            memcpy(a, &tmp, sizeof(tmp));
            a->kind = 5; a->atFrame = (int)g_frames; a->x = nLen;
            g_qTail = next;
        }
        LeaveCriticalSection(&g_cs);
        wsprintfA(reply, "{\"ok\":\"cpkt\",\"len\":%d}", nLen);
        return;
    }
    int x = 0, y = 0, vk = 0;
    if (sscanf(line, "click %d %d", &x, &y) == 2) {
        LPARAM lp = (LPARAM)((y << 16) | (x & 0xFFFF));
        Enqueue(WM_MOUSEMOVE,   0, lp, 0, 1, x, y);
        Enqueue(WM_LBUTTONDOWN, 1, lp, 1, 1, x, y);
        Enqueue(WM_LBUTTONUP,   0, lp, 4, 1, x, y);
        lstrcpynA(reply, "{\"ok\":\"click\"}", replyMax); return;
    }
    // A click is DOWN then UP three frames later. The in-game left button is not an event
    // the game acts on directly - INPUT_FormMouseHandler only sets gnWinMainState, and the
    // game loop polls that - so a 3-frame press can be missed entirely. `down`/`up` let a
    // test hold the button across as many frames as it takes.
    if (sscanf(line, "down %d %d", &x, &y) == 2) {
        LPARAM lp = (LPARAM)((y << 16) | (x & 0xFFFF));
        Enqueue(WM_MOUSEMOVE,   0, lp, 0, 1, x, y);
        Enqueue(WM_LBUTTONDOWN, 1, lp, 1, 1, x, y);
        lstrcpynA(reply, "{\"ok\":\"down\"}", replyMax); return;
    }
    if (sscanf(line, "up %d %d", &x, &y) == 2) {
        LPARAM lp = (LPARAM)((y << 16) | (x & 0xFFFF));
        Enqueue(WM_LBUTTONUP, 0, lp, 0, 1, x, y);
        lstrcpynA(reply, "{\"ok\":\"up\"}", replyMax); return;
    }
    if (sscanf(line, "move %d %d", &x, &y) == 2) {
        LPARAM lp = (LPARAM)((y << 16) | (x & 0xFFFF));
        Enqueue(WM_MOUSEMOVE, 0, lp, 0, 1, x, y);
        lstrcpynA(reply, "{\"ok\":\"move\"}", replyMax); return;
    }
    if (sscanf(line, "key %d", &vk) == 1) {
        Enqueue(WM_KEYDOWN, (WPARAM)vk, 1, 0, 0, 0, 0);
        Enqueue(WM_KEYUP,   (WPARAM)vk, 1, 2, 0, 0, 0);
        lstrcpynA(reply, "{\"ok\":\"key\"}", replyMax); return;
    }
    if (!strncmp(line, "shot ", 5)) {
        EnterCriticalSection(&g_cs);
        int next = (g_qTail + 1) % MAXQ;
        if (next != g_qHead) {
            Action* a = &g_queue[g_qTail];
            memset(a, 0, sizeof(*a));
            a->kind = 1; a->atFrame = (int)g_frames;
            lstrcpynA(a->path, line + 5, (int)sizeof(a->path));
            g_qTail = next;
        }
        LeaveCriticalSection(&g_cs);
        lstrcpynA(reply, "{\"ok\":\"shot\"}", replyMax); return;
    }
    if (!strncmp(line, "text ", 5)) {
        char* p = line + 5; int d = 0;
        for (; *p; ++p, ++d) Enqueue(WM_CHAR, (WPARAM)(unsigned char)*p, 1, d, 0, 0, 0);
        lstrcpynA(reply, "{\"ok\":\"text\"}", replyMax); return;
    } else if (!strncmp(line, "enter ", 6)) {
        char* p = line + 6;
        while (*p == ' ') ++p;
        lstrcpynA(g_enterName, p, sizeof(g_enterName));
        g_enterNote[0] = 0;
        g_enterWait = 0;
        InterlockedExchange(&g_enterStep, 0);
        wsprintfA(reply, "{\"ok\":\"enter\",\"name\":\"%s\"}", g_enterName);
        return;
    }
    lstrcpynA(reply, "{\"err\":\"unknown\"}", replyMax);
}

// The game calls WSACleanup on its own schedule - leaving a lobby tears the whole
// Winsock refcount down, and when it reaches zero every socket in the process is
// closed, this listener included. Extra startups keep the count off the floor so
// the control channel survives the transition into the game.
static void HoldWinsockOpen(void) {
    WSADATA wsa;
    for (int i = 0; i < 16; i++) WSAStartup(MAKEWORD(2, 2), &wsa);
}

static SOCKET OpenListener(void) {
    SOCKET srv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (srv == INVALID_SOCKET) return INVALID_SOCKET;
    BOOL yes = TRUE;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
    struct sockaddr_in a; memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET; a.sin_port = htons(D2CONTROL_PORT);
    memset(&a.sin_addr, 0, sizeof(a.sin_addr));   // INADDR_ANY; the tree ships its own in_addr shim
    if (bind(srv, (struct sockaddr*)&a, sizeof(a)) != 0 || listen(srv, 4) != 0) {
        D2TraceFmt("D2CONTROL: bind/listen failed err=%d", (int)WSAGetLastError());
        closesocket(srv); return INVALID_SOCKET;
    }
    return srv;
}

// Logs the first write to the guarded page, then un-protects and lets it proceed, so a
// caller can re-arm and capture the next one. Registered FIRST so it sees the fault before
// the tree's own crash handler.
static volatile LONG g_bGuardRearm = 0;
static volatile LONG g_bGuardArmed = 0;

// Arms the write-watch from an arbitrary point in the run. Blind arming covers the wrong
// instant; arming at a known-good trace point bounds the window to the code that follows it.
// g_bGuardRearm is one global flag, so two threads faulting at once lose a re-protect and the
// page stays writable for the rest of the run - which reads exactly like "nothing wrote it".
// Re-applying the protection every frame bounds that hole to a single frame.
extern "C" void D2ProbeRearmClipGuard(void) {
    if (!g_bGuardArmed) return;
    SYSTEM_INFO si; GetSystemInfo(&si);
    uintptr_t nPage = (uintptr_t)g_pHwbpTarget & ~(uintptr_t)(si.dwPageSize - 1);
    DWORD dwOld = 0;
    VirtualProtect((void*)nPage, si.dwPageSize, PAGE_READONLY, &dwOld);
}

extern "C" void D2ProbeArmClipGuard(void) {
    if (g_bGuardArmed) return;
    SYSTEM_INFO si; GetSystemInfo(&si);
    uintptr_t nPage = (uintptr_t)g_pHwbpTarget & ~(uintptr_t)(si.dwPageSize - 1);
    DWORD dwOld = 0;
    if (VirtualProtect((void*)nPage, si.dwPageSize, PAGE_READONLY, &dwOld)) {
        g_bGuardArmed = 1;
        D2TraceFmt("GUARDARM page=%08X target=%08X value=%08X",
                   (unsigned)nPage, (unsigned)(uintptr_t)g_pHwbpTarget, *(unsigned*)(uintptr_t)g_pHwbpTarget);
    }
}



static LONG CALLBACK GuardVeh(EXCEPTION_POINTERS* pEx) {
    if (pEx->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;
    if (pEx->ExceptionRecord->NumberParameters < 2) return EXCEPTION_CONTINUE_SEARCH;
    if (pEx->ExceptionRecord->ExceptionInformation[0] != 1) return EXCEPTION_CONTINUE_SEARCH;  // writes only
    uintptr_t nHit = (uintptr_t)pEx->ExceptionRecord->ExceptionInformation[1];
    uintptr_t nWant = (uintptr_t)g_pHwbpTarget;
    SYSTEM_INFO si; GetSystemInfo(&si);
    uintptr_t nPage = nWant & ~(uintptr_t)(si.dwPageSize - 1);
    if ((nHit & ~(uintptr_t)(si.dwPageSize - 1)) != nPage) return EXCEPTION_CONTINUE_SEARCH;
    DWORD dwOld = 0;
    VirtualProtect((void*)nPage, si.dwPageSize, PAGE_READWRITE, &dwOld);
    if (nHit == nWant) {
        D2TraceFmt("GUARDHIT *** TARGET *** addr=%08X eip=%08X value=%08X",
                   (unsigned)nHit, (unsigned)(uintptr_t)pEx->ContextRecord->Eip,
                   *(unsigned*)(uintptr_t)g_pHwbpTarget);
    } else {
        static LONG nOther = 0;
        LONG n = InterlockedIncrement(&nOther);
        if (n <= 40)
            D2TraceFmt("GUARDOTHER addr=%08X eip=%08X (n=%d)", (unsigned)nHit,
                       (unsigned)(uintptr_t)pEx->ContextRecord->Eip, (int)n);
    }
    // Single-step over the faulting store, then re-protect on the trap so the watch is
    // continuous instead of one-shot.
    pEx->ContextRecord->EFlags |= 0x100;
    g_bGuardRearm = 1;
    return EXCEPTION_CONTINUE_EXECUTION;
}

// Hardware write-watch on gnTileClipYMin via DR0. A debug-register watchpoint does not
// perturb timing the way page protection does, and Dr6 bit 0 identifies our breakpoint.
#define D2_ARM_HWBP_EXCEPTION 0x2D324857u

// The address the hardware write-watch covers. Settable from the control channel
// (`watch <hex>`) so a new "who zeroes this global?" question does not need a rebuild.
volatile void* g_pHwbpTarget = (void*)&D2gfx::gnTileClipYMin;

static LONG CALLBACK HwbpVeh(EXCEPTION_POINTERS* pEx) {
    // Debug registers cannot be installed on a running thread with SetThreadContext, but a
    // context patched inside a handler IS applied on continue. So arming is done by raising
    // a private exception from the thread that must carry the watch.
    if (pEx->ExceptionRecord->ExceptionCode == D2_ARM_HWBP_EXCEPTION) {
        uintptr_t nAddr = (uintptr_t)g_pHwbpTarget;
        pEx->ContextRecord->Dr0 = (DWORD)nAddr;
        pEx->ContextRecord->Dr6 = 0;
        // L0 enabled, RW0 = write-only (01), LEN0 = 4 bytes (11).
        pEx->ContextRecord->Dr7 &= ~(DWORD)0x000F0003;
        pEx->ContextRecord->Dr7 |= 0x1u | (0x1u << 16) | (0x3u << 18);
        pEx->ContextRecord->ContextFlags |= CONTEXT_DEBUG_REGISTERS;
        D2TraceFmt("HWBPARM target=%08X value=%08X", (unsigned)nAddr, *(unsigned*)nAddr);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (pEx->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP) return EXCEPTION_CONTINUE_SEARCH;
    if (!(pEx->ContextRecord->Dr6 & 0x1)) return EXCEPTION_CONTINUE_SEARCH;
    D2TraceFmt("HWBP WRITE target=%08X eip=%08X value=%08X ret0=%08X",
               (unsigned)(uintptr_t)g_pHwbpTarget,
               (unsigned)(uintptr_t)pEx->ContextRecord->Eip,
               *(unsigned*)(uintptr_t)g_pHwbpTarget,
               (unsigned)*(DWORD*)(uintptr_t)pEx->ContextRecord->Esp);
    // The writer is often a CRT memcpy or user32 SetRect, so eip alone names nothing.
    // Walk the stack for the first few words that land inside a module's image and
    // print them as module+offset: the first one inside d2recon is the caller.
    {
        const DWORD* sp = (const DWORD*)(uintptr_t)pEx->ContextRecord->Esp;
        int nFound = 0;
        for (int i = 0; i < 256 && nFound < 6; i++) {
            DWORD v = 0;
            if (IsBadReadPtr(sp + i, 4)) break;
            v = sp[i];
            HMODULE h = 0;
            if (v < 0x10000 || !GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)(uintptr_t)v, &h) || !h) continue;
            char szMod[MAX_PATH]; szMod[0] = 0;
            GetModuleFileNameA(h, szMod, sizeof(szMod));
            const char* base = strrchr(szMod, '\\'); base = base ? base + 1 : szMod;
            D2TraceFmt("HWBP   frame[%d] esp+%03X %s+0x%X", nFound, i * 4, base, (unsigned)(v - (uintptr_t)h));
            nFound++;
        }
    }
    pEx->ContextRecord->Dr6 = 0;
    return EXCEPTION_CONTINUE_EXECUTION;
}

// Arms the calling thread. Polling for new threads always leaves a window in which a thread
// created during world entry can run before it is covered; arming from the thread's own first
// instruction leaves none.
extern "C" void D2ProbeArmClipHwbpSelf(void) {
    RaiseException(D2_ARM_HWBP_EXCEPTION, 0, 0, 0);
}

extern "C" void D2ProbeArmClipHwbp(void) {
    static volatile LONG bDone = 0;
    if (InterlockedExchange(&bDone, 1)) return;
    RaiseException(D2_ARM_HWBP_EXCEPTION, 0, 0, 0);
    // Control experiment: a store this thread is known to make. If this one does not trap,
    // the watch is not installed and any silence from it proves nothing about the real writer.
    int nSaved = D2gfx::gnTileClipYMin;
    *(volatile int*)&D2gfx::gnTileClipYMin = 12345;
    *(volatile int*)&D2gfx::gnTileClipYMin = nSaved;
    D2TraceFmt("HWBPSELFTEST done value=%d", D2gfx::gnTileClipYMin);
}

// Debug registers are per-thread, so a watch on the observing thread proves nothing about a
// writer on another one. Threads are also created during world entry, so this is re-run
// periodically and skips the ones already carrying the watch.
static DWORD WINAPI ClipArmerThread(LPVOID);

static HANDLE g_ahWatched[256];
static volatile LONG g_nWatched = 0;

extern "C" void D2ProbeArmClipHwbpAllThreads(void) {
    static DWORD aArmed[256];
    static int nArmed = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;
    THREADENTRY32 te; te.dwSize = sizeof(te);
    DWORD dwPid = GetCurrentProcessId(), dwSelf = GetCurrentThreadId();
    uintptr_t nAddr = (uintptr_t)&D2gfx::gnTileClipYMin;
    if (Thread32First(hSnap, &te)) {
        do {
            if (te.th32OwnerProcessID != dwPid) continue;
            if (te.th32ThreadID == dwSelf) continue;
            int bSeen = 0;
            for (int i = 0; i < nArmed; i++) if (aArmed[i] == te.th32ThreadID) { bSeen = 1; break; }
            if (bSeen || nArmed >= 256) continue;
            HANDLE hT = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,
                                   FALSE, te.th32ThreadID);
            if (!hT) continue;
            if (SuspendThread(hT) != (DWORD)-1) {
                CONTEXT ctx; ZeroMemory(&ctx, sizeof(ctx));
                ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
                if (GetThreadContext(hT, &ctx)) {
                    ctx.Dr0 = (DWORD)nAddr;
                    ctx.Dr6 = 0;
                    ctx.Dr7 &= ~(DWORD)0x000F0003;
                    ctx.Dr7 |= 0x1u | (0x1u << 16) | (0x3u << 18);
                    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
                    if (SetThreadContext(hT, &ctx)) {
                        aArmed[nArmed++] = te.th32ThreadID;
                        // Kept open so the freeze path can suspend without a snapshot, which
                        // is what let the writer escape into a wait last time.
                        LONG n = g_nWatched;
                        if (n < 256 && DuplicateHandle(GetCurrentProcess(), hT, GetCurrentProcess(),
                                                       &g_ahWatched[n], 0, FALSE,
                                                       DUPLICATE_SAME_ACCESS)) {
                            g_nWatched = n + 1;
                        }
                        D2TraceFmt("HWBPARM thread=%u target=%08X", (unsigned)te.th32ThreadID,
                                   (unsigned)nAddr);
                    }
                }
                ResumeThread(hT);
            }
            CloseHandle(hT);
        } while (Thread32Next(hSnap, &te));
    }
    CloseHandle(hSnap);
}

static LONG CALLBACK GuardStepVeh(EXCEPTION_POINTERS* pEx) {
    if (pEx->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP) return EXCEPTION_CONTINUE_SEARCH;
    if (!g_bGuardRearm) return EXCEPTION_CONTINUE_SEARCH;
    g_bGuardRearm = 0;
    {
        static LONG nStep = 0;
        LONG n = InterlockedIncrement(&nStep);
        if (n <= 5) D2TraceFmt("GUARDSTEP reprotect n=%d", (int)n);
    }
    if (!g_bGuardArmed) return EXCEPTION_CONTINUE_EXECUTION;
    SYSTEM_INFO si; GetSystemInfo(&si);
    uintptr_t nPage = (uintptr_t)g_pHwbpTarget & ~(uintptr_t)(si.dwPageSize - 1);
    DWORD dwOld = 0;
    VirtualProtect((void*)nPage, si.dwPageSize, PAGE_READONLY, &dwOld);
    return EXCEPTION_CONTINUE_EXECUTION;
}

// Threads created during world entry did the write before the frame-paced armer reached
// them. This polls hard enough that a new thread carries the watch within about a
// millisecond of being created, which is well before it runs any game code.
// A hardware watchpoint traps CPU stores only. A write the KERNEL performs into user memory
// on a thread's behalf - a socket receive or a file read - never trips it, which is what a
// validated watch on every thread seeing nothing points at. These wrappers report any such
// call whose destination range covers the target, and the return address names the caller.
typedef int (WSAAPI *PFN_recv)(SOCKET, char*, int, int);
typedef int (WSAAPI *PFN_WSARecv)(SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, LPWSAOVERLAPPED,
                                  LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef BOOL (WINAPI *PFN_ReadFile)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);

static PFN_recv     g_pfnRecv     = 0;
static PFN_WSARecv  g_pfnWSARecv  = 0;
static PFN_ReadFile g_pfnReadFile = 0;

static void ReportIfCovers(const char* pszApi, const void* pBuf, size_t nLen, void* pRet) {
    uintptr_t nTgt = (uintptr_t)&D2gfx::gnTileClipYMin;
    uintptr_t nLo  = (uintptr_t)pBuf;
    if (!nLo) return;
    if (nTgt >= nLo && nTgt < nLo + nLen)
        D2TraceFmt("KWRITE *** %s covers target *** buf=%08X len=%u ret=%08X",
                   pszApi, (unsigned)nLo, (unsigned)nLen, (unsigned)(uintptr_t)pRet);
}

static int WSAAPI HookRecv(SOCKET s, char* pBuf, int nLen, int nFlags) {
    ReportIfCovers("recv", pBuf, (size_t)(nLen > 0 ? nLen : 0), __builtin_return_address(0));
    return g_pfnRecv(s, pBuf, nLen, nFlags);
}

static int WSAAPI HookWSARecv(SOCKET s, LPWSABUF pBufs, DWORD nBufCount, LPDWORD pRecvd,
                              LPDWORD pFlags, LPWSAOVERLAPPED pOv,
                              LPWSAOVERLAPPED_COMPLETION_ROUTINE pCr) {
    void* pRet = __builtin_return_address(0);
    for (DWORD i = 0; pBufs && i < nBufCount; i++)
        ReportIfCovers("WSARecv", pBufs[i].buf, (size_t)pBufs[i].len, pRet);
    return g_pfnWSARecv(s, pBufs, nBufCount, pRecvd, pFlags, pOv, pCr);
}

static BOOL WINAPI HookReadFile(HANDLE h, LPVOID pBuf, DWORD nLen, LPDWORD pRead, LPOVERLAPPED pOv) {
    ReportIfCovers("ReadFile", pBuf, (size_t)nLen, __builtin_return_address(0));
    return g_pfnReadFile(h, pBuf, nLen, pRead, pOv);
}

// Redirects one import across every module in the process, so a call made from any of the
// game's DLLs is covered rather than only the ones made from the executable.
static void PatchIatEverywhere(const char* pszDll, const char* pszFn, void* pNew, void** ppOld) {
    HMODULE hReal = GetModuleHandleA(pszDll);
    if (!hReal) return;
    void* pReal = (void*)GetProcAddress(hReal, pszFn);
    if (!pReal) return;
    *ppOld = pReal;
    HMODULE ahMods[512]; int nMods = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hSnap == INVALID_HANDLE_VALUE) return;
    MODULEENTRY32 me; me.dwSize = sizeof(me);
    if (Module32First(hSnap, &me)) {
        do { if (nMods < 512) ahMods[nMods++] = me.hModule; } while (Module32Next(hSnap, &me));
    }
    CloseHandle(hSnap);
    for (int m = 0; m < nMods; m++) {
        BYTE* pBase = (BYTE*)ahMods[m];
        IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pBase;
        if (pDos->e_magic != IMAGE_DOS_SIGNATURE) continue;
        IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)(pBase + pDos->e_lfanew);
        if (pNt->Signature != IMAGE_NT_SIGNATURE) continue;
        DWORD nRva = pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        if (!nRva) continue;
        IMAGE_IMPORT_DESCRIPTOR* pImp = (IMAGE_IMPORT_DESCRIPTOR*)(pBase + nRva);
        for (; pImp->Name; pImp++) {
            IMAGE_THUNK_DATA* pThunk = (IMAGE_THUNK_DATA*)(pBase + pImp->FirstThunk);
            for (; pThunk->u1.Function; pThunk++) {
                if ((void*)pThunk->u1.Function != pReal) continue;
                DWORD dwOld = 0;
                if (VirtualProtect(&pThunk->u1.Function, sizeof(void*), PAGE_READWRITE, &dwOld)) {
                    pThunk->u1.Function = (uintptr_t)pNew;
                    VirtualProtect(&pThunk->u1.Function, sizeof(void*), dwOld, &dwOld);
                    D2TraceFmt("KHOOK %s!%s patched in module %d", pszDll, pszFn, m);
                }
            }
        }
    }
}

typedef HANDLE (WINAPI *PFN_CreateThread)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE,
                                          LPVOID, DWORD, LPDWORD);
static PFN_CreateThread g_pfnCreateThread = 0;

struct D2ThreadSeed { LPTHREAD_START_ROUTINE pfnStart; LPVOID pParam; };

static DWORD WINAPI ArmedThreadTrampoline(LPVOID pRaw) {
    D2ThreadSeed seed = *(D2ThreadSeed*)pRaw;
    HeapFree(GetProcessHeap(), 0, pRaw);
    D2ProbeArmClipHwbpSelf();
    D2TraceFmt("KTHREAD armed at entry tid=%u start=%08X", (unsigned)GetCurrentThreadId(),
               (unsigned)(uintptr_t)seed.pfnStart);
    return seed.pfnStart(seed.pParam);
}

static HANDLE WINAPI HookCreateThread(LPSECURITY_ATTRIBUTES pSa, SIZE_T nStack,
                                      LPTHREAD_START_ROUTINE pfnStart, LPVOID pParam,
                                      DWORD dwFlags, LPDWORD pTid) {
    D2ThreadSeed* pSeed = (D2ThreadSeed*)HeapAlloc(GetProcessHeap(), 0, sizeof(D2ThreadSeed));
    if (!pSeed) return g_pfnCreateThread(pSa, nStack, pfnStart, pParam, dwFlags, pTid);
    pSeed->pfnStart = pfnStart;
    pSeed->pParam = pParam;
    return g_pfnCreateThread(pSa, nStack, &ArmedThreadTrampoline, pSeed, dwFlags, pTid);
}

// The game's own thread procedures go through _beginthreadex, not the kernel32 import that was
// patched, which is why only one thread was ever armed at entry. Routing them through here puts
// the watch in place before the new thread executes any game code.
struct D2BtSeed { unsigned (__stdcall* pfnStart)(void*); void* pArg; };

static unsigned __stdcall ArmedBeginThreadTrampoline(void* pRaw) {
    D2BtSeed seed = *(D2BtSeed*)pRaw;
    HeapFree(GetProcessHeap(), 0, pRaw);
    D2ProbeArmClipHwbpSelf();
    D2TraceFmt("BTARM tid=%u start=%08X", (unsigned)GetCurrentThreadId(),
               (unsigned)(uintptr_t)seed.pfnStart);
    return seed.pfnStart(seed.pArg);
}

extern "C" void* D2ProbeWrapThreadStart(void* pfnStart, void* pArg, void** ppArgOut) {
    D2BtSeed* pSeed = (D2BtSeed*)HeapAlloc(GetProcessHeap(), 0, sizeof(D2BtSeed));
    if (!pSeed) { *ppArgOut = pArg; return pfnStart; }
    pSeed->pfnStart = (unsigned (__stdcall*)(void*))pfnStart;
    pSeed->pArg = pArg;
    *ppArgOut = pSeed;
    return (void*)&ArmedBeginThreadTrampoline;
}

extern "C" void D2ProbeInstallKernelWriteHooks(void) {
    static volatile LONG bDone = 0;
    if (InterlockedExchange(&bDone, 1)) return;
    PatchIatEverywhere("ws2_32.dll", "recv",     (void*)&HookRecv,     (void**)&g_pfnRecv);
    PatchIatEverywhere("ws2_32.dll", "WSARecv",  (void*)&HookWSARecv,  (void**)&g_pfnWSARecv);
    PatchIatEverywhere("kernel32.dll", "ReadFile", (void*)&HookReadFile, (void**)&g_pfnReadFile);
    PatchIatEverywhere("kernel32.dll", "CreateThread", (void*)&HookCreateThread, (void**)&g_pfnCreateThread);
}

// The write traps on nothing, so instead of waiting for a fault this busy-polls the value and
// freezes the process the moment it changes. Whichever thread performed the store is then still
// parked within a few instructions of it, so its EIP and stack name the writer directly.
static void DumpAllThreadsAt(const char* pszWhy) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;
    THREADENTRY32 te; te.dwSize = sizeof(te);
    DWORD dwPid = GetCurrentProcessId(), dwSelf = GetCurrentThreadId();
    if (Thread32First(hSnap, &te)) {
        do {
            if (te.th32OwnerProcessID != dwPid || te.th32ThreadID == dwSelf) continue;
            HANDLE hT = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
                                   FALSE, te.th32ThreadID);
            if (!hT) continue;
            if (SuspendThread(hT) != (DWORD)-1) {
                CONTEXT ctx; ZeroMemory(&ctx, sizeof(ctx));
                ctx.ContextFlags = CONTEXT_FULL;
                if (GetThreadContext(hT, &ctx)) {
                    D2TraceFmt("FREEZE %s tid=%u eip=%08X esp=%08X ebp=%08X",
                               pszWhy, (unsigned)te.th32ThreadID, (unsigned)ctx.Eip,
                               (unsigned)ctx.Esp, (unsigned)ctx.Ebp);
                    // Walk a few EBP frames so a thread parked in a kernel wait still shows
                    // the game code that led there.
                    uintptr_t nFp = (uintptr_t)ctx.Ebp;
                    for (int d = 0; d < 8 && nFp; d++) {
                        if (IsBadReadPtr((void*)nFp, 8)) break;
                        uintptr_t nRet = *(uintptr_t*)(nFp + 4);
                        if (!nRet) break;
                        D2TraceFmt("FREEZE   tid=%u frame%d ret=%08X", (unsigned)te.th32ThreadID, d,
                                   (unsigned)nRet);
                        uintptr_t nNext = *(uintptr_t*)nFp;
                        if (nNext <= nFp) break;
                        nFp = nNext;
                    }
                }
                ResumeThread(hT);
            }
            CloseHandle(hT);
        } while (Thread32Next(hSnap, &te));
    }
    CloseHandle(hSnap);
}

static DWORD WINAPI ClipArmerThread(LPVOID) {
    D2TraceFmt("IMGBASE %08X", (unsigned)(uintptr_t)GetModuleHandleA(0));
    for (;;) {
        D2ProbeArmClipHwbpAllThreads();
        Sleep(1);
    }
    return 0;
}

// Separate from the armer so the detection loop stays tight. Suspending through handles held
// open in advance freezes the writer within microseconds of the store instead of milliseconds.
static DWORD WINAPI ClipWatchThread(LPVOID) {
    int nLast = *(volatile int*)&D2gfx::gnTileClipYMin;
    int nDumped = 0;
    for (;;) {
        int nNow = *(volatile int*)&D2gfx::gnTileClipYMin;
        if (nNow != nLast) {
            if (nNow == 0 && !nDumped) {
                nDumped = 1;
                LONG nCount = g_nWatched;
                for (LONG i = 0; i < nCount; i++) SuspendThread(g_ahWatched[i]);
                D2TraceFmt("CLIPZERO %d -> %d, froze %d threads", nLast, nNow, (int)nCount);
                for (LONG i = 0; i < nCount; i++) {
                    CONTEXT ctx; ZeroMemory(&ctx, sizeof(ctx));
                    ctx.ContextFlags = CONTEXT_FULL;
                    if (!GetThreadContext(g_ahWatched[i], &ctx)) continue;
                    D2TraceFmt("FROZE idx=%d eip=%08X ebp=%08X", (int)i, (unsigned)ctx.Eip,
                               (unsigned)ctx.Ebp);
                    uintptr_t nFp = (uintptr_t)ctx.Ebp;
                    for (int d = 0; d < 10 && nFp; d++) {
                        if (IsBadReadPtr((void*)nFp, 8)) break;
                        uintptr_t nRet = *(uintptr_t*)(nFp + 4);
                        if (!nRet) break;
                        D2TraceFmt("FROZE   idx=%d f%d ret=%08X", (int)i, d, (unsigned)nRet);
                        uintptr_t nNext = *(uintptr_t*)nFp;
                        if (nNext <= nFp) break;
                        nFp = nNext;
                    }
                }
                for (LONG i = 0; i < nCount; i++) ResumeThread(g_ahWatched[i]);
            }
            nLast = nNow;
        }
        Sleep(1);
    }
    return 0;
}

static DWORD WINAPI ServerThread(LPVOID) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { D2Trace("D2CONTROL: WSAStartup failed"); return 0; }
    HoldWinsockOpen();
    SOCKET srv = OpenListener();
    if (srv == INVALID_SOCKET) { D2Trace("D2CONTROL: socket failed"); return 0; }
    AddVectoredExceptionHandler(1, HwbpVeh);
    AddVectoredExceptionHandler(1, GuardVeh);
    AddVectoredExceptionHandler(1, GuardStepVeh);
    D2TraceFmt("D2CONTROL: listening on %d", D2CONTROL_PORT);
    for (;;) {
        SOCKET c = accept(srv, 0, 0);
        // A dead listener would otherwise spin here for the rest of the run and answer
        // nothing, which reads from the outside exactly like a hung game.
        if (c == INVALID_SOCKET) {
            int nErr = (int)WSAGetLastError();
            if (nErr == WSAEINTR) continue;
            D2TraceFmt("D2CONTROL: accept failed err=%d - rebinding", nErr);
            closesocket(srv);
            HoldWinsockOpen();
            Sleep(200);
            srv = OpenListener();
            if (srv == INVALID_SOCKET) { Sleep(800); }
            continue;
        }
        // One connection at a time, so a client that connects and then says nothing
        // (a status poller holding its socket open) would park every later caller in
        // the backlog forever. Three seconds of silence ends the turn.
        { DWORD tmo = 3000; setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tmo, sizeof(tmo)); }
        char buf[1024]; int used = 0;
        for (;;) {
            int n = recv(c, buf + used, (int)sizeof(buf) - 1 - used, 0);
            if (n <= 0) break;
            used += n; buf[used] = 0;
            char* nl;
            while ((nl = strchr(buf, '\n')) != 0) {
                *nl = 0;
                // 512 was enough while every reply was a status line; `units` returns a
                // list, and a truncated JSON line is worse than none.
                static char reply[8192];
                HandleLine(buf, reply, (int)sizeof(reply) - 2);
                lstrcatA(reply, "\n");
                send(c, reply, (int)lstrlenA(reply), 0);
                int rest = used - (int)(nl - buf) - 1;
                memmove(buf, nl + 1, rest > 0 ? rest : 0);
                used = rest > 0 ? rest : 0; buf[used] = 0;
            }
            if (used >= (int)sizeof(buf) - 1) used = 0;
        }
        closesocket(c);
    }
}

extern "C" void D2CONTROL_Start(void) {
    if (InterlockedExchange(&g_started, 1)) return;
    InitializeCriticalSection(&g_cs);
    // Our-side runtime patches go in before any game code that needs them runs.
    // They live in support/ so the generated engine tree stays untouched.
    D2PATCH_InstallAll();
    CreateThread(0, 0, ServerThread, 0, 0, 0);
}


// One step of the `enter` sequence per frame. Each call returns to the game so the
// frames the previous step needs actually run - the character list is populated by
// the normal out-of-game loop, not by the call that starts it.
static void D2CONTROL_StepEnter(void) {
    long step = g_enterStep;
    if (step < 0) return;

    switch (step) {
    case 0:
        D2Launch::MainMenus::UIMENU_MainMenu();
        g_enterWait = 10;
        InterlockedExchange(&g_enterStep, 1);
        return;

    case 1:
        if (--g_enterWait > 0) return;
        D2Launch::MainMenus::OogCurrentCharSelectionMode = 0;                       // single player
        D2Launch::MainMenus::MAINMENU_CloseAndLaunchCharSelect();
        g_enterWait = 300;
        InterlockedExchange(&g_enterStep, 2);
        return;

    case 2:
        // The saves are enumerated over the following frames, not synchronously.
        if (D2Launch::CharSel::TotalCurrentChars == 0) {
            if (--g_enterWait > 0) return;
            lstrcpynA(g_enterNote, "no characters found", sizeof(g_enterNote));
            InterlockedExchange(&g_enterStep, -1);
            return;
        }
        InterlockedExchange(&g_enterStep, 3);
        return;

    case 3: {
        D2CharSelStrc* pChar = D2Launch::CharSel::D2CharSelStrcFirst;
        while (pChar && lstrcmpiA(pChar->szCharname, g_enterName) != 0) pChar = pChar->pNext;
        if (!pChar) {
            lstrcpynA(g_enterNote, "character not found", sizeof(g_enterNote));
            InterlockedExchange(&g_enterStep, -1);
            return;
        }
        D2TraceFmt("ENTER: %s class=%d flags=%04x",
                   pChar->szCharname, (int)pChar->ePlayerClassID,
                   (unsigned)pChar->nCharacterFlags);
        // Entering this way also sets what the difficulty dialog would have set.
        D2Launch::MainMenus::SelectedCharBnetSingleTcpIp(
            pChar, (eD2PlayerStatus)pChar->nCharacterFlags,
            (int32_t)pChar->ePlayerClassID, (char*)"");
        lstrcpynA(g_enterNote, "entered", sizeof(g_enterNote));
        InterlockedExchange(&g_enterStep, -1);
        return;
    }

    default:
        InterlockedExchange(&g_enterStep, -1);
        return;
    }
}

// A hang leaves no trace at all: the game loop stops calling Pump, the control channel goes
// quiet, and the window keeps repainting because a different thread owns it. Every hang so
// far has therefore been indistinguishable from every other one. This watchdog notices the
// frame counter standing still, suspends the game thread and walks its EBP chain the same
// way the fault handler does, so a hang reports a stack instead of a silence.
// Walk one already-suspended thread's EBP chain. The caller owns the suspend so that
// the whole process can be frozen for a consistent picture across threads.
static void DumpSuspendedThread(const char* pszTag, DWORD dwTid, HANDLE hThread) {
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_CONTROL;
    if (!GetThreadContext(hThread, &ctx)) return;
    DWORD base = (DWORD)(UINT_PTR)GetModuleHandleA(0);
    D2TraceFmt("%s tid=%lu eip=%08lX rva=%08lX ebp=%08lX esp=%08lX",
               pszTag, dwTid, ctx.Eip, ctx.Eip - base, ctx.Ebp, ctx.Esp);
    DWORD* pFrame = (DWORD*)ctx.Ebp;
    for (int i = 0; i < 20 && pFrame; i++) {
        if (IsBadReadPtr(pFrame, 8)) break;
        DWORD ret = pFrame[1];
        if (!ret) break;
        D2TraceFmt("%sFRAME tid=%lu %2d ret=%08lX rva=%08lX", pszTag, dwTid, i, ret, ret - base);
        DWORD* pNext = (DWORD*)pFrame[0];
        if (pNext <= pFrame) break;
        pFrame = pNext;
    }
}

// A stalled game thread is usually waiting on another thread, so a stack for the game
// thread alone names the victim and never the culprit - the async I/O worker parked in a
// sprite decode looked identical to a game loop that had simply stopped. Freeze every
// thread in the process and walk all of them.
static void DumpAllThreads(void) {
    DWORD dwSelf = GetCurrentThreadId();
    DWORD dwPid  = GetCurrentProcessId();
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;
    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    HANDLE  ahThreads[64];
    DWORD   adwTids[64];
    int     nThreads = 0;
    if (Thread32First(hSnap, &te)) {
        do {
            if (te.th32OwnerProcessID != dwPid || te.th32ThreadID == dwSelf) continue;
            if (nThreads >= 64) break;
            HANDLE h = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, te.th32ThreadID);
            if (!h) continue;
            if (SuspendThread(h) == (DWORD)-1) { CloseHandle(h); continue; }
            adwTids[nThreads]  = te.th32ThreadID;
            ahThreads[nThreads] = h;
            nThreads++;
        } while (Thread32Next(hSnap, &te));
    }
    CloseHandle(hSnap);
    for (int i = 0; i < nThreads; i++) DumpSuspendedThread("D2HANGTHREAD", adwTids[i], ahThreads[i]);
    for (int i = 0; i < nThreads; i++) { ResumeThread(ahThreads[i]); CloseHandle(ahThreads[i]); }
}


// ---------------------------------------------------------------------------
// Unit enumeration
//
// The client keeps one hash table per unit type: 128 buckets of D2UnitStrc*,
// chained through pListNext. The tables sit back to back, 0x200 bytes apart, and the
// game itself addresses them by that arithmetic rather than through the declared
// `[128][6]` shape - which is transposed and would index the wrong bucket. Do the same
// arithmetic here rather than trusting the declaration.
//
// Screen position is the transform the item-label code uses (ShowItems.cpp, 004c0b60):
// the unit's target coordinate minus the world offset. That is CLIENT space - exactly
// what `click x y` takes - so the two compose without a scale factor anywhere.
// Perspective mode needs D2GFX_AdjustPerspectivePosition instead; it is off in the
// windowed test configuration, and `persp` in the reply says so rather than silently
// reporting coordinates that are 40 pixels out.
static int UnitsReply(char* out, int outMax, int nWantType) {
    const int nOffX = D2Client::UI::Path::DRAW_GetWorldOffsetX();
    const int nOffY = D2Client::UI::Path::DRAW_GetWorldOffsetY();
    const int bPersp = (int)D2gfx::D2GFX::D2GFX_IsPerspectiveActive();

    // Both tables are walked and tagged. `UNITS_FindClientSideUnit` reads
    // ServerSideUnitHashTables and `UNITS_FindServerSideUnit` reads the Client one, so one
    // pair of names is wrong in Ghidra and guessing which would put the NPC list on the
    // wrong side. Report the table each unit came from and let the caller see it.
    void* apTables[2] = { (void*)D2Client::UNIT::CUnit::ClientSideUnitHashTables, (void*)D2Client::UNIT::CUnit::ServerSideUnitHashTables };
    int used = wsprintfA(out, "{\"persp\":%d,\"units\":[", bPersp);
    int nFound = 0;
    for (int nTable = 0; nTable < 2; nTable++) {
    for (int nType = 0; nType < 6; nType++) {
        if (nWantType >= 0 && nType != nWantType) continue;
        D2UnitStrc** ppTable = (D2UnitStrc**)((char*)apTables[nTable] + nType * 0x200);
        for (int nBucket = 0; nBucket < 128; nBucket++) {
            for (D2UnitStrc* pUnit = ppTable[nBucket]; pUnit; pUnit = pUnit->pListNext) {
                if (used > outMax - 260) { nFound++; continue; }
                const int nX = D2Common::Units::GetTargetX(pUnit) - nOffX;
                const int nY = D2Common::Units::GetTargetY(pUnit) - nOffY;
                // For a monster, the MonStats row says whether it is a shopkeeper. `interact`
                // is the bit that decides whether a click opens a menu at all, so it is the
                // one a "can I buy something" test has to see.
                char szName[40]; szName[0] = 0;
                int bNpc = 0, bInteract = 0, bInTown = 0;
                // isAtt is what MONSTER init turns into UNITFLAG_CANBEATTACKED, and that bit
                // is the whole gate on whether a click opens an NPC's menu. Report the txt
                // bit and the live unit flag side by side so a mismatch is visible without a
                // rebuild - reading only one of them cannot tell a bad txt row from a bad
                // struct offset.
                int bIsAtt = -1, bIsSel = -1;
                // For an item, which inventory page it landed in. "the shop packet never
                // arrived" and "it arrived and the item went to the wrong page" both render
                // as an empty merchant grid; only the page tells them apart.
                int nPage = -1;
                // Whether this object is a SHRINE, straight from the engine's own predicate.
                // Object class ids are not memorable and the Objects.txt token is two bytes,
                // so hunting a shrine by class id is guesswork; this makes the test exact.
                int bShrine = -1;
                if (pUnit->eUnitType == UNIT_OBJECT)
                    bShrine = D2Common::Units::IsShrine(pUnit) ? 1 : 0;
                if (pUnit->eUnitType == UNIT_ITEM)
                    nPage = (int)D2Common::ITEMS::Items::ITEM_GetContainingInventoryPage(pUnit);
                if (pUnit->eUnitType == UNIT_MONSTER && D2Common::ITEMS::Items::sgptDataTable &&
                    pUnit->nClassId >= 0 && pUnit->nClassId < D2Common::ITEMS::Items::sgptDataTable->nTxtMonStatsSize) {
                    const D2MonStatsTxt* pRow = &D2Common::ITEMS::Items::sgptDataTable->pTxtMonStats[pUnit->nClassId];
                    bNpc = pRow->npc; bInteract = pRow->interact; bInTown = pRow->inTown;
                    const D2MonStats2Txt* p2 =
                        D2Common::DATATBLS::MonsterTbls::TXT_MonStats_GetMonStats2(pUnit->nClassId);
                    if (p2) { bIsAtt = p2->isAtt; bIsSel = p2->isSel; }
                    const uint16_t* pwsz = D2Lang::StrTable::strtable::GetLocaleString(pRow->NameStr);
                    if (pwsz) { int k = 0; for (; k < 32 && pwsz[k]; k++) szName[k] = (char)(pwsz[k] & 0x7f); szName[k] = 0; }
                }
                used += wsprintfA(out + used,
                    "%s{\"tbl\":%d,\"t\":%d,\"cls\":%d,\"guid\":%d,\"x\":%d,\"y\":%d,\"p\":\"0x%X\",\"room\":%d,"
                    "\"npc\":%d,\"intr\":%d,\"town\":%d,\"uf\":\"0x%X\",\"isatt\":%d,\"issel\":%d,\"page\":%d,\"shrine\":%d,\"name\":\"%s\"}",
                    nFound ? "," : "", nTable, (int)pUnit->eUnitType, (int)pUnit->nClassId,
                    (int)pUnit->nUnitGUID, nX, nY, (unsigned)(UINT_PTR)pUnit,
                    D2Common::Units::UNITS_GetRoom(pUnit) ? 1 : 0, bNpc, bInteract, bInTown,
                    (unsigned)pUnit->eUnitFlags, bIsAtt, bIsSel, nPage, bShrine, szName);
                nFound++;
            }
        }
    }
    }
    used += wsprintfA(out + used, "],\"n\":%d}", nFound);
    return used;
}

static DWORD WINAPI D2CONTROL_HangWatchdog(LPVOID) {
    long   nLastFrame  = -1;
    int    nStalls     = 0;
    int    nReported   = 0;
    for (;;) {
        Sleep(1000);
        long nNow = g_frames;
        if (nNow != nLastFrame) { nLastFrame = nNow; nStalls = 0; nReported = 0; continue; }
        if (++nStalls < 8 || nReported >= 3 || !g_gameThread) continue;
        nReported++;
        D2TraceFmt("D2HANG frame stuck at %ld for %ds", nNow, nStalls);
        DumpAllThreads();
    }
}

// ---------------------------------------------------------------------------------------
// In-game walker. Driving the player by injecting clicks from outside costs a round trip
// per step, so crossing a town takes longer than the session that is watching it. This
// runs on the game thread instead: one movement order every few frames straight into
// CLIENT_MovePlayerToPosition, the same entry the click path reaches, with the target
// stepped along in short hops because the pathfinder only ever plans a dozen tiles ahead.
// It also watches the unit tables each frame, so it reports a shopkeeper the moment one
// comes into spawn range rather than when someone remembers to ask.

static void WalkerScanForShop(void) {
    // interact=1 in MonStats is the bit that decides whether a click opens a menu at all,
    // so it is the only test that answers "can I trade with this".
    if (!D2Common::ITEMS::Items::sgptDataTable) return;
    void* apTables[2] = { (void*)D2Client::UNIT::CUnit::ClientSideUnitHashTables, (void*)D2Client::UNIT::CUnit::ServerSideUnitHashTables };
    for (int nTable = 0; nTable < 2; nTable++) {
        D2UnitStrc** ppTable = (D2UnitStrc**)((char*)apTables[nTable] + UNIT_MONSTER * 0x200);
        for (int nBucket = 0; nBucket < 128; nBucket++) {
            for (D2UnitStrc* pUnit = ppTable[nBucket]; pUnit; pUnit = pUnit->pListNext) {
                if (pUnit->nClassId < 0 || pUnit->nClassId >= (int)D2Common::ITEMS::Items::sgptDataTable->nTxtMonStatsSize) continue;
                const D2MonStatsTxt* pRow = &D2Common::ITEMS::Items::sgptDataTable->pTxtMonStats[pUnit->nClassId];
                if (!pRow->interact) continue;
                if (pUnit->nClassId == 175) continue;          // Warriv: interactable, sells nothing
                // Remember every class already reported, not just the last one: with several
                // shopkeepers in range a single-slot check re-reports each of them on every
                // frame and buries the log.
                {
                    bool bSeen = false;
                    for (int k = 0; k < g_nSeen; k++) if (g_aSeen[k] == (int)pUnit->nClassId) { bSeen = true; break; }
                    if (bSeen) continue;
                    if (g_nSeen < (int)(sizeof(g_aSeen) / sizeof(g_aSeen[0]))) g_aSeen[g_nSeen++] = (int)pUnit->nClassId;
                }
                g_foundCls  = (int)pUnit->nClassId;
                g_foundGuid = (int)pUnit->nUnitGUID;
                g_foundX = D2Common::Units::GetTargetX(pUnit);
                g_foundY = D2Common::Units::GetTargetY(pUnit);
                const uint16_t* pwsz = D2Lang::StrTable::strtable::GetLocaleString(pRow->NameStr);
                int k = 0; if (pwsz) for (; k < 32 && pwsz[k]; k++) g_foundName[k] = (char)(pwsz[k] & 0x7f);
                g_foundName[k] = 0;
                D2TraceFmt("WALKER found cls=%d guid=%d at %d,%d '%s'",
                           g_foundCls, g_foundGuid, g_foundX, g_foundY, g_foundName);
            }
        }
    }
}

static void WalkerStep(void) {
    if (!g_walkOn || !D2Client::UNIT::CUnit::pPlayerUnit) return;
    D2DynamicPathStrc* pPath = D2Client::UNIT::CUnit::pPlayerUnit->pPath.pDynamicPath;
    if (!pPath) return;
    int nX = pPath->Coords.tSubtile.nPosX, nY = pPath->Coords.tSubtile.nPosY;

    WalkerScanForShop();

    if (nX != g_walkX || nY != g_walkY) { g_walkStuck = 0; g_walkX = nX; g_walkY = nY; }

    int nDX = g_walkTX - nX, nDY = g_walkTY - nY;
    if (nDX * nDX + nDY * nDY <= 9) { InterlockedExchange(&g_walkOn, 0); return; }

    // The pump runs thousands of times a second. Re-ordering on a pump counter restarts
    // the path before a single step of it is walked, so the player shuffles in place.
    // Order again only when the previous path is spent, with a wall-clock floor so a
    // refused path still retries.
    DWORD dwNow = GetTickCount();
    bool bPathSpent = (pPath->dwPathPoints == 0) || (pPath->nPathPointIndex >= pPath->dwPathPoints);
    if (!bPathSpent && dwNow - (DWORD)g_walkFrames < 1200) return;
    if (dwNow - (DWORD)g_walkFrames < 150) return;
    g_walkFrames = (int)dwNow;

    // The pathfinder plans a short distance at a time, so aim at a point on the way.
    int nLen = nDX < 0 ? -nDX : nDX;
    int nAbsY = nDY < 0 ? -nDY : nDY;
    if (nAbsY > nLen) nLen = nAbsY;
    int nStepX = nX + (nLen > 12 ? nDX * 12 / nLen : nDX);
    int nStepY = nY + (nLen > 12 ? nDY * 12 / nLen : nDY);

    // Boxed in against geometry: jog sideways for a while rather than re-issuing an order
    // the pathfinder has already refused.
    if (nX == g_walkX && nY == g_walkY) g_walkStuck++;
    if (g_walkStuck > 3) {
        g_walkJog = (g_walkJog + 1) & 3;
        static const int aJogX[4] = { 10, -10, 0, 0 };
        static const int aJogY[4] = { 0, 0, 10, -10 };
        nStepX = nX + aJogX[g_walkJog];
        nStepY = nY + aJogY[g_walkJog];
        g_walkStuck = 0;
    }

    g_walkSteps++;
    D2Client::UNIT::Player::CLIENT_MovePlayerToPosition_Extern(D2Client::UNIT::CUnit::pPlayerUnit, (int16_t)nStepX, (int16_t)nStepY, 1);
}

extern "C" void D2CONTROL_Pump(void) {
    D2CONTROL_Start();                 // idempotent; avoids picking an init site
    if (!g_gameThread) {
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
                        &g_gameThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
        g_imageBase = (DWORD)(UINT_PTR)GetModuleHandleA(0);
        DWORD dwWatchdogId = 0;
        CloseHandle(CreateThread(0, 0, D2CONTROL_HangWatchdog, 0, 0, &dwWatchdogId));
    }
    long frame = InterlockedIncrement(&g_frames);
    // The game-view write-watch, armed from a flag file when the control channel is
    // not available: `test\watchview` beside the exe arms DR0 on D2GameView->pView.bottom
    // the first frame a game view exists, on this thread (the writer's). One-shot.
    {
        static bool bViewWatchTried = false;
        if (!bViewWatchTried && D2Client::GAME::Game::D2GameView != nullptr &&
            GetFileAttributesA("test\\watchview") != INVALID_FILE_ATTRIBUTES) {
            bViewWatchTried = true;
            g_pHwbpTarget = (void*)((char*)D2Client::GAME::Game::D2GameView + 0x10);
            D2TraceFmt("VIEWWATCH arming on %08X (D2GameView=%08X)", (unsigned)(uintptr_t)g_pHwbpTarget,
                       (unsigned)(uintptr_t)D2Client::GAME::Game::D2GameView);
            D2ProbeArmClipHwbpSelf();
        }
    }
    // NOT armed by default. D2TRACE_WatchWrite sets an x86 hardware write watchpoint, and the
    // trap handler writes a trace line from a vectored exception handler - on an address the
    // game writes every frame that is a guaranteed hang, and it took a whole build to diagnose
    // as one. Arm it deliberately from a `watch <hexaddr>` command when hunting a wild store,
    // never on a normal run.
    HWND hWnd = D2gfx::D2GFX::GetHwnd();

    // Which level the player is standing in. "the town looks wrong" and "the town IS the
    // wrong town" are different bugs and the level id is what separates them.
    int nPlayerLevelId = -1, nPlayerAct = -1;
    if (D2Client::UNIT::CUnit::pPlayerUnit) {
        nPlayerAct = (int)D2Client::UNIT::CUnit::pPlayerUnit->nAct;
        D2RoomStrc* pRoom = D2Common::Units::UNITS_GetRoom(D2Client::UNIT::CUnit::pPlayerUnit);
        if (pRoom) nPlayerLevelId = (int)D2Common::DUNGEON::Dungeon::DRLGROOM_GetLevelIdFromRoom(pRoom);
    }

    // Publish a snapshot. pPlayerUnit going non-null is the signal that a character is
    // actually loaded into a game rather than sitting in a menu.
    EnterCriticalSection(&g_cs);
    wsprintfA(g_state,
        "{\"frame\":%ld,\"hwnd\":\"0x%X\",\"appmode\":%d,"
        "\"player\":\"0x%X\",\"ingame\":%d,\"enter\":%ld,\"chars\":%u,\"lvl\":%d,\"act\":%d,\"loaded\":%d,\"note\":\"%s\"}",
        frame, (unsigned)(UINT_PTR)hWnd, (int)D2Client::ClientModeInGame::gdwAppMode,
        (unsigned)(UINT_PTR)D2Client::UNIT::CUnit::pPlayerUnit, D2Client::UNIT::CUnit::pPlayerUnit ? 1 : 0,
        g_enterStep, (unsigned)D2Launch::CharSel::TotalCurrentChars,
        nPlayerLevelId, nPlayerAct, (int)D2Client::GAME::Game::ServerLoadingComplete, g_enterNote);
    LeaveCriticalSection(&g_cs);

    D2CONTROL_StepEnter();
    WalkerStep();

    if (!hWnd) return;
    for (;;) {
        Action a;
        EnterCriticalSection(&g_cs);
        if (g_qHead == g_qTail || g_queue[g_qHead].atFrame > frame) { LeaveCriticalSection(&g_cs); break; }
        a = g_queue[g_qHead];
        g_qHead = (g_qHead + 1) % MAXQ;
        LeaveCriticalSection(&g_cs);

        // Position rides in the lParam and the game's own handler chain stores it, exactly
        // as it would for a real message - nothing here writes the cursor globals directly
        // (the gnD2WinMouse* pair is Mac-only anyway, guarded by D2_PLATFORM_MAC).
        if (a.kind == 3) {
            // Debug registers cannot be installed on a RUNNING thread from outside, so the
            // arm has to happen on the thread that will do the writing - the game thread.
            D2ProbeArmClipHwbpSelf();
        } else if (a.kind == 2) {
            D2UnitStrc* pTarget = nullptr;
            void* apT[2] = { (void*)D2Client::UNIT::CUnit::ClientSideUnitHashTables, (void*)D2Client::UNIT::CUnit::ServerSideUnitHashTables };
            for (int t = 0; t < 2 && !pTarget; t++) {
                D2UnitStrc** ppT = (D2UnitStrc**)((char*)apT[t] + (int)a.wp * 0x200);
                for (int b = 0; b < 128 && !pTarget; b++)
                    for (D2UnitStrc* p = ppT[b]; p; p = p->pListNext)
                        if (p->nUnitGUID == (int)a.lp) { pTarget = p; break; }
            }
            if (pTarget) {
                // GetTargetX/Y minus the world offsets IS client screen space here - the
                // same conversion UnitsReply uses, and it puts the player at 400,292, dead
                // centre of the 800x600 client. Running CoordsMapToScreen on top of that
                // (as d2bs does against the real D2COMMON export) double-converts and lands
                // the click at -1292608,560644.
                int32_t cx = D2Common::Units::GetTargetX(pTarget) - D2Client::UI::Path::DRAW_GetWorldOffsetX();
                int32_t cy = D2Common::Units::GetTargetY(pTarget) - D2Client::UI::Path::DRAW_GetWorldOffsetY();
                D2Client::GAME::Select::SetSelectedUnit_I(pTarget);
                D2Client::UNIT::Player::ClickMap(a.x, cx, cy, 0);
                D2TraceFmt("CLICKUNIT t=%d guid=%d -> screen %d,%d type=%d",
                           (int)a.wp, (int)a.lp, (int)cx, (int)cy, a.x);
            } else {
                D2TraceFmt("CLICKUNIT t=%d guid=%d NOT FOUND", (int)a.wp, (int)a.lp);
            }
        } else if (a.kind == 4) {
            // The cast path bails before it sends anything if the REAL cursor is below the
            // bottom panel (PLAYER_ExecuteModeBasedAction's first two lines), so the cursor
            // has to be where the click says it is - a queued WM_MOUSEMOVE is not enough.
            D2Client::Cursor::SetMouseXY(a.x, a.y);
            D2Client::UNIT::Player::ClickMap((int32_t)a.wp, a.x, a.y, 0);
            D2TraceFmt("CLICKMAP-CMD type=%d at %d,%d mouse=%d,%d panelY=%d", (int)a.wp, a.x, a.y,
                       (int)D2Client::Cursor::GetMouseX(), (int)D2Client::Cursor::GetMouseY(),
                       (int)D2Client::UI::Draw::DRAW_GetBottomPanelY());
        } else if (a.kind == 6) {
            D2Client::UNIT::UnitMode::UNITMODE_ExecuteSkillAction(
                (eD2UnitModeAction)(int)a.wp, D2Client::UNIT::Player::GetPlayerUnit_UI(), a.x, a.y, 0);
            D2TraceFmt("CASTDBG action=%d at world %d,%d", (int)a.wp, a.x, a.y);
        } else if (a.kind == 5) {
            D2Client::GAME::Msg::NET_D2GS_CLIENT_Send_Extern((byte*)a.path, a.x);
            D2TraceFmt("CPKT op=%02X len=%d", (unsigned char)a.path[0], a.x);
        } else if (a.kind == 1) {
            // The game's own capture: it reads the real back buffer, so it sees exactly
            // what was rendered - no PrintWindow, no foreground, no DPI scaling to undo.
            D2Win::Src::D2WinMain::D2WIN_TakeThrottledScreenshot(a.path);
        } else {
            D2Win::Src::D2WinMain::WndProc(hWnd, a.msg, (uint32_t)a.wp, a.lp);
        }
    }

    // Catch the exact frame gnTileClipYMin changes. The trace lines immediately before the
    // flip name the subsystem responsible, which is what the watchpoint could not deliver.
    {
        static int nLastClip = 0x7FFFFFFF;
        if (D2gfx::gnTileClipYMin != nLastClip) {
            D2TraceFmt("CLIPFLIP frame=%d %d -> %d (clipBottom=%d)",
                       (int)g_frames, nLastClip, D2gfx::gnTileClipYMin, D2gfx::D2GFX::gnTileClipBottom);
            nLastClip = D2gfx::gnTileClipYMin;
        }
    }
    if (InterlockedExchange(&g_reqShopList, 0)) {
        int u = wsprintfA(g_shopList, "{\"npc\":%d,\"items\":[",
                          (int)D2Client::_Interaction::nCurrentInteractedNpcGUID);
        int n = 0;
        D2UnitStrc* pNpc = D2Client::UNIT::Player::UNITS_FindClientSideUnit(
            D2Client::_Interaction::nCurrentInteractedNpcGUID, UNIT_MONSTER);
        D2InventoryStrc* pInv = pNpc ? pNpc->pInventory : nullptr;
        if (pInv) {
            for (D2UnitStrc* pIt = D2Common::INVENTORY::Inventory::INVENTORY_GetFirstItem(pInv);
                 pIt && u < (int)sizeof(g_shopList) - 80;
                 pIt = D2Common::INVENTORY::Inventory::INVENTORY_GetNextItem(pIt)) {
                u += wsprintfA(g_shopList + u, "%s{\"guid\":%d,\"cls\":%d}",
                               n++ ? "," : "", (int)pIt->nUnitGUID, (int)pIt->nClassId);
            }
        }
        wsprintfA(g_shopList + u, "],\"n\":%d}", n);
    }

    if (InterlockedExchange(&g_reqMyList, 0)) {
        int u = wsprintfA(g_myList, "{\"items\":[");
        int n = 0;
        D2UnitStrc* pPl = D2Client::UNIT::Player::GetPlayerUnit_UI();
        D2InventoryStrc* pInv = pPl ? pPl->pInventory : nullptr;
        if (pInv) {
            for (D2UnitStrc* pIt = D2Common::INVENTORY::Inventory::INVENTORY_GetFirstItem(pInv);
                 pIt && u < (int)sizeof(g_myList) - 90;
                 pIt = D2Common::INVENTORY::Inventory::INVENTORY_GetNextItem(pIt)) {
                u += wsprintfA(g_myList + u, "%s{\"guid\":%d,\"cls\":%d,\"page\":%d}",
                               n++ ? "," : "", (int)pIt->nUnitGUID, (int)pIt->nClassId,
                               (int)D2Common::ITEMS::Items::ITEM_GetContainingInventoryPage(pIt));
            }
        }
        wsprintfA(g_myList + u, "],\"n\":%d}", n);
    }

    // Two frames apart: the cursor pickup has to be acknowledged before the drop is meaningful.
    long nFree = g_reqFreeGuid;
    if (nFree) {
        if (g_freeStage == 0) {
            D2Client::GAME::Msg::NET_D2GS_CLIENT_Send_INT(0x19, nFree);
            g_freeStage = 1;
        } else if (++g_freeStage > 60) {
            D2Client::GAME::Msg::NET_D2GS_CLIENT_Send_INT(0x17, nFree);
            InterlockedExchange(&g_reqFreeGuid, 0);
            g_freeStage = 0;
        }
    }

    long nBuy = InterlockedExchange(&g_reqBuyGuid, 0);
    if (nBuy) {
        // Exactly what the shop panel's click handler would set before calling the game's
        // own transaction function. The purchase itself is entirely the game's code: it
        // sends 0x32 NpcBuyConfirm and the server moves the item and the gold.
        D2Client::UI::npcmenu::gnNpcMenuTransactionType = (uint32_t)g_reqTxType;   // 1 buy, 2 sell, 3 gamble
        D2Client::UI::npcmenu::gnNpcMenuTransactionItemGUID = (uint32_t)nBuy;
        D2Client::UI::npcmenu::gbNpcMenuTransactionPending = 1;
        D2Client::UI::npcmenu::NPCMENU_ExecuteShopTransaction(0, 0);
    }
}
