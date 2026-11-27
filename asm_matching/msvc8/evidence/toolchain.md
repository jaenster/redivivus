# Toolchain identification: Game.exe 1.14d

Every claim here is a measurement with the command that repeats it. Counts in
other documents should point here rather than restate it.

The image: `Game.exe` 1.14d, FileVersion 1.14.3.71, SHA-256
`631066c1649c4ea9ffe48bf97e24c00bca1f7a6759c21150f1a79982589adaaf`, the
whole-file member `Game.exe` of Blizzard's `LODPatch_114d.exe`
(`scripts/fetch_reference.sh`).

## Verdict

| | |
|-|-|
| Compiler | **Visual C++ 2005 SP1**, `cl.exe` 14.00.50727.762 |
| Linker | `link.exe` 8.00, build 50727 (SP1, with the compiler) |
| CRT | static, multithreaded (`/MT`): VS2005 SP1 `libcmt.lib` / `libcpmt.lib` as updated by KB971090 (8.0.50727.4053) |
| Platform SDK | Windows Server 2003 SP1 SDK as bundled with VS2005 (import libraries built by 7.10.4035) |
| Whole-program optimisation | 12 objects only, compiled `/GL` |
| Build | Release, `d:\diablo2\Diablo2\Exe\Release\Game.pdb`, linked 2016-05-31 19:02:20 UTC |

## 1. The Rich header names the tools and their builds

```
oracle rich reference/Game.exe
```

```
  TimeDateStamp  0x574ddfbc  2016-05-31 19:02:20 UTC
  LinkerVersion  8.0
  prodid  tool                    build  count
  0x006e  Utc1400_CPP             50727    512   C++ objects, VC8 cl.exe
  0x006d  Utc1400_C               50727    174   C objects, VC8 cl.exe (mostly the CRT)
  0x0072  Utc1400_LTCG_CPP        50727     12   C++ objects compiled /GL
  0x007d  Masm800                 50727     47   ml.exe 8.0 (the CRT's assembly)
  0x007e  AliasObj800             50327      1
  0x0078  Linker800               50727      1   the final link
  0x007a  Export800               50727      1
  0x007c  Cvtres800               50727      1
  0x005d  Implib710                4035     31   Platform SDK import libraries
  0x0060  Utc1310_CPP              4035      1
  0x0004  Linker600                8168      5   import libraries made by VC6 tools
  0x0004  Linker600                8447     11   (Bink, Smacker, IJL, DirectX SDK)
  0x0001  Import0                     0    353   imported symbols
```

Product 0x006e build 50727 is cl.exe 14.00.50727, i.e. Visual Studio 2005. The
build number is the third version component only: RTM (14.00.50727.42) and SP1
(14.00.50727.762) both stamp 50727, so the header cannot decide between them.

## 2. The linked CRT is SP1's, function by function

```
oracle crt                 # writes evidence/crtmap.tsv and .summary
```

Every function of `libcmt.lib` and `libcpmt.lib` from both installs is
compared, masked, with every function of the image:

```
libcmt.lib + libcpmt.lib functions: rtm 8558, sp1 8582
original functions identical to a library function: 422
  matched by rtm+sp1    398      unchanged by SP1
  matched by rtm          0
  matched by sp1         24      changed by SP1: _fclose_nolock, _localtime64_s,
                                 _encode_pointer, _lock_file, _openfile, ...
```

Twenty-four functions that SP1 changed match only SP1's library and none match
only RTM's: the image was linked against the SP1 libraries. SP1 is one patch
that replaces the compiler binaries and the libraries together (both come
from `vs80sp1-kb926748`), so the compiler was SP1's as well. This is the
reason `ORACLE_VC` defaults to `sp1`.

The game code confirms it independently: compiled by both builds and audited
against the same function map, SP1 matches 41 functions that RTM does not, and
RTM one that SP1 does not ([flags.md](flags.md)).

### 2a. The libraries are SP1's as updated by KB971090 (8.0.50727.4053)

SP1 is not the last word on the libraries. The 2009 ATL security update for
VS2005 SP1 (KB971090, `vs80sp1-kb971090-x86-intl.exe`, SHA-256
`6a3f706391989ffd6b16e23c78bdff49bccd939e02d549d38ad1deab01763d03`) replaces `libcmt.lib` and `libcpmt.lib` whole (its
`PCW_CAB_VS_*` stream carries the files, not deltas):

```
libcmt.lib   SP1      3bc6c5b90f8507964839fd0264d8565c3b766d01f9efcad336532a11e8f06ac3   (our toolchain, drive_c/vc8/sp1/lib)
libcmt.lib   KB971090 cbc9a9f5f706aa63deda800a01386dd0b46c121f60bb7ea28bd74e5d9399c581
libcpmt.lib  SP1      e1b17cb213bfcf77e3c0988f8cb33ec8be143872d1c52de62c5da07ea1eb8031
libcpmt.lib  KB971090 fe8597decade589a232c45aed8c52d939d9a26f64069cfccb3a77caf9abe4138
```

`oracle libmap` run twice on the same tree, same image, no linker map, once
with each pair (VC8_WINEPREFIX pointed at a prefix holding the KB971090 pair):

```
                 lib-exact  lib-order  lib-similar   still unpaired in the library run
SP1                    498         89           42   (126047 bytes paired)
KB971090               533         82           17   (128016 bytes paired)
```

35 functions are byte-identical only to the KB971090 library, none only to
SP1's: the lowio/stdio family (`_close`, `_read_nolock`, `_lseek_nolock`,
`_set_osfhnd`, `_get_osfhandle`, `__lock_fhandle`, `_tsopen_nolock`, `_ioinit`,
`_flsbuf`, `_filbuf`, `_output_l`, `_input_l`, `_setmode`, ...). The image was
linked against the KB971090 runtime. The compiler is untouched by that update:
our `cl.exe`, `c1xx.dll`, `c2.dll` and `link.exe` are 14.00.50727.762 / 8.00.50727.762
(version resources; cl.exe SHA-256 `71c93ca5bddc9b2816d0e053cac2b952f926f6b9321fab6b1ab6e8603621324c`).

The KB971090 pair is kept at `$VC8_TOOLCHAIN/crt-kb971090/` (with the update
package). The shared prefix still links SP1's; switching `libmap`/the image
link to the KB971090 pair changes only runtime rows, never the game-code
headline (which leaves the runtime out).

## 3. The Platform SDK

```
P=~/q2-dev/asm-matching-toolchain/wine-vc8/drive_c/vc8/psdk/lib
oracle compid $P/Kernel32.Lib $P/User32.Lib ...
Kernel32.Lib: Implib710/4035 x3
```

All thirteen Platform SDK import libraries the image imports from (kernel32,
user32, gdi32, advapi32, ws2_32, winmm, shell32, ole32, version, imm32,
wintrust, crypt32, psapi) carry `Implib710` build 4035 in the Platform SDK
that VS2005 Professional bundles - the same record the image's Rich header
counts. DirectDraw's and DirectSound's import libraries are not in that
bundled subset; they come from elsewhere (the DirectX SDK, or the standalone
Server 2003 SP1 SDK).

## 4. Static CRT

The image imports nothing from `MSVCR80.dll`, and 422 of its functions are
byte-identical to `libcmt.lib` / `libcpmt.lib`: `/MT`.

## 5. What is not settled here

* **Per-project compiler flags.** Before 1.14 every module was its own DLL
  project, and the image mixes frame-pointer use: of the original's functions
  over 30 instructions, 6147 set up EBP and 1567 do not. `/O2` alone omits
  frame pointers, so some modules were evidently built `/Oy-`. Which ones is
  measured per module, not assumed (see ../README.md, "Flags").
* **Which 12 objects are /GL.** Fog/Debug is one: its symbol helpers take
  register arguments from another TU, which `/GL` + `/LTCG` reproduces and a
  plain compile cannot ([flags.md](flags.md), "/GL (LTCG) for Fog/Debug").
  The `Bnclient` tree has 12 translation units,
  a suggestive coincidence, but nothing more yet: its functions taking
  arguments in registers (`CACHE_HashString` at 00518fa0: `mov ebx, eax`) do
  not single it out, because VC8 gives a `static` function a custom register
  convention within one translation unit without `/GL` too (seen compiling a
  test unit: a static helper received its pointer in EAX).
* **AliasObj800 build 50327** is a single object, probably from the CRT; it is
  not needed for anything here.
