// The charger: a monster that stops, winds up and rushes its target in a straight line, then hits
// it if the rush ended next to it. Written on the public API only: a script per monster, an action
// per yield*, and a bare yield to wait one think.
import { MonsterId, Outcome, StateId, StatId, subtile, UnitType, type Monster, type Player, type Think } from "redivivus:server";
import { guard, log, type Script } from "redivivus:shared";

/** How far a charger sees a player, in subtiles. */
const SIGHT = 30;
/** Close enough to hit. */
const MELEE = 3;
/** Closer than this it walks up instead of rushing. */
const CLOSE = 6;
/** Frames it stands still, glowing, before the rush. */
const WIND_UP = 20;
/** How far past the target's spot the rush carries on. */
const OVERSHOOT = 4;
/** Extra speed during the rush, in percent. */
const RUSH_SPEED = 100;

function nearestPlayer(monster: Monster): Player | undefined {
    let best: Player | undefined;
    let bestDistance = SIGHT + 1;
    for (const unit of monster.nearby(SIGHT, UnitType.Player)) {
        if (unit.type !== UnitType.Player || unit.dead) continue;
        const d = monster.distanceTo(unit);
        if (d < bestDistance) {
            best = unit;
            bestDistance = d;
        }
    }
    return best;
}

function outcomeName(o: Outcome): string {
    return o === Outcome.Done ? "done" : o === Outcome.Interrupted ? "interrupted" : o === Outcome.Blocked ? "blocked" : "timeout";
}

/** The AI: one coroutine per charger, from spawn (or revival) until it dies. */
export function* charger(monster: Monster, think: Think): Script<void> {
    void think;
    log(`charger ${monster.id}: awake at ${monster.position.x},${monster.position.y}`);
    while (true) {
        const target = nearestPlayer(monster);
        if (target === undefined) {
            yield* monster.idle(25);
            continue;
        }
        if (monster.distanceTo(target) <= MELEE) {
            const hit = yield* monster.attack(target);
            log(`charger ${monster.id}: attack ${target.name}: ${outcomeName(hit)}`);
            continue;
        }
        if (monster.distanceTo(target) <= CLOSE) {
            yield* monster.walkToward(target);
            continue;
        }

        // Wind up and rush, for as long as the target is there to rush at.
        monster.addState(StateId.Red, WIND_UP + 30);
        monster.addStat(StatId.Velocitypercent, RUSH_SPEED);
        const rush = yield* guard(() => target.valid && !target.dead, charge(monster, target));
        if (monster.valid) {
            monster.addStat(StatId.Velocitypercent, -RUSH_SPEED);
            monster.removeState(StateId.Red);
            log(`charger ${monster.id}: rush ${outcomeName(rush)}, now at ${monster.position.x},${monster.position.y}`);
        }
        // Catch its breath.
        yield* monster.idle(15);
    }
}

/**
 * Stand still for WIND_UP frames, then rush in a straight line from here through where the target
 * stands now, and a little past.
 */
function* charge(monster: Monster, target: Player): Script<Outcome> {
    log(`charger ${monster.id}: winding up, ${target.name} at ${Math.floor(monster.distanceTo(target))}`);
    yield* monster.idle(WIND_UP);
    const from = monster.position;
    const at = target.position;
    const length = Math.sqrt((at.x - from.x) ** 2 + (at.y - from.y) ** 2);
    if (length < 1) return Outcome.Blocked;
    const to = subtile(Math.floor(at.x + ((at.x - from.x) / length) * OVERSHOOT), Math.floor(at.y + ((at.y - from.y) / length) * OVERSHOOT));
    log(`charger ${monster.id}: rush from ${from.x},${from.y} to ${to.x},${to.y}`);
    const rush = yield* monster.runTo(to);
    if (rush !== Outcome.Blocked) return rush;
    return yield* monster.walkTo(to);  // a monster that cannot run walks it
}

/** The class that charges: the Fallen, whose AI the mod replaces in every game. */
export const CHARGER_CLASS = MonsterId.Fallen;
