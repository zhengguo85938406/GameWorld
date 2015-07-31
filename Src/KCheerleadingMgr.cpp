#include "stdafx.h"
#include "KCheerleadingMgr.h"
#include "KRoleDBDataDef.h"
#include "KPlayerServer.h"
#include "KPlayer.h"
#include "KSO3World.h"
#include "game_define.h"
#include "Protocol/gs2cl_protocol.h"

KCheerleadingMgr::KCheerleadingMgr(void)
{
    m_pOwner        = NULL;
    m_dwNextIndex   = 1;
    m_uBuySlotTimes = 0;
    m_uSlotCount    = 0;
    memset(m_SlotInfo, 0, sizeof(m_SlotInfo));
}

KCheerleadingMgr::~KCheerleadingMgr(void)
{
}

BOOL KCheerleadingMgr::Init(KPlayer* pOwner)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	KGLOG_PROCESS_ERROR(pOwner);
    KGLOG_PROCESS_ERROR(!m_pOwner);

    m_uSlotCount = g_pSO3World->m_Settings.m_ConstList.uDefaultCheerleaindgSlotCount;

    m_pOwner = pOwner;

	bResult = true;
Exit0:
	return bResult;
}

void KCheerleadingMgr::UnInit()
{
}

BOOL KCheerleadingMgr::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{           
    BOOL                    bResult             = false;
    BOOL                    bRetCode            = false;
    DWORD                   dwMaxIndex          = 0;

    assert(pLoadBuf);

    const T3DB::KPBCheerleadingMgr& CheerleadingMagr     = pLoadBuf->cheerleadingmgr();
    const T3DB::KPB_CHEERLEADING_DATA& CheerleadingData     = CheerleadingMagr.cheerleadingdata();

    m_uBuySlotTimes     = CheerleadingData.buyslottimes();
    m_uSlotCount        = CheerleadingData.validslotcount();

    for (int i = 0; i < countof(m_SlotInfo); ++i)
    {
        const T3DB::KPB_CHEERLEADING_SLOTINFO& SlotInfo = CheerleadingData.slotinfo(i);

        m_SlotInfo[i].dwCheerleadingIndex = SlotInfo.index();
        m_SlotInfo[i].nOffsetX = SlotInfo.offsetx();
        m_SlotInfo[i].nFaceTo  = SlotInfo.faceto();
    }

    for (int i = 0; i < CheerleadingData.cheerleadingitems_size(); ++i)
    {       
        const T3DB::KPB_CHEERLEADING_ITEM& PBCheerleadingItem   = CheerleadingData.cheerleadingitems(i);
        KCHEERLEADING_ITEM* pCheerleadingItem = &m_mapCheerleadingItems[PBCheerleadingItem.index()];

        pCheerleadingItem->dwID     =   PBCheerleadingItem.id();
        pCheerleadingItem->dwIndex  =   PBCheerleadingItem.index();
        pCheerleadingItem->nEndTime =   PBCheerleadingItem.endtime();

        if (m_pOwner->m_eConnectType == eLogin)
        {
            g_PlayerServer.DoSyncCheerleadingItem(m_pOwner, pCheerleadingItem->dwIndex, pCheerleadingItem->dwID, pCheerleadingItem->nEndTime);
        }

        if (pCheerleadingItem->dwIndex > dwMaxIndex)
            dwMaxIndex = pCheerleadingItem->dwIndex;
    }

    g_PlayerServer.DoSyncCheerleadingSlotInfo(m_pOwner);

    m_dwNextIndex = dwMaxIndex + 1;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KCheerleadingMgr::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                                bResult               = false;
    T3DB::KPB_CHEERLEADING_DATA*        pCheerleadingData     = NULL;
    T3DB::KPB_CHEERLEADING_ITEM*        pPBCheerleadingItem   = NULL;
    T3DB::KPBCheerleadingMgr*             pCheerleadingMagr     = NULL;
    T3DB::KPB_CHEERLEADING_SLOTINFO*    pSlotInfo             = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pCheerleadingMagr = pSaveBuf->mutable_cheerleadingmgr();
    pCheerleadingData = pCheerleadingMagr->mutable_cheerleadingdata();

    pCheerleadingData->set_buyslottimes(m_uBuySlotTimes);
    pCheerleadingData->set_validslotcount(m_uSlotCount);

    for (int i = 0; i < countof(m_SlotInfo); ++i)
    {
        pSlotInfo = pCheerleadingData->add_slotinfo();
        pSlotInfo->set_index(m_SlotInfo[i].dwCheerleadingIndex);
        pSlotInfo->set_offsetx(m_SlotInfo[i].nOffsetX);
        pSlotInfo->set_faceto(m_SlotInfo[i].nFaceTo);
    }

    for (KMAP_CHEERLEADING::iterator it = m_mapCheerleadingItems.begin(); it != m_mapCheerleadingItems.end(); ++it)
    {
        pPBCheerleadingItem = pCheerleadingData->add_cheerleadingitems();
        KCHEERLEADING_ITEM* pCheerleadingItem = &it->second;

        pPBCheerleadingItem->set_id(pCheerleadingItem->dwID);
        pPBCheerleadingItem->set_index(pCheerleadingItem->dwIndex);
        pPBCheerleadingItem->set_endtime(pCheerleadingItem->nEndTime);
    }

    bResult = true;
Exit0:
    return bResult;
}


BOOL KCheerleadingMgr::AddCheerleading(DWORD dwCheerleadingID)
{
	BOOL    bResult     = false;
	BOOL    bRetCode    = false;
    DWORD   dwIndex     = 0;
    int     nEndTime    = 0;
	KCHEERLEADING_ITEM* pCheerleadingItem = NULL;
    KGCheerleadingInfo* pCheerleadingInfo = NULL;

    KGLOG_PROCESS_ERROR((int)m_mapCheerleadingItems.size() < g_pSO3World->m_Settings.m_ConstList.nMaxCheerleadingCount);
    
    pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo(dwCheerleadingID);
    KGLOG_PROCESS_ERROR(pCheerleadingInfo);

    if (pCheerleadingInfo->nOverdueTime > 0)
        nEndTime = g_pSO3World->m_nCurrentTime + pCheerleadingInfo->nOverdueTime;

    dwIndex = m_dwNextIndex++;

    pCheerleadingItem = &m_mapCheerleadingItems[dwIndex];
    pCheerleadingItem->dwIndex  = dwIndex;
    pCheerleadingItem->dwID     = dwCheerleadingID;
    pCheerleadingItem->nEndTime = nEndTime;

    g_PlayerServer.DoSyncCheerleadingItem(m_pOwner, pCheerleadingItem->dwIndex, pCheerleadingItem->dwID, pCheerleadingItem->nEndTime);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCheerleadingMgr::AddSlot(DWORD dwSlotIndex)
{
	BOOL bResult  = false;
	
	KGLOG_PROCESS_ERROR(dwSlotIndex < cdMaxCheerleadingSlotCount);
    KGLOG_PROCESS_ERROR(dwSlotIndex == m_uSlotCount);

    ++m_uSlotCount;
   
    g_PlayerServer.DoSyncCheerleadingSlotInfo(m_pOwner);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCheerleadingMgr::BuySlot(DWORD dwSlotIndex)
{
	BOOL bResult   = false;
	BOOL bRetCode  = false;
	int  nPrice    = 0;
    KCUSTOM_CONSUME_INFO param = {0, 0, 0, 0};

    KGLOG_PROCESS_ERROR(m_uSlotCount == dwSlotIndex);
    KGLOG_PROCESS_ERROR(dwSlotIndex < countof(m_SlotInfo));
  
    KGLOG_PROCESS_ERROR(m_uBuySlotTimes < countof(g_pSO3World->m_Settings.m_ConstList.CheerleadingSlotPrice));
    nPrice = g_pSO3World->m_Settings.m_ConstList.CheerleadingSlotPrice[m_uBuySlotTimes];

    param.nValue1 = (int)dwSlotIndex;
    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pOwner, uctBuyCheerleadingSlot, nPrice, &param);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCheerleadingMgr::UpdateCheerleadingSlotInfo(const KCHEERLEADING_SLOTINFO (&slotInfo)[cdMaxCheerleadingSlotCount])
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    std::vector<DWORD> vecUsedCheerleadingIndex;
    std::vector<DWORD>::iterator it;
    KCHEERLEADING_ITEM* pCheerleadingItem = NULL;  

    vecUsedCheerleadingIndex.reserve(cdMaxCheerleadingSlotCount);
    for (int i = 0; i < countof(slotInfo); ++i)
    {
        if (slotInfo[i].dwCheerleadingIndex == 0)
            continue;

        KGLOG_PROCESS_ERROR(slotInfo[i].nOffsetX >= 0 && slotInfo[i].nOffsetX <= cdMaxCheerleadingOffsetX);
        KGLOG_PROCESS_ERROR(slotInfo[i].nFaceTo == 0 || slotInfo[i].nFaceTo == 1);

        pCheerleadingItem = GetCheerleadingItem(slotInfo[i].dwCheerleadingIndex);
        KGLOG_PROCESS_ERROR(pCheerleadingItem);

        it = std::find(vecUsedCheerleadingIndex.begin(), vecUsedCheerleadingIndex.end(), slotInfo[i].dwCheerleadingIndex);
        KGLOG_PROCESS_ERROR(it == vecUsedCheerleadingIndex.end());

        vecUsedCheerleadingIndex.push_back(slotInfo[i].dwCheerleadingIndex);
    }

    memcpy(m_SlotInfo, slotInfo, sizeof(m_SlotInfo));

    g_PlayerServer.DoSyncCheerleadingSlotInfo(m_pOwner);

    m_pOwner->OnEvent(peEquipCheerleading);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCheerleadingMgr::OnRechargeCheerleadingItemRequest(DWORD dwCheerleadingItemIndex, int nLifeType)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KCUSTOM_CONSUME_INFO info = {0,0,0,0};
    KCHEERLEADING_ITEM* pCheerleadingItem = NULL;
    KGCheerleadingInfo* pCheerleadingInfo = NULL;
    int nMoneyCost = 0;

    KGLOG_PROCESS_ERROR(nLifeType >= KLIFE_TYPE_BEGIN && nLifeType < KLIFE_TYPE_END);

    pCheerleadingItem = GetCheerleadingItem(dwCheerleadingItemIndex);
    KGLOG_PROCESS_ERROR(pCheerleadingItem);
    KGLOG_PROCESS_ERROR(pCheerleadingItem->nEndTime > 0);

    pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo(pCheerleadingItem->dwID);
    KGLOG_PROCESS_ERROR(pCheerleadingInfo);

    KGLOG_PROCESS_ERROR(pCheerleadingInfo->bCanRecharge);

    info.nValue1 = (int)dwCheerleadingItemIndex;
    info.nValue2 = nLifeType;

    KGLOG_PROCESS_ERROR(nLifeType >= 0 && nLifeType < countof(pCheerleadingInfo->Price));
    nMoneyCost = pCheerleadingInfo->Price[nLifeType];
    KGLOG_PROCESS_ERROR(nMoneyCost > 0);

	bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pOwner, uctRechargeCheerleadingItem, nMoneyCost, &info);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

KCHEERLEADING_ITEM* KCheerleadingMgr::GetCheerleadingItem(DWORD dwCheerleadingIndex)
{
    KCHEERLEADING_ITEM* pResult = NULL;
    KMAP_CHEERLEADING::iterator it;

    it = m_mapCheerleadingItems.find(dwCheerleadingIndex);
    if (it != m_mapCheerleadingItems.end())
    {
        pResult = &it->second;
    }

    return pResult;
}

void KCheerleadingMgr::Activate()
{
    KReport_Cheerleading_TimeOut param;

    if ((g_pSO3World->m_nGameLoop - m_pOwner->m_dwID) % GAME_FPS != 0)
        return;

    for (KMAP_CHEERLEADING::iterator it = m_mapCheerleadingItems.begin(); it != m_mapCheerleadingItems.end();)
    {
        if (it->second.nEndTime <= 0 || g_pSO3World->m_nCurrentTime < it->second.nEndTime)
        {
            ++it;
            continue;
        }

        ResetSlotWhichCheerleadingItemIn(it->first);

        g_LogClient.DoFlowRecord(
            frmtCheerleadingAndWardrobe, cawfrCheerleadingDisappear,
            "%s,%u",
            m_pOwner->m_szName,
            it->first
        );

        g_PlayerServer.DoRemoveCheerleadingItem(m_pOwner, it->first);

        //小秘书通知
        param.dwCheerleadingID = it->second.dwID;
        m_pOwner->m_Secretary.AddReport(KREPORT_EVENT_CHEERLEADING_TIMEOUT, (BYTE*)&param, sizeof(param));

        m_mapCheerleadingItems.erase(it++);                
    }
}

void KCheerleadingMgr::ResetSlotWhichCheerleadingItemIn(DWORD dwCheerleadingItemIndex)
{
    for (DWORD i = 0; i < m_uSlotCount; ++i)
    {
        if (m_SlotInfo[i].dwCheerleadingIndex == dwCheerleadingItemIndex)
        {
            m_SlotInfo[i].dwCheerleadingIndex = 0;
            g_PlayerServer.DoSyncCheerleadingSlotInfo(m_pOwner);
            break;
        }
    }
}

BOOL KCheerleadingMgr::OnRechargeCheerleadingItemSuccess(DWORD dwCheerleadingItemIndex, int nLifeType)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KCHEERLEADING_ITEM* pCheerleadingItem = NULL;
    KGCheerleadingInfo* pCheerleadingInfo = NULL;
	
	pCheerleadingItem = GetCheerleadingItem(dwCheerleadingItemIndex);
    KGLOG_PROCESS_ERROR(pCheerleadingItem);
    KGLOG_PROCESS_ERROR(pCheerleadingItem->nEndTime > 0);

    KGLOG_PROCESS_ERROR(nLifeType >= KLIFE_TYPE_BEGIN && nLifeType < KLIFE_TYPE_END);

    pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo(pCheerleadingItem->dwID);
    KGLOG_PROCESS_ERROR(pCheerleadingInfo);

    KGLOG_PROCESS_ERROR(pCheerleadingInfo->bCanRecharge);

    switch(nLifeType)
    {
    case KLIFE_TYPE_SEVEN_DAYS:
        pCheerleadingItem->nEndTime += 7 * SECONDS_PER_DAY;
        break;
    case KLIFE_TYPE_THIRTY_DAYS:
        pCheerleadingItem->nEndTime += 30 * SECONDS_PER_DAY;
        break;
    case KLIFE_TYPE_INFINITE:
        pCheerleadingItem->nEndTime = 0;
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    g_PlayerServer.DoSyncCheerleadingItem(m_pOwner, dwCheerleadingItemIndex, pCheerleadingItem->dwID, pCheerleadingItem->nEndTime);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCheerleadingMgr::OnBuySlotSuccess(DWORD dwSlotIndex)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = AddSlot(dwSlotIndex);
    if (!bRetCode)
    {
        KGLogPrintf(KGLOG_ERR, "Player:%u BuySlot:%u Success, But AddFailed.\n", m_pOwner->m_dwID, dwSlotIndex);
        goto Exit0;
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCheerleadingMgr::GetSlotInfoInBattle(KS2C_Cheerleading_SlotInfo_In_Battle (&slotInfoInBattle)[cdMaxCheerleadingSlotCount])
{
    BOOL bResult  = false;
    KCHEERLEADING_ITEM* pCheerleadingItem = NULL;

    for (int i = 0; i < countof(slotInfoInBattle); ++i)
    {
        if (m_SlotInfo[i].dwCheerleadingIndex == 0)
        {
            slotInfoInBattle[i].cheerleadingID = 0;
            continue;
        }

        pCheerleadingItem = GetCheerleadingItem(m_SlotInfo[i].dwCheerleadingIndex);
        KGLOG_PROCESS_ERROR(pCheerleadingItem);
        
        slotInfoInBattle[i].faceTo  = (short)m_SlotInfo[i].nFaceTo;
        slotInfoInBattle[i].offsetX = (short)m_SlotInfo[i].nOffsetX;
        slotInfoInBattle[i].cheerleadingID = (unsigned short)pCheerleadingItem->dwID;
    }

    bResult = true;
Exit0:
    return bResult;
}
