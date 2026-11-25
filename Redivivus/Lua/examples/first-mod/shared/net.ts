// A typed channel between this mod's client and server halves.
import { defineChannel, t } from "__base__/shared/kit/channels";
import { Pace } from "./settings";

export const net = defineChannel("net", {
    requests: { wave: [t.text(32), t.oneOf([Pace.Slow, Pace.Fast])] },
    events: { waved: [t.int(), t.text(32)] },
});
