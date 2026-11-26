// Points on the two grids. A point is a read-only table carrying its kind, so a tile is never taken
// for a subtile; every API that takes a point checks the kind.
import type { Subtile, SubtileRect, Tile } from "../redivivus/server";
import { PointKind } from "../redivivus/enums";
import { show } from "./check";
import { readonly } from "./proxy";

const SUBTILES_PER_TILE = 5;

// The points the core made (their tables are proxies whose metatable is the core's).
const made = new LuaTable<object, boolean>();
setmetatable(made, { __mode: "k" });

function field(p: object, key: string): unknown {
    return made.get(p) ? (p as Record<string, unknown>)[key] : rawget(p, key);
}

function int(v: unknown, i: number, where: string): number {
    if (type(v) !== "number" || (v as number) !== math.floor(v as number) || (v as number) < -2147483648 || (v as number) > 2147483647) {
        error(`bad argument #${i} to '${where}' (a 32-bit integer expected, got ${show(v)})`, 4);
    }
    return v as number;
}

export function subtile(x: unknown, y: unknown): Subtile {
    const p = readonly<Subtile>({ kind: PointKind.Subtile, x: int(x, 1, "subtile"), y: int(y, 2, "subtile") }, "subtile");
    made.set(p, true);
    return p;
}

export function tile(x: unknown, y: unknown): Tile {
    const p = readonly<Tile>({ kind: PointKind.Tile, x: int(x, 1, "tile"), y: int(y, 2, "tile") }, "tile");
    made.set(p, true);
    return p;
}

export function toTile(p: unknown): Tile {
    const s = subtileArg(p, 1, "toTile");
    return tile(math.floor(s.x / SUBTILES_PER_TILE), math.floor(s.y / SUBTILES_PER_TILE));
}

export function toSubtile(p: unknown): Subtile {
    if (type(p) !== "table" || field(p as object, "kind") !== PointKind.Tile) error("bad argument #1 to 'toSubtile' (a tile point expected)", 3);
    return subtile(int(field(p as object, "x"), 1, "toSubtile") * SUBTILES_PER_TILE, int(field(p as object, "y"), 1, "toSubtile") * SUBTILES_PER_TILE);
}

/** A subtile point argument; a tile point or a bare table is refused, naming the kind. */
export function subtileArg(p: unknown, i: number, where: string): Subtile {
    if (type(p) !== "table") error(`bad argument #${i} to '${where}' (a subtile point expected, got ${type(p)})`, 4);
    const kind = field(p as object, "kind");
    if (kind !== PointKind.Subtile) {
        error(`bad argument #${i} to '${where}' (a subtile point expected${kind === PointKind.Tile ? ", got a tile point: convert it with toSubtile" : ""})`, 4);
    }
    if (made.get(p as object)) return p as Subtile;
    return subtile(int(rawget(p as object, "x"), i, where), int(rawget(p as object, "y"), i, where));
}

export function rect(x: number, y: number, width: number, height: number): SubtileRect {
    return readonly<SubtileRect>({ x, y, width, height }, "rect");
}
