// Games. Everything a mod registers in the control stage hangs off the object its `server` or
// `client` export received, and so off one game: handlers, registry entries and wrappers, scripts,
// channel endpoints, persisted state and storage. A state may serve several games at once; nothing
// here assumes one. Ending a game drops all of it.

/** A registry entry of one game: the implementation at the bottom and the wrappers above it. */
export type Wrapper = { fn: (this: void, ...args: any[]) => any; mod: number; order: number; seq: number; removed: boolean };
export type Entry = { impl?: (this: void, ...args: any[]) => any; owner: number; wrappers: Wrapper[] };
export type Handler = { mod: number; order: number; fn: (this: void, e: object) => void };
export type ScriptRecord = {
    co: LuaThread;
    mod: number;
    due: number;
    event?: string;
    id: number;
    holder?: unknown;
    done: boolean;
};
export type Endpoint = { name: string; requests: ChannelEntry[]; events: ChannelEntry[] };
export type ChannelEntry = { name: string; args: readonly object[]; handler?: (this: void, ...args: any[]) => void };

export type GameState = {
    id: number;
    ended: boolean;
    /** event -> handlers in load order, one per mod. */
    handlers: LuaTable<string, Handler[]>;
    /** registry -> id -> entry. */
    entries: LuaTable<string, LuaTable<string, Entry>>;
    scripts: ScriptRecord[];
    /** mod -> the mod's storage for this game. */
    storage: LuaTable<number, object>;
    /** mod -> channel name -> endpoint. */
    endpoints: LuaTable<number, LuaTable<string, Endpoint>>;
    /** mod -> whether storage came from a save (or a reload). */
    loaded: LuaTable<number, boolean>;
    /** mod -> the Game (or Client) object its export got. */
    objects: LuaTable<number, object>;
};

const games = new LuaTable<number, GameState>();

export function create(id: number): GameState {
    const g: GameState = {
        id, ended: false, handlers: new LuaTable(), entries: new LuaTable(), scripts: [],
        storage: new LuaTable(), endpoints: new LuaTable(), loaded: new LuaTable(), objects: new LuaTable(),
    };
    games.set(id, g);
    return g;
}

export function find(id: number): GameState | undefined {
    return games.get(id);
}

/** Drop the game and everything registered through it. Objects handed out for it stop working. */
export function end(id: number): boolean {
    const g = games.get(id);
    if (!g) return false;
    g.ended = true;
    for (const s of g.scripts) s.done = true;
    games.delete(id);
    return true;
}

/** Drop what one mod registered through the game (a reload of that game); its storage stays. */
export function clearRegistrations(g: GameState): void {
    g.handlers = new LuaTable();
    g.entries = new LuaTable();
    for (const s of g.scripts) s.done = true;
    g.scripts = [];
    g.endpoints = new LuaTable();
}

export function ids(): number[] {
    const out: number[] = [];
    for (const [id] of games) out.push(id);
    return out;
}
