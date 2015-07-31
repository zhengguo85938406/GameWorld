#include "stdafx.h"
#include "KAchievement.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KSystemMailMgr.h"
#include "KLSClient.h"

KAchievement::KAchievement(void)
{
    m_nAchievementPoint = 0;
    m_pPlayer = NULL;
    memset(m_Achievement, 0 ,sizeof(m_Achievement));
}


KAchievement::~KAchievement(void)
{
}

BOOL KAchievement::Init(KPlayer* pPlayer)
{
	BOOL bResult  = false;
	
	assert(pPlayer);
    assert(!m_pPlayer);

    m_pPlayer = pPlayer;

	bResult = true;
Exit0:
	return bResult;
}

void KAchievement::UnInit()
{
    memset(m_Achievement, 0, sizeof(m_Achievement));
    m_pPlayer = NULL;
    m_nAchievementPoint = 0;
}

BOOL KAchievement::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pLoadBuf);
    {
        const T3DB::KPB_ACHIEVEMENT_DATA& achievementData = pLoadBuf->achievementdata();
        m_nAchievementPoint = achievementData.achievementpoint();
        const std::string& stateData = achievementData.statedata();
        memcpy(m_Achievement, stateData.data(), stateData.size());

        for (int i = 0; i < achievementData.progressing_size(); ++i)
        {
            const T3DB::KPB_ACHIEVEMENT_ITEM_PROGRESS_DATA& progressingItem = achievementData.progressing(i);
            DWORD   dwAchievementID = progressingItem.achievementid();
            int     nCurValue       = progressingItem.curvalue();
            
            m_mapProgressingAchievement[dwAchievementID] = nCurValue;
        }
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	T3DB::KPB_ACHIEVEMENT_DATA* pData = NULL;
    int nAchievementCount = 0;
    size_t uByteCount = 0;
    T3DB::KPB_ACHIEVEMENT_ITEM_PROGRESS_DATA* pT3DBProgressingData = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);
	
    pData = pSaveBuf->mutable_achievementdata();
    KGLOG_PROCESS_ERROR(pData);

    nAchievementCount = g_pSO3World->m_Settings.m_AchievementSettings.GetAchievementCount();
    uByteCount = (nAchievementCount / UINT32_BIT_COUNT + 1) * (UINT32_BIT_COUNT / CHAR_BIT);

    pData->set_achievementpoint(m_nAchievementPoint);
    pData->set_statedata((void*)m_Achievement, uByteCount);

    for (KMAP_PROGRESSING_ACHIEVEMENT::iterator it = m_mapProgressingAchievement.begin(); it != m_mapProgressingAchievement.end(); ++it)
    {
        pT3DBProgressingData = pData->add_progressing();
        pT3DBProgressingData->set_achievementid(it->first);
        pT3DBProgressingData->set_curvalue(it->second);
    }

	bResult = true;
Exit0:
	return bResult;
}

void KAchievement::OnEvent(int nEvent, int (&Params)[cdMaxEventParamCount])
{
    BOOL bRetCode = false;
    KMAP_ACHIEVEMENT_UPDATOR::iterator it;

    KGLOG_PROCESS_ERROR(nEvent > peInvalid && nEvent < peTotal);

    it = m_mapValueUpdator.find(nEvent);
    if (it != m_mapValueUpdator.end())
    {
        bRetCode = DoUpdate(it->second, nEvent, Params);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    EventCallbackInScript(nEvent, Params);
Exit0:
    return;
}

BOOL KAchievement::DoDailyReset()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KVEC_DAILY_ACHIEVEMENTS* pVecAchievement = NULL;
    KAchievementSettingsItem* pAchievementSettingsItem = NULL;

    pVecAchievement = g_pSO3World->m_Settings.m_AchievementSettings.GetDailyAchievement();
    KGLOG_PROCESS_ERROR(pVecAchievement);

    for (KVEC_DAILY_ACHIEVEMENTS::iterator it = pVecAchievement->begin(); it != pVecAchievement->end(); ++it)
    {
        pAchievementSettingsItem = *it;
        KGLOG_PROCESS_ERROR(pAchievementSettingsItem);

        bRetCode = ResetOne(pAchievementSettingsItem->dwID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::DoWeeklyReset()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KVEC_WEEKLY_ACHIEVEMENTS* pVecAchievement = NULL;
    KAchievementSettingsItem* pAchievementSettingsItem = NULL;

    pVecAchievement = g_pSO3World->m_Settings.m_AchievementSettings.GetWeeklyAchievement();
    KGLOG_PROCESS_ERROR(pVecAchievement);

    for (KVEC_WEEKLY_ACHIEVEMENTS::iterator it = pVecAchievement->begin(); it != pVecAchievement->end(); ++it)
    {
        pAchievementSettingsItem = *it;
        KGLOG_PROCESS_ERROR(pAchievementSettingsItem);

        bRetCode = ResetOne(pAchievementSettingsItem->dwID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KAchievement::ResetOne(DWORD dwAchievementID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
    bRetCode = ResetState(dwAchievementID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = ResetProgressing(dwAchievementID);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::UpdateAchievementValue(DWORD dwAchievementID, int nEvent, int (&Params)[cdMaxEventParamCount])
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KAchievementSettingsItem* pAchievementInfo = NULL;

    bRetCode = IsSpecifiedEvent(dwAchievementID, nEvent, Params);
    KG_PROCESS_SUCCESS(!bRetCode);

    bRetCode = IsAchievementFinished(dwAchievementID);
    KG_PROCESS_SUCCESS(bRetCode);
   
    bRetCode = IsAchievementValueFinished(dwAchievementID);
    KG_PROCESS_SUCCESS(bRetCode);

    pAchievementInfo = g_pSO3World->m_Settings.m_AchievementSettings.GetItem(dwAchievementID);
    KGLOG_PROCESS_ERROR(pAchievementInfo);

    switch(pAchievementInfo->nValueUpdateType)
    {
    case KEVENT_VALUE_UPDATE_ADDV:
        m_mapProgressingAchievement[dwAchievementID] += pAchievementInfo->nValueUpdateParam;
        break;
    case KEVENT_VALUE_UPDATE_ADDP:
        KGLOG_PROCESS_ERROR(pAchievementInfo->nValueUpdateParam >= 0 && pAchievementInfo->nValueUpdateParam < countof(Params));
        m_mapProgressingAchievement[dwAchievementID] += Params[pAchievementInfo->nValueUpdateParam];
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    g_PlayerServer.DoSyncOneAchievementProgress(m_pPlayer, dwAchievementID, m_mapProgressingAchievement[dwAchievementID]);

    if (pAchievementInfo->bAutoFinish)
    {
        bRetCode = IsAchievementValueFinished(dwAchievementID);
        if (!bRetCode)
            goto Exit1;

        bRetCode = FinishAchievement(dwAchievementID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::FinishAchievement(DWORD dwAchievementID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nCurValue = 0;
    KSystemMailTxt* pSysMailTxt = NULL;
    KMAP_PROGRESSING_ACHIEVEMENT::iterator it;
    KAchievementSettingsItem* pAchievementInfo = NULL;
    int nAchievementIndex = (dwAchievementID - 1) / UINT32_BIT_COUNT;

    pAchievementInfo = g_pSO3World->m_Settings.m_AchievementSettings.GetItem(dwAchievementID);
    KGLOG_PROCESS_ERROR(pAchievementInfo);

    bRetCode = IsAchievementFinished(dwAchievementID);
    KGLOG_PROCESS_ERROR(!bRetCode);

    bRetCode = IsAchievementValueFinished(dwAchievementID);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = ResetProgressing(dwAchievementID);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_nAchievementPoint += pAchievementInfo->nAchievementPoint;
    g_PlayerServer.DoSyncAchievementPoint(m_pPlayer, m_nAchievementPoint);

    assert(nAchievementIndex < countof(m_Achievement));
    m_Achievement[nAchievementIndex] |= (uint32_t)1 << ((dwAchievementID - 1) % UINT32_BIT_COUNT);
    g_PlayerServer.DoSyncOneAchievementState(m_pPlayer, dwAchievementID, true);

    if (pAchievementInfo->nAwardItemCount > 0)
    {
        pSysMailTxt = g_pSO3World->m_Settings.m_SystemMailMgr.GetSystemMailTxt(KMAIL_ACHIEVEMENT);
        KGLOG_PROCESS_ERROR(pSysMailTxt);

        bRetCode = m_pPlayer->SafeAddItem(
            pAchievementInfo->dwAwardItemType, pAchievementInfo->dwAwardItemIndex, 
            pAchievementInfo->nAwardItemCount, pAchievementInfo->nAwardItemValuePoint,
            pSysMailTxt->m_szMailTitl, pSysMailTxt->m_szSendName, pSysMailTxt->m_szMailTxt
        );
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    {
        KMessage67_Para cParam;
        cParam.dwAchievementID = pAchievementInfo->dwID;
        g_PlayerServer.DoDownwardNotify(m_pPlayer, KMESSAGE_FINISH_ACHIEVEMENT, &cParam, sizeof(cParam));
    }

    if (pAchievementInfo->bMessageToFriend)
    {
        KMessage78_Para cParam;
        memcpy(cParam.szPlayerName, m_pPlayer->m_szName, sizeof(cParam.szPlayerName));
        cParam.dwAchievementID = pAchievementInfo->dwID;
        g_LSClient.DoSendMessageToAllFriend(m_pPlayer->m_dwID, KMESSAGE_FINISH_ACHIEVEMENT_TO_FRIEND, &cParam, sizeof(cParam));        
    }

    if (pAchievementInfo->bMessageToServer)
    {
        KMessage79_Para cParam;
        memcpy(cParam.szPlayerName, m_pPlayer->m_szName, sizeof(cParam.szPlayerName));
        cParam.dwAchievementID = pAchievementInfo->dwID;
        g_LSClient.DoBroadcastMessage(KMESSAGE_FINISH_ACHIEVEMENT_TO_SERVER, &cParam, sizeof(cParam));
    }

    if (pAchievementInfo->bMessageToClub && m_pPlayer->m_dwClubID)
    {
        KMessage80_Para cParam;
        memcpy(cParam.szPlayerName, m_pPlayer->m_szName, sizeof(cParam.szPlayerName));
        cParam.dwAchievementID = pAchievementInfo->dwID;
        g_LSClient.DoSendMessageToClub(m_pPlayer->m_dwClubID, KMESSAGE_FINISH_ACHIEVEMENT_TO_CLUB, &cParam, sizeof(cParam));
    }

    m_pPlayer->OnEvent(peFinishAchievement, pAchievementInfo->nAchievementType,  dwAchievementID);
    g_pSO3World->m_Settings.m_HeroTemplate.ReportUnLockHero(m_pPlayer, UNLOCK_TYPE_ACHIEVEMENT, dwAchievementID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KAchievement::BuildEventMap()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    DWORD dwMaxAchievementID = ERROR_ID;
    KAchievementSettingsItem* pAchievementInfo = NULL;

    dwMaxAchievementID = g_pSO3World->m_Settings.m_AchievementSettings.GetMaxAchievementID();
	for (DWORD dwAchievementID = 1; dwAchievementID <= dwMaxAchievementID; ++dwAchievementID)
	{
        pAchievementInfo = g_pSO3World->m_Settings.m_AchievementSettings.GetItem(dwAchievementID);
        if(!pAchievementInfo)
            continue;

        bRetCode = BuildEventMapOf(pAchievementInfo);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::BuildEventMapOf(KAchievementSettingsItem* pAchievementInfo)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nEvent = 0;

    KGLOG_PROCESS_ERROR(pAchievementInfo);

    KG_PROCESS_SUCCESS(pAchievementInfo->cEventCondition.nEvent == peInvalid);

    if (pAchievementInfo->nAchievementType != KACHIEVEMENT_TYPE_DAILY && pAchievementInfo->nAchievementType != KACHIEVEMENT_TYPE_WEEKLY)
    {
        bRetCode = IsAchievementFinished(pAchievementInfo->dwID);
        KG_PROCESS_SUCCESS(bRetCode);
    }

    m_mapValueUpdator[pAchievementInfo->cEventCondition.nEvent].push_back(pAchievementInfo->dwID);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KAchievement::IsAchievementFinished(DWORD dwAchievementID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    size_t uIndexOfUInt32 = 0;
    size_t uOffsetInUInt32 = 0;

    KGLOG_PROCESS_ERROR(dwAchievementID);

    uIndexOfUInt32 = (dwAchievementID - 1) /  UINT32_BIT_COUNT;
    KGLOG_PROCESS_ERROR(uIndexOfUInt32 < countof(m_Achievement));

    uOffsetInUInt32 = (dwAchievementID - 1) %  UINT32_BIT_COUNT;

    if (m_Achievement[uIndexOfUInt32] & ((uint32_t)1 << uOffsetInUInt32))
        bResult = true;

Exit0:
	return bResult;
}

void KAchievement::Refresh(time_t nLastDailyRefreshTime)
{
    BOOL bRetCode = false;

    bRetCode = g_IsSameWeek(nLastDailyRefreshTime - DAILY_VARIABLE_REFRESH_TIME, g_pSO3World->m_nCurrentTime - DAILY_VARIABLE_REFRESH_TIME);
    if (!bRetCode)
    {
        bRetCode = DoWeeklyReset();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = g_IsSameDay(nLastDailyRefreshTime - DAILY_VARIABLE_REFRESH_TIME, g_pSO3World->m_nCurrentTime - DAILY_VARIABLE_REFRESH_TIME);
    if (!bRetCode)
    {
        bRetCode = DoDailyReset();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

Exit0:
    return;
}

BOOL KAchievement::DoSyncAchievementData()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int nAchievementCount = 0;
    size_t uSize = 0;

    g_PlayerServer.DoSyncAchievementPoint(m_pPlayer, m_nAchievementPoint);

    nAchievementCount = g_pSO3World->m_Settings.m_AchievementSettings.GetAchievementCount();

    uSize = (nAchievementCount / UINT32_BIT_COUNT + 1) * (UINT32_BIT_COUNT / CHAR_BIT);
	g_PlayerServer.DoSyncAllAchievementState(m_pPlayer, m_Achievement, uSize);

    for (KMAP_PROGRESSING_ACHIEVEMENT::iterator it = m_mapProgressingAchievement.begin(); it != m_mapProgressingAchievement.end(); ++it)
        g_PlayerServer.DoSyncOneAchievementProgress(m_pPlayer, it->first, it->second);
    
	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::DoUpdate(KVEC_ACHIEVEMENT_UPDATOR vecUpdator, int nEvent, int (&Params)[cdMaxEventParamCount])
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
    for (KVEC_ACHIEVEMENT_UPDATOR::iterator it = vecUpdator.begin(); it != vecUpdator.end(); ++it)
    {
        bRetCode = UpdateAchievementValue(*it, nEvent, Params);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::ClientIncreaseValue(DWORD dwAchievementID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KAchievementSettingsItem* pAchievementInfo = NULL;
    KAchievement* pAchievement = NULL;

    pAchievementInfo = g_pSO3World->m_Settings.m_AchievementSettings.GetItem(dwAchievementID);
    KGLOG_PROCESS_ERROR(pAchievementInfo);

    bRetCode = IsAchievementFinished(dwAchievementID);
    KGLOG_PROCESS_ERROR(!bRetCode);

    bRetCode = IsAchievementValueFinished(dwAchievementID);
    KGLOG_PROCESS_ERROR(!bRetCode);

    KGLOG_PROCESS_ERROR(pAchievementInfo->cEventCondition.nEvent > peInvalid && pAchievementInfo->cEventCondition.nEvent < peClientEnd);

    KGLOG_PROCESS_ERROR(pAchievementInfo->nValueUpdateType == KEVENT_VALUE_UPDATE_ADDV && pAchievementInfo->nValueUpdateParam == 1);

    ++m_mapProgressingAchievement[dwAchievementID];
    if (pAchievementInfo->bAutoFinish)
    {
        FinishAchievement(dwAchievementID);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::ResetState(DWORD dwAchievementID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    size_t uIndexOfUInt32 = 0;
    size_t uOffsetInUInt32 = 0;

    KGLOG_PROCESS_ERROR(dwAchievementID);

    uIndexOfUInt32 = (dwAchievementID - 1) /  UINT32_BIT_COUNT;
    KGLOG_PROCESS_ERROR(uIndexOfUInt32 < countof(m_Achievement));

    uOffsetInUInt32 = (dwAchievementID - 1) %  UINT32_BIT_COUNT;

    if (m_Achievement[uIndexOfUInt32] & ((uint32_t)1 << uOffsetInUInt32))
    {
        m_Achievement[uIndexOfUInt32] &= ~((uint32_t)1 << uOffsetInUInt32);
        g_PlayerServer.DoSyncOneAchievementState(m_pPlayer, dwAchievementID, false);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::ResetProgressing(DWORD dwAchievementID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KMAP_PROGRESSING_ACHIEVEMENT::iterator it;

    it = m_mapProgressingAchievement.find(dwAchievementID);
    if (it != m_mapProgressingAchievement.end())
    {
        m_mapProgressingAchievement.erase(it);
        g_PlayerServer.DoSyncOneAchievementProgress(m_pPlayer, dwAchievementID, 0);
    }
	
	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::EventCallbackInScript(int nEvent, int (&Params)[cdMaxEventParamCount])
{	
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nTopIndex = 0;
    DWORD dwScriptID = ERROR_ID;
    const static char* pcszScriptName = "scripts/global/achievement.lua";
    const static  char* pcszFunctionName = "OnPlayerEvent";

    dwScriptID = g_FileNameHash(pcszScriptName);
    KGLOG_PROCESS_ERROR(dwScriptID);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, pcszFunctionName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(m_pPlayer);
    g_pSO3World->m_ScriptManager.Push(nEvent);

    for (int i = 0; i < countof(Params); ++i)
    {
        g_pSO3World->m_ScriptManager.Push(Params[i]);
    }

    g_pSO3World->m_ScriptManager.Call(dwScriptID, pcszFunctionName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::IsSpecifiedEvent(DWORD dwAchievementID, int nEvent, int (&Params)[cdMaxEventParamCount])
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KAchievementSettingsItem* pAchievementInfo = NULL;

    pAchievementInfo = g_pSO3World->m_Settings.m_AchievementSettings.GetItem(dwAchievementID);
    KGLOG_PROCESS_ERROR(pAchievementInfo);

    bRetCode = pAchievementInfo->cEventCondition.CheckCondition(Params);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KAchievement::IsAchievementValueFinished(DWORD dwAchievementID)
{
    BOOL bResult  = false;
    int nCurValue = 0;
    KAchievementSettingsItem* pAchievementInfo = NULL;

    pAchievementInfo = g_pSO3World->m_Settings.m_AchievementSettings.GetItem(dwAchievementID);
    KGLOG_PROCESS_ERROR(pAchievementInfo);

    nCurValue = GetAchievenmentValue(dwAchievementID);

    KG_PROCESS_ERROR(nCurValue >= pAchievementInfo->nTargetValue);

    bResult = true;
Exit0:
    return bResult;
}

int KAchievement::GetAchievenmentValue(DWORD dwAchievementID)
{
    int nResult = 0;
	KMAP_PROGRESSING_ACHIEVEMENT::iterator it;

    it = m_mapProgressingAchievement.find(dwAchievementID);
    if (it != m_mapProgressingAchievement.end())
    {
        nResult = it->second;
    }
    
    return nResult;
}

BOOL KAchievement::SetAchievementValue(DWORD dwAchievementID, int nNewValue)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KAchievementSettingsItem* pAchievementInfo = NULL;

    pAchievementInfo = g_pSO3World->m_Settings.m_AchievementSettings.GetItem(dwAchievementID);
    KGLOG_PROCESS_ERROR(pAchievementInfo);

	bRetCode = IsAchievementFinished(dwAchievementID);
    KGLOG_PROCESS_ERROR(!bRetCode);

    m_mapProgressingAchievement[dwAchievementID] = nNewValue;
    g_PlayerServer.DoSyncOneAchievementProgress(m_pPlayer, dwAchievementID, nNewValue);

    if (pAchievementInfo->bAutoFinish)
    {
        FinishAchievement(dwAchievementID);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::EnsureEventMapBuild(DWORD dwAchievementID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KAchievementSettingsItem* pAchievementInfo = NULL;
	
    for (KMAP_ACHIEVEMENT_UPDATOR::iterator it = m_mapValueUpdator.begin(); it != m_mapValueUpdator.end(); ++it)
    {
        for (KVEC_ACHIEVEMENT_UPDATOR::iterator itSub = it->second.begin(); itSub != it->second.end(); ++itSub)
        {
            if (*itSub == dwAchievementID)
            {
                goto Exit1;
            }
        }
    }

    pAchievementInfo = g_pSO3World->m_Settings.m_AchievementSettings.GetItem(dwAchievementID);
    KGLOG_PROCESS_ERROR(pAchievementInfo);

    bRetCode = BuildEventMapOf(pAchievementInfo);
    KGLOG_PROCESS_ERROR(bRetCode);
	
Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KAchievement::ProcessAfterDataLoaded()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
    bRetCode = BuildEventMap();
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pPlayer->m_eConnectType == eLogin)
    {
        bRetCode = DoSyncAchievementData();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

	bResult = true;
Exit0:
	return bResult;
}
