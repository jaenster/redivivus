#!/usr/bin/env bash
# fetch_reference.sh - put the original 1.14d Game.exe where the oracle reads it.
#
#   asm_matching/msvc8/scripts/fetch_reference.sh            # download + extract
#   D2_GAME_EXE=/path/to/Game.exe fetch_reference.sh         # use a copy you have
#
# The byte-match target is Blizzard's Game.exe 1.14d. It is NOT in git and must
# never be: this repository ships no Blizzard binaries (README "You need"). So it
# is fetched on demand into asm_matching/msvc8/reference/, which is gitignored.
#
# Source: the official LODPatch_114d.exe, from the patch mirror that
# blizzard-legacy-dl uses (override with D2_PATCH_URL). Game.exe is a whole-file
# record in that patch, so no retail install is needed to rebuild it. Both the
# patch and the extracted image are checked against the SHA-256s below; a
# mismatch is fatal rather than a warning, because every audit row is only as
# good as the image it was diffed against.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
. "$HERE/env.sh"
REF="$(cd "$HERE/.." && pwd)/reference"
mkdir -p "$REF"

GAME_SHA256=631066c1649c4ea9ffe48bf97e24c00bca1f7a6759c21150f1a79982589adaaf
PATCH_SHA256=5ee5b6f09c338324db14ea3ec03a4b8a9b36e4eea249bb12f1fb592558d6434d
: "${D2_PATCH_URL:=https://files.typeguru.nl/diablo/patches/pc/LODPatch_114d.exe}"

check() {  # check <file> <sha256>
    local got
    got=$(sha256sum "$1" | cut -d' ' -f1)
    if [ "$got" != "$2" ]; then
        echo "ERROR: $1 has SHA-256 $got, expected $2" >&2
        return 1
    fi
}

if [ -f "$REF/Game.exe" ] && check "$REF/Game.exe" "$GAME_SHA256" 2>/dev/null; then
    echo "reference: $REF/Game.exe (1.14d, verified)"
    exit 0
fi

if [ -n "${D2_GAME_EXE:-}" ]; then
    check "$D2_GAME_EXE" "$GAME_SHA256"
    cp "$D2_GAME_EXE" "$REF/Game.exe.tmp"
else
    if [ ! -f "$REF/LODPatch_114d.exe" ] || ! check "$REF/LODPatch_114d.exe" "$PATCH_SHA256" 2>/dev/null; then
        echo "downloading $D2_PATCH_URL"
        curl -fSL --retry 3 -o "$REF/LODPatch_114d.exe.tmp" "$D2_PATCH_URL"
        check "$REF/LODPatch_114d.exe.tmp" "$PATCH_SHA256"
        mv "$REF/LODPatch_114d.exe.tmp" "$REF/LODPatch_114d.exe"
    fi
    have_zig || { echo "fetch_reference.sh: needs zig 0.17 on PATH (asm_matching/msvc8/zig builds oracle-zig)" >&2; exit 2; }
    echo "extracting Game.exe (PKWARE DCL, read-only, never executed)"
    oracle_zig mpq-extract "$REF/LODPatch_114d.exe" Game.exe "$REF/Game.exe.tmp"
fi
check "$REF/Game.exe.tmp" "$GAME_SHA256"
mv "$REF/Game.exe.tmp" "$REF/Game.exe"
echo "reference: $REF/Game.exe (1.14d, verified)"
