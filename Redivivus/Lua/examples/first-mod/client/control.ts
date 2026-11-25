import { greet } from "../shared/greet";
import { net } from "../shared/net";
import { Pace } from "../shared/settings";

net.on.waved((from, text) => log(`peer ${from} waved: ${text}`));
script.on_init(() => net.call.wave("hi", Pace.Fast));

script.on_event(Event.Draw, () => {
    // The client draws; it has no storage, registries or scripts.
    void greet("client");
});
