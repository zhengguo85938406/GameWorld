#include "stdafx.h"
#include "KItemList.h"
#include "KGItemDef.h"
#include "KPlayer.h"
#include "IItem.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KGItemInfo.h"
#include "KLSClient.h"

KItemList::KItemList()
{
    m_pPlayer = NULL;
}

KItemList::~KItemList()
{
}

BOOL KItemList::Init(KPlayer* pPlayer)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    
    assert(pPlayer);

    m_pPlayer = pPlayer;

    bRetCode = m_PlayerPackage.Init(pPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KItemList::UnInit()
{
    m_PlayerPackage.UnInit();

    for (KMAP_HERO_PACKAGE::iterator it = m_mapHeroPackage.begin(); it != m_mapHeroPackage.end(); ++it)
    {
        it->second.UnInit();
    }
    m_mapHeroPackage.clear();
}

BOOL KItemList::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    KMAP_HERO_PACKAGE::iterator it;

    assert(pLoadBuf);

    const T3DB::KPBItemList& Itemlist = pLoadBuf->itemlist();
    const T3DB::KPBPlayerPackage& PlayerPackage = Itemlist.playerpackage();

    bRetCode = m_PlayerPackage.LoadFromProtoBuf(&PlayerPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    for (int i = 0; i < Itemlist.heropackage_size(); ++i)
    {
        const T3DB::KPBHeroPackage& HeroPackage = Itemlist.heropackage(i);

        // 背包需要先创建，依赖于英雄数据提前加载时创建背包
        it = m_mapHeroPackage.find(HeroPackage.herotemplateid());
        KGLOG_PROCESS_ERROR(it != m_mapHeroPackage.end());

        bRetCode = it->second.LoadFromProtoBuf(&HeroPackage);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                    bResult	        = false;
    BOOL                    bRetCode        = false;
    T3DB::KPBItemList*        pItemlist       = NULL;
    T3DB::KPBPlayerPackage*   pPlayerPackage  = NULL;
    KMAP_HERO_PACKAGE::iterator it;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pItemlist = pSaveBuf->mutable_itemlist();
    pPlayerPackage = pItemlist->mutable_playerpackage();

    bRetCode = m_PlayerPackage.SaveToProtoBuf(pPlayerPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    for (it = m_mapHeroPackage.begin(); it != m_mapHeroPackage.end(); ++it)
    {
        T3DB::KPBHeroPackage* pHeroPackage = pItemlist->add_heropackage();

        pHeroPackage->set_herotemplateid(it->first);
        bRetCode = it->second.SaveToProtoBuf(pHeroPackage);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::CanAddPlayerItems(IItem** piItems, const int nItemNum)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = m_PlayerPackage.CanAddItems(piItems, nItemNum);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::CanAddPlayerItem(IItem* piItem)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = m_PlayerPackage.CanAddItem(piItem);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::AddPlayerItem(IItem* piItem)
{
	BOOL bResult    = false;
    BOOL bRetCode   = false;
    
    bRetCode = m_PlayerPackage.AddItem(piItem);
    KGLOG_PROCESS_ERROR(bRetCode);
        
    KGLOG_PROCESS_ERROR(m_pPlayer);
    g_PlayerServer.DoAddItemNotify(m_pPlayer->m_nConnIndex, piItem->GetType(), piItem->GetIndex(), piItem->GetStackNum());

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::AddPlayerItemToPos(IItem* piItem, int nPos)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    IItem*  pItem       = NULL;

    pItem = m_PlayerPackage.GetItem(eppiPlayerItemBox, nPos);
    KGLOG_PROCESS_ERROR(pItem == NULL);

    bRetCode = m_PlayerPackage.PlaceItem(piItem, eppiPlayerItemBox, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

IItem*  KItemList::PickUpPlayerItem(int nPackageIndex, int nPos)
{
    return m_PlayerPackage.PickUpItem(nPackageIndex, nPos);
}

IItem*  KItemList::GetPlayerItem(int nPackageIndex, int nPos)
{
    return m_PlayerPackage.GetItem(nPackageIndex, nPos);
}

BOOL KItemList::PlaceItemToHeroPackage(DWORD dwHeroTemplateID, IItem* piItem)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    int  nPos       = -1;
    KHeroPackage* pPackage = NULL;

    pPackage = GetHeroPackage(dwHeroTemplateID);
    KG_PROCESS_ERROR(pPackage);

    nPos = pPackage->GetItemPlacePos(piItem);
    KGLOG_PROCESS_ERROR(nPos >= 0);

    bRetCode = pPackage->PlaceItem(piItem, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

IItem* KItemList::PickUpHeroItem(DWORD dwTemplateID, int nPos)
{
    IItem* pItem = NULL;
    KHeroPackage* pPackage = NULL;

    pPackage = GetHeroPackage(dwTemplateID);
    KG_PROCESS_ERROR(pPackage);

    pItem = pPackage->PickUpItem(nPos);
Exit0:
    return pItem;
}

IItem* KItemList::GetHeroItem(DWORD dwHeroTemplateID, int nPos)
{
    IItem* pItem = NULL;
    KHeroPackage* pPackage = NULL;

    pPackage = GetHeroPackage(dwHeroTemplateID);
    KG_PROCESS_ERROR(pPackage);

    pItem = pPackage->GetItem(nPos);
Exit0:
    return pItem;
}

KHeroPackage* KItemList::GetHeroPackage(DWORD dwHeroTemplateID)
{
    KHeroPackage* pResult = NULL;
    KMAP_HERO_PACKAGE::iterator it;

    it = m_mapHeroPackage.find(dwHeroTemplateID); 
    KG_PROCESS_ERROR(it != m_mapHeroPackage.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

KHeroPackage* KItemList::AddHeroPackage(DWORD dwHeroTemplateID)
{
    KHeroPackage*   pResult     = NULL;
    BOOL            bRetCode    = false;
    KHeroPackage*   pNewPackage = NULL;
    std::pair<KMAP_HERO_PACKAGE::iterator, bool> InsRet;
    BOOL bInsertFlag = false;

    InsRet = m_mapHeroPackage.insert(make_pair(dwHeroTemplateID, KHeroPackage()));
    KGLOG_PROCESS_ERROR(InsRet.second);
    bInsertFlag = true;

    pNewPackage = &InsRet.first->second;
    bRetCode = pNewPackage->Init(m_pPlayer, dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pResult = pNewPackage;
Exit0:
    if (!pResult)
    {
        if (bInsertFlag)
        {
            m_mapHeroPackage.erase(dwHeroTemplateID);
            bInsertFlag = false;
        }
    }
    return pResult;
}

void KItemList::RemoveHeroPackage(DWORD dwHeroTemplateID)
{
    KHeroPackage* pPackage = NULL;
    KMAP_HERO_PACKAGE::iterator it;

    it = m_mapHeroPackage.find(dwHeroTemplateID);
    KG_PROCESS_ERROR(it != m_mapHeroPackage.end());

    pPackage = &it->second;
    pPackage->UnInit();
    m_mapHeroPackage.erase(it);

Exit0:
    return;
}

BOOL KItemList::CostPlayerItem(DWORD dwTabtype, DWORD dwIndex, int nAmount)
{
	BOOL bResult            = false;
    BOOL bRetCode           = false;
    int  nPlayerHasItemNum  = 0;

	KGLOG_PROCESS_ERROR(nAmount >= 0);
    KG_PROCESS_SUCCESS(nAmount == 0);

    nPlayerHasItemNum = GetPlayerItemCount(dwTabtype, dwIndex);
    KGLOG_PROCESS_ERROR(nPlayerHasItemNum >= nAmount);
    
    for (int i = eppiBegin; i < eppiTotal; ++i)
    {
        int nPackageItemCount   = 0;
        int nCostAmount         = 0;

        KPackage* pPackage = m_PlayerPackage.GetPackage(i);
        KGLOG_PROCESS_ERROR(pPackage);

        nPackageItemCount = pPackage->GetItemCount(dwTabtype, dwIndex);
        nCostAmount = MIN(nPackageItemCount, nAmount);

        bRetCode = pPackage->CostItem(dwTabtype, dwIndex, nCostAmount);
        KGLOG_PROCESS_ERROR(bRetCode);

        nAmount -= nCostAmount;

        if (nAmount == 0)
            break;
    }

    assert(nAmount == 0);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}


BOOL g_IsPlayerShotcutBar(int nPackageType, int nPackageIndex)
{
    return (nPackageType == ePlayerPackage && nPackageIndex == eppiPlayerShotcutBar);
}

KPLAYER_PACKAGE_EXCHANGE_RESULT KItemList::CanPlaceShortcutBarItem(
    int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
    int nDestPackageType, int nDestPackageIndex, int nDestPos
)
{
    KPLAYER_PACKAGE_EXCHANGE_RESULT nResult             = eppeInvaild;
    BOOL                            bRetCode            = false;
    IItem*                          piSrcItem           = NULL;
    IItem*                          piDestItem          = NULL;
    BOOL                            bSrcIsShotcutBar    = g_IsPlayerShotcutBar(nSrcPackageType, nSrcPackageIndex);
    BOOL                            bDestIsShotcutBar   = g_IsPlayerShotcutBar(nDestPackageType, nDestPackageIndex);

    KGLOG_PROCESS_ERROR(bDestIsShotcutBar);
    KG_PROCESS_ERROR(nSrcPackageType == ePlayerPackage); // 只能从玩家背包中拖物品到快捷栏

    piSrcItem = GetItem(nSrcPackageType, nSrcPackageIndex, nSrcPos);
    piDestItem = GetItem(nDestPackageType, nDestPackageIndex, nDestPos);

    // 快捷栏上的物品交换位置
    if (bSrcIsShotcutBar && bDestIsShotcutBar)
        goto Exit1;
    
    if (piSrcItem && !piDestItem)
    {
        KGOtherExtInfo* pOtherInfo = NULL;
        const KItemProperty* pcProperty = piSrcItem->GetProperty();

        KPackage*               pPlayerShotcutBarPackage    = NULL;
        int                     nShotcutBarPackageSize      = 0;

        KGLOG_PROCESS_ERROR(pcProperty->pItemInfo);
        KG_PROCESS_ERROR_RET_CODE(pcProperty->pItemInfo->nGenre == igCommonItem, eppeSrcItemCanNotPutInShotcutBar);

        pOtherInfo = pcProperty->pItemInfo->pOtherExtInfo;
        KGLOG_PROCESS_ERROR(pOtherInfo);
        KG_PROCESS_ERROR_RET_CODE(pOtherInfo->bUseInScene, eppeSrcItemCanNotUseInScene);

        // 检查快捷栏是否已经有该物品
        pPlayerShotcutBarPackage = m_PlayerPackage.GetPackage(eppiPlayerShotcutBar);
        KGLOG_PROCESS_ERROR(pPlayerShotcutBarPackage);

        nShotcutBarPackageSize = pPlayerShotcutBarPackage->GetSize();
        KGLOG_PROCESS_ERROR(nShotcutBarPackageSize > 0);

        for (int i = 0; i < nShotcutBarPackageSize; ++i)
        {
            IItem*                  pShotcutBarItem         = NULL;
            const KItemProperty*    pShotcutBarItemProperty = NULL;

            pShotcutBarItem = pPlayerShotcutBarPackage->GetItem(i);
            if (!pShotcutBarItem)
                continue;

            pShotcutBarItemProperty = pShotcutBarItem->GetProperty();
            KGLOG_PROCESS_ERROR(pShotcutBarItem);

            KG_PROCESS_ERROR_RET_CODE(
                pShotcutBarItemProperty->dwTabType != pcProperty->dwTabType || 
                pShotcutBarItemProperty->dwTabIndex != pcProperty->dwTabIndex, eppeShotcutBarHasSameItem
            );
        }
    }
    //else if (!piSrcItem && piDestItem) // 源物品没有，目标物品有，直接换出
    //{
    //    KG_PROCESS_ERROR(nSrcPackageType == ePlayerPackage);
    //}
    else if (piSrcItem && piDestItem)
    {
        const KItemProperty* pcSrcProperty = NULL;            
        const KItemProperty* pcDstProperty = NULL;

        pcSrcProperty = piSrcItem->GetProperty();
        KGLOG_PROCESS_ERROR(pcSrcProperty);

        pcDstProperty = piDestItem->GetProperty();
        KGLOG_PROCESS_ERROR(pcDstProperty);

        KG_PROCESS_ERROR_RET_CODE(pcSrcProperty->dwTabType == pcDstProperty->dwTabType, eppeDstItemNotEqualToSrcItem);
        KG_PROCESS_ERROR_RET_CODE(pcSrcProperty->dwTabIndex == pcDstProperty->dwTabIndex, eppeDstItemNotEqualToSrcItem);
    }

Exit1:
    nResult = eppeSuccessed;
Exit0:
    return nResult;
}

BOOL KItemList::PlaceShortcutBarItem(
    IItem* pSrcItem, int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
    IItem* pDestItem, int nDestPackageType, int nDestPackageIndex, int nDestPos
)
{
    BOOL                    bResult	                    = false;
    BOOL                    bRetCode	                = false;
    const KItemProperty*    pSrcItemProperty            = NULL;
    const KItemProperty*    pDestItemProperty           = NULL;
    DWORD                   dwSrcItemStackNum           = 0;
    DWORD                   dwDestItemStackNum          = 0;
    DWORD                   dwSrcItemShotcutBarStackNum = 0;
    KPackage*               pPlayerShotcutBarPackage    = NULL;
    int                     nShotcutBarPackageSize      = 0;
    IItem*                  pShotcutBarItem             = NULL;
    BOOL                    bSrcIsShotcutBar            = g_IsPlayerShotcutBar(nSrcPackageType, nSrcPackageIndex);
    BOOL                    bDestIsShotcutBar           = g_IsPlayerShotcutBar(nDestPackageType, nDestPackageIndex);

    KGLOG_PROCESS_ERROR(bDestIsShotcutBar);
    KGLOG_PROCESS_ERROR(pSrcItem || pDestItem);

    // 都是快捷栏，直接交换
    if (bSrcIsShotcutBar && bDestIsShotcutBar)
    {
        if (pSrcItem)
        {
            bRetCode = PlaceItem(pSrcItem, nDestPackageType, nDestPackageIndex, nDestPos);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        
        if (pDestItem)
        {
            bRetCode = PlaceItem(pDestItem, nSrcPackageType, nSrcPackageIndex, nSrcPos);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        
        goto Exit1;
    }
    
    // 只有目标位置是快捷栏
    // 源物品有，目标物品没有
    if (pSrcItem && !pDestItem)
    {
        pSrcItemProperty = pSrcItem->GetProperty();
        KGLOG_PROCESS_ERROR(pSrcItemProperty);
        KGLOG_PROCESS_ERROR(pSrcItemProperty->pItemInfo);

        dwSrcItemStackNum = pSrcItem->GetStackNum();
        KGLOG_PROCESS_ERROR(dwSrcItemStackNum >= 0);

        dwSrcItemShotcutBarStackNum = pSrcItemProperty->pItemInfo->dwShotcutBarMaxStackNum;
        KGLOG_PROCESS_ERROR(dwSrcItemShotcutBarStackNum >= 0);

        // State 1-a:A有B没有且A堆叠数小于等于A贴身允许堆叠上限
        if (dwSrcItemStackNum <= dwSrcItemShotcutBarStackNum)
        {
            bRetCode = PlaceItem(pSrcItem, nDestPackageType, nDestPackageIndex, nDestPos);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else
        {
            pDestItem = g_pSO3World->m_ItemHouse.CloneItem(pSrcItem);
            KGLOG_PROCESS_ERROR(pDestItem);

            bRetCode = pDestItem->SetStackNum(dwSrcItemShotcutBarStackNum);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = PlaceItem(pDestItem, nDestPackageType, nDestPackageIndex, nDestPos);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = pSrcItem->SetStackNum(dwSrcItemStackNum - dwSrcItemShotcutBarStackNum);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = PlaceItem(pSrcItem, nSrcPackageType, nSrcPackageIndex, nSrcPos);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }
    else if (!pSrcItem && pDestItem) // 源物品没有，目标物品有，直接换出
    {
        bRetCode = PlaceItem(pDestItem, nSrcPackageType, nSrcPackageIndex, nSrcPos);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else // 源物品有，目标物品也有
    {
        pSrcItemProperty = pSrcItem->GetProperty();
        KGLOG_PROCESS_ERROR(pSrcItemProperty);
        KGLOG_PROCESS_ERROR(pSrcItemProperty->pItemInfo);

        dwSrcItemStackNum = pSrcItem->GetStackNum();
        KGLOG_PROCESS_ERROR(dwSrcItemStackNum >= 0);

        dwSrcItemShotcutBarStackNum = pSrcItemProperty->pItemInfo->dwShotcutBarMaxStackNum;
        KGLOG_PROCESS_ERROR(dwSrcItemShotcutBarStackNum >= 0);
        
        pDestItemProperty = pDestItem->GetProperty();
        KGLOG_PROCESS_ERROR(pDestItemProperty);
        KGLOG_PROCESS_ERROR(pDestItemProperty->pItemInfo);

        dwDestItemStackNum = pDestItem->GetStackNum();
        KGLOG_PROCESS_ERROR(dwDestItemStackNum >= 0);

        KGLOG_PROCESS_ERROR(pSrcItemProperty->dwTabType == pDestItemProperty->dwTabType);
        KGLOG_PROCESS_ERROR(pSrcItemProperty->dwTabIndex == pDestItemProperty->dwTabIndex);

        // 目前只处理AB同种情况
        // State 2-a:A有B有且A堆叠数与B堆叠书之和小于等于此种物品物品贴身允许堆叠上限
        if (dwSrcItemStackNum + dwDestItemStackNum <= dwSrcItemShotcutBarStackNum)
        {
            bRetCode = pDestItem->SetStackNum(dwSrcItemStackNum + dwDestItemStackNum);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = PlaceItem(pDestItem, nDestPackageType, nDestPackageIndex, nDestPos);
            KGLOG_PROCESS_ERROR(bRetCode);

            g_pSO3World->m_ItemHouse.DestroyItem(pSrcItem);
        }
        else if (dwSrcItemStackNum + dwDestItemStackNum > dwSrcItemShotcutBarStackNum) // State 2-b:A有B有且A堆叠数与B堆叠书之和大于此种物品物品贴身允许堆叠上限
        {
            int nSrcLeftStackNum = dwSrcItemStackNum + dwDestItemStackNum - dwSrcItemShotcutBarStackNum;

            bRetCode = pDestItem->SetStackNum(dwSrcItemShotcutBarStackNum);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = PlaceItem(pDestItem, nDestPackageType, nDestPackageIndex, nDestPos);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = pSrcItem->SetStackNum(nSrcLeftStackNum);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = PlaceItem(pSrcItem, nSrcPackageType, nSrcPackageIndex, nSrcPos);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }

Exit1:
    g_PlayerServer.DoExchangeItemRespond(
        m_pPlayer->m_nConnIndex, true, nSrcPackageType, nSrcPackageIndex, nSrcPos, 
        nDestPackageType, nDestPackageIndex, nDestPos, eppeSuccessed
    );

    bResult	= true;
Exit0:
    return bResult;
}

KPLAYER_PACKAGE_EXCHANGE_RESULT KItemList::CanExchangeItem(
    int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
    int nDestPackageType, int nDestPackageIndex, int nDestPos
)
{
    KPLAYER_PACKAGE_EXCHANGE_RESULT nResult = eppeInvaild;
    BOOL    bRetCode        = false;
    IItem*  piSrcItem       = NULL;
    IItem*  piDestItem      = NULL;
    KHeroData* pHeroData    = NULL;

    KGLOG_PROCESS_ERROR(!g_IsPlayerShotcutBar(nSrcPackageType, nSrcPackageIndex));
    KGLOG_PROCESS_ERROR(!g_IsPlayerShotcutBar(nDestPackageType, nDestPackageIndex));

    piSrcItem = GetItem(nSrcPackageType, nSrcPackageIndex, nSrcPos);
    piDestItem = GetItem(nDestPackageType, nDestPackageIndex, nDestPos);

    KG_PROCESS_ERROR(piSrcItem || piDestItem);
    
    if (piSrcItem) // 检查源物品能否放到目标位置
    {
        if (nDestPackageType == eHeroPackage) // 英雄背包
        {
            KHeroPackage* pHeroPackage = GetHeroPackage(nDestPackageIndex);
            KGLOG_PROCESS_ERROR(pHeroPackage);

            bRetCode = pHeroPackage->CheckItemType(piSrcItem, nDestPos);
            KG_PROCESS_ERROR_RET_CODE(bRetCode, eppeSrcItemCanNotPutInHeroPackage);

            pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(nDestPackageIndex);
            KGLOG_PROCESS_ERROR(pHeroData);

            KG_PROCESS_ERROR_RET_CODE(pHeroData->m_nLevel >= piSrcItem->GetRequireLevel(), eppeHeroLevelInconformityRequireLevel);
        }
        else
        {
            bRetCode = m_PlayerPackage.CanPutInPackage(piSrcItem, nDestPackageIndex);
            KG_PROCESS_ERROR_RET_CODE(bRetCode, eppeSrcItemCanNotPutInPlayerPackage);
        }
    }
    
    if (piDestItem) // 检查目标物品能否放到源位置
    {
        if (nSrcPackageType == eHeroPackage) // 英雄背包
        {
            KHeroPackage* pHeroPackage = GetHeroPackage(nSrcPackageIndex);
            KGLOG_PROCESS_ERROR(pHeroPackage);

            bRetCode = pHeroPackage->CheckItemType(piDestItem, nSrcPos);
            KG_PROCESS_ERROR_RET_CODE(bRetCode, eppeDstItemCanNotPutInHeroPackage);

            pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(nSrcPackageIndex);
            KGLOG_PROCESS_ERROR(pHeroData);

            KG_PROCESS_ERROR_RET_CODE(pHeroData->m_nLevel >= piDestItem->GetRequireLevel(), eppeHeroLevelInconformityRequireLevel);
        }
        else
        {
            bRetCode = m_PlayerPackage.CanPutInPackage(piDestItem, nSrcPackageIndex);
            KG_PROCESS_ERROR_RET_CODE(bRetCode, eppeDstItemCanNotPutInPlayerPackage);
        }
    }

    nResult = eppeSuccessed;
Exit0:
    return nResult;
}

BOOL KItemList::ExchangeItem(
    int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
    int nDestPackageType, int nDestPackageIndex, int nDestPos
)
{
    BOOL                                bResult             = false;
    BOOL                                bRetCode            = false;
    IItem*                              piSrcItem           = NULL;
    IItem*                              piDestItem          = NULL;
    IItem*                              piBindItem          = NULL;
    KITEMPOS                            pos                 = {0};
    KPLAYER_PACKAGE_EXCHANGE_RESULT     nExChangeResult     = eppeInvaild;
    BOOL                                bSrcIsShotcutBar    = g_IsPlayerShotcutBar(nSrcPackageType, nSrcPackageIndex);
    BOOL                                bDestIsShotcutBar   = g_IsPlayerShotcutBar(nDestPackageType, nDestPackageIndex);
	const KItemProperty*				pItemProperty		= NULL;
    
    bRetCode = IsValidPos(nSrcPackageType, nSrcPackageIndex, nSrcPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = IsValidPos(nDestPackageType, nDestPackageIndex, nDestPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bSrcIsShotcutBar || bDestIsShotcutBar)
    {
        // 保证在PlaceShortcutBarItem函数中，目标位置一定是快捷栏
        if (bDestIsShotcutBar)
        {
            nExChangeResult = CanPlaceShortcutBarItem(nSrcPackageType, nSrcPackageIndex, nSrcPos, nDestPackageType, nDestPackageIndex, nDestPos);
            KG_PROCESS_ERROR(nExChangeResult == eppeSuccessed); 

            piSrcItem = PickUpItem(nSrcPackageType, nSrcPackageIndex, nSrcPos);
            piDestItem = PickUpItem(nDestPackageType, nDestPackageIndex, nDestPos);

            bRetCode = PlaceShortcutBarItem(
                piSrcItem, nSrcPackageType, nSrcPackageIndex, nSrcPos, piDestItem, nDestPackageType, nDestPackageIndex, nDestPos
            );
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else
        {
            nExChangeResult = CanPlaceShortcutBarItem(nDestPackageType, nDestPackageIndex, nDestPos, nSrcPackageType, nSrcPackageIndex, nSrcPos);
            KG_PROCESS_ERROR(nExChangeResult == eppeSuccessed); 

            piSrcItem = PickUpItem(nSrcPackageType, nSrcPackageIndex, nSrcPos);
            piDestItem = PickUpItem(nDestPackageType, nDestPackageIndex, nDestPos);

            bRetCode = PlaceShortcutBarItem(
                piDestItem, nDestPackageType, nDestPackageIndex, nDestPos, piSrcItem, nSrcPackageType, nSrcPackageIndex, nSrcPos
            );        
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        
        goto Exit1;
    }

    nExChangeResult = CanExchangeItem(nSrcPackageType, nSrcPackageIndex, nSrcPos, nDestPackageType, nDestPackageIndex, nDestPos);
    KG_PROCESS_ERROR(nExChangeResult == eppeSuccessed);

    piSrcItem = PickUpItem(nSrcPackageType, nSrcPackageIndex, nSrcPos);
    piDestItem = PickUpItem(nDestPackageType, nDestPackageIndex, nDestPos);
    
    // 处理物品堆叠
    if (piSrcItem && piDestItem)
    {
        bRetCode = piDestItem->CanStackItem(piSrcItem);
        if (bRetCode) // 能堆叠
        {
            int nMaxStackNum = piDestItem->GetMaxStackNum();
            int nDestStackNum = piDestItem->GetStackNum();
            int nSrcStackNum = piSrcItem->GetStackNum();

            if (nDestStackNum + nSrcStackNum <= nMaxStackNum)
            {
                nDestStackNum += nSrcStackNum;
                bRetCode = piDestItem->SetStackNum(nDestStackNum);
                KGLOG_PROCESS_ERROR(bRetCode);

                bRetCode = PlaceItem(piDestItem, nDestPackageType, nDestPackageIndex, nDestPos);
                KGLOG_PROCESS_ERROR(bRetCode);

                g_pSO3World->m_ItemHouse.DestroyItem(piSrcItem);
            }
            else
            {
                nSrcStackNum -= nMaxStackNum - nDestStackNum;
                bRetCode = piDestItem->SetStackNum(nMaxStackNum);
                KGLOG_PROCESS_ERROR(bRetCode);

                bRetCode = PlaceItem(piDestItem, nDestPackageType, nDestPackageIndex, nDestPos);
                KGLOG_PROCESS_ERROR(bRetCode);

                bRetCode = piSrcItem->SetStackNum(nSrcStackNum);
                KGLOG_PROCESS_ERROR(bRetCode);

                bRetCode = PlaceItem(piSrcItem, nSrcPackageType, nSrcPackageIndex, nSrcPos);
                KGLOG_PROCESS_ERROR(bRetCode);
            }

            goto Exit1;
        }
    }
    
    // 交换
    if (piSrcItem)
    {
        bRetCode = PlaceItem(piSrcItem, nDestPackageType, nDestPackageIndex, nDestPos);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (piDestItem)
    {
        bRetCode = PlaceItem(piDestItem, nSrcPackageType, nSrcPackageIndex, nSrcPos);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    g_PlayerServer.DoExchangeItemRespond(
        m_pPlayer->m_nConnIndex, true, nSrcPackageType, nSrcPackageIndex, nSrcPos, 
        nDestPackageType, nDestPackageIndex, nDestPos, nExChangeResult
    );

    // bind
    if (nSrcPackageType == eHeroPackage)
    {
        piBindItem = piDestItem;
        pos.nPackageType = nSrcPackageType;
        pos.nPackageIndex = nSrcPackageIndex;
        pos.nPos = nSrcPos;
		g_LSClient.DoHeroEquipScoreChangeNotify(m_pPlayer, (DWORD)nSrcPackageIndex);
		g_LSClient.DoTeamEquipScoreChangeNotify(m_pPlayer);
    }
    else if (nDestPackageType == eHeroPackage)
    {
        piBindItem = piSrcItem;
        pos.nPackageType = nDestPackageType;
        pos.nPackageIndex = nDestPackageIndex;
        pos.nPos = nDestPos;
		g_LSClient.DoHeroEquipScoreChangeNotify(m_pPlayer, (DWORD)nDestPackageIndex);
		g_LSClient.DoTeamEquipScoreChangeNotify(m_pPlayer);
    }

    if (piBindItem)
    {
        const KItemProperty* pItemProperty = NULL;
        pItemProperty = piBindItem->GetProperty();
        KGLOG_PROCESS_ERROR(pItemProperty);
        KGLOG_PROCESS_ERROR(pItemProperty->pItemInfo);

        if (pItemProperty->pItemInfo->nBindType == ibtBindOnEquipped)
        {
            piBindItem->SetBindFlag(true);
            g_PlayerServer.DoSyncItemBind(m_pPlayer->m_nConnIndex, pos.nPackageType, pos.nPackageIndex, pos.nPos, true);
        }
    }

Exit1:
    if (piSrcItem)
    {
        pItemProperty = piSrcItem->GetProperty();
        KGLOG_PROCESS_ERROR(pItemProperty);

        if (nSrcPackageType != eHeroPackage && nDestPackageType == eHeroPackage)
        {
            m_pPlayer->OnEvent(pePutOnEquipment, pItemProperty->dwTabType, pItemProperty->dwTabIndex);
        }

        if (!bSrcIsShotcutBar && bDestIsShotcutBar)
        {
            m_pPlayer->OnEvent(peMoveItemToShortcutBar, pItemProperty->dwTabType, pItemProperty->dwTabIndex);
        }
    }
	
    bResult = true;
Exit0:
    if (!bResult)
    {
         g_PlayerServer.DoExchangeItemRespond(
            m_pPlayer->m_nConnIndex, false, nSrcPackageType, nSrcPackageIndex, nSrcPos, 
            nDestPackageType, nDestPackageIndex, nDestPos, nExChangeResult
        );
    }

    return bResult;
}

IItem*  KItemList::GetItem(int nPackageType, int nPackageIndex, int nPos)
{
    IItem* pResult  = NULL;
    IItem* pItem    = NULL;

    if (nPackageType == ePlayerPackage)
    {
        pItem = m_PlayerPackage.GetItem(nPackageIndex, nPos);
        KG_PROCESS_ERROR(pItem);
    }
    else
    {
        KHeroPackage* pHeroPackage = GetHeroPackage(nPackageIndex);
        KGLOG_PROCESS_ERROR(pHeroPackage);

        pItem = pHeroPackage->GetItem(nPos);
        KG_PROCESS_ERROR(pItem);
    }

    pResult = pItem;
Exit0:
    return pResult;
}

IItem*  KItemList::PickUpItem(int nPackageType, int nPackageIndex, int nPos)
{
    IItem* pResult  = NULL;
    IItem* pItem    = NULL;

    if (nPackageType == ePlayerPackage)
    {
        pItem = m_PlayerPackage.PickUpItem(nPackageIndex, nPos);
        KG_PROCESS_ERROR(pItem);
    }
    else
    {
        KHeroPackage* pHeroPackage = GetHeroPackage(nPackageIndex);
        KGLOG_PROCESS_ERROR(pHeroPackage);

        pItem = pHeroPackage->PickUpItem(nPos);
        KG_PROCESS_ERROR(pItem);
    }

    // 通知客户端删除改位置的物品
    g_PlayerServer.DoDestroyItem(m_pPlayer->m_nConnIndex, nPackageType, nPackageIndex, nPos);

    pResult = pItem;
Exit0:
    return pResult;
}

BOOL KItemList::PlaceItem(IItem* pItem, int nPackageType, int nPackageIndex, int nPos)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    if (nPackageType == ePlayerPackage)
    {
        bRetCode = m_PlayerPackage.PlaceItem(pItem, nPackageIndex, nPos);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        KHeroPackage* pHeroPackage = GetHeroPackage(nPackageIndex);
        KGLOG_PROCESS_ERROR(pHeroPackage);

        bRetCode = pHeroPackage->PlaceItem(pItem, nPos);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    g_PlayerServer.DoSyncItemData(
        m_pPlayer->m_nConnIndex, m_pPlayer->m_dwID, pItem, nPackageType, nPackageIndex, nPos
    );

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::CanUseItem(KGItemInfo* pItemInfo)
{
    BOOL bResult = true;
    KGOtherExtInfo* pOtherExtInfo = NULL;

    KGLOG_PROCESS_ERROR(pItemInfo);

    pOtherExtInfo = pItemInfo->pOtherExtInfo;
    KG_PROCESS_ERROR(pOtherExtInfo);
    
    if (pItemInfo->dwScriptID != 0)
        return true;

    for (int i = 0; i < countof(pOtherExtInfo->dwHeroBuffID); ++i)
    {
        uint32_t dwBuffID = pOtherExtInfo->dwHeroBuffID[i];
        if (dwBuffID != 0)
            return true;
    }

    for (int i = 0; i < countof(pOtherExtInfo->dwPlayerBuffID); ++i)
    {
        uint32_t dwBuffID = pOtherExtInfo->dwPlayerBuffID[i];
        if (dwBuffID != 0)
            return true;
    }

    if (pOtherExtInfo->dwAwardTableID > 0)
        return true;

    if (pOtherExtInfo->dwRecipeID)
        return true;

Exit0:
    return false;
}

BOOL KItemList::UseItem(int nPackageType, int nPackageIndex, int nPos)
{
	BOOL            bResult    = false;
    BOOL            bRetCode   = false;
    IItem*          piItem   = NULL;
    const KItemProperty* pcItemProperty = NULL;
    DWORD           dwScriptID = ERROR_ID;
    uint32_t        dwCoolDownID = 0;
    KGOtherExtInfo* pOtherExtInfo = NULL;
    KHero*          pHero = NULL;
    BOOL            bUseSuccessd = false;
    KHeroData*      pHeroData = NULL;
    KGItemInfo*     pItemInfo = NULL;
    int             nTabType  = 0;
    int             nTabIndex = 0;

    piItem = GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(piItem);

    pcItemProperty = piItem->GetProperty();
    KGLOG_PROCESS_ERROR(pcItemProperty && pcItemProperty->pItemInfo);

    bRetCode = CanUseItem(pcItemProperty->pItemInfo);
    KG_PROCESS_ERROR(bRetCode);

    pItemInfo = pcItemProperty->pItemInfo;
    KGLOG_PROCESS_ERROR(pItemInfo);

    nTabType = (int)pcItemProperty->dwTabType;
    nTabIndex = (int)pcItemProperty->dwTabIndex;

    KG_PROCESS_ERROR(pcItemProperty->pItemInfo->nGenre == igCommonItem); // 只有普通类型的道具可以使用

    pOtherExtInfo = pcItemProperty->pItemInfo->pOtherExtInfo;
    KG_PROCESS_ERROR(pOtherExtInfo);

    if (pOtherExtInfo->dwAwardTableID)
    {
        if (pOtherExtInfo->bAwardAll)
        {
            bRetCode = g_pSO3World->m_AwardMgr.HasEnoughBagToAwardAll(m_pPlayer, pOtherExtInfo->dwAwardTableID);
        }
        else
        {
            bRetCode = g_pSO3World->m_AwardMgr.HasEnoughBagToAwardRandomOne(m_pPlayer, pOtherExtInfo->dwAwardTableID);
        }

        if (!bRetCode)
        {
            g_PlayerServer.DoDownwardNotify(m_pPlayer, KMESSAGE_NOT_ENOUGH_BAG);
            goto Exit0;
        }
    }

    if (pItemInfo->nSub == cstRecipe) // 配方不受RequireLevel影响
    {
        KRecipe* pRecipe = g_pSO3World->m_Settings.m_RecipeData.GetRecipe(pOtherExtInfo->dwRecipeID);
        KGLOG_PROCESS_ERROR(pRecipe);
        bRetCode = m_pPlayer->m_MakingMachine.CanLearnRecipe(pRecipe, true);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
	else if (pItemInfo->nSub == csTeamLogo)
	{
		bRetCode = m_pPlayer->CanAddTeamLogo(WORD(pOtherExtInfo->dwRecipeID));
		if (!bRetCode)
		{
			g_PlayerServer.DoDownwardNotify(m_pPlayer, KMESSAGE_TEAMLOGO_HAS);
			KG_PROCESS_ERROR(bRetCode);
		}
	}
    else
    {
        pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(m_pPlayer->m_dwMainHeroTemplateID);
        KGLOG_PROCESS_ERROR(pHeroData);
        KG_PROCESS_ERROR(pHeroData->m_nLevel >= pcItemProperty->pItemInfo->nRequireLevel);
    }

    pHero = m_pPlayer->GetFightingHero();
    if (pHero) // 在场景中
        KG_PROCESS_ERROR(pOtherExtInfo->bUseInScene);
    else
        KG_PROCESS_ERROR(!pOtherExtInfo->bUseInScene);

    dwCoolDownID = pOtherExtInfo->dwCoolDownID;
    if (dwCoolDownID > 0)
    {
        int nCDInterval = 0;

        bRetCode = m_pPlayer->m_CDTimerList.CheckTimer(dwCoolDownID);
        KG_PROCESS_ERROR(bRetCode);

        nCDInterval = g_pSO3World->m_Settings.m_CoolDownList.GetCoolDownValue(dwCoolDownID);
        m_pPlayer->m_CDTimerList.ResetTimer(dwCoolDownID, nCDInterval);
    }

    dwScriptID = pcItemProperty->pItemInfo->dwScriptID;
    if (dwScriptID)
    {
        bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
        if (bRetCode && g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, SCRIPT_ON_USE_ITEM))
        {
            int nTopIndex = 0;

            g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
            g_pSO3World->m_ScriptManager.Push(m_pPlayer);
            g_pSO3World->m_ScriptManager.Push(piItem);
            g_pSO3World->m_ScriptManager.Call(dwScriptID, SCRIPT_ON_USE_ITEM, 1);
            g_pSO3World->m_ScriptManager.GetValuesFromStack("b", &bUseSuccessd);
            g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

            KG_PROCESS_ERROR(bUseSuccessd);
        }
    }

    // 确认道具还存在
    piItem = GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(piItem);

    if (pHero) // 场景广播道具使用
    {
        g_PlayerServer.DoBroadcastUseItem(
            pHero->m_pScene, pHero->m_dwID, pcItemProperty->dwTabType, pcItemProperty->dwTabIndex
        );
    }

    PLAYER_LOG(m_pPlayer, "item,useitem,%d-%d", pcItemProperty->dwTabType, pcItemProperty->dwTabIndex);

    if (pOtherExtInfo->bDestroyAfterUse)
    {
        if (piItem->CanStack())
        {
            bRetCode = CostOneStackItem(nPackageType, nPackageIndex, nPos);
			KGLOG_PROCESS_ERROR(bRetCode);
        }
        else
        {
            bRetCode = DestroyItem(nPackageType, nPackageIndex, nPos);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        if (nPackageType == ePlayerPackage && nPackageIndex == eppiPlayerShotcutBar)
        {
            m_PlayerPackage.PushReloadInfo(nPos, nTabType, nTabIndex);
        }
    }

    // 应用后续效果，注意道具已经不存在了

    if (pOtherExtInfo->dwAwardTableID)
    {
        if (pOtherExtInfo->bAwardAll)
        {
            bRetCode = g_pSO3World->m_AwardMgr.AwardAll(m_pPlayer, pOtherExtInfo->dwAwardTableID, "useitem");
            KGLOG_CHECK_ERROR(bRetCode);
        }
        else
        {
            KAWARD_ITEM* pAwardItem = g_pSO3World->m_AwardMgr.AwardRandomOne(m_pPlayer, pOtherExtInfo->dwAwardTableID);
            KGLOG_CHECK_ERROR(pAwardItem);
        }
    }
    
    if (pItemInfo->nSub == cstRecipe)
    {
        if (pOtherExtInfo->dwRecipeID)
        {
            bRetCode = m_pPlayer->LearnRecipe(pOtherExtInfo->dwRecipeID);
            KGLOG_CHECK_ERROR(bRetCode);
        }
    }
    else if (pItemInfo->nSub == csTeamLogo)	
    {
        bRetCode = m_pPlayer->AddTeamLogo((WORD)pOtherExtInfo->dwRecipeID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (pHero)
    {
        for (int i = 0; i <  countof(pOtherExtInfo->dwHeroBuffID); ++i)
        {
            KBuff* pBuff = NULL;
            uint32_t dwBuffID = pOtherExtInfo->dwHeroBuffID[i];

            if (dwBuffID == 0)
                continue;

            pBuff = g_pSO3World->m_BuffManager.GetBuff(dwBuffID);
            KGLOG_PROCESS_ERROR(pBuff);

            bRetCode = pHero->AddBuff(dwBuffID);
            KGLOG_PROCESS_ERROR(bRetCode);

            g_PlayerServer.DoAddBuffNotify(pHero, dwBuffID, pBuff->m_nFrame);
        }

        ++pHero->m_GameStat.m_nUseItemCount;
    }

    for (int i = 0; i < countof(pOtherExtInfo->dwPlayerBuffID); ++i)
    {
        KBuff* pBuff = NULL;
        uint32_t dwBuffID = pOtherExtInfo->dwPlayerBuffID[i];

        if (dwBuffID == 0)
            continue;
        
        bRetCode = m_pPlayer->AddBuff(dwBuffID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    m_pPlayer->OnEvent(peConsumeItem, pItemInfo->nGenre, pItemInfo->dwID);
       
    bResult = true;
Exit0:
    g_PlayerServer.DoUseItemRespond(m_pPlayer->m_nConnIndex, bResult, nPackageType, nPackageIndex, nPos);
    return bResult;
}

BOOL KItemList::DestroyItem(int nPackageType, int nPackageIndex, int nPos)
{
	BOOL bResult    = false;
    BOOL bRetCode   = false;
    IItem* piItem   = NULL;

    piItem = PickUpItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(piItem);

    g_pSO3World->m_ItemHouse.DestroyItem(piItem);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::CostOneStackItem(int nPackageType, int nPackageIndex, int nPos)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    IItem* piItem   = NULL;
    int    nStackNum = 0;

    piItem = GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(piItem);

    bRetCode = piItem->CanStack();
    KGLOG_PROCESS_ERROR(bRetCode);

    nStackNum = piItem->GetStackNum();
	KGLOG_PROCESS_ERROR(nStackNum > 0);
    if (nStackNum == 1)
    {
        DestroyItem(nPackageType, nPackageIndex, nPos);

        goto Exit1;
    }

    nStackNum = nStackNum - 1;
    bRetCode = piItem->SetStackNum(nStackNum);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoUpdateItemAmount(m_pPlayer->m_nConnIndex, nPackageType, nPackageIndex, nPos, nStackNum);

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

int KItemList::GetPlayerFreeRoomSize(int nPackageIndex)
{
    int         nFreeRoomSize   = 0;
    KPackage*   pPackage        = NULL;

    pPackage = m_PlayerPackage.GetPackage(nPackageIndex);
    KGLOG_PROCESS_ERROR(pPackage);

    nFreeRoomSize = pPackage->GetFreeRoomSize();

Exit0:
    return nFreeRoomSize;
}

int KItemList::GetPlayerItemCount(DWORD dwTabType, DWORD dwIndex)
{
    int         nItemCount      = 0;
    KPackage*   pPackage        = NULL;

    for (int i = eppiBegin; i < eppiTotal; ++i)
    {
        pPackage = m_PlayerPackage.GetPackage(i);
        KGLOG_PROCESS_ERROR(pPackage);

        nItemCount += pPackage->GetItemCount(dwTabType, dwIndex);
    }

Exit0:
    return nItemCount;
}

KPackage* KItemList::GetPlayerPackage(int nPackageIndex)
{
    return m_PlayerPackage.GetPackage(nPackageIndex);
}

BOOL KItemList::CutStackItem(
    int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
    int nDestPackageType, int nDestPackageIndex, int nDestPos, int nCutNum
)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    IItem* pSrcItem = NULL;
    IItem* pDstItem = NULL;
    int    nStackNum = 0;

    pSrcItem = GetItem(nSrcPackageType, nSrcPackageIndex, nSrcPos);
    KGLOG_PROCESS_ERROR(pSrcItem);

    pDstItem = GetItem(nDestPackageType, nDestPackageIndex, nDestPos);
    KGLOG_PROCESS_ERROR(!pDstItem);

    if (nDestPackageType == ePlayerPackage)
    {
        bRetCode = m_PlayerPackage.CanPutInPackage(pSrcItem, nDestPackageIndex);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = pSrcItem->CanStack();
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLOG_PROCESS_ERROR(nCutNum > 0);
    nStackNum = pSrcItem->GetStackNum();
    KGLOG_PROCESS_ERROR(nStackNum >= nCutNum);

    if (nStackNum == nCutNum)
    {
        bRetCode = ExchangeItem(nSrcPackageType, nSrcPackageIndex, nSrcPos, nDestPackageType, nDestPackageIndex, nDestPos);
        KGLOG_PROCESS_ERROR(bRetCode);
        goto Exit1;
    }

    nStackNum -= nCutNum;
    bRetCode = pSrcItem->SetStackNum(nStackNum);
    KGLOG_PROCESS_ERROR(bRetCode);
    
    g_PlayerServer.DoUpdateItemAmount(m_pPlayer->m_nConnIndex, nSrcPackageType, nSrcPackageIndex, nSrcPos, nStackNum);

    pDstItem = g_pSO3World->m_ItemHouse.CloneItem(pSrcItem);
    KGLOG_PROCESS_ERROR(pDstItem);

    bRetCode = pDstItem->SetStackNum(nCutNum);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = PlaceItem(pDstItem, nDestPackageType, nDestPackageIndex, nDestPos);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KItemList::HasFreePosAddItem(DWORD dwTabType, DWORD dwIndex)
{
    return m_PlayerPackage.HasFreePos(eppiPlayerItemBox);
}


BOOL KItemList::AbradeEquip(KHero* pHero)
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    KHeroPackage*   pHeroPackage    = NULL;
    KMAP_HERO_PACKAGE::iterator it;

    assert(pHero);

    it = m_mapHeroPackage.find(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(it != m_mapHeroPackage.end());

    pHeroPackage = &it->second;
    KGLOG_PROCESS_ERROR(pHeroPackage);

    bRetCode = pHeroPackage->AbradeEquip();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::RepairAllHeroEquip()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KMAP_HERO_PACKAGE::iterator it;

    for (it = m_mapHeroPackage.begin(); it != m_mapHeroPackage.end(); ++it)
    {
        bRetCode = it->second.RepairAllEquip();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

int KItemList::GetRepairTotalPrice()
{
    int nResult                 = -1;
	int nTotalPrice 			= 0;
    int nSingleHeroRepairPrice  = 0;
    KMAP_HERO_PACKAGE::iterator it;

    for (it = m_mapHeroPackage.begin(); it != m_mapHeroPackage.end(); ++it)
    {
        nSingleHeroRepairPrice = it->second.GetRepairPrice();
        KGLOG_PROCESS_ERROR(nSingleHeroRepairPrice >= 0);
        KGLOG_PROCESS_ERROR(INT_MAX - nSingleHeroRepairPrice > nResult);
		nTotalPrice += nSingleHeroRepairPrice;
    }
	
	nResult = nTotalPrice;
Exit0:
    return nResult;
}

BOOL KItemList::CheckItemValidity()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = m_PlayerPackage.CheckItemValidity();
    KGLOG_CHECK_ERROR(bRetCode);

    for (KMAP_HERO_PACKAGE::iterator it = m_mapHeroPackage.begin(); it != m_mapHeroPackage.end(); ++it)
    {
        bRetCode = it->second.CheckItemValidity();
        KGLOG_CHECK_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::HasItem(DWORD dwTabType, DWORD dwTabIndex, int nCount)
{
	BOOL bResult = false;
    int nItemCount = 0;

    KGLOG_PROCESS_ERROR(nCount > 0);

	nItemCount = GetPlayerItemCount(dwTabType, dwTabIndex);

	bResult = (nItemCount >= nCount);
Exit0:
	return bResult;
}

BOOL KItemList::CostItem(int nPackageType, int nPackageIndex, int nPos, int nCount)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
    if (nPackageType == ePlayerPackage)
    {
        bRetCode = m_PlayerPackage.CostItem(nPackageIndex, nPos, nCount);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        KHeroPackage* pHeroPackage = GetHeroPackage(nPackageIndex);
        KGLOG_PROCESS_ERROR(pHeroPackage);

        bRetCode = pHeroPackage->CostItem(nPos, nCount);
        KGLOG_PROCESS_ERROR(bRetCode);
    }	

	bResult = true;
Exit0:
	return bResult;

}

BOOL KItemList::HasFreeRoom(int (&RoomSize)[eppiTotal])
{
    BOOL bResult = false;
    int nRoomSize = 0;
    for (int i = 0; i < countof(RoomSize); ++i)
    {
        if (!RoomSize[i])
            continue;

        nRoomSize = GetPlayerFreeRoomSize(i);
        KG_PROCESS_ERROR(nRoomSize >= RoomSize[i]);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KItemList::DecomposeEquip(size_t nItemCount, KDecomposeItems* pItems)
{
	BOOL    bResult 	= false;
	BOOL    bRetCode 	= false;
    std::vector<IItem*> vecDecomposeResult; 
    int nItemNum = 0;
    int nFreeRoomSize = 0;
    KPackage* pPackage = NULL;

    assert(pItems);

    for (size_t i = 0; i < nItemCount; ++i)
    {
        bRetCode = DecomposeOneEquip(pItems[i].uPackageTypeEquipIn, pItems[i].uPackageIndexEquipIn, pItems[i].uPosEquipIn, vecDecomposeResult);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    // 将相同道具合并
    for (size_t i = 0; i < vecDecomposeResult.size(); ++i)
    {
        IItem* pTemp1 = vecDecomposeResult[i];
        if (pTemp1 == NULL)
            continue;

        for (size_t j = i + 1; j < vecDecomposeResult.size(); ++j)
        {
            IItem* pTemp2 = vecDecomposeResult[j];
            if (pTemp2 == NULL)
                continue;

            if (pTemp1->CanStackItem(pTemp2))
            {
                int nStackNum1 = pTemp1->GetStackNum();
                int nStackNum2 = pTemp2->GetStackNum();

                if (nStackNum1 + nStackNum2 <= pTemp1->GetMaxStackNum())
                {
                    bRetCode = pTemp1->SetStackNum(nStackNum1 + nStackNum2);
                    KGLOG_PROCESS_ERROR(bRetCode);

                    g_pSO3World->m_ItemHouse.DestroyItem(pTemp2);
                    vecDecomposeResult[j] = NULL;
                }
            }
        }
    }

    // 检测背包空间是否足够
    for (size_t i = 0; i < vecDecomposeResult.size(); ++i)
    {
        if (vecDecomposeResult[i] == NULL)
            continue;

        ++nItemNum;
    }

    pPackage = m_PlayerPackage.GetPackage(eppiPlayerItemBox);
    KGLOG_PROCESS_ERROR(pPackage);

    nFreeRoomSize = pPackage->GetFreeRoomSize();
    if (nFreeRoomSize <= nItemNum)
    {
        g_PlayerServer.DoDownwardNotify(m_pPlayer, KMESSAGE_NOT_ENOUGH_BAG);
        goto Exit0;
    }

    // 给东西
    for (size_t i = 0; i < nItemCount; ++i)
    {
        bRetCode = DestroyItem(pItems[i].uPackageTypeEquipIn, pItems[i].uPackageIndexEquipIn, pItems[i].uPosEquipIn);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    for (size_t i = 0; i < vecDecomposeResult.size(); ++i)
    {
        if (vecDecomposeResult[i] == NULL)
            continue;

        bRetCode = AddPlayerItem(vecDecomposeResult[i]);
        assert(bRetCode);
    }

    g_PlayerServer.DoSyncEquipDecomposeResult(m_pPlayer->m_nConnIndex, vecDecomposeResult);

    vecDecomposeResult.clear();

	m_pPlayer->OnEvent(peDecomposeEquip);

	bResult = true;
Exit0:
    if (!bResult)
    {
        for (size_t i = 0; i < vecDecomposeResult.size(); ++i)
        {
            if (vecDecomposeResult[i])
            {
                g_pSO3World->m_ItemHouse.DestroyItem(vecDecomposeResult[i]);
            }
        }
        vecDecomposeResult.clear();
    }
	return bResult;
}

BOOL KItemList::DecomposeOneEquip(int nPackageType, int nPackageIndex, int nPos, std::vector<IItem*>& vecDecomposeResult)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    IItem*  pItem       = NULL;
    const KItemProperty* pItemProperty = NULL;
    KGEquipExtInfo*      pEquipInfo = NULL;
    IItem* pTemp = NULL;
    int    nCritMultiple = 1;
    KDecomposeQuality*  pDecomposeQuality = NULL;

    std::map<long long, int> mapDecomposeStrengthenEquip;
   
    pItem = GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(pItem);

    pItemProperty = pItem->GetProperty();
    KGLOG_PROCESS_ERROR(pItemProperty);
    KGLOG_PROCESS_ERROR(pItemProperty->pItemInfo && pItemProperty->pItemInfo->nGenre == igEquipment);

    pEquipInfo = pItemProperty->pItemInfo->pEquipExtInfo;
    KGLOG_PROCESS_ERROR(pEquipInfo);

    pDecomposeQuality = g_pSO3World->m_Settings.m_DecomposeQuality.GetByID(pItem->GetQuality());
    KGLOG_PROCESS_ERROR(pDecomposeQuality);

    if (g_Random(HUNDRED_NUM) < (unsigned)g_pSO3World->m_Settings.m_ConstList.nCECritRatePercent)
        nCritMultiple = g_pSO3World->m_Settings.m_ConstList.nCECritMultiple;

    // 装备分解道具
    if (pEquipInfo->nDecomposeTabType1 > 0)
    {
        pTemp = g_pSO3World->m_ItemHouse.CreateItem(pEquipInfo->nDecomposeTabType1, pEquipInfo->nDecomposeTabIndex1, 0, 0, 0);
        KGLOG_PROCESS_ERROR(pTemp);

        bRetCode = pTemp->CanStack();
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = pTemp->SetStackNum(pEquipInfo->nDecomposeStackNum1 * nCritMultiple * pDecomposeQuality->nMultiple);
        KGLOG_PROCESS_ERROR(bRetCode);

        vecDecomposeResult.push_back(pTemp);
        pTemp = NULL;
    }

    if ((pEquipInfo->nDecomposeTabType2 > 0) && (g_Random(HUNDRED_NUM) < (unsigned)pDecomposeQuality->nRatePercent))
    {
        pTemp = g_pSO3World->m_ItemHouse.CreateItem(pEquipInfo->nDecomposeTabType2, pEquipInfo->nDecomposeTabIndex2, 0, 0, 0);
        KGLOG_PROCESS_ERROR(pTemp);

        bRetCode = pTemp->CanStack();
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = pTemp->SetStackNum(pEquipInfo->nDecomposeStackNum2 * nCritMultiple);
        KGLOG_PROCESS_ERROR(bRetCode);

        vecDecomposeResult.push_back(pTemp);
        pTemp = NULL;
    }

    // 强化等级道具
    if (pItemProperty->nStrengthenLevel > 0)
    {
        KEQUIPSTRENGTHEN_LOGICDATA* pStrengthInfo = NULL;
        int nEquipLevel = 0;
        int nExtParam = 80 + g_Random(40);

        nEquipLevel = pItemProperty->pItemInfo->nLevel;
        KGLOG_PROCESS_ERROR(nEquipLevel >= 1 && nEquipLevel < cdMaxEquipLevel);

        for (int i = 0; i < pItemProperty->nStrengthenLevel; ++i)
        {
            pStrengthInfo = g_pSO3World->m_ItemHouse.GetStrengthLogicData(i);
            KGLOG_PROCESS_ERROR(pStrengthInfo);

            mapDecomposeStrengthenEquip[MAKE_INT64(pStrengthInfo->Materials[nEquipLevel - 1].dwItemType, pStrengthInfo->Materials[nEquipLevel - 1].dwItemIndex)] += pStrengthInfo->Materials[nEquipLevel - 1].nCount;
        }

        pStrengthInfo = g_pSO3World->m_ItemHouse.GetStrengthLogicData(pItemProperty->nStrengthenLevel - 1);
        KGLOG_PROCESS_ERROR(pStrengthInfo);

        for (std::map<long long, int>::iterator it = mapDecomposeStrengthenEquip.begin(); it != mapDecomposeStrengthenEquip.end(); ++it)
        {
            int nTabType = HIGH_DWORD_IN_UINT64(it->first);
            int nTabIndex = LOW_DWORD_IN_UINT64(it->first);

            int nStackNum = it->second * pStrengthInfo->nDecomposeRatePercent * nExtParam * nCritMultiple / (HUNDRED_NUM * HUNDRED_NUM);
            if (nStackNum < 1)
                continue;

            pTemp = g_pSO3World->m_ItemHouse.CreateItem(nTabType, nTabIndex, 0, 0, 0);
            KGLOG_PROCESS_ERROR(pTemp);

            bRetCode = pTemp->CanStack();
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = pTemp->SetStackNum(nStackNum);
            KGLOG_PROCESS_ERROR(bRetCode);

            vecDecomposeResult.push_back(pTemp);
            pTemp = NULL;
        }
    }

    // 幸运物品
    if (pItemProperty->nStrengthenLevel > 0)
    {
        std::vector<int> vecAllKey;
        KDecomposeEquip* pDecomposeEquip = NULL;
        g_pSO3World->m_Settings.m_DecomposeEquipLuckItem.GetAllKey(vecAllKey);

        for (size_t i = 0; i < vecAllKey.size(); ++i)
        {
            pDecomposeEquip = g_pSO3World->m_Settings.m_DecomposeEquipLuckItem.GetByID(vecAllKey[i]);
            KGLOG_PROCESS_ERROR(pDecomposeEquip);

            if (pItemProperty->nStrengthenLevel < pDecomposeEquip->nLowLevel)
                continue;

            if (pDecomposeEquip->nTabType == 0)
                continue;

            if (g_Random(HUNDRED_NUM) > (unsigned)pDecomposeEquip->nRatePercent)
                continue;

            pTemp = g_pSO3World->m_ItemHouse.CreateItem(pDecomposeEquip->nTabType, pDecomposeEquip->nTabIndex, 0, 0, 0);
            KGLOG_PROCESS_ERROR(pTemp);

            bRetCode = pTemp->CanStack();
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = pTemp->SetStackNum(pDecomposeEquip->nCount * nCritMultiple);
            KGLOG_PROCESS_ERROR(bRetCode);

            vecDecomposeResult.push_back(pTemp);
            pTemp = NULL;
        }
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (pTemp)
        {
            g_pSO3World->m_ItemHouse.DestroyItem(pTemp);
            pTemp = NULL;
        }
    }
    return bResult;
}

BOOL KItemList::IsValidPos(int nPackageType, int nPackageIndex, int nPos)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KPackage* pPackage = NULL;

    KGLOG_PROCESS_ERROR(nPackageType == ePlayerPackage || nPackageType == eHeroPackage);
    if (nPackageType == ePlayerPackage)
    {
        KGLOG_PROCESS_ERROR(nPackageIndex == eppiPlayerShotcutBar || nPackageIndex == eppiPlayerItemBox);

        KPackage* pPackage = GetPlayerPackage(nPackageIndex);
        KGLOG_PROCESS_ERROR(pPackage);
        KGLOG_PROCESS_ERROR(nPos >= 0 && nPos < pPackage->GetSize());
    }
    else
    {
        KHeroPackage* pPackage = GetHeroPackage(nPackageIndex);
        KGLOG_PROCESS_ERROR(pPackage);
        KGLOG_PROCESS_ERROR(nPos >= 0 && nPos < pPackage->GetSize());
    }
	
	bResult = true;
Exit0:
	return bResult;
}

int KItemList::GetAllHeroEquipScore()
{
	int nScore = 0;
	for (KMAP_HERO_PACKAGE::iterator it = m_mapHeroPackage.begin(); it != m_mapHeroPackage.end(); ++it)
	{
		nScore += it->second.GetAllEquipScore();
	}
	return nScore;
}

int KItemList::GetHeroEquipScore(DWORD dwHeroTemplateID)
{
	int nScore = 0;
	KHeroPackage* pPkg = NULL;

	pPkg = GetHeroPackage(dwHeroTemplateID);
	KGLOG_PROCESS_ERROR(pPkg);

	nScore = pPkg->GetAllEquipScore();
Exit0:
	return nScore;
}

int KItemList::GetMaxStrengthenLevel()
{
	int nMaxStrengthenLevel = 0;
	for (KMAP_HERO_PACKAGE::iterator it = m_mapHeroPackage.begin(); it != m_mapHeroPackage.end(); ++it)
	{
		int nLevel = it->second.GetMaxStrengthenLevel();
		if (nLevel > nMaxStrengthenLevel)
			nMaxStrengthenLevel = nLevel;
	}
	return nMaxStrengthenLevel;
}
