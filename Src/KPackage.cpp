#include "stdafx.h"
#include "KPackage.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KPlayer.h"

BOOL KPackage::Init(int nSize, int nMaxSize, KPlayer* pPlayer, int nPackageType, int nPackageIndex)
{
	BOOL bResult = false;

    assert(pPlayer);
    KGLOG_PROCESS_ERROR(nSize > 0 && nMaxSize > 0);
	KGLOG_PROCESS_ERROR(nSize <= nMaxSize);
    KGLOG_PROCESS_ERROR(nPackageType == ePlayerPackage || nPackageType == eHeroPackage);

	m_nSize         = nSize;
    m_nMaxSize      = nMaxSize;
    m_pPlayer       = pPlayer;
    m_nPackageType  = nPackageType;
    m_nPackageIndex = nPackageIndex;

    m_ppiItemArray = (IItem**)KMEMORY_ALLOC(sizeof(IItem*) * nMaxSize);
    assert(m_ppiItemArray);

	ZeroMemory(m_ppiItemArray, sizeof(m_ppiItemArray) * nMaxSize);

	bResult = true;
Exit0:
	return bResult;
}

void KPackage::UnInit()
{
    for (int i = 0; i < m_nSize; i++)
    {
        if (m_ppiItemArray[i])
        {
            g_pSO3World->m_ItemHouse.DestroyItem(m_ppiItemArray[i]);
            m_ppiItemArray[i] = NULL;
        }
    }

    KMEMORY_FREE(m_ppiItemArray);
}

BOOL KPackage::LoadFromProtoBuf(const T3DB::KPBPackage* pPackage)
{
    BOOL        			bResult     	= false;
    BOOL        			bRetCode    	= false;
    IItem*      			piItem      	= NULL;

    assert(pPackage);

    for (int nIndex = 0; nIndex < pPackage->equidata_size(); nIndex++)
    {
        int nX = 0;
        const T3DB::KPB_EQUI_DATA& EquiData = pPackage->equidata(nIndex);

        nX = EquiData.pos();

        if (piItem)
        {
            g_pSO3World->m_ItemHouse.DestroyItem(piItem);
            piItem = NULL;
        }

        piItem = g_pSO3World->m_ItemHouse.CreateItemFromProtoBuf(&EquiData);
        if (piItem == NULL)
        {
            KGLogPrintf(KGLOG_ERR, "[ITEM] GenerateItemFromBinaryData failed. \n");
            continue;
        }

        bRetCode = PlaceItem(piItem, nX);
        if (!bRetCode)
            continue;

        if (m_pPlayer->m_eConnectType == eLogin)
        {
            g_PlayerServer.DoSyncItemData(
                m_pPlayer->m_nConnIndex, m_pPlayer->m_dwID, piItem, m_nPackageType, m_nPackageIndex, nX
                );
        }

        piItem = NULL;
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_ERR, "[ITEM] Load item failed. \n");

    if (piItem)
    {
        g_pSO3World->m_ItemHouse.DestroyItem(piItem);
        piItem = NULL;
    }
    return bResult;
}

BOOL KPackage::SaveToProtoBuf(T3DB::KPBPackage* pPackage)
{
    BOOL    bResult	                    = false;
    BOOL    bRetCode                    = false;
   
    WORD*   pwItemCount                 = NULL;

    KGLOG_PROCESS_ERROR(pPackage);

    for (int i = 0; i < m_nSize; ++i)
    {
        IItem*              piItem              = m_ppiItemArray[i];
        if (piItem == NULL)
            continue;
        T3DB::KPB_EQUI_DATA* pEquiData = pPackage->add_equidata();
        pEquiData->set_pos(i);
        bRetCode = piItem->GetProtoBufData(pEquiData);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPackage::SetSize(int nNewSize)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;

    KGLOG_PROCESS_ERROR(nNewSize > 0 && nNewSize <= m_nMaxSize);

    m_nSize = nNewSize;

	bResult = true;
Exit0:
	return bResult;
}

IItem* KPackage::GetItem(int nPos)
{
	IItem* piItem = NULL;

	assert(m_nSize <= m_nMaxSize);

    KGLOG_PROCESS_ERROR(nPos >= 0);
	KGLOG_PROCESS_ERROR(nPos < m_nSize);

	piItem = m_ppiItemArray[nPos];
Exit0:
	return piItem;
}

BOOL KPackage::PlaceItem(IItem* piItem, int nPos)
{
	BOOL bResult = false;

	assert(m_nSize <= m_nMaxSize);

	KGLOG_PROCESS_ERROR(piItem);
    KGLOG_PROCESS_ERROR(nPos >= 0);
	KGLOG_PROCESS_ERROR(nPos < m_nSize);
	KGLOG_PROCESS_ERROR(m_ppiItemArray[nPos] == NULL);
    
	m_ppiItemArray[nPos] = piItem;

	bResult = true;
Exit0:
	return bResult;
}

// 清除装备，返回被清的装备指针
IItem* KPackage::PickUpItem(int nPos)
{
	IItem* piItem = NULL;

	assert(m_nSize <= m_nMaxSize);
    KGLOG_PROCESS_ERROR(nPos >= 0);
	KGLOG_PROCESS_ERROR(nPos < m_nSize);

	piItem = m_ppiItemArray[nPos];
	m_ppiItemArray[nPos] = NULL;

Exit0:
	return piItem;
}

int KPackage::GetFreePos()
{
    for (int i = 0; i < (int)m_nSize; i++)
    {
        if (m_ppiItemArray[i] == NULL)
            return i;
    }
    return -1;
}

int KPackage::GetCanStackPos(IItem* piSrcItem)
{
    int                     nResult             = -1;
    BOOL                    bRetCode            = false;
    int                     nSrcItemStack       = 0;
    const KItemProperty*    pSrcItemProperty    = NULL;
    int                     nMaxStackNum        = 0;
    DWORD                   dwShotcutBarMaxStack= 0;

    assert(piSrcItem);

    pSrcItemProperty = piSrcItem->GetProperty();
    KGLOG_PROCESS_ERROR(pSrcItemProperty);

    nSrcItemStack = piSrcItem->GetStackNum();

    bRetCode = piSrcItem->CanStack();
    KG_PROCESS_ERROR(bRetCode);

    for (int nX = 0; nX < m_nSize; ++nX)
    {
        IItem*                  piItem          = m_ppiItemArray[nX];
        const KItemProperty*    pItemProperty   = NULL;
        int                     nItemStack      = 0;

        if (piItem == NULL)
            continue;

        pItemProperty = piItem->GetProperty();
        KGLOG_PROCESS_ERROR(pItemProperty);

        bRetCode = piItem->CanStackItem(piSrcItem);
        if (!bRetCode)
            continue;
            
        nMaxStackNum = piSrcItem->GetMaxStackNum();
        nItemStack = piItem->GetStackNum();

        if (m_nPackageIndex == eppiPlayerShotcutBar)
        {
            KGLOG_PROCESS_ERROR(pSrcItemProperty->pItemInfo);
            dwShotcutBarMaxStack = pSrcItemProperty->pItemInfo->dwShotcutBarMaxStackNum;
            KG_PROCESS_ERROR(dwShotcutBarMaxStack > 0);

            KGLOG_PROCESS_ERROR(dwShotcutBarMaxStack <= (DWORD)nMaxStackNum);
            KG_PROCESS_ERROR(nItemStack + nSrcItemStack <= (int)dwShotcutBarMaxStack);

            nResult = nX;
            break;
        }
        else if (nItemStack + nSrcItemStack <= nMaxStackNum)
        {
            nResult = nX;
            break;
        }
    }
    
Exit0:
    return nResult;
}

int KPackage::CostItem(DWORD dwTabType, DWORD dwIndex, int nAmount)
{
    BOOL bResult     = false;
    BOOL bRetCode    = false;
    int  nItemAmount = 0;
    int  nGenre = 0;
    DWORD dwID = ERROR_ID;

    KGLOG_PROCESS_ERROR(nAmount >= 0);
    KG_PROCESS_SUCCESS(nAmount == 0);

    nItemAmount = GetItemCount(dwTabType, dwIndex);
    KGLOG_PROCESS_ERROR(nItemAmount >= nAmount);

    for (int nX = 0; nX < m_nSize; ++nX)
    {
        IItem*					piItem			= m_ppiItemArray[nX];
        const KItemProperty*	pItemProperty	= NULL;
        KGItemInfo*				pItemInfo		= NULL;

        if (piItem == NULL)
            continue;

        pItemProperty = piItem->GetProperty();
        pItemInfo = pItemProperty->pItemInfo;
        KGLOG_PROCESS_ERROR(pItemInfo);

        if (pItemProperty->dwTabType != dwTabType || pItemProperty->dwTabIndex != dwIndex)
            continue;

        nGenre  = pItemInfo->nGenre;
        dwID = pItemInfo->dwID;

        if (piItem->CanStack())
        {
            if (pItemProperty->nStackNum > nAmount)
            {
                piItem->SetStackNum(pItemProperty->nStackNum - nAmount);
                
				g_PlayerServer.DoUpdateItemAmount(m_pPlayer->m_nConnIndex, m_nPackageType, m_nPackageIndex, nX, piItem->GetStackNum());

                nAmount = 0;
                goto Exit1;
            }

            if (pItemProperty->nStackNum == nAmount)
            {
                piItem = PickUpItem(nX);
                KGLOG_PROCESS_ERROR(piItem);

                g_pSO3World->m_ItemHouse.DestroyItem(piItem);
				g_PlayerServer.DoDestroyItem(m_pPlayer->m_nConnIndex, m_nPackageType, m_nPackageIndex, nX);

                piItem = NULL;
                nAmount = 0;
                goto Exit1;
            }

            piItem = PickUpItem(nX);
            KGLOG_PROCESS_ERROR(piItem);

            nAmount -= pItemProperty->nStackNum;
            g_pSO3World->m_ItemHouse.DestroyItem(piItem);
            
            g_PlayerServer.DoDestroyItem(m_pPlayer->m_nConnIndex, m_nPackageType, m_nPackageIndex, nX);

            piItem = NULL;
        }
        else
        {
            piItem = PickUpItem(nX);
            KGLOG_PROCESS_ERROR(piItem);

            g_pSO3World->m_ItemHouse.DestroyItem(piItem);
			g_PlayerServer.DoDestroyItem(m_pPlayer->m_nConnIndex, m_nPackageType, m_nPackageIndex, nX);
        	
            piItem = NULL;
            --nAmount;
            if (nAmount == 0)
                goto Exit1;
        }
    }

Exit1:
    m_pPlayer->OnEvent(peConsumeItem, nGenre, dwID, nAmount);
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPackage::CostItem(int nPos, int nCount)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IItem* piItem = NULL;
    const KItemProperty* pItemProperty = NULL;
    KGItemInfo* pItemInfo = NULL;

    KGLOG_PROCESS_ERROR(m_nSize <= m_nMaxSize);

    KGLOG_PROCESS_ERROR(nPos >= 0);
    KGLOG_PROCESS_ERROR(nPos < m_nSize);
    KGLOG_PROCESS_ERROR(nCount > 0);

    piItem = m_ppiItemArray[nPos];
    KGLOG_PROCESS_ERROR(piItem);

    pItemProperty = piItem->GetProperty();
    KGLOG_PROCESS_ERROR(pItemProperty);

    pItemInfo = pItemProperty->pItemInfo;
    KGLOG_PROCESS_ERROR(pItemInfo);

    if (piItem->CanStack())
    {
        int nStackNum = piItem->GetStackNum();
        KGLOG_PROCESS_ERROR(nStackNum >= nCount);

        if (nStackNum > nCount)
        {
            bRetCode = piItem->SetStackNum(nStackNum - nCount);
            KGLOG_PROCESS_ERROR(bRetCode);

            g_PlayerServer.DoUpdateItemAmount(m_pPlayer->m_nConnIndex, m_nPackageType, m_nPackageIndex, nPos, piItem->GetStackNum());
            goto Exit1;
        }
        
        bRetCode = DestroyItem(nPos);
        KGLOG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    KGLOG_PROCESS_ERROR(nCount == 1);

    bRetCode = DestroyItem(nPos);
    KGLOG_PROCESS_ERROR(bRetCode);
    
Exit1:
    m_pPlayer->OnEvent(peConsumeItem, pItemInfo->nGenre, pItemInfo->dwID, nCount);
    bResult = true;
Exit0:
    return bResult;
}

int KPackage::GetItemCount(DWORD dwTabType, DWORD dwIndex)
{
    int nResult = 0;

    for (int nX = 0; nX < m_nSize; nX++)
    {
        IItem* piItem = m_ppiItemArray[nX];
        const KItemProperty* pItemProperty = NULL;
        KGItemInfo*	pItemInfo = NULL;

        if (piItem == NULL)
            continue;

        pItemProperty = piItem->GetProperty();
        pItemInfo = pItemProperty->pItemInfo;
        if (pItemProperty->dwTabType != dwTabType || pItemProperty->dwTabIndex != dwIndex)
            continue;

        if (piItem->CanStack())
            nResult += piItem->GetStackNum();
        else
            ++nResult;
    }
    
    return nResult;
}

int KPackage::GetFreeRoomSize()
{
    int nFreeRoomSize = 0;

    for (int i = 0; i < m_nSize; ++i)
    {
        if (m_ppiItemArray[i] == NULL)
            ++nFreeRoomSize;
    }
    return nFreeRoomSize;
}

BOOL KPackage::AbradeItemDurability()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    for (int nX = 0; nX < m_nSize; ++nX)
    {
        IItem* piItem = m_ppiItemArray[nX];
        if (piItem == NULL)
            continue;
			
        bRetCode = piItem->CanAbrade();
        if (!bRetCode)
            continue; 

        bRetCode = piItem->Abrade();
        KGLOG_PROCESS_ERROR(bRetCode);       

        g_PlayerServer.DoUpdateItemDurability(
            m_pPlayer->m_nConnIndex, m_nPackageType, m_nPackageIndex, nX, piItem->GetCurrentDurability()
        );        
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPackage::RepairAllEquip()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KGLOG_PROCESS_ERROR(m_nPackageType == eHeroPackage);

    for (int nX = 0; nX < m_nSize; ++nX)
    {
        IItem* piItem = m_ppiItemArray[nX];
        if (piItem == NULL || !piItem->CanRepair())
            continue;
        
        bRetCode = piItem->Repair();
        KGLOG_PROCESS_ERROR(bRetCode); 
		
        int                     nCurrentDurability  = ERROR_ID;
        const KItemProperty*    pItemProperty       = NULL;

        pItemProperty = piItem->GetProperty();
        KGLOG_PROCESS_ERROR(pItemProperty);

        nCurrentDurability = piItem->GetCurrentDurability();
        KGLOG_PROCESS_ERROR(nCurrentDurability == pItemProperty->pItemInfo->nMaxDurability);
        
	    bRetCode = g_PlayerServer.DoUpdateItemDurability(m_pPlayer->m_nConnIndex, m_nPackageType, m_nPackageIndex, nX, piItem->GetCurrentDurability());        
        KGLOG_PROCESS_ERROR(bRetCode);
    }    

    bResult = true;
Exit0:
    return bResult;
}

int KPackage::GetRepairPrice()
{
    int     nResult                 = ERROR_ID;
    int     nMaxDurability          = ERROR_ID;
    int     nCurrentDurability      = ERROR_ID;
    int     nValuePoint             = ERROR_ID;
    int     nQuality                = ERROR_ID;
    int     nPerEquipPrice          = ERROR_ID;
    const KItemProperty* pItemProperty = NULL;

    for (int nX = 0; nX < m_nSize; ++nX)
    {
        IItem* piItem = m_ppiItemArray[nX];
        if (piItem == NULL || !piItem->CanRepair())
            continue;
        
        pItemProperty = piItem->GetProperty();
        KGLOG_PROCESS_ERROR(pItemProperty);

        nMaxDurability      = pItemProperty->pItemInfo->nMaxDurability;
        nCurrentDurability  = pItemProperty->nCurrentDurability;
        nValuePoint         = pItemProperty->nValuePoint;
        nQuality            = pItemProperty->nQuality;

        nPerEquipPrice = g_pSO3World->m_ShopMgr.GetRepairSinglePrice(nCurrentDurability, nMaxDurability, nQuality, nValuePoint);
        KGLOG_PROCESS_ERROR(nPerEquipPrice >= 0);

        nResult += nPerEquipPrice;
    }

Exit0:
    return nResult;
}

BOOL KPackage::CheckItemValidity()
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    const KItemProperty*    pItemProperty   = NULL;
    KGItemInfo*             pItemInfo       = NULL;
    int                     nOverdueTime    = -1;
    int                     nPos            = 0;

    for (nPos = 0; nPos < m_nSize; ++nPos)
    {
        IItem* piItem = m_ppiItemArray[nPos];
        if (piItem == NULL)
            continue;
        
        pItemProperty = piItem->GetProperty();
        KGLOG_PROCESS_ERROR(pItemProperty);

        pItemInfo = pItemProperty->pItemInfo;
		KGLOG_PROCESS_ERROR(pItemInfo);
        
		if (pItemInfo->tOverdueTime == 0 && pItemInfo->tOverduePoint == 0)
            continue;       

        if (pItemInfo->tOverdueTime > 0)
        {
            nOverdueTime = (int)pItemInfo->tOverdueTime;
        
            if (g_pSO3World->m_nCurrentTime > pItemProperty->nGenTime + nOverdueTime)
            {
                bRetCode = m_pPlayer->m_ItemList.DestroyItem(m_nPackageType, m_nPackageIndex, nPos);
                KGLOG_PROCESS_ERROR(bRetCode);
            }
        }
        else
        {
            if (g_pSO3World->m_nCurrentTime > pItemInfo->tOverduePoint)
            {
                bRetCode = m_pPlayer->m_ItemList.DestroyItem(m_nPackageType, m_nPackageIndex, nPos);
                KGLOG_PROCESS_ERROR(bRetCode);
            }            
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

int KPackage::GetAllEquipScore()
{
    int                     nTotalValuePoint = 0;
    BOOL                    bRetCode        = false;
    const KItemProperty*    pItemProperty   = NULL;
    KGItemInfo*             pItemInfo       = NULL;
    int                     nOverdueTime    = -1;
    int                     nPos            = 0;

    for (nPos = 0; nPos < m_nSize; ++nPos)
    {
        IItem* piItem = m_ppiItemArray[nPos];
        if (piItem == NULL)
            continue;

        pItemProperty = piItem->GetProperty();
        KGLOG_PROCESS_ERROR(pItemProperty);

        pItemInfo = pItemProperty->pItemInfo;
        KGLOG_PROCESS_ERROR(pItemInfo);

        if (pItemInfo->nGenre != igEquipment)
            continue;        

        nTotalValuePoint += piItem->GetScore();
    }

Exit0:
    return nTotalValuePoint;
}

BOOL KPackage::DestroyItem(int nPos)
{
    BOOL bResult = false;
    IItem* piItem = NULL;

    KGLOG_PROCESS_ERROR(nPos >= 0);
    KGLOG_PROCESS_ERROR(nPos < m_nSize);

    piItem = m_ppiItemArray[nPos];
    m_ppiItemArray[nPos] = NULL;

    g_pSO3World->m_ItemHouse.DestroyItem(piItem);
    piItem = NULL;

    g_PlayerServer.DoDestroyItem(m_pPlayer->m_nConnIndex, m_nPackageType, m_nPackageIndex, nPos);

    bResult = true;
Exit0:
    return bResult;
}

int KPackage::GetMaxStrengthenLevel()
{
	int                     nMaxStrengthenLevel = 0;
	BOOL                    bRetCode        = false;
	const KItemProperty*    pItemProperty   = NULL;
	KGItemInfo*             pItemInfo       = NULL;
	int                     nOverdueTime    = -1;
	int                     nPos            = 0;

	for (nPos = 0; nPos < m_nSize; ++nPos)
	{
		IItem* piItem = m_ppiItemArray[nPos];
		if (piItem == NULL)
			continue;

		pItemProperty = piItem->GetProperty();
		KGLOG_PROCESS_ERROR(pItemProperty);

		pItemInfo = pItemProperty->pItemInfo;
		KGLOG_PROCESS_ERROR(pItemInfo);

		if (pItemInfo->nGenre != igEquipment)
			continue;   
			     
		if (pItemProperty->nStrengthenLevel > nMaxStrengthenLevel)
			nMaxStrengthenLevel = pItemProperty->nStrengthenLevel;
	}

Exit0:
	return nMaxStrengthenLevel;
}
