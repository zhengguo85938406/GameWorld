#include "stdafx.h"
#include "KCDTimerList.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KCoolDownList.h"

KCDTimerList::KCDTimerList(void)
{
    m_pOwner = NULL;
}

KCDTimerList::~KCDTimerList(void)
{
    assert(m_pOwner == NULL);
}

BOOL KCDTimerList::Init(KPlayer* pOwner)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pOwner);

    m_pOwner = pOwner;
    m_TimerList.clear();

    bResult = true;
Exit0:
    return bResult;
}

void KCDTimerList::UnInit()
{
    m_TimerList.clear();
    m_pOwner = NULL;
}

BOOL KCDTimerList::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL                                    bResult             = false;
    std::pair<TIMER_LIST::iterator, bool>   InsRet;
    BOOL                                    bRetCode            = false;

    assert(pLoadBuf);

    const T3DB::KPBCDTimerList& CDTimerList = pLoadBuf->cdtimerlist();
    const T3DB::KPB_COOLDOWN_LIST_DATA&   CoolDownListData   = CDTimerList.cooldownlist();

    // 清除原有数据
    m_TimerList.clear();

    // 读取计时器表
    for (int nIndex = 0; nIndex < CoolDownListData.cooldowndata_size(); nIndex++)
    {
        KTIMER_DATA TimerData;
        const T3DB::KPB_COOLDOWN_DATA& CDData = CoolDownListData.cooldowndata(nIndex);

        if (CDData.time() <= g_pSO3World->m_nCurrentTime)
            continue;
        
        TimerData.nEndFrame = g_pSO3World->m_nGameLoop +
            (int)(CDData.time() - g_pSO3World->m_nCurrentTime) * GAME_FPS;

        TimerData.nInterval = CDData.interval();

        InsRet = m_TimerList.insert(
            std::make_pair(CDData.timerid(), TimerData)
        );
        KGLOG_PROCESS_ERROR(InsRet.second);

        if (m_pOwner->m_eConnectType == eLogin)
        {
            g_PlayerServer.DoSetCoolDownNotify(
                m_pOwner->m_nConnIndex, CDData.timerid(), 
                TimerData.nEndFrame,    TimerData.nInterval
            );
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL  KCDTimerList::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                            bResult             = false;
    T3DB::KPBCDTimerList*           pCDTimerList        = NULL;
    BOOL                            bRetCode            = false;
    T3DB::KPB_COOLDOWN_LIST_DATA*   pCoolDownListData   = NULL;
    T3DB::KPB_COOLDOWN_DATA*        pCDData             = NULL;
  
    KGLOG_PROCESS_ERROR(pSaveBuf);

    pCDTimerList = pSaveBuf->mutable_cdtimerlist();
    pCoolDownListData = pCDTimerList->mutable_cooldownlist();
    for (TIMER_LIST::iterator it = m_TimerList.begin(); it != m_TimerList.end(); ++it)
    {
        bRetCode = g_pSO3World->m_Settings.m_CoolDownList.IsCoolDownNeedSave(it->first);
        if (!bRetCode)
            continue;        

        if (it->second.nEndFrame <= g_pSO3World->m_nGameLoop + LOGOUT_WAITING_TIME)
            continue;

        pCDData = pCoolDownListData->add_cooldowndata();
        pCDData->set_timerid(it->first);
        pCDData->set_time((int)(
            (it->second.nEndFrame - g_pSO3World->m_nGameLoop) / GAME_FPS + g_pSO3World->m_nCurrentTime
        ));
        pCDData->set_interval((it->second).nInterval);
    }

    bResult = true;
Exit0:
    return bResult;
}

void KCDTimerList::ResetTimer(DWORD dwTimerID, int nInterval)
{
	KTIMER_DATA TimerData;

	TimerData.nEndFrame = g_pSO3World->m_nGameLoop + nInterval;
	TimerData.nInterval = nInterval;
    
    g_PlayerServer.DoSetCoolDownNotify(m_pOwner->m_nConnIndex, dwTimerID, TimerData.nEndFrame, TimerData.nInterval);

	m_TimerList[dwTimerID] = TimerData;

Exit0:
	return;
}

BOOL KCDTimerList::CheckTimer(DWORD dwTimerID)
{
    BOOL                    bResult = false;
	TIMER_LIST::iterator    it;
    std::vector<DWORD> vecGroupTimerID;
    
    g_pSO3World->m_Settings.m_CoolDownList.GetGroupCoolDown(dwTimerID, vecGroupTimerID);

    for (size_t i = 0; i < vecGroupTimerID.size(); ++i)
    {
        it = m_TimerList.find(vecGroupTimerID[i]);
        if (it == m_TimerList.end())
            continue;

        KG_PROCESS_ERROR(g_pSO3World->m_nGameLoop >= it->second.nEndFrame);
    
        m_TimerList.erase(it);
    }

    bResult = true;
Exit0:
    return bResult;
}

void KCDTimerList::ClearTimer(DWORD dwTimerID)
{
	TIMER_LIST::iterator it = m_TimerList.find(dwTimerID);
	
    KG_PROCESS_ERROR(it != m_TimerList.end());

	m_TimerList.erase(it);

    g_PlayerServer.DoSetCoolDownNotify(m_pOwner->m_nConnIndex, dwTimerID, -1, 0); // 给客户端同步一个nEndFrame为-1的值，用来Clear客户端的CD

Exit0:
	return;
}

BOOL KCDTimerList::GetTimerData(DWORD dwTimerID, int& rnInterval, int& rnEndFrame)
{
    BOOL                    bResult = false;
	TIMER_LIST::iterator    it      = m_TimerList.find(dwTimerID);

	KG_PROCESS_ERROR(it != m_TimerList.end());

	rnInterval = it->second.nInterval;
	rnEndFrame = it->second.nEndFrame;

    bResult = true;
Exit0:
	return bResult;
}
