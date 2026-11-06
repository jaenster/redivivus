/**
 * Data defined in D2Client/GAME/Game/Game.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations (used only by pointer here)
struct D2EndGameUiState;

extern char string_IpAddressTcpIp[24];
extern char string_IpaddressGameserverBattlenet[24];
extern char RealmName[32];
extern eD2PlayerClassID nD2PlayerClassID;
// Long connection timeout start tick (5 min check in Client_CheckConnectionTimeouts).
extern int gConnectLongTimeoutStartTick;
extern uint32_t gbBloodFlag;
// End-game UI state block (congrats/cutscene flags).
extern D2EndGameUiState gEndGameUiState;
// Beta screen slideshow index (page).
extern uint8_t gBetaScreenIndex;
extern eD2GSGameType eD2GSGameType_007a0610;
extern int D2GS_Connected;
extern BOOL D2CLIENT_ExitVar3;
// Level load in-progress flag: set on unload complete, cleared on load complete.
extern BOOL gLevelLoadInProgress;
extern uint32_t ExitingGame;
extern uint32_t EnteringGame;
extern uint32_t gnCameraWorldDeltaX;
extern uint32_t gnCameraWorldDeltaY;

namespace D2Game::Save {
// lpBuffer parameter of GetUserNameA
// param_1 parameter of wsprintfA
extern char szSaveCharacterName[16];
} // namespace D2Game::Save
