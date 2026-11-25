// State that belongs to an engine object: kept in the mod's storage (so it is saved with the game)
// under the owner handle, and removed once the owner's object is gone.
//
//   const state = persist(monster, { charges: 0, lastSeen: 0 });
//   state.charges++;
//
// The first call for an owner stores a copy of the defaults; later calls, after a load too, give
// the stored table. State is plain data, as everything in storage is.
//
// storage.__kit_persist holds { owner = handle, state = table } entries. A script of the mod (one,
// started when this module loads) drops the entries whose owner is gone, once a tick.

type PersistEntry = { owner: Handle; state: object };

const KEY = "__kit_persist";

// Owner handle -> entry, for the entry list it was built from. Handles are interned, so a handle
// read back from storage is the same value as one the engine hands out for the same object.
let index = new LuaTable<Handle, PersistEntry>();
let indexed: PersistEntry[] | undefined;

function entries(): PersistEntry[] {
    let list = storage[KEY] as PersistEntry[] | undefined;
    if (!list) {
        list = [];
        storage[KEY] = list;
    }
    if (list !== indexed) {
        index = new LuaTable<Handle, PersistEntry>();
        for (const e of list) index.set(e.owner, e);
        indexed = list;
    }
    return list;
}

function copy<T>(value: T): T {
    if (type(value) !== "table") return value;
    const out = new LuaTable<AnyNotNil, unknown>();
    for (const [k, v] of pairs(value as object)) out.set(k as AnyNotNil, copy(v));
    return out as unknown as T;
}

/** The persisted state of owner, created from defaults the first time. */
export function persist<T extends object>(owner: Handle, defaults: T): T {
    if (type(owner) !== "userdata" || !owner.valid) error("persist: the owner must be a valid handle", 2);
    const list = entries();
    const found = index.get(owner);
    if (found) return found.state as T;
    const entry: PersistEntry = { owner, state: copy(defaults) };
    list.push(entry);
    index.set(owner, entry);
    return entry.state as T;
}

/** Whether owner has persisted state. */
export function hasPersisted(owner: Handle): boolean {
    entries();
    return index.has(owner);
}

/** Drop the state of every owner whose object is gone. The module's script calls it each tick. */
export function sweep(): integer {
    const list = storage[KEY] as PersistEntry[] | undefined;
    if (!list) return 0;
    entries();
    let removed = 0;
    for (let i = list.length - 1; i >= 0; i--) {
        if (!list[i].owner.valid) {
            index.delete(list[i].owner);
            list.splice(i, 1);
            removed++;
        }
    }
    return removed;
}

script.start(function* (): Script {
    for (;;) {
        yield;
        sweep();
    }
});
