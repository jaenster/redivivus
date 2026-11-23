# mac/ - the 1.14d Intel Mac oracle, compare stage

The Mac build of 1.14d carries code the Windows executable does not: Storm's Mac
layer (a Win32 API on pthreads and Carbon), the OpenGL renderer, the Carbon
front end. That code is reconstructed by hand under `D2_OS_MAC`, and this tool
measures it against the shipped binary the way `msvc8/` measures the Windows
tree, function by function.

It is the compare stage only. The compile stage is the Mac build itself
(`cmake/mac.cmake`, toolchain `cmake/mac-i386-clang425.cmake`: Apple
clang-425.0.28 from Xcode 4.6.2, the compiler that built the original, with the
10.7 SDK). Linking is not needed: the comparison reads relocations.

## Running it

```
cmake -S . -B build-mac -DCMAKE_TOOLCHAIN_FILE=cmake/mac-i386-clang425.cmake
cmake --build build-mac --target d2_mac_objs
node asm_matching/mac/cmp.ts --build build-mac --binary <DiabloII_macho> [-v] [--tsv out.tsv] [filter...]
```

Node 22.6 or later (it runs the TypeScript directly), and Homebrew `llvm` for
`llvm-objdump`, `llvm-nm` and `llvm-cxxfilt`. The binary is the Intel Mac
`DiabloII_macho` of 1.14d, sha256 `1e8dd6dd...0d640d177`; the tool refuses any
other. `D2_MAC_BINARY` may name it instead of `--binary`. The binary's
disassembly is cached under `asm_matching/mac/build/`. A filter keeps the files
whose path contains it, or the function of that name.

## What it compares

A function or global is paired with the original by the comment on the line
before its definition, `// 1.14d mac: <addr>`, in any source under
`3rdparty/Storm/Source/Mac/`. A declaration in a header may carry the comment
too, which names a function or global that lives outside the reconstructed code.

For each function our object's instructions are compared with the original's,
over the original's whole extent, which the binary's `LC_FUNCTION_STARTS`
bounds. Trailing padding is ignored. Before comparing, both sides are
normalised:

- a branch target becomes the index of the instruction it lands on, so a
  branch to the wrong place is a difference while a shift of addresses is not;
- a call becomes its callee: an imported symbol by name, a reconstructed
  function by name (through its address comment), `call $+5` for the PIC
  thunk;
- a PIC reference, `[reg + disp]` off the register that popped the PIC base,
  becomes what it points at: an imported symbol (`&_name`, through the
  indirect symbol table), a string literal by its text, a paired global plus
  an offset, or a paired function.

A reference that one side can name and the other cannot is counted as
unverified, not as a difference: it is reported beside the result. `SAME`
with no unverified references is instruction-identical including every
callee and data reference. With `-v` each unverified reference is listed
under its function, both sides shown.

## Output

One line per function (`SAME` or `DIFF` with the first differing
instruction), then per source file and for the whole run the count and share
of functions that are instruction-identical:

```
== 3rdparty/Storm/Source/Mac/Win32/Win32_sync.cpp: 9/9 same (100.0%)
== total 77/92 instruction-identical (83.7%)
```

The share is over the functions reconstructed so far, not over the Mac-only
code as a whole. `--tsv` writes the per-function rows for tracking.

## What it does not prove

Instruction identity is not byte identity: an encoding choice (a short versus a
long jump) that prints the same is not seen, and neither is data. Function
order and alignment inside the TU are not compared, and neither is which TU a
function sits in, since the original's file boundaries inside the Mac layer are
mostly unknown.
