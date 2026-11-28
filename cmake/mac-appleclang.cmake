# The modern macOS build: Apple clang from Xcode, the current macOS SDK, arm64 first.
#
#   cmake -S . -B build-modern -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mac-appleclang.cmake
#   ninja -C build-modern -k 0 d2_modern_objs
#
# x86_64 or a universal build: -DCMAKE_OSX_ARCHITECTURES=x86_64 (or "arm64;x86_64").
# The target is cmake/mac-modern.cmake; the 1.14d Mac build is cmake/mac-i386-clang425.cmake.
#
# D2_XCODE   the Xcode developer directory (default /Applications/Xcode.app/Contents/Developer).
#            The SDK is taken from it explicitly: an SDKROOT in the environment may name a
#            Command Line Tools SDK that is not installed, and Homebrew's clang is not the
#            compiler this build ships with.

set(CMAKE_SYSTEM_NAME Darwin)

set(D2_MAC_MODERN ON CACHE BOOL "The modern macOS build (cmake/mac-modern.cmake)" FORCE)

if(NOT D2_XCODE)
    set(D2_XCODE "/Applications/Xcode.app/Contents/Developer")
endif()
set(D2_XCODE "${D2_XCODE}" CACHE PATH "Xcode developer directory")
set(_d2_clang "${D2_XCODE}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang")
set(_d2_sdk "${D2_XCODE}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
if(NOT EXISTS "${_d2_clang}")
    message(FATAL_ERROR "No Apple clang at ${_d2_clang}: set D2_XCODE to an Xcode developer directory.")
endif()
if(NOT EXISTS "${_d2_sdk}")
    message(FATAL_ERROR "No macOS SDK at ${_d2_sdk}.")
endif()

set(CMAKE_C_COMPILER   "${_d2_clang}")
set(CMAKE_CXX_COMPILER "${_d2_clang}++")
set(CMAKE_OSX_SYSROOT "${_d2_sdk}" CACHE PATH "")
set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "arm64, x86_64, or both")
set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0 CACHE STRING "")
