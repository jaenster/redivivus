// Mephisto: the stock AI of Mephisto, ported to TypeScript twice: mechanically (switch.ts, the gotos
// as a switch) and composed of small actions (mephisto.ts). A setting picks which one runs, or keeps
// the game's own.
import { AiTargeting, MonsterAiId, type Game } from "redivivus:server";
import { log } from "redivivus:shared";
import { mephisto } from "./mephisto";
import { options } from "./options";
import { mephistoSwitch } from "./switch";

let current: Game | undefined;

/** Run Mephisto with one of the ports in the game this state hosts last ("stock" leaves the game's own). */
export function useVersion(version: string): void {
    const game = current;
    if (game === undefined) return;
    if (version === "switch") game.registry.ai.register(MonsterAiId.Mephisto, mephistoSwitch, AiTargeting.Basic);
    else if (version === "composed") game.registry.ai.register(MonsterAiId.Mephisto, mephisto, AiTargeting.Basic);
    log(`mephisto: the ${version} AI runs Mephisto in game ${game.id}`);
}

export function server(game: Game) {
    current = game;
    useVersion(options.version);
}
