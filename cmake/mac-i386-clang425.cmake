# Compile Redivivus's Mac-only code with the compiler the 1.14d Mac build used:
# Apple clang-425.0.28 (Xcode 4.6.2 command-line tools), i386, -Os, 10.7 SDK, libc++.
#
#   cmake -S . -B build-mac -DCMAKE_TOOLCHAIN_FILE=cmake/mac-i386-clang425.cmake
#   cmake --build build-mac --target d2_mac_objs
#
# Compile only. The 10.7 system libraries are not on a current Mac, so nothing links;
# the target is an OBJECT library (cmake/mac.cmake).
#
# D2_MAC_TOOLCHAIN  the extracted command-line tools: <dir>/usr/bin/clang. The 2016 binary
#                   is x86_64, so on Apple silicon it runs under Rosetta. Source:
#                   archive.org item xcode4630916281a, xcode462_cltools_10_76938260a.dmg,
#                   pkgutil --expand-full of DeveloperToolsCLI.pkg (nothing installed).
# D2_MAC_SDK        a MacOSX10.7.sdk with its subframeworks (CoreServices/CarbonCore...).
#                   The CLT's DevSDK.pkg lacks those; Xcode 4.x's SDK has them.
# D2_MAC_CLANG      instead of the two above' compiler: any clang that targets
#                   i386-apple-macosx10.7 (an interim check, not the original compiler).

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR i386)

set(_d2_probe "$ENV{HOME}/code/re/diablo2/oracle/mac-probe")
if(NOT D2_MAC_TOOLCHAIN AND EXISTS "${_d2_probe}/cli/Payload/usr/bin/clang")
    set(D2_MAC_TOOLCHAIN "${_d2_probe}/cli/Payload")
endif()
if(NOT D2_MAC_SDK AND EXISTS "${_d2_probe}/sdk107/MacOSX10.7.sdk")
    set(D2_MAC_SDK "${_d2_probe}/sdk107/MacOSX10.7.sdk")
endif()
set(D2_MAC_TOOLCHAIN "${D2_MAC_TOOLCHAIN}" CACHE PATH "Xcode 4.6.2 CLT payload (holds usr/bin/clang)")
set(D2_MAC_SDK "${D2_MAC_SDK}" CACHE PATH "MacOSX10.7.sdk")
set(D2_MAC_CLANG "${D2_MAC_CLANG}" CACHE FILEPATH "Interim: a modern clang for i386-apple-macosx10.7")

if(D2_MAC_CLANG)
    set(CMAKE_C_COMPILER   "${D2_MAC_CLANG}")
    set(CMAKE_CXX_COMPILER "${D2_MAC_CLANG}++")
    set(CMAKE_C_COMPILER_TARGET   i386-apple-macosx10.7)
    set(CMAKE_CXX_COMPILER_TARGET i386-apple-macosx10.7)
elseif(D2_MAC_TOOLCHAIN)
    set(CMAKE_C_COMPILER   "${D2_MAC_TOOLCHAIN}/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "${D2_MAC_TOOLCHAIN}/usr/bin/clang++")
else()
    message(FATAL_ERROR "No Mac compiler: set D2_MAC_TOOLCHAIN (clang-425) or D2_MAC_CLANG (interim).")
endif()
if(NOT D2_MAC_SDK)
    message(FATAL_ERROR "No 10.7 SDK: set D2_MAC_SDK to a MacOSX10.7.sdk.")
endif()

set(CMAKE_OSX_ARCHITECTURES i386 CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7 CACHE STRING "")
set(CMAKE_OSX_SYSROOT "${D2_MAC_SDK}" CACHE PATH "")
# The compiler checks link a test program; nothing links here.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH "${D2_MAC_SDK}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
