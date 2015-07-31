#include "stdafx.h"
#include "KBusinessStreet.h"
#include "GlobalMacro.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"

bool KStoreInfo::CanGetMoney()
{
    return (nGetMoneyTimes == 0 || g_pSO3World->m_nCurrentTime >= nNextGetMoneyTime);
}

KBusinessStreet::KBusinessStreet()
{
    m_pPlayer = NULL;
    m_nBuyLandTimes = 0;
    m_bNeedNotify = true;
}

KBusinessStreet::~KBusinessStreet()
{
}

BOOL KBusinessStreet::Init(KPlayer* pPlayer)
{
    m_pPlayer = pPlayer;
    
    return true;
}

void KBusinessStreet::UnInit()
{
}

BOOL KBusinessStreet::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL bResult  = false;
    KStoreInfo info;
    std::pair<KMAP_STORE::iterator, bool> InsRet;

    assert(pLoadBuf);

    const T3DB::KPBBusinessStreet& Street = pLoadBuf->businessstreet();
    const T3DB::KPB_BUSINESSSTREET_DATA& StreetData = Street.streetdata();
    m_nBuyLandTimes = StreetData.buylandtimes();

    for (int i = 0; i < StreetData.freelandids_size(); ++i)
    {
        m_vecFreeLandID.push_back(StreetData.freelandids(i));
    }

    for (int i = 0; i < StreetData.store_size(); ++i)
    {
        const T3DB::KPB_STORE_DATA& Store = StreetData.store(i);

        info.dwID               = Store.id();
        info.eType              = (KSTORE_TYPE)Store.type();
        info.nGetMoneyTimes     = Store.getmoneytimes();
        info.nLevel             = Store.level();
        info.nNextGetMoneyTime  = Store.nextgetmoneytime();
        info.nState             = Store.state();

        InsRet = m_mapStore.insert(std::make_pair(info.dwID, info));
        KGLOG_PROCESS_ERROR(InsRet.second);
    }

    if (m_pPlayer->m_eConnectType == eLogin)
    {
        g_PlayerServer.DoBSSyncData(m_pPlayer);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult  = false;
    T3DB::KPB_BUSINESSSTREET_DATA* pStreetData = NULL;
    T3DB::KPBBusinessStreet* pStreet = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pStreet = pSaveBuf->mutable_businessstreet();
    pStreetData = pStreet->mutable_streetdata();

    pStreetData->set_buylandtimes(m_nBuyLandTimes);

    for (size_t i = 0; i < m_vecFreeLandID.size(); ++i)
    {
        pStreetData->add_freelandids(m_vecFreeLandID[i]);
    }

    for (KMAP_STORE::iterator it = m_mapStore.begin(); it != m_mapStore.end(); ++it)
    {
        T3DB::KPB_STORE_DATA* pStoreData = pStreetData->add_store();

        pStoreData->set_id(it->second.dwID);
        pStoreData->set_type(it->second.eType);
        pStoreData->set_level(it->second.nLevel);
        pStoreData->set_getmoneytimes(it->second.nGetMoneyTimes);
        pStoreData->set_nextgetmoneytime(it->second.nNextGetMoneyTime);
        pStoreData->set_state(it->second.nState);
    }

    bResult = true;
Exit0:
    return bResult;
}

KStoreInfo* KBusinessStreet::GetStore(uint32_t dwID)
{
    KStoreInfo* pResult = NULL;
    KMAP_STORE::iterator it;

    it = m_mapStore.find(dwID);
    KGLOG_PROCESS_ERROR(it != m_mapStore.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

void KBusinessStreet::Activate()
{
    KVIPConfig* pVipConfig = NULL;
    KG_PROCESS_ERROR(m_pPlayer->IsVIP());
    KG_PROCESS_ERROR((g_pSO3World->m_nGameLoop - m_pPlayer->m_dwID) % (GAME_FPS * 60) == 0);

    pVipConfig = g_pSO3World->m_Settings.m_VIPConfig.GetByID(m_pPlayer->m_nVIPLevel);
    KGLOG_PROCESS_ERROR(pVipConfig);

    if (pVipConfig->bAutoGetMoney)
        GetAllMoney(true);
    else
        NotifyGetMoney();

Exit0:
    return;
}

BOOL KBusinessStreet::BuyLandRequest(int nLandID)
{
	BOOL bResult    = false;
	BOOL bRetCode   = false;
    int  nCost      = 0;
    KCUSTOM_CONSUME_INFO CCInfo;    

    KGLOG_PROCESS_ERROR(m_nBuyLandTimes < MAX_BUY_LAND_COUNT);

    nCost = g_pSO3World->m_Settings.m_ConstList.nLandCost[m_nBuyLandTimes];
    KGLOG_PROCESS_ERROR(nCost > 0);

    memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));

    CCInfo.nValue1 = (int)nLandID;

    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctBusinessStreetBuyLand, nCost, &CCInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KBusinessStreet::BuyLandResult(int nLandID)
{
    ++m_nBuyLandTimes;
    m_vecFreeLandID.push_back(nLandID);
    
    g_PlayerServer.DoBSBuyLandResult(m_pPlayer->m_nConnIndex, nLandID, m_nBuyLandTimes);

    m_pPlayer->OnEvent(peBuyLand);

    return true;
}

BOOL KBusinessStreet::BuildStoreRequest(int nLandID, KSTORE_TYPE eTargetType)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    std::pair<KMAP_STORE::iterator, bool> InsRet;
    KStoreLevelData* pStoreInfo = NULL;
    std::vector<int>::iterator it;

    KGLOG_PROCESS_ERROR(eTargetType > estypeInvalid && eTargetType < estypeTotal);
    
    bRetCode = m_pPlayer->m_UpgradeQueue.HasFreeSlot();
    KGLOG_PROCESS_ERROR(bRetCode);

    it = std::find(m_vecFreeLandID.begin(), m_vecFreeLandID.end(), nLandID);
    KGLOG_PROCESS_ERROR(it != m_vecFreeLandID.end());

    pStoreInfo = g_pSO3World->m_Settings.m_Store[eTargetType].GetByID(0);
    KGLOG_PROCESS_ERROR(pStoreInfo);

    if (pStoreInfo->nCostMoney > 0)
    {
        bRetCode = m_pPlayer->m_MoneyMgr.CanAddMoney(emotMoney, -pStoreInfo->nCostMoney);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (pStoreInfo->nCostCoin > 0)
    {
        bRetCode = m_pPlayer->m_MoneyMgr.CanAddMoney(emotCoin, -pStoreInfo->nCostCoin);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    // 检查结束

    if (pStoreInfo->nCostMoney > 0)
    {
        bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, -pStoreInfo->nCostMoney);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (pStoreInfo->nCostCoin > 0)
    {
        // 请求paysys扣费
        KCUSTOM_CONSUME_INFO CCInfo;
        memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));

        CCInfo.nValue1 = nLandID;
        CCInfo.nValue2 = (int)eTargetType;
        bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctBusinessStreetBuildStore, pStoreInfo->nCostCoin, &CCInfo);
        KGLOG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    bRetCode = BuildStoreResult(nLandID, eTargetType);
    KGLOG_PROCESS_ERROR(bRetCode);
    
Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KBusinessStreet::BuildStoreResult(int nLandID, KSTORE_TYPE eTargetType)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    std::pair<KMAP_STORE::iterator, bool> InsRet;
    KStoreInfo info;
    KStoreLevelData* pStoreInfo = NULL;
    std::vector<int>::iterator it;

    KGLOG_PROCESS_ERROR(eTargetType > estypeInvalid && eTargetType < estypeTotal);

    bRetCode = m_pPlayer->m_UpgradeQueue.HasFreeSlot();
    KGLOG_PROCESS_ERROR(bRetCode);

    it = std::find(m_vecFreeLandID.begin(), m_vecFreeLandID.end(), nLandID);
    KGLOG_PROCESS_ERROR(it != m_vecFreeLandID.end());
    m_vecFreeLandID.erase(it);

    pStoreInfo = g_pSO3World->m_Settings.m_Store[eTargetType].GetByID(0);
    KGLOG_PROCESS_ERROR(pStoreInfo);
    
    info.dwID = nLandID;
    info.eType = eTargetType;
    info.nGetMoneyTimes = 0;
    info.nLevel = 1;
    info.nNextGetMoneyTime = 0;
    info.nState = essBuilding;

    CalcNextGetMoneyTime(&info);

    InsRet = m_mapStore.insert(std::make_pair(info.dwID, info));
    KGLOG_PROCESS_ERROR(InsRet.second);

    g_PlayerServer.DoBSSyncNewStore(
        m_pPlayer->m_nConnIndex, m_vecFreeLandID.size(), info.dwID, info.eType, info.nLevel, info.nGetMoneyTimes, info.nNextGetMoneyTime
    );

    bRetCode = m_pPlayer->m_UpgradeQueue.AddUpgradeNode(info.GetUpgradeNodeEnum(), pStoreInfo->nCDTime, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_pPlayer->OnEvent(peBuildStore);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::UpgradeStoreRequest(uint32_t dwStoreID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KStoreInfo* pInfo = NULL;
    KStoreLevelData* pStoreLevelInfo = NULL;

    pInfo = GetStore(dwStoreID);
    KGLOG_PROCESS_ERROR(pInfo);
    KGLOG_PROCESS_ERROR(pInfo->eType > estypeInvalid && pInfo->eType < estypeTotal);
    KGLOG_PROCESS_ERROR(pInfo->nLevel < m_pPlayer->m_nLevel);

    pStoreLevelInfo = g_pSO3World->m_Settings.m_Store[pInfo->eType].GetByID(pInfo->nLevel);
    KGLOG_PROCESS_ERROR(pStoreLevelInfo);

    bRetCode = m_pPlayer->m_UpgradeQueue.CanAddUpgradeNode(pInfo->GetUpgradeNodeEnum(), 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (pStoreLevelInfo->nCostMoney > 0)
    {
        bRetCode = m_pPlayer->m_MoneyMgr.CanAddMoney(emotMoney, -pStoreLevelInfo->nCostMoney);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (pStoreLevelInfo->nCostCoin > 0)
    {
        bRetCode = m_pPlayer->m_MoneyMgr.CanAddMoney(emotCoin, -pStoreLevelInfo->nCostCoin);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    // 检查结束

    if (pStoreLevelInfo->nCostMoney > 0)
    {
        bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, -pStoreLevelInfo->nCostMoney);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (pStoreLevelInfo->nCostCoin > 0)
    {
        // 请求paysys扣费
        KCUSTOM_CONSUME_INFO CCInfo;
        memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));

        CCInfo.nValue1 = (int)dwStoreID;
        bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctBusinessStreetUpgradeStore, pStoreLevelInfo->nCostCoin, &CCInfo);
        KGLOG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    UpgradeStoreResult(dwStoreID);

Exit1:
	g_LogClient.DoFlowRecord(
        frmtBusinessStreet, bsfrUpgrade,
        "%s,%d,%d,%u,%d,%d",
        m_pPlayer->m_szName,
        pStoreLevelInfo->nCostMoney,
        pStoreLevelInfo->nCostCoin,
        pInfo->dwID,
        pInfo->eType,
        pInfo->nLevel + 1
    );
	
	bResult = true;
Exit0:
	return bResult;
}

BOOL KBusinessStreet::UpgradeStoreResult(uint32_t dwStoreID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pInfo = NULL;
    KStoreLevelData* pStoreLevelInfo = NULL;

    pInfo = GetStore(dwStoreID);
    KGLOG_PROCESS_ERROR(pInfo);
    KGLOG_PROCESS_ERROR(pInfo->eType > estypeInvalid && pInfo->eType < estypeTotal);
    KGLOG_PROCESS_ERROR(pInfo->nLevel < m_pPlayer->m_nLevel);

    pStoreLevelInfo = g_pSO3World->m_Settings.m_Store[pInfo->eType].GetByID(pInfo->nLevel);
    KGLOG_PROCESS_ERROR(pStoreLevelInfo);

    bRetCode = m_pPlayer->m_UpgradeQueue.CanAddUpgradeNode(pInfo->GetUpgradeNodeEnum(), 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    ++pInfo->nLevel;
    pInfo->nState = essUpgrade;

    g_PlayerServer.DoBSUpgradeStoreResult(m_pPlayer->m_nConnIndex, dwStoreID, pInfo->nLevel);

    m_pPlayer->OnEvent(peStoreLevelup, pInfo->eType + 1, pInfo->nLevel);

    bRetCode = m_pPlayer->m_UpgradeQueue.AddUpgradeNode(pInfo->GetUpgradeNodeEnum(), pStoreLevelInfo->nCDTime, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::CanGetMoney(uint32_t dwStoreID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    KStoreLevelData* pStoreLevelData = NULL;

    pStoreInfo = GetStore(dwStoreID);
    KGLOG_PROCESS_ERROR(pStoreInfo);
    KGLOG_PROCESS_ERROR(pStoreInfo->eType > estypeInvalid && pStoreInfo->eType < estypeTotal);

    KG_PROCESS_ERROR(pStoreInfo->nGetMoneyTimes == 0 || g_pSO3World->m_nCurrentTime >= pStoreInfo->nNextGetMoneyTime);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::GetMoney(uint32_t dwStoreID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    KStoreLevelData* pStoreLevelData = NULL;

    pStoreInfo = GetStore(dwStoreID);
    KGLOG_PROCESS_ERROR(pStoreInfo);
    KGLOG_PROCESS_ERROR(pStoreInfo->eType > estypeInvalid && pStoreInfo->eType < estypeTotal);

    pStoreLevelData = g_pSO3World->m_Settings.m_Store[pStoreInfo->eType].GetByID(pStoreInfo->nLevel);
    KGLOG_PROCESS_ERROR(pStoreLevelData);

    bRetCode = pStoreInfo->CanGetMoney();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, pStoreLevelData->nAwardMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    {
        KMessage70_Para cPara;
        cPara.nMoneyCount = pStoreLevelData->nAwardMoney;
        g_PlayerServer.DoDownwardNotify(m_pPlayer, KMESSAGE_BUSINESSSTREET_GET_MONEY, &cPara, sizeof(cPara));
    }

    m_pPlayer->OnEvent(peObtainMoneyInBusinessStreet, emotMoney + 1, pStoreLevelData->nAwardMoney);

    ++pStoreInfo->nGetMoneyTimes;
    CalcNextGetMoneyTime(pStoreInfo);

    g_PlayerServer.DoBSSyncGetMoneyInfo(
        m_pPlayer->m_nConnIndex, dwStoreID, pStoreInfo->nGetMoneyTimes, pStoreInfo->nNextGetMoneyTime
    );

    PLAYER_LOG(m_pPlayer, "active,getstoremoney,%s,%u,%d,%d,%d",
        m_pPlayer->m_szName,
        pStoreInfo->dwID,
        pStoreInfo->eType,
        pStoreInfo->nLevel,
        pStoreLevelData->nAwardMoney
    );

    g_LogClient.DoFlowRecord(
        frmtBusinessStreet, bsfrGetMoney,
        "%s,%u,%d,%d,%d",
        m_pPlayer->m_szName,
        pStoreInfo->dwID,
        pStoreInfo->eType,
        pStoreInfo->nLevel,
        pStoreLevelData->nAwardMoney
    );

	bResult = true;
Exit0:
	return bResult;
}

BOOL KBusinessStreet::NotifyGetMoney()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    KStoreLevelData* pStoreLevelData = NULL;
    int nTotalMoney = 0;

    for (KMAP_STORE::iterator it = m_mapStore.begin(); it != m_mapStore.end(); ++it)
    {
        pStoreInfo = &it->second;
        KGLOG_PROCESS_ERROR(pStoreInfo);
        KGLOG_PROCESS_ERROR(pStoreInfo->eType > estypeInvalid && pStoreInfo->eType < estypeTotal);

        pStoreLevelData = g_pSO3World->m_Settings.m_Store[pStoreInfo->eType].GetByID(pStoreInfo->nLevel);
        KGLOG_PROCESS_ERROR(pStoreLevelData);

        bRetCode = pStoreInfo->CanGetMoney();
        if (!bRetCode)
            continue;

        nTotalMoney += pStoreLevelData->nAwardMoney;
    }

    if (nTotalMoney == 0)
        m_bNeedNotify = true;

    if (m_bNeedNotify && nTotalMoney)
    {
        m_bNeedNotify = false;
        m_pPlayer->m_Secretary.AddReport(KREPORT_EVENT_CANCOLLECT_PROFITS, NULL, 0);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::GetAllMoney(BOOL bAuto)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    KStoreLevelData* pStoreLevelData = NULL;
    KReport_Autocollect_Profits param;
    int nTotalMoney = 0;

    for (KMAP_STORE::iterator it = m_mapStore.begin(); it != m_mapStore.end(); ++it)
    {
        pStoreInfo = &it->second;
        KGLOG_PROCESS_ERROR(pStoreInfo);
        KGLOG_PROCESS_ERROR(pStoreInfo->eType > estypeInvalid && pStoreInfo->eType < estypeTotal);

        pStoreLevelData = g_pSO3World->m_Settings.m_Store[pStoreInfo->eType].GetByID(pStoreInfo->nLevel);
        KGLOG_PROCESS_ERROR(pStoreLevelData);

        bRetCode = pStoreInfo->CanGetMoney();
        if (!bRetCode)
            continue;

        nTotalMoney += pStoreLevelData->nAwardMoney;

        m_pPlayer->OnEvent(peObtainMoneyInBusinessStreet, emotMoney + 1, pStoreLevelData->nAwardMoney);

        ++pStoreInfo->nGetMoneyTimes;
        CalcNextGetMoneyTime(pStoreInfo);

        g_PlayerServer.DoBSSyncGetMoneyInfo(
            m_pPlayer->m_nConnIndex, pStoreInfo->dwID, pStoreInfo->nGetMoneyTimes, pStoreInfo->nNextGetMoneyTime
        );
    }

    bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, nTotalMoney);
    KGLOG_PROCESS_ERROR(bRetCode);
    if (bAuto && nTotalMoney)
    {
        param.addMoney = nTotalMoney;
        m_pPlayer->m_Secretary.AddReport(KREPORT_EVENT_AUTOCOLLECT_PROFITS, (BYTE*)&param, sizeof(param));
    }

    PLAYER_LOG(m_pPlayer, "active,getallmoney,%d", nTotalMoney);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::ClearGetMoneyCDRequest(uint32_t dwStoreID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    KStoreLevelData* pStoreLevelData = NULL;
    int nCostCoin = 0;

    pStoreInfo = GetStore(dwStoreID);
    KGLOG_PROCESS_ERROR(pStoreInfo);
    KGLOG_PROCESS_ERROR(pStoreInfo->eType > estypeInvalid && pStoreInfo->eType < estypeTotal);

    pStoreLevelData = g_pSO3World->m_Settings.m_Store[pStoreInfo->eType].GetByID(pStoreInfo->nLevel);
    KGLOG_PROCESS_ERROR(pStoreLevelData);

    bRetCode = pStoreInfo->CanGetMoney();
    KGLOG_PROCESS_ERROR(!bRetCode);

    nCostCoin = ((pStoreInfo->nNextGetMoneyTime - g_pSO3World->m_nCurrentTime + SECONDS_PER_MINUTE) / SECONDS_PER_MINUTE) * g_pSO3World->m_Settings.m_ConstList.nDirectGetMoneyNeedCoinPerMinute;

    // 请求paysys扣费
	{
	    KCUSTOM_CONSUME_INFO CCInfo;
	    memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));

	    CCInfo.nValue1 = (int)dwStoreID;
	    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctBusinessStreetClearGetMoneyCD, nCostCoin, &CCInfo);
	    KGLOG_PROCESS_ERROR(bRetCode);

        g_LogClient.DoFlowRecord(
            frmtBusinessStreet, bsfrClearGetMoneyCD,
            "%s,%d,%d",
            m_pPlayer->m_szName,
            nCostCoin,
            pStoreInfo->nNextGetMoneyTime - g_pSO3World->m_nCurrentTime
        );
	}

    PLAYER_LOG(m_pPlayer, "clearcd,getmoney,%d,%d,%d", pStoreInfo->nLevel, pStoreInfo->nNextGetMoneyTime - g_pSO3World->m_nCurrentTime, nCostCoin);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::ClearGetMoneyCDResult(uint32_t dwStoreID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    KStoreLevelData* pStoreLevelData = NULL;

    pStoreInfo = GetStore(dwStoreID);
    KGLOG_PROCESS_ERROR(pStoreInfo);

    pStoreInfo->nNextGetMoneyTime = 0;

    g_PlayerServer.DoBSSyncGetMoneyInfo(m_pPlayer->m_nConnIndex, dwStoreID, pStoreInfo->nGetMoneyTimes, pStoreInfo->nNextGetMoneyTime);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::ClearAllGetMoneyCDRequest()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    KStoreLevelData* pStoreLevelData = NULL;
    int nTotalCostCoin = 0;

    for (KMAP_STORE::iterator it = m_mapStore.begin(); it != m_mapStore.end(); ++it)
    {
        pStoreInfo = &it->second;
        KGLOG_PROCESS_ERROR(pStoreInfo);
        KGLOG_PROCESS_ERROR(pStoreInfo->eType > estypeInvalid && pStoreInfo->eType < estypeTotal);

        pStoreLevelData = g_pSO3World->m_Settings.m_Store[pStoreInfo->eType].GetByID(pStoreInfo->nLevel);
        KGLOG_PROCESS_ERROR(pStoreLevelData);

        bRetCode = pStoreInfo->CanGetMoney();
        if (bRetCode)
            continue;

        nTotalCostCoin += ((pStoreInfo->nNextGetMoneyTime - g_pSO3World->m_nCurrentTime + SECONDS_PER_MINUTE) / SECONDS_PER_MINUTE) * g_pSO3World->m_Settings.m_ConstList.nDirectGetMoneyNeedCoinPerMinute;
    }
    
    // 请求paysys扣费
    {
        KCUSTOM_CONSUME_INFO CCInfo;
        memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));

        bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctBusinessStreetClearAllGetMoneyCD, nTotalCostCoin, NULL);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::ClearAllGetMoneyCDResult()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    
    for (KMAP_STORE::iterator it = m_mapStore.begin(); it != m_mapStore.end(); ++it)
    {
        pStoreInfo = &it->second;
        KGLOG_PROCESS_ERROR(pStoreInfo);

        bRetCode = pStoreInfo->CanGetMoney();
        if (bRetCode)
            continue;

        pStoreInfo->nNextGetMoneyTime = 0;

        g_PlayerServer.DoBSSyncGetMoneyInfo(
            m_pPlayer->m_nConnIndex, pStoreInfo->dwID, pStoreInfo->nGetMoneyTimes, pStoreInfo->nNextGetMoneyTime
        );
    }
    
    bResult = true;
Exit0:
    return bResult;
}

void KBusinessStreet::RefreshGetMoneyTimes()
{
    for (KMAP_STORE::iterator it = m_mapStore.begin(); it != m_mapStore.end(); ++it)
    {
        it->second.nGetMoneyTimes = 0;
        it->second.nNextGetMoneyTime = 0;
        g_PlayerServer.DoBSSyncGetMoneyInfo(m_pPlayer->m_nConnIndex, it->second.dwID, it->second.nGetMoneyTimes, it->second.nNextGetMoneyTime);
    }
}

BOOL KBusinessStreet::ChangeStoreTypeRequest(uint32_t dwStoreID, KSTORE_TYPE eTargetType)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;
    int nNeedCoin = 0;

    KGLOG_PROCESS_ERROR(eTargetType > estypeInvalid && eTargetType < estypeTotal);

    pStoreInfo = GetStore(dwStoreID);
    KGLOG_PROCESS_ERROR(pStoreInfo);
    KGLOG_PROCESS_ERROR(pStoreInfo->eType < eTargetType);

    bRetCode = m_pPlayer->m_UpgradeQueue.CanAddUpgradeNode(pStoreInfo->GetUpgradeNodeEnum(), 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    nNeedCoin = GetNeedCoin(eTargetType, pStoreInfo->nLevel);
    nNeedCoin -= GetNeedCoin(pStoreInfo->eType, pStoreInfo->nLevel);

    KGLOG_PROCESS_ERROR(nNeedCoin > 0);

    // 请求paysys扣费
	{
	    KCUSTOM_CONSUME_INFO CCInfo;
	    memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));

	    CCInfo.nValue1 = (int)dwStoreID;
	    CCInfo.nValue2 = (int)eTargetType;
	    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctBusinessStreetChangeStoreType, nNeedCoin, &CCInfo);
	    KGLOG_PROCESS_ERROR(bRetCode);
    }
	
    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::ChangeStoreTypeRespond(uint32_t dwStoreID, KSTORE_TYPE eTargetType)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KStoreInfo* pStoreInfo = NULL;

    KGLOG_PROCESS_ERROR(eTargetType > estypeInvalid && eTargetType < estypeTotal);

    pStoreInfo = GetStore(dwStoreID);
    KGLOG_PROCESS_ERROR(pStoreInfo);
    KGLOG_PROCESS_ERROR(pStoreInfo->eType < eTargetType);

    bRetCode = m_pPlayer->m_UpgradeQueue.CanAddUpgradeNode(pStoreInfo->GetUpgradeNodeEnum(), 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    pStoreInfo->eType = eTargetType;
    pStoreInfo->nState = essChangeType;

    g_PlayerServer.DoBSSyncStoreNewType(m_pPlayer->m_nConnIndex, dwStoreID, eTargetType);

    bRetCode = m_pPlayer->m_UpgradeQueue.AddUpgradeNode(
        pStoreInfo->GetUpgradeNodeEnum(), g_pSO3World->m_Settings.m_ConstList.nChangeStoreTypeCDTime, 0
    );
    KGLOG_PROCESS_ERROR(bRetCode);
    
    bResult = true;
Exit0:
    return bResult;
}

BOOL KBusinessStreet::GetLandSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize)
{
	BOOL                    bResult         = false;
	BOOL                    bRetCode        = false;
    size_t                  uLeftSize       = uBufferSize;
    BYTE*                   pbyOffset       = pbyBuffer;
    KS2C_Bs_Sync_Land_Data* pPak            = (KS2C_Bs_Sync_Land_Data*)pbyBuffer;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KS2C_Bs_Sync_Land_Data));
    pbyOffset += sizeof(KS2C_Bs_Sync_Land_Data);
    uLeftSize -= sizeof(KS2C_Bs_Sync_Land_Data);

    pPak->protocolID            = s2c_bs_sync_land_data;
    pPak->byBuyLandTimes        = (BYTE)m_nBuyLandTimes;
    pPak->byFreelandIDLength    = (WORD)m_vecFreeLandID.size();

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(unsigned char) * pPak->byFreelandIDLength);
    pbyOffset += sizeof(unsigned char) * pPak->byFreelandIDLength;
    uLeftSize -= sizeof(unsigned char) * pPak->byFreelandIDLength;

    for (int i = 0; i < pPak->byFreelandIDLength; ++i)
    {
        pPak->byFreelandID[i] = (BYTE)m_vecFreeLandID[i];
    }

    uUsedSize = uBufferSize - uLeftSize;

	bResult = true;
Exit0:
	return bResult;
}


BOOL KBusinessStreet::GetStoreSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    size_t                      uLeftSize       = uBufferSize;
    BYTE*                       pbyOffset       = pbyBuffer;
    KS2C_Bs_Sync_Store_Data*    pPak            = (KS2C_Bs_Sync_Store_Data*)pbyBuffer;
    KNET_STORE_DATA*            pStoreData      = NULL;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KS2C_Bs_Sync_Store_Data));
    pbyOffset += sizeof(KS2C_Bs_Sync_Store_Data);
    uLeftSize -= sizeof(KS2C_Bs_Sync_Store_Data);

    pPak->protocolID        = s2c_bs_sync_store_data;
    pPak->storedataLength   = (WORD)m_mapStore.size();

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KNET_STORE_DATA) * pPak->storedataLength);
    pbyOffset += sizeof(KNET_STORE_DATA) * pPak->storedataLength;
    uLeftSize -= sizeof(KNET_STORE_DATA) * pPak->storedataLength;

    pStoreData = &pPak->storedata[0];
    for (KMAP_STORE::iterator it = m_mapStore.begin(); it != m_mapStore.end(); ++it)
    {
        pStoreData->wID                 = (WORD)it->second.dwID;
        pStoreData->byType              = (BYTE)it->second.eType;
        pStoreData->byLevel             = (BYTE)it->second.nLevel;
        pStoreData->byGetMoneyTimes     = (BYTE)it->second.nGetMoneyTimes;
        pStoreData->nNextGetMoneyTime   = it->second.nNextGetMoneyTime;
        pStoreData->byState             = (BYTE)it->second.nState;
        ++pStoreData;
    }

    uUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

void KBusinessStreet::OnUpgradeQueueFinished(KUPGRADE_TYPE eUpgradeType)
{
    for (KMAP_STORE::iterator it = m_mapStore.begin(); it != m_mapStore.end(); ++it)
    {
        if (it->second.GetUpgradeNodeEnum() != eUpgradeType)
            continue;

        it->second.nState = essNormal;
        break;
    }
}

void KBusinessStreet::OnFirstLogin()
{
    for (int i = 0; i < INIT_LAND_COUNT; ++i)
    {
        m_vecFreeLandID.push_back(i);
    }

    g_PlayerServer.DoBSSyncData(m_pPlayer);
}

int  KBusinessStreet::GetStoreLevel(uint32_t dwID)
{
    int nLevel = 0;
    KStoreInfo* pStoreInfo = GetStore(dwID);
    KGLOG_PROCESS_ERROR(pStoreInfo);

    nLevel = pStoreInfo->nLevel;
Exit0:
    return nLevel;
}

void KBusinessStreet::CalcNextGetMoneyTime(KStoreInfo* pInfo)
{
    int nAddSeconds = 0;

    assert(pInfo);
    nAddSeconds = pInfo->nGetMoneyTimes * 3600;
    MAKE_IN_RANGE(nAddSeconds, 0, 3600 * 20);

    pInfo->nNextGetMoneyTime = g_pSO3World->m_nCurrentTime + nAddSeconds;
    return;
}

int  KBusinessStreet::GetNeedCoin(KSTORE_TYPE eType, int nLevel)
{
    int  nResult    = 0;    
    int  nNeedCoin  = 0;
    KStoreLevelData* pStoreLevelData = NULL;
    int i = 0;

    KGLOG_PROCESS_ERROR(eType > estypeInvalid && eType < estypeTotal);

    while (i < nLevel)
    {
        pStoreLevelData = g_pSO3World->m_Settings.m_Store[eType].GetByID(i);
        KGLOG_PROCESS_ERROR(pStoreLevelData);

        nNeedCoin += pStoreLevelData->nCostCoin;
        ++i;
    }
    
    nResult = nNeedCoin;
Exit0:
    return nNeedCoin;
}

int KBusinessStreet::GetStoreCount()
{
    return (int)m_mapStore.size();
}
