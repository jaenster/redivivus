#!/usr/bin/env bash
# setup_toolchain.sh - build the VC8 (Visual C++ 2005) oracle toolchain from
# Microsoft's own packages, without running any installer.
#
#   asm_matching/msvc8/scripts/setup_toolchain.sh
#
# Result, inside the wine prefix $VC8_WINEPREFIX (default
# ~/q2-dev/asm-matching-toolchain/wine-vc8):
#
#   C:\vc8\rtm\{bin,include,lib}   cl.exe 14.00.50727.42   (VS2005 RTM)
#   C:\vc8\sp1\{bin,include,lib}   cl.exe 14.00.50727.762  (VS2005 SP1)
#   C:\vc8\psdk\{include,lib}      the Platform SDK that VS2005 Professional
#                                  bundles (Windows Server 2003 SP1 SDK)
#
# Packages (downloaded into $VC8_SRC if absent, then hash-checked):
#   VC.iso                      Visual C++ 2005 Express, the RTM compiler
#   vs80sp1-VC-kb926748         its SP1; carries whole compiler binaries
#   en_vs_2005_pro_dvd.iso      VS2005 Professional, for the Platform SDK
# The two Express packages are what the compilers come from; Professional is
# needed only because Express ships no Platform SDK (windows.h).
#
# Idempotent: re-running re-lays the trees from the unpacked packages.
# Needs: wine (box64 on aarch64), 7z, cabextract, msitools, python3.
set -euo pipefail
. "$(dirname "$0")/env.sh"

AO=https://archive.org/download
RTM_ISO="$VC8_SRC/rtm-iso/VC.iso"
RTM_ISO_URL="$AO/vs80sp1-all-langs/VS2005EE/VC.iso"
RTM_ISO_MD5=7694252cae9f509a26196ba4635a617e
SP1X="$VC8_SRC/sp1-express/vs80sp1-VC-kb926748-x86-intl.exe"
SP1X_URL="$AO/vs80sp1-all-langs/SP1/ForExpress/vs80sp1-VC-kb926748-x86-intl_8b9a0172efad64774aa122f29e093ad2043b308d.exe"
SP1X_SHA1=8b9a0172efad64774aa122f29e093ad2043b308d   # Microsoft's own, from the download-center URL
PRO_ISO="$VC8_SRC/pro-iso/en_vs_2005_pro_dvd.iso"
PRO_ISO_URL="$AO/en_vs_2005_pro_dvd_202303/en_vs_2005_pro_dvd.iso"
PRO_ISO_MD5=d8391b57b355def0530eceb03dc41ba4

for tool in wine 7z cabextract msiinfo python3; do
    command -v "$tool" >/dev/null || { echo "missing tool: $tool" >&2; exit 2; }
done

fetch() {  # fetch <file> <url> <md5|sha1> <digest>
    local file="$1" url="$2" kind="$3" want="$4" got
    if [ ! -f "$file" ]; then
        mkdir -p "$(dirname "$file")"
        echo "downloading $url"
        curl -fSL --retry 3 -C - -o "$file.part" "$url"
        mv "$file.part" "$file"
    fi
    got=$("${kind}sum" "$file" | cut -d' ' -f1)
    [ "$got" = "$want" ] || { echo "ERROR: $file: $kind $got, expected $want" >&2; exit 1; }
}

fetch "$RTM_ISO" "$RTM_ISO_URL" md5 "$RTM_ISO_MD5"
fetch "$SP1X" "$SP1X_URL" sha1 "$SP1X_SHA1"

W="$VC8_SRC/work"
VC8="$WINEPREFIX/drive_c/vc8"

# ---- prefix -------------------------------------------------------------------
if [ ! -f "$WINEPREFIX/system.reg" ]; then
    echo "creating wine prefix $WINEPREFIX"
    wineboot --init >/dev/null 2>&1
    # Ubuntu's wine package exposes wineserver-stable, not always wineserver.
    if command -v wineserver >/dev/null 2>&1; then
        wineserver -w
    elif command -v wineserver-stable >/dev/null 2>&1; then
        wineserver-stable -w
    else
        echo "missing wineserver or wineserver-stable" >&2
        exit 2
    fi
fi

# ---- RTM ------------------------------------------------------------------------
if [ ! -f "$W/rtm/paths.tsv" ]; then
    rm -rf "$W/rtm" && mkdir -p "$W/rtm/ixp" "$W/rtm/keyed"
    7z x -y -o"$W/rtm" "$RTM_ISO" Ixpvc.exe >/dev/null
    cabextract -q -d "$W/rtm/ixp" "$W/rtm/Ixpvc.exe"
    cabextract -q -d "$W/rtm/keyed" "$W/rtm/ixp/vcsetup1.cab"
    python3 -I "$SCRIPTS/msi_paths.py" "$W/rtm/ixp/vcsetup.msi" > "$W/rtm/paths.tsv.tmp"
    mv "$W/rtm/paths.tsv.tmp" "$W/rtm/paths.tsv"
fi
rm -rf "$VC8/rtm"
python3 -I "$SCRIPTS/vc8_layout.py" "$W/rtm/paths.tsv" "$W/rtm/keyed" "$VC8/rtm"

# ---- SP1: the RTM tree with every file the patch carries replaced ---------------
if [ ! -d "$W/sp1/keyed" ]; then
    rm -rf "$W/sp1" && mkdir -p "$W/sp1/keyed"
    7z x -y -o"$W/sp1" "$SP1X" >/dev/null
    msiinfo extract "$W/sp1/VS80sp1-KB926748-X86-INTL.msp" PCW_CAB_VS_10292 > "$W/sp1/pcw.cab"
    cabextract -q -d "$W/sp1/keyed" "$W/sp1/pcw.cab"
fi
rm -rf "$VC8/sp1"
cp -a "$VC8/rtm" "$VC8/sp1"
python3 -I "$SCRIPTS/vc8_layout.py" "$W/rtm/paths.tsv" "$W/sp1/keyed" "$VC8/sp1"

# ---- the tools' own CRT: Microsoft's, as a side-by-side assembly ------------------
# cl.exe, c1xx.dll, c2.dll and link.exe bind MSVCR80/MSVCP80 through an embedded
# manifest, so a copy next to cl.exe is never consulted. Wine pre-registers its
# builtin CRT as assembly version 8.0.50727.9672, the highest present, and the
# lookup takes the highest - so that placeholder's manifest is set aside and
# Microsoft's 8.0.50727.762 assembly installed. WINEDLLOVERRIDES (env.sh) is
# native-only, so a compile that succeeds has provably run on Microsoft's CRT;
# if wine ever re-registers its placeholder, cl.exe stops loading instead of
# silently switching. (A machine with SP1's runtime redirects RTM tools to .762
# by publisher policy too, so one assembly serves both compilers.)
SXS="$WINEPREFIX/drive_c/windows/winsxs"
ASM=x86_microsoft.vc80.crt_1fc8b3b9a1e18e3b_8.0.50727.762_none_10b2f55f9bffb8f8
mkdir -p "$SXS/$ASM" "$SXS/manifests"
for dll in msvcr80 msvcp80 msvcm80; do
    cp "$(ls "$W/sp1/keyed/$dll.dll.8.0.50727.762."* | head -1)" "$SXS/$ASM/$dll.dll"
done
cp "$(grep -l '<file name="msvcr80.dll"' "$W/sp1/keyed/manifest.8.0.50727.762."* | head -1)" \
    "$SXS/manifests/$ASM.manifest"
for m in "$SXS"/manifests/x86_microsoft.vc80.crt_1fc8b3b9a1e18e3b_*_none_deadbeef.manifest; do
    [ -f "$m" ] && mv "$m" "$m.disabled"
done

# ---- Platform SDK, from VS2005 Professional --------------------------------------
"$SCRIPTS/setup_psdk.sh"

# ---- smoke test: each compiler must run and stamp Utc1400_CPP build 50727 --------
"$SCRIPTS/smoke_test.sh"
