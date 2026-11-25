// Engine enums: numeric, values from the engine, members only.

const unit: UnitType = UnitType.Monster;
const skill: number = SkillId.Teleport;
const flags: number = ItemFlag.Ethereal | ItemFlag.Identified;
function quality(q: ItemQuality): boolean {
    return q === ItemQuality.Unique || q >= ItemQuality.Rare;
}
void [unit, skill, flags, quality(ItemQuality.Set)];

// @ts-expect-error a member the enum does not have
void UnitType.Dragon;

// @ts-expect-error a sentinel the allow-list leaves out
void PlayerClass.Count;

// @ts-expect-error a string is not a UnitType
const s: UnitType = "player";

// @ts-expect-error a skill id is not a stat id
const stat: StatId = SkillId.Teleport;
void [s, stat];

// @ts-expect-error the data-table row enums are not exported (states come with the data stage)
void StateId.Poison;
