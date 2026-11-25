// Redivivus' own code, not recovered from any Blizzard binary.
//
// The dedicated server's entry point. d2host is Game.exe started in its server app mode:
// main() hands the command line to the game's own WinMain (Game/Main.cpp, 004066e0), which
// goes through PreInitApplication and ApplicationMain to the APPMODE_server row. Compiled
// with D2_TARGET_SERVER, that row is the server host (ServerHost.h), and ApplicationMain
// leaves out the window, the renderer, the sound and the client's archives.
//
// What is left here is what a console program needs and Game.exe never had: the console
// entry point and the command line in WinMain's form. Ctrl-C and the console's close
// button reach the host through its console handler (ServerHost.cpp).
#include <windows.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include "Redivivus/Server/ServerLog.h"

extern "C" int32_t __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int32_t nShowCmd);

namespace Redivivus::Server {
namespace {

// The raw command line minus the program name, as WinMain's lpCmdLine would be.
const char * RawCommandLine() {
    const char * szCmdLine = GetCommandLineA();
    if (*szCmdLine == '"') {
        szCmdLine = strchr(szCmdLine + 1, '"');
        return szCmdLine ? szCmdLine + 1 : "";
    }
    while (*szCmdLine && *szCmdLine != ' ' && *szCmdLine != '\t') {
        szCmdLine++;
    }
    return szCmdLine;
}

// The game picks its app mode from the command line (-server, -client, ...; the last one
// wins, CLIENT_CheckIfApplicationModeIsInCommandLineAndSetItToArgument at 00406340), and
// d2host has only the server mode, so -server goes last.
std::string ServerCommandLine() {
    std::string sCmdLine = RawCommandLine();
    sCmdLine += " -server";
    return sCmdLine;
}

}  // namespace
}  // namespace Redivivus::Server

int main(int, char **) {
    std::string sCmdLine = Redivivus::Server::ServerCommandLine();
    Redivivus::Server::ServerLog("Redivivus dedicated server, command line:%s", sCmdLine.c_str());
    const int32_t nResult = WinMain(GetModuleHandleA(nullptr), nullptr, &sCmdLine[0], SW_SHOWDEFAULT);
    Redivivus::Server::ServerLog("stopped");
    return nResult;
}
