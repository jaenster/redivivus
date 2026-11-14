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

A correction is a proposal, sent with the server's `propose` tool. The full guide, with every op and
evidence kind, is https://redivivus.app/agent-setup/proposals.md; the house rules it applies are
https://redivivus.app/agent-setup/rules.md. Read both before the first proposal. In short:

- **One proposal is one claim**: 1-250 ops, applied all-or-nothing. Each op names its target by
  address (for locals the function address plus the storage, for struct fields the type path and
  offset) and carries `expect`, the value you read just before, so a change made in the meantime is
  caught.
- **The ops**: `rename_function`, `rename_global`, `rename_local` (parameters too), `retype_local`,
  `set_prototype`, `set_global_type`, `struct_field`, and on Windows two that record what a VC8
  compile proved: `vc8form` (a source-form tag) and `linkage` (file-static, or the file a function is
  defined in). Both of those need an `oracle` evidence item for their function.
- **Evidence a reviewer can re-check**: at least one item that is not `free_text` - an xref, a call
  site, a Mac pair, an assert or string reference, a field access, or the VC8 audit status before and
  after (`oracle`). "It looks like" or "the name suggests" is not evidence.
- **Checked at once**: `propose` verifies every op and evidence item live against the databases and
  stores nothing if one fails, listing every problem. `dryRun: true` runs the checks only.
  `my_proposals`, `proposal_status` and `withdraw` follow it from there.
- **A vc8form tag is tied to its function's decompile** (its `dh`). A `set_prototype`, or a renamed or
  retyped parameter, changes that decompile (a prototype change its callers' too), and the generator
  then ignores the tag. When your change touches a tagged function, measure the tag again on the
  changed decompile and put it in the same proposal as a `vc8form` op with `replace`.

Accepted proposals are applied to Ghidra and so land in the next regeneration of the source.

Open an issue on https://github.com/jaenster/redivivus/issues instead when the checker cannot take the
claim:

- **renaming a name someone already chose**, a function, global, local, parameter or field; the checker
  only fills in default names (below) and refuses the rest;
- a defect no op expresses: a lost register, a truncated table, a wrong generator output;
- a new generator form, a filing rule, anything the generator has to learn (with the measurements);
- a disagreement with these rules or with a refusal of the checker.

An issue is one claim too, with the same target, value seen and evidence.

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
  wrong, open an issue with the evidence and let review decide.
- **Reordering parameters is not renaming them.** A `set_prototype` that only puts the existing
  parameters in another order (each keeps its name and type; same count, return type and convention;
  storage may follow the convention or be custom) leaves every chosen name alone. It changes what every
  call site passes, though, so it needs evidence of the new order: a `mac_pair` whose paired function
  takes its parameters in that order (by name, or by type where that tells them apart; a swap of two
  parameters of one type needs the pair's names), or a `call_site` for a call to the function (or to
  its Mac pair) whose pushes show the order. A reorder that also renames or retypes a chosen-name
  parameter is refused as any rename of a chosen name is.
- Name by **meaning derived from field accesses and callee names**, never from a decompiler guess or a
  name that merely looks similar elsewhere.
- **Unclear: use the type-based fallback** (`pTmp`, `nTmp`, `bTmp`, `dwTmp`). A wrong specific name is
  worse than a default one, because it reads as evidence.

Default names that need a name:
- params: `param_N`, generic `dwParam` / `nParam` / `bParam` and the pointer forms `pnParam` / `pdwParam` /
  `pbParam` / `pwParam` / `pcParam` / `ppParam`
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
|`h` / `u`|Win32 handle (`hWnd`) / Win32 `UINT` (`uMsg`)|
|`i` / `j`|loop counters only|

A pointer to something that has its own prefix puts `p` (or `pp`) in front of that prefix: `pnPosition`
(`int *`), `pdwFlags`, `pbDone`, `pwPort`, `pbyData`, `peMode`, `pfScale`, `pszName`, `pwszName`,
`ppszList`. The checker reads these as pointers. Plain `p` stays right for a pointer to a struct or to
memory with no prefix of its own (`pUnit`, `pBuffer`).

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
