# Cross-compile Redivivus for 64-bit Windows with mingw-w64 (x86_64).
#
#   cmake -S . -B build64 -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-x86_64.cmake \
#         -DREDIVIVUS_PERMISSIVE=ON -DREDIVIVUS_FFMPEG_INCLUDE=<ffmpeg shared build>/include
#
# Needs x86_64-w64-mingw32-{gcc,g++,windres} on PATH (Homebrew: mingw-w64,
# Debian/Ubuntu: g++-mingw-w64-x86-64). Override the prefix with -DD2_TOOL_PREFIX=.
#
# The 64-bit executable is an experiment beside the 32-bit target, not a release:
# the decompiled code assumes the x86 ABI and still needs -fpermissive here. The
# three licensed 32-bit DLLs (Bink, Smacker, IJL) have no 64-bit originals; the
# build replaces them with the shims in Redivivus/x64/ (see docs/building.md).

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

if(NOT D2_TOOL_PREFIX)
    set(D2_TOOL_PREFIX x86_64-w64-mingw32-)
endif()

set(CMAKE_C_COMPILER   ${D2_TOOL_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${D2_TOOL_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${D2_TOOL_PREFIX}gcc)
set(CMAKE_RC_COMPILER  ${D2_TOOL_PREFIX}windres)

# Search the host for programs, the target sysroot for everything else.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
