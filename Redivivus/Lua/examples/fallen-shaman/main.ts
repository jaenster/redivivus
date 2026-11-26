// Fallen Shaman: the stock AI of the Fallen Shaman, translated to TypeScript (shaman.ts), runs
// every shaman in the game. With the stock setting the game's own AI stays and is only wrapped, so
// the two can be compared in the log; with demo a shaman and its Fallen are spawned near a player
// out of town and one Fallen is killed, so a resurrect can be watched without a fight.
import { AiTargeting, Event, MonsterAiId, MonsterId, MonsterMode, subtile, UnitType, type Game, type Monster } from "redivivus:server";
import { log } from "redivivus:shared";
import { options } from "./options";
import { fallenShaman, setTrace } from "./shaman";

/** Demo: spawn check interval, in ticks (25 a second). */
const CHECK_EVERY = 50;
/** Demo: how far east of the player the group is spawned. */
const SPAWN_OFFSET = 14;
/** Demo: ticks after the spawn at which one of its Fallen is killed. */
const KILL_AFTER = 75;

function modeName(m: MonsterMode): string {
    switch (m) {
        case MonsterMode.Sequence: return "sequence";
        case MonsterMode.Attack1: return "attack";
        case MonsterMode.Walk: return "walk";
        case MonsterMode.Neutral: return "neutral";
        case MonsterMode.Cast: return "cast";
        default: return `mode ${m}`;
    }
}

export function server(game: Game) {
    setTrace(options.trace);
    if (options.stock) {
        game.registry.ai.wrap(MonsterAiId.FallenShaman, (next, monster) => {
            next();
            if (options.trace && monster.valid) {
                const t = monster.target;
                const at = t === undefined || !t.valid ? "" : ` at ${t.id}`;
                log(`stock shaman ${monster.id}: ${modeName(monster.mode)}${at}`);
            }
        });
        log(`fallen-shaman: the stock AI runs the shamans in game ${game.id} (wrapped to log)`);
    } else {
        game.registry.ai.register(MonsterAiId.FallenShaman, fallenShaman, AiTargeting.Basic);
        log(`fallen-shaman: the TypeScript AI runs the shamans in game ${game.id}`);
    }
    if (!options.demo) return;

    let spawned: Monster | undefined;
    let spawnedAt = 0;
    let killed = false;
    game.on(Event.Tick, (e) => {
        if (spawned !== undefined && !killed && e.tick - spawnedAt >= KILL_AFTER) {
            killed = true;
            const shaman = spawned;
            if (!shaman.valid) return;
            for (const fallen of shaman.minions()) {
                if (fallen.classId !== MonsterId.Fallen) continue;
                log(`fallen-shaman: killing Fallen ${fallen.id} of shaman ${shaman.id} at ${fallen.position.x},${fallen.position.y}`);
                fallen.kill();
                return;
            }
            log(`fallen-shaman: shaman ${shaman.id} has no living Fallen to kill`);
            return;
        }
        if (spawned !== undefined || e.tick % CHECK_EVERY !== 0) return;
        for (const player of game.players()) {
            if (player.dead || player.level === undefined || player.level.isTown) continue;
            const at = subtile(player.position.x + SPAWN_OFFSET, player.position.y);
            const shaman = game.spawnMonster(MonsterId.FallenShaman, at, { radius: 8, minions: true });
            if (shaman === undefined) continue;
            let group = 0;
            for (const unit of shaman.nearby(12, UnitType.Monster)) if (unit.type === UnitType.Monster && unit.classId === MonsterId.Fallen) group++;
            log(`fallen-shaman: spawned shaman ${shaman.id} with ${group} Fallen near ${player.name}`);
            spawned = shaman;
            spawnedAt = e.tick;
            return;
        }
    });
}
