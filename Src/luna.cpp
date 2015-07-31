/************************************************************************/
/* Copyright : Kingsoft 2007										    */
/* Author	 : Zhao Chunfeng										    */
/* History	 :															*/
/*		2007.10.12	Create												*/
/*      拦截Lua对一些无效对象的引用                                     */
/************************************************************************/

#include "stdafx.h"
#include "Luna.h"
#include "KHero.h"

template <>
int Dispatcher<KHero>(lua_State* L)
{
    Luna<KHero>::KObjData* pObjData = (Luna<KHero>::KObjData*)lua_touserdata(L, lua_upvalueindex(1));
    Luna<KHero>::KLuaData* pLuaData = (Luna<KHero>::KLuaData*)lua_touserdata(L, lua_upvalueindex(2));
    KHero* pHero = pObjData->pObj;

    if (pHero->m_pScene == NULL)
    {
        KGLogPrintf(KGLOG_WARNING, "[Lua] index to a invalid hero !\n");
        return 0;
    }
	
    return (pHero->*(Luna<KHero>::LuaFunc)(pLuaData->pGetFunc))(L);
}
