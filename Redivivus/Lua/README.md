# Redivivus/Lua

The Lua mod host. Built only with the CMake option
`REDIVIVUS_MODS` (`D2_FEATURE_MODS=1`); with the option off none of this is compiled.

| Path | What |
|-|-|
| `d2_lua_host.*` | The sandboxed LuaJIT state: per-mod environments, the time budget per entry, the memory cap, the owner stack that decides which mod a fault belongs to |
| `d2_lua_pattern.cpp` | `string.find/match/gmatch/gsub` for mods: Lua 5.1 patterns as LuaJIT has them, with a step counter that honours the deadline |
| `d2_lua_mods.*`, `d2_lua_json.*` | Mod discovery, `info.json`, dependencies (Factorio syntax), load order |
| `d2_lua_runtime.*`, `d2_lua_rt.h` | A state per side running a mod list: entry files, `require` rules, events, scripts, storage save/restore, reload, source-mapped errors |
| `d2_lua_natives.cpp` | The natives behind the API: `script`, `remote`, `registries` (chains of wrappers), `storage`, `log`, `data` |
| `d2_lua_handle.cpp` | Handle classes: engine objects named by (class, id), resolved on every access |
| `d2_lua_storage.cpp` | `storage` as bytes and back |
| `d2_lua_messages.cpp` | Mod messages: the versioned envelope (format, mod index, payload), `script.send` / `script.on_message`, delivery as a guarded entry, the transport interface and the in-process loopback |
| `d2_lua_settings.cpp` | Mod settings: the settings file, declarations checked and their values settled, the host's list of every setting |
| `d2_lua_boot.*` | What the game runs at boot (`D2LUA_Boot` in `d2_lua.cpp`): discovery under `mods/`, load order with broken mods disabled, the settings stage, all logged |
| `d2_lua_api.h` | What generated glue and natives build on; the native calling convention |
| `api/` | The API as TypeScript declarations: the spec, and the `@redivivus/api` typings package (one entry per side) |
| `generated/` | Binding glue generated from `api/` (committed; checked by the build when Node is there) |
| `api/docs/reference.md`, `api/luals/` | Generated reference and LuaLS annotations for plain-Lua authors (`api/luals/.luarc.json` switches off the libraries the sandbox removes) |
| `api/lua/` | Generated: the Lua standard library a mod has, as TypeScript (lua-types trimmed to the sandbox, `tools/src/luaenv.ts`) |
| `api/ls-plugin/` | Generated: a TypeScript language service plugin (`tools/src/lsplugin.ts`) that shows the build's Script errors in an editor, at the call |
| `api/engine-enums.json`, `api/enums.d.ts` | The engine enums mods see: the allow-list (with what is left out and why), and the enums generated from `Source/D2Headers/d2_enums.h` (`tools/src/enums.ts`); their values are checked against the header in `generated/*_asserts.gen.cpp`, which only the game build compiles |
| `kit/` | The kit, TypeScript compiled into `mods/base/<side>/kit/` by the generator: typed settings, typed channels, the script scheduler, persisted per-owner state. Usable from plain Lua |
| `tools/` | The generator (`src/gen.ts`), the mod CLI (`src/cli.ts`), the TypeScriptToLua plugin (`src/plugin.ts`), `src/selftest.ts` |
| `mods/base/` | The base mod: the TypeScriptToLua runtime (once, for every TypeScript mod) and small shared Lua modules |
| `examples/` | Two TypeScript mods (the second imports from the first) and the type-level tests |
| `test/` | The host test, its own native class (`test/api`, a stand-in unit world) and fixture mods |

## Commands

```
cd Redivivus/Lua/tools && npx pnpm@10 install          # packages, pinned in package.json / pnpm-lock.yaml
node Redivivus/Lua/tools/src/gen.ts [--check]          # regenerate (or check) glue, docs, LuaLS files
node Redivivus/Lua/tools/src/cli.ts build --out DIR MOD...
node Redivivus/Lua/tools/src/cli.ts check MOD...       # every side type-checks; plugin checks run
node Redivivus/Lua/tools/src/cli.ts create DIR --name NAME
node Redivivus/Lua/tools/src/selftest.ts --host-test BUILD/d2_lua_host_test[.exe]
cmake -S Redivivus/Lua/test -B build-luatest [-DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake]
```

Node 22.18 or newer runs the tools' TypeScript directly; nothing is compiled first.
