# Visual Studio 2026 / 2022: cl or clang-cl, x86 or x64, for building and stepping
# through the tree in the Visual Studio debugger. Included by CMakeLists.txt for an
# MSVC-compatible compiler; CMakePresets.json carries the configurations.
#
# This is a debugging build beside the reference ones (mingw-w64 g++ for the game,
# VC8 for the byte-matching oracle), not a replacement for either: nothing here
# changes what those two compile. Every configuration writes a PDB, the Release
# ones too, so a crash report and the debugger name source lines in both.
#
# Exports to CMakeLists.txt:
#   D2_MSVC_LINK_OPTIONS        link options for d2 and d2host
#   d2_msvc_implibs(dir out)    the x86 Bink/Smacker/IJL import libraries

if(REDIVIVUS_LOUD)
    message(FATAL_ERROR "REDIVIVUS_LOUD uses GCC's sanitizers and guard sections; build it with mingw-w64")
endif()
if(REDIVIVUS_PERMISSIVE)
    message(STATUS "REDIVIVUS_PERMISSIVE has no MSVC meaning here; ignored")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(D2_CLANG_CL ON)
else()
    set(D2_CLANG_CL OFF)
endif()

# The static CRT, as the mingw build links -static: one self-contained executable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# /RTC1 (CMake's Debug default) traps on an ESP mismatch across a call and on reads
# of uninitialised locals. Decompiled code does both where the original did (D2_UNINIT
# marks the reads), and the mingw build never checks, so it is opt-in here.
option(REDIVIVUS_MSVC_RTC "MSVC Debug: keep /RTC1 run-time checks" OFF)
if(NOT REDIVIVUS_MSVC_RTC)
    foreach(v CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
        string(REGEX REPLACE "[/-]RTC[1csu]+" "" ${v} "${${v}}")
    endforeach()
endif()

# PDBs in every configuration: /Zi in the compile (/FS: Ninja runs many cl at once
# on one PDB), /DEBUG:FULL in the link. Release keeps its optimisation; /DEBUG turns
# /OPT:REF and /OPT:ICF off unless asked for, so they are asked for there.
foreach(lang C CXX)
    foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        string(REGEX REPLACE "[/-]Z[7iI]" "" CMAKE_${lang}_FLAGS_${cfg} "${CMAKE_${lang}_FLAGS_${cfg}}")
        string(APPEND CMAKE_${lang}_FLAGS_${cfg} " /Zi")
    endforeach()
endforeach()
set(D2_MSVC_LINK_OPTIONS /DEBUG:FULL /INCREMENTAL:NO "$<$<NOT:$<CONFIG:Debug>>:/OPT:REF;/OPT:ICF>")

# The source is UTF-8, as GCC reads and writes it (/utf-8: source and execution
# character set). /bigobj: the module data units exceed the default section count.
# /permissive- (standard conformance, two-phase lookup) except /Zc:strictStrings: a
# string literal binds to `char*`, as G++ allows it, because the recovered signatures
# take `char*` where the original passed literals (SMemAlloc's file name and the like).
# Warnings off, as -w: a type hole is an error, everything else is noise from code
# no person wrote. The deprecation macros silence the CRT's "use the _s variant"
# and POSIX-name notes, which are warnings and so off anyway, for any TU that
# enables warnings itself.
target_compile_options(d2_flags INTERFACE
    "$<${CXX_ONLY}:/w;/utf-8;/bigobj;/EHsc;/FS;/Zc:__cplusplus;/permissive-;/Zc:strictStrings->"
    "$<${CXX_ONLY}:/FI${D2_PLATFORM_HEADER}>")
if(EXISTS "${D2_SUPPORT}/d2_hand.h")
    target_compile_options(d2_flags INTERFACE "$<${CXX_ONLY}:/FI${D2_SUPPORT}/d2_hand.h>")
endif()
target_compile_definitions(d2_flags INTERFACE
    _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
if(D2_CLANG_CL)
    # -fwrapv as in the mingw build: signed overflow wraps (cl never exploits it).
    # __FILE__ tree-relative, as -fmacro-prefix-map gives the mingw build.
    target_compile_options(d2_flags INTERFACE
        "$<${CXX_ONLY}:/clang:-fwrapv;/clang:-fmacro-prefix-map=${D2_ROOT}/=>")
endif()

# Bink, Smacker and IJL (x86 only; x64 uses Redivivus/x64/). The checked-in .def files
# are dlltool's: `Name@N == ImportName` gives the archive symbol from the left and the
# name the loader asks the DLL for on the right. lib.exe spells that `EXPORTAS`
# (Visual Studio 2022 17.8 and later); its `=` form would import the left-hand name.
# Generated at configure time, again whenever a .def changes.
function(d2_msvc_implibs defdir outvar)
    get_filename_component(tooldir "${CMAKE_LINKER}" DIRECTORY)
    find_program(D2_MSVC_LIB NAMES lib HINTS "${tooldir}" NO_CACHE REQUIRED)
    set(outdir "${CMAKE_BINARY_DIR}/implibs")
    file(MAKE_DIRECTORY "${outdir}")
    set(libs)
    foreach(dll binkw32 smackw32 ijl11)
        set(def "${defdir}/${dll}.def")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${def}")
        file(READ "${def}" text)
        string(REPLACE " == " " EXPORTAS " text "${text}")
        file(WRITE "${outdir}/${dll}.def" "${text}")
        execute_process(
            COMMAND "${D2_MSVC_LIB}" /nologo /machine:x86 "/def:${outdir}/${dll}.def" "/out:${outdir}/${dll}.lib"
            RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR "lib.exe could not build ${dll}.lib (needs Visual Studio 2022 17.8+ for EXPORTAS):\n${out}")
        endif()
        list(APPEND libs "${outdir}/${dll}.lib")
    endforeach()
    set(${outvar} ${libs} PARENT_SCOPE)
endfunction()
