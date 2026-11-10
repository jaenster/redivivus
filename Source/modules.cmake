# Generated from project.json "modules" by ghidra-mcp — DO NOT EDIT; change the module table there
# The modules the Windows build compiles. cmake/check-modules.cmake checks this file
# against project.json at configure time.
#   D2_MODULES                 every module, in the executable's link order
#   D2_MODULE_<m>_DIR          its directory
#   D2_MODULE_<m>_KIND         exe, dll-module, headers-only, 3rdparty or ours
#   D2_MODULE_<m>_DEPENDS      the modules it depends on directly
#   D2_MODULE_<m>_IMPORTS      of those, the static imports: the libraries it links
#   D2_MODULE_<m>_LOADS        of those, the LoadLibrary edges: it sees their interface only
#   D2_MODULE_<m>_PUBLIC       its public header, the one header another module includes
#   D2_MODULE_<m>_SEES         every module whose headers it may include, itself included
#   D2_MODULE_<m>_EXCEPTIONS   modules it references by a documented exception
#   D2_MODULE_<m>_STRICT       ON: a wrong-direction include is a compile error
#   D2_MODULE_<m>_GENERATED    ON: its code is generated from the Ghidra database
#   D2_MODULE_<m>_DATA         the unit holding its globals (Source/globals/<m>.cpp)
#   D2_MODULE_<m>_UNIT_ORDER   original-address: its library takes its units in D2_MODULE_<m>_UNITS's
#                              order; path: in path order
#   D2_MODULE_<m>_UNITS        its units in the 1.14d link's order, each placed by the address of its
#                              first function in the module's runs, with the count of runs its
#                              functions form (a split unit); a unit with no function last. The data
#                              unit is not among them: the library takes it after them

set(D2_MODULES crashy Fog Game Storm D2Client D2Launch D2Multi D2MCPClient D2gfx D2Win D2Glide D2DDraw D2Sound D2Hell Bnclient D2Lang D2Net D2Game D2CMP D2Common D2Direct3D D2Gdi D2Headers Redivivus libbacktrace)

# crashy -> Fog (import): 1.14-only crash reporter (no 1.13d counterpart); it calls Fog's FILETOOLS_*_UTF8 and STRING_* (00401100, 00401220, 00401790), Safesock WSACleanup (004016c0) and InitializeDebugSubsystem (00401640)
# crashy -> Storm (import): 1.14-only crash reporter; it frees through Storm's MISC_OperatorDeleteShim (00401090)
set(D2_MODULE_crashy_DIR "3rdparty/crashy")
set(D2_MODULE_crashy_KIND "3rdparty")
set(D2_MODULE_crashy_DEPENDS Fog Storm)
set(D2_MODULE_crashy_IMPORTS Fog Storm)
set(D2_MODULE_crashy_LOADS )
set(D2_MODULE_crashy_PUBLIC "3rdparty/crashy/crashy.h")
set(D2_MODULE_crashy_SEES crashy Fog Storm D2Hell D2Headers Redivivus)
set(D2_MODULE_crashy_EXCEPTIONS )
set(D2_MODULE_crashy_STRICT ON)
set(D2_MODULE_crashy_GENERATED ON)
set(D2_MODULE_crashy_DATA "Source/globals/crashy.cpp")
set(D2_MODULE_crashy_UNIT_ORDER "original-address")
set(D2_MODULE_crashy_UNITS
    3rdparty/crashy/crashyRoot.cpp  # 00401000 (outside the module's runs), 4 runs
    3rdparty/crashy/Report.cpp  # 00401070 (outside the module's runs), 1 run
)

# Fog -> Storm (import): 1.13d Fog import table: 32 symbol(s) from Storm; also in 1.13c, 1.10f
# Fog -> D2Lang exception (MISC_GetLanguageCode): WriteCrashReportProcessInfo (00407a80) is 1.14-only: no <Inspector.*>/<Locale.Game> strings in any 1.13d binary, so the 1.13d import graph cannot judge Fog -> D2Lang here
# Fog -> D2Lang exception (STRTABLE_GetRegistryLanguage): WriteCrashReportProcessInfo (00407a80) is 1.14-only: no <Inspector.*>/<Locale.Game> strings in any 1.13d binary, so the 1.13d import graph cannot judge Fog -> D2Lang here
set(D2_MODULE_Fog_DIR "Source/Fog")
set(D2_MODULE_Fog_KIND "dll-module")
set(D2_MODULE_Fog_DEPENDS Storm)
set(D2_MODULE_Fog_IMPORTS Storm)
set(D2_MODULE_Fog_LOADS )
set(D2_MODULE_Fog_PUBLIC "Source/Fog/Fog.h")
set(D2_MODULE_Fog_SEES Fog Storm D2Hell D2Lang D2Headers Redivivus)
set(D2_MODULE_Fog_EXCEPTIONS D2Lang)
set(D2_MODULE_Fog_STRICT OFF)
set(D2_MODULE_Fog_GENERATED ON)
set(D2_MODULE_Fog_DATA "Source/globals/Fog.cpp")
set(D2_MODULE_Fog_UNIT_ORDER "original-address")
set(D2_MODULE_Fog_UNITS
    Source/Fog/Src/ErrorManager.cpp  # 004016b0, 9 runs
    Source/Fog/Debug.cpp  # 00401b00, 6 runs
    Source/Fog/File.cpp  # 00403040, 2 runs
    Source/Fog/Engine/Application.cpp  # 004046d0, 2 runs
    Source/Fog/Source/SFile.cpp  # 00406870, 2 runs
    Source/Fog/Src/QServer/QServer.cpp  # 004080a0, 4 runs
    Source/Fog/FogRoot.cpp  # 00408f20, 10 runs
    Source/Fog/Memory.cpp  # 00409020, 6 runs
    Source/Fog/Src/AsyncData.cpp  # 0040a390, 1 run
    Source/Fog/Source/SSComp.cpp  # 0040abf0, 2 runs
    Source/Fog/Src/D2Net.cpp  # 0040b0c0, 2 runs
    Source/Fog/Src/Safesock.cpp  # 0040d880, 1 run
    Source/Fog/Src/LogManager.cpp  # 0040e250, 11 runs
    Source/Fog/BitBuffer.cpp  # 00410b10, 1 run
    Source/Fog/Save.cpp  # 00411130, 2 runs
    Source/Fog/SMem.cpp  # 004111a0, 1 run
    Source/Fog/Src/String/FogString.cpp  # 00411320, 1 run
    Source/Fog/Src/D2QSSocket.cpp  # 006bc010, 2 runs
    Source/Fog/Src/DataArrays.cpp  # 006bca10, 2 runs
    Source/Fog/Src/Excel.cpp  # 006bcda0, 1 run
    Source/Fog/Skills.cpp  # 006c0b80, 1 run
    Source/Fog/Src/D2QSQueue.cpp  # 006c1f60, 1 run
    Source/Fog/Src/QServer/QServerNT.cpp  # 006c2110, 1 run
    Source/Fog/Src/QServer/QServer98.cpp  # 006c33d0, 1 run
)

# Game -> Storm (import): 1.13d Game import table: 7 symbol(s) from Storm; also in 1.13c, 1.10f
# Game -> Fog (import): 1.13d Game import table: 13 symbol(s) from Fog; also in 1.13c, 1.10f
# Game -> D2gfx (import): 1.13d Game import table: 7 symbol(s) from D2gfx; also in 1.13c, 1.10f
# Game -> D2Common (import): 1.13d Game import table: 2 symbol(s) from D2Common; 1.13d only
# Game -> D2MCPClient (import): 1.13d Game import table: 1 symbol(s) from D2MCPClient; also in 1.13c, 1.10f
# Game -> D2Sound (import): 1.13d Game import table: 2 symbol(s) from D2Sound; also in 1.13c, 1.10f
# Game -> D2Win (import): 1.13d Game import table: 8 symbol(s) from D2Win; also in 1.13c, 1.10f
# Game -> D2Client (load): LoadLibrary + QueryInterface from Game; the host sees the interface, not what it imports
# Game -> D2Launch (load): LoadLibrary + QueryInterface from Game; the host sees the interface, not what it imports
# Game -> D2Multi (load): LoadLibrary + QueryInterface from Game; the host sees the interface, not what it imports
set(D2_MODULE_Game_DIR "Source/Game")
set(D2_MODULE_Game_KIND "exe")
set(D2_MODULE_Game_DEPENDS Storm Fog D2gfx D2Common D2MCPClient D2Sound D2Win D2Client D2Launch D2Multi)
set(D2_MODULE_Game_IMPORTS Storm Fog D2gfx D2Common D2MCPClient D2Sound D2Win)
set(D2_MODULE_Game_LOADS D2Client D2Launch D2Multi)
set(D2_MODULE_Game_PUBLIC "Source/Game/Game.h")
set(D2_MODULE_Game_SEES Fog Game Storm D2Client D2Launch D2Multi D2MCPClient D2gfx D2Win D2Sound D2Hell D2Lang D2CMP D2Common D2Headers Redivivus)
set(D2_MODULE_Game_EXCEPTIONS )
set(D2_MODULE_Game_STRICT ON)
set(D2_MODULE_Game_GENERATED ON)
set(D2_MODULE_Game_DATA "Source/globals/Game.cpp")
set(D2_MODULE_Game_UNIT_ORDER "original-address")
set(D2_MODULE_Game_UNITS
    Source/Game/Main.cpp  # 00405410 (outside the module's runs), 2 runs
)

set(D2_MODULE_Storm_DIR "3rdparty/Storm")
set(D2_MODULE_Storm_KIND "3rdparty")
set(D2_MODULE_Storm_DEPENDS )
set(D2_MODULE_Storm_IMPORTS )
set(D2_MODULE_Storm_LOADS )
set(D2_MODULE_Storm_PUBLIC "3rdparty/Storm/Storm.h")
set(D2_MODULE_Storm_SEES Storm D2Hell D2Headers Redivivus)
set(D2_MODULE_Storm_EXCEPTIONS )
set(D2_MODULE_Storm_STRICT OFF)
set(D2_MODULE_Storm_GENERATED ON)
set(D2_MODULE_Storm_DATA "Source/globals/Storm.cpp")
set(D2_MODULE_Storm_UNIT_ORDER "original-address")
set(D2_MODULE_Storm_UNITS
    3rdparty/Storm/Source/SMem.cpp  # 00411620, 2 runs
    3rdparty/Storm/Source/SSTR.cpp  # 00413490, 3 runs
    3rdparty/Storm/Source/SReg.cpp  # 00414930, 1 run
    3rdparty/Storm/Source/SFile.cpp  # 00415140, 17 runs
    3rdparty/Storm/Source/SEVT.cpp  # 004177c0, 23 runs
    3rdparty/Storm/Source/SSystem.cpp  # 0041beb0, 5 runs
    3rdparty/Storm/Source/SErr.cpp  # 0041c0e0, 9 runs
    3rdparty/Storm/WindowHandle.cpp  # 0041c2a0, 8 runs
    3rdparty/Storm/Source/SComp.cpp  # 0041c3d0, 10 runs
    3rdparty/Storm/Source/SSignature.cpp  # 0041d320, 4 runs
    3rdparty/Storm/Source/SLog.cpp  # 0041fa40, 1 run
    3rdparty/Storm/StormRoot.cpp  # 00420290, 1 run
    3rdparty/Storm/Source/SBig.cpp  # 00420eb0, 15 runs
    3rdparty/Storm/3rdparty/zlib/trees.cpp  # 00421040, 4 runs
    3rdparty/Storm/3rdparty/zlib/deflate.cpp  # 00427160 (outside the module's runs), 1 run
    3rdparty/Storm/3rdparty/zlib/inflate.cpp  # 00429810 (outside the module's runs), 1 run
    3rdparty/Storm/3rdparty/zlib/adler32.cpp  # 0042b660 (outside the module's runs), 1 run
    3rdparty/Storm/3rdparty/zlib/crc32.cpp  # 0042d450 (outside the module's runs), 1 run
    3rdparty/Storm/3rdparty/zlib/zutil.cpp  # 0042dc30 (outside the module's runs), 1 run
    3rdparty/Storm/3rdparty/zlib/inffast.cpp  # 0042dca0 (outside the module's runs), 1 run
    3rdparty/Storm/3rdparty/zlib/inftrees.cpp  # 0042e120 (outside the module's runs), 1 run
    3rdparty/Storm/3rdparty/PKWARE/explode.cpp  # 006b01b0, 1 run
    3rdparty/Storm/3rdparty/PKWARE/implode.cpp  # 006b0870, 1 run
    3rdparty/Storm/Source/SCMD.cpp  # 006c3cb0, 9 runs
    3rdparty/Storm/Source/stpl_CMDDEF.cpp  # 006c4040, 5 runs
    3rdparty/Storm/Source/SBMP.cpp  # 006c50d0, 1 run
    3rdparty/Storm/3rdparty/zlib/uncompr.cpp  # 006c79c0 (outside the module's runs), 1 run
)

# D2Client -> Storm (import): 1.13d D2Client import table: 50 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Client -> Fog (import): 1.13d D2Client import table: 56 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Client -> D2CMP (import): 1.13d D2Client import table: 36 symbol(s) from D2CMP; also in 1.13c, 1.10f
# D2Client -> D2Lang (import): 1.13d D2Client import table: 27 symbol(s) from D2Lang; also in 1.13c, 1.10f
# D2Client -> D2Net (import): 1.13d D2Client import table: 12 symbol(s) from D2Net; also in 1.13c, 1.10f
# D2Client -> D2gfx (import): 1.13d D2Client import table: 58 symbol(s) from D2gfx; also in 1.13c, 1.10f
# D2Client -> D2Common (import): 1.13d D2Client import table: 627 symbol(s) from D2Common; also in 1.13c, 1.10f
# D2Client -> D2MCPClient (import): 1.13d D2Client import table: 1 symbol(s) from D2MCPClient; also in 1.13c, 1.10f
# D2Client -> D2Sound (import): 1.13d D2Client import table: 43 symbol(s) from D2Sound; also in 1.13c, 1.10f
# D2Client -> D2Game (import): 1.13d D2Client import table: 13 symbol(s) from D2Game; also in 1.13c, 1.10f
# D2Client -> D2Win (import): 1.13d D2Client import table: 38 symbol(s) from D2Win; also in 1.13c, 1.10f
# D2Client -> Bnclient (load): LoadLibrary + QueryInterface from D2Client; the host sees the interface, not what it imports
set(D2_MODULE_D2Client_DIR "Source/D2Client")
set(D2_MODULE_D2Client_KIND "dll-module")
set(D2_MODULE_D2Client_DEPENDS Storm Fog D2CMP D2Lang D2Net D2gfx D2Common D2MCPClient D2Sound D2Game D2Win Bnclient)
set(D2_MODULE_D2Client_IMPORTS Storm Fog D2CMP D2Lang D2Net D2gfx D2Common D2MCPClient D2Sound D2Game D2Win)
set(D2_MODULE_D2Client_LOADS Bnclient)
set(D2_MODULE_D2Client_PUBLIC "Source/D2Client/D2Client.h")
set(D2_MODULE_D2Client_SEES Fog Storm D2Client D2MCPClient D2gfx D2Win D2Sound D2Hell Bnclient D2Lang D2Net D2Game D2CMP D2Common D2Headers Redivivus)
set(D2_MODULE_D2Client_EXCEPTIONS )
set(D2_MODULE_D2Client_STRICT OFF)
set(D2_MODULE_D2Client_GENERATED ON)
set(D2_MODULE_D2Client_DATA "Source/globals/D2Client.cpp")
set(D2_MODULE_D2Client_UNIT_ORDER "original-address")
set(D2_MODULE_D2Client_UNITS
    Source/D2Client/BNGatewayAccess.cpp  # 0042e680 (outside the module's runs), 2 runs
    Source/D2Client/ComCallback.cpp  # 004497f0 (outside the module's runs), 1 run
    Source/D2Client/McpConnect.cpp  # 0044b1f0 (outside the module's runs), 1 run
    Source/D2Client/ClientModeInGame.cpp  # 0044b430, 5 runs
    Source/D2Client/D2ClientRoot.cpp  # 0044b550, 39 runs
    Source/D2Client/GAME/Game/Game.cpp  # 0044b870, 27 runs
    Source/D2Client/GAME/Server.cpp  # 0044cf20, 1 run
    Source/D2Client/GAME/stpl_SGAMEDATA.cpp  # 0044dd90, 7 runs
    Source/D2Client/GAME/Game/SGAMEDATA.cpp  # 0044fbb0, 2 runs
    Source/D2Client/GAME/Game/stpl_SGAMEDATA.cpp  # 0044fbe0, 2 runs
    Source/D2Client/Lng.cpp  # 004512c0, 1 run
    Source/D2Client/GAME/Record.cpp  # 00451400, 1 run
    Source/D2Client/Message.cpp  # 00451a90, 1 run
    Source/D2Client/DataTbls/MonsterTbls.cpp  # 00451fb0, 1 run
    Source/D2Client/UI/ui.cpp  # 00452090, 35 runs
    Source/D2Client/UI/stpl_SGAMEDATA.cpp  # 004545a0, 1 run
    Source/D2Client/UI/Draw.cpp  # 00456550, 2 runs
    Source/D2Client/UNIT.cpp  # 00457460, 1 run
    Source/D2Client/UI/Automap.cpp  # 004574d0, 2 runs
    Source/D2Client/Engine.cpp  # 0045ae50, 8 runs
    Source/D2Client/UI/Path.cpp  # 0045ae60, 4 runs
    Source/D2Client/UI/Text.cpp  # 0045afe0, 9 runs
    Source/D2Client/GAME/SCmd.cpp  # 0045b090, 3 runs
    Source/D2Client/Archive.cpp  # 0045b960, 4 runs
    Source/D2Client/UNIT/Player.cpp  # 00460830, 6 runs
    Source/D2Client/UNIT/CUnit.cpp  # 00463c90, 4 runs
    Source/D2Client/GAME/Select.cpp  # 00466dc0, 2 runs
    Source/D2Client/Cursor.cpp  # 00467df0, 3 runs
    Source/D2Client/UI/Panel.cpp  # 00468f40, 7 runs
    Source/D2Client/GAME/Msg.cpp  # 00468f90, 2 runs
    Source/D2Client/UI/CmdTbl.cpp  # 00469000, 5 runs
    Source/D2Client/Source/ClientEngine.cpp  # 0046aee0, 4 runs
    Source/D2Client/Engine/Gfx.cpp  # 0046db30, 5 runs
    Source/D2Client/SRC/CelCmp.cpp  # 0046f360, 2 runs
    Source/D2Client/Units.cpp  # 0046f950, 2 runs
    Source/D2Client/Draw/Weather.cpp  # 004722f0, 2 runs
    Source/D2Client/Draw/LightMap.cpp  # 004740d0, 1 run
    Source/D2Client/GAME/View.cpp  # 00475aa0, 1 run
    Source/D2Client/GAME/PalShift.cpp  # 00476e30, 1 run
    Source/D2Client/GAME/Rendering.cpp  # 004776c0, 4 runs
    Source/D2Client/UI/Chat.cpp  # 00477970, 3 runs
    Source/D2Client/Core/Archive.cpp  # 004788b0, 1 run
    Source/D2Client/GAME/RosterPets.cpp  # 00478a50, 1 run
    Source/D2Client/GAME/Roster.cpp  # 004792e0, 1 run
    Source/D2Client/UI/EscMenu.cpp  # 0047cc90, 11 runs
    Source/D2Client/GAME/Sound.cpp  # 0047cd80, 8 runs
    Source/D2Client/Sound/Sound.cpp  # 0047ce30, 8 runs
    Source/D2Client/UI/MiniPanel.cpp  # 0047e720, 3 runs
    Source/D2Client/Warden/WardenClient.cpp  # 0047f8c0, 5 runs
    Source/D2Client/Warden/NWardenClient/Library.cpp  # 0047fc30, 1 run
    Source/D2Client/UNIT/Path.cpp  # 00480420, 1 run
    Source/D2Client/UNIT/UnitMode.cpp  # 00480930, 1 run
    Source/D2Client/Sound/SoundHdr.cpp  # 00481720, 2 runs
    Source/D2Client/SkillHelpers.cpp  # 004834a0, 1 run
    Source/D2Client/UI/Inv.cpp  # 004835b0, 6 runs
    Source/D2Client/UI/npcmenu.cpp  # 00486e00, 12 runs
    Source/D2Client/UI/Hireables.cpp  # 00492c10, 1 run
    Source/D2Client/UI/Help.cpp  # 004942d0, 2 runs
    Source/D2Client/UI/Party.cpp  # 00499e10, 2 runs
    Source/D2Client/UI/Waypoint.cpp  # 0049c490, 1 run
    Source/D2Client/UI/QuestLog.cpp  # 004a18f0, 3 runs
    Source/D2Client/UI/SkillDesc.cpp  # 004a4260, 2 runs
    Source/D2Client/UI/SpellSel.cpp  # 004a8a30, 1 run
    Source/D2Client/UNIT/MonUnique.cpp  # 004ac740, 4 runs
    Source/D2Client/UNIT/Monster.cpp  # 004add90, 4 runs
    Source/D2Client/UNIT/Object.cpp  # 004b1d50, 2 runs
    Source/D2Client/UI/npcmenu/Merc.cpp  # 004b3240, 5 runs
    Source/D2Client/UI/Dialog.cpp  # 004b70f0, 2 runs
    Source/D2Client/UI/Trade.cpp  # 004b8a30, 2 runs
    Source/D2Client/UI/UIWidget.cpp  # 004babe0, 2 runs
    Source/D2Client/UI/Button/ButtonImplementation.cpp  # 004babf0, 3 runs
    Source/D2Client/UI/Button/ButtonWrapper.cpp  # 004badd0, 2 runs
    Source/D2Client/UI/Editbox/EditBoxImplementation.cpp  # 004bb170, 2 runs
    Source/D2Client/UI/Editbox/EditBoxWrapper.cpp  # 004bb9f0, 2 runs
    Source/D2Client/UI/Spinner/SpinnerImplementation.cpp  # 004bbe10, 2 runs
    Source/D2Client/UI/Spinner/SpinnerWrapper.cpp  # 004bc1f0, 2 runs
    Source/D2Client/UI/ListBox/ListBoxImplementation.cpp  # 004be090, 7 runs
    Source/D2Client/UI/ListBox/ListBoxWrapper.cpp  # 004becc0, 2 runs
    Source/D2Client/UI/UseItem.cpp  # 004bfa70, 2 runs
    Source/D2Client/UI/ShowItems.cpp  # 004c0640, 3 runs
    Source/D2Client/UNIT/Item.cpp  # 004c0d20, 4 runs
    Source/D2Client/SKILLS/Skills.cpp  # 004c51e0, 1 run
    Source/D2Client/SKILLS/SkillsBar.cpp  # 004c7c90, 1 run
    Source/D2Client/SKILLS/SkillsPal.cpp  # 004c9490, 1 run
    Source/D2Client/UNIT/UnitSnd.cpp  # 004ca2c0, 1 run
    Source/D2Client/UNIT/Missile.cpp  # 004cc800, 2 runs
    Source/D2Client/UNIT/PlrSkills.cpp  # 004d84c0, 3 runs
    Source/D2Client/Engine/Particle.cpp  # 004da000, 2 runs
    Source/D2Client/Engine/GfxUtil.cpp  # 004da720, 3 runs
    Source/D2Client/UNIT/CUnitEvent.cpp  # 004dc0c0, 1 run
    Source/D2Client/UNIT/PlayerList.cpp  # 004dc270, 1 run
    Source/D2Client/GAME/Wall2.cpp  # 004dc510, 3 runs
    Source/D2Client/GAME/Draw.cpp  # 004dde80, 1 run
    Source/D2Client/SKILLS/SkillsMon.cpp  # 004e0a30, 1 run
    Source/D2Client/SKILLS/SkillsSor.cpp  # 004e2f40, 2 runs
    Source/D2Client/Utils.cpp  # 004e4020, 3 runs
    Source/D2Client/SKILLS/SkillsEMon.cpp  # 004efcf0, 2 runs
    Source/D2Client/SKILLS.cpp  # 004f1430, 4 runs
    Source/D2Client/SKILLS/SkillsDru.cpp  # 004f1640, 1 run
    Source/D2Client/SKILLS/SkillsAss.cpp  # 004f2010, 2 runs
    Source/D2Client/SKILLS/SkillsNec.cpp  # 004f34d0, 1 run
    Source/D2Client/SKILLS/SkillsAma.cpp  # 004f4140, 1 run
    Source/D2Client/net_sid.cpp  # 00523ac0 (outside the module's runs), 2 runs
    Source/D2Client/Renderer/Direct3D.cpp  # 006b1d80 (outside the module's runs), 2 runs
    Source/D2Client/UI/Button.cpp  # 006ca0d8 (outside the module's runs), 1 run
    Source/D2Client/UI/Editbox.cpp  # 006ca138 (outside the module's runs), 1 run
    Source/D2Client/UI/Spinner.cpp  # 006ca198 (outside the module's runs), 1 run
    Source/D2Client/UI/ListBox.cpp  # 006ca263 (outside the module's runs), 1 run
)

# D2Launch -> Storm (import): 1.13d D2Launch import table: 19 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Launch -> Fog (import): 1.13d D2Launch import table: 27 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Launch -> D2Lang (import): 1.13d D2Launch import table: 21 symbol(s) from D2Lang; also in 1.13c, 1.10f
# D2Launch -> D2Net (import): 1.13d D2Launch import table: 4 symbol(s) from D2Net; also in 1.13c, 1.10f
# D2Launch -> D2gfx (import): 1.13d D2Launch import table: 2 symbol(s) from D2gfx; also in 1.13c, 1.10f
# D2Launch -> D2MCPClient (import): 1.13d D2Launch import table: 21 symbol(s) from D2MCPClient; also in 1.13c, 1.10f
# D2Launch -> D2Sound (import): 1.13d D2Launch import table: 11 symbol(s) from D2Sound; also in 1.13c, 1.10f
# D2Launch -> Bnclient (import): 1.13d D2Launch import table: 4 symbol(s) from Bnclient; also in 1.13c, 1.10f
# D2Launch -> D2Win (import): 1.13d D2Launch import table: 81 symbol(s) from D2Win; also in 1.13c, 1.10f
set(D2_MODULE_D2Launch_DIR "Source/D2Launch")
set(D2_MODULE_D2Launch_KIND "dll-module")
set(D2_MODULE_D2Launch_DEPENDS Storm Fog D2Lang D2Net D2gfx D2MCPClient D2Sound Bnclient D2Win)
set(D2_MODULE_D2Launch_IMPORTS Storm Fog D2Lang D2Net D2gfx D2MCPClient D2Sound Bnclient D2Win)
set(D2_MODULE_D2Launch_LOADS )
set(D2_MODULE_D2Launch_PUBLIC "Source/D2Launch/D2Launch.h")
set(D2_MODULE_D2Launch_SEES Fog Storm D2Launch D2MCPClient D2gfx D2Win D2Sound D2Hell Bnclient D2Lang D2Net D2CMP D2Common D2Headers Redivivus)
set(D2_MODULE_D2Launch_EXCEPTIONS )
set(D2_MODULE_D2Launch_STRICT OFF)
set(D2_MODULE_D2Launch_GENERATED ON)
set(D2_MODULE_D2Launch_DATA "Source/globals/D2Launch.cpp")
set(D2_MODULE_D2Launch_UNIT_ORDER "original-address")
set(D2_MODULE_D2Launch_UNITS
    Source/D2Launch/MainMenus.cpp  # 0042e5d0, 10 runs
    Source/D2Launch/D2LaunchRoot.cpp  # 0042f7a0, 7 runs
    Source/D2Launch/CharSel.cpp  # 00437a20, 3 runs
    Source/D2Launch/Launcher.cpp  # 0043b490, 2 runs
)

# D2Multi -> Storm (import): 1.13d D2Multi import table: 11 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Multi -> Fog (import): 1.13d D2Multi import table: 15 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Multi -> D2Lang (import): 1.13d D2Multi import table: 17 symbol(s) from D2Lang; also in 1.13c, 1.10f
# D2Multi -> D2Common (import): 1.13d D2Multi import table: 1 symbol(s) from D2Common; 1.13d only
# D2Multi -> D2MCPClient (import): 1.13d D2Multi import table: 28 symbol(s) from D2MCPClient; also in 1.13c, 1.10f
# D2Multi -> D2Sound (import): 1.13d D2Multi import table: 5 symbol(s) from D2Sound; also in 1.13c, 1.10f
# D2Multi -> D2Win (import): 1.13d D2Multi import table: 77 symbol(s) from D2Win; also in 1.13c, 1.10f
set(D2_MODULE_D2Multi_DIR "Source/D2Multi")
set(D2_MODULE_D2Multi_KIND "dll-module")
set(D2_MODULE_D2Multi_DEPENDS Storm Fog D2Lang D2Common D2MCPClient D2Sound D2Win)
set(D2_MODULE_D2Multi_IMPORTS Storm Fog D2Lang D2Common D2MCPClient D2Sound D2Win)
set(D2_MODULE_D2Multi_LOADS )
set(D2_MODULE_D2Multi_PUBLIC "Source/D2Multi/D2Multi.h")
set(D2_MODULE_D2Multi_SEES Fog Storm D2Multi D2MCPClient D2gfx D2Win D2Sound D2Hell D2Lang D2CMP D2Common D2Headers Redivivus)
set(D2_MODULE_D2Multi_EXCEPTIONS )
set(D2_MODULE_D2Multi_STRICT OFF)
set(D2_MODULE_D2Multi_GENERATED ON)
set(D2_MODULE_D2Multi_DATA "Source/globals/D2Multi.cpp")
set(D2_MODULE_D2Multi_UNIT_ORDER "original-address")
set(D2_MODULE_D2Multi_UNITS
    Source/D2Multi/ChatDlg.cpp  # 0043cc30, 15 runs
    Source/D2Multi/OOG.cpp  # 0043d9a0, 2 runs
    Source/D2Multi/OOGUtilities.cpp  # 0043da50, 9 runs
    Source/D2Multi/ComCallback.cpp  # 0043db90, 5 runs
    Source/D2Multi/OOG/ChatDlg.cpp  # 0043ed50, 2 runs
    Source/D2Multi/D2MultiRoot.cpp  # 0043f970, 6 runs
)

# D2MCPClient -> Storm (import): 1.13d D2MCPClient import table: 2 symbol(s) from Storm; also in 1.13c, 1.10f
# D2MCPClient -> Fog (import): 1.13d D2MCPClient import table: 16 symbol(s) from Fog; also in 1.13c, 1.10f
# D2MCPClient -> D2Lang (import): 1.13d D2MCPClient import table: 6 symbol(s) from D2Lang; 1.13d only
set(D2_MODULE_D2MCPClient_DIR "Source/D2MCPClient")
set(D2_MODULE_D2MCPClient_KIND "dll-module")
set(D2_MODULE_D2MCPClient_DEPENDS Storm Fog D2Lang)
set(D2_MODULE_D2MCPClient_IMPORTS Storm Fog D2Lang)
set(D2_MODULE_D2MCPClient_LOADS )
set(D2_MODULE_D2MCPClient_PUBLIC "Source/D2MCPClient/D2MCPClient.h")
set(D2_MODULE_D2MCPClient_SEES Fog Storm D2MCPClient D2Hell D2Lang D2Headers Redivivus)
set(D2_MODULE_D2MCPClient_EXCEPTIONS )
set(D2_MODULE_D2MCPClient_STRICT OFF)
set(D2_MODULE_D2MCPClient_GENERATED ON)
set(D2_MODULE_D2MCPClient_DATA "Source/globals/D2MCPClient.cpp")
set(D2_MODULE_D2MCPClient_UNIT_ORDER "original-address")
set(D2_MODULE_D2MCPClient_UNITS
    Source/D2MCPClient/McpConnect.cpp  # 00449ad0, 6 runs
)

# D2gfx -> Storm (import): 1.13d D2gfx import table: 4 symbol(s) from Storm; also in 1.13c, 1.10f
# D2gfx -> Fog (import): 1.13d D2gfx import table: 6 symbol(s) from Fog; also in 1.13c, 1.10f
# D2gfx -> D2Gdi (load): LoadLibrary + QueryInterface from D2gfx; the host sees the interface, not what it imports
# D2gfx -> D2DDraw (load): LoadLibrary + QueryInterface from D2gfx; the host sees the interface, not what it imports
# D2gfx -> D2Direct3D (load): LoadLibrary + QueryInterface from D2gfx; the host sees the interface, not what it imports
# D2gfx -> D2Glide (load): LoadLibrary + QueryInterface from D2gfx; the host sees the interface, not what it imports
set(D2_MODULE_D2gfx_DIR "Source/D2gfx")
set(D2_MODULE_D2gfx_KIND "dll-module")
set(D2_MODULE_D2gfx_DEPENDS Storm Fog D2Gdi D2DDraw D2Direct3D D2Glide)
set(D2_MODULE_D2gfx_IMPORTS Storm Fog)
set(D2_MODULE_D2gfx_LOADS D2Gdi D2DDraw D2Direct3D D2Glide)
set(D2_MODULE_D2gfx_PUBLIC "Source/D2gfx/D2gfx.public.h")
set(D2_MODULE_D2gfx_SEES Fog Storm D2gfx D2Glide D2DDraw D2Hell D2Direct3D D2Gdi D2Headers Redivivus)
set(D2_MODULE_D2gfx_EXCEPTIONS )
set(D2_MODULE_D2gfx_STRICT OFF)
set(D2_MODULE_D2gfx_GENERATED ON)
set(D2_MODULE_D2gfx_DATA "Source/globals/D2gfx.cpp")
set(D2_MODULE_D2gfx_UNIT_ORDER "original-address")
set(D2_MODULE_D2gfx_UNITS
    Source/D2gfx/D2GFX.cpp  # 004f5030, 7 runs
    Source/D2gfx/D2GfxHelperStrc.cpp  # 004f7e70, 1 run
)

# D2Win -> Storm (import): 1.13d D2Win import table: 18 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Win -> Fog (import): 1.13d D2Win import table: 27 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Win -> D2CMP (import): 1.13d D2Win import table: 14 symbol(s) from D2CMP; also in 1.13c, 1.10f
# D2Win -> D2Lang (import): 1.13d D2Win import table: 16 symbol(s) from D2Lang; also in 1.13c, 1.10f
# D2Win -> D2gfx (import): 1.13d D2Win import table: 38 symbol(s) from D2gfx; also in 1.13c, 1.10f
# D2Win -> D2Sound (import): 1.13d D2Win import table: 8 symbol(s) from D2Sound; also in 1.13c, 1.10f
set(D2_MODULE_D2Win_DIR "Source/D2Win")
set(D2_MODULE_D2Win_KIND "dll-module")
set(D2_MODULE_D2Win_DEPENDS Storm Fog D2CMP D2Lang D2gfx D2Sound)
set(D2_MODULE_D2Win_IMPORTS Storm Fog D2CMP D2Lang D2gfx D2Sound)
set(D2_MODULE_D2Win_LOADS )
set(D2_MODULE_D2Win_PUBLIC "Source/D2Win/D2Win.h")
set(D2_MODULE_D2Win_SEES Fog Storm D2gfx D2Win D2Sound D2Hell D2Lang D2CMP D2Headers Redivivus)
set(D2_MODULE_D2Win_EXCEPTIONS )
set(D2_MODULE_D2Win_STRICT OFF)
set(D2_MODULE_D2Win_GENERATED ON)
set(D2_MODULE_D2Win_DATA "Source/globals/D2Win.cpp")
set(D2_MODULE_D2Win_UNIT_ORDER "original-address")
set(D2_MODULE_D2Win_UNITS
    Source/D2Win/D2GFX.cpp  # 004f8fe0, 1 run
    Source/D2Win/Src/D2WinMain.cpp  # 004f9140, 2 runs
    Source/D2Win/Src/D2WinArchive.cpp  # 004fa9b0, 1 run
    Source/D2Win/Src/D2WinPalette.cpp  # 004fb010, 1 run
    Source/D2Win/Src/D2WinTextBox.cpp  # 004fb5a0, 1 run
    Source/D2Win/Src/D2WinImage.cpp  # 004fd060, 1 run
    Source/D2Win/Src/D2WinTimer.cpp  # 004fd820, 1 run
    Source/D2Win/Src/D2WinEditBox.cpp  # 004fda80, 1 run
    Source/D2Win/D2WinAnimImage.cpp  # 00500300, 1 run
    Source/D2Win/Src/D2WinButton.cpp  # 00500930, 1 run
    Source/D2Win/Src/D2WinFont.cpp  # 005013a0, 2 runs
    Source/D2Win/Src/D2Comp.cpp  # 005031d0, 2 runs
    Source/D2Win/Src/D2WinSmack.cpp  # 005068a0, 1 run
    Source/D2Win/Forms.cpp  # 00506c40, 3 runs
    Source/D2Win/Src/D2WinList.cpp  # 00506e60, 1 run
    Source/D2Win/Src/D2WinAccountList.cpp  # 00507850, 1 run
    Source/D2Win/Src/D2WinProgressBar.cpp  # 00507db0, 1 run
    Source/D2Win/Src/D2WinScrollbar.cpp  # 00507ee0, 1 run
    Source/D2Win/Src/D2WinPopup.cpp  # 005085a0, 1 run
)

# D2Glide -> Storm (import): 1.13d D2Glide import table: 5 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Glide -> Fog (import): 1.13d D2Glide import table: 7 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Glide -> D2CMP (import): 1.13d D2Glide import table: 10 symbol(s) from D2CMP; also in 1.13c, 1.10f
# D2Glide -> D2Sound (import): 1.13d D2Glide import table: 1 symbol(s) from D2Sound; also in 1.13c, 1.10f
set(D2_MODULE_D2Glide_DIR "Source/D2Glide")
set(D2_MODULE_D2Glide_KIND "dll-module")
set(D2_MODULE_D2Glide_DEPENDS Storm Fog D2CMP D2Sound)
set(D2_MODULE_D2Glide_IMPORTS Storm Fog D2CMP D2Sound)
set(D2_MODULE_D2Glide_LOADS )
set(D2_MODULE_D2Glide_PUBLIC "Source/D2Glide/D2Glide.h")
set(D2_MODULE_D2Glide_SEES Fog Storm D2gfx D2Glide D2Sound D2Hell D2CMP D2Headers Redivivus)
set(D2_MODULE_D2Glide_EXCEPTIONS )
set(D2_MODULE_D2Glide_STRICT OFF)
set(D2_MODULE_D2Glide_GENERATED ON)
set(D2_MODULE_D2Glide_DATA "Source/globals/D2Glide.cpp")
set(D2_MODULE_D2Glide_UNIT_ORDER "original-address")
set(D2_MODULE_D2Glide_UNITS
    Source/D2Glide/Renderer/Glide.cpp  # 00508f90, 1 run
    Source/D2Glide/glTextureCache.cpp  # 0050e890, 1 run
    Source/D2Glide/glSubtileCache.cpp  # 0050ed90, 1 run
    Source/D2Glide/glFloorCache.cpp  # 0050f320, 1 run
    Source/D2Glide/glBlocks.cpp  # 0050f870, 1 run
)

# D2DDraw -> Storm (import): 1.13d D2DDraw import table: 7 symbol(s) from Storm; also in 1.13c, 1.10f
# D2DDraw -> Fog (import): 1.13d D2DDraw import table: 9 symbol(s) from Fog; also in 1.13c, 1.10f
# D2DDraw -> D2CMP (import): 1.13d D2DDraw import table: 11 symbol(s) from D2CMP; also in 1.13c, 1.10f
# D2DDraw -> D2Sound (import): 1.13d D2DDraw import table: 1 symbol(s) from D2Sound; also in 1.13c, 1.10f
set(D2_MODULE_D2DDraw_DIR "Source/D2DDraw")
set(D2_MODULE_D2DDraw_KIND "dll-module")
set(D2_MODULE_D2DDraw_DEPENDS Storm Fog D2CMP D2Sound)
set(D2_MODULE_D2DDraw_IMPORTS Storm Fog D2CMP D2Sound)
set(D2_MODULE_D2DDraw_LOADS )
set(D2_MODULE_D2DDraw_PUBLIC "Source/D2DDraw/D2DDraw.h")
set(D2_MODULE_D2DDraw_SEES Fog Storm D2gfx D2DDraw D2Sound D2Hell D2CMP D2Headers Redivivus)
set(D2_MODULE_D2DDraw_EXCEPTIONS )
set(D2_MODULE_D2DDraw_STRICT OFF)
set(D2_MODULE_D2DDraw_GENERATED ON)
set(D2_MODULE_D2DDraw_DATA "Source/globals/D2DDraw.cpp")
set(D2_MODULE_D2DDraw_UNIT_ORDER "original-address")
set(D2_MODULE_D2DDraw_UNITS
    Source/D2DDraw/Renderer/DirectDraw.cpp  # 00510020, 5 runs
)

# D2Sound -> Storm (import): 1.13d D2Sound import table: 13 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Sound -> Fog (import): 1.13d D2Sound import table: 10 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Sound -> D2gfx (import): 1.13d D2Sound import table: 1 symbol(s) from D2gfx; also in 1.13c, 1.10f
set(D2_MODULE_D2Sound_DIR "Source/D2Sound")
set(D2_MODULE_D2Sound_KIND "dll-module")
set(D2_MODULE_D2Sound_DEPENDS Storm Fog D2gfx)
set(D2_MODULE_D2Sound_IMPORTS Storm Fog D2gfx)
set(D2_MODULE_D2Sound_LOADS )
set(D2_MODULE_D2Sound_PUBLIC "Source/D2Sound/D2Sound.h")
set(D2_MODULE_D2Sound_SEES Fog Storm D2gfx D2Sound D2Hell D2Headers Redivivus)
set(D2_MODULE_D2Sound_EXCEPTIONS )
set(D2_MODULE_D2Sound_STRICT OFF)
set(D2_MODULE_D2Sound_GENERATED ON)
set(D2_MODULE_D2Sound_DATA "Source/globals/D2Sound.cpp")
set(D2_MODULE_D2Sound_UNIT_ORDER "original-address")
set(D2_MODULE_D2Sound_UNITS
    Source/D2Sound/D2SoundRoot.cpp  # 00513a20, 4 runs
    Source/D2Sound/Sound.cpp  # 00513af0, 5 runs
    Source/D2Sound/Src/D2SoundFast.cpp  # 00514710, 2 runs
    Source/D2Sound/Src/D2SoundSmp.cpp  # 00515180, 2 runs
    Source/D2Sound/D2SoundUntil.cpp  # 00516950, 1 run
)

# D2Hell -> Storm (import): 1.13d import table
# D2Hell -> Fog (import): 1.13d import table
# D2Hell -> Game exception (MISC_OperatorNewShim): D2FILE_VerifySignatureWinTrust (00517ad6) and D2FILE_GetSignerOrganization (00517c30) are 1.14-only WinTrust code (no WINTRUST import in any 1.13d binary) and allocate through the program's global operator new (00405410, Game::Main)
set(D2_MODULE_D2Hell_DIR "Source/D2Hell")
set(D2_MODULE_D2Hell_KIND "dll-module")
set(D2_MODULE_D2Hell_DEPENDS Storm Fog)
set(D2_MODULE_D2Hell_IMPORTS Storm Fog)
set(D2_MODULE_D2Hell_LOADS )
set(D2_MODULE_D2Hell_PUBLIC "Source/D2Hell/D2Hell.h")
set(D2_MODULE_D2Hell_SEES Fog Game Storm D2Hell D2Headers Redivivus)
set(D2_MODULE_D2Hell_EXCEPTIONS Game)
set(D2_MODULE_D2Hell_STRICT OFF)
set(D2_MODULE_D2Hell_GENERATED ON)
set(D2_MODULE_D2Hell_DATA "Source/globals/D2Hell.cpp")
set(D2_MODULE_D2Hell_UNIT_ORDER "original-address")
set(D2_MODULE_D2Hell_UNITS
    Source/D2Hell/D2HellRoot.cpp  # 00516d8b (outside the module's runs), 4 runs
)

# Bnclient -> Storm (import): 1.13d Bnclient import table: 49 symbol(s) from Storm; also in 1.13c, 1.10f
# Bnclient -> Fog (import): 1.13d Bnclient import table: 20 symbol(s) from Fog; also in 1.13c, 1.10f
# Bnclient -> D2Lang (import): 1.13d Bnclient import table: 1 symbol(s) from D2Lang; also in 1.13c, 1.10f
# Bnclient -> D2Common (import): 1.13d Bnclient import table: 1 symbol(s) from D2Common; 1.13d only
set(D2_MODULE_Bnclient_DIR "Source/Bnclient")
set(D2_MODULE_Bnclient_KIND "dll-module")
set(D2_MODULE_Bnclient_DEPENDS Storm Fog D2Lang D2Common)
set(D2_MODULE_Bnclient_IMPORTS Storm Fog D2Lang D2Common)
set(D2_MODULE_Bnclient_LOADS )
set(D2_MODULE_Bnclient_PUBLIC "Source/Bnclient/Bnclient.h")
set(D2_MODULE_Bnclient_SEES Fog Storm D2Hell Bnclient D2Lang D2CMP D2Common D2Headers Redivivus)
set(D2_MODULE_Bnclient_EXCEPTIONS )
set(D2_MODULE_Bnclient_STRICT OFF)
set(D2_MODULE_Bnclient_GENERATED ON)
set(D2_MODULE_Bnclient_DATA "Source/globals/Bnclient.cpp")
set(D2_MODULE_Bnclient_UNIT_ORDER "original-address")
set(D2_MODULE_Bnclient_UNITS
    Source/Bnclient/BNGatewayAccess.cpp  # 00517e60, 1 run
    Source/Bnclient/BnclientRoot.cpp  # 00518b60, 6 runs
    Source/Bnclient/Debug.cpp  # 00518ba0, 4 runs
    Source/Bnclient/BnMessQueue.cpp  # 00518c20, 2 runs
    Source/Bnclient/Bnet.cpp  # 00518c70, 3 runs
    Source/Bnclient/Cache.cpp  # 00518de0, 3 runs
    Source/Bnclient/BnSend.cpp  # 0051a490, 5 runs
    Source/Bnclient/BnDownload.cpp  # 0051e440, 3 runs
    Source/Bnclient/net_sid.cpp  # 00520580, 4 runs
    Source/Bnclient/SRP.cpp  # 00527dc0, 4 runs
    Source/Bnclient/SHA1.cpp  # 00528c90, 1 run
    Source/Bnclient/SHA.cpp  # 0052a120, 1 run
)

# D2Lang -> Storm (import): 1.13d D2Lang import table: 9 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Lang -> Fog (import): 1.13d D2Lang import table: 10 symbol(s) from Fog; also in 1.13c, 1.10f
set(D2_MODULE_D2Lang_DIR "Source/D2Lang")
set(D2_MODULE_D2Lang_KIND "dll-module")
set(D2_MODULE_D2Lang_DEPENDS Storm Fog)
set(D2_MODULE_D2Lang_IMPORTS Storm Fog)
set(D2_MODULE_D2Lang_LOADS )
set(D2_MODULE_D2Lang_PUBLIC "Source/D2Lang/D2Lang.h")
set(D2_MODULE_D2Lang_SEES Fog Storm D2Hell D2Lang D2Headers Redivivus)
set(D2_MODULE_D2Lang_EXCEPTIONS )
set(D2_MODULE_D2Lang_STRICT ON)
set(D2_MODULE_D2Lang_GENERATED ON)
set(D2_MODULE_D2Lang_DATA "Source/globals/D2Lang.cpp")
set(D2_MODULE_D2Lang_UNIT_ORDER "original-address")
set(D2_MODULE_D2Lang_UNITS
    Source/D2Lang/StrTable/strtable.cpp  # 00524930, 1 run
    Source/D2Lang/D2LangRoot.cpp  # 00525fb0, 1 run
    Source/D2Lang/Unicode/UNISYS.cpp  # 005264f0, 1 run
)

# D2Net -> Storm (import): 1.13d D2Net import table: 2 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Net -> Fog (import): 1.13d D2Net import table: 37 symbol(s) from Fog; also in 1.13c, 1.10f
set(D2_MODULE_D2Net_DIR "Source/D2Net")
set(D2_MODULE_D2Net_KIND "dll-module")
set(D2_MODULE_D2Net_DEPENDS Storm Fog)
set(D2_MODULE_D2Net_IMPORTS Storm Fog)
set(D2_MODULE_D2Net_LOADS )
set(D2_MODULE_D2Net_PUBLIC "Source/D2Net/D2Net.h")
set(D2_MODULE_D2Net_SEES Fog Storm D2Hell D2Net D2Headers Redivivus)
set(D2_MODULE_D2Net_EXCEPTIONS )
set(D2_MODULE_D2Net_STRICT ON)
set(D2_MODULE_D2Net_GENERATED ON)
set(D2_MODULE_D2Net_DATA "Source/globals/D2Net.cpp")
set(D2_MODULE_D2Net_UNIT_ORDER "original-address")
set(D2_MODULE_D2Net_UNITS
    Source/D2Net/SRC/Client.cpp  # 0052a610, 1 run
    Source/D2Net/SRC/D2Net.cpp  # 0052afe0, 2 runs
    Source/D2Net/Game/Server.cpp  # 0052b050, 2 runs
)

# D2Game -> Storm (import): 1.13d D2Game import table: 10 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Game -> Fog (import): 1.13d D2Game import table: 30 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Game -> D2CMP (import): 1.13d D2Game import table: 1 symbol(s) from D2CMP; also in 1.13c, 1.10f
# D2Game -> D2Lang (import): 1.13d D2Game import table: 2 symbol(s) from D2Lang; also in 1.13c, 1.10f
# D2Game -> D2Net (import): 1.13d D2Game import table: 11 symbol(s) from D2Net; also in 1.13c, 1.10f
# D2Game -> D2Common (import): 1.13d D2Game import table: 716 symbol(s) from D2Common; also in 1.13c, 1.10f
set(D2_MODULE_D2Game_DIR "Source/D2Game")
set(D2_MODULE_D2Game_KIND "dll-module")
set(D2_MODULE_D2Game_DEPENDS Storm Fog D2CMP D2Lang D2Net D2Common)
set(D2_MODULE_D2Game_IMPORTS Storm Fog D2CMP D2Lang D2Net D2Common)
set(D2_MODULE_D2Game_LOADS )
set(D2_MODULE_D2Game_PUBLIC "Source/D2Game/D2Game.h")
set(D2_MODULE_D2Game_SEES Fog Storm D2Hell D2Lang D2Net D2Game D2CMP D2Common D2Headers Redivivus)
set(D2_MODULE_D2Game_EXCEPTIONS )
set(D2_MODULE_D2Game_STRICT OFF)
set(D2_MODULE_D2Game_GENERATED ON)
set(D2_MODULE_D2Game_DATA "Source/globals/D2Game.cpp")
set(D2_MODULE_D2Game_UNIT_ORDER "original-address")
set(D2_MODULE_D2Game_UNITS
    Source/D2Game/Archive.cpp  # 0045c370 (outside the module's runs), 1 run
    Source/D2Game/GAME/Server.cpp  # 0052be20, 23 runs
    Source/D2Game/D2GameRoot.cpp  # 0052cbb0, 19 runs
    Source/D2Game/GAME/stpl_SGAMEDATA.cpp  # 0052e470, 4 runs
    Source/D2Game/PLAYER/PlayerSave.cpp  # 00530f40, 2 runs
    Source/D2Game/PLAYER/Player.cpp  # 00534800, 1 run
    Source/D2Game/UNIT/sunitproxy.cpp  # 00535e80, 1 run
    Source/D2Game/GAME/Clients.cpp  # 005376c0, 9 runs
    Source/D2Game/ITEMS/ItemMode.cpp  # 00538350, 6 runs
    Source/D2Game/GAME/Level.cpp  # 0053a500, 4 runs
    Source/D2Game/GAME/Server/ServerCmd.cpp  # 0053b0e0, 2 runs
    Source/D2Game/PLAYER/PlayerModes.cpp  # 0053b210, 3 runs
    Source/D2Game/GAME/SCmd.cpp  # 0053b280, 4 runs
    Source/D2Game/GAME/CCmd.cpp  # 0053eff0, 1 run
    Source/D2Game/GAME/Arena.cpp  # 0053f470, 1 run
    Source/D2Game/UNIT/Party.cpp  # 0053ff00, 1 run
    Source/D2Game/GAME/Event.cpp  # 00540b70, 1 run
    Source/D2Game/UNIT/Monster.cpp  # 00541830, 1 run
    Source/D2Game/UNIT/SUnitInactive.cpp  # 00541890, 1 run
    Source/D2Game/Quests/QUESTS.cpp  # 00543520, 7 runs
    Source/D2Game/Objects/ObjectsRgn.cpp  # 00546c60, 1 run
    Source/D2Game/Monster/MonsterRegion.cpp  # 005475e0, 1 run
    Source/D2Game/PLAYER/PlayerMsg.cpp  # 00547f50, 2 runs
    Source/D2Game/Monsters/Spawn.cpp  # 0054dac0, 1 run
    Source/D2Game/Objects/Objects.cpp  # 0054f0d0, 3 runs
    Source/D2Game/UNIT/SUnit.cpp  # 00552d80, 3 runs
    Source/D2Game/ITEMS/Items/Drop.cpp  # 00555cc0, 3 runs
    Source/D2Game/MISSILES/Missiles.cpp  # 00557f10, 2 runs
    Source/D2Game/UNIT/PlayerList.cpp  # 0055b110, 1 run
    Source/D2Game/GAME/Task.cpp  # 00564210, 5 runs
    Source/D2Game/PLAYER/PlayerTrade.cpp  # 00564cd0, 1 run
    Source/D2Game/PLAYER/PlayerSave2.cpp  # 00568d60, 1 run
    Source/D2Game/SKILLS/SkillsServer.cpp  # 0056b3e0, 2 runs
    Source/D2Game/UNIT/SUnitMsg.cpp  # 005706a0, 1 run
    Source/D2Game/PLAYER/PlayerIntro.cpp  # 00572360, 1 run
    Source/D2Game/Monster/MonsterAI.cpp  # 005726c0, 1 run
    Source/D2Game/Monster/Monster.cpp  # 00573470, 1 run
    Source/D2Game/PLAYER/PlayerPets.cpp  # 00574410, 2 runs
    Source/D2Game/UNIT/SUNITNPC.cpp  # 00576070, 1 run
    Source/D2Game/UNIT/SUnitDmg.cpp  # 0057a830, 1 run
    Source/D2Game/Objects/ObjectsMode.cpp  # 00581250, 1 run
    Source/D2Game/Quests/QUESTS/Quest40.cpp  # 00586b30, 2 runs
    Source/D2Game/Quests/QUESTS/A5Q1.cpp  # 00586c80, 1 run
    Source/D2Game/Quests/QUESTS/A5Q2.cpp  # 005879d0, 1 run
    Source/D2Game/Quests/QUESTS/A5Q3.cpp  # 00588eb0, 2 runs
    Source/D2Game/Quests/QUESTS/A5Q4.cpp  # 0058aea0, 1 run
    Source/D2Game/Quests/QUESTS/A5Q5.cpp  # 0058bd50, 1 run
    Source/D2Game/Quests/QUESTS/A5Q6.cpp  # 0058d770, 1 run
    Source/D2Game/Ai/AiGeneral.cpp  # 0058ebc0, 1 run
    Source/D2Game/Quests/QUESTS/Quest37.cpp  # 0058f870, 1 run
    Source/D2Game/Quests/QUESTS/A1Q0.cpp  # 0058fa90, 1 run
    Source/D2Game/Quests/QUESTS/A1Q1.cpp  # 0058fbe0, 1 run
    Source/D2Game/Quests/QUESTS/A1Q2.cpp  # 00590810, 1 run
    Source/D2Game/Quests/QUESTS/A1Q3.cpp  # 005912e0, 2 runs
    Source/D2Game/Quests/QUESTS/A1Q4.cpp  # 00592090, 2 runs
    Source/D2Game/Quests/QUESTS/A1Q5.cpp  # 00594740, 1 run
    Source/D2Game/Quests/QUESTS/A1Q6.cpp  # 00595a50, 1 run
    Source/D2Game/Quests/QUESTS/A1Q7.cpp  # 00596a90, 1 run
    Source/D2Game/UNIT/Item.cpp  # 005973f0, 2 runs
    Source/D2Game/Monster/MonsterMsg.cpp  # 00597d70, 1 run
    Source/D2Game/Quests/QUESTS/Quest38.cpp  # 005983e0, 1 run
    Source/D2Game/Quests/QUESTS/A2Q0.cpp  # 00598640, 1 run
    Source/D2Game/Quests/QUESTS/A2Q1.cpp  # 005988b0, 1 run
    Source/D2Game/Quests/QUESTS/A2Q2.cpp  # 00599420, 2 runs
    Source/D2Game/Quests/QUESTS/A2Q3.cpp  # 00599f50, 4 runs
    Source/D2Game/Quests/QUESTS/A2Q4.cpp  # 0059aca0, 2 runs
    Source/D2Game/Quests/QUESTS/A2Q5.cpp  # 0059bb40, 1 run
    Source/D2Game/Quests/QUESTS/A2Q6.cpp  # 0059c330, 3 runs
    Source/D2Game/Quests/QUESTS/A2Q7.cpp  # 0059d7c0, 3 runs
    Source/D2Game/Quests/QUESTS/A2Q8.cpp  # 0059e3c0, 1 run
    Source/D2Game/ITEMS/Items.cpp  # 0059f840, 1 run
    Source/D2Game/Monster/Monsters.cpp  # 005a0000, 4 runs
    Source/D2Game/Monsters/Umod.cpp  # 005a0ce0, 1 run
    Source/D2Game/PLAYER/PlayerScreen.cpp  # 005a5690, 1 run
    Source/D2Game/Monster/MonsterMode.cpp  # 005a6190, 2 runs
    Source/D2Game/GAME/Event/Dispatch/Monster.cpp  # 005a7ef0, 1 run
    Source/D2Game/MISSILES/MissilesMode/SrvDoFunc.cpp  # 005a8730, 1 run
    Source/D2Game/MISSILES/MissilesMode.cpp  # 005a88e0, 3 runs
    Source/D2Game/Monster/AI/AI.cpp  # 005b0bd0, 11 runs
    Source/D2Game/GAME/Targets.cpp  # 005b1880, 1 run
    Source/D2Game/Monster/Create.cpp  # 005b1bc0, 2 runs
    Source/D2Game/Quests/QUESTS/A4Q0.cpp  # 005b36a0, 1 run
    Source/D2Game/Quests/QUESTS/A4Q1.cpp  # 005b37f0, 1 run
    Source/D2Game/Quests/QUESTS/A4Q2.cpp  # 005b4390, 2 runs
    Source/D2Game/Quests/QUESTS/A4Q3.cpp  # 005b5750, 2 runs
    Source/D2Game/Quests/QUESTS/A4Q4.cpp  # 005b6940, 1 run
    Source/D2Game/Quests/QUESTS/Quest39.cpp  # 005b6c60, 1 run
    Source/D2Game/Quests/QUESTS/A3Q0.cpp  # 005b6ed0, 1 run
    Source/D2Game/Quests/QUESTS/A3Q1.cpp  # 005b70b0, 2 runs
    Source/D2Game/Quests/QUESTS/A3Q2.cpp  # 005b7f90, 3 runs
    Source/D2Game/Quests/QUESTS/A3Q3.cpp  # 005b8b20, 1 run
    Source/D2Game/Quests/QUESTS/A3Q4.cpp  # 005b9d90, 1 run
    Source/D2Game/Quests/QUESTS/A3Q5.cpp  # 005bac70, 1 run
    Source/D2Game/Quests/QUESTS/A3Q6.cpp  # 005bbf80, 1 run
    Source/D2Game/Quests/QUESTS/A3Q7.cpp  # 005bce80, 2 runs
    Source/D2Game/SKILLS/SkillItem.cpp  # 005bdb20, 2 runs
    Source/D2Game/UNIT/SUnitEvent.cpp  # 005c0a90, 1 run
    Source/D2Game/SKILLS/Items.cpp  # 005c0d00, 1 run
    Source/D2Game/SKILLS/SkillNec.cpp  # 005c2ff0, 1 run
    Source/D2Game/SKILLS/SkillDruid.cpp  # 005c6910, 2 runs
    Source/D2Game/SKILLS/SkillSor.cpp  # 005c8bc0, 2 runs
    Source/D2Game/SKILLS/SkillMonst.cpp  # 005cae40, 1 run
    Source/D2Game/SKILLS/SkillPal.cpp  # 005ce790, 1 run
    Source/D2Game/SKILLS/SkillEMonst.cpp  # 005d10c0, 1 run
    Source/D2Game/SKILLS/SkillAss.cpp  # 005d30a0, 2 runs
    Source/D2Game/SKILLS/SkillBar.cpp  # 005d7ea0, 1 run
    Source/D2Game/SKILLS/SkillAma.cpp  # 005da8b0, 1 run
    Source/D2Game/Monster/AI/BloodRaven.cpp  # 005df680, 1 run
    Source/D2Game/Monster/AI/Baal.cpp  # 005ef5e0, 2 runs
    Source/D2Game/Seed.cpp  # no function
)

# D2CMP -> Storm (import): 1.13d D2CMP import table: 13 symbol(s) from Storm; also in 1.13c, 1.10f
# D2CMP -> Fog (import): 1.13d D2CMP import table: 19 symbol(s) from Fog; also in 1.13c, 1.10f
set(D2_MODULE_D2CMP_DIR "Source/D2CMP")
set(D2_MODULE_D2CMP_KIND "dll-module")
set(D2_MODULE_D2CMP_DEPENDS Storm Fog)
set(D2_MODULE_D2CMP_IMPORTS Storm Fog)
set(D2_MODULE_D2CMP_LOADS )
set(D2_MODULE_D2CMP_PUBLIC "Source/D2CMP/D2CMP.h")
set(D2_MODULE_D2CMP_SEES Fog Storm D2Hell D2CMP D2Headers Redivivus)
set(D2_MODULE_D2CMP_EXCEPTIONS )
set(D2_MODULE_D2CMP_STRICT OFF)
set(D2_MODULE_D2CMP_GENERATED ON)
set(D2_MODULE_D2CMP_DATA "Source/globals/D2CMP.cpp")
set(D2_MODULE_D2CMP_UNIT_ORDER "original-address")
set(D2_MODULE_D2CMP_UNITS
    Source/D2CMP/SRC/SpriteCache.cpp  # 005fde10, 5 runs
    Source/D2CMP/D2CMPRoot.cpp  # 005fde50, 6 runs
    Source/D2CMP/SRC/TileProjects.cpp  # 006006b0, 1 run
    Source/D2CMP/Pallete.cpp  # 006008d0, 1 run
    Source/D2CMP/SRC/CelCmp.cpp  # 00600c90, 12 runs
    Source/D2CMP/SRC/stpl_CELLIST.cpp  # 00601b80, 17 runs
    Source/D2CMP/SRC/stpl_TCELENTRY.cpp  # 00601fa0, 6 runs
    Source/D2CMP/SRC/CelCmp/TCELENTRY.cpp  # 00602a40, 1 run
    Source/D2CMP/SRC/CelCmp/stpl_CELLIST.cpp  # 00602aa0, 5 runs
    Source/D2CMP/SRC/CelCmp/CELLIST.cpp  # 00602b20, 3 runs
    Source/D2CMP/SRC/CelCmp/stpl_TCELENTRY.cpp  # 00602bf0, 1 run
    Source/D2CMP/SRC/stpl_MappingEntryNode.cpp  # 00604070, 1 run
    Source/D2CMP/DUNGEON/Dungeon.cpp  # 00604a40, 1 run
    Source/D2CMP/Draw.cpp  # 00604df0, 1 run
    Source/D2CMP/SRC/FastCmp.cpp  # 00606a10, 1 run
    Source/D2CMP/SRC/LRUCache.cpp  # 00609020, 1 run
    Source/D2CMP/SRC/Tilecmp.cpp  # 006094b0, 1 run
    Source/D2CMP/SRC/GfxHash.cpp  # 0060a9d0, 2 runs
    Source/D2CMP/SRC/CelDataHash.cpp  # 0060aeb0, 1 run
    Source/D2CMP/SRC/Codec.cpp  # 0060b200, 1 run
    Source/D2CMP/SRC/FindTiles.cpp  # 0060cd70, 1 run
    Source/D2CMP/SRC/Raw.cpp  # 0060d190, 1 run
    Source/D2CMP/SRC/SubTile.cpp  # 0060d310, 2 runs
)

# D2Common -> Storm (import): 1.13d D2Common import table: 19 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Common -> Fog (import): 1.13d D2Common import table: 42 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Common -> D2CMP (import): 1.13d D2Common import table: 8 symbol(s) from D2CMP; also in 1.13c, 1.10f
# D2Common -> D2Lang (import): 1.13d D2Common import table: 11 symbol(s) from D2Lang; also in 1.13c, 1.10f
set(D2_MODULE_D2Common_DIR "Source/D2Common")
set(D2_MODULE_D2Common_KIND "dll-module")
set(D2_MODULE_D2Common_DEPENDS Storm Fog D2CMP D2Lang)
set(D2_MODULE_D2Common_IMPORTS Storm Fog D2CMP D2Lang)
set(D2_MODULE_D2Common_LOADS )
set(D2_MODULE_D2Common_PUBLIC "Source/D2Common/D2Common.h")
set(D2_MODULE_D2Common_SEES Fog Storm D2Hell D2Lang D2CMP D2Common D2Headers Redivivus)
set(D2_MODULE_D2Common_EXCEPTIONS )
set(D2_MODULE_D2Common_STRICT OFF)
set(D2_MODULE_D2Common_GENERATED ON)
set(D2_MODULE_D2Common_DATA "Source/globals/D2Common.cpp")
set(D2_MODULE_D2Common_UNIT_ORDER "original-address")
set(D2_MODULE_D2Common_UNITS
    Source/D2Common/stpl_HASHKEY_STRI.cpp  # 0060dc70, 2 runs
    Source/D2Common/Seed.cpp  # 0060dcf0, 3 runs
    Source/D2Common/Chat/Ignorelist/ChatIgnoreList.cpp  # 0060dd50, 2 runs
    Source/D2Common/Chat/Ignorelist/stpl_HASHKEY_STRI.cpp  # 0060dd90, 1 run
    Source/D2Common/Chat/Ignorelist.cpp  # 0060ddb0, 11 runs
    Source/D2Common/Chat/Ignorelist/IgnoreList.cpp  # 0060dde0, 11 runs
    Source/D2Common/Chat/stpl_StringEntryNode.cpp  # 0060e0e0, 3 runs
    Source/D2Common/Chat/stpl_MappingEntryNode.cpp  # 0060e110, 3 runs
    Source/D2Common/Chat/stpl_HASHKEY_STRI.cpp  # 0060e130, 1 run
    Source/D2Common/Drlg/DrlgGrid.cpp  # 0060e290, 20 runs
    Source/D2Common/Drlg/stpl_StringEntryNode.cpp  # 0060e2a0, 10 runs
    Source/D2Common/Drlg/stpl_MappingEntryNode.cpp  # 0060e300, 9 runs
    Source/D2Common/Chat/Ignorelist/MappingEntryNode.cpp  # 0060ed30, 3 runs
    Source/D2Common/Chat/Ignorelist/stpl_StringEntryNode.cpp  # 0060ed60, 15 runs
    Source/D2Common/Chat/Ignorelist/stpl_MappingEntryNode.cpp  # 0060ee60, 19 runs
    Source/D2Common/Chat/Ignorelist/StringEntryNode.cpp  # 0060f160, 1 run
    Source/D2Common/Logging/ProfCore.cpp  # 00611600, 4 runs
    Source/D2Common/INVENTORY/Inventory.cpp  # 00611610, 11 runs
    Source/D2Common/DATATBLS/DataTbls.cpp  # 00611760, 2 runs
    Source/D2Common/DUNGEON/Dungeon.cpp  # 006194a0, 3 runs
    Source/D2Common/D2CommonRoot.cpp  # 0061bb50, 5 runs
    Source/D2Common/Environment.cpp  # 0061bb80, 1 run
    Source/D2Common/DATATBLS/LvlTbls.cpp  # 0061c540, 1 run
    Source/D2Common/Units.cpp  # 00620100, 2 runs
    Source/D2Common/Logging.cpp  # 00624700, 3 runs
    Source/D2Common/Src/LogManager.cpp  # 00624b50, 1 run
    Source/D2Common/Stats/StatsEx.cpp  # 00624bd0, 1 run
    Source/D2Common/ITEMS/Items.cpp  # 00627c20, 5 runs
    Source/D2Common/DATATBLS/ItemTbls.cpp  # 006314a0, 3 runs
    Source/D2Common/DATATBLS/ItemStats.cpp  # 00639db0, 1 run
    Source/D2Common/Monsters/MONSTERS.cpp  # 0063db80, 3 runs
    Source/D2Common/DATATBLS/ObjectTbls.cpp  # 0063f690, 1 run
    Source/D2Common/Collision.cpp  # 00641530, 3 runs
    Source/D2Common/Drlg/Drlg.cpp  # 00641f20, 18 runs
    Source/D2Common/Txt/Cube.cpp  # 006429d0, 11 runs
    Source/D2Common/Drlg/Transform.cpp  # 00643260, 1 run
    Source/D2Common/Txt/Skills.cpp  # 00643580, 7 runs
    Source/D2Common/PATH/Path.cpp  # 00647fb0, 3 runs
    Source/D2Common/PATH/DynamicPath.cpp  # 00648460, 1 run
    Source/D2Common/Units/Missile.cpp  # 0064a100, 2 runs
    Source/D2Common/Unit.cpp  # 0064bf60, 1 run
    Source/D2Common/COMPOSIT/Composit.cpp  # 0064efe0, 1 run
    Source/D2Common/DATATBLS/MonsterTbls.cpp  # 00650eb0, 4 runs
    Source/D2Common/Units/UnitFinds.cpp  # 0065a590, 1 run
    Source/D2Common/DATATBLS/TokTbls.cpp  # 0065adc0, 2 runs
    Source/D2Common/QuestRecord.cpp  # 0065c310, 1 run
    Source/D2Common/ITEMS/ItemMods.cpp  # 0065c660, 1 run
    Source/D2Common/DATATBLS/BeltsTbls.cpp  # 00660290, 1 run
    Source/D2Common/Waypoints.cpp  # 00660d90, 1 run
    Source/D2Common/Chat.cpp  # 00661110, 2 runs
    Source/D2Common/Text.cpp  # 00661240, 1 run
    Source/D2Common/DATATBLS/MissileTbls.cpp  # 006617b0, 1 run
    Source/D2Common/Merc.cpp  # 006632c0, 2 runs
    Source/D2Common/Item.cpp  # 00663a20, 1 run
    Source/D2Common/Units/MissileStream.cpp  # 00663d40, 1 run
    Source/D2Common/DATATBLS/OverlayTbls.cpp  # 00664150, 1 run
    Source/D2Common/DATATBLS/ArenaTbls.cpp  # 006649b0, 2 runs
    Source/D2Common/Drlg/Preset.cpp  # 00665930, 5 runs
    Source/D2Common/DATATBLS/FieldTbls.cpp  # 0066a2b0, 2 runs
    Source/D2Common/DATATBLS/AnimTbls.cpp  # 0066a770, 1 run
    Source/D2Common/Drlg/DrlgRoom.cpp  # 0066ab00, 3 runs
    Source/D2Common/Drlg/DrlgWarp.cpp  # 0066af30, 1 run
    Source/D2Common/Drlg/DrlgLogic.cpp  # 0066c580, 1 run
    Source/D2Common/Drlg/DrlgAnim.cpp  # 0066d290, 2 runs
    Source/D2Common/Drlg/RoomTile.cpp  # 0066d9e0, 1 run
    Source/D2Common/Drlg/Outdoors.cpp  # 0066f380, 5 runs
    Source/D2Common/Drlg/TileSub.cpp  # 006704e0, 2 runs
    Source/D2Common/Drlg/Maze.cpp  # 00670880, 3 runs
    Source/D2Common/Drlg/Maze/Act3/Durance.cpp  # 00672f60, 1 run
    Source/D2Common/Drlg/Maze/Act5/IceCaves.cpp  # 00673530, 1 run
    Source/D2Common/Drlg/OutPlace.cpp  # 006755c0, 4 runs
    Source/D2Common/Drlg/Path.cpp  # 00678b10, 1 run
    Source/D2Common/Drlg/DrlgVer.cpp  # 0067cdd0, 1 run
    Source/D2Common/Drlg/OutRoom.cpp  # 0067d610, 1 run
    Source/D2Common/Drlg/OutSiege.cpp  # 0067da70, 1 run
    Source/D2Common/Drlg/OutJung.cpp  # 0067e910, 2 runs
    Source/D2Common/Drlg/OutDesr.cpp  # 0067f470, 1 run
    Source/D2Common/Drlg/OutWild.cpp  # 0067fc70, 1 run
)

# D2Direct3D -> Storm (import): 1.13d D2Direct3D import table: 7 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Direct3D -> Fog (import): 1.13d D2Direct3D import table: 9 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Direct3D -> D2CMP (import): 1.13d D2Direct3D import table: 10 symbol(s) from D2CMP; also in 1.13c, 1.10f
# D2Direct3D -> D2Sound (import): 1.13d D2Direct3D import table: 1 symbol(s) from D2Sound; also in 1.13c, 1.10f
set(D2_MODULE_D2Direct3D_DIR "Source/D2Direct3D")
set(D2_MODULE_D2Direct3D_KIND "dll-module")
set(D2_MODULE_D2Direct3D_DEPENDS Storm Fog D2CMP D2Sound)
set(D2_MODULE_D2Direct3D_IMPORTS Storm Fog D2CMP D2Sound)
set(D2_MODULE_D2Direct3D_LOADS )
set(D2_MODULE_D2Direct3D_PUBLIC "Source/D2Direct3D/D2Direct3D.h")
set(D2_MODULE_D2Direct3D_SEES Fog Storm D2gfx D2Sound D2Hell D2CMP D2Direct3D D2Headers Redivivus)
set(D2_MODULE_D2Direct3D_EXCEPTIONS )
set(D2_MODULE_D2Direct3D_STRICT OFF)
set(D2_MODULE_D2Direct3D_GENERATED ON)
set(D2_MODULE_D2Direct3D_DATA "Source/globals/D2Direct3D.cpp")
set(D2_MODULE_D2Direct3D_UNIT_ORDER "original-address")
set(D2_MODULE_D2Direct3D_UNITS
    Source/D2Direct3D/Renderer/Direct3D.cpp  # 006b1b60, 4 runs
    Source/D2Direct3D/d3dTextureCache.cpp  # 006ba420, 1 run
    Source/D2Direct3D/d3dSubtileCache.cpp  # 006ba9a0, 1 run
    Source/D2Direct3D/d3dFloorCache.cpp  # 006bafc0, 1 run
)

# D2Gdi -> Storm (import): 1.13d D2Gdi import table: 3 symbol(s) from Storm; also in 1.13c, 1.10f
# D2Gdi -> Fog (import): 1.13d D2Gdi import table: 4 symbol(s) from Fog; also in 1.13c, 1.10f
# D2Gdi -> D2CMP (import): 1.13d D2Gdi import table: 11 symbol(s) from D2CMP; also in 1.13c, 1.10f
# D2Gdi -> D2gfx (import): 1.13d D2Gdi import table: 1 symbol(s) from D2gfx; also in 1.13c
set(D2_MODULE_D2Gdi_DIR "Source/D2Gdi")
set(D2_MODULE_D2Gdi_KIND "dll-module")
set(D2_MODULE_D2Gdi_DEPENDS Storm Fog D2CMP D2gfx)
set(D2_MODULE_D2Gdi_IMPORTS Storm Fog D2CMP D2gfx)
set(D2_MODULE_D2Gdi_LOADS )
set(D2_MODULE_D2Gdi_PUBLIC "Source/D2Gdi/D2Gdi.h")
set(D2_MODULE_D2Gdi_SEES Fog Storm D2gfx D2Hell D2CMP D2Gdi D2Headers Redivivus)
set(D2_MODULE_D2Gdi_EXCEPTIONS )
set(D2_MODULE_D2Gdi_STRICT OFF)
set(D2_MODULE_D2Gdi_GENERATED ON)
set(D2_MODULE_D2Gdi_DATA "Source/globals/D2Gdi.cpp")
set(D2_MODULE_D2Gdi_UNIT_ORDER "original-address")
set(D2_MODULE_D2Gdi_UNITS
    Source/D2Gdi/Renderer/Windowed.cpp  # 006c7a70, 4 runs
)

set(D2_MODULE_D2Headers_DIR "Source/D2Headers")
set(D2_MODULE_D2Headers_KIND "headers-only")
set(D2_MODULE_D2Headers_DEPENDS )
set(D2_MODULE_D2Headers_IMPORTS )
set(D2_MODULE_D2Headers_LOADS )
set(D2_MODULE_D2Headers_PUBLIC "")
set(D2_MODULE_D2Headers_SEES D2Hell D2Headers Redivivus)
set(D2_MODULE_D2Headers_EXCEPTIONS )
set(D2_MODULE_D2Headers_STRICT OFF)
set(D2_MODULE_D2Headers_GENERATED ON)
set(D2_MODULE_D2Headers_DATA "")
set(D2_MODULE_D2Headers_UNIT_ORDER "path")
set(D2_MODULE_D2Headers_UNITS )

# Redivivus -> libbacktrace (import): Redivivus/d2_crash.cpp symbolises the stack with libbacktrace
set(D2_MODULE_Redivivus_DIR "Redivivus")
set(D2_MODULE_Redivivus_KIND "ours")
set(D2_MODULE_Redivivus_DEPENDS libbacktrace)
set(D2_MODULE_Redivivus_IMPORTS libbacktrace)
set(D2_MODULE_Redivivus_LOADS )
set(D2_MODULE_Redivivus_PUBLIC "")
set(D2_MODULE_Redivivus_SEES crashy Fog Game Storm D2Client D2Launch D2Multi D2MCPClient D2gfx D2Win D2Glide D2DDraw D2Sound D2Hell Bnclient D2Lang D2Net D2Game D2CMP D2Common D2Direct3D D2Gdi D2Headers Redivivus libbacktrace)
set(D2_MODULE_Redivivus_EXCEPTIONS )
set(D2_MODULE_Redivivus_STRICT OFF)
set(D2_MODULE_Redivivus_GENERATED OFF)
set(D2_MODULE_Redivivus_DATA "")
set(D2_MODULE_Redivivus_UNIT_ORDER "path")
set(D2_MODULE_Redivivus_UNITS )

set(D2_MODULE_libbacktrace_DIR "3rdparty/libbacktrace")
set(D2_MODULE_libbacktrace_KIND "3rdparty")
set(D2_MODULE_libbacktrace_DEPENDS )
set(D2_MODULE_libbacktrace_IMPORTS )
set(D2_MODULE_libbacktrace_LOADS )
set(D2_MODULE_libbacktrace_PUBLIC "")
set(D2_MODULE_libbacktrace_SEES D2Hell D2Headers Redivivus libbacktrace)
set(D2_MODULE_libbacktrace_EXCEPTIONS )
set(D2_MODULE_libbacktrace_STRICT OFF)
set(D2_MODULE_libbacktrace_GENERATED OFF)
set(D2_MODULE_libbacktrace_DATA "")
set(D2_MODULE_libbacktrace_UNIT_ORDER "path")
set(D2_MODULE_libbacktrace_UNITS )
