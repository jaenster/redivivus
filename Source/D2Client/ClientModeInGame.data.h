/**
 * Data defined in D2Client/ClientModeInGame.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Points at gCmdLineHelpText ("Available command line options: ..."), the text CmdLine_ShowHelpIfNotDeveloper / CmdLine_SetAppMode show.
// Provenance: .rdata value 0x6d5b20; the loop at 0044b6c8 only reads gCmdLineOptionTable[0] (ESI < 4).
extern char * gpszCmdLineHelpText;

namespace D2Client::ClientModeInGame {
extern char gszCmdLineSwitchP[2];
extern char gszCmdLineSwitchC[2];
extern char gszCmdLineSwitchHelp[5];
extern char gszCmdLineSwitchH[2];
extern char gszCmdLineSwitchQuestionMark[2];
} // namespace D2Client::ClientModeInGame

namespace D2Common::Txt::Cube {
extern char gpCubeRecipeItemTypeFlags_7[4];
} // namespace D2Common::Txt::Cube
