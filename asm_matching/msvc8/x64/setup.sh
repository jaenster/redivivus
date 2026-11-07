#!/usr/bin/env bash
# setup.sh - lay out the VC8 x64 cross compiler, C:\vc8\rtm-x64 and C:\vc8\psdk-x64.
#
# Visual Studio 2005 Professional carries an x86-hosted compiler for x64
# (VC\bin\x86_amd64, cl.exe 14.00.50727.42, RTM: no SP1 package on hand has x64
# binaries), its x64 runtime libraries (VC\lib\amd64) and the Platform SDK's x64
# import libraries (VC\PlatformSDK\Lib\AMD64). They are taken from the same DVD
# image scripts/setup_psdk.sh reads, by MSI file key, without running an
# installer; nothing else is unpacked. Run `oracle setup` first: it provides the
# image, the wine prefix, the RTM x86 tools (whose PDB writer and cvtres the x64
# tools load) and the shared headers.
#
#   x64/setup.sh              VC8_TOOLCHAIN / VC8_WINEPREFIX as for the oracle
#   X64_DEST=<dir> x64/setup.sh   lay out under <dir> instead of the prefix's C:\vc8
set -euo pipefail
. "$(dirname "$0")/../scripts/env.sh"

PRO_ISO="$VC8_SRC/pro-iso/en_vs_2005_pro_dvd.iso"
PRO_ISO_MD5=d8391b57b355def0530eceb03dc41ba4
W="${X64_SETUP_WORK:-$VC8_SRC/work/pro-x64}"
VC8="$WINEPREFIX/drive_c/vc8"
DEST="${X64_DEST:-$VC8}"
VS='Program Files/Microsoft Visual Studio 8/VC/'

[ -f "$PRO_ISO" ] || { echo "ERROR: no $PRO_ISO - run 'oracle setup' first (or place your copy of the VS2005 Professional DVD image there)" >&2; exit 1; }
[ -f "$VC8/rtm/bin/mspdb80.dll" ] || { echo "ERROR: no $VC8/rtm - run 'oracle setup' first" >&2; exit 1; }
got=$(md5sum "$PRO_ISO" | cut -d' ' -f1)
[ "$got" = "$PRO_ISO_MD5" ] || { echo "ERROR: $PRO_ISO: md5 $got, expected $PRO_ISO_MD5" >&2; exit 1; }

if [ ! -f "$W/x64.tsv" ]; then
    rm -rf "$W" && mkdir -p "$W/keyed" "$W/cabs"
    PATHS="$VC8_SRC/work/pro/paths.tsv"
    if [ ! -f "$PATHS" ]; then
        PATHS="$W/paths.tsv"
        7z x -y -o"$W" "$PRO_ISO" vs/vs_setup.msi >/dev/null
        python3 -I "$SCRIPTS/msi_paths.py" --cabs "$W/vs/vs_setup.msi" > "$PATHS"
    fi
    MSI="$(dirname "$PATHS")/vs/vs_setup.msi"
    [ -f "$MSI" ] || 7z x -y -o"$(dirname "$PATHS")" "$PRO_ISO" vs/vs_setup.msi >/dev/null
    awk -F'\t' -v p="$VS" 'index($2, p "bin/x86_amd64/") == 1 || index($2, p "lib/amd64/") == 1 ||
        index($2, p "PlatformSDK/Lib/AMD64/") == 1' "$PATHS" > "$W/x64.tsv.tmp"
    cut -f3 "$W/x64.tsv.tmp" | sort -u | while read -r cab; do
        case "$cab" in
            '#'*) msiinfo extract "$MSI" "${cab#\#}" > "$W/cabs/${cab#\#}" ;;
            *)    7z x -y -o"$W/cabs" "$PRO_ISO" "vs/$cab" >/dev/null && mv "$W/cabs/vs/$cab" "$W/cabs/$cab" ;;
        esac
        cabextract -q -d "$W/keyed" "$W/cabs/${cab#\#}"
    done
    mv "$W/x64.tsv.tmp" "$W/x64.tsv"
fi

rm -rf "$DEST/rtm-x64" "$DEST/psdk-x64"
python3 -I - "$W/x64.tsv" "$W/keyed" "$DEST" "$VS" <<'EOF'
import os, shutil, sys
tsv, keyed, dest, prefix = sys.argv[1:5]
where = (('bin/x86_amd64/', 'rtm-x64/bin/'), ('lib/amd64/', 'rtm-x64/lib/'),
         ('PlatformSDK/Lib/AMD64/', 'psdk-x64/lib/'))
n = 0
for line in open(tsv, encoding='latin1'):
    key, path, _cab = line.rstrip('\n').split('\t')
    rest = path[len(prefix):]
    out = next(d + rest[len(s):] for s, d in where if rest.startswith(s))
    src = os.path.join(keyed, key)
    if not os.path.isfile(src):
        sys.exit('missing cabinet member %s for %s' % (key, path))
    out = os.path.join(dest, out)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    shutil.copyfile(src, out)
    n += 1
print('%s: %d files' % (dest, n))
EOF
# The x64 tools load the x86 PDB writer and resource converter of the same build,
# which Professional installs once, under VC\bin. The headers serve both targets.
for f in mspdb80.dll msobj80.dll mspdbcore.dll mspdbsrv.exe cvtres.exe; do
    cp "$VC8/rtm/bin/$f" "$DEST/rtm-x64/bin/$f"
done
cp -R "$VC8/rtm/include" "$DEST/rtm-x64/include"
echo "$DEST/rtm-x64, $DEST/psdk-x64: done"
