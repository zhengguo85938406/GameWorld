#include "stdafx.h"
#include "KShopMgr.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KGItem.h"
#include "KSO3World.h"
#include "KRelayClient.h"
#include "KTools.h"
#include "KLSClient.h"
#include "KLogClient.h"
#include "KSystemMailMgr.h"

#define GOODS_LIST_NAME "item/GoodsList.tab"
#define SHOP_LIST_NAME  "item/ShopList.tab"

KShopMgr::KShopMgr()
{
}

KShopMgr::~KShopMgr()
{
}

BOOL KShopMgr::Init()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = LoadGoodsList();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = LoadShopList();
    KGLOG_PROCESS_ERROR(bRetCode);
    
    //////////////////////////////////////////////////////////////////////////
    memset(m_ProcessCustomConsumeFunc, 0, sizeof(m_ProcessCustomConsumeFunc));

    REGISTER_CUSTOM_CONSUME_FUNC(uctBusinessStreetBuyLand, &KShopMgr::BuyLandResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBusinessStreetBuildStore, &KShopMgr::BuildStoreResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBusinessStreetUpgradeStore, &KShopMgr::UpgradeStoreResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBusinessStreetClearGetMoneyCD, &KShopMgr::ClearGetMoneyCDResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBusinessStreetClearAllGetMoneyCD, &KShopMgr::ClearAllGetMoneyCDResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBusinessStreetChangeStoreType, &KShopMgr::ChangeStoreTypeRespond);
    REGISTER_CUSTOM_CONSUME_FUNC(uctDirectEndUseEquip, &KShopMgr::DirectEndUseEquipResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctClearCD, &KShopMgr::ClearCDRespond);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBuyFatiguePoint, &KShopMgr::BuyFatigueSuccess);
    REGISTER_CUSTOM_CONSUME_FUNC(uctChargeVIP, &KShopMgr::ChargeVIPResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctPresentVIP, &KShopMgr::PresentVIPResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBuyFashion, &KShopMgr::BuyFashionResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctChargeFashion, &KShopMgr::ChargeFashionResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBuyCheerleadingSlot, &KShopMgr::BuyCheerleadingSlotResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctRechargeCheerleadingItem, &KShopMgr::RechargeCheerleadingItemResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctBuyHeroSlot, &KShopMgr::BuyHeroSlotResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctPresentItem, &KShopMgr::PresentItemResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctExtendPlayerPackage, &KShopMgr::ExtendPlayerPackageResult);
    REGISTER_CUSTOM_CONSUME_FUNC(uctScriptConsume, &KShopMgr::ScriptConsumeResult);
	REGISTER_CUSTOM_CONSUME_FUNC(uctBuyHero, &KShopMgr::BuyHeroResult);

    bResult = true;
Exit0:
    return bResult;
}

void KShopMgr::UnInit()
{
}

BOOL KShopMgr::OnPlayerBuyGoods(KPlayer* pPlayer, int nShopID, int nGoodsID, int nCount)
{
    BOOL                                bResult             = false;
    BOOL                                bRetCode            = false;
    int                                 nTotalPrice         = 0;
    uint32_t                            dwDiscountRate      = HUNDRED_NUM;
    GOODSINFO*                          pGoodsInfo          = NULL;
    IItem*                              pItem               = NULL;    
    KFASHION_INFO*                      pFashion            = NULL;
    KGCheerleadingInfo*                 pCheerleadingInfo   = NULL;
    KTEAM_LOGO_INFO*                    pTeamLogoInfo       = NULL;
    DWORD                               dwRandSeed          = KG_GetTickCount();
    int                                 nPayCoin            = 0;
    
    KGLOG_PROCESS_ERROR(pPlayer);

    if (pPlayer->m_nIBActionTime + IB_ACTION_TIMEOUT_SEC >= g_pSO3World->m_nCurrentTime)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_IB_ACTION_BUSY);
        goto Exit0;
    }

    pGoodsInfo = GetValidGoodsInfo(nShopID, nGoodsID);
    KGLOG_PROCESS_ERROR(pGoodsInfo);    

    nTotalPrice  = GetBuyGoodsTotalPrice(pGoodsInfo, nCount);
    KGLOG_PROCESS_ERROR(nTotalPrice > 0);

    bRetCode = pPlayer->m_MoneyMgr.CanAddMoney(pGoodsInfo->eMoneyType, -nTotalPrice);
    KGLOG_PROCESS_ERROR(bRetCode);

    switch (pGoodsInfo->eTabType)
    {
    case ittEquip:
    case ittOther:
        bRetCode = pPlayer->CanAddItem(pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, nCount); 
        KGLOG_PROCESS_ERROR(bRetCode);
        break;
    case ittFashion:
        pFashion = g_pSO3World->m_ItemHouse.GetFashionInfo(pGoodsInfo->nTabIndex);
        KGLOG_PROCESS_ERROR(pFashion);
        break;
    case ittCheerleading:
        pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo((DWORD)pGoodsInfo->nTabIndex);
        KGLOG_PROCESS_ERROR(pCheerleadingInfo);
        break;
    case ittTeamLogo:
        pTeamLogoInfo   = g_pSO3World->m_ItemHouse.GetTeamLogoInfo((DWORD)pGoodsInfo->nTabIndex);
        KGLOG_PROCESS_ERROR(pTeamLogoInfo);
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    if (pGoodsInfo->eMoneyType == emotCoin)
    {
        nPayCoin = pPlayer->m_MoneyMgr.GetPayCoin();
        if (nPayCoin >= nTotalPrice) // 先消耗充值点卷
        {
            bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotCoin, -nTotalPrice);
            KGLOG_PROCESS_ERROR(bRetCode);

            PLAYER_LOG(pPlayer, "buygoods, usecoinrequest, %d, %d", nTotalPrice, nGoodsID);

            g_RelayClient.DoCoinShopBuyItemRequest(
                pPlayer, pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, pGoodsInfo->nID, nCount, nTotalPrice, NULL
            );
            goto Exit1;
        }
        else if (nPayCoin > 0) //有一部分充值点卷
        {
            int nCostFreeCoin = nTotalPrice - nPayCoin;
            bRetCode = pPlayer->m_MoneyMgr.AddFreeCoin(-nCostFreeCoin);
            KGLOG_PROCESS_ERROR(bRetCode);

            PLAYER_LOG(pPlayer, "buygoods, usefreecoin, %d, %d", nCostFreeCoin, nGoodsID);

            nTotalPrice -= nCostFreeCoin;

            bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotCoin, -nTotalPrice);
            KGLOG_PROCESS_ERROR(bRetCode);

            PLAYER_LOG(pPlayer, "buygoods, usecoinrequest, %d, %d", nTotalPrice, nGoodsID);

            g_RelayClient.DoCoinShopBuyItemRequest(
                pPlayer, pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, pGoodsInfo->nID, nCount, nTotalPrice, NULL
            );
            goto Exit1;
        }
        else // 完全没有充值点卷
        {
            R2S_COIN_SHOP_BUY_ITEM_RESPOND respond;

            bRetCode = pPlayer->m_MoneyMgr.AddFreeCoin(-nTotalPrice);
            KGLOG_PROCESS_ERROR(bRetCode);

            memset(&respond, 0, sizeof(respond));

            respond.bSucceed = true;
            respond.dwPlayerID = pPlayer->m_dwID;
            respond.dwTabType = pGoodsInfo->eTabType;
            respond.dwTabIndex = pGoodsInfo->nTabIndex;
            respond.nGoodsID = pGoodsInfo->nID;
            respond.nCount = nCount;
            respond.nCoinPrice = nTotalPrice;

            g_RelayClient.OnCoinShopBuyItemRespond((BYTE*)&respond, sizeof(respond));

            PLAYER_LOG(pPlayer, "buygoods, usefreecoin, %d, %d", nTotalPrice, nGoodsID);

            goto Exit1;
        }
    }
    
    bRetCode = pPlayer->m_MoneyMgr.AddMoney(pGoodsInfo->eMoneyType, -nTotalPrice);
    KGLOG_PROCESS_ERROR(bRetCode);

    switch (pGoodsInfo->eTabType)
    {
    case ittEquip:
    case ittOther:
        {
            KGItemInfo* pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(pGoodsInfo->eTabType, pGoodsInfo->nTabIndex);
            KGLOG_PROCESS_ERROR(pItemInfo);

            if (pItemInfo->bCanStack)
            {
                int nTotalCount = 0;
                int nAddTime = 0;
                int nLeftCount = 0;

                KGLOG_PROCESS_ERROR(pItemInfo->nMaxStackNum > 0);
                
                nTotalCount = pGoodsInfo->nStack * nCount;
                nAddTime   = nTotalCount / pItemInfo->nMaxStackNum;

                for (int i = 0; i < nAddTime; ++i)
                {
                    pItem = pPlayer->AddItem(pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, pItemInfo->nMaxStackNum, pGoodsInfo->nValuePoint, dwRandSeed);
                    KGLOG_PROCESS_ERROR(pItem);
                    dwRandSeed *= (i + 2);
                }

                nLeftCount = nTotalCount - nAddTime * pItemInfo->nMaxStackNum;
                if (nLeftCount > 0)
                {
                    pItem = pPlayer->AddItem(
                        pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, nLeftCount, pGoodsInfo->nValuePoint, dwRandSeed
                    );
                    KGLOG_PROCESS_ERROR(pItem);
                }
            }
            else
            {
                for (int i = 0; i < nCount; ++i)
                {
                    pItem = pPlayer->AddItem(pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, pGoodsInfo->nStack, pGoodsInfo->nValuePoint, dwRandSeed);
                    KGLOG_PROCESS_ERROR(pItem);
                    dwRandSeed *= (i + 2);
                }
            }
		}
        break;
    case ittFashion:
        bRetCode = pPlayer->m_Wardrobe.AddFashion(pGoodsInfo->nTabIndex);
        KGLOG_PROCESS_ERROR(bRetCode);
        g_LogClient.DoFlowRecord(
            frmtCheerleadingAndWardrobe, cawfrBuyFashion, 
            "%s,%d,%d",
            pPlayer->m_szName,
            nTotalPrice,
            pGoodsInfo->nTabIndex
        );
        break;
    case ittCheerleading:
    	bRetCode = pPlayer->m_CheerleadingMgr.AddCheerleading(pCheerleadingInfo->dwID);
        g_LogClient.DoFlowRecord(
			frmtCheerleadingAndWardrobe, cawfrBuyCheerleading, 
            "%s,%d,%u",
            pPlayer->m_szName,
            nTotalPrice,
            pCheerleadingInfo->dwID
        );
        KGLOG_PROCESS_ERROR(bRetCode);
        break;
    case ittTeamLogo:
        bRetCode = pPlayer->AddTeamLogo((WORD)pTeamLogoInfo->nID);
        KGLOG_PROCESS_ERROR(bRetCode);
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

	pPlayer->OnEvent(peBuyItem, pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, pGoodsInfo->nID);

    PLAYER_LOG(pPlayer, "money,costmoney,%d,%s,%d-%d,%d,%d", pGoodsInfo->eMoneyType, "buyitem", pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, nCount, pGoodsInfo->nCost);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::OnPlayerSellGoods(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime, WORD wCount)
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    int     nPrice          = 0;
    int     nUpdateStack    = 0;
    IItem*  pItem           = NULL;

    assert(pPlayer);
	KGLOG_PROCESS_ERROR(wCount > 0);
	
    pItem = pPlayer->m_ItemList.GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(pItem);

    bRetCode = pItem->CanTrade();
    KGLOG_PROCESS_ERROR(bRetCode);
    
    bRetCode = CheckGoodsCoherence(pPlayer, nPackageType, nPackageIndex, nPos, dwTabType, dwTabIndex, nGenTime, wCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    nPrice = GetSellGoodsPrice(pPlayer, nPackageType, nPackageIndex, nPos, wCount);
    KGLOG_PROCESS_ERROR(nPrice >= 0);

    if (pItem->CanStack() && wCount < pItem->GetStackNum())
    {
        nUpdateStack = pItem->GetStackNum() - wCount;
        KGLOG_PROCESS_ERROR(nUpdateStack > 0);

        bRetCode = pItem->SetStackNum(nUpdateStack);
        KGLOG_PROCESS_ERROR(bRetCode);

        g_PlayerServer.DoUpdateItemAmount(pPlayer->m_nConnIndex, nPackageType, nPackageIndex, nPos, nUpdateStack);
    }
    else
    {
        bRetCode = pPlayer->m_ItemList.DestroyItem(nPackageType, nPackageIndex, nPos);
        KGLOG_PROCESS_ERROR(bRetCode);
    }   

    bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotMoney, nPrice);
    KGLOG_PROCESS_ERROR(bRetCode);

    PLAYER_LOG(pPlayer, "money,addmoney,%d,%s,%d-%d,%d,%d", emotMoney, "sellitem", dwTabType, dwTabIndex, 0, nPrice);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::OnPlayerRepairSingleEquip(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime)
{
    BOOL            bResult             = false;
    BOOL            bRetCode            = false;
    int             nRepairPrice        = ERROR_ID;
    int             nCurrentDurability  = ERROR_ID;
    int             nMaxDurability      = ERROR_ID;
    int             nQuality            = ERROR_ID;
    int             nValuePoint         = ERROR_ID;
    IItem*          piItem              = NULL;
    const KItemProperty*  pItemProperty = NULL;
    
    KGLOG_PROCESS_ERROR(pPlayer && pPlayer->m_eGameStatus == gsInHall);

    piItem = pPlayer->m_ItemList.GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(piItem && piItem->CanRepair());

    pItemProperty = piItem->GetProperty();
    KGLOG_PROCESS_ERROR(pItemProperty);
    KGLOG_PROCESS_ERROR(pItemProperty->dwTabType == dwTabType && pItemProperty->dwTabIndex == dwTabIndex);
    KGLOG_PROCESS_ERROR(pItemProperty->nGenTime == nGenTime);

    nCurrentDurability  = piItem->GetCurrentDurability();
    nMaxDurability      = pItemProperty->pItemInfo->nMaxDurability;
    nQuality            = piItem->GetQuality();
    nValuePoint         = pItemProperty->nValuePoint;

    nRepairPrice = GetRepairSinglePrice(nCurrentDurability, nMaxDurability, nQuality, nValuePoint);
    KGLOG_PROCESS_ERROR(nRepairPrice);

    bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotMoney, -nRepairPrice);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->RepairEquip(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::OnPlayerRepairAllEquip(KPlayer* pPlayer)
{
    BOOL    bResult             = false;
    BOOL    bRetCode            = false;
    int     nRepairTotalPrice   = ERROR_ID;

    KGLOG_PROCESS_ERROR(pPlayer);

    nRepairTotalPrice = GetRepairTotalPrice(pPlayer);
    KG_PROCESS_ERROR(nRepairTotalPrice);

    bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotMoney, -nRepairTotalPrice);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->m_ItemList.RepairAllHeroEquip();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KShopMgr::OnPlayerPresentGoods(KPlayer* pSender)
{
    BOOL        bResult	    = false;
    BOOL        bRetCode	= false;
    GOODSINFO*  pGoodsInfo  = NULL;
    int         nTotalPrice = 0;
    KCUSTOM_CONSUME_INFO    CCInfo = {0};
    IItem*      piItem      = NULL;
    KC2S_Present_Goods_Request* pRequest = (KC2S_Present_Goods_Request*)pSender->m_pbyPresentGoodInfo;

    KGLOG_PROCESS_ERROR(pSender && pSender->m_pbyPresentGoodInfo);

    KGLOG_PROCESS_ERROR(pRequest->nCount < HUNDRED_NUM);

    if (STR_CASE_CMP(pSender->m_szName, pRequest->szDstName) == 0)  // 不能赠送给自己
    {
        g_PlayerServer.DoSendMailRespond(pSender->m_nConnIndex, 0, mrcDstNotSelf);
        goto Exit0;
    }

    pGoodsInfo = GetValidGoodsInfo(pRequest->nShopID, pRequest->nGoodsID);
    KGLOG_PROCESS_ERROR(pGoodsInfo);
    KGLOG_PROCESS_ERROR(pGoodsInfo->eMoneyType == emotCoin);

    nTotalPrice = GetBuyGoodsTotalPrice(pGoodsInfo, pRequest->nCount);
    KGLOG_PROCESS_ERROR(nTotalPrice);

    bRetCode = DoCustomConsumeRequest(pSender, uctPresentItem, nTotalPrice);
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLogPrintf(
        KGLOG_INFO, "[PRESENT] player %s present goods(%d,%d) to player %s request paysys cost coin!", 
        pSender->m_szName, pRequest->nShopID, pRequest->nGoodsID, pRequest->szDstName
    );

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::DoCustomConsumeRequest(
    KPlayer* pPlayer, CUSTOM_CONSUME_TYPE eUndefinitionType, 
    int nTotalConsumeMoney, KCUSTOM_CONSUME_INFO* pCCInfo
)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nPayCoin    = 0;

    KGLOG_PROCESS_ERROR(pPlayer);
    if (pPlayer->m_nIBActionTime + IB_ACTION_TIMEOUT_SEC >= g_pSO3World->m_nCurrentTime)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_IB_ACTION_BUSY);
        goto Exit0;
    }

    KGLOG_PROCESS_ERROR(eUndefinitionType > uctInvalid && eUndefinitionType < uctTotal);
    KGLOG_PROCESS_ERROR(nTotalConsumeMoney > 0);

    bRetCode = pPlayer->m_MoneyMgr.CanAddMoney(emotCoin, -nTotalConsumeMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    nPayCoin = pPlayer->m_MoneyMgr.GetPayCoin();
    if (nPayCoin >= nTotalConsumeMoney) // 先消耗充值点卷
    {
        bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotCoin, -nTotalConsumeMoney);
        KGLOG_PROCESS_ERROR(bRetCode);

        PLAYER_LOG(pPlayer, "coin, usepaycoin, %d, %d", nTotalConsumeMoney, eUndefinitionType);

        g_RelayClient.DoCoinShopBuyItemRequest(pPlayer, 0, (int)eUndefinitionType, 0, 1, nTotalConsumeMoney, pCCInfo);
        goto Exit1;
    }
    else if (nPayCoin > 0) //有一部分充值点卷
    {
        int nCostFreeCoin = nTotalConsumeMoney - nPayCoin;
        bRetCode = pPlayer->m_MoneyMgr.AddFreeCoin(-nCostFreeCoin);
        KGLOG_PROCESS_ERROR(bRetCode);

        nTotalConsumeMoney -= nCostFreeCoin;

        PLAYER_LOG(pPlayer, "coin, usefreecoinandpaycoin, %d, %d, %d", nCostFreeCoin, nPayCoin, eUndefinitionType);

        bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotCoin, -nTotalConsumeMoney);
        KGLOG_PROCESS_ERROR(bRetCode);

        g_RelayClient.DoCoinShopBuyItemRequest(pPlayer, 0, (int)eUndefinitionType, 0, 1, nTotalConsumeMoney, pCCInfo);
        goto Exit1;
    }
    else // 完全没有充值点卷
    {
        bRetCode = pPlayer->m_MoneyMgr.AddFreeCoin(-nTotalConsumeMoney);
        KGLOG_PROCESS_ERROR(bRetCode);

        OnCustomConsumeRespond(pPlayer, true, eUndefinitionType, nTotalConsumeMoney, pCCInfo);

        g_PlayerServer.DoIBActionResultNotify(pPlayer, true);

        PLAYER_LOG(pPlayer, "coin, usefreecoin, %d, %d", nTotalConsumeMoney, eUndefinitionType);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::OnCustomConsumeRespond(
    KPlayer* pPlayer, BOOL bSuccess, CUSTOM_CONSUME_TYPE eUndefinitionType,
	int nTotalConsumeMoney, KCUSTOM_CONSUME_INFO* pCCInfo
)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    PROCESS_CUSTOM_CONSUME_FUNC pFunc       = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    PLAYER_LOG(pPlayer, "coin, usecoinrespond, %d, %d, %d", bSuccess, nTotalConsumeMoney, eUndefinitionType);

    if (!bSuccess)
    {
        bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotCoin, nTotalConsumeMoney);
        KGLOG_PROCESS_ERROR(bRetCode);

        KGLogPrintf(
            KGLOG_INFO, "Paysys respond Failed!:\nInfo --> player:%d try to act type:%d with:%d Coins nValue1 - 4:%d %d %d %d ", 
            pPlayer->m_dwID, eUndefinitionType, nTotalConsumeMoney, pCCInfo->nValue1, pCCInfo->nValue2, pCCInfo->nValue3, pCCInfo->nValue4
        );
		goto Exit1;
    }

    KGLOG_PROCESS_ERROR(eUndefinitionType > uctInvalid && eUndefinitionType < uctTotal);
    pFunc = m_ProcessCustomConsumeFunc[eUndefinitionType];
    KGLOG_PROCESS_ERROR(pFunc);

    bRetCode = (this->*pFunc)(pPlayer, pCCInfo, nTotalConsumeMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

int KShopMgr::GetGoodsStack(int nGoodsID)
{
    int nResult = 0;
    KGOODS_LIST_INFO_TABLE::iterator GoodsIter;

    GoodsIter = m_GoodsListMap.find(nGoodsID);
    KGLOG_PROCESS_ERROR(GoodsIter != m_GoodsListMap.end());

    nResult = GoodsIter->second.nStack;
Exit0:
    return nResult;
}

#define REPAIR_SCALE 0.005f
int KShopMgr::GetRepairSinglePrice(int nCurrentDurability, int nMaxDurability, int nQuality, int nValuePoint)
{
    int     nResult                 = ERROR_ID;
    int     nQualityScale[eqTotal]  = {1, 2, 3, 5, 8, 11, 15};   //品质系数
    int     nPerDurabilityPrice     = ERROR_ID;
    int     nEquipPrice             = ERROR_ID;

    KGLOG_PROCESS_ERROR(nQuality >= eqWhite && nQuality <= eqTotal);

    nPerDurabilityPrice = (int)(REPAIR_SCALE * nValuePoint * nQualityScale[nQuality]);
    nEquipPrice         = nPerDurabilityPrice * (nMaxDurability - nCurrentDurability);
    KGLOG_PROCESS_ERROR(nEquipPrice >= 0);

    nResult = nEquipPrice;
Exit0:
    return nResult;
}

GOODSINFO* KShopMgr::GetGoodsInfo(int nGoodsID)
{
    GOODSINFO* pGoodsInfo = NULL;
    KGOODS_LIST_INFO_TABLE::iterator it;

    it = m_GoodsListMap.find(nGoodsID);
    KGLOG_PROCESS_ERROR(it != m_GoodsListMap.end());

    pGoodsInfo = &it->second;
    KGLOG_PROCESS_ERROR(pGoodsInfo);

Exit0:
    return pGoodsInfo;
}

GOODSINFO* KShopMgr::GetGoodsInfo(DWORD dwTabType, DWORD dwTabIndex)
{
    KGOODS_LIST_INFO_TABLE::iterator it;
    for (it = m_GoodsListMap.begin(); it != m_GoodsListMap.end(); ++it)
    {
        if (it->second.eTabType == (int)dwTabType && it->second.nTabIndex == (int)dwTabIndex)
        {
            return &it->second;
        }
    }
    
    return NULL;
}

BOOL KShopMgr::LoadGoodsList()
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    int             nHeight         = 0;
    ITabFile*       piTabFile       = NULL;
    char            szFilePath[MAX_PATH]        = {0};
    char            szStringBuffer[_NAME_LEN]   = {0};
    GOODSINFO       SingleGoodsInfo;
    std::pair<KGOODS_LIST_INFO_TABLE::iterator, bool> InsRet;

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, GOODS_LIST_NAME);
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();
    for (int nRowIndex = 2; nRowIndex <= nHeight; nRowIndex++)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, &SingleGoodsInfo.nID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(SingleGoodsInfo.nID > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TabType", 0, (int*)&SingleGoodsInfo.eTabType);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(SingleGoodsInfo.eTabType > ittInvalid && SingleGoodsInfo.eTabType < ittTotal);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TabIndex", 0, &SingleGoodsInfo.nTabIndex);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(SingleGoodsInfo.nTabIndex > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Stack", 0, &SingleGoodsInfo.nStack);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(SingleGoodsInfo.nStack > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "ValuePoint", 0, &SingleGoodsInfo.nValuePoint);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MoneyType", 0, (int*)&SingleGoodsInfo.eMoneyType);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(SingleGoodsInfo.eMoneyType >= emotMoney && SingleGoodsInfo.eMoneyType < emotTotal);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Cost", 0, &SingleGoodsInfo.nCost);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(SingleGoodsInfo.nCost >= 0);

        bRetCode = piTabFile->GetString(nRowIndex, "SaleStartTime", "", szStringBuffer, sizeof(szStringBuffer));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = g_GetTimeFromString(szStringBuffer, strlen(szStringBuffer), SingleGoodsInfo.dwSaleStartTime);
        KGLOG_PROCESS_ERROR(bRetCode);
        memset(szStringBuffer, 0, sizeof(szStringBuffer));

        bRetCode = piTabFile->GetString(nRowIndex, "SaleOverTime", "", szStringBuffer, sizeof(szStringBuffer));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = g_GetTimeFromString(szStringBuffer, strlen(szStringBuffer), SingleGoodsInfo.dwSaleOverTime);
        KGLOG_PROCESS_ERROR(bRetCode);
        memset(szStringBuffer, 0, sizeof(szStringBuffer));

        bRetCode = piTabFile->GetString(nRowIndex, "DiscountStartTime", "", szStringBuffer, sizeof(szStringBuffer));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = g_GetTimeFromString(szStringBuffer, strlen(szStringBuffer), SingleGoodsInfo.dwDiscountStartTime);
        KGLOG_PROCESS_ERROR(bRetCode);
        memset(szStringBuffer, 0, sizeof(szStringBuffer));

        bRetCode = piTabFile->GetString(nRowIndex, "DiscountOverTime", "", szStringBuffer, sizeof(szStringBuffer));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = g_GetTimeFromString(szStringBuffer, strlen(szStringBuffer), SingleGoodsInfo.dwDiscountOverTime);
        KGLOG_PROCESS_ERROR(bRetCode);
        memset(szStringBuffer, 0, sizeof(szStringBuffer));

        bRetCode = piTabFile->GetInteger(nRowIndex, "DiscountRate", 100, (int*)&SingleGoodsInfo.dwDiscountRate);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(SingleGoodsInfo.dwDiscountRate > 0 && SingleGoodsInfo.dwDiscountRate <= 100);

        bRetCode = piTabFile->GetInteger(nRowIndex, "UseType", 0, &SingleGoodsInfo.nUseType);
        KGLOG_PROCESS_ERROR(bRetCode);

        InsRet = m_GoodsListMap.insert(make_pair(SingleGoodsInfo.nID, SingleGoodsInfo));
        KGLOG_PROCESS_ERROR(InsRet.second);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KShopMgr::LoadShopList()
{
    BOOL            bResult                 = false;
    BOOL            bRetCode                = false;
    int             nHeight                 = 0;
    int             nGoodsIDNum             = 0;
    ITabFile*       piTabFile               = NULL;
    char            szFilePath[MAX_PATH]    = {0};
    KSHOP_INFO      SingleShopInfo;
    std::pair<KSHOP_LIST_TABLE::iterator, bool> ShopInsRet;
    KVEC_GOODS_ID::iterator vecIt;

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, SHOP_LIST_NAME);
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight     = piTabFile->GetHeight();
    nGoodsIDNum = piTabFile->GetWidth() - 2;
    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, &SingleShopInfo.nID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(SingleShopInfo.nID > 0);

        bRetCode = piTabFile->GetString(nRowIndex, "ShopName", "", SingleShopInfo.szTIP, _NAME_LEN);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        SingleShopInfo.vecGoodsID.clear();
        for (int nWidth = 1; nWidth <= nGoodsIDNum; nWidth++)
        {
            int     nGoodsID                = 0;
            char    szGoodsID[_NAME_LEN]    = {0};

            bRetCode = snprintf(szGoodsID, sizeof(szGoodsID), "GoodsID%d", nWidth);
            KGLOG_PROCESS_ERROR(bRetCode < sizeof(szGoodsID));
            szGoodsID[sizeof(szGoodsID) -1] = '\0';

            bRetCode = piTabFile->GetInteger(nRowIndex, szGoodsID, 0, &nGoodsID);
            KGLOG_PROCESS_ERROR(bRetCode != 0);
            if (nGoodsID <= 0)
                break;

            vecIt = std::find(SingleShopInfo.vecGoodsID.begin(), SingleShopInfo.vecGoodsID.end(), nGoodsID);
            KGLOG_PROCESS_ERROR(vecIt == SingleShopInfo.vecGoodsID.end());

            SingleShopInfo.vecGoodsID.push_back(nGoodsID);
        }

        ShopInsRet = m_ShopListMap.insert(make_pair(SingleShopInfo.nID, SingleShopInfo));
        KGLOG_PROCESS_ERROR(ShopInsRet.second);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

GOODSINFO* KShopMgr::GetValidGoodsInfo(int nShopID, int nGoodsID)
{
    GOODSINFO*  pResult   = NULL;
    BOOL        bRetCode  = false;
    KSHOP_LIST_TABLE::iterator ShopIter;
    GOODSINFO*  pGoodsInfo= NULL;

    ShopIter = m_ShopListMap.find(nShopID);
    KGLOG_PROCESS_ERROR(ShopIter != m_ShopListMap.end());

    bRetCode = IsGoodsExist(nShopID, nGoodsID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = IsGoodsOnSale(nGoodsID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pGoodsInfo = GetGoodsInfo(nGoodsID);
    KGLOG_PROCESS_ERROR(pGoodsInfo);

    pResult = pGoodsInfo;
Exit0:
    return pResult;
}

int KShopMgr::GetBuyGoodsTotalPrice(GOODSINFO* pGoodsInfo, int nCount)
{
    int         nResult	    = 0;
    BOOL        bRetCode    = false;
    time_t      tNow        = 0;
    int         nTotalPrice = 0;
    uint32_t    dwDiscountRate  = HUNDRED_NUM;
	
	assert(pGoodsInfo);

    tNow = g_pSO3World->m_nCurrentTime;

    if ((uint32_t)tNow >= pGoodsInfo->dwDiscountStartTime && (uint32_t)tNow <= pGoodsInfo->dwDiscountOverTime)
    {
        dwDiscountRate = pGoodsInfo->dwDiscountRate;
    }

    nTotalPrice  = nCount * pGoodsInfo->nCost * dwDiscountRate / HUNDRED_NUM;
    KGLOG_PROCESS_ERROR(nTotalPrice > 0);

    nResult	= nTotalPrice;
Exit0:
    return nResult;
}

int KShopMgr::GetSellGoodsPrice(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, WORD wCount)
{
    int                     nPrice          = 0;
    IItem*                  pItem           = NULL;
    const KItemProperty*    pItemProperty   = NULL;
    double                  fQualityParam[eqTotal] = {0.2, 0.4, 0.6, 1.2, 2, 3, 4.5};

    pItem = pPlayer->m_ItemList.GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(pItem);

    pItemProperty = pItem->GetProperty();
    KGLOG_PROCESS_ERROR(pItemProperty);

    nPrice = pItemProperty->pItemInfo->nPrice;
    if (pItem->CanStack())
    {   
        KGLOG_PROCESS_ERROR(pItemProperty->nStackNum >= (int)wCount);
        nPrice = nPrice * wCount;
        KGLOG_PROCESS_ERROR(nPrice >= 0);
    }
    
    if (pItemProperty->dwTabType == ittEquip)
    {
        KGLOG_PROCESS_ERROR(pItemProperty->nQuality >= eqWhite && pItemProperty->nQuality < eqTotal);
        nPrice = (int)(pItemProperty->nValuePoint * fQualityParam[pItemProperty->nQuality]);
    }

    KGLOG_PROCESS_ERROR(nPrice >= 0);
    
Exit0:
    return nPrice;
}

int KShopMgr::GetRepairTotalPrice(KPlayer* pPlayer)
{
    int nTotalPrice = ERROR_ID;

    assert(pPlayer);

    nTotalPrice = pPlayer->m_ItemList.GetRepairTotalPrice();

Exit0:
    return nTotalPrice;
}

BOOL KShopMgr::CheckGoodsCoherence(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime, WORD wCount)
{
    BOOL                    bResult         = false;
    IItem*                  pItem           = NULL;
    const KItemProperty*    pItemProperty   = NULL;

    pItem = pPlayer->m_ItemList.GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(pItem);

    pItemProperty = pItem->GetProperty();
    KGLOG_PROCESS_ERROR(pItemProperty);

    KGLOG_PROCESS_ERROR(pItemProperty->nStackNum >= (int)wCount);
    KGLOG_PROCESS_ERROR(pItemProperty->dwTabType == dwTabType);
    KGLOG_PROCESS_ERROR(pItemProperty->dwTabIndex == dwTabIndex);
    KGLOG_PROCESS_ERROR(pItemProperty->nGenTime == nGenTime);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::IsGoodsExist(int nShopID, int nGoodsID)
{
    BOOL bResult = false;
    KSHOP_LIST_TABLE::iterator shopIt;    
    KGOODS_LIST_INFO_TABLE::iterator goodsIt;

    shopIt = m_ShopListMap.find(nShopID);
    KG_PROCESS_ERROR(shopIt != m_ShopListMap.end());

    goodsIt = m_GoodsListMap.find(nGoodsID);
    KG_PROCESS_ERROR(goodsIt != m_GoodsListMap.end());    

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::IsGoodsOnSale(int nGoodsID)
{
    BOOL        bResult     = false;
    time_t      tNow        = 0;
    GOODSINFO*  pGoodsInfo  = NULL;
    KGOODS_LIST_INFO_TABLE::iterator it;

    pGoodsInfo = GetGoodsInfo(nGoodsID);
    KGLOG_PROCESS_ERROR(pGoodsInfo);

    tNow = g_pSO3World->m_nCurrentTime;

    KG_PROCESS_ERROR((uint32_t)tNow >= pGoodsInfo->dwSaleStartTime);
    KG_PROCESS_ERROR((uint32_t)tNow <= pGoodsInfo->dwSaleOverTime);

    bResult = true;
Exit0:
    return bResult;
}

//////////////////////////////////////////////////////////////////////////

BOOL KShopMgr::BuyLandResult(KPlayer* pPlayer,   KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->m_BusinessStreet.BuyLandResult(pCCInfo->nValue1);
    KGLOG_PROCESS_ERROR(bRetCode); 

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::BuildStoreResult(KPlayer* pPlayer,   KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    assert(pCCInfo);

    bRetCode = pPlayer->m_BusinessStreet.BuildStoreResult(pCCInfo->nValue1, (KSTORE_TYPE)pCCInfo->nValue2);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::UpgradeStoreResult(KPlayer* pPlayer,   KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->m_BusinessStreet.UpgradeStoreResult(pCCInfo->nValue1);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::ClearGetMoneyCDResult(KPlayer* pPlayer,   KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->m_BusinessStreet.ClearGetMoneyCDResult((uint32_t)pCCInfo->nValue1);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::ClearAllGetMoneyCDResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->m_BusinessStreet.ClearAllGetMoneyCDResult();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::ChangeStoreTypeRespond(KPlayer* pPlayer,   KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->m_BusinessStreet.ChangeStoreTypeRespond((uint32_t)pCCInfo->nValue1, (KSTORE_TYPE)pCCInfo->nValue2);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::DirectEndUseEquipResult(KPlayer* pPlayer,   KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->m_Gym.DirectEndUseEquipResult((uint32_t)pCCInfo->nValue1, nCostCoin);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::ClearCDRespond(KPlayer* pPlayer,   KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->m_UpgradeQueue.ClearCDRespond((KUPGRADE_TYPE)pCCInfo->nValue1, pCCInfo->nValue2, nCostCoin);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::BuyFatigueSuccess(KPlayer* pPlayer,   KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->OnBuyFatigueSuccess();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::ChargeVIPResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

    bRetCode = pPlayer->ChargeVIPTimeResult(pCCInfo->nValue1, NULL);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KShopMgr::PresentVIPResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = pPlayer->PresentVIPTimeToFriendResult(pCCInfo->nValue1, pCCInfo->nValue2);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::BuyFashionResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

//     暂时关闭使用自定义付费购买外装
//     bRetCode = pPlayer->m_Wardrobe.OnBuyFashionSuccess(pCCInfo->nValue1);
//     KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::ChargeFashionResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = pPlayer->m_Wardrobe.OnChargeFashionSuccess(pCCInfo->nValue1);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::BuyCheerleadingSlotResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = pPlayer->m_CheerleadingMgr.OnBuySlotSuccess(pCCInfo->nValue1);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KShopMgr::RechargeCheerleadingItemResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
    bRetCode = pPlayer->m_CheerleadingMgr.OnRechargeCheerleadingItemSuccess((DWORD)pCCInfo->nValue1, pCCInfo->nValue2);
	KGLOG_PROCESS_ERROR(bRetCode);
	
	bResult = true;
Exit0:
	return bResult;
}

BOOL KShopMgr::BuyHeroSlotResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
     
    KG_PROCESS_ERROR(pPlayer->m_HeroDataList.GetMaxHeroCount() < g_pSO3World->m_Settings.m_ConstList.nMaxHeroSlot); 

    bRetCode = pPlayer->m_HeroDataList.AddMaxHeroCount(1);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KShopMgr::BuyHeroResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KHeroTemplateInfo* pHeroTemplate = NULL;
	KHeroData* pHero = NULL;
	DWORD dwTemplateID = pCCInfo->nValue1;

	bRetCode = pPlayer->DoBuyHero(dwTemplateID);
	KGLOG_PROCESS_ERROR(bRetCode);
	
	bResult = true;
Exit0:
	return bResult;
}

BOOL KShopMgr::PresentItemResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL                    bResult	                = false;
    BOOL                    bRetCode	            = false;
    BOOL                    bItemCanStack           = false;
    IItem*                  piItem                  = NULL;
    KMail*                  pMail                   = NULL;
    uint32_t                uMailLen                = 0;
    GOODSINFO*              pGoodsInfo              = NULL;

    IItem*                  pItem[KMAIL_MAX_ITEM_COUNT]        = {0};
    KC2S_Present_Goods_Request* pRequest = (KC2S_Present_Goods_Request*)pPlayer->m_pbyPresentGoodInfo;

    KGLOG_PROCESS_ERROR(pPlayer && pPlayer->m_pbyPresentGoodInfo);

    pGoodsInfo = GetValidGoodsInfo(pRequest->nShopID, pRequest->nGoodsID);
    KGLOG_PROCESS_ERROR(pGoodsInfo);    

    if (pGoodsInfo->eTabType == ittEquip || pGoodsInfo->eTabType == ittOther)
    {
        piItem = g_pSO3World->m_ItemHouse.CreateItem(pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, 0, 0, pGoodsInfo->nValuePoint);
        KGLOG_PROCESS_ERROR(piItem);
        
        bItemCanStack = piItem->CanStack();
        if (bItemCanStack)
        {
            KGLOG_PROCESS_ERROR(pGoodsInfo->nStack <= piItem->GetMaxStackNum());
            piItem->SetStackNum(pGoodsInfo->nStack);
        }
        pItem[0] = piItem;
        piItem = NULL;

        pMail    = (KMail*)m_byTempData;
        bRetCode = g_pSO3World->FillMail(
            eMailType_Shop, pPlayer->m_szName, pRequest->szTitle, 
            pRequest->text, pRequest->textLength, pItem, 1, pMail, uMailLen
        );
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    
    g_LSClient.DoSendMailRequest(0, pRequest->szDstName, pMail, uMailLen);

    g_PlayerServer.DoRemoteCall(pPlayer->m_nConnIndex, "onPresentItemResult", 1);

    KGLogPrintf(
        KGLOG_INFO, "[PRESENT] player %s present goods(%d,%d) to player %s send mail success!", 
        pPlayer->m_szName, pRequest->nShopID, pRequest->nGoodsID, pRequest->szDstName
    );

    bResult	= true;
Exit0:
    if (pPlayer)
        KMEMORY_FREE(pPlayer->m_pbyPresentGoodInfo);

    for (size_t i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
    {
        if (pItem[i])
        {
            g_pSO3World->m_ItemHouse.DestroyItem(pItem[i]);
            pItem[i] = NULL;
        }
    }
    return bResult;
}

BOOL KShopMgr::ExtendPlayerPackageResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
    BOOL        bResult	        = false;
    BOOL        bRetCode	    = false;
    KPackage*   pPackage        = NULL;
    int         nPackageSize    = 0;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pCCInfo);

    pPackage = pPlayer->m_ItemList.GetPlayerPackage(pCCInfo->nValue1);
    KGLOG_PROCESS_ERROR(pPackage);

    nPackageSize = pPackage->GetSize();
    KGLOG_PROCESS_ERROR(nPackageSize >= 0);

    nPackageSize += cdPerExtendPlayerPackageSize;
    KGLOG_PROCESS_ERROR(nPackageSize <= pPackage->GetMaxSize());

    bRetCode = pPackage->SetSize(nPackageSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->OnEvent(peExtendBag, nPackageSize, pCCInfo->nValue1 + 1);

    g_PlayerServer.DoSyncPlayerPackageSize(pPlayer->m_nConnIndex, pCCInfo->nValue1, nPackageSize);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KShopMgr::ScriptConsumeResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int nCallBackID = ERROR_ID;
    int nTopIndex = 0;
    static const DWORD dwScriptID = g_FileNameHash("scripts/global/CallBack.lua");
    static const char* pszFuncName = "DoCallBack";

    KGLOG_PROCESS_ERROR(pCCInfo);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    nCallBackID = pCCInfo->nValue1;

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(nCallBackID);
    g_pSO3World->m_ScriptManager.Call(dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

	bResult = true;
Exit0:
	return bResult;
}
