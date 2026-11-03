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
#include "D2Client/Sound/Sound.h"
#include "D2Client/UI/Automap.h"
#include "D2Client/UI/Chat.h"
#include "D2Client/UI/CmdTbl.h"
#include "D2Client/UI/EscMenu.h"
#include "D2Client/UI/Inv.h"
#include "D2Client/UI/Panel.h"
#include "D2Client/UI/Path.h"
#include "D2Client/UI/QuestLog.h"
#include "D2Client/UI/ShowItems.h"
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
#include "D2Common/Merc.h"
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
#include "Storm/Source/SComp.h"
#include "Storm/Source/SErr.h"
#include "Storm/Source/SFile.h"
#include "Storm/Storm.h"
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
struct D2CubeMainTxt;
struct D2CursorDataStrc;
struct D2D3DDeviceInfoStrc;
struct D2D3DDeviceSlotStrc;
struct D2D3DVertexStrc;
struct D2DamageStrc;
struct D2DataTableTxtStrc;
struct D2DialogStrc;
struct D2DifficultyLevelsTxt;
struct D2DrawFloorStrc;
struct D2DrlgActEnvironmentData;
struct D2DrlgActStrc;
struct D2DrlgFileCacheNodeStrc;
struct D2DrlgFileStrc;
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
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

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
struct D2RendererFunctionsStrc;
struct D2ResistTableEntry;
struct D2RoomExStrc;
struct D2RoomStrc;
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
struct D2SkillChargeDescStrc;
struct D2SkillDescTxt;
struct D2SkillStrc;
struct D2SkillsHeaderTxt;
struct D2SkillsPerClassCounter;
struct D2SkillsTxt;
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
struct D2SrpCredentialsStrc;
struct D2SrpSaltVerifierStrc;
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
struct IDirect3DTexture2;
struct IDirectDrawSurface1;
struct IDirectDrawSurface7;
struct IDirectSound3DBuffer;
struct IDirectSound;
struct IDirectSoundBuffer;
struct IKsPropertySet;
struct IgnoreList;
struct ListBoxWrapper;
struct SCompCodecEntryStrc;
struct SCompEncodeCodecEntryStrc;
struct SErrAppFatalContextStrc;
struct SGAMEDATA;
struct SMemHeapStatsStrc;
struct ScreenMessage_t;
struct SpinnerWrapper;
struct StormLinkedListStrc;
struct WardenClient;
struct WardenClientVtable;
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
typedef BOOL (__stdcall *pfnSymGetLineFromAddr64)(HANDLE hProcess, DWORD qwAddrLow, DWORD qwAddrHigh, DWORD * pdwDisplacement, void * pLine64);
typedef BOOL (__stdcall *pfnSymGetModuleInfo64)(HANDLE hProcess, DWORD qwAddrLow, DWORD qwAddrHigh, void * pModuleInfo);
typedef BOOL (__stdcall *pfnSymGetSymFromAddr64)(HANDLE hProcess, DWORD AddressLow, DWORD AddressHigh, void * pDisplacement, void * pSymbol);
typedef BOOL (__stdcall *pfnSymInitialize)(HANDLE hProcess, char * UserSearchPath, BOOL fInvadeProcess);
typedef BOOL (__stdcall *pfnSymSetSearchPath)(HANDLE hProcess, char * SearchPath);
typedef BOOL (__stdcall *pfnSymUnloadModule)(HANDLE hProcess, DWORD BaseOfDll);
typedef BOOL (__stdcall *pfnSymUnloadModule64)(HANDLE hProcess, DWORD BaseOfDllLow, DWORD BaseOfDllHigh);
typedef D2FontGlyphStrc * (__fastcall *pfnFontGetGlyphEntry)(uint16_t wCharCode);
typedef D2RoomExStrc * (__fastcall *fpDrlgMazeFindEdgeRoom)(D2DrlgLevelStrc * pLevel);
typedef D2UnitStrc * (__fastcall *D2InitUnitObject)(::SGAMEDATA * pGame, D2RoomStrc * pRoom, int32_t nMonStatsId, int32_t nPosX, int32_t nPosY, BOOL bUseMonStats);
typedef D2UnitStrc * (__fastcall *D2ObjectPopulateFnFunc)(::SGAMEDATA * pGame, D2RoomStrc * pRoom, uint8_t nDensity, int nObjectClassId, uint16_t nChancePercent);
typedef DWORD (__stdcall *fnGetUnknownFlag)();
typedef DWORD (__stdcall *fnSendKeepAlive)();
typedef DWORD (__stdcall *pfnSymGetOptions)();
typedef DWORD (__stdcall *pfnSymSetOptions)(DWORD SymOptions);
typedef DWORD (__stdcall *pfnUnDecorateSymbolName)(char * DecoratedName, char * UnDecoratedName, DWORD UndecoratedLength, DWORD Flags);
typedef FxBool (__stdcall *grLfbLock)(GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t * info);
typedef FxBool (__stdcall *grLfbUnlock)(GrLock_t type, GrBuffer_t buffer);
typedef FxU32 (__stdcall *grTexMaxAddress)(GrChipID_t tmu);
typedef FxU32 (__stdcall *grTexTextureMemRequired)(FxU32 evenOdd, GrTexInfo * info);
typedef FxU32 (__stdcall *pfnGlideGammaCorrectionRGB)(FxFloat red, FxFloat green, FxFloat blue);
typedef GrContext_t (__stdcall *grSstWinOpen)(FxU32 hWnd, GrScreenResolution_t screen_resolution, GrScreenRefresh_t refresh_rate, GrColorFormat_t color_format, GrOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers);
typedef GrProc (__stdcall *grGetProcAddress)(char * procName);
typedef bool (__fastcall *fnObjectClientFn)(D2UnitStrc * pUnit);
typedef bool (__fastcall *pfnAiMapDirectionHandler)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, void * pAiParam, uint32_t nSeed);
typedef char (__stdcall *fnGetProtocolVersionChar)();
typedef char * (__stdcall *grGetString)(FxU32 pname);
typedef eD2ClientAppMode (__stdcall *fnClientAppModeProc)();
typedef eD2ServerIncomingStatus (__fastcall *D2NET_SERVER_CLTtoD2GS)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, char * pBytes, uint32_t nSize);
typedef int (*fnLogIndent)(int nAction, int nParam);
typedef int (__fastcall *D2MissileSrvDoFunc)(::SGAMEDATA * pGame, D2UnitStrc * pMissile);
typedef int (__fastcall *D2PathTypeFunc)(D2PathTypeArg * pPathTypeArg);
typedef int (__fastcall *SkillSelectProcFn)(D2UnitStrc * pUnit, D2UnitStrc * pTargetUnit);
typedef int (__fastcall *fnApplyPrepatch)(int * pdw);
typedef int (__fastcall *fnCacheFreeMem)(void * pMem);
typedef int (__fastcall *fnConnectToBattleNet)();
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
typedef uint64_t (__stdcall *pfnSymGetModuleBase64)(HANDLE hProcess, DWORD qwAddrLow, DWORD qwAddrHigh);
typedef uint64_t (__stdcall *pfnSymLoadModule64)(HANDLE hProcess, HANDLE hFile, char * ImageName, char * ModuleName, DWORD BaseOfDllLow, DWORD BaseOfDllHigh, DWORD SizeOfDll);
typedef void (*fnLogWriteLine)(char * szFmt, ...);
typedef void (*pfnUIConfigTabAction)();
typedef void (__fastcall *AI_Init)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2AiParamStrc * pAiParam);
typedef void (__fastcall *AI_Main)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2AiParamStrc * pAiParam);
typedef void (__fastcall *AI_Secondary)(::SGAMEDATA * pGame, D2UnitStrc * pUnit, D2AiParamStrc * pAiParam);
typedef void (__fastcall *D2MissileCltDoFunc)(D2UnitStrc * pMissile);
typedef void (__fastcall *D2MissileSrvDmgFunc)(::SGAMEDATA * pGame, D2UnitStrc * pMissile, D2UnitStrc * pTarget, D2DamageStrc * pDamage);
typedef void (__fastcall *D2NET_CLIENT_D2GStoCLT)(char * pBytes);
typedef void (__fastcall *D2NET_CLIENT_D2GStoCLT_Unit)(D2UnitStrc * pUnit, char * pBytes);
typedef void (__fastcall *D2ObjectsInitFnFunc)(D2ObjectInitFnArg * pInitArg);
typedef void (__fastcall *D2SkillDescAttackFunc)(D2UnitStrc * pUnit, D2SkillStrc * pSkill, D2SkillsTxt * pSkillsTxt, int32_t nSkillLevel, int32_t * pnMinDamage, int32_t * pnMinColor, int32_t * pnMaxDamage, int32_t * pnMaxColor);
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
typedef void (__fastcall *pfnMonUniqueModInit)(D2UnitStrc * pUnit, int32_t eModifier, int32_t bApply);
typedef void (__fastcall *pfnNewsEntryCallback)(uint32_t dwEntryId, char * szMessage);
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
typedef void (__stdcall *grAlphaCombine)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert);
typedef void (__stdcall *grBufferClear)(GrColor_t color, GrAlpha_t alpha, FxU32 depth);
typedef void (__stdcall *grBufferSwap)(FxU32 swap_interval);
typedef void (__stdcall *grChromakeyMode)(GrChromakeyMode_t mode);
typedef void (__stdcall *grChromakeyValue)(GrColor_t value);
typedef void (__stdcall *grClipWindow)(FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy);
typedef void (__stdcall *grColorCombine)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert);
typedef void (__stdcall *grColorMask)(FxBool rgb, FxBool a);
typedef void (__stdcall *grCoordinateSpace)(GrCoordinateSpaceMode_t mode);
typedef void (__stdcall *grDepthBufferFunction)(GrCmpFnc_t function);
typedef void (__stdcall *grDepthBufferMode)(GrDepthBufferMode_t mode);
typedef void (__stdcall *grDepthMask)(FxBool mask);
typedef void (__stdcall *grDisable)(GrEnableMode_t mode);
typedef void (__stdcall *grDitherMode)(GrDitherMode_t mode);
typedef void (__stdcall *grDrawLine)(void * v1, void * v2);
typedef void (__stdcall *grDrawPoint)(void * pt);
typedef void (__stdcall *grDrawTriangle)(void * a, void * b, void * c);
typedef void (__stdcall *grEnable)(GrEnableMode_t mode);
typedef void (__stdcall *grGlideShutdown)();
typedef void (__stdcall *grLoadGammaTable)(FxU32 nentries, FxU32 * red, FxU32 * green, FxU32 * blue);
typedef void (__stdcall *grSstSelect)(int which_sst);
typedef void (__stdcall *grTexDownloadMipMap)(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo * info);
typedef void (__stdcall *grTexDownloadTable)(GrTexTable_t type, void * data);
typedef void (__stdcall *grTexMipMapMode)(GrChipID_t tmu, GrMipMapMode_t mode, FxBool lodBlend);
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
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

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

struct D2DnsThreadParamStrc {
    /* 0x00 */ char * pszOut;
    /* 0x04 */ char * pszHost;
};

struct D2EndGameUiState {
    /* 0x00 */ BOOL showCongrats; // Show end-game congrats flow
    /* 0x04 */ BOOL showCutscene; // End-game cutscene/congrats active flag
    /* 0x08 */ BOOL congratzOkClicked; // Congrats OK clicked
};

struct D2ExitCutsceneState {
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

    /* 0x00 */ int pendingCutsceneId; // Pending exit cutscene id
    /* 0x04 */ BOOL cutscenePlayed; // Exit cutscene played/active flag
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
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

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
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

    /* 0x00 */ int nDamageFieldOffset; // byte offset into the damage struct
    /* 0x04 */ int bSecondary;
};

struct D2MonStatAnimEntryStrc {
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

    /* 0x00 */ uint32_t dwFlags;
    /* 0x04 */ uint32_t dwValidFlags;
};

struct D2NET_SERVER_D2GS_ClientToServer {
    /* 0x00 */ eD2ServerIncomingStatus (__fastcall *fpHandler)(::SGAMEDATA *, D2UnitStrc *, char *, uint32_t);
    /* 0x04 */ BOOL field1_0x4;
};

struct D2NpcControlInitStrc {
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

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
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

    /* 0x00 */ int nX;
    /* 0x04 */ int nY;
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
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

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
    operator uint64_t() const { return *reinterpret_cast<const uint64_t*>(this); }

    /* 0x00 */ int nUniqueIdx; // uniqueitems.txt row index
    /* 0x04 */ int nCumulativeWeight; // running rarity total
};

struct __attribute__((packed)) D2UniqueCandidatePoolStrc {
    /* 0x00 */ uint32_t nGoldCodeInitFlag; // gItemDropGoldCodeInitFlag; the pick loop folds its address as the table base minus one row
    /* 0x04 */ uint32_t _pad;
    /* 0x08 */ D2UniqueCandidateStrc aCandidates[4096];
};

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

struct ZlibHuffCode {
    operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }

    /* 0x00 */ uint8_t op; // operation, extra bits, table bits
    /* 0x01 */ uint8_t bits; // bits in this part of the code
    /* 0x02 */ short val; // offset in table or code value
};

struct __attribute__((packed)) D2ZlibStaticDTreeDistCodeStrc {
    /* 0x00 */ ct_data static_dtree[30]; // zlib trees.h static_dtree[D_CODES]
    /* 0x78 */ uint8_t _dist_code[512]; // zlib trees.h _dist_code[DIST_CODE_LEN]; _tr_tally/compress_block read it at base-1+dist folded
};

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
extern uint8_t gbChatDlgUpdateMarker;
extern double gdConst0_25;
extern double gdBytesToMegabytes;
extern float gfTwoPow32;
extern double gfFloatGammaCorrectionInit;
extern double gdblConstantTen;
extern char gbSaveFileReadMarker[3];
extern char s_Cyrix_006cdeb0[6];
extern char s_AMD_006cdeb8[4];
extern char s_Intel_006cdebc[8];
// param_2 parameter of wsprintfA
extern char param_2_006ce200[5];
// Bit masks 1<<n for n=0..31 (0x1..0x80000000); 622 references, used with OR to set a bit and TEST/AND to test one (e.g. D2WINMAIN_SetControlVisible 004f96f8); ERRORMANAGER_InitErrorManager checks every entry against 1<<n at startup (00408e58).
extern uint32_t gadwBitMasks[32];
// Inverted bit masks ~(1<<n) for n=0..31 (0xfffffffe..0x7fffffff); 37 of the 38 references AND with it to clear a bit (e.g. D2WINMAIN_SetControlVisible 004f9701); the 38th is the startup self-check CMP in ERRORMANAGER_InitErrorManager (00408e63).
extern uint32_t gadwInvBitMasks[32];
extern uint8_t gbFloatConstantBase;
extern char s_File___s_006cf1e8[10];
extern char s__s_006cf248[6];
extern char s_Expr___s_006cf250[11];
extern char szOOGPasswordDialogTimeFmt;
extern char * z_errmsg[10];
extern char * gaFrontEndPaletteFiles[4];
extern char * gpszPaletteSkyDat[4];
extern char * gpszPaletteSkyPl2[4];
extern char gaLanguageCodeFallback[4];
extern char gaLanguageCodeAbbrevTable[4];
extern char gpCutsceneVideoFrameBuffer[8];
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
extern char gszCmdLineSwitchP[2];
extern char gszCmdLineSwitchC[2];
extern char gpCubeRecipeItemTypeFlags_7[4];
extern char gszCmdLineSwitchHelp[5];
extern char gszCmdLineSwitchH[2];
extern char gszCmdLineSwitchQuestionMark[2];
extern uint8_t gbExpansionInitialized;
extern char * gpszD2LngFileName;
extern char * gpszLngManagerStartError;
extern char * gpszLngCppError;
extern D2UIFlagInitStrc g_UIFlagInitDrawMouseDown[4];
// param_2 parameter of wsprintfA
extern char param_2_006d6454[3];
extern char gaFreeUnitsOrderData[3];
extern uint8_t gaMonsterDenEnvironmentData;
extern uint8_t gaWeatherTransitionConstant;
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
extern float FLOAT_006da6b8;
extern uint64_t gaIntensityTableConstant;
extern double gdConst255;
extern char * gpszPaletteAct1Dat;
extern char * gpszPaletteAct1Pl2;
extern char * gpszPaletteActPathFormat;
extern char * gpszPalDatName;
extern char * gpszPalPl2Name;
extern unsigned int gaItemTypeTxtIndexData;
extern GUID gIID_IDirectSound3DBuffer;
extern GUID gIID_IKsPropertySet;
extern uint8_t gEAXPropertyGuid_Reserved2[16];
extern char gaLanguageCodeLookup[4];
extern uint8_t gaBnSendRawBytesHeader[4];
extern char gaCDKeyAuthData[4];
extern uint8_t gaNetSidSubstitutionBox[480];
extern char * gpszStringTblPathFormat;
extern char * gpszFontMapPathFormat;
extern char * gpszPatchStringTblPathFormat;
extern char * gpszExpansionStringTblPathFormat;
extern char * gpszD2DataMpqName;
extern char * gpszPatchD2MpqName;
extern char * gpszD2ExpMpqName;
extern D2PlayerModeFnPairStrc gaPlayerModeMoveFuncs[20];
extern D2ShrinesTableEntryStrc OBJECTSHRINEFUNCTION[24];
// Indexed by monster class id - 540. MONSTER_EquipFromClassTable 005b1c7a uses LEA ESI,[ESI*8+0x6e1bb0], a base 540 rows before the table that lands inside an unrelated string; the decompiler cannot fold that bias.
extern D2MonsterEquipEntryStrc gaMonsterEquipTable[12];
extern uint32_t gaCelCmpDataBlock_006e36a0[224];
extern char gsGameMonsterSpawnContext[4];
extern char gsGameMissileTblsContext[4];
extern char gsGameDataTblsContext[4];
extern unsigned int gszTxtColName_op;
extern char s_C_006ed5b4[2];
extern char s_HDM_006ee7c0[4];
extern int gnAct5OuterWallGridOffsetByType[24];
extern int gaAct5SceneryPresetTable_0[15][7];
extern int gaAct5SceneryPresetTable_0_0__1_[15][7];
extern uint8_t gaWallNeighborOrientTable[256];
extern char s_SAM_006fad3c[4];
extern char * gaAtlRegistryRootKeys[12];
extern char s_Unable_to_open_response_file___s_006fcbc0[33];
extern char s_Invalid_argument___s_006fcc0c[21];
extern char * gaSCMD_FallbackMessages[3];
// lpMultiByteStr parameter of MultiByteToWideChar
extern char lpMultiByteStr_006ff97c[2];
// lpSubBlock parameter of VerQueryValueA
extern char lpSubBlock_006ff9b8[2];
// param_2 parameter of wsprintfA
extern char param_2_006ffc58[3];
// lpOutputString parameter of OutputDebugStringA
extern char lpOutputString_006ffcb0[2];
extern uint8_t gszIniConfigKeywordCharacterAutoSave[32];
extern uint8_t gszIniConfigKeywordNetworkPort[32];
extern uint8_t gszIniConfigKeywordDifficultySetting[32];
extern uint8_t gszIniConfigKeywordLanguageCode[28];
extern uint8_t gszIniConfigKeywordTextureFilter[96];
extern uint8_t gszIniConfigKeywordGammaCorrection[96];
extern uint8_t gszIniConfigKeywordVsyncOption[264];
extern uint8_t gszIniConfigKeywordExperienceDisplay[96];
extern uint8_t gszIniConfigKeywordVideoDriver[336];
// lpServiceStatus parameter of SetServiceStatus
extern _SERVICE_STATUS lpServiceStatus_007065fc;
extern _SERVICE_STATUS gszCommandLineLocaleCode;
extern char * gaMonthAbbreviations[12];
extern uint8_t gabHuffmanCodeLengths[256];
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
extern int32_t gaMenuFormFontParamsFont3[2];
extern int32_t gaMenuFormFontParamsFont2[2];
extern int32_t gaMenuFormFontParamsFont7[2];
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
extern eD2ClientAppMode gdwAppMode;
extern char s___AUSGAMEDATA___0070f56c[16];
extern char s___AV__TSExplicitList_USGAMEDATA__0070f584[48];
extern char s___AV__TSHashObjectChunk_USGAMEDA_0070f5f4[56];
extern uint32_t gnLastMouseButtonEventType;
extern int nAutoMapZoomDivisor;
extern uint32_t gaScreenResolutionTableIndexArray;
extern uint32_t gaScreenResolutionModeArray;
extern int ScreenSizeX;
extern int ScreenSizeY;
extern char * gaLanguageNames_00711d04[15];
extern uint32_t gbLightmapColorEnabled;
extern int32_t gpInventoryContextUi;
extern char * gaLanguageNames_00722414[15];
extern char * gaLanguageNames_00722470[15];
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
extern char * gaLanguageNames_007273a8[15];
extern int nSoundVolumeLeftChannel;
extern int nSoundVolumeRightChannel;
extern int nCashDisplayItemCount;
extern D2UIFlagInitStrc gUIFlagMapInit[8];
// Former interior labels: gnCursorInvTypeAutoOpenB=+0x4
extern uint32_t gnCursorInvTypeAutoOpenC[5];
extern uint32_t gnCursorInvTypeAutoOpenA;
extern int gaUnitSoundIdsAmazon[11];
extern char * gaLanguageNames_0072a300[15];
extern char * gaLanguageNames_0072d8d8[15];
extern char * gaLanguageNames_0072d914[15];
extern char * gaLanguageNames_0072d950[15];
extern char * gaLanguageNames_0072d994[15];
extern char * gaLanguageNames_0072d9d0[15];
extern char * gaLanguageNames_0072da0c[15];
extern int gaGroundTileRowOffsets[15];
extern int gaGroundTileRowWidths[15];
extern int gaGroundTileSrcPointers[15];
extern uint32_t gaCompWeaponClassTokens[15];
extern int gaAmbientSoundFlags;
// Ambient sound table for area-based background audio. Array of {char* szFilename, int bPlayed} entries. Count stored in gaAmbientSoundFlags. SOUND_PlayNextAmbientSound iterates through this table, alternates with gaAmbientSoundEntryTable based on gnAmbientSoundLanguageCode flag. Reset by SOUND_ResetAmbientSoundFlags.
extern int gAmbientSoundTable[16];
extern int gaAmbientSoundEntryTable[16];
extern uint32_t gnAmbientSoundCount;
extern char * gaszLanguageNames[32];
extern uint32_t CLIENT_SocketToServer;
extern BOOL DontNoSafeGameBool;
extern uint8_t gbQuestReconnectMode;
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
extern D2NPCMessageTableStrc gbQuestA5Q2Unknown[6];
extern char * gaLanguageNames_0073c6e0[15];
extern uint8_t gaSkeletonComponentsByMasteryLvl[11][9];
extern char s___AUTCELENTRY___00741f14[16];
extern char s___AUCELLIST___00741f2c[16];
extern char s___AV__TSExplicitList_UCELLIST____00741f44[48];
extern char s___AV__TSHashObjectChunk_UCELLIST_00741fb4[52];
extern char s___AUMappingEntryNode___0074362c[24];
extern char s___AUStringEntryNode___0074364c[24];
extern char s___AV__TSExplicitList_UStringEntr_0074366c[56];
extern char s___AV__TSExplicitList_UMappingEnt_007436ac[56];
extern D2DataTableTxtStrc * sgptDataTable;
extern char * gaLanguageNames_0074430c_11[15];
extern char * gaLanguageNames_007443a8[15];
extern uint32_t gnTimeRates[3];
extern char * gaLanguageNames_007444c8[15];
extern char * gaLanguageNames_00744508[15];
extern char * gaLanguageNames_00744588[15];
extern char * gaLanguageNames_007445c4[15];
extern char * gaLanguageNames_00744600[15];
extern char * gaLanguageNames_00745648[15];
extern char * gaLanguageNames_00745684[15];
extern char * gaLanguageNames_007456c0[15];
extern char * gaLanguageNames_007456fc[15];
extern char * gaLanguageNames_00745738[15];
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
extern char * gaLanguageNames_00748a58[16];
extern char * gaLanguageNames_00748a98[15];
extern int gpsPresetObjectTable[750];
extern uint32_t DEFAULT_SECURITY_COOKIE;
extern double DOUBLE_0074adb4;
// CRT x87 dispatch: maps the FXAM condition bits (C3,C2,C1,C0 folded into 4 bits) to a byte offset into the caller's per-class jump table.
// Provenance: 00698548/006985af MOV EBX,0x74adbc; XLAT; MOVSX; ADD EBX,EDX+0x10; JMP [EBX] (code not yet in a function).
extern uint8_t gabyFxamClassOffset[16];
extern unsigned int gaCRT_FPremData_3;
extern int TokenHashMapBucketSize;
extern eD2ApplicationMode geD2ApplicationMode;
extern SERVICE_STATUS_HANDLE ghServiceCtrlHandler;
extern uint32_t gnServiceRunning;
extern uint32_t gaHuffmanFastDecodeTable[256];
extern uint8_t gabHuffmanNextTable[256];
extern void * gpEventHashBuckets;
extern D2IniConfigStrc * D2IniData_launcher;
extern eOogCurrentCharSelectionMode OogCurrentCharSelectionMode;
extern D2WinTextBox * gpWinTextBoxRealmSelect;
extern D2BNetClientCallbacksStrc * gpBNetCallbacks;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0077ae28;
extern char gszOogChatInputBuffer[512];
extern D2IniConfigStrc * D2IniData_multiplayer;
extern uint16_t LastGameRequestId;
extern void * gpChatFormList;
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
extern int gnFilteredGameListCount;
extern int nGameListDifficultyFilter;
extern uint32_t CreateGameDifflevel;
extern char gszProfileCharacterName[16];
extern int gnProfileUserHardcoreFlag;
extern int gnProfileUserExpansionFlag;
extern D2IniConfigStrc * D2IniData_client;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION gHostGameStateCs;
extern D2ClientGameDataStrc gClientGameData;
extern int ServerLoadingComplete;
extern D2DrlgActStrc * sgpDrlgAct;
extern D2Game_Line0x546 * D2GameGame0x546;
extern uint32_t gnWeatherOverlayActId;
extern D2GameViewStrc * D2GameView;
// NAME IS WRONG - this is the accelerator table, not a "loaded" flag. It was typed `bool` while all
// three of its accesses are dword: CLIENT_LoadLNG stores LoadAcceleratorsA(ghLngLibrary,0x64) here,
// FreeLanguage clears it, and MessageGameLoop passes it to TranslateAcceleratorA. Retyped HACCEL on
// the operand width.
extern HACCEL gbLanguageDllLoaded;
extern HMODULE ghLngLibrary;
// lpLibFileName parameter of LoadLibraryExA
extern char lpLibFileName_007a0710[260];
extern int gnGoldDialogTransactionType;
extern SpinnerWrapper * gpCurrentDialog;
extern EditBoxWrapper * gpDialogDefaultActionCallback;
extern ButtonWrapper * gpUiGoldTransactionDialog;
extern ButtonWrapper * gpUiGoldDialogCancelButton;
extern uint32_t gbKeyConfigModeActiveInDialog;
extern D2UIFlagStrc GFX_bShowUIFlag;
extern int gnPanelOffsetX;
extern int gnPanelOffsetY;
extern D2UIFlagDescStrc gaUIFlagDescsClient[3];
extern uint32_t gnGoldDialogTransactionMode;
extern uint16_t gU16ImmunityStringBuffer2[256];
extern int nAutoMapScrollX;
extern int nAutoMapScrollY;
extern uint32_t MouseYOffset;
extern uint32_t MouseXOffset;
extern uint32_t nScreenMode;
extern int WindowSizeY;
extern int WindowSizeX;
extern uint32_t gbCharScreenOkPressed;
extern uint32_t gbCharScreenOkButtonConfirmed;
extern uint32_t gbCharScreenCancelPressed;
extern D2UnitStrc * ClientSideUnitHashTables[6][128];
extern D2UnitStrc * ClientSideUnitHashTables_1__0_[6][128];
extern D2UnitStrc * ClientSideUnitHashTables_1__1_[6][128];
extern D2UnitStrc * ClientSideUnitHashTables_2__0_[6][128];
extern D2UnitStrc * ClientSideUnitHashTables_3__0_[6][128];
extern D2UnitStrc * ServerSideUnitHashTables[6][128];
extern D2UnitStrc * ServerSideUnitHashTables_0__1_[6][128];
extern D2UnitStrc * ServerSideUnitHashTables_1__0_[6][128];
extern D2UnitStrc * ServerSideUnitHashTables_1__1_[6][128];
extern D2UnitStrc * ServerSideUnitHashTables_2__0_[6][128];
extern D2UnitStrc * ServerSideUnitHashTables_3__0_[6][128];
extern D2UnitStrc * ServerSideUnitHashTables_4__0_[6][128];
extern D2UnitStrc * pPlayerUnit;
extern uint32_t gbSuppressItemSelection;
extern int MouseY;
extern int MouseX;
extern int ItemTypeOnHand;
// D2ClientLightPulseStrc (44 bytes, 0x007a742c..0x007a7457). CMD_InitKeyConfigUI (0046ae50) opens with memset(&gRoomLightPulse, 0, 0x2c) then writes bRoomLightPulseActive / nFadeInDuration / nFadeOutDuration / nHoldDuration / nTotalDuration / eLevelId - so the 44-byte clear and the field writes are one object.
extern D2ClientLightPulseStrc gRoomLightPulse;
extern D2MonsterCofStatusEntryStrc * g_pMonsterSpriteCofStatusTable;
extern int gnGfxDirectionCountCache;
extern int gnLightmapWorldRefreshFlag;
extern int gnLightmapBufferState;
extern D2LightMapCellStrc gaLightMapCells[2304];
extern D2SeedStrc gAncientsCloudSeed;
extern uint32_t gnLastLightmapUpdateFrame;
extern uint32_t gnEngineMode;
extern uint32_t fps;
extern int gaIMECompositionBuffer;
// wchar16[257] - the chat input line. Was typed LPCWSTR (4 bytes). Size proven three ways: CHAT_ClearInput (0047bdc0) does memset(this, 0, 0x202) = 514 bytes = 257 wide chars; CHAT_* callers pass it to WSTRING_PutWideCharIntoBuffer with a 0x100 char limit and to WideCharToMultiByte with cbMultiByte 0x100 (256 chars + terminator); INPUT_HandleChatCharInput indexes it as *(uint16_t*)(base + i*2). Next symbol gaChatHistoryBuffer is 520 bytes away, so 514 fits. Declared as 4 bytes, every keystroke in the chat box wrote past the symbol.
extern uint16_t gwszChatInputText[257];
extern uint32_t gbInvBodySlotUnderCursorExists;
extern int g_nCurrentInventoryPage;
extern D2UnitStrc * pItemToDescribe;
extern uint32_t gbInvExpansionTabVisible;
extern uint32_t gbTradeConfirmationPending;
extern uint32_t gpInvItemDescPending;
extern uint8_t gbWaypointButtonPressed;
extern uint32_t ClickingDownOnWaypointEntry;
extern uint32_t gnTextOverheadCounter;
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
extern uint32_t gnNpcMenuTransactionType;
extern D2BitBufferStrc * gpNpcMenuBitBuffer;
extern uint8_t gbNpcMenuTransactionPending;
extern uint32_t gnNpcMenuTransactionItemGUID;
extern int32_t gnDialogStructAllocCount;
extern uint32_t gnTradePlayerGoldAmount;
extern uint32_t gdwTradeThrottleTimestamp;
extern bool gbTradeResetFlag;
extern uint32_t gnTradeAcceptedFlags;
extern int32_t g_nTradeWindowState;
extern DC6 * gpMessageLogDC6;
extern uint32_t nHoradricCubeUIState;
extern int gnPlayerCashDisplay;
extern int gnNpcShopSecondActiveTab;
extern uint32_t gnNpcShopActiveTab;
extern uint32_t gnCursorItemUnitId;
extern uint32_t gnTextSystemInitFlag;
extern DC6 * gpUpgradeMenuDC6;
extern DC6 * gpHoradricBackgroundDC6;
extern int gaLevelAmbientMusicPlayedFlags[1024];
extern int gnSoundLastInitTick;
extern uint32_t gdwSoundChannelIndex;
extern uint32_t gnSoundInitCooldown;
extern unsigned int gnSoundLastPlayerAnimModeTime;
extern int geSoundLastPlayerAnimMode;
extern BOOL pAudioMixerInterface;
extern int32_t int32_t_007c8c7c;
extern uint32_t gnActiveSoundChannelCount;
extern eD2RendererMode eRendererMode;
extern uint32_t gnGfxOverlayMode;
extern int RENDERER_nWindowWidth;
extern int RENDERER_nWindowHeight;
extern D2GfxHelperStrc * D2GfxRendereInitFunctionsStrc;
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
extern D2FogArchiveStrc * MPQ_d2Xmusic;
extern D2FogArchiveStrc * MPQ_d2sfx;
extern D2FogArchiveStrc * MPQ_d2delta;
extern D2FogArchiveStrc * MPQ_d2char;
extern D2FogArchiveStrc * MPQ_d2video;
extern D2FogArchiveStrc * MPQ_d2Xtalk;
extern D2FogArchiveStrc * MPQ_d2data;
extern D2FogArchiveStrc * MPQ_d2kfixup_mpq;
extern D2FogArchiveStrc * MPQ_d2kfixup;
extern D2FogArchiveStrc * MPQ_d2speech;
extern D2FogArchiveStrc * MPQ_d2music;
extern D2FogArchiveStrc * MPQ_d2Xvideo;
extern D2FogArchiveStrc * MPQ_patch_d2;
extern D2FogArchiveStrc * MPQ_d2exp;
extern uint32_t gbCompactLineSpacingMode;
extern D2TooltipTextStrc DisplayedTooltip;
extern int nD2CompItemsSize;
extern void * pD2CompItems;
extern grDrawLine GLIDEDLL__grDrawLine_8;
extern grDepthMask GLIDEDLL_grDepthMask_4;
extern grDrawPoint GLIDEDLL_grDrawPoint_4;
extern grGlideShutdown GLIDEDLL_grGlideShutdown_0;
extern grDitherMode GLIDEDLL_grDitherMode_4;
extern grTexDownloadMipMap GLIDEDLL_grTexDownloadMipMap_16;
extern pfnGlideGammaCorrectionRGB GLIDEDLL_guGammaCorrectionRGB_12;
extern grLfbUnlock GLIDEDLL_grLfbUnlock_8;
extern grLfbLock GLIDEDLL_grLfbLock_24;
extern grLoadGammaTable GLIDEDLL_grLoadGammaTable_16;
extern grAlphaCombine GLIDEDLL_grAlphaCombine_20;
extern grSstWinOpen GLIDEDLL_grSstWinOpen_28;
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
extern D2GlideVertexStrc gaGlideSmackerVertices[4];
extern uint32_t gnGlideSmackTex1StartAddr;
extern uint32_t gnGlideSmackTex2StartAddr;
extern uint8_t (*gpGlideSmackerTexBuf2)[32768];
extern uint32_t gnGlideTMUHandle;
extern IDirectDrawSurface1 * gpDDrawPrimarySurface;
extern IKsPropertySet * gpDirectSoundEAXInterface;
extern uint32_t gbHasLegacyEMU10K1Driver;
extern uint32_t gbHasLegacySoundDriverCrlds3d;
extern uint32_t LanguageCode;
extern D2SoundSmpStrc * gpMusicStreamBuffer;
extern uint32_t gnAmbientSoundLanguageCode;
extern uint32_t gbAmbientSoundLangInitialized;
extern D2BNetGatewayAccessStr ActualRealmInfo;
extern int gnBnetLogFileHandle;
extern unsigned int gbBnetLogInitialized;
extern unsigned int gnNetworkSendState;
extern char gbBnetFtpFileTransferActive[256];
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
// lpThreadId parameter of CreateThread
extern LPDWORD lpThreadId_00882b30;
extern int gnNetConnectionTimeoutMs;
extern eD2GSConnectionType nD2GSConnectionType;
extern uint32_t gnQServerListenSocket;
extern uint32_t gptItemCodeArmor;
extern uint32_t gptItemCodeSwordType;
extern int gQServerClientTablesInitialized;
extern D2TaskQueueSlotStrc * gaTaskQueueHeadArray[32];
extern D2TileProjectStrc * gpTileProjectListHead;
extern TSExportTableSync<CELLIST, HCELLIST__*, HLOCKEDCELLIST__*, CCritSect, 0> gCelCmpCacheHandler;
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_008f03cc;
extern unsigned int gpfnProfCoreInitCallback;
extern D2DataTableTxtStrc sgtDataTable;
extern D2SeedStrc gsAutoMapSeed;
extern D2LvlTypesTxt * pTxtLvlTypes;
extern int nTxtLvlTypesSize;
extern D2LvlWarpTxt * pTxtLvlWarp;
extern int nTxtLvlWarpSize;
extern D2LvlMazeTxt * sgpTxtLvlMaze;
extern int nTxtLvlMazeSize;
extern D2LvlSubTxt * sgpTxtLvlSub;
extern int nTxtLvlSubSize;
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
// lpCriticalSection parameter of DeleteCriticalSection
// lpCriticalSection parameter of InitializeCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0096ca30;
extern FILE * pLOGFILE;
extern BOOL bLOGFILE_ENABLED;
extern unsigned long long gpItemTypeTableContext;
extern D2MagicAffixTableStrc gsMagicAffixTable;
extern D2MagicAffixTableStrc pTxtMagicAffixes;
extern D2GemsTableStrc nTxtGemsSize;
extern D2RunesTxtTableStrc nTxtRunesSize;
extern D2ObjectsTxt * sgpTxtObjects;
extern int nTxtObjectsSize;
extern D2BeltsTxt * pTxtBelts;
extern D2DrlgFileCacheNodeStrc * gpDrlgFileCache;
extern D2CubeMainTxt * pTxtCubeMain;
extern int nTxtCubeMainSize;
extern unsigned int gdwDrlgLevelPlacementFlags;
extern unsigned int UINT_0096d6a8;
extern uint32_t dwCurrentThreadId;
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

namespace D2Game::GAME::Event::Dispatch {

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

namespace Storm::Source::SFile {

// struct Storm::SFile::ARCHIVEREC RTTI Type Descriptor
extern uint8_t RTTI_Type_Descriptor;

} // namespace Storm::Source::SFile

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

extern D2AutomapLayerStrc * sgpAutomapLayer;
extern D2AutomapLayerStrc * sgpAutomapLayerEx;

} // namespace D2Client::UI::Automap

namespace D2Client::NPC {

extern int D2NpcMenuOptionsCount;
extern struct D2NpcMenuOptions D2NpcMenuOptions[48];

} // namespace D2Client::NPC

namespace D2Client::UI::ui {

extern uint16_t CharacterLookupTable[256];

} // namespace D2Client::UI::ui

namespace D2Game::Quests::QUESTS::Quest40 {

extern D2NPCMessageTableStrc aNpcGossipData[4];

} // namespace D2Game::Quests::QUESTS::Quest40

namespace D2Game::Quests::QUESTS::A5Q1 {

extern D2NPCMessageTableStrc aNpcGossipData[8];

} // namespace D2Game::Quests::QUESTS::A5Q1

namespace D2Game::Quests::QUESTS::A5Q2 {

extern D2NPCMessageTableStrc aNpcGossipData[12];

} // namespace D2Game::Quests::QUESTS::A5Q2

namespace D2Game::Quests::QUESTS::A5Q3 {

extern D2NPCMessageTableStrc aNpcGossipData[10];

} // namespace D2Game::Quests::QUESTS::A5Q3

namespace D2Game::Quests::QUESTS::A5Q4 {

extern D2NPCMessageTableStrc aNpcGossipData[8];

} // namespace D2Game::Quests::QUESTS::A5Q4

namespace D2Game::Quests::QUESTS::A5Q5 {

extern D2NPCMessageTableStrc aNpcGossipData[9];

} // namespace D2Game::Quests::QUESTS::A5Q5

namespace D2Game::Quests::QUESTS::A5Q6 {

extern D2NPCMessageTableStrc aNpcGossipData[9];

} // namespace D2Game::Quests::QUESTS::A5Q6

namespace D2Game::Quests::QUESTS::Quest37 {

extern D2NPCMessageTableStrc aNpcGossipData[3];

} // namespace D2Game::Quests::QUESTS::Quest37

namespace D2Game::Quests::QUESTS::A1Q0 {

extern D2NPCMessageTableStrc aNpcGossipData[3];

} // namespace D2Game::Quests::QUESTS::A1Q0

namespace D2Game::Quests::QUESTS::A1Q1 {

extern D2NPCMessageTableStrc aNpcGossipData[8];

} // namespace D2Game::Quests::QUESTS::A1Q1

namespace D2Game::Quests::QUESTS::A1Q2 {

extern D2NPCMessageTableStrc aNpcGossipData[6];

} // namespace D2Game::Quests::QUESTS::A1Q2

namespace D2Game::Quests::QUESTS::A1Q3 {

extern D2NPCMessageTableStrc aNpcGossipData[6];

} // namespace D2Game::Quests::QUESTS::A1Q3

namespace D2Game::Quests::QUESTS::A1Q4 {

extern D2NPCMessageTableStrc aNpcGossipData[11];

} // namespace D2Game::Quests::QUESTS::A1Q4

namespace D2Game::Quests::QUESTS::A1Q5 {

extern D2NPCMessageTableStrc aNpcGossipData[5];

} // namespace D2Game::Quests::QUESTS::A1Q5

namespace D2Game::Quests::QUESTS::A1Q6 {

extern D2NPCMessageTableStrc aNpcGossipData[6];

} // namespace D2Game::Quests::QUESTS::A1Q6

namespace D2Game::Quests::QUESTS::A1Q7 {

extern D2NPCMessageTableStrc aNpcGossipData[6];

} // namespace D2Game::Quests::QUESTS::A1Q7

namespace D2Game::Quests::QUESTS::Quest38 {

extern D2NPCMessageTableStrc aNpcGossipData[3];

} // namespace D2Game::Quests::QUESTS::Quest38

namespace D2Game::Quests::QUESTS::A2Q0 {

extern D2NPCMessageTableStrc aNpcGossipData[3];

} // namespace D2Game::Quests::QUESTS::A2Q0

namespace D2Game::Quests::QUESTS::A2Q1 {

extern D2NPCMessageTableStrc aNpcGossipData[9];

} // namespace D2Game::Quests::QUESTS::A2Q1

namespace D2Game::Quests::QUESTS::A2Q2 {

extern D2NPCMessageTableStrc aNpcGossipData[11];

} // namespace D2Game::Quests::QUESTS::A2Q2

namespace D2Game::Quests::QUESTS::A2Q3 {

extern D2NPCMessageTableStrc aNpcGossipData[6];

} // namespace D2Game::Quests::QUESTS::A2Q3

namespace D2Game::Quests::QUESTS::A2Q4 {

extern D2NPCMessageTableStrc aNpcGossipData[12];

} // namespace D2Game::Quests::QUESTS::A2Q4

namespace D2Game::Quests::QUESTS::A2Q5 {

extern D2NPCMessageTableStrc aNpcGossipData[4];

} // namespace D2Game::Quests::QUESTS::A2Q5

namespace D2Game::Quests::QUESTS::A2Q6 {

extern D2NPCMessageTableStrc aNpcGossipData[8];

} // namespace D2Game::Quests::QUESTS::A2Q6

namespace D2Game::Quests::QUESTS::A2Q7 {

extern D2NPCMessageTableStrc aNpcGossipData[6];

} // namespace D2Game::Quests::QUESTS::A2Q7

namespace D2Game::Quests::QUESTS::A2Q8 {

extern D2NPCMessageTableStrc aNpcGossipData[2];

} // namespace D2Game::Quests::QUESTS::A2Q8

namespace D2Game::Quests::QUESTS::A4Q0 {

extern D2NPCMessageTableStrc aNpcGossipData[2];

} // namespace D2Game::Quests::QUESTS::A4Q0

namespace D2Game::Quests::QUESTS::A4Q1 {

extern D2NPCMessageTableStrc aNpcGossipData[6];

} // namespace D2Game::Quests::QUESTS::A4Q1

namespace D2Game::Quests::QUESTS::A4Q2 {

extern D2NPCMessageTableStrc aNpcGossipData[7];

} // namespace D2Game::Quests::QUESTS::A4Q2

namespace D2Game::Quests::QUESTS::A4Q3 {

extern D2NPCMessageTableStrc aNpcGossipData[5];

} // namespace D2Game::Quests::QUESTS::A4Q3

namespace D2Game::Quests::QUESTS::A4Q4 {

extern D2NPCMessageTableStrc aNpcGossipData[3];

} // namespace D2Game::Quests::QUESTS::A4Q4

namespace D2Game::Quests::QUESTS::Quest39 {

extern D2NPCMessageTableStrc aNpcGossipData[3];

} // namespace D2Game::Quests::QUESTS::Quest39

namespace D2Game::Quests::QUESTS::A3Q0 {

extern D2NPCMessageTableStrc aNpcGossipData[3];

} // namespace D2Game::Quests::QUESTS::A3Q0

namespace D2Game::Quests::QUESTS::A3Q1 {

extern D2NPCMessageTableStrc aNpcGossipData[6];

} // namespace D2Game::Quests::QUESTS::A3Q1

namespace D2Game::Quests::QUESTS::A3Q2 {

extern D2NPCMessageTableStrc aNpcGossipData[13];

} // namespace D2Game::Quests::QUESTS::A3Q2

namespace D2Game::Quests::QUESTS::A3Q3 {

extern D2NPCMessageTableStrc aNpcGossipData[8];

} // namespace D2Game::Quests::QUESTS::A3Q3

namespace D2Game::Quests::QUESTS::A3Q4 {

extern D2NPCMessageTableStrc aNpcGossipData[9];

} // namespace D2Game::Quests::QUESTS::A3Q4

namespace D2Game::Quests::QUESTS::A3Q5 {

extern D2NPCMessageTableStrc aNpcGossipData[8];

} // namespace D2Game::Quests::QUESTS::A3Q5

namespace D2Game::Quests::QUESTS::A3Q6 {

extern D2NPCMessageTableStrc aNpcGossipData[8];

} // namespace D2Game::Quests::QUESTS::A3Q6

namespace D2Client::Waypoint {

extern int32_t nWaypointSignature;
extern D2WaypointStrc * pWaypoint;
extern bool bNeedAllocation;

} // namespace D2Client::Waypoint

namespace D2Common::Drlg::DrlgGrid {

extern fpFlagOperations apFlagOperations[6];

} // namespace D2Common::Drlg::DrlgGrid

namespace D2Game::GameData {

extern uint32_t gnAlwaysRun;

} // namespace D2Game::GameData

namespace D2Client::UI::Dialog {

extern D2DialogStrc * gpDialog;
extern D2DialogStrc * gpNpcTradeErrorDialog;
extern D2DialogStrc * gpDialogActual;
extern uint32_t gnDialogCounter_b;
extern D2DialogStrc * gpTradeAgreeDialog;
extern D2DialogStrc * gpTradeCancelDialog;
extern D2DialogStrc * gpHoradricCubeDialog;

} // namespace D2Client::UI::Dialog

namespace D2Client::Draw::LightMap {

extern D2LightMapStrc * gpLightMap;

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::Roster {

extern D2RosterStrc * gpRosterPlayers;

} // namespace D2Client::GAME::Roster

namespace D2Client::_Interaction {

extern int nCurrentInteractedNpcGUID;

} // namespace D2Client::_Interaction

namespace D2Client::Merc {

extern int nMercReviveCost;

} // namespace D2Client::Merc

namespace D2Client::Mouse {

extern int nPosX;
extern int nPosY;

} // namespace D2Client::Mouse

namespace D2Client::Forms {

extern D2ControlStrc * pFormsList;

} // namespace D2Client::Forms

namespace D2Client::_CdKey {

extern char * KeyClassic;
extern char * KeyExpansion;
extern char * KeyOwner;

} // namespace D2Client::_CdKey

// =============================================================================
// Globals recovered from multi-function body references
// (classified static-local by xref count, but named in other bodies too)
// =============================================================================

extern uint32_t nNextGUID;
extern uint32_t dwCycleLastSecond;

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
