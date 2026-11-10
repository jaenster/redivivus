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
# The build also writes the module gate report (build/module-gates.tsv: every
# include of a module the includer may not see, per module pair). It is printed
# here and must exist; CHECK_BUILD_REPORTS=<dir> keeps a copy of it.
#
# CHECK_BUILD_WORK picks a fixed parent for an owned work directory instead of
# a fresh temporary one, so the paths in every compile line stay the same
# between runs. Other files in the parent are preserved. When ccache is
# installed it is used as the compiler launcher, and a fixed path lets it hit.
set -euo pipefail

SRC=${1:-$(git -C "$(dirname "$0")/.." rev-parse --show-toplevel)}
REF=${2:-HEAD}
if [ -n "${CHECK_BUILD_WORK:-}" ]; then
  mkdir -p "$CHECK_BUILD_WORK"
  WORK_PARENT=$(cd "$CHECK_BUILD_WORK" && pwd -P)
  WORK="$WORK_PARENT/.redivivus-check-build"
  OWNER="$WORK/.redivivus-check-build-owned"
  WORK_MARKER=redivivus-check-build-v1
  LOCK="$WORK_PARENT/.redivivus-check-build.lock"
  LOCK_OWNER="$LOCK/owner"
  LOCK_TOKEN="$$:$RANDOM:$RANDOM"
  WORK_OWNED=0
  if ! mkdir "$LOCK" 2>/dev/null; then
    echo "work path is in use: $LOCK (if stale, confirm no run is active before removing the lock)" >&2
    exit 2
  fi
  if ! printf '%s\n' "$LOCK_TOKEN" > "$LOCK_OWNER"; then
    rmdir "$LOCK" 2>/dev/null || true
    echo "could not mark work lock: $LOCK" >&2
    exit 2
  fi
  cleanup_fixed_work() {
    if [ -d "$LOCK" ] && [ ! -L "$LOCK" ] &&
       [ -f "$LOCK_OWNER" ] && [ ! -L "$LOCK_OWNER" ] &&
       [ "$(cat "$LOCK_OWNER")" = "$LOCK_TOKEN" ]; then
      if [ "$WORK_OWNED" = 1 ] && { [ -e "$WORK" ] || [ -L "$WORK" ]; }; then
        if [ -d "$WORK" ] && [ ! -L "$WORK" ] &&
           [ -f "$OWNER" ] && [ ! -L "$OWNER" ] &&
           [ "$(cat "$OWNER")" = "$WORK_MARKER" ]; then
          rm -rf "$WORK" || return 1
        else
          echo "leaving work path with changed ownership: $WORK" >&2
        fi
      fi
      rm -f "$LOCK_OWNER"
      rmdir "$LOCK"
    fi
  }
  trap cleanup_fixed_work EXIT
  if [ -e "$WORK" ] || [ -L "$WORK" ]; then
    if [ ! -d "$WORK" ] || [ -L "$WORK" ] || [ ! -f "$OWNER" ] || [ -L "$OWNER" ] ||
       [ "$(cat "$OWNER")" != "$WORK_MARKER" ]; then
      echo "refusing to remove unowned work path: $WORK" >&2
      exit 2
    fi
    WORK_OWNED=1
    rm -rf "$WORK"
  fi
  mkdir "$WORK"
  WORK_OWNED=1
  printf '%s\n' "$WORK_MARKER" > "$OWNER"
else
  WORK=$(mktemp -d)
  trap 'rm -rf "$WORK"' EXIT
fi

for tool in git cmake i686-w64-mingw32-g++ i686-w64-mingw32-objdump; do
  command -v "$tool" >/dev/null || { echo "missing tool: $tool" >&2; exit 2; }
done
GENERATOR="Unix Makefiles"; KEEP_GOING=-k
command -v ninja >/dev/null && { GENERATOR=Ninja; KEEP_GOING="-k 0"; }
JOBS=${CHECK_BUILD_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}
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

# An #include whose case differs from the tracked path builds on macOS and Windows
# and fails on Linux, so check it here whatever the host's file system does.
CASE_BAD=$(cd "$WORK/src" && {
  git ls-files | sed 's/^/F\t/'
  git ls-files '*.c' '*.cpp' '*.h' '*.hpp' '*.inl' | while IFS= read -r f; do
    { grep -E '^[[:space:]]*#[[:space:]]*include[[:space:]]*"' "$f" || true; } | sed -E 's/.*"([^"]+)".*/\1/' |
      while IFS= read -r inc; do printf 'I\t%s\t%s\n' "$f" "$inc"; done
  done
} | awk -F'\t' '
  function norm(p,   n, i, out, parts, k) {
    n = split(p, parts, "/"); k = 0
    for (i = 1; i <= n; i++) {
      if (parts[i] == "." || parts[i] == "") continue
      if (parts[i] == ".." && k > 0) { k--; continue }
      out[++k] = parts[i]
    }
    p = ""; for (i = 1; i <= k; i++) p = p (i > 1 ? "/" : "") out[i]
    return p
  }
  $1 == "F" { exact[$2] = 1; low[tolower($2)] = $2; next }
  {
    dir = $2; sub(/[^\/]*$/, "", dir)
    a = norm($3); b = norm(dir $3)
    if ((a in exact) || (b in exact)) next
    if (tolower(a) in low) print $2 ": \"" $3 "\" is " low[tolower(a)]
    else if (tolower(b) in low) print $2 ": \"" $3 "\" is " low[tolower(b)]
  }')
if [ -n "$CASE_BAD" ]; then
  echo "$CASE_BAD" >&2
  echo "FAIL: #include case differs from the tracked path" >&2; exit 1
fi

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

GATES="$WORK/build/module-gates.tsv"; GATE_SUMMARY="$WORK/build/module-gates-summary.tsv"
[ -f "$GATES" ] && [ -f "$GATE_SUMMARY" ] || { echo "FAIL: no module gate report" >&2; exit 1; }
echo "module gates (wrong-direction includes, per module; strict modules fail the build):"
column -t -s $'\t' "$GATE_SUMMARY" 2>/dev/null | sed 's/^/  /' || sed 's/^/  /' "$GATE_SUMMARY"
awk -F'\t' 'NR>1 && $3=="violation" {p++; h+=$5} END {printf "module gates: %d module pair(s), %d header(s) included against the graph\n", p, h}' "$GATES"
if [ -n "${CHECK_BUILD_REPORTS:-}" ]; then
  mkdir -p "$CHECK_BUILD_REPORTS" && cp "$GATES" "$GATE_SUMMARY" "$CHECK_BUILD_REPORTS/"
fi
echo "OK: d2.exe, $(stat -c %s "$WORK/build/d2.exe" 2>/dev/null || stat -f %z "$WORK/build/d2.exe") bytes"
