#include "stdafx.h"
#include "KActivityCenter.h"
#include "KPlayerServer.h"
#include "KPlayer.h"
#include "KSO3World.h"

BOOL KActivityCenter::Init()
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	
    bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KActivityCenter::UnInit()
{
    m_ActivityData.clear();
}

BOOL KActivityCenter::OnActivityFinished(KPlayer* pPlayer, DWORD dwActivityID, int nFinishType)
{
	BOOL bResult = false;
	BOOL bRetCode = false;
    DWORD dwAwardTableID = ERROR_ID;
    KACTIVITY_ITEM* pAwardItem = NULL;

    assert(pPlayer);

    bRetCode = EnsureQuestFinished(pPlayer, dwActivityID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pAwardItem = GetAwardItem(dwActivityID, nFinishType);
    if (pAwardItem)
        g_PlayerServer.DoActivityCanAwardNotify(pPlayer, dwActivityID);

    PLAYER_LOG(pPlayer, "activity,finish,%u", dwActivityID);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KActivityCenter::OnCheckAndUpdateActivityStateRespond(KPlayer* pPlayer, DWORD dwActivityID, int nFinishType)
{
	BOOL bResult = false;
	BOOL bRetCode = false;
    KACTIVITY_ITEM* pAwardItem = NULL;
    KSystemMailTxt* pSysMailTxt = NULL;
    IItem* pItem = NULL;

    assert(pPlayer);

    pAwardItem = GetAwardItem(dwActivityID, nFinishType);
    KGLOG_PROCESS_ERROR(pAwardItem);

    pSysMailTxt = g_pSO3World->m_Settings.m_SystemMailMgr.GetSystemMailTxt(KMAIL_QUESTIONNAIRE);
    KGLOG_PROCESS_ERROR(pSysMailTxt); 

    bRetCode = pPlayer->SafeAddItem(pAwardItem->dwType, pAwardItem->dwIndex, pAwardItem->nCount, pAwardItem->nValuePoint, 
        pSysMailTxt->m_szMailTitl, pSysMailTxt->m_szSendName, pSysMailTxt->m_szMailTxt);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoActivityAwardedNotify(pPlayer, dwActivityID, KACTIVIY_AWARD_RET_SUCCESS);

    PLAYER_LOG(pPlayer, "activity,questionnaireaward");

	bResult = true;
Exit0:
	return bResult;
}

BOOL KActivityCenter::LoadData()
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	ITabFile* piTabFile = NULL;
    KActivity* pActivity = NULL;
    DWORD dwActivityID = ERROR_ID;
    KACTIVITY_ITEM* pAwardItem = NULL;
    char szValue[MAX_PATH] = "";
    char szTemp[MAX_PATH];
    
    piTabFile = g_OpenTabFile(SETTING_DIR"/activity/Activity.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); ++nRowIndex) 
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)&dwActivityID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(dwActivityID > 0);

        pActivity = &m_ActivityData[dwActivityID];
        pActivity->dwActivityID = dwActivityID;

        bRetCode = piTabFile->GetString(nRowIndex, "BeginTime", "", szTemp, sizeof(szTemp));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        szTemp[countof(szTemp) - 1] = '\0';

        bRetCode = g_StringToTime(szTemp, pActivity->nBeginTime);
        KGLOG_PROCESS_ERROR(bRetCode);
        szTemp[countof(szTemp) - 1] = '\0';

        bRetCode = piTabFile->GetString(nRowIndex, "EndTime", "", szTemp, sizeof(szTemp));
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = g_StringToTime(szTemp, pActivity->nEndTime);
        KGLOG_PROCESS_ERROR(bRetCode);

        for (int i = 0; i < countof(pActivity->Award); ++i)
        {
            pAwardItem = &pActivity->Award[i];

            sprintf(szTemp, "AwardItem%d", i + 1);
            bRetCode = piTabFile->GetString(nRowIndex, szTemp, "", szValue, sizeof(szValue));
            KGLOG_PROCESS_ERROR(bRetCode);
            szValue[countof(szValue) - 1]  = '\0';

            bRetCode = sscanf(szValue, "%u,%u,%d,%d", &pAwardItem->dwType, &pAwardItem->dwIndex, &pAwardItem->nCount, &pAwardItem->nValuePoint);
            KGLOG_PROCESS_ERROR(bRetCode == 4);
        }

        bRetCode = piTabFile->GetInteger(nRowIndex, "QuestID", 0, (int*)&pActivity->dwQuestID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = m_TesterPoint.Load("activity/TesterPoint.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_TesterAwardConfig.Load("activity/TesterAwardConfig.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_LevelupAwardConfig.Load("activity/LevelupAwardConfig.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
	return bResult;
}

KACTIVITY_ITEM*  KActivityCenter::GetAwardItem(DWORD dwActivityID, int nFinishType)
{
    KACTIVITY_ITEM* pResult = NULL;
    KMAP_ACTIVITY::iterator it;
    time_t timeNow = g_pSO3World->m_nCurrentTime;
    struct tm* p = localtime(&timeNow);
    int nTimeNow = mktime(p);

    it = m_ActivityData.find(dwActivityID);
    KG_PROCESS_ERROR(it != m_ActivityData.end());

    KG_PROCESS_ERROR(nTimeNow >= it->second.nBeginTime && nTimeNow <= it->second.nEndTime);

    KGLOG_PROCESS_ERROR(nFinishType >= 0 && nFinishType < countof(it->second.Award));

    pResult = &it->second.Award[nFinishType];
Exit0:
    return pResult;
}

BOOL KActivityCenter::EnsureQuestFinished(KPlayer* pPlayer, DWORD dwActivityID)
{
	BOOL bResult = false;
    BOOL bRetCode = false;
	KActivity* pActivity = NULL;
    KMAP_ACTIVITY::iterator it;
    DWORD dwQuestID = ERROR_ID;
    QUEST_STATE eCurState = qsInvalid;
    
    assert(pPlayer);
    it = m_ActivityData.find(dwActivityID);
    KGLOG_PROCESS_ERROR(it != m_ActivityData.end());

    dwQuestID = it->second.dwQuestID;
    KG_PROCESS_SUCCESS(!dwQuestID);
    
    eCurState = pPlayer->m_QuestList.GetQuestState(dwQuestID);
    KG_PROCESS_SUCCESS(eCurState == qsFinished);

    bRetCode = pPlayer->m_QuestList.SetQuestState(dwQuestID, qsFinished); 
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoSyncOneQuestState(pPlayer->m_nConnIndex, dwQuestID, qsFinished);

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

int KActivityCenter::GetTesterAwardLevel(KPlayer* pPlayer)
{
    int  nResult  = 0;
    BOOL bRetCode = false;
    int  nGetAwardFlag = 0;
    KTesterPoint* pTesterPoint = NULL;
    KTesterAwardConfig* pAward = NULL;

    assert(pPlayer);

    bRetCode = pPlayer->m_PlayerValue.GetValue(PLAYER_VALUE_TESTER_AWARD, nGetAwardFlag);
    KGLOG_PROCESS_ERROR(bRetCode);
    KGLOG_PROCESS_ERROR(nGetAwardFlag == 0);

    pTesterPoint = m_TesterPoint.GetByID(pPlayer->m_szAccount);
    KG_PROCESS_ERROR(pTesterPoint);

    pAward = m_TesterAwardConfig.GetLastSmallItem(pTesterPoint->nPoint);
    KGLOG_PROCESS_ERROR(pAward);

    nResult = pAward->nLevel;
Exit0:
    return nResult;
}

BOOL KActivityCenter::GetTesterAward(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nGetAwardFlag = 0;
    KTesterPoint* pTesterPoint = NULL;
    KTesterAwardConfig* pAward = NULL;

    assert(pPlayer);

    bRetCode = pPlayer->m_PlayerValue.GetValue(PLAYER_VALUE_TESTER_AWARD, nGetAwardFlag);
    KGLOG_PROCESS_ERROR(bRetCode);
    KG_PROCESS_ERROR(nGetAwardFlag == 0);

    pTesterPoint = m_TesterPoint.GetByID(pPlayer->m_szAccount);
    KG_PROCESS_ERROR(pTesterPoint);

    pAward = m_TesterAwardConfig.GetLastSmallItem(pTesterPoint->nPoint);
    KGLOG_PROCESS_ERROR(pAward);

    // 执行发奖
    bRetCode = pPlayer->m_PlayerValue.SetValue(PLAYER_VALUE_TESTER_AWARD, 1);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->AddItemByMail(
        pAward->nAwardItemTabType, pAward->nAwardItemTabIndex, pAward->nAwardItemCount, 0, 
        pAward->szTitle, "system", pAward->szText
    );
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KActivityCenter::GetLevelupAward(KPlayer* pPlayer, int nLevel)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KLevelupAwardConfig* pAward = NULL;
    int nModLevel = nLevel / 10;
    int nPlayerValueIndex = 0;
    int nGetAwardFlag = 0;

    assert(pPlayer);

    KGLOG_PROCESS_ERROR(nModLevel > 0);
    KGLOG_PROCESS_ERROR(pPlayer->m_nLevel >= nLevel);

    pAward = m_LevelupAwardConfig.GetByID(nModLevel * 10);
    KGLOG_PROCESS_ERROR(pAward);

    nPlayerValueIndex = KPLAYER_VALUE_ID_LEVEL_AWARD_10 + nModLevel - 1;
    KGLOG_PROCESS_ERROR(nPlayerValueIndex <= KPLAYER_VALUE_ID_LEVEL_AWARD_80);

    bRetCode = pPlayer->m_PlayerValue.GetValue(nPlayerValueIndex, nGetAwardFlag);
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLOG_PROCESS_ERROR(nGetAwardFlag == 0); // 已经领过
    
    bRetCode = g_pSO3World->m_AwardMgr.CanAwardAll(pPlayer, pAward->nAwardID);
    if (!bRetCode)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_NOT_ENOUGH_BAG);
        goto Exit0;
    }

    // 执行发奖
    bRetCode = pPlayer->m_PlayerValue.SetValue(nPlayerValueIndex, 1);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_AwardMgr.AwardAll(pPlayer, pAward->nAwardID, "levelup award");
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KActivityCenter::ViewOnlineAward(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nOldSeed = 0;
    int  nRandomSeed = 0;
    KAWARD_ITEM* pTempItem = NULL;
    KAWARD_ITEM* pRealAwardItem = NULL;
    int nRealAwardItemIndex = 0;
    KAWARD_ITEM* apAllAwardItems[cdOnlineAwardItemCount] = {0};
    int  nTargetItemIndex = 0;

    assert(pPlayer);

    nRandomSeed = (g_pSO3World->m_nCurrentTime / 86400 + pPlayer->m_nOnlineGetAwardTimes) * (pPlayer->m_nOnlineGetAwardTimes + 1) * (pPlayer->m_dwID + 1);

    nOldSeed = g_GetRandomSeed();
    g_RandomSeed(nRandomSeed);

    pRealAwardItem = g_pSO3World->m_AwardMgr.GetAwardItem(g_pSO3World->m_Settings.m_ConstList.nRealAwardTableID, nRandomSeed); // 真正给玩家的奖励物品
    KGLOG_PROCESS_ERROR(pRealAwardItem);

    apAllAwardItems[cdOnlineAwardItemCount - 1] = pRealAwardItem;
    nRealAwardItemIndex = cdOnlineAwardItemCount - 1;

    // 剩下的用来显示
    for (int i = 0; i < cdOnlineAwardItemCount - 1; ++i)
    {
        pTempItem = g_pSO3World->m_AwardMgr.AwardRandomOne(g_pSO3World->m_Settings.m_ConstList.nUnRealAwardTableID);
        KGLOG_PROCESS_ERROR(pTempItem);

        apAllAwardItems[i] = pTempItem;
    }

    // 打乱顺序
    nTargetItemIndex = g_Random(cdOnlineAwardItemCount);

    std::swap(apAllAwardItems[nRealAwardItemIndex], apAllAwardItems[nTargetItemIndex]);
    nRealAwardItemIndex = nTargetItemIndex;

    g_PlayerServer.DoSyncOnlineAwardItems(pPlayer->m_nConnIndex, nRealAwardItemIndex, cdOnlineAwardItemCount, apAllAwardItems);

    bResult = true;
Exit0:
    if (nOldSeed)
    {
        g_RandomSeed(nOldSeed);
        nOldSeed = 0;
    }
    return bResult;
}

BOOL KActivityCenter::GetOnlineAward(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nRandomSeed = 0;
    KAWARD_ITEM* pRealAwardItem = NULL;
    int nValuePoint = 0;

    assert(pPlayer);

    KGLOG_PROCESS_ERROR(g_pSO3World->m_nCurrentTime >= pPlayer->m_nOnlineAwardTime);
    KGLOG_PROCESS_ERROR(pPlayer->m_nOnlineGetAwardTimes < cdOnlineAwardTimes);

    nRandomSeed = (g_pSO3World->m_nCurrentTime / 86400 + pPlayer->m_nOnlineGetAwardTimes) * (pPlayer->m_nOnlineGetAwardTimes + 1) * (pPlayer->m_dwID + 1);

    pRealAwardItem = g_pSO3World->m_AwardMgr.GetAwardItem(g_pSO3World->m_Settings.m_ConstList.nRealAwardTableID, nRandomSeed); // 真正给玩家的奖励物品
    KGLOG_PROCESS_ERROR(pRealAwardItem);

    if (pRealAwardItem->dwTabType != 0)
    {
        bRetCode = pPlayer->CanAddItem(pRealAwardItem->dwTabType, pRealAwardItem->dwIndex, pRealAwardItem->nStackNum);
        if (!bRetCode)
        {
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_NOT_ENOUGH_BAG);
            goto Exit0;
        }
    }
    else if (pRealAwardItem->nMoney > 0)
    {
        bRetCode = pPlayer->m_MoneyMgr.CanAddMoney(pRealAwardItem->eMoneyType, pRealAwardItem->nMoney);
        if (!bRetCode)
        {
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_MONEY_FAILED);
            goto Exit0;
        }
    }
    else
    {
        KGLOG_PROCESS_ERROR(false);
    }
    
    // 执行发奖
    ++pPlayer->m_nOnlineGetAwardTimes;
    pPlayer->m_nOnlineAwardTime = pPlayer->GetNextOnlineAwardTime();
    g_PlayerServer.DoSyncOnlineAwardInfo(pPlayer->m_nConnIndex, pPlayer->m_nOnlineGetAwardTimes, pPlayer->m_nOnlineAwardTime);

    if (pRealAwardItem->dwTabType != 0)
    {
        IItem* pItem = NULL;

        bRetCode = g_pSO3World->m_AwardMgr.GetAwardItemValuePoint(pRealAwardItem, nValuePoint);
        KGLOG_PROCESS_ERROR(bRetCode);

        pItem = pPlayer->AddItem(pRealAwardItem->dwTabType, pRealAwardItem->dwIndex, pRealAwardItem->nStackNum, nValuePoint);
        if (pItem)
        {
            PLAYER_LOG(
                pPlayer, "item,additem,%u-%u,%d,%d,%d,%s,%d",
                pRealAwardItem->dwTabType, pRealAwardItem->dwIndex, pRealAwardItem->nStackNum, nValuePoint, pItem->GetQuality(),
                "onlineaward", pPlayer->m_dwMapID
            );
        }
    }
    else if (pRealAwardItem->nMoney > 0)
    {
        bRetCode = pPlayer->m_MoneyMgr.AddMoney(pRealAwardItem->eMoneyType, pRealAwardItem->nMoney);
        KGLOG_PROCESS_ERROR(bRetCode);
        PLAYER_LOG(pPlayer, "money,addmoney,%d,%s,%d-%d,%d,%d", pRealAwardItem->eMoneyType, "onlineaward", 0, 0, 0, pRealAwardItem->nMoney);
    }
    
    bResult = true;
Exit0:
    return bResult;
}
