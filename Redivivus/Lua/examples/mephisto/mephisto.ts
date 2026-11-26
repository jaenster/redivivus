// Mephisto's AI (AITHINK_Fn1_Mephisto, D2Game Monster/Ai/AI.cpp, 1.14d 005f78b0), composed of small
// actions: each helper below is a Script that takes the monster (and what it needs of the think),
// runs one or more thinks and hands back what happened. Same decisions, same rolls of the monster's
// seed in the same order, the parameters read from its MonStats row; the host test holds it equal to
// a replica of the C++ and to the mechanical port (switch.ts), think by think.
//
// Where the C++ state went:
//   dwArg2 (the state the next think starts from) is where the script is: inside rotation() or not.
//     Its other values are never stored (0 and 4 only lead into each other through a case that
//     nothing reaches; 1 and 3 are reset in the think that sets them).
//   dwArg0 (casts left in a rotation) is a local of rotation().
//   dwArg1 (the rotation's cadence: approach first, then cast) outlives a rotation: a rotation that
//     starts while the last one's cadence is under way casts at once. That is history, not a place
//     in the program, so it stays a variable (Memory.cadence).
import { Outcome, type Monster, type Think, type Unit } from "redivivus:server";
import { guard, type Script } from "redivivus:shared";
import { alternateTarget, endOfThink, hurtOf, P, Slot, WALL_MASK } from "./common";

/** What the AI remembers across rotations (dwArg1): 0 approach first, else cast each think. */
export interface Memory {
    cadence: number;
}

/** The rotation goes on while the target is not in melee range and not farther than this. */
const ROTATION_REACH = 20;

/** Too far off: wander near the target, or walk to 6 from it, or wander where it stands (case 4). */
export function* reposition(monster: Monster, target: Unit): Script<void> {
    monster.setPathOverrides(0, 50, 0);
    if ((yield* monster.wanderAround(target, 6)) !== Outcome.Blocked) return;
    if ((yield* monster.walkToDistance(target, 12, 6)) !== Outcome.Blocked) return;
    yield* monster.wander(12);
}

/** Close in along a curve, or wander where it stands when that cannot start (approach_or_wander). */
export function* closeIn(monster: Monster, target: Unit): Script<void> {
    if ((yield* monster.circleToward(target, 3, true)) !== Outcome.Blocked) return;
    yield* monster.wander(12);
}

/** In melee range and not casting: hit, or cast Skill3, or close in (case 3). */
export function* melee(monster: Monster, target: Unit, hurt: number): Script<void> {
    const row = monster.data;
    if (monster.random(100) < hurt + 80) {
        if (monster.random(100) < 80 - hurt) yield* monster.attack(target);
        else yield* monster.castSkill(row.skills[Slot.Close], target, row.skillModes[Slot.Close]);
        return;
    }
    monster.setPathOverrides(0, 50, 0);
    yield* closeIn(monster, target);
}

/** A cast the rotation chose: the skill slot and whom at (none: no skills, it wanders). */
export interface Pick {
    readonly slot?: Slot;
    readonly at: Unit;
}

/** Choose the rotation's cast, at the target or one it switches to (the end of the C++; its rolls). */
export function chooseCast(monster: Monster, target: Unit, distance: number): Pick {
    const row = monster.data;
    const at = alternateTarget(monster, row.aiParams, target);
    let count = 0;
    while (count < 8 && row.skills[count] >= 0) count++;
    if (count === 0) return { at };
    const share = Math.trunc(100 / count);
    const roll = monster.random(100);
    const hard = monster.game.difficulty > 0;
    if (hard && (monster.collidesWith(at, WALL_MASK) || distance > 30)) return { slot: Slot.Escape, at };
    if (distance < 15 && roll < share && hard) return { slot: Slot.Near, at };
    if (roll < share * 2) return { slot: Slot.Third, at };
    if (roll < share * 3) return { slot: Slot.Second, at };
    return { slot: Slot.First, at };
}

/** Cast what was chosen (or wander when it has no skills). */
export function* castPick(monster: Monster, pick: Pick): Script<void> {
    if (pick.slot === undefined) {
        yield* monster.wander(6);
        return;
    }
    const row = monster.data;
    yield* monster.castSkill(row.skills[pick.slot], pick.at, row.skillModes[pick.slot]);
}

/** One think of a rotation: approach first in a new cadence, then a cast each think. */
export function* rotationStep(monster: Monster, think: Think, memory: Memory, hurt: number): Script<void> {
    if (memory.cadence === 0) {
        memory.cadence = 2;
        yield* closeIn(monster, think.target!);
        return;
    }
    memory.cadence++;
    const pick = chooseCast(monster, think.target!, think.distance);
    // The C++ makes this roll after starting the cast. A step ends with its action, and a guard may
    // drop the rotation before the next think, so the roll comes first here: the same order unless
    // the cast's own start rolls the monster's seed.
    if (monster.random(100) < 50 - hurt) memory.cadence = 0;
    yield* castPick(monster, pick);
}

function* rotationSteps(monster: Monster, think: Think, memory: Memory, steps: number): Script<void> {
    for (let i = 0; i < steps; i++) {
        const frame = monster.game.frame;
        yield* rotationStep(monster, think, memory, hurtOf(monster));
        yield* endOfThink(monster, frame);
    }
}

/**
 * A rotation of 3 to 5 thinks (dwArg0): the first in the think it starts in, the rest while the
 * target stays out of melee range and within reach; otherwise the next think decides anew.
 */
export function* rotation(monster: Monster, think: Think, memory: Memory): Script<void> {
    const steps = monster.random(3) + 3;
    const frame = monster.game.frame;
    yield* rotationStep(monster, think, memory, hurtOf(monster));
    yield* endOfThink(monster, frame);
    yield* guard(() => !think.engaged && think.distance <= ROTATION_REACH, rotationSteps(monster, think, memory, steps - 1));
}

/** Badly hurt and close: walk on past the target, or, when that cannot start, a rotation step (case 1). */
export function* retreat(monster: Monster, think: Think, memory: Memory, hurt: number): Script<void> {
    monster.setPathOverrides(0, 50, 0);
    if ((yield* monster.walkPast(think.target!, 8, true)) !== Outcome.Blocked) return;
    yield* rotationStep(monster, think, memory, hurt);
}

/** Nothing better to do: walk up to the target or close in, or stand a moment. */
export function* roam(monster: Monster, target: Unit, distance: number): Script<void> {
    if (monster.random(100) >= 65) {
        yield* monster.idle(10);
        return;
    }
    if (monster.random(100) >= 65 && distance <= 5) {
        monster.setPathOverrides(0, 0, 4);
        yield* monster.walkToward(target, 7);
        return;
    }
    yield* monster.circleToward(target, 4);
}

/** One decision from scratch: what the C++ does when it has no rotation running. */
export function* decide(monster: Monster, think: Think, memory: Memory): Script<void> {
    const target = think.target!;
    const distance = think.distance;
    const hurt = hurtOf(monster);
    if (think.engaged) {
        if (monster.random(100) > monster.data.aiParams[P.SkillChance]) yield* melee(monster, target, hurt);
        else yield* rotation(monster, think, memory);
        return;
    }
    if (distance > ROTATION_REACH) {
        yield* reposition(monster, target);
        return;
    }
    if (monster.lifePercent <= 20 && distance < 5 && monster.random(100) < 40) {
        yield* retreat(monster, think, memory, hurt);
        return;
    }
    if (monster.random(100) < hurt + 50) {
        yield* rotation(monster, think, memory);
        return;
    }
    yield* roam(monster, target, distance);
}

/** The AI. Registered with AiTargeting.Basic, so it only ever thinks with a target. */
export function* mephisto(monster: Monster, think: Think): Script<void> {
    const memory: Memory = { cadence: 0 };
    while (true) {
        const frame = monster.game.frame;
        yield* decide(monster, think, memory);
        yield* endOfThink(monster, frame);
    }
}
