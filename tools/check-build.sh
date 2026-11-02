#!/usr/bin/env bash
# Build Redivivus from a clean clone, the way a stranger would, and say whether it
# produced the executable.
#
#   tools/check-build.sh [repo-url-or-path] [ref]
#
# Defaults to this checkout at HEAD. Needs only public tools: git, cmake and the
# mingw-w64 i686 cross compiler (Debian/Ubuntu: g++-mingw-w64-i686; Homebrew:
# mingw-w64). Uses Ninja when it is installed and make otherwise. Exits 0 when
# build/d2.exe exists, links with no undefined symbol, and every translation unit
# compiled without -fpermissive.
#
# CHECK_BUILD_WORK picks a fixed work directory instead of a fresh temporary one,
# so the paths in every compile line stay the same between runs. When ccache is
# installed it is used as the compiler launcher, and a fixed directory is what
# lets it hit across runs.
set -euo pipefail

SRC=${1:-$(git -C "$(dirname "$0")/.." rev-parse --show-toplevel)}
REF=${2:-HEAD}
if [ -n "${CHECK_BUILD_WORK:-}" ]; then
  WORK=$CHECK_BUILD_WORK; rm -rf "$WORK"; mkdir -p "$WORK"
else
  WORK=$(mktemp -d)
fi
trap 'rm -rf "$WORK"' EXIT

for tool in git cmake i686-w64-mingw32-g++ i686-w64-mingw32-objdump; do
  command -v "$tool" >/dev/null || { echo "missing tool: $tool" >&2; exit 2; }
done
GENERATOR="Unix Makefiles"; KEEP_GOING=-k
command -v ninja >/dev/null && { GENERATOR=Ninja; KEEP_GOING="-k 0"; }
JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
LAUNCHER=()
if command -v ccache >/dev/null; then
  export CCACHE_BASEDIR=$WORK CCACHE_NOHASHDIR=true
  LAUNCHER=(-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache)
  ccache -z >/dev/null
fi

git clone -q "$SRC" "$WORK/src"
git -C "$WORK/src" checkout -q "$REF"
echo "source: $SRC @ $(git -C "$WORK/src" rev-parse --short HEAD)"
echo "compiler: $(i686-w64-mingw32-g++ --version | head -1)"

cmake -S "$WORK/src" -B "$WORK/build" -G "$GENERATOR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_TOOLCHAIN_FILE="$WORK/src/cmake/mingw-i686.cmake" ${LAUNCHER[@]+"${LAUNCHER[@]}"} >"$WORK/configure.log" 2>&1 \
  || { cat "$WORK/configure.log"; exit 1; }

if grep -q -- '-fpermissive' "$WORK/build/compile_commands.json"; then
  echo "FAIL: the build compiles with -fpermissive" >&2; exit 1
fi

set +e
cmake --build "$WORK/build" -j "$JOBS" -- $KEEP_GOING >"$WORK/build.log" 2>&1
RC=$?
set -e

TUS=$(grep -c '"file":' "$WORK/build/compile_commands.json")
ERRORS=$(grep -c ' error: ' "$WORK/build.log" || true)
UNDEF=$({ grep -o "undefined reference to \`[^']*'" "$WORK/build.log" || true; } | sort -u | wc -l | tr -d ' ')
echo "translation units: $TUS, compile errors: $ERRORS"
echo "undefined symbols at link: $UNDEF"
[ -n "${LAUNCHER[*]:-}" ] && ccache -s | grep -iE 'hits|misses' | head -3

if [ "$RC" -ne 0 ] || [ ! -f "$WORK/build/d2.exe" ]; then
  { grep -E ' error: |undefined reference' "$WORK/build.log" || tail -40 "$WORK/build.log"; } | head -40 >&2
  echo "FAIL: no d2.exe" >&2
  exit 1
fi

i686-w64-mingw32-objdump -f "$WORK/build/d2.exe" | grep -q 'file format pei-i386' \
  || { echo "FAIL: d2.exe is not a 32-bit PE" >&2; exit 1; }
echo "OK: d2.exe, $(stat -c %s "$WORK/build/d2.exe" 2>/dev/null || stat -f %z "$WORK/build/d2.exe") bytes"
