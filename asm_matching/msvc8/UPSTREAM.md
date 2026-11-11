# What the generator and the database should change

The oracle finds differences between the tree and `Game.exe` that the
generator (ghidra-reconstruct) or the Ghidra database produces the same way
everywhere. Each was first fixed in this tree by a Python fixer (the
`fix_*.py` named with each rule, since retired: the rules are
ghidra-reconstruct passes, Ghidra database fixes, or forms the vc8form
permuter finds), and `evidence/fixes.tsv` records every run: sites proposed,
edits kept, functions that came to match. Here is each rule, with the
evidence it rests on.

## The generator

**Comparisons in the form the instruction shows** (`fix_compare.py`). Ghidra
prints only `<` and `<=`, moving constants and operands to fit: `x >= 0`
comes out as `-1 < x`, `x <= 4` as `x < 5`, `a >= b` as `b <= a`. VC8
compiles each form as written, so the binary says which one Blizzard wrote:
the constant (`cmp x, 4` against `cmp x, 5`), the strictness of the jump
(`jl` against `jle`) and the order of the operands. Example:
FILETOOLS_GetUserMyDocumentsDiabloIIGameLogsFolder (0040f000), `-1 < nResult`
compiles to `cmp eax, -1; jle`, the original has `test eax, eax; jl`.
An unsigned `x > 0` comes out as `x != 0`; where the original's `jbe`
stands against the tree's `jne` the if/else is also the other way round,
and both are restored together (GFX_PrecacheMonsterCofSprites, 0046f650:
`if (c == '\0') {A} else {B}` is `if ((uint32_t)c > 0) {B} else {A}`).

**Assertions in front of the code they guard** (`fix_assert.py`). Blizzard's
assertion is `if (!(cond)) { ERROR_UnrecoverableInternalError_Halt(...);
exit(-1); }` followed by the guarded code; the decompiler turns it inside out
when the guarded code returns: `if (cond) { return x; } halt; exit(-1);`.
ASYNCDATA_GetReadSize (0040aa40) matches once it is put back.

**If/else branches in the original's order** (`fix_ifelse.py`). VC8 lays an
if/else out in source order, the then-branch falling through; where the
original falls through into what the tree has as the else branch, the
branches are swapped and the condition negated (DumpLog, 004082e0). With
the branches the other way round the layouts often do not align, so every
`if (!x) {A} else {B}` of one operand is also tried as `if (x) {B} else {A}`
and kept where its function comes out more similar: Blizzard tests the
pointer first and handles its absence after.

**`static` for file-local functions the original passes arguments in
registers** (`fix_static.py`). VC8 gives a static function whose every call
it sees a convention of its own - an argument in EAX, ESI, EDI - and the
original is full of them; the tree declares every function extern. The
original's live-in registers (a data flow over its code, `classify.live_in`)
say which. IsOtherAct (00548a80) takes pUnit2 in EAX and matches once static.
A promotion is also undone where the function's flags make VC8 choose a
convention the original does not have (`--demote`).

**Files by object, not by namespace** (`fix_refile.py`). The generator files
a function in the file of its Ghidra namespace, and so splits some of
Blizzard's objects: a function the original passes arguments in registers of
VC8's choosing was compiled in one object with every caller - only a static
function of its callers' TU gets such a convention. The CACHE_ functions of
the `Bnclient` namespace, CACHE_ComputeCRC32 (00518de0) the first, take their
arguments in EAX, ECX, EDX and ESI from Bnclient/Cache and lie right ahead of
its functions in the original, in the object CACHE.CPP made; the GfxInfo
functions annotated `.\ENGINE\Gfx.cpp` lie among Gfx's. The rule: file a
function with the functions its convention binds it to - in the file the
annotations around them name (an annotation names the object around its
function: `.\MONSTER\MonsterMode.cpp` speaks for the Dispatch/Monster
functions next to MONSTER_SetVelocityPercent, 005a6380), else with most of
them - at its address among them, and keep its namespace, which is its name;
make static what only that file then calls. Name prefixes are no evidence:
they are the database's, and DRLGPATH_SetPathPosition (0064fb90) lies among
Source/D2Common/PATH/Path's functions. A group whose callers lie hundreds of
functions apart is no object - /GL, or a misread convention - and stays
where it is (`evidence/objects.tsv` lists what is left).

**Switch statements** (`fix_switch.py`). VC8 lowers a switch over a few
cases as successive subtractions (`sub eax, 1; je`, `sub eax, 3; je`), an
if-chain as comparisons; Ghidra hands the switch back as nested or chained
ifs. Where the original subtracts, the chain was a switch
(ERRORMANAGER_ServiceControlHandler, 00406410). A switch whose cases all
leave the function also comes back as separate ifs, `if (!v) { ...; return
a; } if (v == 1) { ...; return b; }` (SFILE_ReadFileChecked, 004154b0). Only
clean chains and such runs on one variable are converted here; the nested
forms need the decompiler's own switch recovery.

**Cases in the order they were written** (`fix_caseorder.py`). The
decompiler writes a switch's cases in order of their values. VC8 lays the
case bodies out in source order, and the jump table names each body's
address, so the original's table gives the order Blizzard wrote them in:
sort the case groups by the address their values jump to.
PLAYER_EndInteraction (005350f0) matches with UNIT_MONSTER, UNIT_OBJECT,
UNIT_PLAYER, UNIT_ITEM in that order. The generator, which knows the
table, could emit the cases in the order of their bodies' addresses (a
group that falls through into the next keeps its neighbour).

**Loops as they were written** (`fix_forloop.py`). Ghidra rotates a for
loop into `if (0 < n) { int i = 0; do { ...; i++; } while (i < n); }`;
compiled again the counter is set after the test instead of before it.
InitPal (00476ea0) matches as `for (int i = 0; i < n; i++)`. The generator
could recognise the rotation (the if's condition is the loop's, at the
initial value) and emit the for loop.

**No explicit stack-cookie checks** (`fix_guardstack.py`). Ghidra shows `/GS`'s
`__security_check_cookie` as a call, and the tree has it as
`Fog::Debug::GuardStack();`; VC8 under `/GS` emits the check itself, so the
statement adds a second call (NET_D2GS_SERVER_Send_0x95, 0053c320).

**Stores in the order they were written** (`fix_order.py`). The decompiler
lists independent stores in the order the original's instructions make them,
which is VC8's schedule, not the source's; compiled again they come out in
yet another order. Blizzard filled packet structs field by field:
NET_D2GS_SERVER_Send_0x0C_UNITCHANGE (0053b430) matches once its fields are
assigned in member order - header byte first - and VC8 schedules them as
the original has them. The generator could emit a run of independent stores
into one local in member order. Member order is not always it: the three
fields D2GfxDataStrc gets after `memset(&sGfxData, 0, 0x48)` stand in all
six orders in the tree, and which one compiles to the original's schedule
is found by trying them (`fix_permute.py`). A global's store is part of
such a run too: SFILE_InitializeFileSystem (00406870) matches with
`D2Ini_bUseDirect = bResult` written before `dwSysOptionBuf[0] = 0x8000`.

**Operators Ghidra could not fold back** (`fix_idioms.py`). The seed modulo
`s.nSeedLow + (int)(((uint64_t)d2_bits_of(s) & 0xffffffff) / N) * -N` is
`s.nSeedLow % N`; written with the 64-bit cast VC8 calls `_aulldiv`, the
original multiplies by a reciprocal. A real 64-bit division the decompiler
shows as the runtime call with its operands split into dwords,
`__alldiv((uint32_t)E, (uint32_t)((uint64_t)E >> 0x20), 100, 0)`, which the
tree's d2_platform.h helper puts back together before dividing; written as
`(int64_t)E / 100`, VC8 pushes the halves and calls `_alldiv` as the
original does (D2ApplyPercent, 00483360). The same 64-bit cast hides the
32-bit remainder of the seed's low dword, `(uint32_t)(((uint64_t)d2_bits_of(s)
& 0xffffffff) % (uint64_t)n)`: Ghidra's reading of `xor edx, edx; div`, which
is `s.nSeedLow % n` (UNIT_GetModuloFromSeed, 0045c390, the inlined
SEED_RollLimitedRandomNumber); likewise the quotient, and a bit test of the
low dword, `(uint64_t)d2_bits_of(s) & 1`, is `s.nSeedLow & 1`. A zero extension, a
cast to a byte or a word, comes back as a mask, `nUpper & 0xff`: compiled as
written VC8 masks (`and eax, 0xff`) where the original has `movzx eax, al`
(SFILE_Hash, 00415330); `(uint8_t)nUpper` is what it compiles to that.

**Zero-fills and copies as the loops they were** (`fix_stringops.py`). VC8
SP1 expands no memset beyond 32 bytes, and an index loop it unrolls or turns
into `rep stosd`: `for (i = 0; i < 25; i++) a[i] = 0;` on a uint16_t array is
the original's twelve dword stores and a word store. Ghidra gives these back
as `memset(a, 0, 25 * sizeof(a[0]))`, or, for `rep stosd`, as a pointer loop
`for (n = K; n; n--) { p[0] = 0; p[1] = 0; p += 2; }`; compiled again, the
first is a call and the second a loop of word stores
(SKILLDESC_FormatSignedValue, 004e79e0; AUTOMAP_DrawGamePassword, 00459e90).
`rep movsd` comes back as `for (n = K; n; n--) { *d = *s; s++; d++; }`, where
memcpy of a constant size or a struct assignment is what VC8 compiles to it.
The generator can recognise both string instructions and emit the loop and
the memcpy.

**Calls nested in argument lists** (`fix_nestcall.py`). VC8 pushes stack
arguments right to left, so a nested call `G(F(x), y, 4)` evaluates `4` and
`y` before calling F; the original shows it as a push ahead of the inner call
(Q04_ActiveFilter, 00592fb0: `push 0x20736b62` before `call GetGamePointer`).
The decompiler stores F's result in a temporary used once, which VC8
compiles call first. Where the arguments right of the temporary are pushed
before the inner call in the original, the generator can fold the temporary
back into the argument list.

**Allocator calls with the original's source location** (`fix_allocargs.py`).
Fog's allocator takes the caller's file and line, and the original passes
Blizzard's (`".\\Src\\Exceptions.cpp"`, 0x1f7 in FOG_FreeModuleList, 0040b660).
Since 1b24c75 the tree writes these calls as D2_ALLOC / D2_FREE / D2_NEW and
their _POOL forms, which pass the reconstruction's own `__FILE__` and
`__LINE__`; every such call then differs from the original, and most of the
functions that commit stopped matching differ in nothing else. The values
are in the original's code at each call, so the generator can keep them -
for instance with macros that take the location as arguments.

## The database

**Functions marked inline that the original calls** (`fix_uninline.py`). A
function marked inline is decompiled into its callers; the original calls
it. InitSeed_Param_666 (00650e40) is the clear case: its two assignments
stand where the original has `call 00650e40`. STRING_ZeroOneWCHAR
(0042f530) and UNIT_GetModuloFromSeed (0045c390) are called by the original
from functions the tree inlines them into.

**Return values the signatures dropped** (`fix_return.py`). A function the
database types `void` whose original loads EAX with a constant before it
returns (`mov eax, 1` ahead of the epilogue:
NET_SID_CLIENT_Send_0x3D_CreateAccount2, 0051a740) returned that value; where
it is one constant on every path the fixer restores it. A setter whose EAX holds the
same argument at every return (AUTOMAP_SetFade) returns that parameter. Functions that return
different values on different paths are left for the database: which `return`
gets which value is the signature's and the body's to say. The ones an earlier
version of the fixer retyped with one value were set by hand from EAX at each
of the original's returns - early exits returning 0 where the end returns 1
(the SKILLDESC_Format* family, 004e79e0 and on), a callee's result (`return
SKILLDESC_FormatSignedRangeOrValue(...)`, 004e7f40; SMSG_InternalRegister,
00420a70), a flag (`return bRechargedAny;`, ITEMMODE_HandleCursorItem,
0055fe80) or a truth value (`return D2CheckCorpseConsumed(pUnit, 7) != 0;`,
CorpseConsumedCheck, 005c3300). Two stay `void` for now:
INV_UpdateItemUnderCursor (00487000), whose EAX at its returns is no value,
and SKILL_ExecuteItemCastAtCoords (004ca230), which returns what
SKILL_ExecuteItemCast (004ca060) returns - a value that function computes
(`mov eax, edi`) and the database types `void`.

**Arguments a function ignores.** A parameter the body never reads leaves
no trace in the function, only in its callers: GetObjectControlSeed
(00546fa0) is `mov eax, [ecx+0x10f0]; ret`, yet all but one of its callers
load EDX with the room first - its signature is `(pGame, pRoom)`. The tree had
one parameter, and the decompiler dropped the room computed for it (a bare
`UNITS_GetRoom(pUnit);` statement before the call); the tree now passes it.
GFX_LoadUnitModeCof (00470610) is the same: its callers load EDX with 0
(STATES_ResetUnitGfxAndMode with 1), and the tree now passes it. Where
callers set ECX or EDX right before calling a function that does not read
them, the database is missing a parameter; CACHE_FreeMem (005193c0),
STRTABLE_LoadStringTableFile (00525920), ITEM_CltRecalcAllEquippedItems
(004c1bc0) and NET_D2GS_SERVER_Send_0x8F_HEARTBEAT_PING (0053e020) still
lack theirs.

**Functions filed apart from their callers** (`evidence/objects.tsv`, kind
`object`). A function the original passes arguments in EAX/EBX/ESI/EDI was
compiled in one object with every caller; where the tree files a caller in
another TU, the two belong in one file - the generator files by Ghidra
namespace (`fix_refile.py` refiles them, see "Files by object" above).
UI_ShowMonsterNameBar (00452be0) takes its argument in ESI and is called
from Source/D2Client/UI/Draw, whose functions lie inside the range of the
original's `ui.cpp` object (`oracle tulayout`). What objects.tsv still
lists are groups no object can hold, or whose functions' names other files
use.

**Calling conventions** (`evidence/objects.tsv`, kind `convention`). A
function whose original takes `this` in ECX is a member function
(`__thiscall`), whatever the tree declares; VC8 refuses `__thiscall` on a free
function, so the generator has to emit it as a member
(`Fog::_3rdparty::crashy::Constructor`, 00401070, and the rest of that class).
1b24c75 took the convention off functions whose original shows it, and
every caller then cleans up after them or pushes what the original passes in
registers: CheckItemFlag (006280a0), GetSkillId (0064a280),
ITEM_GetItemData_eD2ItemFlag_IsEthereal (0062a8d0), GetMagicSuffix and
ITEM_GetItemData_nItemLevel end in `ret N` (`__stdcall`); SHA1_Init
(00529f50), CheckMonsterType, MONSTERTBLS_GetMonSeqFrameCount,
IsInInventoryScreenRange and INV_IsMouseOverCloseButton take their first
arguments in ECX and EDX (`__fastcall`). The branch restores them, and
`fix_fastcall.py` declares `__fastcall` wherever the original takes the
first arguments in ECX and EDX and pops what `__fastcall` leaves on the
stack (LeaveCriticalSection_Safesock, 0040dde0). Reading a register as an
argument takes care: VC8 reserves a 4-byte local with `push ecx`, and
`cpuid` reads ECX, so neither is evidence of an ECX argument (the classifier's
live-in analysis discounts both; DEBUG_CrashDumpRequestThreadProc,
004027c0, and DetectCpuVendor, 0040e250, take none).

**Parameters typed by what the function reads** (`fix_paramtype.py`).
Ghidra gives a parameter the type of what its function reads of it: a
function testing only the low byte of an argument gets `uint8_t`. The
original's callers show the type it had: for a `__fastcall` argument they
load the whole register, `mov edx, 5` or `mov edx, dword ptr [...]`, where a
byte parameter makes VC8 load DL (QUEST_SetStateAndBroadcast, 00544300,
and its quest callers). The rule: a parameter the original's callers pass
in a full register is an int - the type of a parameter is evidence at its
call sites, not only in its function.

**Local types.** D2Game sends packets from `uint8_t aPacket[N]` locals where
the original has packet structs: VC8 protects byte arrays with a stack cookie
and structs of scalars not, so these functions have a cookie the original
does not (`evidence/flags.md`, "/GS-"). `fix_gsbuffer.py` stands in with
anonymous structs of byte fields; the database's packet types are the real
fix. The generator's padding does the same from inside a type: D2DamageStrc
spells its two bytes of alignment as `uint8_t nReserved66[2]`, and every
function with a local D2DamageStrc gets a cookie the original does not have
(SetUninterruptableState, 005544b0); `fix_gsbuffer.py --members` makes the
member a struct of byte fields. Padding the compiler inserts by itself needs
no member at all. A local typed by what the function reads of it, `uint8_t
byFlags`, is loaded into AL and stored as a byte, where the original
zero-extends the load and stores a dword: it was an int (SMEM_DestroyHeap,
00411f00; `fix_localtype.py`). A packet built on the stack can also come apart:
NET_SID_CLIENT_Send_0x52_AuthAccountCreate (00522130) fills a salt, a
verifier and the user name and then sends `nUsernameLen + 0x41` bytes from
the salt - one packet struct in the original, three arrays in the tree, which
works only while a compiler happens to lay them out back to back. Char
buffers typed as dword arrays (`uint32_t szFormatted[64]`, used through
`(char*)` casts) lose the place `/GS` gives a char array next to the cookie:
SErrReportNamedResourceLeak (0041d010) matches once it is `char[256]`. Byte
buffers typed `void *` - D2DataTableTxt's
pSkillsCode, pSkillDescCode, pMissCode, pItemsCode, pTxtItemTypesUnknown and
pTxtMonTypeUnknown, the shrine tables' aShrinesRng - are indexed with
arithmetic on `void *`, which GCC takes as byte arithmetic and VC8 refuses
(C2036), so every function doing it was quarantined (ITEMS_IsA, 00629bb0,
and more); the tree now casts to `uint8_t *` at those sites, the database
should type the members as byte pointers. The signedness of variables is the
other recurring type difference: `oracle classify --tag signedness` lists
the functions whose comparison is signed where the original's is unsigned, or
the other way round.

## Not upstream

Compiler flags are this oracle's to settle: `flags.tsv`, measured in
`evidence/flags.md`.
