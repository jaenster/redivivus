# asm_matching/ - measuring how faithful the reconstruction is

The tree builds and runs, but running proves little about any one function:
most code paths are never exercised, and a function can be wrong in a way that
does not crash. An ASM-matching oracle measures it directly. It recompiles the
reconstruction with the compiler that built the original and compares the
emitted code with the shipped `Game.exe`, function by function. A function
whose bytes match is the original code, provably; one that does not shows
where it differs.

| oracle | target | compiler | workspace |
|-|-|-|-|
| **msvc8** | `Game.exe` 1.14d (Windows) | Visual C++ 2005 SP1, under wine | [`msvc8/`](../asm_matching/msvc8/README.md) |
| **mac** | `DiabloII_macho` 1.14d (Intel Mac), the Mac-only code | Apple clang-425.0.28, native under Rosetta | [`mac/`](../asm_matching/mac/README.md) |

The method is the one three Quake II reconstructions use (the Gladiator bot
library, Rocket Arena 2 and OSP Tourney), whose VC6 and gcc 2.7.2.3 oracles
measured their trees function by function against the shipped DLLs. The
compiler was identified from the binary, not assumed:
[`msvc8/evidence/toolchain.md`](../asm_matching/msvc8/evidence/toolchain.md).

The live measurement is `msvc8/evidence/audit.tsv` and its `.summary`. Read
those; this file deliberately quotes no counts.

The second oracle, `mac/`, covers the code only the Mac build of 1.14 carries
(the project README notes the Mac executable has code the Windows one does
not). It compares instructions rather than bytes; its README says what that
does and does not prove.

## Credits

The msvc8 oracle is Niehztog's work (github.com/Niehztog/redivivus, branch `msvc8-oracle`, vendored at
1d82411). Nugwa628 found and published many of the difference classes it measures.

## Running it on macOS

It runs without a VM under Homebrew wine 11 (arm64, x86 through wow64 and Rosetta). GNU coreutils must
come first on `PATH` (`realpath -m`, `md5sum`, `sha1sum`), and each checkout needs its own drive letter
in the shared prefix (`ORACLE_DRIVE`). `oracle all` takes 8-9 minutes on 16 cores. The `sourcemap` tier
of the function map reads the generator's `*.cpp.map` files, which the tree does not commit; without them
fewer functions are paired.
