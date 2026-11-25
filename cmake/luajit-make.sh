#!/usr/bin/env bash
# Build the vendored LuaJIT (3rdparty/luajit, upstream source, unmodified) as a static library
# with its own Makefile, out of tree, for one target. Used by cmake/luajit.cmake.
#
#   cmake/luajit-make.sh --out DIR [--cross PREFIX] [--target-sys Windows|Linux|Darwin]
#                        [--bits 32|64] [--xcflags "FLAGS"] [--cc "COMPILER FLAGS"] [--src DIR]
#
# --cc sets the target compiler of a native build (no --cross), e.g. "gcc -m32" for a 32-bit
# build on a 64-bit x86 Linux.
#
# Result: DIR/src/libluajit.a. Run it again and make rebuilds only what changed; a change of
# cross prefix, target or XCFLAGS cleans DIR first.
#
# LuaJIT builds and runs two host tools (minilua, buildvm) on the build machine, and buildvm
# must have the target's pointer size. The host compiler (HOST_CC) is therefore:
#   - 64-bit target:              the build machine's cc
#   - 32-bit target, x86 Linux:   "gcc -m32" (needs gcc-multilib)
#   - 32-bit target, anything else (an Apple Silicon or Intel Mac has no 32-bit compiler):
#     the same upstream make runs in a pinned linux/amd64 container (needs docker), for the
#     i686-w64-mingw32 target only.
# LUAJIT_HOST_CC overrides the choice; LUAJIT_DOCKER=1 forces the container.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd -P)
SRC="$HERE/../3rdparty/luajit"
OUT="" CROSS="" TARGET_SYS="" BITS="" XCFLAGS="" TARGET_CC=""
while [ $# -gt 0 ]; do
  case "$1" in
    --src) SRC=$2; shift 2 ;;
    --out) OUT=$2; shift 2 ;;
    --cross) CROSS=$2; shift 2 ;;
    --target-sys) TARGET_SYS=$2; shift 2 ;;
    --bits) BITS=$2; shift 2 ;;
    --xcflags) XCFLAGS=$2; shift 2 ;;
    --cc) TARGET_CC=$2; shift 2 ;;
    *) echo "luajit-make: unknown argument $1" >&2; exit 2 ;;
  esac
done
[ -n "$OUT" ] || { echo "luajit-make: --out is required" >&2; exit 2; }
SRC=$(cd "$SRC" && pwd -P)
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd -P)

# debian:trixie (linux/amd64), pinned by digest; the container adds gcc-multilib and Debian's
# i686 mingw-w64 cross compiler. Built once and kept under a tag derived from this recipe.
BASE_IMAGE=debian@sha256:2dc419c6d7fe3dad0a2f9fd469c34cc90b0f9ba642d8fde837fe3d7d34a0e7f1
DOCKERFILE="FROM $BASE_IMAGE
RUN apt-get update -qq && DEBIAN_FRONTEND=noninteractive apt-get install -y -qq --no-install-recommends \\
    make gcc-multilib gcc-mingw-w64-i686-win32 binutils-mingw-w64-i686 && rm -rf /var/lib/apt/lists/*"

HOST_OS=$(uname -s)
HOST_ARCH=$(uname -m)
USE_DOCKER=${LUAJIT_DOCKER:-0}
HOST_CC=${LUAJIT_HOST_CC:-}
if [ -z "$HOST_CC" ] && [ "$USE_DOCKER" != 1 ]; then
  if [ "${BITS:-64}" = 64 ]; then
    HOST_CC=cc
  elif [ "$HOST_OS" = Linux ] && { [ "$HOST_ARCH" = x86_64 ] || [ "$HOST_ARCH" = i686 ]; }; then
    HOST_CC="gcc -m32"
  else
    USE_DOCKER=1
  fi
fi

MAKEARGS=(BUILDMODE=static)
[ -n "$CROSS" ] && MAKEARGS+=("CROSS=$CROSS")
[ -n "$TARGET_SYS" ] && MAKEARGS+=("TARGET_SYS=$TARGET_SYS")
[ -n "$XCFLAGS" ] && MAKEARGS+=("XCFLAGS=$XCFLAGS")
[ -n "$TARGET_CC" ] && [ -z "$CROSS" ] && MAKEARGS+=("CC=$TARGET_CC")

# A different configuration in the same directory starts from a clean copy.
CONFIG="cross=$CROSS cc=$TARGET_CC sys=$TARGET_SYS bits=$BITS xcflags=$XCFLAGS docker=$USE_DOCKER hostcc=$HOST_CC"
if [ -f "$OUT/.config" ] && [ "$(cat "$OUT/.config")" != "$CONFIG" ]; then
  echo "luajit-make: configuration changed, cleaning $OUT"
  rm -rf "$OUT/src"
fi

if [ "$USE_DOCKER" = 1 ]; then
  [ "$CROSS" = i686-w64-mingw32- ] || {
    echo "luajit-make: the container build covers only CROSS=i686-w64-mingw32- (asked: '$CROSS')." >&2
    echo "  Set LUAJIT_HOST_CC to a 32-bit host compiler to build natively." >&2; exit 1; }
  command -v docker >/dev/null && docker info >/dev/null 2>&1 || {
    echo "luajit-make: a 32-bit LuaJIT target needs a 32-bit host compiler for buildvm, and this" >&2
    echo "  machine ($HOST_OS $HOST_ARCH) has none. The fallback runs LuaJIT's make in a" >&2
    echo "  linux/amd64 docker container, but docker is not installed or not running." >&2
    echo "  Start docker, or set LUAJIT_HOST_CC to a 32-bit host compiler." >&2; exit 1; }
  TAG="redivivus-luajit-build:$(printf '%s' "$DOCKERFILE" | shasum -a 256 | cut -c1-16)"
  if ! docker image inspect "$TAG" >/dev/null 2>&1; then
    echo "luajit-make: building the container image $TAG"
    printf '%s\n' "$DOCKERFILE" | docker build --platform linux/amd64 -q -t "$TAG" - >/dev/null
  fi
  printf '%s' "$CONFIG" > "$OUT/.config"
  # cp -p keeps the source mtimes, so a second run rebuilds only what changed.
  docker run --rm --platform linux/amd64 -u "$(id -u):$(id -g)" \
    -v "$SRC:/luajit:ro" -v "$OUT:/out" "$TAG" bash -euo pipefail -c '
      cp -Rp /luajit/. /out/
      make -j"$(nproc)" -C /out/src HOST_CC="gcc -m32" "$@" libluajit.a' _ "${MAKEARGS[@]}"
else
  printf '%s' "$CONFIG" > "$OUT/.config"
  cp -Rp "$SRC/." "$OUT/"
  if [ "$TARGET_SYS" = Darwin ] || { [ -z "$TARGET_SYS" ] && [ "$HOST_OS" = Darwin ]; }; then
    export MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-11.0}
  fi
  JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
  make -j"$JOBS" -C "$OUT/src" HOST_CC="$HOST_CC" "${MAKEARGS[@]}" libluajit.a
fi
test -f "$OUT/src/libluajit.a"
