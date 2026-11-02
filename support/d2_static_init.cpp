// Run the game's own C++ static initialisers, in the order the binary lists them.
//
// MSVC emits a table of parameterless constructors and _initterm walks it before
// WinMain. Diablo II's table is 66 entries at 006cc618. Only four had been recovered
// as functions, so nothing ran the other 62 and the reconstruction met zeroed state
// on the first lock it touched.
//
// Two of them sat in the excluded _Wrappers namespace and were skipped here on the
// reasoning that they were the CRT's job. That was wrong. 006ca740 was called
// CRT_InitExceptionHandler but initialises the log message queue -
// gLogMessageQueueTail = &gLogMessageQueueTail, an empty circular list - and without
// it EnqueueFormattedLogMessage reads address 4. Both are now in their real
// namespaces and all 66 run.
//
// Order is the table's, not address order - see docs/data/cpp-static-init-order.tsv.
// Run from a prioritised constructor rather than by editing the generated WinMain,
// after support/d2_trace.cpp at 150 so a fault inside one of these still logs.

#include <windows.h>

#include "globals/globals.h"
#include "support/d2_trace.h"
#include "D2Net/SRP.h"
#include "Fog/Src/LogManager.h"
#include "Storm/Source/SSystem.h"
#include "_unnamespaced.h"
#include "Bnclient/BnDownload.h"
#include "Bnclient/BnSend.h"
#include "Bnclient/Bnet.h"
#include "Bnclient/Cache.h"
#include "D2CMP/SRC/CelCmp.h"
#include "D2Client/ClientModeInGame.h"
#include "D2Client/UI/Chat.h"
#include "D2Client/UI/CmdTbl.h"
#include "D2Client/UI/Inv.h"
#include "D2Client/UI/QuestLog.h"
#include "D2Client/UI/Text.h"
#include "D2Client/UI/Trade.h"
#include "D2Client/UI/npcmenu.h"
#include "D2Client/UI/ui.h"
#include "D2Common/Drlg/Preset.h"
#include "D2Game/GAME/Server.h"
#include "D2Game/GAME/Task.h"
#include "D2Game/Objects/Objects.h"
#include "D2Game/UNIT/SUnit.h"
#include "D2Launch/CharSel.h"
#include "D2Win/Src/D2WinFont.h"
#include "D2gfx/D2GFX.h"
#include "Storm/Source/SCMD.h"
#include "Storm/Source/SComp.h"
#include "Storm/Source/SEVT.h"
#include "Storm/Source/SErr.h"
#include "Storm/Source/SFile.h"
#include "Storm/WindowHandle.h"

#ifdef _MSC_VER
// Visual C++ has no constructor attribute. The same function goes into the CRT's
// own initialiser table instead: _initterm walks .CRT$XCA..XCZ before WinMain,
// and .CRT$XCU is the group the compiler gives a TU's dynamic initialisers.
extern "C" void D2RunCppStaticInitializers();
#pragma section(".CRT$XCU", read)
extern "C" __declspec(allocate(".CRT$XCU")) void (__cdecl* const gpfnD2RunCppStaticInitializers)(void) =
    D2RunCppStaticInitializers;
extern "C" void D2RunCppStaticInitializers() {
#else
extern "C" __attribute__((constructor(200))) void D2RunCppStaticInitializers() {
#endif
    D2Trace("staticinit[00] 006ca700");
    Fog::Src::LogManager::StaticInit_6ca700();
    D2Trace("staticinit[01] 006ca720");
    Fog::Src::LogManager::StaticInit_6ca720();
    D2Trace("staticinit[02] 006ca740");
    Fog::Src::LogManager::LOGMGR_InitMessageQueue();
    D2Trace("staticinit[03] 006ca7b0");
    Storm::Source::SFile::StaticInit_6ca7b0();
    D2Trace("staticinit[04] 006ca7d0");
    Storm::Source::SFile::StaticInit_6ca7d0();
    D2Trace("staticinit[05] 006ca7f0");
    Storm::Source::SFile::StaticInit_6ca7f0();
    D2Trace("staticinit[06] 006ca810");
    Storm::Source::SFile::StaticInit_6ca810();
    D2Trace("staticinit[07] 006ca830");
    Storm::Source::SFile::StaticInit_6ca830();
    D2Trace("staticinit[08] 006ca850");
    Storm::Source::SFile::StaticInit_6ca850();
    D2Trace("staticinit[09] 006ca870");
    Storm::Source::SFile::StaticInit_6ca870();
    D2Trace("staticinit[10] 006ca890");
    Storm::Source::SSystem::StaticInit_6ca890();
    D2Trace("staticinit[11] 006ca8a0");
    Storm::Source::SErr::StaticInit_6ca8a0();
    D2Trace("staticinit[12] 006ca8c0");
    Storm::Source::SComp::StaticInit_6ca8c0();
    D2Trace("staticinit[13] 006ca8e0");
    Storm::WindowHandle::StaticInit_6ca8e0();
    D2Trace("staticinit[14] 006ca900");
    Storm::Source::SEVT::StaticInit_6ca900();
    D2Trace("staticinit[15] 006ca920");
    Storm::Source::SEVT::StaticInit_6ca920();
    D2Trace("staticinit[16] 006ca940");
    D2Launch::CharSel::StaticInit_6ca940();
    D2Trace("staticinit[17] 006ca960");
    StaticInit_6ca960();
    D2Trace("staticinit[18] 006ca980");
    StaticInit_6ca980();
    D2Trace("staticinit[19] 006ca9a0");
    D2Client::ClientModeInGame::StaticInit_6ca9a0();
    D2Trace("staticinit[20] 006ca9b0");
    D2Client::ClientModeInGame::StaticInit_6ca9b0();
    D2Trace("staticinit[21] 006ca9c0");
    D2Client::ClientModeInGame::QSERVER_InitGlobalGameState();
    D2Trace("staticinit[22] 006caa10");
    D2Client::UI::ui::StaticInit_6caa10();
    D2Trace("staticinit[23] 006caa30");
    D2Client::UI::ui::StaticInit_6caa30();
    D2Trace("staticinit[24] 006caa50");
    D2Client::UI::ui::StaticInit_6caa50();
    D2Trace("staticinit[25] 006caa70");
    D2Client::UI::ui::StaticInit_6caa70();
    D2Trace("staticinit[26] 006caa90");
    StaticInit_6caa90();
    D2Trace("staticinit[27] 006caaa0");
    StaticInit_6caaa0();
    D2Trace("staticinit[28] 006caab0");
    StaticInit_6caab0();
    D2Trace("staticinit[29] 006caad0");
    D2Client::UI::CmdTbl::StaticInit_6caad0();
    D2Trace("staticinit[30] 006caaf0");
    D2Client::UI::Chat::StaticInit_6caaf0();
    D2Trace("staticinit[31] 006cab10");
    D2Client::UI::Chat::StaticInit_6cab10();
    D2Trace("staticinit[32] 006cab30");
    D2Client::UI::Chat::StaticInit_6cab30();
    D2Trace("staticinit[33] 006cab90");
    D2Client::UI::Chat::StaticInit_6cab90();
    D2Trace("staticinit[34] 006cabf0");
    D2Client::UI::Inv::StaticInit_6cabf0();
    D2Trace("staticinit[35] 006cac10");
    D2Client::UI::Text::StaticInit_6cac10();
    D2Trace("staticinit[36] 006cac30");
    D2Client::UI::QuestLog::StaticInit_6cac30();
    D2Trace("staticinit[37] 006cacd0");
    D2Client::UI::QuestLog::StaticInit_6cacd0();
    D2Trace("staticinit[38] 006cace0");
    D2Client::UI::ui::StaticInit_6cace0();
    D2Trace("staticinit[39] 006cad00");
    StaticInit_6cad00();
    D2Trace("staticinit[40] 006cad10");
    D2Client::UI::npcmenu::StaticInit_6cad10();
    D2Trace("staticinit[41] 006cad80");
    D2Client::UI::Trade::StaticInit_6cad80();
    D2Trace("staticinit[42] 006cadb0");
    StaticInit_6cadb0();
    D2Trace("staticinit[43] 006cade0");
    StaticInit_6cade0();
    D2Trace("staticinit[44] 006cadf0");
    D2gfx::D2GFX::StaticInit_6cadf0();
    D2Trace("staticinit[45] 006cae00");
    D2gfx::D2GFX::StaticInit_6cae00();
    D2Trace("staticinit[46] 006cae10");
    D2gfx::D2GFX::StaticInit_6cae10();
    D2Trace("staticinit[47] 006cae20");
    D2Win::Src::D2WinFont::StaticInit_6cae20();
    D2Trace("staticinit[48] 006cae40");
    D2Win::Src::D2WinFont::StaticInit_6cae40();
    D2Trace("staticinit[49] 006cae50");
    D2Win::Src::D2WinFont::StaticInit_6cae50();
    D2Trace("staticinit[50] 006cae5f");
    // Renamed in Ghidra once the 12-byte list head at 0072fa14 was identified as the
    // fault-handler list (server v892). This file is hand-written, so the new name has
    // to be carried across by hand.
    StaticInit_gFaultHandlerList();
    D2Trace("staticinit[51] 006cae80");
    Bnclient::Bnet::StaticInit_6cae80();
    D2Trace("staticinit[52] 006caea0");
    Bnclient::Cache::StaticInit_6caea0();
    D2Trace("staticinit[53] 006caec0");
    Bnclient::BnSend::StaticInit_6caec0();
    D2Trace("staticinit[54] 006caed0");
    Bnclient::BnDownload::StaticInit_6caed0();
    D2Trace("staticinit[55] 006caef0");
    D2Net::SRP::SRP_GlobalInit();
    D2Trace("staticinit[56] 006caf10");
    D2Game::GAME::Server::StaticInit_6caf10();
    D2Trace("staticinit[57] 006caf30");
    D2Game::Objects::StaticInit_6caf30();
    D2Trace("staticinit[58] 006caf60");
    D2Game::UNIT::SUnit::StaticInit_6caf60();
    D2Trace("staticinit[59] 006caf70");
    D2Game::GAME::Task::StaticInit_6caf70();
    D2Trace("staticinit[60] 006caf90");
    D2Game::GAME::Task::StaticInit_6caf90();
    D2Trace("staticinit[61] 006cafc0");
    D2CMP::SRC::CelCmp::StaticInit_6cafc0();
    D2Trace("staticinit[62] 006cb010");
    D2Common::Drlg::Preset::StaticInit_6cb010();
    D2Trace("staticinit[63] 006cb030");
    Storm::Source::StaticInit_6cb030();
    D2Trace("staticinit[64] 006cb050");
    Storm::Source::StaticInit_6cb050();
    D2Trace("staticinit[65] 006ca6f0");
    Storm::Source::SSystem::SetCurrentThreadId();
    D2Trace("staticinit all done");
}
