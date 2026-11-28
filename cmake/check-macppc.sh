#!/usr/bin/env bash
# Syntax-check the classic Mac PowerPC renderers, Source/D2Rave and Source/D2Uni, against the
# stub headers in Redivivus/Platform/MacClassic. No build compiles them (CMakeLists.txt and
# the VC8 oracle leave them out), so this is what keeps them checked.
#
#   cmake/check-macppc.sh [file.cpp ...]     default: every .cpp in both modules
#
# Every file is guarded by `!defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC`, so the axes are
# forced on. Current clang has no Apple PowerPC triple: a unit that needs only the stub
# headers is checked as 32-bit big-endian PowerPC, freestanding; a unit that includes the
# shared tree headers (d2_platform.h needs a hosted C++ library) falls back to i386 Mac with
# the host SDK. Exit status 1 on any error.
set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
CXX=${CXX_MACPPC:-clang++}
SDK=${SDK_MACPPC:-$(xcrun --show-sdk-path 2>/dev/null || true)}
AXES=(-std=gnu++17 -fsyntax-only -DD2_OS_MAC=1 -DD2_MAC_CARBON=1 -DD2_ARCH_PPC=1 -I"$ROOT")
PPC=(-target powerpc-unknown-none -ffreestanding)
HOSTED=(-target i386-apple-macos10.13 -Wno-#warnings -I"$ROOT/Source/D2Headers")
[ -n "$SDK" ] && HOSTED+=(-isysroot "$SDK")

files=("$@")
if [ ${#files[@]} -eq 0 ]; then
  while IFS= read -r f; do files+=("$f"); done \
    < <(find "$ROOT/Source/D2Rave" "$ROOT/Source/D2Uni" -name '*.cpp' | LC_ALL=C sort)
fi

rc=0; total=0
for f in "${files[@]}"; do
  # A unit that reaches past the stubs into the tree's own headers needs the hosted fallback:
  # all of D2Uni (uniInternal.h includes Storm, D2CMP and D2Client), and D2Rave's table unit.
  if [[ "$f" == */Source/D2Uni/* ]] || grep -q '#include "Source/D2gfx/' "$f"; then
    mode=hosted; out=$("$CXX" "${AXES[@]}" "${HOSTED[@]}" "$f" 2>&1)
  else
    mode=ppc; out=$("$CXX" "${AXES[@]}" "${PPC[@]}" "$f" 2>&1)
  fi
  n=$(printf '%s\n' "$out" | grep -c ' error: ')
  w=$(printf '%s\n' "$out" | grep -c ' warning: ')
  total=$((total + n))
  printf '%-40s %-6s errors=%d warnings=%d\n' "${f#"$ROOT"/}" "$mode" "$n" "$w"
  [ "$n" -ne 0 ] && { printf '%s\n' "$out" | grep -A3 ' error: ' | head -40; rc=1; }
done
echo "macppc syntax check: ${#files[@]} files, $total errors"
exit $rc
