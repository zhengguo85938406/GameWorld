#include "stdafx.h"
#include "KAwardMgr.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "game_define.h"
#include "KSO3World.h"
#include "KLSClient.h"

void KAwardTable::RatePreProcess()
{
    BOOL    bResult         = false;
    int     nCurRateSum     = 0;
    std::pair<KAWARD_ITEM_RATE_MAP::iterator, bool> InsRet;

    for (KAWARD_ITEM_MAP::iterator it = m_mapAwardItem.begin(); it != m_mapAwardItem.end(); ++it)
    {
        if (it->second.nAwardItemRate == 0)
            continue;

        nCurRateSum += it->second.nAwardItemRate;

        InsRet = m_mapAwardItemRate.insert(std::make_pair(nCurRateSum, it->first));
        KGLOG_PROCESS_ERROR(InsRet.second);
    }

    bResult = true;
Exit0:
    if (!bResult)
        m_mapAwardItemRate.clear();

    return;
}

KAWARD_ITEM* KAwardTable::GetRandomAward()
{
    KAWARD_ITEM* pResult = NULL;
    int nRandomNum = g_Random(1000000);
    KAWARD_ITEM_RATE_MAP::iterator itUpper;
    KAWARD_ITEM_MAP::iterator it;

    itUpper = m_mapAwardItemRate.upper_bound(nRandomNum);
    KG_PROCESS_ERROR(itUpper != m_mapAwardItemRate.end());

    it = m_mapAwardItem.find(itUpper->second);
    KGLOG_PROCESS_ERROR(it != m_mapAwardItem.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

KAWARD_ITEM* KAwardTable::GetAwardItem(int nIndex)
{
    int nCount = 0;
    int nItemNum = m_mapAwardItem.size();
    if (nItemNum == 0)
        return NULL;

    int nItemIndex = nIndex % nItemNum;
    for (KAWARD_ITEM_MAP::iterator it = m_mapAwardItem.begin(); it != m_mapAwardItem.end(); ++it, ++nCount)
    {
        if (nCount == nItemIndex)
            return &it->second;
    }

    return NULL;
}

KAWARD_ITEM* KAwardTable::GetOneAwardItemByEqualRate()
{
    KAWARD_ITEM*    pResult = NULL;
    unsigned        uCount  = 0U;
    unsigned        uIndex  = 0U;
    unsigned        i       = 0U;
    std::vector<DWORD> vecCanRandomItem;
    KAWARD_ITEM_MAP::iterator it;

    for (it = m_mapAwardItem.begin(); it != m_mapAwardItem.end(); ++it)
    {
        vecCanRandomItem.push_back(it->first);
    }

    uCount = vecCanRandomItem.size();
    KG_PROCESS_ERROR(uCount > 0);

    uIndex = g_Random(KILO_NUM) % uCount;
    it = m_mapAwardItem.find(vecCanRandomItem[uIndex]);
    KGLOG_PROCESS_ERROR(it != m_mapAwardItem.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

//////////////////////////////////////////////////////////////////////////

KAwardMgr::KAwardMgr()
{
}

KAwardMgr::~KAwardMgr()
{
}

BOOL KAwardMgr::Init()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = LoadAllAward();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KAwardMgr::UnInit()
{
    m_AwardTable.clear();
}

BOOL KAwardMgr::Reload()
{
    UnInit();
    return Init();
}

DWORD KAwardMgr::ScenenBuffAward(DWORD dwAwardID)
{
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;
    DWORD           dwBuffID        = 0;

    pAwardTable = g_pSO3World->m_AwardMgr.GetAwardTable(dwAwardID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    pAwardItem = pAwardTable->GetRandomAward();
    KGLOG_PROCESS_ERROR(pAwardItem);
    KGLOG_PROCESS_ERROR(pAwardItem->dwTabType == 0);
    KGLOG_PROCESS_ERROR(pAwardItem->dwBuffID);
    dwBuffID = pAwardItem->dwBuffID;

Exit0:
    return dwBuffID;
}

BOOL KAwardMgr::ScenenItemAward(std::vector<KPlayer*>& vecPlayer, DWORD dwAwardID)
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;
    KPlayer*        pPlayer         = NULL; 
    KSystemMailTxt* pSysMailTxt     = NULL;
    int             nValuePoint     = 0;

    pAwardTable = g_pSO3World->m_AwardMgr.GetAwardTable(dwAwardID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    pAwardItem = pAwardTable->GetRandomAward();
    KGLOG_PROCESS_ERROR(pAwardItem);
    KGLOG_PROCESS_ERROR(pAwardItem->dwTabType);

    pSysMailTxt = g_pSO3World->m_Settings.m_SystemMailMgr.GetSystemMailTxt(KMAIL_BATTLE_AWARD);
    KGLOG_PROCESS_ERROR(pSysMailTxt);

    for (size_t i = 0; i < vecPlayer.size(); ++i)
    {
        pPlayer = vecPlayer[i];

        KGLOG_PROCESS_ERROR(pPlayer);

        if (pAwardItem->dwTabType == ittOther || pAwardItem->dwTabType == ittEquip)
        {
            bRetCode = GetAwardItemValuePoint(pAwardItem, nValuePoint);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = pPlayer->SafeAddItem(pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint, 
                pSysMailTxt->m_szMailTitl, pSysMailTxt->m_szSendName, pSysMailTxt->m_szMailTxt
            );
            KGLOG_PROCESS_ERROR(bRetCode);

            PLAYER_LOG(
                pPlayer, "item,additem,%u-%u,%d,%d,%s,%d",
                pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint, 
                "SceneAward", pPlayer->m_dwMapID
            );
        }
        else if (pAwardItem->dwTabType == ittFashion)
        {
            bRetCode = pPlayer->m_Wardrobe.AddFashion(pAwardItem->dwIndex);
            KGLOG_CHECK_ERROR(bRetCode);
        }
        else if (pAwardItem->dwTabType == ittCheerleading)
        {
            KGCheerleadingInfo* pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo(pAwardItem->dwIndex);
            KGLOG_PROCESS_ERROR(pCheerleadingInfo);

            bRetCode = pPlayer->m_CheerleadingMgr.AddCheerleading(pCheerleadingInfo->dwID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else if (pAwardItem->dwTabType == ittTeamLogo)
        {
            KTEAM_LOGO_INFO*    pTeamLogoInfo = NULL;

            pTeamLogoInfo = g_pSO3World->m_ItemHouse.GetTeamLogoInfo(pAwardItem->dwIndex);
            KGLOG_PROCESS_ERROR(pTeamLogoInfo);

            bRetCode = pPlayer->AddTeamLogo((WORD)pTeamLogoInfo->nID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KAwardMgr::Award(KPlayer* pPlayer, KAWARD_CARD_TYPE eType, DWORD dwAwardID, unsigned uChooseIndex, BOOL bEndChoose)
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    IItem*          pItem           = NULL;
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;
    KS2C_Award_Item arrAwardList[cdVipCardCount] = {0};
    IItem*          pLookItem = NULL;
    KSystemMailTxt* pSysMailTxt = NULL;
    KAWARD_ITEM*    pTempItem = NULL;
    KS2C_Award_Item* pAwardItemInfo = NULL;
    KS2C_Award_Item AwardItem;
    DWORD           dwRandSeed = 0;
    int nValuePoint = 0;

    assert(pPlayer);

    KGLOG_PROCESS_ERROR(uChooseIndex < countof(arrAwardList));

    pAwardTable = GetAwardTable(dwAwardID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    pAwardItem = pAwardTable->GetRandomAward();
    if (pAwardItem)
    {
        if (pAwardItem->dwTabType == ittOther || pAwardItem->dwTabType == ittEquip)
        { 
            pSysMailTxt = g_pSO3World->m_Settings.m_SystemMailMgr.GetSystemMailTxt(KMAIL_CHOOSE);
            KGLOG_PROCESS_ERROR(pSysMailTxt);

            bRetCode = GetAwardItemValuePoint(pAwardItem, nValuePoint);
            KGLOG_PROCESS_ERROR(bRetCode);
            
            dwRandSeed = KG_GetTickCount();

            bRetCode = pPlayer->SafeAddItem(pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint, 
                pSysMailTxt->m_szMailTitl, pSysMailTxt->m_szSendName, pSysMailTxt->m_szMailTxt, dwRandSeed
            );

            pItem = g_pSO3World->m_ItemHouse.CreateItem(pAwardItem->dwTabType, pAwardItem->dwIndex, 0, dwRandSeed, nValuePoint);

            if (bRetCode)
            {
                BroadCastAwardItem(pPlayer, pAwardItem);
 
                if (eType == KAWARD_CARD_TYPE_NORMAL)
                {
                    PLAYER_LOG(
                        pPlayer, "item,additem,%u-%u,%d,%d,%s,%d",
                        pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint, 
                        "missionaward", pPlayer->m_dwMapID
                        );
                }
                else if (eType == KAWARD_CARD_TYPE_GOLD)
                {
                    PLAYER_LOG(
                        pPlayer, "item,additem,%u-%u,%d,%d,%s,%d",
                        pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint,
                        "goldaward", pPlayer->m_dwMapID
                        );
                }
                else if (eType == KAWARD_CARD_TYPE_VIP)
                {
                    PLAYER_LOG(
                        pPlayer, "item,additem,%u-%u,%d,%d,%s,%d",
                        pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint,
                        "vipaward", pPlayer->m_dwMapID
                    );
                }
            }
        }
        else if (pAwardItem->dwTabType == ittFashion)
        {
            bRetCode = pPlayer->m_Wardrobe.AddFashion(pAwardItem->dwIndex);
            KGLOG_CHECK_ERROR(bRetCode);
        }
        else if (pAwardItem->dwTabType == ittCheerleading)
        {
            KGCheerleadingInfo* pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo(pAwardItem->dwIndex);
            KGLOG_PROCESS_ERROR(pCheerleadingInfo);

            bRetCode = pPlayer->m_CheerleadingMgr.AddCheerleading(pCheerleadingInfo->dwID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else if (pAwardItem->dwTabType == ittTeamLogo)
        {
            KTEAM_LOGO_INFO*    pTeamLogoInfo = NULL;

            pTeamLogoInfo = g_pSO3World->m_ItemHouse.GetTeamLogoInfo(pAwardItem->dwIndex);
            KGLOG_PROCESS_ERROR(pTeamLogoInfo);

            bRetCode = pPlayer->AddTeamLogo((WORD)pTeamLogoInfo->nID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else if (pAwardItem->nMoney > 0)
        {
            bRetCode = pPlayer->m_MoneyMgr.AddMoney(pAwardItem->eMoneyType, pAwardItem->nMoney);
            KGLOG_PROCESS_ERROR(bRetCode);
            PLAYER_LOG(pPlayer, "money,addmoney,%d,%s,%d-%d,%d,%d", pAwardItem->eMoneyType, "awardtable", 0, 0, 0, pAwardItem->nMoney);


            BroadCastAwardMoney(pPlayer, pAwardItem);
        }

        memset(&AwardItem, 0, sizeof(AwardItem));

        pAwardItemInfo = &AwardItem;

        memset(pAwardItemInfo->RandomAttr, 0, sizeof(pAwardItemInfo->RandomAttr));

        bRetCode = GetAwardItemValuePoint(pAwardItem, nValuePoint);
        KGLOG_PROCESS_ERROR(bRetCode);

        pAwardItemInfo->itemType    = (unsigned char)pAwardItem->dwTabType;
        pAwardItemInfo->itemIndex   = pAwardItem->dwIndex;
        pAwardItemInfo->nCount      = (short)pAwardItem->nStackNum;
        pAwardItemInfo->byMoneyType = (BYTE)pAwardItem->eMoneyType;
        pAwardItemInfo->nMoney      = (short)pAwardItem->nMoney;
        pAwardItemInfo->nValuePoint = nValuePoint;

        if (pItem)
        {
            const KItemProperty*    pProperty   = pItem->GetProperty();
            KAttribute*			    pExtAttr    = pProperty->pExtAttr;  

            for (int j = 0; j < countof(pAwardItemInfo->RandomAttr); ++j)
            {
                if (pExtAttr == NULL)
                    break;

                pAwardItemInfo->RandomAttr[j].wKey = (unsigned short)pExtAttr->nKey;
                pAwardItemInfo->RandomAttr[j].wValue = (unsigned short)pExtAttr->nValue1;
                pExtAttr = pExtAttr->pNext;
            }
        }

        g_PlayerServer.DoAwardItemNotify(pPlayer, eType, AwardItem, uChooseIndex);
    }

    KG_PROCESS_SUCCESS(!bEndChoose);

    memset(arrAwardList, 0, sizeof(arrAwardList));
    for (unsigned i = 0; i < countof(arrAwardList); ++i)
    {
        pTempItem = NULL;
        pAwardItemInfo = &arrAwardList[i];

        memset(pAwardItemInfo->RandomAttr, 0, sizeof(pAwardItemInfo->RandomAttr));

        if (i != uChooseIndex)
        {
            const KItemProperty*    pProperty = NULL;
            KAttribute*			    pExtAttr  = NULL;

            pTempItem = pAwardTable->GetOneAwardItemByEqualRate();
            if (pTempItem == NULL)
                continue;

            bRetCode = GetAwardItemValuePoint(pTempItem, nValuePoint);
			KGLOG_PROCESS_ERROR(bRetCode);

            pAwardItemInfo->itemType    = (unsigned char)pTempItem->dwTabType;
            pAwardItemInfo->itemIndex   = pTempItem->dwIndex;
            pAwardItemInfo->nCount      = (short)pTempItem->nStackNum;
            pAwardItemInfo->byMoneyType = (BYTE)pTempItem->eMoneyType;
            pAwardItemInfo->nMoney      = (short)pTempItem->nMoney;
            pAwardItemInfo->nValuePoint = nValuePoint;

            if (pTempItem->dwTabType > 0 && pTempItem->dwIndex > 0)
            {
                pLookItem = g_pSO3World->m_ItemHouse.CreateItem(pTempItem->dwTabType, pTempItem->dwIndex, 0, 0, nValuePoint);
                KGLOG_CHECK_ERROR(pLookItem);
                if (pLookItem)
                {
                    pProperty   = pLookItem->GetProperty();
                    pExtAttr    = pProperty->pExtAttr;  

                    for (int j = 0; j < countof(pAwardItemInfo->RandomAttr); ++j)
                    {
                        if (pExtAttr == NULL)
                            break;

                        pAwardItemInfo->RandomAttr[j].wKey = (unsigned short)pExtAttr->nKey;
                        pAwardItemInfo->RandomAttr[j].wValue = (unsigned short)pExtAttr->nValue1;
                        pExtAttr = pExtAttr->pNext;
                    }

                    g_pSO3World->m_ItemHouse.DestroyItem(pLookItem);
                    pLookItem = NULL;
                }

            }
        }
    }

    g_PlayerServer.DoAwardItemListNotify(pPlayer, eType, arrAwardList, countof(arrAwardList));

Exit1:
    bResult = true;
Exit0:
    if (pLookItem)
    {
        g_pSO3World->m_ItemHouse.DestroyItem(pLookItem);
        pLookItem = NULL;
    }

    if (pItem)
    {
        g_pSO3World->m_ItemHouse.DestroyItem(pItem);
        pItem = NULL;
    }
    return bResult;
}

BOOL KAwardMgr::LoadAllAward()
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    ITabFile*	piTabFile   = NULL;
    DWORD       dwAwardID   = 0;
    char        szAwardFileName[MAX_PATH];
    char        szFilePath[MAX_PATH];
    std::pair<KAWARD_TABLE_MAP::iterator, bool> InsRet;
    KAwardTable* pAwardTable = NULL;

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, "award/award.tab");
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)&dwAwardID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetString(nRowIndex, "AwardItem", "", szAwardFileName, sizeof(szAwardFileName));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(szAwardFileName[0] != '\0');

        InsRet = m_AwardTable.insert(std::make_pair(dwAwardID, KAwardTable()));
        KGLOG_PROCESS_ERROR(InsRet.second);

        pAwardTable = &InsRet.first->second;

        bRetCode = LoadOneAward(szAwardFileName, pAwardTable);
        if (!bRetCode)
        {
            KGLogPrintf(KGLOG_DEBUG, "Load award file:%s failed!", szAwardFileName);
            goto Exit0;
        }
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KAwardMgr::LoadOneAward(const char cszAwardFile[], KAwardTable* pAwardTable)
{
    BOOL        bResult         = false;
    BOOL        bRetCode        = false;
    ITabFile*	piTabFile       = NULL;
    DWORD       dwAwardItemID   = 0;
    char        szFilePath[MAX_PATH];
    std::pair<KAWARD_ITEM_MAP::iterator, bool> InsRet;
    KAWARD_ITEM* pAwardItem = NULL;
    char        szValue[64];

    assert(cszAwardFile);
    assert(pAwardTable);

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s/%s", SETTING_DIR, "award", cszAwardFile);
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    strncpy(pAwardTable->m_szAwardTableName, szFilePath, sizeof(pAwardTable->m_szAwardTableName));
    pAwardTable->m_szAwardTableName[countof(pAwardTable->m_szAwardTableName) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)&dwAwardItemID);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(dwAwardItemID > 0);

        InsRet = pAwardTable->m_mapAwardItem.insert(std::make_pair(dwAwardItemID, KAWARD_ITEM()));
        KGLOG_PROCESS_ERROR(InsRet.second);

        pAwardItem = &InsRet.first->second;
        pAwardItem->dwID = dwAwardItemID;

        bRetCode = piTabFile->GetInteger(nRowIndex, "Tabtype", 0, (int*)&pAwardItem->dwTabType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TabIndex", 0, (int*)&pAwardItem->dwIndex);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Stacknum", 0, (int*)&pAwardItem->nStackNum);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MoneyType", emotMoney, (int*)&pAwardItem->eMoneyType);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(pAwardItem->eMoneyType == emotMoney || pAwardItem->eMoneyType == emotMenterPoint);
 
        bRetCode = piTabFile->GetInteger(nRowIndex, "Money", 0, (int*)&pAwardItem->nMoney);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BuffID", 0, (int*)&pAwardItem->dwBuffID);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AwardItemRate", 0, (int*)&pAwardItem->nAwardItemRate);
        KGLOG_PROCESS_ERROR(bRetCode);

        pAwardItem->nQualityLevel = -1;
        bRetCode = piTabFile->GetString(nRowIndex, "QualityLevel", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode); 
        szValue[countof(szValue) - 1] = '\0';
        if (bRetCode > 0)
        {
            bRetCode = ENUM_STR2INT(ENUM_EQUIP_QUALITY, szValue, pAwardItem->nQualityLevel);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        bRetCode = piTabFile->GetInteger(nRowIndex, "MinValuePoint", 0, &pAwardItem->nMinValuePoint);
        KGLOG_PROCESS_ERROR(bRetCode);
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "MaxValuePoint", 0, &pAwardItem->nMaxValuePoint);
        KGLOG_PROCESS_ERROR(bRetCode);
        
        KGLOG_PROCESS_ERROR(pAwardItem->nMaxValuePoint >= pAwardItem->nMinValuePoint && pAwardItem->nMinValuePoint >= 0);
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "IsBroadcast", 0, &pAwardItem->bIsBroadcast);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    pAwardTable->RatePreProcess();

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

KAwardTable* KAwardMgr::GetAwardTable(DWORD dwAwardID)
{
    KAwardTable* pResult = NULL;
    KAWARD_TABLE_MAP::iterator it = m_AwardTable.find(dwAwardID);
    KG_PROCESS_ERROR(it != m_AwardTable.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

DWORD KAwardMgr::GetAwardTableIDByName(const char(&cszFileName)[MAX_PATH])
{
    DWORD dwResult = ERROR_ID;
    KAwardTable* pAwardTable = NULL;
    for (KAWARD_TABLE_MAP::iterator it = m_AwardTable.begin(); it != m_AwardTable.end(); ++it)
    {
        pAwardTable = &it->second;
        if (strncmp(pAwardTable->m_szAwardTableName, cszFileName, sizeof(pAwardTable->m_szAwardTableName)))
            continue;

        dwResult = it->first;
        break;
    }

    return dwResult;
}

BOOL KAwardMgr::CanAwardAll(KPlayer* pPlayer, DWORD dwAwardTableID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KAwardTable* pAwardTable = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(dwAwardTableID);

    pAwardTable = GetAwardTable(dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    bRetCode = HasEnoughBagToAwardAll(pPlayer, dwAwardTableID);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KAwardMgr::AwardAll(KPlayer* pPlayer, DWORD dwAwardTableID, const char szWayID[])
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KAwardTable* pAwardTable = NULL;
    int nValuePoint = 0;

    assert(szWayID);

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(dwAwardTableID);

    pAwardTable = GetAwardTable(dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    bRetCode = HasEnoughBagToAwardAll(pPlayer, dwAwardTableID);
    KGLOG_PROCESS_ERROR(bRetCode);

    for (KAWARD_ITEM_MAP::iterator it = pAwardTable->m_mapAwardItem.begin(); it != pAwardTable->m_mapAwardItem.end(); ++it)
    {
        KAWARD_ITEM* pAwardItem = &it->second;
        if (pAwardItem->dwTabType == ittOther || pAwardItem->dwTabType == ittEquip)
        {
			IItem* pItem = NULL;

            bRetCode = GetAwardItemValuePoint(pAwardItem, nValuePoint);
            KGLOG_PROCESS_ERROR(bRetCode);

            pItem = pPlayer->AddItem(pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint);
            if (pItem)
            {
                PLAYER_LOG(
                    pPlayer, "item,additem,%u-%u,%d,%d,%d,%s,%d",
                    pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint, pItem->GetQuality(),
                    szWayID, pPlayer->m_dwMapID
                );
            }
        }
        else if (pAwardItem->dwTabType == ittFashion)
        {
            bRetCode = pPlayer->m_Wardrobe.AddFashion(pAwardItem->dwIndex);
            KGLOG_CHECK_ERROR(bRetCode);
        }
        else if (pAwardItem->dwTabType == ittCheerleading)
        {
            KGCheerleadingInfo* pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo(pAwardItem->dwIndex);
            KGLOG_PROCESS_ERROR(pCheerleadingInfo);

            bRetCode = pPlayer->m_CheerleadingMgr.AddCheerleading(pCheerleadingInfo->dwID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else if (pAwardItem->dwTabType == ittTeamLogo)
        {
            KTEAM_LOGO_INFO*    pTeamLogoInfo = NULL;

            pTeamLogoInfo = g_pSO3World->m_ItemHouse.GetTeamLogoInfo(pAwardItem->dwIndex);
            KGLOG_PROCESS_ERROR(pTeamLogoInfo);

            bRetCode = pPlayer->AddTeamLogo((WORD)pTeamLogoInfo->nID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else if (pAwardItem->nMoney > 0)
        {
            bRetCode = pPlayer->m_MoneyMgr.AddMoney(pAwardItem->eMoneyType, pAwardItem->nMoney);
            KGLOG_PROCESS_ERROR(bRetCode);

            PLAYER_LOG(pPlayer, "money,addmoney,%d,%s,%d-%d,%d,%d", pAwardItem->eMoneyType, szWayID, 0, 0, 0, pAwardItem->nMoney);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KAwardMgr::HasEnoughBagToAwardAll(KPlayer* pPlayer, DWORD dwAwardTableID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KAwardTable* pAwardTable = NULL;
    int nItemCount = 0;
    int nFreePosCount = 0;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(dwAwardTableID);

    pAwardTable = GetAwardTable(dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    nItemCount = pAwardTable->m_mapAwardItem.size();

    nFreePosCount = pPlayer->m_ItemList.GetPlayerFreeRoomSize(eppiPlayerItemBox);
    KG_PROCESS_ERROR(nFreePosCount >= nItemCount);

    bResult = true;
Exit0:
    return bResult;
}

KAWARD_ITEM*  KAwardMgr::AwardRandomOne(KPlayer* pPlayer, DWORD dwAwardTableID)
{
    BOOL            bRetCode        = false;
    IItem*          pItem           = NULL;
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;
    int             nValuePoint = 0;

    assert(pPlayer);

    bRetCode = HasEnoughBagToAwardRandomOne(pPlayer, dwAwardTableID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pAwardTable = GetAwardTable(dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    pAwardItem = pAwardTable->GetRandomAward();
    if (pAwardItem)
    {
        if (pAwardItem->dwTabType == ittOther || pAwardItem->dwTabType == ittEquip)
        {
            bRetCode = GetAwardItemValuePoint(pAwardItem, nValuePoint);
            KGLOG_PROCESS_ERROR(bRetCode);

            pItem = pPlayer->AddItem(pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint);
            if (pItem)
            {
                PLAYER_LOG(
                    pPlayer, "item,additem,%u-%u,%d,%d,%d,%s,%d",
                    pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint, pItem->GetQuality(),
                    "useitem", pPlayer->m_dwMapID
                );
            }
        }
        else if (pAwardItem->dwTabType == ittFashion)
        {
            bRetCode = pPlayer->m_Wardrobe.AddFashion(pAwardItem->dwIndex);
            KGLOG_CHECK_ERROR(bRetCode);
        }
        else if (pAwardItem->dwTabType == ittCheerleading)
        {
            KGCheerleadingInfo* pCheerleadingInfo = g_pSO3World->m_ItemHouse.GetCheerleadingInfo(pAwardItem->dwIndex);
            KGLOG_PROCESS_ERROR(pCheerleadingInfo);

            bRetCode = pPlayer->m_CheerleadingMgr.AddCheerleading(pCheerleadingInfo->dwID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else if (pAwardItem->dwTabType == ittTeamLogo)
        {
            KTEAM_LOGO_INFO*    pTeamLogoInfo = NULL;

            pTeamLogoInfo = g_pSO3World->m_ItemHouse.GetTeamLogoInfo(pAwardItem->dwIndex);
            KGLOG_PROCESS_ERROR(pTeamLogoInfo);

            bRetCode = pPlayer->AddTeamLogo((WORD)pTeamLogoInfo->nID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else if (pAwardItem->nMoney > 0)
        {
            bRetCode = pPlayer->m_MoneyMgr.AddMoney(pAwardItem->eMoneyType, pAwardItem->nMoney);
            KGLOG_PROCESS_ERROR(bRetCode);
            PLAYER_LOG(pPlayer, "money,addmoney,%d,%s,%d-%d,%d,%d", pAwardItem->eMoneyType, "useitem", 0, 0, 0, pAwardItem->nMoney);
        }
    }

Exit0:
    return pAwardItem;
}

KAWARD_ITEM* KAwardMgr::AwardRandomOne(DWORD dwAwardTableID)
{
    KAWARD_ITEM*    pResult         = NULL;
    BOOL            bRetCode        = false;
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;

    pAwardTable = GetAwardTable(dwAwardTableID);
    KG_PROCESS_ERROR(pAwardTable);

    pAwardItem = pAwardTable->GetRandomAward();
    KG_PROCESS_ERROR(pAwardItem);

    pResult = pAwardItem;
Exit0:
    return pResult;
}

KAWARD_ITEM* KAwardMgr::GetAwardItem(DWORD dwAwardTableID, int nItemIndex)
{
    KAWARD_ITEM*    pResult         = NULL;
    BOOL            bRetCode        = false;
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;

    pAwardTable = GetAwardTable(dwAwardTableID);
    KG_PROCESS_ERROR(pAwardTable);

    pAwardItem = pAwardTable->GetAwardItem(nItemIndex);
    KG_PROCESS_ERROR(pAwardItem);

    pResult = pAwardItem;
Exit0:
    return pResult;
}

BOOL KAwardMgr::GetAwardItems(DWORD dwAwardTableID, int& nItemCount, KAWARD_ITEM* pRetAwardItem[])
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;
    int             nCount          = 0;

    assert(nItemCount > 0);
    assert(pRetAwardItem);

    pAwardTable = GetAwardTable(dwAwardTableID);
    KG_PROCESS_ERROR(pAwardTable);

    for (KAWARD_ITEM_MAP::iterator it = pAwardTable->m_mapAwardItem.begin(); it != pAwardTable->m_mapAwardItem.end(); ++it)
    {
        ++nCount;
        pRetAwardItem[nCount - 1] = &it->second;

        if (nCount == nItemCount)
            break;
    }

    nItemCount = nCount;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KAwardMgr::HasEnoughBagToAwardRandomOne(KPlayer* pPlayer, DWORD dwAwardTableID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KAwardTable* pAwardTable = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(dwAwardTableID);

    pAwardTable = GetAwardTable(dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    for (KAWARD_ITEM_MAP::iterator it = pAwardTable->m_mapAwardItem.begin(); it != pAwardTable->m_mapAwardItem.end(); ++it)
    {
        KAWARD_ITEM* pAwardItem = &it->second;
        if (pAwardItem->dwTabType == 0)
            continue;

        bRetCode = pPlayer->HasFreePosAddItem(pAwardItem->dwTabType, pAwardItem->dwIndex);
        KG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KAwardMgr::GetRoomSizeToAwardRandomOne(DWORD dwAwardTableID, int(&RoomSize)[eppiTotal])
{
	BOOL            bResult         = false;
	BOOL            bRetCode        = false;
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;
    KGItemInfo*     pItemInfo       = NULL;
    int             nPackageIndex   = 0;
	
    memset(RoomSize, 0, sizeof(RoomSize));
    KG_PROCESS_SUCCESS(dwAwardTableID == ERROR_ID);

    pAwardTable = GetAwardTable(dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    for (KAWARD_ITEM_MAP::iterator it = pAwardTable->m_mapAwardItem.begin(); it != pAwardTable->m_mapAwardItem.end(); ++it)
    {
        pAwardItem = &it->second;

        if (pAwardItem->dwTabType == 0)
            continue;

        if (pAwardItem->nAwardItemRate == 0)
            continue;

        RoomSize[eppiPlayerItemBox] = 1;
    }
	
Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KAwardMgr::GetRoomSizeToAwardAll(DWORD dwAwardTableID, int(&RoomSize)[eppiTotal])
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KAwardTable*    pAwardTable = NULL;
    KAWARD_ITEM*    pAwardItem  = NULL;
    int             nPackageIndex   = 0;

    memset(RoomSize, 0, sizeof(RoomSize));
    KG_PROCESS_SUCCESS(dwAwardTableID == ERROR_ID);

    pAwardTable = GetAwardTable(dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardTable);

    for (KAWARD_ITEM_MAP::iterator it = pAwardTable->m_mapAwardItem.begin(); it != pAwardTable->m_mapAwardItem.end(); ++it)
    {
        pAwardItem = &it->second;

        if (pAwardItem->dwTabType == 0)
            continue;

        RoomSize[eppiPlayerItemBox] += 1;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KAwardMgr::GetAwardItemValuePoint(KAWARD_ITEM* pAwardItem, int& nValuePoint)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KGItemInfo* pItemInfo = NULL;

	KGLOG_PROCESS_ERROR(pAwardItem);

    nValuePoint = 0;

    KG_PROCESS_SUCCESS(pAwardItem->dwTabType != ittEquip);

    if (pAwardItem->nQualityLevel != -1)
    {
        pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(pAwardItem->dwTabType, pAwardItem->dwIndex);
        KGLOG_PROCESS_ERROR(pItemInfo);

        nValuePoint = g_pSO3World->m_ItemHouse.GetRandomValuePoint(pItemInfo->nLevel, pAwardItem->nQualityLevel);
    }
    else
    {
        int nDeltaValuePoint = pAwardItem->nMaxValuePoint - pAwardItem->nMinValuePoint;
        KGLOG_PROCESS_ERROR(nDeltaValuePoint >= 0);

        nValuePoint = pAwardItem->nMinValuePoint;
        if (nDeltaValuePoint > 0)
            nValuePoint += (int)g_Random(nDeltaValuePoint);
    }

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

void KAwardMgr::BroadCastAwardItem(KPlayer* pPlayer, KAWARD_ITEM* pAwardItem)
{
    assert(pPlayer);
    assert(pAwardItem);

    KGItemInfo* pItemInfo = NULL;

    pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(pAwardItem->dwTabType, pAwardItem->dwIndex);
    KGLOG_PROCESS_ERROR(pItemInfo);
    KG_PROCESS_ERROR(pAwardItem->bIsBroadcast);
    switch (pPlayer->m_nLastBattleType)
    {
        case KBATTLE_TYPE_LADDER_PVP:
            {
                KMessage86_Para cParam;
                memcpy(cParam.szPlayerName, pPlayer->m_szName, sizeof(cParam.szPlayerName));
                memcpy(cParam.szItemName, pItemInfo->szName, sizeof(cParam.szItemName));
                cParam.nCount = pAwardItem->nStackNum;
                
                if (cParam.nCount > 1)
                    g_LSClient.DoBroadcastMessage(KMESSAGE_PVP_AWARD_ITEMS_BROADCAST, &cParam, sizeof(KMessage86_Para));
                else
                    g_LSClient.DoBroadcastMessage(KMESSAGE_PVP_AWARD_ITEM_BROADCAST, &cParam, sizeof(KMessage85_Para));
            }
            break;
        case KBATTLE_TYPE_MISSION:
            {
                KMessage88_Para cParam;
                memcpy(cParam.szPlayerName, pPlayer->m_szName, sizeof(cParam.szPlayerName));
                memcpy(cParam.szItemName, pItemInfo->szName, sizeof(cParam.szItemName));
                memcpy(cParam.szMissionName, pPlayer->m_szLastMissionName, sizeof(cParam.szMissionName));
                cParam.nLevel = pPlayer->m_nLastMissionLevel;
                cParam.nCount = pAwardItem->nStackNum;
                if (cParam.nCount > 1)
                    g_LSClient.DoBroadcastMessage(KMESSAGE_PVE_AWARD_ITEMS_BROADCAST, &cParam, sizeof(KMessage88_Para));
                else
                    g_LSClient.DoBroadcastMessage(KMESSAGE_PVE_AWARD_ITEM_BROADCAST, &cParam, sizeof(KMessage87_Para));
            }
            break;
        default:
            break;
    }

Exit0:
    return;
}
 
void KAwardMgr::BroadCastAwardMoney(KPlayer* pPlayer, KAWARD_ITEM* pAwardItem)
{
    assert(pPlayer);
    assert(pAwardItem);

    KG_PROCESS_ERROR(pAwardItem->bIsBroadcast);
    switch (pPlayer->m_nLastBattleType)
    {
    case KBATTLE_TYPE_LADDER_PVP:
        {
            KMessage89_Para cParam;
            memcpy(cParam.szPlayerName, pPlayer->m_szName, sizeof(cParam.szPlayerName));
            cParam.nCount = pAwardItem->nMoney;
            g_LSClient.DoBroadcastMessage(KMESSAGE_PVP_AWARD_MONEY_BROADCAST, &cParam, sizeof(cParam));
        }
        break;
    case KBATTLE_TYPE_MISSION:
        {
            KMessage90_Para cParam;
            memcpy(cParam.szPlayerName, pPlayer->m_szName, sizeof(cParam.szPlayerName));
            memcpy(cParam.szMissionName, pPlayer->m_szLastMissionName, sizeof(cParam.szMissionName));
            cParam.nLevel = pPlayer->m_nLastMissionLevel;
            cParam.nCount = pAwardItem->nMoney;
            g_LSClient.DoBroadcastMessage(KMESSAGE_PVE_AWARD_MONEY_BROADCAST, &cParam, sizeof(cParam));
        }
        break;
    default:
        break;
    }

Exit0:
    return;
}
