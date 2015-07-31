#include "stdafx.h"
#include "KPlayer.h"
#include "Luna.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KRelayClient.h"
#include "KLSClient.h"
#include "KScene.h"


int KPlayer::LuaTestReport(Lua_State* L)
{
    BOOL        bRetCode            = false;
    int         nTopIndex           = 0;
    DWORD       dwNum               = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwNum = (DWORD)Lua_ValueToNumber(L, 1);

    while(dwNum--)
    {    
       m_Secretary.AddReport(KREPORT_EVENT_MONEY_OUT_OF_LIMIT, NULL, 0);
    }

Exit0:
    return 0;
}

int KPlayer::LuaAddNewHero(Lua_State* L)
{
    BOOL        bRetCode            = false;
    int         nTopIndex           = 0;
    DWORD       dwHeroTemplateID    = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = AddNewHero(dwHeroTemplateID, 1);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaDelHero(Lua_State* L)
{
    BOOL        bRetCode            = false;
    int         nTopIndex           = 0;
    DWORD       dwHeroTemplateID    = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = DelHero(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaSetMailRead(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwMailID = 0;
    KGLOG_PROCESS_ERROR(nParamCount == 1);
    dwMailID = (DWORD)Lua_ValueToNumber(L, 1);
    g_LSClient.DoSetMailRead(m_dwID,dwMailID);
Exit0:
    return 0;
}

int KPlayer::LuaDelMail(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwMailID = 0;
    KGLOG_PROCESS_ERROR(nParamCount == 1);
    dwMailID = (DWORD)Lua_ValueToNumber(L, 1);
    g_LSClient.DoDeleteMail(m_dwID,dwMailID);
Exit0:
    return 0;
}

int KPlayer::LuaGetMailAll(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwMailID = 0;
    KGLOG_PROCESS_ERROR(nParamCount == 1);
    dwMailID = (DWORD)Lua_ValueToNumber(L, 1);
    g_LSClient.DoAcquireMailAllRequest(m_dwID,dwMailID);
Exit0:
    return 0;
}

int KPlayer::LuaSeeMail(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwMailID = 0;
    KGLOG_PROCESS_ERROR(nParamCount == 1);
    dwMailID = (DWORD)Lua_ValueToNumber(L, 1);
    g_LSClient.DoQueryMailContent(m_dwID,dwMailID,eMailType_System);
Exit0:
    return 0;
}

int KPlayer::LuaGetMailList(Lua_State* L)
{
    g_LSClient.DoGetMailListRequest(m_dwID, 0);
    return 0;
}

int KPlayer::LuaMailItem(Lua_State* L)
{
    BOOL                    bResult         = false;
    DWORD                   dwTabType       = 0;
    DWORD                   dwIndex         = 0;
    int                     nStackNum       = 0;
    KGItemInfo*             pItemInfo       = NULL;
    int                     nValuePoint     = 0;
    IItem*                  pItem           = NULL;

    int nParamCount = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(
        nParamCount == 2 || nParamCount == 3 || nParamCount == 4
        );

    dwTabType   = (DWORD)Lua_ValueToNumber(L, 1);
    dwIndex     = (DWORD)Lua_ValueToNumber(L, 2);

    pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(dwTabType, dwIndex);
    KGLOG_PROCESS_ERROR(pItemInfo);

    nStackNum = pItemInfo->nMaxStackNum;
    if (nParamCount >= 3)
        nStackNum = (int)Lua_ValueToNumber(L, 3);

    if (nParamCount == 4)
        nValuePoint = (int)Lua_ValueToNumber(L, 4);

    AddItemByMail(dwTabType, dwIndex, nStackNum, nValuePoint, "test", "system", "test send item by mail");

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaSetDescript(Lua_State* L)
{
    int     nParamCount = Lua_GetTopIndex(L);
    int     nType       = 0;
    char*   pszText     = NULL;
    
    KGLOG_PROCESS_ERROR(nParamCount == 2);
	
	KGLOG_PROCESS_ERROR(m_dwClubID);
	
    nType = (int)Lua_ValueToNumber(L, 1);
    pszText = (char*)Lua_ValueToString(L, 2);
	
	KGLOG_PROCESS_ERROR(pszText);
	
    g_LSClient.DoSetDescriptRequest(
		m_dwID, m_dwClubID, nType, strlen(pszText), pszText
	);

Exit0:
    return 0;
}

int KPlayer::LuaSetPost(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwPlayerID = 0;
    int   nPost = 0;
    KGLOG_PROCESS_ERROR(m_dwClubID);
    KGLOG_PROCESS_ERROR(nParamCount == 2);
    dwPlayerID = (DWORD)Lua_ValueToNumber(L, 1);
    nPost = (DWORD)Lua_ValueToNumber(L, 2);

    g_LSClient.DoClubSetPostRequest(m_dwID, m_dwClubID, dwPlayerID, nPost);

Exit0:
    return 0;
}

int KPlayer::LuaInvite(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    char*   pszName = NULL;

    KGLOG_PROCESS_ERROR(nParamCount == 1);
    pszName = (char*)Lua_ValueToString(L, 1);

    g_LSClient.DoClubInviteRequest(m_dwID, m_dwClubID, pszName);

Exit0:
    return 0;
}

int KPlayer::LuaAccept(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwPlayerID = 0;

    KGLOG_PROCESS_ERROR(m_dwClubID);
    KGLOG_PROCESS_ERROR(nParamCount == 1);
    dwPlayerID = (DWORD)Lua_ValueToNumber(L, 1);
    g_LSClient.DoClubAcceptApplyRequest(m_dwID ,m_dwClubID, dwPlayerID, true);

Exit0:
    return 0;
}

int KPlayer::LuaApply(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwClubID = 0;

    if (m_dwClubID != 0)
    {
        return 0; 
    }
    KGLOG_PROCESS_ERROR(nParamCount == 1);
    dwClubID = (DWORD)Lua_ValueToNumber(L, 1);
    g_LSClient.DoApplyJoinClubRequest(m_dwID, dwClubID);

    ++m_nClubApplyNum;

Exit0:
    return 0;
}

int KPlayer::LuaMemberList(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwVersion = 0;

    KGLOG_PROCESS_ERROR(nParamCount == 1);
    dwVersion = (DWORD)Lua_ValueToNumber(L, 1);
    g_LSClient.DoApplyClubMemberList(m_dwID, m_dwClubID, dwVersion);
Exit0:
    return 0;
}

int KPlayer::LuaCreateClub(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    char*   pszName = NULL;

    KGLOG_PROCESS_ERROR(nParamCount == 1);
    pszName = (char*)Lua_ValueToString(L, 1);
    g_LSClient.DoCreateClubRequest(m_dwID, pszName);
Exit0:
    return 0;
}

int KPlayer::LuaDelMember(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwPlayerID = 0;

    KGLOG_PROCESS_ERROR(nParamCount == 1);
    dwPlayerID = (DWORD)Lua_ValueToNumber(L, 1);
    g_LSClient.DoDelMemberRequest(m_dwID, m_dwClubID, dwPlayerID);
Exit0:
    return 0;
}

int KPlayer::LuaRandomClub(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    DWORD dwPlayerID = 0;

    g_LSClient.DoRandomClubRequest(m_dwID);
Exit0:
    return 0;
}

int KPlayer::LuaAddActive(Lua_State* L)
{
    int nParamCount = Lua_GetTopIndex(L);
    int nActivePoint = 0;

    KGLOG_PROCESS_ERROR(nParamCount == 1);
    nActivePoint = (int)Lua_ValueToNumber(L, 1);
    m_ActivePlayer.AddActivePoint(nActivePoint);
Exit0:
    return 0;
}

int KPlayer::LuaAddItem(Lua_State* L)
{
    BOOL                    bResult         = false;
    DWORD                   dwTabType       = 0;
    DWORD                   dwIndex         = 0;
    int                     nStackNum       = 0;
    KGItemInfo*             pItemInfo       = NULL;
    int                     nValuePoint     = 0;
    IItem*                  pItem           = NULL;

    int nParamCount = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(
        nParamCount == 2 || nParamCount == 3 || nParamCount == 4
    );

    dwTabType   = (DWORD)Lua_ValueToNumber(L, 1);
    dwIndex     = (DWORD)Lua_ValueToNumber(L, 2);

    pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(dwTabType, dwIndex);
    KGLOG_PROCESS_ERROR(pItemInfo);

    nStackNum = pItemInfo->nMaxStackNum;
    if (nParamCount >= 3)
        nStackNum = (int)Lua_ValueToNumber(L, 3);

    if (nParamCount == 4)
        nValuePoint = (int)Lua_ValueToNumber(L, 4);

    pItem = AddItem(dwTabType, dwIndex, nStackNum, nValuePoint);
    if (pItem)
    {
        PLAYER_LOG(
            this, "item,additem,%u-%u,%d,%d,%d,%s,%d",
            dwTabType, dwIndex, nStackNum, nValuePoint, pItem->GetQuality(),
            "scriptadd", m_dwMapID
        );
    }

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaDestroyItem(Lua_State* L)
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    int     nPackageType    = 0;
    int     nPackageIndex   = 0;
    int     nPos            = 0;

    KG_PROCESS_ERROR(Lua_GetTopIndex(L) == 3);

    nPackageType    = (int)Lua_ValueToNumber(L, 1);
    nPackageIndex   = (int)Lua_ValueToNumber(L, 2);
    nPos            = (int)Lua_ValueToNumber(L, 3);

    bRetCode = m_ItemList.DestroyItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaCostItem(Lua_State* L)
{
    BOOL        bResult       = false;
    BOOL        bRetCode      = false;
    DWORD       dwType	      = 0;
    DWORD       dwIndex	      = 0;
    int         nAmount	      = 0;
    int         nTopIndex     = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    dwType	        = (DWORD)Lua_ValueToNumber(L, 1);
    dwIndex	        = (DWORD)Lua_ValueToNumber(L, 2);
    nAmount 	    = (int)Lua_ValueToNumber(L, 3);

    bRetCode = m_ItemList.CostPlayerItem(dwType, dwIndex, nAmount);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaExchangeItem(Lua_State* L)
{
    BOOL        bResult             = false;
    BOOL        bRetCode            = false;
    int         nSrcPackageType     = 0;
    int         nSrcPackageIndex    = 0;
    int         nSrcPos             = 0;
    int         nDestPackageType    = 0;
    int         nDestPackageIndex   = 0;
    int         nDestPos            = 0;
    int         nTopIndex           = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(nTopIndex == 6);

    nSrcPackageType     = (int)Lua_ValueToNumber(L, 1);
    nSrcPackageIndex    = (int)Lua_ValueToNumber(L, 2);
    nSrcPos	            = (int)Lua_ValueToNumber(L, 3);
    nDestPackageType    = (int)Lua_ValueToNumber(L, 4);
    nDestPackageIndex	= (int)Lua_ValueToNumber(L, 5);
    nDestPos 	        = (int)Lua_ValueToNumber(L, 6);

    bRetCode = m_ItemList.ExchangeItem(
        nSrcPackageType, nSrcPackageIndex, nSrcPos, nDestPackageType, nDestPackageIndex, nDestPos
        );
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaAddMoney(Lua_State* L)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    int             nAddMoney   = 0;
    ENUM_MONEY_TYPE eMoneyType  = emotBegin;
    int             nTopIndex   = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    eMoneyType  = (ENUM_MONEY_TYPE)(int)Lua_ValueToNumber(L, 1);
    nAddMoney   = (int)Lua_ValueToNumber(L, 2);

    KGLOG_PROCESS_ERROR(eMoneyType != emotCoin);

    bRetCode    = m_MoneyMgr.AddMoney(eMoneyType, nAddMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (nAddMoney > 0)
        PLAYER_LOG(this, "money,addmoney,%d,%s,%d-%d,%d,%d", eMoneyType, "scriptadd", 0, 0, 0, nAddMoney);
    else
        PLAYER_LOG(this, "money,costmoney,%d,%s,%d-%d,%d,%d", eMoneyType, "scriptadd", 0, 0, 0, -nAddMoney);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaGetFightingHero(Lua_State* L)
{
    int     nRet    = 0;
    KHero*  pHero   = NULL;

    pHero = GetFightingHero();
    KG_PROCESS_ERROR(pHero);

    nRet = pHero->LuaGetObj(L);

Exit0:
    return nRet;
}

int KPlayer::LuaRename(Lua_State* L)
{
    int             nTopIndex   = Lua_GetTopIndex(L);
    char*           pszNewName  = NULL;             

    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    pszNewName  = (char*)Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszNewName);
    KGLOG_PROCESS_ERROR(m_bCanRename);

    g_RelayClient.DoRenameRequest(m_dwID, pszNewName);

Exit0:
    return 0;
}

int KPlayer::LuaSwitchMap(Lua_State* L)
{
    int             nTopIndex   = Lua_GetTopIndex(L);
    DWORD           dwMapID     = 0;
    int             nCopyIndex  = 0;

    KGLOG_PROCESS_ERROR(nTopIndex == 1 || nTopIndex == 2);

    dwMapID = (DWORD)Lua_ValueToNumber(L, 1);

    if (nTopIndex == 2)
        nCopyIndex = (int)Lua_ValueToNumber(L, 2);

    SwitchMap(dwMapID, nCopyIndex);

Exit0:
    return 0;
}

int KPlayer::LuaSendSystemMail(Lua_State* L)
{
    BOOL                    bRetCode        = false;
    int                     nSendMoney      = 0;
    int                     nParamCount     = Lua_GetTopIndex(L);
    int                     nMailTxtID      = 0;
    DWORD                   dwTabType       = 0;
    DWORD                   dwIndex         = 0;
    int                     nStackNum       = 0;
    int                     nValuePoint     = 0;
    KMail*                  pMail           = NULL;
    uint32_t                uMailLen        = 0;
    IItem*                  piItem          = NULL;
    const KItemProperty*	pItemProperty	= NULL;
    IItem*                  pItem[KMAIL_MAX_ITEM_COUNT]        = {0};
    int                     nItemCount      = 0;
    KSystemMailTxt*         pMailTxt        = NULL;

    KGLOG_PROCESS_ERROR(nParamCount >= 1);

    nMailTxtID = (int)Lua_ValueToNumber(L, 1);

    if (nParamCount >= 2)
        nSendMoney = (int)Lua_ValueToNumber(L, 2);
    if (nParamCount >= 3)
        dwTabType = (DWORD)Lua_ValueToNumber(L, 3);
    if (nParamCount >= 4)
        dwIndex = (DWORD)Lua_ValueToNumber(L, 4);
    if (nParamCount >= 5)
        nStackNum = (int)Lua_ValueToNumber(L, 5);
    if (nParamCount == 6)
        nValuePoint = (int)Lua_ValueToNumber(L, 6);

    if (dwTabType != 0)
    {
        piItem = g_pSO3World->m_ItemHouse.CreateItem(dwTabType, dwIndex, 0, 0, nValuePoint);
        KGLOG_PROCESS_ERROR(piItem);

        pItemProperty = piItem->GetProperty();
        if (piItem->CanStack())
        {
            if (nStackNum > 0)
                nStackNum = MIN(nStackNum, piItem->GetMaxStackNum());
            else
                nStackNum = piItem->GetMaxStackNum();

            piItem->SetStackNum(nStackNum);
        }
        else
        {
            piItem->SetCurrentDurability(pItemProperty->pItemInfo->nMaxDurability);
        }
        pItem[0] = piItem;

        nItemCount = 1;
    }

    pMail = (KMail*)s_byTempData;
    memset(pMail, 0, sizeof(KMail));

    pMailTxt = g_pSO3World->m_Settings.m_SystemMailMgr.GetSystemMailTxt(nMailTxtID);
    KGLOG_PROCESS_ERROR(pMailTxt);

    bRetCode = g_pSO3World->FillMail((KMAIL_TYPE)pMailTxt->m_nMailType, pMailTxt->m_szSendName, pMailTxt->m_szMailTitl, pMailTxt->m_szMailTxt, strlen(pMailTxt->m_szMailTxt), pItem, nItemCount, pMail, uMailLen);
    pMail->nMoney = nSendMoney;
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_LSClient.DoSendMailRequest(0, m_szName, pMail, uMailLen);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
    {
        if (pItem[i])
        {
            g_pSO3World->m_ItemHouse.DestroyItem(pItem[i]);
            pItem[i] = NULL;
        }
    }
    return 0;
}

int KPlayer::LuaSendMail(Lua_State* L)
{
    BOOL                    bRetCode        = false;
    int                     nSendMoney      = 0;
    int                     nParamCount     = Lua_GetTopIndex(L);
    char*                   pszDstName      = NULL;
    char*                   pszTitle        = NULL;
    char*                   pszText         = NULL;
    DWORD                   dwTabType       = 0;
    DWORD                   dwIndex         = 0;
    int                     nStackNum       = 0;
    int                     nValuePoint     = 0;
    KMail*                  pMail           = NULL;
    uint32_t                uMailLen        = 0;
    IItem*                  piItem          = NULL;
    const KItemProperty*	pItemProperty	= NULL;
    IItem*                  pItem[KMAIL_MAX_ITEM_COUNT]        = {0};
    int                     nItemCount      = 0;

    KGLOG_PROCESS_ERROR(nParamCount >= 3);

    pszDstName = (char*)Lua_ValueToString(L, 1);
    pszTitle = (char*)Lua_ValueToString(L, 2);
    pszText = (char*)Lua_ValueToString(L, 3);

    if (nParamCount >= 4)
        nSendMoney = (int)Lua_ValueToNumber(L, 4);
    if (nParamCount >= 5)
        dwTabType = (DWORD)Lua_ValueToNumber(L, 5);
    if (nParamCount >= 6)
        dwIndex = (DWORD)Lua_ValueToNumber(L, 6);
    if (nParamCount >= 7)
        nStackNum = (int)Lua_ValueToNumber(L, 7);
    if (nParamCount == 8)
        nValuePoint = (int)Lua_ValueToNumber(L, 8);

    if (dwTabType != 0)
    {
        piItem = g_pSO3World->m_ItemHouse.CreateItem(dwTabType, dwIndex, 0, 0, nValuePoint);
        KGLOG_PROCESS_ERROR(piItem);

        pItemProperty = piItem->GetProperty();
        if (piItem->CanStack())
        {
            if (nStackNum > 0)
                nStackNum = MIN(nStackNum, piItem->GetMaxStackNum());
            else
                nStackNum = piItem->GetMaxStackNum();

            piItem->SetStackNum(nStackNum);
        }
        else
        {
            piItem->SetCurrentDurability(pItemProperty->pItemInfo->nMaxDurability);
        }
        pItem[0] = piItem;

        nItemCount = 1;
    }

    pMail = (KMail*)s_byTempData;
    memset(pMail, 0, sizeof(KMail));

    bRetCode = g_pSO3World->FillMail(eMailType_System, m_szName, pszTitle, pszText, strlen(pszText), pItem, nItemCount, pMail, uMailLen);
    pMail->nMoney = nSendMoney;
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_LSClient.DoSendMailRequest(0, pszDstName, pMail, uMailLen);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
    {
        if (pItem[i])
        {
            g_pSO3World->m_ItemHouse.DestroyItem(pItem[i]);
            pItem[i] = NULL;
        }
    }
    return 0;
}

int KPlayer::LuaSendMailToSelf(Lua_State* L)
{
    BOOL                    bRetCode        = false;
    int                     nSendMoney      = 0;
    int                     nParamCount     = Lua_GetTopIndex(L);
    char*                   pszSenderName   = NULL;
    char*                   pszTitle        = NULL;
    char*                   pszText         = NULL;
    DWORD                   dwTabType       = 0;
    DWORD                   dwIndex         = 0;
    int                     nStackNum       = 0;
    int                     nValuePoint     = 0;
    KMail*                  pMail           = NULL;
    uint32_t                uMailLen        = 0;
    IItem*                  piItem          = NULL;
    const KItemProperty*	pItemProperty	= NULL;
    IItem*                  pItem[KMAIL_MAX_ITEM_COUNT]        = {0};
    int                     nItemCount      = 0;

    KGLOG_PROCESS_ERROR(nParamCount >= 3);

    pszSenderName = (char*)Lua_ValueToString(L, 1);
    pszTitle = (char*)Lua_ValueToString(L, 2);
    pszText = (char*)Lua_ValueToString(L, 3);

    if (nParamCount >= 4)
        nSendMoney = (int)Lua_ValueToNumber(L, 4);
    if (nParamCount >= 5)
        dwTabType = (DWORD)Lua_ValueToNumber(L, 5);
    if (nParamCount >= 6)
        dwIndex = (DWORD)Lua_ValueToNumber(L, 6);
    if (nParamCount >= 7)
        nStackNum = (int)Lua_ValueToNumber(L, 7);
    if (nParamCount == 8)
        nValuePoint = (int)Lua_ValueToNumber(L, 8);

    if (dwTabType != 0)
    {
        piItem = g_pSO3World->m_ItemHouse.CreateItem(dwTabType, dwIndex, 0, 0, nValuePoint);
        KGLOG_PROCESS_ERROR(piItem);

        pItemProperty = piItem->GetProperty();
        if (piItem->CanStack())
        {
            if (nStackNum > 0)
                nStackNum = MIN(nStackNum, piItem->GetMaxStackNum());
            else
                nStackNum = piItem->GetMaxStackNum();

            piItem->SetStackNum(nStackNum);
            pItem[0] = piItem;
            nItemCount = 1;
        }
        else
        {
            for (int i = 0; i < nStackNum && i < KMAIL_MAX_ITEM_COUNT; ++i)
            {
                ++nItemCount;
                if (piItem == NULL)
                {
                    piItem = g_pSO3World->m_ItemHouse.CreateItem(dwTabType, dwIndex, 0, 0, nValuePoint);
                    KGLOG_PROCESS_ERROR(piItem);
                }
                piItem->SetCurrentDurability(pItemProperty->pItemInfo->nMaxDurability);
                pItem[i] = piItem;
                piItem = NULL;
            }
        }

    }

    pMail = (KMail*)s_byTempData;
    memset(pMail, 0, sizeof(KMail));

    bRetCode = g_pSO3World->FillMail(eMailType_System, pszSenderName, pszTitle, pszText, strlen(pszText), pItem, nItemCount, pMail, uMailLen);
    pMail->nMoney = nSendMoney;
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_LSClient.DoSendMailRequest(0, m_szName, pMail, uMailLen);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
    {
        if (pItem[i])
        {
            g_pSO3World->m_ItemHouse.DestroyItem(pItem[i]);
            pItem[i] = NULL;
        }
    }
    return 0;
}

int KPlayer::LuaAddMaxHeroCount(Lua_State* L)
{
    BOOL            bRetCode        = false;
    int             nTopIndex       = Lua_GetTopIndex(L);
    int             nAddHeroCount   = 0;

    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nAddHeroCount = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nAddHeroCount > 0);

    bRetCode = m_HeroDataList.AddMaxHeroCount(nAddHeroCount);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}


int KPlayer::LuaSelectMainHero(Lua_State* L)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nTopIndex   = 0;
    DWORD   dwHeroTemplateID = ERROR_ID;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);
    bRetCode = SelectMainHero(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_RelayClient.DoSelectHeroRequest(this);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaSelectAssistHero(Lua_State* L)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nTopIndex   = 0;
    DWORD   dwHeroTemplateID = ERROR_ID;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);
    bRetCode = SelectAssistHero(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_RelayClient.DoSelectHeroRequest(this);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaEnterMission(Lua_State* L)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    int         nTopIndex   = 0;
    int         nMissionType  = 0;
    int         nMissionLevel = 0;
    int         nMissionStep  = 0;
    uint32_t    dwMapID     = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nMissionType = (int)Lua_ValueToNumber(L, 1);
    nMissionStep = (int)Lua_ValueToNumber(L, 2);
    nMissionLevel = (int)Lua_ValueToNumber(L, 3);

    bRetCode = EnterMission(nMissionType, nMissionStep, nMissionLevel);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaLeaveMission(Lua_State* L)
{
    BOOL        bRetCode    = false;
    KHero*      pHero       = NULL;
    KMISSION_AWARD_DATA awardData[MAX_TEAM_MEMBER_COUNT];
    KHero* teammates[MAX_TEAM_MEMBER_COUNT] = {NULL};
    int  nScore[MAX_TEAM_MEMBER_COUNT] = {0, 0};
    BOOL bHasWard[KAWARD_CARD_TYPE_TOTAL] = {0};

    memset(awardData, 0, sizeof(awardData));

    pHero = GetFightingHero();
    KGLOG_PROCESS_ERROR(pHero);

    pHero->Detach();
    if (m_pAITeammate)
        m_pAITeammate->Detach();
    ReturnToHall();

    g_PlayerServer.DoMissionBattleFinished(m_nConnIndex, pHero->m_nSide, nScore, awardData, teammates, bHasWard, 0);
    g_PlayerServer.DoPlayerEnterHallNotify(m_nConnIndex);

Exit0:
    return 0; 
}

int KPlayer::LuaOpenMission(Lua_State* L)
{
    BOOL        bRetCode        = false;
    int         nMissionType    = 0;
    int         nMissionStep    = 0;
    int         nMissionLevel   = 0;
    int         nTopIndex       = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nMissionType = (int)Lua_ValueToNumber(L, 1);
    nMissionStep = (int)Lua_ValueToNumber(L, 2);
    nMissionLevel = (int)Lua_ValueToNumber(L, 3);

    bRetCode = m_MissionData.OpenMission(nMissionType, nMissionStep, nMissionLevel);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0; 
}


int KPlayer::LuaFinishMission(Lua_State* L)
{
    BOOL        bRetCode        = false;
    int         nMissionType    = 0;
    int         nMissionStep    = 0;
    int         nMissionLevel   = 0;
    int         nTopIndex       = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nMissionType = (int)Lua_ValueToNumber(L, 1);
    nMissionStep = (int)Lua_ValueToNumber(L, 2);
    nMissionLevel = (int)Lua_ValueToNumber(L, 3);

    bRetCode = m_MissionData.FinishMission(nMissionType, nMissionStep, nMissionLevel);
    KG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0; 
}

int KPlayer::LuaAcceptQuest(Lua_State* L)
{
    BOOL        bRetCode    = false;
    int         nQuestID    = 0;
    int         nTopIndex   = 0;
    BOOL        bIgnoreRules = false;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1 || nTopIndex == 2);

    nQuestID = (int)Lua_ValueToNumber(L, 1);
    if (nTopIndex == 2)
        bIgnoreRules = (BOOL)Lua_ValueToNumber(L, 2);

    bRetCode = m_QuestList.Accept(nQuestID, bIgnoreRules);
    KGLOG_PROCESS_ERROR(bRetCode == qrcSuccess);

Exit0:
    return 0; 
}

int KPlayer::LuaCancelQuest(Lua_State* L)
{
    BOOL        bRetCode    = false;
    int         nQuestID    = 0;
    int         nTopIndex   = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nQuestID = (int)Lua_ValueToNumber(L, 1);

    bRetCode = m_QuestList.Cancel(nQuestID);
    KGLOG_PROCESS_ERROR(bRetCode == qrcSuccess);

Exit0:
    return 0; 
}

int KPlayer::LuaClearQuest(Lua_State* L)
{
    BOOL        bRetCode    = false;
    int         nQuestID    = 0;
    int         nTopIndex   = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nQuestID = (int)Lua_ValueToNumber(L, 1);

    m_QuestList.ClearQuest(nQuestID);

Exit0:
    return 0; 
}

int KPlayer::LuaSetQuestValue(Lua_State* L)
{
    BOOL        bRetCode    = false;
    int         nQuestID    = 0;
    int         nTopIndex   = 0;
    int         nQuestValueIndex = 0;
    int         nNewQuestValue = 0;
    int         nQuestIndex = -1;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nQuestID = (int)Lua_ValueToNumber(L, 1);
    nQuestValueIndex = (int)Lua_ValueToNumber(L, 2);
    nNewQuestValue = (int)Lua_ValueToNumber(L, 3);

    nQuestIndex = m_QuestList.GetQuestIndex(nQuestID);
    KGLOG_PROCESS_ERROR(nQuestIndex >= 0);

    bRetCode = m_QuestList.SetQuestValue(nQuestIndex, nQuestValueIndex, nNewQuestValue);
    KGLOG_PROCESS_ERROR(bRetCode == qrcSuccess);

Exit0:
    return 0; 
}

int KPlayer::LuaFinishQuest(Lua_State* L)
{
    BOOL        bRetCode    = false;
    int         nQuestID    = 0;
    int         nTopIndex   = 0;
    int         nChoice1    = 0;
    int         nChoice2    = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nQuestID = (int)Lua_ValueToNumber(L, 1);
    nChoice1 = (int)Lua_ValueToNumber(L, 2);
    nChoice2 = (int)Lua_ValueToNumber(L, 3);

    bRetCode = m_QuestList.Finish(nQuestID, nChoice1, nChoice2, true);
    KGLOG_PROCESS_ERROR(bRetCode == qrcSuccess);

Exit0:
    return 0; 
}

int KPlayer::LuaGetQuestPhase(Lua_State* L)
{
    BOOL        bRetCode    = false;
    int         nQuestID    = 0;
    int         nTopIndex   = 0;
    int         nQuestPhase = eqpInvalid;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nQuestID = (int)Lua_ValueToNumber(L, 1);

    nQuestPhase = m_QuestList.GetQuestPhase(nQuestID);
Exit0:
    lua_pushinteger(L, nQuestPhase);
    return 1; 
}

int KPlayer::LuaCreateTeam(Lua_State* L)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode        = CreateTeam();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaLeaveTeam(Lua_State* L)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode        = LeaveTeam();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bRetCode);
    return 1;
}

int KPlayer::LuaTeamInvitePlayer(Lua_State* L)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    int nTopIndex = 0;
    const char* pcszTargetName = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    pcszTargetName = (const char*)Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pcszTargetName);

    bRetCode = TeamInvitePlayer(pcszTargetName);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaTeamKickoutPlayer(Lua_State* L)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    uint32_t    dwTargetID  = ERROR_ID;
    int         nTopIndex   = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(1);

    dwTargetID = (uint32_t)Lua_ValueToNumber(L, 1);

    bRetCode = TeamKickoutPlayer(dwTargetID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaAutoMatch(Lua_State* L)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = AutoMatch();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return bResult;
}

int KPlayer::LuaCancelAutoMatch(Lua_State* L)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = CancelAutoMatch();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return bResult;
}

int KPlayer::LuaFireEvent(Lua_State* L)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    int         nTopIndex   = 0;
    int         nEvent = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nEvent = (int)Lua_ValueToNumber(L, 1);

    OnEvent(nEvent);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaAddHeroExp(Lua_State* L)
{
    BOOL       bRetCode  = false;
    int        nTopIndex = 0;
    DWORD      dwHeroTemplateID = 0;
    int        nAddExp   = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);
    nAddExp = (int)Lua_ValueToNumber(L, 2);

    bRetCode = AddHeroExpNoAdditional(dwHeroTemplateID, nAddExp);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaGetExtPoint(Lua_State* L)
{
    int     nResult         = 0;
    int     nTopIndex       = 0;
    int     nExtPointIndex  = 0;
    int     nExtPointValue  = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nExtPointIndex = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nExtPointIndex >= 0 && nExtPointIndex < MAX_EXT_POINT_COUNT);

    nExtPointValue = m_ExtPointInfo.nExtPoint[nExtPointIndex];

    Lua_PushNumber(L, nExtPointValue);

    nResult = 1;
Exit0:
    return nResult;
}

int KPlayer::LuaSetExtPoint(Lua_State* L)
{
    BOOL    bRetCode        = false;
    int     nTopIndex       = 0;
    int     nExtPointIndex  = 0;
    int     nExtPointValue  = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    nExtPointIndex = (int)Lua_ValueToNumber(L, 1);
    nExtPointValue = (int)Lua_ValueToNumber(L, 2);

    bRetCode = SetExtPoint(nExtPointIndex, nExtPointValue);
    if (!bRetCode)
    {
        KGLogPrintf(
            KGLOG_INFO, "Apply SetExtPoint failed. RoleName:'%s', "
            "PlayerID = %d, ExtPointIndex = %d, ExtPointValue = %d",
            m_szName, m_dwID, nExtPointIndex, nExtPointValue
            );
        goto Exit0;
    }

Exit0:
    return 0;
}

int KPlayer::LuaGetExtPointByBits(Lua_State* L)
{
    int		nResult             = 0;
    BOOL	bRetCode			= false;
    int		nTopIndex           = 0;
    int		nExtPointIndex      = 0;
    int		nExtPointBitIndex   = 0;
    int		nExtPointBitLength  = 0;
    int		nExtPointValue		= 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nExtPointIndex = (int)Lua_ValueToNumber(L, 1);
    nExtPointBitIndex = (int)Lua_ValueToNumber(L, 2);
    nExtPointBitLength = (int)Lua_ValueToNumber(L, 3);

    bRetCode = GetExtPoint(nExtPointIndex, nExtPointBitIndex, nExtPointBitLength, nExtPointValue);
    KGLOG_PROCESS_ERROR(bRetCode);

    Lua_PushNumber(L, nExtPointValue);

    nResult = 1;
Exit0:
    return nResult;
}

int KPlayer::LuaSetExtPointByBits(Lua_State* L)
{
    BOOL        bRetCode            = false;
    int         nTopIndex           = 0;
    int         nExtPointIndex      = 0;
    int         nExtPointBitIndex   = 0;
    int         nExtPointBitLength  = 0;
    int         nExtPointBitValue   = 0;
    int         nExtPointValue      = 0;
    int         nOldExtPointValue   = 0;
    const int   cMaxBit             = CHAR_BIT * sizeof(int);

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 4);

    nExtPointIndex = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nExtPointIndex >= 0 && nExtPointIndex < MAX_EXT_POINT_COUNT);

    nExtPointBitIndex = (int)Lua_ValueToNumber(L, 2);
    KGLOG_PROCESS_ERROR(nExtPointBitIndex >= 0 && nExtPointBitIndex < cMaxBit);

    nExtPointBitLength = (int)Lua_ValueToNumber(L, 3);
    KGLOG_PROCESS_ERROR(nExtPointBitLength > 0 && nExtPointBitIndex + nExtPointBitLength <= cMaxBit);

    nExtPointBitValue = (int)Lua_ValueToNumber(L, 4);
    KGLOG_PROCESS_ERROR(nExtPointBitValue >= 0);

    nExtPointValue = nExtPointBitValue;
    if (nExtPointBitLength != cMaxBit)
    {
        KGLOG_PROCESS_ERROR(nExtPointValue < (1 << nExtPointBitLength));

        nOldExtPointValue = m_ExtPointInfo.nExtPoint[nExtPointIndex];
        nExtPointValue = nExtPointValue << nExtPointBitIndex;

        for (int i = nExtPointBitIndex; i < nExtPointBitIndex + nExtPointBitLength; i++)
        {
            int nBitValue = 1 << i;

            nOldExtPointValue &= ~nBitValue;
            nOldExtPointValue |= nExtPointValue & nBitValue;
        }

        nExtPointValue = nOldExtPointValue;
    }

    bRetCode = SetExtPoint(nExtPointIndex, nExtPointValue);
    if (!bRetCode)
    {
        KGLogPrintf(
            KGLOG_INFO, "Apply SetExtPointByBits failed. RoleName:'%s', PlayerID = %d, "
            "ExtPointIndex = %d, ExtPointBitIndex = %d, ExtPointBitLength = %d, ExtPointValue = %d",
            m_szName, m_dwID, nExtPointIndex, nExtPointBitIndex, nExtPointBitLength, nExtPointBitValue
            );
        goto Exit0;
    }

Exit0:
    return 0;
}

int KPlayer::LuaGetHeroLevel(Lua_State* L)
{
    int		    nResult             = 0;
    BOOL	    bRetCode			= false;
    int		    nTopIndex           = 0;
    int         nTemplateID         = 0;
    KHeroData*  pHeroData           = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nTemplateID = (int)Lua_ValueToNumber(L, 1);

    pHeroData = m_HeroDataList.GetHeroData(nTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    Lua_PushNumber(L, pHeroData->m_nLevel);

    nResult = 1;
Exit0:
    return nResult;
}

int KPlayer::LuaGetAITeammate(Lua_State* L)
{
    int		    nResult             = 0;

    KG_PROCESS_ERROR(m_pAITeammate);

    nResult = m_pAITeammate->LuaGetObj(L);
Exit0:
    return nResult;
}

int KPlayer::LuaCreateFreePVPRoom(Lua_State* L)
{
    BOOL bResult = false;
    DWORD dwMapID = ERROR_ID;
    const char* pcszRoomName = NULL;
    const char* pcszPassword = NULL;
    BYTE byRoomType = 0;
    int nTopIndex = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    dwMapID = (DWORD)Lua_ValueToNumber(L, 1);
    pcszRoomName = Lua_ValueToString(L, 2);
    pcszPassword = Lua_ValueToString(L, 3);

    g_RelayClient.DoApplyCreateRoom(this, dwMapID, pcszRoomName, pcszPassword);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaProduceItem(Lua_State* L)
{
    BOOL  bResult    = false;
    BOOL  bRetCode   = false;
    int   nTopIndex  = 0;
    DWORD dwRecipeID = ERROR_ID;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwRecipeID = (DWORD)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(dwRecipeID);

    bRetCode = ProduceItem(dwRecipeID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaLearnRecipe(Lua_State* L)
{
    BOOL  bResult    = false;
    BOOL  bRetCode   = false;
    int   nTopIndex  = 0;
    DWORD dwRecipeID = ERROR_ID;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwRecipeID = (DWORD)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(dwRecipeID);

    bRetCode = LearnRecipe(dwRecipeID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaRepairEquip(Lua_State* L)
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    int             nPackageType    = ERROR_ID;
    int             nPackageIndex   = ERROR_ID;
    int             nPos            = ERROR_ID;
    int             nTopIndex       = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(nTopIndex == 0 || nTopIndex == 3);

    if (nTopIndex == 3)
    {
        nPackageType = (int)Lua_ValueToNumber(L, 1);
        KGLOG_PROCESS_ERROR(nPackageType == ePlayerPackage || nPackageType == eHeroPackage);

        nPackageIndex = (int)Lua_ValueToNumber(L, 2);
        KGLOG_PROCESS_ERROR(nPackageIndex >= 0);

        nPos = (int)Lua_ValueToNumber(L, 3);
        KGLOG_PROCESS_ERROR(nPos >= 0);

        bRetCode = RepairEquip(nPackageType, nPackageIndex, nPos);
        KGLOG_PROCESS_ERROR(bRetCode);
        goto Exit1;
    }

    bRetCode = m_ItemList.RepairAllHeroEquip();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit1:
    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;    
}

int KPlayer::LuaAbradeHeroEquip(Lua_State* L)
{
    BOOL            bResult             = false;
    BOOL            bRetCode            = false;
    int             nTopIndex           = 0;
    uint32_t        dwHeroTemplateID    = 0;
    KHeroPackage*   pHeroPackage        = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwHeroTemplateID = (uint32_t)Lua_ValueToNumber(L, 1);

    pHeroPackage = m_ItemList.GetHeroPackage(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroPackage);

    bRetCode = pHeroPackage->AbradeEquip();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaDownwardNotify(Lua_State* L)
{
    BOOL bResult = false;
    int nTopIndex = 0;
    DWORD dwMessageID = ERROR_ID;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwMessageID = (DWORD)Lua_ValueToNumber(L, 1);

    g_PlayerServer.DoDownwardNotify(this, dwMessageID);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaBuyGoods(Lua_State* L)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nTopIndex   = 0;
    int     nShopID     = 0;
    int     nGoodsID    = 0;
    int     nCount      = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nShopID = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nShopID > 0);

    nGoodsID = (int)Lua_ValueToNumber(L, 2);
    KGLOG_PROCESS_ERROR(nGoodsID > 0);

    nCount = (int)Lua_ValueToNumber(L, 3);
    KGLOG_PROCESS_ERROR(nCount > 0);

    bRetCode = BuyGoods(nShopID, nGoodsID, nCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaAddTeamLogo(Lua_State* L)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nTopInde    = 0;
    WORD    wTeamLogoID = 0;

    nTopInde = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopInde == 1);

    wTeamLogoID = (WORD)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(wTeamLogoID >= 0);

    bRetCode = AddTeamLogo(wTeamLogoID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaSetTeamLogo( Lua_State* L )
{
	BOOL    bResult     = false;
	BOOL    bRetCode    = false;
	int     nTopIndex   = 0;
	WORD wFrameID = 0;
	WORD wEmblemID = 0;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 2);

	wFrameID = (WORD)Lua_ValueToNumber(L, 1);
	KGLOG_PROCESS_ERROR(wFrameID >= 0);

	wEmblemID = (WORD)Lua_ValueToNumber(L, 2);
	KGLOG_PROCESS_ERROR(wEmblemID >= 0);

	bRetCode = SetTeamLogo(wFrameID, wEmblemID);
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KPlayer::LuaSetFatiguePoint(Lua_State* L)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nTopIndex   = 0;
    int     nTemp       = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nTemp = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nTemp >= 0);
    KGLOG_PROCESS_ERROR(nTemp <= USHRT_MAX);
    m_nCurFatiguePoint = nTemp;

    g_PlayerServer.DoSyncPlayerStateInfo(this);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaAddTeamExp(Lua_State* L)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nTopIndex = 0;
    int nExp = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nExp = (int)Lua_ValueToNumber(L, 1);

    bRetCode = AddTeamExp(nExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaSwitchToExistScene(Lua_State * L)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    int nTopIndex = 0;
    DWORD dwMapID = ERROR_ID;
    int nMapCopyIndex = 0;
    int nSide = 0;
    KScene* pScene = NULL;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    dwMapID = (DWORD)Lua_ValueToNumber(L, 1);
    nMapCopyIndex = (int)Lua_ValueToNumber(L, 2);
    nSide = (int)Lua_ValueToNumber(L, 3);

    pScene = g_pSO3World->GetScene(dwMapID, nMapCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    bRetCode = pScene->AddPlayer(this, nSide);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = SwitchMap(dwMapID, nMapCopyIndex);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaBuyFatiguePoint(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    int  nBuySN     = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nBuySN = (int)Lua_ValueToNumber(L, 1);

    bRetCode = BuyFatiguePoint(nBuySN);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaSetFatigueValue(Lua_State* L)
{
    BOOL bResult 	            = false;
    BOOL bRetCode	            = false;
    int	 nTopIndex	            = 0;
    int  nFatiguePoint          = m_nCurFatiguePoint;
    int  nTodayBuyTimes         = m_nTodayBuyFatiguePointTimes;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1 || nTopIndex == 2);

    nFatiguePoint = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nFatiguePoint >= 0 && nFatiguePoint <= USHRT_MAX);

    if (nTopIndex >= 2)
    {
        nTodayBuyTimes = (int)Lua_ValueToNumber(L, 2);
        KGLOG_PROCESS_ERROR(nTodayBuyTimes >= 0);
    }

    m_nTodayBuyFatiguePointTimes = nTodayBuyTimes;
    m_nCurFatiguePoint           = nFatiguePoint;

    g_PlayerServer.DoSyncPlayerStateInfo(this);

    if (m_nCurFatiguePoint == 0)
    {
        bRetCode = m_Secretary.AddReport(KREPORT_EVENT_ZERO_FATIGUE_POINT, NULL, 0);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaAddVIPExp(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    int  nVIPExp    = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nVIPExp = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nVIPExp >= 0);

    bRetCode = AddVIPExp(nVIPExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaChargeVIPTime(Lua_State* L)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nChargeSeconds = 0;
    int	 nTopIndex	= 0;
    KReport_Charge_VIP_Time param;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nChargeSeconds = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nChargeSeconds > 0);

    param.addVIPTime = nChargeSeconds;
    param.isOpen = (BYTE)(m_nVIPEndTime == 0);

    if (IsVIP())
    {
        m_nVIPEndTime = m_nVIPEndTime + nChargeSeconds;
    }
    else
    {
        m_nVIPEndTime = g_pSO3World->m_nCurrentTime + nChargeSeconds;
        g_RelayClient.DoUpdateVIPInfo(this);
    }

    g_PlayerServer.DoSyncVIPEndTime(m_nConnIndex, m_nVIPEndTime);

    ApplyVIP();
    
    g_LSClient.DoRemoteCall("OnUpdateVIPEndTime", (int)m_dwID, (int)m_dwClubID, m_nVIPEndTime);
    m_Secretary.AddReport(KREPORT_EVENT_CHARGE_VIP_TIME, (BYTE*)&param, sizeof(param));
    UpdateVIPAwardCount();

Exit0:
    return 0;
}

int KPlayer::LuaBuyCheerleadingSlot(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    DWORD dwSlotIndex = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwSlotIndex = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_CheerleadingMgr.BuySlot(dwSlotIndex);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaAddCheerleadingSlot(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    DWORD dwSlotIndex = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwSlotIndex = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_CheerleadingMgr.AddSlot(dwSlotIndex);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaGetCheerleadingSlotCount(Lua_State* L)
{
    lua_pushinteger(L, m_CheerleadingMgr.m_uSlotCount);
    return 1;
}

int KPlayer::LuaAddCheerleadingItem(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    DWORD dwCheerleadingID = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwCheerleadingID = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_CheerleadingMgr.AddCheerleading(dwCheerleadingID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaResetFatiguePoint(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    int nDays = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nDays = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nDays > 0);

    bRetCode = ResetFatiguePoint(nDays);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaGetPlayerValue(Lua_State* L)
{
    int     nResult         = 0;
    BOOL    bRetCode        = false;
	int	    nTopIndex	    = 0;
    DWORD   dwPlayerValueID = ERROR_ID;
    int     nPlayerValue    = 0;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwPlayerValueID = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_PlayerValue.GetValue(dwPlayerValueID, nPlayerValue);
    KGLOG_PROCESS_ERROR(bRetCode);

    Lua_PushNumber(L, nPlayerValue);

	nResult = 1;
Exit0:
	return nResult;
}

int KPlayer::LuaSetPlayerValue(Lua_State* L)
{
    BOOL    bResult 	    = false;
    BOOL    bRetCode	    = false;
    int	    nTopIndex	    = 0;
    DWORD   dwPlayerValueID = ERROR_ID;
    int     nValue          = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    dwPlayerValueID = (DWORD)Lua_ValueToNumber(L, 1);
    nValue          = (int)Lua_ValueToNumber(L, 2);

    bRetCode = SetPlayerValue(dwPlayerValueID, nValue);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaDoCustomConsumeRequest(Lua_State* L)
{
	BOOL bResult 	= false;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    int nConsumeType = 0;
    int nTotalConsumeMoney;
    KCUSTOM_CONSUME_INFO CCInfo;
    KCUSTOM_CONSUME_INFO* pCCInfo = NULL;
    const char* pszString = NULL;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex >= 2);

    memset(&CCInfo, 0, sizeof(CCInfo));

    switch(nTopIndex)
    {
    case 7:
        pszString = Lua_ValueToString(L, 7);
        KGLOG_PROCESS_ERROR(pszString);
        strncpy(CCInfo.szString1, pszString, countof(CCInfo.szString1));
    case 6:
        CCInfo.nValue4 = (int)Lua_ValueToNumber(L, 6);
    case 5:
        CCInfo.nValue3 = (int)Lua_ValueToNumber(L, 5);
    case 4:
        CCInfo.nValue2 = (int)Lua_ValueToNumber(L, 4);
    case 3:
        CCInfo.nValue1 = (int)Lua_ValueToNumber(L, 3);
        pCCInfo = &CCInfo;
    case 2:
        nConsumeType = (int)Lua_ValueToNumber(L, 1);
        nTotalConsumeMoney = (int)Lua_ValueToNumber(L, 2);
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
    }

    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(this, (CUSTOM_CONSUME_TYPE)nConsumeType, nTotalConsumeMoney, pCCInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KPlayer::LuaGetDailyResetUserData(Lua_State* L)
{
    int nResult = 0;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    const char* pcszKey = NULL;
	KMAP_DAILYRESET_USERDATA::iterator it;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    pcszKey = Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pcszKey);

    it = m_mapDailyResetUserData.find(pcszKey);
    KG_PROCESS_ERROR(it != m_mapDailyResetUserData.end());

    Lua_PushNumber(L, it->second);

	nResult = 1;
Exit0:
	return nResult;
}

int KPlayer::LuaSetDailyResetUserData(Lua_State* L)
{
    BOOL bResult    = false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    const char* pcszKey = NULL;
    int nValue = 0;
    KMAP_DAILYRESET_USERDATA::iterator it;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    pcszKey = Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pcszKey);

    nValue = (int)Lua_ValueToNumber(L, 2);

    m_mapDailyResetUserData[pcszKey] = nValue;

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return bResult;
}

int KPlayer::LuaAwardRandomOne(Lua_State* L)
{
	BOOL bResult 	= false;
	int	 nTopIndex	= 0;
    DWORD dwAwardTableID = ERROR_ID;
    KAWARD_ITEM* pAwardItem = NULL;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwAwardTableID = (DWORD)Lua_ValueToNumber(L, 1);

    pAwardItem = g_pSO3World->m_AwardMgr.AwardRandomOne(this, dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardItem);

	bResult = true;
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KPlayer::LuaAwardAll(Lua_State* L)
{
	BOOL bResult 	= false;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    DWORD dwAwardTableID = ERROR_ID;
    const char* pcszWayID = NULL;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 2);

    dwAwardTableID = (DWORD)Lua_ValueToNumber(L, 1);
    pcszWayID = Lua_ValueToString(L, 2);
    KGLOG_PROCESS_ERROR(pcszWayID);

    bRetCode = g_pSO3World->m_AwardMgr.AwardAll(this, dwAwardTableID, pcszWayID);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KPlayer::LuaClientCall(Lua_State* L)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int nTopIndex = 0;
    DWORD dwScriptID = ERROR_ID;
	const static char* pcszScriptName = "scripts/global/ClientCall.lua";
    const static  char* pcszFunctionName = "ClientCall";
    int nCallID = 0;
    int nArg1 = 0;
    int nArg2 = 0;
	
    nTopIndex = Lua_GetTopIndex(L);

    switch(nTopIndex)
    {
    case 3:
        nArg2 = (int)Lua_ValueToNumber(L, 3);
    case 2:
        nArg1 = (int)Lua_ValueToNumber(L, 2);
    case 1:
        nCallID = (int)Lua_ValueToNumber(L, 1);
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    dwScriptID = g_FileNameHash(pcszScriptName);
    KGLOG_PROCESS_ERROR(dwScriptID);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, pcszFunctionName);
    KGLOG_PROCESS_ERROR(bRetCode);

    lua_pushstring(L, "player");
    LuaGetObj(L);
    lua_settable(L, LUA_GLOBALSINDEX);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(nCallID);
    g_pSO3World->m_ScriptManager.Push(nArg1);
    g_pSO3World->m_ScriptManager.Push(nArg2);
    g_pSO3World->m_ScriptManager.Call(dwScriptID, pcszFunctionName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
	return 0;
}

int KPlayer::LuaWinGame(Lua_State* L)
{
    KScene* pScene = NULL;
    KHero* pMainHero = GetFightingHero();
    KGLOG_PROCESS_ERROR(pMainHero);

    pScene = pMainHero->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    KGLOG_PROCESS_ERROR(pMainHero->m_nSide == sidLeft || pMainHero->m_nSide == sidRight);

    if (pScene->m_Battle.m_eMode == ebtTime)
    {
        pScene->m_nScore[pMainHero->m_nSide] = SHRT_MAX;
        pScene->m_Battle.SetBattleTotalFrame(1, 1);
    }
    else if (pScene->m_Battle.m_eMode == ebtScore)
    {
        pScene->m_nScore[pMainHero->m_nSide] = SHRT_MAX;
        pScene->m_Battle.m_bUptoTotalScore = true;
    }
    
Exit0:
    return 0;
}

int KPlayer::LuaLoseGame(Lua_State* L)
{
    KScene* pScene = NULL;
    KHero* pMainHero = GetFightingHero();
    KGLOG_PROCESS_ERROR(pMainHero);

    pScene = pMainHero->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    KGLOG_PROCESS_ERROR(pMainHero->m_nSide == sidLeft || pMainHero->m_nSide == sidRight);

    if (pScene->m_Battle.m_eMode == ebtTime)
    {
        if (pMainHero->m_nSide == sidLeft)
            pScene->m_nScore[sidRight] = SHRT_MAX;
        else
            pScene->m_nScore[sidLeft] = SHRT_MAX;

        pScene->m_Battle.SetBattleTotalFrame(1, 1);
    }
    else if (pScene->m_Battle.m_eMode == ebtScore)
    {
        if (pMainHero->m_nSide == sidLeft)
            pScene->m_nScore[sidRight] = SHRT_MAX;
        else
            pScene->m_nScore[sidLeft] = SHRT_MAX;
        pScene->m_Battle.m_bUptoTotalScore = true;
    }
    
Exit0:
    return 0;
}

int KPlayer::LuaAddFreeCoin(Lua_State* L)
{
    BOOL bResult 	    = false;
    BOOL bRetCode	    = false;
    int	 nTopIndex	    = 0;
    int  nAddFreeCoin   = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nAddFreeCoin = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_MoneyMgr.AddFreeCoin(nAddFreeCoin);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return 0;
}

int KPlayer::LuaIsAchievementFinished(Lua_State* L)
{
	BOOL bResult 	= false;
	int	 nTopIndex	= 0;
    DWORD dwAchievementID = ERROR_ID;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwAchievementID = (DWORD)Lua_ValueToNumber(L, 1);

    bResult = m_Achievement.IsAchievementFinished(dwAchievementID);
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KPlayer::LuaSetAchievementValue(Lua_State* L)
{
	BOOL bResult 	= false;
	int	 nTopIndex	= 0;
	DWORD dwAchievementID = ERROR_ID;
    int nNewValue = 0;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 2);

    dwAchievementID = (DWORD)Lua_ValueToNumber(L, 1);
    nNewValue = (int)Lua_ValueToNumber(L, 2);

    bResult = m_Achievement.SetAchievementValue(dwAchievementID, nNewValue);
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KPlayer::LuaGetAchievementValue(Lua_State* L)
{
    int nResult = 0;
    int	 nTopIndex	= 0;
	DWORD dwAchievementID = ERROR_ID;
    int nCurValue = 0;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nCurValue = m_Achievement.GetAchievenmentValue(dwAchievementID);
    
    Lua_PushNumber(L, nCurValue);
    nResult = 1;
Exit0:
	return nResult;
}

int KPlayer::LuaResetAchievement(Lua_State* L)
{
	int  nResult 	= 0;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    DWORD dwAchievementID = ERROR_ID;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwAchievementID = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_Achievement.ResetOne(dwAchievementID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Achievement.EnsureEventMapBuild(dwAchievementID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
	return nResult;
}

int KPlayer::LuaDailySignAward(Lua_State* L)
{
	int  nResult 	= 0;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
	DWORD dwAwardTableID = ERROR_ID;
    KAWARD_ITEM* pAwardItem = NULL;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwAwardTableID = (DWORD)Lua_ValueToNumber(L, 1);

    pAwardItem = g_pSO3World->m_AwardMgr.AwardRandomOne(this, dwAwardTableID);
    KGLOG_PROCESS_ERROR(pAwardItem);

    g_PlayerServer.DoDailySignAwardNotify(m_nConnIndex, pAwardItem);

    Lua_PushBoolean(L, true);
    nResult = 1;
Exit0:
	return nResult;
}

int KPlayer::LuaEnterChallenge(Lua_State* L)
{
    int  nResult 	= 0;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    int  nStep      = 1;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nStep = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nStep > 0 && nStep <= g_pSO3World->m_Settings.m_ConstList.nChallengeMissionCount);
    
    SetLastChallengeStep(nStep);
    SetStartChallengeStep(nStep);

    EnterMission(GetCurrentChallengeType(), nStep, 1);

Exit0:
    return 0;
}

int KPlayer::LuaAddLadderExp(Lua_State* L)
{
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    DWORD dwHeroTempalteID = 0;
    int nExp = 0;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 2);

    dwHeroTempalteID = (DWORD)Lua_ValueToNumber(L, 1);
    nExp = (int)Lua_ValueToNumber(L, 2);

    bRetCode = AddHeroLadderExp(dwHeroTempalteID, nExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_LSClient.DoLadderDataChangeNotify(this, dwHeroTempalteID);

Exit0:
	return 0;
}

int KPlayer::LuaPlayerLog(Lua_State* L)
{
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    char* pszLog = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    pszLog = (char*)Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszLog);

    PLAYER_LOG(this, pszLog);

Exit0:
    return 0;
}

int KPlayer::LuaIsCheerleadingEmpty(Lua_State* L)
{
	int bResult = true;

	for (DWORD i = 0; i < m_CheerleadingMgr.m_uSlotCount; ++i)
	{
		if (m_CheerleadingMgr.m_SlotInfo[i].dwCheerleadingIndex != 0)
		{
			bResult = false;
			break;
		}
	}

	Lua_PushBoolean(L, bResult);
	return 1;
}

int KPlayer::LuaGetHeroFashionID(Lua_State* L)
{
	int	 nTopIndex	= 0;
	DWORD dwHeroTemplateID = 0;
	KHeroData* pHeroData = NULL;
    DWORD dwFashionID = ERROR_ID;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

	dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);

	pHeroData = m_HeroDataList.GetHeroData(dwHeroTemplateID);
	KGLOG_PROCESS_ERROR(pHeroData);
	
    dwFashionID = pHeroData->m_wFashionID;
Exit0:
    Lua_PushNumber(L, dwFashionID);
	return 1;
}

int KPlayer::LuaTestFlowRecord(Lua_State* L)
{
    int nTopIndex   = 0;
    int nTestTime   = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nTestTime = (int)Lua_ValueToNumber(L, 1);

    for (int i = 0; i < nTestTime; ++i)
    {
        g_LogClient.DoFlowRecord(frmtBattle, bfrChallengeStart, "%s,%s,%d,%d,%d", "Player1", "Player2", 1, 2, 3);
        g_LogClient.DoFlowRecord(frmtBattle, bfrChallengeEnd, "%s,%s,%s,%s,%d,%d", "LeftPlayer1", "LeftPlayer2", "RightPlayer1", "RightPlayer2", 100, 100);
        g_LogClient.DoFlowRecord(frmtBattle, bfrChallengeDirectEnd, "%s,%d,%d,%d", "PlayerName", 1, 2, 3);
        g_LogClient.DoFlowRecord(frmtBattle, bfrLadderPVPStart, "%s,%s,%s,%s", "LeftPlayer1", "LeftPlayer2", "RightPlayer1", "RightPlayer2");
        g_LogClient.DoFlowRecord(frmtBattle, bfrLadderPVPEnd, "%s,%s,%s,%s,%d,%d", "LeftPlayer1", "LeftPlayer2", "RightPlayer1", "RightPlayer2", 100, 100);
        g_LogClient.DoFlowRecord(frmtBattle, bfrLadderPVPDirectEnd, "%s", "PlayerName");

        g_LogClient.DoFlowRecord(frmtDailyQuest, dqfrAcceptQuest, "%s,%d", "acceptquest", 1); 
        g_LogClient.DoFlowRecord(frmtDailyQuest, dqfrFinishQuest, "%s,%d", "finishquest", 1);

        g_LogClient.DoFlowRecord(frmtMakingAndStrengthen, masfrMakeEquip, "%s,%d,%u,%u,%d,%u,%u,%d,%d,%s", "PlayerName", 1, 1, 1, 1, 1, 1, 1, 1, "ItemName");
        g_LogClient.DoFlowRecord(frmtMakingAndStrengthen, masfrStrengthenFailed, "%s,%d,%u,%u,%d,%s", "PlayerName", 1, 1, 1, 1, "EquipmentName");

        g_LogClient.DoFlowRecord(frmtHeroTrainning, htfrBeginTrain, "%s,%d,%u,%d", "PlayerName", 1, 1, 60);
        g_LogClient.DoFlowRecord(frmtHeroTrainning, htfrEndTrain, "%s,%u,%d,%d", "PlayerName", 1, 1, 100);

        g_LogClient.DoFlowRecord(frmtGym, gfrBeginTrain, "%s,%d,%u,%d,%d", "PlayerName", 100, 1, 60, 1);
        g_LogClient.DoFlowRecord(frmtGym, gfrEndTrain, "%s,%u,%d,%d", "PlayerName", 1, 1, 1);
        g_LogClient.DoFlowRecord(frmtGym, gfrClearTrainCD, "%s,%d,%u,%d,%d", "PlayerName", 100, 1, 123123123, 1);

        g_LogClient.DoFlowRecord(frmtCheerleadingAndWardrobe, cawfrBuyCheerleading, "%s,%d,%u", "PlayerName", 100, 1);
        g_LogClient.DoFlowRecord(frmtCheerleadingAndWardrobe, cawfrCheerleadingDisappear, "%s,%u", "PlayerName", 1);
        g_LogClient.DoFlowRecord(frmtCheerleadingAndWardrobe, cawfrBuyFashion, "%s,%d,%d", "PlayerName", 100, 1);
        g_LogClient.DoFlowRecord(frmtCheerleadingAndWardrobe, cawfrFashionDisappear, "%s,%u", "PlayerName", 1);

        g_LogClient.DoFlowRecord(frmtBusinessStreet, bsfrUpgrade, "%s,%d,%d,%u,%d,%d", "PlayerName", 100, 100, 1, 1, 1);
        g_LogClient.DoFlowRecord(frmtBusinessStreet, bsfrGetMoney, "%s,%u,%d,%d,%d", "PlayerName", 1, 1, 1, 100);
        g_LogClient.DoFlowRecord(frmtBusinessStreet, bsfrClearGetMoneyCD, "%s,%d,%d", "PlayerName", 100, 123123123);
        
        g_LogClient.DoFlowRecord(frmtUpgradeQueue, uqfrClearCD, "%s,%d,%d,%d", "PlayerName", 100, 1, 60);

        g_LogClient.DoFlowRecord(frmtHeroBuyAndFire, hbaffrBuyHero, "%s,%d,%u", "PlayerName", 100, 1);
        g_LogClient.DoFlowRecord(frmtHeroBuyAndFire, hbaffrFireHero, "%s,%u", "PlayerName", 1);

        printf("All FlowRecord Test Over! Times:%d/%d\r", i + 1, nTestTime);
    }  

    /*
    frmtBattle,                     //全球挑战与大师赛流水
    frmtDailyQuest,                 //日常流水
    frmtMakingAndStrengthen,        //装备制造与强化流水
    frmtHeroTrainning,              //篮球场训练流水
    frmtGym,                        //健身房训练流水
    frmtCheerleadingAndWardrobe,    //拉拉队流水与外装流水
    frmtBusinessStreet,             //商业街流水与金币领取流水
    frmtUpgradeQueue,               //清除升级队列流水
    frmtHeroBuyAndFire,
    */

Exit0:
    Lua_PushBoolean(L, true);
    return 1;
}

int KPlayer::LuaAddPlayerBuff(Lua_State* L)
{
    BOOL    bResult             = false;
    BOOL    bRetCode	        = false;
    int	    nTopIndex	        = 0;
    DWORD   dwBuffID            = 0;
    int     nFrames             = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    dwBuffID = (DWORD)Lua_ValueToNumber(L, 1);
    nFrames = (int)Lua_ValueToNumber(L, 2);

    bRetCode = AddBuff(dwBuffID, nFrames);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1  ;
}

int KPlayer::LuaAddBuffTime(Lua_State* L)
{
    BOOL    bResult             = false;
	BOOL    bRetCode	        = false;
	int	    nTopIndex	        = 0;
    DWORD   dwHeroTempelateID   = ERROR_ID;
    DWORD   dwBuffID            = 0;
    int     nFrames             = 0;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 3);

    dwHeroTempelateID = (DWORD)Lua_ValueToNumber(L, 1);
    dwBuffID = (DWORD)Lua_ValueToNumber(L, 2);
    nFrames = (int)Lua_ValueToNumber(L, 3);

    bRetCode = m_HeroDataList.AddBuffTime(dwHeroTempelateID, dwBuffID, nFrames);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
	return 1  ;
}

int KPlayer::LuaHasHero(Lua_State* L)
{
	BOOL bResult = false;
	int	 nTopIndex	= 0;
    DWORD dwHeroTemplateID = ERROR_ID;
    KHeroData* pHeroData = NULL;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);

    pHeroData = m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KG_PROCESS_ERROR(pHeroData);

    KG_PROCESS_ERROR(!pHeroData->m_bFired);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
	return 1;
}

int KPlayer::LuaS2CRemoteCall(Lua_State* L)
{
    BOOL    bResult     = false;
    int	    nTopIndex	= 0;
    int     nParamCount = 0;
    char*   pszFuncName = NULL;
    KVARIABLE param[4] = {rpcInvalid, rpcInvalid, rpcInvalid, rpcInvalid};

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex >= 1 && nTopIndex <= 5);

    pszFuncName = (char*)Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszFuncName);

    nParamCount = nTopIndex - 1;

    for (int i = 0; i < nParamCount; ++i)
    {
        // 只支持整数
        param[i] = (int)lua_tointeger(L, i + 2);
    }

    g_PlayerServer.DoRemoteCall(m_nConnIndex, pszFuncName, param[0], param[1], param[2], param[3]);
    
    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KPlayer::LuaUpgradeSafeBox(Lua_State* L)
{
    BOOL bRetCode = false;

    bRetCode = m_MoneyMgr.UpgradeSafeBox();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaUpgradeEquip(Lua_State* L)
{
    BOOL bRetCode = false;

    bRetCode = m_Gym.UpgradeEquip();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaUpgradeTrainTeacher(Lua_State* L)
{
    BOOL bRetCode = false;

    bRetCode = m_HeroTrainingMgr.UpgradeTeacherLevel();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaHeroUseEquip(Lua_State* L)
{
    BOOL bRetCode = false;
    int	 nTopIndex	= 0;
    DWORD dwHeroTemplateID = ERROR_ID;
    int   nEquipType = 0;
    int   nCount     = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);
    nEquipType = (int)Lua_ValueToNumber(L, 2);
    nCount = (int)Lua_ValueToNumber(L, 3);

    bRetCode = m_Gym.HeroUseEquip(dwHeroTemplateID, (KGYM_TYPE)nEquipType, nCount);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaHeroTrain(Lua_State* L)
{
    BOOL bRetCode = false;
    int	 nTopIndex	= 0;
    DWORD dwHeroTemplateID = ERROR_ID;
    int   nTrainID = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    dwHeroTemplateID = (DWORD)Lua_ValueToNumber(L, 1);
    nTrainID = (int)Lua_ValueToNumber(L, 2);

    bRetCode = m_HeroTrainingMgr.BeginTrainingHero(dwHeroTemplateID, nTrainID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaRecommendFriend(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;

    g_LSClient.DoApplyRecommendFriend(m_dwID, m_nHighestHeroLevel);
    bResult = true;
Exit0:
    return 0;
}

int KPlayer::LuaDecomposeEquip(Lua_State* L)
{
    BOOL bRetCode       = false;
    int	 nTopIndex	    = 0;
    KDecomposeItems item;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    item.uPackageTypeEquipIn = (unsigned char)Lua_ValueToNumber(L, 1);
    item.uPackageIndexEquipIn = (unsigned char)Lua_ValueToNumber(L, 2);
    item.uPosEquipIn = (unsigned char)Lua_ValueToNumber(L, 3);

    bRetCode = m_ItemList.DecomposeEquip(1, &item);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KPlayer::LuaSecretaryReport(Lua_State* L)
{
    int	        nTopIndex	    = 0;
    int         nEvent          = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nEvent = (int)Lua_ValueToNumber(L, 1);
    m_Secretary.AddReport(nEvent, NULL, 0);

Exit0:
    return 0;
}

int KPlayer::LuaUpgradeHeroLevel(Lua_State* L)
{
    BOOL        bRetCode        = false;
    int         nResult         = false;
    int	        nTopIndex	    = 0;
    int         nHeroTemplateID = 0;
    int         nUpLevel        = 0;
    KHeroData*  pHeroData       = NULL;
    KReport_Hero_LevelUP param;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    nHeroTemplateID = (int)Lua_ValueToNumber(L, 1);
    nUpLevel        = (int)Lua_ValueToNumber(L, 2);

    pHeroData = m_HeroDataList.GetHeroData(nHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_nLevel + nUpLevel <= cdMaxHeroLevel);

    pHeroData->m_nLevel += nUpLevel;
    pHeroData->m_wFreeTalent += (WORD)nUpLevel;

    g_PlayerServer.DoHeroLevelUp(m_nConnIndex, pHeroData->m_dwTemplateID, pHeroData->m_nLevel);
    g_PlayerServer.DoSyncFreeTalent(m_nConnIndex, pHeroData->m_dwTemplateID, pHeroData->m_wFreeTalent);

    g_RelayClient.DoUpdateHeroLevel(this, nHeroTemplateID);

    param.heroTemplateID = nHeroTemplateID;
    param.level          = (unsigned short)pHeroData->m_nLevel;

    bRetCode = m_Secretary.AddReport(KREPORT_EVENT_HERO_LEVELUP, (BYTE*)&param, sizeof(param));
    KGLOG_PROCESS_ERROR(bRetCode);

    g_LSClient.DoLadderDataChangeNotify(this, pHeroData->m_dwTemplateID);

    nResult = true;
Exit0:
    Lua_PushBoolean(L, nResult);
    return 1;
}

int KPlayer::LuaCanAddItem(Lua_State* L)
{
    BOOL        nResult         = false;
    int	        nTopIndex	    = 0;
    DWORD       dwTabType       = 0;
    DWORD       dwIndex         = 0;
    int         nCount          = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    dwTabType = (DWORD)Lua_ValueToNumber(L, 1);
    dwIndex = (DWORD)Lua_ValueToNumber(L, 2);
    nCount = (int)Lua_ValueToNumber(L, 3);

    nResult = CanAddItem(dwTabType, dwIndex, nCount);
Exit0:
    Lua_PushBoolean(L, nResult);
    return 1;
}

int KPlayer::LuaGetFreeRoomSize(Lua_State* L)
{
    int nResult = 0;

    nResult = m_ItemList.GetPlayerFreeRoomSize(eppiPlayerItemBox);
Exit0:
    Lua_PushNumber(L, nResult);
    return 1;
}

int KPlayer::LuaGetHeroMaxLevel(Lua_State* L)
{
	int nMaxLevel = 0;
	KHeroData* pHeroData = NULL;
	std::vector<DWORD> vecHeroList;

	m_HeroDataList.GetHeroList(vecHeroList);
	for (size_t i = 0; i < vecHeroList.size(); ++i)
	{
		pHeroData = m_HeroDataList.GetHeroData(vecHeroList[i]);
		if (pHeroData)
		{
			nMaxLevel = pHeroData->m_nLevel > nMaxLevel ? pHeroData->m_nLevel : nMaxLevel;
		}
	}

	Lua_PushNumber(L, nMaxLevel);
	return 1;
}

int KPlayer::LuaGetStoreCount(Lua_State* L)
{
    int nStoreCount = 0;

    nStoreCount = m_BusinessStreet.GetStoreCount();

    Lua_PushNumber(L, nStoreCount);

    return 1;
}

//////////////////////////////////////////////////////////////////////////

DEFINE_LUA_CLASS_BEGIN(KPlayer)
    REGISTER_LUA_DWORD(KPlayer, ID)
    REGISTER_LUA_STRING(KPlayer, Name)
    REGISTER_LUA_STRING(KPlayer, Account)
    REGISTER_LUA_TIME(KPlayer, CreateTime)
    REGISTER_LUA_TIME(KPlayer, LastLoginTime)
    REGISTER_LUA_TIME(KPlayer, LastSaveTime)
    REGISTER_LUA_DWORD(KPlayer, MainHeroTemplateID)
    REGISTER_LUA_DWORD(KPlayer, AssistHeroTemplateID)
    REGISTER_LUA_INTEGER(KPlayer, AILevelCoe)
    REGISTER_LUA_INTEGER(KPlayer, Level)
    REGISTER_LUA_DWORD_READONLY(KPlayer, ClubID)

    REGISTER_LUA_FUNC(KPlayer, AddNewHero)
    REGISTER_LUA_FUNC(KPlayer, DelHero)
    REGISTER_LUA_FUNC(KPlayer, AddItem)
    REGISTER_LUA_FUNC(KPlayer, DestroyItem)
    REGISTER_LUA_FUNC(KPlayer, CostItem)
    REGISTER_LUA_FUNC(KPlayer, ExchangeItem)
    REGISTER_LUA_FUNC(KPlayer, AddMoney)
    REGISTER_LUA_FUNC(KPlayer, GetFightingHero)
    REGISTER_LUA_FUNC(KPlayer, Rename)
    REGISTER_LUA_FUNC(KPlayer, SwitchMap)
    REGISTER_LUA_FUNC(KPlayer, SendMailToSelf)
    REGISTER_LUA_FUNC(KPlayer, SendMail)
    REGISTER_LUA_FUNC(KPlayer, AddMaxHeroCount)
    REGISTER_LUA_FUNC(KPlayer, SelectMainHero)
    REGISTER_LUA_FUNC(KPlayer, SelectAssistHero)
    REGISTER_LUA_FUNC(KPlayer, EnterMission)
    REGISTER_LUA_FUNC(KPlayer, LeaveMission)
    REGISTER_LUA_FUNC(KPlayer, OpenMission)
    REGISTER_LUA_FUNC(KPlayer, FinishMission)
    REGISTER_LUA_FUNC(KPlayer, AcceptQuest)
    REGISTER_LUA_FUNC(KPlayer, CancelQuest)
    REGISTER_LUA_FUNC(KPlayer, ClearQuest)
    REGISTER_LUA_FUNC(KPlayer, SetQuestValue)
    REGISTER_LUA_FUNC(KPlayer, FinishQuest)
    REGISTER_LUA_FUNC(KPlayer, GetQuestPhase)
    REGISTER_LUA_FUNC(KPlayer, CreateTeam)
    REGISTER_LUA_FUNC(KPlayer, LeaveTeam)
    REGISTER_LUA_FUNC(KPlayer, TeamInvitePlayer)
    REGISTER_LUA_FUNC(KPlayer, AutoMatch)
    REGISTER_LUA_FUNC(KPlayer, CancelAutoMatch)
    REGISTER_LUA_FUNC(KPlayer, FireEvent)
    REGISTER_LUA_FUNC(KPlayer, AddHeroExp)
    REGISTER_LUA_FUNC(KPlayer, GetExtPoint)
    REGISTER_LUA_FUNC(KPlayer, SetExtPoint)
    REGISTER_LUA_FUNC(KPlayer, GetExtPointByBits)
    REGISTER_LUA_FUNC(KPlayer, SetExtPointByBits)
    REGISTER_LUA_FUNC(KPlayer, GetHeroLevel)
    REGISTER_LUA_FUNC(KPlayer, GetAITeammate)
    REGISTER_LUA_FUNC(KPlayer, CreateFreePVPRoom)
    REGISTER_LUA_FUNC(KPlayer, ProduceItem)
    REGISTER_LUA_FUNC(KPlayer, LearnRecipe)
    REGISTER_LUA_FUNC(KPlayer, RepairEquip)
    REGISTER_LUA_FUNC(KPlayer, AbradeHeroEquip)
    REGISTER_LUA_FUNC(KPlayer, DownwardNotify)
    REGISTER_LUA_FUNC(KPlayer, BuyGoods)
    REGISTER_LUA_FUNC(KPlayer, AddTeamLogo)
	REGISTER_LUA_FUNC(KPlayer, SetTeamLogo)
    REGISTER_LUA_FUNC(KPlayer, SetFatiguePoint)
    REGISTER_LUA_FUNC(KPlayer, AddTeamExp)
    REGISTER_LUA_FUNC(KPlayer, SwitchToExistScene)
    REGISTER_LUA_FUNC(KPlayer, BuyFatiguePoint)
    REGISTER_LUA_FUNC(KPlayer, SetFatigueValue)
    REGISTER_LUA_FUNC(KPlayer, AddVIPExp)
	REGISTER_LUA_FUNC(KPlayer, AddLadderExp)
    REGISTER_LUA_FUNC(KPlayer, BuyCheerleadingSlot)
    REGISTER_LUA_FUNC(KPlayer, AddCheerleadingSlot)
    REGISTER_LUA_FUNC(KPlayer, GetCheerleadingSlotCount)
    
    REGISTER_LUA_FUNC(KPlayer, AddCheerleadingItem)
    REGISTER_LUA_FUNC(KPlayer, ChargeVIPTime)
    REGISTER_LUA_FUNC(KPlayer, ResetFatiguePoint)
    REGISTER_LUA_FUNC(KPlayer, GetMailList)
    REGISTER_LUA_FUNC(KPlayer, SeeMail)
    REGISTER_LUA_FUNC(KPlayer, GetMailAll)
    REGISTER_LUA_FUNC(KPlayer, SetMailRead)
    REGISTER_LUA_FUNC(KPlayer, MailItem)
    REGISTER_LUA_FUNC(KPlayer, DelMail)
	REGISTER_LUA_FUNC(KPlayer, GetPlayerValue)

    REGISTER_LUA_FUNC(KPlayer, CreateClub)
    REGISTER_LUA_FUNC(KPlayer, MemberList)
    REGISTER_LUA_FUNC(KPlayer, Apply)
    REGISTER_LUA_FUNC(KPlayer, Accept)
    REGISTER_LUA_FUNC(KPlayer, Invite)
    REGISTER_LUA_FUNC(KPlayer, SetPost)
    REGISTER_LUA_FUNC(KPlayer, SetDescript)
    REGISTER_LUA_FUNC(KPlayer, DelMember)
    REGISTER_LUA_FUNC(KPlayer, RandomClub)

    REGISTER_LUA_FUNC(KPlayer, SetPlayerValue)
    REGISTER_LUA_FUNC(KPlayer, DoCustomConsumeRequest)
    REGISTER_LUA_FUNC(KPlayer, GetDailyResetUserData)
    REGISTER_LUA_FUNC(KPlayer, SetDailyResetUserData)
    REGISTER_LUA_FUNC(KPlayer, AwardRandomOne)
    REGISTER_LUA_FUNC(KPlayer, AwardAll)
    REGISTER_LUA_FUNC(KPlayer, ClientCall)
    REGISTER_LUA_FUNC(KPlayer, WinGame)
    REGISTER_LUA_FUNC(KPlayer, LoseGame)
    REGISTER_LUA_FUNC(KPlayer, AddFreeCoin)
    REGISTER_LUA_FUNC(KPlayer, IsAchievementFinished)
    REGISTER_LUA_FUNC(KPlayer, SetAchievementValue)
    REGISTER_LUA_FUNC(KPlayer, GetAchievementValue)
    REGISTER_LUA_FUNC(KPlayer, ResetAchievement)
    REGISTER_LUA_FUNC(KPlayer, DailySignAward)
    REGISTER_LUA_FUNC(KPlayer, EnterChallenge)
    REGISTER_LUA_FUNC(KPlayer, AddActive)
    REGISTER_LUA_FUNC(KPlayer, AddLadderExp)
    REGISTER_LUA_FUNC(KPlayer, PlayerLog)
	REGISTER_LUA_FUNC(KPlayer, IsCheerleadingEmpty)
	REGISTER_LUA_FUNC(KPlayer, GetHeroFashionID)
    REGISTER_LUA_FUNC(KPlayer, TestFlowRecord)
    REGISTER_LUA_FUNC(KPlayer, AddPlayerBuff)
    REGISTER_LUA_FUNC(KPlayer, AddBuffTime)
    REGISTER_LUA_FUNC(KPlayer, HasHero)
    REGISTER_LUA_FUNC(KPlayer, S2CRemoteCall)
    REGISTER_LUA_FUNC(KPlayer, UpgradeEquip)
    REGISTER_LUA_FUNC(KPlayer, UpgradeSafeBox)
    REGISTER_LUA_FUNC(KPlayer, UpgradeTrainTeacher)
    REGISTER_LUA_FUNC(KPlayer, HeroUseEquip)
    REGISTER_LUA_FUNC(KPlayer, HeroTrain)
    REGISTER_LUA_FUNC(KPlayer, RecommendFriend)
	REGISTER_LUA_FUNC(KPlayer, DecomposeEquip)
	REGISTER_LUA_FUNC(KPlayer, SecretaryReport)
    REGISTER_LUA_FUNC(KPlayer, UpgradeHeroLevel)

    REGISTER_LUA_FUNC(KPlayer, CanAddItem)
    REGISTER_LUA_FUNC(KPlayer, GetFreeRoomSize)

	REGISTER_LUA_FUNC(KPlayer, GetHeroMaxLevel)
	REGISTER_LUA_FUNC(KPlayer, TestReport)

    REGISTER_LUA_FUNC(KPlayer, GetStoreCount)
DEFINE_LUA_CLASS_END(KPlayer)

