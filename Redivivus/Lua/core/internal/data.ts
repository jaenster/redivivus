// The data stage's shared table: every mod's data entry sees the same raw prototypes (that is the
// point of the stage). extend reads what a mod passes raw.
import { native } from "./native";

const SIDE_DATA = 1;

export const raw = new LuaTable<string, LuaTable<string, object>>();

export function extend(list: unknown): void {
    if (native.side() !== SIDE_DATA) error("data.extend is for the data stage (data, data-updates, data-final-fixes)", 3);
    if (type(list) !== "table") error("data.extend: a list of prototypes", 3);
    const n = (list as unknown[]).length;
    for (let i = 1; i <= n; i++) {
        const proto = rawget(list as object, i);
        if (type(proto) !== "table") error(`data.extend: entry ${i} is not a table`, 3);
        const t = rawget(proto, "type");
        const name = rawget(proto, "name");
        if (type(t) !== "string" || type(name) !== "string") error(`data.extend: entry ${i} needs string fields type and name`, 3);
        let byName = raw.get(t);
        if (!byName) {
            byName = new LuaTable();
            raw.set(t, byName);
        }
        byName.set(name, proto);
    }
}
