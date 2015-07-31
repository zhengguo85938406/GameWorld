///////////////////////////////////////////////////////////////////////////////
//  Copyright(c) Kingsoft
//  FileName	: KShopMgr.h 
//  Creator 	: SunXun  
//  Date		: 04/11/2012
//  Comment		: 
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Ksdef/TypeDef.h"
#include "GlobalEnum.h"
#include <map>
#include "Relay_GS_Protocol.h"

#define REGISTER_CUSTOM_CONSUME_FUNC(eCustomConsumeType, FuncName)	    \
{m_ProcessCustomConsumeFunc[eCustomConsumeType] = FuncName;}            \

struct GOODSINFO 
{
    int             nID;
    KItemTableType  eTabType;
    int             nTabIndex;
    int             nStack;
    int             nValuePoint;    
    ENUM_MONEY_TYPE eMoneyType;
    int             nCost;
    uint32_t        dwSaleStartTime;                    //商品上架时间
    uint32_t        dwSaleOverTime;                     //商品下架时间
    uint32_t        dwDiscountStartTime;                //商品打折开始时间
    uint32_t        dwDiscountOverTime;                 //商品打折结束时间
    uint32_t        dwDiscountRate;                     //商品折扣率         100为底
    int             nUseType;                           //商品使用类型：用来与Paysys交互
};

typedef std::map<int, GOODSINFO> KGOODS_LIST_INFO_TABLE;

typedef std::vector<int> KVEC_GOODS_ID;
struct KSHOP_INFO
{
    int             nID;
    char            szTIP[_NAME_LEN];
    KVEC_GOODS_ID   vecGoodsID;
};

typedef std::map<int, KSHOP_INFO> KSHOP_LIST_TABLE;

class KPlayer;
class KShopMgr
{
public:
    KShopMgr();
    ~KShopMgr();

    BOOL Init();
    void UnInit();
    
    BOOL OnPlayerBuyGoods(KPlayer* pPlayer, int nShopID, int nGoodsID, int nCount);
    BOOL OnPlayerSellGoods(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime, WORD wCount);
    BOOL OnPlayerRepairSingleEquip(KPlayer* pPlayer,int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime);
    BOOL OnPlayerRepairAllEquip(KPlayer* pPlayer);
    BOOL OnPlayerPresentGoods(KPlayer* pSender);

    BOOL DoCustomConsumeRequest(
        KPlayer* pPlayer, CUSTOM_CONSUME_TYPE eUndefinitionType, 
        int nTotalConsumeMoney, KCUSTOM_CONSUME_INFO* pCCInfo = NULL
    );
    BOOL OnCustomConsumeRespond(KPlayer* pPlayer, BOOL bSuccess, CUSTOM_CONSUME_TYPE eUndefinitionType, int nTotalConsumeMoney, KCUSTOM_CONSUME_INFO* pCCInfo);

    int  GetGoodsStack(int nGoodsID);
    int  GetRepairSinglePrice(int nCurrentDurability, int nMaxDurability, int nQuality, int nValuePoint);
    GOODSINFO* GetGoodsInfo(int nGoodsID);
    GOODSINFO* GetGoodsInfo(DWORD dwTabType, DWORD dwTabIndex);

private:
    BOOL LoadGoodsList();
    BOOL LoadShopList();
    BOOL RegisterCustomConsumeFunc();

    GOODSINFO* GetValidGoodsInfo(int nShopID, int nGoodsID);

    // Get Goods Price
    int  GetBuyGoodsTotalPrice(GOODSINFO* pGoodsInfo, int nCount);
    int  GetSellGoodsPrice(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, WORD wCount);
    int  GetRepairTotalPrice(KPlayer* pPlayer);

    // Verify Goods Info
    BOOL CheckGoodsCoherence(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime, WORD wCount);
    BOOL IsGoodsExist(int nShopID, int nGoodsID);
    BOOL IsGoodsOnSale(int nGoodsID);

    KGOODS_LIST_INFO_TABLE   m_GoodsListMap;
    KSHOP_LIST_TABLE         m_ShopListMap;

    BYTE m_byTempData[MAX_EXTERNAL_PACKAGE_SIZE];

private:
    typedef BOOL (KShopMgr::*PROCESS_CUSTOM_CONSUME_FUNC)(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    PROCESS_CUSTOM_CONSUME_FUNC m_ProcessCustomConsumeFunc[uctTotal];

    BOOL BuyLandResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL BuildStoreResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL UpgradeStoreResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL ClearGetMoneyCDResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL ClearAllGetMoneyCDResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);

    BOOL ChangeStoreTypeRespond(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL DirectEndUseEquipResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL BuyTrainingSeatResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL DirectEndTrainingHeroResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL ClearCDRespond(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL BuyFatigueSuccess(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL ChargeVIPResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL PresentVIPResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL BuyFashionResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL ChargeFashionResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL BuyCheerleadingSlotResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL RechargeCheerleadingItemResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL BuyHeroSlotResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL PresentItemResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL ExtendPlayerPackageResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
    BOOL ScriptConsumeResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
	BOOL BuyHeroResult(KPlayer* pPlayer, KCUSTOM_CONSUME_INFO* pCCInfo, int nCostCoin);
};
