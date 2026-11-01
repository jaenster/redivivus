# Redivivus

[![Discord](https://img.shields.io/badge/Discord-join%20the%20chat-5865F2?logo=discord&logoColor=white)](https://discord.gg/MHK2Dg9)
[![redivivus.app](https://img.shields.io/badge/redivivus.app-site-8f2b2b)](https://redivivus.app)

Diablo II: Lord of Destruction 1.14d, reverse-engineered into C++ that builds and runs.

*Redivivus* is Latin for a thing restored to life. It does what
[D2MOO](https://github.com/ThePhrozenKeep/D2MOO) does for the game mechanics, and carries it
through to the parts that make the game run at all: the program host and its main loop, the
renderers, sound, input, files and archives, the network layer, and Blizzard's Storm and Fog
libraries. Together they build into a program you can start and play.

The tree is organised by the module names the game itself uses (`D2Client`, `D2Common`,
`D2Game`, `D2Win`, `D2gfx`, `Storm`, `Fog`, ...). It is generated from reverse-engineered
Ghidra databases and corrected where needed: the names, types and structure are
reconstructions, and some functions still read like a decompiler wrote them, because one did.

The Ghidra databases behind the tree, 1.14d Windows and Mac, are open to read for anyone with
an AI agent; see [Contributing](#contributing). How the project came about since 2021:
[redivivus.app/about](https://redivivus.app/about).

## Scope

Redivivus is a single-player project, made to study and preserve the game. Online play on
Battle.net is outside its scope: it is not designed, tested or supported for it.

## Status

- Every translation unit compiles with a strict `-std=c++17`, without `-fpermissive`,
  and the executable links with no undefined symbol.
- It starts, reaches the menus, and plays single player: create a character, enter the
  game, walk, fight, change areas.
- It is not finished. Some areas and features still crash. See
  [Known issues](#known-issues).

## Build and run

You need your own Diablo II: Lord of Destruction 1.14 installation; this repository holds no
game data. With a mingw-w64 cross compiler for 32-bit Windows, CMake and Ninja:

```sh
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake
cmake --build build
```

Copy `build/d2.exe` next to `Game.exe` in your Diablo II folder and run `d2.exe -w`.

- [docs/building.md](docs/building.md): getting the game, the toolchain per platform.
- [docs/original-compilers.md](docs/original-compilers.md): building with the compilers that
  built 1.14d, Visual C++ 2005 SP1 and Apple LLVM 4.2, and their flags.

## Known issues

- **It crashes.** Not constantly, but a session of any length will probably hit one. Every
  crash so far has been a single wrong type or a lost value somewhere in the
  reconstruction, and each one fixed takes a class of others with it. A crash writes a
  report with a symbolised stack trace to `d2crash-<date>-<time>.txt` next to the
  executable; please attach it to an [issue](https://github.com/jaenster/redivivus/issues).

## Verification

[`asm_matching/`](asm_matching/README.md) compiles the tree with Visual C++ 2005 SP1 and
compares the machine code of each function with 1.14d. It is a check on the decompilation:
where the code comes out identical, the decompiled function was read correctly.
At this commit, generated from version 1788 of the Ghidra database:

|Measure|Identical machine code|
|-|-|
|Functions|5,818 of 14,759 (39.4%)|
|Code|435,792 of 2,605,160 bytes (16.73%)|

These leave out the C runtime that Visual C++ 2005 linked in statically (libcmt): the
rebuild links a runtime instead of reconstructing it. Over the whole binary, runtime
included: 6,373 of 15,711 functions (40.6%), 534,219 of
2,748,975 bytes (19.43%).

## Contributing

Changes are welcome as pull requests or patches against a release tag. Most of the tree
is regenerated, so a fix that belongs in the reverse-engineering (a wrong type, a missing
parameter) is best reported as an issue with the evidence; it will land upstream and
survive the next regeneration.

[CONTRIBUTING.md](CONTRIBUTING.md) has the detail: which paths are generated and which are
hand-written, and which door a given finding goes through so that it survives.

With an AI agent you can read the Ghidra databases behind the tree (1.14d Windows and Mac)
through a read-only MCP server, and report such fixes with the evidence. In Claude Code:

```
claude plugin marketplace add jaenster/redivivus
claude plugin install redivivus@redivivus
```

The plugin brings the MCP server and the house rules; sign in with GitHub from `/mcp`. Other
agents: see https://redivivus.app/agent-setup/prompt.md. Corrections go in as proposals that are
checked against the databases, reviewed, and applied with you as the git author; see
[how proposals work](https://redivivus.app/about#contributing).

## Credits

- Much of the 1.14d reverse-engineering comes from [blizzhackers](https://blizzhackers.dev),
  dzik in particular.
- The byte-match oracle in `asm_matching/` is Niehztog's, and Nugwa628 found many of the
  difference classes it measures.
- [D2MOO](https://github.com/ThePhrozenKeep/D2MOO) supplied many of the structure layouts
  this work started from in 2021, though it went another direction since.

Diablo and Blizzard Entertainment are trademarks of Blizzard Entertainment, Inc. This
project is not affiliated with or endorsed by Blizzard.
