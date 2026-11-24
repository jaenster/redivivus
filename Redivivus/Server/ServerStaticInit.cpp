// Redivivus' own code, not recovered from any Blizzard binary.
//
// The dedicated server's C++ static initialisers: the entries of Game.exe's _initterm
// table (66 at 006cc618) that belong to a module the server links, in the table's order.
//
// Redivivus/d2_static_init.cpp runs all 66 for the game. Entries 16-49 construct state of
// D2Launch, D2Multi, D2Client, D2gfx and D2Win, and 51-55 of Bnclient; none of those is
// linked here. Entry 21 builds the QServer game table, a D2Client file-static; the server
// host builds its own (ServerHost.cpp). See docs/data/cpp-static-init-order.tsv for the
// table and d2_static_init.cpp for why every entry must run.
//
// Same priority as the game's list: after Redivivus/d2_trace.cpp at 150, so a fault in
// one of these still logs.
#include <windows.h>

#include "Source/globals/globals.h"
#include "Redivivus/d2_trace.h"
#include "Source/Fog/Src/LogManager.h"
#include "Source/Storm/Source/SSystem.h"
#include "3rdparty/crashy/crashyRoot.h"
#include "Source/D2Hell/D2HellRoot.h"
#include "Source/D2CMP/Src/CelCmp.h"
#include "Source/D2Common/Drlg/Preset.h"
#include "Source/D2Game/Game/Server.h"
#include "Source/D2Game/Game/Task.h"
#include "Source/D2Game/Objects/Objects.h"
#include "Source/D2Game/Unit/SUnit.h"
#include "Source/Storm/Source/SCMD.h"
#include "Source/Storm/Source/SComp.h"
#include "Source/Storm/Source/SEVT.h"
#include "Source/Storm/Source/SErr.h"
#include "Source/Storm/Source/SFile.h"
#include "Source/Storm/WindowHandle.h"

extern "C" __attribute__((constructor(200))) void D2RunServerStaticInitializers() {
    D2Trace("server staticinit[00] 006ca700");
    Fog::Src::LogManager::StaticInit_gLogMessageQueueCS();
    D2Trace("server staticinit[01] 006ca720");
    Fog::Src::LogManager::StaticInit_gLogFileCS();
    D2Trace("server staticinit[02] 006ca740");
    Fog::Src::LogManager::LOGMGR_InitMessageQueue();
    D2Trace("server staticinit[03] 006ca7b0");
    Storm::Source::SFile::StaticInit_gMpqArchiveList();
    D2Trace("server staticinit[04] 006ca7d0");
    Storm::Source::SFile::StaticInit_gSFileAsyncReqQueue();
    D2Trace("server staticinit[05] 006ca7f0");
    Storm::Source::SFile::StaticInit_CriticalSection_00779068();
    D2Trace("server staticinit[06] 006ca810");
    Storm::Source::SFile::StaticInit_gSFileOpenFileList();
    D2Trace("server staticinit[07] 006ca830");
    Storm::Source::SFile::StaticInit_gSFileAudioStreamList();
    D2Trace("server staticinit[08] 006ca850");
    Storm::Source::SFile::StaticInit_gSFileStreamRecordList();
    D2Trace("server staticinit[09] 006ca870");
    Storm::Source::SFile::StaticInit_CriticalSection_00779080();
    D2Trace("server staticinit[10] 006ca890");
    Storm::Source::SSystem::StaticInit_6ca890();
    D2Trace("server staticinit[11] 006ca8a0");
    Storm::Source::SErr::StaticInit_gStormErrorCallbackList();
    D2Trace("server staticinit[12] 006ca8c0");
    Storm::Source::SComp::StaticInit_CriticalSection_00779214();
    D2Trace("server staticinit[13] 006ca8e0");
    Storm::WindowHandle::StaticInit_gWndTrackingList();
    D2Trace("server staticinit[14] 006ca900");
    Storm::Source::SEVT::StaticInit_gSevtBreakCmdList();
    D2Trace("server staticinit[15] 006ca920");
    Storm::Source::SEVT::StaticInit_CriticalSection_0077930c();
    D2Trace("server staticinit[50] 006cae5f");
    D2Hell::StaticInit_gFaultHandlerList();
    D2Trace("server staticinit[56] 006caf10");
    D2Game::GAME::Server::StaticInit_Frames_TimeBetweenFrames();
    D2Trace("server staticinit[57] 006caf30");
    D2Game::Objects::StaticInit_gaObjectSpawnOffsetTables();
    D2Trace("server staticinit[58] 006caf60");
    D2Game::UNIT::SUnit::StaticInit_gnSeedCounter();
    D2Trace("server staticinit[59] 006caf70");
    D2Game::GAME::Task::StaticInit_CriticalSection_0088caa8();
    D2Trace("server staticinit[60] 006caf90");
    D2Game::GAME::Task::StaticInit_aTaskQueueCriticalSections();
    D2Trace("server staticinit[61] 006cafc0");
    D2CMP::SRC::CelCmp::StaticInit_gCelCmpCacheHandler();
    D2Trace("server staticinit[62] 006cb010");
    D2Common::Drlg::Preset::StaticInit_CriticalSection_0096d610();
    D2Trace("server staticinit[63] 006cb030");
    Storm::Source::StaticInit_gStormCmdListBackslash();
    D2Trace("server staticinit[64] 006cb050");
    Storm::Source::StaticInit_gStormCmdListSlash();
    D2Trace("server staticinit[65] 006ca6f0");
    Fog::_3rdparty::crashy::SetCurrentThreadId();
    D2Trace("server staticinit all done");
}
