# env.sh - shared settings for the msvc8 oracle scripts. Source it; do not run it.
#
# Every setting is an environment variable with a default, so an experiment
# changes the environment rather than a script. Anything that changes code
# generation (ORACLE_VC, ORACLE_CFLAGS) marks the build as an EXPERIMENT; evidence
# from an experiment build is never committed (see ../README.md).

ORACLE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"   # asm_matching/msvc8
REPO_ROOT="$(cd "$ORACLE_ROOT/../.." && pwd)"
SCRIPTS="$ORACLE_ROOT/scripts"

# Where the toolchain lives: Microsoft's packages as downloaded, and the wine
# prefix they are laid out in. Shared with the sibling Quake II projects'
# toolchain directory by default, but the prefix is this project's own, so its
# drive letters cannot collide with theirs.
: "${VC8_TOOLCHAIN:=$HOME/q2-dev/asm-matching-toolchain}"
: "${VC8_SRC:=$VC8_TOOLCHAIN/vc8-src}"
: "${VC8_WINEPREFIX:=$VC8_TOOLCHAIN/wine-vc8}"
export WINEPREFIX="$VC8_WINEPREFIX"
export WINEDEBUG=-all BOX64_NOBANNER=1 BOX64_LOG=0
# The compilers' own CRT must be Microsoft's, not wine's builtin msvcr80: c1xx
# converts floating-point literals through it.
export WINEDLLOVERRIDES='msvcr80,msvcp80=n'

# Which compiler: rtm = 14.00.50727.42, sp1 = 14.00.50727.762. The Rich header
# cannot tell them apart (both stamp build 50727); see ../README.md.
: "${ORACLE_VC:=sp1}"

# The wine drive letter that maps the repository. It belongs to this prefix
# only. Override per session if two checkouts share the prefix.
: "${ORACLE_DRIVE:=r}"
ORACLE_DRIVE_WIN="$(printf '%s' "$ORACLE_DRIVE" | tr 'a-z' 'A-Z'):"

# map_drive - make $ORACLE_DRIVE: point at this checkout. A letter already
# pointing at another checkout is an error, never silently re-pointed: a build
# racing a re-point compiles the other tree's sources into this tree's objects.
map_drive() {
    local link="$WINEPREFIX/dosdevices/${ORACLE_DRIVE}:"
    if [ -L "$link" ]; then
        [ "$(readlink -f "$link")" = "$REPO_ROOT" ] && return 0
        echo "ERROR: wine drive $ORACLE_DRIVE_WIN maps $(readlink -f "$link"), not $REPO_ROOT." >&2
        echo "  Use another letter (ORACLE_DRIVE=s), or re-point it if nothing else uses it:" >&2
        echo "  ln -sfn $REPO_ROOT $link" >&2
        return 1
    fi
    ln -s "$REPO_ROOT" "$link"
}

# win_path <path under the repo> - the same path as the compiler sees it.
win_path() {
    local p
    p="$(realpath -m "$1")"
    case "$p" in
        "$REPO_ROOT"/*) p="${p#$REPO_ROOT/}" ;;
        "$REPO_ROOT") p="" ;;
        *) echo "win_path: $1 is outside the repository" >&2; return 1 ;;
    esac
    printf '%s\\%s' "$ORACLE_DRIVE_WIN" "${p//\//\\}"
}

# Build outputs and the reference image. An experiment (another compiler or
# other flags) builds in a directory of its own, so it can never overwrite the
# objects the evidence is made from, and switching back costs no rebuild.
if [ -z "${ORACLE_BUILD:-}" ]; then
    if [ "$ORACLE_VC" != sp1 ] || [ -n "${ORACLE_CFLAGS:-}" ]; then
        ORACLE_BUILD="$ORACLE_ROOT/build/experiment-$ORACLE_VC-$(printf '%s' "${ORACLE_CFLAGS:-}" | md5sum | cut -c1-8)"
    else
        ORACLE_BUILD="$ORACLE_ROOT/build"
    fi
fi
export ORACLE_BUILD ORACLE_VC
REFERENCE_EXE="$ORACLE_ROOT/reference/Game.exe"

# zig/ (oracle-zig) is the oracle in Zig: every stage and tool. It needs zig 0.17 on PATH;
# a script that runs it checks have_zig first and stops when it is missing.
have_zig() { command -v zig >/dev/null 2>&1; }

# oracle_zig <subcommand> [args]: build oracle-zig (a no-op when nothing changed) and run it.
# The build's own output goes to stderr: a caller may be capturing stdout.
oracle_zig() {
    (cd "$ORACLE_ROOT/zig" && zig build --release=fast) >&2
    "$ORACLE_ROOT/zig/zig-out/bin/oracle-zig" "$@"
}

