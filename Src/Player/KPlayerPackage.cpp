#include "stdafx.h"
#include "KPlayerPackage.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KRoleDBDataDef.h"
#include "KPlayer.h"

KPlayerPackage::KPlayerPackage()
{
    m_pPlayer = NULL;
    memset(m_ReloadInfo, 0, sizeof(m_ReloadInfo));
}

KPlayerPackage::~KPlayerPackage()
{
}

BOOL KPlayerPackage::Init(KPlayer* pPlayer)
{
	BOOL bResult    = false;
	BOOL bRetCode   = false;

    m_pPlayer = pPlayer;

    bRetCode = m_Package[eppiPlayerItemBox].Init(cdPlayerPageSize * cdInitPageCount, cdPlayerPageSize * cdMaxPageCount, pPlayer, ePlayerPackage, eppiPlayerItemBox);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Package[eppiPlayerShotcutBar].Init(cdShotcutSize, cdShotcutSize, pPlayer, ePlayerPackage, eppiPlayerShotcutBar);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KPlayerPackage::UnInit()
{
    for (int i = eppiBegin; i < eppiTotal; ++i)
    {
        m_Package[i].UnInit();
    }
}

BOOL KPlayerPackage::LoadFromProtoBuf(const T3DB::KPBPlayerPackage* pPlayerPackage)
{
    BOOL    bResult 	    = false;
    BOOL    bRetCode 	    = false;

    KGLOG_PROCESS_ERROR(pPlayerPackage);

    for (int i = eppiBegin; i < eppiTotal; ++i)
    {
        const T3DB::KPBPackage& Package = pPlayerPackage->package(i);

        bRetCode = m_Package[i].SetSize(Package.size());
        KGLOG_PROCESS_ERROR(bRetCode);

        if (m_pPlayer->m_eConnectType == eLogin)
        {
            g_PlayerServer.DoSyncPlayerPackageSize(m_pPlayer->m_nConnIndex, i, Package.size());
        }

        bRetCode = m_Package[i].LoadFromProtoBuf(&Package);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerPackage::SaveToProtoBuf(T3DB::KPBPlayerPackage* pPlayerPackage)
{
    BOOL    bResult 	    = false;
    BOOL    bRetCode 	    = false;

    KGLOG_PROCESS_ERROR(pPlayerPackage);

    for (int i = eppiBegin; i < eppiTotal; ++i)
    {
        T3DB::KPBPackage* pPackage = pPlayerPackage->add_package();

        pPackage->set_size(m_Package[i].GetSize());
        bRetCode = m_Package[i].SaveToProtoBuf(pPackage);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

IItem* KPlayerPackage::GetItem(int nPackageIndex, int nPos)
{
    IItem* pResult = NULL;
    IItem* pItem   = NULL;

    KGLOG_PROCESS_ERROR(nPackageIndex >= eppiBegin && nPackageIndex < eppiTotal);
    
    pItem = m_Package[nPackageIndex].GetItem(nPos);
    KG_PROCESS_ERROR(pItem);

    pResult = pItem;
Exit0:
    return pResult;
}

IItem* KPlayerPackage::PickUpItem(int nPackageIndex, int nPos)
{
    IItem* pResult = NULL;
    IItem* pItem   = NULL;

    KGLOG_PROCESS_ERROR(nPackageIndex >= eppiBegin && nPackageIndex < eppiTotal);
    
    pItem = m_Package[nPackageIndex].PickUpItem(nPos);
    KG_PROCESS_ERROR(pItem);

    pResult = pItem;
Exit0:
    return pResult;
}

BOOL KPlayerPackage::PlaceItem(IItem* piItem, int nPackageIndex, int nPos)
{
    BOOL                    bResult                 = false;
    BOOL                    bRetCode                = false;    

    KGLOG_PROCESS_ERROR(nPackageIndex >= eppiBegin && nPackageIndex < eppiTotal);    

    bRetCode = m_Package[nPackageIndex].PlaceItem(piItem, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:    
    return bResult;
}

BOOL KPlayerPackage::CanAddItems(IItem** piItems, const int nItemNum)
{
    BOOL    bResult 	= false;
    int     nCount		= 0;

    for (int i = 0; i < nItemNum; ++i)
    {
        if (piItems[i])
        {
            ++nCount;
        }
    }
    KG_PROCESS_ERROR(nCount <= m_Package[eppiPlayerItemBox].GetFreeRoomSize());

    bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerPackage::CanAddItems(DWORD dwTabType[], int nTabIndex[], const int nItemNum)
{
    BOOL	bResult 	= false;
	int		nCount		= 0;

	for (int i = 0; i < nItemNum; ++i)
	{
		if (dwTabType[i] && nTabIndex[i])
		{
			++nCount;
		}
	}

    KG_PROCESS_ERROR(nCount <= m_Package[eppiPlayerItemBox].GetFreeRoomSize());

    bResult = true; 
Exit0:
    return bResult;
}

BOOL KPlayerPackage::CanAddItem(IItem* piItem)
{
    return HasFreePos(eppiPlayerItemBox);
}

BOOL KPlayerPackage::AddItem(IItem* piItem)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    int  nIndex     = -1;
    int  nPos       = -1;

    KGLOG_PROCESS_ERROR(piItem);

    nIndex = eppiPlayerShotcutBar;
    nPos = m_Package[eppiPlayerShotcutBar].GetCanStackPos(piItem);
    if (nPos == -1)
    {
        nIndex = eppiPlayerItemBox;
        nPos = m_Package[nIndex].GetCanStackPos(piItem);
    }   

    if (nPos != -1)
    {
        IItem* pDestItem = m_Package[nIndex].GetItem(nPos);
        int nSrcStackNum = 0;
        int nDestStackNum = 0;

        KGLOG_PROCESS_ERROR(pDestItem);
        nSrcStackNum = piItem->GetStackNum();
        nDestStackNum = pDestItem->GetStackNum();

        bRetCode = pDestItem->SetStackNum(nSrcStackNum + nDestStackNum);
        KGLOG_PROCESS_ERROR(bRetCode);
        
        g_PlayerServer.DoUpdateItemAmount(
            m_pPlayer->m_nConnIndex, ePlayerPackage, nIndex, nPos, nSrcStackNum + nDestStackNum
        );

        g_pSO3World->m_ItemHouse.DestroyItem(piItem);
        piItem = NULL;

        goto Exit1;
    }

    nPos = m_Package[nIndex].GetFreePos();
    KGLOG_PROCESS_ERROR(nPos != -1);

    bRetCode = m_Package[nIndex].PlaceItem(piItem, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoSyncItemData(
        m_pPlayer->m_nConnIndex, m_pPlayer->m_dwID, piItem, ePlayerPackage, nIndex, nPos
    );

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerPackage::CanPutInPackage(IItem* pItem, int nPackageIndex)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    const KItemProperty* pItemProperty = NULL;
    int nGenre = 0;

    assert(pItem);

    pItemProperty = pItem->GetProperty();
    KGLOG_PROCESS_ERROR(pItemProperty && pItemProperty->pItemInfo);
    nGenre = pItemProperty->pItemInfo->nGenre;
    
    if (nPackageIndex == eppiPlayerShotcutBar)
    {
        KG_PROCESS_ERROR(nGenre == igCommonItem);
        assert(pItemProperty->pItemInfo->pOtherExtInfo);
        KG_PROCESS_ERROR(pItemProperty->pItemInfo->pOtherExtInfo->bUseInScene);
    }

    bResult = true;
Exit0:
    return bResult;
}

KPackage* KPlayerPackage::GetPackage(int nPackageIndex)
{
    KPackage* pResult = NULL;

    KGLOG_PROCESS_ERROR(nPackageIndex >= eppiBegin && nPackageIndex < eppiTotal);
 
    pResult = &m_Package[nPackageIndex];
Exit0:
    return pResult;
}

BOOL KPlayerPackage::HasFreePos(int nPackageIndex)
{
    BOOL bResult = false;
    int nPos = 0;
    
    KGLOG_PROCESS_ERROR(nPackageIndex >= eppiBegin && nPackageIndex < eppiTotal);

    nPos = m_Package[nPackageIndex].GetFreePos();
    KGLOG_PROCESS_ERROR(nPos >= 0);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerPackage::CheckItemValidity()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    for (int nIndex = eppiBegin; nIndex < eppiTotal; ++nIndex)
    {
        bRetCode = m_Package[nIndex].CheckItemValidity();
        KGLOG_PROCESS_ERROR(bRetCode);
    }    

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerPackage::CostItem(int nPackageIndex, int nPos, int nCount)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(nPackageIndex >= eppiBegin && nPackageIndex < eppiTotal);

    bRetCode = m_Package[nPackageIndex].CostItem(nPos, nCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerPackage::FillShortcutBar()
{
    BOOL        bResult	                = false;
    BOOL        bRetCode	            = false;
    int         nShotcutBarPackageSize  = 0;
    int         nCommonItemPackageSize  = 0;
    int         nCommonItemStackNum     = 0;
    int         nAbsenceItemNum         = 0;
    DWORD       dwShotcutBarMaxStackNum = 0;
    KPackage*   pShotcutBarPackage      = NULL;
    KPackage*   pCommonItemPackage      = NULL;
    IItem*      pShotcutBarItem         = NULL;
    IItem*      pCommonItem             = NULL;
    uint32_t    dwShotcutBarTabType     = 0;
    uint32_t    dwShotcutBarTabIndex    = 0;
    const KItemProperty*    pCommonItemProperty = NULL;
    int nShotcutBarItemStackNum = 0;

    pShotcutBarPackage = &m_Package[eppiPlayerShotcutBar];
    KGLOG_PROCESS_ERROR(pShotcutBarPackage);

    pCommonItemPackage = &m_Package[eppiPlayerItemBox];
    KGLOG_PROCESS_ERROR(pCommonItemPackage);

    assert(countof(m_ReloadInfo) == pShotcutBarPackage->GetSize());
    for (int i = 0; i < countof(m_ReloadInfo); ++i)
    {
        if (m_ReloadInfo[i].nTabType == 0)
            continue;

        pShotcutBarItem = pShotcutBarPackage->GetItem(i);
        if (pShotcutBarItem && !pShotcutBarItem->CanStack())
            continue;

        if (pShotcutBarItem == NULL)
        {
            KGItemInfo* pItemInfo = NULL;

            dwShotcutBarTabType = m_ReloadInfo[i].nTabType;
            dwShotcutBarTabIndex= m_ReloadInfo[i].nTabIndex;

            pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(dwShotcutBarTabType, dwShotcutBarTabIndex);
            KGLOG_PROCESS_ERROR(pItemInfo);

            nShotcutBarItemStackNum = 0;
            nAbsenceItemNum = pItemInfo->dwShotcutBarMaxStackNum;
        }
        else
        {
            const KItemProperty*    pShotcutBarProperty = NULL;

            nShotcutBarItemStackNum = pShotcutBarItem->GetStackNum();
            KGLOG_PROCESS_ERROR(nShotcutBarItemStackNum >= 0);

            pShotcutBarProperty = pShotcutBarItem->GetProperty();
            KGLOG_PROCESS_ERROR(pShotcutBarProperty);
            dwShotcutBarTabType = pShotcutBarProperty->dwTabType;
            dwShotcutBarTabIndex= pShotcutBarProperty->dwTabIndex;

            KGLOG_PROCESS_ERROR(pShotcutBarProperty->pItemInfo);
            dwShotcutBarMaxStackNum = pShotcutBarProperty->pItemInfo->dwShotcutBarMaxStackNum;
            KGLOG_PROCESS_ERROR(nShotcutBarItemStackNum <= (int)dwShotcutBarMaxStackNum);      

            nAbsenceItemNum = dwShotcutBarMaxStackNum - nShotcutBarItemStackNum;
            KGLOG_PROCESS_ERROR(nAbsenceItemNum >= 0);
        }
        
        if (nAbsenceItemNum == 0)
            continue;    
     
        nCommonItemPackageSize = pCommonItemPackage->GetSize();
        KGLOG_PROCESS_ERROR(nCommonItemPackageSize >= 0);

        for (int nCommonItemPos = 0; nCommonItemPos < nCommonItemPackageSize; ++nCommonItemPos)
        {
            pCommonItem = pCommonItemPackage->GetItem(nCommonItemPos);
            if (!pCommonItem)
                continue;
            
            pCommonItemProperty = pCommonItem->GetProperty();
            KGLOG_PROCESS_ERROR(pCommonItemProperty);

            if (
                pCommonItemProperty->dwTabType  != dwShotcutBarTabType ||
                pCommonItemProperty->dwTabIndex != dwShotcutBarTabIndex
            )
                continue;

            if (pShotcutBarItem) // 快捷物品没有消耗完的情况
            {
                nCommonItemStackNum = pCommonItem->GetStackNum();
                if (nCommonItemStackNum > nAbsenceItemNum)
                {
                    bRetCode = pShotcutBarItem->SetStackNum(dwShotcutBarMaxStackNum);
                    KGLOG_PROCESS_ERROR(bRetCode);

                    bRetCode = pCommonItem->SetStackNum(nCommonItemStackNum - nAbsenceItemNum);
                    KGLOG_PROCESS_ERROR(bRetCode);

                    g_PlayerServer.DoUpdateItemAmount(
                        m_pPlayer->m_nConnIndex, ePlayerPackage, eppiPlayerItemBox, nCommonItemPos, nCommonItemStackNum - nAbsenceItemNum
                    );

                    g_PlayerServer.DoUpdateItemAmount(
                        m_pPlayer->m_nConnIndex, ePlayerPackage, eppiPlayerShotcutBar, i, dwShotcutBarMaxStackNum
                    );

                    break;
                }        

                bRetCode = pShotcutBarItem->SetStackNum(nShotcutBarItemStackNum + nCommonItemStackNum);
                KGLOG_PROCESS_ERROR(bRetCode);

                nAbsenceItemNum -= nCommonItemStackNum;
                KGLOG_PROCESS_ERROR(nAbsenceItemNum >= 0);

                bRetCode = pCommonItemPackage->DestroyItem(nCommonItemPos);
                KGLOG_PROCESS_ERROR(bRetCode);

                g_PlayerServer.DoUpdateItemAmount(
                    m_pPlayer->m_nConnIndex, ePlayerPackage, eppiPlayerShotcutBar, i, dwShotcutBarMaxStackNum - nAbsenceItemNum
                );  
            }
            else // 快捷物品消耗完了
            {
                bRetCode = m_pPlayer->m_ItemList.ExchangeItem(ePlayerPackage, eppiPlayerItemBox, nCommonItemPos, ePlayerPackage, eppiPlayerShotcutBar, i);
                KGLOG_PROCESS_ERROR(bRetCode);
            }
        }        
    } 

    memset(m_ReloadInfo, 0, sizeof(m_ReloadInfo));

    bResult	= true;
Exit0:
    return bResult;
}

void KPlayerPackage::PushReloadInfo(int nPos, int nTabType, int nTabIndex)
{
    KITEM_TAB_INFO info;
    KGLOG_PROCESS_ERROR(nPos >= 0 && nPos < cdShotcutSize);

    info.nTabType = nTabType;
    info.nTabIndex = nTabIndex;

    m_ReloadInfo[nPos] = info;

Exit0:
    return;
}
