// What both versions of Mephisto's AI share: the meaning of its MonStats columns and the target
// switch the C++ makes in its own function (DecideMephistoAlternateTarget, 1.14d 005f77c0).
import type { Monster, Unit } from "redivivus:server";
import type { Script } from "redivivus:shared";

/** The AI parameters of its row (aiParams[i] is aip(i+1)). */
export const enum P {
    /** Engaged: the chance, in percent, to go on casting rather than fight in melee. */
    SkillChance = 0,
    /** The chance to turn a cast on the weakest unit around. */
    WeakestChance = 1,
    /** The chance to turn it on the nearest (the scan never fills that one for a monster). */
    NearestChance = 2,
}

/** The skill slots of its row, by what the AI does with them. */
export const enum Slot {
    /** Skill1..Skill4: the rotation's casts, Skill4 twice as likely as the others below it. */
    First = 0,
    Second = 1,
    /** Skill3: what it casts at a target in melee range instead of hitting it. */
    Close = 2,
    Third = 3,
    /** Skill5: in Nightmare and Hell, a cast it favours at a target nearer than 15. */
    Near = 4,
    /** Skill6: in Nightmare and Hell, what it casts at a target behind a wall or farther than 30. */
    Escape = 5,
}

/** The collision bits TestCollisionWithUnit is asked about (a wall between the two). */
export const WALL_MASK = 4;

/**
 * Turn the cast on another unit, by chance: the weakest around it, or (never, for a monster) the
 * nearest. Both rolls are the engine's, in its order; the second returns the weakest as the C++ does.
 */
export function alternateTarget(monster: Monster, aip: readonly number[], fallback: Unit): Unit {
    const scan = monster.scanTargets();
    if (scan.weakest !== undefined && monster.random(100) < aip[P.WeakestChance]) return scan.weakest;
    if (scan.nearestWhenPlayer !== undefined && monster.random(100) < aip[P.NearestChance]) return scan.weakest ?? fallback;
    return fallback;
}

/** The life bands: how many fifths of its life it has lost, as the C++ rounds. */
export function hurtOf(monster: Monster): number {
    const hurt = Math.trunc((100 - monster.lifePercent) / 5);
    return hurt < 0 ? 0 : hurt;
}

/**
 * If a think started nothing (every action it tried could not start), stand for its AI delay: what
 * the mod host does for any script think that arms nothing. The C++ leaves such a think to the
 * engine; without this the script would decide again at once, in the same think.
 */
export function* endOfThink(monster: Monster, frame: number): Script<void> {
    if (monster.game.frame === frame) yield* monster.idle();
}
