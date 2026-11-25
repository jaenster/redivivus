# Redivivus Lua API reference

Generated from `Redivivus/Lua/api/*.d.ts` by `Redivivus/Lua/tools/src/gen.ts`. Do not edit.
The declarations are the spec: TypeScript mods use them directly (`@redivivus/api/<side>`),
plain Lua mods get the same as LuaLS annotations in `api/luals/`.

Calls on globals are dot calls (`script.on_event(...)`); methods of handle classes are colon
calls (`unit:damage(5)`). A handle is valid while its object exists; `.valid` says so, and
any other use of an invalid handle raises an error.

## data

### `mod_settings` (LuaModSettings)

Settings of every mod.

- `declare(name: string, declaration: SettingDeclaration): void;`
  Declare a setting of this mod, once per state.
- `get(name: string): boolean | number | string;`
  This mod's value of a setting it declared. A setting is read only on its scope's side.
- `list(): SettingInfo[];`
  Every setting declared in this state, all mods, in declaration order.

### `declare function log(this: void, ...values: unknown[]): void;`

Write a line to the game's log, prefixed with the mod's name. Values are converted with
tostring and separated by spaces.

### `data: LuaData`

The data stage's shared table. Every mod sees the same one.

## server

### `mod_settings` (LuaModSettings)

Settings of every mod.

- `declare(name: string, declaration: SettingDeclaration): void;`
  Declare a setting of this mod, once per state.
- `get(name: string): boolean | number | string;`
  This mod's value of a setting it declared. A setting is read only on its scope's side.
- `list(): SettingInfo[];`
  Every setting declared in this state, all mods, in declaration order.

### `declare function log(this: void, ...values: unknown[]): void;`

Write a line to the game's log, prefixed with the mod's name. Values are converted with
tostring and separated by spaces.

### `remote` (LuaRemote)

Late-bound calls between mods.

- `add_interface(name: string, functions: { [name: string]: (this: void, ...args: any[]) => any }): void;`
  Publish an interface: a table of functions other mods call through remote.call.
- `remove_interface(name: string): boolean;`
  Remove an interface. True when it existed.
- `call<I extends keyof RemoteInterfaces & string, F extends keyof RemoteInterfaces[I] & string>( iface: I, fn: F, ...args: RegistryArgs<RemoteInterfaces[I][F]>): RegistryResult<RemoteInterfaces[I][F]>;`
  Call a function of an interface. It runs as the mod that added the interface; its errors
  reach the caller.
- `readonly interfaces: LuaTable<string, LuaTable<string, true>>;`
  Every interface and its function names: `{ [iface]: { [fn]: true } }`.

### `script` (LuaServerScript)

This mod's handle on the control stage: events, lifecycle, scripts.

- `readonly mod_name: string;`
  The name of the mod this script object belongs to.
- `readonly tick: integer;`
  The current tick.
- `on_event<K extends keyof ServerEvents>(event: K, handler: ((this: void, e: ServerEvents[K]) => void) | undefined): void;`
  Subscribe to an event; undefined unsubscribes. One handler per event per mod.
- `on_init(handler: ((this: void) => void) | undefined): void;`
  Run once when a new game starts, after every mod's control file. storage is fresh.
- `on_load(handler: ((this: void) => void) | undefined): void;`
  Run when a game is loaded or the control stage reloads, after storage is restored.
- `start(body: (this: void) => Script<void>, owner?: Handle): integer;`
  Start a script: the host runs body in a coroutine of its own from the next tick on,
  resuming it each time what it waits for has come. A script ends when body returns or
  raises, when it is cancelled, or when the object of its owner handle stops existing.
  Scripts do not survive a reload. Every tick resumes the scripts that are due in the order
  they were started. The result is the script's id.
- `cancel(id: integer): boolean;`
  Stop a script this mod started. It is not resumed again (a script that cancels itself
  stops at its next wait). True when it was still running.
- `running(id: integer): boolean;`
  Whether a script this mod started is still running.
- `readonly side: Side;`
  Which side this state is.
- `send(payload: string, peer?: integer): void;`
  Send bytes to this mod's other half: to the client with this peer id, or to every client (0, the default). At most 4096 bytes; the kit's channels build typed calls on this.
- `on_message(handler: ((this: void, payload: string, peer: integer) => void) | undefined): void;`
  Receive what this mod's other half sends: the payload and the peer it came from (the client's id). Each delivery is a guarded entry with its own time budget; nothing in the payload is trusted. undefined stops receiving.
- `wait(ticks?: integer): Script<integer>;`
  Wait ticks (default 1) inside a script. The outcome is the tick it resumes on.
- `wait_event<K extends keyof ServerEvents>(event: K): Script<ServerEvents[K]>;`
  Wait for an event inside a script. The outcome is the event's payload.

### `storage: { [key: string]: any }`

The mod's persistent state: plain data and handles, saved with the game (see the storage
rules in the reference).

### `registries: ServerRegistries`

The behaviour registries.

### Events (ServerEvents)

- `Event.Tick` (`"on_tick"`): `EventBase` - Every game tick, before the scripts that are due resume.

### Registry `registries.example` (keyed, ids in ExampleRegistry)

- `base:greet`: `(this: void, name: string) => string` - Greets: "hello, <name>".

### Registry `registries.example_slot` (single slot, ExampleSlot)

- the slot: `(this: void, value: number) => number` - The example single-slot registry: doubles a number.

## client

### `mod_settings` (LuaModSettings)

Settings of every mod.

- `declare(name: string, declaration: SettingDeclaration): void;`
  Declare a setting of this mod, once per state.
- `get(name: string): boolean | number | string;`
  This mod's value of a setting it declared. A setting is read only on its scope's side.
- `list(): SettingInfo[];`
  Every setting declared in this state, all mods, in declaration order.

### `declare function log(this: void, ...values: unknown[]): void;`

Write a line to the game's log, prefixed with the mod's name. Values are converted with
tostring and separated by spaces.

### `remote` (LuaRemote)

Late-bound calls between mods.

- `add_interface(name: string, functions: { [name: string]: (this: void, ...args: any[]) => any }): void;`
  Publish an interface: a table of functions other mods call through remote.call.
- `remove_interface(name: string): boolean;`
  Remove an interface. True when it existed.
- `call<I extends keyof RemoteInterfaces & string, F extends keyof RemoteInterfaces[I] & string>( iface: I, fn: F, ...args: RegistryArgs<RemoteInterfaces[I][F]>): RegistryResult<RemoteInterfaces[I][F]>;`
  Call a function of an interface. It runs as the mod that added the interface; its errors
  reach the caller.
- `readonly interfaces: LuaTable<string, LuaTable<string, true>>;`
  Every interface and its function names: `{ [iface]: { [fn]: true } }`.

### `script` (LuaClientScript)

This mod's handle on the control stage: events, lifecycle, scripts.

- `readonly mod_name: string;`
  The name of the mod this script object belongs to.
- `on_event<K extends keyof ClientEvents>(event: K, handler: ((this: void, e: ClientEvents[K]) => void) | undefined): void;`
  Subscribe to an event; undefined unsubscribes.
- `on_init(handler: ((this: void) => void) | undefined): void;`
  Run once when the client stage starts.
- `on_load(handler: ((this: void) => void) | undefined): void;`
  Run when the client stage reloads.
- `readonly side: Side;`
  Which side this state is.
- `send(payload: string): void;`
  Send bytes to this mod's other half: to the server. At most 4096 bytes; the kit's channels build typed calls on this.
- `on_message(handler: ((this: void, payload: string, peer: integer) => void) | undefined): void;`
  Receive what this mod's other half sends: the payload and the peer it came from (0, the server). Each delivery is a guarded entry with its own time budget; nothing in the payload is trusted. undefined stops receiving.

### Events (ClientEvents)

- `Event.Tick` (`"on_tick"`): `EventBase` - Every client frame tick.
- `Event.Draw` (`"on_draw"`): `EventBase` - When the UI is drawn.

## Enums

Declared as `declare const enum`: TypeScript inlines the values. Plain Lua has a read-only table of the
same name in every environment of the enum's sides (`UnitType.Player`); `pairs` walks its members.

### `UnitType` (C++ `eD2UnitType`), 6 members, data server client

What kind of unit a unit is.
C++ eD2UnitType.

`Player` = `0`, `Monster` = `1`, `Object` = `2`, `Missile` = `3`, `Item` = `4`, `Warp` = `5`

### `PlayerMode` (C++ `eD2PlayerAnimMode`), 20 members, data server client

A player's mode (animation and action).
C++ eD2PlayerAnimMode.

`Death` = `0`, `Neutral` = `1`, `Walk` = `2`, `Run` = `3`, `GetHit` = `4`, `TownNeutral` = `5`, `TownWalk` = `6`, `Attack1` = `7`, `Attack2` = `8`, `Block` = `9`, `Cast` = `10`, `Throw` = `11`, `Kick` = `12`, `Skill1` = `13`, `Skill2` = `14`, `Skill3` = `15`, `Skill4` = `16`, `Dead` = `17`, `Sequence` = `18`, `KnockBack` = `19`

### `MonsterMode` (C++ `eD2MonsterAnimMode`), 16 members, data server client

A monster's mode (animation and action).
C++ eD2MonsterAnimMode.

`Death` = `0`, `Neutral` = `1`, `Walk` = `2`, `GetHit` = `3`, `Attack1` = `4`, `Attack2` = `5`, `Block` = `6`, `Cast` = `7`, `Skill1` = `8`, `Skill2` = `9`, `Skill3` = `10`, `Skill4` = `11`, `Dead` = `12`, `Knockback` = `13`, `Sequence` = `14`, `Run` = `15`

### `ObjectMode` (C++ `eD2ObjectAnimMode`), 8 members, data server client

An object's mode.
C++ eD2ObjectAnimMode.

`Neutral` = `0`, `Operating` = `1`, `Opened` = `2`, `Special1` = `3`, `Special2` = `4`, `Special3` = `5`, `Special4` = `6`, `Special5` = `7`

### `ItemMode` (C++ `eD2ItemAnimMode`), 7 members, data server client

Where an item is: in a container, equipped, in the belt, on the ground, on the cursor...
C++ eD2ItemAnimMode.

`Container` = `0`, `Equipment` = `1`, `Belt` = `2`, `Ground` = `3`, `Cursor` = `4`, `Dropped` = `5`, `InItem` = `6`

### `UiVar` (C++ `eD2UIvars`), 35 members, data server client

The client's UI flags: which panel or state is open.
C++ eD2UIvars.

`Game` = `0`, `Inventory` = `1`, `Statscreen` = `2`, `Skillselect` = `3`, `Skilltree` = `4`, `Chatbox` = `5`, `Newstats` = `6`, `Newskills` = `7`, `Npcmenu` = `8`, `Escmenu` = `9`, `Automap` = `10`, `Config` = `11`, `Npcshop` = `12`, `Holdalt` = `13`, `Anvil` = `14`, `Questscreen` = `15`, `Questscroll` = `16`, `Questlog` = `17`, `Nodraw` = `18`, `Hireables` = `19`, `Waypoint` = `20`, `Minipanel` = `21`, `Partyscreen` = `22`, `Mptrade` = `23`, `Msglog` = `24`, `Stash` = `25`, `Cube` = `26`, `Steegstone` = `27`, `Guildvault` = `28`, `Beltrows` = `31`, `Helpscreen` = `33`, `Helpscreenbtn` = `34`, `Hireicons` = `35`, `Mercinv` = `36`, `Recipescroll` = `37`

### `AiTargeting` (C++ `D2AITypes`), 6 members, data server client

How the engine picks a target before an AI thinks (an AI entry's targeting type).
C++ D2AITypes.

`Default` = `0`, `Basic` = `1`, `Advanced` = `2`, `Special` = `3`, `Multilayer` = `4`, `Hidden` = `5`

### `QuestId` (C++ `eD2Quests`), 43 members, data server client

The quest records: slots of a player's quest flags.
C++ eD2Quests.

`SpokeToWarriv` = `0`, `DenOfEvil` = `1`, `SistersBurialGrounds` = `2`, `ToolsOfTheTrade` = `3`, `TheSearchForCain` = `4`, `ForgottenTower` = `5`, `SistersToTheSlaughter` = `6`, `AbleGoToAct2` = `7`, `SpokeToJerhyn` = `8`, `RadamentsLair` = `9`, `TheHoradricStaff` = `10`, `TheTaintedSun` = `11`, `TheArcaneSanctuary` = `12`, `TheSummoner` = `13`, `TheSevenTombs` = `14`, `AbleGoToAct3` = `15`, `SpokeToHratli` = `16`, `LamEsensTome` = `17`, `KhalimsWill` = `18`, `BladeOfTheOldReligion` = `19`, `TheGoldenBird` = `20`, `TheBlackenedTemple` = `21`, `TheGuardian` = `22`, `AbleGoToAct4` = `23`, `SpokeToTyrael` = `24`, `TheFallenAngel` = `25`, `TerrorsEnd` = `26`, `HellsForge` = `27`, `AbleGoToAct5` = `28`, `UnknownDuplicate2` = `29`, `Act2Guard4Gossip` = `30`, `Act2Guard5Gossip` = `31`, `UnknownDuplicate1` = `32`, `SpokeToMalachai` = `33`, `Act4Traversed` = `34`, `SiegeOnHaggorath` = `35`, `RescueOnMountArreat` = `36`, `PrisonOfIce` = `37`, `BetrayalOfHaggorath` = `38`, `RiteOfPassage` = `39`, `EveOfDestruction` = `40`, `Respec` = `41`, `ActIntroduced` = `42`

### `QuestState` (C++ `eD2QuestState`), 16 members, data server client

The bits of one quest record.
C++ eD2QuestState.

`Completed` = `0`, `RewardAwaits` = `1`, `Step0x02` = `2`, `Step0x03` = `3`, `Step0x04` = `4`, `Step0x05` = `5`, `Step0x06` = `6`, `Step0x07` = `7`, `Step0x08` = `8`, `Step0x09` = `9`, `Step0x10` = `10`, `Step0x11` = `11`, `BoxGrayed` = `12`, `JustCompleted` = `13`, `CannotCompleteQuestInGame` = `14`, `FinishedInPreviousGame` = `15`

### `ItemQuality` (C++ `eD2ItemQuality`), 10 members, data server client

An item's quality.
C++ eD2ItemQuality.

`Invalid` = `0`, `Low` = `1`, `Normal` = `2`, `Superior` = `3`, `Magic` = `4`, `Set` = `5`, `Rare` = `6`, `Unique` = `7`, `Crafted` = `8`, `Tampered` = `9`

### `DamageType` (C++ `eD2ElementalDamageType`), 13 members, data server client

The element of damage.
C++ eD2ElementalDamageType.

`None` = `0`, `Fire` = `1`, `Lightning` = `2`, `Magic` = `3`, `Cold` = `4`, `Poison` = `5`, `Lifedrain` = `6`, `Manadrain` = `7`, `Stamdrain` = `8`, `Stun` = `9`, `Random` = `10`, `Burning` = `11`, `Freeze` = `12`

### `StatId` (C++ `eD2UnitStat`), 359 members, data server client

Stat ids: the stock rows of ItemStatCost, which the engine refers to by number.
C++ eD2UnitStat.

`Strength` = `0`, `Energy` = `1`, `Dexterity` = `2`, `Vitality` = `3`, `Statpts` = `4`, `Newskills` = `5`, `Hitpoints` = `6`, `Maxhp` = `7`, `Mana` = `8`, `Maxmana` = `9`, `Stamina` = `10`, `Maxstamina` = `11`, ... (all in the declarations)

### `SkillId` (C++ `eD2Skills`), 357 members, data server client

Skill ids: the stock rows of Skills, which the engine refers to by number.
C++ eD2Skills.

`Attack` = `0`, `Kick` = `1`, `Throw` = `2`, `Unsummon` = `3`, `LeftHandThrow` = `4`, `LeftHandSwing` = `5`, `MagicArrow` = `6`, `FireArrow` = `7`, `InnerSight` = `8`, `CriticalStrike` = `9`, `Jab` = `10`, `ColdArrow` = `11`, ... (all in the declarations)

### `DrawMode` (C++ `DRAWMODE`), 8 members, data server client

How a sprite is blended when drawn.
C++ DRAWMODE.

`Verytransparent` = `0`, `Transparent` = `1`, `Lesstransparent` = `2`, `Lighttransparent` = `3`, `Darktransparent` = `4`, `Solid` = `5`, `Darkersolid` = `6`, `Lightersolid` = `7`

### `Difficulty` (C++ `eD2Difficulty`), 3 members, data server client

The game's difficulty.
C++ eD2Difficulty.

`Normal` = `0`, `Nightmare` = `1`, `Hell` = `2`

### `PlayerClass` (C++ `eD2PlayerClassID`), 7 members, data server client

A player's character class.
C++ eD2PlayerClassID.

`Amazon` = `0`, `Sorceress` = `1`, `Necromancer` = `2`, `Paladin` = `3`, `Barbarian` = `4`, `Druid` = `5`, `Assassin` = `6`

### `BodyLoc` (C++ `eD2BodyLoc`), 13 members, data server client

Equipment slots.
C++ eD2BodyLoc.

`None` = `0`, `Head` = `1`, `Neck` = `2`, `Torso` = `3`, `RArm` = `4`, `LArm` = `5`, `RRing` = `6`, `LRing` = `7`, `Belt` = `8`, `Feet` = `9`, `Gloves` = `10`, `SwappedRArm` = `11`, `SwappedLArm` = `12`

### `ItemFlag` (C++ `eD2ItemFlag`), 31 members, data server client

An item's flag bits.
C++ eD2ItemFlag.

`Tagged` = `-2147483648`, `Reload` = `1`, `Bought` = `2`, `Cursor` = `4`, `Ignore` = `8`, `Identified` = `16`, `Removed` = `32`, `Added` = `64`, `Taken` = `128`, `Broken` = `256`, `Restored` = `512`, `Sorted` = `1024`, `Socketed` = `2048`, `Monster` = `4096`, `New` = `8192`, `Disabled` = `16384`, `Hardcore` = `32768`, `Bodypart` = `65536`, `Beginner` = `131072`, `Restrict` = `262144`, `Server` = `524288`, `Compact` = `2097152`, `Ethereal` = `4194304`, `Saved` = `8388608`, `Playername` = `16777216`, `Crude` = `33554432`, `Runeword` = `67108864`, `Magical` = `134217728`, `Staffmods` = `268435456`, `Cursed` = `536870912`, `Drow` = `1073741824`

### `MonsterTypeFlag` (C++ `eD2MonsterType`), 8 members, data server client

A monster's type bits: champion, unique, minion, ghostly...
C++ eD2MonsterType.

`Other` = `1`, `Superunique` = `2`, `Champion` = `4`, `Unique` = `8`, `Minion` = `16`, `Possessed` = `32`, `Ghostly` = `64`, `Multishot` = `128`

### `Act` (C++ `eAct`), 6 members, data server client

An act.
C++ eAct.

`I` = `0`, `II` = `1`, `III` = `2`, `IV` = `3`, `V` = `4`, `None` = `255`

### `Event`, 2 members, data server client

Every event, server and client. The value is the event's name on the wire and in logs; which
events a side raises is its events map (ServerEvents, ClientEvents), and subscribing to one the
side does not raise is a type error in TypeScript and an error at run time in Lua.

`Tick` = `"on_tick"`, `Draw` = `"on_draw"`

### `Side`, 3 members, data server client

The sides a mod runs on, each its own Lua state.

`Data` = `1`, `Server` = `2`, `Client` = `4`

### `Outcome`, 4 members, data server client

How an action or a wait ended.

`Done` = `0`, `Interrupted` = `1`, `Blocked` = `2`, `Timeout` = `3`

### `SettingType`, 5 members, data server client

The value type of a mod setting.

`Bool` = `"bool"`, `Int` = `"int"`, `Number` = `"number"`, `String` = `"string"`, `Choice` = `"choice"`

### `SettingScope`, 3 members, data server client

Who reads a mod setting, and when it can change.

`Startup` = `"startup"`, `Game` = `"game"`, `Player` = `"player"`

### `ExampleId`, 1 members, server

The ids of the example registry. A mod adds its own by declaring members of this enum.

`Greet` = `"base:greet"`

## storage

A mod's `storage` is saved with the game. It holds booleans, numbers, strings, tables without a
metatable, and handles; keys are strings, numbers or booleans. Shared tables and cycles are kept.
Functions, coroutines and other userdata are refused with the path of the value in the error.
