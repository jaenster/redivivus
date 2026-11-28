# The Mac build: the Mac-only code of 1.14d, compiled, not linked (included from
# CMakeLists.txt when the target is Apple; toolchain cmake/mac-i386-clang425.cmake).
#
# d2_mac_objs  StormMac, the Mac Storm project (Source/StormMac/), the game's Mac-only
#              module (Source/MacSpecific/Source/: the Mac shell and CD2Textures) and the
#              OpenGL renderer (Source/D2OpenGL/Src) on the 10.7 SDK. The renderer's Win32/WGL
#              and Win32Posix restorations (MacSdkOnWin32.cpp, MacSdkOnPosix.cpp, oglSdl.cpp) and
#              its D2gfx adapter (D2gfxRenderer.cpp) are the other builds' alone. The 1.14d Mac flags:
#              -Os -fstack-protector, libc++, Pascal string literals (Xcode's
#              default; "\p" literals sit in __TEXT,__const).
#
# clang-425 is a C++11 compiler: units that reach the shared tree's C++17 nested namespace
# definitions compile through cmake/mac-cxx.ts, which rewrites them (node, as cmp.ts).

file(GLOB_RECURSE D2_MAC_SOURCES CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/Source/StormMac/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/StormMac/*.CPP"
    "${CMAKE_SOURCE_DIR}/Source/StormMac/*.mm"
    "${CMAKE_SOURCE_DIR}/Source/D2gfx/Mac/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/D2gfx/Mac/*.mm"
    "${CMAKE_SOURCE_DIR}/Source/D2Hell/Mac/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/MacSpecific/Source/MacShell/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/MacSpecific/Source/CD2Textures/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/D2OpenGL/Src/*.cpp")
list(FILTER D2_MAC_SOURCES EXCLUDE REGEX "/Source/D2OpenGL/Src/(MacSdkOnWin32|MacSdkOnPosix|oglSdl|D2gfxRenderer)\\.cpp$")
# Win32Posix is the modern build's layer (cmake/mac-modern.cmake), not Storm's Mac one.
list(FILTER D2_MAC_SOURCES EXCLUDE REGEX "/Source/StormMac/Win32Posix/")
list(REMOVE_DUPLICATES D2_MAC_SOURCES)
list(SORT D2_MAC_SOURCES)
# SRegMac.mm is Objective-C++; the project enables C and C++ only.
set_source_files_properties(${D2_MAC_SOURCES} PROPERTIES LANGUAGE CXX)
foreach(_src ${D2_MAC_SOURCES})
    if(_src MATCHES "\\.mm$")
        set_source_files_properties(${_src} PROPERTIES COMPILE_FLAGS "-x objective-c++")
    endif()
endforeach()

add_library(d2_mac_objs OBJECT ${D2_MAC_SOURCES})
find_program(D2_NODE node REQUIRED)
set_target_properties(d2_mac_objs PROPERTIES
    CXX_COMPILER_LAUNCHER "${D2_NODE};${CMAKE_SOURCE_DIR}/cmake/mac-cxx.ts")
target_include_directories(d2_mac_objs PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/Source/D2Headers")
# The target axes (Source/D2Headers/D2Arch.h): Mac OS X, i386, the client.
target_compile_definitions(d2_mac_objs PRIVATE D2_OS_MAC=1 D2_MAC_CARBON=1 D2_ARCH_X86=1 D2_TARGET_CLIENT=1)
target_compile_options(d2_mac_objs PRIVATE
    -Os -fstack-protector -fpascal-strings
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11 -stdlib=libc++>
    -Werror=implicit-function-declaration)

# d2_mac_shared_objs  the shared game code the 1.14d Mac binary carries beside the Mac-only
#              code (D2Common, D2Game, D2Client, Fog, Storm, D2Win ...), compiled with the Mac
#              flags against Storm's Win32-on-Mac layer (Win32Mac.h) as the original was. Its
#              own target, outside `all`, so d2_mac_objs stays a clean gate while the shared
#              tree is brought up: `ninja -k 0 d2_mac_shared_objs` measures it.
set(D2_MAC_SHARED_MODULES
    D2Common D2CMP D2Lang D2Game D2Client Fog Storm D2Win D2gfx D2Sound D2Net D2Launch D2Hell
    D2Multi D2MCPClient Bnclient Game)
set(D2_MAC_SHARED_SOURCES)
foreach(m IN LISTS D2_MAC_SHARED_MODULES)
    file(GLOB_RECURSE _m_sources CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "${CMAKE_SOURCE_DIR}/Source/${m}/*.cpp")
    list(APPEND D2_MAC_SHARED_SOURCES ${_m_sources})
    if(EXISTS "${CMAKE_SOURCE_DIR}/Source/globals/${m}.cpp")
        list(APPEND D2_MAC_SHARED_SOURCES "${CMAKE_SOURCE_DIR}/Source/globals/${m}.cpp")
    endif()
endforeach()
list(APPEND D2_MAC_SHARED_SOURCES "${CMAKE_SOURCE_DIR}/Source/globals/globals.cpp")
# The Mac variants of D2gfx and D2Hell are Mac-only code, in d2_mac_objs.
list(FILTER D2_MAC_SHARED_SOURCES EXCLUDE REGEX "/Source/(D2gfx|D2Hell)/Mac/")
# Storm's Mac build is Source/StormMac/SOURCE (see Source/StormMac/README.md): the Mac binary
# implements SStr*, SMem*, SFile*, SLog*, SReg*, SErr*, SBmp*, SCmd*, SMsg* and the signature
# check in StormMac's SSTR/SMEM/SFILE/SLOG/SRegMac/SERR/SBMP/SCMD/SMSG/SSignature units (all in
# d2_mac_objs, and the win-twin rows of its coverage.tsv name Win32-only counterparts: the
# registry, the Win32 file API, WinTrust, the window-handle list). The same-named Source/Storm
# units are the Windows implementations and are not in the Mac binary; compiling them here would
# only duplicate StormMac's API against Win32 calls the Mac layer does not have.
list(FILTER D2_MAC_SHARED_SOURCES EXCLUDE REGEX
    "/Source/Storm/(WindowHandle|Source/(SBMP|SCMD|SErr|SFile|SLog|SMem|SReg|SSignature|SSTR))\\.cpp$")
# StormMac/SOURCE also holds the Mac binary's SBig.cpp, SCOMP.CPP and SEVT.CPP, and the binary has one
# copy of each: its __FILE__ strings are ".../StormMac/Storm Mac/SOURCE/SBig.cpp" (SBigNew), "SCOMP.CPP" and
# "SEVT.CPP", no Storm/Source path, and their functions are StormMac's (0000a629..0000bf46 SBig,
# 0000eb42.. SCOMP, ..00011f70 SEVT; asm_matching/mac/coverage.tsv). The same-named Source/Storm units are the
# Windows build's. StormRoot.cpp's one function, SMsgBreakHandlerChain, is StormMac's SMSG.CPP (00018cdb).
# Storm's third-party code: the Mac SCOMP.CPP decompresses PKWARE implode with blast.c (00039fdf..0003a591),
# so PKWARE's implode.cpp/explode.cpp are not in the binary; its zlib is inflate only (adler32, crc32,
# inffast, inflate, inftrees, uncompr, zutil at 000371f9..00039fdf; no deflate, no "deflate" string), so
# deflate.cpp and trees.cpp are not either.
list(FILTER D2_MAC_SHARED_SOURCES EXCLUDE REGEX
    "/Source/Storm/(StormRoot|Source/(SBig|SComp|SEVT))\\.cpp$")
list(FILTER D2_MAC_SHARED_SOURCES EXCLUDE REGEX
    "/Source/Storm/3rdparty/(PKWARE/(implode|explode)|zlib/(deflate|trees))\\.cpp$")
# Not in the 1.14d Mac binary: Lng.cpp is the Windows LNG resource DLL loader (no "LNG File" or
# "D2.LNG" string, no LoadAcceleratorsA user, anywhere in DiabloII_macho).
list(FILTER D2_MAC_SHARED_SOURCES EXCLUDE REGEX "/Source/D2Client/Core/Lng\\.cpp$")
# Not in the 1.14d Mac binary: WardenClient.cpp is the Windows Warden client (module loader, RC4, packet
# handler). DiabloII_macho has no "warden" string and no such code; the game calls four empty stubs
# instead (0012b628..0012b640), which WardenMac.cpp carries. NWardenClient/ is the module interface
# the loaded Warden DLL implements.
list(FILTER D2_MAC_SHARED_SOURCES EXCLUDE REGEX "/Source/D2Client/Warden/(WardenClient\\.cpp|NWardenClient/.*)$")
list(REMOVE_DUPLICATES D2_MAC_SHARED_SOURCES)
list(SORT D2_MAC_SHARED_SOURCES)

# Headers are included by tree-relative path, a handful by bare name from their own
# directory: every directory of the shared tree that holds headers is on the path, as in
# the Windows build, less the Mac-only and renderer trees whose globals.h would shadow.
file(GLOB_RECURSE _mac_shared_headers CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/Source/*.h")
list(FILTER _mac_shared_headers EXCLUDE REGEX
    "^${CMAKE_SOURCE_DIR}/Source/(D2OpenGL|MacSpecific|StormMac|D2Rave|D2Uni|D2Server)/")
set(D2_MAC_SHARED_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}")
foreach(h IN LISTS _mac_shared_headers)
    get_filename_component(_dir "${h}" DIRECTORY)
    list(APPEND D2_MAC_SHARED_INCLUDE_DIRS "${_dir}")
endforeach()
list(REMOVE_DUPLICATES D2_MAC_SHARED_INCLUDE_DIRS)

add_library(d2_mac_shared_objs OBJECT EXCLUDE_FROM_ALL ${D2_MAC_SHARED_SOURCES})
set_target_properties(d2_mac_shared_objs PROPERTIES
    CXX_COMPILER_LAUNCHER "${D2_NODE};${CMAKE_SOURCE_DIR}/cmake/mac-cxx.ts")
target_include_directories(d2_mac_shared_objs PRIVATE ${D2_MAC_SHARED_INCLUDE_DIRS})
target_compile_definitions(d2_mac_shared_objs PRIVATE D2_OS_MAC=1 D2_MAC_CARBON=1 D2_ARCH_X86=1 D2_TARGET_CLIENT=1)
# d2_platform.h is forced ahead of every shared unit, as in the Windows build.
target_compile_options(d2_mac_shared_objs PRIVATE
    -Os -fstack-protector -fpascal-strings
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11 -stdlib=libc++>
    "SHELL:-include ${CMAKE_SOURCE_DIR}/Source/D2Headers/d2_platform.h"
    -Werror=implicit-function-declaration -ferror-limit=0)

# d2_mac_link_deps  what the Mac objects reference and the tree does not define: the third-party
#              code the 1.14d Mac binary embeds (zlib 1.2.8's inflate and uncompress, PKWARE blast
#              from zlib's contrib, Apple's MoreFiles IterateDirectory), vendored unmodified under
#              3rdparty/ (each directory has a PROVENANCE), and link stubs for the two proprietary
#              libraries (Bink, Intel JPEG) and for Blizzard's crashy, whose source is not public
#              (3rdparty/macstubs). Outside `all`; asm_matching/mac/link-probe.ts links it in.
#              The Windows build never sees any of it (CMakeLists.txt excludes these directories).
set(D2_3RDPARTY "${CMAKE_SOURCE_DIR}/3rdparty")
add_library(d2_mac_link_deps OBJECT EXCLUDE_FROM_ALL
    ${D2_3RDPARTY}/zlib/adler32.c ${D2_3RDPARTY}/zlib/crc32.c ${D2_3RDPARTY}/zlib/inffast.c
    ${D2_3RDPARTY}/zlib/inflate.c ${D2_3RDPARTY}/zlib/inftrees.c ${D2_3RDPARTY}/zlib/uncompr.c
    ${D2_3RDPARTY}/zlib/zutil.c
    ${D2_3RDPARTY}/blast/blast.c
    ${D2_3RDPARTY}/MoreFiles/IterateDirectory.c
    ${D2_3RDPARTY}/MoreFiles/MoreFilesExtras.c ${D2_3RDPARTY}/MoreFiles/FSpCompat.c
    ${D2_3RDPARTY}/MoreFiles/MoreFiles.c
    ${D2_3RDPARTY}/macstubs/bink_stub.c ${D2_3RDPARTY}/macstubs/ijl_stub.c
    ${D2_3RDPARTY}/macstubs/crashy_stub.cpp)
# MoreFiles includes the Carbon headers by bare name: the SDK's FlatCarbon directory.
target_include_directories(d2_mac_link_deps PRIVATE
    "${D2_3RDPARTY}/zlib" "${D2_3RDPARTY}/blast" "${D2_3RDPARTY}/MoreFiles"
    "${D2_MAC_SDK}/Developer/Headers/FlatCarbon")
target_compile_options(d2_mac_link_deps PRIVATE -Os -fstack-protector -w
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11 -stdlib=libc++>)
