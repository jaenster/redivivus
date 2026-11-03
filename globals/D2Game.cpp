/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "D2Client/UNIT/Missile.h"
#include "D2Client/UNIT/Object.h"
#include "D2Client/UNIT/Player.h"
#include "D2Game/D2QuestDataStrc.h"
#include "D2Game/GAME/Event/Dispatch/Monster.h"
#include "D2Game/GAME/Task.h"
#include "D2Game/ITEMS/ItemMode.h"
#include "D2Game/Monster/AI/AI.h"
#include "D2Game/Monster/MonsterMode.h"
#include "D2Game/Monster/Monsters.h"
#include "D2Game/Objects/ObjectsMode.h"
#include "D2Game/PLAYER/PlayerModes.h"
#include "D2Game/PLAYER/PlayerTrade.h"
#include "D2Game/Quests/QUESTS.h"
#include "D2Game/SKILLS/SkillsServer.h"
#include "D2Game/UNIT/sunitproxy.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2PlayerModeFnPairStrc gaPlayerModeMoveFuncs[20] = {
    {
        /* .pfnMoveToLocation = */ nullptr,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_Death
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetIdleMode,
        /* .pfnStartMode = */ nullptr
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetMoveTargetPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetTargetAndStartPath
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetMoveTargetPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetTargetAndStartPath
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_ChangeAnimModeAndResetSkill,
        /* .pfnStartMode = */ nullptr
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetIdleMode,
        /* .pfnStartMode = */ nullptr
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetMoveTargetPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetTargetAndStartPath
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartModeWithTarget,
        /* .pfnStartMode = */ nullptr
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_HandlePlayerDeath,
        /* .pfnStartMode = */ nullptr
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_StartCastAtEntity
    },
    {
        /* .pfnMoveToLocation = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetMoveTargetPosition,
        /* .pfnStartMode = */ &D2Game::PLAYER::PlayerModes::PLRMODES_SetTargetAndStartPath
    }
}; D2_GLOBAL_GUARD(gaPlayerModeMoveFuncs, "gaPlayerModeMoveFuncs");

D2ShrinesTableEntryStrc OBJECTSHRINEFUNCTION[24] = {
    {
        /* .fpShrineHandler = */ nullptr,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_RefillFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_HealthBoostFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_ManaBoostFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_HealthExchangeFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_ManaExchangeFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StatBoostFunction,
        /* .eStat = */ UNITSTAT_skill_armor_percent,
        /* .nState = */ STATE_shrine_armor
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_CombatBoostFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_shrine_combat
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StatBoostFunction,
        /* .eStat = */ UNITSTAT_fireresist,
        /* .nState = */ STATE_shrine_resist_fire
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StatBoostFunction,
        /* .eStat = */ UNITSTAT_coldresist,
        /* .nState = */ STATE_shrine_resist_cold
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StatBoostFunction,
        /* .eStat = */ UNITSTAT_lightresist,
        /* .nState = */ STATE_shrine_resist_lightning
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StatBoostFunction,
        /* .eStat = */ UNITSTAT_poisonresist,
        /* .nState = */ STATE_shrine_resist_poison
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_SkillBoostFunction,
        /* .eStat = */ UNITSTAT_maxstamina,
        /* .nState = */ STATE_shrine_skill
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StatBoostFunction,
        /* .eStat = */ UNITSTAT_manarecoverybonus,
        /* .nState = */ STATE_shrine_mana_regen
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StaminaBoostFunction,
        /* .eStat = */ UNITSTAT_skill_staminapercent,
        /* .nState = */ STATE_shrine_stamina
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StatBoostFunction,
        /* .eStat = */ UNITSTAT_item_addexperience,
        /* .nState = */ STATE_shrine_experience
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_ShrineOfEnirhsFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_PortalShrineFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_GemShrineFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_StormShrineFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_WarpingShrineFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_ExplodingShrineFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ &D2Game::Objects::ObjectsMode::Shrines_PoisonShrineFunction,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    },
    {
        /* .fpShrineHandler = */ nullptr,
        /* .eStat = */ (eD2UnitStat)-1,
        /* .nState = */ STATE_none
    }
}; D2_GLOBAL_GUARD(OBJECTSHRINEFUNCTION, "OBJECTSHRINEFUNCTION");

D2TaskQueueSlotStrc * gaTaskQueueHeadArray[32] = {}; D2_GLOBAL_GUARD(gaTaskQueueHeadArray, "gaTaskQueueHeadArray");

// =============================================================================
// Initialized scalars
// =============================================================================

BOOL DontNoSafeGameBool = 0x1; D2_GLOBAL_GUARD(DontNoSafeGameBool, "DontNoSafeGameBool");
uint8_t gbQuestReconnectMode = 0x1; D2_GLOBAL_GUARD(gbQuestReconnectMode, "gbQuestReconnectMode");
uint32_t gptItemCodeArmor = 0; D2_GLOBAL_GUARD(gptItemCodeArmor, "gptItemCodeArmor");
uint32_t gptItemCodeSwordType = 0; D2_GLOBAL_GUARD(gptItemCodeSwordType, "gptItemCodeSwordType");
int gQServerClientTablesInitialized = 0; D2_GLOBAL_GUARD(gQServerClientTablesInitialized, "gQServerClientTablesInitialized");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace D2Client::Warden::NWardenClient {

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

} // namespace D2Client::Warden

namespace D2Client::UI {

} // namespace D2Client::UI

namespace D2Client::UI::Button::ButtonImplementation {

} // namespace D2Client::UI::Button::ButtonImplementation

namespace D2Client::UI::Button::ButtonWrapper {

} // namespace D2Client::UI::Button::ButtonWrapper

namespace D2Client::UI::Editbox::EditBoxImplementation {

} // namespace D2Client::UI::Editbox::EditBoxImplementation

namespace D2Client::UI::Editbox::EditBoxWrapper {

} // namespace D2Client::UI::Editbox::EditBoxWrapper

namespace D2Client::UI::Spinner::SpinnerImplementation {

} // namespace D2Client::UI::Spinner::SpinnerImplementation

namespace D2Client::UI::Spinner::SpinnerWrapper {

} // namespace D2Client::UI::Spinner::SpinnerWrapper

namespace D2Client::UI::ListBox::ListBoxWrapper {

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

} // namespace D2Client::UI::ListBox::ListBoxImplementation

namespace D2Game::GAME::Event::Dispatch {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2TimerTypeMethodsStrc pTimerMethods = {
    /* .fpFunctionByType = */ {
        nullptr,
        nullptr,
        nullptr,
        &D2Game::ITEMS::ItemMode::ITEMMODE_ValidateTradeItem,
        &D2Game::ITEMS::ItemMode::ITEMMODE_NullSub,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &D2Game::ITEMS::ItemMode::ITEMMODE_RemoveExpiredStatLists,
        nullptr,
        nullptr
    }
}; D2_GLOBAL_GUARD(pTimerMethods, "D2Game::GAME::Event::Dispatch::pTimerMethods");

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::GAME::Event::Dispatch::Player {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2TimerTypeMethodsExStrc pTimerMethods = {
    /* .fpFunctionExByType = */ {
        (fpTimerFunctionEx)&D2Game::PLAYER::PlayerModes::PLRMODES_ProcessPlayerMode,
        (fpTimerFunctionEx)&D2Game::PLAYER::PlayerModes::PLRMODES_OnModeComplete,
        nullptr,
        (fpTimerFunctionEx)&D2Game::PLAYER::PlayerModes::PLRMODES_PeriodicStatRegen,
        nullptr,
        (fpTimerFunctionEx)&D2Game::SKILLS::SkillsServer::SKILLS_DispatchStateActiveFunc,
        (fpTimerFunctionEx)&D2Game::PLAYER::PlayerModes::HoverTextEvent,
        nullptr,
        (fpTimerFunctionEx)&D2Game::SKILLS::SkillsServer::SKILLS_PeriodicSkillsCallback,
        (fpTimerFunctionEx)&D2Game::SKILLS::SkillsServer::SKILLS_PeriodicStatsCallback,
        nullptr,
        (fpTimerFunctionEx)&D2Game::PLAYER::PlayerModes::PLRMODES_BroadcastAllyAndPartyInfo,
        (fpTimerFunctionEx)&D2Game::PLAYER::PlayerModes::PLRMODES_RemoveExpiredStatLists,
        (fpTimerFunctionEx)&D2Game::PLAYER::PlayerTrade::PLRTRADE_ProcessTradeStateTick,
        nullptr
    }
}; D2_GLOBAL_GUARD(pTimerMethods, "D2Game::GAME::Event::Dispatch::Player::pTimerMethods");

} // namespace D2Game::GAME::Event::Dispatch::Player

namespace D2Game::GAME::Event::Dispatch::Object {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2TimerTypeMethodsStrc pTimerMethods = {
    /* .fpFunctionByType = */ {
        &D2Game::Objects::ObjectsMode::OBJMODE_UpdateTrapObject,
        &D2Game::Objects::ObjectsMode::OBJMODE_OpenObject,
        &D2Game::Objects::ObjectsMode::OBJMODE_UpdateFillProgress,
        &D2Game::Objects::ObjectsMode::OBJMODE_UpdateProximityStateMachine,
        &D2Game::Objects::ObjectsMode::OBJMODE_DispatchChestType,
        &D2Game::Objects::ObjectsMode::OBJMODE_DeactivateShrine,
        &D2Game::Objects::ObjectsMode::OBJMODE_UpdateHoverTextExpiry,
        &D2Game::Objects::ObjectsMode::QUEST_DispatchObjectActivation,
        &D2Game::Objects::ObjectsMode::OBJMODE_UpdateDayNightState,
        &D2Game::Objects::ObjectsMode::KillUnit,
        &D2Game::Objects::ObjectsMode::OBJMODE_RollChestLoot,
        &D2Game::Objects::ObjectsMode::OBJMODE_CreateRedPortal,
        nullptr,
        nullptr,
        nullptr
    }
}; D2_GLOBAL_GUARD(pTimerMethods, "D2Game::GAME::Event::Dispatch::Object::pTimerMethods");

} // namespace D2Game::GAME::Event::Dispatch::Object

namespace D2Game::GAME::Event::Dispatch::Monster {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2TimerTypeMethodsExStrc pTimerMethods = {
    /* .fpFunctionExByType = */ {
        (fpTimerFunctionEx)&D2Game::Monster::MonsterMode::MONMODE_DispatchDeathDrop,
        (fpTimerFunctionEx)&D2Game::Monster::MonsterMode::ExecuteDropHandlerFn3,
        (fpTimerFunctionEx)&D2Game::Monster::AI::MONSTERAI_ExecuteAiFunction,
        (fpTimerFunctionEx)&D2Game::Monster::MonsterMode::RegenAndPsnDecreaser,
        nullptr,
        (fpTimerFunctionEx)&D2Game::SKILLS::SkillsServer::SKILLS_DispatchStateActiveFunc,
        (fpTimerFunctionEx)&D2Game::GAME::Event::Dispatch::Monster::MONMODE_UpdateHoverTextExpiry,
        (fpTimerFunctionEx)&D2Game::Monsters::MONSTER_DispatchUModOnHit,
        (fpTimerFunctionEx)&D2Game::SKILLS::SkillsServer::SKILLS_PeriodicSkillsCallback,
        (fpTimerFunctionEx)&D2Game::SKILLS::SkillsServer::SKILLS_PeriodicStatsCallback,
        (fpTimerFunctionEx)&D2Game::GAME::Event::Dispatch::Monster::AI_ClearSwitchAiTargetParamsCallback,
        nullptr,
        (fpTimerFunctionEx)&D2Game::GAME::Event::Dispatch::Monster::RemoveStatListFromUnit,
        nullptr,
        nullptr
    }
}; D2_GLOBAL_GUARD(pTimerMethods, "D2Game::GAME::Event::Dispatch::Monster::pTimerMethods");

} // namespace D2Game::GAME::Event::Dispatch::Monster

namespace D2Common::Chat::Ignorelist::ChatIgnoreList {

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common::Drlg::Outdoors {

} // namespace D2Common::Drlg::Outdoors

namespace Define_the_symbol__ATL_MIXED {

} // namespace Define_the_symbol__ATL_MIXED

namespace Fog::_3rdparty::crashy {

} // namespace Fog::_3rdparty::crashy

namespace Storm::Source::SFile {

} // namespace Storm::Source::SFile

namespace Class::FILEREC {

} // namespace Class::FILEREC

namespace Class::EVENTREC {

} // namespace Class::EVENTREC

namespace Class::REQUEST {

} // namespace Class::REQUEST

namespace Class::AUDIOSTREAM {

} // namespace Class::AUDIOSTREAM

namespace Class::HANDLER {

} // namespace Class::HANDLER

namespace D2Client::CharScreen {

} // namespace D2Client::CharScreen

namespace D2Client::UI::ui {

} // namespace D2Client::UI::ui

namespace D2Game::Quests::QUESTS::Quest40 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[4] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e2e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e45,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e55,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e61,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e23,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e2e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e47,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e56,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e63,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e23,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e2e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e46,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e57,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e62,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e23,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::Quest40::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::Quest40

namespace D2Game::Quests::QUESTS::A5Q1 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[8] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e6d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e6e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e6f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e70,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e71,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e72,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e73,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e74,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e75,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e76,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e77,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e78,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e79,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e7a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e7b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e7c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e7d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e7e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e7f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e7a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e7b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e7c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e7d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e7e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e7f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A5Q1::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A5Q1

namespace D2Game::Quests::QUESTS::A5Q2 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[12] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e80,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e84,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e82,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e83,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e85,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e86,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e81,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e8b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e89,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e8a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e8c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e8d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e87,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e91,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e8f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e90,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e92,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e93,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e8e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e91,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e8f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4e90,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e92,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e93,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e8e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e88,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A5Q2::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A5Q2

namespace D2Game::Quests::QUESTS::A5Q3 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[10] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e94,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e97,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e96,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e95,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e98,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e99,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4e9c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4e9b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e9a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x202,
                /* .nStringIndex = */ 0x4e9d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4e9e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ea1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ea0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x20f,
                /* .nStringIndex = */ 0x4ea3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4e9f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ea2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ea1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ea0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ea2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x3
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ea6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ea5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ea8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ea4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ea7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ea6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ea5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ea8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ea4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ea7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A5Q3::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A5Q3

namespace D2Game::Quests::QUESTS::A5Q4 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[8] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ea9,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ead,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4eab,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4eac,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4eae,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x4
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4eb1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4eb0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4eaf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4eb2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4eb3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4eb6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4eb5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4eb4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4eb7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4eb8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4eb6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4eb5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4eb4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4eb7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4eb8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A5Q4::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A5Q4

namespace D2Game::Quests::QUESTS::A5Q5 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[9] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4eb9,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ebd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ebb,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ebe,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4eba,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ebc,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ec2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ec0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ec1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ec3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ebf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ec7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ec5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ec6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ec8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ec4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x219,
                /* .nStringIndex = */ 0x4e22,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x21a,
                /* .nStringIndex = */ 0x4e22,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x21b,
                /* .nStringIndex = */ 0x4e22,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x3
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ec7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ec5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ec6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ec8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ec4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A5Q5::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A5Q5

namespace D2Game::Quests::QUESTS::A5Q6 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[9] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ecb,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4eca,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ecc,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ece,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ecd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ed2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ed1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ed3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x209,
                /* .nStringIndex = */ 0x4ecf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ed4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ed0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ed2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ed3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x209,
                /* .nStringIndex = */ 0x4ecf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ed4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ed0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x1ff,
                /* .nStringIndex = */ 0x4ed2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x201,
                /* .nStringIndex = */ 0x4ed3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x209,
                /* .nStringIndex = */ 0x4ecf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x203,
                /* .nStringIndex = */ 0x4ed4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x200,
                /* .nStringIndex = */ 0x4ed0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ed1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x208,
                /* .nStringIndex = */ 0x4ed1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A5Q6::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A5Q6

namespace D2Game::Quests::QUESTS::Quest37 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[3] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0xb,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x2d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x24,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x18,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x4
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0xc,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x2e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x25,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x19,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x4
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::Quest37::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::Quest37

namespace D2Game::Quests::QUESTS::A1Q0 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[3] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A1Q0::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A1Q0

namespace D2Game::Quests::QUESTS::A1Q1 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[8] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x40,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x41,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x46,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x45,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x42,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x43,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x48,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x4b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x49,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x47,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x4a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x4d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x50,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x4e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x4c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x4f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x50,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x4e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x4f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x3
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A1Q1::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A1Q1

namespace D2Game::Quests::QUESTS::A1Q2 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[6] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x51,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x52,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x56,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x53,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x55,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x54,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x57,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x5b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x59,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x58,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x5a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x5c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x60,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x5e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x5d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x5f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x60,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x5c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x5d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x5f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x4
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A1Q2::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A1Q2

namespace D2Game::Quests::QUESTS::A1Q3 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[6] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x92,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x94,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x95,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x93,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x96,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x97,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x99,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x9c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x9f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x9d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x9a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x9b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x9e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0xa2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0xa5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0xa3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0xa0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0xa1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0xa4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0xa2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0xa5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0xa0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0xa1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0xa4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A1Q3::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A1Q3

namespace D2Game::Quests::QUESTS::A1Q4 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[11] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x61,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x63,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x62,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x64,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x66,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x65,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x69,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x6b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x67,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x68,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x6a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x6c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x6f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x6d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x70,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x6e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x71,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x74,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x72,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x75,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x73,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x77,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x7a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x79,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x7b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x76,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x78,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x7d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x7b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x76,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x78,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x3
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x7d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x92,
                /* .nStringIndex = */ 0x7c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A1Q4::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A1Q4

namespace D2Game::Quests::QUESTS::A1Q5 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[5] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x83,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x81,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x82,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x84,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x85,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x80,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x86,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x88,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x89,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x8a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x87,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x8b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x8c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x8d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x91,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x90,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x8f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x8e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0x8c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0x8d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0x91,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0x90,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0x8f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0x8e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A1Q5::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A1Q5

namespace D2Game::Quests::QUESTS::A1Q6 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[6] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0xa6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0xa8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0xac,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0xa9,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0xa7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0xaa,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0xab,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0xb2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0xb1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0xaf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0xad,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0xb0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0xae,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0xb5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0xb8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0xb4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0xb6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0xb7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0xb3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x96,
                /* .nStringIndex = */ 0xb5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x109,
                /* .nStringIndex = */ 0xb8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9a,
                /* .nStringIndex = */ 0xb4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x93,
                /* .nStringIndex = */ 0xb6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x9b,
                /* .nStringIndex = */ 0xb7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x94,
                /* .nStringIndex = */ 0xb3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A1Q6::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A1Q6

namespace D2Game::Quests::QUESTS::A1Q7 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[6] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x10a,
                /* .nStringIndex = */ 0x3b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x10a,
                /* .nStringIndex = */ 0x3c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x10a,
                /* .nStringIndex = */ 0x3d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x10a,
                /* .nStringIndex = */ 0x3e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x10a,
                /* .nStringIndex = */ 0x3f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A1Q7::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A1Q7

namespace D2Game::Quests::QUESTS::Quest38 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[3] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0xcb,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0xbe,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0xdd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0xe6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0xf1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x107,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0xd7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x112,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x11d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x9
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0xcc,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0xbe,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0xdd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0xe7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0xf2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x108,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0xd7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x112,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x11e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x9
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::Quest38::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::Quest38

namespace D2Game::Quests::QUESTS::A2Q0 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[3] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0xfd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x7d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q0::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q0

namespace D2Game::Quests::QUESTS::A2Q1 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[9] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x130,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x136,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x133,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x131,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x132,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x138,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x139,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x13a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x137,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x134,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x135,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xa
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x13d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x13b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x13e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x140,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x142,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x141,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x144,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x13c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x13f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x143,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xa
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x14e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x145,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x14a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x146,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x147,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x14d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x148,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x149,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x14c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x9
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x14b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x14e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x145,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x14a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x146,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x14d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x148,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x149,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x14c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x9
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q1::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q1

namespace D2Game::Quests::QUESTS::A2Q2 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[11] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x14f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x150,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x151,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x152,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x153,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x153,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x14f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x150,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x151,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x152,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q2::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q2

namespace D2Game::Quests::QUESTS::A2Q3 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[6] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x158,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x157,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x155,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x156,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x15c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x15b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x15e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x15d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x159,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x154,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x15a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x158,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x157,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x155,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x156,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x15c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x15b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x15e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x15d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x159,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x154,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x15a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x169,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x161,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x160,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x167,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x166,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x165,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x168,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x163,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x15f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x162,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x164,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x170,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x16e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x16b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x16c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x173,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x172,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x174,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x16f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x16d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x16a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xa
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x170,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x16e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x16b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x16a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x16c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x173,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x172,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x16f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x16d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x171,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xa
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q3::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q3

namespace D2Game::Quests::QUESTS::A2Q4 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[12] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x175,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x175,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x179,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x17c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x17d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x177,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x178,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x17f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x17e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x17a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x179,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x180,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x17b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x176,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x189,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x18a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x183,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x181,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x184,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x182,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x185,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x18b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x188,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x187,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x186,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x196,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x193,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x18d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x190,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x18e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x18f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x195,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x197,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x192,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x191,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x194,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x193,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x18d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x190,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x18e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x18f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x195,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x197,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x192,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x191,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x194,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xa
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x14b,
                /* .nStringIndex = */ 0xb9,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x14b,
                /* .nStringIndex = */ 0xba,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x14b,
                /* .nStringIndex = */ 0xbb,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x14b,
                /* .nStringIndex = */ 0xbc,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x14b,
                /* .nStringIndex = */ 0xbd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q4::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q4

namespace D2Game::Quests::QUESTS::A2Q5 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[4] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x19e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x19d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x198,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x1a1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x19a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x19b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x1a2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x19c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x199,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x19f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x1a0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x1ab,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x1a8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x1a3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x1a7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x1a6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x1aa,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x1ad,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x1a9,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x1a5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x1a4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x1ac,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x1ab,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x1a8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x1a3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x1a7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x1a6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x1aa,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x1ad,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x1a9,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x1a5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x1a4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x1ac,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q5::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q5

namespace D2Game::Quests::QUESTS::A2Q6 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[8] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x1ae,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x1b2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x1b1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x1b9,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x1b0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x1af,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x1b7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x1b6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x1b8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x1b4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x1b3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x1b5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfb,
                /* .nStringIndex = */ 0x12e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x1bd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x1be,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x1c3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x1bb,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x1c1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x1ba,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xfb,
                /* .nStringIndex = */ 0x12e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x1bc,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x1c4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x1c2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x1c0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x1bf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xc
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x1bd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x1be,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc6,
                /* .nStringIndex = */ 0x1c3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc7,
                /* .nStringIndex = */ 0x1bb,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x1c1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc9,
                /* .nStringIndex = */ 0x1ba,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfb,
                /* .nStringIndex = */ 0x12e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x1bc,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x1c4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xc8,
                /* .nStringIndex = */ 0x1c0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x1bf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0xb
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xd2,
                /* .nStringIndex = */ 0x1c2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xb0,
                /* .nStringIndex = */ 0x1bd,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xaf,
                /* .nStringIndex = */ 0x1be,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xb1,
                /* .nStringIndex = */ 0x1c1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xca,
                /* .nStringIndex = */ 0x1bc,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xf4,
                /* .nStringIndex = */ 0x1c4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xb2,
                /* .nStringIndex = */ 0x1bf,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q6::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q6

namespace D2Game::Quests::QUESTS::A2Q7 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[6] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x179,
                /* .nStringIndex = */ 0x3b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x179,
                /* .nStringIndex = */ 0x3c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x179,
                /* .nStringIndex = */ 0x3d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x179,
                /* .nStringIndex = */ 0x3e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x179,
                /* .nStringIndex = */ 0x3f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q7::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q7

namespace D2Game::Quests::QUESTS::A2Q8 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[2] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x17a,
                /* .nStringIndex = */ 0x12f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A2Q8::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A2Q8

namespace D2Game::Quests::QUESTS::A4Q0 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[2] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x298,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A4Q0::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A4Q0

namespace D2Game::Quests::QUESTS::A4Q1 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[6] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x29e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x29f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2a0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x2a1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2a2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x2a4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2a5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x196,
                /* .nStringIndex = */ 0x2a3,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x3
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x2a4,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2a5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A4Q1::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A4Q1

namespace D2Game::Quests::QUESTS::A4Q2 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[7] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x2a9,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x2ab,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2aa,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x2ac,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2ad,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x2ac,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2ad,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x4e20,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x4e21,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x16f,
                /* .nStringIndex = */ 0x4e20,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x4e21,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A4Q2::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A4Q2

namespace D2Game::Quests::QUESTS::A4Q3 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[5] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2a6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2a7,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2a8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf6,
                /* .nStringIndex = */ 0x2a8,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A4Q3::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A4Q3

namespace D2Game::Quests::QUESTS::A4Q4 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[3] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x198,
                /* .nStringIndex = */ 0x29c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x198,
                /* .nStringIndex = */ 0x29d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A4Q4::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A4Q4

namespace D2Game::Quests::QUESTS::Quest39 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[3] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x1c5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x1ca,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x1de,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x1ea,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x1f5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x202,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x1c5,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x1df,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x1eb,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x1f6,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x203,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x0
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::Quest39::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::Quest39

namespace D2Game::Quests::QUESTS::A3Q0 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[3] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x1d1,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x1d2,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A3Q0::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A3Q0

namespace D2Game::Quests::QUESTS::A3Q1 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[6] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x225,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x226,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x227,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x228,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x229,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x22a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x22b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x22c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x22d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x22e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x22f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x230,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x231,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x232,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x233,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x234,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x235,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x236,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x237,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x238,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x239,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x23a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x239,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x23a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A3Q1::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A3Q1

namespace D2Game::Quests::QUESTS::A3Q2 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[13] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x21f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x221,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x220,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x222,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x223,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x224,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x21f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x221,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x220,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x222,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x223,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x224,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A3Q2::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A3Q2

namespace D2Game::Quests::QUESTS::A3Q3 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[8] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x23b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x23c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x23d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x23e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x23f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x240,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x241,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x242,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x243,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x244,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x245,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x246,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x247,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x248,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x249,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x24a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x24b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x24c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x24e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x24f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x250,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x24c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x24a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x24f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x250,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x4
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x24c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x24d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x24a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x24f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x250,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x251,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A3Q3::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A3Q3

namespace D2Game::Quests::QUESTS::A3Q4 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[9] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x20f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x210,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x211,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x213,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x216,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x212,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x214,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x215,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x213,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x214,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x215,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x3
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x216,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x217,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x218,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x219,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x4
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x21a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x21b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x21c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x21d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x4
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x21a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x21b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x21c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x21d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x21e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x20f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x210,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x2
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A3Q4::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A3Q4

namespace D2Game::Quests::QUESTS::A3Q5 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[8] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x252,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x253,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x255,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x257,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x259,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x25b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x25d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x25f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x254,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x256,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x258,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x25a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x25c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x25e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x260,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x261,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x263,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x264,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x266,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x268,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x26a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x26b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x262,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x265,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x267,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x269,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x26c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x5
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x26d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x26e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x26f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x270,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x271,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x272,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x273,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x26d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x26e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x26f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x270,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x271,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x272,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x273,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A3Q5::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A3Q5

namespace D2Game::Quests::QUESTS::A3Q6 {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2NPCMessageTableStrc aNpcGossipData[8] = {
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x274,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x1
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x275,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x277,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x279,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x27b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x27d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x27f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x281,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x276,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x278,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x27a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x27e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x280,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x282,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x6
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x283,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x285,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x287,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x289,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x28b,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x28d,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x28f,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x284,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x286,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x288,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x28a,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x28c,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x28e,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x290,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x291,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x292,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x293,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x294,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x295,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x296,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x297,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x1
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ 0xfe,
                /* .nStringIndex = */ 0x291,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xff,
                /* .nStringIndex = */ 0x292,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x108,
                /* .nStringIndex = */ 0x293,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfc,
                /* .nStringIndex = */ 0x294,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xfd,
                /* .nStringIndex = */ 0x295,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0xf5,
                /* .nStringIndex = */ 0x296,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {
                /* .nMonStatsId = */ 0x129,
                /* .nStringIndex = */ 0x297,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x7
    },
    {
        /* .pMessages = */ {
            {
                /* .nMonStatsId = */ -0x1,
                /* .nStringIndex = */ 0x0,
                /* .nPad = */ 0x0,
                /* .bMenu = */ 0x2
            },
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {}
        },
        /* .nMessagesCount = */ 0x0
    }
}; D2_GLOBAL_GUARD(aNpcGossipData, "D2Game::Quests::QUESTS::A3Q6::aNpcGossipData");

} // namespace D2Game::Quests::QUESTS::A3Q6

namespace D2Common::Drlg::DrlgGrid {

} // namespace D2Common::Drlg::DrlgGrid

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap
