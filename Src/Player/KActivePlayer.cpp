#include "stdafx.h"
#include "KActivePlayer.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KLSClient.h"
#include "KActiveSettings.h"

KActivePlayer::KActivePlayer(void)
{
    m_pPlayer = NULL;
    m_nCurActivePoint = 0;
    m_nOnlineFrame = 0;
    m_nCostFatiguePoint = 0;
}

KActivePlayer::~KActivePlayer(void)
{
}

BOOL KActivePlayer::Init(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(!m_pPlayer);

    m_pPlayer = pPlayer;

    m_vecProcActive.resize(g_pSO3World->m_Settings.m_ActivePlayerSettings.GetMaxActiveID());

    for (size_t i = 0; i < m_vecProcActive.size(); ++i)
    {
        m_vecProcActive[i].nValue = 0;
        m_vecProcActive[i].bFinished = false;
    }
    bResult = true;
Exit0:
    return bResult;
}

void KActivePlayer::UnInit()
{
    m_pPlayer = NULL;
}

BOOL KActivePlayer::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pLoadBuf);
    {
        const T3DB::KPB_ACTIVEPLAYER_DATA& activePlayerData = pLoadBuf->activeplayerdata();

        m_nCurActivePoint = activePlayerData.curactivepoint();
        m_nCostFatiguePoint = activePlayerData.costfatiguepoint();
        m_nOnlineFrame = activePlayerData.onlineframe();

        for (int i = 0; i < activePlayerData.activeproc_size(); ++i)
        {
            const T3DB::KPB_ACTIVE_PROC& activeProc = activePlayerData.activeproc(i);

            m_vecProcActive[i].bFinished = activeProc.finished();
            m_vecProcActive[i].nValue = activeProc.value();
        }

        for (int i = 0; i < activePlayerData.awardalreadyget_size(); ++i)
        {
            m_setAwardAlreadyGet.insert(activePlayerData.awardalreadyget(i));
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KActivePlayer::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    std::set<DWORD>::iterator it;
    T3DB::KPB_ACTIVEPLAYER_DATA* pData = NULL;
    T3DB::KPB_ACTIVE_PROC* pProc = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pData = pSaveBuf->mutable_activeplayerdata();
    KGLOG_PROCESS_ERROR(pData);

    pData->set_curactivepoint(m_nCurActivePoint);
    pData->set_costfatiguepoint(m_nCostFatiguePoint);
    pData->set_onlineframe(m_nOnlineFrame);

    for (it = m_setAwardAlreadyGet.begin(); it != m_setAwardAlreadyGet.end(); ++it)
    {
        pData->add_awardalreadyget(*it);
    }

    for (size_t i = 0; i < m_vecProcActive.size(); ++i)
    {
        pProc = pData->add_activeproc();
        KGLOG_PROCESS_ERROR(pProc);

        pProc->set_finished(m_vecProcActive[i].bFinished);
        pProc->set_value(m_vecProcActive[i].nValue);
    }

    bResult = true;
Exit0:
    return bResult;
}

void KActivePlayer::OnEvent(int nEvent)
{
    DWORD                   dwActiveID = 0;
    KActiveSettingsItem*    pActiveItem = NULL;
    KVEC_ACTIVE_UPDATOR*    pActiveUpdator = NULL;

    pActiveUpdator = g_pSO3World->m_Settings.m_ActivePlayerSettings.GetActiveByEvent(nEvent);
    KG_PROCESS_ERROR(pActiveUpdator);

    for (size_t i = 0; i < pActiveUpdator->size(); ++i)
    {
        dwActiveID = pActiveUpdator->at(i);

        UpdateActiveValue(dwActiveID);
    }

Exit0:
    return;
}

BOOL KActivePlayer::UpdateActiveValue(DWORD dwActiveID)
{
    BOOL                    bResult  = false;
    KActiveSettingsItem*    pActiveItem = NULL;
    KActiveProcess*         pActiveProc = NULL;

    KGLOG_PROCESS_ERROR(dwActiveID  > 0 && dwActiveID  <= m_vecProcActive.size());

    pActiveProc = &m_vecProcActive[dwActiveID - 1];
    pActiveItem = g_pSO3World->m_Settings.m_ActivePlayerSettings.GetItem(dwActiveID);
    KGLOG_PROCESS_ERROR(pActiveItem);
    KG_PROCESS_SUCCESS(pActiveProc->bFinished);

    ++pActiveProc->nValue;

    if (pActiveItem->nFininshValue <= pActiveProc->nValue)
        pActiveProc->bFinished = true;

    g_PlayerServer.DoUpdateActiveProcess(m_pPlayer->m_nConnIndex, dwActiveID, pActiveProc->nValue, pActiveProc->bFinished);

    if (pActiveProc->bFinished)
        FinishActivePoint(dwActiveID);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KActivePlayer::AddActivePoint(DWORD dwActivePoint)
{
    std::vector<int> vecID;
    KActivePlayerAwardData* pAwardData = NULL;
    int nOldActivePoint = m_nCurActivePoint;

    m_nCurActivePoint += dwActivePoint;
    g_PlayerServer.DoUpdateActivePoint(m_pPlayer->m_nConnIndex, m_nCurActivePoint);

    g_pSO3World->m_Settings.m_ActivePlayerAward.GetAllKey(vecID);
    for (size_t i = 0; i < vecID.size(); ++i)
    {
        pAwardData = g_pSO3World->m_Settings.m_ActivePlayerAward.GetByID(vecID[i]);
        KGLOG_PROCESS_ERROR(pAwardData);
        if (pAwardData->nActivePoint > nOldActivePoint && pAwardData->nActivePoint <= m_nCurActivePoint)
        {
            m_pPlayer->m_Secretary.AddReport(KREPORT_EVNET_ACTIVEPLAYER_AWARD, NULL, 0);
            break;
        }
    }

Exit0:
    return;
}

void KActivePlayer::FinishActivePoint(DWORD dwActiveID)
{
    KActiveSettingsItem*    pActiveItem = NULL;
    KActiveProcess*         pActiveProc = NULL;

    KGLOG_PROCESS_ERROR(dwActiveID  > 0 && dwActiveID  <= m_vecProcActive.size());

    pActiveProc =  &m_vecProcActive[dwActiveID - 1];
    pActiveItem = g_pSO3World->m_Settings.m_ActivePlayerSettings.GetItem(dwActiveID);

    AddActivePoint(pActiveItem->nAwardActive);

    if (pActiveItem->bUnLimit)
        pActiveProc->bFinished = false;

    PLAYER_LOG(m_pPlayer, "activity,activepoint,%d,%d", dwActiveID, pActiveItem->nAwardActive);

Exit0:
    return;
}

void KActivePlayer::Refresh()
{
    BOOL bRetCode = false;

    for (size_t i = 0; i < m_vecProcActive.size(); ++i)
    {
        m_vecProcActive[i].nValue = 0;
        m_vecProcActive[i].bFinished = false;
    }

    m_nCurActivePoint = 0;
    m_setAwardAlreadyGet.clear();
    m_nCostFatiguePoint = 0;
    m_nOnlineFrame = 0;

    g_PlayerServer.DoClearActivePlayerData(m_pPlayer->m_nConnIndex);

Exit0:
    return;
}

BOOL KActivePlayer::ApplyAward(DWORD dwAwardID)
{
    BOOL bResult = false;
    std::set<DWORD>::iterator it;
    KActivePlayerAwardData* pAwardData = NULL;
    KSystemMailTxt* pSysMailTxt     = NULL;

    pAwardData = g_pSO3World->m_Settings.m_ActivePlayerAward.GetByID(dwAwardID);
    KGLOG_PROCESS_ERROR(pAwardData);

    it = m_setAwardAlreadyGet.find(dwAwardID);
    KGLOG_PROCESS_ERROR(it == m_setAwardAlreadyGet.end());

    KGLOG_PROCESS_ERROR(pAwardData->nActivePoint <= m_nCurActivePoint);
    
    if (pAwardData->nTabIndex == 0 && pAwardData->nTabType == 0)
    {
        m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, pAwardData->nMoney);
    }
    else
    {
        pSysMailTxt = g_pSO3World->m_Settings.m_SystemMailMgr.GetSystemMailTxt(KMAIL_ACTIVEPLAYER);

        m_pPlayer->SafeAddItem((DWORD)pAwardData->nTabType, (DWORD)pAwardData->nTabIndex, pAwardData->nCount, pAwardData->nValuePoint,
            pSysMailTxt->m_szMailTitl, pSysMailTxt->m_szSendName, pSysMailTxt->m_szMailTxt
        );
    }

    
    m_setAwardAlreadyGet.insert(dwAwardID);

    PLAYER_LOG(m_pPlayer, "activity,activepointaward");

    bResult = true;
Exit0:
    g_PlayerServer.DoApplyActiveAwardRespond(m_pPlayer->m_nConnIndex, dwAwardID, bResult);
    return bResult;
}

void KActivePlayer::CallReportNotSign()
{
    BOOL bRetCode               = false;
    int  nLuaTopIndex           = 0;
    const char* pszFuncName     = "ReportNotSign";
    DWORD dwScriptID            = g_FileNameHash(SCRIPT_DIR"/activity/activity.lua");

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nLuaTopIndex);
    g_pSO3World->m_ScriptManager.Push(m_pPlayer);
    g_pSO3World->m_ScriptManager.Call(dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nLuaTopIndex);

Exit0:
    return;
}

void KActivePlayer::Activate()
{
    ++m_nOnlineFrame;
    switch (m_nOnlineFrame)
    {
    case TWO_HOUR_FRAME:
        m_pPlayer->OnEvent(peOnline2Hour);
        break;
    case FIVE_MINIUTE_FRAME:
        CallReportNotSign();
        break;
    default:
        break;
    }

    m_pPlayer->ReportOnlineAward();
}

void KActivePlayer::AddCostFatiguePoint(int nCostFatiguePoint)
{
    int nOldCostFatiguePoint = m_nCostFatiguePoint;

    m_nCostFatiguePoint += nCostFatiguePoint;

    g_PlayerServer.DoSyncCostFatiguePoint(m_pPlayer->m_nConnIndex, m_nCostFatiguePoint);

    if (m_nCostFatiguePoint >= CAST_FATIGUE_POINT_IOO && nOldCostFatiguePoint < CAST_FATIGUE_POINT_IOO)
    {
        m_pPlayer->OnEvent(peCastFatiguePoint100);
    }
}

void KActivePlayer::SyncActivePlayer()
{
    g_PlayerServer.DoSyncActiveProcess(m_pPlayer->m_nConnIndex, (int)m_vecProcActive.size(), &m_vecProcActive[0]);
    g_PlayerServer.DoSyncActiveAward(m_pPlayer->m_nConnIndex, m_nCurActivePoint, m_nOnlineFrame / 24, m_setAwardAlreadyGet);
    g_PlayerServer.DoSyncCostFatiguePoint(m_pPlayer->m_nConnIndex, m_nCostFatiguePoint);
}