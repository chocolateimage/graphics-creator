#include "lua_state.hpp"
#include "lua_code.hpp"
#include <iostream>

static const luaL_Reg luaLibrariesLoad[] = {
    {"", luaopen_base},
    {LUA_LOADLIBNAME, luaopen_package},
    {LUA_TABLIBNAME, luaopen_table},
    {LUA_STRLIBNAME, luaopen_string},
    {LUA_MATHLIBNAME, luaopen_math},
    {LUA_BITLIBNAME, luaopen_bit},
    {LUA_FFILIBNAME, luaopen_ffi},
    {LUA_JITLIBNAME, luaopen_jit},
};

lua_State *createLuaState() {
    lua_State *L = luaL_newstate();

    for (auto lib : luaLibrariesLoad) {
        lua_pushcfunction(L, lib.func);
        lua_pushstring(L, lib.name);
        lua_call(L, 1, 0);
    }

    luaJIT_setmode(L, -1, LUAJIT_MODE_ON);

    if (luaL_dostring(L, LUA_GLOBAL_CODE) != LUA_OK) {
        auto err = lua_tostring(L, -1);
        std::cout << err << std::endl;
        lua_pop(L, 1);
    }

    return L;
}
