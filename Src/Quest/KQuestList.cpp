#include "stdafx.h"
#include "KQuestList.h"
#include "KRelayClient.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KMath.h"
#include "KScene.h"
#include "GlobalMacro.h"
#include "KTools.h"

BOOL KQuestList::Init(KPlayer* pPlayer)
{
	assert(pPlayer);

	m_nAcceptedCount    = 0;
	m_pPlayer           = pPlayer;

	memset(m_byQuestStateList, 0, sizeof(m_byQuestStateList));
	memset(m_AcceptedQuestList, 0, sizeof(m_AcceptedQuestList));
    memset(m_DailyQuest, 0, sizeof(m_DailyQuest));

    return true;
}

void KQuestList::UnInit()
{
    m_mapQuestValueIncreasor.clear();
}

BOOL KQuestList::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;

    assert(pLoadBuf);

    const T3DB::KPBQuestList& QuestList  = pLoadBuf->questlist();
    const std::string& strState = QuestList.queststatelist();

    bRetCode = LoadStateFromProtoBuf(&strState);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pPlayer->m_eConnectType == eLogin)
        g_PlayerServer.DoSyncQuestData(m_pPlayer->m_nConnIndex, m_pPlayer->m_dwID, (const BYTE*)strState.data(), strState.size(), (BYTE)qdtQuestState);

    bRetCode = LoadListFromProtoBuf(&QuestList);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pPlayer->m_eConnectType == eLogin)
        g_PlayerServer.DoSyncQuestList(m_pPlayer);

    bRetCode = LoadDailyFromProtoBuf(&QuestList);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pPlayer->m_eConnectType == eLogin)
        g_PlayerServer.DoSyncDailyQuestData(m_pPlayer);

    bResult = true;
Exit0:
    return bResult; 
}
BOOL KQuestList::LoadStateFromProtoBuf(const std::string*   pStrState)
{
    BOOL        bResult         = false;
    size_t      uLeftSize       = 0;
    const BYTE* pbyOffset       = NULL;
    int         nDwordBitCount  = sizeof(DWORD) * CHAR_BIT;

    assert(pStrState);

    pbyOffset = (const BYTE*)pStrState->data();
    uLeftSize = pStrState->size();

    for (int nQuestIndex = 0; nQuestIndex < MAX_QUEST_COUNT / nDwordBitCount; nQuestIndex++)
    {
        DWORD dwQuestState = 0;

        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(DWORD));
        dwQuestState = *(DWORD*)pbyOffset;
        uLeftSize -= sizeof(DWORD);
        pbyOffset += sizeof(DWORD);

        for (int nIndex = 0; nIndex < nDwordBitCount; nIndex++)
        {
            if (dwQuestState & (0x01 << nIndex))
            {
                m_byQuestStateList[nQuestIndex * nDwordBitCount + nIndex] = qsFinished;
            }
        }
    }

    bResult = true;
Exit0:
    return bResult;
}
BOOL KQuestList::LoadListFromProtoBuf(const T3DB::KPBQuestList*   pQuestList)
{
    BOOL    bResult             = false;
    int     nDBAcceptedCount    = 0;

    assert(pQuestList);

    nDBAcceptedCount = pQuestList->acceptedquestlist_size();

    KGLOG_PROCESS_ERROR(nDBAcceptedCount <= cdMaxAcceptQuestCount);

    // 载入已接任务表
    for (int i = 0; i < nDBAcceptedCount; ++i)
    {
        const T3DB::KPB_ACCEPT_QUEST& PBQuest = pQuestList->acceptedquestlist(i);
        KQuestInfo*         pQuestInfo  = NULL;
        int                 nQuestIndex = 0;

        pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(PBQuest.questid());
        if (pQuestInfo == NULL)
        {
            // 任务已经被策划删除了
            continue;
        }

        nQuestIndex = PBQuest.questindex();
        KGLOG_PROCESS_ERROR(nQuestIndex >= 0 && nQuestIndex < cdMaxAcceptQuestCount);

        m_AcceptedQuestList[nQuestIndex].pQuestInfo = pQuestInfo;

        for (int nIndex = 0; nIndex < countof(m_AcceptedQuestList[nQuestIndex].nQuestValue); nIndex++)
        {
            m_AcceptedQuestList[nQuestIndex].nQuestValue[nIndex] = PBQuest.questvalue(nIndex);
        }

        m_nAcceptedCount++;

        RegisterQuestValueProccessor(nQuestIndex);
    }

    bResult = true;
Exit0:
    return bResult;
}
BOOL KQuestList::LoadDailyFromProtoBuf(const T3DB::KPBQuestList*   pQuestList)
{
    BOOL                    bResult             = false;
    int                     nDailyQuestCount    = 0;

    assert(pQuestList);

    const T3DB::KPB_DAILY_QUEST_DATA& DailyQuestData = pQuestList->dailyquest();

    nDailyQuestCount = DailyQuestData.dailyquest_size();
    if (nDailyQuestCount > MAX_DAILY_QUEST_COUNT)
    {
        nDailyQuestCount = MAX_DAILY_QUEST_COUNT;
    }

    for (int i = 0; i < nDailyQuestCount; ++i)
    {
        const T3DB::KPB_DAILY_QUEST& DailyQuest = DailyQuestData.dailyquest(i);

        m_DailyQuest[i].dwQuestID = DailyQuest.questid();
        m_DailyQuest[i].nNextAcceptTime = (time_t)DailyQuest.nextaccepttime();

        if (m_DailyQuest[i].dwQuestID != 0 && m_DailyQuest[i].nNextAcceptTime == 0)
        {
            int nQuestIndex = GetQuestIndex(m_DailyQuest[i].dwQuestID);
            if (nQuestIndex == -1)
            {
                m_DailyQuest[i].dwQuestID = 0; // 任务已经不存在
            }
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL  KQuestList::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{   
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    T3DB::KPBQuestList*   pQuestList  = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pQuestList = pSaveBuf->mutable_questlist();

    bRetCode = SaveStateToProtoBuf(pQuestList);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = SaveListToProtoBuf(pQuestList);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = SaveDailyToProtoBuf(pQuestList);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}
BOOL  KQuestList::SaveStateToProtoBuf(T3DB::KPBQuestList*   pQuestList)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    int                 nDwordBitCount  = sizeof(DWORD) * CHAR_BIT;
    BYTE                byStateBuff[MAX_QUEST_COUNT/CHAR_BIT];
    BYTE*               pbyOffset       = byStateBuff;
    size_t              uBufferSize     = MAX_QUEST_COUNT/CHAR_BIT;
    size_t              uLeftSize       = uBufferSize;

    KGLOG_PROCESS_ERROR(pQuestList);

    for (int nQuestIndex = 0; nQuestIndex < MAX_QUEST_COUNT / nDwordBitCount; nQuestIndex++)
    {
        DWORD*  pdwData         = NULL;
        DWORD   dwQuestState    = 0;

        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(DWORD));
        pdwData = (DWORD*)pbyOffset;
        uLeftSize -= sizeof(DWORD);
        pbyOffset += sizeof(DWORD);

        for (int nIndex = 0; nIndex < nDwordBitCount; nIndex++)
        {
            if (m_byQuestStateList[nQuestIndex * nDwordBitCount + nIndex] == qsFinished)
            {
                dwQuestState |= (0x01 << nIndex);
            }
        }

        *pdwData = dwQuestState;
    }
    pQuestList->set_queststatelist(byStateBuff,uBufferSize - uLeftSize);
   
    bResult = true;
Exit0:
    return bResult;  
}

BOOL  KQuestList::SaveListToProtoBuf(T3DB::KPBQuestList*   pQuestList)
{
    BOOL    bResult             = false;
    T3DB::KPB_ACCEPT_QUEST* pAcceptQuest = NULL;

    KGLOG_PROCESS_ERROR(pQuestList);

    for (int nQuestIndex = 0; nQuestIndex < cdMaxAcceptQuestCount; nQuestIndex++)
    {
        if (m_AcceptedQuestList[nQuestIndex].pQuestInfo == NULL)
        {
            continue;
        }

        pAcceptQuest = pQuestList->add_acceptedquestlist();

        pAcceptQuest->set_questid(m_AcceptedQuestList[nQuestIndex].pQuestInfo->dwQuestID);
        pAcceptQuest->set_questindex(nQuestIndex);

        for (int nIndex = 0; nIndex < countof(m_AcceptedQuestList[nQuestIndex].nQuestValue); nIndex++)
        {
            pAcceptQuest->add_questvalue(m_AcceptedQuestList[nQuestIndex].nQuestValue[nIndex]);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KQuestList::SaveDailyToProtoBuf(T3DB::KPBQuestList*   pQuestList)
{
    BOOL                    bResult         = false;
    T3DB::KPB_DAILY_QUEST_DATA* pDailyQuestData = NULL;

    KGLOG_PROCESS_ERROR(pQuestList);

    pDailyQuestData = pQuestList->mutable_dailyquest();
    for (int i = 0; i < MAX_DAILY_QUEST_COUNT; ++i)
    {
        if (m_DailyQuest[i].dwQuestID == 0)
        {
            continue;
        }

        if (m_DailyQuest[i].nNextAcceptTime != 0 && g_pSO3World->m_nCurrentTime > m_DailyQuest[i].nNextAcceptTime)
        {
            continue;
        }

        T3DB::KPB_DAILY_QUEST* pDailyQuest = pDailyQuestData->add_dailyquest();
        pDailyQuest->set_questid(m_DailyQuest[i].dwQuestID);
        pDailyQuest->set_nextaccepttime(m_DailyQuest[i].nNextAcceptTime);
    }

    bResult = true;
Exit0:
    return bResult;
}

int KQuestList::GetDailyQuest(BYTE* pbyBuffer, size_t uBufferSize)
{
    size_t                  uLeftSize       = uBufferSize;
    KS2C_Sync_Daily_Quest*  pDailyQuestData = NULL;
    KDailyQuest*            pDailyQuest     = NULL;
    int                     nSaveIndex      = 0;
    
    assert(pbyBuffer);
    
    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KS2C_Sync_Daily_Quest));
    pDailyQuestData = (KS2C_Sync_Daily_Quest*)pbyBuffer;
    uLeftSize -= sizeof(KS2C_Sync_Daily_Quest);

    for (int i = 0; i < MAX_DAILY_QUEST_COUNT; ++i)
    {
        if (m_DailyQuest[i].dwQuestID == 0)
        {
            continue;
        }

        if (m_DailyQuest[i].nNextAcceptTime != 0 && g_pSO3World->m_nCurrentTime > m_DailyQuest[i].nNextAcceptTime)
        {
            continue;
        }
        
        KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KDailyQuest));

        pDailyQuest = &(pDailyQuestData->dailyQuest[nSaveIndex++]);
        
        pDailyQuest->wQuestID        = (WORD)m_DailyQuest[i].dwQuestID;
        pDailyQuest->nNextAcceptTime = m_DailyQuest[i].nNextAcceptTime;

        uLeftSize -= sizeof(KDailyQuest);
    }
    
Exit0:
    return nSaveIndex;
}

QUEST_RESULT_CODE KQuestList::CanAccept(KQuestInfo* pQuestInfo)
{
    QUEST_RESULT_CODE   nResult         = qrcFailed;
    QUEST_RESULT_CODE   eRetCode        = qrcFailed;
    BOOL                bRetCode        = false;
    BOOL                bShared         = false;
	int					nQuestIndex     = 0;
    int                 nFreeRoomSize   = 0;
    int                 nTime           = 0;
    int                 nOfferItemCount = 0;
    struct tm*          pLocalTime      = NULL;
    DWORD               dwExistAmount   = 0;
    
	KGLOG_PROCESS_ERROR(pQuestInfo);
	KGLOG_PROCESS_ERROR(pQuestInfo->dwQuestID < MAX_QUEST_COUNT);

    assert(m_pPlayer);

    // 检测是否已经接受该任务
	for (nQuestIndex = 0; nQuestIndex < cdMaxAcceptQuestCount; nQuestIndex++)
	{
		KG_PROCESS_ERROR_RET_CODE(
			m_AcceptedQuestList[nQuestIndex].pQuestInfo == NULL || 
			m_AcceptedQuestList[nQuestIndex].pQuestInfo->dwQuestID != pQuestInfo->dwQuestID,
			qrcAlreadyAcceptQuest
        );
	}
	
    KG_PROCESS_ERROR_RET_CODE(pQuestInfo->bAccept, qrcNoNeedAccept);

    if (pQuestInfo->nQuestType == qtRandom) // 随机任务
    {
        bRetCode = CanAcceptRandomQuest(pQuestInfo->dwQuestID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    
    if (pQuestInfo->bDaily)
    {
        eRetCode = CanAcceptDailyQuest(pQuestInfo);
        KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
    }

    eRetCode = CommonCheck(pQuestInfo);
    KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);

	// 判断任务是否已经满了。玩家任务满了，客户端也要有显示，所以这个条件客户端不检查
	nQuestIndex = 0;
	while ((nQuestIndex < cdMaxAcceptQuestCount) && m_AcceptedQuestList[nQuestIndex].pQuestInfo)
	{
		++nQuestIndex;
	}
	KG_PROCESS_ERROR_RET_CODE(nQuestIndex < cdMaxAcceptQuestCount, qrcQuestListFull);

	// 检测接任务时段限制
	pLocalTime = localtime(&g_pSO3World->m_nCurrentTime);
	nTime = pLocalTime->tm_hour * 3600 + pLocalTime->tm_min * 60 + pLocalTime->tm_sec;
	KG_PROCESS_ERROR_RET_CODE(nTime >= pQuestInfo->nStartTime, qrcTooEarlyTime);
	KG_PROCESS_ERROR_RET_CODE(nTime <= pQuestInfo->nEndTime, qrcTooLateTime);
	
    bRetCode = m_pPlayer->m_ItemList.m_PlayerPackage.CanAddItems(pQuestInfo->dwOfferItemType, pQuestInfo->nOfferItemIndex, QUEST_OFFER_ITEM_COUNT);
    KG_PROCESS_ERROR_RET_CODE(bRetCode, qrcNotEnoughFreeRoom);
    
	nResult = qrcSuccess;
Exit0:
	return nResult;
}

QUEST_RESULT_CODE KQuestList::Accept(DWORD dwQuestID, BOOL bIgnoreRules)
{
    QUEST_RESULT_CODE   	nResult     	= qrcFailed;
    QUEST_RESULT_CODE   	eRetCode     	= qrcFailed;
	BOOL                	bRetCode    	= false;
	KQuestInfo*         	pQuestInfo  	= NULL;
	int                 	nQuestIndex 	= 0;
    IItem*              	piItem			= NULL;
	const KItemProperty*	pItemProperty	= NULL;
    BOOL                	bDailyQuest		= false;

	KGLOG_PROCESS_ERROR_RET_CODE(dwQuestID < MAX_QUEST_COUNT, qrcErrorQuestID);

    assert(g_pSO3World);

	pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(dwQuestID);
	KGLOG_PROCESS_ERROR_RET_CODE(pQuestInfo, qrcCannotFindQuest);
    
	// 检测是否满足接任务的条件
	if (!bIgnoreRules)
	{
		eRetCode = CanAccept(pQuestInfo);
		KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
	}
    
    while ((nQuestIndex < cdMaxAcceptQuestCount) && m_AcceptedQuestList[nQuestIndex].pQuestInfo)
	{
		++nQuestIndex;
	}
	KG_PROCESS_ERROR_RET_CODE(nQuestIndex < cdMaxAcceptQuestCount, qrcQuestListFull);
             
    for (int nIndex = 0; nIndex < QUEST_OFFER_ITEM_COUNT; nIndex++)
    {
        if (pQuestInfo->dwOfferItemType[nIndex] == 0 || pQuestInfo->nOfferItemIndex[nIndex] == 0)
            break;

        m_pPlayer->AddItem(
            pQuestInfo->dwOfferItemType[nIndex], pQuestInfo->nOfferItemIndex[nIndex], 
            pQuestInfo->nOfferItemAmount[nIndex], 0
        );
    }

	// 添加任务
	memset(&(m_AcceptedQuestList[nQuestIndex]), 0, sizeof(KQuest));
	m_AcceptedQuestList[nQuestIndex].pQuestInfo = pQuestInfo;
	m_nAcceptedCount++;

    if (pQuestInfo->bDaily)
    {
        bRetCode = AcceptDailyQuest(pQuestInfo);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    g_PlayerServer.DoAcceptQuestRespond(m_pPlayer->m_nConnIndex, dwQuestID, nQuestIndex, qrcSuccess);

	if (pQuestInfo->dwScriptID)
	{	
		bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pQuestInfo->dwScriptID);
		if (bRetCode && g_pSO3World->m_ScriptManager.IsFuncExist(pQuestInfo->dwScriptID, SCRIPT_ON_ACCEPT_QUEST))
		{
			int nTopIndex = 0;
			g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

			g_pSO3World->m_ScriptManager.Push(m_pPlayer);
            g_pSO3World->m_ScriptManager.Push(dwQuestID);
            
			g_pSO3World->m_ScriptManager.Call(pQuestInfo->dwScriptID, SCRIPT_ON_ACCEPT_QUEST, 0);
			g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);
		}
	}
	
    RegisterQuestValueProccessor(nQuestIndex);

    PLAYER_LOG(m_pPlayer, "quest,accept,%u", dwQuestID);

	nResult = qrcSuccess;
Exit0:
    if (nResult != qrcSuccess && piItem)
    {
        g_pSO3World->m_ItemHouse.DestroyItem(piItem);
        piItem = NULL;
    }

    if (nResult != qrcSuccess)
    {    
        g_PlayerServer.DoAcceptQuestRespond(m_pPlayer->m_nConnIndex, dwQuestID, nQuestIndex, nResult);
    }

	return nResult;
}

QUEST_RESULT_CODE KQuestList::Finish(
    DWORD dwQuestID, int nPresentChoice1, int nPresentChoice2, BOOL bIgnoreRules
)
{
	QUEST_RESULT_CODE   nResult             = qrcFailed;
    QUEST_RESULT_CODE   eRetCode            = qrcFailed;
	BOOL                bRetCode            = false;
    KQuestInfo*         pQuestInfo          = NULL;
	int                 nQuestIndex         = -1;
    KScene*             pScene              = NULL;
    int                 nQuestCountAchID    = -1;

	KGLOG_PROCESS_ERROR_RET_CODE(dwQuestID < MAX_QUEST_COUNT, qrcErrorQuestID);
    KGLOG_PROCESS_ERROR(nPresentChoice1 >= 0 && nPresentChoice1 < QUEST_PARAM_COUNT);
    KGLOG_PROCESS_ERROR(nPresentChoice2 >= QUEST_PARAM_COUNT && nPresentChoice2 < cdQuestParamCount);
    
	// 取任务的详细信息
	pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(dwQuestID);
	KGLOG_PROCESS_ERROR_RET_CODE(pQuestInfo, qrcCannotFindQuest);

	// 检测该任务是否需要接后才能交
	if (pQuestInfo->bAccept)
	{
		nQuestIndex = GetQuestIndex(dwQuestID);
		KGLOG_PROCESS_ERROR_RET_CODE(nQuestIndex != -1, qrcNeedAccept);
	}

	// 检测是否已经完成
	if (!bIgnoreRules)
	{
		eRetCode = CanFinish(pQuestInfo);
		KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
	}

	// 检测能否成功发放任务奖励
    eRetCode = CanPresentItems(pQuestInfo, nPresentChoice1, nPresentChoice2);
	KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);

    bRetCode = CostQuestRequireItem(pQuestInfo, false);
    KGLOG_PROCESS_ERROR(bRetCode);

    //bRetCode = CostQuestOfferItem(pQuestInfo);
    //KGLOG_CHECK_ERROR(bRetCode);

	// 扣除金钱
	bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, -(pQuestInfo->nCostMoney));
	KGLOG_PROCESS_ERROR(bRetCode);

    if (pQuestInfo->nCostMoney > 0)
        PLAYER_LOG(m_pPlayer, "money,costmoney,%d,%s,%d-%d,%d,%d", emotMoney, "finishquest", 0, 0, 0, pQuestInfo->nCostMoney);

    if (!pQuestInfo->bRepeat)
    {
        m_byQuestStateList[dwQuestID] = qsFinished;
    }

	// 发放任务奖励
	eRetCode = Present(pQuestInfo, nPresentChoice1, nPresentChoice2);
	KGLOG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);

    if (pQuestInfo->bDaily)
        FinishDailyQuest(pQuestInfo);

	// 切换状态
	if (nQuestIndex != -1)
	{
        UnRegisterQuestValueProccessor(nQuestIndex);
		m_AcceptedQuestList[nQuestIndex].pQuestInfo = NULL;
		m_nAcceptedCount--;
	}

	if (pQuestInfo->dwCoolDownID != 0)
	{
		int nCDInterval = g_pSO3World->m_Settings.m_CoolDownList.GetCoolDownValue(pQuestInfo->dwCoolDownID);
		m_pPlayer->m_CDTimerList.ResetTimer(pQuestInfo->dwCoolDownID, nCDInterval);
	}
    
	g_PlayerServer.DoFinishQuestRespond(m_pPlayer->m_nConnIndex, qrcSuccess, nQuestIndex);

	if (pQuestInfo->dwScriptID)
	{
		bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pQuestInfo->dwScriptID);
		if (bRetCode && g_pSO3World->m_ScriptManager.IsFuncExist(pQuestInfo->dwScriptID, SCRIPT_ON_FINISH_QUEST))
		{
			int nTopIndex = 0;
			g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

			g_pSO3World->m_ScriptManager.Push(m_pPlayer);
			g_pSO3World->m_ScriptManager.Push(nPresentChoice1);
			g_pSO3World->m_ScriptManager.Push(nPresentChoice2);
			g_pSO3World->m_ScriptManager.Push(dwQuestID);

			g_pSO3World->m_ScriptManager.Call(pQuestInfo->dwScriptID, SCRIPT_ON_FINISH_QUEST, 0);
			g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);
		}
	}

    if (pQuestInfo->nQuestType == qtRandom)
    {
        FinishRandomQuest(dwQuestID);
    }

	PLAYER_LOG(m_pPlayer, "quest,finish,%u", dwQuestID);

	m_pPlayer->OnEvent(peFinishQuest, dwQuestID);
    g_pSO3World->m_Settings.m_HeroTemplate.ReportUnLockHero(m_pPlayer, UNLOCK_TYPE_QUEST, dwQuestID);

	nResult = qrcSuccess;
Exit0:
	if (nResult != qrcSuccess)
	{
		g_PlayerServer.DoFinishQuestRespond(m_pPlayer->m_nConnIndex, nResult, nQuestIndex);
	}
    
	return nResult;
}

QUEST_RESULT_CODE KQuestList::Cancel(int nQuestIndex)
{
   	BOOL              bRetCode   = false;
    DWORD             dwQuestID  = 0;
    KQuestInfo*       pQuestInfo = NULL;
	QUEST_RESULT_CODE nResult    = qrcFailed;

	KG_PROCESS_ERROR_RET_CODE(
        (nQuestIndex >= 0) && (nQuestIndex < cdMaxAcceptQuestCount), qrcErrorQuestIndex
    );

	//取任务的详细信息
	pQuestInfo = m_AcceptedQuestList[nQuestIndex].pQuestInfo;
	KGLOG_PROCESS_ERROR_RET_CODE(pQuestInfo, qrcCannotFindQuest);
    KGLOG_PROCESS_ERROR_RET_CODE(pQuestInfo->nQuestType != qtTrunk, qrcFailed);

	dwQuestID = pQuestInfo->dwQuestID;

    bRetCode = CostQuestRequireItem(pQuestInfo, true);
    KGLOG_PROCESS_ERROR(bRetCode);

    //bRetCode = CostQuestOfferItem(pQuestInfo);
    //KGLOG_PROCESS_ERROR(bRetCode);

	//删除任务
    UnRegisterQuestValueProccessor(nQuestIndex);
	m_AcceptedQuestList[nQuestIndex].pQuestInfo = NULL;
	--m_nAcceptedCount;

	if (pQuestInfo->dwScriptID)
	{
        bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pQuestInfo->dwScriptID);
		if (bRetCode && g_pSO3World->m_ScriptManager.IsFuncExist(pQuestInfo->dwScriptID, SCRIPT_ON_CANCEL_QUEST))
		{
			int nTopIndex = 0;
			g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

			g_pSO3World->m_ScriptManager.Push(m_pPlayer);
            g_pSO3World->m_ScriptManager.Push(dwQuestID);
			
            g_pSO3World->m_ScriptManager.Call(pQuestInfo->dwScriptID, SCRIPT_ON_CANCEL_QUEST, 0);
			g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);
		}
	}

    if (pQuestInfo->bDaily)
        ClearDailyQuest(pQuestInfo);

    PLAYER_LOG(m_pPlayer, "quest,cancel,%u", dwQuestID);

	nResult = qrcSuccess;
Exit0:
    g_PlayerServer.DoCancelQuestRespond(m_pPlayer->m_nConnIndex, nResult, nQuestIndex);
	return nResult;
}

BOOL KQuestList::ClearQuest(DWORD dwQuestID)
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    int             nQuestIndex     = -1;
    KQuestInfo*     pQuestInfo      = NULL;

	pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(dwQuestID);
	KG_PROCESS_ERROR(pQuestInfo);    

	m_byQuestStateList[dwQuestID] = qsUnfinished;

    bRetCode = CostQuestRequireItem(pQuestInfo, true);
    KGLOG_PROCESS_ERROR(bRetCode);

    //bRetCode = CostQuestOfferItem(pQuestInfo);
    //KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoClearQuest(m_pPlayer->m_nConnIndex, dwQuestID);

    nQuestIndex = GetQuestIndex(dwQuestID);
    if (nQuestIndex != -1)
    {
        assert(nQuestIndex >= 0 && nQuestIndex < cdMaxAcceptQuestCount);
        UnRegisterQuestValueProccessor(nQuestIndex);
        m_AcceptedQuestList[nQuestIndex].pQuestInfo = NULL;
        memset(m_AcceptedQuestList[nQuestIndex].nQuestValue, 0, sizeof(m_AcceptedQuestList[nQuestIndex].nQuestValue));
        m_nAcceptedCount--;
    }

    if (pQuestInfo->bDaily)
        ClearDailyQuest(pQuestInfo);

    bResult = true;
Exit0:
    return bResult;
}

QUEST_RESULT_CODE KQuestList::CanFinish(KQuestInfo* pQuestInfo)
{
	QUEST_RESULT_CODE   nResult         = qrcFailed;
	BOOL                bRetCode        = false;
	int                 nQuestIndex     = 0;
    KQuest*             pQuest          = NULL;
	int                 nDistance2      = 0;
    int                 nMoney          = 0;
    int                 nAddMoney       = 0;
    DWORD               dwTabType       = 0;
    DWORD               dwIndex         = 0;
    DWORD               dwCurrentAmount = 0;
    DWORD               dwRequireAmount = 0;
    KGItemInfo*         pItemInfo       = NULL;

	KGLOG_PROCESS_ERROR(m_pPlayer);
	KGLOG_PROCESS_ERROR(pQuestInfo);

    KGLOG_PROCESS_ERROR(!m_pPlayer->m_bIsLimited);

	KG_PROCESS_ERROR_RET_CODE(
        m_byQuestStateList[pQuestInfo->dwQuestID] == qsUnfinished || pQuestInfo->bRepeat, qrcErrorQuestState
    );
    
	nQuestIndex = GetQuestIndex(pQuestInfo->dwQuestID);
	if (nQuestIndex != -1)
	{
		pQuest = &(m_AcceptedQuestList[nQuestIndex]);
	}

	if (pQuestInfo->bAccept)
	{
		KG_PROCESS_ERROR_RET_CODE(nQuestIndex != -1, qrcNeedAccept);
	}
    else // 不用接的任务要检查一些条件
    {
        QUEST_RESULT_CODE   eRetCode     = qrcFailed;

        eRetCode = CommonCheck(pQuestInfo);
        KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
    }

	//Cooldown计时器
    bRetCode = m_pPlayer->m_CDTimerList.CheckTimer(pQuestInfo->dwCoolDownID);
    KG_PROCESS_ERROR_RET_CODE(bRetCode, qrcCooldown);

	if (pQuest)
	{
		//检测任务变量的数值
		for (int nIndex = 0; nIndex < countof(pQuest->nQuestValue); nIndex++)
		{
			if (pQuestInfo->AllEvent[nIndex].nQuestValue != 0)
			{
				KG_PROCESS_ERROR_RET_CODE(
					pQuest->nQuestValue[nIndex] >= pQuestInfo->AllEvent[nIndex].nQuestValue,
					qrcErrorTaskValue);
			}
		}
	}
    
    if (pQuestInfo->nCostMoney > 0)
    {
        nMoney = m_pPlayer->m_MoneyMgr.GetMoney(emotMoney);
	    KG_PROCESS_ERROR_RET_CODE(nMoney >= pQuestInfo->nCostMoney, qrcNotEnoughMoney);
    }
    
    bRetCode = IsQuestEndItemEnough(pQuestInfo);
	KG_PROCESS_ERROR_RET_CODE(bRetCode, qrcErrorItemCount);

    if (pQuestInfo->nPresentMoney > 0)
    {
        bRetCode = m_pPlayer->m_MoneyMgr.CanAddMoney(emotMoney, pQuestInfo->nPresentMoney);
        KG_PROCESS_ERROR_RET_CODE(bRetCode, qrcMoneyLimit);
    }

	nResult = qrcSuccess;
Exit0:
	return nResult;
}

int KQuestList::GetAcceptedCount()
{
	return m_nAcceptedCount;
}

//获取指定任务状态
QUEST_STATE	KQuestList::GetQuestState(DWORD dwQuestID)
{
	KGLOG_PROCESS_ERROR(dwQuestID < MAX_QUEST_COUNT);

	return (QUEST_STATE)m_byQuestStateList[dwQuestID];
Exit0:
	return qsInvalid;
}

BOOL KQuestList::SetQuestState(DWORD dwQuestID, QUEST_STATE eState)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;

    KGLOG_PROCESS_ERROR(dwQuestID < MAX_QUEST_COUNT);
    KGLOG_PROCESS_ERROR(eState == qsFinished || eState == qsUnfinished);

    m_byQuestStateList[dwQuestID] = (BYTE)eState;

	bResult = true;
Exit0:
	return bResult;
}

int KQuestList::GetQuestPhase(DWORD dwQuestID)
{
	int         nResult         = eqpInvalid;
	BOOL        bRetCode        = false;
	DWORD       dwQuestIndex    = 0;
	KQuest*     pQuest          = NULL;
	KQuestInfo* pQuestInfo      = NULL;

	if (m_byQuestStateList[dwQuestID] == qsFinished)
	{
		nResult = eqpFinish;
		goto Exit0;
	}
	
	KGLOG_PROCESS_ERROR(dwQuestID >= 0 && dwQuestID < MAX_QUEST_COUNT);

	nResult = eqpNotAccept;
	for (dwQuestIndex = 0; dwQuestIndex < cdMaxAcceptQuestCount; dwQuestIndex++)
	{
		if (m_AcceptedQuestList[dwQuestIndex].pQuestInfo && 
			m_AcceptedQuestList[dwQuestIndex].pQuestInfo->dwQuestID == dwQuestID)
		break;
	}
	KG_PROCESS_ERROR(dwQuestIndex < cdMaxAcceptQuestCount);

	nResult = eqpAcceptNotDone;
	pQuest = &(m_AcceptedQuestList[dwQuestIndex]);
	pQuestInfo = pQuest->pQuestInfo;
	if (pQuest)
	{			
		//检测任务变量的数值
		for (int nIndex = 0; nIndex < countof(pQuest->nQuestValue); nIndex++)
		{
			if (pQuestInfo->AllEvent[nIndex].nQuestValue != 0)
			{
				KG_PROCESS_ERROR(pQuest->nQuestValue[nIndex] >= pQuestInfo->AllEvent[nIndex].nQuestValue);
			}
		}
	}

	// 检测任务物品
    bRetCode = IsQuestEndItemEnough(pQuestInfo);
    KG_PROCESS_ERROR(bRetCode);

	nResult = eqpDoneNotFinish;
Exit0:
	return nResult;
}

DWORD KQuestList::GetQuestID(int nQuestIndex)
{
	KG_PROCESS_ERROR((nQuestIndex >= 0) && (nQuestIndex < cdMaxAcceptQuestCount));

	if (m_AcceptedQuestList[nQuestIndex].pQuestInfo)
    {
		return m_AcceptedQuestList[nQuestIndex].pQuestInfo->dwQuestID;
    }

Exit0:
	return 0;
}

int KQuestList::GetQuestIndex(DWORD dwQuestID)
{
	for (int nIndex = 0; nIndex < cdMaxAcceptQuestCount; nIndex++)
	{
		if (m_AcceptedQuestList[nIndex].pQuestInfo == NULL)
            continue;

        if (m_AcceptedQuestList[nIndex].pQuestInfo->dwQuestID == dwQuestID)
		{
			return nIndex;
		}
	}

	return -1;
}

int KQuestList::GetQuestValue(int nQuestIndex, int nValueIndex)
{
	KGLOG_PROCESS_ERROR((nQuestIndex >= 0) && (nQuestIndex < cdMaxAcceptQuestCount));
	KGLOG_PROCESS_ERROR((nValueIndex >= 0) && (nValueIndex < cdMaxQuestEventCount));

	return m_AcceptedQuestList[nQuestIndex].nQuestValue[nValueIndex];
Exit0:
	return 0;
}

BOOL KQuestList::SetQuestValue(int nQuestIndex, int nValueIndex, int nNewValue)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KQuest* pQuest = NULL;

	KGLOG_PROCESS_ERROR((nQuestIndex >= 0) && (nQuestIndex < cdMaxAcceptQuestCount));
	KGLOG_PROCESS_ERROR((nValueIndex >= 0) && (nValueIndex < cdMaxQuestEventCount));
    
    pQuest = &m_AcceptedQuestList[nQuestIndex];
    if (pQuest->nQuestValue[nValueIndex] == nNewValue)
        goto Exit1;
    
   pQuest->nQuestValue[nValueIndex] = nNewValue;

    assert(m_pPlayer);
    g_PlayerServer.DoSyncQuestValue(m_pPlayer->m_nConnIndex, nQuestIndex, nValueIndex, nNewValue);

    if (pQuest->nQuestValue[nValueIndex] == pQuest->pQuestInfo->AllEvent[nValueIndex].nQuestValue) // 完成某任务变量
    {
        if (pQuest->pQuestInfo->dwScriptID)
        {
            bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pQuest->pQuestInfo->dwScriptID);
            if (bRetCode && g_pSO3World->m_ScriptManager.IsFuncExist(pQuest->pQuestInfo->dwScriptID, SCRIPT_ON_FINISH_QUEST_VALUE))
            {
                int nTopIndex = 0;
                g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

                g_pSO3World->m_ScriptManager.Push(m_pPlayer);
                g_pSO3World->m_ScriptManager.Push(pQuest->pQuestInfo->dwQuestID);
                g_pSO3World->m_ScriptManager.Push(nValueIndex);
                g_pSO3World->m_ScriptManager.Call(pQuest->pQuestInfo->dwScriptID, SCRIPT_ON_FINISH_QUEST_VALUE, 0);
                g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);
            }
        }
    }

    PLAYER_LOG(m_pPlayer, "quest,setquestvalue,%u, %d", pQuest->pQuestInfo->dwQuestID, nValueIndex);

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KQuestList::GetQuestRepeatCutPercent(KQuestInfo* pQuestInfo, int* pnRepeatCutPercent)
{
    assert(pQuestInfo);
    assert(pnRepeatCutPercent);
    
    *pnRepeatCutPercent = 100;
    if (pQuestInfo->bRepeat)
    {
        *pnRepeatCutPercent = pQuestInfo->byRepeatCutPercent;
    }
    
    return true;
}

BOOL KQuestList::IsQuestEndItemEnoughByIndex(KQuestInfo* pQuestInfo, int nIndex)
{
    BOOL        bResult         = false;
    DWORD       dwTabType       = 0; 
    DWORD       dwTabIndex      = 0;
    KGItemInfo* pItemInfo       = NULL;
    DWORD       dwCurrentAmount = 0;
    DWORD       dwRequireAmount = 0;
    int         nBookRecipeID   = -1;

    assert(pQuestInfo);
    KGLOG_PROCESS_ERROR(nIndex >= 0 && nIndex < QUEST_END_ITEM_COUNT);

    dwTabType       = pQuestInfo->byEndRequireItemType[nIndex];
    dwTabIndex      = pQuestInfo->wEndRequireItemIndex[nIndex];
    dwRequireAmount = pQuestInfo->wEndRequireItemAmount[nIndex];

	if (dwTabType == 0 || dwTabIndex == 0)
	{
        goto Exit1;
    }
        
    dwCurrentAmount = m_pPlayer->m_ItemList.GetPlayerItemCount(dwTabType, dwTabIndex);
	KG_PROCESS_ERROR(dwCurrentAmount >= dwRequireAmount);
    
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KQuestList::ClearQuestList()
{
    memset(m_AcceptedQuestList, 0, sizeof(m_AcceptedQuestList));
    m_mapQuestValueIncreasor.clear();
}

BOOL KQuestList::IsQuestEndItemEnough(KQuestInfo* pQuestInfo)
{
    BOOL        bResult         = false;
    BOOL        bRetCode        = false;

    assert(pQuestInfo);

    for (int nIndex = 0; nIndex < QUEST_END_ITEM_COUNT; nIndex++)
	{
        bRetCode = IsQuestEndItemEnoughByIndex(pQuestInfo, nIndex);
        KG_PROCESS_ERROR(bRetCode);
	}
    
    bResult = true;
Exit0:
    return bResult;
}

QUEST_RESULT_CODE KQuestList::CommonCheck(KQuestInfo* pQuestInfo)
{
    QUEST_RESULT_CODE nResult       = qrcFailed;
    QUEST_RESULT_CODE nRetCode      = qrcFailed;
    BOOL              bRetCode      = false;
    DWORD             dwSkillLevel  = 0;

    assert(pQuestInfo);

    KG_PROCESS_ERROR_RET_CODE(m_byQuestStateList[pQuestInfo->dwQuestID] == qsUnfinished, qrcAlreadyFinishedQuest);
    
	if (pQuestInfo->dwPrequestID != 0)
    {
        KGLOG_PROCESS_ERROR(pQuestInfo->dwPrequestID < MAX_QUEST_COUNT);

        KG_PROCESS_ERROR_RET_CODE(
            m_byQuestStateList[pQuestInfo->dwPrequestID] == qsFinished, qrcPrequestUnfinished
        );
    }
            
	//KG_PROCESS_ERROR_RET_CODE(m_pPlayer->m_nLevel >= pQuestInfo->byMinLevel, qrcTooLowLevel);
    //KG_PROCESS_ERROR_RET_CODE(m_pPlayer->m_nLevel <= pQuestInfo->byMaxLevel, qrcTooHighLevel);
    
	KG_PROCESS_ERROR_RET_CODE(pQuestInfo->byRequireGender == 0 || m_pPlayer->m_eGender == pQuestInfo->byRequireGender, qrcErrorGender);
    
    // Cooldown计时器
	bRetCode = m_pPlayer->m_CDTimerList.CheckTimer(pQuestInfo->dwCoolDownID);
	KG_PROCESS_ERROR_RET_CODE(bRetCode, qrcCooldown);
    
    nResult = qrcSuccess;
Exit0:
    return nResult;
}

float GetExpParam(int nHeroLevel, int nQuestLevel)
{
    float param[5] = {1.0f, 1.0f, 0.6f, 0.1f, 0.01f};
    float fExpParam = 0.0f;
    int   nLevel = nHeroLevel;

    if (nLevel >= nQuestLevel)
    {
        fExpParam = 1.0f;
    }
    else if (nQuestLevel - nLevel <= 5)
    {
        fExpParam = 1.0f;
    }
    else if (nQuestLevel - nLevel <= 8)
    {
        fExpParam = 0.6f;
    }
    else if (nQuestLevel - nLevel <= 12)
    {
        fExpParam = 0.1f;
    }
    else
    {
        fExpParam = 0.01f;
    }

    return fExpParam;
}

QUEST_RESULT_CODE KQuestList::Present(KQuestInfo* pQuestInfo, int nPresentChoice1, int nPresentChoice2)
{
	QUEST_RESULT_CODE	nResult		        = qrcFailed;
    QUEST_RESULT_CODE	eRetCode            = qrcFailed;
	BOOL				bRetCode	        = false;
    int                 nHeroLevel          = 0;
    int                 nExpAdded           = 0;
    float               fExpParam           = 0.0f;

	assert(pQuestInfo);

	// 检测是否是重复完成
    if (
        m_byQuestStateList[pQuestInfo->dwQuestID] == qsFinished ||
        (pQuestInfo->bRepeat && pQuestInfo->byRepeatCutPercent == 100)
    )
    {
	    eRetCode = PresentItems(pQuestInfo, nPresentChoice1, nPresentChoice2);
        KGLOG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
    }
    
	bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, pQuestInfo->nPresentMoney);
	KGLOG_PROCESS_ERROR_RET_CODE(bRetCode, qrcAddMoneyFailed);

    PLAYER_LOG(m_pPlayer, "money,addmoney,%d,%s,%d-%d,%d,%d", emotMoney, "questpresent", 0, 0, 0, pQuestInfo->nPresentMoney);
    
    nHeroLevel = m_pPlayer->GetMainHeroLevel();
    if (nHeroLevel > 0)
    {
        fExpParam = GetExpParam(nHeroLevel, pQuestInfo->byLevel);
        nExpAdded = (int)(pQuestInfo->nPresentExp * fExpParam);

        bRetCode = m_pPlayer->AddHeroExpNoAdditional(m_pPlayer->m_dwMainHeroTemplateID, nExpAdded);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    
    nHeroLevel = m_pPlayer->GetAssistHeroLevel();
    if (nHeroLevel > 0)
    {
        fExpParam = GetExpParam(nHeroLevel, pQuestInfo->byLevel);
        nExpAdded = (int)(pQuestInfo->nPresentExp * fExpParam);

        bRetCode = m_pPlayer->AddHeroExpNoAdditional(m_pPlayer->m_dwAssistHeroTemplateID, nExpAdded);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    for (int i = 0; i < QUEST_AWARD_HERO_COUNT; ++i)
    {
        if (pQuestInfo->dwAwardHeroTemplateID[i] > 0 && pQuestInfo->nAwardHeroLevel[i] > 0)
        {
            bRetCode = m_pPlayer->AddNewHero(pQuestInfo->dwAwardHeroTemplateID[i], pQuestInfo->nAwardHeroLevel[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }

	nResult = qrcSuccess;
Exit0:
	return nResult;
}

BOOL KQuestList::CheckFreeRoomForPresent(KQuestInfo* pQuestInfo, int nPresentChoice1, int nPresentChoice2)
{
    BOOL    bResult             = false;
    BOOL    bRetCode            = false;
    DWORD   dwTabType[QUEST_PARAM_COUNT * 2] = {0};
    int     nTabIndex[QUEST_PARAM_COUNT * 2] = {0};
    int     nItemNum = 0;

    assert(pQuestInfo);

    KGLOG_PROCESS_ERROR(nPresentChoice1 >= 0 && nPresentChoice1 < QUEST_PARAM_COUNT);
    KGLOG_PROCESS_ERROR(nPresentChoice2 >= QUEST_PARAM_COUNT && nPresentChoice2 < cdQuestParamCount);

    if (!pQuestInfo->bPresentAll[0]) // 选择一个
    {
        if (pQuestInfo->byPresentItemType[nPresentChoice1] == ittOther || pQuestInfo->byPresentItemType[nPresentChoice1] == ittEquip)
        {
            dwTabType[nItemNum] = pQuestInfo->byPresentItemType[nPresentChoice1];
            nTabIndex[nItemNum] = pQuestInfo->wPresentItemIndex[nPresentChoice1];
            ++nItemNum;
        }
    }
    else
    {
        for (int nIndex = 0; nIndex < QUEST_PARAM_COUNT; nIndex++)
        {
            if (pQuestInfo->byPresentItemType[nIndex] == 0 || pQuestInfo->wPresentItemIndex[nIndex] == 0) 
                continue;

            if (pQuestInfo->byPresentItemType[nIndex] == ittOther || pQuestInfo->byPresentItemType[nIndex] == ittEquip)
            {
                dwTabType[nItemNum] = pQuestInfo->byPresentItemType[nIndex];
                nTabIndex[nItemNum] = pQuestInfo->wPresentItemIndex[nIndex];
                ++nItemNum;
            }
        }
    }

    if (!pQuestInfo->bPresentAll[1]) // 选择一个
    {
        if (pQuestInfo->byPresentItemType[nPresentChoice2] == ittOther || pQuestInfo->byPresentItemType[nPresentChoice2] == ittEquip)
        {
            dwTabType[nItemNum] = pQuestInfo->byPresentItemType[nPresentChoice2];
            nTabIndex[nItemNum] = pQuestInfo->wPresentItemIndex[nPresentChoice2];
            ++nItemNum;
        }
    }
    else
    {
        for (int nIndex = QUEST_PARAM_COUNT; nIndex < QUEST_PARAM_COUNT * 2; nIndex++)
        {
            if (pQuestInfo->byPresentItemType[nIndex] == 0)
                continue;

            if (pQuestInfo->wPresentItemIndex[nIndex] == 0)
                continue;

            if (pQuestInfo->byPresentItemType[nIndex] == ittOther || pQuestInfo->byPresentItemType[nIndex] == ittEquip)
            {
                dwTabType[nItemNum] = pQuestInfo->byPresentItemType[nIndex];
                nTabIndex[nItemNum] = pQuestInfo->wPresentItemIndex[nIndex];
                ++nItemNum;
            }
        }
    }

    bRetCode = m_pPlayer->m_ItemList.m_PlayerPackage.CanAddItems(dwTabType, nTabIndex, nItemNum);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

QUEST_RESULT_CODE KQuestList::CanPresentItems(KQuestInfo* pQuestInfo, int nPresentChoice1, int nPresentChoice2)
{
    QUEST_RESULT_CODE   nResult     = qrcFailed;
    QUEST_RESULT_CODE   eRetCode    = qrcFailed;
    BOOL                bRetCode    = false;

    assert(pQuestInfo);
    
    bRetCode = CheckFreeRoomForPresent(pQuestInfo, nPresentChoice1, nPresentChoice2);
    KG_PROCESS_ERROR_RET_CODE(bRetCode, qrcNotEnoughFreeRoom);

	if (pQuestInfo->bPresentAll[0])
	{
		for (int nIndex = 0; nIndex < QUEST_PARAM_COUNT; nIndex++)
		{
			if (pQuestInfo->byPresentItemType[nIndex] != 0 || pQuestInfo->wPresentItemIndex[nIndex] != 0)
			{
                eRetCode = CanPresentItem(
                    pQuestInfo->byPresentItemType[nIndex], pQuestInfo->wPresentItemIndex[nIndex], 
                    pQuestInfo->wPresentItemAmount[nIndex]
                );
                KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
			}
		}
	}
	else
	{
		if (pQuestInfo->byPresentItemType[nPresentChoice1] != 0 || pQuestInfo->wPresentItemIndex[nPresentChoice1] != 0)
		{
			eRetCode = CanPresentItem(
                pQuestInfo->byPresentItemType[nPresentChoice1], pQuestInfo->wPresentItemIndex[nPresentChoice1], 
                pQuestInfo->wPresentItemAmount[nPresentChoice1]
            );
            KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
		}
	}

	if (pQuestInfo->bPresentAll[1])
	{
		for (int nIndex = QUEST_PARAM_COUNT; nIndex < cdQuestParamCount; nIndex++)
		{
			if (pQuestInfo->byPresentItemType[nIndex] != 0 || pQuestInfo->wPresentItemIndex[nIndex] != 0)
			{
				eRetCode = CanPresentItem(
                    pQuestInfo->byPresentItemType[nIndex], pQuestInfo->wPresentItemIndex[nIndex], 
                    pQuestInfo->wPresentItemAmount[nIndex]
                );
                KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
			}
		}
	}
	else
	{
		if (pQuestInfo->byPresentItemType[nPresentChoice2] != 0 || pQuestInfo->wPresentItemIndex[nPresentChoice2] != 0)
		{
			eRetCode = CanPresentItem(
                pQuestInfo->byPresentItemType[nPresentChoice2], pQuestInfo->wPresentItemIndex[nPresentChoice2], 
                pQuestInfo->wPresentItemAmount[nPresentChoice2]
            );
            KG_PROCESS_ERROR_RET_CODE(eRetCode == qrcSuccess, eRetCode);
		}
	}
    
    nResult = qrcSuccess;
Exit0:
	return nResult;
}

QUEST_RESULT_CODE  KQuestList::CanPresentItem(DWORD dwTabType, DWORD dwIndex, int nAmount)
{
    return qrcSuccess;
}

QUEST_RESULT_CODE KQuestList::PresentItems(KQuestInfo* pQuestInfo, int nPresentChoice1, int nPresentChoice2)
{
    QUEST_RESULT_CODE   nResult     = qrcFailed;
    QUEST_RESULT_CODE   nRetCode    = qrcFailed;

    assert(pQuestInfo);

    // 发放第一组任务奖励
	if (pQuestInfo->bPresentAll[0])
	{
		for (int nIndex = 0; nIndex < QUEST_PARAM_COUNT; nIndex++)
		{
			if (pQuestInfo->byPresentItemType[nIndex] != 0 || pQuestInfo->wPresentItemIndex[nIndex] != 0)
			{
                nRetCode = PresentItem(
                    pQuestInfo->byPresentItemType[nIndex], pQuestInfo->wPresentItemIndex[nIndex], 
                    pQuestInfo->wPresentItemAmount[nIndex], pQuestInfo->wPresentItemValuePoint[nIndex]
                );
                KGLOG_PROCESS_ERROR_RET_CODE(nRetCode == qrcSuccess, nRetCode);
			}
		}
	}
	else
	{
		if (pQuestInfo->byPresentItemType[nPresentChoice1] != 0 || pQuestInfo->wPresentItemIndex[nPresentChoice1] != 0)
		{
			nRetCode = PresentItem(
                pQuestInfo->byPresentItemType[nPresentChoice1], pQuestInfo->wPresentItemIndex[nPresentChoice1], 
                pQuestInfo->wPresentItemAmount[nPresentChoice1], pQuestInfo->wPresentItemValuePoint[nPresentChoice1]
            );
            KGLOG_PROCESS_ERROR_RET_CODE(nRetCode == qrcSuccess, nRetCode);
		}
	}

	// 发放第二组任务道具
	if (pQuestInfo->bPresentAll[1])
	{
		for (int nIndex = QUEST_PARAM_COUNT; nIndex < cdQuestParamCount; nIndex++)
		{
			if (pQuestInfo->byPresentItemType[nIndex] != 0 || pQuestInfo->wPresentItemIndex[nIndex] != 0)
			{
				nRetCode = PresentItem(
                    pQuestInfo->byPresentItemType[nIndex], pQuestInfo->wPresentItemIndex[nIndex], 
                    pQuestInfo->wPresentItemAmount[nIndex], pQuestInfo->wPresentItemValuePoint[nIndex]
                );
                KGLOG_PROCESS_ERROR_RET_CODE(nRetCode == qrcSuccess, nRetCode);
			}
		}
	}
	else
	{
		if (pQuestInfo->byPresentItemType[nPresentChoice2] != 0 || pQuestInfo->wPresentItemIndex[nPresentChoice2] != 0)
		{
			nRetCode = PresentItem(
                pQuestInfo->byPresentItemType[nPresentChoice2], pQuestInfo->wPresentItemIndex[nPresentChoice2], 
                pQuestInfo->wPresentItemAmount[nPresentChoice2], pQuestInfo->wPresentItemValuePoint[nPresentChoice2]
            );
            KGLOG_PROCESS_ERROR_RET_CODE(nRetCode == qrcSuccess, nRetCode);
		}
	}
    
    nResult = qrcSuccess;
Exit0:
	return nResult;
}

QUEST_RESULT_CODE KQuestList::PresentItem(DWORD dwTabType, DWORD dwIndex, int nAmount, int nValuePoint)
{
    QUEST_RESULT_CODE	        nResult			    = qrcAddItemFailed;
    BOOL                        bRetCode            = false;
    IItem*                      pItem               = NULL;
    KFASHION_INFO*              pFashion            = NULL;
    KGCheerleadingInfo*         pCheerleadingInfo   = NULL;
    KTEAM_LOGO_INFO*            pTeamLogoInfo       = NULL;

    switch (dwTabType)
    {
    case ittEquip:
    case ittOther:
        {
            pItem = m_pPlayer->AddItem(dwTabType, dwIndex, nAmount, nValuePoint);
            KG_PROCESS_ERROR(pItem);

            PLAYER_LOG(m_pPlayer, "item,additem,%u-%u,%d,%d,%d,%s,%d-%d-%d",dwTabType, dwIndex, nAmount, nValuePoint, pItem->GetQuality(), "quest", 0,0,0);
        }
        break;
    case ittFashion:
        bRetCode = m_pPlayer->m_Wardrobe.AddFashion(dwIndex);
        KGLOG_PROCESS_ERROR(bRetCode);
        break;

    case ittCheerleading:
        pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo(dwIndex);
        KGLOG_PROCESS_ERROR(pCheerleadingInfo);

        bRetCode = m_pPlayer->m_CheerleadingMgr.AddCheerleading(pCheerleadingInfo->dwID);
        KGLOG_PROCESS_ERROR(bRetCode);
        break;

    case ittTeamLogo:
        pTeamLogoInfo = g_pSO3World->m_ItemHouse.GetTeamLogoInfo(dwIndex);
        KGLOG_PROCESS_ERROR(pTeamLogoInfo);

        bRetCode = m_pPlayer->AddTeamLogo((WORD)pTeamLogoInfo->nID);
        KGLOG_PROCESS_ERROR(bRetCode);
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    nResult = qrcSuccess;
Exit0:
	return nResult;
}

BOOL KQuestList::CostQuestRequireItem(KQuestInfo* pQuestInfo, BOOL bCancelQuest)
{
    BOOL        bResult         = false;
    BOOL        bRetCode        = false;
	DWORD       dwItemType	    = 0;
	DWORD       dwItemIndex     = 0;
    KGItemInfo* pItemInfo       = NULL;
    int         nBookRecipeID   = -1;

	for (int nIndex = 0; nIndex < QUEST_END_ITEM_COUNT; nIndex++)
	{
		int     nIndex2         = 0;
        DWORD   dwCurrentAmount = 0;
        DWORD   dwCostAmount    = 0;
        
        dwItemType	    = pQuestInfo->byEndRequireItemType[nIndex];
        dwItemIndex     = pQuestInfo->wEndRequireItemIndex[nIndex];
        dwCostAmount    = pQuestInfo->wEndRequireItemAmount[nIndex];
        
        if (dwItemType == 0 || dwItemIndex == 0)
            continue;
        
        if (!pQuestInfo->bIsDeleteEndRequireItem[nIndex] && !bCancelQuest) // 完成任务时是否删除任务需求道具
            continue;

        pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(dwItemType, dwItemIndex);
        KGLOG_PROCESS_ERROR(pItemInfo);

		for (nIndex2 = 0; nIndex2 < nIndex; nIndex2++)
		{
			if (
                dwItemType == pQuestInfo->byEndRequireItemType[nIndex2] && 
                dwItemIndex == pQuestInfo->wEndRequireItemIndex[nIndex2]
            )
                break;
		}
                
        dwCurrentAmount = m_pPlayer->m_ItemList.GetPlayerItemCount(dwItemType, dwItemIndex);
        dwCostAmount = MIN(dwCurrentAmount, dwCostAmount);

        if (dwCostAmount == 0)
        {
            continue;
        }

		bRetCode = m_pPlayer->m_ItemList.CostPlayerItem(
            dwItemType, dwItemIndex, dwCostAmount
        );
		KGLOG_PROCESS_ERROR(bRetCode);
	}

    bResult = true;
Exit0:
    return bResult;
}

BOOL KQuestList::CostQuestOfferItem(KQuestInfo* pQuestInfo)
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    DWORD   dwCurrentAmount = 0;
    DWORD   dwCostAmount    = 0;

	for (int nIndex = 0; nIndex < QUEST_PARAM_COUNT; nIndex++)
	{
		DWORD dwTabType	    = pQuestInfo->dwOfferItemType[nIndex];
		DWORD dwTabIndex    = pQuestInfo->nOfferItemIndex[nIndex];
        
        if (dwTabType == 0 || dwTabIndex == 0)
        {
            continue;
        }

		dwCurrentAmount = m_pPlayer->m_ItemList.GetPlayerItemCount(dwTabType, dwTabIndex);
        dwCostAmount = pQuestInfo->nOfferItemAmount[nIndex];

        if (dwCostAmount > dwCurrentAmount)
        {
            dwCostAmount = dwCurrentAmount;
        }
        
        if (dwCostAmount == 0)
        {
            continue;
        }

		bRetCode = m_pPlayer->m_ItemList.CostPlayerItem(dwTabType, dwTabIndex, dwCostAmount);
		KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

void KQuestList::OnEvent(int nEvent, int (&Params)[cdMaxEventParamCount])
{
    BOOL        bRetCode		= false;
    KQuest*     pQuest			= NULL;
    KQuestInfo* pQuestInfo		= NULL;
    int         nQuestIndex		= 0;
    int         nValueIndex		= 0;
	KHero*		pFightingHero	= NULL;
	KHero*		pTeammate		= NULL;
    KMAP_QUESTVALUE_INCREASOR::iterator it;
    std::vector<DWORD> vecNeedCheckFinishQuest;
    std::set<int> setProcessedQuest;
    std::set<int>::iterator setIt;
    KVEC_QUESTVALUE_INCREASOR::iterator itSub;

    it = m_mapQuestValueIncreasor.find(nEvent);
    KG_PROCESS_ERROR(it != m_mapQuestValueIncreasor.end());

    itSub = it->second.begin();
    while (itSub != it->second.end())
    {
        nQuestIndex = itSub->nQuestIndex;
        nValueIndex = itSub->nValueIndex;
        assert((nQuestIndex >= 0) && (nQuestIndex < cdMaxAcceptQuestCount));
        assert((nValueIndex >= 0) && (nValueIndex < cdMaxQuestEventCount));
        assert(m_AcceptedQuestList[nQuestIndex].pQuestInfo != NULL);

        pQuest      = &m_AcceptedQuestList[nQuestIndex];
        pQuestInfo  = m_AcceptedQuestList[nQuestIndex].pQuestInfo;

		if (pQuestInfo->byRequireSingle)
		{
			pFightingHero = m_pPlayer->GetFightingHero();
			if (pFightingHero)
			{
				pTeammate = pFightingHero->GetTeammate();
				if (pTeammate)
				{
					if (!pTeammate->m_bAiMode)
                    {
                        ++itSub;
						continue;
                    }
				}
			}
		}

		bRetCode = pQuestInfo->AllEvent[nValueIndex].CheckCondition(Params);
		if	(!bRetCode)
        {
            ++itSub;
			continue;
        }

        if (pQuestInfo->nRequireMissionType > 0 && nEvent != peSelectHeroSkill)
        {
            DWORD dwMapID = ERROR_ID;
            KMission* pMission = NULL;
            KHero* pFightHero = NULL;

            pFightHero = m_pPlayer->GetFightingHero();
            if (pFightHero == NULL)
            {
                ++itSub;
                continue;
            }

            assert(pFightHero->m_pScene);
            dwMapID = pFightHero->m_pScene->m_dwMapID;

            pMission = g_pSO3World->m_MissionMgr.GetMission(dwMapID);
            if (pMission == NULL)
            {
                ++itSub;
                continue;
            }

            if (pQuestInfo->nRequireMissionType != pMission->eType)
            {
                ++itSub;
                continue;
            }

            if (pQuestInfo->nRequireMissionLevel != 0 && pQuestInfo->nRequireMissionLevel != pMission->nLevel)
            {
                ++itSub;
                continue;
            }

            if (pQuestInfo->nRequireMissionStep != 0 && pQuestInfo->nRequireMissionStep != pMission->nStep)
            {
                ++itSub;
                continue;
            }
        }

        if (pQuestInfo->nRequireMapType > 0 && nEvent != peSelectHeroSkill)
        {
            DWORD dwMapID = ERROR_ID;
            KHero* pFightHero = NULL;
            KMapParams* pMapInfo = NULL;

            pFightHero = m_pPlayer->GetFightingHero();
            if (pFightHero == NULL)
            {
                ++itSub;
                continue;
            }

            assert(pFightHero->m_pScene);
            dwMapID = pFightHero->m_pScene->m_dwMapID;

            pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(dwMapID);
            KGLOG_PROCESS_ERROR(pMapInfo);
            
            if (pMapInfo->nType != pQuestInfo->nRequireMapType)
            {
                ++itSub;
                continue;
            }
        }

        if (pQuest->nQuestValue[nValueIndex] >= pQuestInfo->AllEvent[nValueIndex].nQuestValue)
        {
            itSub = it->second.erase(itSub);
            continue;
        }

        if (nValueIndex > 0 && pQuestInfo->bEventOrder) // 按顺序完成
        {
            if (pQuest->nQuestValue[nValueIndex - 1] < pQuestInfo->AllEvent[nValueIndex - 1].nQuestValue)
            {
                ++itSub;
                continue;
            }
        }

        if (pQuest->nQuestValue[nValueIndex] >= pQuestInfo->AllEvent[nValueIndex].nQuestValue)
        {
            ++itSub;
            continue;
        }

        setIt = setProcessedQuest.find(nQuestIndex);
        if (setIt != setProcessedQuest.end())
        {
            ++itSub;
            continue;
        }

        bRetCode = SetQuestValue(nQuestIndex, nValueIndex, pQuest->nQuestValue[nValueIndex] + 1);
        KGLOG_PROCESS_ERROR(bRetCode);

        setProcessedQuest.insert(nQuestIndex);

        // npc 倒地后不起来
        if (nEvent == peNpcDie)
        {
            KHero* pNpc = NULL;
            KScene* pScene = m_pPlayer->GetCurrentScene();
            KGLOG_PROCESS_ERROR(pScene);

            pNpc = pScene->GetHeroById((DWORD)Params[0]);
            KGLOG_PROCESS_ERROR(pNpc);

            pNpc->m_nMoveFrameCounter = 65535;
        }

        vecNeedCheckFinishQuest.push_back(pQuestInfo->dwQuestID);
	    
        ++itSub;
    }

    for (size_t i = 0; i < vecNeedCheckFinishQuest.size(); ++i)
        CheckAutoFinish(vecNeedCheckFinishQuest[i]);

Exit0:
    return;
}

void KQuestList::RegisterQuestValueProccessor(int nQuestIndex)
{
    KQuestInfo* pQuestInfo = NULL;
    KVEC_QUESTVALUE_INCREASOR* pvecIncreasor = NULL;
    KQUESTVALUE_INCREASOR sQuestValueIncreasor = {0, 0};

    assert((nQuestIndex >= 0) && (nQuestIndex < cdMaxAcceptQuestCount));
    assert(m_AcceptedQuestList[nQuestIndex].pQuestInfo != NULL);

    pQuestInfo = m_AcceptedQuestList[nQuestIndex].pQuestInfo;
    for (int i = 0; i < countof(pQuestInfo->AllEvent); ++i)
    {
        if (pQuestInfo->AllEvent[i].nEvent == peInvalid)
            continue;

        pvecIncreasor = &m_mapQuestValueIncreasor[pQuestInfo->AllEvent[i].nEvent];
        sQuestValueIncreasor.nQuestIndex = nQuestIndex;
        sQuestValueIncreasor.nValueIndex = i;
        pvecIncreasor->push_back(sQuestValueIncreasor);
    }
}

void KQuestList::UnRegisterQuestValueProccessor(int nQuestIndex)
{
    BOOL bRetCode = false;
    
    for (KMAP_QUESTVALUE_INCREASOR::iterator it = m_mapQuestValueIncreasor.begin(); it != m_mapQuestValueIncreasor.end(); ++it)
    {
        for (KVEC_QUESTVALUE_INCREASOR::iterator itSub = it->second.begin(); itSub != it->second.end();)
        {
            if (itSub->nQuestIndex == nQuestIndex)
            {
                itSub = it->second.erase(itSub);
                continue;
            }
            ++itSub;
        }
    }
}

QUEST_RESULT_CODE KQuestList::CanAcceptDailyQuest(KQuestInfo* pQuestInfo)
{
    QUEST_RESULT_CODE  nResult               = qrcFailed;
    BOOL               bRetCode              = false; 
    BOOL               bDailyQuestNotFull    = false;

    assert(pQuestInfo);

    for (int i = 0; i < MAX_DAILY_QUEST_COUNT; ++i)
    {
        if (m_DailyQuest[i].dwQuestID == 0)
        {
            bDailyQuestNotFull = true;
            continue;
        }

        if (m_DailyQuest[i].dwQuestID == pQuestInfo->dwQuestID)
        {
            if (m_DailyQuest[i].nNextAcceptTime == 0)
            {
                nResult = qrcAlreadyAcceptQuest;
                goto Exit0;
            }
            
            KG_PROCESS_ERROR(g_pSO3World->m_nCurrentTime > m_DailyQuest[i].nNextAcceptTime);
            bDailyQuestNotFull = true;
            break;
        }
    }

    KG_PROCESS_ERROR_RET_CODE(bDailyQuestNotFull, qrcDailyQuestFull);

    nResult = qrcSuccess;
Exit0:
    return nResult;
}

BOOL KQuestList::AcceptDailyQuest(KQuestInfo* pQuestInfo)
{
    assert(pQuestInfo->bDaily);

    for (int i = 0; i < MAX_DAILY_QUEST_COUNT; ++i)
    {
        if (m_DailyQuest[i].dwQuestID == ERROR_ID)
        {
            m_DailyQuest[i].dwQuestID = pQuestInfo->dwQuestID;
            m_DailyQuest[i].nNextAcceptTime = 0;

            g_LogClient.DoFlowRecord(
                frmtDailyQuest, dqfrAcceptQuest, 
                "%s,%d", 
                m_pPlayer->m_szName,
                pQuestInfo->dwQuestID
            );

            return true;
        }
        else if (m_DailyQuest[i].dwQuestID == pQuestInfo->dwQuestID)
        {
            KGLOG_PROCESS_ERROR(g_pSO3World->m_nCurrentTime >= m_DailyQuest[i].nNextAcceptTime);
            m_DailyQuest[i].dwQuestID = pQuestInfo->dwQuestID;
            m_DailyQuest[i].nNextAcceptTime = 0;

            g_LogClient.DoFlowRecord(
                frmtDailyQuest, dqfrAcceptQuest, 
                "%s,%d", 
                m_pPlayer->m_szName,
                pQuestInfo->dwQuestID
            );

            return true;
        }
    }
Exit0:
	return false;
}

void KQuestList::ClearDailyQuest(KQuestInfo* pQuestInfo)
{
    assert(pQuestInfo->bDaily);

    for (int i = 0; i < MAX_DAILY_QUEST_COUNT; ++i)
    {
        if (m_DailyQuest[i].dwQuestID == pQuestInfo->dwQuestID)
        {
            m_DailyQuest[i].dwQuestID = ERROR_ID;
            m_DailyQuest[i].nNextAcceptTime = 0;
        }
    }

    return;
}

void KQuestList::FinishDailyQuest(KQuestInfo* pQuestInfo)
{
    int           nDailyQuestIndex  = -1;

    assert(pQuestInfo);

    // 每日任务一定是可重复而且是必须要接的
    KG_PROCESS_ERROR(pQuestInfo->bRepeat && pQuestInfo->bAccept && pQuestInfo->bDaily);

    for (int i = 0; i < MAX_DAILY_QUEST_COUNT; ++i)
    {
        if (m_DailyQuest[i].dwQuestID != pQuestInfo->dwQuestID)
            continue;

        nDailyQuestIndex = i;
        break;
    }
    
    KGLOG_PROCESS_ERROR(nDailyQuestIndex != -1);

    m_DailyQuest[nDailyQuestIndex].nNextAcceptTime = g_GetNextCycleTime(DAILY_CYCLE, DAILY_OFFSET);
    KGLOG_CHECK_ERROR(m_DailyQuest[nDailyQuestIndex].nNextAcceptTime > g_pSO3World->m_nCurrentTime);

    g_PlayerServer.DoSyncOneDailyQuestData(m_pPlayer->m_nConnIndex, pQuestInfo->dwQuestID, m_DailyQuest[nDailyQuestIndex].nNextAcceptTime);

    if (pQuestInfo->nQuestType == qtDaily)
        m_pPlayer->OnEvent(peFinishDailyQuest, (int)pQuestInfo->dwQuestID);

    if (pQuestInfo->nQuestType == qtRandom)
        m_pPlayer->OnEvent(peFinishRandomQuest, (int)pQuestInfo->dwQuestID);
        
    m_pPlayer->OnEvent(peFinishQuest, (int)pQuestInfo->dwQuestID);

    g_LogClient.DoFlowRecord(
        frmtDailyQuest, dqfrFinishQuest, 
        "%s,%d", 
        m_pPlayer->m_szName,
        pQuestInfo->dwQuestID
    );
Exit0:
    return;
}

void KQuestList::FinishRandomQuest(DWORD dwQuestID)
{
	DWORD       dwRandomQuestID = 0;
	
    ++m_pPlayer->m_nRandomQuestIndex;
	
	dwRandomQuestID = g_pSO3World->m_Settings.m_QuestInfoList.GetRandomQuest(
        m_pPlayer->GetMainHeroLevel(), m_pPlayer->m_nRandomQuestIndex
    );

    m_pPlayer->OnEvent(peFinishOneChainQuest);

	g_PlayerServer.DoSyncRandomQuest(m_pPlayer->m_nConnIndex, dwRandomQuestID);
}

const KQuest* KQuestList::GetQuestByIndex(int nQuestIndex) const
{
    const KQuest* pResult = NULL;

    if (nQuestIndex >= 0 && nQuestIndex <= cdMaxAcceptQuestCount)
        pResult = &m_AcceptedQuestList[nQuestIndex];

    return pResult;
}

QUEST_RESULT_CODE KQuestList::ClientSetQuestValue(int nQuestIndex, int nValueIndex, int nNewValue)
{
	QUEST_RESULT_CODE nResult = qrcFailed;
	BOOL bRetCode = false;
	KQuestInfo* pQuestInfo = NULL;
    KPLAYER_EVENT eEvent = peInvalid;

    KGLOG_PROCESS_ERROR((nQuestIndex >= 0) && (nQuestIndex < cdMaxAcceptQuestCount));
    KGLOG_PROCESS_ERROR((nValueIndex >= 0) && (nValueIndex < cdMaxQuestEventCount));

    pQuestInfo = m_AcceptedQuestList[nQuestIndex].pQuestInfo;
    KGLOG_PROCESS_ERROR_RET_CODE(pQuestInfo != NULL, qrcNeedAccept);

    eEvent = (KPLAYER_EVENT)pQuestInfo->AllEvent[nValueIndex].nEvent;
    KGLOG_PROCESS_ERROR_RET_CODE(eEvent > peInvalid && eEvent < peClientEnd, qrcClientCannotModify);

    if (nValueIndex > 0 && pQuestInfo->bEventOrder)
    {
        KQuest* pQuest = &m_AcceptedQuestList[nQuestIndex];
        KG_PROCESS_ERROR(pQuest->nQuestValue[nValueIndex - 1] >= pQuestInfo->AllEvent[nValueIndex - 1].nQuestValue);
    }

    bRetCode = SetQuestValue(nQuestIndex, nValueIndex, nNewValue);
    KGLOG_PROCESS_ERROR(bRetCode);

    CheckAutoFinish(pQuestInfo->dwQuestID);

    nResult = qrcSuccess;
Exit0:
	return nResult;
}

void KQuestList::CheckAutoFinish(DWORD dwQuestID)
{
    QUEST_RESULT_CODE eRetCode = qrcFailed; 
    KQuestInfo* pQuestInfo = NULL;

    pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(dwQuestID);
    KGLOG_PROCESS_ERROR(pQuestInfo);

    KG_PROCESS_ERROR(pQuestInfo->bAutoFinish);

    eRetCode = CanFinish(pQuestInfo);
    KG_PROCESS_ERROR(eRetCode == qrcSuccess);

    eRetCode = Finish(dwQuestID, 0, 4);
    KGLOG_PROCESS_ERROR(eRetCode == qrcSuccess);

Exit0:
    return;
}

BOOL KQuestList::CanClientForceFinish(DWORD dwQuestID)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KQuestInfo* pQuestInfo = NULL;

    pQuestInfo = g_pSO3World->m_Settings.m_QuestInfoList.GetQuestInfo(dwQuestID);
    KGLOG_PROCESS_ERROR(pQuestInfo);

    KG_PROCESS_ERROR(pQuestInfo->bClientCanFinish);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KQuestList::CanAcceptRandomQuest(DWORD dwQuestID)
{
	BOOL        bResult         = false;
	BOOL        bRetCode        = false;
    KQuestInfo* pQuestInfo      = NULL;
    DWORD       dwRandomQuestID = 0;

    for (int nQuestIndex = 0; nQuestIndex < cdMaxAcceptQuestCount; nQuestIndex++)
    {
        pQuestInfo = m_AcceptedQuestList[nQuestIndex].pQuestInfo;

        if (pQuestInfo == NULL)
            continue;

        KG_PROCESS_ERROR(pQuestInfo->nQuestType != qtRandom);
    }

    dwRandomQuestID = g_pSO3World->m_Settings.m_QuestInfoList.GetRandomQuest(
        m_pPlayer->GetMainHeroLevel(), m_pPlayer->m_nRandomQuestIndex
    );
    KG_PROCESS_ERROR(dwRandomQuestID == dwQuestID);

	bResult = true;
Exit0:
	return bResult;
}

void KQuestList::ClearRandomQuest()
{
    BOOL        bRetCode        = false;
    KQuestInfo* pQuestInfo      = NULL;
    DWORD       dwRandomQuestID = 0;

    for (int nQuestIndex = 0; nQuestIndex < cdMaxAcceptQuestCount; nQuestIndex++)
    {
        pQuestInfo = m_AcceptedQuestList[nQuestIndex].pQuestInfo;

        if (pQuestInfo == NULL)
            continue;

        if (pQuestInfo->nQuestType != qtRandom)
            continue;

        ClearQuest(pQuestInfo->dwQuestID);
    }

Exit0:
    return;
}
