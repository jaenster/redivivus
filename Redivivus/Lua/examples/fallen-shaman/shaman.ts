// The Fallen Shaman's AI, translated from the game's own (AITHINK_Fn1_FallenShaman, D2Game
// Monster/Ai/AI.cpp, 1.14d 005f1440) onto the public API: the same decisions in the same order,
// the same rolls of the monster's own seed, the parameters read from its MonStats row.
//
// The stock AI is a function the engine calls once per think; it decides and returns after one
// action. Here one pass of the loop is one think: every branch ends in exactly one yield* (the
// action the stock AI starts there), and the loop goes round when the engine thinks again.
import { Alignment, MinionOrder, MonsterId, MonsterMode, StateMask, type Corpse, type Monster, type Think } from "redivivus:server";
import { log, type Script } from "redivivus:shared";

/** What the AI parameters of its row mean (monai.txt FallenShaman; aiParams[i] is aip(i+1)). */
const enum Param {
    /** Chance in percent to order the group to attack, and (again) to resurrect. */
    ResurrectChance = 0,
    /** Chance to throw fire at a target close enough. */
    ShootChance = 1,
    /** Chance to hit back when engaged, and to circle in. */
    AttackChance = 2,
    /** How far it looks for a corpse to raise. */
    ResurrectRadius = 3,
    /** How close a target must be for fire. */
    ShootRange = 4,
}

/** The skill slots of its row: Skill1 raises, Skill2 burns. */
const RESURRECT_SLOT = 0;
const FIRE_SLOT = 1;
/** The stock AI's idle when nothing else fires, in frames. */
const IDLE_FRAMES = 10;
/** How many steps the curve in toward the target has. */
const CIRCLE_STEPS = 3;

/** Whether to say in the log what each think decided (off: the AI is silent). */
let trace = true;
export function setTrace(on: boolean): void {
    trace = on;
}

function note(monster: Monster, text: string): void {
    if (trace) log(`shaman ${monster.id}: ${text}`);
}

/** A roll as the stock AI makes it: the monster's seed, modulo 100, under a chance in percent. */
function chance(monster: Monster, percent: number): boolean {
    return monster.random(100) < percent;
}

/**
 * The corpse to raise, as the engine's two scans pick it: the LAST match, not the nearest (the
 * callbacks overwrite their result and never stop the walk).
 */
function findCorpse(monster: Monster, radius: number): Corpse | undefined {
    let found: Corpse | undefined;
    if (!monster.unique || monster.champion) {
        // ForEach entry 9 (AI_ForEachFn09_FindOwnMinionCorpse): the dead of the group it leads. The
        // engine compares its weighted, unsquared AI distance against radius squared, so this reach
        // is all but unlimited; kept as the engine has it.
        const leader = monster.leader;
        if (leader === undefined || !leader.valid) return undefined;
        for (const corpse of leader.minionCorpses()) {
            if (monster.aiDistanceTo(corpse) <= radius * radius && corpse.mode === MonsterMode.Dead && corpse.targetable
                && !corpse.hasStateMask(StateMask.Udead) && corpse.leader === monster) {
                found = corpse;
            }
        }
    } else {
        // A unique shaman (MONSTERAI_FindFallenForRevival): any Fallen or Shaman corpse around it.
        for (const corpse of monster.corpsesNear(radius)) {
            if ((corpse.baseId === MonsterId.Fallen || corpse.baseId === MonsterId.FallenShaman) && corpse.alignment === Alignment.Evil
                && corpse.targetable && !corpse.hasStateMask(StateMask.Udead) && corpse.mode === MonsterMode.Dead
                && !corpse.hasStateMask(StateMask.Hide) && !corpse.unique && !corpse.champion) {
                found = corpse;
            }
        }
    }
    return found;
}

/** The AI. Registered with AiTargeting.Basic, so it only ever thinks with a target. */
export function* fallenShaman(monster: Monster, think: Think): Script<void> {
    const row = monster.data;
    const p = row.aiParams;
    const resurrect = row.skills[RESURRECT_SLOT];
    const fire = row.skills[FIRE_SLOT];
    const fireMode = row.skillModes[FIRE_SLOT];
    note(monster, `awake (aip ${p[0]} ${p[1]} ${p[2]} ${p[3]} ${p[4]}, skills ${resurrect} ${fire})`);

    while (true) {
        const target = think.target!;

        // In melee range: hit back, by chance.
        if (think.engaged && chance(monster, p[Param.AttackChance])) {
            note(monster, "attacks");
            yield* monster.attack(target);
            continue;
        }

        const corpse = findCorpse(monster, p[Param.ResurrectRadius]);

        // By chance, order the group to attack (the roll is made every think).
        if (chance(monster, p[Param.ResurrectChance])) monster.orderMinions(MinionOrder.Attack);

        // A corpse found: raise it, by chance, if the skill can reach it.
        if (corpse !== undefined && chance(monster, p[Param.ResurrectChance]) && monster.canCast(resurrect, corpse)) {
            note(monster, `resurrects ${corpse.id}`);
            yield* monster.castSequence(resurrect, corpse);
            continue;
        }

        // Its target close enough: fire, by chance.
        if (think.distance < p[Param.ShootRange] && chance(monster, p[Param.ShootChance])) {
            note(monster, `fire at ${think.distance}`);
            yield* monster.castSkill(fire, target, fireMode);
            continue;
        }

        // Another target close enough: fire at that one, by chance.
        const other = monster.findTarget();
        if (other !== undefined && other.distance < p[Param.ShootRange] && chance(monster, p[Param.ShootChance])) {
            note(monster, `fire at another at ${other.distance}`);
            yield* monster.castSkill(fire, other.target, fireMode);
            continue;
        }

        // Close in along a curve, by chance; otherwise stand a moment.
        if (chance(monster, p[Param.AttackChance])) {
            yield* monster.circleToward(target, CIRCLE_STEPS);
            continue;
        }
        yield* monster.idle(IDLE_FRAMES);
    }
}
