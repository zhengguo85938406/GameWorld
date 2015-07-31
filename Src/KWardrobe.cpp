#include "stdafx.h"
#include "KWardrobe.h"
#include "KRoleDBDataDef.h"
#include "KGItemInfoList.h"
#include "KSO3World.h"
#include "KHeroDataList.h"
#include "KPlayer.h"
#include "KPlayerServer.h"

BOOL KWardrobe::Init(KPlayer* pPlayer)
{
    BOOL bResult = false;

    m_nMaxFashionCount = g_pSO3World->m_Settings.m_ConstList.nMaxFashionCount;
    KGLOG_PROCESS_ERROR(m_nMaxFashionCount > 0);

    KGLOG_PROCESS_ERROR(pPlayer);
    m_pPlayer = pPlayer;

    bResult = true;
Exit0:
    return bResult;
}

void KWardrobe::UnInit()
{
    m_WardrobeInfoMap.clear();
}

BOOL KWardrobe::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    size_t                  uFashionCounts  = 0;
    KPLAYER_FASHION_INFO    FashionInfo;
    std::pair<WARDROBE_INFO_TABLE::iterator, bool> InsRet;

    assert(pLoadBuf);

    const T3DB::KPBWardrobe& Wardrobe = pLoadBuf->wardrobe();
    const T3DB::KPB_WARDROBE_DATA& WardrodeData   = Wardrobe.wardrobedata();

    uFashionCounts = WardrodeData.fashiondata_size();

    for (int i = 0; (uint32_t)i < uFashionCounts; ++i)
    {
        const T3DB::KPB_FASHION_DATA& PBFashionInfo = WardrodeData.fashiondata(i);

        FashionInfo.dwFashionID     = PBFashionInfo.fashionid();
        FashionInfo.dwOverduePoint  = PBFashionInfo.overduepoint();
        FashionInfo.dwBelongtoHeroID= PBFashionInfo.belongtoheroid();

        InsRet = m_WardrobeInfoMap.insert(std::make_pair(FashionInfo.dwFashionID, FashionInfo));
        KGLOG_PROCESS_ERROR(InsRet.second);
    }

    if (m_pPlayer->m_eConnectType == eLogin)
        g_PlayerServer.DoSyncWardrobeInfo(m_pPlayer);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KWardrobe::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                        bResult         = false;
    T3DB::KPB_WARDROBE_DATA*    pWardrodeData   = NULL;
    T3DB::KPB_FASHION_DATA*     pFashionInfo    = NULL;
    T3DB::KPBWardrobe*          pWardrobe       = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pWardrobe = pSaveBuf->mutable_wardrobe();
    pWardrodeData = pWardrobe->mutable_wardrobedata();

    for (WARDROBE_INFO_TABLE::iterator it = m_WardrobeInfoMap.begin(); it != m_WardrobeInfoMap.end(); ++it)
    {
        pFashionInfo = pWardrodeData->add_fashiondata();
        
        pFashionInfo->set_fashionid(it->second.dwFashionID);
        pFashionInfo->set_overduepoint(it->second.dwOverduePoint);
        pFashionInfo->set_belongtoheroid(it->second.dwBelongtoHeroID);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KWardrobe::OnBuyFashionRequest(uint32_t dwFashionIndex)
{
    BOOL            bResult         = false;

//     暂时不提供此途径进行购买

//     BOOL            bRetCode        = false;
//     int             nPrice          = 0;
//     GOODSINFO*      pGoodsInfo      = NULL;
//     KFASHION_INFO*  pFashionInfo    = NULL;
//     KCUSTOM_CONSUME_INFO     CCInfo;
// 
//     pFashionInfo = g_pSO3World->m_ItemHouse.GetFashionInfo(dwFashionIndex);
//     KGLOG_PROCESS_ERROR(pFashionInfo);
// 
//     memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));
//     CCInfo.nValue1 = (int)dwFashionIndex;
// 
//     pGoodsInfo = g_pSO3World->m_ShopMgr.GetGoodsInfo(ittFashion, dwFashionIndex);
//     KGLOG_PROCESS_ERROR(pGoodsInfo);
// 
//     nPrice = pGoodsInfo->nCost;
//     KGLOG_PROCESS_ERROR(nPrice > 0);
// 
//     bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctBuyFashion, nPrice, &CCInfo);
//     KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KWardrobe::AddFashion(uint32_t dwFashionIndex)
{
    BOOL            bResult	        = false;
    BOOL            bRetCode	    = false;
    BOOL            bForever        = false;
    char            szForeverTime[] = "2015-12-24-20";
    uint32_t        dwForeverOverdueTimePoint  = 0;
    KFASHION_INFO*  pFashionInfo    = NULL;
    KPLAYER_FASHION_INFO FashionInfo;
    std::pair<WARDROBE_INFO_TABLE::iterator, bool> InsRet;

    bRetCode = IsWardrobeFull();
    KGLOG_PROCESS_ERROR(bRetCode);

    pFashionInfo = g_pSO3World->m_ItemHouse.GetFashionInfo(dwFashionIndex);
    KGLOG_PROCESS_ERROR(pFashionInfo);

    FashionInfo.dwFashionID         = (WORD)pFashionInfo->dwFashionID;
    FashionInfo.dwBelongtoHeroID    = pFashionInfo->dwBelongtoHeroTemplateID;
    FashionInfo.dwOverduePoint      = pFashionInfo->dwOverduePoint;

    bForever = (pFashionInfo->dwOverdueTime == 0 ) && (pFashionInfo->dwOverduePoint == 0);
    
    bRetCode = g_GetTimeFromString(szForeverTime, strlen(szForeverTime), dwForeverOverdueTimePoint);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (IsFashionExist((WORD)pFashionInfo->dwFashionID))
    {
        InsRet.first = m_WardrobeInfoMap.find((WORD)pFashionInfo->dwFashionID);
        KGLOG_PROCESS_ERROR(InsRet.first != m_WardrobeInfoMap.end());

        if (bForever || InsRet.first->second.dwOverduePoint == dwForeverOverdueTimePoint)
            InsRet.first->second.dwOverduePoint = dwForeverOverdueTimePoint;
        else
            InsRet.first->second.dwOverduePoint += pFashionInfo->dwOverdueTime;

        g_PlayerServer.DoSyncFashionInfo(m_pPlayer, InsRet.first->first, InsRet.first->second.dwOverduePoint);

        goto Exit1;
    }

    FashionInfo.dwOverduePoint = bForever ? dwForeverOverdueTimePoint : g_pSO3World->m_nCurrentTime + pFashionInfo->dwOverdueTime;
    KGLOG_PROCESS_ERROR(FashionInfo.dwOverduePoint > (uint32_t)g_pSO3World->m_nCurrentTime);

    InsRet = m_WardrobeInfoMap.insert(std::make_pair(pFashionInfo->dwFashionID, FashionInfo));
    KGLOG_PROCESS_ERROR(InsRet.second);

    g_PlayerServer.DoSyncFashionInfo(m_pPlayer, InsRet.first->first, InsRet.first->second.dwOverduePoint);

Exit1:
    bResult	= true;
Exit0:
    return bResult;
}

BOOL KWardrobe::OnChargeFashionRequest(uint32_t dwFashionIndex, uint32_t dwChargeType)
{
    BOOL            bResult	        = false;
    BOOL            bRetCode	    = false;
    uint32_t        dwFashionID     = 0;
    int             nPrice          = 0;
    GOODSINFO*      pGoodsInfo      = NULL;
    KFASHION_INFO*  pFashionInfo    = NULL;
    KCUSTOM_CONSUME_INFO     CCInfo;

    pFashionInfo = g_pSO3World->m_ItemHouse.GetFashionInfo(dwFashionIndex);
    KGLOG_PROCESS_ERROR(pFashionInfo);
    KGLOG_PROCESS_ERROR(pFashionInfo->bCanRecharge);
    dwFashionID  = pFashionInfo->dwFashionID;

    bRetCode = IsFashionExist(dwFashionID);
    KGLOG_PROCESS_ERROR(bRetCode);    

    switch (dwChargeType)
    {
    case KLIFE_TYPE_SEVEN_DAYS:
        nPrice = (int)pFashionInfo->dwChargeType_7Days;
        KGLOG_PROCESS_ERROR(nPrice > 0);
    	break;
    case KLIFE_TYPE_THIRTY_DAYS:
        nPrice = (int)pFashionInfo->dwChargeType_30Days;
        KGLOG_PROCESS_ERROR(nPrice > 0);
        break;
    case KLIFE_TYPE_INFINITE:
        nPrice = (int)pFashionInfo->dwChargeType_Forever;
        KGLOG_PROCESS_ERROR(nPrice > 0);
        break;
    default:
        KGLOG_PROCESS_ERROR(0);
    }

    memset(&CCInfo, 0, sizeof(KCUSTOM_CONSUME_INFO));
    CCInfo.nValue1 = (int)dwFashionIndex;
    CCInfo.nValue2 = (int)dwChargeType;

    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(m_pPlayer, uctChargeFashion, nPrice, &CCInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KWardrobe::OnChargeFashionSuccess(uint32_t dwFashionIndex)
{
    BOOL                    bResult	        = false;
    BOOL                    bRetCode	    = false;
    KFASHION_INFO*          pFashionInfo    = NULL;
    WORD                    wFashionID      = 0;
    uint32_t                dwFashionOverduePoint   = 0;
    KPLAYER_FASHION_INFO    FashionInfo;
    std::pair<WARDROBE_INFO_TABLE::iterator, bool> InsRet;

    pFashionInfo = g_pSO3World->m_ItemHouse.GetFashionInfo(dwFashionIndex);
    KGLOG_PROCESS_ERROR(pFashionInfo);
    wFashionID  = (WORD)pFashionInfo->dwFashionID;

    if (IsFashionExist((WORD)wFashionID))
    {
        InsRet.first = m_WardrobeInfoMap.find((WORD)wFashionID);
        KGLOG_PROCESS_ERROR(InsRet.first != m_WardrobeInfoMap.end());

        InsRet.first->second.dwOverduePoint += pFashionInfo->dwOverdueTime;
        dwFashionOverduePoint = InsRet.first->second.dwOverduePoint;

        goto Exit1;
    }
    else
    {
        FashionInfo.dwFashionID         = (WORD)pFashionInfo->dwFashionID;
        FashionInfo.dwBelongtoHeroID    = pFashionInfo->dwBelongtoHeroTemplateID;
        FashionInfo.dwOverduePoint      = pFashionInfo->dwOverduePoint;

        dwFashionOverduePoint           = FashionInfo.dwOverduePoint;

        InsRet = m_WardrobeInfoMap.insert(std::make_pair(FashionInfo.dwFashionID, FashionInfo));
        KGLOG_PROCESS_ERROR(InsRet.second);
    }

Exit1:
    g_PlayerServer.DoSyncFashionInfo(m_pPlayer, wFashionID, dwFashionOverduePoint);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KWardrobe::IsFashionExist(uint32_t dwFashionID)
{
    BOOL bResult	= false;
    WARDROBE_INFO_TABLE::iterator it;

    it = m_WardrobeInfoMap.find((WORD)dwFashionID);
    KG_PROCESS_ERROR(it != m_WardrobeInfoMap.end());

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KWardrobe::ChangeFashion(uint32_t dwHeroTemplateID, uint32_t dwTargetFashionID)
{
    BOOL                    bResult	        = false;
    BOOL                    bRetCode	    = false;
    KHeroData*              pHeroData       = NULL;
    KPLAYER_FASHION_INFO*   pTargetFashion  = NULL;
    WARDROBE_INFO_TABLE::iterator it;

    if (dwTargetFashionID == 0) // 发 0 卸下 外装
    {
        KGLOG_PROCESS_ERROR(m_pPlayer);

        pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
        KGLOG_PROCESS_ERROR(pHeroData);
        pHeroData->m_wFashionID = (WORD)dwTargetFashionID;

        g_PlayerServer.DoChangeFashionRespond(m_pPlayer, true, dwHeroTemplateID, dwTargetFashionID);

        goto Exit1;
    }

    it = m_WardrobeInfoMap.find((WORD)dwTargetFashionID);
    KGLOG_PROCESS_ERROR(it != m_WardrobeInfoMap.end());

    pTargetFashion = &it->second;
    KGLOG_PROCESS_ERROR(pTargetFashion);
    KGLOG_PROCESS_ERROR(pTargetFashion->dwBelongtoHeroID == dwHeroTemplateID);

    KGLOG_PROCESS_ERROR(m_pPlayer);
    pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    bRetCode = IsFashionExist(dwTargetFashionID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pHeroData->m_wFashionID = (WORD)dwTargetFashionID;

    g_PlayerServer.DoChangeFashionRespond(m_pPlayer, true, dwHeroTemplateID, dwTargetFashionID);

    m_pPlayer->OnEvent(peEquipFashion);

Exit1:
    bResult	= true;
Exit0:
    return bResult;
}

BOOL KWardrobe::CheckValidity()
{
    BOOL        bResult	        = false;
    BOOL        bRetCode	    = false;
    uint32_t    dwCurrentTime   = 0;
    WARDROBE_INFO_TABLE::iterator it;
    KHeroData*	pHeroData       = NULL;
    KFASHION_INFO*  pFishion    = NULL;
    KReport_Fasion_TimeOut param;

    dwCurrentTime = g_pSO3World->m_nCurrentTime;

    it = m_WardrobeInfoMap.begin();
    while (it != m_WardrobeInfoMap.end())
    {
        if (dwCurrentTime >= it->second.dwOverduePoint && it->second.dwOverduePoint != 0)
        {
			pHeroData = m_pPlayer->m_HeroDataList.GetHeroData(it->second.dwBelongtoHeroID);
			if (pHeroData)
			{
				if (pHeroData->m_wFashionID == it->second.dwFashionID)
					pHeroData->m_wFashionID = 0;
			}

            g_LogClient.DoFlowRecord(
                frmtCheerleadingAndWardrobe, cawfrFashionDisappear,
                "%s,%u",
                m_pPlayer->m_szName,
                it->second.dwFashionID
            );

			g_PlayerServer.DoSyncFashionInfo(m_pPlayer, it->second.dwFashionID, it->second.dwOverduePoint);
            
            //小秘书通知
            param.fashionID = it->second.dwFashionID;
            param.heroTemplateID = it->second.dwBelongtoHeroID;
            m_pPlayer->m_Secretary.AddReport(KREPORT_EVENT_FASHION_TIMEOUT, (BYTE*)&param, sizeof(param));

			m_WardrobeInfoMap.erase(it++);

            continue;
        }

        ++it;
    }

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KWardrobe::GetSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize)
{
    BOOL                bResult	        = false;
    BOOL                bRetCode	    = false;
    BYTE*               pbyOffset       = pbyBuffer;
    size_t              uLeftSize       = uBufferSize;
    KPlayerFashionInfo* pFashionInfo    = NULL;

    KS2C_Sync_Wardrobe_Info* pPak = (KS2C_Sync_Wardrobe_Info*)pbyBuffer;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KS2C_Sync_Wardrobe_Info));
    pbyOffset += sizeof(KS2C_Sync_Wardrobe_Info);
    uLeftSize -= sizeof(KS2C_Sync_Wardrobe_Info);

    pPak->protocolID                = s2c_sync_wardrobe_info;
    pPak->wardrobeinfodataLength    = (WORD)m_WardrobeInfoMap.size();

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KPlayerFashionInfo) * pPak->wardrobeinfodataLength);

    pFashionInfo = &pPak->wardrobeinfodata[0];
    for (WARDROBE_INFO_TABLE::iterator it = m_WardrobeInfoMap.begin(); it != m_WardrobeInfoMap.end(); ++it)
    {
        pFashionInfo->wID = (WORD)it->second.dwFashionID;
        pFashionInfo->dwBelongtoHeroID = it->second.dwBelongtoHeroID;
        pFashionInfo->dwOverduePoint = it->second.dwOverduePoint;

        ++pFashionInfo;
    }

    pbyOffset += sizeof(KPlayerFashionInfo) * pPak->wardrobeinfodataLength;
    uLeftSize -= sizeof(KPlayerFashionInfo) * pPak->wardrobeinfodataLength;

    uUsedSize = uBufferSize - uLeftSize;

    bResult	= true;
Exit0:
    return bResult;
}
