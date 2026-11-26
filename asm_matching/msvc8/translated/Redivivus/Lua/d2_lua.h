// Mods, on the Lua runtime: set up once the game's log is up (discovery,
// load order, the settings stage; see d2_lua_boot.h) and closed when the application returns.
//
// Built only with the REDIVIVUS_MODS option (CMake), which sets D2_FEATURE_MODS=1. Otherwise both calls expand to nothing, so a call site in generated code
// leaves that unit's code exactly as it was.
#pragma once

#if D2_FEATURE_MODS
// Discover the mods under mods/ beside the executable, order them and run the settings stage.
// Never fatal: every failure is written to the game's log and the game carries on without mods.
extern "C" void D2LUA_Boot(void);

// Close the state. Safe without a successful boot and safe to call twice.
extern "C" void D2LUA_Shutdown(void);
#else
// Macros, not empty inline functions: the build is unoptimised, and GCC at -O0 leaves a nop
// where even an always_inline empty function was inlined, which would change the generated
// unit's code with the option off.
#define D2LUA_Boot() ((void)0)
#define D2LUA_Shutdown() ((void)0)
#endif
