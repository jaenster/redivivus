/**
 * Auto-generated global data declarations
 * Binary: Game.exe
 * 
 * This file contains extern declarations for global data symbols
 * that are referenced by multiple functions.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include "d2_platform.h"
#include "Bnclient/BnSend.h"
#include "Bnclient/SRP.h"
#include "Bnclient/net_sid.h"
#include "D2CMP/Draw.h"
#include "D2CMP/SRC/Codec.h"
#include "D2CMP/SRC/LRUCache.h"
#include "D2Client/ClientModeInGame.h"
#include "D2Client/Cursor.h"
#include "D2Client/D2BNetGatewayAccessStr.h"
#include "D2Client/D2Client.h"
#include "D2Client/D2GSPacketSrv0x2A.h"
#include "D2Client/D2GfxMemoryStrc.h"
#include "D2Client/D2GfxSettingsStrc.h"
#include "D2Client/D2IniConfigStrc.h"
#include "D2Client/Draw/LightMap.h"
#include "D2Client/Draw/Weather.h"
#include "D2Client/Engine/Gfx.h"
#include "D2Client/GAME/Draw.h"
#include "D2Client/GAME/Game/Game.h"
#include "D2Client/GAME/Record.h"
#include "D2Client/GAME/SCmd.h"
#include "D2Client/GAME/Sound.h"
#include "D2Client/Renderer/Glide.h"
#include "D2Client/SKILLS/SkillsPal.h"
#include "D2Client/Sound/Sound.h"
#include "D2Client/UI/Automap.h"
#include "D2Client/UI/Chat.h"
#include "D2Client/UI/CmdTbl.h"
#include "D2Client/UI/EscMenu.h"
#include "D2Client/UI/Inv.h"
#include "D2Client/UI/MiniPanel.h"
#include "D2Client/UI/Panel.h"
#include "D2Client/UI/Path.h"
#include "D2Client/UI/QuestLog.h"
#include "D2Client/UI/SkillDesc.h"
#include "D2Client/UI/SpellSel.h"
#include "D2Client/UI/Text.h"
#include "D2Client/UI/UseItem.h"
#include "D2Client/UI/Waypoint.h"
#include "D2Client/UI/npcmenu.h"
#include "D2Client/UI/npcmenu/Merc.h"
#include "D2Client/UI/ui.h"
#include "D2Client/UNIT/Item.h"
#include "D2Client/UNIT/Missile.h"
#include "D2Client/UNIT/Monster.h"
#include "D2Client/UNIT/Object.h"
#include "D2Client/UNIT/Player.h"
#include "D2Client/UNIT/UnitSnd.h"
#include "D2Client/Warden/WardenClient.h"
#include "D2Common/COMPOSIT/Composit.h"
#include "D2Common/D2SeedStrc.h"
#include "D2Common/DATATBLS/MonsterTbls.h"
#include "D2Common/DUNGEON/Dungeon.h"
#include "D2Common/Drlg/D2DrlgLevelDataList.h"
#include "D2Common/Drlg/D2DrlgReplaceRoomStrc.h"
#include "D2Common/Drlg/Drlg.h"
#include "D2Common/Drlg/OutSiege.h"
#include "D2Common/INVENTORY/Inventory.h"
#include "D2Common/ITEMS/ItemMods.h"
#include "D2Common/ITEMS/Items.h"
#include "D2Common/Logging/ProfCore.h"
#include "D2Common/Unit.h"
#include "D2Direct3D/Renderer/Direct3D.h"
#include "D2Direct3D/d3dFloorCache.h"
#include "D2Direct3D/d3dSubtileCache.h"
#include "D2Direct3D/d3dTextureCache.h"
#include "D2Game/D2QuestDataStrc.h"
#include "D2Game/GAME/Arena.h"
#include "D2Game/GAME/Task.h"
#include "D2Game/Monster/AI/AI.h"
#include "D2Game/Monster/AI/Baal.h"
#include "D2Game/Monster/MonsterMode.h"
#include "D2Game/Objects/Objects.h"
#include "D2Game/PLAYER/PlayerMsg.h"
#include "D2Game/Quests/QUESTS.h"
#include "D2Game/SKILLS/Items.h"
#include "D2Game/SKILLS/SkillAss.h"
#include "D2Game/UNIT/SUNITNPC.h"
#include "D2Game/UNIT/SUnitDmg.h"
#include "D2Game/UNIT/sunitproxy.h"
#include "D2Glide/glFloorCache.h"
#include "D2Glide/glSubtileCache.h"
#include "D2Lang/D2Lang.h"
#include "D2Lang/StrTable/strtable.h"
#include "D2Launch/CharSel.h"
#include "D2Launch/MainMenus.h"
#include "D2Multi/OOGUtilities.h"
#include "D2Win/D2WinAnimImage.h"
#include "D2Win/Src/D2Comp.h"
#include "D2Win/Src/D2WinFont.h"
#include "D2gfx/D2GFX.h"
#include "Fog/D2MemoryManagerStrc.h"
#include "Fog/D2PoolManagerStrc.h"
#include "Fog/D2QServerStrc.h"
#include "Fog/Engine/Application.h"
#include "Fog/Memory.h"
#include "Fog/SMem.h"
#include "Fog/Save.h"
#include "Fog/Source/SFile.h"
#include "Fog/Source/SSComp.h"
#include "Fog/Src/LogManager.h"
#include "Storm/3rdparty/zlib/deflate.h"
#include "Storm/3rdparty/zlib/trees.h"
#include "Storm/Source/SCMD.h"
#include "Storm/Source/SErr.h"
#include "Storm/Source/SSystem.h"
#include "Storm/Storm.h"
#include "Storm/_unnamespaced.h"
#include "Storm/H/stpl.h"

// Forward declarations
struct ARGLIST;
struct BetaScreenEntry;
struct BnMessQueue;
struct ButtonWrapper;
struct CMDPARAMS;
struct D2Act5SpecialPresetStrc;
struct D2AiDispatchCallbacksStrc;
struct D2AiListStrc;
struct D2AiParamStrc;
struct D2AllStatesDataTbl;
struct D2AnimDataStrc;
struct D2ArchiveStrc;
struct D2ArenaTxt;
struct D2ArmTypeTxt;
struct D2AsyncDataStrc;
struct D2AsyncData_Line0x128;
struct D2AttackSpeedStringStrc;
struct D2AuraEnchantedStrc;
struct D2AutoMapLvlTbls;
struct D2AutomapLayerStrc;
struct D2AutomapRelatedTxt;
struct D2BNetClientCallbacksStrc;
struct D2BNetGatewayAccessStr;
struct D2BNetRealmEntryStrc;
struct D2BaalFxSpiritSpawnStrc;
struct D2BattleNetEventCallbackTable;
struct D2BeltsTxt;
struct D2BitBufferStrc;
struct D2BnCacheFileHeaderStrc;
struct D2BnNewsTableStrc;
struct D2BodyLocsTxt;
struct D2BooksTableStrc;
struct D2BooksTxt;
struct D2ChannelUserStrc;
struct D2CharSelCompStrc;
struct D2CharSelStrc;
struct D2CharStatsTxt;
struct D2CharTemplateTxt;
struct D2ChatCommandStrc;
struct D2ClassIconWithNameStrc;
struct D2ClientGameDataStrc;
struct D2ClientSoundChannelStrc;
struct D2ClientStrc;
struct D2ClientTimerStrc;
struct D2CodecFrameComponentStrc;
struct D2CodecPixelBufferEntryStrc;
struct D2CofCacheStrc;
struct D2ColorsTxt;
struct D2CompCharacterCompositeItemStrc;
struct D2CompCodeTxt;
struct D2CompositListStrc;
struct D2CompositTxt;
struct D2ConfigControlDescStrc;
struct D2ControlMsg;
struct D2ControlStrc;
struct D2CoordStrc;
struct D2CrashReportStrc;
struct D2CubeMainTxt;
struct D2CursorDataStrc;
struct D2D3DDeviceInfoStrc;
struct D2D3DDeviceSlotStrc;
struct D2D3DVertexStrc;
struct D2DamageStrc;
struct D2DataArrayStrc;
struct D2DataTableTxtStrc;
struct D2DialogStrc;
struct D2DifficultyLevelsTxt;
struct D2DrawFloorStrc;
struct D2DrlgActEnvironmentData;
struct D2DrlgActStrc;
struct D2DrlgFileCacheNodeStrc;
struct D2DrlgLevelDataList;
struct D2DrlgLevelPlacementStrc;
struct D2DrlgLevelStrc;
struct D2DrlgPresetTypeTablesStrc;
struct D2DrlgReplaceRoomStrc;
struct D2DrlgRoomCoordsStrc;
struct D2ElemDmgStatTripleStrc;
struct D2ElemTypesTxt;
struct D2EscMenuHeaderStrc;
struct D2EscMenuItemStrc;
struct D2EscMenuUIFlagStrc;
struct D2EventsTxt;
struct D2ExpansionLvlTblsTxt;
struct D2ExperienceTxt;
struct D2FogArchiveStrc;
struct D2FontCacheEntryStrc;
struct D2FontGlyphStrc;
struct D2FontTableHeaderStrc;
struct D2GFXWindowPlacementStrc;
struct D2GSPacketSrv0x27_TextStrc;
struct D2GSPacketSrv0x2A;
struct D2GambleTableStrc;
struct D2GameListEntryStrc;
struct D2GamePalShiftGreenBloodStrc;
struct D2GameRecordFileHeaderStrc;
struct D2GameRecordStrc;
struct D2GameViewStrc;
struct D2Game_Line0x546 {
    D2Game_Line0x546() = default;
    D2Game_Line0x546(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2Game_Line0x546(int a0, int a1) : nInitSeed(a0), nAutomap(a1) {}

    /* 0x00 */ int nInitSeed;
    /* 0x04 */ int nAutomap;
};
struct D2GemsTxt;
struct D2GfxHelperStrc;
struct D2GfxMemoryStrc;
struct D2GfxSettingsStrc;
struct D2GlideVertexStrc;
struct D2GlideVertexTexCacheBlockStrc;
struct D2GlideWallCornerStrc;
struct D2GridScreenCoordinates;
struct D2GroundItemLabelStrc;
struct D2HighResTimestampStrc;
struct D2HireDescTxt;
struct D2HirelingStatLabelStrc;
struct D2HirelingTxt;
struct D2HitClassTxt;
struct D2IniConfigStrc;
struct D2IniConfigValuesStrc;
struct D2InventoryEntryStrc;
struct D2InventoryScreenCoordinates;
struct D2InventorySizesStrc;
struct D2InventoryStrc;
struct D2InventoryTxt;
struct D2ItemRatioTxt;
struct D2ItemSpellStrc;
struct D2ItemStatCostTxt;
struct D2ItemTypeDefaultSlotStrc;
struct D2ItemTypesTxt;
struct D2ItemWeaponClassCodeStrc;
struct D2ItemsEventMethodStrc;
struct D2ItemsTableStrc;
struct D2ItemsTxt;
struct D2KeyBindStrc;
struct D2LRUCacheNodeStrc;
struct D2LRUCacheStrc;
struct D2LangTableEntryStrc;
struct D2LevelDefsTxt;
struct D2LevelsTxt;
struct D2LightMapCellStrc;
struct D2LightMapStrc;
struct D2LinkNodeStrc;
struct D2LobbyControlsStrc;
struct D2LowQualityItemsTableStrc;
struct D2LowQualityItemsTxt;
struct D2LvlMazeTxt;
struct D2LvlPrestTxt;
struct D2LvlSubTxt;
struct D2LvlTblsTxt;
struct D2LvlTypesTxt;
struct D2LvlWarpTxt;
struct D2MagicAffixTableStrc;
struct D2MagicAffixTxt;
struct D2McpGameInfoStrc;
struct D2MemoryManagerStrc;
struct D2MercBaseStrc;
struct D2MercDifficultyStrc;
struct D2MercForHire;
struct D2MercQuaternaryStrc;
struct D2MercQuinaryStrc;
struct D2MercSecondaryStrc;
struct D2MercTertiaryStrc;
struct D2MercWeaponAnimMapStrc;
struct D2MissCalcTxt;
struct D2Missile031SpreadStrc;
struct D2Missile032SpreadStrc;
struct D2MissileSrvDoFuncData;
struct D2MissilesTxt;
struct D2MonAiTxt;
struct D2MonEquipTxt;
struct D2MonItemPercentTxt;
struct D2MonLvlTxt;
struct D2MonModeTxt;
struct D2MonModeTxtSmall;
struct D2MonMsgAnimModeStrc;
struct D2MonPlaceTxt;
struct D2MonPresetTxt;
struct D2MonPresetTxtPlace;
struct D2MonPropTxt;
struct D2MonSeqMonsterTbls;
struct D2MonSeqTxt;
struct D2MonSoundsTxt;
struct D2MonStats2Txt;
struct D2MonStatsTxt;
struct D2MonTypeTxt;
struct D2MonUModTxt;
struct D2MonsterAiParameterStruct;
struct D2MonsterAnimModeFlagsStrc;
struct D2MonsterCofStatusEntryStrc;
struct D2MonsterEquipEntryStrc;
struct D2MonsterForEachStruct;
struct D2MonsterKillStrc;
struct D2MonsterUModHandlerRowStrc;
struct D2NET_CLIENT_D2GS_ServerToClient;
struct D2NPCMessageTableStrc;
struct D2NPCSpeechStrc;
struct D2NpcCashTabStrc;
struct D2NpcGreetingSoundsStrc;
struct D2NpcItemTablesStrc;
struct D2NpcMenuOptions;
struct D2NpcQuestCallbackEntry;
struct D2NpcShopButtonStrc;
struct D2NpcShopTabStrc;
struct D2NpcTxt;
struct D2ObjGroupTxt;
struct D2ObjModeTxt;
struct D2ObjTypeTxt;
struct D2ObjectInitFnArg;
struct D2ObjectOperateFnArg;
struct D2ObjectPopulateEntryStrc;
struct D2ObjectSpawnOffsetGroupStrc;
struct D2ObjectsTxt;
struct D2OnePacketClientStrc;
struct D2OverlayTxt;
struct D2PacketsClientStrc;
struct D2PaletteEntryStrc;
struct D2ParsedTreasureClassStrc;
struct D2PassiveSkillsPerClassStrc;
struct D2PathDirectionVectorStrc;
struct D2PathTypeArg;
struct D2PetTypeTxt;
struct D2PlayerClassTxt;
struct D2PlayerModeAttribsStrc;
struct D2PlayerModeFnPairStrc;
struct D2PlayerModeNetMsgStrc;
struct D2PlrModeTxt;
struct D2PlrTypeModeTxt;
struct D2PoolBlockEntryStrc;
struct D2PoolBlockStrc;
struct D2ProfilingDumpFrequency;
struct D2PropertiesTxt;
struct D2QSSocketStrc;
struct D2QServerStrc;
struct D2QualityItemsTableStrc;
struct D2QualityItemsTxt;
struct D2QuestAreaIntroStrc;
struct D2QuestDataStrc;
struct D2QuestInitDataExtended;
struct D2QuestLogInfoStrc;
struct D2QuestLogPanelRectStrc;
struct D2QuestLogStrc;
struct D2RareAffixTableStrc;
struct D2RareAffixTxt;
struct D2RealmCharListCacheEntryStrc;
struct D2RendererCoreStrc;
struct D2RendererFunctionsStrc;
struct D2ResistTableEntry;
struct D2RoomExStrc;
struct D2RoomStrc;
struct D2RosterPartyListStrc;
struct D2RosterPetStrc;
struct D2RosterStrc;
struct D2RunesTxt;
struct D2RunesTxtTableStrc;
struct D2SFileSignatureKeyStrc;
struct D2SLogStrc;
struct D2SSCompDeflateBlockEntryStrc;
struct D2SaveFileHeaderStrc;
struct D2ScreenMessageListStrc;
struct D2SeedStrc;
struct D2SetItemsTxt;
struct D2SetsTxt;
struct D2SetsTxtFStrc;
struct D2ShrinesTableEntryStrc;
struct D2ShrinesTxt;
struct D2SkillCalcTxt;
struct D2SkillDescTxt;
struct D2SkillStrc;
struct D2SkillsHeaderTxt;
struct D2SkillsPerClassCounter;
struct D2SkillsTxt;
struct D2SoundChannelNodeStrc;
struct D2SoundChannelStrc;
struct D2SoundEnvironTxt;
struct D2SoundSmpStrc;
struct D2SoundStringKeyStrc;
struct D2SoundsTxt;
struct D2SoundsTxtSmall;
struct D2SpeechBankStrc;
struct D2SpellSelGridRegionStrc;
struct D2SpriteCache;
struct D2SpriteCacheEntryStrc;
struct D2Srp;
struct D2SrpCredentialsStrc;
struct D2StatDescRowStrc;
struct D2StatScreenButtonStrc;
struct D2StatesDataTbls;
struct D2StatesTxt;
struct D2StorePageTxt;
struct D2StormAsyncOpStrc;
struct D2StormListHead;
struct D2StringTableTblFileStrc;
struct D2SuperUniquesTxt;
struct D2TaskQueueSlotStrc;
struct D2TextBoxStrc;
struct D2TileProjectStrc;
struct D2TombCanyonPresetRegionStrc;
struct D2TooltipTextStrc;
struct D2TxtLinkStrc;
struct D2UIConfigTabEntryStrc;
struct D2UIFlagInitStrc;
struct D2UIKeyBindingSetStrc;
struct D2UIMouseMsgStrc;
struct D2UberBossImmunityStrc;
struct D2UiPartyStrc;
struct D2UniqueCandidatePoolStrc;
struct D2UniqueItemsTxt;
struct D2UniqueNameTablesStrc;
struct D2UnitStatDiminishedEntryStrc;
struct D2UnitStrc;
struct D2Utf8DecodeRowStrc;
struct D2WardenModuleStrc;
struct D2WarpChanceStrc;
struct D2WaypointListEntryStrc;
struct D2WaypointStrc;
struct D2WeatherLineOffsetStrc;
struct D2WinButton;
struct D2WinEditBox;
struct D2WinImage;
struct D2WinPopup;
struct D2WinSmack;
struct D2WinTextBox;
struct D2WinTimer;
struct D2ZlibStaticDTreeDistCodeStrc;
struct D2ddDebugFont;
struct D3DFloorCacheNodeStrc;
struct D3DFloorCacheStrc;
struct D3DSubtileCacheNodeStrc;
struct D3DSubtileCacheStrc;
struct D3DTextureCacheNodeStrc;
struct D3DTextureCacheSetStrc;
struct DC6;
struct DC6Block;
struct EditBoxWrapper;
struct FORMS_AnimFrameStrc;
struct FORMS_PredefinedFormStrc;
struct GlFloorCacheNodeStrc;
struct GlFloorCacheStrc;
struct GlSubtileCacheNodeStrc;
struct GlSubtileCacheStrc;
struct IDirect3DDevice;
struct IDirect3DInterface7;
struct IDirect3DTexture2;
struct IDirect3DVertexBuffer;
struct IDirect3DViewport3;
struct IDirectDraw1;
struct IDirectDraw;
struct IDirectDrawGammaControl;
struct IDirectDrawPalette;
struct IDirectDrawSurface1;
struct IDirectDrawSurface7;
struct IDirectSound3DBuffer;
struct IDirectSound;
struct IDirectSoundBuffer;
struct IKsPropertySet;
struct IgnoreList;
struct ListBoxWrapper;
struct SErrAppFatalContextStrc;
struct SGAMEDATA;
struct SMemHeapStatsStrc;
struct ScreenMessage_t;
struct SpinnerWrapper;
struct StormLinkedListStrc;
struct WardenClient;
struct WardenClientVtable;
struct ct_data;
struct deflate_state;
struct static_tree_desc;
typedef BOOL (__fastcall *D2CubeSpecialOutput)(::SGAMEDATA * pGame, D2UnitStrc * pUnit);
typedef BOOL (__fastcall *D2ItemSpellDestroy)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2UnitStrc * pItem, D2UnitStrc * pTarget, int nX, int nY, eD2Skills eD2Skill);
typedef BOOL (__fastcall *D2ItemSpellInit)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2UnitStrc * pItem, D2UnitStrc * pTarget, int nX, int nY, eD2Skills eD2Skill);
typedef BOOL (__fastcall *D2ItemStatCostItemEventFunc)(::SGAMEDATA * pGame, uint32_t nUnitEvent, D2UnitStrc * pAttacker, D2UnitStrc * pTarget, D2DamageStrc * pDamage, eD2Skills eSkill, int nSkillLvl);
typedef BOOL (__fastcall *D2MissileSrvHitFunc)(::SGAMEDATA * pGame, D2UnitStrc * pMissile, D2UnitStrc * pTarget);
typedef BOOL (__fastcall *D2NET_CLIENT_MCPtoCLT)(char * pBytes, size_t nSize);
typedef BOOL (__fastcall *D2NET_CLIENT_SIDtoCLT)(char * pBytes, size_t nSize);
typedef BOOL (__fastcall *D2ObjectOperateFnFunc)(D2ObjectOperateFnArg * pOperation);
typedef BOOL (__fastcall *D2SkillCltDoFunc)(D2UnitStrc * pUnit, eD2Skills eD2Skill, int nSkillLvl);
typedef BOOL (__fastcall *D2SkillCltStFunc)(D2UnitStrc * pUnit, eD2Skills eD2Skill, int nSkillLvl);
typedef BOOL (__fastcall *D2SkillSrvDoFunc)(::SGAMEDATA * pGame, D2UnitStrc * pAttacker, eD2Skills eD2Skill, int nSkillLvl);
typedef BOOL (__fastcall *D2SkillSrvStFunc)(::SGAMEDATA * pGame, D2UnitStrc * pAttacker, eD2Skills eD2Skill, int nSkillLvl);
typedef BOOL (__fastcall *fnCacheReadData)(char * szKey, int * pCrc, void * ppBuf, int * pSize);
typedef BOOL (__fastcall *fnCacheWriteData)(char * szKey, void * pData, uint32_t nSize, int * pTime, uint32_t nExp, int nCacheExp);
typedef BOOL (__fastcall *fnD2DropHandler)(::SGAMEDATA * pGame, D2MonsterKillStrc * pMonsterKill);
typedef BOOL (__fastcall *fnGetEnterChatFlags)(uint32_t * pFlags);
typedef BOOL (__fastcall *fnSendChangeEmail)(char * szAcct, char * szOld, char * szNew);
typedef BOOL (__fastcall *fnSendResetPassword)(char * szAcct, char * szEmail);
typedef BOOL (__fastcall *fnWaitForChangePassword)(uint32_t * pOut);
typedef BOOL (__fastcall *fnWaitForCreateAccount)(uint32_t * pOut);
typedef BOOL (__fastcall *fpDrawGroundTile)(int a1, int a2, int a3, int a4);
typedef BOOL (__fastcall *fpLevelDataEntry)(D2DrlgLevelPlacementStrc * pLevelUnknown);
typedef BOOL (__fastcall *pfnComCallback)(uint32_t dwCbArg1, uint32_t dwCbArg2, char * szText1, char * szText2);
typedef BOOL (__stdcall *OnPress)(D2ControlMsg * pControlMsg);
typedef BOOL (__stdcall *SCMDCALLBACK)(CMDPARAMS * pParams, char * szCmdLine);
typedef BOOL (__stdcall *fnCmdLineOptionInit)();
typedef BOOL (__stdcall *fnDisconnect)();
typedef BOOL (__stdcall *fnIsSocketConnected)();
typedef BOOL (__stdcall *fnNewsShutdown)();
typedef BOOL (__stdcall *fnWaitForLogonResponse)();
typedef BOOL (__stdcall *pfnEnumerateLoadedModules64)(HANDLE hProcess, void * EnumLoadedModulesCallback, void * UserContext);
typedef BOOL (__stdcall *pfnEscMenuItemVisible)();
typedef BOOL (__stdcall *pfnMiniDumpWriteDump)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, DWORD DumpType, void * ExceptionParam, void * UserStreamParam, void * CallbackParam);
typedef BOOL (__stdcall *pfnStackWalk64)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, void * StackFrame, void * ContextRecord, void * ReadMemoryRoutine, void * FunctionTableAccessRoutine, void * GetModuleBaseRoutine, void * TranslateAddress);
typedef BOOL (__stdcall *pfnSymCleanup)(HANDLE hProcess);
typedef BOOL (__stdcall *pfnSymEnumerateModules64)(HANDLE hProcess, void * EnumModulesCallback, void * UserContext);
typedef BOOL (__stdcall *pfnSymFindFileInPath)(HANDLE hProcess, char * SearchPath, char * FileName, void * id, DWORD two, DWORD three, DWORD flags, char * FoundFile, void * callback, void * context);
typedef BOOL (__stdcall *pfnSymGetLineFromAddr)(HANDLE hProcess, DWORD dwAddr, void * pdwDisplacement, void * pLine);
typedef BOOL (__stdcall *pfnSymGetLineFromAddr64)(HANDLE hProcess, DWORD qwAddrLow, DWORD qwAddrHigh, DWORD * pdwDisplacement, void * pLine64);
typedef BOOL (__stdcall *pfnSymGetModuleInfo)(HANDLE hProcess, DWORD dwAddr, void * pModuleInfo);
typedef BOOL (__stdcall *pfnSymGetModuleInfo64)(HANDLE hProcess, DWORD qwAddrLow, DWORD qwAddrHigh, void * pModuleInfo);
typedef BOOL (__stdcall *pfnSymGetSymFromAddr)(HANDLE hProcess, DWORD dwAddr, void * pDisplacement, void * pSymbol);
typedef BOOL (__stdcall *pfnSymGetSymFromAddr64)(HANDLE hProcess, DWORD AddressLow, DWORD AddressHigh, void * pDisplacement, void * pSymbol);
typedef BOOL (__stdcall *pfnSymInitialize)(HANDLE hProcess, char * UserSearchPath, BOOL fInvadeProcess);
typedef BOOL (__stdcall *pfnSymSetSearchPath)(HANDLE hProcess, char * SearchPath);
typedef BOOL (__stdcall *pfnSymUnloadModule)(HANDLE hProcess, DWORD BaseOfDll);
typedef BOOL (__stdcall *pfnSymUnloadModule64)(HANDLE hProcess, DWORD BaseOfDllLow, DWORD BaseOfDllHigh);
typedef D2FontGlyphStrc * (__fastcall *pfnFontGetGlyphEntry)(uint16_t wCharCode);
typedef D2RoomExStrc * (__fastcall *fpDrlgMazeFindEdgeRoom)(D2DrlgLevelStrc * pLevel);
typedef D2UnitStrc * (__fastcall *D2InitUnitObject)(::SGAMEDATA * pGame, D2RoomStrc * pRoom, int32_t nMonStatsId, int32_t nPosX, int32_t nPosY, BOOL bUseMonStats);
typedef DWORD (__stdcall *fnGetUnknownFlag)();
typedef DWORD (__stdcall *fnSendKeepAlive)();
typedef DWORD (__stdcall *pfnSymGetModuleBase)(HANDLE hProcess, DWORD dwAddr);
typedef DWORD (__stdcall *pfnSymGetOptions)();
typedef DWORD (__stdcall *pfnSymLoadModule)(HANDLE hProcess, HANDLE hFile, char * ImageName, char * ModuleName, DWORD BaseOfDll, DWORD SizeOfDll);
typedef DWORD (__stdcall *pfnSymSetOptions)(DWORD SymOptions);
typedef DWORD (__stdcall *pfnUnDecorateSymbolName)(char * DecoratedName, char * UnDecoratedName, DWORD UndecoratedLength, DWORD Flags);
typedef FxBool (__stdcall *grLfbLock)(GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t * info);
typedef FxBool (__stdcall *grLfbUnlock)(GrLock_t type, GrBuffer_t buffer);
typedef FxBool (__stdcall *grSstWinClose)(GrContext_t context);
typedef FxU32 (__stdcall *grGet)(FxU32 pname, FxU32 plength, FxI32 * params);
typedef FxU32 (__stdcall *grTexMaxAddress)(GrChipID_t tmu);
typedef FxU32 (__stdcall *grTexMinAddress)(GrChipID_t tmu);
typedef FxU32 (__stdcall *grTexTextureMemRequired)(FxU32 evenOdd, GrTexInfo * info);
typedef FxU32 (__stdcall *pfnGlideDrawVertexArray)(FxU32 mode, FxU32 Count, void * pointers);
typedef FxU32 (__stdcall *pfnGlideGammaCorrectionRGB)(FxFloat red, FxFloat green, FxFloat blue);
typedef GrContext_t (__stdcall *grSstWinOpen)(FxU32 hWnd, GrScreenResolution_t screen_resolution, GrScreenRefresh_t refresh_rate, GrColorFormat_t color_format, GrOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers);
typedef GrProc (__stdcall *grGetProcAddress)(char * procName);
typedef LRESULT (__stdcall *WndProcB)(HWND param_1, uint32_t uMsg, uint32_t param_3, LPARAM param_4);
typedef bool (__fastcall *fnObjectClientFn)(D2UnitStrc * pUnit);
typedef bool (__fastcall *pfnAiMapDirectionHandler)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, void * pAiParam, uint32_t nSeed);
typedef char (__stdcall *fnGetProtocolVersionChar)();
typedef char * (__stdcall *grGetString)(FxU32 pname);
typedef eD2ClientAppMode (__stdcall *fnClientAppModeProc)();
typedef eD2ServerIncomingStatus (__fastcall *D2NET_SERVER_CLTtoD2GS)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, char * pBytes, uint32_t nSize);
typedef int (*fnConnectToBattleNet)();
typedef int (*fnLogIndent)(int nAction, int nParam);
typedef int (__fastcall *D2MissileSrvDoFunc)(::SGAMEDATA * pGame, D2UnitStrc * pMissile);
typedef int (__fastcall *D2PathTypeFunc)(D2PathTypeArg * pPathTypeArg);
typedef int (__fastcall *SkillSelectProcFn)(D2UnitStrc * pUnit, D2UnitStrc * pTargetUnit);
typedef int (__fastcall *fnApplyPrepatch)(int * pdw);
typedef int (__fastcall *fnCacheFreeMem)(void * pMem);
typedef int (__fastcall *fnDequeueMessage)(int * pTypeOut, void * pBufOut);
typedef int (__fastcall *fnEnterChatAndJoinChannel)(char * szUser, char * szStat, char * szAcct, char * szChan, uint32_t dwFlags);
typedef int (__fastcall *fnGetStateAndClear)(int * pOut);
typedef int (__fastcall *fnGetStoredReadUserData)(int nRow, int nCol, char * * ppOut, int nMax);
typedef int (__fastcall *fnSendChangePassword)(char * szUser, char * szOld, char * szNew);
typedef int (__fastcall *fnSendClickAd)(uint32_t dwAdId, int nParam);
typedef int (__fastcall *fnSendCreateAccount)(char * szUser, char * szPass);
typedef int (__fastcall *fnSendGetChannelList)(int nProduct);
typedef int (__fastcall *fnSendLogonRequest)(char * szUser, char * szPass);
typedef int (__fastcall *fnSendNewsInfo)(uint32_t dw1, uint32_t dw2);
typedef int (__fastcall *fnSendQueryAdURL)(uint32_t dwParam);
typedef int (__fastcall *fpMissileSrvDoFuncData)(D2UnitStrc * pUnit, void * pContext);
typedef int (__fastcall *pfnFogExceptionFilter)(D2CrashReportStrc * pExceptionReport, void * pUserContext);
typedef int (__fastcall *pfnSkillCalcOp)(int32_t nOperand1, int32_t nOperand2, uint32_t nOperand3, void * pContext);
typedef int (__fastcall *pfnTextScrollComplete)(void * pUserParam, uint32_t nReason, void * pDismissCallback);
typedef int (__stdcall *ChatDlgCallback)(uint32_t dwParam);
typedef int (__stdcall *D2NpcMenuOptionFunc)();
typedef int (__stdcall *fnGetDownloadProgress)();
typedef int (__stdcall *fnGetGameVersion)();
typedef int (__stdcall *fnSendCheckAd)();
typedef int (__stdcall *fnSendLeaveChat)();
typedef int (__stdcall *fnSendLeaveGame)();
typedef int (__stdcall *pfnSFileReadError)(void * pnBytesRead, uint32_t dwLastError, int32_t nRetryCount);
typedef int32_t (__fastcall *D2MissileCltHitFunc)(D2UnitStrc * pMissile, D2UnitStrc * pTarget);
typedef int32_t (__fastcall *D2ObjectPopulateFn)(::SGAMEDATA * pGame, D2RoomStrc * pRoom, D2DrlgRoomCoordsStrc * pRoomCoords);
typedef int32_t (__fastcall *D2PropertyHandler_fn)(int32_t nType, D2UnitStrc * pUnit, D2UnitStrc * pItemUnit, D2SetsTxtFStrc * pProperty, eD2UnitStat eStatCode, int32_t nParam2, int32_t nParam3, uint32_t nParam, int32_t nContext);
typedef int32_t (__fastcall *fnAppModeRun)(D2IniConfigStrc * pD2IniConfig);
typedef fnAppModeRun * (__fastcall *D2AppModeInitFunc)(HMODULE hModule);
typedef int32_t (__fastcall *fnCUnitEventCallback)(D2UnitStrc * pUnit, D2UnitStrc * pOwnerUnit, uint32_t dwArg1, uint32_t dwArg2, uint32_t dwEventData1, uint32_t dwEventData2);
typedef int32_t (__fastcall *pfnDeflateBlockFunc)(deflate_state * pDeflateState, int32_t nFlush);
typedef int32_t (__fastcall *pfnPlrModeProcess)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, int32_t nArg1, int32_t nArg2);
typedef uint32_t (__fastcall *D2ObjModeHandler_fn)(::SGAMEDATA * pGame, D2UnitStrc * pObject, int32_t nModeFlags);
typedef uint32_t (__fastcall *EQUIP_Handler)(D2UnitStrc * param_1, D2InventoryStrc * param_2, uint32_t param_1_00);
typedef uint32_t (__fastcall *fnInitiatePatchDownload)(uint8_t * pbData, uint32_t nSize);
typedef uint32_t (__fastcall *fnSendReadUserData)(char * szAccount, uint32_t nCount, uint32_t nRequestId, char * * pszKeys);
typedef uint32_t (__fastcall *fnSendWriteUserData)(char * szAccount, uint32_t nCount, char * * pszKeys, char * * pszValues);
typedef uint32_t (__fastcall *fpCelDecodeCachedByFormat)(int32_t pDCCSrc, DC6 * pDCCDest);
typedef uint32_t (__fastcall *fpMonsterAiForEach)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2UnitStrc * pOther, D2MonsterAiParameterStruct * test);
typedef uint32_t (__fastcall *fpProperties)(uint32_t param_1, D2UnitStrc * param_2, D2UnitStrc * param_3, uint32_t param_4, uint32_t param_5, int param_6, uint32_t param_7, uint32_t param_8, int param_9, uint32_t param_10, uint32_t param_11);
typedef uint64_t (__stdcall *pfnSymGetModuleBase64)(HANDLE hProcess, DWORD qwAddrLow, DWORD qwAddrHigh);
typedef uint64_t (__stdcall *pfnSymLoadModule64)(HANDLE hProcess, HANDLE hFile, char * ImageName, char * ModuleName, DWORD BaseOfDllLow, DWORD BaseOfDllHigh, DWORD SizeOfDll);
typedef void (*RecordCleanupFn)();
typedef void (*fnLogWriteLine)(char * szFmt, ...);
typedef void (*pfnUIConfigTabAction)();
typedef void (__fastcall *AI_Init)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2AiParamStrc * pAiParam);
typedef void (__fastcall *AI_Main)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2AiParamStrc * pAiParam);
typedef void (__fastcall *AI_Secondary)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2AiParamStrc * pAiParam);
typedef void (__fastcall *D2MissileCltDoFunc)(D2UnitStrc * pMissile);
typedef void (__fastcall *D2MissileSrvDmgFunc)(::SGAMEDATA * pGame, D2UnitStrc * pMissile, D2UnitStrc * pTarget, D2DamageStrc * pDamage);
typedef void (__fastcall *D2NET_CLIENT_D2GStoCLT)(char * pBytes);
typedef void (__fastcall *D2NET_CLIENT_D2GStoCLT_Unit)(D2UnitStrc * pUnit, char * pBytes);
typedef void (__fastcall *D2ObjectPopulateFnFunc)(::SGAMEDATA * pGame, D2RoomStrc * pRoom, uint8_t nDensity, int nObjectClassId, uint16_t nChancePercent);
typedef void (__fastcall *D2ObjectsInitFnFunc)(D2ObjectInitFnArg * pInitArg);
typedef void (__fastcall *D2SkillDescAttackFunc)(D2UnitStrc * pUnit, D2SkillStrc * pSkill, D2SkillsTxt * pSkillsTxt, int32_t nSkillLevel, int32_t * pnMinDamage, int32_t * pnMinColor, int32_t * pnMaxDamage, int32_t * pnMaxColor);
typedef void (__fastcall *D2SkillDescDamageFunc)(D2UnitStrc * pUnit, D2SkillStrc * pSkill, D2SkillsTxt * pSkillsTxt, int32_t nSkillLevel, int32_t nXLeft, int32_t nY, int32_t nXRight);
typedef void (__fastcall *D2StatesRemFunc)(D2UnitStrc * pUnit, eD2States eState);
typedef void (__fastcall *D2StatesSetFunc)(D2UnitStrc * pUnit, eD2States eState);
typedef void (__fastcall *D2UnitIdToNameFunc)(char * szText, int32_t nId);
typedef void (__fastcall *PFN_ObjectOperateCallback)(::SGAMEDATA * pGame, D2UnitStrc * pUnit);
typedef void (__fastcall *PFN_PlayerModeMoveFn)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, DWORD nModeIndex, int32_t nX, int32_t nY);
typedef void (__fastcall *PFN_PlayerModeStartFn)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, DWORD nModeIndex, D2UnitStrc * pTargetUnit);
typedef void (__fastcall *QUESTINIT)(D2QuestDataStrc * pQuestData);
typedef void (__fastcall *fnClientDrawProc)(int32_t nParam);
typedef void (__fastcall *fnD2DropHandlerFn3)(::SGAMEDATA * pGame, D2UnitStrc * pUnit);
typedef void (__fastcall *fnGetConnectionState)(int * pOut);
typedef void (__fastcall *fnMonUniqueModCallback)(D2UnitStrc * pUnit, int32_t nUMod, int32_t bIsChampion);
typedef void (__fastcall *fnSendChatCommand)(uint8_t * pbCmd);
typedef void (__fastcall *fnSendGetFileTime)(char * szFile, uint32_t dwTime);
typedef void (__fastcall *fnSendJoinChannel)(char * szChan, int nFlags, int nParam);
typedef void (__fastcall *fnSendNotifyJoin)(char * szName, char * szPass);
typedef void (__fastcall *fnSendSetEmail)(char * szEmail);
typedef void (__fastcall *fnSendStartAdvEx3)(char * szName, char * szPass, char * szDesc, int nFlags);
typedef void (__fastcall *fnSetLoggingFlag)(uint32_t dwParam);
typedef void (__fastcall *fnShrineClientOverlayUpdate)(D2UnitStrc * pUnit, D2ShrinesTxt * pShrinesTxt);
typedef void (__fastcall *fnShrineServerActivate)(D2UnitStrc * pUnit, D2UnitStrc * pPlayer, uint32_t dwParam);
typedef void (__fastcall *fnSpriteCacheEntryRelease)(void * pHashEntry);
typedef void (__fastcall *fpCelAllocDecodedByFormat)(DC6 * pDCC, DC6 * pDCCDest, uint8_t * pPaletteRemapFunc, uint32_t dwColorKey);
typedef void (__fastcall *fpCelDrawFrameByFormat)(uint8_t * pImageHeader, uint8_t * pbDecompressedOutput, int32_t nDrawX, int32_t nClipLeft, int32_t nClipRight, uint32_t nDstPitch, int32_t nStartRow, int32_t nRowCount, int32_t pLookupTable1, int32_t pBlendTable, int32_t pLookupTable2);
typedef void (__fastcall *fpCelRemapFrameByFormat)(DC6Block * pFrame, BYTE * pPaletteRemap);
typedef void (__fastcall *fpD2ShrineTableHandler)(D2ObjectOperateFnArg * pArguments, D2ShrinesTxt * pTxtShrines);
typedef void (__fastcall *fpDrawBlendedVisTile)(int a1, int a2, int a3, int a4);
typedef void (__fastcall *fpDrawRoofTile)(int a1, int a2, int a3, int a4, BYTE a5);
typedef void (__fastcall *fpDrawVisTile)(uint8_t * pbTileData, int32_t nScreenX, int32_t nScreenY);
typedef void (__fastcall *fpDrawWallTile)(int a1, int a2, int a3, int a4);
typedef void (__fastcall *fpFillYBufferTable)(void * ppvBits, int nWidth, int nHeight, int a4);
typedef void (__fastcall *fpFlagOperations)(int * pFlag, int nFlag);
typedef void (__fastcall *fpMonsterUModHandler)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, uint32_t dwParam1, int32_t nParam2);
typedef void (__fastcall *fpServerInitFunctions)(D2IniConfigStrc * pIni);
typedef void (__fastcall *fpTimerFunction)(::SGAMEDATA * pGame, D2UnitStrc * pUnit);
typedef void (__fastcall *fpTimerFunctionEx)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, int nArg1, int nArg2);
typedef void (__fastcall *fpUmodInit)(D2UnitStrc * pUnit, int32_t nIndex, BOOL bUnknown);
typedef void (__fastcall *fpUnknown)(int a1, int a2, int a3, int a4);
typedef void (__fastcall *pfnBlizzardError)(char * szMessage, char * szTitle, char * * ppszButtonText, uint32_t dwFlags, uint32_t dwErrorCode, bool bIsFatal);
typedef void (__fastcall *pfnCelPixelDataFree)(int * pPixelData);
typedef void (__fastcall *pfnEscMenuItemAction)(D2EscMenuItemStrc * pItem, D2UIMouseMsgStrc * pMsg);
typedef void (__fastcall *pfnLruFreeEntry)(void * pEntry);
typedef void (__fastcall *pfnMCPGameListEntry)(uint32_t nEntryField0, uint32_t nEntryCount, uint32_t nEntryField2, uint32_t nListFlag, void * pEntryData);
typedef void (__fastcall *pfnMCPMessage)(uint32_t nMessageId, uint32_t nDataLength, void * pMessageData);
typedef void (__fastcall *pfnMonUniqueModInit)(D2UnitStrc * pUnit, int32_t eModifier, int32_t bApply);
typedef void (__fastcall *pfnPartyScreenAction)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2UnitStrc * pTarget, uint32_t dwArg, uint32_t dwArg2);
typedef void (__fastcall *pfnPlayerModeNetMsg)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2ClientStrc * pClient, int32_t nModeIndex);
typedef void (__fastcall *pfnRoomExActivationHandler)(D2RoomExStrc * pRoomEx);
typedef void (__fastcall *pfnRoomExDeactivationHandler)(D2RoomExStrc * pRoomEx);
typedef void (__fastcall *pfnSCompCodec)(void * pDst, uint32_t * pnDstLen, void * pSrc, uint32_t nSrcLen, uint32_t dwParam);
typedef void (__fastcall *pfnSCompEncodeCodec)(short * pDst, uint32_t * pnDstLen, short * pSrc, uint32_t nSrcLen, void * pUserData, uint32_t nBitsPerSample);
typedef void (__fastcall *pfnSpriteCacheValidate)(void * pCelContext, void * pCelData, void * pFrameInfo);
typedef void (__stdcall *D2AppModeShutdownFunc)();
typedef void (__stdcall *D2ClientQuestEventFn)();
typedef void (__stdcall *D2WinWndHandlerFn)(D2UIMouseMsgStrc * pMsg);
typedef void (__stdcall *fnD2InitUIFlagFunction)(D2UIMouseMsgStrc * param_1);
typedef void (__stdcall *fnDecodeCDKeys)();
typedef void (__stdcall *fnSetDownloadCancelFlag)();
typedef void (__stdcall *fpD2CursorDraw)();
typedef void (__stdcall *fpProfCoreRegisterSlotCb)(uint32_t nSlotId, uint8_t * pSlotData, uint32_t dwFlags);
typedef void (__stdcall *grAlphaBlendFunction)(GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df);
typedef void (__stdcall *grAlphaCombine)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert);
typedef void (__stdcall *grBufferClear)(GrColor_t color, GrAlpha_t alpha, FxU32 depth);
typedef void (__stdcall *grBufferSwap)(FxU32 swap_interval);
typedef void (__stdcall *grChromakeyMode)(GrChromakeyMode_t mode);
typedef void (__stdcall *grChromakeyValue)(GrColor_t value);
typedef void (__stdcall *grClipWindow)(FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy);
typedef void (__stdcall *grColorCombine)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert);
typedef void (__stdcall *grColorMask)(FxBool rgb, FxBool a);
typedef void (__stdcall *grConstantColorValue)(GrColor_t value);
typedef void (__stdcall *grCoordinateSpace)(GrCoordinateSpaceMode_t mode);
typedef void (__stdcall *grDepthBufferFunction)(GrCmpFnc_t function);
typedef void (__stdcall *grDepthBufferMode)(GrDepthBufferMode_t mode);
typedef void (__stdcall *grDepthMask)(FxBool mask);
typedef void (__stdcall *grDisable)(GrEnableMode_t mode);
typedef void (__stdcall *grDitherMode)(GrDitherMode_t mode);
typedef void (__stdcall *grDrawLine)(void * v1, void * v2);
typedef void (__stdcall *grDrawPoint)(void * pt);
typedef void (__stdcall *grDrawTriangle)(void * a, void * b, void * c);
typedef void (__stdcall *grDrawVertexArrayContiguous)(FxU32 mode, FxU32 Count, void * pointers, FxU32 stride);
typedef void (__stdcall *grEnable)(GrEnableMode_t mode);
typedef void (__stdcall *grFinish)();
typedef void (__stdcall *grGlideInit)();
typedef void (__stdcall *grGlideShutdown)();
typedef void (__stdcall *grLoadGammaTable)(FxU32 nentries, FxU32 * red, FxU32 * green, FxU32 * blue);
typedef void (__stdcall *grSstSelect)(int which_sst);
typedef void (__stdcall *grTexCombine)(GrChipID_t tmu, GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor, GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor, FxBool rgb_invert, FxBool alpha_invert);
typedef void (__stdcall *grTexDownloadMipMap)(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info);
typedef void (__stdcall *grTexDownloadTable)(GrTexTable_t type, void * data);
typedef void (__stdcall *grTexFilterMode)(GrChipID_t tmu, GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode);
typedef void (__stdcall *grTexMipMapMode)(GrChipID_t tmu, GrMipMapMode_t mode, FxBool lodBlend);
typedef void (__stdcall *grTexSource)(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info);
typedef void (__stdcall *grVertexLayout)(FxU32 param, FxI32 offset, FxU32 mode);
typedef void (__stdcall *pfnFaultError)(LPCSTR szFile, int nLine, LPCSTR szExpression);
typedef void (__stdcall *pfnSFileOpenCallback)(char * szFileName, void * pUserParam);
typedef void (__stdcall *pfnSSystemShutdown)(uint32_t dwShutdownCode);
typedef void (__stdcall *pfnUIMenuInstance)();
typedef void (__stdcall *pfnVoidNoArgCallback)();
typedef void * (__fastcall *fnSendGetAdvListEx)(char * szName, char * szPass, uint32_t dw1, uint32_t dw2);
typedef void * (__fastcall *fnSendLogonRealmEx)(char * szRealm, char * szPass);
typedef void * (__stdcall *fnBnetLoadAndReturn)();
typedef void * (__stdcall *fnGetChannelNamePtr)();
typedef void * (__stdcall *fnSendQueryRealms2)();
typedef void * (__stdcall *pfnSymFunctionTableAccess)(HANDLE hProcess, DWORD AddrBase);
typedef BOOL (__stdcall *pfnStackWalk)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, void * StackFrame, void * ContextRecord, void * ReadMemoryRoutine, pfnSymFunctionTableAccess FunctionTableAccessRoutine, pfnSymGetModuleBase GetModuleBaseRoutine, void * TranslateAddress);
typedef void * (__stdcall *pfnSymFunctionTableAccess64)(HANDLE hProcess, DWORD AddrBaseLow, DWORD AddrBaseHigh);

// Full type definitions (used by value in globals)
struct CLIENT_ID {
    /* 0x00 */ void * UniqueProcess;
    /* 0x04 */ void * UniqueThread;
};

struct D2AppModeBootstrapStrc {
    /* 0x00 */ fnAppModeRun * (__fastcall *pfnInit)(HMODULE); // App-mode entry: receives the module handle, returns the app-mode function table.
    /* 0x04 */ void (__stdcall *pfnShutdown)(); // App-mode teardown, called after the run loop returns.
};

struct D2BodyClickTable {
    /* 0x00 */ uint32_t (__fastcall *NONE)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x04 */ uint32_t (__fastcall *head)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x08 */ uint32_t (__fastcall *neck)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x0C */ uint32_t (__fastcall *torso)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x10 */ uint32_t (__fastcall *rightArm)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x14 */ uint32_t (__fastcall *leftArm)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x18 */ uint32_t (__fastcall *rightRing)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x1C */ uint32_t (__fastcall *leftRing)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x20 */ uint32_t (__fastcall *belt)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x24 */ uint32_t (__fastcall *feet)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x28 */ uint32_t (__fastcall *gloves)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
    /* 0x2C */ uint32_t (__fastcall *rightSecondary)(D2UnitStrc *, D2InventoryStrc *, uint32_t);
};

struct D2ClientLightPulseStrc {
    /* 0x00 */ BOOL bRoomLightPulseActive;
    /* 0x04 */ uint32_t nLightPulseIntensity;
    /* 0x08 */ uint8_t nLightRoomRed;
    /* 0x09 */ uint8_t nLightRoomGreen;
    /* 0x0A */ uint8_t nLightRoomBlue;
    /* 0x0B */ uint8_t _pad0b;
    /* 0x0C */ uint32_t nFadeInDuration;
    /* 0x10 */ uint32_t nFadeOutDuration;
    /* 0x14 */ uint32_t nHoldDuration;
    /* 0x18 */ uint32_t nTotalDuration;
    /* 0x1C */ uint32_t nCurrentTick;
    /* 0x20 */ uint32_t nSinOffset;
    /* 0x24 */ int32_t eLevelId;
    /* 0x28 */ uint32_t bApplied;
};

struct D2ClientMonAnimModeEntryStrc {
    /* 0x00 */ int32_t nField0;
    /* 0x04 */ int32_t nField4;
    /* 0x08 */ int32_t nField8;
};

struct D2ClientSpriteLoadStatusStrc {
    /* 0x00 */ int32_t aLoaded[20]; // indexed by player anim mode
    /* 0x50 */ int32_t aAsyncPending[20]; // indexed by player anim mode
};

struct D2CmdBindingStrc {
    /* 0x00 */ void (__stdcall *pfnHandler)(); // primary key-command handler; the dispatcher reads it at base + 12*cmd
    /* 0x04 */ void (__stdcall *pfnAltHandler)(); // alternate handler, assigned into the gpfnMouse*AltHandler globals
    /* 0x08 */ int32_t nHasParam; // non-zero if the command requires a parameter; checked before dispatch
};

struct D2CofPathOverrideStrc {
    D2CofPathOverrideStrc() = default;
    D2CofPathOverrideStrc(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2CofPathOverrideStrc(uint32_t a0, uint32_t a1) : dwToken(a0), nMode(a1) {}

    /* 0x00 */ uint32_t dwToken; // COF token chars, matched first
    /* 0x04 */ uint32_t nMode; // mode the override applies to
};

struct D2CutsceneDescStrc {
    /* 0x00 */ int nCutsceneId; // act-end cutscene id 0..7
    /* 0x04 */ char * pszVideoPath;
    /* 0x08 */ uint8_t bHasAltSoundPath;
    /* 0x09 */ uint8_t bCharacterAnimationPlayed;
    /* 0x0A */ uint8_t pad[2];
};

struct __attribute__((packed)) D2D3DDeviceInfoStrc {
    /* 0x000 */ uint32_t dwFlags;
    /* 0x004 */ GUID guid;
    /* 0x014 */ uint32_t dwVendorId;
    /* 0x018 */ uint32_t dwDeviceId;
    /* 0x01C */ char szName[256];
    /* 0x11C */ uint32_t nBytesPerPixel;
};

struct D2D3DTileCornerStrc {
    /* 0x00 */ int32_t nX;
    /* 0x04 */ int32_t nY;
    /* 0x08 */ int32_t nZ;
};

struct D2EndGameUiState {
    /* 0x00 */ bool showCongrats; // Show end-game congrats flow
    /* 0x01 */ uint8_t _pad0[3];
    /* 0x04 */ bool showCutscene; // End-game cutscene/congrats active flag
    /* 0x05 */ uint8_t _pad1[3];
    /* 0x08 */ bool congratzOkClicked; // Congrats OK clicked
    /* 0x09 */ uint8_t _pad2[3];
};

struct D2ExitCutsceneState {
    /* 0x00 */ int pendingCutsceneId; // Pending exit cutscene id
    /* 0x04 */ bool cutscenePlayed; // Exit cutscene played/active flag
    /* 0x05 */ uint8_t _pad0[3];
};

struct __attribute__((packed)) D2GameListEntryStrc {
    /* 0x00 */ uint8_t unk0x00;
    /* 0x01 */ char szName[16];
    /* 0x11 */ char szDescription[32];
    /* 0x31 */ uint16_t wDisplayedCount; // printed with itoa radix 10 in the list
    /* 0x33 */ uint16_t nJoinId; // first argument to OOGMENU_JoinGame; deduplication key
    /* 0x35 */ uint16_t unk0x35;
    /* 0x37 */ uint32_t dwIpAddr; // to NET_MakeIPV4AddressHumanFriendly
};

struct D2GemsTableStrc {
    /* 0x00 */ int nSize;
    /* 0x04 */ D2GemsTxt * pTable;
};

struct D2HashSeedPairStrc {
    D2HashSeedPairStrc() = default;
    D2HashSeedPairStrc(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2HashSeedPairStrc(uint32_t a0, int a1) : dwHashLow(a0), nHashHigh(a1) {}

    /* 0x00 */ uint32_t dwHashLow; // Low 32 bits of hash seed for nibble
    /* 0x04 */ int nHashHigh; // High 32 bits of hash seed for nibble
};

struct __attribute__((packed)) D2HirelingStatLabelStrc {
    /* 0x00 */ int nX;
    /* 0x04 */ int nCenterWidth;
    /* 0x08 */ int nY;
    /* 0x0C */ uint16_t wStrId;
};

struct __attribute__((packed)) D2IniConfigValuesStrc {
    /* 0x00 */ char szAppName[27];
    /* 0x1B */ char szKeyName[27];
    /* 0x36 */ char szCmdLineArg[27];
    /* 0x51 */ uint8_t nType;
    /* 0x52 */ uint8_t field_0x52;
    /* 0x53 */ uint8_t field_0x53;
    /* 0x54 */ int nOffset;
    /* 0x58 */ int nDefaultValue;
};

struct D2LeapArcSegmentStrc {
    /* 0x00 */ int nMaxDist;
    /* 0x04 */ int nSlope;
    /* 0x08 */ int nIntercept;
};

struct D2ManaAbsorbFieldStrc {
    D2ManaAbsorbFieldStrc() = default;
    D2ManaAbsorbFieldStrc(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2ManaAbsorbFieldStrc(int a0, int a1) : nDamageFieldOffset(a0), bSecondary(a1) {}

    /* 0x00 */ int nDamageFieldOffset; // byte offset into the damage struct
    /* 0x04 */ int bSecondary;
};

struct D2MonStatAnimEntryStrc {
    D2MonStatAnimEntryStrc() = default;
    D2MonStatAnimEntryStrc(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2MonStatAnimEntryStrc(uint32_t a0, uint32_t a1) : dwFlags(a0), dwValidFlags(a1) {}

    /* 0x00 */ uint32_t dwFlags;
    /* 0x04 */ uint32_t dwValidFlags;
};

struct D2NET_SERVER_D2GS_ClientToServer {
    /* 0x00 */ eD2ServerIncomingStatus (__fastcall *fpHandler)(::SGAMEDATA *, D2UnitStrc *, char *, uint32_t);
    /* 0x04 */ BOOL field1_0x4;
};

struct D2NpcControlInitStrc {
    D2NpcControlInitStrc() = default;
    D2NpcControlInitStrc(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2NpcControlInitStrc(int a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4) : nMonStatsId(a0), nFlags(a1), nData(a2), nName(a3), nPad(a4) {}

    /* 0x00 */ int nMonStatsId; // monstats id this record configures
    /* 0x04 */ uint8_t nFlags; // gossip/trade/repair flags
    /* 0x05 */ uint8_t nData;
    /* 0x06 */ uint8_t nName;
    /* 0x07 */ uint8_t nPad;
};

struct __attribute__((packed)) D2NpcShopTabStrc {
    /* 0x00 */ int nTextX; // tab label X, panel-relative
    /* 0x04 */ int nTextY; // tab label Y, panel-relative
    /* 0x08 */ uint16_t nStringId; // strtable id of the tab label
    /* 0x0A */ uint32_t nSelected; // tab is the highlighted one
    /* 0x0E */ uint32_t nHasItems; // tab holds at least one item
};

struct D2ObjModeFuncEntryStrc {
    /* 0x00 */ uint32_t (__fastcall *pfnHandler)(::SGAMEDATA *, D2UnitStrc *, int32_t); // Mode dispatch function pointer
    /* 0x04 */ uint32_t nSpriteThreshold; // Sprite state threshold (percent or tick count)
    /* 0x08 */ uint32_t nModeFlags; // Mode flags/enabled
};

struct D2PropertyFuncEntryStrc {
    /* 0x00 */ int32_t (__fastcall *pfnHandler)(int32_t, D2UnitStrc *, D2UnitStrc *, D2SetsTxtFStrc *, eD2UnitStat, int32_t, int32_t, uint32_t, int32_t); // Property function handler pointer
    /* 0x04 */ int32_t nStatCode; // Stat code parameter passed to handler
};

struct D2Quest33RewardEntryStrc {
    /* 0x00 */ char * szPriceList;
    /* 0x04 */ int nPriceCount;
};

struct D2Quest33RewardListStrc {
    /* 0x00 */ D2Quest33RewardEntryStrc sClassRewads[7];
};

struct D2QuestIconsPositionningStrc {
    /* 0x00 */ int nImagePosX;
    /* 0x04 */ int nImagePosY;
    /* 0x08 */ int nClickAreaX;
    /* 0x0C */ int nClickAreaY;
};

struct D2QuestInitDataSimple {
    /* 0x00 */ void (__fastcall *fpQuestData)(D2QuestDataStrc *);
    /* 0x04 */ int nAct;
};

struct D2RenderStatisticsStrc {
    /* 0x00 */ int nTextureMemoryUsageBytes; // floor cache bytes
    /* 0x04 */ int field_0x4[2]; // [1] reset by ResetStats; [2] floor cache misses
    /* 0x0C */ uint32_t nGlideSubtileCacheMemBytes; // subtile cache bytes
    /* 0x10 */ int field_0x10[2]; // [4] reset by ResetStats; [5] subtile cache misses
    /* 0x18 */ int nGlideSmallTextureCacheMemBytes; // sprite cache 1 bytes
    /* 0x1C */ int nGlideMediumTextureCacheMemBytes; // sprite cache 2 bytes
    /* 0x20 */ uint32_t nGlideLargeTextureCacheMemBytes; // sprite cache 3 bytes
    /* 0x24 */ int field_0x24[18]; // stat dwords 9..26: misses 12..14, accesses per frame 18..20
};

struct D2ShrineDispatchEntryStrc {
    /* 0x00 */ void (__fastcall *fpClientOverlayUpdate)(D2UnitStrc *, D2ShrinesTxt *); // Called by OBJECT_DispatchShrineClientFunc on client tick — updates GFX overlay state
    /* 0x04 */ void (__fastcall *fpServerActivate)(D2UnitStrc *, D2UnitStrc *, uint32_t); // Called by OBJECT_HandleWellOpen when player activates shrine/well — server-side spawn or effect
    /* 0x08 */ int nOverlayIdActive; // GFX overlay ID shown when shrine is active/charged (-1 = none)
    /* 0x0C */ int nOverlayIdInactive; // GFX overlay ID shown when shrine is depleted/inactive (-1 = none)
    /* 0x10 */ int nSoundId; // Sound ID played when player activates shrine (0 = none)
};

struct D2SmackerGlideBufferStrc {
    /* 0x000 */ uint8_t aPaletteLUT[256]; // 256-entry palette lookup table for pixel translation
    /* 0x100 */ void * pFrameBuffer; // Pointer to Smacker decoded frame buffer
};

struct D2SocketOffsetStrc {
    D2SocketOffsetStrc() = default;
    D2SocketOffsetStrc(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2SocketOffsetStrc(int a0, int a1) : nX(a0), nY(a1) {}

    /* 0x00 */ int nX;
    /* 0x04 */ int nY;
};

struct D2SrpSaltVerifierStrc {
    /* 0x00 */ uint8_t aSalt[32]; // new-password SRP salt s
    /* 0x20 */ uint8_t aVerifier[32]; // new-password SRP verifier v
};

struct __attribute__((packed)) D2StatDescRowStrc {
    /* 0x00 */ int nX1;
    /* 0x04 */ int nY;
    /* 0x08 */ int nX2;
    /* 0x0C */ uint16_t nStringId;
    /* 0x0E */ int nTextWidth;
};

struct D2StatPanelRowStrc {
    /* 0x00 */ int nX1;
    /* 0x04 */ int nY;
    /* 0x08 */ int nX2;
    /* 0x0C */ int nStatId;
};

struct __attribute__((packed)) D2StatScreenButtonStrc {
    /* 0x00 */ int nX;
    /* 0x04 */ int nY;
    /* 0x08 */ int bHitTest;
    /* 0x0C */ uint16_t nStatId;
};

struct D2TileCmpSubTileOffset {
    D2TileCmpSubTileOffset() = default;
    D2TileCmpSubTileOffset(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2TileCmpSubTileOffset(int32_t a0, int32_t a1) : nPixelCol(a0), nPixelRow(a1) {}

    /* 0x00 */ int32_t nPixelCol; // column pixel offset into DT1 subtile source data
    /* 0x04 */ int32_t nPixelRow; // row multiplier (x0xa0) into DT1 subtile source data
};

struct D2TimerTypeMethodsExStrc {
    /* 0x00 */ fpTimerFunctionEx fpFunctionExByType[15];
};

struct D2TimerTypeMethodsStrc {
    /* 0x00 */ fpTimerFunction fpFunctionByType[15];
};

struct D2TombCanyonPresetStrc {
    /* 0x00 */ int nPresetId;
    /* 0x04 */ int nVariant;
    /* 0x08 */ int nX;
    /* 0x0C */ int nY;
};

struct __attribute__((packed)) D2TombCanyonPresetRegionStrc {
    /* 0x00 */ D2TombCanyonPresetStrc aTombEntries[9]; // PlaceTombEntriesInCanyon walks exactly 9 of these
    /* 0x90 */ int aPresetVariants[24]; // 12 rows of 2; DRLGOUTROOM_GetPresetVariantFromGridCell folds the base to 0x6f2680
};

struct __attribute__((packed)) D2UIConfigTabEntryStrc {
    /* 0x00 */ uint16_t nLabelStringId; // String table SID for this column/tab label - read by CONTROLCONFIG_CalculateColumnBounds and DRAW_ConfigControlsMenu
    /* 0x02 */ void (*pfnAction)(); // Function called when this tab's button is clicked (unaligned - the table is packed)
    /* 0x06 */ int nTabId; // Tab identifier (1=keys, 2=mouse, 3=gamepad)
    /* 0x0A */ uint8_t aBounds[16]; // Four ints: left, right, top, bottom of this column, filled in by CONTROLCONFIG_CalculateColumnBounds
};

struct D2UIFlagDescStrc {
    /* 0x00 */ uint32_t anDismissMode[38]; // indexed by eD2UIvars; old nDismissMode@0x54 is anDismissMode[0x15]
};

struct D2UIFlagStrc {
    /* 0x00 */ BOOL Game;
    /* 0x04 */ BOOL Inventory;
    /* 0x08 */ BOOL CharStat;
    /* 0x0C */ BOOL QuickSkill;
    /* 0x10 */ BOOL Skill;
    /* 0x14 */ BOOL ChatInput;
    /* 0x18 */ BOOL EquipmentPane;
    /* 0x1C */ BOOL NewSkill;
    /* 0x20 */ BOOL NpcMenu;
    /* 0x24 */ BOOL EscMenu;
    /* 0x28 */ BOOL Automap;
    /* 0x2C */ BOOL ConfigControls;
    /* 0x30 */ BOOL NpcShop;
    /* 0x34 */ BOOL AltShowItems;
    /* 0x38 */ BOOL Cash;
    /* 0x3C */ BOOL Quest;
    /* 0x40 */ BOOL PartyPane;
    /* 0x44 */ BOOL QuestLogButton;
    /* 0x48 */ BOOL StatusArea;
    /* 0x4C */ BOOL PartyIcons;
    /* 0x50 */ BOOL WaypointMenu;
    /* 0x54 */ BOOL MiniPanel;
    /* 0x58 */ BOOL PartyMenu;
    /* 0x5C */ BOOL TradePrompt;
    /* 0x60 */ BOOL MessageLog;
    /* 0x64 */ BOOL StashMenu;
    /* 0x68 */ BOOL CubeMenu;
    /* 0x6C */ BOOL SteegStone;
    /* 0x70 */ BOOL GuildVault;
    /* 0x74 */ BOOL _x1D;
    /* 0x78 */ BOOL _x1E;
    /* 0x7C */ BOOL FullBelt;
    /* 0x80 */ BOOL _x20;
    /* 0x84 */ BOOL HelpMenu;
    /* 0x88 */ BOOL HelpScreenRedButton;
    /* 0x8C */ BOOL HireIcons;
    /* 0x90 */ BOOL MercMenu;
    /* 0x94 */ BOOL UnusedScroll;
};

struct D2UniqueCandidateStrc {
    D2UniqueCandidateStrc() = default;
    D2UniqueCandidateStrc(uint64_t v) { *reinterpret_cast<uint64_t*>(this) = v; }
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }
    D2UniqueCandidateStrc(int a0, int a1) : nUniqueIdx(a0), nCumulativeWeight(a1) {}

    /* 0x00 */ int nUniqueIdx; // uniqueitems.txt row index
    /* 0x04 */ int nCumulativeWeight; // running rarity total
};

#pragma pack(push, 1)
struct D2UniqueCandidatePoolStrc {
    /* 0x00 */ uint32_t nGoldCodeInitFlag; // gItemDropGoldCodeInitFlag; the pick loop folds its address as the table base minus one row
    /* 0x04 */ uint32_t _pad;
    /* 0x08 */ D2UniqueCandidateStrc aCandidates[4096];
};
#pragma pack(pop)

struct D2WinWndHandlerStrc {
    /* 0x00 */ int32_t nType; // Handler kind, selecting the branch in D2WIN_RegisterWindowHandler @00506c40: 0 message, 1 command, 2 key down, 3 key up.
    /* 0x04 */ int32_t nMessage; // Windows message id the handler is registered for, e.g. 0x200 WM_MOUSEMOVE.
    /* 0x08 */ void (__stdcall *pfnCallback)(D2UIMouseMsgStrc *); // Handler. Must stay a POINTER type: emitted as an integer it carries the ORIGINAL binary's address and the call goes nowhere.
};

struct D2ZlibExtraLBitsBlOrderStrc {
    /* 0x00 */ int extra_lbits[29]; // zlib trees.c extra_lbits[LENGTH_CODES]
    /* 0x74 */ uint8_t bl_order[19]; // zlib trees.c bl_order[BL_CODES]; build_bl_tree reads it at base-1..-3 folded
    /* 0x87 */ uint8_t pad; // alignment padding before extra_dbits
};

struct __attribute__((packed)) D3DTextureCacheSetStrc {
    /* 0x00 */ D3DTextureCacheStrc aCaches[3]; // mip levels 1..3, indexed [iMip-1]
    /* 0x54 */ IDirect3DTexture2 * apSpriteTextures[3]; // [iMip-1]
    /* 0x60 */ IDirectDrawSurface7 * apSpriteSurfaces[3]; // [iMip-1]
};

struct DSCAPS {
    /* 0x00 */ uint32_t dwSize; // Set to 0x60 by the caller before GetCaps; the recon writes `movb $0x60` here.
    /* 0x04 */ uint32_t dwFlags;
    /* 0x08 */ uint32_t dwMinSecondarySampleRate;
    /* 0x0C */ uint32_t dwMaxSecondarySampleRate;
    /* 0x10 */ uint32_t dwPrimaryBuffers;
    /* 0x14 */ uint32_t dwMaxHwMixingAllBuffers;
    /* 0x18 */ uint32_t dwMaxHwMixingStaticBuffers;
    /* 0x1C */ uint32_t dwMaxHwMixingStreamingBuffers;
    /* 0x20 */ uint32_t dwFreeHwMixingAllBuffers;
    /* 0x24 */ uint32_t dwFreeHwMixingStaticBuffers;
    /* 0x28 */ uint32_t dwFreeHwMixingStreamingBuffers;
    /* 0x2C */ uint32_t dwMaxHw3DAllBuffers; // Was mislabelled gnDSoundFreeBufferCount @0088170c; it is DSCAPS+0x2c, the 3D-buffer gate.
    /* 0x30 */ uint32_t dwMaxHw3DStaticBuffers;
    /* 0x34 */ uint32_t dwMaxHw3DStreamingBuffers;
    /* 0x38 */ uint32_t dwFreeHw3DAllBuffers;
    /* 0x3C */ uint32_t dwFreeHw3DStaticBuffers;
    /* 0x40 */ uint32_t dwFreeHw3DStreamingBuffers;
    /* 0x44 */ uint32_t dwTotalHwMemBytes;
    /* 0x48 */ uint32_t dwFreeHwMemBytes;
    /* 0x4C */ uint32_t dwMaxContigFreeHwMemBytes;
    /* 0x50 */ uint32_t dwUnlockTransferRateHwBuffers;
    /* 0x54 */ uint32_t dwPlayCpuOverheadSwBuffers;
    /* 0x58 */ uint32_t dwReserved1;
    /* 0x5C */ uint32_t dwReserved2;
};

struct SCompCodecEntryStrc {
    /* 0x00 */ uint32_t dwMethodMask;
    /* 0x04 */ void (__fastcall *pfnCodec)(void *, uint32_t *, void *, uint32_t, uint32_t);
};

struct SCompEncodeCodecEntryStrc {
    /* 0x00 */ uint32_t dwMethodMask;
    /* 0x04 */ void (__fastcall *pfnCodec)(short *, uint32_t *, short *, uint32_t, void *, uint32_t);
};

struct ZlibHuffCode {
    ZlibHuffCode() = default;
    ZlibHuffCode(uint32_t v) { *reinterpret_cast<uint32_t*>(this) = v; }
    operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
    ZlibHuffCode(uint8_t a0, uint8_t a1, short a2) : op(a0), bits(a1), val(a2) {}

    /* 0x00 */ uint8_t op; // operation, extra bits, table bits
    /* 0x01 */ uint8_t bits; // bits in this part of the code
    /* 0x02 */ short val; // offset in table or code value
};

#pragma pack(push, 1)
struct D2ZlibStaticDTreeDistCodeStrc {
    /* 0x00 */ ct_data static_dtree[30]; // zlib trees.h static_dtree[D_CODES]
    /* 0x78 */ uint8_t _dist_code[512]; // zlib trees.h _dist_code[DIST_CODE_LEN]; _tr_tally/compress_block read it at base-1+dist folded
};
#pragma pack(pop)

struct D2PickableStrc {
    /* 0x00 */ eD2Quests eQuestId; // Quest that must be completed for pickup restriction
    /* 0x04 */ uint8_t nQuestItemBit; // bQuest value from items.txt that triggers this restriction
    /* 0x05 */ uint8_t pad5[3]; // alignment padding
};

struct __attribute__((packed)) D2AuraEnchantedStrc {
    /* 0x00 */ int nLevelThreshold; // compared against the monster level to size the RNG modulo
    /* 0x04 */ int nSkillLevelBase;
    /* 0x08 */ int nSkillLevelMul;
    /* 0x0C */ int nSkillLevelDiv;
    /* 0x10 */ eD2Skills eSkill; // aura skill granted
};

struct __attribute__((packed)) D2ShrinesTableEntryStrc {
    /* 0x00 */ void (__fastcall *fpShrineHandler)(D2ObjectOperateFnArg *, D2ShrinesTxt *);
    /* 0x04 */ eD2UnitStat eStat;
    /* 0x08 */ eD2States nState;
};

// =============================================================================
// Global data (referenced by multiple functions)
// =============================================================================

extern uint8_t IMAGE_DOS_HEADER__00400000[128];
extern char s_Unknown_006cc780[8];
extern char s_English_with_Korean_Chat_006cc788[28];
extern char s_Russian_006cc7a4[8];
extern char s_Polish_006cc7ac[8];
extern char s_Traditional_Chinese_006cc7b4[20];
extern char s_Simplified_Chinese_006cc7c8[20];
extern char s_Korean_006cc7dc[8];
extern char s_Japanese_006cc7e4[12];
extern char s_Italian_006cc7f0[8];
extern char s_Portuguese_006cc7f8[12];
extern char s_French_006cc804[8];
extern char s_German_006cc80c[8];
extern char s_Spanish_006cc814[8];
extern char s_English_006cc81c[8];
extern char gszHeapTagOperatorNew[4];
// lpDefault parameter of GetPrivateProfileStringA
extern char lpDefault_006cc837[1];
// lpCaption parameter of MessageBoxA
extern LPCSTR lpCaption_006cc848;
extern uint8_t gbChatDlgUpdateMarker;
extern char gbCompressionDecoderMarker[2];
extern char s_Dec_006ccb00[4];
extern char s_Nov_006ccb04[4];
extern char s_Oct_006ccb08[4];
extern char s_Sep_006ccb0c[4];
extern char s_Aug_006ccb10[4];
extern char s_Jul_006ccb14[4];
extern char s_Jun_006ccb18[4];
extern char s_May_006ccb1c[4];
extern char s_Apr_006ccb20[4];
extern char s_Mar_006ccb24[4];
extern char s_Feb_006ccb28[4];
extern char s_Jan_006ccb2c[4];
extern double gdConst0_25;
extern double gdBytesToMegabytes;
extern float gfTwoPow32;
extern double gfFloatGammaCorrectionInit;
extern double gdblConstantTen;
extern char gaHexCharsLookupTable[4];
extern uint16_t gnSymbolSearchPathSeparator;
extern char gbSaveFileReadMarker[3];
extern char s_Cyrix_006cdeb0[6];
extern char s_AMD_006cdeb8[4];
extern char s_Intel_006cdebc[8];
extern char cLogFileReadMode[2];
// param_2 parameter of wsprintfA
extern char param_2_006ce200[5];
// Bit masks 1<<n for n=0..31 (0x1..0x80000000); 622 references, used with OR to set a bit and TEST/AND to test one (e.g. D2WINMAIN_SetControlVisible 004f96f8); ERRORMANAGER_InitErrorManager checks every entry against 1<<n at startup (00408e58).
extern uint32_t gadwBitMasks[32];
// Inverted bit masks ~(1<<n) for n=0..31 (0xfffffffe..0x7fffffff); 37 of the 38 references AND with it to clear a bit (e.g. D2WINMAIN_SetControlVisible 004f9701); the 38th is the startup self-check CMP in ERRORMANAGER_InitErrorManager (00408e63).
extern uint32_t gadwInvBitMasks[32];
extern uint32_t gdwBitBufferSeed[9];
extern double gdConst1;
extern double gfFloatScalingFactor;
extern uint8_t gbFloatConstantBase;
extern char s_File___s_006cf1e8[10];
extern char s__s_006cf248[6];
extern char s_Expr___s_006cf250[11];
extern char gdwSystemMemoryMax[2];
extern int gnSSCompDeflateFlags[32];
extern uint32_t gdwADPCMCoefficient[89];
extern SCompCodecEntryStrc paSCompDecompressCodecTable[5];
extern char szOOGPasswordDialogTimeFmt;
// param_2 parameter of wsprintfA
extern char param_2_006cff28[3];
extern D2SSCompDeflateBlockEntryStrc gaSSCompDeflateBlockTable[10];
extern ZlibHuffCode gaZlibInflateTableMid[512];
extern ZlibHuffCode gaZlibInflateTableStart[32];
extern D2ZlibExtraLBitsBlOrderStrc gZlibExtraLBitsBlOrder;
extern int gaZlibExtraDBits[30];
extern int gaZlibExtraBlBits[19];
extern ct_data gaZlibStaticLTree[288];
extern D2ZlibStaticDTreeDistCodeStrc gZlibStaticDTreeDistCode;
extern uint8_t gaZlibLengthCodeTable[256];
extern uint32_t gdwCrc32Table0[256];
extern uint32_t gdwCrc32Table5[256];
extern char * z_errmsg[10];
extern char * gaFrontEndPaletteFiles[4];
extern char * gpszPaletteSkyDat[4];
extern char * gpszPaletteSkyPl2[4];
extern char gaLanguageCodeFallback[4];
extern char gaLanguageCodeAbbrevTable[4];
extern char gpCutsceneVideoFrameBuffer[8];
extern char gpCutscenesBlackBarTexture[8];
extern uint8_t gaNewCharDefaultEquipTemplate[16];
extern char s_ama_006d5564[4];
extern char s_sor_006d5568[4];
extern char s_nec_006d556c[4];
extern char s_pal_006d5570[4];
extern char s_bar_006d5574[4];
extern char s_dru_006d5578[4];
extern char s_asn_006d557c[4];
// param_2 parameter of wsprintfA
extern char param_2_006d57bc[3];
// NOT a D2CharSelStrc: .rdata string pool starting with "*", then "<%s> ", "<%s%s> ", channel messages
extern char gsCharSelContext[4];
extern uint8_t gaBnetCdKeyCharDecodeTable[256];
extern char gszCmdLineSwitchP[2];
extern char gszCmdLineSwitchC[2];
extern char gpCubeRecipeItemTypeFlags_7[4];
extern char gszCmdLineSwitchHelp[5];
extern char gszCmdLineSwitchH[2];
extern char gszCmdLineSwitchQuestionMark[2];
// Points at gCmdLineHelpText ("Available command line options: ..."), the text CmdLine_ShowHelpIfNotDeveloper / CmdLine_SetAppMode show.
// Provenance: .rdata value 0x6d5b20; the loop at 0044b6c8 only reads gCmdLineOptionTable[0] (ESI < 4).
extern char * gpszCmdLineHelpText;
extern ARGLIST gaClientCmdLineArgs[8];
extern uint8_t gbExpansionInitialized;
extern char * gpszD2LngFileName;
extern char * gpszLngManagerStartError;
extern char * gpszLngCppError;
extern D2UIFlagInitStrc g_UIFlagInitDrawMouseDown[4];
extern D2UIFlagInitStrc gUIFlagDrawConsoleInit[4];
extern D2UIFlagInitStrc gUIFlagDrawTownInit[2];
extern D2UIFlagInitStrc gUIFlagDrawMercInit;
extern D2UIFlagInitStrc gUIFlagDrawSkillInit[2];
extern D2UIFlagInitStrc gUIFlagDrawAreaInit[2];
extern D2UIFlagInitStrc gUIFlagDrawExperienceInit;
extern D2UIFlagInitStrc gUIFlagDrawHealthInit[2];
extern D2UIFlagInitStrc gUIFlagDrawPanelInit;
extern D2UIFlagInitStrc gUIFlagDrawManufactureInit[7];
// Concatenated per-panel UI event-handler groups, not one flat table. Group starts referenced from code: 006d6084 (automap), 006d60c0 (config), 006d615c (quest log), 006d618c, 006d61b0. Two rows defeat the D2UIFlagInitStrc.pfHandler typing and were left alone: [22] @006d618c = {eEventType 5, code -1, pfHandler 0x00000006} - a non-pointer payload on an event type no other row uses - and [24] @006d61a4 = {-1, -1, 0x0070f8d8} which points at gUIFlagDrawGoldInit, a sibling D2UIFlagInitStrc, and is referenced by nothing. Separately the declared extent [26] ends at 006d61bb, orphaning the row at 006d61bc ({0, 0x204, TEXT_OnClickCloseQuestScroll}) that belongs to the group anchored at 006d61b0; gUIFlagDrawMapInit starts at 006d61c8, so 27 is the arithmetically consistent count. Left as found - settling [22] and [24] needs SetUiFlagInit / UI_RegisterAutoMapEventHandlers read out first.
extern D2UIFlagInitStrc gaUIFlagInitTable[22];
extern D2UIFlagInitStrc gaUIFlagInitQuestScrollHandlers[2];
extern D2UIFlagInitStrc gUIFlagDrawMapInit[5];
extern D2UIFlagInitStrc g_UIFlagInitDrawUnknown[2];
extern D2UIFlagInitStrc gUIFlagDrawLevelInit[2];
extern D2UIFlagInitStrc gsUIDrawFlagInit0[5];
extern D2UIFlagInitStrc gsUIDrawFlagInit1[2];
extern D2UIFlagInitStrc g_UIFlagInitDrawPanel2[3];
extern D2UIFlagInitStrc g_UIFlagInitDrawPanel3[3];
extern D2UIFlagInitStrc g_UIFlagInitDrawPanel4[2];
// param_2 parameter of wsprintfA
extern char param_2_006d6454[3];
// 13 points of {x,y} walked with a sliding 8-byte window by the automap unit/party X-marker draw. Four separate labels on its first four dwords left only the first segment with real coordinates, so D2GFX_DrawLine was called 11 times per marker with whatever followed in .rdata.
extern POINT gaAutomapXMarkerPolyline[13];
extern char gaFreeUnitsOrderData[3];
extern uint8_t gaMonsterDenEnvironmentData;
extern uint8_t gaWeatherTransitionConstant;
extern int cChatCmdExclaim;
extern char cChatCmdSlash[2];
extern uint8_t gabWardenEmbeddedModuleHash[16];
extern uint8_t gabWardenModuleRc4Key[16];
// NOT a pool manager: .rdata string pool starting with "end". D2PoolManagerStrc (6092 B, correct type, correctly applied at 00800000 and 0088cadc) was misapplied here
extern char gTxtCompilationMemPool[4];
extern char s_D2X_OUT__s_BIK_006d912c[15];
extern char s_ACT04END_s_BIK_006d9150[15];
extern char s_ACT04START_s_BIK_006d9160[17];
extern char s_ACT03START_s_BIK_006d9174[17];
extern char s_ACT02START_s_BIK_006d9188[17];
extern char s_d2intro_s_bik_006d919c[14];
// param_2 parameter of wsprintfA
extern char param_2_006d9be8[4];
// param_2 parameter of wsprintfA
extern char param_2_006da0cc[4];
extern char s_a5q6_006da1ec[5];
extern char s_a5q5_006da1f4[5];
extern char s_a5q4_006da1fc[5];
extern char s_a5q3_006da204[5];
extern char s_a5q2_006da20c[5];
extern char s_a5q1_006da214[5];
extern char s_a4q3_006da21c[5];
extern char s_a4q2_006da224[5];
extern char s_a4q1_006da22c[5];
extern char s_a3q6_006da234[5];
extern char s_a3q5_006da23c[5];
extern char s_a3q4_006da244[5];
extern char s_a3q3_006da24c[5];
extern char s_a3q2_006da254[5];
extern char s_a3q1_006da25c[5];
extern char s_a2q6_006da264[5];
extern char s_a2q5_006da26c[5];
extern char s_a2q4_006da274[5];
extern char s_a2q3_006da27c[5];
extern char s_a2q2_006da284[5];
extern char s_a2q1_006da28c[5];
extern char s_a1q6_006da294[5];
extern char s_a1q5_006da29c[5];
extern char s_a1q4_006da2a4[5];
extern char s_a1q3_006da2ac[5];
extern char s_a1q2_006da2b4[5];
extern char s_a1q1_006da2bc[5];
extern D2MonMsgAnimModeStrc gaMonMsgAnimModeTable[30];
extern float FLOAT_006da690;
extern float FLOAT_006da6b8;
extern int ganMissileRingSinOffsets[64];
extern int ganMissileRingCosOffsets[64];
// 8-direction delta table {0,-1,-2,-1,0,1,2,1}; string .\SKILLS\SkillsPal.cpp at +0x20
extern int gnPaladinSkillIndexTable[8];
extern D2Missile032SpreadStrc gaMissile032SpreadConstData[6];
extern uint64_t gaIntensityTableConstant;
extern uint32_t gnIntensityTableModes[10];
extern double gdConst255;
extern eD2States gaeSetBonusStates[6];
extern char * gpszPaletteAct1Dat;
extern char * gpszPaletteAct1Pl2;
extern char * gpszPaletteActPathFormat;
extern char * gpszPalDatName;
extern char * gpszPalPl2Name;
extern uint32_t gnStringTruncateConstant;
extern uint32_t gnPaletteLoadBufferSize;
extern uint8_t gaPaletteLoadData;
extern unsigned int gaItemTypeTxtIndexData;
extern uint64_t gaEnvironmentConstantData1;
extern GUID gIID_IDirectSound3DBuffer;
extern GUID gIID_IKsPropertySet;
extern uint8_t gEAXPropertyGuid_Reserved2[16];
extern uint8_t gEAXPropertyGuid_Obstruction[16];
extern uint8_t gEAXPropertyGuid_Reserved1[16];
extern uint8_t gaEAXPropertyGuid_ObstructionParams[9];
extern char gaLanguageCodeLookup[4];
extern uint8_t gaBnSendRawBytesHeader[4];
extern char gaCDKeyAuthData[4];
extern uint8_t gaNetSidSubstitutionBox[480];
extern char gaLangAbbreviation_ChineseTraditional[4];
extern char gaLangAbbreviation_Japanese[4];
extern char s_SIN_006e0264[4];
extern char s_HAN_006e0268[4];
extern char gaLangAbbreviation_Korean[4];
extern char gaLangAbbreviation_Cyrillic[4];
extern char s_ITA_006e0274[4];
extern char s_BRA_006e0278[4];
extern char s_POR_006e027c[4];
extern char s_FRA_006e0280[4];
extern char s_DEU_006e0284[4];
extern char gaLangNameFormat_Nominative[4];
extern char * gpszStringTblPathFormat;
extern char * gpszFontMapPathFormat;
extern char * gpszPatchStringTblPathFormat;
extern char * gpszExpansionStringTblPathFormat;
extern char * gpszD2DataMpqName;
extern char * gpszPatchD2MpqName;
extern char * gpszD2ExpMpqName;
extern uint16_t gaPossessiveNameData;
extern char s_ColonSeparator[2];
extern D2NET_SERVER_D2GS_ClientToServer NET_D2GS_SERVER_INCOMING[103];
extern int ganUnitHashTableBaseOffsets[6];
// gaStartingLevelByAct - uint[5] = {1, 15, 20, 26, 32}. Indexed by act (D2Game::Game::Clients::GetAct result, clamped to <= 4 and bounded by gnActCount == 5) and passed straight to DAMAGE_SetUnitToLevel. Was mis-named gaPlayerInitStats and typed uint[4]; element [4] at 006e1530 was separately mis-labelled gnSkillAmaStrafe_MissileCount, a name with no supporting reference - its only two xrefs are the indexed loads at 00570834 and 005706bc.
extern uint32_t gaStartingLevelByAct[5];
extern uint32_t PlayerMultiplyer[9];
extern uint32_t PlayerMultiplyer2[9];
extern D2PlayerModeFnPairStrc gaPlayerModeMoveFuncs[20];
extern D2ShrinesTableEntryStrc OBJECTSHRINEFUNCTION[24];
extern int gaMissileOffsets_LightningUniqueY[4];
// Indexed by monster class id - 540. MONSTER_EquipFromClassTable 005b1c7a uses LEA ESI,[ESI*8+0x6e1bb0], a base 540 rows before the table that lands inside an unrelated string; the decompiler cannot fold that bias.
extern D2MonsterEquipEntryStrc gaMonsterEquipTable[12];
extern uint32_t gnAuraDamageScale_ManaAbsorb[3];
extern uint8_t BYTE_ARRAY_006e3138[80];
extern char gszExtDc6[5];
extern char gszExtDcc[5];
extern char gszDirObjects[20];
extern fpCelDrawFrameByFormat gaaCelDrawFuncTable[2];
extern uint32_t gaCelCmpDataBlock_006e36a0[224];
extern uint32_t gaCelCmpDirectionLookupByDirectionAndFrame[512];
extern double gdConst180;
// DCC field-width codes: {0,1,2,4,6,8,10,12,14,16,20,24,26,28,30,32}. Sixteen entries, but only fifteen fit here - the sixteenth (32) is the word at 006e5fac, which gdwD2CMPCodecTab0 also claims as its entry 0 because the compiler biased that table's base by one word. Ghidra cannot express the overlap, so this stays uint32_t[15]; the reconstruction has to carry the sixteenth element itself or a field-width code of 15 reads out of bounds.
extern uint32_t g_aCodecBitMaskLookupTable[48];
// Low-bit masks for the DCC bit reader: entry n is (1 << n) - 1, for n = 0..32. CODEC_ReadBits indexes it with the field width, which reaches 32, so the array is 33 entries and the last one is 0xffffffff.
// Was typed int32_t[32]; a stray label (gElemType_E_ElementTypeCode) sat on the final element, all nine of whose references came from CODEC_DecompressAllFrames.
extern int32_t gaCodecLookupTable[33];
// content is ~((1<<n)-1) for n=0..31; string "Sprite Decompression Error" at +0x80
extern uint32_t gdwD2CMPCodecTab1[32];
extern char gsGameMonsterSpawnContext[4];
extern char gsGameMissileTblsContext[4];
extern char gsGameDataTblsContext[4];
extern uint32_t gszTxtCol_Act;
extern unsigned int gszTxtCol_Red;
extern int gaPlayerAnimModeCheckTable[100];
extern int gaAnimTableClassic[80];
extern int gaAnimTableExpansion[80];
extern eD2States aSetItemsStateList[5];
extern unsigned int gszTxtColName_lvl;
extern unsigned int gszTxtColName_op;
extern unsigned int gszTxtColName_S8;
extern unsigned int gszTxtColName_S7;
extern unsigned int gszObjMonCompToken_S6;
extern unsigned int gszObjMonCompToken_S5;
extern unsigned int gszObjMonCompToken_S4;
extern unsigned int gszObjMonCompToken_S3;
extern unsigned int gszObjMonCompToken_S2;
extern unsigned int gszObjMonCompToken_S1;
extern unsigned int gszObjMonCompToken_SH;
extern unsigned int gszObjMonCompToken_LH;
extern unsigned int gszObjMonCompToken_RH;
extern unsigned int gszObjMonCompToken_LA;
extern unsigned int gszObjMonCompToken_RA;
extern unsigned int gszObjMonCompToken_LG;
extern unsigned int gszObjMonCompToken_TR;
extern unsigned int gszObjMonCompToken_HD;
extern int INT_ARRAY_006eb308[16];
extern int gaUnitTypeInteractSizes[6];
// index formula b+9*a (LEA [ECX+EAX*8]; ADD EAX) with a,b<=4 -> max index 40; string at +0xa4 = 41*4
extern int gaPathDirectionStepDeltas[41];
extern uint8_t DIFFDIRECTIONS[256];
extern uint32_t gszHirelingFieldName_act;
extern char s_A_006ed5ac[2];
extern char s_B_006ed5b0[2];
extern char s_C_006ed5b4[2];
extern int gaSetItemBonusStates[6];
extern char s_HDM_006ee7c0[4];
extern D2DrlgPresetTypeTablesStrc gPresetTypeTables;
extern int gnOutdoorsGridWidthByLevel[53];
extern int gRoadPresetIdLookupTable[24];
extern D2DrlgLevelDataList ACT1_D2DrlgLevelData1;
extern D2DrlgLevelDataList ACT1_D2DrlgLevelData2;
extern D2DrlgLevelDataList ACT2_D2DrlgLevelData1;
extern D2DrlgLevelDataList ACT2_D2DrlgLevelData2;
extern D2DrlgLevelDataList ACT5_D2DrlgLevelData3;
extern D2DrlgLevelDataList ACT4_D2DrlgLevelData1;
extern D2DrlgLevelDataList ACT4_D2DrlgLevelData2;
extern int gaRoadPresetDirectionLookup[9];
extern int gaOutPlaceAdjacentRoadIndex[81];
extern int gaWildernessLevelSeedOffsets[170];
extern int gPathDirectionTable[76];
extern uint8_t gaDrlgPathDirectionTable[80];
extern int gPathDirectionOffsetsX[16];
extern int gnDrlgLevelSeed[64];
extern int gnDrlgMemoryMax[48];
extern int gnPathSearchMax[128];
extern int gnPathNodeMax[128];
extern uint32_t gaPathDirectionLookupTable[6];
extern int gnDrlgLevelGenerationState[8];
extern int gnAct5OuterWallGridOffsetByType[24];
extern int gaAct5SceneryPresetTable_0[15][7];
extern int gaAct5SceneryPresetTable_0_0__1_[15][7];
extern D2TombCanyonPresetRegionStrc gaTombCanyonPresetOffsets;
extern uint8_t gaWallNeighborOrientTable[256];
extern uint8_t gaOutRoomPathDirectionDelta[24];
extern char s_SAM_006fad3c[4];
extern char * gaAtlRegistryRootKeys[12];
extern char s_Unable_to_open_response_file___s_006fcbc0[33];
extern char s_Invalid_argument___s_006fcc0c[21];
extern char * gaSCMD_FallbackMessages[3];
extern int gdwUTF8ConversionTab[6];
extern uint8_t gaUTF8ByteCountTable[256];
// UTF-8 lead byte mark by sequence length (0,0,C0,E0,F0,F8,FC); split from gaUTF8ByteCountTable, read by the SSTR UTF-8 encoder
extern int gaUTF8LeadByteMarks[8];
extern uint16_t gszUnicodeLatin1_1[256];
extern uint16_t gszUnicodeLatin2_1[256];
extern uint16_t gszUnicodeKorean_1[256];
extern GUID gIID_IDirect3DSurface;
extern GUID IID_IDirect3DHALDevice;
extern GUID D3DGuid;
extern GUID IID_IDirectDraw7;
// lpMultiByteStr parameter of MultiByteToWideChar
extern char lpMultiByteStr_006ff97c[2];
// lpSubBlock parameter of VerQueryValueA
extern char lpSubBlock_006ff9b8[2];
extern char gDebugSpaceString;
// param_2 parameter of wsprintfA
extern char param_2_006ffc58[3];
// lpOutputString parameter of OutputDebugStringA
extern char lpOutputString_006ffcb0[2];
extern char gdwFileReadPosition[3];
extern char gszCrashDumpRegisterFormatString[4];
extern uint8_t gszIniConfigKeywordCharacterAutoSave[32];
extern uint8_t gszIniConfigKeywordNetworkPort[32];
extern uint8_t gszIniConfigKeywordDifficultySetting[32];
extern uint8_t gszIniConfigKeywordLanguageCode[28];
extern uint8_t gszIniConfigKeywordTextureFilter[96];
extern uint8_t gszIniConfigKeywordGammaCorrection[96];
extern uint8_t gszIniConfigKeywordVsyncOption[264];
extern uint8_t gszIniConfigKeywordExperienceDisplay[96];
extern uint8_t gszIniConfigKeywordVideoDriver[336];
extern char * gaLanguageNames_00705000[15];
extern D2IniConfigValuesStrc D2IniConfigValues[58];
// lpServiceStatus parameter of SetServiceStatus
extern _SERVICE_STATUS lpServiceStatus_007065fc;
extern _SERVICE_STATUS gszCommandLineLocaleCode;
extern char * gaErrorTypeNames[12];
extern char * gaMonthAbbreviations[12];
extern uint8_t gabHuffmanCodeLengths[256];
extern float sinTable_f32[512];
extern uint32_t gpServerMemHeap;
extern char gszLogFilePath[260];
extern char s___AUMessage_LogManager___00708124[28];
extern uint32_t gnSFileDataChunkSize;
extern uint32_t gnSFileReadErrorMode;
extern uint32_t gbSFileSeekOptimization;
extern D2StormListHead gMpqArchiveList;
extern D2StormListHead gSFileAsyncReqQueue;
extern D2StormListHead gSFileOpenFileList;
extern D2StormListHead gSFileAudioStreamList;
extern D2StormListHead gSFileStreamRecordList;
extern char sRTTI_ARCHIVEREC_AllocTag[32];
extern char sRTTI_EVENTREC_AllocTag[16];
extern char sRTTI_REQUEST_AllocTag[16];
extern char sRTTI_AUDIOSTREAM_AllocTag[20];
extern D2StormListHead gnStormErrorCallbackCount;
extern char gszSSystemListAllocTag[14];
extern char s___AUWNDREC___00708850[16];
extern D2StormListHead gpWndTrackingListHead;
extern char s__I_00708874[4];
extern char s__E_00708880[4];
extern char s__D_0070888c[4];
extern char s___AUBREAKCMD___007088d8[16];
extern D2StormListHead gpSevtBreakCmdListHead;
extern int32_t gaMenuFormFontParamsFont3[2];
extern int32_t gaMenuFormFontParamsFont2[2];
extern int32_t gaMenuFormFontParamsFont7[2];
extern int32_t gaMenuFormFontParamsFont1[2];
extern int32_t gaMenuFormFontParamsFont0Ex4[2];
extern int32_t gaMenuFormFontParamsFont4Ex1[2];
extern int32_t gaMenuFormFontParamsFont5[2];
extern int32_t gaMenuFormFontParamsFont4[2];
extern int32_t gaMenuFormFontParamsFont8[2];
extern int32_t gaMenuTextBoxScrollbarGeom0[3];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesAssassin[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesDruid[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesAmazon[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesNecromancer[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesBarbarian[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesSorceress[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesPaladin[5];
extern FORMS_AnimFrameStrc gaMainMenuLogoLeftAnimFrames[5];
extern FORMS_AnimFrameStrc gaMainMenuLogoRightAnimFrames[5];
extern int32_t gaMenuTextBoxScrollbarGeom1[3];
extern int32_t gaMenuTextBoxScrollbarGeom2[3];
extern int32_t gaMenuTextBoxScrollbarGeom3[3];
extern int32_t gaMenuTextBoxScrollbarGeom4[3];
extern int32_t gaMenuTextBoxScrollbarGeom5[3];
extern uint32_t gnClassDescriptionTooltipClassId;
// Form records read by FORMS_CreateAnyForm (48 bytes each). pTypeArg1/pTypeArg2 (+0x24/+0x28) are constructor arguments whose meaning depends on neD2FormType: code, data or a small integer (the Button initial image index 0/2/4/6, e.g. gaPredefinedForms_b[12..14]); the struct field comments list each case. Typed void* for that reason.
// Provenance: FORMS_CreateAnyForm 004f93c0 and a census of every record.
extern FORMS_PredefinedFormStrc gaPredefinedForms[333];
extern int gnClassDescriptionTooltipState;
extern int gnCharSelLoadMoreEntryIndex;
extern int gnCharSelLoadingEntryIndex;
extern int gnCharSelScreenSlotCount;
extern uint8_t gaNewCharDefaultAppearance[16];
// Profile keys profile\sex, profile\location, profile\description passed as one array (count 3) to the network interface.
// Provenance: PUSH 0x70cd54 / MOV EDX,3 at 0043f611 and 00440cc2.
extern char * gaszChatDlgProfileKeys[3];
extern unsigned short CreateJoinGameRequestId;
extern uint8_t g_nLadderFilterMode;
extern uint8_t gnLadderCurrentFilterMode;
extern uint32_t gnChatRoomListSelectedBase;
extern uint32_t gbLadderFilterPending;
extern uint32_t gbLadderExpansionEnabled;
extern uint32_t gnChatTabCompletionIndex;
extern uint32_t gnLadderBoardSelectedRank;
extern uint32_t gnLadderRequestedCharIndex;
extern int32_t gaMultiFormFontParamsFont4[2];
extern int32_t gaMultiFormFontParamsFont3[2];
extern int32_t gaMultiFormFontParamsFont2[2];
extern int32_t gaMultiFormFontParamsFont1[2];
extern int32_t gaMultiFormFontParamsFont7[2];
extern int32_t gaMultiTextBoxScrollbarGeom0[3];
extern int32_t gaMultiTextBoxScrollbarGeom1[3];
extern int32_t gaMultiTextBoxScrollbarGeom2[3];
extern int32_t gaMultiTextBoxScrollbarGeom3[3];
extern int32_t gaMultiTextBoxScrollbarGeom4[3];
extern int32_t gaMultiTextBoxScrollbarGeom5[3];
extern int32_t gaMultiTextBoxScrollbarGeom6[3];
extern FORMS_AnimFrameStrc gaMultiLogoAnimFrames[5];
// Form records read by FORMS_CreateAnyForm (48 bytes each). pTypeArg1/pTypeArg2 (+0x24/+0x28) are constructor arguments whose meaning depends on neD2FormType: code, data or a small integer (the Button initial image index 0/2/4/6, e.g. gaPredefinedForms_b[12..14]); the struct field comments list each case. Typed void* for that reason.
// Provenance: FORMS_CreateAnyForm 004f93c0 and a census of every record.
// Index 0x80 (0070e650, FORM_Timer -> CHATDLG_OnTimerRequestLadderData) is the last record: UIMENU_LadderBoard_PrintClassesAndTableHeaders 00440887 calls FORMS_CreateAnyFormFromListEx(0x80).
extern FORMS_PredefinedFormStrc gaPredefinedForms_b[129];
extern FORMS_PredefinedFormStrc FORMS_PredefinedFormStrc_0070e920;
extern FORMS_PredefinedFormStrc FORMS_PredefinedFormStrc_0070e950;
extern FORMS_PredefinedFormStrc FORMS_PredefinedFormStrc_0070e980;
extern char gszOogLevelRestrictionText[4];
extern char gszOogMaxPlayersText[4];
extern int32_t gnPendingGameInfoCharIndex;
extern uint32_t gnCurrentChannelContextId;
// COM callback dispatch table — 24 entries (indices 0x00–0x17). Indexed by COM event type code. See ComCallback.cpp ~L1065: (*gaComCallbackDispatch[eventType])(a, b)
extern pfnComCallback gaComCallbackDispatch[24];
extern eD2ClientAppMode gdwAppMode;
// Points at "Diablo II"; copied into the command-line caption pointers by StaticInit_6ca9a0/6ca9b0.
// Provenance: 006ca9a0, 006ca9b0 MOV EAX,[0x70ee50].
extern char * gpszDiabloIICaption;
extern BOOL gbQuitGame;
extern char string_IpAddressTcpIp[24];
extern char string_IpaddressGameserverBattlenet[24];
extern uint32_t gnPrimaryClickState;
extern int gnMouseButtonEventState;
extern int gnActTransitionFadeFrames;
extern D2UIFlagInitStrc g_UIFlagInitGameDataPanel1;
extern D2UIFlagInitStrc g_UIFlagInitGameDataPanel2[2];
extern D2UIFlagInitStrc g_UIFlagInitGameDataPanel3[2];
extern char s___AUSGAMEDATA___0070f56c[16];
extern char s___AV__TSExplicitList_USGAMEDATA__0070f584[48];
extern char s___AV__TSHashObjectChunk_USGAMEDA_0070f5f4[56];
extern D2GameRecordStrc * gpCurrentGameRecord;
extern uint32_t gnLastMouseButtonEventType;
extern D2UIFlagInitStrc gaUIFlagInitMessageHandlers[4];
extern int nHealthBarScreenX;
extern int gnMercenaryHealthBarScreenY;
extern D2UIFlagInitStrc gaUIFlagInitNpcShopHandlers[8];
extern D2UIFlagInitStrc gUIFlagDrawGoldInit[7];
extern D2UIFlagInitStrc gUIFlagDrawInventoryInit[13];
extern int gnUIFlagCount;
extern D2UIFlagInitStrc gUIFlagDrawInfoInit[8];
extern D2UIFlagInitStrc gaUIFlagInitCubeHandlers[7];
extern D2UIFlagInitStrc g_UIFlagInitDrawPanel1[2];
extern uint32_t gbCursorAnimationPending;
extern D2UIFlagDescStrc gaUIFlagDescs[38];
extern D2UIFlagDescStrc * gUIFlagDismissModeTable[38];
extern int nAutoMapZoomDivisor;
extern uint32_t gaScreenResolutionTableIndexArray;
extern uint32_t gaScreenResolutionModeArray;
extern int ScreenSizeX;
extern int ScreenSizeY;
extern char s___AUTALLOCREC___00711480[16];
extern D2NET_CLIENT_D2GS_ServerToClient NET_D2GS_CLIENT_INCOMING[175];
extern char * gaLanguageNames_00711d04[15];
extern D2UIFlagInitStrc gsUIEngineFlagInit[4];
extern D2UIFlagInitStrc gsUIEngineFlagInit2[4];
extern uint32_t nNextGUID;
extern uint32_t gnSelectConstant13_X;
extern uint32_t gnSelectConstant13_Y;
extern D2UIFlagInitStrc gUIFlagGeneralInit[3];
// Seven cursors, 28 bytes each, 00712010..007120d4. The five int columns of record 0 had been labelled as five standalone globals (gnUIFrameCounter, gdwUIRedrawFlags, gnUIMouseMode, gnUIInputState, gnUIKeyPressCount) which the emitter turned into function-local statics, and the table itself was declared 20 bytes late. UI_GetCursorMaxFrames therefore returned 1<<8 for every cursor and UI_HandleCursorMouseMove saw bKeyPressSwitch set on EVERY mouse move, forcing ItemTypeOnHand=5 and nCursorDataIndex=4. The dword at 007120d4 is the count, 7 - the three asserts test that, not CursorData[6].nFrameOffset.
extern D2CursorDataStrc CursorData[7];
extern int gnCursorDataCount;
extern int gaKeyBindingsDefault[285];
extern D2CmdBindingStrc gaCmdBindings[57];
extern D2UIFlagInitStrc gsUICmdTblFlagInit[4];
extern int gnDenOfEvilLightTimerTick;
extern int gnDiabloSpawnLightTimer;
extern uint32_t gnQuestMonsterUnitId;
extern uint32_t gaMonsterModePrecacheEntries[15];
extern uint32_t gnMonsterModePrecacheCount;
extern uint32_t gnBlizzardSoundPlayed;
extern uint32_t gbLightmapColorEnabled;
extern D2SeedStrc * gpArcaneStarsSeed;
extern eD2ArenaFlags CLIENT_ArenaFlags;
extern eD2PlayerClassIDDWORD eD2PlayerClassIDDWORD_00712f00;
extern int gnChatHistoryReadIndex;
extern uint32_t gnChatHistoryWriteIndex;
extern D2UIFlagInitStrc gUIFlagChatInit;
extern D2UIFlagInitStrc g_UIFlagInitChatPanel;
extern D2EscMenuHeaderStrc gEscMenuHeaderInitData;
// 38 records of {nEnabled,nHideable,nSavedState}. Split into three labels, so opening the ESC menu wrote 456 bytes past a 4-byte object and read its per-flag config from garbage.
extern D2EscMenuUIFlagStrc gaEscMenuUIFlags[38];
extern D2EscMenuItemStrc gaEscMenuMainTable[3];
extern D2EscMenuHeaderStrc gaEscMenuHeaderOptions;
extern D2EscMenuHeaderStrc gaEscMenuHeaderSaveExit;
extern D2EscMenuItemStrc gaEscMenuOptionsTable[5];
extern D2EscMenuItemStrc gaEscMenuSaveExitTable[8];
extern D2EscMenuHeaderStrc gaEscMenuHeaderSwitchClass;
extern D2EscMenuHeaderStrc gaEscMenuHeaderDeleteChar;
extern D2EscMenuItemStrc gaEscMenuSwitchClassTable[7];
extern D2EscMenuItemStrc gaEscMenuDeleteCharTable[8];
extern D2EscMenuHeaderStrc gaEscMenuHeaderExpansionOptions;
extern D2EscMenuHeaderStrc gaEscMenuHeaderExpansionSaveExit;
extern D2EscMenuItemStrc gaEscMenuClassicSaveExitTable[6];
extern D2EscMenuItemStrc gaEscMenuExpansionSaveExitTable[7];
extern uint32_t gpWardenModuleParam;
extern D2CutsceneDescStrc gdwSoundHeaderCacheIndex[8];
extern uint32_t gnInventoryActiveUnitId;
extern int gnItemDescTitleMaxWidth;
extern int gaInventoryItemSlots;
extern int gaInventoryState;
extern int nInventoryContextMode;
extern int32_t pInventoryContextUi;
extern int32_t gpInventoryContextUi;
extern int gaInventorySelectState;
extern uint32_t gnStashNpcGuid;
extern uint32_t gnHirelingLArmSlotImageOffsetY;
extern uint32_t gnHirelingLArmSlotImageOffsetX;
extern uint32_t gnHirelingHeadSlotImageOffsetY;
// @data gnInvMercWeaponSlot2ImageOffsetY
// @wasnamed gnHirelingHeadSlotImageOffsetX
// @evidence At 0048b121/0048f653 added to the head rect's nBottom and passed as
// the Y argument for gpInvMercWeaponSlotImage2. @wasnamed gnHirelingHeadSlotImageOffsetX, shared
// with 007220bc.
extern uint32_t gnInvMercWeaponSlot2ImageOffsetY;
extern uint32_t gnHirelingRArmSlotImageOffsetX;
extern uint32_t gnHirelingRArmSlotImageOffsetY;
extern uint32_t gnInvLArmSlotImageOffsetX;
extern uint32_t gnInvLArmSlotImageOffsetY;
extern D2NpcShopTabStrc gaNpcShopTabs[4];
extern int AmountOfNPCTabs;
extern uint32_t CurrentNpcButtonSelection;
extern int gaNpcShopButtonTable[2];
extern int NPCButtonArray[16];
extern int gnBeltPanelState;
extern BOOL gbPanelClickProcessed;
extern int32_t gnBeltSlotUnderCursor;
extern int gnBeltTooltipX;
extern int gnBeltTooltipY;
extern char * gaLanguageNames_00722414[15];
extern char * gaLanguageNames_00722470[15];
extern int gaWaypointRowLayout[55];
// @data gnScrollingTextBoxTopY
// @wasnamed gnTextOverheadCounter
// @evidence = 12. Stored to rect.top at 004a0ef0 with +0x70 giving rect.bottom, and
// passed at 0049d663 as the Y paired with gnTextScrollBoxX @007bf278. It is a layout coordinate, not
// a counter. @wasnamed gnTextOverheadCounter - the real counter is 007bf20e.
extern unsigned int gnScrollingTextBoxTopY;
extern D2UIFlagInitStrc gsUITextFlagInit[7];
extern uint16_t gaQuestDescData_DenOfEvil[32];
extern uint16_t gaQuestDescData_SistersBurialGrounds[32];
extern uint16_t gaQuestDescData_ToolsOfTheTrade[32];
extern uint16_t gaQuestDescData_SearchForCain[32];
extern uint16_t gaQuestDescData_ForgottenTower[32];
extern uint16_t gaQuestDescData_SistersToTheSlaughter[32];
extern uint16_t gaQuestDescData_RadamentsLair[32];
extern uint16_t gaQuestDescData_HoradricStaff[32];
extern uint16_t gaQuestDescData_TaintedSun[32];
extern uint16_t gaQuestDescData_ArcaneSanctuary[32];
extern uint16_t gaQuestDescData_TheSummoner[32];
extern uint16_t gaQuestDescData_SevenTombs[32];
extern uint16_t gaQuestDescData_LamEsensTome[32];
extern uint16_t gaQuestDescData_KhalimsWill[32];
extern uint16_t gaQuestDescData_BladeOfTheOldReligion[32];
extern uint16_t gaQuestDescData_GoldenBird[32];
extern uint16_t gaQuestDescData_BlackenedTemple[32];
extern uint16_t gaQuestDescData_TheGuardian[32];
extern uint16_t gaQuestDescData_FallenAngel[32];
extern uint16_t gaQuestDescData_TerrorsEnd[32];
extern uint16_t gaQuestDescData_HellsForge[32];
extern uint16_t gaQuestDescData_SiegeOnHarrogath[32];
extern uint16_t gaQuestDescData_RescueMountArreat[32];
extern uint16_t gaQuestDescData_PrisonOfIce[32];
extern uint16_t gaQuestDescData_BetrayalOfHarrogath[32];
extern uint16_t gaQuestDescData_RiteOfPassage[32];
extern uint16_t gaQuestDescData_EveOfDestruction[32];
extern D2QuestIconsPositionningStrc gaQuestIconPositions[6];
extern D2QuestLogInfoStrc gaQuestInfoTable[41];
extern D2QuestLogPanelRectStrc gaQuestLogPanelRects[4];
extern D2ConfigControlDescStrc * gpStatDescriptorTable;
extern D2ConfigControlDescStrc gaConfigControlBindings[51];
extern int nConfigControlSelected;
extern D2UIConfigTabEntryStrc g_UIConfigTabTable[3];
extern uint32_t gnConfigBindingFirstClick;
extern D2UIFlagInitStrc gsUIFlagInit1;
extern D2UIFlagInitStrc gsUIFlagInit0;
extern D2UIFlagInitStrc gsUIFlagInit2;
extern D2UIFlagInitStrc gsUIFlagInit3;
extern D2StatDescRowStrc gaStatDescRows[15];
// The four character-screen attribute (+) buttons, 14 bytes each. Every column of record 0 was its own symbol, so only the first button existed - the other three hit-tested and sent their stat id from unrelated memory.
extern D2StatScreenButtonStrc gaStatScreenButtons[4];
extern char * gaLanguageNames_00724a80[15];
extern D2ClassIconWithNameStrc pSpellsClassSkillicon[7];
extern int nSkillTreeActiveTab;
extern D2ClassIconWithNameStrc pSpellsClassSkilltree[7];
extern int gnSkillTreeHighlightIndex;
extern char * gaLanguageNames_007251e0[15];
extern D2SpeechBankStrc gaNpcSpeechData_725498[12];
extern int D2NPCSpeechCount;
extern D2SpeechBankStrc gaNpcSpeechBank_725550[11];
extern D2SpeechBankStrc gaNpcDialogOptionData_7255f8[9];
extern D2SpeechBankStrc gaNpcDialogOptionData_725680[7];
extern D2SpeechBankStrc gaNpcDialogOptionData_7256f0[7];
extern D2SpeechBankStrc gaNpcDialogOptionData_725760[8];
extern D2SpeechBankStrc gaNpcDialogOptionData_7257d8[9];
extern D2SpeechBankStrc gaNpcDialogOptionData_725860[13];
extern D2SpeechBankStrc gaNpcDialogOptionData_725928[10];
extern D2SpeechBankStrc gaNpcDialogOptionData_7259c0[12];
extern D2SpeechBankStrc gaNpcDialogOptionData_725a78[14];
extern D2SpeechBankStrc gaNpcDialogOptionData_725b50[12];
extern D2SpeechBankStrc gaNpcDialogOptionData_725c08[11];
extern D2SpeechBankStrc gaNpcSpeechBank_725cb0[10];
extern D2SpeechBankStrc gaNpcSpriteFrameData_725d48[12];
extern D2SpeechBankStrc gaNpcSpriteFrameData_725e00[11];
extern D2SpeechBankStrc gaNpcDialogOptionData_725ea8[12];
extern D2SpeechBankStrc gaNpcDialogOptionData_725f60[7];
extern D2SpeechBankStrc gaNpcDialogOptionData_725fd0[11];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726078[12];
extern D2SpeechBankStrc gaNpcSpeechBank_726130[9];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_007261b8[11];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726260[12];
extern D2SpeechBankStrc gaNpcSpriteFrameData_726314[3];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726344[3];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726378[6];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_007263d8[12];
extern D2SpeechBankStrc gaNpcSpriteFrameData_726490[12];
extern D2SpeechBankStrc gaNpcSpeechBank_726548[12];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_00726600[16];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_007266f0[12];
extern D2SpeechBankStrc D2SpeechBankStrc_ARRAY_007267a8[11];
extern D2NPCSpeechStrc D2NPCSpeech[46];
extern char * gaLanguageNames_007273a8[15];
extern D2UIFlagInitStrc g_UIFlagInitDialogMouseWheel;
// Fourteen {eEventType, nMessageOrKeyCode, pfHandler} records, 007273f0..00727498. Every handler is in 004b7xxx and the table ends exactly where the ASCII "dialogbackground" begins at 00727498, which pins the count. Typed as a single struct, every dialog's mouse and key handlers were registered from whatever followed record 0 in memory and no menu line could ever be clicked - the NPC menu appeared but nothing in it responded.
extern D2UIFlagInitStrc gsUIDialogFlagInit[14];
extern int nSoundVolumeLeftChannel;
extern int nSoundVolumeRightChannel;
extern uint16_t gaButtonDescriptors[4];
extern fnObjectClientFn gpObjectClientFunctionDispatchTable[19];
extern int nCashDisplayItemCount;
// @data gaNpcCashTabs_nTooltipStringId_col
// @wasnamed gnNpcCashTabBoundsY1
// @evidence u16 = 0x0FB1. Read at 004bfef3 as movzx word [piVar1-0x16] and passed
// to GetLocaleString. A string id, not a Y bound. @wasnamed gnNpcCashTabBoundsY1 (3-way collision).
extern D2NpcCashTabStrc gaNpcCashTabs[2];
extern D2UIFlagInitStrc gUIFlagMapInit;
// Former interior labels: gnCursorInvTypeAutoOpenB=+0x4
extern uint32_t gnCursorInvTypeAutoOpenC[5];
extern uint32_t gnCursorInvTypeAutoOpenA;
extern D2SkillCltDoFunc SKILLCLTDOFUNCS[130];
extern int SKILLCLTDOFUNCS_SIZE;
extern int bUnitSoundTableInit[11];
extern int gaUnitSoundTable[28];
extern eD2Sounds eD2Sounds_ARRAY_00727f48[12];
extern eD2Sounds eD2Sounds_ARRAY_00727f78[12];
extern eD2Sounds eD2Sounds_ARRAY_00727fa8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728008[12];
extern eD2Sounds eD2Sounds_ARRAY_00728038[12];
extern eD2Sounds eD2Sounds_ARRAY_00728068[12];
extern eD2Sounds eD2Sounds_ARRAY_00728098[12];
extern eD2Sounds eD2Sounds_ARRAY_007280c8[12];
extern eD2Sounds eD2Sounds_ARRAY_007280f8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728128[12];
extern eD2Sounds eD2Sounds_ARRAY_00728188[12];
extern eD2Sounds eD2Sounds_ARRAY_007281b8[12];
extern eD2Sounds eD2Sounds_ARRAY_007281e8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728218[12];
extern eD2Sounds eD2Sounds_ARRAY_00728278[12];
extern eD2Sounds eD2Sounds_ARRAY_007282a8[12];
extern eD2Sounds eD2Sounds_ARRAY_007282d8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728308[12];
extern eD2Sounds eD2Sounds_ARRAY_00728368[12];
extern eD2Sounds eD2Sounds_ARRAY_00728398[12];
extern eD2Sounds eD2Sounds_ARRAY_007283c8[12];
extern eD2Sounds eD2Sounds_ARRAY_007283f8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728428[12];
extern eD2Sounds eD2Sounds_ARRAY_00728458[12];
extern eD2Sounds eD2Sounds_ARRAY_00728488[12];
extern eD2Sounds eD2Sounds_ARRAY_007284b8[12];
extern eD2Sounds eD2Sounds_ARRAY_007284e8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728518[12];
extern eD2Sounds eD2Sounds_ARRAY_00728548[12];
extern eD2Sounds eD2Sounds_ARRAY_00728578[12];
extern eD2Sounds eD2Sounds_ARRAY_007285a8[12];
extern eD2Sounds eD2Sounds_ARRAY_007285d8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728608[12];
extern eD2Sounds eD2Sounds_ARRAY_00728638[12];
extern eD2Sounds eD2Sounds_ARRAY_00728668[12];
extern eD2Sounds eD2Sounds_ARRAY_00728698[12];
extern eD2Sounds eD2Sounds_ARRAY_007286c8[12];
extern eD2Sounds eD2Sounds_ARRAY_007286f8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728728[12];
extern eD2Sounds eD2Sounds_ARRAY_00728758[12];
extern eD2Sounds eD2Sounds_ARRAY_00728788[12];
extern eD2Sounds eD2Sounds_ARRAY_007287b8[12];
extern eD2Sounds eD2Sounds_ARRAY_007287e8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728848[12];
extern eD2Sounds eD2Sounds_ARRAY_00728878[12];
extern eD2Sounds eD2Sounds_ARRAY_007288a8[12];
extern eD2Sounds eD2Sounds_ARRAY_007288d8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728908[12];
extern eD2Sounds eD2Sounds_ARRAY_00728938[12];
extern eD2Sounds eD2Sounds_ARRAY_00728968[12];
extern eD2Sounds eD2Sounds_ARRAY_00728998[12];
extern eD2Sounds eD2Sounds_ARRAY_007289c8[12];
extern eD2Sounds eD2Sounds_ARRAY_007289f8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728a28[12];
extern eD2Sounds eD2Sounds_ARRAY_00728a58[12];
extern eD2Sounds eD2Sounds_ARRAY_00728a88[12];
extern eD2Sounds eD2Sounds_ARRAY_00728ab8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728ae8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728b18[12];
extern eD2Sounds eD2Sounds_ARRAY_00728b48[12];
extern eD2Sounds eD2Sounds_ARRAY_00728b78[12];
extern eD2Sounds eD2Sounds_ARRAY_00728ba8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728bd8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728c08[12];
extern eD2Sounds eD2Sounds_ARRAY_00728c38[12];
extern eD2Sounds eD2Sounds_ARRAY_00728c68[12];
extern eD2Sounds eD2Sounds_ARRAY_00728c98[12];
extern eD2Sounds eD2Sounds_ARRAY_00728cc8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728cf8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728d28[12];
extern eD2Sounds eD2Sounds_ARRAY_00728d58[12];
extern eD2Sounds eD2Sounds_ARRAY_00728d88[12];
extern eD2Sounds eD2Sounds_ARRAY_00728db8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728e18[12];
extern eD2Sounds eD2Sounds_ARRAY_00728e48[12];
extern eD2Sounds eD2Sounds_ARRAY_00728e78[12];
extern eD2Sounds eD2Sounds_ARRAY_00728ea8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728ed8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728f08[12];
extern eD2Sounds eD2Sounds_ARRAY_00728f38[12];
extern eD2Sounds eD2Sounds_ARRAY_00728f68[12];
extern eD2Sounds eD2Sounds_ARRAY_00728f98[12];
extern eD2Sounds eD2Sounds_ARRAY_00728fc8[12];
extern eD2Sounds eD2Sounds_ARRAY_00728ff8[12];
extern eD2Sounds eD2Sounds_ARRAY_00729028[12];
extern eD2Sounds eD2Sounds_ARRAY_00729058[12];
extern eD2Sounds eD2Sounds_ARRAY_00729088[12];
extern eD2Sounds eD2Sounds_ARRAY_007290b8[12];
extern eD2Sounds eD2Sounds_ARRAY_007290e8[12];
extern eD2Sounds eD2Sounds_ARRAY_00729118[12];
extern eD2Sounds eD2Sounds_ARRAY_00729148[12];
extern eD2Sounds eD2Sounds_ARRAY_00729178[12];
extern eD2Sounds eD2Sounds_ARRAY_007291a8[12];
extern eD2Sounds eD2Sounds_ARRAY_007291d8[12];
extern eD2Sounds eD2Sounds_ARRAY_00729208[12];
extern eD2Sounds eD2Sounds_ARRAY_00729238[12];
extern eD2Sounds eD2Sounds_ARRAY_00729268[12];
extern eD2Sounds eD2Sounds_ARRAY_00729298[12];
extern eD2Sounds eD2Sounds_ARRAY_007292c8[12];
extern eD2Sounds eD2Sounds_ARRAY_007292f8[12];
extern eD2Sounds gaUnitSoundTableModeChange[12];
extern eD2Sounds eD2Sounds_ARRAY_00729358[12];
extern eD2Sounds eD2Sounds_ARRAY_00729388[12];
extern eD2Sounds eD2Sounds_ARRAY_007293b8[12];
extern eD2Sounds eD2Sounds_ARRAY_007293e8[12];
extern eD2Sounds eD2Sounds_ARRAY_00729418[12];
extern eD2Sounds eD2Sounds_ARRAY_00729448[12];
extern eD2Sounds eD2Sounds_ARRAY_00729478[12];
extern eD2Sounds eD2Sounds_ARRAY_007294a8[12];
extern eD2Sounds eD2Sounds_ARRAY_007294d8[12];
extern eD2Sounds eD2Sounds_ARRAY_00729508[12];
extern eD2Sounds eD2Sounds_ARRAY_00729538[12];
extern eD2Sounds eD2Sounds_ARRAY_00729568[12];
extern eD2Sounds eD2Sounds_ARRAY_00729598[12];
extern eD2Sounds eD2Sounds_ARRAY_007295c8[12];
extern int gaUnitSoundIdsBarbarian[11];
extern int gaUnitSoundIdsNecromancer[11];
extern int gaUnitSoundIdsPaladin[11];
extern int gaUnitSoundIdsSorceress[11];
extern int gaUnitSoundIdsAssassin[11];
extern int gaUnitSoundIdsDruid[11];
extern int * gaUnitSoundTableByClassId[7];
extern int gaAmbientMusicLevelSetTable[168];
extern char * gaLanguageNames_0072a300[15];
extern uint32_t gnLightmapQualityLastCheck;
extern D2MissileSrvDoFuncData MISSILECLTCOLLIDEFUNCS[9];
extern int32_t aIntensityTable_Mode0[120];
extern int32_t aIntensityTable_Mode1[120];
extern uint32_t gnDialogDataInitialized;
extern D2SkillDescDamageFunc gapSkillDamageDescFuncTable[36];
extern D2SkillDescAttackFunc gpSkillDescAttackFuncTable[18];
extern int32_t gaSkillDescLayoutPositions[36];
extern char * gaLanguageNames_0072d8d8[15];
extern char * gaLanguageNames_0072d914[15];
extern char * gaLanguageNames_0072d950[15];
extern char * gaLanguageNames_0072d994[15];
extern char * gaLanguageNames_0072d9d0[15];
extern char * gaLanguageNames_0072da0c[15];
extern D2GfxSettingsStrc D2GFXRenderedData;
extern BOOL bAspect;
extern uint32_t IsOSCursorVisible;
extern char * gpszDiabloIIClassNameSource;
// @data gnD2GFX_ActiveScaleFactor
// @wasnamed nD2GFX_ScaleFactor
// @evidence What D2GFX_GetScaleFactor @004f65e0 actually returns; reset to 0x100 at
// 004f65fd. The active value, fed from 0072db40.
extern uint32_t gnD2GFX_ActiveScaleFactor;
extern int gaGroundTileRowOffsets[15];
extern int gaGroundTileRowWidths[15];
extern int gaGroundTileSrcPointers[15];
extern uint32_t gbCursorVisible;
extern uint32_t gbWindowIsActive;
extern D2WinWndHandlerStrc gaUIWindowHandlers[33];
extern uint32_t gbMessageLoopActive;
extern uint32_t gbPaletteGammaShiftEnabled;
extern uint32_t gnTextBoxLineSpacing;
extern int32_t gnLastUsedFontId;
extern uint32_t gnFontIdSmall;
extern uint32_t gnFontIdLarge;
extern uint32_t gnFontIdBody;
extern uint32_t gaCompCharClassTokens[25];
extern uint32_t gnD2CompCOFLayerCount;
extern uint32_t gaCompAnimModeTokens[20];
extern uint32_t gaCompWeaponClassTokens[15];
extern D2CompCharacterCompositeItemStrc gaCompDefaultCompositeItems[283];
extern int gaCompWeaponClassByCodeIndex[14];
extern int gaCompWeaponClassCodeMap[26];
extern unsigned int gnCompWeaponClassCodeMapCount;
extern uint32_t gnGlideTextureMode;
extern int gaPerspectiveImageSizeTable[14];
extern GrTexInfo gTexInfoDebugFontGlide;
extern void * gapGlideStripVertexPtrsA[10];
extern void * gapGlideStripVertexPtrsB[10];
extern D2GlideWallCornerStrc gaGlideWallCornerTable[60];
extern GrTexInfo gTexInfoBlocks0Glide;
extern GrTexInfo gTexInfoBlocks1Glide;
extern GrTexInfo g_GrTexInfoGlBlocks;
extern GrTexInfo gTexInfoGlideSurfaceBlocks;
extern int gnDirectDrawPaletteSize;
extern uint32_t gnDirectDrawGamma;
extern uint32_t gnSoundHardwareAcceleration;
extern int gaAmbientSoundFlags;
// Ambient sound table for area-based background audio. Array of {char* szFilename, int bPlayed} entries. Count stored in gaAmbientSoundFlags. SOUND_PlayNextAmbientSound iterates through this table, alternates with gaAmbientSoundEntryTable based on gnAmbientSoundLanguageCode flag. Reset by SOUND_ResetAmbientSoundFlags.
extern int gAmbientSoundTable[16];
extern int gaAmbientSoundEntryTable[16];
extern uint32_t gnAmbientSoundCount;
extern uint32_t gnSoundGlobalVolume;
extern char s___AUTMESSAGESOURCE___0072f9fc[24];
extern StormLinkedListStrc gFaultHandlerList;
extern char * gaszLanguageNames[32];
extern D2BNetClientCallbacksStrc sgBNetClientCallbacks;
extern HANDLE ghCacheFileHandle;
extern uint32_t gaBnCacheCrc32Table[256];
extern char * szBnNewsChannelName;
extern char s___AUItem_NewsTable___0072ff8c[24];
extern uint32_t g_nRealmLogonResult;
extern uint32_t g_dwProductId;
extern uint16_t gwStrTableDefaultEncoding[256];
extern D2LangTableEntryStrc gaLanguageTable[13];
extern uint16_t gcAbbrevKilSymbol;
extern uint16_t gcAbbrevMilSymbol;
extern uint16_t gcAbbrevBilSymbol;
extern uint16_t gcAbbrevTrilSymbol;
extern short gnNpcMenuQuestIdTable[256];
extern D2Utf8DecodeRowStrc gaUtf8DecodeTable[7];
extern uint32_t CLIENT_SocketToServer;
extern int NET_D2GS_CLIENT_INCOMING_SIZE[181];
extern unsigned short QServerNextToken;
extern uint32_t GameSeed;
extern uint32_t dwCycleLastSecond;
extern int32_t Frames_PerSecond;
extern D2UnitIdToNameFunc PFN_UnitTypeCallbackTable[5];
extern BOOL DontNoSafeGameBool;
extern uint32_t gpInactiveMonsterStateFlagSentinel;
extern uint32_t gnMonsterStateFlagMask1;
extern uint32_t gnMonsterStateFlagMask2;
extern uint8_t gbQuestReconnectMode;
extern int nQuestDataExtendedSize;
extern int NPCINTRO_ListAct1[6];
extern int NPCINTRO_ListAct2[11];
extern int NPCINTRO_ListAct3[7];
extern int NPCINTRO_ListAct5[7];
// Player-animation-mode net-message table, indexed by eD2PlayerAnimMode (20 values, Death..KnockBack). 20 records of {pfnPlayerModeNetMsg *pfnSend; int32 nMsgId; int32 nMsgIdAlt} = 240 bytes, 0x7319e8..0x731ad7; the dword at 0x731ad8 is 0x43, not a code pointer, which bounds the table. DispatchPlayerModeNetMsg @0x5484b0 reads (&base)[mode*3] as the handler, so the function pointer is field 0.
extern D2PlayerModeNetMsgStrc gaPlayerModeNetMsgTable[20];
extern int gnDirectionXOffsets[8];
extern int gnDirectionYOffsets[8];
extern D2ObjectPopulateFnFunc OBJECTSPOPULATEFN[10];
extern int gaObjectSpawnChanceTbl[5];
extern uint32_t gnObjectClassIdLight;
extern int gaObjectSpawnOffsetGroup0[50];
// Room object-populate dispatch table. 7 records of {int32 nEnabled; D2ObjectPopulateFn *pfnPopulate}. Base and stride proven by OBJECT_RollAndDispatchPopulateFn @0x552400, which reads *(int*)(idx*8 + 0x731e68) and *(code**)(idx*8 + 0x731e6c). Index range 1..6 (the mask loop yields 1..7 and the guard requires idx < count <= 7), so 7 records = 56 bytes end exactly at RackType @0x731ea0.
extern D2ObjectPopulateEntryStrc gaObjectPopulateTable[7];
extern D2ObjectSpawnOffsetGroupStrc gaObjectSpawnOffsetTables[4];
extern D2SkillSrvStFunc SKILLSRVSTFUNCS[91];
extern D2SkillSrvDoFunc SKILLSRVDOFUNCS[191];
extern int gnActCount;
extern int gnNPCSpokenBitIndexMapCount;
extern int gaNPCSpokenBitIndexMap[35][2];
extern PFN_ObjectOperateCallback gapObjectOperateCallbacks[10];
extern int gnObjectOperateCallbackCount;
extern D2ObjectOperateFnFunc OBJECTSOPERATEFN[101];
extern int SHRINESCOUNT;
extern char gaQ33_Normal_AmazonBowCodes[5][4];
extern char gaQ33_Normal_SorceressOrbCodes[5][4];
extern char gaQ33_Normal_NecromancerWandCodes[5][4];
extern char gaQ33_Normal_PaladinScepterCodes[5][4];
extern char gaQ33_Normal_BarbarianAxeCodes[5][4];
extern char gaQ33_Normal_DruidHelmCodes[5][4];
extern char gaQ33_Normal_AssassinClawCodes[7][4];
extern char gaQ33_Nightmare_AmazonBowCodes[5][4];
extern char gaQ33_Nightmare_SorceressOrbCodes[5][4];
extern char gaQ33_Nightmare_NecromancerWandCodes[5][4];
extern char gaQ33_Nightmare_PaladinScepterCodes[5][4];
extern char gaQ33_Nightmare_BarbarianAxeCodes[5][4];
extern char gaQ33_Nightmare_DruidHelmCodes[5][4];
extern char gaQ33_Nightmare_AssassinClawCodes[7][4];
extern char gaQ33_Hell_AmazonBowCodes[5][4];
extern char gaQ33_Hell_SorceressOrbCodes[5][4];
extern char gaQ33_Hell_NecromancerWandCodes[5][4];
extern char gaQ33_Hell_PaladinScepterCodes[5][4];
extern char gaQ33_Hell_BarbarianAxeCodes[5][4];
extern char gaQ33_Hell_DruidHelmCodes[5][4];
extern char gaQ33_Hell_AssassinClawCodes[7][4];
extern uint8_t gbQuestA5Q2Unknown;
extern D2MonsterUModHandlerRowStrc g_MonsterUModHandlerTable[43];
extern char * gaLanguageNames_0073c6e0[15];
extern D2MissileSrvDoFuncData gaMissileSrvDoFuncTable[9];
extern int MISSILESSRVHITFUNCS_SIZE;
extern D2MissileSrvHitFunc MISSILESSRVHITFUNCS[71];
extern D2MissileSrvDmgFunc MISSILESSRVDMGFUNCS[31];
extern uint8_t gaSkillLevelCaps[60];
extern D2ItemSpellStrc ITEMSPELLFUNCS[31];
extern uint8_t gaSkeletonComponentsByMasteryLvl[11][9];
extern uint32_t bUseCompressedData;
extern void* gpszExtDcc;
extern void* gpszExtDc6;
extern char s___AUTCELENTRY___00741f14[16];
extern char s___AUCELLIST___00741f2c[16];
extern char s___AV__TSExplicitList_UCELLIST____00741f44[48];
extern char s___AV__TSHashObjectChunk_UCELLIST_00741fb4[52];
extern char s___AUMappingEntryNode___0074362c[24];
extern char s___AUStringEntryNode___0074364c[24];
extern char s___AV__TSExplicitList_UStringEntr_0074366c[56];
extern char s___AV__TSExplicitList_UMappingEnt_007436ac[56];
// Profiling records {name, accumulated ticks, call count}; PROFCORE_AccumulateTime adds into [n], D2ProfileDump prints all 23.
// Provenance: 0061168d-00611699 ADD/ADC [n*0x78+0x743868], ADD [n*0x78+0x743870],1; D2ProfileDump name at ESI-0x68 and loop end 0x744330; the names ("Sample", "Server Loop", ...) start at 0x743800.
extern D2ProfilingDumpFrequency gaProfilingDumpFrequency[23];
extern D2DataTableTxtStrc * sgptDataTable;
extern int gpaMiscTxtTables;
extern char * gaLanguageNames_0074430c[15];
extern char * gaLanguageNames_0074430c_11[15];
// RoomEx activation dispatch, indexed by the room's priority 0..3. Called __fastcall(pRoomEx) from InitNearRooms (0061b407, 0061b460) and AddRoomDataActual (0061b4d1). Four entries: the neighbouring table at 00744394 is a separate array, not elements [4..7] of this one.
extern pfnRoomExActivationHandler gaRoomExActivationHandlers[4];
// RoomEx deactivation dispatch, indexed by the room's priority 0..3. Entries 0-2 are ECX->EAX adapters that tail-jump into DRLGROOMEX_RecalcPriority; entry 3 is ClearRoomExIfFlag0x01. Reached from DRLGROOMEX_PropagateDecRefCount, DRLGROOMEX_SwapActiveRoomEx and the indirect JMP at 0061b5a3.
extern pfnRoomExDeactivationHandler gaRoomExDeactivationHandlers[4];
extern char * gaLanguageNames_007443a8[15];
extern uint32_t gnTimeRates[3];
extern uint32_t aENVIRONMENTDATA1[18];
extern D2DrlgActEnvironmentData aENVIRONMENTDATA3[6];
extern D2DrlgActEnvironmentData aENVIRONMENTDATA2[6];
extern char * gaLanguageNames_007444c8[15];
extern D2SeedStrc * psDataTableSeed;
extern char * gaLanguageNames_00744508[15];
extern char * gaLanguageNames_00744588[15];
extern char * gaLanguageNames_007445c4[15];
extern char * gaLanguageNames_00744600[15];
extern D2InventorySizesStrc gsInventoryBodyLocGridSize;
extern D2InventorySizesStrc g_InventorySizesDefaultConfig;
extern char * gaLanguageNames_00745648[15];
extern char * gaLanguageNames_00745684[15];
extern char * gaLanguageNames_007456c0[15];
extern char * gaLanguageNames_007456fc[15];
extern char * gaLanguageNames_00745738[15];
extern D2ItemsEventMethodStrc gaSkillCalcMethods[7];
extern int32_t gnSkillCalcMethodCount;
extern fpProperties PROPERTIESFUNCTIONS[37];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_007466dc[7];
extern uint8_t gaMonSeqMonsterTblData_Unknown;
extern uint8_t gaMonSeqAnimData;
extern uint32_t DWORD_007467b0;
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00746820[21];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007468a0[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00746948[8];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00746978[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00746a20[16];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00746a80[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00746b28[16];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00746b88[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00746c30[6];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00746c58[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00746d00[16];
extern D2MonSeqMonsterTbls gaMonSeqMonsterTblData[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00746e08[33];
extern D2MonSeqMonsterTbls gaMonSeqMonsterTblsExtra[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00746f78[45];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747088[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747130[32];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007471f0[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747298[8];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007472c8[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747370[17];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007473d8[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747480[20];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007474f8[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_007475a0[16];
extern D2MonSeqMonsterTbls gaMonSeqMonsterTbls[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_007476a8[104];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747918[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_007479c0[12];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747a08[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747ab0[28];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747b58[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747c00[20];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747c78[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747d20[20];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747d98[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747e40[16];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747ea0[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00747f48[17];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00747fb0[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00748058[24];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007480e8[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00748190[16];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_007481f0[14];
extern D2MonSeqTxt D2MonSeqTxt_ARRAY_00748298[20];
extern D2MonSeqMonsterTbls D2MonSeqMonsterTbls_ARRAY_00748310[14];
extern uint32_t gaOverlayVariantCodes[43];
extern uint32_t gaOverlayVariantCodes2[12];
extern char * gaLanguageNames_00748a58[16];
extern char * gaLanguageNames_00748a98[15];
extern int gpsPresetObjectTable[750];
extern int gaFieldStepDeltaX[9];
extern int gaFieldStepDeltaY[9];
extern uint32_t DEFAULT_SECURITY_COOKIE;
extern double DOUBLE_0074adb4;
// CRT x87 dispatch: maps the FXAM condition bits (C3,C2,C1,C0 folded into 4 bits) to a byte offset into the caller's per-class jump table.
// Provenance: 00698548/006985af MOV EBX,0x74adbc; XLAT; MOVSX; ADD EBX,EDX+0x10; JMP [EBX] (code not yet in a function).
extern uint8_t gabyFxamClassOffset[16];
extern unsigned int gaCRT_FPremData_3;
extern uint8_t gaCRT_PKWareLitLenBitsTableC[256];
extern BOOL BOOL_0074c020;
extern uint32_t gsD3DTextureDescTemplate[8];
extern uint16_t gaD3DPerspectiveStripIndices[22];
extern D2D3DTileCornerStrc gaD3DWallCornerTable[60];
extern char * gnExcelRecordFieldNotFound;
extern int TokenHashMapBucketSize;
extern uint32_t nTokenBucketMask;
extern uint32_t gnQServerIpConnCountModulo;
extern uint32_t gnQServerHacklistBucketCount;
extern char s___AUCMDDEF___0074c47c[16];
extern D2StormListHead gStormCmdListHeadBackslash;
extern D2StormListHead gStormCmdListHeadSlash;
extern HANDLE ghCurrentProcessHandle;
extern eD2ApplicationMode geD2ApplicationMode;
extern SERVICE_STATUS_HANDLE ghServiceCtrlHandler;
extern HINSTANCE WinMain_hInstance;
extern uint32_t gnServiceRunning;
extern BOOL D2Ini_bUseDirect;
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of DeleteCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0074c830;
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of DeleteCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0074d848;
extern uint32_t g_nProjectId;
extern char * szProduct;
extern char * g_szApplicationName;
extern char * g_szModuleName;
extern char * g_szMilestone;
extern char * g_szBuildNumber;
extern char * g_szPlatform;
extern char * g_szPrimaryAssignment;
extern char * g_szAssignments;
extern int gbSystemInfoInitialized;
extern uint32_t g_eOSType;
extern int fGameVersion;
extern uint32_t g_dwTotalPhysicalMemory;
// lpSystemInfo parameter of GetSystemInfo
extern _SYSTEM_INFO gSystemInfo;
extern char gszCrashReportProcessPath[2048];
extern uint32_t g_dwErrorState;
extern uint32_t gbFogErrorManagerInitialized;
extern int gnAsyncDataPriorityDeferDepth;
extern D2AsyncData_Line0x128 * gpAsyncDataDeferredList;
extern uint32_t gbAsyncDataInitialized;
extern uint32_t gnAsyncDataHandleBufferSize;
extern D2AsyncDataStrc * pD2AsyncDataStrc;
extern uint32_t gaHuffmanFastDecodeTable[256];
extern uint8_t gabHuffmanNextTable[256];
extern uint64_t gaServerSentPacketByteCounts[256];
// Running 64-bit total of payload bytes passed to NET_D2GS_SERVER_LogSentPacket (nSize); zeroed by NET_SERVER_SentPacketLog_Clear.
// Provenance: 0040b181-0040b1a4 ADD [lo],x / ADC [hi],0.
extern unsigned long long gnSentPacketRawBytes;
// Running 64-bit total of encoded bytes on the wire: ceil(huffman bits/8) plus the 1- or 2-byte length header.
// Provenance: 0040b181-0040b1a4 ADD [lo],x / ADC [hi],0.
extern unsigned long long gnSentPacketWireBytes;
// Running 64-bit total of length-header bytes (1, or 2 when the encoded size is >= 0xf0).
// Provenance: 0040b181-0040b1a4 ADD [lo],x / ADC [hi],0.
extern unsigned long long gnSentPacketHeaderBytes;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0075cfbc;
extern uint32_t gnDbgSymInitRefcount;
// cp parameter of inet_addr
extern char cp_0075d040[256];
extern int gLogMessageQueueCount;
extern int gLogMessageQueueTotalBytes;
// 20 log-file slots of 16 chars, i.e. char[20][16] - stride 0x10, not char[16][20]. Proof: FindOrOpenLogFileSlot indexes gLogFileNameArray[nSlot] with nSlot 0..19 and advances the walker by 0x10 (0040ee1e SHL EDI,4), CloseLogFileByName steps 0x10 and bails at nSlot > 0x13, and the length guard halts on SStrLen > 0xf. HAZARD for the reconstruction: the scan loop at 0040eef0 terminates on the hard-coded address 0x75d288, which is one-past-the-end of this array. The generator binds that constant to the next symbol (gLogRealmStartupLocalTime) instead of to &gLogFileNameArray[20]; in the recompiled binary the two are ~44 KB apart, so the loop runs ~2810 iterations and nSlot runs far out of bounds.
extern char gLogFileNameArray[20][16];
// SYSTEMTIME filled by GetLocalTime in StartLogWriterThread (00410562) and read field-by-field by WriteRealmStartupHeader (0040ed7a..0040eda8). Was mistyped as a 2-byte short with its wMonth/wDay/wHour/wMinute/wSecond/wMilliseconds members labelled as six separate globals (gRealmStartupPacketField1..3, gNetworkStateData3..5); those labels were the struct members and have been folded in. Note 0075d288 is also one-past-the-end of gLogFileNameArray and is used as the scan-loop terminator in FindOrOpenLogFileSlot (0040eef0).
extern _SYSTEMTIME gLogRealmStartupLocalTime;
// gLogFileSlotMutex is a 20-element int array, one flag per log-file slot, not a scalar. StartLogWriterThread (004105a4..004105e0) sets all 20 to 1 with unrolled stores; FindOrOpenLogFileSlot reads gLogFileSlotMutex[nSlot] and clears it after WriteRealmStartupHeader. It was typed as a single int with the other 19 elements carrying auto-generated labels gaLogFileSlotMutex_0..12, which made the generator emit them as function-local statics in StartLogWriterThread while the indexed access ran off the 4-byte scalar into gLogWriterThreadHandle / gLogManagerInitialized / gLogWriterThreadStopFlag / gLogWriterThreadId / gLogMessageQueueHead / gpLogMessageQueueHead. Extent 0075d298..0075d2e7 ends exactly at gLogWriterThreadHandle.
extern int gLogFileSlotMutex[20];
extern HANDLE gLogWriterThreadHandle;
extern uint32_t gLogManagerInitialized;
extern uint32_t gLogFileLastAccessTimeArray[20];
extern char gLogFilePathArray[20][260];
extern FILE * globalOpenFileHandleArray[20];
extern uint32_t gLogFileLastWriteTimeArray[20];
extern uint32_t gLogWriterThreadStopFlag;
// lpThreadId parameter of CreateThread
extern LPDWORD gLogWriterThreadId;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern CRITICAL_SECTION gLogMessageQueueCS;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION gLogFileCS;
extern D2StormListHead gLogMessageQueueHead;
extern D2HighResTimestampStrc gHighResTimerInitTimestamp;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
extern CRITICAL_SECTION gHighResTimerCS;
extern SMemHeapStatsStrc gaSMemHeapStats[4096];
extern int g_SMemLastTickCount;
extern void * g_SMemHeapBuckets[256];
extern uint32_t g_SMemPageSize;
extern uint32_t ProtectMemory;
extern CRITICAL_SECTION DebugMemoryCriticalSection[256];
extern CRITICAL_SECTION gUICtrlSkillColumnState[256];
extern CRITICAL_SECTION gUICtrlPanelArrowState[256];
extern uint32_t g_SMemUnknownFlag;
extern uint32_t DebugMemory;
extern int g_SMemHeapCleanupFlags[256];
extern void * g_SMemPendingCleanupHeap;
extern uint32_t g_SMemDebugFillEnabled;
extern uint32_t g_SMemInitialized;
extern unsigned int guStringAlignmentTemp;
extern uint32_t gbSStrConversionTablesInitialized;
extern double gdPow10Table[200];
extern IDirectSound * g_pDirectSound;
extern uint32_t gbAsyncIOWorkerShutdown;
// lpFilename parameter of GetModuleFileNameA
extern char szSFileBasePath[260];
extern D2StormAsyncOpStrc * gpCurrentAsyncIORequest;
extern pfnSFileReadError pfnSFileReadErrorHandler;
extern HANDLE ghAsyncIOThread;
extern uint32_t gnSFileWaveStreamBufferSize;
extern void * gpSFileExplodeWorkBuffer;
extern uint32_t gbSFileAsyncIOShutdown;
extern void * g_pStormMpqCryptTable;
extern uint32_t gnSFileAsyncBudgetBytesPerSec;
extern void * ghAsyncIOCompletionPort;
extern HANDLE ghAsyncIOEvent;
extern uint32_t gnSFileDirectAccessFlags;
extern uint16_t gnSFileSearchLocale;
extern uint8_t gnSFileSearchHashOffset;
extern pfnSFileOpenCallback fpSFileOpenCallback;
extern void * gpSFileOpenCallbackParam;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_00779068;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_00779080;
extern uint32_t gnSSystemOption3;
extern uint32_t gnSSystemOption1;
extern uint32_t gnSSystemWaveBufferSize;
extern uint32_t gnSSystemAsyncReadSize;
extern uint32_t gbCompressionDebugOutput;
extern SErrAppFatalContextStrc gpSSystemThreadContextCallback;
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_007790cc;
extern void * gpErrorModuleList;
extern uint32_t gbFormatErrorModulesInitialized;
extern uint32_t gbSSystemShutdownInProgress;
extern uint32_t dwLastErrorCode;
extern pfnBlizzardError pfnBlizzardErrorCallback;
extern void * gpScompTempBuffer;
extern uint32_t gnScompTempBufferOwnerThreadId;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_00779214;
// @blizzard s_critsect[4] (SLOG.CPP)
extern CRITICAL_SECTION gaSLogCritSect[4];
// @blizzard s_loghead[4] (SLOG.CPP)
extern D2SLogStrc * gaSLogHead[4];
extern uint32_t gpActiveWindow;
extern uint32_t gnSevtSnapshotActive;
extern void * gpEventHashBuckets;
extern void * gaEventRegistryHead;
extern uint32_t gnSevtRegisteredEventCount;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0077930c;
extern uint32_t gbTermsOfServiceCancelled;
extern D2ControlStrc * bCharacterCreationInProgress;
extern D2WinEditBox * gpBnetPasswordEditBox;
extern D2WinTextBox * gpCtlTcpIpConnectAddressLabel;
extern D2WinTextBox * gpCtlCharCreateBNetRealmTextBox;
extern D2WinEditBox * gpCtlChangePasswordInput;
extern D2WinTimer * gpMainMenuTcpIpTimerControl;
extern D2WinButton * gpCtlCreateCharLadderCheckbox;
extern D2WinEditBox * gpaCharNameBuffers;
extern D2ControlStrc * FORMS_pListExEx[64];
extern D2ControlStrc * FORMS_pList[64];
extern char gszPasswordChangeNewPasswordMB[24];
extern D2WinTextBox * gpCtlCreateCharExpClassPanel;
// Character-create class anim-image controls, in screen order:
// [0] Barbarian [1] Necromancer [2] Amazon [3] Sorceress [4] Paladin [5] Assassin [6] Druid
// Classic walks [0..4] (loop bound base+20); expansion walks [0..6] (base+28) - see ClickOnClassCreate 00433bf0.
// Was seven separate D2ControlStrc* globals; merged because both consumers walk the run strided.
extern D2ControlStrc * gaAnimImgCharCreateByClass[7];
extern D2WinTextBox * gpCtlTcpIpLocalIpTextBox;
extern D2WinEditBox * gpBnetAccountEditBox;
extern D2WinTextBox * gpMainMenuGatewayListTextBox;
extern D2ControlStrc * gpCtlBNetPasswordChangeButton;
extern D2ControlStrc * gpCtlCreateCharCancelButton;
extern uint32_t gnTcpIpConnectionSuccessFlag;
extern D2ControlStrc * gpCtlTcpIpAddressButton;
extern D2ControlStrc * gpCtlTcpIpConnectButton;
extern D2WinEditBox * gpBnetPasswordConfirmEditBox;
extern D2WinButton * gpCtlRealmNameButton;
extern D2WinTextBox * gpCtlCreateCharHardcoreLabel;
extern D2ControlStrc * gpCtlCreateCharExpansionCheckboxImage;
extern D2WinTextBox * gpCtlCreateCharLadderLabel;
extern pfnUIMenuInstance gpCurrentUIMenuInstance;
extern D2ControlStrc * gpCtlCreateCharHardcoreCheckbox;
extern D2WinButton * gpCtlCreateCharExpansionCheckbox;
extern D2WinTextBox * gpCtlCreateCharOKButton;
extern D2IniConfigStrc * D2IniData_launcher;
extern D2WinTextBox * gpCtlClassDescriptionTooltipPrimary;
extern D2WinTextBox * gpCtlClassDescriptionTooltipSecondary;
extern D2WinTextBox * gpCtlTcpIpAddress;
extern D2WinTextBox * gpCtlTcpIpPort;
extern uint32_t gnSelectedCharGameState;
extern eOogCurrentCharSelectionMode OogCurrentCharSelectionMode;
extern uint32_t gnIsFecharPalette;
extern uint32_t gnBNetBusyFlag;
extern char gszLastTcpIpAddress[256];
extern DC6 * DC6_FrontEnd_blizno;
extern DC6 * DC6_FrontEnd_trademark;
extern DC6 * DC6_FrontEnd_Diablo2;
extern DC6 * DC6_FrontEnd_D2logoBlackLeft;
extern DC6 * DC6_FrontEnd_D2logoFireLeft;
extern DC6 * DC6_FrontEnd_D2logoBlackRight;
extern DC6 * DC6_FrontEnd_D2logoFireRight;
extern DC6 * DC6_FrontEnd_TitleScreen_or_gameselectscreenEXP;
extern DC6 * DC6_CURSOR_ohand;
extern DC6 * DC6_CharSelect_creditsbckg;
extern DC6 * DC6_FrontEnd_CharacterCreate_or_charactercreationscreenEXP;
extern DC6 * DC6_FrontEnd_fire;
extern DC6 * DC6_CharSelect_Realmselectbutton;
extern DC6 * DC6_CharSelect_TallButtonBlank;
extern DC6 * DC6_CharSelect__characterselectscreenEXP;
extern DC6 * DC6_FrontEnd_WideButtonBlank;
extern DC6 * DC6_FrontEnd_MediumButtonBlank;
extern DC6 * DC6_CharSelect_ShortButtonBlank;
extern DC6 * DC6_FrontEnd_MediumSelButtonBlank;
extern DC6 * DC6_FrontEnd_textbox2;
extern DC6 * DC6_FrontEnd_3WideButtonBlank;
extern DC6 * DC6_FrontEnd_EditTextBoxWide;
extern DC6 * DC6_FrontEnd_WideButtonBlank02;
extern DC6 * DC6_FrontEnd_NarrowButtonBlank;
extern DC6 * DC6_FrontEnd_GWListBox;
extern DC6 * DC6_FrontEnd_TermsofUseBckg;
extern DC6 * DC6_CharSelect_charselectbckg;
extern DC6 * DC6_CharSelect_DifficultyLevels;
extern DC6 * DC6_FrontEnd_CinematicsSelection;
extern DC6 * DC6_FrontEnd_CinematicsSelectionEXP;
extern DC6 * DC6_CharSelect_charselectbox;
extern DC6 * DC6_CharSelect_charselectboxgrey;
extern DC6 * DC6_FrontEnd_PopUpOKCancel;
extern DC6 * DC6_FrontEnd_PopUpOKCancel2;
extern DC6 * DC6_CharSelect_selectchararrows;
extern DC6 * DC6_FrontEnd_CurRealm_or_Realmselect;
extern DC6 * DC6_FrontEnd_NarrowButtonBlank02_or_RealmSelectbuttonthin;
extern DC6 * DC6_CharSelect_CreateNewCharImage;
extern DC6 * DC6_FrontEnd_PopUpOK;
extern DC6 * DC6_FrontEnd_PopUp_340x224;
extern DC6 * DC6_FrontEnd_PopUpWide;
extern DC6 * DC6_FrontEnd_PopUpLargest;
extern DC6 * DC6_FrontEnd_TileableDialog;
extern DC6 * DC6_FrontEnd_PopUpOK2;
extern DC6 * DC6_FrontEnd_OkCancelButtonBlank;
extern DC6 * DC6_FrontEnd_CancelButtonBlank;
extern DC6 * DC6_CharSelect_password;
extern DC6 * DC6_FrontEnd_textbox;
extern DC6 * DC6_FrontEnd_clickbox;
extern DC6 * DC6_FrontEnd_joingameclickboxgrey;
extern DC6 * DC6_FrontEnd_realmbckg;
extern DC6 * DC6_FrontEnd_joingamescrollbars;
extern DC6 * DC6_FrontEnd_tcpipbckg_or_tcpipscreen;
extern DC6 * DC6_FrontEnd_IPAddressBox;
extern DC6 * DC6_FrontEnd_downloadpatchbckg;
extern DC6 * DC6_FrontEnd_downloadpatchball2;
// Character-create class preview DC6s. Seven contiguous 5-element runs follow, one per class, in
// element order [0]=nu1 [1]=nu2 [2]=fw [3]=nu3 [4]=bw (see the load sequence in IMAGE_LoadPredefinedImagesEx).
// Each run is freed by a strided loop bounded at head+20 in IMAGE_FreePredefinedImages, so each run is
// one array, not five independent globals. Three of the old element labels named the wrong class
// (gpImageSorceressWeapon was barbarian\banu2, gpImagePaladinWeapon was sorceress\sonu2,
// gpImageNecromancerWeapon was paladin\panu2).
extern DC6 * gaDC6CharCreateAssassin[5];
extern DC6 * gaDC6CharCreateDruid[5];
extern DC6 * gaDC6CharCreateAmazon[5];
extern DC6 * gaDC6CharCreateNecromancer[5];
extern DC6 * gaDC6CharCreateBarbarian[5];
extern DC6 * gaDC6CharCreateSorceress[5];
extern DC6 * gaDC6CharCreatePaladin[5];
extern DC6 * gaDC6CharCreateNecromancerSel[3];
extern DC6 * gaDC6CharCreateSorceressSel[3];
extern DC6 * DC6_FrontEnd_barbarian_bafws;
extern DC6 * DC6_FrontEnd_paladin_pafws;
extern uint32_t gbCinematicsMenuShown;
extern uint32_t fpInsertPlayCd;
extern int fpLoadAllMediaMpqCallback;
extern uint32_t gbCdCheckPassed[25];
extern int FORMS_nListExExCount;
extern int FORMS_nListCount;
extern uint32_t gbMainMenuUIActive;
extern D2WinEditBox * gpMainMenuFormPtrs;
extern D2WinTextBox * gpMainMenuCreditsPairLeftTextBox;
extern D2WinTextBox * gpMainMenuCreditsPairRightTextBox;
extern D2WinTextBox * gpMainMenuCreditsLineTextBox;
extern int gnMainMenuCreditsFrameIndex;
extern uint32_t gnPaletteLoadedFlag;
extern uint32_t gnMainMenuConnectFlag;
extern D2WinTextBox * gpPatchProgressTextBox;
extern D2WinImage * gpCtlPatchProgressAnimImage;
extern uint32_t gbMainMenuShowBNetRealmPanel;
extern D2BNetRealmEntryStrc * * g_pBNetRealmList;
extern int gnRealmListCount;
extern uint32_t gnMCPConnectThreadHandle;
extern D2ControlStrc * gpCtlBnetProgressBar2;
extern D2WinTextBox * gpCtlBnetProgressBar;
extern D2WinTextBox * gbBnetConnectionTimerActive;
extern int gbBnetProgressActive;
extern HANDLE ghBnetConnectHandle;
extern unsigned int gbBNetConnectionProgressFlag;
extern D2WinEditBox * gpCharSelAccountNameInputControl;
extern D2WinEditBox * gpCtlDisconnectMessage1;
extern D2WinEditBox * gpCtlDisconnectMessage2;
extern D2WinEditBox * gpCtlDisconnectMessage3;
extern D2WinEditBox * gpnEmailBuffer;
extern D2WinEditBox * gpCtlEnterEmailInput;
extern D2ControlStrc * gpCtlEmailConfirmButton;
extern uint32_t gbEmailAddressScreenReady;
extern D2WinEditBox * gpCharSelPasswordInputControl;
extern D2WinEditBox * gpCtlDisconnectDialog;
extern D2ControlStrc * gpCtlBnetPasswordOkButton;
extern D2WinTextBox * gpWinTextBoxRealmSelect;
extern D2ControlStrc * gpCtlRealmSelectionPanel;
extern D2WinTextBox * gpCharSelDeleteButton;
extern D2WinButton * gpCtlCharSelDownArrow;
extern D2ControlStrc * gpCharSelExitButton;
extern int gnCharSlotYPositions[8];
extern D2WinButton * gpCtlCharSelOKButton;
extern D2WinTextBox * gaCharSelScreenSlotControls[8];
extern char PreferredClosedRealm[256];
extern D2ControlStrc * gaCharSelExpansionIconControls[8];
extern D2WinTextBox * gpaCharSelectDisplay;
extern D2ControlStrc * gpCtlCharSelectGameTypeBtn;
extern D2ControlStrc * gpCtlCharSelDeleteButton;
extern uint32_t gnCharSelSelectedIndex;
extern D2WinTextBox * gpCharSelExpansionToggle;
extern D2ControlStrc * gnCharSelImageOffset;
extern D2WinImage * gpControlCharSelImage;
extern D2WinTextBox * gpCtlRealmSelectionLabel;
extern D2WinButton * gpCharSelScreenControl;
extern D2WinTextBox * gpCtlCharSelScrollbar;
extern char PreferedClosedRealm2[256];
extern D2ControlStrc * gnRealmSelectionOffset;
extern D2WinTextBox * gpControlCharSelList;
extern D2ControlStrc * gpControlCharSelArrows;
extern D2WinImage * gpaCharSelCharacterList;
extern D2ControlStrc * gpControlRealmSelectDropdown;
extern D2WinTextBox * gpControlRealmSelect;
extern uint32_t gbCharSelectInProgress;
extern int nCharacterSelectionIndex;
extern uint32_t gnCharSelBNetLoadState;
extern char * gszPendingRealmName;
extern uint32_t gbCharSelScreenWaitTimeout;
extern uint32_t gbCharSelArrowsHidden;
extern D2CharSelStrc * gpCharSelPendingEntry;
extern int32_t g_nCharSelNavigationIndex;
extern uint32_t gpCharSelScrollbar;
extern D2WinTextBox * gpCharSelStatusTextBox;
extern D2WinTextBox * gnCharSelScrollOffset;
extern uint32_t gnCharSelLoadedCharCount;
extern uint32_t gnCharSelCreateNewButtonClickCount;
extern D2RealmCharListCacheEntryStrc * gpRealmCharListCache;
extern uint16_t pBetaRealmNames[7][256];
extern D2BNetClientCallbacksStrc * gpBNetCallbacks;
extern uint32_t gnBusyFlag;
extern uint32_t gbLauncherBnetConnectInProgress;
extern D2SaveFileHeaderStrc SAVEFILE;
extern int32_t gnLadderEntryTotal;
extern int32_t gnLadderListRowCount;
// Eleven lobby user-list row buttons. A stray label on element 1 kept it at one pointer, so rows 1..10 were created into, compared against and destroyed through adjacent globals.
extern D2WinButton * gaChatUserListButtonArray[11];
extern uint32_t gbChatOptionButtonState;
extern D2ControlStrc * FORMS_pListEx[32];
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0077ae28;
extern char gszOogChatInputBuffer[512];
extern char gszLadderCharNameFilter[16];
extern D2ControlStrc * gpCtlLobbyChat[2];
extern D2LobbyControlsStrc gpLobbyForm0;
extern D2BNetClientCallbacksStrc * pNetworkUnknown;
extern uint32_t gnLadderScrollPageSize;
extern uint32_t ghBannerAdProcess;
extern char * gapszProfileReadBuffers[3];
extern char gszOogReplyCommandBuffer[256];
extern char gaChatDlgHistoryBuffer[10][256];
extern D2IniConfigStrc * D2IniData_multiplayer;
extern uint16_t LastGameRequestId;
extern uint16_t gwLastGameInfoRequestId;
extern void * gpChatFormList;
extern int FORMS_nListExCount;
extern uint32_t gbLadderNewsExpanded;
extern uint32_t gdwGameJoinRequestTick;
extern uint32_t gnLadderScrollPos;
extern uint32_t gbLobbyInitialized;
extern D2CharSelCompStrc * gpChatDlgCharSelCompositeUnit;
extern DC6 * gpDC6BnetBackground;
extern DC6 * gpDC6WaitingRoomBackground;
extern DC6 * gpDC6PopUpLargest2;
extern DC6 * gpDC6MediumButtonBlank2;
extern DC6 * gpDC6CursorOpenHand;
extern DC6 * gpDC6CursorLRotate;
extern DC6 * gpDC6ChatScrollbar;
extern DC6 * gpDC6JoinGameScrollbar;
extern DC6 * gpDC6ChatLeftButtons1;
extern DC6 * gpDC6ChatLeftButtons2;
extern DC6 * gpDC6ChatLeftButtons3;
extern DC6 * gpDC6ChatRightTopButtonsAlt;
extern DC6 * gpDC6ChatRightTopButtons;
extern DC6 * gpDC6ChatArrows;
extern D2WinTextBox * gpCtlConfirmDialog;
extern D2ControlStrc * gpCtlLobbyAccountList;
extern D2WinButton * gpCtlLobbyLadderNewsToggle;
extern D2WinTextBox * pGameListCache;
extern DC6 * DCC_JoinGame;
extern DC6 * gpDC6BlankBckg2;
extern DC6 * gpDC6GemButton;
extern DC6 * gpDC6GameButtonBlank;
extern DC6 * gpDC6JoinGameClickBox;
extern DC6 * gpDC6GreyLevelBox;
extern DC6 * gpDC6RadioButton;
extern DC6 * gpDC6RadioButtonGrey;
extern DC6 * gpDC6PopUpOk;
extern DC6 * gpDC6CancelButtonBlankAlt;
extern DC6 * gpDC6OverallTab;
extern DC6 * gpDC6AmazonTab;
extern DC6 * gpDC6SorceressTab;
extern DC6 * gpDC6NecromancerTab;
extern DC6 * gpDC6PaladinTab;
extern DC6 * gpDC6BarbarianTab;
extern DC6 * gpDC6DruidTab;
extern DC6 * gpDC6AssassinTab;
extern D2WinButton * gpCtlLadderBtnOverall;
extern D2WinButton * gpCtlLadderBtnAmazon;
extern D2WinButton * gpCtlLadderBtnSorceress;
extern D2WinButton * gpCtlLadderBtnNecromancer;
extern D2WinButton * gpCtlLadderBtnPaladin;
extern D2WinButton * gpCtlLadderBtnBarbarian;
extern D2WinButton * gpCtlLadderBtnDruid;
extern D2WinButton * gpCtlLadderBtnAssassin;
extern DC6 * gpDC6SquelchAura;
extern DC6 * gpDC6SelectAura;
extern DC6 * gpDC6NumberArrows1;
extern DC6 * gpDC6NumberArrows2;
extern DC6 * gpDC6NumberArrows3;
extern DC6 * gpDC6NumberArrows4;
extern DC6 * gpDC6NumberGreyArrows;
extern DC6 * gpDC6CreateJoinBckg;
extern DC6 * gpDC6OkayButton;
extern D2ControlStrc * gpCtlChannelDlgBackground;
extern D2WinTextBox * gpCtlChannelDlgTitle;
extern D2WinTextBox * gpCtlChannelNameLabel;
extern D2WinTextBox * gpCtlChannelListLabel;
extern D2ControlStrc * gpCtlChannelOkButton;
extern D2ControlStrc * gpCtlChannelDlgCancelButton;
extern D2WinEditBox * gpCtlChannelNameEditBox;
extern D2WinTextBox * gpCtlChannelList;
extern D2WinTextBox * g_pLadderBoardTextBox;
extern D2ControlStrc * ghWndGameNameEditBox;
extern DC6 * gpDC6CreateGameBckg;
extern DC6 * gpDC6CreateGameButton;
extern DC6 * gpDC6WideButtonBlank;
extern DC6 * gpLauncherImageResource2;
extern DC6 * gpDC6YesNoButtonBlank;
extern DC6 * gpDC6LadderChoiceEXP;
extern DC6 * gpDC6HardcoreLadderBckg;
extern DC6 * gpDC6PlayerProfileBckg;
extern DC6 * gpDC6PlayerProfileBckg2;
extern DC6 * gpDC6HelpBckg;
extern DC6 * gpDC6D2LogoBlackBnet;
extern DC6 * gpDC6D2LogoBnet;
extern uint32_t gnChatHistoryEntryCount;
extern uint32_t gnNotifyEnabled;
extern uint32_t gbIsChannelScreen;
extern uint32_t gdwLadderRankRequestTimeout;
extern uint32_t gnMCPConnectionRestoreResult;
// Initial item list of FORM_List record gaPredefinedForms_b[30] (pTypeArg1 -> D2WinList pnItemArray): zero-terminated
// locale string ids. .bss and never written, so the list starts empty. Neither record is referenced by address in 1.14d.
// Provenance: D2WinList 005075e0 reads *pnItemArray until 0; size = extent to the next symbol (0077bd74 / 0077bd84).
extern int gaFormList30InitialItems[4];
// Initial item list of FORM_List record gaPredefinedForms_b[32] (pTypeArg1 -> D2WinList pnItemArray): zero-terminated
// locale string ids. .bss and never written, so the list starts empty. Neither record is referenced by address in 1.14d.
// Provenance: D2WinList 005075e0 reads *pnItemArray until 0; size = extent to the next symbol (0077bd74 / 0077bd84).
extern int gaFormList32InitialItems[4];
extern int32_t gaChatTextBoxFontParams[2];
extern D2WinTextBox * gpChatMessagesTextBox;
extern D2WinEditBox * gpChatDlgInputEditBox;
extern D2WinTextBox * BattleNetNews;
extern D2WinTextBox * gpControlJoinGameError;
extern uint32_t gnOpenBNetMode;
extern ChatDlgCallback gpfnChatDlgCallback;
extern void * gpChatMessageQueueHead;
extern void * gpChatMessageQueueTail;
extern void * gpChatPlayerNameListHead;
extern unsigned int gdwLastGameListRefreshTick;
// Former interior labels: gszLanGameListEntryName=+0x1 gszLanGameListEntryDescription=+0x11 gbOOGLobbyScreenStatePadding=+0x12 gwOOGLobbyScreenState=+0x31 gwGameListCacheEntryPort=+0x33 gdwGameListCacheEntryFlags=+0x37 gPlayerInputState1=+0x6e gbDamageModConfigLoaded=+0xa3a5
extern D2GameListEntryStrc gbLanGameListEntryNamePadding2[1000];
// Former interior labels: gaLanGameListEntryName=+0x1 gChatBufferData3=+0x2 gChatBufferData2=+0x3 gwLanGameListEntryPortPrev=+0x31 gnLanGameListEntryPort=+0x33 gnLanGameIPAddr=+0x37 gChatBufferData1=+0x3c gSaveFileBuffer=+0xbd2c gbSaveKeyBindingsFlag=+0xc12b gbOOGUnusedFlag_797261=+0xce21 gbUnusedOOGDataFlag=+0xd029 gbOOGUnusedFlag_79746c=+0xd02c
extern D2GameListEntryStrc gbLanGameListEntryNamePadding[1000];
extern char gszOOGRequestedGameName[256];
extern D2WinTextBox * gpOOGGameListTextBox;
extern D2WinTextBox * gpOogGameListDetailsPanelControl;
extern D2ControlStrc * gpOOGJoinGameButton;
extern D2ControlStrc * gpCtlGameListStatus;
extern D2ControlStrc * gpOOGLobbyControl2;
extern D2WinEditBox * gpOogGamePasswordEditControl;
extern D2WinEditBox * gpCtlGameListTop;
extern D2WinTextBox * gpCtlJoinGameLabel;
extern D2WinTextBox * pUIGameUsername;
extern D2WinTextBox * pUIGamePassword;
extern D2WinTextBox * pUIGamePlayers;
extern D2ControlStrc * gpCtlCreateGameDlgBackground;
extern D2WinTextBox * gpCtlCreateGameDlgTitle;
extern D2WinTextBox * gpCtlGameNameLabelOrQueueStatus;
extern D2WinTextBox * gpCtlCreateGamePasswordLabel;
extern D2WinTextBox * gUIGameListServerMessageText;
extern D2WinTextBox * gUIGameListMaxPlayersLabel;
extern D2WinTextBox * gUIGameListLevelRestrictionLabel;
extern D2WinTextBox * gUIGameListLevelRestrictionLevelsLabel;
extern D2ControlStrc * gUIGameListMaxPlayersInactiveImage;
extern D2ControlStrc * gUIGameListMaxPlayersInactiveArrowButton;
extern D2ControlStrc * gUIGameListMaxPlayersUpButton;
extern D2WinButton * gUIGameListMaxPlayersDownButton;
extern D2ControlStrc * gUIGameListLevelRestrictionUpButton;
extern D2WinButton * gUIGameListLevelRestrictionDownButton;
extern D2ControlStrc * gUIGameListLevelRestrictionInactiveArrowButton;
extern D2ControlStrc * gUIGameListCreateButton;
extern D2WinEditBox * gpOogGameNameEditControl;
extern D2WinEditBox * gUIGameListGameNameEditBox;
extern D2WinEditBox * gUIGameListGamePasswordEditBox;
extern D2WinEditBox * gUIGameListMaxPlayersEditBox;
// Edit box control handle for level restriction field in Create Game dialog (TCP/IP mode only). Populated in OOGUTIL_OpenCreateGameDialog, read by NET_MCP_CLIENT_PrepareGameCreatePacket. Part of Create Game UI state cluster at 0x00798c28-0x00798c64.
extern D2WinEditBox * gUILevelRestrictionEditBox;
extern D2WinButton * gUIGameListLevelRestrictionCheckbox;
extern D2ControlStrc * gUIGameListLevelRestrictionInactiveImage;
extern D2ControlStrc * gpCtlCreateGameDlgCancelButton;
extern D2WinButton * gUIGameListDifficultyNormalButton;
extern D2WinButton * gUIGameListDifficultyNightmareButton;
extern D2WinButton * gUIGameListDifficultyHellButton;
extern D2WinTextBox * gUIGameListStatusLabel;
extern D2WinTextBox * gUIGameListPlayerListText;
extern D2WinTextBox * gUIGameListActLabel;
extern D2ControlStrc * gpProfileViewBackground;
extern D2WinTextBox * gpProfileViewAccountName;
extern D2WinTextBox * gpProfileViewSex;
extern D2WinTextBox * gpProfileViewLocation;
extern D2WinTextBox * gpProfileViewDescription;
extern D2ControlStrc * gpProfileEditBackground;
extern D2WinTextBox * gpProfileEditAccountName;
extern D2WinEditBox * gpProfileEditSex;
extern D2WinEditBox * gpProfileEditLocation;
extern D2WinEditBox * gpProfileEditDescription;
extern D2ControlStrc * gpProfileCloseButton;
extern D2WinTextBox * gUIPlayerProfileCharacterNameLabel;
extern D2WinTextBox * gUIPlayerProfileCharacterLevelLabel;
extern D2WinTextBox * gUIPlayerProfileCharacterClassLabel;
extern D2WinTextBox * gUIPlayerProfileExperienceLabel;
extern D2ControlStrc * gpProfileOKButton;
extern D2ControlStrc * gpProfileLadderButton;
extern D2WinTextBox * gUIPlayerProfilePlaytimeLabel;
extern int32_t gnProfileDialogMode;
extern D2WinTextBox * gpChatCommandsTitle;
extern D2WinTextBox * gpChatCommandsTextBox;
extern D2ControlStrc * gpChatCommandsOKButton;
extern D2ControlStrc * gpChatCommandsBackground;
extern uint32_t gnGameListFilterState;
extern uint32_t gnMCPCreateGamePendingFlag;
extern uint32_t gnFilteredGameListCount;
extern int nGameListDifficultyFilter;
extern uint32_t CreateGameDifflevel;
extern char gszProfileCharacterName[16];
extern int gnProfileUserHardcoreFlag;
extern int gnProfileUserExpansionFlag;
extern char gszCurrentChannelName[256];
extern D2ChannelUserStrc * gpChannelUserListHead;
extern void * gpChannelUserLinkedListHead;
extern int gnChannelUserTotalCount;
extern int gnChannelUserCount;
extern uint32_t gnChannelUserListScrollOffset;
extern void * gpCurrentChatRoomContext;
extern uint32_t gdwLastChatSendTick;
extern uint32_t g_nMCPGameDifficulty;
extern uint32_t g_bMCPCharCreateSuccess;
extern char g_szMCPGamePassword[28];
extern uint32_t IsMcpGameJoinSuccesful;
extern uint16_t nMcpPacketSequence;
extern uint8_t gMCPConnectionState[328];
extern uint32_t g_nMCPJoinTimestamp;
extern D2McpGameInfoStrc g_CharSelectBuffer;
extern char g_szMCPGameName[4];
extern uint32_t g_bMCPJoinGameSuccess;
extern HANDLE MCPThread;
extern uint32_t g_bMCPCharLogonSuccess;
extern uint32_t g_bMCPConnectionEstablished;
extern HANDLE pMCPEventHandle;
extern uint32_t g_bMCPCreateGameSuccess;
extern pfnMCPGameListEntry g_pfnMCPGameListCallback;
extern pfnVoidNoArgCallback g_pfnMCPGameListCompleteCallback;
extern pfnMCPMessage g_pfnMCPMessageCallback;
// Former interior labels: gInterfaceConfig6=+0x4 gInterfaceConfig5=+0x8 gInterfaceConfig4=+0xc gInterfaceConfig3=+0x10 gInterfaceConfig2=+0x14
extern uint8_t gMCPGameListBuffer[28680];
extern eD2ClientAppMode gClientAppModeQueue[5];
extern D2IniConfigStrc * D2IniData_client;
extern uint32_t gnClientAppModeQueueState;
extern int gClientAppModeQueueCounter;
extern LPCSTR gpszCmdLineCaptionStr;
extern uint32_t gbQServerRunning;
extern unsigned int gdwLastPacketActivityTick;
extern uint32_t gnClientModeInitValue;
extern unsigned int gdwQServerStartTime;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION gHostGameStateCs;
extern D2ClientGameDataStrc gClientGameData;
extern char RealmName[32];
extern short gnTcpIpJoinId;
extern eD2PlayerClassID nD2PlayerClassID;
// Long connection timeout start tick (5 min check in Client_CheckConnectionTimeouts).
extern int gConnectLongTimeoutStartTick;
extern HANDLE HostGameThread;
extern uint32_t gnTcpIpComInt;
extern uint32_t gnConnectionRefusedReason;
extern char CLIENT_GameName[20];
extern D2BNetClientCallbacksStrc * gpBNetClientCallbacks;
// Short connection timeout start tick (30 sec check in Client_CheckConnectionTimeouts).
extern int gConnectShortTimeoutStartTick;
extern uint32_t gbBloodFlag;
// End-game UI state block (congrats/cutscene flags).
extern D2EndGameUiState gEndGameUiState;
extern D2EndGameUiState gShowEndGameCutscene;
extern D2EndGameUiState gCongratzOkClicked;
extern uint8_t CLIENT_nDifficulty;
// Beta screen slideshow index (page).
extern uint8_t gBetaScreenIndex;
extern uint8_t gbActInitExpansionFlag;
extern eD2GSGameType eD2GSGameType_007a0610;
extern int D2GS_Connected;
extern int ServerLoadingComplete;
extern BOOL D2CLIENT_ExitVar3;
// Level load in-progress flag: set on unload complete, cleared on load complete.
extern BOOL gLevelLoadInProgress;
// Exit cutscene state: pending id + played flag.
extern D2ExitCutsceneState gExitCutsceneState;
extern D2ExitCutsceneState gExitCutscenePlayed;
extern D2DrlgActStrc * sgpDrlgAct;
extern D2Game_Line0x546 * D2GameGame0x546;
extern uint32_t gnWeatherOverlayActId;
extern D2GameViewStrc * D2GameView;
extern uint32_t ExitingGame;
extern uint32_t EnteringGame;
// Queued primary click flag processed by ProcessQueuedPrimaryClick each frame.
extern uint32_t gQueuedPrimaryClick;
// Queued secondary click flag processed by ProcessQueuedSecondaryClick each frame.
extern int gQueuedSecondaryClick;
extern uint32_t gbMouseButtonEventPending;
// Run modifier key state; adds run flag to ClickMap (cleared on key-up).
extern BOOL gRunKeyDown;
// Click modifier key state used to add flag 0x4 in GetMouseClickFlags.
extern int gClickModifierDown;
extern uint32_t gnQueuedPrimaryClickX;
extern uint32_t gnQueuedSecondaryClickX;
extern BOOL D2CLIENT_ExitVar2;
extern uint32_t gnCameraWorldDeltaX;
extern uint32_t gnCameraWorldDeltaY;
extern uint32_t gbMinimapClickValid;
// Load screen timer duration in ms (paired with gLoadTimerStartTick).
extern D2ClientTimerStrc gLoadTimerDurationMs;
extern TSExportTableSync<SGAMEDATA, HSGAMEDATA__*, HSGAMEDATALOCK__*, CCritSect, 0> gQServerGameState;
// NAME IS WRONG - this is the accelerator table, not a "loaded" flag. It was typed `bool` while all
// three of its accesses are dword: CLIENT_LoadLNG stores LoadAcceleratorsA(ghLngLibrary,0x64) here,
// FreeLanguage clears it, and MessageGameLoop passes it to TranslateAcceleratorA. Retyped HACCEL on
// the operand width.
extern HACCEL gbLanguageDllLoaded;
extern HMODULE ghLngLibrary;
// lpLibFileName parameter of LoadLibraryExA
extern char lpLibFileName_007a0710[260];
extern D2GameRecordStrc gsGameRecordContext;
// lpBuffer parameter of ReadFile
extern D2GameRecordFileHeaderStrc lpBuffer_007a2768;
extern uint32_t gdwQuestResurrectDismissThrottle;
extern DC6 * DC6_EnteredAreaName;
extern DC6 * pDC6_Frame;
extern uint32_t gnGoldDialogTransactionType;
extern SpinnerWrapper * gpCurrentDialog;
extern EditBoxWrapper * gpDialogDefaultActionCallback;
extern ButtonWrapper * gpUiGoldTransactionDialog;
extern ButtonWrapper * gpUiGoldDialogCancelButton;
extern uint32_t gbKeyConfigModeActiveInDialog;
extern uint32_t gbLoadScreenLevelLoadInProgress;
extern D2UIFlagStrc GFX_bShowUIFlag;
extern int gnPanelOffsetX;
extern int gnPanelOffsetY;
extern BOOL gbAutomapPanelOpened;
extern uint32_t gnLastOpenPanelId;
extern DC6 * gpUiCursorFocus16DC6;
extern DC6 * pUi_Panel_buysellbtn;
extern DC6 * gnLoadedActIndex;
extern int nLoadedActNo;
extern DC6 * gpDC6YouDiedHardcoreImage;
extern DC6 * gpDC6DeathInstructionImage;
extern D2UIFlagDescStrc gaUIFlagDescsClient[3];
extern uint32_t gnGoldDialogTransactionMode;
extern uint32_t gnLoadScreenAnimFrame;
extern uint16_t gU16ImmunityStringBuffer2[256];
extern uint32_t gnImmunityText2Color;
extern uint16_t UNICODE_007a2c80[80];
extern uint32_t gnTooltipScreenX;
extern uint32_t gnTooltipScreenY;
extern int gnTooltipHpBarValue;
extern uint16_t gU16ImmunityStringBuffer1[256];
extern uint32_t gnImmunityText1Color;
extern uint16_t gnMonsterHealthBarValue[256];
extern uint32_t gnAutoMapCellPriority[2048];
extern int nAutoMapScrollX;
extern int nAutoMapScrollY;
extern uint32_t bAutoMapRevealAll;
extern uint32_t nAutoMapRevealDelay;
extern uint8_t nAutoMapColorSelf;
extern uint8_t nAutoMapColorHostile;
extern uint8_t nAutoMapColorCorpse;
extern uint8_t nAutoMapColorParty;
extern uint8_t nAutoMapColorMerc;
extern uint8_t nAutoMapColorPartyPet;
extern uint8_t nAutoMapColorNPC;
extern uint8_t nAutoMapColorShrine;
extern uint32_t MouseYOffset;
extern uint32_t MouseXOffset;
extern uint32_t gnBorderType;
extern uint32_t MouseYRelatedB;
extern uint32_t nScreenMode;
extern int WindowSizeY;
extern int WindowSizeX;
extern size_t nD2PacketsClientBufferSize;
extern uint32_t gbConnectionLost;
// @data IMG_popupok_DiscPrompt
// @wasnamed IMG_popupok
// @evidence Loaded at 004606b4 and drawn by DrawPopupOk(1) from
// LOADER_DrawInsertDiscScreen / fpGameLoopFunction.
// @caveat both this and 007a5244 are loaded from the IDENTICAL path Data\Global\UI\Bigmenu\popupok
// with identical flags (004606a8 and 004606b9, ECX=0x6d6a14 both times). They are two independent
// handles to the same asset; only the consumer distinguishes them, and that is what these names
// record. Nothing in the binary makes them semantically different.
extern D2ArchiveStrc * IMG_popupok_DiscPrompt;
extern D2ArchiveStrc * IMG_okcancelbtn;
// @data IMG_popupok_ConnLost
// @wasnamed IMG_popupok
// @evidence Loaded at 004606c5 and drawn by DrawPopupOk(0) from
// LOADER_DrawConnectionLostScreen. See the caveat on 007a523c.
extern D2ArchiveStrc * IMG_popupok_ConnLost;
extern uint32_t gnCharScreenInputState;
extern uint32_t gnCharScreenOkButtonPressed;
extern uint32_t gnConnectionLostScreenState;
extern uint32_t gbCharScreenOkPressed;
extern uint32_t gbCharScreenOkButtonConfirmed;
extern uint32_t gbCharScreenCancelPressed;
extern BOOL BOOL_007a5260;
extern BOOL bRecursionClick;
extern D2UnitStrc * ClientSideUnitHashTables[6][128];
extern D2UnitStrc * ClientSideUnitHashTables_3__0_[6][128];
extern D2UnitStrc * ServerSideUnitHashTables[6][128];
extern D2UnitStrc * pPlayerUnit;
extern int gnSelectExclusionRectLeft;
extern int gnSelectExclusionRectTop;
extern int gnSelectExclusionRectRight;
extern int gnSelectExclusionRectBottom;
extern uint32_t gbSuppressItemSelection;
extern uint32_t gbSelectExclusionActive;
extern int MouseY;
extern int MouseX;
extern uint32_t gnCursorType;
extern D2UnitStrc * pCursorItem;
// Current animation frame index for the cursor sprite. Incremented by animation speed from CursorDataTable, wraps using UI_GetCursorMaxFrames(). Used by MOUSE_DrawCursorSprite to select frame from pCursorsDC6[6].
extern int32_t gCursorAnimationFrame;
extern uint32_t gnCursorAnimStartTick;
extern int ItemTypeOnHand;
extern unsigned int gnD2GSBytesSentTotal;
extern uint32_t gnD2GSPacketsSentTotal;
extern int gnCursorInitialized;
extern pfnVoidNoArgCallback gpfnMouseLeftButtonHandler;
extern pfnVoidNoArgCallback gpfnMouseRightButtonHandler;
extern D2KeyBindStrc gaKeyBindingsBackup[114];
extern pfnVoidNoArgCallback gpfnMouseMiddleButtonHandler;
extern D2KeyBindStrc g_KeyBindingsTable[114];
extern pfnVoidNoArgCallback gpfnScrollUpButtonHandler;
extern pfnVoidNoArgCallback gpfnScrollDownButtonHandler;
extern pfnVoidNoArgCallback gpfnMouseLeftButtonAltHandler;
extern pfnVoidNoArgCallback gpfnMouseRightButtonAltHandler;
extern pfnVoidNoArgCallback gpfnMouseMiddleButtonAltHandler;
extern unsigned int gbKeyBindingsEnabled;
extern uint16_t gwKeyNameBuffer[4];
extern unsigned int gbKeyConfigModeActive;
// D2ClientLightPulseStrc (44 bytes, 0x007a742c..0x007a7457). CMD_InitKeyConfigUI (0046ae50) opens with memset(&gRoomLightPulse, 0, 0x2c) then writes bRoomLightPulseActive / nFadeInDuration / nFadeOutDuration / nHoldDuration / nTotalDuration / eLevelId - so the 44-byte clear and the field writes are one object.
extern D2ClientLightPulseStrc gRoomLightPulse;
extern BOOL DenOfEvilLight;
extern uint32_t gnDiabloSpawnLightActive;
extern int32_t gnGameEndTime;
extern uint32_t gbWorldstoneShouldShake;
extern D2UnitStrc * gaActiveUnitSlots[8];
extern D2ClientSpriteLoadStatusStrc gaSpriteLoadStatusTable[7];
extern D2CofCacheStrc gMonsterCofCache;
extern D2MonsterCofStatusEntryStrc * g_pMonsterSpriteCofStatusTable;
extern D2CofCacheStrc gPlayerCofCache;
extern int gnGfxThrottleUpdateFrame;
extern uint32_t gbCofCacheLRUEnabled;
extern int gnGfxSystemFlags;
extern int gnGfxThrottleUpdateFrameCountDynamic;
extern int gnGfxDirectionCountCache;
extern int DC6_UncompOverlays_bubble_FrameCount[4];
extern DC6 * DC6_UncompOverlays_bubble[4];
extern char gWeatherPaletteRain1Colors[12];
extern int gnWeatherRainTimingMin[4];
extern int gnWeatherTransitionCountdown;
extern int gnWeatherTransitionState;
extern int gnWeatherRainParticleTimingMs[2];
extern char gWeatherPaletteDayColors[12];
extern char gWeatherPaletteLightingNightColors[12];
extern float gnWeatherTransitionInitialized;
extern char gWeatherPaletteMudDuskColors[12];
extern DC6 * DC6_UncompOverlays_Rain[4];
extern int DC6_UncompOverlays_Rain_FrameCount[4];
extern int gnWeatherParticleState;
extern int gnWeatherRngCountdown;
extern uint32_t gbWeatherTransitionTriggered;
extern char gWeatherPaletteLightingSoftwareColors[12];
extern int gnWeatherRngParam;
extern int gnWeatherTransitionProgress;
extern int gnWeatherFrameIndex;
extern int nWeatherParticleCountAct5;
extern int gnWeatherAct5ParticleCount;
extern int gnWeatherStateAct5;
extern int gnWeatherTransitionLock;
extern int gnWeatherCurrentTransitionState;
extern uint32_t gnLightmapInterpDirY;
extern uint32_t gnLightmapInterpSubdivRatio;
extern uint32_t gLightMapBuffer[4096];
extern uint32_t gaLightmapInterpBuffer[4096];
extern unsigned int gnLightmapInterpDirX;
extern int gnLightmapWorldRefreshFlag;
extern int gnLightmapBufferState;
extern int gnLightmapWorldMaxX;
extern int gnLightmapWorldMaxY;
extern uint32_t gnLightmapInterpolationState[257];
extern D2LightMapCellStrc gaLightMapCells[2304];
extern D2LightMapCellStrc gLightmapCellData[2304];
extern D2LightMapCellStrc gLightmapCellDataRed[2304];
extern D2LightMapCellStrc gLightmapCellDataGreen[2304];
extern D2LightMapCellStrc gLightmapCellDataBlue[2304];
extern D2SeedStrc gAncientsCloudSeed;
extern uint8_t gaArcaneStarColors[256];
extern uint32_t gaSpriteArcaneStarYOffset;
extern uint32_t gaSpriteArcaneStarXOffset;
extern int nScaleFactor;
extern int32_t gaArcaneStarYPos[256];
// byte[0x3100] = 49 tables x 256. D2WINPAL_CopyPalette (004fb1c0) does memcpy(ECX=0x7b5c10, 0x81ea68, 0x3100); called with ECX=0x7b5c10 from VIEW_InitLookupTables at 00475e58. Readers index with MOVZX byte / SHL 8 (00475cc0, 00475ce0, 00475d92). Ends exactly at the next symbol (007b8d10).
extern uint8_t gPaletteTransitionTable[12544];
extern uint8_t gaArcaneStarColorTable[8];
extern int32_t gaArcaneStarXPos[256];
extern int32_t gaArcaneStarXVelocity[256];
extern uint32_t gnLastLightmapUpdateFrame;
extern DC6 * gpAncientsBgDC6;
extern DC6 * gnDrawBackgroundRenderOffset;
extern void * gpMonsterPalShiftList;
extern void * gpPalShiftEntryListHead;
// byte[0x1e00] = 30 transforms x 256. D2FILE_ReadFile(handle, 0x7b9580, 0x1e00) at InitPal 00476f1d. PALSHIFT_GetMonsterTransform bounds the index at <= 0x1d (CMP ESI,0x1d / JA at 0047759a) then SHL 8 + base. Ends exactly at gbPalShiftRandTransformLoaded (007bb380). Was typed D2GamePalShiftRandTransformStrc* (4 bytes); that struct is a 7679-byte all-undefined auto blob, one byte short of the real 0x1e00 - do not use it.
extern uint8_t PalShift_RandTransform[7680];
extern int gbPalShiftRandTransformLoaded;
extern D2GamePalShiftGreenBloodStrc * PalShift_GreenBlood;
extern uint32_t gnEngineMode;
extern uint32_t fps;
extern uint32_t gbFPSStatsEnabled;
extern uint32_t gnCurrentEngineFlags;
extern uint32_t gbHideExtendedFPSStats;
extern uint32_t gnLoadingProgressRatio;
extern uint32_t gnChatHistoryScrollIndex;
extern int gaIMECompositionBuffer;
extern char gaChatInputBuffer;
extern uint8_t gbChatInputEnabled;
extern uint16_t gnChatHistoryFillCount;
extern uint32_t gnChatInputTextCursorPosition;
extern int gnChatInputTextLength;
extern uint32_t gnIMEInputMode;
// @data gpIMECandidateList
// @wasnamed gpIMECompositionString
// @evidence The allocated buffer passed at 0047adb4 as lpCandList to ImmGetCandidateListA.
// A candidate list, not a composition string. @wasnamed gpIMECompositionString.
extern LPCANDIDATELIST gpIMECandidateList;
extern uint16_t gwcImeCompositionBuffer[32];
// wchar16[257] - the chat input line. Was typed LPCWSTR (4 bytes). Size proven three ways: CHAT_ClearInput (0047bdc0) does memset(this, 0, 0x202) = 514 bytes = 257 wide chars; CHAT_* callers pass it to WSTRING_PutWideCharIntoBuffer with a 0x100 char limit and to WideCharToMultiByte with cbMultiByte 0x100 (256 chars + terminator); INPUT_HandleChatCharInput indexes it as *(uint16_t*)(base + i*2). Next symbol gaChatHistoryBuffer is 520 bytes away, so 514 fits. Declared as 4 bytes, every keystroke in the chat box wrote past the symbol.
extern uint16_t gwszChatInputText[257];
// byte[0x1010] = 8 entries x 0x202. Stride 0x202 (IMUL EAX,EAX,0x202 at 0047b9c5; IMUL EDI,EDI,0x202 at 0047ba5f). Index bounded 0..7 in CHAT_AddToHistory (wraps to 7 at 0047ba1c and 0047ba4f). Ends exactly at gwcIMECompositionString (007bc850). gaChatHistoryBuffer_Entry7 at 007bc64e is base + 7*0x202 = entry 7, not an independent object - its only xrefs are the same MOVSD/MOVSW at 0047ba72/74 that write through the computed pointer.
extern uint8_t gaChatHistoryBuffer[4112];
// WCHAR[32] IME composition string (64 bytes, 007bc850..007bc88f), drawn by CHAT_DrawIMEComposition. Filled at 0047ac49 from the GCS_COMPSTR (8) string converted by MultiByteToWideChar, capped at 31 characters; element [31] (007bc88e) is the terminator written at 0047ac5f.
// Zeroed as 16 dwords by StaticInit_6cab30; cleared ([0]=0) at 0047ab98 after the GCS_RESULTSTR (0x800) commit, in INPUT_HandleIMEStartComposition (0047acb8), INPUT_HandleIMEEndComposition (0047ad1c), CHAT_InitIME (0047b112) and CHAT_ShutdownIME (0047b379).
// Provenance: write xrefs in 1.14d; the interior label gpIMECompositionStringEnd (007bc88e) was folded into this array.
extern uint16_t gwcIMECompositionString[32];
extern int gnMiniPanelButtonX[8];
extern uint32_t gnMiniPanelButtonY[8];
extern uint32_t gnMiniPanelButtonFrame[8];
extern uint32_t gnMiniPanelButtonState[8];
extern uint32_t gnMiniPanelButtonState6[8];
extern int gnEscMenuSelectedIndex;
// Global pointer to current escape menu header. Points to static menu table headers that contain item count, spacing, row height, and star decoration offset. Reassigned during menu transitions (main menu, options, save/exit, etc).
extern D2EscMenuHeaderStrc * gpEscMenuItemCount;
extern D2EscMenuItemStrc * gpEscMenuCurrentTable;
extern uint32_t gbEscMenuSliderDragging;
extern uint32_t gnEscMenuSliderValue;
extern DC6 * gpEscMenuOptBarDC6;
extern DC6 * gpEscMenuOptBarCDC6;
extern DC6 * gpEscMenuOptSkullDC6;
extern int SizeCurrentSelectedEscMenuItem;
extern DC6 * gpMiniPanelBackgroundDC6;
extern DC6 * gpMiniPanelButtonsDC6;
extern uint32_t gnMiniPanelRightScreen;
extern uint32_t gnMiniPanelLeftScreen;
extern uint32_t gnMiniPanelMouseOverButton;
extern uint32_t gbMiniPanelSinglePlayer;
extern uint32_t gnMiniPanelHoverState;
extern void * gpWardenResponseData;
extern uint32_t gnWardenResponseSize;
extern D2WardenModuleStrc * gpWardenActiveModule;
extern D2WardenModuleStrc * gpWardenPendingModule;
extern D2BNetClientCallbacksStrc * sgpUnknownSequence;
extern void * pSoundChannelDataArray;
extern D2SoundsTxt * pTxtSounds;
extern int nTxtSoundsSize;
extern D2SoundEnvironTxt * pTxtSoundEnviron;
extern int nTxtSoundEnvironSize;
extern uint32_t gpSoundHdrGlobal;
extern uint32_t gnSoundIdRangeMax;
extern void * gpSoundLRUListHead;
extern uint32_t gnCurrentGameAudioVolume;
extern unsigned int gdwLastSoundAsyncLoadTick;
extern int gnSoundEvictedAudioVolume;
extern uint32_t gnSoundAsyncLoadCount;
extern uint32_t gnSoundBufferMaxBytes;
extern uint32_t gnSoundBufferBytesLoaded;
extern char gbCutsceneActive;
extern uint8_t gbLoggerStateInactive;
extern uint8_t gbItemDescDirtyFlag;
extern uint8_t geCutsceneCurrentSoundId;
extern D2NpcShopButtonStrc gaNpcShopButtons[4];
extern unsigned int gdwInvLastActionTickCount;
extern int gnInvMouseX;
extern BOOL gbInvOverlayFlag;
extern DC6 * gpInvShopTabImage;
extern uint32_t gpInvCursorItem;
extern int gbInvNpcScreenCoordsInitialized;
extern uint32_t gpInvItemUnderCursor;
extern uint32_t gbInvBodySlotUnderCursorExists;
extern BOOL DrawShopRelated;
extern int g_nCurrentInventoryPage;
extern D2UnitStrc * pItemToDescribe;
extern D2UnitStrc * gpInvHoveredItem;
extern uint8_t CurrentInventoryPage;
extern uint8_t TypeOfShopMaybe;
extern uint8_t gbInvImageLoadToggle;
extern uint32_t gnInvShopPage;
extern uint32_t gbInvNeedsRefresh;
extern uint32_t gpInvCubeImage1;
extern uint32_t gpInvCubeImage2;
extern uint32_t gpInvCubeImage3;
extern DC6 * gpInvCubeImage4;
extern uint32_t gnInvBodySlotUnderCursor;
extern DC6 * UI_GFX;
extern DC6 * gpInvShopTab1Image;
extern DC6 * gpInvStashImage1;
extern DC6 * gpInvCubeBgImage;
extern DC6 * gpInvGoldCoinImage;
extern DC6 * gpInvShopTab2Image;
extern DC6 * gpInvStashImage2;
extern int gnInvShopButton;
extern uint32_t gnInvShopButton2;
extern uint32_t gbInvGambleMode;
extern uint32_t gbInvExpansionTabVisible;
extern uint32_t gnInvCubeCloseClickState;
extern uint32_t gnInvCubeTransmuteClickState;
extern D2InventoryEntryStrc gaPlayerBodySlotScreenCoords[11];
extern uint32_t gbInvTradeWindowOpen;
extern D2InventoryEntryStrc gaHirelingBodySlotScreenCoords[11];
extern uint32_t gbInvTradeWindowOpen2;
extern uint32_t gbTradeConfirmationPending;
extern uint32_t gbInvTradeAcceptClicked;
extern uint32_t gbInventoryDragInProgress;
extern uint32_t gbInvTradeCancelClicked;
extern uint32_t gpInvItemDescPending;
extern uint32_t gbInvGoldDialogOpen;
extern uint32_t gnInvInventoryGold;
extern uint32_t gnInvStashGold;
extern uint32_t gbInvStashPageOpen;
extern uint32_t gbInvStashDepositClicked;
extern uint32_t gbInvCubePageOpen;
extern uint32_t gbInvCubeTransmuteClicked;
extern uint32_t gbInvCubePageOpen2;
extern uint32_t gbInvCubeWithdrawClicked;
extern DC6 * gpInvMercHelmetSlotImage;
extern DC6 * gpInvMercArmorSlotImage;
extern DC6 * gpInvMercWeaponSlotImage1;
extern DC6 * gpInvMercWeaponSlotImage2;
extern DC6 * gpInvBodySlotImage1;
extern DC6 * gpInvBodySlotImage2;
extern DC6 * gpInvBodySlotImage3;
extern DC6 * gpInvBodySlotImage4;
extern DC6 * gpInvMercBodyImage;
extern DC6 * gpInvCubeRecipeScrollImage;
// @data gbInvPanelButtonPressed
// @wasnamed gnInvMouseX
// @evidence A boolean, not a coordinate: 0048ff4c does SETNZ then ADD EAX,0xa to
// select DC6 frame 10 or 11, and the slot is only ever written 0 or 1.
// @caveat WHICH button is not settled - it needs the pDC6 assigned into the D2GfxDataStrc before
// 0048ff31 in DRAW_UI_Inventory. @wasnamed gnInvMouseX; the real gnInvMouseX is 007bca4c.
extern uint32_t gbInvPanelButtonPressed;
extern int gnInvMouseY;
extern uint32_t gShopTradeUIState;
extern uint32_t gnInvMercBodySlot2;
extern uint32_t gbInvRecipeScrollOpen;
extern uint8_t gpInvRecipeScrollImage[8192];
extern uint32_t gnHireDisplayMode;
extern uint32_t gpHireHoverUnit;
extern uint32_t gnHireIconDrawX;
extern uint32_t gnHireIconHoverX;
extern uint32_t gnHireIconHoverY;
extern uint8_t gbHireIconsVisible;
extern uint32_t gnHireIconHoverIndex;
extern RGBQUAD nColor;
extern uint32_t gpHelpCloseButton;
extern uint32_t gpHelpScrollButtonDown;
extern uint32_t gpHelpScrollButtonUp;
extern int gaPanelImageLoadFlags[12];
extern uint8_t gaPanelBeltColors[4];
extern BOOL gbPanelLeftBeltArrowClicked;
extern uint8_t gbPanelRightBeltArrowClicked[12];
extern uint32_t gpPanelBeltSlot;
extern uint32_t gnPanelBeltRowsExpanded;
extern uint32_t gbPanelBeltAnimating;
extern uint32_t gbPanelBeltSlotHovered;
extern uint32_t gpPanelClickedButton;
extern D2UnitStrc * gpPanelBeltHoverItem;
extern D2UnitStrc * gpPanelBeltLastHoverItem;
// Four per-stat colour bytes indexed with an explicit bound. Typed as one byte, every index past 0 read a neighbouring global.
extern uint8_t gaPanelStatColors[4];
// Six belt-slot bytes that PANEL_DrawBeltSlots both reads AND writes. Typed as one byte, the writes corrupted up to five adjacent globals every time the mouse moved over the belt.
extern uint8_t gaPanelBeltArrowsNeeded[6];
extern uint8_t gbPanelStaminaColorBlue;
extern uint8_t gbPanelGoldColorGreen;
extern uint32_t gpPanelMenuButton;
extern uint32_t gpPanelRunButton;
extern uint8_t gbPanelStaminaColorRed;
extern int gpPartyAcceptButton;
extern int32_t gpWaypointButtonHover;
extern D2WaypointListEntryStrc gaWaypointListEntries[9];
extern uint8_t gbWaypointButtonPressed;
extern uint32_t ClickingDownOnWaypointEntry;
extern DC6 * WaypointWayGateBackGround;
extern DC6 * WaypointWayGateTabs;
extern DC6 * WaypointWaygateIcons;
extern uint32_t WaypointCurrentSelectedAct;
extern int MaximumAmountOfWaypointInSelectedAct;
extern uint32_t NoOtherWaypointAvailableBool;
extern uint16_t gnQuestScrollLine[6];
extern D2TextBoxStrc * gpTextScrollingBox;
extern RECT gnTextBoxExclusionLeft[16];
extern D2TextBoxStrc * gpStaticTextPopup;
extern uint32_t gdwTextPopupExpireTime;
extern ListBoxWrapper * gpTextMessageLogHead;
extern uint32_t gbTextScrollBoxAnimating;
extern uint32_t gbTextScrollBoxActive;
extern uint32_t gbTextStaticPopupActive;
extern uint32_t gpTextStaticPopupBox;
extern int gaTextStaticPopupRect;
extern D2ScreenMessageListStrc ScreenMessagesMaybe;
extern D2ScreenMessageListStrc ScreenPartyMessages;
extern D2ScreenMessageListStrc AllMessagesMaybe;
extern uint32_t gnTextDisplayMode;
extern uint32_t gbTextScrollBoxVisible;
extern uint32_t gnTextNpcGuid;
extern uint32_t gpOverHeadUnitGUID;
extern uint32_t gnTextNpcInteractGuid;
extern uint32_t gnTextOverheadCounter;
extern uint16_t gnTextScrollBoxWidth;
// lprc parameter of SetRect
extern RECT lprcTextOverheadRect;
extern uint32_t gbTextOverheadActive;
extern void * gpQuestScrollTextBlock;
extern unsigned int gbTextNpcMenuActive;
extern uint32_t gpTextScrollBoxImage;
extern uint32_t gnTextScrollBoxHeight;
extern uint8_t gbTextNpcMenuSelection;
extern uint32_t gnTextNpcMenuIndex;
extern uint32_t gpTextDismissCallback;
extern uint32_t gpTextScrollBoxBuffer;
extern D2GSPacketSrv0x27_TextStrc * gpOverHeadStrc;
extern uint32_t gpQuestScrollUI;
extern pfnTextScrollComplete gpTextScrollCompleteCallback;
extern void * gpTextScrollCompleteParam;
extern uint32_t gpTextScrollBoxDC6;
extern DC6 * gpQuestScrollImageTop;
extern DC6 * gpQuestScrollImageBottom;
extern DC6 * gpQuestScrollImageBg;
extern uint32_t gnTextBoxExclusionX;
extern uint32_t gbTextEventHandlersRegistered;
extern int gQuestLogSelectedQuestByAct[5];
extern D2QuestLogStrc gQuestLog;
extern int gnUICtrlConfigTotalRows;
extern int gnUICtrlConfigScrollMaxLines;
extern uint32_t gnUICtrlConfigColumnWidth;
extern uint32_t gnUICtrlConfigMouseButtonState;
extern uint32_t gnUICtrlConfigButton3State;
// int[7] - one mouse-capture flag per control-config UI region. UI_UpdateConfigControlsState (004a4ad0) and UI_ConfigHandleMouseClick contain seven inlined copies of the same scan: for (p = &gaControlConfigRegionMouseState[0]; p < &gaControlConfigRegionMouseState[7]; p++) if (*p && p != &self) goto skip; i.e. "claim the mouse only if no OTHER region already holds it". Both the do-while bound (base+28, 0x007c02a4) and the last loop's exit test (0x007c02a3 < p) prove the extent: 7 dwords, 0x007c0288..0x007c02a3.
// Indices: [0] list area  [1] up arrow  [2] down arrow  [3] scroll bar  [4] scroll thumb  [5] below thumb  [6] tab/button row.
extern int gaControlConfigRegionMouseState[7];
extern uint32_t gnUICtrlConfigScrollOffset;
extern int g_ControlConfigUIState;
extern int gnUICtrlConfigActiveButton;
extern BOOL gbUICtrlConfigKeyBindMode;
extern DC6 * gpUICtrlConfigScrollbarDC6;
extern int gnUICtrlConfigPendingKeyStrId;
extern unsigned int gdwUICtrlConfigKeyBindTimestamp;
extern int gnUICtrlConfigScrollPosition;
extern DC6 * gpUIStatLevelUpPanelImages;
extern DC6 * gpUIStatLevelUpButtonImages;
extern BOOL gbUIStatButtonHovered;
extern BOOL gbUISkillButtonHovered;
extern uint64_t gbUIGoldStashMode;
extern BOOL gbUIStatScreenMouseDown;
extern uint64_t gbUIStatWindowDrawFlag;
// One 965-byte region: four int[60] grids plus a 5-byte index. SPELLSEL_Initialize and SPELLSEL_RenderAllSkillGrids memset all 0x3c5 bytes of it. Modelled as four separate objects the memset targeted only the first 240-byte array, so 725 bytes of whatever the linker placed after it were zeroed on entering a game and again on every skill-grid render, while the three sibling arrays were never cleared.
extern D2SpellSelGridRegionStrc gSpellSelGridRegion;
// Spell selection UI state array — skill IDs for each slot. Part of large 1396-byte spell selection state (0x007c06c8-0x007c0c3c). Initialized to 0xFFFFFFFF by SpellSel initialization. Indexed by slot number, values sent to server via packet 0x3C.
extern int gSpellSelectionSkillIDs[16];
extern int gbSpellSelActiveRightSkillSlot;
extern int32_t gSpellSelLeftSkillBindings[16];
extern int gSpellSelRightSkillBindings[16];
extern DC6 * gpSpellSelSkillIconDC6;
extern unsigned int gbSpellSelMouseHandFlag;
extern int gbSpellSelSkillTreeNodeHovered[128];
// 128 per-skill-node state ints, indexed 19 times as (&base)[n*4]. Typed bool (one byte), every node past 0 read and wrote outside the object.
extern int gaSpellSelSkillTreeState[128];
extern BOOL gbSpellSelSkillTreeBackground;
extern uint32_t BooleanWaitingForSkillingMaybe;
extern uint32_t gnSpellSelSkillTreeOffsetY;
extern uint32_t gnSpellSelSkillTreeClickY;
extern uint8_t gnSpellSelTabHighlightColor;
// GetTickCount of the last time the 1-second throttle at 004ab7b0 (SpellSel, no function yet) passed.
// Provenance: asm 004ab7b0-004ab7d3: now - [0x7c0c54] > 1000 -> store now, return 1; else return 0.
extern uint32_t gdwSpellSelThrottleTick;
extern uint16_t gnMonsterUniqueNameBuffer[2];
extern uint32_t gnNpcMenuMercHireCost1;
extern uint32_t gnNpcMenuMercHireCost2;
// NPC-menu state region: 0x007c0c68 .. 0x007c0dc7 (0x160 bytes).
// NPCMENU_Initialize @004b23e0 zeroes this whole run with one memset(0x007c0c68, 0, 0x160);
// the run holds 45 individually named globals and ends exactly on the next symbol
// (gpNpcMenuShopInventoryDC6 @007c0dc8, which the same function assigns right after).
// 339 of the 352 bytes are defined data; the 13 remaining are unnamed filler
// (007c0c6f-72, 007c0d35-38, 007c0d3a-3c, 007c0dac-ad).
// The memset is legitimate here: this is one contiguous .data region in the original.
// It must NOT be collapsed into a struct - the members carry their own xrefs
// (nUnknownUiRelatedMaybe 76, nCurrentInteractedNpcGUID 103, bInteractingWithNpc 53, ...).
// The defect is in the emitted C++ layout, not in this program: the generator emits
// memset(&gbNpcMenuInitialized, 0, 0x160) through a 1-byte global whose neighbours the
// linker may place anywhere. The emitter must either lay this address run out contiguously
// or expand the memset into per-global stores.
extern uint8_t gbNpcMenuInitialized;
extern uint8_t gnNpcMenuActiveMode;
extern uint8_t gbNpcMenuRandomDialogSeed;
extern int PLAYER_CurrentGold;
extern uint16_t gnNpcMenuRepairCostGold;
extern int16_t gnNpcMenuTradeActionGold;
extern uint16_t gnNpcMenuScrollCompleteMode;
// Global pointer to current NPC speech/trade menu state. Points to D2NPCSpeechStrc entries in the global D2NPCSpeech array. Contains dialog bank, count, gossip index, and flags for NPC interactions. Reassigned when entering/exiting trade/gamble menus.
extern D2NPCSpeechStrc * gpNpcCurrentSpeech;
extern uint32_t gnNpcMenuCainIdentifyGUID;
extern uint32_t gnNpcMenuTransactionType;
extern uint32_t gnNpcMenuPreviousGold;
extern D2BitBufferStrc * gpNpcMenuBitBuffer;
extern D2BitBufferStrc * gpNpcMenuDialogOptions;
extern uint32_t gnNpcMenuCurrentNpcClassId;
extern uint32_t gnNpcMenuCurrentNpcGUID;
extern uint32_t gnNpcMenuLastTransactionGold;
extern uint32_t nTransactionCost;
extern ListBoxWrapper * gpNpcMenuMercListBox;
extern D2GSPacketSrv0x2A D2GSPacketSrv0x2A_007c0d87;
extern uint32_t gnNpcMenuActiveContainer;
// Nine already-handled NPC dialog message ids. A stray label on element 1 truncated it to one uint16, so NPCMENU_AddHandledMessageId wrote 16 bytes past it and NPCMENU_FindNextUnhandledMessage picked the greeting line from garbage.
extern short gaNpcMenuHandledMessageIds[9];
extern uint16_t gnNpcMenuScrollMessageHead;
extern uint32_t gnNpcMenuShopMode;
extern uint32_t gnNpcMenuNpcSoundIndex;
extern uint32_t gnNpcMenuNpcClassId;
extern uint32_t gnNpcMenuDialogX;
extern uint32_t gnNpcMenuDialogY;
extern DC6 * gpNpcMenuShopInventoryDC6;
extern DC6 * gpNpcMenuShopInventoryBuffer;
extern D2UnitStrc * gpNpcShopOwnerUnit;
extern short gbNpcMenuShopOpened;
extern uint8_t gnNpcMenuMercListCount;
extern uint8_t gbNpcMenuTransactionPending;
extern BOOL gbNpcMenuTransactionConfirmed;
extern uint32_t gnNpcMenuTransactionItemGUID;
extern unsigned int gbNpcMenuShopTransactionActive;
extern uint32_t gnNpcMenuShopTransactionCost;
extern unsigned int gdwNpcMenuLastClickTimestamp;
extern uint16_t gNpcMenuConfirmDialogStrings[40];
extern int32_t gnDialogStructAllocCount;
extern uint32_t gnTradeGoldUpdateBuffer;
extern uint32_t gnTradePlayerGoldAmount;
extern uint32_t gdwTradeThrottleTimestamp;
extern bool gbTradeResetFlag;
extern uint32_t gnTradeAcceptedFlags;
extern int32_t g_nTradeWindowState;
extern uint8_t gbTradeWindowOpen;
extern uint16_t gwszTradePartnerName[16];
extern uint8_t aQuestAvailabilityPacket[37];
extern BOOL bReceivedQuestAvailabilityPacket;
extern D2SoundChannelNodeStrc * gpActiveSoundChannelsHead;
extern uint32_t gnSoundChannelCount;
extern uint32_t gpSoundChannelsArray;
extern uint32_t gbSoundChaosEnabled;
extern DC6 * gpMessageLogDC6;
extern uint32_t nHoradricCubeUIState;
extern int gnPlayerCashDisplay;
extern int gnNpcShopSecondActiveTab;
extern uint32_t gnNpcShopActiveTab;
extern uint32_t gnCursorItemUnitId;
extern uint32_t gnTextSystemInitFlag;
extern DC6 * gpUpgradeMenuDC6;
extern DC6 * gpHoradricBackgroundDC6;
extern int gItemDrawFlags;
extern D2GroundItemLabelStrc gaGroundItemLabels[32];
extern short gnWeaponHandClassId_Right;
extern short gnWeaponHandClassId_Left;
extern int gaLevelAmbientMusicPlayedFlags[1024];
extern int gnSoundLastInitTick;
extern uint32_t gdwSoundChannelIndex;
extern uint32_t gnSoundInitCooldown;
extern unsigned int gnSoundLastPlayerAnimModeTime;
extern int geSoundLastPlayerAnimMode;
extern int gPlrModeTokens[20];
extern int gTxtCompositTokens[16];
extern int gObjModeTokens[8];
extern int gMonModeTokens[16];
extern int gArmTypeTokens[3];
extern uint32_t gdwWarpFadeExpireTime;
extern unsigned int gdwAmbientMusicLevelChangeTick;
extern unsigned int gdwAmbientMusicRestartTick;
extern int geCutsceneAmbientMusicSoundId;
extern uint32_t geAmbientMusicLevelId;
extern uint32_t gdwAmbientMusicChangeTime;
extern uint32_t gbSoundSequenceActive;
extern uint32_t geSoundSequenceSoundId;
extern uint32_t gnSoundSequenceResumeIndex;
extern uint32_t gbPendingSoundSequencePlay;
extern uint32_t geDelayedSoundId;
extern uint32_t gnDelayedSoundPlayIndex;
extern uint32_t gbPendingDelayedSoundPlay;
extern uint32_t gnSoundSequenceEndIndex;
extern uint32_t geLastAmbientMusicLevelId;
extern int UNIT_CenterXCoordsDiv5;
extern int gnPlayerTileGridIndex;
extern int UNIT_CenterYCoordsDiv5;
extern uint32_t BackgroundSoundsActive;
// lpAddend parameter of InterlockedDecrement
// lpAddend parameter of InterlockedIncrement
extern LONG * lpAddend_007c8a74;
extern D2ClientSoundChannelStrc gaSoundChannels[16];
extern BOOL pAudioMixerInterface;
extern int32_t int32_t_007c8c7c;
extern uint32_t gnActiveSoundChannelCount;
extern void * pAudioDeviceContext;
extern uint32_t gnSoundChannelIndex;
extern uint32_t gnSoundChannelPlaybackState;
extern int32_t int32_t_007c8c90;
extern unsigned int UINT_007c8c94;
extern int32_t int32_t_007c8c98;
extern HINSTANCE hInstance;
extern eD2RendererMode eRendererMode;
extern BOOL bD2GFX_Windowed;
extern uint32_t nResolutionMode;
extern HWND hwndD2GFX_Window;
extern D2RendererFunctionsStrc * RENDERER_CurrentRenderedFunctions;
extern uint32_t gnGfxOverlayMode;
extern uint8_t gaDirectDrawDitherPaletteRemap[1024];
extern D2RenderStatisticsStrc gnTextureMemoryUsageBytes;
extern D2RendererCoreStrc * RENDERER_SelectedRendererCore;
extern int RENDERER_nWindowWidth;
extern int RENDERER_nWindowHeight;
extern int gnGlideWindowHandle;
extern int gnRenderState;
extern D2GfxSettingsStrc * pD2GFXRenderedDataStrc;
extern D2GfxHelperStrc * D2GfxRendereInitFunctionsStrc;
extern BYTE * * gpPaletteTable;
// ppvBits parameter of CreateDIBSection
extern BYTE * gpRenderBackBufferBits;
extern LPCSTR gpszWindowClassName;
// struct tagRECT rInitialWindowRect
// lpRect parameter of GetWindowRect
extern tagRECT rInitialWindowRect;
extern int32_t nWindowSizeY;
extern D2GFXWindowPlacementStrc gaD2GFXWindowPlacements[4];
extern uint32_t ghWndMain;
extern uint32_t D2GFX_EndCutSceneCurrentState;
extern uint32_t D2GFX_EndCutScene;
extern LPCSTR szWindowClassName;
extern LPCSTR gpszWindowTitle;
extern int nD2GFX_PerspectiveScaleY;
extern int32_t nD2GFX_PerspectiveScaleX;
extern int gnTileInnerClipZMin;
extern int gnTileClipXMax;
extern int gnTileClipXMin;
extern int gnTileClipYMin;
extern int gnTileInnerClipYMax;
extern int gnTileInnerClipXMin;
extern int gnTileInnerXMax;
extern int gnTileClipZMax;
extern int gnMinTileScreenY;
extern int gnTileViewportXOffset;
extern int32_t gaYBufferRowOffsets[732];
extern int gnTileClipLeft;
extern int gnTileInnerClipYMinRow;
extern uint8_t gpLineBufferTileArray[32][32][32];
extern int gnTileInnerClipXMax;
extern int gnTileInnerClipYMin;
extern int gnTileInnerClipXMax_b;
extern int gnMinTileDrawY;
// Screen-space tile clip bounds. SIGNED: the extended view starts off-screen, so the minimums are genuinely negative (typically -16 for Z and -32 for X against a 800x553 view). Left as undefined4 the reconstruction emitted them uint32_t, which promotes `gnTileClipZMin &lt;= nSubTileZ` to an unsigned compare - 0xFFFFFFF0 &lt;= 120 is false, so fpDrawGroundTile clipped away every sub-tile of every floor tile and the world rendered black while DrawGroundTile still reported success.
extern int gnTileClipZMin;
extern int gnMinTileScreenY_b;
extern int gnTileClipBottom;
extern int gnTileClipRight;
extern uint32_t gbPerspectiveRenderEnabled;
extern uint8_t gaPaletteColorLUT[49][256];
extern uint32_t pRenderSurfaceBuffer;
// Render buffer pitch (bytes per scanline) for tile drawing. Set by fpFillYBufferTable from nWidth parameter. Used by fpDrawVisTile, fpDrawBlendedVisTile, fpDrawRoofTile, fpDrawGroundTile to calculate Y-offsets when blitting tile data to framebuffer.
extern int32_t gRenderBufferPitch;
extern BYTE * * gpPaletteBlendLookupTable;
extern eD2RendererMode D2GFX_eRendererMode;
extern DC6 * gpCursorDC6;
extern uint32_t gnCursorDC6FrameNumber;
extern HWND ghwndD2GFX_CachedWindow;
extern uint32_t gnWindowInitState;
extern BOOL gbWindowed;
extern int gnResolutionMode;
extern int gnFormControlCount;
extern uint32_t gbGfxWindowCreated;
extern uint32_t gnWinMainState;
extern uint32_t gbFormActivated;
extern uint32_t gnInitialFormState;
extern pfnVoidNoArgCallback gpfnExitCallback;
extern uint32_t gbMouseInWindow;
extern uint32_t ghwndMsgHook;
extern unsigned int gdwLastScreenshotTick;
extern D2FogArchiveStrc * MPQ_d2Xmusic;
extern D2FogArchiveStrc * MPQ_d2sfx;
extern D2FogArchiveStrc * MPQ_d2delta;
extern D2FogArchiveStrc * MPQ_d2char;
extern D2FogArchiveStrc * MPQ_d2video;
extern D2FogArchiveStrc * MPQ_d2Xtalk;
extern D2FogArchiveStrc * MPQ_d2data;
extern D2FogArchiveStrc * MPQ_d2kfixup;
extern D2FogArchiveStrc * MPQ_d2speech;
extern D2FogArchiveStrc * MPQ_d2music;
extern D2FogArchiveStrc * MPQ_d2Xvideo;
extern D2FogArchiveStrc * MPQ_patch_d2;
extern D2FogArchiveStrc * MPQ_d2exp;
extern int gnPaletteGammaShiftState;
extern uint8_t gnPaletteIndexCounter[39];
extern uint8_t gPaletteTransposeBase[3328];
extern uint8_t gPaletteMenuInGame[256];
// byte[0x8000] = 128 palettes x 256. memcpy(0x7d6468, src, 0x8000) at D2WINPAL_LoadPaletteFiles 004fb2c5..004fb2e8. Stride 0x100 from D2WINPAL_GetActPalettePointer (SHL EAX,8 / ADD EAX,0x7d6468 at 004fb0c2). Ends exactly at gPaletteAct2 (007de468).
extern uint8_t gPaletteAct1Outdoor[32768];
extern uint8_t gPaletteAct2[65536];
extern uint8_t gPaletteAct1[196608];
extern uint8_t gPaletteUnits[256];
// Was a bare undefined1 named gPaletteMenuInGame - a DUPLICATE of the byte[256] at 007d6368, which
// made the emitted tree write one of them as (&gPaletteMenuInGame)[i] and fail to compile.
// Evidence for byte[256]: D2WINPAL_LoadPaletteFiles stores MOV byte ptr [EDI + 0x81e568],AL with EDI
// an index, the next symbol is exactly 256 bytes away, and D2WINPALETTE_LoadPalettes walks this whole
// region from gPaletteDataBase in 0x100 steps up to 0x821b68. That walk is also why the name carries
// "Data": 007d6368 belongs to the 0x40-dword transpose run, 0081e568 to the palette-data run, and
// D2WINPALETTE_LoadPalettes hands both to D2GFX_SetPaletteTable as a pair.
extern uint8_t gPaletteDataMenuInGame[256];
extern PALETTEENTRY MostLikelyCurrentPalletePtr[256];
extern uint8_t gPaletteDataBase[12544];
extern uint8_t gPaletteAct5[65536];
extern uint8_t gPaletteActNeutral[65536];
extern uint32_t gbGammaShiftActive;
extern unsigned int gdwPaletteShiftStartTick;
extern uint32_t gbCompactLineSpacingMode;
extern DC6 * * gppDC6FontCache;
extern pfnFontGetGlyphEntry gpfnFontGetCharWidth;
extern DC6 * gpMonsterIndicatorDC6;
extern D2FontCacheEntryStrc gaFontCache[14];
extern uint8_t gcTooltipBarColor;
extern uint8_t gbFontIsKoreanChatMode;
extern D2TooltipTextStrc DisplayedTooltip;
extern uint16_t gnFontTableCharWidthCache[2];
extern uint16_t gwFontHeightDefault[2];
extern int nD2CompItemsSize;
extern void * pD2CompItems;
// 9 item palette-transform slots of 0x6900 bytes (D2FILE_ReadFile writes 0x1500 at +nSlot*0x6900) = 0x3B100 bytes.
// The guild-emblem primary colour table follows at 0087d810; D2COMP_LoadGuildEmblemIcon reads it as base-1 (0087d80f + n, n = 1..10).
extern uint8_t gaItemPaletteTransformData[241920];
// Guild-emblem PRIMARY colour table: 10 palette indices written by D2COMP_InitEmblemColorTables (005055f3), read 1-based by D2COMP_LoadGuildEmblemIcon (00505425: [0x87d80f + byPrimaryColor]); 2 bytes pad to gpHitClassTxt. Twin of gaGlideEmblemSecondaryColorTableEntries.
extern uint8_t gaGlideEmblemPrimaryColorTableEntries[12];
extern D2TxtLinkStrc * gnItemTypesCacheHandle;
extern void * gpItemTypesTxt;
extern size_t gnD2CompCompressionBufferSize;
extern int gnMaxCharacters;
extern D2CompCharacterCompositeItemStrc gaCompCharacterCompositeItems[255];
// Per composite item (parallel to gaCompCharacterCompositeItems, 255 entries): weapon class from Items.txt 2handedwclass.
// D2COMP_AllocTxt_EverythingItems 0050665e stores the gaCompWeaponClassByCodeIndex entry matched on D2ItemsTxt.dw2HandedWClass (+0xc4).
extern int gaCompTwoHandedWeaponClass[255];
// Guild emblem SECONDARY colour table: 10 used entries + 2 pad. D2COMP_InitEmblemColorTables writes [0..9]; D2COMP_LoadGuildEmblemIcon reads it base-1 from 0x0087e82b (compiler-folded -1), which is the top byte of gpItemTypeHierarchy[254]. That base cannot be given to this array without truncating gpItemTypeHierarchy to 254 real entries, so the reader still resolves through the neighbour and will read 10 bytes past gpItemTypeHierarchy once relinked - wrong secondary emblem colour, read-only, no corruption. Deliberate: the alternative reintroduced a 1004-byte out-of-bounds WRITE. Real fix belongs in the emitter (prefer "one before symbol X" over "past the end of symbol Y").
extern uint8_t gaGlideEmblemSecondaryColorTableEntries[12];
extern D2CharSelCompStrc * gpCompositeUnitListHead;
extern void * gpCOFCacheHead;
extern DC6 * gpUniqueCursorDC6;
extern int gnUniqueCursorMaxFrames;
extern DC6 * gpPentaSpinCursorDC6;
extern int gnPentaSpinCursorMaxFrames;
extern char gpSmackerVideoBuffer[260];
extern uint8_t gbSmackerVideoFlags;
extern uint32_t gbSmackerKeepTempFile;
extern DC6 * gpPentSpinCursorDC6;
// Global pointer to D2Win popup control form. Contains D2ControlStrc base, message count, array of 8 popup message entries (132 bytes each), and selected index. Created/destroyed dynamically for tooltip popups and context menus. 1128 bytes total.
extern D2WinPopup * gpD2WinPopupControl;
extern grDrawLine GLIDEDLL__grDrawLine_8;
extern grDepthMask GLIDEDLL_grDepthMask_4;
extern grDrawPoint GLIDEDLL_grDrawPoint_4;
extern grGlideShutdown GLIDEDLL_grGlideShutdown_0;
extern grDitherMode GLIDEDLL_grDitherMode_4;
extern grTexDownloadMipMap GLIDEDLL_grTexDownloadMipMap_16;
extern pfnGlideGammaCorrectionRGB GLIDEDLL_guGammaCorrectionRGB_12;
extern grDrawVertexArrayContiguous GLIDEDLL_grDrawVertexArrayContiguous_16;
extern grAlphaBlendFunction GLIDEDLL_grAlphaBlendFunction_16;
extern grTexCombine GLIDEDLL_grTexCombine_28;
extern grLfbUnlock GLIDEDLL_grLfbUnlock_8;
extern grLfbLock GLIDEDLL_grLfbLock_24;
extern grLoadGammaTable GLIDEDLL_grLoadGammaTable_16;
extern grAlphaCombine GLIDEDLL_grAlphaCombine_20;
extern grSstWinOpen GLIDEDLL_grSstWinOpen_28;
extern grTexFilterMode GLIDEDLL_grTexFilterMode_12;
extern grGlideInit GLIDEDLL_grGlideInit_0;
extern grFinish GLIDEDLL_grFinish_0;
extern pfnGlideDrawVertexArray GLIDEDLL_grDrawVertexArray_12;
extern grGet GLIDEDLL_grGet_12;
extern grTexMinAddress GLIDEDLL_grTexMinAddress_4;
extern grColorCombine GLIDEDLL_grColorCombine_20;
extern grColorMask GLIDEDLL_grColorMask_8;
extern grChromakeyValue GLIDEDLL_grChromakeyValue_4;
extern grCoordinateSpace GLIDEDLL_grCoordinateSpace_4;
extern grSstSelect GLIDEDLL_grSstSelect_4;
extern grTexDownloadTable GLIDEDLL_grTexDownloadTable_8;
extern grTexMaxAddress GLIDEDLL_grTexMaxAddress_4;
extern GlSubtileCacheStrc gGlideSubtileTextureCache;
extern GlFloorCacheStrc gGlideFloorTextureCache;
extern grBufferClear GLIDEDLL_grBufferClear_12;
extern grChromakeyMode GLIDEDLL_grChromakeyMode_4;
extern grBufferSwap GLIDEDLL_grBufferSwap_4;
extern uint32_t ghGlideSstWindow;
extern grTexSource GLIDEDLL_grTexSource_16;
extern grVertexLayout GLIDEDLL_grVertexLayout_12;
extern uint32_t gGlidePaletteEntry0[256];
extern grConstantColorValue GLIDEDLL_grConstantColorValue_4;
extern grSstWinClose _GLIDEDLL_grSstWinClose_4;
extern HMODULE lpGLIDE3x_dll;
extern int gnGlideTextureMemoryUsed;
extern uint32_t gbGlideIsDebugBuild;
extern FxI32 gnGlideTextureMemoryMask;
extern int gnGlideSecondaryTMUCount[2];
extern uint32_t ghGlideWindow;
extern uint32_t gbGlideInitialized;
extern uint32_t gbPlayingCutScene;
extern int gbWINDOWED;
extern uint32_t gnGlideColorCombineMode;
extern uint32_t gnGlideAlphaBlendMode;
extern uint32_t gnGlideTextureFilteringMode;
extern uint32_t gbGlideInScene;
extern uint32_t gbGlidePerspectiveMode;
extern uint32_t gnGlideSpriteSecondaryTMUEnabled;
extern uint32_t gnGlideTMUIndex;
extern D2GlideVertexTexCacheBlockStrc gGlideVertex0;
extern void * gpGlideSpriteBuffer;
extern void * gpGlideSpritePixelBuffer;
extern uint32_t gaGlideColorDebugGradientIndices[1024];
extern uint32_t gaGlideDebugFontTexStartAddrs[96];
extern float gfGlideDebugFontVertex0X;
// @data gfGlideDebugFontVertex0Y
// @wasnamed gfGlideDebugFontVertex1OOW
// @evidence Vertex 0, field +4 (y). 0050bc6e calls
// grDrawVertexArrayContiguous(FAN, 4, 0x880258, 0x1c): 4 vertices of stride 0x1c with fields
// +0 x, +4 y, +8 argb, +0xc oow, +0x10 s, +0x14 t. This slot is written at 0050bc14 with the glyph Y.
// @wasnamed gfGlideDebugFontVertex1OOW - wrong vertex and wrong field.
extern float gfGlideDebugFontVertex0Y;
// @data gfGlideDebugFontVertex1X
// @wasnamed gfGlideDebugFontVertex1Y
// @evidence Vertex 1, field +0 (x): copied from vertex 0's x at 0050bc25.
// @wasnamed gfGlideDebugFontVertex1Y.
extern float gfGlideDebugFontVertex1X;
// @data gfGlideDebugFontVertex1Y
// @wasnamed gfGlideDebugFontVertex2S
// @evidence Vertex 1, field +4 (y): written at 0050bc38 with y+16.
// @wasnamed gfGlideDebugFontVertex2S.
extern float gfGlideDebugFontVertex1Y;
extern float gfGlideDebugFontVertex3S;
extern float gfGlideDebugFontVertex2T;
// @data gfGlideDebugFontVertex3X
// @wasnamed gfGlideDebugFontVertex3OOW
// @evidence Vertex 3, field +0 (x): written at 0050bc4d with x+8.
// @wasnamed gfGlideDebugFontVertex3OOW.
extern float gfGlideDebugFontVertex3X;
extern float gfGlideDebugFontVertex3T;
// lpBuffer parameter of GetCharWidthA
extern void * lpBuffer_008802c8[96];
extern void * gpGlideDebugFontTexture;
extern uint32_t gnGlideDebugFontTMU;
extern int * gpnLightmapRowPointers;
extern D2GlideVertexStrc gaGlidePerspectiveVertices[15];
extern D2GlideVertexStrc gGlidePerspectiveVertexBuffer[4];
extern D2GlideVertexStrc gaGlideGroundTileVertices[36];
// @data gapGlideGroundStripVertexPtrs
// @wasnamed gapGlideTileVertexPtrs
// @evidence Built at runtime by 0050d646 and consumed by the loop at 0050daf8,
// which calls grDrawVertexArray(STRIP, 12, .) five times up to 00880b58. A strip table, not the
// static fan table at 0072f670. @wasnamed gapGlideTileVertexPtrs.
extern void * gapGlideGroundStripVertexPtrs[60];
extern uint8_t gbGlideGlobalLightGreen;
extern uint8_t gbGlideGlobalLightBlue;
extern uint8_t gbGlideGlobalLightRed;
extern uint8_t * gpGlideLightingTable;
extern int gnGlidePerspectiveOriginX;
extern int32_t gnGlidePerspectiveScale;
extern int gnGlidePerspectiveOriginY;
extern void * gpPerspectiveLookupTable;
extern uint8_t (*gpGlideSmackerTexBuf0)[32768];
extern uint8_t (*gpGlideSmackerTexBuf1)[32768];
extern uint32_t gnGlideSmackTex1StartAddr;
extern uint32_t gnGlideSmackTex2StartAddr;
extern uint8_t (*gpGlideSmackerTexBuf2)[32768];
extern uint32_t gnGlideTMUHandle;
extern uint32_t gnGlideTMUId;
extern void * gpGlBlocksWallBuffer;
extern void * gpGlBlocksFloorBuffer;
extern uint32_t gnBinkGlideYOffset;
extern uint32_t gnBinkGlideXOffset;
extern void * gpBinkVideoGlide;
extern uint32_t gnGlideBinkSurfaceType;
extern uint32_t g_bGlideBinkVideoQuitRequested;
extern PALETTEENTRY gaPaletteEntries[256];
extern uint32_t gdwDDrawInitFlags;
extern GUID gGlideOpenWindowConfig;
extern uint32_t gnDDrawSurfaceWidth;
extern uint32_t gnDDrawDisplayModeIndex;
extern uint32_t gnDDrawSurfaceHeight;
extern uint32_t gbDDrawWindowInitialized;
extern BOOL gbDDrawPaletteSet;
extern uint32_t gnCutSceneActiveFlag;
extern IDirectDraw1 * pWindowHandleBuffer;
extern IDirectDrawSurface1 * gpDDrawPrimarySurface;
extern IDirectDrawSurface1 * gpDDrawBackBuffer;
extern IDirectDrawSurface1 * pDirectDrawSurface;
extern IDirectDrawPalette * pDirectDrawPalette;
extern uint32_t gnDDrawFlipState;
extern uint32_t gnDDrawPaletteSize;
extern uint32_t gnDDrawSurfaceXMin;
extern uint32_t gnDDrawRenderWidth;
extern uint32_t gaD2ddDebugFontCharOffsets[96];
// lpBuffer parameter of GetCharWidthA
extern void * lpBuffer_00881428[96];
extern D2ddDebugFont * gpD2ddDebugFont;
extern uint8_t gaSmackerPaletteRemap[260];
// @data gbSmackerPaletteDirty
// @wasnamed gnBinkDDrawYOffset
// @evidence Set to 1 after a palette rebuild at 00513674, gates
// RENDERER_DDRAW_BlitSmackerFrame at 005136aa, cleared to 0 at 00513493. A dirty flag, not an offset.
// @wasnamed gnBinkDDrawYOffset; the genuine offset is 008816b8, computed at 005138ef as
// (WindowHeight - binkHeight) >> 1 and passed to BinkCopyToBuffer.
extern uint32_t gbSmackerPaletteDirty;
extern uint32_t gnBinkDDrawYOffset;
extern uint32_t gnBinkDDrawXOffset;
extern void * gpBinkVideoDirectDraw;
extern uint32_t gnBinkDDrawSurfaceType;
extern uint32_t g_bBinkDDrawVideoQuitRequested;
extern IKsPropertySet * gpDirectSoundEAXInterface;
extern HWND g_hwndSoundWindow;
extern IDirectSound3DBuffer * gpEAXInterface1;
extern IDirectSoundBuffer * gpEAXInterface2;
extern IDirectSoundBuffer * gpDSoundPrimaryBuffer;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION gSoundSmpCs;
extern uint32_t gnDirectSoundInitialized;
extern IDirectSound * gnDirectSoundInitStatus;
extern uint32_t gnDSoundPrimaryBufferDelay;
extern uint32_t gnSound3DLoudness;
extern int gnSoundQualityMode;
extern uint32_t g_bEAXEnabled;
extern uint32_t gbDirectSoundCritSecInitialized;
extern uint32_t g_bSoundDriverScanComplete;
extern uint32_t gbDirectSoundDelayWorkaround;
extern uint32_t gbHasLegacyEMU10K1Driver;
extern uint32_t gbHasLegacySoundDriverCrlds3d;
extern uint32_t LanguageCode;
extern D2SoundSmpStrc * gpMusicStreamBuffer;
extern uint32_t gnMusicEnabled;
extern uint32_t gbSoundFastPathActive;
extern uint32_t gnAmbientSoundLanguageCode;
extern uint32_t gbAmbientSoundLangInitialized;
extern uint32_t g_nSoundMixerType;
extern uint32_t g_nSoundMasterVolume;
extern uint32_t g_nSoundMusicVolume;
extern uint32_t g_nSoundPositionalBias;
extern uint32_t g_nSoundNpcSpeech;
extern uint32_t g_nSoundOptionsMusic;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_008817c4;
extern D2SoundSmpStrc * gpaSoundEffectBuffers[8];
extern uint32_t gnSoundEffectSampleCount;
extern void * gpWavFileCacheListHead;
extern uint32_t g_nSoundSampleParam;
extern int * gpSoundSampleList;
// @data ghSoundThreadStopEvent
// @wasnamed ghSoundThread
// @evidence Stores the result of CreateEventA at 0051654a - an EVENT, not a thread.
// SetEvent(it) at 00515129 and CloseHandle at 00515148. The genuine thread handle is 00881810, which
// stores CreateThread at 00516566 and is the one passed to WaitForSingleObject at 0051513a.
extern HANDLE ghSoundThreadStopEvent;
extern HANDLE ghSoundThread;
extern CRITICAL_SECTION gcsErrorHandlerList;
extern uint32_t gbFaultHandlerListInitialized;
extern pfnFaultError g_pfnFaultErrorCallback;
extern D2BNetGatewayAccessStr ActualRealmInfo;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
// lpCriticalSection parameter of DeleteCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_00881860;
extern int gnBnetLogFileHandle;
extern unsigned int gbBnetLogInitialized;
extern D2BnCacheFileHeaderStrc * gpCacheFileHeader;
extern void * gpCacheBlockTable;
extern void * g_pCacheCryptTable;
extern uint8_t * gpCacheBufferPoolFirst;
extern uint32_t gbCacheIsOpen;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
// lpCriticalSection parameter of DeleteCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_00881894;
extern uint32_t g_nBNetServerToken;
extern uint32_t g_nBNetClientToken;
extern void * g_pBNetChannelListData;
extern void * gpRealmQueryData;
extern void * g_pBNetGameListData;
extern void * gpBNetWriteUserData;
extern void * g_pBNetReadUserData;
extern void * g_pBNetCDKeyHashData;
extern uint32_t g_nBNetConnectionState;
// Destination parameter of InterlockedCompareExchange
extern LONG * g_pBNetSocketData;
// lpAddend parameter of InterlockedDecrement
// lpAddend parameter of InterlockedIncrement
// Destination parameter of InterlockedCompareExchange
extern LONG * Destination_00881bf4;
extern uint32_t gbBNetSocketConnected;
// Destination parameter of InterlockedCompareExchange
// Target parameter of InterlockedExchange
extern long Target_00881bfc;
// Destination parameter of InterlockedCompareExchange
// lpAddend parameter of InterlockedDecrement
// lpAddend parameter of InterlockedIncrement
extern LONG * Destination_00881c00;
extern uint32_t g_nBNetThreadInitFlag;
extern uint32_t g_pfnNewsChannelCallback;
extern pfnVoidNoArgCallback g_pfnMotdCallback;
extern pfnVoidNoArgCallback g_pfnNewsCallback;
extern D2BnNewsTableStrc g_nNewsMaxCacheSize;
extern uint32_t g_nFirstUnreadNewsIndex;
extern uint32_t g_dwNewsTimestamp;
extern unsigned int gnNetworkSendState;
extern unsigned int gbBnetDownloadCancelFlag;
// lpFileName parameter of DeleteFileA
extern char lpFileName_00881c58[260];
// lpAddend parameter of InterlockedDecrement
// lpAddend parameter of InterlockedIncrement
extern LONG * lpAddend_00881d5c;
extern uint8_t gbBnetFtpFileTransferActive;
extern uint8_t bBnetFtpDownloadActive;
extern char gszBNetChatInputBuffer[260];
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_00881f64;
extern char g_szBnetConnectionStateString[128];
extern uint32_t g_nBattleNetConnectionData;
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
// lpCriticalSection parameter of DeleteCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_00882004;
extern uint32_t g_nBattleNetConnectionState;
extern BnMessQueue * nLobbyChatCurrentMessage;
extern BnMessQueue * nLobbyChatQueueMessages;
extern char g_szCurrentChannelName[256];
extern BOOL NET_SID_INCOMING_CLIENT_PacketRequireAnswer[94];
extern uint32_t g_nSIDResponseCount1;
extern uint32_t g_nSIDResponseCount2;
// Indexed by SID message id; former element labels: gbSIDVersionCheckComplete=[7] g_bGameCreationSuccess=[28] g_dwSIDCreateAccountStatus=[42] gbSIDChangePasswordComplete=[49] gbSIDCreateAccount2Complete=[61] gbCDKeyValidationSuccess=[66] gbAuthCheckVersionValid=[81] gbSIDAccountCreateComplete=[82] gbAuthChangeProofComplete=[85]
extern uint32_t g_SIDPacketResponseFlags[94];
extern char gszCDKeyInUseByUsername[256];
extern uint32_t g_bSIDProtocolReady;
extern uint32_t g_dwLastAdId;
extern uint32_t g_bSIDFlag2;
extern char g_szStoredUsername[256];
extern D2SrpSaltVerifierStrc g_srpSaltVerifier;
extern D2SrpCredentialsStrc g_srpCredentials;
extern D2Srp * g_pSrpContext;
extern uint8_t g_SHA1Contexts[276];
extern uint8_t g_IDEAKeySchedules[312];
extern void * gpUntranslatedStringCacheHead;
extern D2StringTableTblFileStrc * gpStringTableBase;
extern uint16_t * * gpStringIndexBase;
extern uint16_t * * gpStringIndexPatchD2;
extern uint16_t * * gpStringIndexExpansion;
extern uint16_t * gpStringBufferBase;
extern uint16_t * gpStringBufferPatchD2;
extern uint16_t * gpStringBufferExpansion;
extern D2StringTableTblFileStrc * gpStringTablePatchD2;
extern D2StringTableTblFileStrc * gpStringTableExpansion;
extern int gnStringTableRefCount;
extern uint16_t gnUnicodeKamapEntryCount;
extern uint8_t gnUnicodeKamapGroupCount;
extern void * gpUnicodeKamapToCodeTable;
extern void * g_pUnicodeKamapTable;
extern uint16_t * gpUnicodeCodeToMapTable;
// lpThreadId parameter of CreateThread
extern LPDWORD lpThreadId_00882b30;
extern D2PacketsClientStrc * pD2PacketsClientBuffer;
extern HANDLE hClientConnectionThread;
extern D2OnePacketClientStrc * pD2PacketClient_from_0xAF;
extern HANDLE SERVER_THREAD;
extern D2OnePacketClientStrc * pD2PacketClient_to_0xAE;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_00882ce8;
extern uint32_t gbClientConnectionShutdown;
extern int gnNetConnectionTimeoutMs;
extern D2QServerStrc * pQServer;
extern uint32_t gnDefaultClientGameToken;
extern eD2GSConnectionType nD2GSConnectionType;
extern uint32_t gnQServerListenSocket;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION gQServerGameListCs;
extern SGAMEDATA * gaQServerGameByToken[1025];
extern TSExportTableSync<SGAMEDATA, HSGAMEDATA__*, HSGAMEDATALOCK__*, CCritSect, 0> * gpQServerGameState;
extern int StartingAct;
extern uint32_t maxMemoryUsage;
extern int gbQServerSendPlayerPos;
extern D2BattleNetEventCallbackTable * BattleNetServerService;
extern BOOL IsBattleNetServer;
extern uint32_t Frames_TimeBetweenFrames;
extern uint32_t PlayerCountOverride;
// 17 records of 16 bytes, one per NPC vendor store index. The count is pinned two ways: SUNITPROXY_FreeAllNpcItemTables walks from &record[0].pQuestItems to that address + 272, and 00883d78 + 17*16 lands exactly on gbaNpcItemTableBuilt at 00883e88. Ghidra had typed the four fields of record 0 as four separate globals (gpaNpcTradeItemTable, ganNpcTradeItemCounts, gpaNpcQuestItemTable, ganNpcTradeQuestItemCounts), so only vendor 0 had real storage and every other vendor's item pointers and counts were read from and written to unrelated .data - the shop never produced a valid item list.
extern D2NpcItemTablesStrc gaNpcItemTables[17];
// One byte per vendor store index, 17 of them - the 'item table already built' guard. Two stray interior labels at +1 and +8 had truncated it to a single byte.
extern uint8_t gbaNpcItemTableBuilt[17];
extern uint32_t gptItemCodeArmor;
extern uint32_t gptItemCodeSwordType;
extern D2ClientStrc * QServerClientBucketByName[256];
extern D2ClientStrc * QServerClientBucketById[256];
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern CRITICAL_SECTION gQServerClientByIdCs;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern CRITICAL_SECTION gQServerClientByNameCs;
extern int gQServerClientTablesInitialized;
extern BOOL DEBUGPRINT;
extern uint32_t g_bWhisperSquelched;
extern uint32_t g_bWhisperDelivered;
extern uint32_t gnSeedCounter;
extern D2UniqueCandidatePoolStrc gUniqueCandidatePool;
extern D2TaskQueueSlotStrc * gaTaskQueueHeadArray[32];
extern D2LinkNodeStrc gsTaskQueueListHead;
// lpFrequency parameter of QueryPerformanceFrequency
extern LARGE_INTEGER gnPerformanceFrequency;
extern uint32_t gnTaskQueueSlotCounter;
extern uint32_t gnTaskQueueTailIndex;
extern uint32_t gbPerformanceCounterAvailable;
extern CRITICAL_SECTION aTaskQueueCriticalSections[32];
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0088caa8;
extern uint32_t gnMonsterSpawnTotalCount;
extern D2LRUCacheStrc gSpriteCacheLru;
extern int gbSpriteCacheLruInitialized;
extern void * gpSpriteCachePoolManager;
extern pfnLruFreeEntry gpfnSpriteCacheFreeEntry;
extern BYTE gaCelDataHashTable[4096];
extern D2LRUCacheStrc gCelDataCacheLru;
extern BYTE gaGfxHashTable[65536];
extern uint32_t gbSpriteCacheConversionEnabled;
extern D2GfxMemoryStrc D2GfxMemoryStrc_0089db60;
extern D2TileProjectStrc * gpTileProjectListHead;
extern uint8_t gaItemPaletteData[241920];
extern pfnCelPixelDataFree gpCelPixelDataFreeCallback;
extern TSExportTableSync<CELLIST, HCELLIST__*, HLOCKEDCELLIST__*, CCritSect, 0> gCelCmpCacheHandler;
extern uint32_t gbTransparentColorEnabled;
extern uint32_t gnPaletteMaxIndex;
extern uint32_t gaColorDistanceLookupTable[512];
extern uint8_t gaPaletteMaskData[256];
extern uint32_t gnPaletteColorDepth;
extern uint32_t gbPaletteLoaded;
extern D2PaletteEntryStrc gaGlobalPaletteRed[256];
extern uint32_t gnDrawTransparentColorRGB;
extern uint32_t gbBlackColorIsTransparent;
extern uint32_t gbPaletteMaskEnabled;
extern int32_t gnFastCmpEffectMapMaxRow[80];
extern int32_t gnFastCmpEffectMapMinRow[80];
extern uint32_t gaFastCmpEffectPixelOffsetTable[6400];
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_008f03cc;
// lpAddend parameter of InterlockedDecrement
// lpAddend parameter of InterlockedIncrement
extern LONG * lpAddend_008f03e4;
extern uint8_t gaCodecFrameWorkBuffer2[262144];
extern D2CodecPixelBufferEntryStrc gaCodecFrameDecodeBuffer[5625];
extern IgnoreList * gpIgnoreList;
extern unsigned int gpfnProfCoreInitCallback;
extern D2DataTableTxtStrc sgtDataTable;
extern D2SeedStrc gsAutoMapSeed;
extern D2AutomapRelatedTxt gpaAutoMapRelated[36];
extern D2LvlTypesTxt * pTxtLvlTypes;
extern void * gpPortalLevelArray;
extern unsigned int nPortalLevelCount;
extern int nTxtLvlTypesSize;
extern D2LvlWarpTxt * pTxtLvlWarp;
extern int nTxtLvlWarpSize;
extern D2LvlMazeTxt * sgpTxtLvlMaze;
extern int nTxtLvlMazeSize;
extern D2LvlSubTxt * sgpTxtLvlSub;
extern int nTxtLvlSubSize;
extern int * gpnLvlSubTypeToIndex;
extern D2ExpansionLvlTblsTxt * pExpansionLvlTbls;
extern D2LvlTblsTxt * pTxtLvlTbls;
extern void * gpaDrlgLvlSubPresetFiles;
extern D2AutoMapLvlTbls * pAutoMapLvlTbls;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0096ca30;
extern FILE * pLOGFILE;
extern BOOL bLOGFILE_ENABLED;
extern unsigned long long gpItemTypeTableContext;
extern D2ItemsTableStrc gsItemsTable;
extern D2ItemsTableStrc sgpTxtItems;
extern short * pTxtItemsVersion;
extern D2MagicAffixTableStrc gsMagicAffixTable;
extern D2GemsTableStrc nTxtGemsSize;
extern D2RunesTxtTableStrc nTxtRunesSize;
extern D2RareAffixTableStrc gsRareAffixTable;
extern D2GambleTableStrc nTxtGambleSize;
extern int nTxtItemRatioSize;
extern D2ItemRatioTxt * pTxtItemRatio;
extern D2QualityItemsTableStrc nTxtQualityItemsSize;
extern D2LowQualityItemsTableStrc nTxtLowQualityItemsSize;
extern D2BooksTableStrc gsBooksTable;
extern uint32_t gaItemTypeBaseIdTable[510];
extern D2ShrinesTxt * pTxtShrines;
extern int nTxtShrinesSize;
extern D2ObjectsTxt * sgpTxtObjects;
extern int nTxtObjectsSize;
extern D2ObjGroupTxt * pTxtObjGroup;
extern int nTxtObjGroupSize;
extern unsigned int gnTreasureClassArraySize;
extern D2MonItemPercentTxt * pMonItemPercentTxt;
extern D2UniqueNameTablesStrc gsTxtUniqueNameTables;
extern unsigned int gnAutoItemTypeTCCount;
extern unsigned short gwMonsterTblsDefaultId;
extern unsigned int nTxtMonModeSize;
extern D2MonModeTxt * pTxtMonMode1;
extern D2MonModeTxt * pTxtMonMode2;
extern D2MonModeTxt * pTxtMonMode3;
extern unsigned int nTxtObjTypeModeSize;
extern D2ObjTypeTxt * pTxtObjTypeMode;
extern D2ObjTypeTxt * pTxtObjType;
extern D2ObjModeTxt * pTxtObjMode;
extern unsigned int nTxtPlrTypeModeSize;
extern D2PlrTypeModeTxt * pTxtPlrTypeMode1;
extern D2PlrTypeModeTxt * pTxtPlrTypeMode2;
extern D2PlrTypeModeTxt * pTxtPlrTypeMode3;
extern D2ArmTypeTxt * pTxtArmType;
extern D2CompositTxt * pTxtComposit;
extern int nTxtInventorySize;
extern D2InventoryTxt * pTxtInventory;
extern D2BeltsTxt * pTxtBelts;
extern int sgCharTemplateLevelIndex[64];
extern D2CharTemplateTxt * sgpTxtCharTemplate;
extern int nTxtCharTemplateSize;
extern D2ArenaTxt * sgpTxtArena;
extern D2DrlgFileCacheNodeStrc * gpDrlgFileCache;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0096d610;
extern D2CubeMainTxt * pTxtCubeMain;
extern int nTxtCubeMainSize;
extern void * gpFieldDataTable;
extern unsigned int gnFieldDataTableWidth;
extern unsigned int gnFieldDataTableHeight;
extern int gnFieldDataTableSize;
extern unsigned int gnFieldTableHalfWidth;
extern unsigned int gnFieldTableHalfHeight;
extern unsigned int gdwDrlgLevelPlacementFlags;
extern unsigned int UINT_0096d6a8;
extern GUID gsD3DAdapterGuid;
extern unsigned int g_nVideoMemoryTotal;
extern unsigned int g_nVideoMemoryFree;
extern unsigned int gnD3DTextureCacheBlockSize;
extern D2D3DDeviceInfoStrc gdwD3DDeviceFlags[4];
extern uint16_t g_Palette16Bit[256];
extern D3DSubtileCacheStrc gD3DSubtileTextureCache;
extern D3DFloorCacheStrc gD3DFloorTextureCache;
extern uint16_t g_OriginalGammaRamp[3][256];
extern int gnD3DTextureCacheCount;
extern uint32_t g_Palette32Bit[256];
extern D2D3DDeviceSlotStrc gD3DDeviceBlock0[2];
extern unsigned int g_bDeviceInitialized;
extern int g_bCutsceneModeActive;
extern BOOL g_bD3DCoreInitialized;
extern unsigned int g_bSupportsTextureCachingInternal;
// @data g_nCurrentTextureOpMode
// @wasnamed g_nCurrentBlendMode
// @evidence Caches SetTextureStageState COLOROP/ALPHAOP and ALPHATESTENABLE at
// 006b1ea0. @wasnamed g_nCurrentBlendMode, shared with 00970e58 which caches a different state group.
extern unsigned int g_nCurrentTextureOpMode;
// @data g_nCurrentAlphaBlendMode
// @wasnamed g_nCurrentBlendMode
// @evidence Caches ALPHABLENDENABLE / SRCBLEND / DESTBLEND at 006b1f40.
extern unsigned int g_nCurrentAlphaBlendMode;
extern unsigned int g_nCurrentFilterMode;
extern IDirectDraw * g_pDirectDraw7;
extern IDirect3DInterface7 * g_pDirect3D7;
extern IDirectDrawSurface7 * g_pD3DSurfaceBackbuffer;
extern IDirectDrawSurface7 * D3DSurfaceSecondary;
extern IDirectDrawGammaControl * g_pDDGammaControl;
extern IDirect3DDevice * g_pD3DDevice;
extern IDirect3DViewport3 * g_pD3DViewport;
extern unsigned int g_nBytesPerPixel;
extern HWND ghD3DEnumWindowHandle;
extern int gnDirect3DDeviceCount;
extern unsigned int g_bPerspectiveMode;
extern D3DTextureCacheSetStrc gD3DGenericTextureCache;
extern void * gpD3DTextureSpritePoolMemory0;
extern void * gpD3DTextureSpritePoolMemory1;
extern IDirect3DVertexBuffer * g_pD3DSpriteQuadVB;
extern int gd3dVertexBufferOffset;
extern int gD3DFontVertex0;
extern float gfD3DFontVertex0X;
extern float gfD3DFontVertex0Z;
extern float gfD3DFontVertex0Y;
extern float gfD3DFontVertex2Y;
extern float gfD3DFontVertex1Y;
extern float gfD3DFontVertex2X;
extern float gfD3DFontVertex3X;
// lpBuffer parameter of GetCharWidthA
extern void * g_pFontTextureBuffer[96];
extern void * g_pFontTextureSurface[96];
extern void * g_pFontTextureHandle[96];
extern D2D3DVertexStrc gaD3DPerspectiveStripVertices[15];
// @data gaD3DPerspectiveVertices
// @wasnamed gpD3DPerspectiveVertexBuffer
// @evidence The vertex array itself (this address is vertex 0's x), not a pointer to
// one: 006b80af does FST float with 0.0 into it, and the stride is a 32-byte D3DTLVERTEX (z=0.5 at
// +8, rhw=1.0 at +0xc, tu/tv at +0x18/+0x1c). @wasnamed gpD3DPerspectiveVertexBuffer.
extern D2D3DVertexStrc gaD3DPerspectiveVertices[4];
extern IDirect3DVertexBuffer * g_pD3DGroundTileVB;
extern uint8_t GlobalGreen;
extern uint8_t GlobalBlue;
extern uint8_t GlobalRed;
extern uint8_t g_aLightingTable[65536];
extern int gnPerspectiveScaleY;
extern int gnPerspectiveScaleZ;
extern uint32_t g_aPerspectiveZDivisionTable[8192];
extern uint32_t g_aSmackerSurfaceArray;
extern void * pSmackerDecodeBuffer;
extern IDirectDrawSurface7 * gpD3DSmackerSystemSurface0[2];
extern IDirect3DTexture2 * gpD3DSmackerSystemTexture0[2];
extern IDirectDrawSurface7 * gpD3DSmackerVideoSurface0[2];
extern IDirect3DTexture2 * gpD3DSmackerVideoTexture0[2];
extern IDirectDrawSurface7 * gpD3DFloorTileSystemSurface;
extern IDirect3DTexture2 * gpD3DFloorTileMemoryTexture;
extern IDirectDrawSurface7 * gpD3DSubtileSystemSurface;
extern IDirect3DTexture2 * gpD3DSubtileMemoryTexture;
extern uint32_t gnD3DBinkVideoYOffset;
extern uint32_t gnD3DBinkVideoXOffset;
extern void * gpBinkVideoD3D;
extern uint32_t gnBinkDDSurfaceType;
extern uint32_t gbBinkVideoShouldStop;
extern uint32_t gbQServerUseDedicatedIoWorkers;
extern unsigned int gdwHacklistLastLogTime;
extern uint32_t gbScmdHasPriorityCommand;
// prgbq parameter of SetDIBColorTable
extern RGBQUAD prgbq_00989c40[256];
extern BYTE * gpGlidePaletteTableData[72];
extern HPALETTE RENDERER_WINDOWED_hPalette;
extern HBITMAP RENDERER_WINDOWED_hDIBitmap;
extern uint32_t RENDERER_WINDOWED_nScaleOffsetY;
extern uint32_t RENDERER_WINDOWED_nScaleWidth;
extern HFONT RENDERER_WINDOWED_hDebugFont;
extern D2SmackerGlideBufferStrc gnSmackerGlideYOffset;
extern uint32_t gnSmackerDecodeState;
extern char gszSystemGlobalString[1024];
extern HMODULE hDbgHelpDll;
extern pfnSymGetModuleInfo64 pSymGetModuleInfo64;
extern pfnSymGetSymFromAddr64 pSymGetSymFromAddr64;
extern pfnSymSetSearchPath pSymSetSearchPath;
extern pfnSymUnloadModule64 pSymUnloadModule64;
extern pfnSymFunctionTableAccess64 pSymFunctionTableAccess64;
extern pfnSymGetModuleBase64 pSymGetModuleBase64;
extern pfnStackWalk64 pStackWalk64;
extern pfnEnumerateLoadedModules64 pEnumerateLoadedModules64;
extern pfnSymInitialize pSymInitializeApi;
extern pfnSymGetOptions pSymGetOptionsApi;
extern uint8_t bDebugSymbolsInitialized;
extern uint8_t bEnableCrashDumps;
extern pfnSymSetOptions pSymSetOptionsApi;
extern pfnSymCleanup pSymCleanupApi;
extern HANDLE hCrashDumpCompleteEvent;
extern HANDLE hCrashDumpRequestEvent;
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0098a6c4;
extern pfnSymLoadModule64 pSymLoadModule64;
extern pfnMiniDumpWriteDump pMiniDumpWriteDumpApi;
extern void * pCrashDumpInfo;
extern pfnSymGetLineFromAddr64 pSymGetLineFromAddr64;
extern pfnSymFindFileInPath pSymFindFileInPath;
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0098a6f0;
extern char szCustomSymbolSearchPath[32768];
extern char gszCrashLogTempDir[1024];
extern pfnFogExceptionFilter gpCurrentExceptionFilter;
extern void * gpCurrentExceptionContext;
extern uint32_t dwCurrentThreadId;
extern uint32_t gaEnumProcessModuleHandles[1028];
extern uint8_t IMAGE_RESOURCE_DIRECTORY_00995000[16];
// Rsrc_Icon_1_409 Size of resource: 0x2e8 bytes
extern uint8_t Rsrc_Icon_1_409[744];
// Rsrc_Icon_2_409 Size of resource: 0x128 bytes
extern uint8_t Rsrc_Icon_2_409[296];
// Rsrc_Icon_3_409 Size of resource: 0x2e8 bytes
extern uint8_t Rsrc_Icon_3_409[744];
// Rsrc_Icon_4_409 Size of resource: 0x128 bytes
extern uint8_t Rsrc_Icon_4_409[296];
// Rsrc_Icon_5_409 Size of resource: 0x8a8 bytes
extern uint8_t Rsrc_Icon_5_409[2216];
// Rsrc_Icon_6_409 Size of resource: 0x128 bytes
extern uint8_t Rsrc_Icon_6_409[296];
// Rsrc_GroupIcon_65_409 Size of resource: 0x22 bytes
extern uint8_t Rsrc_GroupIcon_65_409[34];
// Rsrc_GroupIcon_66_409 Size of resource: 0x22 bytes
extern uint8_t Rsrc_GroupIcon_66_409[34];
// Rsrc_GroupIcon_67_409 Size of resource: 0x22 bytes
extern uint8_t Rsrc_GroupIcon_67_409[34];
// Rsrc_Version_1_409 Size of resource: 0x454 bytes
extern uint8_t Rsrc_Version_1_409[92];
// Rsrc_Manifest_1_409 Size of resource: 0x56 bytes
extern char Rsrc_Manifest_1_409[1745];
extern void * ExceptionList;
extern void * SubSystemTib;
extern void * FiberData;
extern void * ArbitraryUserPointer;
extern void * Self;
extern void * EnvironmentPointer;
extern CLIENT_ID ClientId;
extern void * ActiveRpcHandle;
extern void * ThreadLocalStoragePointer;
extern uint32_t LastErrorValue;
extern uint32_t CountOfOwnedCriticalSections;
extern void * CsrClientThread;
extern void * Win32ThreadInfo;
extern uint32_t User32Reserved[26];
extern uint32_t UserReserved[5];
extern void * WOW32Reserved;
extern uint32_t CurrentLocale;
extern uint32_t FpSoftwareStatusRegister;
extern void * SystemReserved1[54];
extern uint32_t ExceptionCode;
extern void * ActivationContextStackPointer;
extern uint8_t SpareBytes[36];
extern uint32_t TxFsContext;
extern uint32_t GdiTebBatch;
extern CLIENT_ID RealClientId;
extern void * GdiCachedProcessHandle;
extern uint32_t GdiClientPID;
extern uint32_t GdiCLientTID;
extern void * GdiThreadLocalInfo;
extern void * Win32ClientInfo[62];
extern void * glDispatchTable[233];
extern void * glReserved1[29];
extern void * glReserved2;
extern void * glSectionInfo;
extern void * glSection;
extern void * glTable;
extern void * glCurrentRC;
extern void * glContext;
extern uint32_t LastStatusValue;
extern uint16_t StaticUnicodeBuffer[261];
extern void * DeallocationStack;
extern void * TlsSlots[64];
extern void * TlsLinks_Flink;
extern void * TlsLinks_Blink;
extern void * Vdm;
extern void * ReservedForNtRpc;
extern void * DbgSsReserved[2];
extern uint32_t HardErrorMode;
extern void * Instrumentation[9];
extern GUID ActivityId;
extern void * SubProcessTag;
extern void * EtwLocalData;
extern void * EtwTraceData;
extern void * WinSockData;
extern uint32_t GdiBatchCount;
extern uint32_t IdealProcessorValue;
extern uint32_t GuaranteedStackBytes;
extern void * ReservedForPerf;
extern void * ReservedForOle;
extern uint32_t WaitingOnLoaderLock;
extern void * SavedPriorityState;
extern void * SoftPatchPtr1;
extern void * ThreadPoolData;
extern void * TlsExpansionSlots;
extern uint32_t MuiGeneration;
extern uint32_t IsImpersonating;
extern void * NlsCache;
extern void * pShimData;
extern uint32_t HeapVirtualAffinity;
extern void * CurrentTransactionHandle;
extern void * ActiveFrame;
extern void * FlsData;
extern void * PreferredLanguages;
extern void * UserPrefLanguages;
extern void * MergedPrefLanguages;
extern uint32_t MuiImpersonation;
extern unsigned short CrossTebFlags;
extern unsigned short SameTebFlags;
extern void * TxnScopeEnterCallback;
extern void * TxnScopeExitCallback;
extern void * TxnScopeContext;
extern uint32_t LockCount;
extern void * ResourceRetValue;

namespace D2Common::Drlg::Maze {

extern uint8_t gaMazeLevelTypeCaseTable[40];

} // namespace D2Common::Drlg::Maze

namespace Fog::Src::Excel {

extern uint8_t TXT_ParseFieldTypeCaseIndex[26];

} // namespace Fog::Src::Excel

namespace D2Client::Warden::NWardenClient {

// meta pointer for NWardenClient::Library::vftable
extern void* vftable_meta_ptr;
// const NWardenClient::Library::vftable
extern void * vftable[7];

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

// meta pointer for WardenClient::vftable
extern void* vftable_meta_ptr;
// const WardenClient::vftable
extern void * vftable[7];

} // namespace D2Client::Warden

namespace D2Client::UI {

// meta pointer for UIWidget::vftable
extern uint8_t vftable_meta_ptr;
// const UIWidget::vftable
extern void * vftable[12];

} // namespace D2Client::UI

namespace D2Client::UI::Button::ButtonImplementation {

// meta pointer for ButtonImplementation::vftable
extern uint8_t vftable_meta_ptr;
// const ButtonImplementation::vftable
extern void * vftable[13];

} // namespace D2Client::UI::Button::ButtonImplementation

namespace D2Client::UI::Button::ButtonWrapper {

// meta pointer for ButtonWrapper::vftable
extern uint8_t vftable_meta_ptr;
// const ButtonWrapper::vftable
extern void * vftable[13];

} // namespace D2Client::UI::Button::ButtonWrapper

namespace D2Client::UI::Editbox::EditBoxImplementation {

// meta pointer for EditBoxImplementation::vftable
extern uint8_t vftable_meta_ptr;
// const EditBoxImplementation::vftable
extern void * vftable[19];

} // namespace D2Client::UI::Editbox::EditBoxImplementation

namespace D2Client::UI::Editbox::EditBoxWrapper {

// meta pointer for EditBoxWrapper::vftable
extern uint8_t vftable_meta_ptr;
// const EditBoxWrapper::vftable
extern void * vftable[19];

} // namespace D2Client::UI::Editbox::EditBoxWrapper

namespace D2Client::UI::Spinner::SpinnerImplementation {

// meta pointer for SpinnerImplementation::vftable
extern void* vftable_meta_ptr;
// const SpinnerImplementation::vftable
extern void * vftable[13];

} // namespace D2Client::UI::Spinner::SpinnerImplementation

namespace D2Client::UI::Spinner::SpinnerWrapper {

// meta pointer for SpinnerWrapper::vftable
extern void* vftable_meta_ptr;
// const SpinnerWrapper::vftable
extern void * vftable[13];

} // namespace D2Client::UI::Spinner::SpinnerWrapper

namespace D2Client::UNIT::Object {

extern D2ShrineDispatchEntryStrc aShrineDispatchTable[23];
extern int nShrineDispatchTableSize;

} // namespace D2Client::UNIT::Object

namespace D2Client::UI::ListBox::ListBoxWrapper {

// meta pointer for ListBoxWrapper::vftable
extern int vftable_meta_ptr;
// const ListBoxWrapper::vftable
extern void * vftable[16];

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

// meta pointer for ListBoxImplementation::vftable
extern void* vftable_meta_ptr;
// const ListBoxImplementation::vftable
extern void * vftable[16];

} // namespace D2Client::UI::ListBox::ListBoxImplementation

namespace D2Win::_FONT {

extern void * aFontNames[14];
extern char szKOR[4];
extern char sLanguageCode[8];

} // namespace D2Win::_FONT

namespace D2Game::GAME::Event::Dispatch {

extern int aTimerIndexUnitTypeLookupTable[6];
extern D2TimerTypeMethodsStrc pTimerMethods;

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::GAME::Event::Dispatch::Player {

extern D2TimerTypeMethodsExStrc pTimerMethods;

} // namespace D2Game::GAME::Event::Dispatch::Player

namespace D2Game::GAME::Event::Dispatch::Object {

extern D2TimerTypeMethodsStrc pTimerMethods;

} // namespace D2Game::GAME::Event::Dispatch::Object

namespace D2Game::GAME::Event::Dispatch::Monster {

extern D2TimerTypeMethodsExStrc pTimerMethods;

} // namespace D2Game::GAME::Event::Dispatch::Monster

namespace D2Common::Chat::Ignorelist::ChatIgnoreList {

// meta pointer for ChatIgnoreList::vftable
extern void* vftable_meta_ptr;
// const ChatIgnoreList::vftable
extern void * vftable[12];

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

// meta pointer for IgnoreList::vftable
extern void* vftable_meta_ptr;
// const IgnoreList::vftable
extern void * vftable[12];

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common::Drlg::Outdoors {

extern int aAct4OutdoorSceneryBaseByLevel[3];
extern int aAct4OutdoorPresetBaseByLevel[3];

} // namespace D2Common::Drlg::Outdoors

namespace Define_the_symbol__ATL_MIXED {

// meta pointer for Define_the_symbol__ATL_MIXED::Thank_you::vftable
extern void* vftable_meta_ptr;

} // namespace Define_the_symbol__ATL_MIXED

namespace Fog::_3rdparty::crashy {

// meta pointer for crashy::Report::vftable
extern void* vftable_meta_ptr;
// const crashy::Report::vftable
extern void * vftable[1];

} // namespace Fog::_3rdparty::crashy

namespace Storm::SFile {

// struct Storm::SFile::ARCHIVEREC RTTI Type Descriptor
extern uint8_t RTTI_Type_Descriptor;

} // namespace Storm::SFile

namespace Class::FILEREC {

// struct FILEREC RTTI Type Descriptor
extern uint8_t RTTI_Type_Descriptor;

} // namespace Class::FILEREC

namespace Class::EVENTREC {

// struct EVENTREC RTTI Type Descriptor
extern uint8_t RTTI_Type_Descriptor;

} // namespace Class::EVENTREC

namespace Class::REQUEST {

// struct REQUEST RTTI Type Descriptor
extern uint8_t RTTI_Type_Descriptor;

} // namespace Class::REQUEST

namespace Class::AUDIOSTREAM {

// struct AUDIOSTREAM RTTI Type Descriptor
extern uint8_t RTTI_Type_Descriptor;

} // namespace Class::AUDIOSTREAM

namespace Class::HANDLER {

// struct HANDLER RTTI Type Descriptor
extern uint8_t RTTI_Type_Descriptor;

} // namespace Class::HANDLER

namespace D2Client::CharScreen {

extern int nSelectedCharIndex;
extern D2CharSelStrc * D2CharSelStrcFirst;
extern int TotalCurrentChars;

} // namespace D2Client::CharScreen

namespace D2Client::UI::Automap {

extern int nAutoMapCenters;
extern int nAutoMapParty;
extern uint32_t AutoMapPartyNames;
extern uint32_t MiniMapType;
extern void * pClientAutomapCellActual;
extern size_t nAutomapCellListSize;
extern D2AutomapLayerStrc * sgpAutomapLayer;
extern D2AutomapLayerStrc * sgpAutomapLayerEx;
extern DC6 * pAutoMapDC6[4];
extern int nAutoMapDC6FrameCount[4];
extern int AutoMapMoveX;
extern int AutoMapMoveY;
extern int nAutoMapFade;
extern BOOL bAutoMapFade;
extern int AUTOMAP_TextHeightOffset;
extern int gnAutomapClipLeft;
extern int gnAutomapClipTop;
extern int gnAutomapClipRight;
extern int gnAutomapClipBottom;
extern int nAutomapCenterY;
extern int gnAutomapCenterX;
extern int AutoMapLeft;
extern uint32_t AutoMapLeft2;
extern uint32_t AutoMapLeft3;

} // namespace D2Client::UI::Automap

namespace D2Game::GAME::Rendering {

extern int CurrentRendingEngine;

} // namespace D2Game::GAME::Rendering

namespace D2Client::UI::Help {

extern BOOL bSeenHelpMenu;
extern DC6 * pHelpBorder;
extern DC6 * HelpWhiteBullet;
extern DC6 * HelpYellowBullet;
extern int32_t gbMiniPanel;

} // namespace D2Client::UI::Help

namespace D2Client::UI::npcmenu {

extern short nMercNameId;

} // namespace D2Client::UI::npcmenu

namespace D2Client::NPC {

extern int D2NpcMenuOptionsCount;
extern struct D2NpcMenuOptions D2NpcMenuOptions[48];

} // namespace D2Client::NPC

namespace D2Client::UNIT::UnitSnd {

extern eD2Sounds * gaUnitSoundTableByClassIdExtra[573];

} // namespace D2Client::UNIT::UnitSnd

namespace D2Client::UI::ui {

extern uint16_t CharacterLookupTable[256];
extern DC6 * gpDC6CursorIcon;

} // namespace D2Client::UI::ui

namespace D2Game::Quests::QUESTS::Quest40 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::Quest40

namespace D2Game::Quests::QUESTS::A5Q1 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A5Q1

namespace D2Game::Quests::QUESTS::A5Q2 {

extern D2NPCMessageTableStrc aNpcGossipData[12];

} // namespace D2Game::Quests::QUESTS::A5Q2

namespace D2Game::Quests::QUESTS::A5Q3 {

extern D2NPCMessageTableStrc aNpcGossipData[10];

} // namespace D2Game::Quests::QUESTS::A5Q3

namespace D2Game::Quests::QUESTS::A5Q4 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A5Q4

namespace D2Game::Quests::QUESTS::A5Q5 {

extern D2NPCMessageTableStrc aNpcGossipData[9];

} // namespace D2Game::Quests::QUESTS::A5Q5

namespace D2Game::Quests::QUESTS::A5Q6 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A5Q6

namespace D2Game::Quests::QUESTS::Quest37 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::Quest37

namespace D2Game::Quests::QUESTS::A1Q0 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A1Q0

namespace D2Game::Quests::QUESTS::A1Q1 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A1Q1

namespace D2Game::Quests::QUESTS::A1Q2 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A1Q2

namespace D2Game::Quests::QUESTS::A1Q3 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A1Q3

namespace D2Game::Quests::QUESTS::A1Q4 {

extern D2NPCMessageTableStrc aNpcGossipData[11];

} // namespace D2Game::Quests::QUESTS::A1Q4

namespace D2Game::Quests::QUESTS::A1Q5 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A1Q5

namespace D2Game::Quests::QUESTS::A1Q6 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A1Q6

namespace D2Game::Quests::QUESTS::A1Q7 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A1Q7

namespace D2Game::Quests::QUESTS::Quest38 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::Quest38

namespace D2Game::Quests::QUESTS::A2Q0 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A2Q0

namespace D2Game::Quests::QUESTS::A2Q1 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A2Q1

namespace D2Game::Quests::QUESTS::A2Q2 {

extern D2NPCMessageTableStrc aNpcGossipData[11];

} // namespace D2Game::Quests::QUESTS::A2Q2

namespace D2Game::Quests::QUESTS::A2Q3 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A2Q3

namespace D2Game::Quests::QUESTS::A2Q4 {

extern D2NPCMessageTableStrc aNpcGossipData[12];

} // namespace D2Game::Quests::QUESTS::A2Q4

namespace D2Game::Quests::QUESTS::A2Q5 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A2Q5

namespace D2Game::Quests::QUESTS::A2Q6 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A2Q6

namespace D2Game::Quests::QUESTS::A2Q7 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A2Q7

namespace D2Game::Quests::QUESTS::A2Q8 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A2Q8

namespace D2Common::Monsters::MONSTERS::Umod {

extern fpUmodInit aUmodInitTable[43];

} // namespace D2Common::Monsters::MONSTERS::Umod

namespace D2Game::Quests::QUESTS::A4Q0 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A4Q0

namespace D2Game::Quests::QUESTS::A4Q1 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A4Q1

namespace D2Game::Quests::QUESTS::A4Q2 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A4Q2

namespace D2Game::Quests::QUESTS::A4Q3 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A4Q3

namespace D2Game::Quests::QUESTS::A4Q4 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A4Q4

namespace D2Game::Quests::QUESTS::Quest39 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::Quest39

namespace D2Game::Quests::QUESTS::A3Q0 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A3Q0

namespace D2Game::Quests::QUESTS::A3Q1 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A3Q1

namespace D2Game::Quests::QUESTS::A3Q2 {

extern D2NPCMessageTableStrc aNpcGossipData[13];

} // namespace D2Game::Quests::QUESTS::A3Q2

namespace D2Game::Quests::QUESTS::A3Q3 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A3Q3

namespace D2Game::Quests::QUESTS::A3Q4 {

extern D2NPCMessageTableStrc aNpcGossipData[9];

} // namespace D2Game::Quests::QUESTS::A3Q4

namespace D2Game::Quests::QUESTS::A3Q5 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A3Q5

namespace D2Game::Quests::QUESTS::A3Q6 {

extern D2NPCMessageTableStrc aNpcGossipData;

} // namespace D2Game::Quests::QUESTS::A3Q6

namespace D2Client::Waypoint {

extern int32_t nWaypointsBitMasksSize;
extern uint16_t aaWaypointsBitMasks[112][2];
extern int32_t nWaypointSignature;
extern D2WaypointStrc * pWaypoint;
extern bool bNeedAllocation;

} // namespace D2Client::Waypoint

namespace D2Common::Drlg::DrlgGrid {

extern fpFlagOperations apFlagOperations[6];

} // namespace D2Common::Drlg::DrlgGrid

namespace Fog::Memory {

extern D2MemoryManagerStrc pGlobalPoolSystem;

} // namespace Fog::Memory

namespace Fog::Debug {

extern HMODULE pDbgHelpOrImageHlp;
extern pfnSymCleanup pSymCleanup;
extern pfnSymFunctionTableAccess pSymFunctionTableAccess;
extern pfnSymGetLineFromAddr pSymGetLineFromAddr;
extern pfnSymGetModuleBase pSymGetModuleBase;
extern pfnSymGetModuleInfo pSymGetModuleInfo;
extern pfnSymGetOptions pSymGetOptions;
extern pfnSymGetSymFromAddr pSymGetSymFromAddr;
extern pfnSymInitialize pSymInitialize;
extern pfnSymLoadModule pSymLoadModule;
extern pfnSymSetOptions pSymSetOptions;
extern pfnStackWalk pStackWalk;
extern BOOL bBetterStackCrawls;

} // namespace Fog::Debug

namespace D2Client::MCP {

extern uint16_t D2GSRequestId;
extern uint32_t D2GSGameToken;
extern D2QSSocketStrc * MCPD2QSSocket;
extern uint32_t D2GSJoinResult;
extern char D2GSConnectIp[18];
extern void * CharListPacket;

} // namespace D2Client::MCP

namespace D2Client::Bnet {

extern uint32_t D2GSGameHash;

} // namespace D2Client::Bnet

namespace D2Game::Save {

// lpBuffer parameter of GetUserNameA
// param_1 parameter of wsprintfA
extern char szSaveCharacterName[16];
extern uint8_t * pRawSaveBuffer;
extern uint32_t nRawSaveBufferSize;

} // namespace D2Game::Save

namespace D2Game::GameData {

extern uint32_t gnAlwaysRun;

} // namespace D2Game::GameData

namespace D2Client::GAME::Record {

extern RecordCleanupFn CleanUpFn;
extern uint32_t NotRecording;
extern BOOL SettedUp;
// @data gbIsRecording
// @wasnamed IsRecording
// @evidence BOOL flag read by D2Game::Game::Record::IsRecording @00451980 (which is the
// only reader) and written by LoadGameRecordFromFile, StopRecordingSetFlags and SetupRecording. It had
// the SAME fully-qualified name as the accessor function, which the emitter cannot express.
extern BOOL gbIsRecording;
// @data gbIsPlaybackActive
// @wasnamed IsPlaybackActive
// @evidence BOOL flag read by D2Game::Game::Record::IsPlaybackActive @00451990 and
// written by RecordCurrentStep, SetUnkownOn1 and LoadGameRecordFromFile. Same name as its accessor.
extern BOOL gbIsPlaybackActive;
extern WndProcB pWndProcB;

} // namespace D2Client::GAME::Record

namespace D2Client::UI::Dialog {

extern D2DialogStrc * gpDialog;
extern D2DialogStrc * DialogHandle;
extern D2DialogStrc * gpNpcTransactionConfirmDialog;
extern D2DialogStrc * gpNpcTradeErrorDialog;
extern D2DialogStrc * gpNpcTransactionWaitDialog;
extern D2DialogStrc * gpNpcInteractionDialog;
extern D2DialogStrc * gpNpcGossipDialog;
extern D2DialogStrc * gpNpcActionConfirmDialog;
extern D2DialogStrc * gpNpcMercListDialog;
extern D2DialogStrc * gpNpcMainMenuDialog;
extern int32_t gnDialogCounter;
extern D2DialogStrc * gpDialogActual;
extern uint32_t gnDialogCounter_b;
extern D2DialogStrc * gpTradeAgreeDialog;
extern D2DialogStrc * gpTradeCancelDialog;
extern D2DialogStrc * gpHoradricCubeDialog;

} // namespace D2Client::UI::Dialog

namespace D2Client::_Selection {

extern int nSelectedUnitGUID;
extern eD2UnitType eSelectedUnitType;
extern BOOL bHasSelectedUnit;
extern BOOL bCanSelectUnit;

} // namespace D2Client::_Selection

namespace D2Client::Cursor {

extern DC6 * pCursorsDC6[7];
extern int nCursorDataIndex;

} // namespace D2Client::Cursor

namespace D2Client::Draw::Weather {

extern int gnWeatherRngRangeMax;
extern int gnWeatherRngRangeMaxAct5;
extern int gnWeatherOffsetXModulo;
extern int gnWeatherParticleOffsetYModulo;
extern int gnWeatherRngRangeMin;
extern int gnWeatherRngRangeMinAct5;
extern D2DataArrayStrc * pEnvironmentSplashes;
extern D2DataArrayStrc * pEnvironmentBubbles;
extern D2DataArrayStrc * pEnvironmentParticles;

} // namespace D2Client::Draw::Weather

namespace D2Client::Draw::LightMap {

extern D2LightMapStrc * gpLightMap;

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::View {

extern uint32_t SHAKE_StartTickcount;
extern uint32_t SHAKE_Arg3;
extern uint32_t SHAKE_Arg1;
extern uint32_t SHAKE_Arg2;
extern uint32_t SHAKE_OFFSET_Y;
extern int gnSoundShakeVolume;
extern uint32_t SHAKE_Amplitude;
extern uint32_t SHAKE_OFFSET_X;
extern uint8_t * * gpViewPalPointerTable32x8;
extern uint8_t * gpViewPalByteTable32x16;
extern uint8_t * gpViewPalByteTable16x32;
extern uint8_t * * gpViewPalPointerTable32x32;
extern uint8_t * gpViewPalByteTable32x40;

} // namespace D2Client::GAME::View

namespace D2Client::GAME::RosterPets {

extern D2RosterPetStrc * pRosterPet;

} // namespace D2Client::GAME::RosterPets

namespace D2Client::GAME::Roster {

extern D2RosterStrc * gpRosterPlayers;
extern D2RosterStrc * gpRosterOther;
extern D2RosterPartyListStrc * pRosterPartyList;

} // namespace D2Client::GAME::Roster

namespace D2Client::UI::Inv {

extern D2GridScreenCoordinates pTradePage2GridScreenCoordinates;
extern D2GridScreenCoordinates pBigBankPageGridScreenCoordinates;
// One 5-byte palette of inventory/automap highlight colours, indexed as an array. Split across three globals and two function-local statics it could never be contiguous.
extern uint8_t gaInvHighlightColors[5];
extern D2GridScreenCoordinates pTradePage1GridScreenCoordinates;
extern D2GridScreenCoordinates pBankPageGridScreenCoordinates;
extern D2GridScreenCoordinates pNpcInventoryGridScreenCoordinates;
extern D2GridScreenCoordinates pTransmogrifyBoxGridScreenCoordinates;
extern D2GridScreenCoordinates pInventoryGridScreenCoordinated;
extern D2GridScreenCoordinates pHirelingGridScreenCoordinates;

} // namespace D2Client::UI::Inv

namespace D2Client::UI::Hireables {

extern DC6 * ClassIcons[7];
extern uint8_t HealthBarGreen;
extern uint8_t HealthBarYellowish;
extern uint8_t HealthBarRed;
extern bool HealthBarColorsInitialized;

} // namespace D2Client::UI::Hireables

namespace D2Client::UI::Panel {

extern DC6 * runbutton;
extern DC6 * ctrlpnl_popbelt;
extern DC6 * GemSocket;
extern DC6 * _00BorderFrame;
extern DC6 * CtrlPnl7;
extern DC6 * skillpoints;
extern DC6 * Invwarn;
extern DC6 * hostilepic;
extern DC6 * menupanel;
extern DC6 * DuroIcon;
extern DC6 * InvChar;
extern DC6 * Hlthmana;
extern DC6 * menubutton;
extern DC6 * _00CtrlPnl7;
extern DC6 * Arrow;
extern DC6 * btn;
extern DC6 * overlap;
extern BOOL bShowHpText;
extern BOOL bShowmpText;

} // namespace D2Client::UI::Panel

namespace D2Client::UI::Party {

extern D2UiPartyStrc * pUiParty;
extern DC6 * pMenu_party;
extern DC6 * pMenu_partybuttons;
extern DC6 * pMenu_partyboxes;
extern DC6 * pPanel_inv_goldbtn;
extern DC6 * pMenu_partybar;
extern uint8_t cRed;

} // namespace D2Client::UI::Party

namespace D2Client::_Interaction {

extern eD2NpcUiState geNpcUiState;
extern int nCurrentInteractedNpcGUID;
extern BOOL bInteractingWithNpc;
extern int nCurrentInteractedNpcClassId;

} // namespace D2Client::_Interaction

namespace D2Client::Merc {

extern struct D2MercForHire D2MercForHire[10];
extern int nMercReviveCost;

} // namespace D2Client::Merc

namespace D2Client::Mouse {

extern int nPosX;
extern int nPosY;

} // namespace D2Client::Mouse

namespace D2Client::Forms {

extern D2ControlStrc * pFormsList;
extern D2ControlStrc * pFormUnknown;
extern D2ControlStrc * pFormSecond;
extern D2WinEditBox * pFormTextAndEditBox;
extern D2ControlStrc * pFormFourth;
extern D2ControlStrc * pFormImage;

} // namespace D2Client::Forms

namespace D2Client::_CdKey {

extern char * KeyClassic;
extern char * KeyExpansion;
extern char * KeyOwner;

} // namespace D2Client::_CdKey

namespace D2Direct3D::Renderer {

extern unsigned int gnD3DTextureCapFlags;
extern unsigned int g_bCutscenePlaying;

} // namespace D2Direct3D::Renderer

// =============================================================================
// Declaration closure
//
// Symbols the emitted bodies reference that no emitted file declares: callees
// in namespaces this build excludes, data symbols the globals filters dropped,
// and Ghidra's own names for data it never typed. Where the content behind one
// is known, globals.cpp defines it; where it is not, the symbol stays undefined
// and its absence is a link question, not a compile one.
// =============================================================================

// Ghidra names for untyped data (the address is in the name)
extern char s_207_82_87_133_0070f130[];
extern char s_AppID_006fad8c[];
extern char s_Assertion_Failure_006ccb6c[];
extern char s_Available_command_line_options____006d5b20[];
extern char s_BLIZZARDKEY_006cef6c[];
extern char s_BNNews_D2DV_006df210[];
extern char s_Blizzard_Entertainment__Inc__006deb80[];
extern char s_CLSID_006fad84[];
extern char s_CLen_006e163c[];
extern char s_Cold_006e164c[];
extern char s_Component_Categories_006fad6c[];
extern char s_D2X_INTRO__s_BIK_006d913c[];
extern char s_D2_LNG_006d5ee0[];
extern char s_DIABLO2SRV_007065dc[];
extern char s_Deadlock_006ccb34[];
extern char s_Diablo_2_EXE_006cc838[];
extern char s_Diablo_II_006cc8b8[];
extern char s_Direct3D_Enumeration_0074c058[];
extern char s_Do_you_wish_to_terminate_the_appl_006cf1f4[];
extern char s_ERROR___u__0x_08x__006cf2e0[];
extern char s_Enumeration_Class_0074c044[];
extern char s_Error_1__Diablo_II_is_unable_to_p_0072daa8[];
extern char s_Error_1__Diablo_II_is_unable_to_p_0072daf0[];
extern char s_FLen_006e1634[];
extern char s_FileType_006fad60[];
extern char s_File_Read_Error_006ccb94[];
extern char s_File_Write_Error_006ccb80[];
extern char s_File___s_Line___d__006cf284[];
extern char s_Fire_006e165c[];
extern char s_Font16_006dc970[];
extern char s_Font24_006dc930[];
extern char s_Font30_006dc968[];
extern char s_Font42_006dc960[];
extern char s_Font6_006dc938[];
extern char s_Font8_006dc978[];
extern char s_FontExocet10_006dc910[];
extern char s_FontExocet8_006dc8f4[];
extern char s_FontFormal10_006dc950[];
extern char s_FontFormal11_006dc920[];
extern char s_FontFormal12_006dc940[];
extern char s_FontInGameChat_006dc8d0[];
extern char s_FontRidiculous_006dc900[];
extern char s_Function___s__006cf274[];
extern char s_Gaunt_006d6b80[];
extern char s_Halt_006ccb64[];
extern char s_Handle___s__006cf25c[];
extern char s_Hardware_006fad48[];
extern char s_Interface_006fad54[];
extern char s_Mana_006e1614[];
extern char s_Mime_006fad40[];
extern char s_Missing_Dll_006ccb50[];
extern char s_No_Error_006ccbc8[];
extern char s_Object___s__006cf268[];
extern char s_Out_of_Memory_Error_006ccba4[];
extern char s_PKWARE_Data_Compression_Library_f_0074be60[];
extern char s_PLen_006e162c[];
extern char s_Press_OK_to_terminate_the_applica_006cf220[];
extern char s_Program___s__006cf298[];
extern char s_ReallyTheLastSucker_006dc8e0[];
extern char s_Registry_Error_006ccb40[];
extern char s_SECURITY_006fad30[];
extern char s_SYSTEM_006fad28[];
extern char s_Software_006d71ec[];
extern char s_Stupid_Lng_cpp_error__006d5ea8[];
extern char s_The_syntax_of_the_command_is_inco_006fcbe4[];
extern char s_This_application_has_encountered__006cf2a8[];
extern char s_TypeLib_006fad20[];
extern char s_Unable_to_start_LNG_manager__006d5ec0[];
extern char s_Unknown_Error_006ccbb8[];
extern char s_Warning_006ccb5c[];
extern char s___H__006ed5a4[];
extern char s___N__006ed59c[];
extern char s__not_xlated_call_ken_w_00730520[];
extern char s__s_palette_act_d__s_006dc7d0[];
extern char s_block_006e1730[];
extern char s_blocks_006e1738[];
extern char s_buffer_error_006d37e0[];
extern char s_buysell_006d6b50[];
extern char s_client_006cc920[];
extern char s_cmncof_a1_d2_006d6c44[];
extern char s_cmncof_a2_d2_006d6c54[];
extern char s_cmncof_a3_d2_006d6c64[];
extern char s_cmncof_a4_d2_006d6c74[];
extern char s_cmncof_a6_d2_006d6c84[];
extern char s_d2_promo_bik_006d91ac[];
extern char s_d2data_mpq_006dc770[];
extern char s_d2exp_mpq_006cd3a0[];
extern char s_data_error_006d3804[];
extern char s_data_global_music_act1_caves_wav_006dea30[];
extern char s_data_global_music_act1_crypt_wav_006de9e4[];
extern char s_data_global_music_act1_monastery__006dea08[];
extern char s_data_global_music_act2_desert_wav_006de8c4[];
extern char s_data_global_music_act2_harem_wav_006de9c0[];
extern char s_data_global_music_act2_sewer_wav_006de8a0[];
extern char s_data_global_music_act2_tombs_wav_006de99c[];
extern char s_data_global_music_act3_kurast_wav_006de87c[];
extern char s_data_global_music_act3_kurastsewe_006de950[];
extern char s_data_global_music_act3_spider_wav_006de978[];
extern char s_data_global_music_act4_diablo_wav_006de858[];
extern char s_data_global_music_act5_icecaves_w_006de90c[];
extern char s_data_global_music_act5_xtemple_wa_006de8e8[];
extern char s_data_global_music_common_options__006dea54[];
extern char s_data_global_music_introedit_wav_006de930[];
extern char s_data_local_font__s_default_map_006e02d8[];
extern char s_data_local_lng__s_expansionstring_006e028c[];
extern char s_data_local_lng__s_patchstring_tbl_006e02b4[];
extern char s_data_local_lng__s_string_tbl_006e02f8[];
extern char s_dialogbackground_00727498[];
extern char s_expand_006cc8f8[];
extern char s_file_error_006d3820[];
extern char s_grasp_006d6b78[];
extern char s_incompatible_version_006d37c8[];
extern char s_insufficient_memory_006d37f0[];
extern char s_launcher_006cc900[];
extern char s_mode_006d5bc4[];
extern char s_modstate0_006cc928[];
extern char s_multiplayer_006cc90c[];
extern char s_need_dictionary_006d3838[];
extern char s_ohand_006d6b70[];
extern char s_orotate_006d6b68[];
extern char s_pal_dat_006dc7c8[];
extern char s_pal_pl2_006dc7c0[];
extern char s_palette_act1_pal_dat_006dc7fc[];
extern char s_palette_act1_pal_pl2_006dc7e4[];
extern char s_palette_fechar_pal_dat_006d39e8[];
extern char s_palette_fechar_pal_pl2_006d39d0[];
extern char s_palette_sky_pal_dat_006d39bc[];
extern char s_palette_sky_pal_pl2_006d39a8[];
extern char s_patch_d2_mpq_006dc728[];
extern char s_ppress_006d6b60[];
extern char s_profile_description_006d4eb8[];
extern char s_profile_location_006d4ecc[];
extern char s_profile_sex_006d4ee0[];
extern char s_protate_006d6b58[];
extern char s_screen01_006d5d10[];
extern char s_screen02_006d5d04[];
extern char s_screen03_006d5cf8[];
extern char s_screen04_006d5cec[];
extern char s_screen05_006d5ce0[];
extern char s_screen06_006d5cd4[];
extern char s_screen07_006d5cc8[];
extern char s_screen08_006d5cbc[];
extern char s_screen09_006d5cb0[];
extern char s_screen10_006d5ca4[];
extern char s_server_006cc918[];
extern char s_stream_end_006d382c[];
extern char s_stream_error_006d3810[];
