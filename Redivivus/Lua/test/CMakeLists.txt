# The Lua host's test program, on its own or as part of the game's build (REDIVIVUS_MODS=ON adds
# this directory). On its own it builds for whatever the toolchain targets, natively too:
#
#   cmake -S Redivivus/Lua/test -B build-luatest                                    (this machine)
#   cmake -S Redivivus/Lua/test -B build-luatest32 -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake
#   cmake -S Redivivus/Lua/test -B build-luatest64 -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-x86_64.cmake
#   cmake --build build-luatest && build-luatest/d2_lua_host_test
cmake_minimum_required(VERSION 3.24)
if(NOT D2_ROOT)
    project(d2_lua_host_test LANGUAGES C CXX)
    get_filename_component(D2_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
endif()
if(NOT TARGET luajit)
    include("${D2_ROOT}/cmake/luajit.cmake")
endif()

# The whole host (every unit of Redivivus/Lua and the generated main API), the test's own native
# class and its generated glue, and the test program.
file(GLOB _lua_host_test_sources CONFIGURE_DEPENDS
    "${D2_ROOT}/Redivivus/Lua/*.cpp"
    "${D2_ROOT}/Redivivus/Lua/generated/*.cpp"
    "${D2_ROOT}/Redivivus/Lua/test/*.cpp"
    "${D2_ROOT}/Redivivus/Lua/test/generated/*.cpp")
list(REMOVE_ITEM _lua_host_test_sources "${D2_ROOT}/Redivivus/Lua/d2_lua.cpp")
# The engine enum value checks include the engine's headers, which do not build beside this one's.
list(FILTER _lua_host_test_sources EXCLUDE REGEX "_asserts\\.gen\\.cpp$")
add_executable(d2_lua_host_test EXCLUDE_FROM_ALL ${_lua_host_test_sources})
if(PROJECT_NAME STREQUAL d2_lua_host_test)
    set_target_properties(d2_lua_host_test PROPERTIES EXCLUDE_FROM_ALL OFF)
endif()
target_compile_features(d2_lua_host_test PRIVATE cxx_std_17)
if(D2_LUAJIT_CHECKHOOK)
    set(_checkhook 1)
else()
    set(_checkhook 0)
endif()
target_compile_definitions(d2_lua_host_test PRIVATE
    D2_FEATURE_MODS=1 D2_LUA_HOST_TEST=1 D2_LUAJIT_CHECKHOOK=${_checkhook})
target_compile_options(d2_lua_host_test PRIVATE -g -Wall -Wextra)
target_include_directories(d2_lua_host_test PRIVATE "${D2_ROOT}")
if(WIN32)
    target_link_options(d2_lua_host_test PRIVATE -static -static-libgcc -static-libstdc++)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(d2_lua_host_test PRIVATE Threads::Threads)
endif()
target_link_libraries(d2_lua_host_test PRIVATE luajit)
add_dependencies(d2_lua_host_test luajit_make)
if(NOT DEFINED D2_LUA_API_CHECK)
    include("${D2_ROOT}/cmake/lua-api.cmake")
endif()
if(D2_LUA_API_CHECK)
    add_dependencies(d2_lua_host_test ${D2_LUA_API_CHECK})
endif()
