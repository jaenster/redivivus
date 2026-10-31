---
name: redivivus
description: How to read the Redivivus Ghidra databases for Diablo II 1.14d (Windows Game.exe and the Mac build) through the redivivus MCP server, and how to propose a correction that survives review - the evidence it needs and the house naming rules. Load before any redivivus tool call or before proposing a name, type or prototype.
---

# Working with Redivivus

Redivivus is Diablo II 1.14d recovered as C++ source that compiles, links and runs. The source is
generated from two Ghidra databases. Better names, types and prototypes in Ghidra make better source,
so that is where a contribution lands.

## Reading

- Every tool takes `program`: `"win"` (1.14d `Game.exe`, the default and ground truth) or `"mac"` (the
  Intel Mac build from the same source revision, built with another compiler).
- Everything is read-only. Address functions and data by address (`0x00456789`), not by name: names
  change, addresses do not.
- Start narrow: `get_function_info` or `decompile` one function, then `get_xrefs` and the callers or
  callees it touches. The batch tools have a size cap and a tighter quota.

## Proposing

The server has no proposal tools yet. Until the review flow arrives, a correction goes as an issue on
https://github.com/jaenster/redivivus/issues, using the **Correction: name, type or prototype**
template. One issue is one claim, and it carries:

- **the target by address**: for locals, the function address plus the storage (stack offset or
  register); for types, the type path.
- **the value you saw**, so a reviewer can tell whether it changed in the meantime.
- **evidence a reviewer can re-check**: an xref, a call site, the Mac counterpart, an assert string, a
  string reference. "It looks like" or "the name suggests" is not evidence.

Accepted corrections land in Ghidra and so in the next regeneration of the source.

## Structural suspicions - file, never propose

Some defects are not a wrong name or type but a wrong **function set**: data that is really code,
code that is really data, one function that should be two, two that should be one, a call newly
found never to return. Never route these through a proposal, however good the evidence. File them
with the **Structural: function discovery/boundary** issue template.

The reason is cost, not ceremony. A change to the function set moves the set of translation units,
so it invalidates every cached artefact - the dependency graph, the per-function export, the
emitted tree. The daemon cannot absorb that incrementally; it detects the change and triggers a
full rebuild, the same path as the weekly full reload. So a human applies it in the writable
database and schedules that rebuild. A proposal that quietly depends on a function which does not
exist yet fails at review - later, and more expensively than the issue would have.

One issue is one claim, and it carries:

- **the address or range**, and how Ghidra classifies it today (undefined, defined data, code
  inside an existing function, already its own function);
- **what you think it should be**, in a sentence;
- **how you found it**: the vtable or pointer table whose other slots are entry points, the xref,
  the call site - plus a disassembly snippet showing the prologue or the boundary you are claiming;
- **the addresses of the functions you expect to change**: the callers, the function being split,
  the one the bytes currently sit inside. Best effort is fine; it is what makes the rebuild's
  result checkable.

Do not disassemble or define anything to "check" first. Everything you have is read-only, and the
suspicion is reportable from xrefs, the surrounding bytes and the table you found it through.

## When to name at all

- **Only default names are fair game.** Never replace a name someone already chose. If you think it is
  wrong, propose that with the evidence and let review decide.
- Name by **meaning derived from field accesses and callee names**, never from a decompiler guess or a
  name that merely looks similar elsewhere.
- **Unclear: use the type-based fallback** (`pTmp`, `nTmp`, `bTmp`, `dwTmp`). A wrong specific name is
  worse than a default one, because it reads as evidence.

Default names that need a name:
- params: `param_N`, generic `dwParam` / `nParam` / `bParam`
- locals: `local_XX`, `uVarN` `iVarN` `cVarN` `bVarN` `pVarN` and the like, `uStackN` `iStackN`,
  `extraout_*`, `in_*`, `unaff_*`
- globals: `DAT_*`, `PTR_*`, `UNK_*`; functions: `FUN_*`, `thunk_FUN_*`

Leave alone: stack-cookie and SEH slots, and CRT/STL/Win32 runtime code (`std::*`, `__crt*`, `_malloc`,
`mem*`, `str*`, `__security_*`, `_Eh*`, printf/scanf cores). **Storm and Fog are Blizzard code: name them.**

## Variables and parameters

|Prefix|Meaning|
|-|-|
|`p` / `pp`|pointer / pointer-to-pointer|
|`n`|int count, index, id|
|`b`|bool / BOOL|
|`e`|enum|
|`dw` / `w` / `by`|dword / word / byte (bit pattern, flags, raw)|
|`sz` / `wsz`|C string / wide string|
|`f`|float|
|`pfn`|function pointer|
|`a`|array|
|`i` / `j`|loop counters only|

Then PascalCase meaning: `pUnit`, `pRoom`, `nSkillId`, `dwFlags`, `szPath`, `bIsValid`. A free function
taking a struct in ECX names that parameter for the struct (`pGame`), never `this`.

## Globals

`g` plus the variable prefix: `gp`, `gn`, `gdw`, `gb`, `ga` (array), `gsz`, `gpfn`. Examples: `gpGame`,
`gaChatDlgHistoryBuffer`.

## Functions

- `MODULE_VerbObject`: an ALLCAPS module prefix, an underscore, then a PascalCase verb phrase:
  `ITEMS_GetQuality`, `DRLGROOM_AddClientToRoom`.
- Inside a namespace, no doubled tag: `Preset_InitGridsFromDS1File`, not
  `Preset_DRLGPRESET_InitGridsFromDS1File`.
- Packet handlers keep the opcode: `NET_D2GS_SERVER_Send_0x26_Chat`.

## Types, fields, enums

- Structs: `D2<Thing>Strc` (`D2UnitStrc`, `D2RoomStrc`), compatible with D2MOO and PlugY.
- Fields use the variable prefixes: `pNext`, `nNumClients`, `dwFlags`, `char szName[N]`.
- Enums: an `eD2<Thing>` type with ALLCAPS constants under a family prefix: `UNIT_PLAYER`.

## Evidence, strongest first

1. The developers' own words: assert strings and `__FILE__` paths in the binary.
2. The paired function on the other build (Windows and Mac).
3. Field accesses on an already-typed struct, and the names of the callees it feeds.
4. Strings it references (error text, txt column names).
5. Inference from behaviour alone. Use a fallback name if this is all you have.

Keep misspellings that are evidence (`AiTatics.cpp`). Never correct a Blizzard typo.

## Ghidra traps

Most real defects in the decompile are not bad names but values Ghidra lost: a dropped register, a
truncated table, a wrong calling convention. Read `reference.md` (next to this file) before proposing a
type, prototype or convention change: it lists each trap, how to confirm it from the asm and what to
propose.

## Windows and Mac stay in sync

A paired function or global ends with the same name on both builds. Windows is ground truth. If the Mac
side has assert evidence that Windows lacks, say so in the proposal rather than renaming either side.
