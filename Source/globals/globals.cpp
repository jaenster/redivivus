/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

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
#include "Source/D2Client/D2Client.globals.h"
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
#include "Source/D2Common/D2Common.globals.h"
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
#include "Source/D2Direct3D/Renderer/Direct3D.h"
#include "Source/D2Direct3D/d3dFloorCache.h"
#include "Source/D2Direct3D/d3dSubtileCache.h"
#include "Source/D2Direct3D/d3dTextureCache.h"
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
#include "Source/D2Glide/Renderer/Glide.h"
#include "Source/D2Glide/glFloorCache.h"
#include "Source/D2Glide/glSubtileCache.h"
#include "Source/D2Hell/D2Hell.h"
#include "Source/D2Lang/D2Lang.h"
#include "Source/D2Lang/StrTable/strtable.h"
#include "Source/D2Launch/MainMenus.h"
#include "Source/D2Multi/OOGUtilities.h"
#include "Source/D2Win/D2WinAnimImage.h"
#include "Source/D2Win/Src/D2Comp.h"
#include "Source/D2Win/Src/D2WinFont.h"
#include "Source/D2gfx/D2GFX.h"
#include "3rdparty/crashy/crashy.globals.h"
#include "Source/Fog/D2MemoryManagerStrc.h"
#include "Source/Fog/D2PoolManagerStrc.h"
#include "Source/Fog/D2QServerStrc.h"
#include "Source/Fog/Fog.globals.h"
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
#include "3rdparty/Storm/Storm.globals.h"
#include "3rdparty/Storm/Storm.h"
#include "Source/D2Headers/d2_platform.h"
#include "Source/Fog/Source/SFile.data.h"
#include "Source/D2Client/GAME/Sound.h"
#include "Source/D2Common/Logging/ProfCore.h"
#include "Source/D2Common/Merc.h"
#include "Source/D2Common/Txt/Skills.h"
#include "Source/D2Glide/Renderer/Glide.h"
#include "3rdparty/Storm/H/stpl.h"
#include "3rdparty/Storm/Storm.h"

// =============================================================================
// Declaration closure — definitions
//
// The storage behind the closure declarations in globals.h whose content Ghidra
// actually has. The bytes are read from the binary, never rebuilt from the
// label: a Ghidra string label is a truncated, mangled rendering of its own
// content and agrees with it only by accident.
// =============================================================================

char s_14_006cc77c[] = "14"; D2_GLOBAL_GUARD(s_14_006cc77c, "s_14_006cc77c");
char s_207_82_87_133_0070f130[] = "207.82.87.133"; D2_GLOBAL_GUARD(s_207_82_87_133_0070f130, "s_207_82_87_133_0070f130");
char s_AppID_006fad8c[] = "AppID"; D2_GLOBAL_GUARD(s_AppID_006fad8c, "s_AppID_006fad8c");
char s_Assertion_Failure_006ccb6c[] = "Assertion Failure"; D2_GLOBAL_GUARD(s_Assertion_Failure_006ccb6c, "s_Assertion_Failure_006ccb6c");
char s_BLIZZARDKEY_006cef6c[] = "BLIZZARDKEY"; D2_GLOBAL_GUARD(s_BLIZZARDKEY_006cef6c, "s_BLIZZARDKEY_006cef6c");
char s_BNNews_D2DV_006df210[] = "BNNews-D2DV"; D2_GLOBAL_GUARD(s_BNNews_D2DV_006df210, "s_BNNews_D2DV_006df210");
char s_Blizzard_Entertainment__Inc__006deb80[] = "Blizzard Entertainment, Inc."; D2_GLOBAL_GUARD(s_Blizzard_Entertainment__Inc__006deb80, "s_Blizzard_Entertainment__Inc__006deb80");
char s_CLSID_006fad84[] = "CLSID"; D2_GLOBAL_GUARD(s_CLSID_006fad84, "s_CLSID_006fad84");
char s_CLen_006e163c[] = "CLen"; D2_GLOBAL_GUARD(s_CLen_006e163c, "s_CLen_006e163c");
char s_Cold_006e164c[] = "Cold"; D2_GLOBAL_GUARD(s_Cold_006e164c, "s_Cold_006e164c");
char s_Component_Categories_006fad6c[] = "Component Categories"; D2_GLOBAL_GUARD(s_Component_Categories_006fad6c, "s_Component_Categories_006fad6c");
char s_D2X_INTRO__s_BIK_006d913c[] = "D2X_INTRO_%s.BIK"; D2_GLOBAL_GUARD(s_D2X_INTRO__s_BIK_006d913c, "s_D2X_INTRO__s_BIK_006d913c");
char s_D2_LNG_006d5ee0[] = "D2.LNG"; D2_GLOBAL_GUARD(s_D2_LNG_006d5ee0, "s_D2_LNG_006d5ee0");
char s_DIABLO2SRV_007065dc[] = "DIABLO2SRV"; D2_GLOBAL_GUARD(s_DIABLO2SRV_007065dc, "s_DIABLO2SRV_007065dc");
char s_Deadlock_006ccb34[] = "Deadlock"; D2_GLOBAL_GUARD(s_Deadlock_006ccb34, "s_Deadlock_006ccb34");
char s_Diablo_2_EXE_006cc838[] = "Diablo 2.EXE"; D2_GLOBAL_GUARD(s_Diablo_2_EXE_006cc838, "s_Diablo_2_EXE_006cc838");
char s_Diablo_II_006cc8b8[] = "Diablo II"; D2_GLOBAL_GUARD(s_Diablo_II_006cc8b8, "s_Diablo_II_006cc8b8");
char s_Direct3D_Enumeration_0074c058[] = "Direct3D Enumeration"; D2_GLOBAL_GUARD(s_Direct3D_Enumeration_0074c058, "s_Direct3D_Enumeration_0074c058");
char s_Do_you_wish_to_terminate_the_appl_006cf1f4[] = "Do you wish to terminate the application\?"; D2_GLOBAL_GUARD(s_Do_you_wish_to_terminate_the_appl_006cf1f4, "s_Do_you_wish_to_terminate_the_appl_006cf1f4");
char s_ERROR___u__0x_08x__006cf2e0[] = "ERROR #%u (0x%08x)"; D2_GLOBAL_GUARD(s_ERROR___u__0x_08x__006cf2e0, "s_ERROR___u__0x_08x__006cf2e0");
char s_Enumeration_Class_0074c044[] = "Enumeration Class"; D2_GLOBAL_GUARD(s_Enumeration_Class_0074c044, "s_Enumeration_Class_0074c044");
char s_Error_1__Diablo_II_is_unable_to_p_0072daa8[] = "Error 1: Diablo II is unable to proceed.  Unsupported graphics mode."; D2_GLOBAL_GUARD(s_Error_1__Diablo_II_is_unable_to_p_0072daa8, "s_Error_1__Diablo_II_is_unable_to_p_0072daa8");
char s_Error_1__Diablo_II_is_unable_to_p_0072daf0[] = "Error 1: Diablo II is unable to proceed.  Unsupported graphics mode."; D2_GLOBAL_GUARD(s_Error_1__Diablo_II_is_unable_to_p_0072daf0, "s_Error_1__Diablo_II_is_unable_to_p_0072daf0");
char s_FLen_006e1634[] = "FLen"; D2_GLOBAL_GUARD(s_FLen_006e1634, "s_FLen_006e1634");
char s_FileType_006fad60[] = "FileType"; D2_GLOBAL_GUARD(s_FileType_006fad60, "s_FileType_006fad60");
char s_File_Read_Error_006ccb94[] = "File Read Error"; D2_GLOBAL_GUARD(s_File_Read_Error_006ccb94, "s_File_Read_Error_006ccb94");
char s_File_Write_Error_006ccb80[] = "File Write Error"; D2_GLOBAL_GUARD(s_File_Write_Error_006ccb80, "s_File_Write_Error_006ccb80");
char s_File___s_Line___d__006cf284[] = "File:\011%s\012Line:\011%d\012"; D2_GLOBAL_GUARD(s_File___s_Line___d__006cf284, "s_File___s_Line___d__006cf284");
char s_Fire_006e165c[] = "Fire"; D2_GLOBAL_GUARD(s_Fire_006e165c, "s_Fire_006e165c");
char s_Font16_006dc970[] = "Font16"; D2_GLOBAL_GUARD(s_Font16_006dc970, "s_Font16_006dc970");
char s_Font24_006dc930[] = "Font24"; D2_GLOBAL_GUARD(s_Font24_006dc930, "s_Font24_006dc930");
char s_Font30_006dc968[] = "Font30"; D2_GLOBAL_GUARD(s_Font30_006dc968, "s_Font30_006dc968");
char s_Font42_006dc960[] = "Font42"; D2_GLOBAL_GUARD(s_Font42_006dc960, "s_Font42_006dc960");
char s_Font6_006dc938[] = "Font6"; D2_GLOBAL_GUARD(s_Font6_006dc938, "s_Font6_006dc938");
char s_Font8_006dc978[] = "Font8"; D2_GLOBAL_GUARD(s_Font8_006dc978, "s_Font8_006dc978");
char s_FontExocet10_006dc910[] = "FontExocet10"; D2_GLOBAL_GUARD(s_FontExocet10_006dc910, "s_FontExocet10_006dc910");
char s_FontExocet8_006dc8f4[] = "FontExocet8"; D2_GLOBAL_GUARD(s_FontExocet8_006dc8f4, "s_FontExocet8_006dc8f4");
char s_FontFormal10_006dc950[] = "FontFormal10"; D2_GLOBAL_GUARD(s_FontFormal10_006dc950, "s_FontFormal10_006dc950");
char s_FontFormal11_006dc920[] = "FontFormal11"; D2_GLOBAL_GUARD(s_FontFormal11_006dc920, "s_FontFormal11_006dc920");
char s_FontFormal12_006dc940[] = "FontFormal12"; D2_GLOBAL_GUARD(s_FontFormal12_006dc940, "s_FontFormal12_006dc940");
char s_FontInGameChat_006dc8d0[] = "FontInGameChat"; D2_GLOBAL_GUARD(s_FontInGameChat_006dc8d0, "s_FontInGameChat_006dc8d0");
char s_FontRidiculous_006dc900[] = "FontRidiculous"; D2_GLOBAL_GUARD(s_FontRidiculous_006dc900, "s_FontRidiculous_006dc900");
char s_Function___s__006cf274[] = "Function:\011%s\012"; D2_GLOBAL_GUARD(s_Function___s__006cf274, "s_Function___s__006cf274");
char s_Game_006d5b14[] = "Game"; D2_GLOBAL_GUARD(s_Game_006d5b14, "s_Game_006d5b14");
char s_Gaunt_006d6b80[] = "Gaunt"; D2_GLOBAL_GUARD(s_Gaunt_006d6b80, "s_Gaunt_006d6b80");
char s_Halt_006ccb64[] = "Halt"; D2_GLOBAL_GUARD(s_Halt_006ccb64, "s_Halt_006ccb64");
char s_Handle___s__006cf25c[] = "Handle:\011%s\012"; D2_GLOBAL_GUARD(s_Handle___s__006cf25c, "s_Handle___s__006cf25c");
char s_Hardware_006fad48[] = "Hardware"; D2_GLOBAL_GUARD(s_Hardware_006fad48, "s_Hardware_006fad48");
char s_Interface_006fad54[] = "Interface"; D2_GLOBAL_GUARD(s_Interface_006fad54, "s_Interface_006fad54");
char s_Mana_006e1614[] = "Mana"; D2_GLOBAL_GUARD(s_Mana_006e1614, "s_Mana_006e1614");
char s_Mime_006fad40[] = "Mime"; D2_GLOBAL_GUARD(s_Mime_006fad40, "s_Mime_006fad40");
char s_Missing_Dll_006ccb50[] = "Missing Dll"; D2_GLOBAL_GUARD(s_Missing_Dll_006ccb50, "s_Missing_Dll_006ccb50");
char s_No_Error_006ccbc8[] = "No Error"; D2_GLOBAL_GUARD(s_No_Error_006ccbc8, "s_No_Error_006ccbc8");
char s_Object___s__006cf268[] = "Object:\011%s\012"; D2_GLOBAL_GUARD(s_Object___s__006cf268, "s_Object___s__006cf268");
char s_Out_of_Memory_Error_006ccba4[] = "Out of Memory Error"; D2_GLOBAL_GUARD(s_Out_of_Memory_Error_006ccba4, "s_Out_of_Memory_Error_006ccba4");
char s_PKWARE_Data_Compression_Library_f_0074be60[] = "PKWARE Data Compression Library for Win32\015\012Copyright 1989-1995 PKWARE Inc.  All Rights Reserved\015\012Patent No. 5,051,745\015\012PKWARE Data Compression Library Reg. U.S. Pat. and Tm. Off.\015\012Version 1.11"; D2_GLOBAL_GUARD(s_PKWARE_Data_Compression_Library_f_0074be60, "s_PKWARE_Data_Compression_Library_f_0074be60");
char s_PLen_006e162c[] = "PLen"; D2_GLOBAL_GUARD(s_PLen_006e162c, "s_PLen_006e162c");
char s_Press_OK_to_terminate_the_applica_006cf220[] = "Press OK to terminate the application."; D2_GLOBAL_GUARD(s_Press_OK_to_terminate_the_applica_006cf220, "s_Press_OK_to_terminate_the_applica_006cf220");
char s_Program___s__006cf298[] = "Program:\011%s\012"; D2_GLOBAL_GUARD(s_Program___s__006cf298, "s_Program___s__006cf298");
char s_ReallyTheLastSucker_006dc8e0[] = "ReallyTheLastSucker"; D2_GLOBAL_GUARD(s_ReallyTheLastSucker_006dc8e0, "s_ReallyTheLastSucker_006dc8e0");
char s_Registry_Error_006ccb40[] = "Registry Error"; D2_GLOBAL_GUARD(s_Registry_Error_006ccb40, "s_Registry_Error_006ccb40");
char s_SECURITY_006fad30[] = "SECURITY"; D2_GLOBAL_GUARD(s_SECURITY_006fad30, "s_SECURITY_006fad30");
char s_SYSTEM_006fad28[] = "SYSTEM"; D2_GLOBAL_GUARD(s_SYSTEM_006fad28, "s_SYSTEM_006fad28");
char s_Software_006d71ec[] = "Software"; D2_GLOBAL_GUARD(s_Software_006d71ec, "s_Software_006d71ec");
char s_Stupid_Lng_cpp_error__006d5ea8[] = "Stupid Lng.cpp error."; D2_GLOBAL_GUARD(s_Stupid_Lng_cpp_error__006d5ea8, "s_Stupid_Lng_cpp_error__006d5ea8");
char s_The_syntax_of_the_command_is_inco_006fcbe4[] = "The syntax of the command is incorrect."; D2_GLOBAL_GUARD(s_The_syntax_of_the_command_is_inco_006fcbe4, "s_The_syntax_of_the_command_is_inco_006fcbe4");
char s_This_application_has_encountered__006cf2a8[] = "This application has encountered a critical error:\012\012%s\012"; D2_GLOBAL_GUARD(s_This_application_has_encountered__006cf2a8, "s_This_application_has_encountered__006cf2a8");
char s_TypeLib_006fad20[] = "TypeLib"; D2_GLOBAL_GUARD(s_TypeLib_006fad20, "s_TypeLib_006fad20");
char s_Unable_to_start_LNG_manager__006d5ec0[] = "Unable to start LNG manager."; D2_GLOBAL_GUARD(s_Unable_to_start_LNG_manager__006d5ec0, "s_Unable_to_start_LNG_manager__006d5ec0");
char s_Unknown_Error_006ccbb8[] = "Unknown Error"; D2_GLOBAL_GUARD(s_Unknown_Error_006ccbb8, "s_Unknown_Error_006ccbb8");
char s_Warning_006ccb5c[] = "Warning"; D2_GLOBAL_GUARD(s_Warning_006ccb5c, "s_Warning_006ccb5c");
char s___006d727c[] = "@"; D2_GLOBAL_GUARD(s___006d727c, "s___006d727c");
char s___H__006ed5a4[] = " (H)"; D2_GLOBAL_GUARD(s___H__006ed5a4, "s___H__006ed5a4");
char s___N__006ed59c[] = " (N)"; D2_GLOBAL_GUARD(s___N__006ed59c, "s___N__006ed59c");
char s_____006ccb30[] = "\?\?\?"; D2_GLOBAL_GUARD(s_____006ccb30, "s_____006ccb30");
char s__not_xlated_call_ken_w_00730520[] = " -not xlated call ken w"; D2_GLOBAL_GUARD(s__not_xlated_call_ken_w_00730520, "s__not_xlated_call_ken_w_00730520");
char s__s_palette_act_d__s_006dc7d0[] = "%s\\palette\\act%d\\%s"; D2_GLOBAL_GUARD(s__s_palette_act_d__s_006dc7d0, "s__s_palette_act_d__s_006dc7d0");
char s_block_006e1730[] = "block"; D2_GLOBAL_GUARD(s_block_006e1730, "s_block_006e1730");
char s_blocks_006e1738[] = "blocks"; D2_GLOBAL_GUARD(s_blocks_006e1738, "s_blocks_006e1738");
char s_buffer_error_006d37e0[] = "buffer error"; D2_GLOBAL_GUARD(s_buffer_error_006d37e0, "s_buffer_error_006d37e0");
char s_buysell_006d6b50[] = "buysell"; D2_GLOBAL_GUARD(s_buysell_006d6b50, "s_buysell_006d6b50");
char s_client_006cc920[] = "client"; D2_GLOBAL_GUARD(s_client_006cc920, "s_client_006cc920");
char s_cmncof_a1_d2_006d6c44[] = "cmncof_a1.d2"; D2_GLOBAL_GUARD(s_cmncof_a1_d2_006d6c44, "s_cmncof_a1_d2_006d6c44");
char s_cmncof_a2_d2_006d6c54[] = "cmncof_a2.d2"; D2_GLOBAL_GUARD(s_cmncof_a2_d2_006d6c54, "s_cmncof_a2_d2_006d6c54");
char s_cmncof_a3_d2_006d6c64[] = "cmncof_a3.d2"; D2_GLOBAL_GUARD(s_cmncof_a3_d2_006d6c64, "s_cmncof_a3_d2_006d6c64");
char s_cmncof_a4_d2_006d6c74[] = "cmncof_a4.d2"; D2_GLOBAL_GUARD(s_cmncof_a4_d2_006d6c74, "s_cmncof_a4_d2_006d6c74");
char s_cmncof_a6_d2_006d6c84[] = "cmncof_a6.d2"; D2_GLOBAL_GUARD(s_cmncof_a6_d2_006d6c84, "s_cmncof_a6_d2_006d6c84");
char s_d2_promo_bik_006d91ac[] = "d2_promo.bik"; D2_GLOBAL_GUARD(s_d2_promo_bik_006d91ac, "s_d2_promo_bik_006d91ac");
char s_d2data_mpq_006dc770[] = "d2data.mpq"; D2_GLOBAL_GUARD(s_d2data_mpq_006dc770, "s_d2data_mpq_006dc770");
char s_d2exp_mpq_006cd3a0[] = "d2exp.mpq"; D2_GLOBAL_GUARD(s_d2exp_mpq_006cd3a0, "s_d2exp_mpq_006cd3a0");
char s_data_error_006d3804[] = "data error"; D2_GLOBAL_GUARD(s_data_error_006d3804, "s_data_error_006d3804");
char s_data_global_music_act1_caves_wav_006dea30[] = "data\\global\\music\\act1\\caves.wav"; D2_GLOBAL_GUARD(s_data_global_music_act1_caves_wav_006dea30, "s_data_global_music_act1_caves_wav_006dea30");
char s_data_global_music_act1_crypt_wav_006de9e4[] = "data\\global\\music\\act1\\crypt.wav"; D2_GLOBAL_GUARD(s_data_global_music_act1_crypt_wav_006de9e4, "s_data_global_music_act1_crypt_wav_006de9e4");
char s_data_global_music_act1_monastery__006dea08[] = "data\\global\\music\\act1\\monastery.wav"; D2_GLOBAL_GUARD(s_data_global_music_act1_monastery__006dea08, "s_data_global_music_act1_monastery__006dea08");
char s_data_global_music_act2_desert_wav_006de8c4[] = "data\\global\\music\\act2\\desert.wav"; D2_GLOBAL_GUARD(s_data_global_music_act2_desert_wav_006de8c4, "s_data_global_music_act2_desert_wav_006de8c4");
char s_data_global_music_act2_harem_wav_006de9c0[] = "data\\global\\music\\act2\\harem.wav"; D2_GLOBAL_GUARD(s_data_global_music_act2_harem_wav_006de9c0, "s_data_global_music_act2_harem_wav_006de9c0");
char s_data_global_music_act2_sewer_wav_006de8a0[] = "data\\global\\music\\act2\\sewer.wav"; D2_GLOBAL_GUARD(s_data_global_music_act2_sewer_wav_006de8a0, "s_data_global_music_act2_sewer_wav_006de8a0");
char s_data_global_music_act2_tombs_wav_006de99c[] = "data\\global\\music\\act2\\tombs.wav"; D2_GLOBAL_GUARD(s_data_global_music_act2_tombs_wav_006de99c, "s_data_global_music_act2_tombs_wav_006de99c");
char s_data_global_music_act3_kurast_wav_006de87c[] = "data\\global\\music\\act3\\kurast.wav"; D2_GLOBAL_GUARD(s_data_global_music_act3_kurast_wav_006de87c, "s_data_global_music_act3_kurast_wav_006de87c");
char s_data_global_music_act3_kurastsewe_006de950[] = "data\\global\\music\\act3\\kurastsewer.wav"; D2_GLOBAL_GUARD(s_data_global_music_act3_kurastsewe_006de950, "s_data_global_music_act3_kurastsewe_006de950");
char s_data_global_music_act3_spider_wav_006de978[] = "data\\global\\music\\act3\\spider.wav"; D2_GLOBAL_GUARD(s_data_global_music_act3_spider_wav_006de978, "s_data_global_music_act3_spider_wav_006de978");
char s_data_global_music_act4_diablo_wav_006de858[] = "data\\global\\music\\act4\\diablo.wav"; D2_GLOBAL_GUARD(s_data_global_music_act4_diablo_wav_006de858, "s_data_global_music_act4_diablo_wav_006de858");
char s_data_global_music_act5_icecaves_w_006de90c[] = "data\\global\\music\\act5\\icecaves.wav"; D2_GLOBAL_GUARD(s_data_global_music_act5_icecaves_w_006de90c, "s_data_global_music_act5_icecaves_w_006de90c");
char s_data_global_music_act5_xtemple_wa_006de8e8[] = "data\\global\\music\\act5\\xtemple.wav"; D2_GLOBAL_GUARD(s_data_global_music_act5_xtemple_wa_006de8e8, "s_data_global_music_act5_xtemple_wa_006de8e8");
char s_data_global_music_common_options__006dea54[] = "data\\global\\music\\common\\options.wav"; D2_GLOBAL_GUARD(s_data_global_music_common_options__006dea54, "s_data_global_music_common_options__006dea54");
char s_data_global_music_introedit_wav_006de930[] = "data\\global\\music\\introedit.wav"; D2_GLOBAL_GUARD(s_data_global_music_introedit_wav_006de930, "s_data_global_music_introedit_wav_006de930");
char s_data_local_font__s_default_map_006e02d8[] = "data\\local\\font\\%s\\default.map"; D2_GLOBAL_GUARD(s_data_local_font__s_default_map_006e02d8, "s_data_local_font__s_default_map_006e02d8");
char s_data_local_lng__s_expansionstring_006e028c[] = "data\\local\\lng\\%s\\expansionstring.tbl"; D2_GLOBAL_GUARD(s_data_local_lng__s_expansionstring_006e028c, "s_data_local_lng__s_expansionstring_006e028c");
char s_data_local_lng__s_patchstring_tbl_006e02b4[] = "data\\local\\lng\\%s\\patchstring.tbl"; D2_GLOBAL_GUARD(s_data_local_lng__s_patchstring_tbl_006e02b4, "s_data_local_lng__s_patchstring_tbl_006e02b4");
char s_data_local_lng__s_string_tbl_006e02f8[] = "data\\local\\lng\\%s\\string.tbl"; D2_GLOBAL_GUARD(s_data_local_lng__s_string_tbl_006e02f8, "s_data_local_lng__s_string_tbl_006e02f8");
char s_dialogbackground_00727498[] = "dialogbackground"; D2_GLOBAL_GUARD(s_dialogbackground_00727498, "s_dialogbackground_00727498");
char s_expand_006cc8f8[] = "expand"; D2_GLOBAL_GUARD(s_expand_006cc8f8, "s_expand_006cc8f8");
char s_file_error_006d3820[] = "file error"; D2_GLOBAL_GUARD(s_file_error_006d3820, "s_file_error_006d3820");
char s_grasp_006d6b78[] = "grasp"; D2_GLOBAL_GUARD(s_grasp_006d6b78, "s_grasp_006d6b78");
char s_incompatible_version_006d37c8[] = "incompatible version"; D2_GLOBAL_GUARD(s_incompatible_version_006d37c8, "s_incompatible_version_006d37c8");
char s_insufficient_memory_006d37f0[] = "insufficient memory"; D2_GLOBAL_GUARD(s_insufficient_memory_006d37f0, "s_insufficient_memory_006d37f0");
char s_launcher_006cc900[] = "launcher"; D2_GLOBAL_GUARD(s_launcher_006cc900, "s_launcher_006cc900");
char s_mode_006d5bc4[] = "mode"; D2_GLOBAL_GUARD(s_mode_006d5bc4, "s_mode_006d5bc4");
char s_modstate0_006cc928[] = "modstate0"; D2_GLOBAL_GUARD(s_modstate0_006cc928, "s_modstate0_006cc928");
char s_multiplayer_006cc90c[] = "multiplayer"; D2_GLOBAL_GUARD(s_multiplayer_006cc90c, "s_multiplayer_006cc90c");
char s_need_dictionary_006d3838[] = "need dictionary"; D2_GLOBAL_GUARD(s_need_dictionary_006d3838, "s_need_dictionary_006d3838");
char s_ohand_006d6b70[] = "ohand"; D2_GLOBAL_GUARD(s_ohand_006d6b70, "s_ohand_006d6b70");
char s_orotate_006d6b68[] = "orotate"; D2_GLOBAL_GUARD(s_orotate_006d6b68, "s_orotate_006d6b68");
char s_pal_dat_006dc7c8[] = "pal.dat"; D2_GLOBAL_GUARD(s_pal_dat_006dc7c8, "s_pal_dat_006dc7c8");
char s_pal_pl2_006dc7c0[] = "pal.pl2"; D2_GLOBAL_GUARD(s_pal_pl2_006dc7c0, "s_pal_pl2_006dc7c0");
char s_palette_act1_pal_dat_006dc7fc[] = "palette\\act1\\pal.dat"; D2_GLOBAL_GUARD(s_palette_act1_pal_dat_006dc7fc, "s_palette_act1_pal_dat_006dc7fc");
char s_palette_act1_pal_pl2_006dc7e4[] = "palette\\act1\\pal.pl2"; D2_GLOBAL_GUARD(s_palette_act1_pal_pl2_006dc7e4, "s_palette_act1_pal_pl2_006dc7e4");
char s_palette_fechar_pal_dat_006d39e8[] = "palette\\fechar\\pal.dat"; D2_GLOBAL_GUARD(s_palette_fechar_pal_dat_006d39e8, "s_palette_fechar_pal_dat_006d39e8");
char s_palette_fechar_pal_pl2_006d39d0[] = "palette\\fechar\\pal.pl2"; D2_GLOBAL_GUARD(s_palette_fechar_pal_pl2_006d39d0, "s_palette_fechar_pal_pl2_006d39d0");
char s_palette_sky_pal_dat_006d39bc[] = "palette\\sky\\pal.dat"; D2_GLOBAL_GUARD(s_palette_sky_pal_dat_006d39bc, "s_palette_sky_pal_dat_006d39bc");
char s_palette_sky_pal_pl2_006d39a8[] = "palette\\sky\\pal.pl2"; D2_GLOBAL_GUARD(s_palette_sky_pal_pl2_006d39a8, "s_palette_sky_pal_pl2_006d39a8");
char s_patch_d2_mpq_006dc728[] = "patch_d2.mpq"; D2_GLOBAL_GUARD(s_patch_d2_mpq_006dc728, "s_patch_d2_mpq_006dc728");
char s_ppress_006d6b60[] = "ppress"; D2_GLOBAL_GUARD(s_ppress_006d6b60, "s_ppress_006d6b60");
char s_profile_description_006d4eb8[] = "profile\\description"; D2_GLOBAL_GUARD(s_profile_description_006d4eb8, "s_profile_description_006d4eb8");
char s_profile_location_006d4ecc[] = "profile\\location"; D2_GLOBAL_GUARD(s_profile_location_006d4ecc, "s_profile_location_006d4ecc");
char s_profile_sex_006d4ee0[] = "profile\\sex"; D2_GLOBAL_GUARD(s_profile_sex_006d4ee0, "s_profile_sex_006d4ee0");
char s_protate_006d6b58[] = "protate"; D2_GLOBAL_GUARD(s_protate_006d6b58, "s_protate_006d6b58");
char s_screen01_006d5d10[] = "screen01"; D2_GLOBAL_GUARD(s_screen01_006d5d10, "s_screen01_006d5d10");
char s_screen02_006d5d04[] = "screen02"; D2_GLOBAL_GUARD(s_screen02_006d5d04, "s_screen02_006d5d04");
char s_screen03_006d5cf8[] = "screen03"; D2_GLOBAL_GUARD(s_screen03_006d5cf8, "s_screen03_006d5cf8");
char s_screen04_006d5cec[] = "screen04"; D2_GLOBAL_GUARD(s_screen04_006d5cec, "s_screen04_006d5cec");
char s_screen05_006d5ce0[] = "screen05"; D2_GLOBAL_GUARD(s_screen05_006d5ce0, "s_screen05_006d5ce0");
char s_screen06_006d5cd4[] = "screen06"; D2_GLOBAL_GUARD(s_screen06_006d5cd4, "s_screen06_006d5cd4");
char s_screen07_006d5cc8[] = "screen07"; D2_GLOBAL_GUARD(s_screen07_006d5cc8, "s_screen07_006d5cc8");
char s_screen08_006d5cbc[] = "screen08"; D2_GLOBAL_GUARD(s_screen08_006d5cbc, "s_screen08_006d5cbc");
char s_screen09_006d5cb0[] = "screen09"; D2_GLOBAL_GUARD(s_screen09_006d5cb0, "s_screen09_006d5cb0");
char s_screen10_006d5ca4[] = "screen10"; D2_GLOBAL_GUARD(s_screen10_006d5ca4, "s_screen10_006d5ca4");
char s_server_006cc918[] = "server"; D2_GLOBAL_GUARD(s_server_006cc918, "s_server_006cc918");
char s_stream_end_006d382c[] = "stream end"; D2_GLOBAL_GUARD(s_stream_end_006d382c, "s_stream_end_006d382c");
char s_stream_error_006d3810[] = "stream error"; D2_GLOBAL_GUARD(s_stream_error_006d3810, "s_stream_error_006d3810");


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char lpDefault_006cc837[1] = { '\x00' }; D2_GLOBAL_GUARD(lpDefault_006cc837, "lpDefault_006cc837");

char gpCutsceneVideoFrameBuffer[8] = { "640x146" }; D2_GLOBAL_GUARD(gpCutsceneVideoFrameBuffer, "gpCutsceneVideoFrameBuffer");

char param_2_006d57bc[3] = { '%', 'd', '\x00' }; D2_GLOBAL_GUARD(param_2_006d57bc, "param_2_006d57bc");

char gsCharSelContext[4] = { '*', '\x00', '\x00', '\x00' }; D2_GLOBAL_GUARD(gsCharSelContext, "gsCharSelContext");

char gTxtCompilationMemPool[4] = { 'e', 'n', 'd', '\x00' }; D2_GLOBAL_GUARD(gTxtCompilationMemPool, "gTxtCompilationMemPool");

// also labelled: gaAudioPaletteConvertData
double gdConst255 = 255; D2_GLOBAL_GUARD(gdConst255, "gdConst255");

char s_C_006ed5b4[2] = { 'C', '\x00' }; D2_GLOBAL_GUARD(s_C_006ed5b4, "s_C_006ed5b4");

char s_SAM_006fad3c[4] = { 'S', 'A', 'M', '\x00' }; D2_GLOBAL_GUARD(s_SAM_006fad3c, "s_SAM_006fad3c");

char * gaAtlRegistryRootKeys[12] = {
    s_AppID_006fad8c,
    s_CLSID_006fad84,
    s_Component_Categories_006fad6c,
    s_FileType_006fad60,
    s_Interface_006fad54,
    s_Hardware_006fad48,
    s_Mime_006fad40,
    s_SAM_006fad3c,
    s_SECURITY_006fad30,
    s_SYSTEM_006fad28,
    s_Software_006d71ec,
    s_TypeLib_006fad20
}; D2_GLOBAL_GUARD(gaAtlRegistryRootKeys, "gaAtlRegistryRootKeys");

char * gaLanguageNames_00705000[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_00705000, "gaLanguageNames_00705000");

char * gaMonthAbbreviations[12] = {
    s_Jan_006ccb2c,
    s_Feb_006ccb28,
    s_Mar_006ccb24,
    s_Apr_006ccb20,
    s_May_006ccb1c,
    s_Jun_006ccb18,
    s_Jul_006ccb14,
    s_Aug_006ccb10,
    s_Sep_006ccb0c,
    s_Oct_006ccb08,
    s_Nov_006ccb04,
    s_Dec_006ccb00
}; D2_GLOBAL_GUARD(gaMonthAbbreviations, "gaMonthAbbreviations");

char * gaLanguageNames_0072d994[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_0072d994, "gaLanguageNames_0072d994");

char * gaLanguageNames_0072d9d0[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_0072d9d0, "gaLanguageNames_0072d9d0");

char * gaLanguageNames_0072da0c[15] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c
}; D2_GLOBAL_GUARD(gaLanguageNames_0072da0c, "gaLanguageNames_0072da0c");

char * gaszLanguageNames[32] = {
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c,
    nullptr,
    (char*)&Fog::Src::ErrorManager::s_English_006cc81c,
    (char*)&Fog::Src::ErrorManager::s_Spanish_006cc814,
    (char*)&Fog::Src::ErrorManager::s_German_006cc80c,
    (char*)&Fog::Src::ErrorManager::s_French_006cc804,
    (char*)&Fog::Src::ErrorManager::s_Portuguese_006cc7f8,
    (char*)&Fog::Src::ErrorManager::s_Italian_006cc7f0,
    (char*)&Fog::Src::ErrorManager::s_Japanese_006cc7e4,
    (char*)&Fog::Src::ErrorManager::s_Korean_006cc7dc,
    (char*)&Fog::Src::ErrorManager::s_Simplified_Chinese_006cc7c8,
    (char*)&Fog::Src::ErrorManager::s_Traditional_Chinese_006cc7b4,
    (char*)&Fog::Src::ErrorManager::s_Polish_006cc7ac,
    (char*)&Fog::Src::ErrorManager::s_Russian_006cc7a4,
    (char*)&Fog::Src::ErrorManager::s_English_with_Korean_Chat_006cc788,
    (char*)&Fog::Src::LogManager::s_Unknown_006cc780,
    s_14_006cc77c,
    nullptr
}; D2_GLOBAL_GUARD(gaszLanguageNames, "gaszLanguageNames");

double DOUBLE_0074adb4 = 1; D2_GLOBAL_GUARD(DOUBLE_0074adb4, "DOUBLE_0074adb4");

uint8_t gabyFxamClassOffset[16] = {
    0x8,
    0x4,
    0x8,
    0x8,
    0x8,
    0x4,
    0x8,
    0x8,
    0x0,
    0x4,
    0xc,
    0x8,
    0x0,
    0x4,
    0xc,
    0x8
}; D2_GLOBAL_GUARD(gabyFxamClassOffset, "gabyFxamClassOffset");

uint32_t User32Reserved[26] = {}; D2_GLOBAL_GUARD(User32Reserved, "User32Reserved");

uint32_t UserReserved[5] = {}; D2_GLOBAL_GUARD(UserReserved, "UserReserved");

void * SystemReserved1[54] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
}; D2_GLOBAL_GUARD(SystemReserved1, "SystemReserved1");

uint8_t SpareBytes[36] = {
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(SpareBytes, "SpareBytes");

CLIENT_ID RealClientId = {
    /* .UniqueProcess = */ nullptr,
    /* .UniqueThread = */ nullptr
}; D2_GLOBAL_GUARD(RealClientId, "RealClientId");

void * Win32ClientInfo[62] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
}; D2_GLOBAL_GUARD(Win32ClientInfo, "Win32ClientInfo");

void * glDispatchTable[233] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
}; D2_GLOBAL_GUARD(glDispatchTable, "glDispatchTable");

void * glReserved1[29] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
}; D2_GLOBAL_GUARD(glReserved1, "glReserved1");

uint16_t StaticUnicodeBuffer[261] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0
}; D2_GLOBAL_GUARD(StaticUnicodeBuffer, "StaticUnicodeBuffer");

void * TlsSlots[64] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
}; D2_GLOBAL_GUARD(TlsSlots, "TlsSlots");

void * DbgSsReserved[2] = { nullptr, nullptr }; D2_GLOBAL_GUARD(DbgSsReserved, "DbgSsReserved");

void * Instrumentation[9] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
}; D2_GLOBAL_GUARD(Instrumentation, "Instrumentation");

GUID ActivityId = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}} /* 00000000-0000-0000-0000-000000000000 */; D2_GLOBAL_GUARD(ActivityId, "ActivityId");

// =============================================================================
// Initialized scalars
// =============================================================================

uint8_t IMAGE_DOS_HEADER__00400000[128] = {}; D2_GLOBAL_GUARD(IMAGE_DOS_HEADER__00400000, "IMAGE_DOS_HEADER__00400000");
uint8_t gbChatDlgUpdateMarker = 0x2d; D2_GLOBAL_GUARD(gbChatDlgUpdateMarker, "gbChatDlgUpdateMarker");
uint8_t gbFloatConstantBase = 0x0; D2_GLOBAL_GUARD(gbFloatConstantBase, "gbFloatConstantBase");
uint8_t gaMonsterDenEnvironmentData = 0x0; D2_GLOBAL_GUARD(gaMonsterDenEnvironmentData, "gaMonsterDenEnvironmentData");
uint8_t gaWeatherTransitionConstant = 0x0; D2_GLOBAL_GUARD(gaWeatherTransitionConstant, "gaWeatherTransitionConstant");
char * gpszPaletteAct1Dat = s_palette_act1_pal_dat_006dc7fc; D2_GLOBAL_GUARD(gpszPaletteAct1Dat, "gpszPaletteAct1Dat");
char * gpszPaletteAct1Pl2 = s_palette_act1_pal_pl2_006dc7e4; D2_GLOBAL_GUARD(gpszPaletteAct1Pl2, "gpszPaletteAct1Pl2");
char * gpszPaletteActPathFormat = s__s_palette_act_d__s_006dc7d0; D2_GLOBAL_GUARD(gpszPaletteActPathFormat, "gpszPaletteActPathFormat");
char * gpszPalDatName = s_pal_dat_006dc7c8; D2_GLOBAL_GUARD(gpszPalDatName, "gpszPalDatName");
char * gpszPalPl2Name = s_pal_pl2_006dc7c0; D2_GLOBAL_GUARD(gpszPalPl2Name, "gpszPalPl2Name");
unsigned int gaItemTypeTxtIndexData = 0x6d6567; D2_GLOBAL_GUARD(gaItemTypeTxtIndexData, "gaItemTypeTxtIndexData");
uint32_t DEFAULT_SECURITY_COOKIE = 0xbb40e64e; D2_GLOBAL_GUARD(DEFAULT_SECURITY_COOKIE, "DEFAULT_SECURITY_COOKIE");
unsigned int gaCRT_FPremData_3 = 0x6a9a24; D2_GLOBAL_GUARD(gaCRT_FPremData_3, "gaCRT_FPremData_3");
unsigned int UINT_0096d6a8 = {}; D2_GLOBAL_GUARD(UINT_0096d6a8, "UINT_0096d6a8");
uint32_t LastErrorValue = 0x0; D2_GLOBAL_GUARD(LastErrorValue, "LastErrorValue");
uint32_t CountOfOwnedCriticalSections = 0x0; D2_GLOBAL_GUARD(CountOfOwnedCriticalSections, "CountOfOwnedCriticalSections");
void * CsrClientThread = (void *)0x00000000; D2_GLOBAL_GUARD(CsrClientThread, "CsrClientThread");
void * Win32ThreadInfo = (void *)0x00000000; D2_GLOBAL_GUARD(Win32ThreadInfo, "Win32ThreadInfo");
void * WOW32Reserved = (void *)0x00000000; D2_GLOBAL_GUARD(WOW32Reserved, "WOW32Reserved");
uint32_t CurrentLocale = 0x0; D2_GLOBAL_GUARD(CurrentLocale, "CurrentLocale");
uint32_t FpSoftwareStatusRegister = 0x0; D2_GLOBAL_GUARD(FpSoftwareStatusRegister, "FpSoftwareStatusRegister");
uint32_t ExceptionCode = 0x0; D2_GLOBAL_GUARD(ExceptionCode, "ExceptionCode");
void * ActivationContextStackPointer = (void *)0x00000000; D2_GLOBAL_GUARD(ActivationContextStackPointer, "ActivationContextStackPointer");
uint32_t TxFsContext = 0x0; D2_GLOBAL_GUARD(TxFsContext, "TxFsContext");
uint32_t GdiTebBatch = 0x0; D2_GLOBAL_GUARD(GdiTebBatch, "GdiTebBatch");
void * GdiCachedProcessHandle = (void *)0x00000000; D2_GLOBAL_GUARD(GdiCachedProcessHandle, "GdiCachedProcessHandle");
uint32_t GdiClientPID = 0x0; D2_GLOBAL_GUARD(GdiClientPID, "GdiClientPID");
uint32_t GdiCLientTID = 0x0; D2_GLOBAL_GUARD(GdiCLientTID, "GdiCLientTID");
void * GdiThreadLocalInfo = (void *)0x00000000; D2_GLOBAL_GUARD(GdiThreadLocalInfo, "GdiThreadLocalInfo");
void * glReserved2 = (void *)0x00000000; D2_GLOBAL_GUARD(glReserved2, "glReserved2");
void * glSectionInfo = (void *)0x00000000; D2_GLOBAL_GUARD(glSectionInfo, "glSectionInfo");
void * glSection = (void *)0x00000000; D2_GLOBAL_GUARD(glSection, "glSection");
void * glTable = (void *)0x00000000; D2_GLOBAL_GUARD(glTable, "glTable");
void * glCurrentRC = (void *)0x00000000; D2_GLOBAL_GUARD(glCurrentRC, "glCurrentRC");
void * glContext = (void *)0x00000000; D2_GLOBAL_GUARD(glContext, "glContext");
uint32_t LastStatusValue = 0x0; D2_GLOBAL_GUARD(LastStatusValue, "LastStatusValue");
void * DeallocationStack = (void *)0x00000000; D2_GLOBAL_GUARD(DeallocationStack, "DeallocationStack");
void * TlsLinks_Flink = (void *)0x00000000; D2_GLOBAL_GUARD(TlsLinks_Flink, "TlsLinks_Flink");
void * TlsLinks_Blink = (void *)0x00000000; D2_GLOBAL_GUARD(TlsLinks_Blink, "TlsLinks_Blink");
void * Vdm = (void *)0x00000000; D2_GLOBAL_GUARD(Vdm, "Vdm");
void * ReservedForNtRpc = (void *)0x00000000; D2_GLOBAL_GUARD(ReservedForNtRpc, "ReservedForNtRpc");
uint32_t HardErrorMode = 0x0; D2_GLOBAL_GUARD(HardErrorMode, "HardErrorMode");
void * SubProcessTag = (void *)0x00000000; D2_GLOBAL_GUARD(SubProcessTag, "SubProcessTag");
void * EtwLocalData = (void *)0x00000000; D2_GLOBAL_GUARD(EtwLocalData, "EtwLocalData");
void * EtwTraceData = (void *)0x00000000; D2_GLOBAL_GUARD(EtwTraceData, "EtwTraceData");
void * WinSockData = (void *)0x00000000; D2_GLOBAL_GUARD(WinSockData, "WinSockData");
uint32_t GdiBatchCount = 0x0; D2_GLOBAL_GUARD(GdiBatchCount, "GdiBatchCount");
uint32_t IdealProcessorValue = 0x0; D2_GLOBAL_GUARD(IdealProcessorValue, "IdealProcessorValue");
uint32_t GuaranteedStackBytes = 0x0; D2_GLOBAL_GUARD(GuaranteedStackBytes, "GuaranteedStackBytes");
void * ReservedForPerf = (void *)0x00000000; D2_GLOBAL_GUARD(ReservedForPerf, "ReservedForPerf");
void * ReservedForOle = (void *)0x00000000; D2_GLOBAL_GUARD(ReservedForOle, "ReservedForOle");
uint32_t WaitingOnLoaderLock = 0x0; D2_GLOBAL_GUARD(WaitingOnLoaderLock, "WaitingOnLoaderLock");
void * SavedPriorityState = (void *)0x00000000; D2_GLOBAL_GUARD(SavedPriorityState, "SavedPriorityState");
void * SoftPatchPtr1 = (void *)0x00000000; D2_GLOBAL_GUARD(SoftPatchPtr1, "SoftPatchPtr1");
void * ThreadPoolData = (void *)0x00000000; D2_GLOBAL_GUARD(ThreadPoolData, "ThreadPoolData");
void * TlsExpansionSlots = (void *)0x00000000; D2_GLOBAL_GUARD(TlsExpansionSlots, "TlsExpansionSlots");
uint32_t MuiGeneration = 0x0; D2_GLOBAL_GUARD(MuiGeneration, "MuiGeneration");
uint32_t IsImpersonating = 0x0; D2_GLOBAL_GUARD(IsImpersonating, "IsImpersonating");
void * NlsCache = (void *)0x00000000; D2_GLOBAL_GUARD(NlsCache, "NlsCache");
void * pShimData = (void *)0x00000000; D2_GLOBAL_GUARD(pShimData, "pShimData");
uint32_t HeapVirtualAffinity = 0x0; D2_GLOBAL_GUARD(HeapVirtualAffinity, "HeapVirtualAffinity");
void * CurrentTransactionHandle = (void *)0x00000000; D2_GLOBAL_GUARD(CurrentTransactionHandle, "CurrentTransactionHandle");
void * ActiveFrame = (void *)0x00000000; D2_GLOBAL_GUARD(ActiveFrame, "ActiveFrame");
void * FlsData = (void *)0x00000000; D2_GLOBAL_GUARD(FlsData, "FlsData");
void * PreferredLanguages = (void *)0x00000000; D2_GLOBAL_GUARD(PreferredLanguages, "PreferredLanguages");
void * UserPrefLanguages = (void *)0x00000000; D2_GLOBAL_GUARD(UserPrefLanguages, "UserPrefLanguages");
void * MergedPrefLanguages = (void *)0x00000000; D2_GLOBAL_GUARD(MergedPrefLanguages, "MergedPrefLanguages");
uint32_t MuiImpersonation = 0x0; D2_GLOBAL_GUARD(MuiImpersonation, "MuiImpersonation");
unsigned short CrossTebFlags = {}; D2_GLOBAL_GUARD(CrossTebFlags, "CrossTebFlags");
unsigned short SameTebFlags = {}; D2_GLOBAL_GUARD(SameTebFlags, "SameTebFlags");
void * TxnScopeEnterCallback = (void *)0x00000000; D2_GLOBAL_GUARD(TxnScopeEnterCallback, "TxnScopeEnterCallback");
void * TxnScopeExitCallback = (void *)0x00000000; D2_GLOBAL_GUARD(TxnScopeExitCallback, "TxnScopeExitCallback");
void * TxnScopeContext = (void *)0x00000000; D2_GLOBAL_GUARD(TxnScopeContext, "TxnScopeContext");
uint32_t LockCount = 0x0; D2_GLOBAL_GUARD(LockCount, "LockCount");
void * ResourceRetValue = (void *)0x00000000; D2_GLOBAL_GUARD(ResourceRetValue, "ResourceRetValue");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace Fog::Src::LogManager {

} // namespace Fog::Src::LogManager

namespace Fog::Src::ErrorManager {

} // namespace Fog::Src::ErrorManager

namespace Storm::Source::SSignature {

} // namespace Storm::Source::SSignature

namespace Fog::Engine::Application {

} // namespace Fog::Engine::Application

namespace Fog::Source::SFile {

} // namespace Fog::Source::SFile

namespace Fog::Src::DataArrays {

} // namespace Fog::Src::DataArrays

namespace Storm::Source::SSTR {

} // namespace Storm::Source::SSTR

namespace Fog::Debug {

} // namespace Fog::Debug

namespace Storm::Source::SFile {

} // namespace Storm::Source::SFile

namespace Fog::BitBuffer {

} // namespace Fog::BitBuffer

namespace Storm::Source::SErr {

} // namespace Storm::Source::SErr

namespace Storm::Source::SLog {

} // namespace Storm::Source::SLog

namespace Storm::_3rdparty::zlib::deflate {

} // namespace Storm::_3rdparty::zlib::deflate

namespace D2Launch::CharSel {

} // namespace D2Launch::CharSel

namespace D2Lang::StrTable::strtable {

} // namespace D2Lang::StrTable::strtable

namespace D2Multi::ChatDlg {

} // namespace D2Multi::ChatDlg

namespace D2Common::DATATBLS::LvlTbls {

} // namespace D2Common::DATATBLS::LvlTbls

namespace D2Client::ClientModeInGame {

} // namespace D2Client::ClientModeInGame

namespace D2Client::UI::Draw {

} // namespace D2Client::UI::Draw

namespace D2Client::UI::Inv {

} // namespace D2Client::UI::Inv

namespace D2Common::DATATBLS::DataTbls {

} // namespace D2Common::DATATBLS::DataTbls

namespace D2Client::Warden::NWardenClient {

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

} // namespace D2Client::Warden

namespace D2Client::Sound::SoundHdr {

} // namespace D2Client::Sound::SoundHdr

namespace D2Client::UI::ui {

} // namespace D2Client::UI::ui

namespace D2Client::UI::npcmenu {

} // namespace D2Client::UI::npcmenu

namespace D2Client::UI::QuestLog {

} // namespace D2Client::UI::QuestLog

namespace D2Sound::Src::D2SoundSmp {

} // namespace D2Sound::Src::D2SoundSmp

namespace D2Client::UI {

} // namespace D2Client::UI

namespace D2Client::UI::Button::ButtonImplementation {

} // namespace D2Client::UI::Button::ButtonImplementation

namespace D2Client::UI::Button::ButtonWrapper {

} // namespace D2Client::UI::Button::ButtonWrapper

namespace D2Client::UI::Editbox::EditBoxImplementation {

} // namespace D2Client::UI::Editbox::EditBoxImplementation

namespace D2Client::UI::Editbox::EditBoxWrapper {

} // namespace D2Client::UI::Editbox::EditBoxWrapper

namespace D2Client::UI::Spinner::SpinnerImplementation {

} // namespace D2Client::UI::Spinner::SpinnerImplementation

namespace D2Client::UI::Spinner::SpinnerWrapper {

} // namespace D2Client::UI::Spinner::SpinnerWrapper

namespace D2Client::UI::ListBox::ListBoxWrapper {

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

} // namespace D2Client::UI::ListBox::ListBoxImplementation

namespace D2gfx::D2GFX {

} // namespace D2gfx::D2GFX

namespace D2Sound::Sound {

} // namespace D2Sound::Sound

namespace Bnclient::BnSend {

} // namespace Bnclient::BnSend

namespace Bnclient::BnDownload {

} // namespace Bnclient::BnDownload

namespace Bnclient::_net_sid {

} // namespace Bnclient::_net_sid

namespace D2Game::GAME::Event::Dispatch {

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::UNIT::SUnitDmg {

} // namespace D2Game::UNIT::SUnitDmg

namespace D2Game::PLAYER::PlayerModes {

} // namespace D2Game::PLAYER::PlayerModes

namespace D2Game::GAME::Event::Dispatch::Player {

} // namespace D2Game::GAME::Event::Dispatch::Player

namespace D2Game::Objects::ObjectsMode {

} // namespace D2Game::Objects::ObjectsMode

namespace D2Game::GAME::Event::Dispatch::Object {

} // namespace D2Game::GAME::Event::Dispatch::Object

namespace D2Game::GAME::Event::Dispatch::Monster {

} // namespace D2Game::GAME::Event::Dispatch::Monster

namespace D2Game {

} // namespace D2Game

namespace D2CMP::SRC::CelCmp {

} // namespace D2CMP::SRC::CelCmp

namespace D2Common::Chat::Ignorelist::ChatIgnoreList {

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common {

} // namespace D2Common

namespace D2Common::Txt::Cube {

} // namespace D2Common::Txt::Cube

namespace D2Common::DATATBLS::ItemTbls {

} // namespace D2Common::DATATBLS::ItemTbls

namespace D2Common::Drlg::Preset {

} // namespace D2Common::Drlg::Preset

namespace D2Common::Drlg::OutSiege {

} // namespace D2Common::Drlg::OutSiege

namespace D2Common::Drlg::Outdoors {

} // namespace D2Common::Drlg::Outdoors

namespace D2Common::Drlg::OutJung {

} // namespace D2Common::Drlg::OutJung

namespace D2Common::Drlg {

} // namespace D2Common::Drlg

namespace Define_the_symbol__ATL_MIXED {

// =============================================================================
// Initialized scalars
// =============================================================================

void* vftable_meta_ptr = (void*)0x007009cc; D2_GLOBAL_GUARD(vftable_meta_ptr, "Define_the_symbol__ATL_MIXED::vftable_meta_ptr");

} // namespace Define_the_symbol__ATL_MIXED

namespace Storm::Source {

} // namespace Storm::Source

namespace Fog::File {

} // namespace Fog::File

namespace Fog::_3rdparty::crashy {

} // namespace Fog::_3rdparty::crashy

namespace D2Hell {

} // namespace D2Hell

namespace Fog::Source::SSComp {

} // namespace Fog::Source::SSComp

namespace Class::FILEREC {

// =============================================================================
// Uninitialized data (BSS)
// =============================================================================

uint8_t RTTI_Type_Descriptor; D2_GLOBAL_GUARD_BSS(RTTI_Type_Descriptor, "Class::FILEREC::RTTI_Type_Descriptor");

} // namespace Class::FILEREC

namespace Class::EVENTREC {

// =============================================================================
// Uninitialized data (BSS)
// =============================================================================

uint8_t RTTI_Type_Descriptor; D2_GLOBAL_GUARD_BSS(RTTI_Type_Descriptor, "Class::EVENTREC::RTTI_Type_Descriptor");

} // namespace Class::EVENTREC

namespace Class::REQUEST {

// =============================================================================
// Uninitialized data (BSS)
// =============================================================================

uint8_t RTTI_Type_Descriptor; D2_GLOBAL_GUARD_BSS(RTTI_Type_Descriptor, "Class::REQUEST::RTTI_Type_Descriptor");

} // namespace Class::REQUEST

namespace Class::AUDIOSTREAM {

// =============================================================================
// Uninitialized data (BSS)
// =============================================================================

uint8_t RTTI_Type_Descriptor; D2_GLOBAL_GUARD_BSS(RTTI_Type_Descriptor, "Class::AUDIOSTREAM::RTTI_Type_Descriptor");

} // namespace Class::AUDIOSTREAM

namespace Class::HANDLER {

// =============================================================================
// Uninitialized data (BSS)
// =============================================================================

uint8_t RTTI_Type_Descriptor; D2_GLOBAL_GUARD_BSS(RTTI_Type_Descriptor, "Class::HANDLER::RTTI_Type_Descriptor");

} // namespace Class::HANDLER

namespace D2Launch::MainMenus {

} // namespace D2Launch::MainMenus

namespace D2Multi::OOGUtilities {

} // namespace D2Multi::OOGUtilities

namespace D2Client::GAME::Game {

} // namespace D2Client::GAME::Game

namespace D2Client {

} // namespace D2Client

namespace D2Client::GAME::Record {

} // namespace D2Client::GAME::Record

namespace D2Client::_Message {

} // namespace D2Client::_Message

namespace D2Client::Engine {

} // namespace D2Client::Engine

namespace D2Client::GAME::SCmd {

} // namespace D2Client::GAME::SCmd

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::Rendering {

} // namespace D2Client::GAME::Rendering

namespace D2Client::UI::Panel {

} // namespace D2Client::UI::Panel

namespace D2Client::UI::Party {

} // namespace D2Client::UI::Party

namespace D2Client::Sound {

} // namespace D2Client::Sound

namespace D2Client::UI::UseItem {

} // namespace D2Client::UI::UseItem

namespace D2Client::UNIT::UnitSnd {

} // namespace D2Client::UNIT::UnitSnd

namespace D2Client::UI::SkillDesc {

} // namespace D2Client::UI::SkillDesc

namespace D2Glide::Renderer::Glide {

} // namespace D2Glide::Renderer::Glide

namespace D2DDraw::Renderer::DirectDraw {

} // namespace D2DDraw::Renderer::DirectDraw

namespace D2Sound::Src::D2SoundFast {

} // namespace D2Sound::Src::D2SoundFast

namespace D2Lang::Unicode::UNISYS {

} // namespace D2Lang::Unicode::UNISYS

namespace D2Net::SRC::Client {

} // namespace D2Net::SRC::Client

namespace D2Game::PLAYER::PlayerSave {

} // namespace D2Game::PLAYER::PlayerSave

namespace D2Game::Quests::QUESTS::Quest40 {

} // namespace D2Game::Quests::QUESTS::Quest40

namespace D2Game::Quests::QUESTS::A5Q1 {

} // namespace D2Game::Quests::QUESTS::A5Q1

namespace D2Game::Quests::QUESTS::A5Q2 {

} // namespace D2Game::Quests::QUESTS::A5Q2

namespace D2Game::Quests::QUESTS::A5Q3 {

} // namespace D2Game::Quests::QUESTS::A5Q3

namespace D2Game::Quests::QUESTS::A5Q4 {

} // namespace D2Game::Quests::QUESTS::A5Q4

namespace D2Game::Quests::QUESTS::A5Q5 {

} // namespace D2Game::Quests::QUESTS::A5Q5

namespace D2Game::Quests::QUESTS::A5Q6 {

} // namespace D2Game::Quests::QUESTS::A5Q6

namespace D2Game::Quests::QUESTS::Quest37 {

} // namespace D2Game::Quests::QUESTS::Quest37

namespace D2Game::Quests::QUESTS::A1Q0 {

} // namespace D2Game::Quests::QUESTS::A1Q0

namespace D2Game::Quests::QUESTS::A1Q1 {

} // namespace D2Game::Quests::QUESTS::A1Q1

namespace D2Game::Quests::QUESTS::A1Q2 {

} // namespace D2Game::Quests::QUESTS::A1Q2

namespace D2Game::Quests::QUESTS::A1Q3 {

} // namespace D2Game::Quests::QUESTS::A1Q3

namespace D2Game::Quests::QUESTS::A1Q4 {

} // namespace D2Game::Quests::QUESTS::A1Q4

namespace D2Game::Quests::QUESTS::A1Q5 {

} // namespace D2Game::Quests::QUESTS::A1Q5

namespace D2Game::Quests::QUESTS::A1Q6 {

} // namespace D2Game::Quests::QUESTS::A1Q6

namespace D2Game::Quests::QUESTS::A1Q7 {

} // namespace D2Game::Quests::QUESTS::A1Q7

namespace D2Game::Quests::QUESTS::Quest38 {

} // namespace D2Game::Quests::QUESTS::Quest38

namespace D2Game::Quests::QUESTS::A2Q0 {

} // namespace D2Game::Quests::QUESTS::A2Q0

namespace D2Game::Quests::QUESTS::A2Q1 {

} // namespace D2Game::Quests::QUESTS::A2Q1

namespace D2Game::Quests::QUESTS::A2Q2 {

} // namespace D2Game::Quests::QUESTS::A2Q2

namespace D2Game::Quests::QUESTS::A2Q3 {

} // namespace D2Game::Quests::QUESTS::A2Q3

namespace D2Game::Quests::QUESTS::A2Q4 {

} // namespace D2Game::Quests::QUESTS::A2Q4

namespace D2Game::Quests::QUESTS::A2Q5 {

} // namespace D2Game::Quests::QUESTS::A2Q5

namespace D2Game::Quests::QUESTS::A2Q6 {

} // namespace D2Game::Quests::QUESTS::A2Q6

namespace D2Game::Quests::QUESTS::A2Q7 {

} // namespace D2Game::Quests::QUESTS::A2Q7

namespace D2Game::Quests::QUESTS::A2Q8 {

} // namespace D2Game::Quests::QUESTS::A2Q8

namespace D2Game::Monster::Create {

} // namespace D2Game::Monster::Create

namespace D2Game::Quests::QUESTS::A4Q0 {

} // namespace D2Game::Quests::QUESTS::A4Q0

namespace D2Game::Quests::QUESTS::A4Q1 {

} // namespace D2Game::Quests::QUESTS::A4Q1

namespace D2Game::Quests::QUESTS::A4Q2 {

} // namespace D2Game::Quests::QUESTS::A4Q2

namespace D2Game::Quests::QUESTS::A4Q3 {

} // namespace D2Game::Quests::QUESTS::A4Q3

namespace D2Game::Quests::QUESTS::A4Q4 {

} // namespace D2Game::Quests::QUESTS::A4Q4

namespace D2Game::Quests::QUESTS::Quest39 {

} // namespace D2Game::Quests::QUESTS::Quest39

namespace D2Game::Quests::QUESTS::A3Q0 {

} // namespace D2Game::Quests::QUESTS::A3Q0

namespace D2Game::Quests::QUESTS::A3Q1 {

} // namespace D2Game::Quests::QUESTS::A3Q1

namespace D2Game::Quests::QUESTS::A3Q2 {

} // namespace D2Game::Quests::QUESTS::A3Q2

namespace D2Game::Quests::QUESTS::A3Q3 {

} // namespace D2Game::Quests::QUESTS::A3Q3

namespace D2Game::Quests::QUESTS::A3Q4 {

} // namespace D2Game::Quests::QUESTS::A3Q4

namespace D2Game::Quests::QUESTS::A3Q5 {

} // namespace D2Game::Quests::QUESTS::A3Q5

namespace D2Game::Quests::QUESTS::A3Q6 {

} // namespace D2Game::Quests::QUESTS::A3Q6

namespace D2Game::SKILLS::SkillNec {

} // namespace D2Game::SKILLS::SkillNec

namespace D2CMP::SRC::CelCmp::CELLIST {

} // namespace D2CMP::SRC::CelCmp::CELLIST

namespace D2CMP {

} // namespace D2CMP

namespace D2CMP::Draw {

} // namespace D2CMP::Draw

namespace D2Common::Chat::Ignorelist::MappingEntryNode {

} // namespace D2Common::Chat::Ignorelist::MappingEntryNode

namespace D2Common::Chat::Ignorelist::StringEntryNode {

} // namespace D2Common::Chat::Ignorelist::StringEntryNode

namespace D2Common::Chat::Ignorelist {

} // namespace D2Common::Chat::Ignorelist

namespace D2Common::Logging::ProfCore {

} // namespace D2Common::Logging::ProfCore

namespace D2Common::DUNGEON::Dungeon {

} // namespace D2Common::DUNGEON::Dungeon

namespace D2Common::Environment {

} // namespace D2Common::Environment

namespace D2Common::Units {

} // namespace D2Common::Units

namespace D2Common::Monsters::MONSTERS {

} // namespace D2Common::Monsters::MONSTERS

namespace D2Common::Merc {

} // namespace D2Common::Merc

namespace D2Common::DATATBLS::OverlayTbls {

} // namespace D2Common::DATATBLS::OverlayTbls

namespace D2Common::Drlg::DrlgGrid {

} // namespace D2Common::Drlg::DrlgGrid

namespace D2Direct3D::Renderer {

} // namespace D2Direct3D::Renderer

namespace Fog::Src {

} // namespace Fog::Src

namespace D2Gdi::Renderer::Windowed {

} // namespace D2Gdi::Renderer::Windowed

namespace D2Launch::Launcher {

} // namespace D2Launch::Launcher

namespace D2Client::Lng {

} // namespace D2Client::Lng

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::UI::SpellSel {

} // namespace D2Client::UI::SpellSel

namespace D2Client::UNIT::CUnit {

} // namespace D2Client::UNIT::CUnit

namespace D2Client::Cursor {

} // namespace D2Client::Cursor

namespace D2Client::Engine::Gfx {

} // namespace D2Client::Engine::Gfx

namespace D2Client::UNIT::Monster {

} // namespace D2Client::UNIT::Monster

namespace D2Client::UI::Trade {

} // namespace D2Client::UI::Trade

namespace D2Client::UNIT::Object {

} // namespace D2Client::UNIT::Object

namespace D2Client::GAME::Sound {

} // namespace D2Client::GAME::Sound

namespace D2gfx {

} // namespace D2gfx

namespace D2Win::D2GFX {

} // namespace D2Win::D2GFX

namespace D2Win::Src::D2WinMain {

} // namespace D2Win::Src::D2WinMain

namespace D2Win::Src::D2WinButton {

} // namespace D2Win::Src::D2WinButton

namespace D2Win::Src::D2WinArchive {

} // namespace D2Win::Src::D2WinArchive

namespace D2Win::Src::D2WinTextBox {

} // namespace D2Win::Src::D2WinTextBox

namespace D2Sound {

} // namespace D2Sound

namespace Bnclient::Bnet {

} // namespace Bnclient::Bnet

namespace Bnclient {

} // namespace Bnclient

namespace D2Net::Game::Server {

} // namespace D2Net::Game::Server

namespace D2Game::UNIT::SUNITNPC {

} // namespace D2Game::UNIT::SUNITNPC

namespace D2Game::UNIT::sunitproxy {

} // namespace D2Game::UNIT::sunitproxy

namespace D2Game::GAME::Clients {

} // namespace D2Game::GAME::Clients

namespace D2Game::Quests::QUESTS {

} // namespace D2Game::Quests::QUESTS

namespace D2Common::DATATBLS::MonsterTbls {

} // namespace D2Common::DATATBLS::MonsterTbls

namespace D2Common::Logging {

} // namespace D2Common::Logging

namespace D2Common::DATATBLS::TokTbls {

} // namespace D2Common::DATATBLS::TokTbls
