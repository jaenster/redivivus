#!/usr/bin/env bash
# Differential test: the same 1.14d functions in the original Game.exe and in ours.
#
#   difftest/run.sh <ours.exe> [picks.txt|auto] [iters] [seed] [out-dir]
#
# Builds the host, writes the spec, runs it under wine in a private prefix and
# prints the summary. Nothing here touches the VM, Ghidra or the oracle's lock.
# <ours.exe> must be UNSTRIPPED (nm finds the functions by name): an i686 CMake
# build's d2.exe.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
OURS=${1:?usage: run.sh <ours.exe> [picks.txt|auto] [iters] [seed] [out-dir]}
PICKS=${2:-"$HERE/picks.txt"}
ITERS=${3:-1000}
SEED=${4:-1}
OUT=${5:-"${TMPDIR:-/tmp}/difftest-$(date +%Y%m%d-%H%M%S)"}
GAME=${D2_GAME_EXE:-"$HOME/code/re/diablo2/d2-clean-114d/Game.exe"}
export WINEPREFIX=${DIFFTEST_WINEPREFIX:-"$HOME/.cache/difftest-wine"}
export WINEDEBUG=-all

mkdir -p "$OUT"
OURS=$(cd "$(dirname "$OURS")" && pwd)/$(basename "$OURS")

i686-w64-mingw32-gcc -O1 -g -Wall -o "$OUT/difftest.exe" "$HERE/host/difftest.c" "$HERE/host/tramp.S" \
  -Wl,--image-base,0x60000000 -Wl,--disable-dynamicbase -static

python3 -I "$HERE/tools/mkspec.py" --ours "$OURS" --out "$OUT/spec.txt" --pick "$PICKS"

# wine wants Windows paths for the images; Z: is the host root
winpath() { echo "Z:$(echo "$1" | tr / '\\')"; }
# One process per function: a function that scribbles over wine's own memory
# wedges only its own run (the host notices and records it as wedged).
rm -f "$OUT/results.jsonl" "$OUT/host.log"
for id in $(awk '$1 == "F" {print $2}' "$OUT/spec.txt"); do
  ( cd "$OUT" && wine difftest.exe "$(winpath "$GAME")" "$(winpath "$OURS")" spec.txt results.jsonl "$ITERS" "$SEED" "$id" \
      >> host.log 2>&1 ) || echo "function $id: host exit $?" >> "$OUT/host.log"
  grep -q "\"id\":$id," "$OUT/results.jsonl" 2>/dev/null || echo "{\"id\":$id,\"name\":\"?\",\"wedged\":true}" >> "$OUT/results.jsonl"
done

python3 -I "$HERE/tools/report.py" "$OUT/results.jsonl" "$OUT/spec.txt" > "$OUT/summary.md"
cat "$OUT/summary.md"
echo
echo "results: $OUT/results.jsonl"
