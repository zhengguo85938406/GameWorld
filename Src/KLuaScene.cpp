#include "stdafx.h"
#include "KScene.h"
#include "KPlayerServer.h"
#include "KBasket.h"
#include "KCandyBag.h"
#include "KLandMine.h"
#include "KPlayer.h"

int KScene::LuaSetTimer(Lua_State* L)
{
    int         nParamCount     = 0;
    DWORD       dwID            = 0;
    int         nTime           = 0;
    const char* pszScriptName   = NULL;
    int         nParam1         = 0;
    int         nParam2         = 0;

    nParamCount = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nParamCount == 4);

    nTime           =   (int)Lua_ValueToNumber(L, 1);
    pszScriptName   =   Lua_ValueToString(L, 2);
    nParam1         =   (int)Lua_ValueToNumber(L, 3);
    nParam2         =   (int)Lua_ValueToNumber(L, 4);

    KGLOG_PROCESS_ERROR(pszScriptName);

    dwID = m_ScriptTimerList.SetTimer(nTime, pszScriptName, nParam1, nParam2);
    KGLOG_PROCESS_ERROR(dwID);

Exit0:
    lua_pushinteger(L, dwID);
    return 1;
}

int KScene::LuaStopTimer(Lua_State* L)
{
    BOOL    bResult        = false;
    BOOL    bRetCode       = false;
    int     nParamCount    = 0;
    DWORD   dwTimerID      = 0;

    nParamCount = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nParamCount == 1);

    dwTimerID = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_ScriptTimerList.StopTimer(dwTimerID);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KScene::LuaRestartTimer(Lua_State* L)
{
    BOOL    bResult        = false;
    BOOL    bRetCode       = false;
    int     nParamCount    = 0;
    DWORD   dwTimerID      = 0;

    nParamCount = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nParamCount == 1);

    dwTimerID = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_ScriptTimerList.RestartTimer(dwTimerID);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KScene::LuaRemoveTimer(Lua_State* L)
{
    BOOL    bResult        = false;
    BOOL    bRetCode       = false;
    int     nParamCount    = 0;
    DWORD   dwTimerID      = 0;

    nParamCount = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nParamCount == 1);

    dwTimerID = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_ScriptTimerList.RemoveTimer(dwTimerID);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KScene::LuaCreateSceneObj(Lua_State* L)
{
    return LuaAddSceneObj(L);
}

int KScene::LuaCreateDoodad(Lua_State* L)
{
    return LuaAddSceneObj(L);
}

int KScene::LuaSetBattleTotalFrame(Lua_State* L)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    int nTopIndex = 0;
    int nTotalFrame = 0;
    BOOL bReTiming = false;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1 || nTopIndex == 2);

    nTotalFrame = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nTotalFrame > 0);

    if (nTopIndex == 2)
        bReTiming = (BOOL)Lua_ValueToBoolean(L, 2);

    bRetCode = m_Battle.SetBattleTotalFrame(nTotalFrame, bReTiming);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}


int KScene::LuaAddHero(Lua_State* L)
{
    int     nResult         = 0;
    BOOL    bRetCode        = false;
    int     nTopIndex       = 0;
    DWORD   dwTemplateID    = ERROR_ID;
    int     nLevel          = 0;
    int     nSide           = 0;
    int     nAIType         = 0;
    KHero*  pHero           = NULL;
    int     nX              = 0;
    int     nY              = 0;
    int     nZ              = 0;
    int     nPos            = -1;
    int     nFaceDir        = csdInvalid;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 4 || nTopIndex == 7 || nTopIndex == 8);

    dwTemplateID    = (DWORD)Lua_ValueToNumber(L, 1);
    nLevel          = (int)Lua_ValueToNumber(L, 2);
    nSide           = (int)Lua_ValueToNumber(L, 3);
    nAIType         = (int)Lua_ValueToNumber(L, 4);

    if (nTopIndex == 7 || nTopIndex == 8)
    {
        nX = (int)Lua_ValueToNumber(L, 5);
        nY = (int)Lua_ValueToNumber(L, 6);
        nZ = (int)Lua_ValueToNumber(L, 7);
    }

    if (nTopIndex == 8)
        nFaceDir = (int)Lua_ValueToNumber(L, 8);

    bRetCode = GetFreePos(nSide, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    pHero = AddHero(dwTemplateID, nLevel, nSide, nPos, nFaceDir);
    KGLOG_PROCESS_ERROR(pHero);

    pHero->SetPosition(nX, nY, nZ);

    if (nAIType != 0)
        pHero->m_AIData.nAIType = nAIType;
    
    pHero->m_bAiMode = true;
    pHero->SetUpAVM();

    g_PlayerServer.DoBroadcastNewHero(pHero, ERROR_ID);

    nResult = pHero->LuaGetObj(L);
Exit0:
    return nResult;
}

int KScene::LuaRemoveDoodad(Lua_State* L)
{
    int     nResult         = 0;
    BOOL    bRetCode        = false;
    int     nTopIndex       = 0;
    int     nPosX           = 0;
    int     nPosY           = 0;
    int     nRange          = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nPosX       = (int)Lua_ValueToNumber(L, 1);
    nPosY       = (int)Lua_ValueToNumber(L, 2);
    nRange      = (int)Lua_ValueToNumber(L, 3);
    
    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!g_IsDoodad(pObj))
            continue;

        assert(pObj->m_pTemplate);

        if (pObj->m_pTemplate->m_bImpregnability) // ²»¿ÉÒÆ³ý
            continue;

        if (nRange != -1) // ¼ì²é¾àÀë
        {
            KPOSITION pos = pObj->GetPosition();
            bRetCode = g_InRange(nPosX, nPosY, 0, pos.nX, pos.nY, 0, nRange);
            if (!bRetCode)
                continue;
        }
        RemoveSceneObject(pObj);
    }
    
Exit0:
    return 0;
}

int KScene::LuaGetPlayerHero(Lua_State* L)
{
    KHero*  pResult         = NULL;
    int     nTopIndex       = 0;
    DWORD   dwTemplateID    = ERROR_ID;
    KHero*  pHero           = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwTemplateID = (DWORD)Lua_ValueToNumber(L, 1);

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;
        if (pHero->IsNpc())
            continue;

        if (pHero->m_dwTemplateID == dwTemplateID)
        {
            pResult = pHero;
            break;
        }
    }

	KG_PROCESS_ERROR(pResult);
    
    return pResult->LuaGetObj(L);
Exit0:
    return 0;
}

int KScene::LuaGetNpcHero(Lua_State* L)
{
    int     nRetCode        = 0;
    KHero*  pResult         = NULL;
    int     nTopIndex       = 0;
    DWORD   dwTemplateID    = ERROR_ID;
    KHero*  pHero           = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwTemplateID = (DWORD)Lua_ValueToNumber(L, 1);

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;
        if (!pHero->IsNpc())
            continue;

        KGLOG_PROCESS_ERROR(pHero->m_pNpcTemplate);

        if (pHero->m_pNpcTemplate->dwID == dwTemplateID)
        {
            pResult = pHero;
            break;
        }
    }
    KG_PROCESS_ERROR(pResult);

    nRetCode = pResult->LuaGetObj(L);
Exit0:
    return nRetCode;
}

int KScene::LuaGetDoodadCount(Lua_State* L)
{
    int     nResult         = 0;
    int     nTopIndex       = 0;
    DWORD   dwTemplateID    = ERROR_ID;
    int     nCount          = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwTemplateID = (DWORD)Lua_ValueToNumber(L, 1);

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!g_IsDoodad(pObj))
            continue;

        assert(pObj->m_pTemplate);
        if (pObj->m_pTemplate->m_dwID != dwTemplateID)
            continue;
            
        ++nCount;
    }

    lua_pushinteger(L, nCount);

    nResult = 1;
Exit0:
    return nResult;
}

int KScene::LuaGetSceneObjCountByType(Lua_State* L)
{
    int                 nResult         = 0;
    int                 nTopIndex       = 0;
    KSceneObjectType    eType           = sotInvalid;
    int                 nCount          = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    eType = (KSceneObjectType)(int)Lua_ValueToNumber(L, 1);

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        assert(pObj);
        
        if (pObj->GetType() == eType)
            ++nCount;
    }

    lua_pushinteger(L, nCount);

    nResult = 1;
Exit0:
    return nResult;
}

int KScene::LuaSetBasketNotCostHP(Lua_State* L)
{
    int     nTopIndex       = 0;
    BOOL    bCanCostHP      = false;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    bCanCostHP = (BOOL)Lua_ValueToBoolean(L, 1);

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() == sotBasket)
        {
            KBasket* pBasket = (KBasket*)pObj;
            pBasket->m_bNotCostHP = bCanCostHP;
        }
    }

Exit0:
    return 0;
}

int KScene::LuaSetScore(Lua_State* L)
{
    int nTopIndex = 0;
    int nSide = 0;
    int nScore = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    nSide = (int)Lua_ValueToNumber(L, 1);
    nScore = (int)Lua_ValueToNumber(L, 2);

    KGLOG_PROCESS_ERROR(nSide == sidRight || nSide == sidLeft);
    KGLOG_PROCESS_ERROR(nScore >= 0);

    m_nScore[nSide] = nScore;

    g_PlayerServer.DoSyncScore(this, nSide, m_nScore[nSide]);

Exit0:
    return 0;
}

int KScene::LuaAddNpc(Lua_State* L)
{
    int         nResult         = 0;
    BOOL        bRetCode        = false;
    KHero*      pNpc            = NULL;
    int         nTopIndex       = 0;
    DWORD       dwNpcTemplateID = ERROR_ID;
    int         nSide           = 0;
    KPOSITION   cPos;
    int         nFaceDir        = 0;
    int         nAIType         = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 6 || nTopIndex == 7);

    dwNpcTemplateID = (DWORD)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(dwNpcTemplateID);

    nSide = (int)Lua_ValueToNumber(L, 2);
    KGLOG_PROCESS_ERROR(nSide == sidRight || nSide == sidLeft);

    cPos.nX = (int)Lua_ValueToNumber(L, 3);
    KGLOG_PROCESS_ERROR(cPos.nX >= 0 && cPos.nX <= GetLength());

    cPos.nY = (int)Lua_ValueToNumber(L, 4);
    KGLOG_PROCESS_ERROR(cPos.nY >= 0 && cPos.nY <= GetWidth());

    cPos.nZ = (int)Lua_ValueToNumber(L, 5);
    KGLOG_PROCESS_ERROR(cPos.nZ >= 0);

    nFaceDir = (int)Lua_ValueToNumber(L, 6);
    KGLOG_PROCESS_ERROR(nFaceDir == csdRight || nFaceDir == csdLeft);

    if (nTopIndex >= 7)
    {
        nAIType = (int)(Lua_ValueToNumber(L, 7));
    }

    pNpc = AddNpc(dwNpcTemplateID, nSide, cPos, nFaceDir, nAIType);
    KGLOG_PROCESS_ERROR(pNpc);

    nResult = pNpc->LuaGetObj(L);
Exit0:
    return nResult;
}

int KScene::LuaAddCandyBagOnBasket(Lua_State* L)
{
    BOOL                    bResult             = false;
    BOOL                    bRetCode            = false;
    int                     nTopIndex           = 0;
    DWORD                   dwCandyTemplateID   = ERROR_ID;
    int                     nSide               = 0;
    int                     nBasketFloor        = 0;
    KSceneObjectTemplate*   pTemplate           = NULL;
    KSceneObject*           pObj                = NULL;
    KCandyBag*              pCandyBag           = NULL;
    KBasket*                pBasket             = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    dwCandyTemplateID   = (DWORD)Lua_ValueToNumber(L, 1);
    nSide               = (int)Lua_ValueToNumber(L, 2);
    nBasketFloor        = (int)Lua_ValueToNumber(L, 3);
    
    pTemplate = g_pSO3World->m_Settings.m_ObjTemplate.GetTemplate(dwCandyTemplateID);
    KGLOG_PROCESS_ERROR(pTemplate);
    KGLOG_PROCESS_ERROR(pTemplate->m_nType == sotCandyBag);

    KGLOG_PROCESS_ERROR(nSide == sidLeft || nSide == sidRight);
    KGLOG_PROCESS_ERROR(nBasketFloor >= bfFirst && nBasketFloor <= bfSixth);

    pBasket = GetBasket(nSide, nBasketFloor);
    KGLOG_PROCESS_ERROR(pBasket);

    pObj = m_pSceneObjMgr->AddByTemplate(dwCandyTemplateID);
    KGLOG_PROCESS_ERROR(pObj);
    KGLOG_PROCESS_ERROR(pObj->Is(sotCandyBag));

    pCandyBag = (KCandyBag*)pObj;
    pCandyBag->FixTo(pBasket->GetPosition());

    g_PlayerServer.DoSyncSceneObject(pCandyBag, -1);

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (pCandyBag)
        {
            pCandyBag->SetDeleteFlag();
            pCandyBag = NULL;
        }
    }

    Lua_PushBoolean(L, bResult);
    return 1;
}

int KScene::LuaAddSceneObj(Lua_State* L )
{
	DWORD                   dwObjID             = 0;
	BOOL                    bRetCode            = false;
	int                     nTopIndex           = 0;
	DWORD                   dwTemplateID        = ERROR_ID;
	int                     nX					= 0;
	int                     nY					= 0;
	int						nZ					= 0;
	int						nVelocityX			= 0;
	int						nVelocityY			= 0;
	int						nVelocityZ			= 0;
	KSceneObjectTemplate*   pTemplate           = NULL;
	KSceneObject*           pObj                = NULL;
	KMovableObject*			pMovableObj			= NULL;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 4 || nTopIndex == 7);

	dwTemplateID        = (DWORD)Lua_ValueToNumber(L, 1);
	nX					= (int)Lua_ValueToNumber(L, 2);
	nY					= (int)Lua_ValueToNumber(L, 3);
	nZ					= (int)Lua_ValueToNumber(L, 4);

    if (nTopIndex == 7)
    {
        nVelocityX			= (int)Lua_ValueToNumber(L, 5);
        nVelocityY			= (int)Lua_ValueToNumber(L, 6);
        nVelocityZ			= (int)Lua_ValueToNumber(L, 7);
    }

	KGLOG_PROCESS_ERROR(nX >= 0 && nX < GetLength() && nY >= 0 && nY < GetWidth() && nZ >= 0);

	pTemplate = g_pSO3World->m_Settings.m_ObjTemplate.GetTemplate(dwTemplateID);
	KGLOG_PROCESS_ERROR(pTemplate);

	pObj = m_pSceneObjMgr->AddByTemplate(dwTemplateID);
	KGLOG_PROCESS_ERROR(pObj);

    pObj->SetPosition(nX, nY, nZ);

    if (pObj->m_pTemplate && pObj->m_pTemplate->m_Obstacle)
    {
        ApplyObstacle(pObj, true); 
    }

	pMovableObj = dynamic_cast<KMovableObject*>(pObj);
    if (pMovableObj)
    {
        pMovableObj->m_nVelocityX = nVelocityX;
        pMovableObj->m_nVelocityY = nVelocityY;
        pMovableObj->m_nVelocityZ = nVelocityZ;
    }

	g_PlayerServer.DoSyncSceneObject(pObj);

	dwObjID = pObj->m_dwID;
Exit0:
	if (dwObjID == 0)
	{
		if (pObj)
		{
			pObj->SetDeleteFlag();
			pObj = NULL;
		}
	}

	Lua_PushNumber(L, dwObjID);
	return 1;
}

int KScene::LuaSetGravity(Lua_State* L)
{
    int nTopIndex   = 0;
    int nNewGravity = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nNewGravity = (int)Lua_ValueToNumber(L, 1);

    m_nGlobalGravity = nNewGravity;

    g_PlayerServer.DoSyncSceneGravity(this);

    LogInfo("Set scene Gravity to %d", nNewGravity);

Exit0:
    return 0;
}

int KScene::LuaGetGravity(Lua_State* L)
{
    lua_pushinteger(L, m_nGlobalGravity);
    return 1;
}

int KScene::LuaGetObjType(Lua_State* L)
{
    int             nResult     = 0;
    int             nTopIndex   = 0;
    uint32_t        dwObjID     = 0;
    KSceneObject*   pObj        = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwObjID = (uint32_t)Lua_ValueToNumber(L, 1);

    pObj = m_pSceneObjMgr->Get(dwObjID);
    KGLOG_PROCESS_ERROR(pObj);

    lua_pushinteger(L, pObj->GetType());

    nResult = 1;
Exit0:
    return nResult;
}

int KScene::LuaRemoveObj(Lua_State* L)
{
    int             nResult     = 0;
    int             nTopIndex   = 0;
    uint32_t        dwObjID     = 0;
    KSceneObject*   pObj        = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwObjID = (uint32_t)Lua_ValueToNumber(L, 1);

    pObj = m_pSceneObjMgr->Get(dwObjID);
    KGLOG_PROCESS_ERROR(pObj);
    RemoveSceneObject(pObj);

Exit0:
    return 0;
}

int KScene::LuaRemoveObjByTemplateID(Lua_State* L)
{
    int             nResult         = 0;
    int             nTopIndex       = 0;
    uint32_t        dwTemplateID    = 0;
    KSceneObject*   pObj            = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwTemplateID = (uint32_t)Lua_ValueToNumber(L, 1);

    m_pSceneObjMgr->RemoveByTemplate(dwTemplateID);

Exit0:
    return 0;
}

int KScene::LuaGetObjPosition(Lua_State* L)
{
    int             nResult     = 0;
    int             nTopIndex   = 0;
    uint32_t        dwObjID     = 0;
    KSceneObject*   pObj        = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwObjID = (uint32_t)Lua_ValueToNumber(L, 1);

    pObj = m_pSceneObjMgr->Get(dwObjID);
    KGLOG_PROCESS_ERROR(pObj);
    
    lua_pushinteger(L, pObj->m_nX);
    lua_pushinteger(L, pObj->m_nY);
    lua_pushinteger(L, pObj->m_nZ);

    nResult = 3;
Exit0:
    return nResult;
}

int KScene::LuaSetObjPosition(Lua_State* L)
{
    int             nResult     = 0;
    int             nTopIndex   = 0;
    uint32_t        dwObjID     = 0;
    KSceneObject*   pObj        = NULL;
    int             nX          = 0;
    int             nY          = 0;
    int             nZ          = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 4);

    dwObjID = (uint32_t)Lua_ValueToNumber(L, 1);
    nX      = (int)Lua_ValueToNumber(L, 2);
    nY      = (int)Lua_ValueToNumber(L, 3);
    nZ      = (int)Lua_ValueToNumber(L, 4);

    pObj = m_pSceneObjMgr->Get(dwObjID);
    KGLOG_PROCESS_ERROR(pObj);

    pObj->SetPosition(nX, nY, nZ);

    g_PlayerServer.DoSyncSceneObjPosition(pObj);
    
Exit0:
    return 0;
}

int KScene::LuaGetBall(Lua_State* L)
{
    KBall* pBall = NULL;
    pBall = GetBall();
    KGLOG_PROCESS_ERROR(pBall);
    return pBall->LuaGetObj(L);
Exit0:
    return 0;
}

int KScene::LuaUpdateSceneObjectObstacle(Lua_State* L)
{
	BOOL bResult 	= false;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    DWORD dwSceneObjectID = ERROR_ID;
    BOOL bApply = false;
    KSceneObject* pSceneObject = NULL;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 2);

    dwSceneObjectID = (DWORD)Lua_ValueToNumber(L, 1);
    bApply = (BOOL)Lua_ValueToNumber(L, 2);

    pSceneObject = GetSceneObjByID(dwSceneObjectID);
    KGLOG_PROCESS_ERROR(pSceneObject);

    ApplyObstacle(pSceneObject, bApply);

	bResult = true;
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KScene::LuaStartDrama(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    int  nDramaID   = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nDramaID = (DWORD)Lua_ValueToNumber(L, 1);
    
    bRetCode = m_DramaPlayer.StartDrama(nDramaID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KScene::LuaStopDrama(Lua_State* L)
{
    m_DramaPlayer.StopDrama();
    return 0;
}

int KScene::LuaShowAllHeroAI(Lua_State* L)
{
    int nAILevel = 0;

    KGLogPrintf(KGLOG_INFO, "Begin ShowAllHeroAI ...");
    
    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.GetValue())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;

        KHero* pHero = (KHero*)pObj;

        g_pSO3World->m_AIManager.GetAILevel(pHero->m_AIData.nAIType, nAILevel);

        KGLogPrintf(KGLOG_INFO, "Hero ID:%u, Sid:%d, SeatPos:%d, IsAIMode:%d, AILevel:%d, AIType:%d", 
            pHero->m_dwID, pHero->m_nSide, pHero->m_nPos, pHero->m_bAiMode, pHero->m_AIData.nAIType, nAILevel);
    }

    KGLogPrintf(KGLOG_INFO, "End ShowAllHeroAI ...");

    return 0;
}

int KScene::LuaFixObj(Lua_State* L)
{
    BOOL bResult 	= false;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    int  nObjID     = 0;
    KSceneObject* pSceneObj = NULL;
    KMovableObject* pMoveObj = NULL;
    KPOSITION pos;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 4);

    nObjID = (DWORD)Lua_ValueToNumber(L, 1);
    pos.nX = (int)Lua_ValueToNumber(L, 2);
    pos.nY = (int)Lua_ValueToNumber(L, 3);
    pos.nZ = (int)Lua_ValueToNumber(L, 4);

    pSceneObj = GetSceneObjByID(nObjID);
    KGLOG_PROCESS_ERROR(pSceneObj);
    
    pMoveObj = dynamic_cast<KMovableObject*>(pSceneObj);
    KGLOG_PROCESS_ERROR(pMoveObj);

    pMoveObj->FixTo(pos);
    g_PlayerServer.DoSyncSceneObject(pMoveObj, -1);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KScene::LuaSetBattleTemplate(Lua_State* L)
{
    BOOL bResult 	        = false;
    BOOL bRetCode	        = false;
    int	 nTopIndex	        = 0;
    int  nBattleTemplateID  = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nBattleTemplateID = (DWORD)Lua_ValueToNumber(L, 1);

    m_Battle.ApplyTemplate(nBattleTemplateID);

    g_PlayerServer.DoSetBattleTemplate(this);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KScene::LuaActiveLandMine(Lua_State* L)
{
    int nResult = 0;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    DWORD  dwLandMineID = 0;
    KLandMine* pLandMine = NULL;
    KSceneObject* pObj = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwLandMineID = (DWORD)Lua_ValueToNumber(L, 1);

    pObj = GetSceneObjByID(dwLandMineID);
    KGLOG_PROCESS_ERROR(pObj);
    KGLOG_PROCESS_ERROR(pObj->GetType() == sotLandMine);

    pLandMine = (KLandMine*)pObj;

    pLandMine->ActiveState();

Exit0:
    return nResult;
}

int KScene::LuaPause(Lua_State* L)
{
	int		nTopIndex	= 0;
	BOOL	bPause		= 0;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

	m_bPause = Lua_ValueToBoolean(L, 1);
	
	g_PlayerServer.DoBroadcastScenePause(this, m_bPause);

Exit0:
	return 0;
}

int KScene::LuaResetBattle(Lua_State* L)
{
    m_nShouldStartFrame = g_pSO3World->m_nGameLoop + BATTLE_START_COUNT_DOWN;
    m_eSceneState = ssCountDown;

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotHero)
            continue;

        KHero* pHero = (KHero*)pObj;
        if (pHero->IsMainHero())
        {
            KPlayer* pPlayer = pHero->GetOwner();
            KGLOG_PROCESS_ERROR(pPlayer);

            g_PlayerServer.DoRemoteCall(pPlayer->m_nConnIndex, "onResetBattle", BATTLE_START_COUNT_DOWN, m_Battle.m_nFramePerBattle);
        }
    }

Exit0:
    return 0;
}

int KScene::LuaGetAllHero(Lua_State* L)
{
    KHero*  pResult         = NULL;
    KHero*  pHero           = NULL;
    int     nIndex          = 1;

    lua_newtable(L);

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;

        lua_pushinteger(L, nIndex);
        pHero->LuaGetObj(L);
        lua_settable(L, -3);

        ++nIndex;
    }

Exit0:
    return 1;
}

DEFINE_LUA_CLASS_BEGIN(KScene)
    REGISTER_LUA_DWORD(KObject, ID)
    REGISTER_LUA_INTEGER(KScene, XGridCount)
    REGISTER_LUA_INTEGER(KScene, YGridCount)

    REGISTER_LUA_FUNC(KScene, SetTimer)
    REGISTER_LUA_FUNC(KScene, StopTimer)
    REGISTER_LUA_FUNC(KScene, RestartTimer)
    REGISTER_LUA_FUNC(KScene, RemoveTimer)
    REGISTER_LUA_FUNC(KScene, CreateDoodad)
    REGISTER_LUA_FUNC(KScene, CreateSceneObj)
    REGISTER_LUA_FUNC(KScene, SetBattleTotalFrame)
    REGISTER_LUA_FUNC(KScene, AddHero)
    REGISTER_LUA_FUNC(KScene, RemoveDoodad)
    REGISTER_LUA_FUNC(KScene, GetDoodadCount)
    REGISTER_LUA_FUNC(KScene, GetSceneObjCountByType)
    REGISTER_LUA_FUNC(KScene, SetBasketNotCostHP)
    REGISTER_LUA_FUNC(KScene, SetScore)
    REGISTER_LUA_FUNC(KScene, AddNpc)
    REGISTER_LUA_FUNC(KScene, AddCandyBagOnBasket)
    REGISTER_LUA_FUNC(KScene, AddSceneObj)
    REGISTER_LUA_FUNC(KScene, SetGravity)
    REGISTER_LUA_FUNC(KScene, GetGravity)
    REGISTER_LUA_FUNC(KScene, GetObjType)
    REGISTER_LUA_FUNC(KScene, RemoveObj)
    REGISTER_LUA_FUNC(KScene, RemoveObjByTemplateID)
    REGISTER_LUA_FUNC(KScene, GetObjPosition)
    REGISTER_LUA_FUNC(KScene, SetObjPosition)
    REGISTER_LUA_FUNC(KScene, GetBall)
    REGISTER_LUA_FUNC(KScene, UpdateSceneObjectObstacle)
    REGISTER_LUA_FUNC(KScene, StartDrama)
    REGISTER_LUA_FUNC(KScene, StopDrama)
    REGISTER_LUA_FUNC(KScene, ShowAllHeroAI)
    REGISTER_LUA_FUNC(KScene, GetPlayerHero)
    REGISTER_LUA_FUNC(KScene, GetNpcHero)
    REGISTER_LUA_FUNC(KScene, FixObj)
    REGISTER_LUA_FUNC(KScene, SetBattleTemplate)
    REGISTER_LUA_FUNC(KScene, ActiveLandMine)
	REGISTER_LUA_FUNC(KScene, Pause)
    REGISTER_LUA_FUNC(KScene, ResetBattle)
    REGISTER_LUA_FUNC(KScene, GetAllHero)

DEFINE_LUA_CLASS_END(KScene)
