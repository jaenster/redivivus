// The host test's checks of the boot path the game takes (d2_lua_boot.h, called by D2LUA_Boot):
// discovery, load order with the broken mods disabled, the settings stage, the log, over the
// fixture directory Redivivus/Lua/test/boot/mods (redivivus itself is built in).
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdio.h>
#include <string.h>

#include <string>

#include "Redivivus/Lua/d2_lua_boot.h"
#include "Redivivus/Lua/generated/d2_lua_api_main.gen.h"
#include "Redivivus/Lua/test/d2_lua_test_util.h"

static void Expect(const char* pszText, const char* pszWhat) {
    if (!Logged(pszText)) {
        printf("  wanted in log: %s\n", pszText);
    }
    Check(Logged(pszText), pszWhat);
}

void RunBootTests(const char* pszRoot) {
    printf("--- boot (the game's path)\n");
    std::string sRoot = pszRoot;
    D2LuaBootDesc tDesc;
    tDesc.aRoots = { sRoot + "/Redivivus/Lua/test/boot/mods" };
    tDesc.sSettingsFile = sRoot + "/Redivivus/Lua/test/boot/mods/settings.json";
    tDesc.aApis = { &g_D2LuaApi_main };
    tDesc.pfnLog = TestLog;
    tDesc.tLimits = { 250, (size_t)64 * 1024 * 1024 };
    D2LuaBootResult tBoot;
    ClearLog();
    D2LuaBoot_Run(tDesc, tBoot);
    Expect("[mods] LuaJIT", "the boot names the runtime");
    Expect("[mods] redivivus 0.1.0 is built in", "the boot names redivivus, built in, with the exe's version");
    Expect("[mods] 8 mod(s) found", "every mod directory with a readable info.json is found");
    Expect("a mod folder named redivivus is not allowed", "a folder named redivivus under mods/ is refused");
    Expect("the name \"mod\" is reserved", "a mod named mod is refused");
    Expect("[mods]   boot-a 1.2.0 \"Boot A\" in ", "  each listed with version, title and path");
    Expect("[mods] error: ", "a broken info.json is an error");
    Check(Logged("boot-bad") && Logged("[mods] error: ") && !Logged("boot-bad 0"), "  and that mod is not found");
    Expect("[mods] error: mod 'boot-old' is disabled: mod 'boot-old' requires 'boot-a >= 9.0.0', but version 1.2.0 is installed",
           "a mod whose dependency is too old is disabled, with the reason");
    Expect("[mods] error: mod 'boot-orphan' is disabled: mod 'boot-orphan' requires 'missing-mod', which is not installed",
           "a mod with a missing dependency is disabled");
    Expect("[mods] error: mod 'boot-needs-old' is disabled: mod 'boot-needs-old' requires 'boot-old'", "  a mod needing a disabled mod too");
    Expect("[mods] error: mod 'boot-cycle1' is disabled: dependency cycle: boot-cycle1 -> boot-cycle2 -> boot-cycle1",
           "a dependency cycle disables its members");
    Expect("[mods] error: mod 'boot-cycle2' is disabled: dependency cycle", "  all of them");
    Expect("[mods] load order: redivivus, boot-a, boot-b, boot-thrower", "the rest load, in dependency order, redivivus first");
    Expect("[lua] [boot-a] boot-a settings stage ran", "the settings stage runs");
    Check(!Logged("boot-a data stage ran") && !Logged("boot-a server ran"),
          "  and no stage that needs engine hooks (data.lua, main)");
    Expect("boot-thrower breaks in its settings stage", "a mod that fails in its settings stage is logged");
    Expect("[mods] error: mod 'boot-thrower' is disabled: it failed in the settings stage",
           "  and disabled, as a mod that runs out of time is");
    Check(tBoot.pSettings && D2LuaRt_IsFaulted(tBoot.pSettings, "boot-thrower") && !D2LuaRt_IsFaulted(tBoot.pSettings, "boot-a"),
          "  it is faulted in the settings state, the others are not");
    std::string sOrder;
    for (const D2LuaModInfo& t : tBoot.aOrder) {
        sOrder += t.sName + " ";
    }
    Check(sOrder == "redivivus boot-a boot-b " && tBoot.aFound.size() == 8 && tBoot.aErrors.size() == 9,
          "  the result leaves it out of the load order and lists the error");
    Expect("[mods] settings stage ran: 2 setting(s) declared", "the declared settings are counted");
    Expect("[mods]   boot-a.volume = 8 (int, startup)", "  listed with the file's value");
    Expect("[mods]   boot-a.lang = \"en\" (choice, player; the file's value was refused: 'fr' is not one of the choices)",
           "  and a refused value with the reason");
    Expect("[mods] not run yet: the data stage", "the log says what did not run");
    Check(tBoot.pSettings && D2LuaRt_Settings(tBoot.pSettings).size() == 2, "the settings stage stays up for a settings screen");
    D2LuaBoot_Close(tBoot);
    D2LuaBoot_Close(tBoot);
    Check(tBoot.pSettings == nullptr, "closing is safe twice");

    D2LuaBootDesc tEmpty = tDesc;
    tEmpty.aRoots = { sRoot + "/Redivivus/Lua/test/boot/no-such-dir" };
    ClearLog();
    D2LuaBoot_Run(tEmpty, tBoot);
    Check(Logged("[mods] 0 mod(s) found") && tBoot.pSettings == nullptr && tBoot.aOrder.empty(), "no mods: nothing runs");
}
#endif
