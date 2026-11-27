# The Mac build: the Mac-only code of 1.14d, compiled, not linked (included from
# CMakeLists.txt when the target is Apple; toolchain cmake/mac-i386-clang425.cmake).
#
# d2_mac_objs  StormMac, the Mac Storm project (Source/StormMac/), the game's Mac-only
#              module (Source/MacSpecific/Source/: the Mac shell and CD2Textures) and the
#              OpenGL renderer (Source/D2OpenGL/Src) on the 10.7 SDK. The renderer's Win32/WGL
#              restoration (MacSdkOnWin32.cpp) and its D2gfx adapter (D2gfxRenderer.cpp) are
#              the Windows build's alone. The 1.14d Mac flags:
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
list(FILTER D2_MAC_SOURCES EXCLUDE REGEX "/Source/D2OpenGL/Src/(MacSdkOnWin32|D2gfxRenderer)\\.cpp$")
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
target_compile_definitions(d2_mac_objs PRIVATE D2_OS_MAC=1 D2_ARCH_X86=1 D2_TARGET_CLIENT=1)
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
target_compile_definitions(d2_mac_shared_objs PRIVATE D2_OS_MAC=1 D2_ARCH_X86=1 D2_TARGET_CLIENT=1)
# d2_platform.h is forced ahead of every shared unit, as in the Windows build.
target_compile_options(d2_mac_shared_objs PRIVATE
    -Os -fstack-protector -fpascal-strings
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11 -stdlib=libc++>
    "SHELL:-include ${CMAKE_SOURCE_DIR}/Source/D2Headers/d2_platform.h"
    -Werror=implicit-function-declaration -ferror-limit=0)
