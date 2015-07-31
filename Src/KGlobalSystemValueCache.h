#ifndef _KGLOBAL_SYSTEM_VALUE_CACHE_H_
#define _KGLOBAL_SYSTEM_VALUE_CACHE_H_

#include "Luna.h"

class KGlobalSystemValueCache
{
public:
    KGlobalSystemValueCache();

public:
    int m_nGameWorldStartTime;
    int m_nMaxPlayerLevel;
    int m_nGameWorldChargeTime;
    int m_nChargeMode;

    DECLARE_LUA_CLASS(KGlobalSystemValueCache);
    DECLARE_LUA_INTEGER(GameWorldStartTime);
    DECLARE_LUA_INTEGER(MaxPlayerLevel);
    DECLARE_LUA_INTEGER(GameWorldChargeTime);
    DECLARE_LUA_INTEGER(ChargeMode);
};
#endif
