// Charger: every Fallen in the game runs the charger AI (charger.ts) instead of its own, and while
// none is near a player outside town, one is spawned a little away from them, so there is always
// one to watch. Not in town: the engine lets no monster path there (PATH_CanMonsterPathInArea).
import { AiTargeting, Event, MonsterAiId, subtile, UnitType, type Game } from "redivivus:server";
import { log } from "redivivus:shared";
import { charger, CHARGER_CLASS } from "./charger";

/** Spawn check interval, in ticks (25 a second). */
const CHECK_EVERY = 50;
/** A player with no charger within this many subtiles gets one. */
const COVER = 30;
/** Where it is spawned: this far east of the player. */
const SPAWN_OFFSET = 12;

export function server(game: Game) {
    game.registry.ai.register(MonsterAiId.Fallen, charger, AiTargeting.Default);
    log(`charger: the Fallen charge in game ${game.id}`);

    game.on(Event.Tick, (e) => {
        if (e.tick % CHECK_EVERY !== 0) return;
        for (const player of game.players()) {
            if (player.dead || player.level === undefined || player.level.isTown) continue;
            let covered = false;
            for (const unit of player.nearby(COVER)) {
                if (unit.type === UnitType.Monster && unit.classId === CHARGER_CLASS) covered = true;
            }
            if (covered) continue;
            const at = subtile(player.position.x + SPAWN_OFFSET, player.position.y);
            const monster = game.spawnMonster(CHARGER_CLASS, at, { radius: 6 });
            log(monster ? `charger: spawned ${monster.id} near ${player.name}` : `charger: no room for a charger near ${player.name}`);
        }
    });
}
