# Reading Ghidra's output for Diablo II 1.14d: traps and good proposals

This is a field guide for proposing changes to the 1.14d `Game.exe` database. Each trap below
compiles cleanly after rebuilding and breaks at runtime. Every entry says what you see in the
decompile, how to confirm it from the asm and xrefs, and what to propose.

Two rules apply to everything here:

- **The machine code decides.** Names, the decompiled call sites and how plausible something looks
  are not evidence. The disassembly, the `RET n` purge, the operand widths, the xrefs and the
  bytes are.
- **Fix it in the database, not in the output.** A type, prototype, convention, label or region
  change in Ghidra fixes every site at once. A cast or hand edit in generated code fixes one site
  and hides the rest.

## Evidence rules

|Rule|What it means in practice|
|-|-|
|Record the evidence class, not only a confidence|Say *how* you know: "RET 0xc purge", "caller loads ECX at every call", "loop bound reads 17 records". "High confidence" says nothing about what could prove you wrong.|
|One disproof invalidates the whole class|If a method is shown wrong once (for example, arity counted from decompiled call sites), re-check every finding that used it. Expect the list to shrink. That is correct.|
|Decompiled arity is not arity|Ghidra can show fewer arguments than the callee takes, or more. The stack purge (`RET n`) gives the stack byte count. Register arguments need the callee's entry read register by register.|
|A name carries no weight|Functions in this database have been named after what they call, or off a table shifted by one row. Never use a name as a premise. If the opcodes disagree with the name, the name changes.|
|The neighbouring symbol proves nothing about extent|Arrays were once filled to the next label, so a declared length often just equals the gap. Settle extents from content, index form, interior xrefs and loop bounds.|
|Rank by callee, not by call site|Hundreds of bad sites usually come from a few dozen callees. Propose one prototype per callee, and list the sites it fixes as the evidence.|
|Occurrence counts lie|A local once named `extraout_EAX` keeps that name after the callee is fixed. A real synthetic is only ever read, never assigned. A function that just sits near many such lines proves nothing.|
|Check it does not already exist|Before proposing a new type or struct, search the data types. Duplicates make things worse.|
|Mac pairing is by address|A Mac/Windows correspondence is (Windows address, Mac address, extents, evidence). Names and types are derived from a pair, never used as the key. Watch for a Windows helper that the Mac build inlined into its caller: the anchor then sits in the caller.|
|A Mac data ref at 0x0061xxxx is not a callback table|Those refs sit in `__LINKEDIT`: they are the uleb128 `LC_FUNCTION_STARTS` entry every Mac function has. Only a pointer in `__DATA` (a table slot such as 0x003c7960) shows the function is called through a table.|
|Outside reimplementations only corroborate|They can confirm an id or enum meaning. They never decide a name or a type.|

A good proposal states: the address, the current state, the proposed change, the asm lines
(address + instruction) that prove it, and the sites it is expected to change. If you are not
sure, say what would settle it.

## Register contract mismatches (`extraout_`, `in_`, `unaff_`, `in_stack_`)

These are one bug in four forms. Ghidra's declared signature disagrees with what the machine does
with registers.

|Form|Decompile shows|Confirm|Propose|
|-|-|-|-|
|Register parameter modelled as a local|A local declared, never assigned, then read or dereferenced (`in_ECX`, or a renamed one)|The callee reads the register before writing it, and every caller loads it within a few instructions before the `CALL`|Add the parameter with register storage, or switch to `__fastcall`/`__thiscall`. If any caller does not set it, it is not a parameter.|
|Void callee that returns a value|Callers read `extraout_EAX`, or a local that is never assigned (often a constructor that returns `this`)|EAX is written on every return path and is live at `RET`; callers use it|Give the callee its real return type|
|Return declared too narrow|`extraout_var << 8 \| x & 0xff` splices in callers|`MOVZX EAX,AL`, `MOV EAX,imm32` or `XOR EAX,EAX` on **every** return path|Widen the return. A Ghidra `bool` is one byte, so a function ending `MOVZX EAX,AL` wants `int`/`BOOL`.|
|Register not preserved across a call|A value split into two variables around a `CALL`; the second half is never assigned|The callee never touches that register (usually EDX or ECX)|Fix the **callee**: a prototype model that leaves the register unaffected, or `inline` on a tiny leaf with few callers|

Traps:

- Check every return path, not one epilogue. An early exit that writes only AL means the callee
  is right and the caller must stop reading 32 bits.
- Do not widen an enum *type* that is also a struct field. Change only the return.
- `float10` returns come back in ST0 and look like `in_register_00001000`. That is correct.
- `__thiscall` supplies `this` in ECX. Declare only the remaining parameters.
- A new phantom after your change is usually caused by it. Toggle the change back to check whether
  the phantom was already there.
- A convention change alters the **callers'** decompile, not the callee's. Re-decompile the callers
  to see the effect.

**Spliced register halves.** `(uint)in_register_00000009 << 8 | (uint)param & 0xff` used as an
index or switch value. `in_register_*` is uninitialised, so the high bits are garbage. Confirm the
callee reads the full register somewhere, then propose the parameter at its true width.

**Narrow stack parameters.** `in_stack_000000NN` where NN is exactly the offset plus the size of a
declared 1- or 2-byte parameter. Confirm the instruction reading the slot is `dword ptr` (`RET n`
agrees with both widths, so it settles nothing here). Propose widening the parameter, then re-check
its callers, because the defect moves up one level.

## Calling conventions

**`unknown` is wrong, not unset.** A funcdef with an unknown convention falls back to the
program default (`__stdcall`) and derives a callee purge from the parameter count. If the real
callee is `__cdecl`, ESP is credited twice and the stack model drifts at every site. Propose the
real convention on every funcdef you touch.

**`code*` calls are cdecl.** `(**(code **)p)(args)` calls through a variadic type, which is cdecl.
If the real target is `__stdcall` it pops its own arguments, and ESP drifts on every call. You only
see it in a loop that never returns. Confirm with the target's `RET n`. Propose a funcdef with the
right convention for the slot or field.

**Indirect calls with no arguments.** `(**(code **)(...))()` with an empty list is almost always a
hidden `__fastcall` or `__thiscall` call. Confirm: `MOV ECX,<obj>` (and often `XOR EDX,EDX`) just
before `CALL <reg>`. Propose, in this order:

1. Type the object: the pointer global, its struct, and the callback fields as funcdef pointers.
   The decompiler then emits `(*p->fpCallback)(p)` by itself.
2. If the code walks the table by byte offset so typing cannot reach it, propose a call-site
   signature override at that call address.

**Untyped virtual calls** drop `this` the same way. Propose typing the receiver and its vtable.

**`__fastcall` versus `__stdcall`.** `RET n` gives the stack bytes but not the convention. Count
the ECX/EDX loads between the argument set-up and the `CALL`. A push-only count is two short for a
fastcall callback.

**Funcdef and function must agree.** A table of `__fastcall` handlers needs a `__fastcall` funcdef.
`void*[N]` is not a fix: a call through `void*` does not compile. Use `Fn*[N]`.

**Varargs typed as fixed.** Symptom: a printf-like function that takes the address of its last
named parameter and steps it forward until NULL, while Ghidra gives it a fixed list. Confirm: the
caller pushes more dwords than declared (count the `ADD ESP,n` after the call), with a NULL last.
Propose marking the function varargs. The decompiler then recovers every argument, including the
sentinel.

**Void callbacks that return a value.** Symptom: a `void` function stored in a slot whose funcdef
returns a value, or cast to a non-void function pointer. Confirm: the body sets EAX or AL on purpose
before `RET`. Propose the real return type. A callee with custom storage may need its return set
with explicit storage, for example AL.

**x87 helpers.** `_ftol`-style helpers pop ST0, which no prototype can express. Callers then read
`extraout_ST0`/`ST1`. Propose a call fixup that truncates ST0 into EAX, shifts the x87 stack, and
also does `ESP = ESP + 4`. Without that last part, paths with different numbers of fixup sites
disagree about the stack where they merge. Integer helpers called between x87 ops only need a
model that does not kill ST.

**`DWORD64` parameters.** Do not declare one as a single 8-byte type. Ghidra 8-byte-aligns it and
x86 does not, so every later argument shifts. Split it into low and high dwords.

## Labels, extents and tables

### Stray label truncates an array

**Symptom.** A table declared far shorter than the code indexes it, with odd-looking labels on
interior elements.

**Confirm.** Run `get_xrefs` on each interior label. If every reference comes from the same
function that reads the array, it is an element, not a global. Compare the largest index the code
uses with the declared length.

**Propose.** Delete the interior labels and retype the array at its real length. The same applies
to stack frames. A frame run split into fragments shows as a regular stride in `get_stack_frame`.

### Table symbol at the centre

**Symptom.** `gTable[dx + dy*3]` with signed deltas and no constant offset, which looks in bounds.

**Confirm.** Take the base literal from the disassembly and subtract `min_index * elemsize`. The
bytes there should be plausible table data, and `base + (max_index+1)*elemsize` should land exactly
on the next symbol.

**Propose.** Delete the centre label and define the array at the true base. Ghidra then renders
the access as `gTable[dx + dy*3 + 4]`.

### Scalar global indexed as an array

**Symptom.** `(&g)[i]`, or `*(T*)((char*)&g + i*stride)`, on a global declared as a scalar. Often
there are strays named `g1`, `g2` next to it.

**Confirm.** The element base is often *before* the symbol: negative offsets from the anchor in the
walk show the struct's real start. Pin the count three ways: a loop bound or count global, an
unrolled test that stops after N, and the run ending exactly at the next real symbol. A sibling at
the same stride that is already typed correctly is strong support.

**Propose.** Delete the strays, then set `T[N]` or `struct[N]` at the true base.

### Init table typed as its first element

**Symptom.** A subsystem that half works. A registration or lookup table has only record 0, and
the rest have no xrefs of their own.

**Confirm.** The free or iterate loop states the end (`&rec[0].field + 272` with stride 16 means 17
records). The end lands on the next symbol. Read the bytes too: the end of a table is often
obvious from the data (an ASCII tag, a terminating record).

**Propose.** The array type at the true base and count.

### Memset spans a run of globals

**Symptom.** `memset(&gSmall, 0, 0x160)` into a global much smaller than the size.

**Confirm.** Find the next labelled symbol and compare the gap with the write size. If other
labelled globals sit inside the range, it is a region, not an undersized scalar.

**Propose.** A struct covering the region, with the existing labels as fields. Do not retype the
first symbol as an array that swallows its neighbours. If only the size is wrong (no labels inside),
it is a simple undersized array.

### Pointer-typed global that is a buffer

**Symptom.** A 4-byte global (often typed as a pointer) that is the destination of a copy loop with
a constant trip count above 1. Confirm the count times the element size matches a known object
(for example, a saved copy of another table). Propose the real array or struct type.

### A heterogeneous table usually has the wrong extent

**Symptom.** A function-pointer table whose entries differ wildly in arity or return type. If the
difference follows *position* (uniform, then abruptly different), the extent covers two or three
objects. Separate the families by stack purge and propose the real tables, each with one funcdef.
Interleaved differences may be genuine.

### Settling an extent

From strongest to weakest: content structure (a known sequence or published table), the index form
at the use sites, xrefs into the declared range (a string reference inside proves the array ends
earlier), and divisibility by the element size. A struct applied over a string pool is the same
defect in another form. Search by what the bytes are. If you cannot settle it, bound the array at
the largest extent with no literals in it and say what would settle it.

## Structs and types

**Bitfield unit is one byte.** Ghidra writes `int:1` but records `size: 1`, packing 8 bits per
byte. The storage unit is the byte. When you check or propose struct layouts, use the field's
`size`, not the `int` prefix. A signed one-bit field holds 0 and -1, never +1, so bitfields should
be unsigned.

**Multi-dimensional arrays.** Ghidra prints dimensions outer first, as C does: `T *[5][64]` is 5 rows
of 64. Before proposing a transposition, take the stride from the disassembly (reads at
`+0x004/+0x104/+0x204` mean a row stride of 0x100). A transposed pair keeps the same total size, so
no size check catches it.

**Untyped field holding a callback.** A struct field left as `undefined` bytes whose initialisers
fold, little-endian, to a function's entry address is a callback. Without a pointer type it is
emitted as a raw, unrelocated address. Propose a funcdef pointer type with the right convention,
and type the table pointer that reaches it too.

**Signedness.** A global typed unsigned that the code divides with `IDIV` or tests with `JL`, `JGE`,
`JNS` or `JLE` is signed. One `int` retype fixes every site.

**Retype before renaming.** Fix types first. Many odd-looking locals turn into ordinary ones once
the struct or parameter type is right.

## Stack frames

### Split packet buffers and split frame objects

**Symptom.** A sender passes `&local, N` to a send or copy helper, where the local is smaller than
N. Or a frame object is emitted as several separate locals.

**Confirm.** `get_stack_frame` shows the contiguous run starting at the buffer's base. Order the
fields by offset. Never infer the layout from the declaration order in the decompile.

**Propose.** One struct or byte array covering the whole run at the base offset.

### Address of a scalar local

**Symptom.** `&firstScalar` passed to a callee that writes `p[1]`, `p[2]` and so on.

**Confirm.** The callee's stores give the byte count. Count bytes, not elements: four `char`
stores into a 4-byte int are one word.

**Propose.** One array (for example `int[9]`) at the first slot's offset. Not a cast at the call.

### Byte store into a PUSH spill slot

**Symptom.** A local whose address is passed and which is never assigned in the decompile, while
the asm stores a byte or a register into it just before the call.

**Confirm.** The slot overlaps a `PUSH <reg>` spill, and Ghidra eliminated the narrow store as a
partial overwrite.

**Propose.** Clear the locals over the slot and declare a `byte[4]` covering the whole slot. An
element store survives where a scalar store does not. Look for a stale oversized local hiding it.

### Frame names on the wrong slot

**Symptom.** The decompile looks fine but reads the neighbouring slot. A value that should be a
count behaves like a pointer.

**Confirm.** Find a call that takes `&local` (any out-parameter) and read the true slot from the
`LEA`/`PUSH` before it. Compare with the listing's name at that offset.

**Propose.** Move the names onto the right slots. Offsets do not change, only which name sits where.

### Folded addresses: `&stack0xNNNN` and interior globals

**Symptom.** `&stack0xNNNN` at an offset in the saved-EBP or return-address area, or
`(cursor - 0x969021) / 0x2c` with a constant inside a global.

**Confirm.** Read it as arithmetic. The frame terms cancel to an address inside one frame object,
or the constant falls inside exactly one global and the divisor equals its element stride.

**Propose.** Type the frame object or array the arithmetic walks, so the base is a real symbol.
If no type change can reach the site, flag it rather than substituting a value.

## Control flow

**`CALL_RETURN` override on a conditional jump.** Symptom: `func_0x<addr>()` in the decompile, or
a "callee" that takes EBP as a parameter and turns every frame local into an offset from it, while
the caller passes garbage via `in_stack_*`. Confirm: the opcode bytes at the referring site are
`0f 8x` (conditional jump), not `e8` (call). Count references to the target that are neither the jump
nor from inside the host: none means it is a split artifact, and any means it is a genuine
conditional tail call. Overrides on `CALL` and unconditional `JMP` are normal, and Ghidra already
undoes the unconditional ones itself. Propose: clear the override, remove the fabricated function,
and fix up the host body so it covers the block again. Keep the host's name and prototype.

**A function that never returns control.** Symptom: a short function with no prologue, no callers,
and reads of another function's EBP frame. A set of "handlers" with the same parameters in permuted
order is another sign. Confirm: the last instruction is not `RET` or `JMP`. The code falls through,
or is a case block of a jump table. Propose: demote it to a label and reclaim the body into its
host. Check whether the host was then missing a parameter.

**`if (false) { switchD_..._caseD_N: ... }`.** This is Ghidra's idiom for a case reached only by a
`goto`. It is usually faithful. Confirm: search the function for a `goto` to that label. A label
plus a goto is fine. A label alone is genuinely unreachable and worth a look.

**Switch tables.** In `get_switch_table`, a `caseD_<n>` target means the cases were recovered and
there is nothing to fix. A `caseD_<addr>` target means they were not, and an override helps. A
"callback table" whose entries are not function starts is usually a jump table.

## A short checklist before you propose

1. Read the disassembly of the function and of at least one caller. Quote the lines.
2. For a prototype: `RET n`, register reads at entry, register loads before each call, and every
   return path.
3. For a data extent: interior xrefs, the next real symbol, the largest index used, and the bytes.
4. Choose the fix at the source: callee rather than caller, type rather than cast, region rather
   than head symbol.
5. State the evidence class and what would prove you wrong.
6. Say which call sites or functions you expect to change after the fix.
