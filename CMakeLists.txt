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
# generated Source/modules.cmake): each Blizzard module under Source/<Module>/, Storm and
# crashy under 3rdparty/, the shared headers in Source/D2Headers/, and the
# project's own support code in Redivivus/. The executable links every library
# whole, in the 1.14d link order. A wrong-direction include between modules is
# reported in build/module-gates.tsv; in a module marked strict it is an error.
#
# The target is 32-bit Windows and the code assumes the MSVC x86 ABI, so the
# only supported compiler is a mingw-w64 i686 g++ (native or cross).

cmake_minimum_required(VERSION 3.24)
project(Redivivus LANGUAGES C CXX ASM)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Redivivus targets 32-bit Windows. Configure with "
        "-DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake (or a native i686 mingw-w64 g++).")
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
    list(FILTER ${list} EXCLUDE REGEX "^${CMAKE_BINARY_DIR}/")
endforeach()
list(SORT D2_SOURCES)

if(NOT REDIVIVUS_DEV)
    list(REMOVE_ITEM D2_SOURCES "${D2_SUPPORT}/d2_control.cpp")
endif()
if(NOT REDIVIVUS_LOUD)
    list(REMOVE_ITEM D2_SOURCES "${D2_SUPPORT}/d2_loud.cpp")
endif()

# Headers are included by tree-relative path ("Source/D2Game/UNIT/SUNITNPC.h"),
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
if(REDIVIVUS_DEV)
    target_compile_definitions(d2_flags INTERFACE REDIVIVUS_DEV)
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
target_link_options(d2 PRIVATE -static -static-libgcc -static-libstdc++)

target_link_libraries(d2 PRIVATE
    kernel32 user32 gdi32 advapi32 shell32 ole32 oleaut32
    winmm ws2_32 version comctl32 comdlg32 imm32 uuid
    ddraw dsound dxguid wintrust crypt32)

# Bink, Smacker and IJL were licensed DLLs, never inside Game.exe. Their import
# libraries are built from the checked-in .def files (derived from the shipped
# export tables - see 3rdparty/implibs/gen-implibs.sh), so the references resolve
# for real rather than against stubs.
set(D2_IMPLIB_DEFS "${D2_ROOT}/3rdparty/implibs")
if(EXISTS "${D2_IMPLIB_DEFS}")
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
