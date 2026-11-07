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
