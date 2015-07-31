#include "stdafx.h"
#include "KHeroTrainingMgr.h"
#include "KRoleDBDataDef.h"
#include "KPlayer.h"
#include "KSO3World.h"
#include "KPlayerServer.h"

KHeroTrainingMgr::KHeroTrainingMgr()
{
    m_pPlayer   = NULL;
    m_nTeacherLevel = 0;
}

KHeroTrainingMgr::~KHeroTrainingMgr()
{
}

BOOL KHeroTrainingMgr::Init(KPlayer* pPlayer)
{
    assert(pPlayer);
    m_pPlayer = pPlayer;
    m_nTeacherLevel = g_pSO3World->m_Settings.m_ConstList.nInitTeacherLevel;
    return true;
}

void KHeroTrainingMgr::UnInit()
{
    m_pPlayer       = NULL;
    m_vecTrainingItems.clear();
}

void KHeroTrainingMgr::Activate()
{
    BOOL bRetCode = false;
    
    if ((g_pSO3World->m_nGameLoop - m_pPlayer->m_dwID) % GAME_FPS != 0)
        return;

    for (size_t i = 0; i < m_vecTrainingItems.size(); ++i)
    {
        const KHERO_TRAINING_ITEM& rTrainingItem = m_vecTrainingItems[i];

        if (g_pSO3World->m_nCurrentTime <= rTrainingItem.nEndTime)
            continue;

        EndTrainingHero(rTrainingItem.dwHeroTemplateID, 0);
        break; // EndTrainingHero会删除节点，迭代器会失效，所以break
    }

Exit0:
    return;
}

BOOL KHeroTrainingMgr::UpgradeTeacherLevel()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KTrainingTeacher* pInfo = NULL;

    pInfo = g_pSO3World->m_Settings.m_TrainingTeacher.GetByID(m_nTeacherLevel);
    KGLOG_PROCESS_ERROR(pInfo);

    KGLOG_PROCESS_ERROR(m_nTeacherLevel < m_pPlayer->m_nLevel);

    bRetCode = m_pPlayer->m_MoneyMgr.CanAddMoney(emotMoney, -pInfo->nCostMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_UpgradeQueue.CanAddUpgradeNode(eutUpgradeTrainTeacher, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, -pInfo->nCostMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    ++m_nTeacherLevel;
    g_PlayerServer.DoSyncHeroTrainTeacherLevel(m_pPlayer->m_nConnIndex, m_nTeacherLevel);

    m_pPlayer->OnEvent(peBasketballCourtLevelup, m_nTeacherLevel);

    bRetCode = m_pPlayer->m_UpgradeQueue.AddUpgradeNode(eutUpgradeTrainTeacher, pInfo->nTime, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KHeroTrainingMgr::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL                    bResult         = false;
    int                     nUsedCount      = 0;
    KHERO_TRAINING_ITEM     sTrainingItem;

    assert(pLoadBuf);

    const T3DB::KPBHeroTrainingMgr& HeroTrainingMgr = pLoadBuf->herotrainingmgr();
    const T3DB::KPB_HEROTRAINING_DATA_HEADER& Header = HeroTrainingMgr.trainingdataheader();

    m_nTeacherLevel = Header.teacherlevel();

    nUsedCount = HeroTrainingMgr.trainingdata_size();

    for (int i = 0; i < nUsedCount; ++i)
    {
        const T3DB::KPB_HEROTRAINING_DATA& PBTrainingData = HeroTrainingMgr.trainingdata(i);

        sTrainingItem.dwHeroTemplateID  = PBTrainingData.herotemplateid();
        sTrainingItem.nEndTime          = PBTrainingData.endtime();
        sTrainingItem.dwTrainingID      = PBTrainingData.trainingid();

        m_vecTrainingItems.push_back(sTrainingItem);
    }

    if (m_pPlayer->m_eConnectType == eLogin)
        g_PlayerServer.DoSyncHeroTrainData(m_pPlayer);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroTrainingMgr::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                    bResult         = false;
    T3DB::KPBHeroTrainingMgr* pHeroTrainingMgr = NULL;
    T3DB::KPB_HEROTRAINING_DATA_HEADER* pHeader = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pHeroTrainingMgr = pSaveBuf->mutable_herotrainingmgr();
    pHeader = pHeroTrainingMgr->mutable_trainingdataheader();

    pHeader->set_teacherlevel(m_nTeacherLevel);

    for (size_t i = 0; i < m_vecTrainingItems.size(); ++i)
    {
        const KHERO_TRAINING_ITEM& rTrainingItem = m_vecTrainingItems[i];
        T3DB::KPB_HEROTRAINING_DATA* pTrainingData = pHeroTrainingMgr->add_trainingdata();

        pTrainingData->set_herotemplateid(rTrainingItem.dwHeroTemplateID);
        pTrainingData->set_endtime(rTrainingItem.nEndTime);
        pTrainingData->set_trainingid(rTrainingItem.dwTrainingID);
    }

    bResult = true;
Exit0:
    return bResult;
}

const KVEC_TRAINING_ITEMS* KHeroTrainingMgr::GetAllTrainingItems() const
{
    return &m_vecTrainingItems;
}

BOOL KHeroTrainingMgr::BeginTrainingHero(uint32_t dwHeroTemplateID, uint32_t dwTrainingID)
{
	BOOL                    bResult         = false;
	BOOL                    bRetCode        = false;
    KTRAINING_TEMPLATE      TrainingTemplate;
    KHERO_TRAINING_ITEM     sTrainingItem   = {ERROR_ID, 0, ERROR_ID};
	DWORD                   dwTotalTime     = 0U;
    KHeroData*              pHeroData       = NULL;
    KUSING_EQUIP_INFO*      pInfo           = NULL;

    pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_nLevel <= m_nTeacherLevel);

    pInfo = m_pPlayer->m_Gym.GetHeroUsingEquipInfo(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pInfo == NULL);

    bRetCode = m_pPlayer->m_UpgradeQueue.HasFreeSlot();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_Settings.m_TrainingTemplateMgr.GetTemplate(dwTrainingID, pHeroData->m_nLevel, TrainingTemplate);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, -TrainingTemplate.m_nCostMeony);
    KGLOG_PROCESS_ERROR(bRetCode);
    
    PLAYER_LOG(m_pPlayer, "money,costmoney,%d,%s,%d-%d,%d,%d", emotMoney, "begintrainhero", 0, 0, 0, TrainingTemplate.m_nCostMeony);

    dwTotalTime = TrainingTemplate.m_nMiniutes * SECONDS_PER_MINUTE;

    sTrainingItem.dwHeroTemplateID  = dwHeroTemplateID;
    sTrainingItem.dwTrainingID      = dwTrainingID;
    sTrainingItem.nEndTime          = (int)g_pSO3World->m_nCurrentTime + dwTotalTime;

    bRetCode = AddTrainingItem(sTrainingItem);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoBeginTrainingHeroRespond(m_pPlayer->m_nConnIndex, dwHeroTemplateID, dwTotalTime, dwTrainingID, true);

    bRetCode = m_pPlayer->m_UpgradeQueue.AddUpgradeNode(eutUpgradeHeroTraining, dwTotalTime, dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_pPlayer->OnEvent(peUseHeroTraining, (int)dwHeroTemplateID, (int)dwTrainingID);

    g_LogClient.DoFlowRecord(
        frmtHeroTrainning, htfrBeginTrain,
        "%s,%d,%u,%d",
        m_pPlayer->m_szName,
        TrainingTemplate.m_nCostMeony,
        dwHeroTemplateID,
        TrainingTemplate.m_nMiniutes
    );

    bResult = true;
Exit0:
    if (!bResult)
        g_PlayerServer.DoBeginTrainingHeroRespond(m_pPlayer->m_nConnIndex, dwHeroTemplateID, dwTotalTime, dwTrainingID, false);
	return bResult;
}

BOOL KHeroTrainingMgr::EndTrainingHero(uint32_t dwHeroTemplateID, int nCostMoney)
{
	BOOL        bResult     = false;
	BOOL        bRetCode    = false;
    int         nExp        = 0;
    KVIPConfig* pVipConfig  = NULL;
	KHERO_TRAINING_ITEM sTrainingItem;
    KTRAINING_TEMPLATE TrainingTemplate;
    KReport_Hero_Finish_Basketball_Training param;
    KHeroData* pHeroData = NULL;

    bRetCode = RemoveTrainingItem(dwHeroTemplateID, sTrainingItem);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoEndTrainingHeroRespond(m_pPlayer->m_nConnIndex, dwHeroTemplateID, true);

    bRetCode = m_pPlayer->HasHero(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    bRetCode = g_pSO3World->m_Settings.m_TrainingTemplateMgr.GetTemplate(sTrainingItem.dwTrainingID, pHeroData->m_nLevel, TrainingTemplate);
    KGLOG_PROCESS_ERROR(bRetCode);
   
    nExp = TrainingTemplate.m_nBaseExp;

    if (m_pPlayer->IsVIP())
    {
        pVipConfig = g_pSO3World->m_Settings.m_VIPConfig.GetByID(m_pPlayer->m_nVIPLevel);
        KGLOG_PROCESS_ERROR(pVipConfig);
        nExp = (pVipConfig->nTrainingExpPercent + HUNDRED_NUM) * nExp / HUNDRED_NUM;
    }
    
    param.heroTemplateID     = dwHeroTemplateID;
    param.trainingTemplateID = (unsigned short)TrainingTemplate.m_dwID;
    param.addExp = nExp;

    bRetCode = m_pPlayer->m_Secretary.AddReport(KREPORT_EVENT_HERO_FINISH_BASKETBALL_TRAINING, (BYTE*)&param, sizeof(param));
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->AddHeroExpNoAdditional(sTrainingItem.dwHeroTemplateID, nExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_LogClient.DoFlowRecord(
        frmtHeroTrainning, htfrEndTrain,
        "%s,%u,%d,%d",
        m_pPlayer->m_szName,
        dwHeroTemplateID,
        nExp,
        nCostMoney
    );

	bResult = true;
Exit0:
    if (!bResult)
        g_PlayerServer.DoEndTrainingHeroRespond(m_pPlayer->m_nConnIndex, dwHeroTemplateID, false);
	return bResult;
}

BOOL KHeroTrainingMgr::IsTraining(uint32_t dwHeroTemplateID) const
{
    KHERO_TRAINING_ITEM sItem;
    return GetTrainingItem(dwHeroTemplateID, sItem);
}

BOOL KHeroTrainingMgr::GetSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    BYTE* pbyOffset = pbyBuffer;
    size_t uLeftSize = uBufferSize;
    KS2C_Sync_Herotrain_Data* pPak = (KS2C_Sync_Herotrain_Data*)pbyBuffer;
    KNET_HEROTRAINING_DATA* pData = NULL;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KS2C_Sync_Herotrain_Data));
    pbyOffset += sizeof(KS2C_Sync_Herotrain_Data);
    uLeftSize -= sizeof(KS2C_Sync_Herotrain_Data);

    pPak->protocolID = s2c_sync_herotrain_data;
    pPak->byTeacherLevel = (BYTE)m_nTeacherLevel;
    pPak->dataLength = (WORD)m_vecTrainingItems.size();

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KNET_HEROTRAINING_DATA) * pPak->dataLength);
    pbyOffset += sizeof(KNET_HEROTRAINING_DATA) * pPak->dataLength;
    uLeftSize -= sizeof(KNET_HEROTRAINING_DATA) * pPak->dataLength;

    pData = &pPak->data[0];
    for (KVEC_TRAINING_ITEMS::iterator it = m_vecTrainingItems.begin(); it != m_vecTrainingItems.end(); ++it)
    {
        if (it->dwHeroTemplateID == 0)
            continue;

        pData->wHeroTemplateID = (WORD)it->dwHeroTemplateID;
        pData->uEndTime = it->nEndTime;
        pData->wTrainingID = (WORD)it->dwTrainingID;

        ++pData;
    }

    uUsedSize = uBufferSize - uLeftSize;

	bResult = true;
Exit0:
	return bResult;
}

void KHeroTrainingMgr::OnUpgradeQueueFinished(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID, int nCostMoney)
{
    KG_PROCESS_ERROR(eType == eutUpgradeHeroTraining);
    KGLOG_PROCESS_ERROR(dwHeroTemplateID > 0);

    EndTrainingHero(dwHeroTemplateID, nCostMoney);

Exit0:
    return;
}

//////////////////////////////////////////////////////////////////////////

BOOL KHeroTrainingMgr::GetTrainingItem(uint32_t dwTemplateID, KHERO_TRAINING_ITEM& rTrainingItem) const
{
    BOOL bResult = false;

    for (KVEC_TRAINING_ITEMS::const_iterator constIt = m_vecTrainingItems.begin(); constIt != m_vecTrainingItems.end(); ++constIt)
    {
        const KHERO_TRAINING_ITEM& rCurItem = *constIt;
        if (rCurItem.dwHeroTemplateID == dwTemplateID)
        {
            rTrainingItem = rCurItem;
            bResult = true;
            break;
        }
    }

    return bResult;
}

BOOL KHeroTrainingMgr::RemoveTrainingItem(uint32_t dwTemplateID, KHERO_TRAINING_ITEM& rRemoved)
{
    BOOL bResult = false;

    for (KVEC_TRAINING_ITEMS::iterator it = m_vecTrainingItems.begin(); it != m_vecTrainingItems.end(); ++it)
    {
        KHERO_TRAINING_ITEM& rCurItem = *it;
        if (rCurItem.dwHeroTemplateID == dwTemplateID)
        {
            rRemoved = rCurItem;
            
            m_vecTrainingItems.erase(it);
            bResult = true;
            break;
        }
    }

    return bResult;
}

BOOL KHeroTrainingMgr::AddTrainingItem(const KHERO_TRAINING_ITEM& rNewItem)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = IsTraining(rNewItem.dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(!bRetCode);

    m_vecTrainingItems.push_back(rNewItem);

    bResult = true;
Exit0:
    return bResult;
}
