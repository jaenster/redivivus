#!/bin/sh
# Builds and runs the Win32Posix user layer's test against Homebrew SDL3, headless (SDL's
# dummy video driver: its offscreen driver makes an EGL surface for every window, which macOS
# lacks).
#   Source/StormMac/Win32Posix/Tests/build-user-test.sh [out-dir] [--live]
# The modern build's flags (cmake/mac-modern.cmake), with the Kernel entry points the layer
# calls supplied by the test.
set -e
ROOT=$(cd "$(dirname "$0")/../../../.." && pwd)
OUT=${1:-$ROOT/build-user-test}
SDL=${SDL3_PREFIX:-$(brew --prefix sdl3 2>/dev/null || echo /opt/homebrew)}
SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
ARCH=$(uname -m)
case $ARCH in arm64) ARCHDEF=-DD2_ARCH_ARM64=1 ;; *) ARCHDEF=-DD2_ARCH_X64=1 ;; esac
mkdir -p "$OUT"
unset SDKROOT
for f in "$ROOT"/Source/StormMac/Win32Posix/User*.cpp "$ROOT"/Source/StormMac/Win32Posix/Tests/UserTest.cpp; do
    xcrun clang++ -isysroot "$SYSROOT" -arch "$ARCH" -mmacosx-version-min=12 -std=gnu++17 -g -fwrapv \
        -fpascal-strings -Wall -Wextra -Wshorten-64-to-32 -Wno-ignored-attributes -Wno-writable-strings \
        -Wno-deprecated-declarations -Wno-unused-parameter -Werror \
        -DD2_OS_MAC=1 -DD2_BACKEND_POSIX=1 -DD2_MAC_CARBON=0 -DD2_TARGET_CLIENT=1 $ARCHDEF \
        -DWIN32POSIX_TEST_KERNEL_STUBS=1 -I"$ROOT" -I"$SDL/include" \
        -include "$ROOT/Source/D2Headers/d2_platform.h" -c "$f" -o "$OUT/$(basename "$f" .cpp).o"
done
xcrun clang++ -isysroot "$SYSROOT" -arch "$ARCH" -mmacosx-version-min=12 "$OUT"/*.o -L"$SDL/lib" -lSDL3 \
    -Wl,-rpath,"$SDL/lib" -o "$OUT/user-test"
shift || true
if [ "$1" = "--live" ]; then
    exec "$OUT/user-test" --live
fi
SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-dummy} "$OUT/user-test"
