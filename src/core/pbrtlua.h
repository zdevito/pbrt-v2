#ifndef PBRT_CORE_LUA_H
#define PBRT_CORE_LUA_H

struct lua_State;
lua_State * pbrtLuaInit();
void pbrtLuaRun(lua_State * L, const char * file);

#endif
