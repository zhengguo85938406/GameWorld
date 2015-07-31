#include "stdafx.h"
#include "KUpgradeQueue.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"

KUpgradeQueue::KUpgradeQueue()
{
    m_pPlayer = NULL;
    m_nMaxUpgradeCount = 0;
    m_nFreeClearCDTimes = 0;
}

KUpgradeQueue::~KUpgradeQueue()
{
}

BOOL KUpgradeQueue::Init(KPlayer* pPlayer)
{
    assert(pPlayer);
    m_nMaxUpgradeCount = g_pSO3World->m_Settings.m_ConstList.nInitUpgradeCount;
    m_pPlayer = pPlayer;
    return true;
}

void KUpgradeQueue::UnInit()
{
}

void KUpgradeQueue::Activate()
{
    if ((g_pSO3World->m_nGameLoop - m_pPlayer->m_dwID) % GAME_FPS != 0)
        return;

    KMAP_UPGRADE_NODE::iterator it = m_mapUpgradeNode.begin();
    while (it != m_mapUpgradeNode.end())
    {
        if (it->first <= g_pSO3World->m_nCurrentTime)
        {
            m_pPlayer->m_BusinessStreet.OnUpgradeQueueFinished(it->second.eType);
            g_PlayerServer.DoDelUpgradeQueueNode(m_pPlayer->m_nConnIndex, it->second.eType, it->second.dwHeroTemplateID); 
            m_mapUpgradeNode.erase(it++);
            continue;
        }

        break;
    }
}

BOOL KUpgradeQueue::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL bResult  = false;

    assert(pLoadBuf);

    const T3DB::KPBUpgradeQueue& UpgradeQueue = pLoadBuf->upgradequeue();
    const T3DB::KPB_UPGRADE_QUEUE_DATA& UpgradeQueueData = UpgradeQueue.upgradequeuedata();

    m_nFreeClearCDTimes = UpgradeQueueData.freeclearcdtimes();
    m_nMaxUpgradeCount  = UpgradeQueueData.maxslotcount();

    for (int i = 0; i < UpgradeQueueData.node_size(); ++i)
    {
        const T3DB::KPB_UPGRADE_NODE& Node = UpgradeQueueData.node(i);
        KUpgradeNode node;

        node.eType = (KUPGRADE_TYPE)Node.upgradetype();
        node.nEndTime = Node.endtime();
        node.dwHeroTemplateID = Node.herotemplateid();

        m_mapUpgradeNode.insert(std::make_pair(Node.endtime(), node));
    }

    if (m_pPlayer->m_eConnectType == eLogin)
    {
        g_PlayerServer.DoSyncUpgradeQueueData(m_pPlayer);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KUpgradeQueue::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult  = false;
    T3DB::KPBUpgradeQueue* pUpgradeQueue = NULL;
    T3DB::KPB_UPGRADE_QUEUE_DATA* pUpgradeQueueData = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pUpgradeQueue = pSaveBuf->mutable_upgradequeue();
    pUpgradeQueueData = pUpgradeQueue->mutable_upgradequeuedata();

    pUpgradeQueueData->set_count(m_mapUpgradeNode.size());
    pUpgradeQueueData->set_freeclearcdtimes(m_nFreeClearCDTimes);
    pUpgradeQueueData->set_maxslotcount(m_nMaxUpgradeCount);

    for (KMAP_UPGRADE_NODE::iterator it = m_mapUpgradeNode.begin(); it != m_mapUpgradeNode.end(); ++it)
    {
        T3DB::KPB_UPGRADE_NODE* pNode = pUpgradeQueueData->add_node();
        pNode->set_upgradetype(it->second.eType);
        pNode->set_endtime(it->first);
        pNode->set_herotemplateid(it->second.dwHeroTemplateID);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KUpgradeQueue::HasFreeSlot()
{
    if (m_mapUpgradeNode.size() < (size_t)m_nMaxUpgradeCount)
        return true;
    return false;
}

BOOL KUpgradeQueue::CanAddUpgradeNode(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID)
{
	BOOL bResult  = false;
	KUpgradeNode* pExist = NULL;

    KG_PROCESS_ERROR(m_mapUpgradeNode.size() < (size_t)m_nMaxUpgradeCount);

    pExist = GetNode(eType, dwHeroTemplateID);
    KG_PROCESS_ERROR(pExist == NULL);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KUpgradeQueue::AddUpgradeNode(KUPGRADE_TYPE eType, int nCDTime, uint32_t dwHeroTemplateID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int  nEndTime = 0;
    KUpgradeNode node;

    bRetCode = CanAddUpgradeNode(eType, dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    nEndTime = g_pSO3World->m_nCurrentTime + nCDTime;

    node.eType = eType;
    node.nEndTime = nEndTime;
    node.dwHeroTemplateID = dwHeroTemplateID;

    m_mapUpgradeNode.insert(std::make_pair(nEndTime, node));

    g_PlayerServer.DoAddUpgradeQueueNode(m_pPlayer->m_nConnIndex, eType, nEndTime, dwHeroTemplateID);

	bResult = true;
Exit0:
	return bResult;
}

KUpgradeNode* KUpgradeQueue::GetNode(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID)
{
    KMAP_UPGRADE_NODE::iterator it = m_mapUpgradeNode.begin();
    for (;it != m_mapUpgradeNode.end(); ++it)
    {
        if (it->second.eType == eType && it->second.dwHeroTemplateID == dwHeroTemplateID)
        {
            return &it->second;
        }
    }

    return NULL;
}

BOOL KUpgradeQueue::FreeClearCDRequest(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KVIPConfig* pConfig = NULL;

    KGLOG_PROCESS_ERROR(m_pPlayer->IsVIP());

    pConfig = g_pSO3World->m_Settings.m_VIPConfig.GetByID(m_pPlayer->m_nVIPLevel);
    KGLOG_PROCESS_ERROR(pConfig);
    
    KGLOG_PROCESS_ERROR(m_nFreeClearCDTimes < pConfig->nFreeClearCDCount);
    ++m_nFreeClearCDTimes;

    g_PlayerServer.DoSyncFreeClearCDTimes(m_pPlayer->m_nConnIndex, m_nFreeClearCDTimes);

    ClearCDRespond(eType, dwHeroTemplateID, 0);

	bResult = true;
Exit0:
	return bResult;
}

void KUpgradeQueue::ResetFreeClearCDTimes()
{
    m_nFreeClearCDTimes = 0;
    g_PlayerServer.DoSyncFreeClearCDTimes(m_pPlayer->m_nConnIndex, m_nFreeClearCDTimes);
}

BOOL KUpgradeQueue::ClearCDRequest(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KUpgradeNode* pNode = NULL;
	KCUSTOM_CONSUME_INFO CCInfo;
    int nNeedCoin = 0;
    int nNeedCoinPerMinute = g_pSO3World->m_Settings.m_ConstList.nClearCDCoinPerMinute;

    if (eType == eutUpgradeHeroTraining)
        nNeedCoinPerMinute = g_pSO3World->m_Settings.m_ConstList.nTrainCoinPerMinute;

    pNode = GetNode(eType, dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pNode);

    KGLOG_PROCESS_ERROR(pNode->nEndTime > g_pSO3World->m_nCurrentTime);

    nNeedCoin = ((pNode->nEndTime - g_pSO3World->m_nCurrentTime) / SECONDS_PER_MINUTE + 1) * nNeedCoinPerMinute;

    // ·¢ËÍÇëÇó¸øpaysys
    memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));

    CCInfo.nValue1 = (int)eType;
    CCInfo.nValue2 = (int)dwHeroTemplateID;
    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctClearCD, nNeedCoin, &CCInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (eType == eutUpgradeSafeBox)
        PLAYER_LOG(m_pPlayer, "clearcd,safebox,%d,%d,%d", m_pPlayer->m_MoneyMgr.GetSafeBoxLevel(), pNode->nEndTime - g_pSO3World->m_nCurrentTime, nNeedCoin);
    else if (eType == eutUpgradeTrainTeacher)
        PLAYER_LOG(m_pPlayer, "clearcd,teacherlevelup,%d,%d,%d", m_pPlayer->m_HeroTrainingMgr.GetTeacherLevel(), pNode->nEndTime - g_pSO3World->m_nCurrentTime, nNeedCoin);
    else if (eType == eutUpgradeGym)
        PLAYER_LOG(m_pPlayer, "clearcd,upgradegym,%d,%d,%d", m_pPlayer->m_Gym.GetEquipLevel(), pNode->nEndTime - g_pSO3World->m_nCurrentTime, nNeedCoin);
    else if (eType >= eutUpgradeStoreBegin && eType < eutUpgradeStoreEnd)
        PLAYER_LOG(m_pPlayer, "clearcd,storelevelup,%d,%d,%d", m_pPlayer->m_BusinessStreet.GetStoreLevel(eType - eutUpgradeStoreBegin), pNode->nEndTime - g_pSO3World->m_nCurrentTime, nNeedCoin);
    else if (eType == eutUpgradeHeroTraining)
        PLAYER_LOG(m_pPlayer, "clearcd,herotraining,%d,%d,%d", 0, (pNode->nEndTime - g_pSO3World->m_nCurrentTime), nNeedCoin);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KUpgradeQueue::ClearCDRespond(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID, int nCostMoney)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KMAP_UPGRADE_NODE::iterator it = m_mapUpgradeNode.begin();
    for (;it != m_mapUpgradeNode.end(); ++it)
    {      
        if (it->second.eType == eType && it->second.dwHeroTemplateID == dwHeroTemplateID)
        {
            int nLeftSeconds = it->second.nEndTime - g_pSO3World->m_nCurrentTime;
            MAKE_IN_RANGE(nLeftSeconds, 0, INT_MAX);
            g_LogClient.DoFlowRecord(
                frmtUpgradeQueue, uqfrClearCD,
                "%s,%d,%d,%d",
                m_pPlayer->m_szName,
                nCostMoney,
                eType,
                nLeftSeconds
            );

            m_pPlayer->m_BusinessStreet.OnUpgradeQueueFinished(eType);
            m_pPlayer->m_Gym.OnUpgradeQueueFinished(eType, it->second.dwHeroTemplateID);
            m_pPlayer->m_HeroTrainingMgr.OnUpgradeQueueFinished(eType, it->second.dwHeroTemplateID, nCostMoney);
            g_PlayerServer.DoDelUpgradeQueueNode(m_pPlayer->m_nConnIndex, eType, it->second.dwHeroTemplateID); 
            m_mapUpgradeNode.erase(it);
            break;
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

void KUpgradeQueue::SetMaxUpgradeCount(int nNewMaxUpgradeCount)
{
    m_nMaxUpgradeCount = nNewMaxUpgradeCount;
    g_PlayerServer.DoSyncUpgradeSlotCount(m_pPlayer->m_nConnIndex, m_nMaxUpgradeCount);
}

BOOL KUpgradeQueue::GetSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    BYTE* pbyOffset = pbyBuffer;
    size_t uLeftSize = uBufferSize;
    KNetUpgradeNode* pUpgradeNode = NULL;

    KS2C_Sync_Upgrade_Queue_Data* pPak = (KS2C_Sync_Upgrade_Queue_Data*)pbyBuffer;
    
    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KS2C_Sync_Upgrade_Queue_Data));
    pbyOffset += sizeof(KS2C_Sync_Upgrade_Queue_Data);
    uLeftSize -= sizeof(KS2C_Sync_Upgrade_Queue_Data);

    pPak->protocolID = s2c_sync_upgrade_queue_data;
    pPak->byMaxSlotCount = (BYTE)m_nMaxUpgradeCount;
    pPak->byFreeClearCDTimes = (BYTE)m_nFreeClearCDTimes;
    pPak->UpgradeNodeLength = (WORD)m_mapUpgradeNode.size();

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KNetUpgradeNode) * pPak->UpgradeNodeLength);

    pUpgradeNode = &pPak->UpgradeNode[0];
    for (KMAP_UPGRADE_NODE::iterator it = m_mapUpgradeNode.begin(); it != m_mapUpgradeNode.end(); ++it)
    {
        pUpgradeNode->nEndTime = it->first;
        pUpgradeNode->byUpgradeType = (BYTE)it->second.eType;
        pUpgradeNode->uHeroTemplateID = it->second.dwHeroTemplateID;
        ++pUpgradeNode;
    }

    pbyOffset += sizeof(KNetUpgradeNode) * pPak->UpgradeNodeLength;
    uLeftSize -= sizeof(KNetUpgradeNode) * pPak->UpgradeNodeLength;

    uUsedSize = uBufferSize - uLeftSize;

	bResult = true;
Exit0:
	return bResult;
}
