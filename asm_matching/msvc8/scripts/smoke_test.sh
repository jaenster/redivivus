#!/usr/bin/env bash
# smoke_test.sh - prove each installed compiler runs and is the build it claims.
#
# For rtm and sp1: print the file versions of cl.exe, c1xx.dll and c2.dll,
# compile a two-function C++ file at /O2, and check the object's @comp.id is
# Utc1400_CPP (0x006e) build 50727 - the record the original's game objects
# carry. With the Platform SDK installed it also compiles against windows.h.
set -euo pipefail
. "$(dirname "$0")/env.sh"
have_zig || { echo "smoke_test.sh: needs zig 0.17 on PATH (asm_matching/msvc8/zig builds oracle-zig)" >&2; exit 2; }
map_drive

T="$ORACLE_BUILD/smoke"
mkdir -p "$T"
cat > "$T/smoke.cpp" <<'EOF'
int add(int a, int b) { return a + b; }
struct S { virtual int f(); int x; };
int S::f() { return x * 3; }
EOF
printf '#include <windows.h>\nDWORD tick(void) { return GetTickCount(); }\n' > "$T/smoke_win.cpp"

version() {  # version <file> - the FileVersion string of a PE
    strings -el "$1" | grep -A1 -m1 '^FileVersion$' | tail -1
}

rc=0
for vc in rtm sp1; do
    bin="$WINEPREFIX/drive_c/vc8/$vc/bin"
    echo "== $vc: cl $(version "$bin/cl.exe"), c1xx $(version "$bin/c1xx.dll"), c2 $(version "$bin/c2.dll")"
    rm -f "$T/smoke_$vc.obj"
    ORACLE_VC=$vc "$SCRIPTS/vc8.sh" cl /nologo /c /O2 "$(win_path "$T/smoke.cpp")" \
        "/Fo$(win_path "$T/smoke_$vc.obj")" >/dev/null || true
    if [ ! -s "$T/smoke_$vc.obj" ]; then echo "   FAIL: no object"; rc=1; continue; fi
    cid=$(oracle_zig compid "$T/smoke_$vc.obj"); cid=${cid#*: }; cid=${cid% x1}
    if [ "$cid" = "Utc1400_CPP/50727" ]; then echo "   object stamped Utc1400_CPP/50727: OK"; else echo "   FAIL: @comp.id $cid"; rc=1; fi
    if [ -n "$(find "$WINEPREFIX/drive_c/vc8/psdk/include" -maxdepth 1 -iname windows.h 2>/dev/null)" ]; then
        rm -f "$T/smoke_win_$vc.obj"
        ORACLE_VC=$vc "$SCRIPTS/vc8.sh" cl /nologo /c /O2 "$(win_path "$T/smoke_win.cpp")" \
            "/Fo$(win_path "$T/smoke_win_$vc.obj")" >/dev/null || true
        if [ -s "$T/smoke_win_$vc.obj" ]; then echo "   windows.h: OK"; else echo "   FAIL: windows.h"; rc=1; fi
    else
        echo "   (no Platform SDK yet - windows.h not tested)"
    fi
done
exit $rc
