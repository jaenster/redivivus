#!/usr/bin/env bash
# setup_psdk.sh - lay out C:\vc8\psdk\{include,lib} from VS2005 Professional.
#
# Visual C++ 2005 Express ships no Platform SDK; Professional bundles one under
# VC\PlatformSDK (the Windows Server 2003 SP1 SDK). That is the SDK the original
# was built against: Game.exe's Rich header carries 31 import libraries stamped
# Implib710 build 4035, the tool build that SDK's import libraries were made
# with. Only its headers and x86 import libraries are taken, and only from the
# cabinets that hold them - which are streams inside vs_setup.msi itself, so
# nothing else on the DVD is unpacked. Called by setup_toolchain.sh.
set -euo pipefail
. "$(dirname "$0")/env.sh"
have_zig || { echo "setup_psdk.sh: needs zig 0.17 on PATH (asm_matching/msvc8/zig builds oracle-zig)" >&2; exit 2; }

PRO_ISO="$VC8_SRC/pro-iso/en_vs_2005_pro_dvd.iso"
PRO_ISO_URL=https://archive.org/download/en_vs_2005_pro_dvd_202303/en_vs_2005_pro_dvd.iso
PRO_ISO_MD5=d8391b57b355def0530eceb03dc41ba4
W="$VC8_SRC/work/pro"
DEST="$WINEPREFIX/drive_c/vc8/psdk"

if [ ! -f "$PRO_ISO" ]; then
    mkdir -p "$(dirname "$PRO_ISO")"
    echo "downloading $PRO_ISO_URL (2.9 GB)"
    curl -fSL --retry 3 -C - -o "$PRO_ISO.part" "$PRO_ISO_URL"
    mv "$PRO_ISO.part" "$PRO_ISO"
fi
got=$(md5sum "$PRO_ISO" | cut -d' ' -f1)
[ "$got" = "$PRO_ISO_MD5" ] || { echo "ERROR: $PRO_ISO: md5 $got, expected $PRO_ISO_MD5" >&2; exit 1; }

PSDK='Program Files/Microsoft Visual Studio 8/VC/PlatformSDK/'
if [ ! -f "$W/psdk.tsv" ]; then
    rm -rf "$W" && mkdir -p "$W/keyed" "$W/cabs"
    7z x -y -o"$W" "$PRO_ISO" vs/vs_setup.msi >/dev/null
    oracle_zig msi-paths --cabs "$W/vs/vs_setup.msi" > "$W/paths.tsv"
    # Headers recursively; libraries at the top of Lib only (AMD64/IA64 below it).
    awk -F'\t' -v p="$PSDK" 'index($2, p "Include/") == 1 ||
        (index($2, p "Lib/") == 1 && substr($2, length(p "Lib/") + 1) !~ /\//)' \
        "$W/paths.tsv" > "$W/psdk.tsv.tmp"
    cut -f3 "$W/psdk.tsv.tmp" | sort -u | while read -r cab; do
        case "$cab" in
            '#'*) msiinfo extract "$W/vs/vs_setup.msi" "${cab#\#}" > "$W/cabs/${cab#\#}" ;;
            *)    7z x -y -o"$W/cabs" "$PRO_ISO" "vs/$cab" >/dev/null && mv "$W/cabs/vs/$cab" "$W/cabs/$cab" ;;
        esac
        cabextract -q -d "$W/keyed" "$W/cabs/${cab#\#}"
    done
    mv "$W/psdk.tsv.tmp" "$W/psdk.tsv"
fi

rm -rf "$DEST"
oracle_zig psdk-layout "$W/psdk.tsv" "$W/keyed" "$DEST" "$PSDK"
