#include "pbrtlua.h"
#include "api.h"
#include "paramset.h"
#include "objs/pbrtlualib.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#define API_FLOATS(_) \
    _(Translate,3,FLOAT3) \
    _(Rotate,4,FLOAT4) \
    _(Scale,3,FLOAT3) \
    _(LookAt,9,FLOAT9) \
    _(ConcatTransform,16,FLOATARR) \
    _(Transform,16,FLOATARR) \
    _(TransformTimes,2,FLOAT2)
    
#define FLOAT2() d[0],d[1]    
#define FLOAT3() d[0],d[1],d[2]
#define FLOAT4() d[0],d[1],d[2],d[3]
#define FLOAT9() d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8]
#define FLOATARR() d

#define API_NONE(_) \
    _(Identity) \
    _(ActiveTransformAll) \
    _(ActiveTransformEndTime) \
    _(ActiveTransformStartTime) \
    _(WorldBegin) \
    _(AttributeBegin) \
    _(AttributeEnd) \
    _(TransformBegin) \
    _(TransformEnd) \
    _(ReverseOrientation) \
    _(ObjectEnd) \
    _(WorldEnd)

#define API_STRING(_) \
    _(CoordinateSystem) \
    _(CoordSysTransform) \
    _(NamedMaterial) \
    _(ObjectBegin) \
    _(ObjectInstance)

#define API_STRING_PARAM(_) \
    _(PixelFilter) \
    _(Film) \
    _(Sampler) \
    _(Accelerator) \
    _(SurfaceIntegrator) \
    _(VolumeIntegrator) \
    _(Renderer) \
    _(Camera) \
    _(Material) \
    _(MakeNamedMaterial) \
    _(LightSource) \
    _(AreaLightSource) \
    _(Shape) \
    _(Volume)

#define API_ALL_BUT_FLOATS(_) \
    API_NONE(_) \
    API_STRING(_) \
    API_STRING_PARAM(_) \
    _(Texture)

typedef float float3[3];

#define DATA_KIND(_)\
 _(Float,float,1)\
 _(Int,int,1)\
 _(Bool,bool,1)\
 _(Point,Point,1)\
 _(Vector,Vector,1)\
 _(Normal,Normal,1)\
 _(RGBSpectrum,float,3)\
 _(XYZSpectrum,float,3)\
 _(BlackbodySpectrum,float,1)\
 _(SampledSpectrum,float,1)


#define P_ENUM(nm,typ,mult) K_##nm,
enum ParamKind { DATA_KIND(P_ENUM) K_String, K_Texture };
#undef P_ENUM

void createParamSet(lua_State * L, int tbl, ParamSet * r) {
    if(lua_gettop(L) < tbl || lua_isnil(L,tbl))
        return; //paramlist is optional
    //iterate over the key value pairs in the table
    lua_pushnil(L);
    while (lua_next(L, tbl) != 0) {
        const char * key = luaL_checkstring(L, -2);
        if(lua_isstring(L,-1)) {
            const std::string value[] = { luaL_checkstring(L,-1) };
            r->AddString(key,value,1);
            lua_pop(L,1);
            continue;
        }
        lua_getfield(L,-1,"kind");
        int kind = luaL_checknumber(L,-1);
        lua_pop(L,1); //the kind number
        switch(kind) {
        #define DATA_ACTION(nm,typ,mult) \
            case K_##nm: {\
                lua_getfield(L,-1,"data"); \
                typ * data = (typ*) lua_topointer(L,-1); \
                lua_pop(L,1); \
                lua_getfield(L,-1,"size"); \
                int sz = mult*luaL_checknumber(L,-1); \
                lua_pop(L,1); \
                r->Add##nm(key,data,sz); \
            } break;
        DATA_KIND(DATA_ACTION)
        #undef DATA_ACTION
            case K_String: {
                int N = lua_objlen(L,-1);
                std::vector<std::string> strings;
                for(int i = 0; i < N; i++) {
                    lua_rawgeti(L,-1,i+1);
                    strings.push_back(luaL_checkstring(L,-1));
                    lua_pop(L,1);
                }
                r->AddString(key, &strings[0], N);
            } break;
            case K_Texture: {
                lua_getfield(L,-1,"data");
                const char * tex = luaL_checkstring(L,-1);
                lua_pop(L,1);
                r->AddTexture(key, tex);
            } break;
            default:
                Error("Entry %s in Parameters has unknown type",key);
                break;
        }
        lua_pop(L,1); //value
    }
    lua_pop(L,1); //iterator nil
}

#define CREATE_NONE(nm) \
    int pbrtLua##nm(lua_State * L) { \
        pbrt##nm(); \
        return 0; \
    }
    
API_NONE(CREATE_NONE)
    

#define CREATE_STRING(nm) \
    int pbrtLua##nm(lua_State * L) { \
        const char * str = luaL_checkstring(L,-1); \
        pbrt##nm(str); \
        return 0; \
    }
    
API_STRING(CREATE_STRING)
    
#define CREATE_FLOATS(nm,N,args) \
    int pbrtLua##nm(lua_State * L) { \
        float d[N]; \
        lua_checkstack(L,N); \
        int idx = lua_gettop(L); \
        for(int i = 0; i < N; i++) { \
            lua_rawgeti(L,idx,i+1); \
            d[i] = luaL_checknumber(L,-1); \
        } \
        lua_pop(L,N); \
        pbrt##nm( args() ); \
        return 0; \
    }

API_FLOATS(CREATE_FLOATS)
    
#define CREATE_STRING_PARAM(nm) \
    int pbrtLua##nm(lua_State * L) { \
        const char * str = luaL_checkstring(L,1); \
        ParamSet params; \
        createParamSet(L,2,&params); \
        pbrt##nm(str,params); \
        return 0; \
    }
API_STRING_PARAM(CREATE_STRING_PARAM)

int pbrtLuaTexture(lua_State * L) {
    const char * name = luaL_checkstring(L,1);
    const char * type = luaL_checkstring(L,2);
    const char * texname = luaL_checkstring(L,3);
    ParamSet params;
    createParamSet(L, 4, &params);
    pbrtTexture(name,type,texname,params);
    return 0;
}
int loadandrunbytecodes(lua_State * L, const char * bytecodes, size_t size, const char * name) {
    return luaL_loadbuffer(L, bytecodes, size, name) 
           || lua_pcall(L,0,LUA_MULTRET,0);
}

lua_State * pbrtLuaInit() {
    lua_State * L = luaL_newstate();
    if(!L)
        Error("no lua!");
    luaL_openlibs(L);
    
    #define REGISTER_FN(nm) lua_pushcfunction(L,pbrtLua##nm); lua_setfield(L,LUA_GLOBALSINDEX,#nm);
    #define REGISTER_FLOAT(nm,dummy0,dummy1) REGISTER_FN(nm)
    API_NONE(REGISTER_FN)
    API_STRING(REGISTER_FN)
    API_STRING_PARAM(REGISTER_FN)
    API_FLOATS(REGISTER_FLOAT)
    REGISTER_FN(Texture)
    #undef REGISTER_FN
    #undef REGISTER_FLOAT
    if(loadandrunbytecodes(L,luaJIT_BC_pbrtlualib,luaJIT_BC_pbrtlualib_SIZE, "core/pbrtlualib.lua")) {
        Error("failed to load lua lib: %s", luaL_checkstring(L,-1));
    }
    
    return L;
}
void pbrtLuaRun(lua_State * L, const char * file) {
     if (luaL_dofile(L,file))
        Error("Error in scene file: %s", luaL_checkstring(L,-1));
}