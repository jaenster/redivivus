// Cheap mana: every skill costs half its mana, the kind of balance change Hellforged makes in C++.
// It overrides the server's own mana deduction, D2Game's CalculateManaUse (called when a player's
// skill is used): next() runs the stock deduction, and the override gives half of what it took back.
// The function is the server's, so the override runs in each game's server state; the client shows
// the mana the server sends.
import { defineMod } from "redivivus:mod";
import { Side, StatId } from "redivivus:enums";
import { log } from "redivivus:shared";

/** How many casts are logged, so the log shows the override running without filling up. */
const LOG_CASTS = 8;

export default defineMod({
    patch(engine) {
        if (engine.side !== Side.Server) return;
        let logged = 0;
        engine.D2Game.SKILLS.SkillsServer.CalculateManaUse.override((next, game, unit, skill, level) => {
            if (unit === undefined || !unit.valid) return next(game, unit, skill, level);
            const before = unit.getStat(StatId.Mana);
            next(game, unit, skill, level);
            const spent = before - unit.getStat(StatId.Mana);
            if (spent > 0) unit.addStat(StatId.Mana, spent / 2);
            if (logged < LOG_CASTS) {
                logged++;
                log(`cheap-mana: skill ${skill} level ${level} took ${spent} mana, ${spent / 2} given back`);
            }
        });
        log("cheap-mana: CalculateManaUse overridden");
    },
});
