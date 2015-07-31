#include "stdafx.h"
#include "KHeroPackage.h"
#include "KSO3World.h"
#include "KPlayer.h"

KHeroPackage::KHeroPackage()
{
}

KHeroPackage::~KHeroPackage()
{
}

BOOL KHeroPackage::Init(KPlayer* pPlayer, int nPackageIndex)
{
	BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = m_Package.Init(eipTotal, eipTotal, pPlayer, eHeroPackage, nPackageIndex);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KHeroPackage::UnInit()
{
    m_Package.UnInit();
}

BOOL KHeroPackage::LoadFromProtoBuf(const T3DB::KPBHeroPackage* pHeroPackage)
{
    BOOL    bResult 	= false;
    BOOL    bRetCode 	= false;

    assert(pHeroPackage);

    const T3DB::KPBPackage& Package = pHeroPackage->package();
    bRetCode = m_Package.LoadFromProtoBuf(&Package);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroPackage::SaveToProtoBuf(T3DB::KPBHeroPackage* pHeroPackage)
{
    T3DB::KPBPackage* pPackage = NULL;

    assert(pHeroPackage);

    pPackage = pHeroPackage->mutable_package();
    return m_Package.SaveToProtoBuf(pPackage);
}

IItem*  KHeroPackage::GetItem(int nPos)
{
    return m_Package.GetItem(nPos);
}

BOOL KHeroPackage::CanPlaceItem(IItem* piItem, int nPos)
{
	BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IItem*                  pExistItem      = NULL;

    assert(piItem);

    pExistItem = GetItem(nPos);
    KG_PROCESS_ERROR(pExistItem == NULL);

    bRetCode = CheckItemType(piItem, nPos);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroPackage::CheckItemType(IItem* piItem, int nPos)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    const KItemProperty*    pcItemProperty  = NULL;
    int                     nItemSubType    = 0;
    KGItemInfo*             pItemInfo       = NULL;

    assert(piItem);

    pcItemProperty = piItem->GetProperty();
    KGLOG_PROCESS_ERROR(pcItemProperty);

    pItemInfo = pcItemProperty->pItemInfo;
    KGLOG_PROCESS_ERROR(pItemInfo);

    switch (nPos)
    {
    case eipHeadWear:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espHeadWear);
        break;
    case eipBangle:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espBangle);
        break;
    case eipChest:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espChest);
        break;
    case eipPants:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espPants);
        break;
    case eipWaist:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espWaist);
        break;
    case eipBoots:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espBoots);
        break;
    case eipAmulet:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espAmulet);
        break;
    case eipRing:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espRing);
        break;
    case eipKneepad:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espKneepad);
        break;
    case eipAidEquip:
        KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);
        KG_PROCESS_ERROR(pItemInfo->nSub == espAidEquip);
        break;

    default:
        goto Exit0;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroPackage::PlaceItem(IItem* piItem, int nPos)
{
	BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = CanPlaceItem(piItem, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Package.PlaceItem(piItem, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);
    
    bResult = true;
Exit0:
    return bResult;
}

IItem*	KHeroPackage::PickUpItem(int nPos)
{
    return m_Package.PickUpItem(nPos);
}

int KHeroPackage::GetItemPlacePos(IItem* piItem)
{
    int                     nPos            = -1;
    const KItemProperty*    pcItemProperty  = NULL;
    KGItemInfo*             pItemInfo       = NULL;

    assert(piItem);

    pcItemProperty = piItem->GetProperty();
    KGLOG_PROCESS_ERROR(pcItemProperty);

    pItemInfo = pcItemProperty->pItemInfo;
    KGLOG_PROCESS_ERROR(pItemInfo);

    KG_PROCESS_ERROR(pItemInfo->nGenre == igEquipment);

    switch (pItemInfo->nSub)
    {
    case espHeadWear:
        nPos = eipHeadWear;
        break;
    case espBangle:
        nPos = eipBangle;
        break;
    case espChest:
        nPos = eipChest;
        break;
    case espPants:
        nPos = eipPants;
        break;
    case espWaist:
        nPos = eipWaist;
        break;
    case espBoots:
        nPos = eipBoots;
        break;
    case espAmulet:
        nPos = eipAmulet;
        break;
    case espRing:
        nPos = eipRing;
        break;
    case espKneepad:
        nPos = eipKneepad;
        break;
    case espAidEquip:
        nPos = eipAidEquip;
        break;
    default:
        goto Exit0;
    }

Exit0:
    return nPos;
}

BOOL KHeroPackage::CostItem(DWORD dwTabType, DWORD dwIndex, int nAmount)
{
    return m_Package.CostItem(dwTabType, dwIndex, nAmount);
}

BOOL KHeroPackage::CostItem(int nPos, int nCount)
{
    return m_Package.CostItem(nPos, nCount);
}

BOOL KHeroPackage::ApplyEquip(KHero* pHero)
{
	BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    const KItemProperty* pcItemProperty = NULL;

    for (int i = eipEquipBegin; i < eipTotal; ++i)
    {
        IItem* pItem = GetItem(i);
        if (pItem == NULL || pItem->GetCurrentDurability() == 0)
            continue;
        
        pcItemProperty = pItem->GetProperty();
        KGLOG_PROCESS_ERROR(pcItemProperty);

        pHero->ApplyAttribute(pcItemProperty->pBaseAttr);
        pHero->ApplyAttribute(pcItemProperty->pExtAttr);
    }    

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroPackage::UnApplyEquip(KHero* pHero)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    const KItemProperty* pcItemProperty = NULL;

    for (int i = eipEquipBegin; i < eipTotal; ++i)
    {
        IItem* pItem = GetItem(i);
        if (pItem == NULL)
            continue;
        
        pcItemProperty = pItem->GetProperty();
        KGLOG_PROCESS_ERROR(pcItemProperty);

        pHero->UnApplyAttribute(pcItemProperty->pBaseAttr);
        pHero->UnApplyAttribute(pcItemProperty->pExtAttr);
    }    

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroPackage::AbradeEquip()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = m_Package.AbradeItemDurability();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroPackage::RepairAllEquip()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = m_Package.RepairAllEquip();
    KGLOG_PROCESS_ERROR(bRetCode);
    
    bResult = true;
Exit0:
    return bResult;
}

int KHeroPackage::GetRepairPrice()
{
    int nResult = ERROR_ID;

    nResult = m_Package.GetRepairPrice();
    KGLOG_PROCESS_ERROR(nResult >= 0);

Exit0:
    return nResult;
}

BOOL KHeroPackage::CheckItemValidity()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = m_Package.CheckItemValidity();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroPackage::GetBinaryDataForClient(uint16_t* pItemLen, BYTE* pbyData, size_t uMaxBufferSize)
{
    BOOL 					bResult    		= false;
    BOOL 					bRetCode   		= false;
    size_t 					uItemSize 		= 0;
    const KItemProperty* 	pcItemProperty 	= NULL;

    for (int i = eipEquipBegin; i < eipTotal; ++i)
    {
        pItemLen[i] = 0;
        IItem* pItem = GetItem(i);
        if (pItem == NULL)
            continue;

        bRetCode = pItem->GetBinaryDataForClient(&uItemSize, pbyData, uMaxBufferSize);
        KGLOG_PROCESS_ERROR(bRetCode);
        
        pItemLen[i] = (uint16_t)uItemSize;
		
        uMaxBufferSize -= uItemSize;
		pbyData += uItemSize;
    }   
	
    bResult = true;
Exit0:
    return bResult;
}
