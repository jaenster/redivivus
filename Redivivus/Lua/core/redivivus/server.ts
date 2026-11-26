/**
 * The server: what a mod's `server` export receives, once for every game this process hosts. The
 * module itself holds types and the enums the game object uses; everything a mod registers goes
 * through the Game it is handed and lasts as long as that game.
 * @side data server client
 */

import { Event } from "./enums";
import type { Act, AiTargeting, Alignment, Difficulty, ExampleId, LevelId, MinionOrder, MonsterAiId, MonsterId, MonsterMode, Outcome, PlayerClassId, PlayerMode, PointKind, SkillId, StateId, StateMask, StatId, UnitType } from "./enums";
import type { ChannelDeclaration, EventBase, Handle, integer, KeyedRegistry, MessageArgs, Messages, Script, ScriptTask, SlotRegistry } from "./shared";
import * as points from "../internal/points";

export { AiTargeting, Alignment, Event, MinionOrder, MonsterAiId, MonsterId, MonsterMode, Outcome, PointKind, SkillId, StatId, StateId, StateMask, UnitType } from "./enums";

/**
 * The events a game raises, keyed by Event. A mod subscribes with game.on.
 * @events server
 */
export interface ServerEvents {
    /** Every game tick, before the scripts that are due resume. */
    [Event.Tick]: EventBase;
}

/**
 * The example registry: shows keyed entries, a native implementation at the bottom of an entry's
 * chain, and ids added by mods (augment ExampleId in redivivus:enums and this interface here).
 * @registry
 */
export interface ExampleRegistry {
    /**
     * Greets: "hello, <name>".
     * @native D2LuaExample_Greet
     */
    [ExampleId.Greet]: (this: void, name: string) => string;
}

/**
 * The example single-slot registry: doubles a number.
 * @native D2LuaExample_Slot
 */
export type ExampleSlot = (this: void, value: number) => number;

/**
 * The behaviour registries of a game.
 * @registries server
 */
export interface ServerRegistries {
    readonly example: KeyedRegistry<ExampleRegistry>;
    readonly exampleSlot: SlotRegistry<ExampleSlot>;
}

/** A channel as the server has it, for one game. */
export interface ServerChannel<R extends Messages, E extends Messages> {
    readonly name: string;
    /** Handle the requests. Each handler gets the sending client's peer id first. */
    implement(this: void, handlers: { [K in keyof R]: (this: void, peer: number, ...args: MessageArgs<R[K]>) => void }): void;
    /** Send an event to one client, or to every client with peer 0. */
    readonly emit: { readonly [K in keyof E]: (this: void, peer: number, ...args: MessageArgs<E[K]>) => void };
}

// --- positions ------------------------------------------------------------------------------------

/** A point on the subtile grid: where units stand, what collision and distances use. */
export interface Subtile {
    readonly kind: PointKind.Subtile;
    readonly x: integer;
    readonly y: integer;
}

/** A point on the tile grid (5 by 5 subtiles): how rooms and levels are laid out. */
export interface Tile {
    readonly kind: PointKind.Tile;
    readonly x: integer;
    readonly y: integer;
}

/** A rectangle of subtiles. */
export interface SubtileRect {
    readonly x: integer;
    readonly y: integer;
    readonly width: integer;
    readonly height: integer;
}

/** A subtile point. */
export function subtile(x: integer, y: integer): Subtile {
    return points.subtile(x, y);
}

/** A tile point. */
export function tile(x: integer, y: integer): Tile {
    return points.tile(x, y);
}

/** The tile a subtile lies in. */
export function toTile(p: Subtile): Tile {
    return points.toTile(p);
}

/** The subtile at a tile's top corner. */
export function toSubtile(p: Tile): Subtile {
    return points.toSubtile(p);
}

// --- the world -------------------------------------------------------------------------------------

/**
 * A level of the game (an area: Blood Moor, a town). A handle: it exists while the level is
 * allocated in this game, which happens when a player first comes near it, never because a mod
 * asked.
 */
export interface Level extends Handle {
    readonly id: LevelId;
    /** The level's name, localised. */
    readonly name: string;
    readonly act: Act;
    readonly isTown: boolean;
    /** Whether its rooms are generated. */
    readonly generated: boolean;
    /** Its extent when generated. */
    readonly bounds: SubtileRect | undefined;
    /** Its active rooms (the ones near a player). */
    rooms(): LuaIterable<Room>;
    /** The units in its active rooms, optionally of one type. A snapshot: units gone since are skipped. */
    units(type?: UnitType): LuaIterable<AnyUnit>;
    monsters(): LuaIterable<Monster>;
    players(): LuaIterable<Player>;
}

/**
 * A room of a level, active while a player is near. A handle that names the room by its place, so it
 * is valid again when the room is loaded again.
 */
export interface Room extends Handle {
    readonly bounds: SubtileRect;
    readonly level: Level;
    /** The active rooms next to it. */
    neighbours(): LuaIterable<Room>;
    units(type?: UnitType): LuaIterable<AnyUnit>;
    monsters(): LuaIterable<Monster>;
    players(): LuaIterable<Player>;
}

/**
 * A unit: a player, a monster, an object, a missile, an item. A handle: it names the unit by type
 * and id and is looked up on every use. It is not valid once the unit is gone, while its room is
 * unloaded (a monster comes back with its room, under the same id), and for a monster while it is
 * dead. Any member but `valid`, `id` and `type` then raises an error.
 */
export interface Unit extends Handle {
    /** The unit's id (its guid; ids are per type). */
    readonly id: integer;
    readonly type: UnitType;
    /** The row of its class table (MonStats for a monster, the class for a player). */
    readonly classId: integer;
    readonly game: Game;
    readonly position: Subtile;
    readonly room: Room | undefined;
    readonly level: Level | undefined;
    readonly act: Act;
    /** The mode: what it is doing. Monsters and players narrow it. */
    readonly mode: integer;
    readonly owner: AnyUnit | undefined;
    /** Straight-line distance in subtiles. */
    distanceTo(other: Unit | Subtile): number;
    /** Units within radius subtiles of this one (itself excluded), optionally of one type. */
    nearby(radius: number, type?: UnitType): LuaIterable<AnyUnit>;
    /** A stat's value with every modifier (life, mana and stamina in whole points). */
    getStat(stat: StatId, layer?: integer): number;
    /** A stat's base value, before modifiers. */
    getBaseStat(stat: StatId, layer?: integer): number;
    /** Set a stat's base value through the stat list; gold goes through the gold rules elsewhere. */
    setStat(stat: StatId, value: number, layer?: integer): void;
    /** Add to a stat's base value. */
    addStat(stat: StatId, delta: number, layer?: integer): void;
    hasState(state: StateId): boolean;
    /** Put the unit in a state for frames (0: until removed). False when the engine refused. */
    addState(state: StateId, frames?: integer): boolean;
    removeState(state: StateId): void;
}

/** A monster (and NPCs, which are monsters too). */
export interface Monster extends Unit {
    readonly type: UnitType.Monster;
    readonly classId: MonsterId;
    readonly mode: MonsterMode;
    readonly champion: boolean;
    readonly unique: boolean;
    readonly superUnique: boolean;
    readonly minion: boolean;
    readonly boss: boolean;
    /** Life in whole points; setting it to 0 or less is refused, use kill. */
    life: number;
    readonly maxLife: number;
    /** The target of what it is doing now (a corpse while it raises one). */
    readonly target: AnyUnit | Corpse | undefined;
    /** Kill it as a script does: death, drops, quest hooks. A killer gets the experience. */
    kill(killer?: Unit): void;
    /** Its MonStats row, as the game's difficulty reads it. */
    readonly data: MonsterData;
    /** The leader of its group as the AI sees it (itself when it leads one), when there is one. */
    readonly leader: Monster | undefined;
    readonly alignment: Alignment;
    /**
     * A roll of the monster's own seed, modulo max (1 or more): the stream the stock AIs roll, so a
     * script that rolls where a stock AI rolls makes the same decisions.
     */
    random(max: integer): integer;
    /** Whether a state of one of the states.txt masks is on it. */
    hasStateMask(mask: StateMask): boolean;
    /** The distance as the AI measures it: the larger axis plus half the smaller, less its size. */
    aiDistanceTo(other: Unit | Corpse): integer;
    /** The living monsters of the group it leads, in the engine's order. */
    minions(): LuaIterable<Monster>;
    /** The corpses of the group it leads, in the engine's order. */
    minionCorpses(): LuaIterable<Corpse>;
    /**
     * The monster corpses in the rooms around it within radius subtiles, in the order the engine's
     * own scans meet them (room by room of its adjacent rooms).
     */
    corpsesNear(radius: number): LuaIterable<Corpse>;
    /** The target the engine's AI would pick now (an AI's FindTargetAndDistance), with its AI distance. */
    findTarget(): { readonly target: AnyUnit; readonly distance: integer } | undefined;
    /** Whether the skill can be cast at the unit or corpse now (its target and placement checks). */
    canCast(skill: SkillId, at: Unit | Corpse): boolean;
    /** Give each monster of the group it is in an order (what the group's AIs read next). */
    orderMinions(order: MinionOrder): void;

    // The actions: only inside this monster's own AI script, each run with yield*.
    /** Stand for frames (default: its AI delay), then think again. */
    idle(frames?: integer): Script<Outcome>;
    /** Walk to a point. Blocked when no path could be started or it stopped short. */
    walkTo(to: Subtile): Script<Outcome>;
    /** Run to a point (walks when it cannot run). */
    runTo(to: Subtile): Script<Outcome>;
    /** Walk toward a unit. Interrupted when the unit is gone or dead. */
    walkToward(target: Unit): Script<Outcome>;
    runToward(target: Unit): Script<Outcome>;
    /** Attack (Attack1, or Attack2 with secondary). */
    attack(target: Unit, secondary?: boolean): Script<Outcome>;
    /** Cast a skill in a mode (default Cast; a MonStats SkMode column gives a skill's own). */
    castSkill(skill: SkillId, at: Unit | Corpse | Subtile, mode?: MonsterMode): Script<Outcome>;
    /** Cast a sequence skill (mode Sequence: the animation the skill's sequence table plays), as a resurrect. */
    castSequence(skill: SkillId, at: Unit | Corpse | Subtile): Script<Outcome>;
    /** Close in on a unit along a curve, clockwise or not by a roll of its seed, steps long. */
    circleToward(target: Unit, steps: integer): Script<Outcome>;
    /** Walk to a random spot within radius. */
    wander(radius: integer): Script<Outcome>;
    /** Run away from a unit for distance subtiles. */
    flee(from: Unit, distance: integer): Script<Outcome>;
}

/** A player character. */
export interface Player extends Unit {
    readonly type: UnitType.Player;
    readonly classId: PlayerClassId;
    readonly mode: PlayerMode;
    readonly name: string;
    readonly charLevel: integer;
    readonly experience: integer;
    readonly gold: integer;
    readonly goldStash: integer;
    readonly dead: boolean;
    /** A line in this player's chat. */
    sendMessage(text: string): void;
}

/**
 * A monster's body: the monster while it lies dead. A handle of its own, named by the monster's id:
 * valid while the monster is dead and its room loaded, and not valid once it is alive again (its
 * Monster handle is then valid) or gone.
 */
export interface Corpse extends Handle {
    /** The monster's id (its Monster handle has the same). */
    readonly id: integer;
    readonly classId: MonsterId;
    /** The base class of its MonStats row (fallen1 for every Fallen). */
    readonly baseId: MonsterId;
    readonly position: Subtile;
    readonly mode: MonsterMode;
    readonly alignment: Alignment;
    /** Whether it can be selected and targeted (a corpse used up by a skill is not). */
    readonly targetable: boolean;
    readonly champion: boolean;
    readonly unique: boolean;
    /** The leader of the group it was in, when there is one. */
    readonly leader: Monster | undefined;
    hasStateMask(mask: StateMask): boolean;
    /** Straight-line distance in subtiles. */
    distanceTo(other: Unit | Corpse | Subtile): number;
}

/** Any unit a query gives: narrow it by `type`. */
export type AnyUnit = Player | Monster | (Unit & { readonly type: Exclude<UnitType, UnitType.Player | UnitType.Monster> });

/** A MonStats row as the AI reads it, for the game's difficulty. A snapshot: read it once. */
export interface MonsterData {
    readonly id: MonsterId;
    readonly baseId: MonsterId;
    readonly ai: MonsterAiId;
    /** Frames between thinks when idle (aidel). */
    readonly aiDelay: integer;
    /** The AI parameters aip1..aip8 at [0]..[7]; what each means is the AI's (monai.txt). */
    readonly aiParams: readonly integer[];
    /** Skill1..Skill8 at [0]..[7]; -1 for none. */
    readonly skills: readonly SkillId[];
    /** Sk1mode..Sk8mode at [0]..[7]: the mode each skill is cast in. */
    readonly skillModes: readonly MonsterMode[];
}

/** What spawnMonster takes besides the monster and the place. */
export interface SpawnOptions {
    /** How far from the point to look for a free spot, in subtiles (default 4). */
    readonly radius?: integer;
    /** Spawn its minion group too (default false). */
    readonly minions?: boolean;
}

// --- the monster AI --------------------------------------------------------------------------------

/** What the engine knows when a monster thinks. Updated before every think of the script. */
export interface Think {
    /** The engine's pick of a target (AiTargeting.Advanced); with Default the script picks its own. */
    readonly target: AnyUnit | undefined;
    /** The distance to the target as the engine measures it. */
    readonly distance: integer;
    /** Whether the monster is engaged in combat. */
    readonly engaged: boolean;
}

/**
 * A monster's AI as a script: it runs once per monster, from the top, when the monster gets the AI
 * (spawned, its room loaded, revived, a curse wears off: whenever the engine attaches an AI) and
 * ends when the monster dies or is gone. A bare `yield` waits one think; `yield* monster.walkTo(p)`
 * runs an action and gives its Outcome. When it returns it starts over at the next think.
 */
export type AiScript = (this: void, monster: Monster, think: Think) => Script<void>;

/**
 * A wrapper around one think of an AI: next() runs one think of what is below (the stock AI, or a
 * script a mod registered, or a lower wrapper). A wrapper that does not call next and starts no
 * action leaves the monster idle for its AI delay.
 */
export type AiWrapper = (this: void, next: (this: void) => void, monster: Monster, think: Think) => void;

/** The monster AIs of a game, by MonsterAiId: every monster whose MonStats AI is the id. */
export interface AiRegistry {
    /** Run script as the AI. Targeting decides what think.target holds (default: none, the script picks). */
    register(this: void, id: MonsterAiId, script: AiScript, targeting?: AiTargeting): void;
    /** Wrap each think of the AI. Later mods wrap earlier ones; a wrapper that errors is removed. */
    wrap(this: void, id: MonsterAiId, wrapper: AiWrapper): void;
    /** Whether a mod registered or wrapped the AI in this game. */
    has(this: void, id: MonsterAiId): boolean;
}

/** The registries of a game: the declared ones and the monster AIs. */
export interface GameRegistries extends ServerRegistries {
    readonly ai: AiRegistry;
}

/** One game, as one mod sees it. Everything registered through it is dropped when the game ends. */
export interface Game {
    /** The game's id in this process. */
    readonly id: number;
    /** The current tick. */
    readonly tick: number;
    /**
     * This mod's saved state for this game under key: the table saved there, or a copy of defaults
     * the first time. Plain data and handles, saved with the game (see the storage rules in the
     * reference); restored before `server` is called. One key, one shape: every call site names the
     * key and its defaults, so two cannot disagree about a slot's type.
     */
    storage<T extends object>(this: void, key: string, defaults: T): T;
    /** Whether storage came from a save (or a reload) rather than starting empty. */
    readonly loaded: boolean;
    /** The behaviour registries of this game. */
    readonly registry: GameRegistries;
    readonly difficulty: Difficulty;
    readonly expansion: boolean;
    /** The game's name (empty in single player). */
    readonly name: string;
    /** The map seed. */
    readonly seed: integer;
    /** The engine's frame counter (25 frames a second). */
    readonly frame: integer;
    /** The players in the game. */
    players(this: void): LuaIterable<Player>;
    /** The level when it exists in this game now (never makes one). */
    level(this: void, id: LevelId): Level | undefined;
    /** A unit by type and id, when it is valid. */
    findUnit(this: void, type: UnitType, id: integer): AnyUnit | undefined;
    /** Units within radius subtiles of a point, optionally of one type. */
    unitsNear(this: void, at: Subtile, radius: number, type?: UnitType): LuaIterable<AnyUnit>;
    /**
     * Spawn a monster at the free spot nearest a point or a unit, in an active room. Undefined when
     * there is no active room there or no free spot.
     */
    spawnMonster(this: void, id: MonsterId, at: Subtile | Unit, options?: SpawnOptions): Monster | undefined;
    /** A system line to every player in the game. */
    message(this: void, text: string): void;
    /** Subscribe to an event; undefined unsubscribes. One handler per event per mod. */
    on<K extends keyof ServerEvents>(this: void, event: K, handler: ((this: void, e: ServerEvents[K]) => void) | undefined): void;
    /**
     * Start a script in this game, owned by this mod and, when given, by owner: it is dropped once
     * the owner's object is gone. It first runs on the next tick; every tick resumes the scripts
     * that are due in the order they started.
     */
    startScript(this: void, body: (this: void) => Script<void>, owner?: Handle): ScriptTask;
    /** Wait for an event of this game inside one of its scripts. The outcome is the payload. */
    waitEvent<K extends keyof ServerEvents>(this: void, event: K): Script<ServerEvents[K]>;
    /** Open a channel of this mod for this game. */
    channel<R extends Messages, E extends Messages>(this: void, declaration: ChannelDeclaration<R, E>): ServerChannel<R, E>;
    /**
     * The persisted state of owner in this game, created from a copy of defaults the first time;
     * kept in storage and dropped once the owner's object is gone.
     */
    persist<T extends object>(this: void, owner: Handle, defaults: T): T;
    /** Whether owner has persisted state in this game. */
    hasPersisted(this: void, owner: Handle): boolean;
}

/** The type of a mod's `server` export. */
export type ServerEntry = (this: void, game: Game) => void;
