// Mephisto's AI, first pass: AITHINK_Fn1_Mephisto (D2Game Monster/Ai/AI.cpp, 1.14d 005f78b0) ported
// mechanically. The C++ is one think per call with gotos and three words of state that outlive the
// call (dwArg0, dwArg1, dwArg2 of its D2AiGeneralStrc). Here the gotos are a switch in a loop
// (a label is a case, a goto sets the label and continues), and the three words are locals of the
// script that live as long as it does. One pass of the outer loop is one think; every way out of a
// think is an action, after which the engine thinks again.
//
// Kept only to show the step from the C++ to the composed version (mephisto.ts); the host test holds
// both equal to a replica of the C++, think by think.
import { Outcome, type Monster, type Think, type Unit } from "redivivus:server";
import type { Script } from "redivivus:shared";
import { alternateTarget, endOfThink, P, Slot, WALL_MASK } from "./common";

const enum Label {
    Top,
    StartAttack,
    Dispatch,
    Rotation,
    ApproachOrWander,
    WanderIfMoveFailed,
    RollPhase,
}

export function* mephistoSwitch(monster: Monster, think: Think): Script<void> {
    const row = monster.data;
    const aip = row.aiParams;
    const skills = row.skills;
    const modes = row.skillModes;
    const difficulty = monster.game.difficulty;

    // dwArg2: the state the next think starts from; dwArg0: casts left in a rotation; dwArg1: the
    // rotation's sub-phase (0: approach first, then a cast each think).
    let next = 0;
    let rotation = 0;
    let phase = 0;

    while (true) {
        const frame = monster.game.frame;
        const engaged = think.engaged;
        const distance = think.distance;
        let target: Unit = think.target!;
        const life = monster.lifePercent;
        let hurt = Math.trunc((100 - life) / 5);
        if (hurt < 0) hurt = 0;

        let state = next;
        let moved = false;
        let label = Label.Top;
        let thinking = true;
        while (thinking) {
            switch (label) {
                case Label.Top: {
                    if (!engaged) {
                        if (distance > 20) {
                            state = 4;
                            label = Label.Dispatch;
                            continue;
                        }
                        if (state === 0) {
                            if (life <= 20 && distance < 5 && monster.random(100) < 40) {
                                state = 1;
                                label = Label.Dispatch;
                                continue;
                            }
                            if (monster.random(100) < hurt + 50) {
                                label = Label.StartAttack;
                                continue;
                            }
                        } else if (state <= 4) {
                            label = Label.Dispatch;
                            continue;
                        }
                        state = 0;
                        if (monster.random(100) < 65) {
                            if (monster.random(100) >= 65 && distance <= 5) {
                                monster.setPathOverrides(0, 0, 4);
                                yield* monster.walkToward(target, 7);
                            } else {
                                yield* monster.circleToward(target, 4);
                            }
                            next = 0;
                            thinking = false;
                            continue;
                        }
                        yield* monster.idle(10);
                        next = state;
                        thinking = false;
                        continue;
                    }
                    if (monster.random(100) > aip[P.SkillChance]) {
                        state = 3;
                        label = Label.Dispatch;
                        continue;
                    }
                    label = Label.StartAttack;
                    continue;
                }
                case Label.StartAttack: {
                    rotation = monster.random(3) + 3;
                    state = 2;
                    label = Label.Dispatch;
                    continue;
                }
                case Label.Dispatch: {
                    if (state === 0) {
                        yield* monster.idle(5);
                        next = 4;
                        thinking = false;
                        continue;
                    }
                    if (state === 1) {
                        state = 0;
                        monster.setPathOverrides(0, 50, 0);
                        if ((yield* monster.walkPast(target, 8, true)) !== Outcome.Blocked) {
                            next = state;
                            thinking = false;
                            continue;
                        }
                        if (engaged) {
                            yield* monster.castSkill(skills[Slot.Close], target, modes[Slot.Close]);
                            next = 0;
                            thinking = false;
                            continue;
                        }
                        label = Label.Rotation;
                        continue;
                    }
                    if (state === 3) {
                        state = 0;
                        if (monster.random(100) < hurt + 80) {
                            if (monster.random(100) < 80 - hurt) {
                                yield* monster.attack(target);
                            } else {
                                yield* monster.castSkill(skills[Slot.Close], target, modes[Slot.Close]);
                            }
                            next = 0;
                            thinking = false;
                            continue;
                        }
                        monster.setPathOverrides(0, 50, 0);
                        label = Label.ApproachOrWander;
                        continue;
                    }
                    if (state === 4) {
                        state = 0;
                        monster.setPathOverrides(0, 50, 0);
                        if ((yield* monster.wanderAround(target, 6)) !== Outcome.Blocked) {
                            next = state;
                            thinking = false;
                            continue;
                        }
                        moved = (yield* monster.walkToDistance(target, 12, 6)) !== Outcome.Blocked;
                        label = Label.WanderIfMoveFailed;
                        continue;
                    }
                    label = Label.Rotation;
                    continue;
                }
                case Label.Rotation: {
                    rotation--;
                    if (rotation === 0) state = 0;
                    if (phase === 0) {
                        phase = 2;
                        label = Label.ApproachOrWander;
                        continue;
                    }
                    phase++;
                    target = alternateTarget(monster, aip, target);
                    let count = 0;
                    while (count < 8 && skills[count] >= 0) count++;
                    if (count > 0) {
                        const share = Math.trunc(100 / count);
                        const roll = monster.random(100);
                        if (difficulty > 0 && (monster.collidesWith(target, WALL_MASK) || distance > 30)) {
                            yield* monster.castSkill(skills[Slot.Escape], target, modes[Slot.Escape]);
                        } else if (distance < 15 && roll < share && difficulty > 0) {
                            yield* monster.castSkill(skills[Slot.Near], target, modes[Slot.Near]);
                        } else if (roll < share * 2) {
                            yield* monster.castSkill(skills[Slot.Third], target, modes[Slot.Third]);
                        } else if (roll < share * 3) {
                            yield* monster.castSkill(skills[Slot.Second], target, modes[Slot.Second]);
                        } else {
                            yield* monster.castSkill(skills[Slot.First], target, modes[Slot.First]);
                        }
                    } else {
                        yield* monster.wander(6);
                    }
                    label = Label.RollPhase;
                    continue;
                }
                case Label.ApproachOrWander: {
                    moved = (yield* monster.circleToward(target, 3, true)) !== Outcome.Blocked;
                    label = Label.WanderIfMoveFailed;
                    continue;
                }
                case Label.WanderIfMoveFailed: {
                    if (!moved) yield* monster.wander(12);
                    next = state;
                    thinking = false;
                    continue;
                }
                case Label.RollPhase: {
                    if (monster.random(100) < 50 - hurt) phase = 0;
                    next = state;
                    thinking = false;
                    continue;
                }
            }
        }
        yield* endOfThink(monster, frame);
    }
}
