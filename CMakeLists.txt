# Redivivus - Diablo II 1.14d Game.exe recovered as source that builds.
#
#   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake
#   cmake --build build -j
#
# Produces build/d2.exe, the same executable recon/runs/link.sh links: every
# translation unit compiled with the same flags, the hand-written assembly
# support, and import libraries for the three licensed DLLs Game.exe used but did
# not contain.
#
# The tree is one library per module (project.json "modules", read through the
# generated Source/modules.cmake): each Blizzard module under Source/<Module>/ (Storm
# too), crashy under 3rdparty/, the shared headers in Source/D2Headers/, and the
# project's own support code in Redivivus/. The executable links every library
# whole, in the 1.14d link order. A wrong-direction include between modules is
# reported in build/module-gates.tsv; in a module marked strict it is an error.
#
# The target is 32-bit Windows and the code assumes the MSVC x86 ABI. The
# reference compiler is a mingw-w64 i686 g++ (native or cross); Visual Studio
# 2026/2022 (cl or clang-cl, x86 and x64) builds the same tree for debugging,
# through CMakePresets.json (docs/building-msvc.md).

cmake_minimum_required(VERSION 3.24)
project(Redivivus LANGUAGES C CXX)
# GNU assembly (Redivivus/d2_link_support.s); an MSVC build defines its one symbol in C++.
if(NOT MSVC)
    enable_language(ASM)
endif()

# Mac OS X: the Mac-only code (Storm's Mac layer), compile only. See cmake/mac.cmake.
if(APPLE)
    include("${CMAKE_SOURCE_DIR}/cmake/mac.cmake")
    return()
endif()

if(NOT WIN32 OR NOT (CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_SIZEOF_VOID_P EQUAL 8))
    message(FATAL_ERROR "Redivivus targets 32-bit Windows. Configure with "
        "-DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake (or a native i686 mingw-w64 g++).")
endif()
# The 64-bit build (cmake/mingw-x86_64.cmake) is an experiment beside the target:
# it selects the shims in Redivivus/x64/ in place of the three licensed 32-bit DLLs
# and leaves the 32-bit build untouched. Nothing in it is a release.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(D2_ARCH_X64 ON)
    message(STATUS "64-bit build: Bink, Smacker and IJL come from Redivivus/x64/ (FFmpeg at run time)")
else()
    set(D2_ARCH_X64 OFF)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# link.sh builds unoptimised with debug info, and the fault handler's RVAs are
# turned back into file:line with addr2line - so that is the default here too.
# An optimising build type changes code generation of decompiled code that may
# lean on undefined behaviour; treat one as an experiment, not a release.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "" CACHE STRING "Empty = link.sh flags (-g, no -O)" FORCE)
endif()

option(REDIVIVUS_MODULE_LIBRARIES
    "One static library per module, linked whole in the original order. OFF compiles the tree into the executable directly." ON)
option(REDIVIVUS_PERMISSIVE
    "Add -fpermissive. Opt-in only: the gate is that every TU compiles strict." OFF)
option(REDIVIVUS_DEV
    "Development build: per-call tracing and the in-process control channel, which listens on an unauthenticated TCP port." OFF)
option(REDIVIVUS_LOUD
    "Loud build: 0xFE-filled locals, stack canaries, array index checks and guard zones between globals that report at the defect (Redivivus/d2_loud.cpp). Not for release." OFF)
option(REDIVIVUS_LOUD_STACK_PROTECTOR
    "With REDIVIVUS_LOUD: also -fstack-protector-strong (fatal on a smashed canary). OFF gives the playable loud build." ON)
option(REDIVIVUS_OPENGL
    "Link the D2OpenGL renderer into the game as video mode 5 (-opengl), defining D2_RENDERER_OPENGL=1. Not part of the 1.14d Windows build." OFF)
option(REDIVIVUS_X64_LOWMEM_SMOKE
    "64-bit smoke test only: link d2 and d2host without large-address awareness, high-entropy VA or ASLR at a low image base, so every address stays below 2 GB and a pointer the code still truncates to 32 bits survives. Never a release setting." OFF)
option(REDIVIVUS_SERVER
    "Build the dedicated server (d2host.exe) from the server modules, every unit compiled with D2_TARGET_SERVER=1. The game executable leaves the default target in such a build directory." OFF)
option(REDIVIVUS_MODS
    "Mods: the mod runtime (Redivivus/Lua, on LuaJIT from 3rdparty/luajit) built, every unit compiled with D2_FEATURE_MODS=1." OFF)
if(REDIVIVUS_MODS AND D2_ARCH_X64)
    message(FATAL_ERROR "REDIVIVUS_MODS is 32-bit only: a 64-bit build with mods has never "
        "been verified. Configure it with cmake/mingw-i686.cmake, or turn REDIVIVUS_MODS off.")
endif()

set(D2_ROOT "${CMAKE_SOURCE_DIR}")
set(D2_SUPPORT "${D2_ROOT}/Redivivus")
set(D2_PLATFORM_HEADER "${D2_ROOT}/Source/D2Headers/d2_platform.h")
include("${D2_ROOT}/Source/modules.cmake")
# The module table is project.json "modules"; modules.cmake is generated from it.
include("${D2_ROOT}/cmake/check-modules.cmake")
d2_check_modules("${D2_ROOT}/project.json")

# ---------------------------------------------------------------------------
# Sources. Globbed, not listed: the generator adds and removes files on every
# regen, and a hand list goes stale. CONFIGURE_DEPENDS re-globs on each build.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE D2_SOURCES CONFIGURE_DEPENDS
    LIST_DIRECTORIES false
    "${D2_ROOT}/Source/*.cpp"
    "${D2_ROOT}/3rdparty/*.cpp"
    "${D2_ROOT}/Redivivus/*.cpp"
    "${D2_ROOT}/Redivivus/*.s")
file(GLOB D2_ROOT_SOURCES CONFIGURE_DEPENDS LIST_DIRECTORIES false "${D2_ROOT}/*.cpp")
list(APPEND D2_SOURCES ${D2_ROOT_SOURCES})
file(GLOB_RECURSE D2_HEADERS CONFIGURE_DEPENDS
    LIST_DIRECTORIES false
    "${D2_ROOT}/Source/*.h"
    "${D2_ROOT}/3rdparty/*.h"
    "${D2_ROOT}/Redivivus/*.h")
foreach(list D2_SOURCES D2_HEADERS)
    # Vendored C library, built as its own target below; its headers (config.h,
    # internal.h) must not land on the game's include path.
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/3rdparty/libbacktrace/")
    # LuaJIT source, built by its own Makefile (cmake/luajit.cmake).
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/3rdparty/luajit/")
    # Mac link dependencies (zlib, blast, MoreFiles, the link stubs): built by cmake/mac.cmake only.
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/3rdparty/(zlib|blast|MoreFiles|macstubs)/")
    # The Lua host's own test program has a main(); it is a target of its own below.
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/Redivivus/Lua/test/")
    # The Lua tools' node_modules (packages for the generator and the mod CLI) hold no game code.
    list(FILTER ${list} EXCLUDE REGEX "/node_modules/")
    list(FILTER ${list} EXCLUDE REGEX "^${CMAKE_BINARY_DIR}/")
    # The 64-bit shims are their own target below, never part of the Redivivus module.
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/Redivivus/x64/")
    # D2Server (the 1.00 game-server host) is its own object library below, never part
    # of the game executable.
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/Source/D2Server/")
    # The dedicated-server host (Redivivus/Server) is its own executable below.
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/Redivivus/Server/")
    # D2OpenGL and the MacSpecific texture manager it builds on are the Mac 1.14d
    # renderer, their own object library below; the rest of MacSpecific (the Mac shell)
    # is Mac-only, built by cmake/mac.cmake, and so is Source/StormMac (Storm's Mac
    # project). Their directories stay off the include path too: they include by
    # tree-relative path, and their globals.h would shadow.
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/Source/(D2OpenGL|MacSpecific|StormMac)/")
    # D2Rave and D2Uni are the classic Mac PowerPC renderers (RAVE, and DrawSprocket with
    # QuickDraw), and Redivivus/Platform holds the stub headers they are checked against:
    # none belongs to any build here. cmake/check-macppc.sh syntax-checks them.
    list(FILTER ${list} EXCLUDE REGEX "^${D2_ROOT}/(Source/D2Rave|Source/D2Uni|Redivivus/Platform)/")
endforeach()
if(D2_ARCH_X64 OR MSVC)
    # GNU i686 assembly: the stub the 64-bit build defines in C++ instead
    # (Redivivus/x64/d2_crt_x64.cpp), and an MSVC build in d2_crt_impl.cpp.
    list(FILTER D2_SOURCES EXCLUDE REGEX "/Redivivus/d2_link_support\\.s$")
endif()
list(SORT D2_SOURCES)

if(NOT REDIVIVUS_DEV)
    list(REMOVE_ITEM D2_SOURCES "${D2_SUPPORT}/d2_control.cpp")
endif()
if(NOT REDIVIVUS_LOUD)
    list(REMOVE_ITEM D2_SOURCES "${D2_SUPPORT}/d2_loud.cpp")
endif()
if(NOT REDIVIVUS_MODS)
    list(FILTER D2_SOURCES EXCLUDE REGEX "^${D2_SUPPORT}/Lua/")
endif()

# Headers are included by tree-relative path ("Source/D2Game/Unit/SUNITNPC.h"),
# with a handful by bare name from their own directory. link.sh puts every
# directory on the include path; the ones that hold headers are the ones that matter.
set(D2_INCLUDE_DIRS "${D2_ROOT}")
foreach(h IN LISTS D2_HEADERS)
    get_filename_component(dir "${h}" DIRECTORY)
    list(APPEND D2_INCLUDE_DIRS "${dir}")
endforeach()
list(REMOVE_DUPLICATES D2_INCLUDE_DIRS)

# ---------------------------------------------------------------------------
# Compile flags - identical to link.sh, shared by every library.
#   -w               warnings off, so real type holes surface as errors
#   -fms-extensions  the decompilation's MSVC-isms
#   -include ...     d2_platform.h carries the MSVC ABI model; Redivivus/d2_hand.h
#                    the hand-written declarations. Few TUs include them, so
#                    both are forced in ahead of every source.
# SHELL: keeps each -include paired with its file; CMake de-duplicates plain
# repeated options, which would orphan the second header.
# The macro prefix map makes __FILE__ tree-relative, as it is when link.sh
# compiles from inside the tree; debug info keeps absolute paths for addr2line.
# -fwrapv: signed overflow wraps. VC8 never exploits signed-overflow UB, so the
# original's arithmetic wraps wherever it overflows; GCC without it may assume the
# overflow away (SEVT_NextPowerOf2 folded its loop that way).
# ---------------------------------------------------------------------------
set(CXX_ONLY "$<COMPILE_LANGUAGE:CXX>")
add_library(d2_flags INTERFACE)
target_compile_features(d2_flags INTERFACE cxx_std_17)
target_include_directories(d2_flags INTERFACE ${D2_INCLUDE_DIRS})
if(MSVC)
    include("${D2_ROOT}/cmake/msvc.cmake")
else()
    target_compile_options(d2_flags INTERFACE
        "$<${CXX_ONLY}:-g;-w;-fms-extensions;-fwrapv>"
        "$<${CXX_ONLY}:SHELL:-include ${D2_PLATFORM_HEADER}>"
        "$<${CXX_ONLY}:-fmacro-prefix-map=${D2_ROOT}/=>")
    if(EXISTS "${D2_SUPPORT}/d2_hand.h")
        target_compile_options(d2_flags INTERFACE
            "$<${CXX_ONLY}:SHELL:-include ${D2_SUPPORT}/d2_hand.h>")
    endif()
    if(REDIVIVUS_PERMISSIVE)
        target_compile_options(d2_flags INTERFACE "$<${CXX_ONLY}:-fpermissive>")
    endif()
endif()
if(REDIVIVUS_DEV)
    target_compile_definitions(d2_flags INTERFACE REDIVIVUS_DEV)
endif()
# The target axes (Source/D2Headers/D2Arch.h), from the toolchain: the operating system,
# the architecture, and the product. Exactly one of D2_TARGET_CLIENT and D2_TARGET_SERVER
# is defined; REDIVIVUS_SERVER (below) selects the server.
target_compile_definitions(d2_flags INTERFACE D2_OS_WIN=1)
if(D2_ARCH_X64)
    target_compile_definitions(d2_flags INTERFACE D2_ARCH_X64=1)
else()
    target_compile_definitions(d2_flags INTERFACE D2_ARCH_X86=1)
endif()
if(NOT REDIVIVUS_SERVER)
    target_compile_definitions(d2_flags INTERFACE D2_TARGET_CLIENT=1)
endif()
# Optional renderers, each a positive flag. The server renders nothing.
if(REDIVIVUS_OPENGL AND NOT REDIVIVUS_SERVER)
    target_compile_definitions(d2_flags INTERFACE D2_RENDERER_OPENGL=1)
endif()
if(REDIVIVUS_MODS)
    target_compile_definitions(d2_flags INTERFACE D2_FEATURE_MODS=1)
endif()

# The loud build: every game TU gets the checks, the code that reports them does
# not - a bounds check inside its own handler would recurse, and the crash report
# must keep working on a smashed stack. See docs/runtime-testing.md, "The loud build".
if(REDIVIVUS_LOUD)
    # REDIVIVUS_LOUD_STACK_PROTECTOR=OFF is the playable variant: the two non-fatal checks
    # without the canaries, whose slot sort breaks a frame run Ghidra still splits.
    if(REDIVIVUS_LOUD_STACK_PROTECTOR)
        set(D2_LOUD_FLAGS
            -ftrivial-auto-var-init=pattern -fstack-protector-strong -fsanitize=bounds-strict)
    else()
        set(D2_LOUD_FLAGS -ftrivial-auto-var-init=pattern -fsanitize=bounds-strict)
    endif()
    # Guard zones between globals (D2_GLOBAL_GUARD in d2_platform.h): definitions keep
    # source order, and an all-zero initializer stays in .data beside its guard.
    list(APPEND D2_LOUD_FLAGS -DREDIVIVUS_LOUD_GUARDS -fno-toplevel-reorder -fno-zero-initialized-in-bss)
    foreach(src IN LISTS D2_SOURCES)
        if(src MATCHES "\\.cpp$" AND NOT src MATCHES "/Redivivus/d2_(loud|crash)\\.cpp$")
            set_property(SOURCE "${src}" APPEND PROPERTY COMPILE_OPTIONS ${D2_LOUD_FLAGS})
        endif()
    endforeach()
endif()

# ---------------------------------------------------------------------------
# The modules. Each library compiles with D2_MODULE_<itself>, D2_SEES_<m> for
# every module its dependencies let it include and D2_LOADS_<m> for each module it
# loads; the generated headers check D2_SEES_<m> (an error only where
# D2_MODULE_STRICT is set) and compile a declaration no other module uses only
# inside its own module. A module's interface is its public header
# (D2_MODULE_<m>_PUBLIC): another module's internal header is an error in a strict
# module too. Each library links the modules it imports. Source/globals/ and any
# other unowned unit are the shared remainder (d2_root), which sees every module.
# ---------------------------------------------------------------------------
set(D2_MODULE_LIBRARIES_LINKED)
set(D2_UNCLAIMED ${D2_SOURCES})
set(D2_ALL_MODULE_DEFS D2_MODULE_LIBRARIES)
foreach(m IN LISTS D2_MODULES)
    list(APPEND D2_ALL_MODULE_DEFS D2_MODULE_${m} D2_SEES_${m} D2_LOADS_${m})
endforeach()

if(REDIVIVUS_MODULE_LIBRARIES)
    foreach(m IN LISTS D2_MODULES)
        set(dir "${D2_ROOT}/${D2_MODULE_${m}_DIR}")
        if(D2_MODULE_${m}_KIND STREQUAL "headers-only")
            add_library(${m} INTERFACE)
            continue()
        endif()
        if(m STREQUAL "libbacktrace")
            continue()
        endif()
        set(srcs ${D2_UNCLAIMED})
        list(FILTER srcs INCLUDE REGEX "^${dir}/")
        # Its units in the 1.14d link's order (D2_MODULE_<m>_UNITS, project.json
        # "unitOrder"), any unit the list does not name after them in path order: the
        # archive's members follow the list, and the executable links them whole in
        # member order.
        if(D2_MODULE_${m}_UNITS)
            set(ordered)
            foreach(u IN LISTS D2_MODULE_${m}_UNITS)
                if("${D2_ROOT}/${u}" IN_LIST srcs)
                    list(APPEND ordered "${D2_ROOT}/${u}")
                endif()
            endforeach()
            if(ordered)
                list(REMOVE_ITEM srcs ${ordered})
            endif()
            set(srcs ${ordered} ${srcs})
        endif()
        if(D2_MODULE_${m}_DATA AND EXISTS "${D2_ROOT}/${D2_MODULE_${m}_DATA}")
            list(APPEND srcs "${D2_ROOT}/${D2_MODULE_${m}_DATA}")
        endif()
        if(NOT srcs)
            continue()
        endif()
        list(REMOVE_ITEM D2_UNCLAIMED ${srcs})
        add_library(${m} STATIC ${srcs})
        target_link_libraries(${m} PRIVATE d2_flags)
        if(D2_MODULE_${m}_KIND STREQUAL "ours")
            target_compile_definitions(${m} PRIVATE ${D2_ALL_MODULE_DEFS})
        else()
            set(defs D2_MODULE_LIBRARIES D2_MODULE_${m})
            foreach(s IN LISTS D2_MODULE_${m}_SEES)
                list(APPEND defs D2_SEES_${s})
            endforeach()
            foreach(l IN LISTS D2_MODULE_${m}_LOADS)
                list(APPEND defs D2_LOADS_${l})
            endforeach()
            if(D2_MODULE_${m}_STRICT)
                list(APPEND defs D2_MODULE_STRICT)
            endif()
            target_compile_definitions(${m} PRIVATE ${defs})
        endif()
        set_target_properties(${m} PROPERTIES
            D2_DEPENDS "${D2_MODULE_${m}_DEPENDS}" D2_KIND "${D2_MODULE_${m}_KIND}")
        # The library's interface: its public header, the one header another module includes.
        if(D2_MODULE_${m}_PUBLIC AND EXISTS "${D2_ROOT}/${D2_MODULE_${m}_PUBLIC}")
            target_sources(${m} INTERFACE FILE_SET HEADERS BASE_DIRS "${D2_ROOT}"
                FILES "${D2_ROOT}/${D2_MODULE_${m}_PUBLIC}")
        endif()
        list(APPEND D2_MODULE_LIBRARIES_LINKED ${m})
    endforeach()
    # Each library depends on the modules it imports (the 1.13d import tables) and takes
    # their interface. Not as a link item: a link dependency makes CMake put the imported
    # library after its users on the executable's line, and the executable links the
    # libraries in the 1.14d order, which decides the COMDAT copy the linker keeps.
    foreach(m IN LISTS D2_MODULE_LIBRARIES_LINKED)
        foreach(i IN LISTS D2_MODULE_${m}_IMPORTS)
            if(TARGET ${i} AND i IN_LIST D2_MODULE_LIBRARIES_LINKED)
                if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.27)
                    target_link_libraries(${m} PUBLIC "$<COMPILE_ONLY:${i}>")
                endif()
                add_dependencies(${m} ${i})
            endif()
        endforeach()
        set_property(TARGET ${m} PROPERTY D2_IMPORTS "${D2_MODULE_${m}_IMPORTS}")
    endforeach()
    # What no module owns: globals.cpp and any root unit.
    if(D2_UNCLAIMED)
        add_library(d2_root STATIC ${D2_UNCLAIMED})
        target_link_libraries(d2_root PRIVATE d2_flags)
        target_compile_definitions(d2_root PRIVATE ${D2_ALL_MODULE_DEFS})
        list(APPEND D2_MODULE_LIBRARIES_LINKED d2_root)
    endif()
    file(WRITE "${CMAKE_BINARY_DIR}/d2_link_anchor.cpp" "")
    add_executable(d2 "${CMAKE_BINARY_DIR}/d2_link_anchor.cpp")
    foreach(lib IN LISTS D2_MODULE_LIBRARIES_LINKED)
        target_link_libraries(d2 PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${lib}>")
    endforeach()
else()
    add_executable(d2 ${D2_SOURCES})
    target_link_libraries(d2 PRIVATE d2_flags)
endif()

# ---------------------------------------------------------------------------
# D2Server: Blizzard's 1.00 game-server host (Source/D2Server), reconstructed from
# the 1.00 D2Server.dll. Compiled, not linked: it is not part of Game.exe, and its
# imports from the 1.00 DLLs are not bound to the tree yet. Not built by default:
#   cmake --build build --target d2server_objs
# ---------------------------------------------------------------------------
file(GLOB_RECURSE D2SERVER_SOURCES CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${D2_ROOT}/Source/D2Server/*.cpp")
list(SORT D2SERVER_SOURCES)
if(D2SERVER_SOURCES)
    add_library(d2server_objs OBJECT EXCLUDE_FROM_ALL ${D2SERVER_SOURCES})
    target_link_libraries(d2server_objs PRIVATE d2_flags)
endif()

# ---------------------------------------------------------------------------
# D2OpenGL: the OpenGL renderer of the Mac 1.14d build (Source/D2OpenGL/Src, with its
# texture manager base in Source/MacSpecific/Source/CD2Textures), with the Mac system
# calls restored on Win32/WGL. Compiled, not linked: the game does not reference it.
# Not built by default:
#   cmake --build build --target d2opengl_objs
# ---------------------------------------------------------------------------
file(GLOB_RECURSE D2OPENGL_SOURCES CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${D2_ROOT}/Source/D2OpenGL/*.cpp"
    "${D2_ROOT}/Source/MacSpecific/Source/CD2Textures/*.cpp")
list(SORT D2OPENGL_SOURCES)
if(D2OPENGL_SOURCES)
    add_library(d2opengl_objs OBJECT EXCLUDE_FROM_ALL ${D2OPENGL_SOURCES})
    target_link_libraries(d2opengl_objs PRIVATE d2_flags)
    # REDIVIVUS_OPENGL links it into the game: D2GFX_Initialize's mode-5 row and
    # ApplicationMain's -opengl choice are compiled in with D2_RENDERER_OPENGL.
    if(REDIVIVUS_OPENGL AND NOT REDIVIVUS_SERVER)
        target_sources(d2 PRIVATE $<TARGET_OBJECTS:d2opengl_objs>)
        target_link_libraries(d2 PRIVATE opengl32 glu32)
    endif()
endif()

# ---------------------------------------------------------------------------
# The module gate report: every include of a module the includer may not see,
# per module pair (build/module-gates.tsv, per module module-gates-summary.tsv).
# Report only, except for a module marked strict in the module table.
# ---------------------------------------------------------------------------
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${D2_ROOT}/Source/modules.cmake")
if(EXISTS "${D2_ROOT}/project.json")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${D2_ROOT}/project.json")
endif()
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/module-gates.tsv" "${CMAKE_BINARY_DIR}/module-gates-summary.tsv"
    COMMAND "${CMAKE_COMMAND}" -DROOT=${D2_ROOT} -DOUT=${CMAKE_BINARY_DIR} -P "${D2_ROOT}/cmake/module-gates.cmake"
    DEPENDS ${D2_SOURCES} ${D2_HEADERS} "${D2_ROOT}/Source/modules.cmake" "${D2_ROOT}/cmake/module-gates.cmake"
    COMMENT "Module gate report"
    VERBATIM)
add_custom_target(module_gates ALL DEPENDS "${CMAKE_BINARY_DIR}/module-gates.tsv")

# ---------------------------------------------------------------------------
# Crash reports (Redivivus/d2_crash.cpp) symbolise the stack from this build's own
# DWARF with libbacktrace, vendored as C. The report names the build, so the git
# describe of the tree is baked in at configure time; only d2_crash.cpp sees it,
# so a new commit recompiles that one file.
# ---------------------------------------------------------------------------
# An MSVC build carries a PDB instead, which d2_crash.cpp reads through DbgHelp.
if(MSVC)
    target_link_libraries(d2 PRIVATE dbghelp)
else()
set(D2_BACKTRACE "${D2_ROOT}/${D2_MODULE_libbacktrace_DIR}")
add_library(libbacktrace STATIC
    ${D2_BACKTRACE}/alloc.c
    ${D2_BACKTRACE}/dwarf.c
    ${D2_BACKTRACE}/fileline.c
    ${D2_BACKTRACE}/pecoff.c
    ${D2_BACKTRACE}/posix.c
    ${D2_BACKTRACE}/read.c
    ${D2_BACKTRACE}/sort.c
    ${D2_BACKTRACE}/state.c)
set_target_properties(libbacktrace PROPERTIES C_STANDARD 99 C_EXTENSIONS ON)
target_include_directories(libbacktrace PRIVATE "${D2_BACKTRACE}")
target_compile_options(libbacktrace PRIVATE -g -w)
target_link_libraries(d2 PRIVATE libbacktrace)
endif()

# ---------------------------------------------------------------------------
# Mods: the Lua mod runtime (Redivivus/Lua) with LuaJIT 2.1 from source
# (3rdparty/luajit), built by its own Makefile for this toolchain's target: see
# cmake/luajit.cmake. A 32-bit target from a Mac builds LuaJIT in docker.
# The host on its own, for a run without the game:
#   cmake --build build-lua --target d2_lua_host_test
#   wine build-lua/d2_lua_host_test.exe
# ---------------------------------------------------------------------------
if(REDIVIVUS_MODS)
    include("${D2_ROOT}/cmake/luajit.cmake")
    target_link_libraries(d2 PRIVATE luajit)
    # The host's units are compiled in whichever module library owns Redivivus/ (or in d2):
    # they get LuaJIT's generated headers, and every game unit waits for the LuaJIT make.
    file(GLOB _lua_host_sources "${D2_SUPPORT}/Lua/*.cpp" "${D2_SUPPORT}/Lua/generated/*.cpp" "${D2_SUPPORT}/Lua/engine/*.cpp")
    set_property(SOURCE ${_lua_host_sources} APPEND PROPERTY INCLUDE_DIRECTORIES "${D2_LUAJIT_OUT}/src")
    add_dependencies(d2_flags luajit_make)
    include("${D2_ROOT}/cmake/lua-api.cmake")
    if(D2_LUA_API_CHECK)
        add_dependencies(d2_flags ${D2_LUA_API_CHECK})
    endif()
    add_subdirectory("${D2_SUPPORT}/Lua/test" "${CMAKE_BINARY_DIR}/lua-host-test")
endif()

if(NOT REDIVIVUS_BUILD_ID)
    execute_process(COMMAND git describe --always --dirty --tags
        WORKING_DIRECTORY "${D2_ROOT}"
        OUTPUT_VARIABLE REDIVIVUS_BUILD_ID OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET RESULT_VARIABLE _describe_rc)
    if(NOT _describe_rc EQUAL 0 OR NOT REDIVIVUS_BUILD_ID)
        set(REDIVIVUS_BUILD_ID "unknown")
    endif()
endif()
if(REDIVIVUS_LOUD AND REDIVIVUS_LOUD_STACK_PROTECTOR)
    string(APPEND REDIVIVUS_BUILD_ID "-loud")
elseif(REDIVIVUS_LOUD)
    string(APPEND REDIVIVUS_BUILD_ID "-loudnosp")
endif()
set_source_files_properties("${D2_SUPPORT}/d2_crash.cpp" PROPERTIES
    COMPILE_DEFINITIONS "REDIVIVUS_BUILD_ID=\"${REDIVIVUS_BUILD_ID}\"")
if(REDIVIVUS_LOUD)
    set_source_files_properties("${D2_SUPPORT}/d2_loud.cpp" PROPERTIES
        COMPILE_DEFINITIONS "REDIVIVUS_BUILD_ID=\"${REDIVIVUS_BUILD_ID}\"")
    if(NOT REDIVIVUS_LOUD_STACK_PROTECTOR)
        set_property(SOURCE "${D2_SUPPORT}/d2_loud.cpp" APPEND PROPERTY
            COMPILE_DEFINITIONS REDIVIVUS_LOUD_NOSP)
    endif()
endif()

# ---------------------------------------------------------------------------
# Link.
# -static*: self-contained exe, no libgcc/libstdc++ DLLs beside it. The AtlThrow
# shim performs a real C++ throw, so libstdc++ is needed regardless.
# ---------------------------------------------------------------------------
set_target_properties(d2 PROPERTIES WIN32_EXECUTABLE ON OUTPUT_NAME d2)
if(MSVC)
    target_link_options(d2 PRIVATE ${D2_MSVC_LINK_OPTIONS})
else()
    target_link_options(d2 PRIVATE -static -static-libgcc -static-libstdc++)
endif()

target_link_libraries(d2 PRIVATE
    kernel32 user32 gdi32 advapi32 shell32 ole32 oleaut32
    winmm ws2_32 version comctl32 comdlg32 imm32 uuid
    ddraw dsound dxguid wintrust crypt32)

# Bink, Smacker and IJL were licensed DLLs, never inside Game.exe. Their import
# libraries are built from the checked-in .def files (derived from the shipped
# export tables - see 3rdparty/implibs/gen-implibs.sh), so the references resolve
# for real rather than against stubs.
set(D2_IMPLIB_DEFS "${D2_ROOT}/3rdparty/implibs")
if(D2_ARCH_X64)
    # No 64-bit binkw32/smackw32/ijl11 exists. Redivivus/x64/ provides the same
    # entry points: Bink and Smacker decode through FFmpeg's DLLs, loaded at run
    # time (LoadLibrary, no import), IJL writes the JPEG with stb_image_write. The
    # FFmpeg headers are a compile-time dependency only; point at the include/ of
    # the same shared build whose DLLs will sit beside the exe (docs/building.md).
    set(REDIVIVUS_FFMPEG_INCLUDE "$ENV{REDIVIVUS_FFMPEG_INCLUDE}" CACHE PATH
        "include/ of an FFmpeg shared build (libavformat/avformat.h ...), 64-bit build only")
    if(NOT EXISTS "${REDIVIVUS_FFMPEG_INCLUDE}/libavformat/avformat.h" AND REDIVIVUS_SERVER)
        # d2host decodes no video; only the game executable, outside the default
        # target of a server build directory, needs the shims.
        message(STATUS "64-bit server build without FFmpeg headers: the game executable cannot link here")
    elseif(NOT EXISTS "${REDIVIVUS_FFMPEG_INCLUDE}/libavformat/avformat.h")
        message(FATAL_ERROR "64-bit build: set -DREDIVIVUS_FFMPEG_INCLUDE=<ffmpeg>/include "
            "(the directory holding libavformat/avformat.h); see docs/building.md")
    endif()
endif()
if(D2_ARCH_X64 AND EXISTS "${REDIVIVUS_FFMPEG_INCLUDE}/libavformat/avformat.h")
    file(GLOB D2_X64_SOURCES CONFIGURE_DEPENDS LIST_DIRECTORIES false "${D2_SUPPORT}/x64/*.cpp")
    add_library(d2_x64deps STATIC ${D2_X64_SOURCES})
    target_compile_features(d2_x64deps PRIVATE cxx_std_17)
    if(NOT MSVC)
        target_compile_options(d2_x64deps PRIVATE -g -Wall -Wextra)
        # stb_image_write.h is static and only part of it is called.
        set_source_files_properties("${D2_SUPPORT}/x64/d2_ijl.cpp" PROPERTIES COMPILE_OPTIONS -Wno-unused-function)
    endif()
    target_include_directories(d2_x64deps PRIVATE "${D2_ROOT}" "${D2_ROOT}/Source/D2Headers"
        "${D2_ROOT}/3rdparty/stb" SYSTEM "${REDIVIVUS_FFMPEG_INCLUDE}")
    target_link_libraries(d2_x64deps PUBLIC winmm)
    target_link_libraries(d2 PRIVATE d2_x64deps)
    # The decode/write path on its own, for a run without the game (docs/building.md).
    add_executable(x64deps_test EXCLUDE_FROM_ALL "${D2_SUPPORT}/x64/test/x64deps_test.cpp")
    target_compile_features(x64deps_test PRIVATE cxx_std_17)
    target_include_directories(x64deps_test PRIVATE "${D2_ROOT}" "${D2_ROOT}/Source/D2Headers" "${D2_ROOT}/3rdparty/stb")
    target_link_libraries(x64deps_test PRIVATE d2_x64deps)
    # A 64-bit glide3x.dll beside it, resolved by the decorated names Glide.cpp asks for.
    add_executable(glidetest WIN32 EXCLUDE_FROM_ALL "${D2_SUPPORT}/x64/test/glidetest.cpp")
    target_compile_features(glidetest PRIVATE cxx_std_17)
    target_link_libraries(glidetest PRIVATE user32 gdi32)
    if(NOT MSVC)
        target_link_options(x64deps_test PRIVATE -static -static-libgcc -static-libstdc++)
        target_link_options(glidetest PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
elseif(NOT D2_ARCH_X64 AND MSVC AND EXISTS "${D2_IMPLIB_DEFS}")
    d2_msvc_implibs("${D2_IMPLIB_DEFS}" D2_IMPLIBS)
    target_link_libraries(d2 PRIVATE ${D2_IMPLIBS})
elseif(NOT D2_ARCH_X64 AND EXISTS "${D2_IMPLIB_DEFS}")
    if(NOT CMAKE_DLLTOOL)
        find_program(CMAKE_DLLTOOL NAMES ${D2_TOOL_PREFIX}dlltool dlltool REQUIRED)
    endif()
    foreach(dll binkw32 smackw32 ijl11)
        set(def "${D2_IMPLIB_DEFS}/${dll}.def")
        set(implib "${CMAKE_BINARY_DIR}/implibs/lib${dll}.a")
        add_custom_command(OUTPUT "${implib}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/implibs"
            COMMAND "${CMAKE_DLLTOOL}" -d "${def}" -l "${implib}"
            DEPENDS "${def}"
            COMMENT "Import library lib${dll}.a")
        list(APPEND D2_IMPLIBS "${implib}")
    endforeach()
    add_custom_target(d2_implibs DEPENDS ${D2_IMPLIBS})
    add_dependencies(d2 d2_implibs)
    target_link_libraries(d2 PRIVATE ${D2_IMPLIBS})
endif()

# ---------------------------------------------------------------------------
# The dedicated server: d2host.exe, built from the modules a game server runs and the
# host in Redivivus/Server (ServerHost.h is the seam; exactly one host is linked). Every
# unit of the build directory gets D2_TARGET_SERVER=1, so code can tell the two targets
# apart with a positive test; the game executable built beside it would carry that too,
# so it leaves the default target here (it can still be built by name).
#   cmake -S . -B build-server -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake -DREDIVIVUS_SERVER=ON
#   cmake --build build-server --target d2host
# -DREDIVIVUS_SERVER_HOST=<file.cpp> links that host in place of Server/ServerHost.cpp.
# The libraries link as an ordinary rescanned group, not whole: only what the server
# reaches is pulled in, so client code the server never calls stays out.
# ---------------------------------------------------------------------------
if(REDIVIVUS_X64_LOWMEM_SMOKE)
    if(NOT D2_ARCH_X64)
        message(FATAL_ERROR "REDIVIVUS_X64_LOWMEM_SMOKE is for the 64-bit build only")
    endif()
    if(MSVC)
        set(D2_X64_LOWMEM_LINK /LARGEADDRESSAWARE:NO /HIGHENTROPYVA:NO /DYNAMICBASE:NO /BASE:0x400000)
        target_link_options(d2 PRIVATE ${D2_X64_LOWMEM_LINK})
    else()
        # GNU ld has no --disable-large-address-aware for PE32+; the flag is cleared after the link.
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        set(D2_X64_LOWMEM_LINK -Wl,--disable-high-entropy-va -Wl,--disable-dynamicbase
            -Wl,--image-base=0x400000)
        set(D2_X64_LOWMEM_POST "${Python3_EXECUTABLE}" "${D2_ROOT}/cmake/pe-clear-laa.py")
        target_link_options(d2 PRIVATE ${D2_X64_LOWMEM_LINK})
        add_custom_command(TARGET d2 POST_BUILD COMMAND ${D2_X64_LOWMEM_POST} "$<TARGET_FILE:d2>")
    endif()
endif()

if(REDIVIVUS_SERVER)
    if(NOT REDIVIVUS_MODULE_LIBRARIES)
        message(FATAL_ERROR "REDIVIVUS_SERVER needs REDIVIVUS_MODULE_LIBRARIES=ON")
    endif()
    target_compile_definitions(d2_flags INTERFACE D2_TARGET_SERVER=1)
    set_target_properties(d2 PROPERTIES EXCLUDE_FROM_ALL ON)

    set(D2_SERVER_MODULES crashy Fog Game Storm D2Hell D2Lang D2Net D2Game D2CMP D2Common d2_root)
    set(D2_SERVER_LIBRARIES)
    foreach(m IN LISTS D2_SERVER_MODULES)
        if(TARGET ${m} AND m IN_LIST D2_MODULE_LIBRARIES_LINKED)
            list(APPEND D2_SERVER_LIBRARIES ${m})
        endif()
    endforeach()

    # The project's support code, compiled into the executable so its constructors run,
    # minus the game's static-init list: the server has its own in Redivivus/Server. The
    # tracer only traces in a development build and probes D2gfx state; outside one the
    # server leaves it out and holds its packet counters in Redivivus/Server.
    get_target_property(D2_SERVER_SUPPORT_SOURCES Redivivus SOURCES)
    list(FILTER D2_SERVER_SUPPORT_SOURCES EXCLUDE REGEX "/Redivivus/d2_static_init\\.cpp$")
    if(NOT REDIVIVUS_DEV)
        list(FILTER D2_SERVER_SUPPORT_SOURCES EXCLUDE REGEX "/Redivivus/d2_trace\\.cpp$")
    endif()
    file(GLOB D2_SERVER_HOST_SOURCES CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "${D2_SUPPORT}/Server/*.cpp")
    list(SORT D2_SERVER_HOST_SOURCES)
    # A host kept outside the tree (a test host that serves players, say) takes the place
    # of the default one; it implements ServerHost.h like ServerHost.cpp does.
    set(REDIVIVUS_SERVER_HOST "" CACHE FILEPATH
        "Host source linked into d2host in place of Redivivus/Server/ServerHost.cpp (empty: the default host)")
    if(REDIVIVUS_SERVER_HOST)
        list(FILTER D2_SERVER_HOST_SOURCES EXCLUDE REGEX "/Server/ServerHost\\.cpp$")
        list(APPEND D2_SERVER_HOST_SOURCES "${REDIVIVUS_SERVER_HOST}")
    endif()

    add_executable(d2host ${D2_SERVER_HOST_SOURCES} ${D2_SERVER_SUPPORT_SOURCES})
    if(D2_ARCH_X64)
        # The x86 link takes _Wrappers::CRT_ExceptionFilter1 from d2_link_support.s;
        # on x64 it is d2_crt_x64.cpp, the one piece of d2_x64deps the server needs.
        target_sources(d2host PRIVATE "${D2_SUPPORT}/x64/d2_crt_x64.cpp")
        target_include_directories(d2host PRIVATE "${D2_ROOT}/Source/D2Headers")
    endif()
    target_link_libraries(d2host PRIVATE d2_flags)
    target_compile_definitions(d2host PRIVATE ${D2_ALL_MODULE_DEFS})
    if(MSVC)
        # link.exe searches every library until nothing more resolves: no group needed.
        target_link_libraries(d2host PRIVATE ${D2_SERVER_LIBRARIES} dbghelp)
        target_link_options(d2host PRIVATE ${D2_MSVC_LINK_OPTIONS})
    else()
        string(JOIN "," D2_SERVER_LINK_GROUP ${D2_SERVER_LIBRARIES} libbacktrace)
        target_link_libraries(d2host PRIVATE "$<LINK_GROUP:RESCAN,${D2_SERVER_LINK_GROUP}>")
        target_link_options(d2host PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
    if(REDIVIVUS_X64_LOWMEM_SMOKE)
        target_link_options(d2host PRIVATE ${D2_X64_LOWMEM_LINK})
        if(NOT MSVC)
            add_custom_command(TARGET d2host POST_BUILD COMMAND ${D2_X64_LOWMEM_POST} "$<TARGET_FILE:d2host>")
        endif()
    endif()
    if(REDIVIVUS_MODS)
        target_link_libraries(d2host PRIVATE luajit)
    endif()
    target_link_libraries(d2host PRIVATE
        kernel32 user32 gdi32 advapi32 shell32 ole32 oleaut32
        winmm ws2_32 version comctl32 imm32 uuid wintrust crypt32)
endif()
