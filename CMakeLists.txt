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
# The target is 32-bit Windows and the code assumes the MSVC x86 ABI, so the
# only supported compiler is a mingw-w64 i686 g++ (native or cross).

cmake_minimum_required(VERSION 3.16)
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

option(REDIVIVUS_PERMISSIVE
    "Add -fpermissive. Opt-in only: the gate is that every TU compiles strict." OFF)
option(REDIVIVUS_DEV
    "Development build: per-call tracing and the in-process control channel, which listens on an unauthenticated TCP port." OFF)
option(REDIVIVUS_LOUD
    "Loud build: 0xFE-filled locals, stack canaries, array index checks and guard zones between globals that report at the defect (support/d2_loud.cpp). Not for release." OFF)
option(REDIVIVUS_LOUD_STACK_PROTECTOR
    "With REDIVIVUS_LOUD: also -fstack-protector-strong (fatal on a smashed canary). OFF gives the playable loud build." ON)

# ---------------------------------------------------------------------------
# Sources. Globbed, not listed: the generator adds and removes files on every
# regen, and a hand list goes stale. CONFIGURE_DEPENDS re-globs on each build.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE D2_SOURCES CONFIGURE_DEPENDS
    LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/support/*.s")
file(GLOB_RECURSE D2_HEADERS CONFIGURE_DEPENDS
    LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/*.h")

# A build directory inside the tree must not feed its own files back in.
foreach(list D2_SOURCES D2_HEADERS)
    list(FILTER ${list} EXCLUDE REGEX "^${CMAKE_BINARY_DIR}/")
    list(FILTER ${list} EXCLUDE REGEX "/\\.git/")
    # Vendored C library, built as its own target below; its headers (config.h,
    # internal.h) must not land on the game's include path.
    list(FILTER ${list} EXCLUDE REGEX "^${CMAKE_SOURCE_DIR}/thirdparty/libbacktrace/")
    # The msvc8 oracle: its shim headers stand in for MSVC's and its build/ holds
    # a translated copy of the tree, so none of it belongs to the game.
    list(FILTER ${list} EXCLUDE REGEX "^${CMAKE_SOURCE_DIR}/asm_matching/")
endforeach()
list(SORT D2_SOURCES)

# Headers are included by tree-relative path ("D2Game/UNIT/SUNITNPC.h"), with a
# handful by bare name from their own directory. link.sh puts every directory
# on the include path; the ones that hold headers are the ones that matter.
set(D2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}")
foreach(h IN LISTS D2_HEADERS)
    get_filename_component(dir "${h}" DIRECTORY)
    list(APPEND D2_INCLUDE_DIRS "${dir}")
endforeach()
list(REMOVE_DUPLICATES D2_INCLUDE_DIRS)

if(NOT REDIVIVUS_DEV)
    list(REMOVE_ITEM D2_SOURCES "${CMAKE_SOURCE_DIR}/support/d2_control.cpp")
endif()
if(NOT REDIVIVUS_LOUD)
    list(REMOVE_ITEM D2_SOURCES "${CMAKE_SOURCE_DIR}/support/d2_loud.cpp")
endif()

add_executable(d2 ${D2_SOURCES})
target_include_directories(d2 PRIVATE ${D2_INCLUDE_DIRS})

# ---------------------------------------------------------------------------
# Compile flags - identical to link.sh.
#   -w               warnings off, so real type holes surface as errors
#   -fms-extensions  the decompilation's MSVC-isms
#   -include ...     d2_platform.h carries the MSVC ABI model; support/d2_hand.h
#                    the hand-written declarations. Few TUs include them, so
#                    both are forced in ahead of every source.
# SHELL: keeps each -include paired with its file; CMake de-duplicates plain
# repeated options, which would orphan the second header.
# The macro prefix map makes __FILE__ tree-relative, as it is when link.sh
# compiles from inside the tree; debug info keeps absolute paths for addr2line.
# ---------------------------------------------------------------------------
set(CXX_ONLY "$<COMPILE_LANGUAGE:CXX>")
set_target_properties(d2 PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
# -fwrapv: signed overflow wraps. VC8 never exploits signed-overflow UB, so the
# original's arithmetic wraps wherever it overflows; GCC without it may assume the
# overflow away (SEVT_NextPowerOf2 folded its loop that way).
target_compile_options(d2 PRIVATE
    "$<${CXX_ONLY}:-g;-w;-fms-extensions;-fwrapv>"
    "$<${CXX_ONLY}:SHELL:-include ${CMAKE_SOURCE_DIR}/d2_platform.h>"
    "$<${CXX_ONLY}:-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=>")
if(EXISTS "${CMAKE_SOURCE_DIR}/support/d2_hand.h")
    target_compile_options(d2 PRIVATE
        "$<${CXX_ONLY}:SHELL:-include ${CMAKE_SOURCE_DIR}/support/d2_hand.h>")
endif()
if(REDIVIVUS_PERMISSIVE)
    target_compile_options(d2 PRIVATE "$<${CXX_ONLY}:-fpermissive>")
endif()
if(REDIVIVUS_DEV)
    target_compile_definitions(d2 PRIVATE REDIVIVUS_DEV)
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
        if(src MATCHES "\\.cpp$" AND NOT src MATCHES "/support/d2_(loud|crash)\\.cpp$")
            set_property(SOURCE "${src}" APPEND PROPERTY COMPILE_OPTIONS ${D2_LOUD_FLAGS})
        endif()
    endforeach()
endif()

# ---------------------------------------------------------------------------
# Crash reports (support/d2_crash.cpp) symbolise the stack from this build's own
# DWARF with libbacktrace, vendored as C. The report names the build, so the git
# describe of the tree is baked in at configure time; only d2_crash.cpp sees it,
# so a new commit recompiles that one file.
# ---------------------------------------------------------------------------
add_library(d2_backtrace STATIC
    thirdparty/libbacktrace/alloc.c
    thirdparty/libbacktrace/dwarf.c
    thirdparty/libbacktrace/fileline.c
    thirdparty/libbacktrace/pecoff.c
    thirdparty/libbacktrace/posix.c
    thirdparty/libbacktrace/read.c
    thirdparty/libbacktrace/sort.c
    thirdparty/libbacktrace/state.c)
set_target_properties(d2_backtrace PROPERTIES C_STANDARD 99 C_EXTENSIONS ON)
target_include_directories(d2_backtrace PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/libbacktrace")
target_compile_options(d2_backtrace PRIVATE -g -w)
target_link_libraries(d2 PRIVATE d2_backtrace)

if(NOT REDIVIVUS_BUILD_ID)
    execute_process(COMMAND git describe --always --dirty --tags
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
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
set_source_files_properties("${CMAKE_SOURCE_DIR}/support/d2_crash.cpp" PROPERTIES
    COMPILE_DEFINITIONS "REDIVIVUS_BUILD_ID=\"${REDIVIVUS_BUILD_ID}\"")
if(REDIVIVUS_LOUD)
    set_source_files_properties("${CMAKE_SOURCE_DIR}/support/d2_loud.cpp" PROPERTIES
        COMPILE_DEFINITIONS "REDIVIVUS_BUILD_ID=\"${REDIVIVUS_BUILD_ID}\"")
    if(NOT REDIVIVUS_LOUD_STACK_PROTECTOR)
        set_property(SOURCE "${CMAKE_SOURCE_DIR}/support/d2_loud.cpp" APPEND PROPERTY
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
# export tables - see thirdparty/gen-implibs.sh), so the references resolve for
# real rather than against stubs.
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty")
    if(NOT CMAKE_DLLTOOL)
        find_program(CMAKE_DLLTOOL NAMES ${D2_TOOL_PREFIX}dlltool dlltool REQUIRED)
    endif()
    foreach(dll binkw32 smackw32 ijl11)
        set(def "${CMAKE_SOURCE_DIR}/thirdparty/${dll}.def")
        set(implib "${CMAKE_BINARY_DIR}/thirdparty/lib${dll}.a")
        add_custom_command(OUTPUT "${implib}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/thirdparty"
            COMMAND "${CMAKE_DLLTOOL}" -d "${def}" -l "${implib}"
            DEPENDS "${def}"
            COMMENT "Import library lib${dll}.a")
        list(APPEND D2_IMPLIBS "${implib}")
    endforeach()
    add_custom_target(d2_implibs DEPENDS ${D2_IMPLIBS})
    add_dependencies(d2 d2_implibs)
    target_link_libraries(d2 PRIVATE ${D2_IMPLIBS})
endif()
