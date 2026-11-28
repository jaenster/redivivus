# Redivivus/Lua

The mod host. Built only with the CMake option `REDIVIVUS_MODS` (`D2_FEATURE_MODS=1`); with the
option off none of this is compiled.

Three layers:

|Layer|Where|What|
|-|-|-|
|Native (C++)|`d2_lua_*.cpp`|Everything that touches engine memory or enforces the sandbox: the state and the per-mod environments, the time budget and its watchdog, the memory limit, entering mod code (the owner a fault is charged to), handles, the module resolver and its grants, storage bytes, the settings file, the message envelope, the mod loader|
|Core (TypeScript, compiled to Lua, embedded)|`core/`|The `redivivus` mod: its public modules (`redivivus:server` and the rest), registries and wrap chains, events, scripts, channels, settings, persisted state, the objects a mod's exports receive. Mods never see the natives the core gets (`core/native/core.d.ts`)|
|Mods|`mods/<name>/`|TypeScript (built by `rdvmod`) or Lua|

## A mod

One program in its namespace `mods/<name>/`: `info.json`, the data stage entries (`settings`,
`data`, `data-updates`, `data-final-fixes`), and the control entry `main`, whose default export is
the mod:

```ts
import { defineMod } from "redivivus:mod";
import { Event, Side, StatId } from "redivivus:enums";

export default defineMod({
    patch(engine) {                                     // once per Lua state, first, with no game
        if (engine.side !== Side.Server) return;
        engine.D2Game.SKILLS.SkillsServer.CalculateManaUse.override((next, game, unit, skill, level) => {
            next(game, unit, skill, level);             // the stock deduction (or the next mod's override)
        });
    },
    server(game) { game.on(Event.Tick, () => {}); },   // once for every game this process hosts
    client(client) { client.on(Event.Draw, () => {}); }, // once for every game this client is in
    menu(menu) {                                        // once, when the main menu first comes up
        const screen = menu.addScreen((s) => <MyScreen close={() => s.close()} />);
        menu.addButton("MY MOD", () => screen.open());
    },
});
```

`defineMod` returns its argument; it is there for the types: every entry point is optional, each
parameter is typed from it, and a key that is no entry point is a compile error (and, from Lua, the
mod fails to load). The same `main` is loaded in the server state and in the client state; each
runs `patch` first, then calls only its own entries (`menu` is the client's too). Everything
registered through `game` or `client` lasts as long as that game. Plain Lua: `main.lua` returns
`{ default = require("redivivus:mod").defineMod({ ... }) }`, or as before a bare table of the entry
functions (named exports in TypeScript keep working the same way).

Modules are `<mod>:<path>`: `redivivus:<entry>` for the API, `other-mod:ids` for a dependency;
relative imports reach the mod's own files. `redivivus` is built into the game (version
`D2LUA_REDIVIVUS_VERSION`, 0.1.0): every mod depends on it (`"redivivus >= 0.1"`), and nothing on
disk may be named after it.

## The modules of redivivus

|Module|Resolves in|Holds|
|-|-|-|
|`redivivus:shared`|every state|`log`, `modName`, `side`, settings (`defineSettings`, `setting`, `listSettings`), channel declarations (`defineChannel`, `t`), the waits scripts use (`wait`, `waitUntil`) and `guard`, `optionalImport`, and the types (`Script`, `Handle`, `EventBase`, `ModMain`, registry types)|
|`redivivus:server`|every state|types: `Game`, the world classes (`Level`, `Room`, `Unit`, `Monster`, `Player`, `AnyUnit`), the monster AI (`AiRegistry`, `AiScript`, `AiWrapper`, `Think`), `ServerEvents`, `ServerRegistries`, `ServerChannel`; points (`subtile`, `tile`, `toTile`, `toSubtile`); re-exports `Event`, `Outcome`, `UnitType`, `MonsterId`, `MonsterAiId`, `StatId`, `StateId`, `AiTargeting`, `PointKind`|
|`redivivus:client`|every state|types: `Client`, `ClientEvents`, `ClientChannel`, `Menu`, `MenuScreen`; re-exports `Event`|
|`redivivus:data`|every state|`data` (`raw`, `extend`; extend only in the data stage)|
|`redivivus:enums`|every state|every enum, canonical: the engine's (from `Source/D2Headers/d2_enums.h`) and the API's|
|`redivivus:lualib`|every state|TypeScriptToLua's runtime, which compiled TypeScript requires|
|`redivivus:mod`|every state|`defineMod` and the mod's types: `ModDefinition`, `Engine` (`ServerEngine`, `ClientEngine`), `EngineFunction`, `Opaque`, `Pointer`|
|`redivivus:mods`|every state|the installed mods as the boot found them: `installedMods()` (state, error, dependencies as met), `readme`, `changelog`, `installedSettings()`; read-only|
|`redivivus:manager`|every state, the built-in mod only|`setEnabled(changes)` (writes `mods/mod-list.json`), `restart()`; no mod can import it|
|`redivivus:test`|server, with the test runner's grant|`world` (the runner's stand-in world), `probe`|
|`redivivus:automation`|client, driver scripts only|nothing yet but `isDriver`|
|`redivivus:ui`|every state|the UI kit (`core/ui`): signals, `h`/`Fragment` for TSX, `Show`/`For`/`Switch`, `mount`, the widget kinds, layout, text, input, `Markdown`, the recorder backend and the two themes, and its enums (`SizeMode`, `Align`, `Direction`, `BoxStyle`, `ButtonVariant`, `FontRole`, `TextColour`, `BlendMode`, `UIEventType`, ...)|

Every module a mod may import resolves in every state; only the grants (`redivivus:test`,
`redivivus:automation`) limit one. `redivivus:ui` too: a mod imports its UI from `main` like any
other module. Only the client state has a renderer and input, so UI built in the server state or
the data stage works exactly the same and is never drawn, as a server registration made on a
client is accepted and never run. Each mod gets its own instance of the kit.

`api/redivivus/*.d.ts` are emitted by the compiler from `core/redivivus/*.ts`; the reference is
`api/docs/reference.md`.

## The main menu and the built-in mod

A mod's `menu` export runs once, in the client state, when the main menu first comes up. Through
the `Menu` it gets, a mod adds main menu buttons (the menu has room for two) and full screens: a
screen is a `redivivus:ui` component that, while open, is the whole menu (it draws every frame and
gets the mouse and keyboard); `close()` or an Escape nothing took returns to the main menu.

The game's own `redivivus` mod is a program like any mod's (`builtin/`, compiled by rdvmod against
the same declarations and embedded in the exe, `generated/d2_lua_builtin.gen.cpp`): its `menu`
export adds MODS and the Mods screen, which reads `redivivus:mods`; only enabling and disabling
mods and restarting go through `redivivus:manager`, which nothing but the built-in mod resolves.
"Apply and restart" writes `mods/mod-list.json`, starts the executable again with the same command
line and quits as the menu's Exit does; the new process waits for the old one to end before its
boot. A mod that is off in the mod list is not loaded; its dependents are disabled with the reason.

In the game (`engine/d2_lua_menu.cpp`) the client state is made at the first main menu; the
backend draws through D2gfx with its own scissor stack (see that file for the drawing rules), the
buttons are the stock short button art appended to the menu's form list, and a screen is one
full-screen control as Credits is.

## Overriding engine functions: `patch(engine)`

Every named function of the compiled game can be overridden, by the names of the source:
`engine.<module>.<namespace>.<function>`, e.g. `engine.D2Common.Skills.GetSkillLevel` or
`engine.D2Game.SKILLS.SkillsServer.CalculateManaUse` (the module is the Source directory, the rest
the C++ namespace; a name two functions share in one namespace gets the address appended,
`Ptr_0060f360`). The registry is generated from the tree's own definitions (`tools/src/overrides.ts`
into `generated/override/`): 14542 functions, 14211 of them overridable. Refused, and listed with the
reason (`engine.X.Y.refused`, and `override` raises it): 280 member functions, 34 that take `...`,
17 that take or return a record by value. An override that the build cannot honour (an entry
without its patch area) is refused at run time the same way.

```ts
engine.D2Common.Skills.SKILLS_GetManaCostMultiplier.override((next, skill) => next(skill) / 2);
```

- `override(fn)` works only inside `patch`, which runs once per Lua state (the client's, and each
  game's server state, made at its first frame) before `menu`, `server` and `client`.
- `fn(next, ...args)` returns the result; `next(...args)` runs what is below. Several mods on one
  function chain in load order: a mod's override wraps those of the mods it depends on, the
  original at the bottom. An override may call `next` with other arguments, twice, or not at all.
- The engine of a state holds that side's functions only: a server state the server's (`D2Game`)
  and the shared ones (`D2Common`, `Fog`, `Storm`, `D2Lang`, `D2Net`, `D2CMP`, `D2Hell`), the
  client state the client's and the shared ones. Narrow it on `engine.side`.
- Which state runs an override: the thread's. A thread processing a game (`ServerGameLoop`'s frame
  and a client's in-game command) uses that game's server state; the client's thread uses the
  client state; any other thread, or a state without an override of the function, runs the
  original. In single player the server runs on the client's thread: there a shared function uses
  the server state inside the game's frame and the client state outside it.
- An override that raises, returns something of the wrong type, yields, or runs out of time or
  memory faults its mod (logged): all of the mod's overrides are removed and the call goes on with
  what is below it. The time budget and the memory cap of every entry apply.
- Overrides run compiled: LuaJIT records the override's body as traces (the host test checks it);
  `next` is a C function the trace stitches around.

### The two tiers

The typed tier, every mod's: numbers are numbers (an integer argument or result is checked against
its C type's range; -1 for an unsigned one wraps as in C), `bool` is a boolean, a unit is a checked
handle of `redivivus:server` in a server state (opaque in the client's), and every other pointer
is an opaque value a mod can only hand on. A pointer handed to `next` or returned must be one the
call was given or one `next` returned, of the same C type; `nil` only where the call was given a
null one. So a typed mod never reads, writes or makes a pointer. Numbers it changes are the
engine's to check, as a data mod's are.

The raw tier, `engine.unsafe`: the same functions, but pointers arrive as LuaJIT FFI cdata of their
C type and `engine.unsafe.ffi` is LuaJIT's ffi (declare a record with `ffi.cdef` to read its
fields; an undeclared one is an incomplete type). `next` takes cdata back, unchecked. It exists only
for mods the player listed in `mods/unsafe-mods.json` (`{ "mods": ["name"] }`, a file nothing but
the player writes); for every other mod `engine.unsafe` is nil.

### How it works, and its limits

A mods build compiles every unit with `-fpatchable-function-entry=5,0` and functions on 16 bytes:
five nops at each entry. Overriding a function replaces them with a jump to its generated thunk
(the function's own calling convention: `__fastcall`, `__stdcall`, `__thiscall`, `__cdecl`), in one
locked 8-byte store while every other thread is suspended and none stands inside the five bytes;
removing the last override stores the nops back. The thunk calls the original five bytes in, so
nothing is relocated, and a function nobody overrides only runs its nops. Every patch is journaled
and put back when its state goes (a game ends, a reload) and, for whatever is left, before the
Mods screen restarts the game and at exit.

- Inlining: the build is unoptimised, and no function of the registry has an inlined copy in it
  (the DWARF of the mods build: 87414 inlined copies, all of 2177 library and platform helpers such
  as `std::forward` and `D2_SeedRoll`). A function marked always_inline would be refused. What the
  original compiler inlined into the decompiled bodies (a helper whose body the source repeats at
  its call sites) is not a call in the tree either: overriding the helper does not reach those
  copies; override the caller.
- Static functions are reachable: each table is compiled into its own Source unit (the
  `.ov.cpp` `#include`s the unit), which takes the unit's place in a mods build.
- Cost: a function nobody overrides runs five nops; a patched one called on a thread whose state
  does not override it costs the thunk's check; an overridden call one entry into Lua (see the
  host test's `--bench-override`).
- The registry is generated and committed; after a regen of `Source/` run `node tools/src/gen.ts`
  (the mods build fails at compile time when a signature changed).

## The world and monster AI (server)

In a game, the `Game` a mod's `server` export gets also reaches the world: `game.players()`,
`game.level(id)`, `game.findUnit(type, id)`, `game.unitsNear(point, radius)`,
`game.spawnMonster(id, at)`, `game.message(text)`. Levels, rooms and units are handles: the engine
object is looked up on every use, `valid` is false once it is gone (a monster also while dead or
while its room is unloaded; it comes back with its room under the same id), and any other member
of an invalid handle raises. Queries return iterators over a snapshot that skip units gone since.
Points are typed (`subtile(x, y)`, `tile(x, y)`); units stand on subtiles.

`game.registry.ai` takes over the monsters of an AI id (`MonsterAiId`): `register(id, script)` runs a
`function*` per monster, where a bare `yield` waits one think and `yield* monster.walkTo(p)` (idle,
runTo, walkToward, runToward, attack, castSkill, wander, flee) runs an action until the engine ends
it and gives its `Outcome`; `wrap(id, (next, monster, think) => ...)` wraps each think, `next()`
running one think of what is below. A script ends when its monster dies or is gone and starts over
when the engine attaches the AI again (spawned, room reloaded, revived, a curse wears off).
`yield* guard(() => target.valid, chase(monster, target))` runs a part of a script while a check
holds: the check runs before each step of the body, and once it fails the body is dropped where it
waits and the outcome is `Outcome.Interrupted` (an action it started runs on as the engine runs it).
The build hands the body over unstarted; it runs on a coroutine of its own that counts as the
script's, so its waits and actions are the script's.

## Files

|Path|What|
|-|-|
|`d2_lua_host.*`, `d2_lua_internal.h`|The sandboxed state: environments, the budget per entry, the memory limit, the owner stack, CallAs/ResumeAs (how the core enters mod code)|
|`d2_lua_core.*`|The core's loader: its environment, the natives table, per-mod instances of the public modules|
|`d2_lua_runtime.*`, `d2_lua_rt.h`|A state per side running a mod list: the resolver and namespaces, entries, games, storage save/restore, reload, source-mapped errors|
|`d2_lua_natives.cpp`|What generated glue builds on: argument checks, the tables the declarations fill|
|`d2_lua_handle.cpp`|Handle classes: engine objects named by (class, id), resolved on every access; the core can fill a class's member tables|
|`d2_lua_world.*`|The game world for a server state: the `D2LuaWorld` interface, the natives the world classes are built on (handle classes Player, Monster, Unit, Room, Level; scoped unit queries), a think's bookkeeping|
|`d2_lua_hooks.h`|The macros Source/ calls (AI lookup, tick, game end, unit freed, monster died, the game scope of a server frame and command, the main menu, palette loads, the font accessors); nothing with the option off|
|`d2_lua_override.*`, `d2_lua_override_gen.h`|Function overrides: the registry, the patcher, the dispatch a thunk runs, the templates the generated tables are built from|
|`generated/override/`|The override tables, one per Source unit (`<unit>.ov.cpp`, which `#include`s the unit), and their index|
|`d2_lua_ui.*`|The drawing backend interface (`D2LuaUi`) and its natives, the catalogue of installed mods, the mod list file|
|`builtin/`|The built-in redivivus mod's program: its `menu` export, the Mods screen (`builtin/mods`) and its game data|
|`engine/`|The game's build only: the world over a real `SGAMEDATA`, a server state per game, the AI overlay and the hooks' bodies (`d2_lua_engine.cpp`); the client state, the D2gfx backend, the main menu buttons and screens (`d2_lua_menu.cpp`)|
|`d2_lua_storage.cpp`|storage as bytes and back|
|`d2_lua_messages.cpp`|The message envelope, the transport, the in-process loopback|
|`d2_lua_settings.cpp`|The settings file, declarations checked and their values settled|
|`d2_lua_pattern.cpp`|`string.find/match/gmatch/gsub` for mods with a step counter that honours the deadline|
|`d2_lua_mods.*`, `d2_lua_json.*`|Discovery, `info.json`, dependencies (Factorio syntax), load order|
|`d2_lua_boot.*`|What the game runs at boot: discovery, load order, the settings stage, all logged|
|`core/`|The core: `redivivus/` (public modules), `internal/` (registries, events, scheduler, channels, ...), `native/` (the natives the core gets, and the test runner's world), `ui/` (the UI kit that `redivivus:ui` serves)|
|`ui/`|The UI kit's own package: the Mods screen and widget gallery, the Node pixel renderer (`ui-preview`), the kit's tests in Node and LuaJIT, golden images (its README)|
|`generated/`|Generated and committed: the glue, the embedded core (`d2_lua_core.gen.cpp`) and its runtime library's licence|
|`api/`|The `@redivivus/api` package: emitted declarations, the sandbox's Lua library as TypeScript, LuaLS files, the reference, the editor plugin|
|`tools/`|The generator (`src/gen.ts`), `rdvmod` (`src/cli.ts`), the TypeScriptToLua plugin, the MPQ library (`src/mpq`, copied from the mod portal; see its PROVENANCE), the selftest|
|`examples/`|`cheap-mana` (a function override), `first-mod` and `second-mod` (TypeScript), `lua-mod` (plain Lua), `ui-mod` (a TSX panel), `charger` (a monster AI), `fallen-shaman` and `mephisto` (stock AIs ported; mephisto twice, with step-level tests), `type-tests`|
|`test/`|The host test, its fixture mods, the adversarial escape suite, a stand-in engine for the world classes and the AI hook (`d2_lua_test_engine.cpp`)|

## Commands

```
cd Redivivus/Lua/tools && npx pnpm@10 install          # packages, pinned in package.json / pnpm-lock.yaml
node Redivivus/Lua/tools/src/gen.ts [--check]          # regenerate (or check) every generated output
node Redivivus/Lua/tools/src/cli.ts build --out DIR MOD...
node Redivivus/Lua/tools/src/cli.ts check MOD...
node Redivivus/Lua/tools/src/cli.ts create DIR --name NAME [--ui]       # --ui: with a TSX panel
node Redivivus/Lua/tools/src/cli.ts pack [--out DIR] BUILT_MOD_DIR     # -> <name>_<version>.mpq
node Redivivus/Lua/tools/src/cli.ts pack --check FILE.mpq
node Redivivus/Lua/tools/src/selftest.ts --host-test BUILD/d2_lua_host_test[.exe]
BUILD/d2_lua_host_test[.exe] --bench-override .         # the cost of an overridden call
cmake -S Redivivus/Lua/test -B build-luatest [-DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake]
```

Node 22.18 or newer runs the tools' TypeScript directly; nothing is compiled first.

An editor gets the build's diagnostics through the plugin: `rdvmod tsconfig MOD` writes a
tsconfig naming `@redivivus/api/ls-plugin`, which tsserver loads from the mod project's
`node_modules` (`--allowLocalPluginLoads`, or the workspace TypeScript installed beside it).
