#include "stdafx.h"
#include "KSO3World.h"
#include "KHero.h"
#include "KScene.h"
#include "KLSClient.h"
#include "KPlayer.h"
#include "KPlayerServer.h"

int LuaTest(Lua_State* L)
{
    g_LSClient.DoRemoteCall("OnRpcTest", 100, 0.5f, (unsigned char)1, "hello");

Exit0:
	return 0;
}

int LuaReloadAward(Lua_State* L)
{
    g_pSO3World->m_AwardMgr.Reload();
    LogInfo("Reload all award!");
    return 0;
}

int LuaReloadQuest(Lua_State* L)
{
    g_pSO3World->m_Settings.m_QuestInfoList.Reload();
    LogInfo("Reload all quest!");
    return 0;
}

int LuaReloadAll(Lua_State* L)
{
    g_pSO3World->m_ScriptManager.ReloadAll();
    LogInfo("Reload all script!");
    return 0;
}

int LuaReloadOne(Lua_State* L)
{
    int nRetCode = false;
    char* pszScript = NULL;

    nRetCode = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nRetCode == 1);

    pszScript = (char*)Lua_ValueToString(L, 1);

    g_pSO3World->m_ScriptManager.Reload(pszScript);

    LogInfo("Reload script:%s successfully!", pszScript);

Exit0:
    return 0;
}

int LuaScriptStatBegin(Lua_State* L)
{
    g_pSO3World->m_ScriptManager.StatBegin();

    return 0;
}

int LuaScriptStatEnd(Lua_State* L)
{
    g_pSO3World->m_ScriptManager.StatEnd();

    return 0;
}

int LuaGetHero(Lua_State* L)
{
    int         nResult     = 0;
    int         nRetCode    = false;
    DWORD       dwSceneID   = 0;
    DWORD       dwHeroID    = 0;
    KSceneObject* pObj      = NULL;
    KScene*     pScene      = NULL;
    KHero*      pHero       = NULL;

    nRetCode = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nRetCode == 2);

    dwSceneID = (DWORD)Lua_ValueToNumber(L, 1);
    dwHeroID = (DWORD)Lua_ValueToNumber(L, 2);

    pScene = g_pSO3World->m_SceneSet.GetObj(dwSceneID);
    KGLOG_PROCESS_ERROR(pScene);

    pObj = pScene->m_pSceneObjMgr->Get(dwHeroID);
    KGLOG_PROCESS_ERROR(pObj);
    KGLOG_PROCESS_ERROR(pObj->GetType() == sotHero);

    pHero = (KHero*)pObj;
    KG_PROCESS_ERROR(pHero->m_pScene);

    nResult = pHero->LuaGetObj(L);
Exit0:
    return nResult;
}

int LuaGetScene(Lua_State* L)
{
    int         nResult     = 0;
    int         nRetCode    = false;
    KScene*     pScene      = NULL;
    DWORD       dwMapID     = ERROR_ID;
    int         nCopyIndex  = 0;

    nRetCode = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nRetCode == 2);

    dwMapID = (DWORD)Lua_ValueToNumber(L, 1);
    nCopyIndex = (int)Lua_ValueToNumber(L, 2);

    pScene = g_pSO3World->GetScene(dwMapID, nCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    nResult = pScene->LuaGetObj(L);
Exit0:
    return nResult;
}

int LuaSetStatLogFlag(Lua_State* L)
{
	int		nTopIndex   = 0;
	BOOL	bFlag		= 0;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

	bFlag = (BOOL)Lua_ValueToBoolean(L, 1);

	g_pSO3World->m_bStatLogFlag = bFlag;

Exit0:
	return 0;
}

int LuaLog(Lua_State* L)
{
	int		nTopIndex   = 0;
    char*   pszLog      = NULL;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

	pszLog = (char*)Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszLog);

	KGLogPrintf(KGLOG_INFO, "%s", pszLog);

Exit0:
	return 0;
}

int LuaSendGlobalSysMsg(Lua_State* L)
{
    int         nTopIndex   = 0;
    const char* pszMsg      = NULL;
    size_t      uDataLen    = 0;
    IKG_Buffer* piBuffer    = NULL;
    BYTE*       pbyBuffer   = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    pszMsg = Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszMsg);

    uDataLen = strlen(pszMsg) + 1;

    piBuffer = KG_MemoryCreateBuffer((unsigned)uDataLen);
    KGLOG_PROCESS_ERROR(piBuffer);

    pbyBuffer = (BYTE*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pbyBuffer);

    strcpy((char*)pbyBuffer, pszMsg);
    pbyBuffer[uDataLen - 1] = '\0';

    g_LSClient.DoTalkMessage(trGlobalSys, ERROR_ID, "", ERROR_ID, "", uDataLen, (const char *)pbyBuffer);

Exit0:
    KG_COM_RELEASE(piBuffer);
    return 0;
}

int LuaSendGmAnnounce(Lua_State* L)
{
    int         nTopIndex   = 0;
    const char* pszMsg      = NULL;
    size_t      uDataLen    = 0;
    IKG_Buffer* piBuffer    = NULL;
    BYTE*       pbyBuffer   = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    pszMsg = Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszMsg);

    uDataLen = strlen(pszMsg) + 1;

    piBuffer = KG_MemoryCreateBuffer((unsigned)uDataLen);
    KGLOG_PROCESS_ERROR(piBuffer);

    pbyBuffer = (BYTE*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pbyBuffer);

    strcpy((char*)pbyBuffer, pszMsg);
    pbyBuffer[uDataLen - 1] = '\0';

    g_LSClient.DoTalkMessage(trGmAnnounce, ERROR_ID, "", ERROR_ID, "", uDataLen, (const char *)pbyBuffer);

Exit0:
    KG_COM_RELEASE(piBuffer);
    return 0;
}

int LuaAddDramaFrameInfo(Lua_State* L)
{
    BOOL    bRetCode    = false;
    int		nTopIndex   = 0;
    int     nDramaID    = 0;
    int     nSeconds    = 0;
    int     nExtFrame   = 0;
    char*   pszFunName  = NULL;
    int     nDialogID   = 0;
    bool    bPauseOnDialog = false;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 6);

    nDramaID = (int)Lua_ValueToNumber(L, 1);
    nSeconds = (int)Lua_ValueToNumber(L, 2);
    nExtFrame = (int)Lua_ValueToNumber(L, 3);
    pszFunName = (char*)Lua_ValueToString(L, 4);
    nDialogID = (int)Lua_ValueToNumber(L, 5);
    bPauseOnDialog = (bool)Lua_ValueToNumber(L, 6);

    bRetCode = g_pSO3World->m_DramaMgr.AddDramaFrameInfo(
        nDramaID, nSeconds * GAME_FPS + nExtFrame, pszFunName, nDialogID, bPauseOnDialog
    );
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int LuaReloadDrama(Lua_State* L)
{
    g_pSO3World->m_DramaMgr.ReloadDrama();
    return 0;
}

int LuaGetStructTime(Lua_State* L)
{
    int nResult = 0;
    int nTopIndex = 0;
    time_t nTime = 0;
    struct tm* pTime = NULL;
    
    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 0 || nTopIndex == 1);

    nTime = g_pSO3World->m_nCurrentTime;
    if (nTopIndex == 1)
        nTime = (time_t)Lua_ValueToNumber(L, 1);

    pTime = localtime(&nTime);
    KGLOG_PROCESS_ERROR(pTime);

    Lua_PushNumber(L, pTime->tm_year + 1900);
    Lua_PushNumber(L, pTime->tm_mon + 1);
    Lua_PushNumber(L, pTime->tm_mday);
    Lua_PushNumber(L, pTime->tm_hour);
    Lua_PushNumber(L, pTime->tm_min);
    Lua_PushNumber(L, pTime->tm_sec);

    nResult = 6;

Exit0:
    return nResult;
}

int LuaGetCurrentTime(Lua_State* L)
{
    Lua_PushNumber(L, g_pSO3World->m_nCurrentTime);
    return 1;
}

int LuaGetPlayerByID(Lua_State* L)
{
    int nResult = 0;
    BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    DWORD dwPlayerID = 0;
    KPlayer* pPlayer = NULL;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwPlayerID = (DWORD)Lua_ValueToNumber(L, 1);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    nResult = pPlayer->LuaGetObj(L);

Exit0:
	return nResult;
}

int LuaGetServerIndex(Lua_State* L)
{
    Lua_PushNumber(L, g_pSO3World->m_nServerIndexInLS);
    return 1;
}

int LuaGetSignSettings(Lua_State* L)
{
	int  nResult 	= 0;
	BOOL bRetCode	= false;
    int nTable = 0;

    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.nSignCoinCost);

    Lua_NewTable(L);
    nTable = Lua_GetTopIndex(L);

    Lua_PushNumber(L, 1);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSignCard.dwItemType);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 2);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSignCard.dwItemIndex);
    Lua_SetTable(L, nTable);
    
    Lua_PushNumber(L, 3);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSignCard.nCount);
    Lua_SetTable(L, nTable);

    Lua_NewTable(L);
    nTable = Lua_GetTopIndex(L);

    Lua_PushNumber(L, 1);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign5DaysAwardTotal.dwItemType);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 2);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign5DaysAwardTotal.dwItemIndex);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 3);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign5DaysAwardTotal.nCount);
    Lua_SetTable(L, nTable);

    Lua_NewTable(L);
    nTable = Lua_GetTopIndex(L);

    Lua_PushNumber(L, 1);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign10DaysAwardTotal.dwItemType);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 2);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign10DaysAwardTotal.dwItemIndex);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 3);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign10DaysAwardTotal.nCount);
    Lua_SetTable(L, nTable);

    Lua_NewTable(L);
    nTable = Lua_GetTopIndex(L);

    Lua_PushNumber(L, 1);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign21DaysAwardTotal.dwItemType);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 2);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign21DaysAwardTotal.dwItemIndex);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 3);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign21DaysAwardTotal.nCount);
    Lua_SetTable(L, nTable);

    Lua_NewTable(L);
    nTable = Lua_GetTopIndex(L);

    Lua_PushNumber(L, 1);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign2DaysAwardContinuous.dwItemType);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 2);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign2DaysAwardContinuous.dwItemIndex);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 3);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign2DaysAwardContinuous.nCount);
    Lua_SetTable(L, nTable);

    Lua_NewTable(L);
    nTable = Lua_GetTopIndex(L);

    Lua_PushNumber(L, 1);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign7DaysAwardContinuous.dwItemType);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 2);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign7DaysAwardContinuous.dwItemIndex);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 3);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign7DaysAwardContinuous.nCount);
    Lua_SetTable(L, nTable);

    Lua_NewTable(L);
    nTable = Lua_GetTopIndex(L);

    Lua_PushNumber(L, 1);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign14DaysAwardContinuous.dwItemType);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 2);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign14DaysAwardContinuous.dwItemIndex);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 3);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cSign14DaysAwardContinuous.nCount);
    Lua_SetTable(L, nTable);

    nResult = 8;
Exit0:
	return nResult;
}

int LuaGetFavoriteAward(Lua_State* L)
{
    int nTable = 0;
    
    Lua_NewTable(L);
    nTable = Lua_GetTopIndex(L);

    Lua_PushNumber(L, 1);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cFavoriteAward.dwItemType);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 2);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cFavoriteAward.dwItemIndex);
    Lua_SetTable(L, nTable);

    Lua_PushNumber(L, 3);
    Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cFavoriteAward.nCount);
    Lua_SetTable(L, nTable);

    return 1;
}

int LuaClearToplest(Lua_State* L)
{
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
	int	 nTopListType = 0;
	int	 nGroup = -1;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1 || nTopIndex == 2);

	nTopListType = (int)Lua_ValueToNumber(L, 1);
	if (nTopIndex == 2)
		nGroup = (int)Lua_ValueToNumber(L, 2);
	g_LSClient.DoClearTopList(nTopListType, nGroup);

Exit0:
	return 0;
}

int LuaGetFirstChargeAwardInfo(Lua_State* L)
{
    int nTable = 0;

    for (int i = 0; i < FIRST_CHARGE_AWARD_COUNT; ++i)
    {
        Lua_NewTable(L);
        nTable = Lua_GetTopIndex(L);

        Lua_PushNumber(L, 1);
        Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cFirstChargeAward[i].dwItemType);
        Lua_SetTable(L, nTable);

        Lua_PushNumber(L, 2);
        Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cFirstChargeAward[i].dwItemIndex);
        Lua_SetTable(L, nTable);

        Lua_PushNumber(L, 3);
        Lua_PushNumber(L, g_pSO3World->m_Settings.m_ConstList.cFirstChargeAward[i].nCount);
        Lua_SetTable(L, nTable);
    }
    return FIRST_CHARGE_AWARD_COUNT;
}

int LuaSetProtocolCloseFlag(Lua_State* L)
{
    BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    int  nProcotol  = 0;
    int  nCloseFlag = 0;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 2);

	nProcotol = (int)Lua_ValueToNumber(L, 1);
	KGLOG_PROCESS_ERROR(nProcotol > client_gs_connection_begin && nProcotol < client_gs_connection_end);

    nCloseFlag = (int)Lua_ValueToNumber(L, 2);

    g_PlayerServer.m_bCloseFlag[nProcotol] = nCloseFlag;

Exit0:
	return 0;
}

int LuaGetProtocolCloseFlag(Lua_State* L)
{
    int  nResult    = 0;
    BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    int  nProcotol  = 0;
    int  nCloseFlag = 0;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

	nProcotol = (int)Lua_ValueToNumber(L, 1);
	KGLOG_PROCESS_ERROR(nProcotol > client_gs_connection_begin && nProcotol < client_gs_connection_end);

    nCloseFlag = g_PlayerServer.m_bCloseFlag[nProcotol];

    Lua_PushNumber(L, nCloseFlag);

    nResult = 1;
Exit0:
	return nResult;
}

int LuaGetTimeByString(Lua_State* L)
{
    int     nResult     = 0;
    BOOL    bRetCode    = false;
    int     nTopIndex   = 0;
    char*   pszTime     = NULL;
    int     nTime       = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);
    
    pszTime = (char*)Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszTime);

    bRetCode = g_StringToTime(pszTime, nTime);
    KGLOG_PROCESS_ERROR(bRetCode);

    Lua_PushNumber(L, nTime);

    nResult = 1;
Exit0:
    return nResult;
}

int LuaLoadTabFile(Lua_State* L)
{
	int  nResult 	= 0;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    int nWidth	= 0;
    int nHeight	= 0;
    char szBuffer[10240];
    ITabFile* 	piTabFile 	= NULL;
    const char* pszTabFile = NULL;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    pszTabFile = Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszTabFile);

	piTabFile = g_OpenTabFile(pszTabFile);
	KGLOG_PROCESS_ERROR(piTabFile);

    nWidth = piTabFile->GetWidth();
    nHeight = piTabFile->GetHeight();

    Lua_NewTable(L);
	for(int i = 1; i <= nHeight; ++i)
	{
	    Lua_NewTable(L);
        for (int j = 1; j <= nWidth; j++)
        {
            bRetCode = piTabFile->GetString(i, j, "", szBuffer, sizeof(szBuffer));
            KGLOG_CHECK_ERROR(bRetCode);

            Lua_PushString(L, szBuffer);
            Lua_RawSetI(L, -2, j);
            	
        }
        Lua_RawSetI(L, -2, i);
	}

    nResult = 1;
Exit0:
    KG_COM_RELEASE(piTabFile);
	return nResult;
}

// ----------------------------------------------

static KLuaFunc g_ScriptFuncList[] = 
{
    {"Test",                            LuaTest}, 
    
    {"ReloadAll",                       LuaReloadAll},
    {"ReloadOne",                       LuaReloadOne},
    {"GetHero",                         LuaGetHero},
    {"ScriptStatBegin",                 LuaScriptStatBegin},
    {"ScriptStatEnd",                   LuaScriptStatEnd},
    {"GetScene",                        LuaGetScene},
    {"SetStatLogFlag",                  LuaSetStatLogFlag},
    {"Log",                             LuaLog},
    {"SendGlobalSysMsg",                LuaSendGlobalSysMsg},
    {"SendGmAnnounce",                  LuaSendGmAnnounce},
    {"AddDramaFrameInfo",               LuaAddDramaFrameInfo},
    {"ReloadDrama",                     LuaReloadDrama},
    {"GetStructTime",                   LuaGetStructTime},
    {"GetCurrentTime",                  LuaGetCurrentTime},
    {"GetPlayerByID",                   LuaGetPlayerByID},
    {"GetServerIndex",                  LuaGetServerIndex},
    {"GetSignSettings",                 LuaGetSignSettings},
    {"GetFavoriteAward",                LuaGetFavoriteAward},
	{"ClearTopList",					LuaClearToplest},
    {"GetFirstChargeAwardInfo",         LuaGetFirstChargeAwardInfo},
    {"SetProtocolCloseFlag",            LuaSetProtocolCloseFlag},
    {"GetProtocolCloseFlag",            LuaGetProtocolCloseFlag},
    {"GetTimeByString",                 LuaGetTimeByString},
    {"ReloadQuest",                     LuaReloadQuest},
    {"ReloadAward",                     LuaReloadAward},
    {"C_LoadTabFile",                   LuaLoadTabFile},

};

BOOL RegisterScriptFunctions(ILuaScriptEx* piScript)
{
	int nCount = (int)(sizeof(g_ScriptFuncList) / sizeof(g_ScriptFuncList[0]));

	piScript->RegisterFunctions(g_ScriptFuncList, nCount);

    return true;
}
