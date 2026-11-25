// Redivivus' own code, not recovered from any Blizzard binary.
//
// The game data a server host loads before it brings the QServer up. ApplicationMain
// (Game/Main.cpp, 00405c30) opens the client's archives only outside APPMODE_server, and the
// client's own app mode loads its tables in its run function; the server row does both in
// its run function, through these two calls.
#pragma once

#include "Source/globals/globals.h"     // D2IniConfigStrc

namespace Redivivus::Server {

// Opens the data archives and loads the string and txt tables. False when the install
// cannot be read; whatever was opened is closed again.
bool ServerData_Load(D2IniConfigStrc * pIni);

// Frees the tables and closes the archives ServerData_Load opened.
void ServerData_Free();

}  // namespace Redivivus::Server
