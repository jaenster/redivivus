// A typed channel between this mod's server and client sides, declared once.
import { defineChannel, t } from "redivivus:shared";
import { Pace } from "./options";

export const net = defineChannel("net", {
    requests: { wave: [t.text(32), t.oneOf([Pace.Slow, Pace.Fast])] },
    events: { waved: [t.int(), t.text(32)] },
});
