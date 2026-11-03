# Compiler flags: what was measured

Each flag decision here is an A/B: the whole tree built both ways, both audited
with the same function map, compared per module (`scripts/abcompare.py`).
A flag is adopted only where it wins. Settled questions are marked DONE so
nobody re-runs them; numbers are as measured at the time and are not updated.

## /Oy- (keep the frame pointer) - DONE, adopted for every module

`/O2` implies `/Oy` (omit frame pointers). The original sets up EBP in most
of its functions, and our `/O2` build in almost none. Measured on tree
e52f3b7 with the map of that day:

```
module                            n  MATCH a  MATCH b  only a  only b    sim a    sim b
Bnclient                        223       53       68       0      15    0.636    0.713
D2CMP                           178       18       37       0      19    0.512    0.637
D2Client                       2932      534      647       1     114    0.581    0.643
D2Common                       1834      103      435       0     332    0.487    0.665
D2DDraw                          65       20       21       0       1    0.600    0.665
D2Direct3D                       75       12       13       0       1    0.557    0.613
D2Game                         2920      332      517       0     185    0.559    0.658
D2Gdi                            46       22       25       0       3    0.744    0.787
D2Glide                          54       16       18       0       2    0.607    0.663
D2Lang                           58        2        6       0       4    0.428    0.541
D2Launch                        265       90      101       0      11    0.750    0.794
D2MCPClient                      48       12       20       0       8    0.525    0.687
D2Multi                         190       41       47       0       6    0.640    0.697
D2Net                            41        4       19       0      15    0.619    0.820
D2Sound                          80       30       33       0       3    0.719    0.764
D2Win                           311       40       77       0      37    0.580    0.682
D2gfx                            86       21       51       0      30    0.640    0.843
Fog                             414       42       88       0      46    0.539    0.668
Storm                           532       59       90       0      31    0.451    0.533
total                         10352     1451     2313       1     863
```

a = `/O2 /EHsc /GS /MT ...`, b = the same plus `/Oy-`. `/Oy-` wins in every
module, by MATCH count and by mean similarity, and loses one function in the
whole tree. It is in the defaults (`scripts/oracle_config.py`), not in
`flags.tsv`. VC8 still omits the frame where a function needs none, which is
why the original's frameless functions match under `/Oy-` too. (D2's own crash
handler walks EBP chains, a plausible reason for the setting.)

## Compiler build: SP1 (14.00.50727.762) vs RTM (.42) - DONE, SP1

The linked CRT already says SP1 ([toolchain.md](toolchain.md), section 2).
The game code agrees. Tree e52f3b7, default flags with `/Oy-`, the same map;
a = SP1, b = RTM (`ORACLE_VC=rtm`):

```
module                            n  MATCH a  MATCH b  only a  only b    sim a    sim b
Bnclient                        211       59       59       0       0    0.716    0.716
D2CMP                           160       39       39       0       0    0.676    0.676
D2Client                       2859      653      621      33       1    0.656    0.651
D2Common                       1817      484      483       1       0    0.678    0.678
D2DDraw                          38       19       19       0       0    0.798    0.795
D2Direct3D                       88       14       14       0       0    0.612    0.612
D2Game                         2994      568      562       6       0    0.658    0.657
D2Gdi                            52       27       27       0       0    0.778    0.778
D2Glide                          56       18       18       0       0    0.665    0.665
D2Lang                           58        7        7       0       0    0.545    0.545
D2Launch                        269       96       96       0       0    0.760    0.760
D2MCPClient                      44       19       19       0       0    0.700    0.704
D2Multi                         176       47       46       1       0    0.700    0.698
D2Net                            38       16       16       0       0    0.806    0.806
D2Sound                          73       36       36       0       0    0.816    0.816
D2Win                           305       74       74       0       0    0.673    0.673
D2gfx                            87       56       56       0       0    0.876    0.875
Fog                             406       98       98       0       0    0.681    0.681
Storm                           446       84       84       0       0    0.541    0.541
total                         10177     2414     2374      41       1
```

SP1 matches 41 functions that RTM does not; RTM matches one that SP1 does
not. Two independent measurements, one answer.

## /Ob1 (inline only what is marked inline) - DONE, adopted for every module

VC6's `/O2` implied `/Ob1`; VC8's implies `/Ob2` (inline any suitable
function). Projects carried over from VC6 keep the former only if it was set
explicitly. Tree e52f3b7, `/O2 /Oy-` defaults, the same map; a = `/Ob2`
(implied), b = `/Ob1`:

```
module                            n  MATCH a  MATCH b  only a  only b    sim a    sim b
Bnclient                        211       59       61       0       2    0.716    0.722
D2CMP                           160       39       39       0       0    0.676    0.677
D2Client                       2859      653      656       0       3    0.656    0.658
D2Common                       1817      484      487       0       3    0.678    0.681
D2DDraw                          38       19       19       0       0    0.798    0.802
D2Direct3D                       88       14       15       0       1    0.612    0.627
D2Game                         2994      568      569       0       1    0.658    0.660
D2Gdi                            52       27       27       0       0    0.778    0.778
D2Glide                          56       18       19       0       1    0.665    0.689
D2Lang                           58        7        7       0       0    0.545    0.542
D2Launch                        269       96       97       0       1    0.760    0.762
D2MCPClient                      44       19       19       0       0    0.700    0.726
D2Multi                         176       47       47       0       0    0.700    0.701
D2Net                            38       16       16       0       0    0.806    0.812
D2Sound                          73       36       37       0       1    0.816    0.819
D2Win                           305       74       74       0       0    0.673    0.673
D2gfx                            87       56       58       0       2    0.876    0.880
Fog                             406       98       98       0       0    0.681    0.683
Storm                           446       84       85       0       1    0.541    0.563
total                         10177     2414     2430       0      16
```

Sixteen functions match only with `/Ob1`, none only without it, and mean
similarity rises in 17 of 19 modules. The mechanism is visible in each gained
function: under `/Ob2` VC8 expands a small callee inline where the original
calls it (e.g. `UI_LoadAllPanelAssets` at 00453d90: two global stores
instead of `call 004b8720`).

## /GS- (no stack cookies) - DONE, adopted for the client-side modules

`/GS` is VC8's default and the original has security cookies (`mov eax,
[___security_cookie]; xor eax, ebp` in the prologue, `__security_check_cookie`
before each return) - but not everywhere. Counted over the mapped functions
on tree 288d593 (a cookie in the original's prologue / in our object's):

```
module        cookie in both   only ours   only original
Bnclient                  61           2               3
D2CMP                      6           2               0
D2Common                  27          15              12
D2Game                    95         187               5
D2Lang                    12           0               0
D2Launch                   1          51               0
D2Net                      3           0               0
Fog                       84           3              22
Storm                     35           5              12
D2Client                   0         370               0
D2DDraw                    0          12               0
D2Direct3D                 0          10               0
D2Gdi                      0           2               0
D2Glide                    0           6               0
D2MCPClient                0          19               0
D2Multi                    0          69               0
D2Sound                    0           2               0
D2Win                      0          53               0
D2gfx                      0           2               0
```

The client-side modules - before 1.14 each its own DLL project - have no
cookie in any function, while their char buffers (`char szPath[260]`) would
get one under `/GS`. The A/B, the whole tree with `/GS-` against the baseline
(a = `/GS`, b = `/GS-`), same map:

```
module                            n  MATCH a  MATCH b  only a  only b    sim a    sim b
Bnclient                        305      102      102       0       0    0.740    0.718
D2CMP                           183       47       47       0       0    0.676    0.674
D2Client                       3139      844      868       0      24    0.694    0.707
D2Common                       2032      562      563       0       1    0.696    0.695
D2DDraw                          68       25       25       0       0    0.708    0.722
D2Direct3D                      137       24       24       0       0    0.621    0.627
D2Game                         3693      849      868       0      19    0.706    0.710
D2Gdi                            66       33       34       0       1    0.769    0.775
D2Glide                         108       32       32       0       0    0.656    0.666
D2Lang                           62        8        8       0       0    0.567    0.559
D2Launch                        268      115      118       0       3    0.818    0.839
D2MCPClient                      77       39       40       0       1    0.766    0.812
D2Multi                         205       60       70       1      11    0.717    0.766
D2Net                            56       32       32       0       0    0.855    0.847
D2Sound                          90       43       43       0       0    0.813    0.815
D2Win                           306       77       80       0       3    0.704    0.722
D2gfx                            98       61       61       0       0    0.854    0.856
Fog                             485      120      118       2       0    0.666    0.645
Storm                           697      158      158       0       0    0.607    0.601
total                         12075     3231     3291       3      63
```

`/GS-` wins, losing nothing, in D2Client, D2Launch, D2MCPClient, D2Multi,
D2Win and D2Gdi, and raises similarity without a MATCH change in D2DDraw,
D2Direct3D, D2Glide, D2Sound and D2gfx, whose originals have no cookie
either: adopted for all eleven (`flags.tsv`). The one function D2Multi loses,
fAPPMODE_multiplayer_ReturnAppMode_Caller (00442150), calls
CHATDLG_RunChatLobbyScreen, which a static promotion had made static; without
a cookie VC8 gives it a register convention of its own (EAX), where the
original passes ECX - the promotion is undone, not the flag. The single
cookie the original has in these modules, D2INIT_CheckCommandLineOrLoadItFromRegistry
(00405f00), lies in Fog's address range: Fog code filed under
D2Launch/Engine/, which keeps `/GS`.

D2Game gains under `/GS-` too, but its original has cookies (95 functions in
the census): it was built with `/GS`. Its functions with a cookie only on our side keep
packets in `uint8_t aPacket[N]` locals where the original had packet structs
(VC8 protects byte arrays, not structs of scalars) - a type in the database,
not a flag. Fog, Storm, Bnclient, D2Lang, D2Net and D2CMP get worse without
`/GS`.

## /O1 (minimise size) - DONE, rejected everywhere

Before 1.14 a module could have been built for size. The whole tree with
`/O1` in place of `/O2` (a = `/O2`, b = `/O1`), same map, on tree d618181
with the next commit applied:

```
module                            n  MATCH a  MATCH b  only a  only b    sim a    sim b
Bnclient                        320      107       43      64       0    0.745    0.552
D2CMP                           186       66       35      31       0    0.707    0.522
D2Client                       3203      971      417     554       0    0.724    0.538
D2Common                       2065      719      284     440       5    0.723    0.544
D2DDraw                          69       26       19       7       0    0.742    0.553
D2Direct3D                      138       31       11      20       0    0.666    0.481
D2Game                         3737      978      279     699       0    0.729    0.511
D2Gdi                            66       36       24      12       0    0.792    0.635
D2Glide                         118       35       15      20       0    0.684    0.506
D2Lang                           66        8        1       7       0    0.568    0.383
D2Launch                        271      127       19     108       0    0.847    0.582
D2MCPClient                      81       44       27      17       0    0.819    0.697
D2Multi                         216       78        9      69       0    0.786    0.558
D2Net                            56       33        5      28       0    0.856    0.577
D2Sound                          92       46       28      18       0    0.823    0.669
D2Win                           311      113       60      53       0    0.752    0.589
D2gfx                            98       69       20      49       0    0.879    0.633
Fog                             490      133       51      83       1    0.675    0.490
Storm                           711      171       70     103       2    0.623    0.454
total                         12294     3791     1417    2382       8
```

Every module loses most of its matches: `/O2` stays the default. The few
functions that match only under `/O1` are too few to say more than that some
object may have been built differently; they are not adopted.


## /O1 for D2Hell - DONE, adopted for D2Hell

The whole-tree `/O1` run above rejects it per module, but D2Hell was not a module
of its own there. In 1.14d the D2Hell library is one tightly packed block,
0x516d8b-0x517e2f (4260 bytes, about 31 functions):

- every function start in the block is unaligned with no padding between functions;
  every `/O2` module aligns its functions to 16 bytes (outside the CRT only this block
  starts functions unaligned);
- `push imm; pop reg` occurs 7 times in the block's 1380 instructions and nowhere else
  in the game code (the CRT, built for size, has 135).

A/B on the D2Hell TU and on the block's functions filed elsewhere, `/O2` against
`/O1 /Oy- /Ob1`, each TU built without the precompiled header: 12 functions newly
exact (729 bytes; in D2Hell GetFileSize, RegisterErrorHandler,
ShowFormattedErrorWithArgs), one worse (TSList_UnlinkAll). `/Os` and `/O1 /Oy-` give
the same code, so the two are not told apart. RTM and SP1 give identical code at `/O1`.

`/Y-` is required: under the precompiled header `/O1` is silently dropped (C4653).
Only the functions filed under `D2Hell/` get the row; the rest of the block is filed in
Fog/Archive, Storm/Source/SSystem, Bnclient, D2Common/DataTbls and D2Client and gains
only once its namespace moves to D2Hell. Pointed out by Mir Drualga (D2Hell was built
`/O1` in earlier versions).

## Symbol phase (the declarations ahead of a function) - DONE, not a flag

Not a flag but the compile context, measured the same way. VC8 breaks some
ties in scheduling and register allocation by the symbol numbers the front
end hands out in declaration order, modulo 64: a function's code can change
when nothing but the number of declarations ahead of a symbol it refers to
does. D2COMP_DestroyCompositeUnit (005041b0) matched on tree ece1012 and
stopped matching on 674b9c2, its TU untouched: that commit removed
declarations from headers the precompiled header includes. On the bench
(`snippet.py`), `extern int` declarations inserted ahead of its TU's code
flip it back and forth - none to three of them differ, four to 52 match,
period 64 (68 and 132 match, 64 and 128 differ) - and so do function
declarations and a struct, while macros, comments and declarations after the
function do not. The symbol that matters is the `""` literal its assertion
pushes: declarations inserted before that literal's first use (in
D2COMP_AssertNotNull) flip it, declarations after it do not, and creating the
literal first makes the inserted declarations inert. It matched again, its
TU still untouched, once `fix_static --demote` put five functions'
declarations back into headers the precompiled header includes.

The whole tree, every symbol shifted by k declarations force-included ahead
of everything (tree 674b9c2, the map of that day; a = baseline, b = shifted):

```
k     MATCH a   MATCH b   only a   only b
16       4153      4154        0        1
32       4153      4154        0        1
64       4153      4153        0        0
```

At k = 64, 159 of the tree's 14266 functions still compile differently
(`objdiff.py`; the residue is not the only input for a few), all of them
differing from the original either way; at 16 and 32 the one function
gained is D2COMP_DestroyCompositeUnit. The effect is
real and rare: it decides about one function's match, and it is not
adopted - a phase tuned per TU would fit a free parameter per object for
almost nothing.

What it does settle:

* **The precompiled header is not codegen-neutral in the strict sense.** It
  numbers the symbols differently: without it, D2COMP_DestroyCompositeUnit
  matches, and its matching and differing residues are exactly those under
  the PCH shifted by 25. A PCH build is one phase among 64, as good as any;
  the 19 objects build.py's docstring cites as identical with and without it
  held no function this sensitive.
* **A header edit can flip an untouched function.** A declaration added to or
  removed from a header in the precompiled header shifts the phase of every
  TU. Before calling such a function's loss a regression of the source,
  shift its phase on the bench (a line of `extern int` declarations ahead of
  its TU's code, a few counts between 0 and 63): if one count brings the
  match back, the source is right and the edit only moved the phase.
