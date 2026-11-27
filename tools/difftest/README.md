# difftest - the original and the rebuild, function against function

Calls the same 1.14d function in the shipped `Game.exe` and in our rebuilt exe with
the same inputs, and compares what each one did: the return value, every byte it
wrote through a pointer, every global it wrote, the callee-saved registers, the
stack it popped, and whether it faulted. Agreement over many random inputs is
evidence of semantic equivalence where the VC8 bytes differ; disagreement is a
silent bug the compile count and the game cannot show, or a harness artifact, and
each one has to be called.

Offline, on the host, under wine. No VM, no Ghidra writes, no daemon. The only
Ghidra input is the read-only d2fn snapshot (`~/.cache/d2fn-v2/snapshot`).

## Run it

```sh
# 1. an i686 build of the tree; d2.exe keeps its symbols (nm finds functions by name)
cmake -G Ninja -S . -B build32 -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake
ninja -C build32 d2

# 2. the run: host build, spec, wine, summary
tools/difftest/run.sh build32/d2.exe                                   # picks.txt, 1000 sets
tools/difftest/run.sh build32/d2.exe auto 200                          # mkspec's own pick
tools/difftest/run.sh build32/d2.exe tools/difftest/selftest.txt 200   # negative controls
```

Needs wine, `i686-w64-mingw32-gcc`, the shipped 1.14d `Game.exe` (`D2_GAME_EXE`) and a
d2fn snapshot of the Ghidra database (`D2FN_SNAPSHOT`, default `~/.cache/d2fn-v2/snapshot`).

`run.sh <ours.exe> [picks|auto] [iters] [seed] [out-dir]` leaves `spec.txt`,
`spec.txt.tsv` (every snapshot function with its testable/untestable verdict and the
reason), `results.jsonl`, `host.log` and `summary.md` in the out-dir. The wine prefix
is `~/.cache/difftest-wine` (`DIFFTEST_WINEPREFIX` overrides); the first run creates
it. 50 functions x 1000 sets takes about ten minutes, nearly all of it in wine's
exception dispatch for inputs that fault on both sides.

**Run `selftest.txt` after changing the host.** Each line pairs the original of one
function with our build of a different function with the same signature; all of them
must come out `different`. One that comes out equivalent means the harness compares
less than it claims.

## How it works

`host/difftest.c` + `host/tramp.S` (mingw, i686) map both PE images by hand and never
start them: no entry point, no CRT, no static initialisers, no TLS callbacks.
`Game.exe` would sit at 0x400000, but wine maps its NLS files low before `main`, so
it is relocated like ours (both carry `.reloc`). Game.exe's imports are all traps
(a distinct no-access address each); ours get `msvcrt`/`kernel32`/`ntdll` for real,
because GCC calls `memcpy`/`memset` for plain C, and traps for the rest.

Per input set both sides start from the same state:

- **arguments** in the registers and stack slots each side's ABI puts them;
- **the arena**: 8 chunks of 32 KB at 0x20000000, each followed by a no-access page,
  byte-identical for both calls. Every pointer argument points into it, so pointer
  values compare as values. Four fill modes: mixed dwords (0, small, 16-bit, arena
  pointers, random), pointer-rich, text with NULs, all zero;
- **mapped globals** filled with the same bytes on both sides (writable ones only);
- **a private 256 KB stack** with the same pattern, at the same address, TEB bounds
  switched with it - an uninitialised local reads the same bytes in both, so a
  difference it causes is the function's;
- **everything else** in both images at file contents: a write watch finds the dirty
  pages of BOTH images after each call, they are compared with a pristine copy and
  restored. Stray writes (outside the mapped globals) are recorded by absolute address,
  so a wild write - NULL plus a huge offset - that lands in the other side's image
  compares equal to the same write from the other side.

`tramp.S` sets EAX..EDI and a sentinel EBP, pushes the stack arguments, calls, and
records EAX/EDX/ST0, the callee-saved registers and how much the callee popped. A
vectored handler turns a fault into a return through `tramp_escape`; a watchdog
thread does the same for a call that runs over 100 ms, but only while EIP is inside
one of the images (stopping a thread inside wine's own code wedges the process).
When only one side hangs and the other returned, the pair is rerun with 3 s each:
our `-O0` code is several times slower than VC8's, and a long finite loop must not
read as a hang. If no call finishes for 8 s more than that, a wild write has hit
wine's own memory; the host records the function as `wedged` and exits, and
`run.sh` runs each function in its own process so that costs one function only.
`DIFFTEST_TRACE=1` prints every mapped global before each call.

Compared afterwards: the return value masked to its declared size (`bool` as
zero/non-zero), ST0 for float returns, the whole arena, each mapped global, the count
of writes into either image outside the mapped globals, EBX/ESI/EDI/EBP where the
original preserved them, and the popped byte count against the spec. An address
into the original image that names a mapped global compares equal to the same
global's address in ours. Faults agree when both raise the same exception code. A
hang against a fault counts as agreeing (the faster side reaches the guard page, the
slower one the watchdog); a hang against a normal return does not.

A failing input is shrunk: each integer argument towards 0/1/2/-1/halves, then the
arena and the globals to all-zero, keeping each step only while it still fails.
`results.jsonl` holds the first failing input, the shrunk one, both diffs and the
first 32 bytes behind each pointer argument.

## The spec

`tools/mkspec.py` joins, on the 1.14d address:

- the d2fn snapshot: Ghidra's parameter storage, callees, flags, globals, the asm;
- the tree: the `.cpp.map` entry (name, namespace, file) and the header declaration
  of that name, preferring the header beside the .cpp (our convention and types);
  a declaration ending `// 1.14d: <addr>` is taken by its address;
- `nm` and one cached `objdump` of our exe: our address, our `ret N`, our calls.

A function is testable when it is a leaf in 1.14d (no CALL, no SEH, no indirect call,
no shared tail), has no RDTSC/CPUID/LOCK/FS: instruction, its parameters are scalars
or pointers (no struct by value, no 64-bit argument), Ghidra and the tree agree on
the parameter count and order (paired by name when the snapshot and the tree name
them alike, positionally only for a pure rename with matching pointer-ness), and both
ABIs check out against the asm: the original's
`ret N` pops exactly Ghidra's stack parameters, every register the original reads
before writing on its entry path is one Ghidra puts a parameter in, and our `ret N`
matches what our declared convention pops. Every writable global the original names
must have exactly one counterpart in our `nm`, by name (up to 32 KB of each is filled
and compared); a label inside another mapped global's extent is dropped, because on
the original side it is the same bytes (Ghidra's interior labels, e.g.
`gLightmapCellData` at `gaLightMapCells+4`); a struct-field label
(`base.field`) resolves to the emitter's split symbol `base_field`, else to `base`
plus the field's offset from the VC8 oracle's `datamap.tsv`. The original's ABI comes
from Ghidra because the binary fixes it; ours comes from the header because that is
what GCC compiled - the two differ freely (`__stdcall` with EAX storage in Ghidra is
emitted as a stack `__stdcall`) and the harness places each side's arguments its own way.

Spec lines (the host reads these):

```
F <id> <name>
A <orig va> <ours va> <ret kind v|i|u|p|b|f|d|q> <ret size> <orig pops> <ours pops>
P <name> <gen int|uint|bool|ptr|str|f32|f64> <size> <orig loc> <ours loc>   (loc: eax..edi|stack)
G <orig va> <ours va> <size> <randomize 0|1> <name>
E
```

## Extending it

- **More functions**: add addresses to a picks file, or `auto` with `--n`. The tsv
  says why each rejected function was rejected; the big buckets are the way in.
- **Callees** (10,982 of 13,357 functions call something): the next step is to
  call through - our callee and the original's callee are both mapped, so a
  non-leaf whose callees are themselves pure works today once the leaf filter lets
  it through and the callee's globals are mapped too. Where a callee has side
  effects (allocation, I/O, the unit lists), stub it on both sides with the same
  recorded answer: patch the original's CALL targets and our symbol's first bytes to
  a host thunk that replays a scripted value and logs the arguments, then compare
  the call logs as one more output.
- **Global state**: only globals Ghidra names in the function's bundle are mapped
  and randomised. A global reached through a pointer the function computes is not;
  its writes show only as a stray-write count. Runtime-initialised tables (the txt
  data tables, `sgptDataTables`) are zero on both sides, because no initialiser ran;
  randomising them or loading real txt is how to reach the code behind them.
- **Floating point**: both sides start with `fninit` and control word 0x27F (the
  MSVC default, 53-bit precision). ST0 is compared bit-exactly for float returns.
  GCC at `-O0` spills x87 temporaries through memory at their declared width while
  VC8 keeps them in 80-bit registers, so a bit-level difference in a float result can
  be the compiler, not the source; compare with a ULP tolerance before calling one a bug.
- **Different builds**: any unstripped PE of the tree works, e.g. a `LOUD` build.
  `d2recon-vc8.exe` has no symbols, so it needs the VC8 oracle's `funcmap.tsv`
  instead of `nm`.

## Limits worth knowing

- A pass means "agreed on these inputs". The generators are random with edge values,
  not coverage-guided; a branch behind a magic value is reached by luck.
- Inputs that violate a function's invariants (a bit position above 7, a negative
  index) make both sides read out of bounds. Where that read lands in a constant
  table's neighbour the two images hold different bytes, so the result differs
  without either being wrong. Read the minimal input before calling a bug.
- Functions that fault on almost every input (struct-walking getters fed random
  structs) count as equivalent on agreeing faults; `summary.md` lists them under
  weak coverage.
- Ghidra storage comes from the d2fn snapshot (v1054); a prototype fixed since then
  shows as an ABI note or an untestable verdict. Refresh with `d2fn snapshot`.
