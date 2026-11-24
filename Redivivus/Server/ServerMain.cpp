// Redivivus' own code, not recovered from any Blizzard binary.
//
// The dedicated server's entry point. It does what Game.exe's WinMain, PreInitApplication
// and ApplicationMain do on the way to the APPMODE_server row, minus everything that
// row never needed: no window, no renderer, no sound, no launcher, no client app mode.
//
// The data MPQs are opened with D2Hell's archive functions directly, as Blizzard's 1.00
// D2Server did (Source/D2Server/Core/CharServ.cpp, LoadMPQArchives). ApplicationMain
// skips D2Win's MPQ_LoadAllDataMpqFiles for APPMODE_server, and D2Win is not linked here.
//
// The data then loads in the client app mode's order (fAPPMODE_client_ReturnAppMode,
// 0044b8a0): the Fog memory managers, the server part of LoadDataForGame, the string
// tables, the txt tables. Only after that does the host bring the QServer up, which is
// the order the working d2gs host uses too.
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Source/globals/globals.h"
#include "Source/D2Client/D2IniConfigStrc.h"    // D2IniConfigStrc's layout; the type lives under D2Client
#include "Source/Fog/Fog.h"
#include "Source/D2Lang/D2Lang.h"
#include "Source/D2Common/D2Common.h"
#include "Source/D2Hell/D2Hell.h"
#include "Redivivus/Server/ServerHost.h"
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

// The command line. The game's own parser (MISC_InitD2IniAndIgnorelist and the
// D2IniConfigValues walk, Game/Main.cpp) lives with the client's WinMain and also reads
// D2.ini and the ignore list, so the server parses the few switches a server acts on
// itself. Switches are case-insensitive and may appear in any order.
bool HasSwitch(const char * szCmdLine, const char * szSwitch) {
    const size_t nLen = strlen(szSwitch);
    for (const char * p = szCmdLine; (p = strchr(p, '-')) != nullptr; p++) {
        if (_strnicmp(p + 1, szSwitch, nLen) == 0) {
            const char c = p[1 + nLen];
            if (c == '\0' || c == ' ' || c == '\t') {
                return true;
            }
        }
    }
    return false;
}

const char * SwitchValue(const char * szCmdLine, const char * szSwitch) {
    const size_t nLen = strlen(szSwitch);
    for (const char * p = szCmdLine; (p = strchr(p, '-')) != nullptr; p++) {
        if (_strnicmp(p + 1, szSwitch, nLen) == 0 && (p[1 + nLen] == ' ' || p[1 + nLen] == '\t')) {
            const char * v = p + 1 + nLen;
            while (*v == ' ' || *v == '\t') {
                v++;
            }
            return *v ? v : nullptr;
        }
    }
    return nullptr;
}

void ParseServerCommandLine(const char * szCmdLine, D2IniConfigStrc * pIni) {
    memset(pIni, 0, sizeof(*pIni));
    pIni->bDIRECT = HasSwitch(szCmdLine, "direct");     // read data from the directory before the MPQs
    pIni->bTXT = HasSwitch(szCmdLine, "txt");           // compile the .txt tables instead of loading the .bin
    pIni->bLOG = HasSwitch(szCmdLine, "log");
    pIni->bNOSAVE = HasSwitch(szCmdLine, "nosave");
    pIni->bNOSOUND = true;
    pIni->nGAMETYPE = GAMETYPE_BNET;
    if (const char * v = SwitchValue(szCmdLine, "seed")) {
        pIni->nSEED = atoi(v);
    }
}

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

int RunServer(HINSTANCE hInstance, const char * szCmdLine) {
    D2IniConfigStrc tIni;
    ParseServerCommandLine(szCmdLine, &tIni);

    char szVersion[32];
    snprintf(szVersion, sizeof(szVersion), "v%d.%02d", 1, 0xe);
    Fog::Source::SFile::FILETOOLS_SetLogFilePathWithoutDateAndExtension2((char *)"D2Server");
    Fog::Src::ErrorManager::ERRORMANAGER_InitErrorManager((char *)"Diablo II", szVersion, 1);
    ServerLog("Redivivus dedicated server %s", szVersion);

    // ApplicationMain (00405c30) up to its first APPMODE_server branch.
    Fog::Source::SFile::SFILE_InitializeFileSystem((BOOL)tIni.bDIRECT, 0);
    Fog::Src::AsyncData::STRUCT_AllocAsyncData(1, 0);
    Fog::Return();
    Fog::Src::D2Net::NET_SERVER_SentPacketLog_Clear();

    if (!OpenDataArchives()) {
        ServerLog("the data archives did not open; run from the game directory or pass -direct");
        CloseDataArchives();
        Fog::Src::AsyncData::STRUCT_FreeAsyncData();
        return 1;
    }
    *(BOOL *)tIni.bD2ExpMpqExistsOrLanugageCode = Fog::D2CheckExpansionGame();
    D2Lang::StrTable::strtable::MISC_GetLanguageCode();

    // fAPPMODE_client_ReturnAppMode's data load, without the client's UI and language DLL.
    Fog::Memory::InitializeMemoryManagers(nullptr);
    Fog::Memory::SetPoolIdForMemoryManager(nullptr, 1);
    LoadDataForServer(&tIni);
    const BOOL bExpansion = Fog::D2CheckExpansionGame();
    if (!D2Lang::StrTable::strtable::TABLES_LoadStrings(nullptr, nullptr, bExpansion)) {
        ServerLog("the string tables did not load");
        CloseDataArchives();
        Fog::Src::AsyncData::STRUCT_FreeAsyncData();
        return 1;
    }
    // The last argument is "this process hosts the games". The client passes it only as
    // an open Battle.net host; a dedicated server always is one (d2gs passes 1 as well).
    D2Common::DATATBLS::DataTbls::TXT_InitTxtFiles(nullptr, 0, 1);
    ServerLog("data loaded (%s): %d levels, %d monster types", bExpansion ? "expansion" : "classic",
        D2Common::ITEMS::Items::sgptDataTable->nTxtLevelsSize,
        D2Common::ITEMS::Items::sgptDataTable->nTxtMonStatsSize);

    // The APPMODE_server row: init, table[0], destroy - ApplicationRunCurrentMode (004059d0).
    int nExitCode = 1;
    fnAppModeRun * pTable = ServerHost_Init(hInstance);
    if (pTable && *pTable) {
        nExitCode = (*pTable)(&tIni);
    } else {
        ServerLog("the server host returned no function table");
    }
    ServerHost_Shutdown();

    // ApplicationMain's and fAPPMODE_client_ReturnAppMode's teardown, the server part.
    D2Common::DATATBLS::DataTbls::FreeTables();
    D2Lang::StrTable::strtable::STRINGTABLE_FreeStringTables();
    CloseDataArchives();
    Fog::Src::AsyncData::STRUCT_FreeAsyncData();
    Fog::Memory::FreeMemoryPool(nullptr);
    ServerLog("stopped, exit code %d", nExitCode);
    return nExitCode;
}

}  // namespace
}  // namespace Redivivus::Server

// A console program, so Ctrl-C and the console's close button reach the host.
int main(int, char **) {
    // The raw command line minus the program name, as WinMain's lpCmdLine would be.
    const char * szCmdLine = GetCommandLineA();
    if (*szCmdLine == '"') {
        szCmdLine = strchr(szCmdLine + 1, '"');
        szCmdLine = szCmdLine ? szCmdLine + 1 : "";
    } else {
        while (*szCmdLine && *szCmdLine != ' ' && *szCmdLine != '\t') {
            szCmdLine++;
        }
    }
    return Redivivus::Server::RunServer(GetModuleHandleA(nullptr), szCmdLine);
}
