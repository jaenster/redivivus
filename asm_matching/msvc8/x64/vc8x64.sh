#!/usr/bin/env bash
# vc8x64.sh - run one VC8 x64 tool (x86-hosted, x86_amd64) in the oracle's wine prefix.
#
#   x64/vc8x64.sh cl /nologo /c /W3 /Wp64 Z:\path\file.cpp /FoZ:\path\file.obj
#   x64/vc8x64.sh link /nologo /MACHINE:X64 /OUT:Z:\path\t.exe Z:\path\t.obj user32.lib
#
# Paths reach wine through Z: (wine's default mapping of /), so no drive letter is
# claimed. The layout is x64/setup.sh's: C:\vc8\rtm-x64 and C:\vc8\psdk-x64.
set -euo pipefail
. "$(dirname "$0")/../scripts/env.sh"
tool="${1:?usage: vc8x64.sh <cl|link|lib|dumpbin|ml64> args...}"; shift
exe="$WINEPREFIX/drive_c/vc8/rtm-x64/bin/$tool.exe"
[ -f "$exe" ] || { echo "no $exe - run asm_matching/msvc8/x64/setup.sh" >&2; exit 2; }
export INCLUDE='C:\vc8\rtm-x64\include;C:\vc8\psdk\include'
export LIB='C:\vc8\rtm-x64\lib;C:\vc8\psdk-x64\lib'
exec wine "$exe" "$@"
