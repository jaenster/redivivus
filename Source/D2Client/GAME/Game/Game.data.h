/**
 * Data defined in D2Client/GAME/Game/Game.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/globals/globals.h"
#include "Source/Bnclient/BnSend.h"
#include "Source/Bnclient/SRP.h"
#include "Source/Bnclient/net_sid.h"
#include "Source/D2CMP/Draw.h"
#include "Source/D2CMP/SRC/Codec.h"
#include "Source/D2CMP/SRC/LRUCache.h"
#include "Source/D2Client/ClientModeInGame.h"
#include "Source/D2Client/Cursor.h"
#include "Source/D2Client/D2BNetGatewayAccessStr.h"
#include "Source/D2Client/D2GSPacketSrv0x2A.h"
#include "Source/D2Client/D2GfxMemoryStrc.h"
#include "Source/D2Client/D2GfxSettingsStrc.h"
#include "Source/D2Client/D2IniConfigStrc.h"
#include "Source/D2Client/Draw/LightMap.h"
#include "Source/D2Client/Draw/Weather.h"
#include "Source/D2Client/Engine/Gfx.h"
#include "Source/D2Client/GAME/Draw.h"
#include "Source/D2Client/GAME/Game/Game.h"
#include "Source/D2Client/GAME/Record.h"
#include "Source/D2Client/GAME/SCmd.h"
#include "Source/D2Client/GAME/Sound.h"
#include "Source/D2Client/Sound/Sound.h"
#include "Source/D2Client/UI/Automap.h"
#include "Source/D2Client/UI/Chat.h"
#include "Source/D2Client/UI/CmdTbl.h"
#include "Source/D2Client/UI/EscMenu.h"
#include "Source/D2Client/UI/Inv.h"
#include "Source/D2Client/UI/Path.h"
#include "Source/D2Client/UI/QuestLog.h"
#include "Source/D2Client/UI/ShowItems.h"
#include "Source/D2Client/UI/SkillDesc.h"
#include "Source/D2Client/UI/SpellSel.h"
#include "Source/D2Client/UI/Text.h"
#include "Source/D2Client/UI/UseItem.h"
#include "Source/D2Client/UI/Waypoint.h"
#include "Source/D2Client/UI/npcmenu.h"
#include "Source/D2Client/UI/npcmenu/Merc.h"
#include "Source/D2Client/UI/ui.h"
#include "Source/D2Client/UNIT/Item.h"
#include "Source/D2Client/UNIT/Missile.h"
#include "Source/D2Client/UNIT/Monster.h"
#include "Source/D2Client/UNIT/Object.h"
#include "Source/D2Client/UNIT/Player.h"
#include "Source/D2Client/UNIT/PlrSkills.h"
#include "Source/D2Client/UNIT/UnitSnd.h"
#include "Source/D2Client/Warden/WardenClient.h"
#include "Source/D2Common/COMPOSIT/Composit.h"
#include "Source/D2Common/D2SeedStrc.h"
#include "Source/D2Common/DATATBLS/ItemTbls.h"
#include "Source/D2Common/DATATBLS/MonsterTbls.h"
#include "Source/D2Common/DUNGEON/Dungeon.h"
#include "Source/D2Common/Drlg/D2DrlgLevelDataList.h"
#include "Source/D2Common/Drlg/D2DrlgReplaceRoomStrc.h"
#include "Source/D2Common/Drlg/Drlg.h"
#include "Source/D2Common/Drlg/OutSiege.h"
#include "Source/D2Common/INVENTORY/Inventory.h"
#include "Source/D2Common/ITEMS/ItemMods.h"
#include "Source/D2Common/ITEMS/Items.h"
#include "Source/D2Common/Logging/ProfCore.h"
#include "Source/D2Common/Merc.h"
#include "Source/D2Common/Txt/Skills.h"
#include "Source/D2Common/Unit.h"
#include "Source/D2Game/D2Game.globals.h"
#include "Source/D2Game/D2QuestDataStrc.h"
#include "Source/D2Game/GAME/Arena.h"
#include "Source/D2Game/GAME/Task.h"
#include "Source/D2Game/Monster/AI/AI.h"
#include "Source/D2Game/Monster/AI/Baal.h"
#include "Source/D2Game/Monster/MonsterMode.h"
#include "Source/D2Game/Objects/Objects.h"
#include "Source/D2Game/PLAYER/PlayerMsg.h"
#include "Source/D2Game/Quests/QUESTS.h"
#include "Source/D2Game/SKILLS/Items.h"
#include "Source/D2Game/SKILLS/SkillAss.h"
#include "Source/D2Game/UNIT/SUnitDmg.h"
#include "Source/D2Game/UNIT/sunitproxy.h"
#include "Source/D2Hell/D2Hell.h"
#include "Source/D2Lang/D2Lang.h"
#include "Source/D2Lang/StrTable/strtable.h"
#include "Source/D2Win/D2WinAnimImage.h"
#include "Source/D2Win/Src/D2Comp.h"
#include "Source/D2Win/Src/D2WinFont.h"
#include "Source/D2gfx/D2GFX.h"
#include "Source/Fog/D2MemoryManagerStrc.h"
#include "Source/Fog/D2PoolManagerStrc.h"
#include "Source/Fog/D2QServerStrc.h"
#include "Source/Fog/Memory.h"
#include "Source/Fog/Save.h"
#include "Source/Fog/Source/SFile.h"
#include "Source/Fog/Source/SSComp.h"
#include "Source/Fog/Src/LogManager.h"
#include "3rdparty/Storm/3rdparty/zlib/deflate.h"
#include "3rdparty/Storm/H/stpl.h"
#include "3rdparty/Storm/Source/SCMD.h"
#include "3rdparty/Storm/Source/SComp.h"
#include "3rdparty/Storm/Source/SErr.h"
#include "3rdparty/Storm/Source/SMem.h"
#include "3rdparty/Storm/Storm.h"
#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Client/UI/MiniPanel.h"
#include "Source/D2Launch/CharSel.h"

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
