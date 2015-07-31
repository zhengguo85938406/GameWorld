#include "stdafx.h"
#include "KMoneyMgr.h"
#include "KRoleDBDataDef.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KSO3World.h"

KMoneyMgr::KMoneyMgr()
{
    m_nSafeBoxLevel = 1;
    m_nFreeCoin     = 0;
}

KMoneyMgr::~KMoneyMgr()
{

}

BOOL KMoneyMgr::Init(KPlayer* pPlayer)
{
    assert(pPlayer);
    memset(m_nMoneys, 0, sizeof(m_nMoneys));
    m_pPlayer = pPlayer;
    m_nSafeBoxLevel = 1;
    return true;
}

void KMoneyMgr::UnInit()
{
}

BOOL KMoneyMgr::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                        bResult         = false;
    T3DB::KPBMoneyMgr*            pMoney          = NULL;
    T3DB::KPB_ROLE_MONEY_DATA*  pRoleMoney  = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);
 
    pMoney = pSaveBuf->mutable_moneymgr();
    pRoleMoney = pMoney->mutable_rolemoneyinfo();
    pRoleMoney->set_safeboxlevel(m_nSafeBoxLevel);
    pRoleMoney->set_freecoin(m_nFreeCoin);

    for (int i = emotBegin; i < emotTotal; i++)
    {
        pRoleMoney->add_money(m_nMoneys[i]);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMoneyMgr::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    assert(pLoadBuf);

    const T3DB::KPBMoneyMgr& MoneyMgr = pLoadBuf->moneymgr();
    const T3DB::KPB_ROLE_MONEY_DATA& RoleMoney = MoneyMgr.rolemoneyinfo();
    
    m_nSafeBoxLevel = RoleMoney.safeboxlevel();
    m_nFreeCoin     = RoleMoney.freecoin();

    for (int i = emotBegin; i < emotTotal; i++)
    {
        m_nMoneys[i] = RoleMoney.money(i);
    }

    if (m_pPlayer->m_eConnectType == eLogin)
    {
        int nCharge = m_pPlayer->m_nCoin - m_nMoneys[emotCoin];

        m_nMoneys[emotCoin] = m_pPlayer->m_nCoin; // 登录把paysys金币设上，注意跨服不用
        
        if (nCharge > 0) // 离线有充值
        {
            m_pPlayer->AddVIPExp(nCharge / 100);
            bRetCode = m_pPlayer->SetPlayerValue(KPLAYER_VALUE_ID_FIRST_CHARGE, 1);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        g_PlayerServer.DoSyncCurrentMoneys(m_pPlayer);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMoneyMgr::CanAddMoney(ENUM_MONEY_TYPE eMoneyType, int nAddMoney)
{
    BOOL    bResult         = false;
    int     nCurrentMoney   = 0;

    KGLOG_PROCESS_ERROR(eMoneyType >= emotBegin && eMoneyType < emotTotal);

    nCurrentMoney = GetMoney(eMoneyType);

    if (nAddMoney < 0)
        KG_PROCESS_ERROR(nCurrentMoney + nAddMoney >= 0);
    else
        KG_PROCESS_ERROR(INT_MAX - nAddMoney > nCurrentMoney);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KMoneyMgr::AddMoney(ENUM_MONEY_TYPE eMoneyType, int nAddMoney)
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    int     nMoneyLimit     = 0;

    KG_PROCESS_SUCCESS(nAddMoney == 0);

    if (nAddMoney > 0)
        KG_PROCESS_SUCCESS(m_pPlayer->m_bIsLimited);

    bRetCode = CanAddMoney(eMoneyType, nAddMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_nMoneys[eMoneyType] += nAddMoney;

    g_PlayerServer.DoSyncMoneyChanged(m_pPlayer->m_nConnIndex, eMoneyType, GetMoney(eMoneyType));

    if (eMoneyType == emotMoney)
    {
        nMoneyLimit = GetGameMoneyLimit();
        if (m_nMoneys[eMoneyType] > nMoneyLimit && nAddMoney > 0)
        {
            bRetCode = m_pPlayer->m_Secretary.AddReport(KREPORT_EVENT_MONEY_OUT_OF_LIMIT, NULL, 0);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    if (nAddMoney > 0)
    {
        m_pPlayer->OnEvent(peObtainMoney, eMoneyType + 1, nAddMoney);
    }
    else
    {
        m_pPlayer->OnEvent(peConsumeMoney, eMoneyType + 1, -nAddMoney);
    }


Exit1:
    bResult = true;
Exit0:
    return bResult;
}

int KMoneyMgr::GetMoney(ENUM_MONEY_TYPE eMoneyType)
{
    int nCurrentMoney = ERROR_ID;
    KG_PROCESS_ERROR(eMoneyType >= emotBegin && eMoneyType < emotTotal);

    nCurrentMoney = m_nMoneys[eMoneyType];

    if (eMoneyType == emotCoin)
    {
        nCurrentMoney += m_nFreeCoin;
    }

Exit0:
    return nCurrentMoney;
}

BOOL KMoneyMgr::UpgradeSafeBox()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KSafeBox* pData = NULL;
    KUPGRADE_TYPE eUpgradeType = eutInvalid;

    KGLOG_PROCESS_ERROR(m_nSafeBoxLevel <= m_pPlayer->m_nLevel);

    pData = g_pSO3World->m_Settings.m_SafeBox.GetByID(m_nSafeBoxLevel);
    KGLOG_PROCESS_ERROR(pData);

    bRetCode = m_pPlayer->m_MoneyMgr.CanAddMoney(emotMoney, -pData->nCostMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_UpgradeQueue.CanAddUpgradeNode(eutUpgradeSafeBox, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_pPlayer->m_MoneyMgr.AddMoney(emotMoney, -pData->nCostMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    ++m_nSafeBoxLevel;

    g_PlayerServer.DoSyncSafeBoxLevel(m_pPlayer->m_nConnIndex, m_nSafeBoxLevel);

    m_pPlayer->OnEvent(peSafeBoxLevelup, m_nSafeBoxLevel);

    bRetCode = m_pPlayer->m_UpgradeQueue.AddUpgradeNode(eutUpgradeSafeBox, pData->nTime, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KMoneyMgr::RefreshGameMoneyLimit()
{
    int nMoneyLimit = 0;

    nMoneyLimit = GetGameMoneyLimit();
    KGLOG_PROCESS_ERROR(nMoneyLimit > 0);

    if (m_nMoneys[emotMoney] > nMoneyLimit)
    {
        m_nMoneys[emotMoney] = nMoneyLimit;
        g_PlayerServer.DoSyncMoneyChanged(m_pPlayer->m_nConnIndex, emotMoney, m_nMoneys[emotMoney]);
    }

Exit0:
    return;
}

int KMoneyMgr::GetGameMoneyLimit()
{
    int nResult = 0;
    KSafeBox* pData = NULL;
    KUPGRADE_TYPE eUpgradeType = eutInvalid;

    pData = g_pSO3World->m_Settings.m_SafeBox.GetByID(m_nSafeBoxLevel);
    KGLOG_PROCESS_ERROR(pData);

    nResult = pData->nMoneyLimit;
Exit0:
    return nResult;
}

BOOL KMoneyMgr::AddFreeCoin(int nAddFreeCoin)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int  nNewCoin = 0; 

    m_nFreeCoin += nAddFreeCoin;

    nNewCoin = GetMoney(emotCoin);
    g_PlayerServer.DoSyncMoneyChanged(m_pPlayer->m_nConnIndex, emotCoin, nNewCoin);

	bResult = true;
Exit0:
	return bResult;
}
