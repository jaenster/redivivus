// Engine enums: numeric, values from the engine, members only.
import { ItemFlag, ItemQuality, PlayerClassId, SkillId, StatId, UiFlag, UnitType } from "redivivus:enums";

const unit: UnitType = UnitType.Monster;
const skill: number = SkillId.Teleport;
const flags: number = ItemFlag.Ethereal | ItemFlag.Identified;
const panel: UiFlag = UiFlag.Inventory;
function quality(q: ItemQuality): boolean {
    return q === ItemQuality.Unique || q >= ItemQuality.Rare;
}
void [unit, skill, flags, panel, quality(ItemQuality.Set)];

// @ts-expect-error a member the enum does not have
void UnitType.Dragon;

// @ts-expect-error a sentinel the allow-list leaves out
void PlayerClassId.Count;

// @ts-expect-error a string is not a UnitType
const s: UnitType = "player";

// @ts-expect-error a skill id is not a stat id
const stat: StatId = SkillId.Teleport;
void [s, stat];
