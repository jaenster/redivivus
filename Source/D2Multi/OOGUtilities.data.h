/**
 * Data defined in D2Multi/OOGUtilities.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Multi)
#error "D2Multi header included by a module that may not use D2Multi (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/globals/globals.h"
#include "Source/D2CMP/Draw.h"
#include "Source/D2CMP/SRC/Codec.h"
#include "Source/D2CMP/SRC/LRUCache.h"
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
#include "Source/D2Hell/D2Hell.h"
#include "Source/D2Lang/D2Lang.h"
#include "Source/D2Lang/StrTable/strtable.h"
#include "Source/D2Multi/D2Multi.globals.h"
#include "Source/D2Multi/OOGUtilities.h"
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

// Forward declarations
struct D2ControlStrc;
struct D2GameListEntryStrc;
struct D2WinEditBox;
struct D2WinTextBox;

extern char gszOogReplyCommandBuffer[256];

namespace D2Multi::OOGUtilities {
// Former interior labels: gszLanGameListEntryName=+0x1 gszLanGameListEntryDescription=+0x11 gbOOGLobbyScreenStatePadding=+0x12 gwOOGLobbyScreenState=+0x31 gwGameListCacheEntryPort=+0x33 gdwGameListCacheEntryFlags=+0x37 gPlayerInputState1=+0x6e gbDamageModConfigLoaded=+0xa3a5
extern D2GameListEntryStrc gbLanGameListEntryNamePadding2[1000];
// Former interior labels: gaLanGameListEntryName=+0x1 gChatBufferData3=+0x2 gChatBufferData2=+0x3 gwLanGameListEntryPortPrev=+0x31 gnLanGameListEntryPort=+0x33 gnLanGameIPAddr=+0x37 gChatBufferData1=+0x3c gSaveFileBuffer=+0xbd2c gbSaveKeyBindingsFlag=+0xc12b gbOOGUnusedFlag_797261=+0xce21 gbUnusedOOGDataFlag=+0xd029 gbOOGUnusedFlag_79746c=+0xd02c
extern D2GameListEntryStrc gbLanGameListEntryNamePadding[1000];
extern char gszOOGRequestedGameName[256];
extern ::D2WinTextBox * gpOOGGameListTextBox;
extern ::D2WinTextBox * gpOogGameListDetailsPanelControl;
extern D2ControlStrc * gpOOGJoinGameButton;
extern D2ControlStrc * gpCtlGameListStatus;
extern D2ControlStrc * gpOOGLobbyControl2;
extern ::D2WinEditBox * gpOogGamePasswordEditControl;
extern ::D2WinEditBox * gpCtlGameListTop;
extern ::D2WinTextBox * gpCtlJoinGameLabel;
extern ::D2WinTextBox * pUIGameUsername;
extern ::D2WinTextBox * pUIGamePassword;
extern ::D2WinTextBox * pUIGamePlayers;
extern D2ControlStrc * gpCtlCreateGameDlgBackground;
extern ::D2WinTextBox * gpCtlCreateGameDlgTitle;
extern ::D2WinTextBox * gpCtlGameNameLabelOrQueueStatus;
extern ::D2WinTextBox * gpCtlCreateGamePasswordLabel;
extern ::D2WinTextBox * gUIGameListServerMessageText;
extern ::D2WinTextBox * gUIGameListMaxPlayersLabel;
extern ::D2WinTextBox * gUIGameListLevelRestrictionLabel;
extern ::D2WinTextBox * gUIGameListLevelRestrictionLevelsLabel;
extern D2ControlStrc * gUIGameListMaxPlayersInactiveImage;
extern D2ControlStrc * gUIGameListMaxPlayersInactiveArrowButton;
extern D2ControlStrc * gUIGameListMaxPlayersUpButton;
extern D2ControlStrc * gUIGameListLevelRestrictionUpButton;
extern D2ControlStrc * gUIGameListLevelRestrictionInactiveArrowButton;
extern D2ControlStrc * gUIGameListCreateButton;
extern ::D2WinEditBox * gpOogGameNameEditControl;
extern ::D2WinEditBox * gUIGameListGameNameEditBox;
extern ::D2WinEditBox * gUIGameListGamePasswordEditBox;
extern ::D2WinEditBox * gUIGameListMaxPlayersEditBox;
// Edit box control handle for level restriction field in Create Game dialog (TCP/IP mode only). Populated in OOGUTIL_OpenCreateGameDialog, read by NET_MCP_CLIENT_PrepareGameCreatePacket. Part of Create Game UI state cluster at 0x00798c28-0x00798c64.
extern ::D2WinEditBox * gUILevelRestrictionEditBox;
extern D2ControlStrc * gUIGameListLevelRestrictionInactiveImage;
extern D2ControlStrc * gpCtlCreateGameDlgCancelButton;
extern ::D2WinTextBox * gUIGameListStatusLabel;
extern ::D2WinTextBox * gUIGameListPlayerListText;
extern ::D2WinTextBox * gUIGameListActLabel;
extern D2ControlStrc * gpProfileViewBackground;
extern ::D2WinTextBox * gpProfileViewAccountName;
extern ::D2WinTextBox * gpProfileViewSex;
extern ::D2WinTextBox * gpProfileViewLocation;
extern ::D2WinTextBox * gpProfileViewDescription;
extern D2ControlStrc * gpProfileEditBackground;
extern ::D2WinTextBox * gpProfileEditAccountName;
extern ::D2WinEditBox * gpProfileEditSex;
extern ::D2WinEditBox * gpProfileEditLocation;
extern ::D2WinEditBox * gpProfileEditDescription;
extern D2ControlStrc * gpProfileCloseButton;
extern ::D2WinTextBox * gUIPlayerProfileCharacterNameLabel;
extern ::D2WinTextBox * gUIPlayerProfileCharacterLevelLabel;
extern ::D2WinTextBox * gUIPlayerProfileCharacterClassLabel;
extern ::D2WinTextBox * gUIPlayerProfileExperienceLabel;
extern D2ControlStrc * gpProfileOKButton;
extern D2ControlStrc * gpProfileLadderButton;
extern ::D2WinTextBox * gUIPlayerProfilePlaytimeLabel;
extern int32_t gnProfileDialogMode;
extern ::D2WinTextBox * gpChatCommandsTitle;
extern ::D2WinTextBox * gpChatCommandsTextBox;
extern D2ControlStrc * gpChatCommandsOKButton;
extern D2ControlStrc * gpChatCommandsBackground;
extern uint32_t gnGameListFilterState;
extern uint32_t gnMCPCreateGamePendingFlag;
extern int gnFilteredGameListCount;
extern int nGameListDifficultyFilter;
extern uint32_t CreateGameDifflevel;
extern char gszProfileCharacterName[16];
extern int gnProfileUserHardcoreFlag;
extern int gnProfileUserExpansionFlag;
} // namespace D2Multi::OOGUtilities
