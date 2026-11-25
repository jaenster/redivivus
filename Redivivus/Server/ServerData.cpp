// Redivivus' own code, not recovered from any Blizzard binary.
//
// The server's data load: the archives, then the data in the client app mode's order
// (fAPPMODE_client_ReturnAppMode, 0044b8a0): the Fog memory managers, the server part of
// LoadDataForGame, the string tables, the txt tables.
//
// The data MPQs are opened with D2Hell's archive functions directly, as Blizzard's 1.00
// D2Server did (Source/D2Server/Core/CharServ.cpp, LoadMPQArchives). ApplicationMain
// skips D2Win's MPQ_LoadAllDataMpqFiles for APPMODE_server, and D2Win is not linked here.
#include <windows.h>

#include "Source/globals/globals.h"
#include "Source/D2Client/D2IniConfigStrc.h"    // D2IniConfigStrc's layout; the type lives under D2Client
#include "Source/Fog/Fog.h"
#include "Source/D2Lang/D2Lang.h"
#include "Source/D2Common/D2Common.h"
#include "Source/D2Hell/D2Hell.h"
#include "Redivivus/Server/ServerData.h"
#include "Redivivus/Server/ServerLog.h"

// Game.exe keeps the handles of the archives it opened in D2Win (MPQ_LoadAllDataMpqFiles,
// 004fab90), and code outside D2Win reads them to learn which archives are open:
// Redivivus/d2_patch.cpp checks the levels table against an open d2data.mpq without
// patch_d2.mpq. D2Win is not linked here, so the server, which opens those archives
// itself, holds them under the same names.
namespace D2Win::Src::D2WinArchive {
D2FogArchiveStrc * MPQ_d2data = nullptr;
D2FogArchiveStrc * MPQ_patch_d2 = nullptr;
D2FogArchiveStrc * MPQ_d2exp = nullptr;
}  // namespace D2Win::Src::D2WinArchive

namespace Redivivus::Server {
namespace {

// The data archives a server reads. d2sfx, d2speech and the media archives hold sound,
// speech and graphics; nothing on the server side opens them. Priorities are the ones
// MPQ_LoadAllDataMpqFiles (004fab90) gives each archive.
struct ServerArchive {
    const char * szName;
    int32_t nPriority;
    bool bRequired;
    D2FogArchiveStrc * pArchive;
};

ServerArchive gaServerArchives[] = {
    { "d2data.mpq", 1000, true, nullptr },
    { "d2exp.mpq", 3000, false, nullptr },      // required below when the install is an expansion one
    { "patch_d2.mpq", 5000, false, nullptr },
};

bool gbTablesLoaded = false;

bool OpenDataArchives() {
    const BOOL bDirect = Fog::Source::SFile::GetUseDirectCommand();
    for (ServerArchive & a : gaServerArchives) {
        a.pArchive = D2Hell::ARCHIVE_LoadMPQ((char *)a.szName, 0, nullptr, a.nPriority);
        ServerLog("archive %s: %s", a.szName, a.pArchive ? "open" : "not found");
        if (!a.pArchive && a.bRequired && !bDirect) {
            return false;
        }
    }
    // D2CheckExpansionGame reads the expansion flag Fog took from the install; an expansion
    // install without d2exp.mpq cannot load its tables (the same test as 004fab90).
    if (!bDirect && Fog::D2CheckExpansionGame() && !gaServerArchives[1].pArchive) {
        ServerLog("expansion install, but d2exp.mpq is missing");
        return false;
    }
    D2Win::Src::D2WinArchive::MPQ_d2data = gaServerArchives[0].pArchive;
    D2Win::Src::D2WinArchive::MPQ_d2exp = gaServerArchives[1].pArchive;
    D2Win::Src::D2WinArchive::MPQ_patch_d2 = gaServerArchives[2].pArchive;
    return true;
}

void CloseDataArchives() {
    D2Win::Src::D2WinArchive::MPQ_d2data = nullptr;
    D2Win::Src::D2WinArchive::MPQ_d2exp = nullptr;
    D2Win::Src::D2WinArchive::MPQ_patch_d2 = nullptr;
    for (int i = (int)(sizeof(gaServerArchives) / sizeof(gaServerArchives[0])) - 1; i >= 0; i--) {
        if (gaServerArchives[i].pArchive) {
            D2Hell::ARCHIVE_FreeMpqFile(gaServerArchives[i].pArchive);
            gaServerArchives[i].pArchive = nullptr;
        }
    }
}

// The server part of LoadDataForGame (0044d9f0). Its 52 entries set up the client's
// renderer, window, sound, character and connection; of them only ClientInit_Log
// (0044d840) and ClientInit_UseTxt (0044d9c0) touch the server's tables.
void LoadDataForServer(D2IniConfigStrc * pIni) {
    if (pIni->bLOG) {
        D2Common::Src::LogManager::LOGGING_EnableLogFile(1);
    }
    D2Common::DATATBLS::DataTbls::sgtDataTableSetupIfUseTxt((int32_t)!pIni->bTXT);
}

}  // namespace

bool ServerData_Load(D2IniConfigStrc * pIni) {
    if (!OpenDataArchives()) {
        ServerLog("the data archives did not open; run from the game directory or pass -direct");
        CloseDataArchives();
        return false;
    }
    // What ApplicationMain records after its archive load, outside APPMODE_server.
    *(BOOL *)pIni->bD2ExpMpqExistsOrLanugageCode = Fog::D2CheckExpansionGame();

    // fAPPMODE_client_ReturnAppMode's data load, without the client's UI and language DLL.
    Fog::Memory::InitializeMemoryManagers(nullptr);
    Fog::Memory::SetPoolIdForMemoryManager(nullptr, 1);
    LoadDataForServer(pIni);
    const BOOL bExpansion = Fog::D2CheckExpansionGame();
    if (!D2Lang::StrTable::strtable::TABLES_LoadStrings(nullptr, nullptr, bExpansion)) {
        ServerLog("the string tables did not load");
        CloseDataArchives();
        return false;
    }
    // The last argument is "this process hosts the games". The client passes it only as
    // an open Battle.net host; a dedicated server always is one (d2gs passes 1 as well).
    D2Common::DATATBLS::DataTbls::TXT_InitTxtFiles(nullptr, 0, 1);
    gbTablesLoaded = true;
    ServerLog("data loaded (%s): %d levels, %d monster types", bExpansion ? "expansion" : "classic",
        D2Common::ITEMS::Items::sgptDataTable->nTxtLevelsSize,
        D2Common::ITEMS::Items::sgptDataTable->nTxtMonStatsSize);
    return true;
}

void ServerData_Free() {
    if (!gbTablesLoaded) {
        return;
    }
    // fAPPMODE_client_ReturnAppMode's teardown, the server part, then the archives.
    // ApplicationMain frees the memory pool after its loop.
    D2Common::DATATBLS::DataTbls::FreeTables();
    D2Lang::StrTable::strtable::STRINGTABLE_FreeStringTables();
    CloseDataArchives();
    gbTablesLoaded = false;
}

}  // namespace Redivivus::Server
