#include "stdafx.h"
#include "KGym.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"

KGym::KGym()
{
    m_pPlayer = NULL;
    m_nEquipLevel = 0;
}

KGym::~KGym()
{
}

BOOL KGym::Init(KPlayer* pOwner)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

    assert(pOwner);

    m_pPlayer = pOwner;
    m_nEquipLevel = 1;

	bResult = true;
Exit0:
	return bResult;
}

void KGym::UnInit()
{
}

BOOL KGym::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL bResult  = false;

    assert(pLoadBuf);

    const T3DB::KPBGym& Gym = pLoadBuf->gym();
    const T3DB::KPB_GYM_DATA& GymData = Gym.gymdata();

    m_nEquipLevel = GymData.equiplevel();

    for (int i = 0; i < GymData.info_size(); ++i)
    {
        const  T3DB::KPB_USINGEQUIP_INFO& Node = GymData.info(i);
        KUSING_EQUIP_INFO node;

        node.dwHeroTemplateID = Node.herotemplateid();
        node.eType = (KGYM_TYPE)Node.type();
        node.nCount = Node.count();
        node.nEndTime = Node.endtime();

        m_vecEquipInfo.push_back(node);
    }

    if (m_pPlayer->m_eConnectType == eLogin)
    {
        g_PlayerServer.DoSyncGymInfo(m_pPlayer);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KGym::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult  = false;
    T3DB::KPBGym* pGym = NULL;
    T3DB::KPB_GYM_DATA* pGymData = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pGym = pSaveBuf->mutable_gym();
    pGymData = pGym->mutable_gymdata();

    pGymData->set_equiplevel(m_nEquipLevel);

    for (size_t i = 0; i < m_vecEquipInfo.size(); ++i)
    {
        T3DB::KPB_USINGEQUIP_INFO* pNode = pGymData->add_info();

        pNode->set_herotemplateid(m_vecEquipInfo[i].dwHeroTemplateID);
        pNode->set_endtime(m_vecEquipInfo[i].nEndTime);
        pNode->set_type(m_vecEquipInfo[i].eType);
        pNode->set_count(m_vecEquipInfo[i].nCount);
    }

    bResult = true;
Exit0:
    return bResult;
}

void KGym::Activate()
{
    if ((g_pSO3World->m_nGameLoop - m_pPlayer->m_dwID) % GAME_FPS != 0)
        return;
    
    KVEC_EQUIP_INFO::iterator it = m_vecEquipInfo.begin();
    while (it != m_vecEquipInfo.end())
    {
        if (g_pSO3World->m_nCurrentTime < it->nEndTime)
        {
            ++it;
            continue;
        }
        
        HeroUseEquipFinished(it->dwHeroTemplateID, it->eType, it->nCount);
        it = m_vecEquipInfo.erase(it);
    }

Exit0:
    return;
}

BOOL KGym::UpgradeEquip()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KEquipLevelData* pData = NULL;

    KGLOG_PROCESS_ERROR(m_nEquipLevel < m_pPlayer->m_nLevel);
    
    pData = g_pSO3World->m_Settings.m_EquipLevelData.GetByID(m_nEquipLevel);
    KGLOG_PROCESS_ERROR(pData);
    
    bRetCode = m_pPlayer->m_MoneyMgr.CanAddMoney(emotMoney, -pData->nCostMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_UpgradeQueue.CanAddUpgradeNode(eutUpgradeGym, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, -pData->nCostMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    ++m_nEquipLevel;

    m_pPlayer->OnEvent(peGymMachineLevelup, m_nEquipLevel);
    
    g_PlayerServer.DoUpgradeGymEquipLevel(m_pPlayer->m_nConnIndex, m_nEquipLevel);

    bRetCode = m_pPlayer->m_UpgradeQueue.AddUpgradeNode(eutUpgradeGym, pData->nCDTime, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

KUPGRADE_TYPE g_GetUseGymUpgradeType(KGYM_TYPE eType)
{
    if (eType == egymWeightLift)
        return eutUpgradeHeroWeightLift;
    else if (eType == egymRunning)
        return eutUpgradeHeroRunning;
    else if (eType == egymBoxing)
        return eutUpgradeHeroBoxing;
    return eutInvalid;
}


KGYM_TYPE g_GetGymType(KUPGRADE_TYPE eType)
{
    if (eType == eutUpgradeHeroWeightLift)
        return egymWeightLift;
    else if (eType == eutUpgradeHeroRunning)
        return egymRunning;
    else if (eType == eutUpgradeHeroBoxing)
        return egymBoxing;
    return egymTotal;
}

BOOL KGym::HeroUseEquip(DWORD dwHeroTemplateID, KGYM_TYPE eType, int nCount)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int  nUsingCount = 0;
    KUSING_EQUIP_INFO* pExist = NULL;
    KUSING_EQUIP_INFO info;
    KHeroTalent* pTalentInfo = NULL;
    KHeroData* pHeroData = NULL;
    KVIPConfig* pVipConfig = NULL;
    int  nUsedTalentCount = 0;
    int  nNeedMoney = 0;
    int  nUseTime = 0;

    KGLOG_PROCESS_ERROR(eType >= egymWeightLift && eType < egymTotal);
    KGLOG_PROCESS_ERROR(nCount > 0);

    if (nCount > 1)
    {
        KGLOG_PROCESS_ERROR(m_pPlayer->IsVIP());

        pVipConfig = g_pSO3World->m_Settings.m_VIPConfig.GetByID(m_pPlayer->m_nVIPLevel);
        KGLOG_PROCESS_ERROR(pVipConfig);
        KGLOG_PROCESS_ERROR(pVipConfig->bContinuousUseGymEquip);
    }
    
    pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_wFreeTalent >= nCount); // 必须有可用的天赋点

    KGLOG_PROCESS_ERROR(pHeroData->m_wTalent[eType] + nCount <= m_nEquipLevel);

    pExist = GetHeroUsingEquipInfo(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pExist == NULL);

    // 检查是否有空位
    bRetCode = m_pPlayer->m_UpgradeQueue.HasFreeSlot();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_HeroTrainingMgr.IsTraining(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(!bRetCode);

    nUsedTalentCount = pHeroData->GetUsedTalentCount();
    
    for (int i = 0; i < nCount; ++i)
    {
        pTalentInfo = g_pSO3World->m_Settings.m_HeroTalent.GetByID(nUsedTalentCount + i);
        KGLOG_PROCESS_ERROR(pTalentInfo);   

        nNeedMoney += pTalentInfo->nCostMoney;
        nUseTime += pTalentInfo->nTime;
    }

    bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, -nNeedMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    info.dwHeroTemplateID   = dwHeroTemplateID;
    info.eType              = eType;
    info.nEndTime           = g_pSO3World->m_nCurrentTime + nUseTime;
    info.nCount             = nCount;

    m_vecEquipInfo.push_back(info);

    m_pPlayer->OnEvent(peUseGymMachine, eType + 1, nCount);

    g_PlayerServer.DoSyncHeroUseEquip(m_pPlayer->m_nConnIndex, info.dwHeroTemplateID, info.eType, info.nEndTime, info.nCount);

    bRetCode = m_pPlayer->m_UpgradeQueue.AddUpgradeNode(g_GetUseGymUpgradeType(eType), nUseTime, dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_LogClient.DoFlowRecord(
        frmtGym, gfrBeginTrain,
        "%s,%d,%u,%d,%d",
        m_pPlayer->m_szName,
        nNeedMoney,
        dwHeroTemplateID,
        nUseTime,
        eType
    );

	bResult = true;
Exit0:
	return bResult;
}

KUSING_EQUIP_INFO* KGym::GetHeroUsingEquipInfo(DWORD dwHeroTemplateID)
{
    for (size_t i = 0; i < m_vecEquipInfo.size(); ++i)
    {
        if (m_vecEquipInfo[i].dwHeroTemplateID == dwHeroTemplateID)
            return &m_vecEquipInfo[i];
    }

    return NULL;
}

int  KGym::GetUsingEquipCount(KGYM_TYPE eType)
{
    int nCount = 0;

    for (size_t i = 0; i < m_vecEquipInfo.size(); ++i)
    {
        if (m_vecEquipInfo[i].eType == eType)
            ++nCount;
    }

    return nCount;
}

BOOL KGym::HeroUseEquipFinished(DWORD dwHeroTemplateID, KGYM_TYPE eType, int nCount)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nStartCount = 0;
    KHeroData* pHeroData = NULL;

    KGLOG_PROCESS_ERROR(eType >= egymWeightLift && eType < egymTotal);

    pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_wFreeTalent > 0); // 必须有可用的天赋点

    nStartCount = pHeroData->m_wTalent[eType];
    pHeroData->m_wFreeTalent -= (WORD)nCount;
    pHeroData->m_wTalent[eType] += (WORD)nCount;

    switch(eType)
    {
    case egymWeightLift:
        {
            KReport_Hero_Finish_WeightLifting_Training param = {dwHeroTemplateID, (unsigned short)nCount, (unsigned short)nStartCount};
            bRetCode = m_pPlayer->m_Secretary.AddReport(KREPORT_EVENT_HERO_FINISH_WEIGHTLIFTING_TRAINING, (BYTE*)&param, sizeof(param));
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        break;
    case egymRunning:
        {
            KReport_Hero_Finish_Running_Training param = {dwHeroTemplateID, (unsigned short)nCount, (unsigned short)nStartCount};
            bRetCode = m_pPlayer->m_Secretary.AddReport(KREPORT_EVENT_HERO_FINISH_RUNNING_TRAINING, (BYTE*)&param, sizeof(param));
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        break;
    case egymBoxing:
        {
            KReport_Hero_Finish_Boxing_Training param = {dwHeroTemplateID, (unsigned short)nCount, (unsigned short)nStartCount};
            bRetCode = m_pPlayer->m_Secretary.AddReport(KREPORT_EVENT_HERO_FINISH_BOXING_TRAINING, (BYTE*)&param, sizeof(param));
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    g_PlayerServer.DoFinishUseEquip(m_pPlayer->m_nConnIndex, dwHeroTemplateID, pHeroData->m_wFreeTalent, eType, pHeroData->m_wTalent[eType]);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KGym::DirectEndUseEquipRequest(DWORD dwHeroTemplateID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int  nCoin    = 0;
    KUSING_EQUIP_INFO* pEquipInfo = NULL;

    pEquipInfo = GetHeroUsingEquipInfo(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pEquipInfo);

    KGLOG_PROCESS_ERROR(pEquipInfo->nEndTime > g_pSO3World->m_nCurrentTime);

    nCoin =  ((pEquipInfo->nEndTime - g_pSO3World->m_nCurrentTime + SECONDS_PER_MINUTE) / SECONDS_PER_MINUTE) * 
        g_pSO3World->m_Settings.m_ConstList.nGymCoinPerMinute;

    // request to paysys
	{
	    KCUSTOM_CONSUME_INFO CCInfo;
	    memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));

	    CCInfo.nValue1 = (int)dwHeroTemplateID;
	    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctDirectEndUseEquip, nCoin, &CCInfo);
	    KGLOG_PROCESS_ERROR(bRetCode);
	}

    PLAYER_LOG(m_pPlayer, "clearcd,herogym,%d,%d,%d", m_nEquipLevel, (pEquipInfo->nEndTime - g_pSO3World->m_nCurrentTime), nCoin);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KGym::DirectEndUseEquipResult(DWORD dwHeroTemplateID, int nCostMoney)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KUSING_EQUIP_INFO* pExist = NULL;

    pExist = GetHeroUsingEquipInfo(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pExist);

    bRetCode = HeroUseEquipFinished(dwHeroTemplateID, pExist->eType, pExist->nCount);
    KGLOG_PROCESS_ERROR(bRetCode);
	
    g_LogClient.DoFlowRecord(
        frmtGym, gfrClearTrainCD, 
        "%s,%d,%u,%d,%d",
        m_pPlayer->m_szName,
        nCostMoney,
        dwHeroTemplateID,
        pExist->nEndTime,
        pExist->eType
    );

    DelUsingEquipInfo(dwHeroTemplateID);    

	bResult = true;
Exit0:
	return bResult;
}

BOOL KGym::GetSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    BYTE* pbyOffset = pbyBuffer;
    size_t uLeftSize = uBufferSize;
    KS2C_Sync_Gym_Info* pPak = (KS2C_Sync_Gym_Info*)pbyOffset;
    KUSINGEQUIP_INFO* pUsingEquipInfo = NULL;

    assert(pbyBuffer);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KS2C_Sync_Gym_Info));
    pbyOffset += sizeof(KS2C_Sync_Gym_Info);
    uLeftSize -= sizeof(KS2C_Sync_Gym_Info);

    pPak->protocolID        = s2c_sync_gym_info;
    pPak->byEquipLevel      = (BYTE)m_nEquipLevel;
    pPak->UsingEquipLength  = (short)m_vecEquipInfo.size();

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KUSINGEQUIP_INFO) * pPak->UsingEquipLength);

    pUsingEquipInfo = &pPak->UsingEquip[0];
    for (int i = 0; i < pPak->UsingEquipLength; ++i)
    {
        pUsingEquipInfo->wHeroTemplateID    = (WORD)m_vecEquipInfo[i].dwHeroTemplateID;
        pUsingEquipInfo->byType             = (BYTE)m_vecEquipInfo[i].eType;
        pUsingEquipInfo->byCount            = (BYTE)m_vecEquipInfo[i].nCount;
        pUsingEquipInfo->nEndTime           = m_vecEquipInfo[i].nEndTime;
        ++pUsingEquipInfo;
    }

    pbyOffset += sizeof(KUSINGEQUIP_INFO) * pPak->UsingEquipLength;
    uLeftSize -= sizeof(KUSINGEQUIP_INFO) * pPak->UsingEquipLength;

    uUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

void KGym::OnUpgradeQueueFinished(KUPGRADE_TYPE eType, uint32_t uHeroTemplateID)
{
    BOOL bRetCode = false;
    KGYM_TYPE eGymType = g_GetGymType(eType);

    for (KVEC_EQUIP_INFO::iterator it = m_vecEquipInfo.begin(); it != m_vecEquipInfo.end(); ++it)
    {
        if (it->eType == eGymType && it->dwHeroTemplateID == uHeroTemplateID) 
        {
            bRetCode = HeroUseEquipFinished(it->dwHeroTemplateID, eGymType, it->nCount);
            KGLOG_PROCESS_ERROR(bRetCode);

            m_vecEquipInfo.erase(it);
            break;
        }
    }

Exit0:
    return;
}

void KGym::DelUsingEquipInfo(DWORD dwHeroTemplateID)
{
    for (KVEC_EQUIP_INFO::iterator it = m_vecEquipInfo.begin(); it != m_vecEquipInfo.end(); ++it)
    {
        if (it->dwHeroTemplateID == dwHeroTemplateID)
        {
            m_vecEquipInfo.erase(it);
            break;
        }
    }

    return;
}
