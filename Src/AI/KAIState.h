////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KAIState.h
//  Version     : 1.0
//  Creator     : Chen Jie, zhaochunfeng
//  Comment     : AI状态结点
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _KAISTATE_H_ 
#define _KAISTATE_H_ 

#include "Luna.h"
#include "KAIAction.h"

class KHero;
class KAILogic;

class KAIState
{
public:
    KAIState(KAILogic* pAILogic, int nStateID) { assert(pAILogic); m_pAILogic = pAILogic; m_nStateID = nStateID;};

    KAIActionHandle GetEventHandler(int nEvent);

private:
    struct KAI_EVENT_HANDLE  
    {
        int             nEvent;
        KAIActionHandle ActionKey;
    };

    std::vector<KAI_EVENT_HANDLE>   m_EventHandleVector;    // Event handles
    KAILogic*                       m_pAILogic;
    int                             m_nStateID;             // 该变量纯粹输出Log用

public:
    DECLARE_LUA_CLASS(KAIState);
    int  LuaHandleEvent(Lua_State* L);
    int  GetStateID() {return m_nStateID;};
    void DebugStateInfo();
};

#endif //_KAISTATE_H_
