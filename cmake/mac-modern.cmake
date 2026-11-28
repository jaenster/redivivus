# The modern macOS build: the shared game code and the D2OpenGL renderer, compiled with
# Apple clang for arm64 (or x86_64) on the current SDK, against the Win32Posix layer
# (Source/StormMac/Win32Posix) in place of Blizzard's Carbon one. Included from
# CMakeLists.txt when the toolchain is cmake/mac-appleclang.cmake.
#
# d2_modern_objs  compile only, outside `all`: `ninja -k 0 d2_modern_objs` measures it.
#
# What it leaves out, and why:
#   Source/StormMac        Storm's Mac project: SOURCE is the Carbon Storm, Win32Mac the
#                          Win32-on-Carbon layer. The shared Source/Storm units (SFile, SMem,
#                          SReg ...) are built instead, on Win32Posix.
#   Source/MacSpecific/Source/MacShell, Source/D2gfx/Mac, Source/D2Hell/Mac
#                          the Carbon shell, the Carbon D2gfx and the Mac copy protection.
#   Source/D2Rave, Source/D2Uni        the classic Mac PowerPC renderers.
#   Source/D2DDraw, D2Direct3D, D2Gdi, D2Glide   the Windows renderers (not in the module list).
#   Source/D2OpenGL/Src/MacSdkOnWin32.cpp        the renderer's Mac SDK on Win32/WGL.
#   3rdparty/macstubs and the other Mac link dependencies (cmake/mac.cmake).
# What it keeps beside the shared modules: D2OpenGL, and the texture manager it builds on
# (Source/MacSpecific/Source/CD2Textures, no Carbon), as the Windows d2opengl_objs does.
#
# The target axes: D2_OS_MAC (macOS behaviour, shared with the 1.14d Mac build),
# D2_BACKEND_POSIX (the Win32Posix layer), D2_MAC_CARBON=0 (no Carbon layer), and one
# architecture macro per slice: -Xarch_ keeps a universal build to one compile per unit.

set(D2_MODERN_MODULES
    D2Common D2CMP D2Lang D2Game D2Client Fog Storm D2Win D2gfx D2Sound D2Net D2Launch D2Hell
    D2Multi D2MCPClient Bnclient Game)
set(D2_MODERN_SOURCES)
foreach(m IN LISTS D2_MODERN_MODULES)
    file(GLOB_RECURSE _m_sources CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "${CMAKE_SOURCE_DIR}/Source/${m}/*.cpp")
    list(APPEND D2_MODERN_SOURCES ${_m_sources})
    if(EXISTS "${CMAKE_SOURCE_DIR}/Source/globals/${m}.cpp")
        list(APPEND D2_MODERN_SOURCES "${CMAKE_SOURCE_DIR}/Source/globals/${m}.cpp")
    endif()
endforeach()
list(APPEND D2_MODERN_SOURCES "${CMAKE_SOURCE_DIR}/Source/globals/globals.cpp")
list(FILTER D2_MODERN_SOURCES EXCLUDE REGEX "/Source/(D2gfx|D2Hell)/Mac/")
# As in the 1.14d Mac build: the Windows LNG resource loader and the Warden client are not
# Mac code (cmake/mac.cmake); WardenMac.cpp carries the Mac's stubs.
list(FILTER D2_MODERN_SOURCES EXCLUDE REGEX "/Source/D2Client/Core/Lng\\.cpp$")
list(FILTER D2_MODERN_SOURCES EXCLUDE REGEX "/Source/D2Client/Warden/(WardenClient\\.cpp|NWardenClient/.*)$")

file(GLOB _ogl_sources CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/Source/D2OpenGL/Src/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/MacSpecific/Source/CD2Textures/*.cpp")
list(FILTER _ogl_sources EXCLUDE REGEX "/Source/D2OpenGL/Src/MacSdkOnWin32\\.cpp$")
list(APPEND D2_MODERN_SOURCES ${_ogl_sources})
list(REMOVE_DUPLICATES D2_MODERN_SOURCES)
list(SORT D2_MODERN_SOURCES)

# Every directory of the shared tree that holds headers, as in the Windows build, less the
# Mac-only and renderer trees whose globals.h would shadow (the renderer's units find their
# own headers beside them).
file(GLOB_RECURSE _modern_headers CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/Source/*.h")
list(FILTER _modern_headers EXCLUDE REGEX
    "^${CMAKE_SOURCE_DIR}/Source/(D2OpenGL|MacSpecific|StormMac|D2Rave|D2Uni|D2Server)/")
set(D2_MODERN_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}")
foreach(h IN LISTS _modern_headers)
    get_filename_component(_dir "${h}" DIRECTORY)
    list(APPEND D2_MODERN_INCLUDE_DIRS "${_dir}")
endforeach()
list(REMOVE_DUPLICATES D2_MODERN_INCLUDE_DIRS)

add_library(d2_modern_objs OBJECT EXCLUDE_FROM_ALL ${D2_MODERN_SOURCES})
set_target_properties(d2_modern_objs PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS ON)
target_include_directories(d2_modern_objs PRIVATE ${D2_MODERN_INCLUDE_DIRS})
target_compile_definitions(d2_modern_objs PRIVATE
    D2_OS_MAC=1 D2_BACKEND_POSIX=1 D2_MAC_CARBON=0 D2_TARGET_CLIENT=1)
set(_d2_arch_flags)
foreach(_arch IN LISTS CMAKE_OSX_ARCHITECTURES)
    if(_arch STREQUAL "arm64")
        list(APPEND _d2_arch_flags "SHELL:-Xarch_arm64 -DD2_ARCH_ARM64=1")
    elseif(_arch STREQUAL "x86_64")
        list(APPEND _d2_arch_flags "SHELL:-Xarch_x86_64 -DD2_ARCH_X64=1")
    else()
        message(FATAL_ERROR "mac-modern: unsupported architecture ${_arch} (arm64 or x86_64)")
    endif()
endforeach()
# -g and no -O, as the Windows build (link.sh); -fwrapv for the same reason (CMakeLists.txt).
# d2_platform.h is forced ahead of every unit, as in every other build.
# Warnings stay on (no -w): this is the build where 64-bit narrowing shows, so
# -Wshorten-64-to-32 is added. Off by name, as noise on this target only: the Windows calling
# conventions the tree spells (no meaning on arm64 or x86-64), string literals held as char *
# (the decompile's spelling), and the SDK's deprecation notes (sprintf, the GL calls).
target_compile_options(d2_modern_objs PRIVATE
    ${_d2_arch_flags}
    -g -fwrapv -fpascal-strings
    -Wshorten-64-to-32 -Wno-ignored-attributes -Wno-writable-strings -Wno-deprecated-declarations
    "SHELL:-include ${CMAKE_SOURCE_DIR}/Source/D2Headers/d2_platform.h"
    -Werror=implicit-function-declaration -ferror-limit=0)

# ---- Win32Posix and the link ----
#
# d2_win32posix_kernel  the kernel half of Win32Posix (Source/StormMac/Win32Posix/Kernel*.cpp):
#                       POSIX only, no SDL, so its unit test links it alone.
# d2_win32posix         the rest of Win32Posix (User, Sockets, DirectSound, Shell ...): every
#                       other .cpp in the directory, picked up as it is added.
# d2_modern             the executable: d2_modern_objs and both halves, on SDL3 and OpenGL.
#                       Outside `all` until it links; asm_matching/mac/modern-link-probe.ts
#                       lists what it still lacks.
# d2_win32posix_test    the kernel's unit test (Source/StormMac/Win32Posix/Tests).
#
# A layer unit is handed d2_platform.h ahead of everything, as the game's units are, unless it
# reads Win32PosixHost.h (directly or through Win32PosixKernelPriv.h): such a unit reads the
# host headers it needs first and the Win32 surface after them.
file(GLOB _posix_all CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/Source/StormMac/Win32Posix/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/StormMac/Win32Posix/*.mm")
set(_posix_kernel ${_posix_all})
list(FILTER _posix_kernel INCLUDE REGEX "/Win32Posix/Kernel[^/]*\\.cpp$")
set(_posix_rest ${_posix_all})
list(FILTER _posix_rest EXCLUDE REGEX "/Win32Posix/Kernel[^/]*\\.cpp$")

foreach(_src IN LISTS _posix_all)
    file(STRINGS "${_src}" _reads_host REGEX "Win32Posix(Host|KernelPriv)\\.h" LIMIT_COUNT 1)
    if(NOT _reads_host)
        set_source_files_properties("${_src}" PROPERTIES
            COMPILE_OPTIONS "-include;${CMAKE_SOURCE_DIR}/Source/D2Headers/d2_platform.h")
    endif()
endforeach()

function(d2_win32posix_setup _target)
    set_target_properties(${_target} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS ON)
    target_include_directories(${_target} PRIVATE "${CMAKE_SOURCE_DIR}")
    target_compile_definitions(${_target} PRIVATE
        D2_OS_MAC=1 D2_BACKEND_POSIX=1 D2_MAC_CARBON=0 D2_TARGET_CLIENT=1)
    target_compile_options(${_target} PRIVATE ${_d2_arch_flags}
        -g -fwrapv -fpascal-strings -Wshorten-64-to-32 -Wno-ignored-attributes -Wno-writable-strings
        -Wno-deprecated-declarations -Werror=implicit-function-declaration)
endfunction()

add_library(d2_win32posix_kernel OBJECT EXCLUDE_FROM_ALL ${_posix_kernel})
d2_win32posix_setup(d2_win32posix_kernel)

find_package(SDL3 CONFIG QUIET PATHS /opt/homebrew /usr/local)
if(SDL3_FOUND)
    if(_posix_rest)
        add_library(d2_win32posix OBJECT EXCLUDE_FROM_ALL ${_posix_rest})
        d2_win32posix_setup(d2_win32posix)
        target_link_libraries(d2_win32posix PRIVATE SDL3::SDL3)
        set(_posix_rest_objs $<TARGET_OBJECTS:d2_win32posix>)
    endif()
    add_executable(d2_modern EXCLUDE_FROM_ALL
        $<TARGET_OBJECTS:d2_modern_objs> $<TARGET_OBJECTS:d2_win32posix_kernel> ${_posix_rest_objs})
    set_target_properties(d2_modern PROPERTIES OUTPUT_NAME "Diablo II")
    target_link_libraries(d2_modern PRIVATE SDL3::SDL3 "-framework OpenGL")
    # SMem keeps its heaps below 4 GB on a 64-bit target (Storm/Source/SMem.cpp). An x86-64
    # executable may shrink __PAGEZERO to give it that range; the arm64 kernel refuses to run
    # one whose __PAGEZERO is under 4 GB, so the arm64 slice keeps the default.
    if("x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES)
        target_link_options(d2_modern PRIVATE "SHELL:-Xarch_x86_64 -Wl,-pagezero_size,0x10000")
    endif()
else()
    message(STATUS "mac-modern: no SDL3 (brew install sdl3); d2_modern is not defined")
endif()

add_executable(d2_win32posix_test EXCLUDE_FROM_ALL
    "${CMAKE_SOURCE_DIR}/Source/StormMac/Win32Posix/Tests/KernelTest.cpp"
    $<TARGET_OBJECTS:d2_win32posix_kernel>)
d2_win32posix_setup(d2_win32posix_test)
