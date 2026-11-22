<!--
Before anything else, check which kind of change this is - it decides whether a pull request is
the right shape at all.

  A function body under Source/, 3rdparty/Storm/ or 3rdparty/crashy/
      -> welcome when `oracle diff <name>` shows MATCH (or a clear improvement) and no other
         function in the unit loses MATCH. One function per commit. Faithful source only: no
         noinline/forceinline, volatile, asm, pragma or register; no raw addresses, no dead
         stores, no temporaries that only steer registers (an evaluation-order temporary gets
         a `// steer-ok: <reason>` comment). A maintainer re-verifies and pins each body.

  Hand-written code, PRs welcome  ->  project.json (the module table), Redivivus/,
                                      3rdparty/libbacktrace/, 3rdparty/implibs/, cmake/, docs/,
                                      asm_matching/, tools/, agent/, CMakeLists.txt, .github/,
                                      README.md

  A wrong name, type, field, prototype or convention
      -> open a "Correction: name, type or prototype" issue, or send it as a proposal if you
         have agent access to the databases. It is applied in Ghidra and in the tree together.

  A function Ghidra has not found, or wrong function boundaries
      -> open a "Structural: function discovery/boundary" issue. It forces a full rebuild and
         goes through a human; it is never a proposal and never a PR.

  The C++ is wrong in a way no Ghidra change fixes (a bad transform, a lost value)
      -> for one function, a hand-written body as above; for a pattern across many, an issue
         here with the addresses - the fix belongs in github.com/jaenster/ghidra-reconstruct.

  A crash while playing
      -> open an issue and attach the d2crash-<date>-<time>.txt report. Do not patch around it;
         fix the wrong type or lost value it comes from.

See CONTRIBUTING.md for the long version. Delete this comment block and fill in below.
-->

## What this changes

<!-- One or two sentences. -->

## Work items

<!--
If this pull request works items from https://redivivus.app/work, name each one on a line of its
own, exactly like this (outside this comment):

  Work-Item: static-0045a2b0

The pr_template tool on the Redivivus MCP server writes this section for you, with each
function's status before and after. A Work-Item line hands the item in for you as the author of
this pull request, even without submit_work, and /work shows the pull request against it. An item
someone else holds is flagged instead, not linked: claim it (claim_work) before you work it.
Leave the section empty when the change is not a work item.
-->

## Which kind of change is it?

- [ ] A function body that now matches the original (`oracle diff <name>` output below)
- [ ] Hand-written code or docs (`project.json`, `Redivivus/`, `3rdparty/libbacktrace/`, `3rdparty/implibs/`, `cmake/`, `docs/`, `asm_matching/`, `tools/`, `agent/`, `CMakeLists.txt`, `.github/`, `README.md`)
- [ ] A deliberate source deviation from the original (for example a string that must read "Redivivus" rather than "Diablo") - see the note below
- [ ] Something else, explained below

## Checks

- [ ] A changed function body is MATCH (or a stated improvement) in `oracle diff`, no other function in its unit lost MATCH, and it changes no declaration (those go through Ghidra).
- [ ] `cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake && cmake --build build` still succeeds.
- [ ] If this touches `asm_matching/`, I have said what it does to the match figures.

## Anything a reviewer should know

<!--
If this IS a deliberate source deviation from the original: say so plainly here. These are
accepted and pinned like any hand-written body, but kept apart from the byte-match figures.
-->
