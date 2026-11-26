# Redivivus

[![Discord](https://img.shields.io/badge/Discord-join%20the%20chat-5865F2?logo=discord&logoColor=white)](https://discord.gg/MHK2Dg9)
[![redivivus.app](https://img.shields.io/badge/redivivus.app-site-8f2b2b)](https://redivivus.app)

[![Functions identical to 1.14d](https://img.shields.io/badge/functions%20identical-79.6%25-2e7d32?style=for-the-badge)](BYTE-MATCHING.md)
[![Code bytes identical](https://img.shields.io/badge/code%20bytes%20identical-54.20%25-1565c0?style=for-the-badge)](BYTE-MATCHING.md)
[![Data bytes identical](https://img.shields.io/badge/data%20bytes%20identical-99.16%25-6a1b9a?style=for-the-badge)](BYTE-MATCHING.md)

Diablo II: Lord of Destruction 1.14d, reverse-engineered into C++ that builds and runs.

At this commit (Ghidra database v2139):

- Builds strict (`-std=c++17`, no `-fpermissive`) and links with no undefined symbol.
- Single player works. A full playthrough has been completed on the Visual C++ 2005 build.
- Functions whose machine code is identical to 1.14d, compiled with Visual C++ 2005 SP1: 11,854 of 14,900 (79.6%), of which 3,167 hand-written.
- Code bytes identical: 1,416,014 of 2,612,801 (54.20%).
- Initialised data identical (tables, globals, initialisers): 341,588 of 344,485 bytes (99.16%), 1,725 of 1,966 symbols.
- A 64-bit build links (mingw x86-64, strict); its dedicated server loads the game data and shuts down cleanly under Wine. 7 pointer-width casts left, from 7,955.

What is compared, what is left out, and the chart: [BYTE-MATCHING.md](BYTE-MATCHING.md). The names, types
and structure layouts are reconstructions from the Ghidra databases; the function bodies are maintained in
this tree and checked against the original's machine code. How the tree is laid out: [docs/layout.md](docs/layout.md).
How it came about: [redivivus.app/about](https://redivivus.app/about).

Redivivus is a single-player project, made to study and preserve the game. Online play on
Battle.net is outside its scope.

## Build and run

You need your own Diablo II: Lord of Destruction 1.14 installation; this repository holds no game data.
Build steps per platform, and with the original compilers: [docs/building.md](docs/building.md).
Copy `d2.exe` next to `Game.exe` in your Diablo II folder and run `d2.exe -w`.

It can still crash, rarely. A crash writes a report with a stack trace next to the executable
([docs/crash-reports.md](docs/crash-reports.md)); please attach it to an [issue](https://github.com/jaenster/redivivus/issues).

## Contributing

A function body that compiles to the original is welcome as a pull request, one function per commit.
A fix to a declaration (a wrong type, a missing parameter) is sent as a proposal against the Ghidra
databases and applied with you as the git author. The rules, the tools and which door a finding takes:
[CONTRIBUTING.md](CONTRIBUTING.md).

## Credits

- Much of the 1.14d reverse-engineering comes from [blizzhackers](https://blizzhackers.dev),
  dzik in particular.
- The byte-match oracle in `asm_matching/` is Niehztog's, and Nugwa628 found many of the
  difference classes it measures.
- [D2MOO](https://github.com/ThePhrozenKeep/D2MOO) supplied many of the structure layouts
  this work started from in 2021, though it went another direction since.

## License

MIT, see [LICENSE](LICENSE). Code under `3rdparty/libbacktrace/` keeps its own license.

The `.cpp.map` files map every function and line back to its address in 1.14d `Game.exe`. They
are part of this work under the same license: if you use them, credit Redivivus and link here.

Diablo and Blizzard Entertainment are trademarks of Blizzard Entertainment, Inc. This
project is not affiliated with or endorsed by Blizzard.
