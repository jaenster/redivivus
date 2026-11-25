# The Lua API's generated binding glue (Redivivus/Lua/generated, Redivivus/Lua/test/generated)
# is committed, so a build needs no Node. When Node (22.18 or newer, which runs TypeScript
# directly) and the tools' packages (pnpm install in Redivivus/Lua/tools) are there, the target
# lua_api_check runs the generator in --check mode and fails the build when the committed glue
# no longer matches the declarations in Redivivus/Lua/api.
#
# Include once; sets D2_LUA_API_CHECK to the target's name when it exists.

if(NOT D2_ROOT)
    get_filename_component(D2_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()
set(D2_LUA_API_CHECK "")
find_program(D2_NODE node)
set(_lua_tools "${D2_ROOT}/Redivivus/Lua/tools")
if(D2_NODE AND EXISTS "${_lua_tools}/node_modules/typescript/package.json")
    execute_process(COMMAND "${D2_NODE}" --version OUTPUT_VARIABLE _node_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^v" "" _node_version "${_node_version}")
    if(_node_version VERSION_GREATER_EQUAL 22.18)
        file(GLOB _lua_api_inputs CONFIGURE_DEPENDS
            "${D2_ROOT}/Redivivus/Lua/api/*.d.ts" "${D2_ROOT}/Redivivus/Lua/api/*.json" "${D2_ROOT}/Redivivus/Lua/test/api/*.d.ts"
            "${_lua_tools}/src/*.ts" "${D2_ROOT}/Source/D2Headers/d2_enums.h")
        add_custom_command(
            OUTPUT "${CMAKE_BINARY_DIR}/lua-api-check.stamp"
            COMMAND "${D2_NODE}" "${_lua_tools}/src/gen.ts" --check
            COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_BINARY_DIR}/lua-api-check.stamp"
            DEPENDS ${_lua_api_inputs}
                "${D2_ROOT}/Redivivus/Lua/generated/d2_lua_api_main.gen.cpp"
                "${D2_ROOT}/Redivivus/Lua/test/generated/d2_lua_api_test.gen.cpp"
            COMMENT "Lua API: the committed binding glue matches the declarations"
            VERBATIM)
        add_custom_target(lua_api_check DEPENDS "${CMAKE_BINARY_DIR}/lua-api-check.stamp")
        set(D2_LUA_API_CHECK lua_api_check)
        message(STATUS "Lua API: node ${_node_version}, the generated glue is checked against the declarations")
    else()
        message(STATUS "Lua API: node ${_node_version} is older than 22.18; the committed glue is used unchecked")
    endif()
else()
    message(STATUS "Lua API: no node or no Redivivus/Lua/tools/node_modules; the committed glue is used unchecked")
endif()
