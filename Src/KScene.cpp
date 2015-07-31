#include "stdafx.h"
#include "KScene.h"
#include "KHero.h"
#include "KMath.h"
#include "KTerrainData.h"
#include "Engine/KMemory.h"
#include "Engine/KG_Time.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KDoodad.h"
#include "KBasket.h"
#include "KBall.h"
#include "KBackboard.h"
#include "KChest.h"
#include "KLadder.h"
#include "KRelayClient.h"
#include "KSceneObjMgr.h"
#include "KLadderLevelData.h"
#include "KPlayer.h"
#include <iterator>
#include "KMovableObstacle.h"
#include "KGold.h"
#include "KLSClient.h"

KScene::KScene()
{
    m_pSceneObjMgr              = NULL;
    m_pTerrainScene             = NULL;
    m_nXGridCount               = 0;
    m_nYGridCount               = 0;
    m_pMapParam                 = NULL;
    m_nShouldStartFrame         = 0;
    m_nGlobalGravity            = g_pSO3World->m_Settings.m_ConstList.nGravityDefault;
    memset(m_bReady, 0, sizeof(m_bReady));
    m_bPause                    = false;
    m_nClientLoadingFrame       = 0;
    m_nEnterPlayerCount         = 0;
    m_bLoadNpcAfterAllPlayerEnter = false;

    memset(m_LadderPVPFatigueCosted, 0, sizeof(m_LadderPVPFatigueCosted));
}

KScene::~KScene()
{
}

BOOL KScene::Init()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    BOOL bDoodadMgrInitialized = false;
    BOOL bCollisionMgrInitialized = false;
    BOOL bBattleInitialized = false;
	KBall* pBall	= NULL;
    BOOL bObjMgrInitialized = false;
    BOOL bScriptTimerInitFlag = false;
    BOOL bDramaPlayerInitFlag = false;
    BOOL bPathFinderInitFlag = false;

    bRetCode = g_pSO3World->m_SceneSet.Register(this, ERROR_ID);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_pSceneObjMgr = KMEMORY_NEW(KSceneObjMgr);
    assert(m_pSceneObjMgr);

    bRetCode = m_pSceneObjMgr->Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bObjMgrInitialized = true;

    bRetCode = m_cCollisionMgr.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bCollisionMgrInitialized = true;

    bRetCode = m_Battle.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bBattleInitialized = true;

    bRetCode = m_ScriptTimerList.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bScriptTimerInitFlag = true;

    bRetCode = m_DramaPlayer.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bDramaPlayerInitFlag = true;

    bRetCode = m_PathFinder.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bPathFinderInitFlag = true;

	m_dwMapID			    = 0;
	m_nCopyIndex	        = 0;
	m_eSceneState		    = ssInvalid;

    memset(&m_nScore[0], 0, sizeof(m_nScore));
    memset(&m_Param, 0, sizeof(m_Param));

    m_pCells = NULL;

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bDramaPlayerInitFlag)
        {
            m_DramaPlayer.UnInit();
            bDramaPlayerInitFlag = false;
        }
        if (bPathFinderInitFlag)
        {
            m_PathFinder.UnInit();
            bPathFinderInitFlag = false;
        }
        

        if (bScriptTimerInitFlag)
        {
            m_ScriptTimerList.UnInit();
            bScriptTimerInitFlag = false;
        }
        
        if (bCollisionMgrInitialized)
        {
            m_cCollisionMgr.UnInit();
            bCollisionMgrInitialized = false;
        }

        if (bObjMgrInitialized)
        {
            m_pSceneObjMgr->UnInit();
            bObjMgrInitialized = false;
        }
        
        KMEMORY_DELETE(m_pSceneObjMgr);

        if (bBattleInitialized)
        {
            m_Battle.UnInit();
            bBattleInitialized = false;
        }
    }

	return bResult;
}

void KScene::UnInit()
{
    m_DramaPlayer.UnInit();
    m_PathFinder.UnInit();
    m_ScriptTimerList.UnInit();
    m_cCollisionMgr.UnInit();
    m_Battle.UnInit();

    if (m_pSceneObjMgr)
    {
        m_pSceneObjMgr->UnInit();
        KMEMORY_DELETE(m_pSceneObjMgr);
    }

    KG_COM_RELEASE(m_pTerrainScene);
    g_pSO3World->m_SceneSet.Unregister(this);
}

int KScene::Activate(int nCurLoop)
{  
    switch (m_eSceneState)
    {
    case ssWaitingClientLoading:
        OnClientLoading(nCurLoop);
        break;
    case ssCountDown:
        OnCountDown(nCurLoop);
        break;
    case ssFighting:
        OnFighting(nCurLoop);
        break;
    }

    return TRUE;
}


BOOL KScene::ServerLoad()
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;

    assert(m_pTerrainScene);

    bRetCode = LoadBaseInfo();
    KGLOG_PROCESS_ERROR(bRetCode);

    if (!m_pTerrainScene->m_bLoadFlag)
    {
        bRetCode = LoadTerrain();
        KGLOG_PROCESS_ERROR(bRetCode);

        m_pTerrainScene->m_bLoadFlag = true;
    }

    bRetCode = m_PathFinder.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = ApplyAllObstacle();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KScene::LoadTerrain()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KMapParams* pMapInfo = NULL;
    char        szFileName[MAX_PATH];
    IFile*          piFile              = NULL;
    IKG_Buffer*     piBuffer            = NULL;
    BYTE*           pbyBuffer           = NULL;
    size_t          uFileSize           = 0;
    size_t          uReadBytes          = 0;
    size_t          uLeftBytes          = 0;
    BYTE*           pbyOffset           = NULL;
    KSCENE_TERRAIN_DATA_HEADER*  pFileHeader = NULL;

    pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapInfo);

    m_pMapParam = pMapInfo;

    if (pMapInfo->szResourceFilePath[0] == '\0') // 没填地图障碍文件,使用默认
    {
        for (int nCellY = 0; nCellY < SCENE_GRID_HEIGHT; nCellY++)
        {
            for (int nCellX = 0; nCellX < SCENE_GRID_WIDTH; nCellX++)
            {
                KCell* pCell = m_pCells + nCellY * SCENE_GRID_WIDTH + nCellX;

                if (nCellX < m_nXGridCount && nCellY < m_nYGridCount)
                    pCell->byObstacleType = Obstacle_NULL;
                else
                    pCell->byObstacleType = Obstacle_Normal;

                pCell->wHeight  = 0;
            }
        }
        goto Exit1;
    }

    snprintf(szFileName, sizeof(szFileName), "%s/%u/%s", MAP_DIR, pMapInfo->nServerResourceID, pMapInfo->szResourceFilePath);
    szFileName[sizeof(szFileName) - 1] = '\0';
        
    piFile = g_OpenFile(szFileName);
    KGLOG_PROCESS_ERROR(piFile);

    uFileSize = piFile->Size();
    KGLOG_PROCESS_ERROR(uFileSize > 0);

    piBuffer = KG_MemoryCreateBuffer((unsigned)uFileSize);
    KGLOG_PROCESS_ERROR(piBuffer);

    pbyBuffer = (BYTE*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pbyBuffer);

    uReadBytes = piFile->Read(pbyBuffer, (unsigned long)uFileSize);
    KGLOG_PROCESS_ERROR(uReadBytes == uFileSize);

    KG_COM_RELEASE(piFile);

    pbyOffset  = pbyBuffer;
    uLeftBytes = uReadBytes;

    KGLOG_PROCESS_ERROR(uLeftBytes >= sizeof(KSCENE_TERRAIN_DATA_HEADER));
    pFileHeader = (KSCENE_TERRAIN_DATA_HEADER*)pbyOffset;

    pbyOffset  += sizeof(KSCENE_TERRAIN_DATA_HEADER);
    uLeftBytes -= sizeof(KSCENE_TERRAIN_DATA_HEADER);

    KGLOG_PROCESS_ERROR(pFileHeader->nVersion == CURRENT_TERRAIN_DATA_VERSION);
    KGLOG_PROCESS_ERROR(pFileHeader->nXCellCount == m_nXGridCount);
    KGLOG_PROCESS_ERROR(pFileHeader->nYCellCount == m_nYGridCount);
    KGLOG_PROCESS_ERROR(m_nXGridCount <= SCENE_GRID_WIDTH);
    KGLOG_PROCESS_ERROR(m_nYGridCount <= SCENE_GRID_HEIGHT);

    for (int nCellY = 0; nCellY < SCENE_GRID_HEIGHT; nCellY++)
    {
        for (int nCellX = 0; nCellX < SCENE_GRID_WIDTH; nCellX++)
        {
            KCell* pCell = m_pCells + nCellY * SCENE_GRID_WIDTH + nCellX;

            if (nCellX < m_nXGridCount && nCellY < m_nYGridCount)
            {
                KCell* pCellData = (KCell*)pbyOffset;

                *pCell = *pCellData;
                pbyOffset   += sizeof(KCell);
                uLeftBytes  -= sizeof(KCell);

                continue;
            }

            pCell->byObstacleType = Obstacle_Normal;            
        }
    }

    KGLOG_PROCESS_ERROR(uLeftBytes == 0);

Exit1:
    bResult = true;
Exit0:
    KG_COM_RELEASE(piBuffer);
    KG_COM_RELEASE(piFile);
    return bResult;
}

BOOL KScene::CallInitScript()
{
    BOOL bRetCode = false;

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(m_dwScriptID);
    if (bRetCode && g_pSO3World->m_ScriptManager.IsFuncExist(m_dwScriptID, SCRIPT_ON_INIT_SCENE))
    {
        int nTopIndex = 0;

        g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
        g_pSO3World->m_ScriptManager.Push(this);
        g_pSO3World->m_ScriptManager.Call(m_dwScriptID, SCRIPT_ON_INIT_SCENE, 0);

        g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);
    }

    return true;
}

BOOL KScene::SaveBaseInfo()
{
    BOOL        bResult                 = false;
    BOOL        bRetCode                = false;
    IIniFile*   piFile                  = NULL;
    char        szFileName[MAX_PATH];
    KMapParams* pMapInfo = NULL;

    pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapInfo);

    snprintf(szFileName, sizeof(szFileName), "%s/%u/baseinfo.ini", MAP_DIR, pMapInfo->nServerResourceID);
    szFileName[sizeof(szFileName) - 1] = '\0';

    piFile = g_CreateIniFile();
    KGLOG_PROCESS_ERROR(piFile);

    bRetCode = piFile->Save(szFileName);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piFile);
    return bResult;
}

BOOL KScene::LoadBaseInfo()
{
    BOOL        bResult                 = false;
    int         nRetCode                = 0;
    IIniFile*   piIniFile               = NULL;
    char        szFileName[MAX_PATH];
    KPOSITION   cPos;
    KMapParams* pMapInfo = NULL;

    pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapInfo);

    snprintf(szFileName, sizeof(szFileName), "%s/%u/baseinfo.ini", MAP_DIR, pMapInfo->nServerResourceID);
    szFileName[sizeof(szFileName) - 1] = '\0';

    piIniFile = g_OpenIniFile(szFileName);
    KGLOG_PROCESS_ERROR(piIniFile);

    nRetCode = piIniFile->GetInteger("Main", "XCellCount", 61, &m_nXGridCount);
    KGLOG_PROCESS_ERROR(nRetCode > 0);

    nRetCode = piIniFile->GetInteger("Main", "YCellCount", 12, &m_nYGridCount);
    KGLOG_PROCESS_ERROR(nRetCode > 0);

    nRetCode = LoadAllDefaultObj();
    KGLOG_PROCESS_ERROR(nRetCode);

    m_pMapParam = pMapInfo;

    bResult = true;
Exit0:
    KG_COM_RELEASE(piIniFile);
    return bResult;
}

BOOL KScene::CompleteLoading()
{
    BOOL bRetCode = false;
    
    switch(m_Param.m_eBattleType)
    {
    case KBATTLE_TYPE_LADDER_PVP:
        {
            bRetCode = LoadAIHeroInLadder();
            KGLOG_CHECK_ERROR(bRetCode);
        }
        break;
    case KBATTLE_TYPE_MISSION:
        {
        }
        break;
    default:
        break;
    }
     
    return true;
}

KCell* KScene::GetCell(int nX, int nY)
{
    int nXCell = nX / CELL_LENGTH;
    int nYCell = nY / CELL_LENGTH;

    assert(nXCell >= 0);
    assert(nXCell < SCENE_GRID_WIDTH);
    assert(nYCell >= 0);
    assert(nYCell < SCENE_GRID_HEIGHT);

    return m_pCells + SCENE_GRID_WIDTH * nYCell + nXCell;
}
// 取地表高度还有优化空间
int  KScene::GetPosHeight(int nX, int nY)
{
    int nHeight = 0;
    KCell* pCell = NULL;
    KDYNAMIC_OBSTACLE_MAP::iterator it;
    KSET_ID::iterator setIt;
    KSceneObject* pObject = NULL;

    int nXCell = nX / CELL_LENGTH;
    int nYCell = nY / CELL_LENGTH;

    assert(nXCell >= 0);
    assert(nXCell < SCENE_GRID_WIDTH);
    assert(nYCell >= 0);
    assert(nYCell < SCENE_GRID_HEIGHT);

    pCell = m_pCells + SCENE_GRID_WIDTH * nYCell + nXCell;
    assert(pCell);

    nHeight = pCell->wHeight;

    it = m_mapDynamicObstacle.find(MAKE_INT64(nXCell, nYCell));
    KG_PROCESS_ERROR(it != m_mapDynamicObstacle.end());

    for (setIt = it->second.begin(); setIt != it->second.end(); ++setIt)
    {
        pObject = GetSceneObjByID(*setIt);
        if (pObject == NULL)
            continue;

        if (nHeight < pObject->m_nZ + pObject->m_nHeight)
            nHeight = pObject->m_nZ + pObject->m_nHeight;
    }
    
Exit0:
    return nHeight;
}

KMovableObstacle* KScene::GetTopObstacle(int nX, int nY)
{
    KMovableObstacle* pResult = NULL;
    int nHeight = 0;
    KCell* pCell = NULL;
    KDYNAMIC_OBSTACLE_MAP::iterator it;
    KSET_ID::iterator setIt;
    KSceneObject* pObject = NULL;

    int nXCell = nX / CELL_LENGTH;
    int nYCell = nY / CELL_LENGTH;

    assert(nXCell >= 0);
    assert(nXCell < SCENE_GRID_WIDTH);
    assert(nYCell >= 0);
    assert(nYCell < SCENE_GRID_HEIGHT);

    pCell = m_pCells + SCENE_GRID_WIDTH * nYCell + nXCell;
    assert(pCell);

    nHeight = pCell->wHeight;

    it = m_mapDynamicObstacle.find(MAKE_INT64(nXCell, nYCell));
    KG_PROCESS_ERROR(it != m_mapDynamicObstacle.end());

    for (setIt = it->second.begin(); setIt != it->second.end(); ++setIt)
    {
        pObject = GetSceneObjByID(*setIt);
        if (pObject == NULL)
            continue;

        assert(g_IsObstacle(pObject));

        if (nHeight < pObject->m_nZ + pObject->m_nHeight)
        {
            nHeight = pObject->m_nZ + pObject->m_nHeight;
            pResult = (KMovableObstacle*)pObject;
        }
        else if (pResult == NULL)
        {
            pResult = (KMovableObstacle*)pObject;
        }
    }

Exit0:
    return pResult;
}

BOOL KScene::AddHeroForPlayer(KPlayer* pPlayer)
{
	BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    int             nTemplateID     = 0;
    KHero*          pFightingHero   = NULL;
    KHero*          pAITeammate     = NULL;
    int             nLeftFrame      = 0;
    KVIPConfig*     pVipConfig      = NULL;

    std::vector<KHERO_INFO>     vecHeroInfo;

    KGLOG_PROCESS_ERROR(pPlayer);   

    KGLOG_PROCESS_ERROR(m_eSceneState == ssWaitingClientLoading);

    // 自动修理装备
    if (pPlayer->IsVIP())
    {
        pVipConfig = g_pSO3World->m_Settings.m_VIPConfig.GetByID(pPlayer->m_nVIPLevel);
        if (pVipConfig && pVipConfig->bAutoRepair && pPlayer->m_bVIPAutoRepair)
        {
            bRetCode = g_pSO3World->m_ShopMgr.OnPlayerRepairAllEquip(pPlayer);
            if (!bRetCode)
            {
                pPlayer->SetVIPAutoRepairFlag(false);
                pPlayer->m_Secretary.AddReport(KREPORT_EVENT_AUTOREPAIR_FAILED, NULL, 0);
            }
        }
    }
    
    GetPlayerHeroInfo(pPlayer, vecHeroInfo);
    KGLOG_PROCESS_ERROR(!vecHeroInfo.empty());

    for (size_t i = 0; i < vecHeroInfo.size(); ++i)
    {
        if (vecHeroInfo[i].bAiMode)
        {
            KGLOG_PROCESS_ERROR(pAITeammate == NULL);
            pAITeammate = AddPlayerHero(pPlayer, vecHeroInfo[i]);
            KGLOG_PROCESS_ERROR(pAITeammate);
        }
        else
        {
            KGLOG_PROCESS_ERROR(pFightingHero == NULL);
            pFightingHero = AddPlayerHero(pPlayer, vecHeroInfo[i]);
            KGLOG_PROCESS_ERROR(pFightingHero);
        }
    }

    KGLOG_PROCESS_ERROR(pFightingHero);
    
    if (m_Param.m_eBattleType == KBATTLE_TYPE_MISSION)
    {
        KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
        if (pMission && pMission->nCostFatiguePoint > 0)
        {
            bRetCode = pPlayer->DoFatiguePointCost(pMission->nCostFatiguePoint);
            KGLOG_PROCESS_ERROR(bRetCode);

            PLAYER_LOG(pPlayer, "fatigue,consume,mission,%u", pMission->nCostFatiguePoint);
        }
    }
    else if (m_Param.m_eBattleType == KBATTLE_TYPE_LADDER_PVP)
    {
        bRetCode = pPlayer->DoLadderPVPFatigueCost();
        m_LadderPVPFatigueCosted[pFightingHero->m_nSide][pFightingHero->m_nPos] = bRetCode;
    }

    // 同步其它英雄给我，不包括自己的主英雄和AI队友
    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();

        if (pObj->GetType() != sotHero)
            continue;

        KHero* pOtherHero = (KHero*)pObj;
        KPlayer* pOwner = pOtherHero->GetOwner();
        if (pOwner && pOwner == pPlayer)
            continue;

        if (pOtherHero->m_pNpcTemplate)
            g_PlayerServer.DoSyncNewNpc(pOtherHero, pPlayer->m_nConnIndex);     
        else
            g_PlayerServer.DoSyncNewHero(pOtherHero, pPlayer->m_nConnIndex);
    }

    if (m_Param.m_eBattleType == KBATTLE_TYPE_MISSION)
    {
        m_vecPlayerAILevelCoe.push_back(pPlayer->m_nAILevelCoe);
    }
    
    // 脚本中可能会添加hero
    if (pFightingHero)
    {
        CallEnterSceneScript(pFightingHero);
        CallGlobalEnterSceneScript(pFightingHero);

        // 把自己的英雄广播出去,主英雄不同步给自己,在DoSyncPlayerHeroInfo同步
        g_PlayerServer.DoBroadcastNewHero(pFightingHero, pFightingHero->m_dwID);
    }
    
    if (pAITeammate)
    {
        CallEnterSceneScript(pAITeammate);
        CallGlobalEnterSceneScript(pAITeammate);

        // 把自己的英雄广播出去,包括自己
        g_PlayerServer.DoBroadcastNewHero(pAITeammate, ERROR_ID);
    }  

    // 同步主英雄
    g_PlayerServer.DoSyncPlayerHeroInfo(pPlayer); // 这个协议作用是同步玩家控制的英雄
    g_PlayerServer.DoUpdateMoveParam(pFightingHero);

    SyncCheerleadingsOnPlayerEnterScene(pPlayer, pFightingHero->m_nSide, pFightingHero->m_nPos);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KScene::RemoveHero(KHero* pHero)
{
	BOOL bResult    = false;
    KPlayer* pPlayer = NULL;

    assert(pHero);
    
    CallGlobalLeaveSceneScript(pHero);
	g_PlayerServer.DoBroadcastHeroRemove(this, pHero->m_dwID, false);

    pPlayer = pHero->GetOwner();
    if (pPlayer)
    {
        KHeroData* pHeroData = NULL;

        pPlayer->UnApplyPlayerBuff(pHero);
        pPlayer->UnApplyHeroEquip(pHero);
        pPlayer->UnApplyHeroTalent(pHero);
        pPlayer->UnApplyHeroFashionAttr(pHero);

        pHeroData = pPlayer->m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
        KGLOG_PROCESS_ERROR(pHeroData);
        pHeroData->OnLeaveScene();

        pPlayer->UpdateHeroData(pHero);
    }

    m_pSceneObjMgr->Delete(pHero->m_dwID);
    pHero = NULL;

    if (!IsAnyPlayerExist())
        m_eSceneState = ssWatingDelete;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KScene::IsAnyPlayerExist()
{
    for(KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        
        if (pObj->GetType() == sotHero)
        {
            KHero* pHero = (KHero*)pObj;
            if (pHero->GetOwner())
                return true;
        }
    }

    return false;
}

void KScene::CallEnterSceneScript(KHero* pHero)
{
    BOOL bRetCode = false;
    int  nTopIndex = 0;
    const char* pszFuncName = "OnHeroEnterScene";

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(m_dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(this);
    g_pSO3World->m_ScriptManager.Push(pHero);
    g_pSO3World->m_ScriptManager.Call(m_dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KScene::CallGlobalEnterSceneScript(KHero* pHero)
{
    BOOL bRetCode = false;
    int  nTopIndex = 0;
    const char* pszFuncName = "OnHeroEnterScene";
    DWORD dwScriptID = g_FileNameHash(SCRIPT_DIR"/global/scene.lua");

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(this);
    g_pSO3World->m_ScriptManager.Push(pHero);
    g_pSO3World->m_ScriptManager.Call(dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KScene::CallGlobalLeaveSceneScript(KHero* pHero)
{
    BOOL bRetCode = false;
    int  nTopIndex = 0;
    const char* pszFuncName = "OnHeroLeaveScene";
    DWORD dwScriptID = g_FileNameHash(SCRIPT_DIR"/global/scene.lua");

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(this);
    g_pSO3World->m_ScriptManager.Push(pHero);
    g_pSO3World->m_ScriptManager.Call(dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KScene::CallPVEMapAddScoreScript(KHero* pHero, int nOldScore, int nNowScore)
{
    BOOL bRetCode = false;
    int  nTopIndex = 0;
    const char* pszFuncName = "OnPVEMapAddScore";
    DWORD dwScriptID = g_FileNameHash(SCRIPT_DIR"/global/scene.lua");

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(this);
    g_pSO3World->m_ScriptManager.Push(pHero);
    g_pSO3World->m_ScriptManager.Push(nOldScore);
    g_pSO3World->m_ScriptManager.Push(nNowScore);
    g_pSO3World->m_ScriptManager.Call(dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KScene::CallOnPlayersInMissionReadyScript()
{
    BOOL bRetCode = false;
    int  nTopIndex = 0;
    const char* pszFuncName = "OnPlayersInMissionReady";
    KHero* pHost = NULL;
    KHero* pTeammate = NULL;

    KG_PROCESS_ERROR(m_Param.m_eBattleType == KBATTLE_TYPE_MISSION);
    
    pHost = GetHeroAt(sidLeft, 0);
    //KGLOG_PROCESS_ERROR(pHost);

    pTeammate = GetHeroAt(sidLeft, 1);
    //KG_PROCESS_ERROR(pTeammate);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(m_dwScriptID);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(m_dwScriptID, pszFuncName);
    KG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(this);
    g_pSO3World->m_ScriptManager.Push(pHost);
    g_pSO3World->m_ScriptManager.Push(pTeammate);
    g_pSO3World->m_ScriptManager.Call(m_dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

BOOL g_IsNotCollide(
    int nX1, int nY1, int nZ1, int nLength1, int nWidth1, int nHeight1,
    int nX2, int nY2, int nZ2, int nLength2, int nWidth2, int nHeight2
)
{
    int nOffsetX = abs(nX1 - nX2);
    int nOffsetY = abs(nY1 - nY2);
    int nOffsetZ = abs(nZ1 - nZ2);
    if (nOffsetX > (nLength1 + nLength2) / 2)
        return true;
    if (nOffsetY > (nWidth1 + nWidth2) / 2)
        return true;

    if (nZ1 > nZ2)
    {
        if (nZ1 - nZ2 > nHeight2)
            return true;
    }
    else
    {
        if (nZ2 - nZ1 > nHeight1)
            return true;
    }
    
    return false;
}

BOOL g_IsCollide(
    int nX1, int nY1, int nZ1, int nLength1, int nWidth1, int nHeight1,
    int nX2, int nY2, int nZ2, int nLength2, int nWidth2, int nHeight2
)
{
    return !g_IsNotCollide(
        nX1, nY1, nZ1, nLength1, nWidth1, nHeight1,
        nX2, nY2, nZ2, nLength2, nWidth2, nHeight2
    );
}

BOOL g_IsNotCollide(const KBODY& cBody1, const KBODY& cBody2)
{
    return g_IsNotCollide(
        cBody1.nX, cBody1.nY, cBody1.nZ, cBody1.nLength, cBody1.nWidth, cBody1.nHeight,
        cBody2.nX, cBody2.nY, cBody2.nZ, cBody2.nLength, cBody2.nWidth, cBody2.nHeight
        );
}

BOOL g_IsCollide(const KBODY& cBody1, const KBODY& cBody2)
{
    return !g_IsNotCollide(cBody1, cBody2);
}


BOOL g_IsNotCollide(const KSceneObject* pObj1, const KSceneObject* pObj2)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KBODY cBody1 = pObj1->GetBody();
    KBODY cBody2 = pObj2->GetBody();

    assert(pObj1);
    assert(pObj2);
    
    bRetCode = g_IsNotCollide(cBody1, cBody2);
    KG_PROCESS_ERROR(bRetCode);
    
    bResult = true;
Exit0:
    return bResult;
}

BOOL g_IsCollide(const KSceneObject* pObj1, const KSceneObject* pObj2)
{
    return !g_IsNotCollide(pObj1, pObj2);
}

struct KCheckCollideTraverse
{
    BOOL operator()(KSceneObject* pObj);
    KVEC_OBJ vecObj;
    KBODY cDstBody;
};

BOOL KCheckCollideTraverse::operator()(KSceneObject* pObj)
{
    assert(pObj);
    
    BOOL bRetCode = false;
    KBODY cBody = pObj->GetBody();

    if (!pObj->m_bCanBeAttack)
        return true;

    bRetCode = g_IsCollide(cDstBody, cBody);
    if (bRetCode)
        vecObj.push_back(pObj);

    return true;
}


void KScene::GetBeAttackedObj(const KBODY& rBody, KVEC_OBJ& vecRetObj)
{
    KCheckCollideTraverse Func;

    Func.cDstBody = rBody;

    TraverseSceneObj(Func);

    std::copy(Func.vecObj.begin(), Func.vecObj.end(), back_inserter(vecRetObj));
}

KBasketSocket* KScene::GetShootBallBasketSocket(KTWO_DIRECTION eShooterFaceDir, int nAimLoop)
{
    KTWO_DIRECTION      eDirFilter  = (eShooterFaceDir == csdRight ? csdLeft : csdRight);
    KBasketSocketFloor  eAimFloor   = GetAimFloor(nAimLoop);   
    KBasketSocket*      pResult     = NULL;
        
    pResult = GetBasketSocket(eDirFilter, (KBasketSocketFloor)(eAimFloor + 1));
    KGLOG_PROCESS_ERROR(pResult);

    if (pResult->IsEmpty())
    {
        pResult = GetBasketSocket(eDirFilter, eAimFloor);
    }
    
Exit0:
    return pResult;
}

KBasketSocket* KScene::GetSlamDunkBasketSokcet(KHero* pHero)
{
    KBasketSocket*      pResult         = NULL;
    BOOL                bRetCode        = false;
    KBODY               cHeroBody;
    
    KGLOG_PROCESS_ERROR(pHero);

    cHeroBody = pHero->GetBody();

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotBasketSocket)
            continue;
        
        KBasketSocket* pSocket = (KBasketSocket*)pObj;
        if (pSocket->IsEmpty())
            continue;
        
        KBODY cSocketBody = pSocket->GetBody();
        int nOffsetX = cSocketBody.nX - cHeroBody.nX;

        if (pHero->m_eFaceDir == csdLeft && nOffsetX >= 0)
            continue;
        
        if(pHero->m_eFaceDir == csdRight && nOffsetX <= 0)
            continue;

        nOffsetX = abs(nOffsetX);
        int nOffsetY = abs(cSocketBody.nY - cHeroBody.nY);
        int nOffsetZ = cSocketBody.nZ - cHeroBody.nZ;


        if (nOffsetX >= 0 && 
            nOffsetX <= (cSocketBody.nLength + cHeroBody.nLength) / 2 &&
            nOffsetY <= cSocketBody.nWidth &&
            nOffsetZ >= cHeroBody.nHeight / 2 &&
            nOffsetZ <= cHeroBody.nHeight &&
            pSocket->m_eDir != pHero->m_eFaceDir)
        {
            if (pResult == NULL || pSocket->m_eFloor <= pResult->m_eFloor)
                pResult = pSocket;
        } 
    }
Exit0:
    return pResult;
}

KBasketSocket* KScene::GetShootBasketBasketSocket(KTWO_DIRECTION eShooterDir, int nAimLoop)
{
    KTWO_DIRECTION          eDirFilter  = eShooterDir == csdRight ? csdLeft : csdRight;
    KBasketSocketFloor      eAimFloor   = GetAimFloor(nAimLoop);
    KBasketSocket*          pResult     = NULL;

    pResult = GetBasketSocket(eDirFilter, eAimFloor);
    KGLOG_PROCESS_ERROR(pResult);

    if (!pResult->IsEmpty())
        pResult = GetBasketSocket(eDirFilter, (KBasketSocketFloor)(eAimFloor + 1));

Exit0:
    return pResult;
}

KDoodad* KScene::GetDoodadById(DWORD dwID) const
{
    KDoodad* pResult = NULL;
    KSceneObject* pObj = NULL;
    
    pObj = m_pSceneObjMgr->Get(dwID);
    KGLOG_PROCESS_ERROR(pObj);

    if (g_IsDoodad(pObj))
        pResult = (KDoodad*)pObj;

Exit0:
    return pResult;
}

KBall* KScene::GetBall() const
{
    KBall* pResult = NULL;
    for(KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        KGLOG_PROCESS_ERROR(pObj);

        if (pObj->GetType() == sotBall)
        {
            pResult = (KBall*)pObj;
            break;
        }
    }

Exit0:
    return pResult;
}

DWORD GetScore(int nBasketFloor)
{
    DWORD dwResult = 3;
    if (nBasketFloor == bfFifth || nBasketFloor == bfSixth)
        dwResult = 4;
    
    return dwResult;
}

void KScene::OnShootBallSuccess(KBall* pBall, KBasketSocket* pSocket)
{
    KBasket*    pBasket     = NULL;
    int         nSide       = 0;
    int         nOtherSide  = 0;
    DWORD       dwScore     = 0;
    int         nBigScoreSide = sidLeft;
    BOOL        bWinTenScoreAtThirtySeconds = false;
    int         nDeltScore  = 0;
    int         nBattleTime = 0;
    KHero*      pShooter    = NULL;
    BOOL        bPlayerScore = false;
    int         nOldScore = 0;
    int         nNowScore = 0;
    KMapParams* pMapInfo  = NULL;
    int         nOldScoreLead = 0;
    int         nNowScoreLead = 0;

    KGLOG_PROCESS_ERROR(pBall);
    KGLOG_PROCESS_ERROR(pSocket);

    pBasket = pSocket->GetBasket();
    KGLOG_PROCESS_ERROR(pBasket);
	
	pBasket->CostHP(pBall->m_dwShooterID, pBall);

    dwScore = GetScore(pSocket->m_eFloor);
    
    nSide = pSocket->m_nSide;
    assert(nSide == sidLeft || nSide == sidRight);

    nOtherSide = sidRight;
    if (nSide == sidRight)
        nOtherSide = sidLeft;

    nOldScoreLead = m_nScore[nSide] - m_nScore[nOtherSide];
    nOldScore = m_nScore[nSide];
    m_nScore[nSide] += dwScore;
    nNowScore = m_nScore[nSide];
    nNowScoreLead = m_nScore[nSide] - m_nScore[nOtherSide];

    g_PlayerServer.DoSyncScore(this, nSide, m_nScore[nSide]);

    m_Battle.OnAddScore(nNowScore);

    pShooter = GetHeroById(pBall->m_dwShooterID);
    if (pShooter && pShooter->m_nSide == nSide)
        pShooter->m_GameStat.m_nScore += dwScore;
    
    {
        if (m_nScore[sidLeft] < m_nScore[sidRight])
            nBigScoreSide = sidRight;

        nDeltScore = abs(m_nScore[sidLeft] - m_nScore[sidRight]);

        nBattleTime = m_Battle.GetStartedFrame();
        if (nDeltScore >= 10 && nBattleTime <= 30 * GAME_FPS)
            bWinTenScoreAtThirtySeconds = true;
    }
    
    // 进球后怒气的处理
    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KHero* pHero  =  NULL;
        KSceneObject* pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;

        if (pHero->m_nSide == nSide) // 己方进球了
        {
            int nAddAngry = g_pSO3World->m_Settings.m_ConstList.nSelfHitAddAngry * dwScore;
            pHero->AddAngry(nAddAngry);
            if (!pHero->IsNpc())
                bPlayerScore = true;
        }
        else
        {
            int nAddAngry = g_pSO3World->m_Settings.m_ConstList.nOtherHitAddAngry * dwScore;
            pHero->AddAngry(nAddAngry);
            //被动技能的触发
            pHero->TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_SCORE);
            pHero->TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_LEAD, nOldScoreLead, nNowScoreLead);
        }

        if (bWinTenScoreAtThirtySeconds)
        {
            BroadcastEvent(pSocket->m_nSide, peWin10ScoreAt30Second);
        }
                 
    }
    //全球挑战赛 玩家每得50分掉箱子
    //if (bPlayerScore && m_Param.m_eBattleType == KBATTLE_TYPE_MISSION && m_nScore[nSide] / AWARD_ITEM_SCORE > m_nLastAwardItem)
    
    pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapInfo);

    if (bPlayerScore && pMapInfo->nType == emtPVEMap && pShooter)
    {
        //m_nLastAwardItem = m_nScore[nSide] / AWARD_ITEM_SCORE;
        CallPVEMapAddScoreScript(pShooter, nOldScore, nNowScore);
    }
    
Exit0:
    return;
}

KBasketSocket* KScene::GetShootObjectSocket(KTWO_DIRECTION eDir, int nShootZ)
{
    KBasketSocket* pResult = NULL;

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotBasketSocket) 
            continue;

        KBasketSocket* pSocket = (KBasketSocket*)pObj;
        if (pSocket->m_eDir == eDir)
            continue;

        if (pSocket->GetPosition().nZ < nShootZ)
            continue;
        
        if (!pResult || pSocket->GetPosition().nZ < pResult->GetPosition().nZ)
        {
            pResult = pSocket;
        }
    }
    
    return pResult;
}

BOOL KScene::LoadAllDefaultObj()
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    std::vector<KBasketSocket*> vecAllSocket;
    std::vector<KBasket*> vecAllBasket;
    KSceneObject* pObj = NULL;
    int nTemplateID = ERROR_ID;
    ITabFile* piTabFile = NULL;
    char szMapObjFileName[_NAME_LEN] = "\0";
    KSceneObjInitInfo cObjInitInfo;
    KMapParams* pMapInfo = NULL;

    pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapInfo);

    snprintf(szMapObjFileName, sizeof(szMapObjFileName), "%s/%u/%s", MAP_DIR, pMapInfo->nServerResourceID, SCENE_OBJ_FILE_NAME);
    szMapObjFileName[sizeof(szMapObjFileName) - 1] = '\0';

    piTabFile = g_OpenTabFile(szMapObjFileName);
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); ++nRowIndex)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ObjTemplateID", ERROR_ID, &nTemplateID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "nX", 0, &cObjInitInfo.m_nX);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        MAKE_IN_RANGE(cObjInitInfo.m_nX, 0, GetLength() - 1);

        bRetCode = piTabFile->GetInteger(nRowIndex, "nY", 0, &cObjInitInfo.m_nY);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        MAKE_IN_RANGE(cObjInitInfo.m_nY, 0, GetWidth() - 1);

        bRetCode = piTabFile->GetInteger(nRowIndex, "nZ", 0, &cObjInitInfo.m_nZ);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "eDir", 0, &cObjInitInfo.m_nDir);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(cObjInitInfo.m_nDir == sidRight || cObjInitInfo.m_nDir == sidLeft);

        bRetCode = piTabFile->GetInteger(nRowIndex, "eFloor", 0, &cObjInitInfo.m_nFloor);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(cObjInitInfo.m_nFloor >= bfInvalid && cObjInitInfo.m_nFloor <= bfSixth);

        pObj = m_pSceneObjMgr->AddByTemplate((DWORD)nTemplateID);
        assert(pObj);

        pObj->ApplyInitInfo(cObjInitInfo);
    }

    GetAllBasketSocket(vecAllSocket);
    GetAllBasket(vecAllBasket);

    for (std::vector<KBasketSocket*>::const_iterator constSocketIt = vecAllSocket.begin(); constSocketIt != vecAllSocket.end(); ++constSocketIt)
    {
        KBasketSocket* pSocket = *constSocketIt;
        if(!pSocket->IsEmpty())
            continue;
        KPOSITION cSocketPos = pSocket->GetPosition();
        for (std::vector<KBasket*>::const_iterator constBasketIt = vecAllBasket.begin(); constBasketIt != vecAllBasket.end(); ++ constBasketIt)
        {
            KBasket* pBasket = *constBasketIt;
            if (cSocketPos.IsEqualWith(pBasket->GetPosition()))
            {
                pSocket->AddBasket(pBasket);
                break;
            }
        }      
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}


KSceneObject* KScene::GetSceneObjByID(DWORD dwID) const
{
    return m_pSceneObjMgr->Get(dwID);
}

KHero* KScene::GetHeroById(DWORD dwID) const
{
    KHero* pHero = NULL;
    KSceneObject* pObj = m_pSceneObjMgr->Get(dwID);
    KG_PROCESS_ERROR(pObj && pObj->GetType() == sotHero);

    pHero = (KHero*)pObj;
 Exit0:
    return pHero;
}

// 这里不同步英雄，在addheroforplayer的时候同步
void KScene::SyncAllObjToPlayer(KPlayer* pPlayer)
{
    int nConnIndex = -1;
    assert(pPlayer);

    nConnIndex = pPlayer->m_nConnIndex;
    KGLOG_PROCESS_ERROR(nConnIndex >= 0);

    // 必须先同步所有篮板，再同步所有插槽，最后同步其它场景物件
    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotBackboard)
            continue;

        g_PlayerServer.DoSyncSceneObject((KSceneObject*)pObj, nConnIndex);
    }

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotBasketSocket)
            continue;
        
        g_PlayerServer.DoSyncSceneObject((KSceneObject*)pObj, nConnIndex);
    }

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();

        if (pObj->GetType() == sotBackboard || pObj->GetType() == sotBasketSocket || pObj->GetType() == sotHero)
            continue;

        g_PlayerServer.DoSyncSceneObject((KSceneObject*)pObj, nConnIndex);
    }

Exit0:
    return;
}

void KScene::ObstacleCellChanged(KMovableObstacle* pObstacleObject, const KPOSITION& cOld, const KPOSITION& cNew)
{
    KDYNAMIC_OBSTACLE_MAP::iterator it;
    std::vector<uint64_t> vecCells;

    assert(pObstacleObject);

    pObstacleObject->GetMyAreaCells(cOld, vecCells);

    for (size_t i = 0; i < vecCells.size(); ++i)
    {
        it = m_mapDynamicObstacle.find(vecCells[i]);
        if (it != m_mapDynamicObstacle.end())
            it->second.erase(pObstacleObject->m_dwID);
    }

    vecCells.clear();
    pObstacleObject->GetMyAreaCells(cNew, vecCells);

    for (size_t i = 0; i < vecCells.size(); ++i)
        m_mapDynamicObstacle[vecCells[i]].insert(pObstacleObject->m_dwID);
}

KBasketSocket* KScene::GetBasketSocket(KTWO_DIRECTION eDirFilter, KBasketSocketFloor eFloorFilter)
{
    KBasketSocket* pResult = NULL;
    
    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();

        if (pObj->GetType() != sotBasketSocket)
            continue;

        KBasketSocket* pSocket = (KBasketSocket*)pObj;

        if (pSocket->m_eDir != eDirFilter)
            continue;

        if (pSocket->m_eFloor != eFloorFilter)
            continue;

        pResult = pSocket;
        break;
    }	

    return pResult;
}

KBasketSocketFloor KScene::GetAimFloor(int nAimLoop)
{
    int nAbsoluteFrame = nAimLoop % (GAME_FPS * 2);
    KBasketSocketFloor eAimFloor = bfInvalid;

    if (nAbsoluteFrame > GAME_FPS)
        nAbsoluteFrame = 2 * GAME_FPS - nAbsoluteFrame;

    if (nAbsoluteFrame < GAME_FPS * 0.6f)
        eAimFloor = bfFirst;
    else if (nAbsoluteFrame < GAME_FPS * 0.8f)
        eAimFloor = bfThird;
    else
        eAimFloor = bfFifth;

    return eAimFloor;
}

int KScene::GetLength() const
{
    return m_nXGridCount * CELL_LENGTH;
}

int KScene::GetWidth() const
{
    return m_nYGridCount * CELL_LENGTH;
}

KObjEnumerator KScene::GetObjEnumerator() const
{
    assert(m_pSceneObjMgr);
    return m_pSceneObjMgr->GetEnumerator();
}

KBasket* KScene::GetBasketByID(DWORD dwID) const
{
    KBasket* pResult = NULL;
    KSceneObject* pObj = NULL;
    
    pObj = m_pSceneObjMgr->Get(dwID);
    KGLOG_PROCESS_ERROR(pObj);

    if (pObj->GetType() == sotBasket)
        pResult = (KBasket*)pObj;

Exit0:
    return pResult;
}

void KScene::GetPlayerHeroInfo(KPlayer* pPlayer, std::vector<KHERO_INFO>& vecHeroInfo)
{
	assert(pPlayer);
	
    for (int i = sidBegin; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (m_Param.m_dwMember[i][j] == pPlayer->m_dwID)
            {
                KHERO_INFO HeroInfo;
                HeroInfo.nSide              = i;
                HeroInfo.nPos               = j;
                HeroInfo.dwHeroTemplateID   = m_Param.m_wHeroTemplate[i][j];
                HeroInfo.bAiMode            = m_Param.m_bIsAIMode[i][j];
                
                vecHeroInfo.push_back(HeroInfo);
            }
        }
    }

    KGLOG_PROCESS_ERROR(vecHeroInfo.size() > 0);

    {
        KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
        if (pMission && pMission->eType == pPlayer->GetCurrentChallengeType())
        {
            if (vecHeroInfo.size() == 1) // 需要把辅助队友加入
            {
                int nPlayerSide = vecHeroInfo[0].nSide;
                int nTeammatePos = vecHeroInfo[0].nPos == 0 ? 1 : 0;

                if (m_Param.m_dwMember[nPlayerSide][nTeammatePos] == ERROR_ID)
                {
                    KHERO_INFO HeroInfo;
                    HeroInfo.nSide              = nPlayerSide;
                    HeroInfo.nPos               = nTeammatePos;
                    HeroInfo.dwHeroTemplateID   = pPlayer->m_dwAssistHeroTemplateID;
                    HeroInfo.bAiMode            = true;

                    vecHeroInfo.push_back(HeroInfo);
                }                       
            }
        }
    }

Exit0:
    return;
}

void KScene::FillAllPlayerShotcutBar()
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    std::vector<KPlayer*> vecAllPlayer;
    std::vector<KHero*> vecAllHero;

    GetAllHero(vecAllHero);

    for (std::vector<KHero*>::iterator it = vecAllHero.begin(); it != vecAllHero.end(); ++it)
    {
        KHero*      pHero   = *it;
        KPlayer*    pPlayer = NULL;
        
        assert(pHero);

        pPlayer = pHero->GetOwner();
        if (pPlayer == NULL)
            continue;

        if (std::find(vecAllPlayer.begin(), vecAllPlayer.end(), pPlayer) == vecAllPlayer.end())
            vecAllPlayer.push_back(pPlayer);
    }

    for (std::vector<KPlayer*>::iterator it = vecAllPlayer.begin(); it != vecAllPlayer.end(); ++it)
    {
        KPlayer* pPlayer = *it;
        pPlayer->FillShortcutBar();
    }

    bResult	= true;
Exit0:
    return ;
}

void KScene::RemoveAllPlayer()
{
    std::vector<KPlayer*> vecAllPlayer;
    std::vector<KHero*> vecAllHero;
    std::map<KHero*, KPlayer*> mapHero2Owner;

    GetAllHero(vecAllHero);

    for (std::vector<KHero*>::iterator it = vecAllHero.begin(); it != vecAllHero.end(); ++it)
    {
        KHero* pHero = *it;
        KPlayer* pPlayer = pHero->GetOwner();
        if (pPlayer == NULL)
            continue;

        pHero->Detach();

        mapHero2Owner[pHero] = pPlayer;

        if (std::find(vecAllPlayer.begin(), vecAllPlayer.end(), pPlayer) == vecAllPlayer.end())
            vecAllPlayer.push_back(pPlayer);
    }

    for (std::vector<KHero*>::iterator it = vecAllHero.begin(); it != vecAllHero.end(); ++it)
    {
        KHero* pHero = *it;

        std::map<KHero*, KPlayer*>::iterator itHero2Owner = mapHero2Owner.find(pHero);
        if (itHero2Owner == mapHero2Owner.end())
            continue;
        
        KHeroData* pHeroData = NULL;
        KPlayer* pPlayer = itHero2Owner->second;

        pPlayer->UnApplyPlayerBuff(pHero);
        pPlayer->UnApplyHeroEquip(pHero);
        pPlayer->UnApplyHeroTalent(pHero);
        pPlayer->UnApplyHeroFashionAttr(pHero);

        pHeroData = pPlayer->m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
        KGLOG_PROCESS_ERROR(pHeroData);
        pHeroData->OnLeaveScene();

        pPlayer->UpdateHeroData(pHero);

    }

    if (!IsAnyPlayerExist())
        m_eSceneState = ssWatingDelete;

    for (std::vector<KPlayer*>::iterator it = vecAllPlayer.begin(); it != vecAllPlayer.end(); ++it)
    {
        KPlayer* pPlayer = *it;
        pPlayer->ReturnToHall();
    }

Exit0:
    return;
}

void KScene::OnBattleFinished()
{
    BOOL    bTrainingScene  = false;
    int     nWinnerSide     = ERROR_ID;

    nWinnerSide = GetWinnerSide();
    g_RelayClient.DoBattleFinishedNotify(m_dwMapID, m_nCopyIndex, nWinnerSide);

    CallPlayerEvent();

    bTrainingScene = g_pSO3World->m_MissionMgr.IsTrainingMission(m_dwMapID);
    if (!bTrainingScene)
        ProcessEquipAbrade();

    FillAllPlayerShotcutBar();
    ProcessGameOver();
    RemoveAllPlayer();
    
    m_eSceneState = ssWatingDelete;
}

int KScene::GetWinnerSide()
{
    DWORD dwResult = cdNoTeamWinID;
    if (m_nScore[sidLeft] > m_nScore[sidRight])
    {
        dwResult = sidLeft;
    }
    else if (m_nScore[sidRight] > m_nScore[sidLeft])
    {
        dwResult = sidRight;
    }
    
    return dwResult;
}

int KScene::GetDeltScore()
{
    return abs(m_nScore[sidLeft] - m_nScore[sidRight]);
}

int KScene::GetEnemyScore(int nSelfSide)
{
    if (nSelfSide == sidLeft)
        return m_nScore[sidRight];
    else if (nSelfSide == sidRight)
        return m_nScore[sidLeft];
    else
        return -1;
}

void KScene::ProcessGameOver()
{
    ProcessGameStat();

    switch (m_Param.m_eBattleType)
    {
    case KBATTLE_TYPE_FREE_PVP:
        ProcessGameOverOnFreePVP();
        break;
    case KBATTLE_TYPE_LADDER_PVP:
        ProcessGameOverOnLadderPVP();
        break;
    case KBATTLE_TYPE_MISSION:
        ProcessGameOverOnMission();
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Unknown Battle Type %d.\n", m_Param.m_eBattleType);
        break;
    }
}

void KScene::ProcessGameStat()
{
    KMAP_SCENE_PLAYERS mapAllPlayers;
    int nWinnerSide = GetWinnerSide();
    int nDeltScore  = GetDeltScore();
    int nEnemySocre = GetEnemyScore(nWinnerSide);
    KMission*  pMission = NULL;

    GetGrouppedHeroWithOwner(mapAllPlayers);

    for (KMAP_SCENE_PLAYERS::iterator it = mapAllPlayers.begin(); it != mapAllPlayers.end(); ++it)
    {
        KPlayer* pPlayer = it->first;
        std::vector<KHero*>& rvecHeros = it->second;
        KHero* pMainHero = NULL;

        if (rvecHeros.empty())
            continue;

        for (size_t i = 0; i < rvecHeros.size(); ++i)
        {
            if (rvecHeros[i]->IsMainHero())
                pMainHero = rvecHeros[i];
        }

        assert(pMainHero);

        if (pMainHero->m_nSide == nWinnerSide) //win
        {
            if (pMainHero->m_GameStat.m_nUseItemCount == 0)
                pPlayer->OnEvent(peNotUseItemAtGame);
            
            if (pMainHero->m_GameStat.m_nDieTimes == 0)
                pPlayer->OnEvent(peNotDieAndWin);
            
            if (nDeltScore >= 40)
                pPlayer->OnEvent(peWin40Score);
            
            if (nEnemySocre == 0)
                pPlayer->OnEvent(peEnemy0Score);
        }

        pPlayer->m_nLastBattleType = pMainHero->m_pScene->m_Param.m_eBattleType;
        pPlayer->m_nLastMissionLevel = 0;
        pMission = g_pSO3World->m_MissionMgr.GetMission(pMainHero->m_pScene->m_dwMapID);
        if (pMission)
        {
            memcpy(pPlayer->m_szLastMissionName, pMission->szName, sizeof(pPlayer->m_szLastMissionName));
            pPlayer->m_nLastMissionLevel = pMission->nLevel;
        }
        
    }
}

void KScene::ProcessEquipAbrade()
{
    BOOL bRetCode   = false;
    std::vector<KHero*> vecAllHero;
    GetAllHero(vecAllHero);

    for (std::vector<KHero*>::iterator it = vecAllHero.begin(); it != vecAllHero.end(); ++it)
    {
        KHero*      pHero   = *it;
        KPlayer*    pPlayer = pHero->GetOwner();

        if (pPlayer == NULL)
            continue;

        bRetCode = pPlayer->AbradeEquip(pHero);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
	
Exit0:
    return;
}

void KScene::ProcessFlowRecord()
{
    BOOL                bTeam       = false;
    KPlayer*            pOwner      = NULL;
    KHero*              pHero       = NULL;
    KSceneObject*       pObj        = NULL;
    KHero*              allHero[sidTotal][MAX_TEAM_MEMBER_COUNT]    = {{NULL,}};
    KPlayer*            allPlayer[sidTotal][MAX_TEAM_MEMBER_COUNT]  = {{NULL,}};
    KMission*           pMission    = NULL;

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;
        KGLOG_PROCESS_ERROR(pHero->m_nSide == sidLeft || pHero->m_nSide == sidRight);
        KGLOG_PROCESS_ERROR(pHero->m_nPos >= 0 && pHero->m_nPos < MAX_TEAM_MEMBER_COUNT);

        pHero->ApplyPassiveSkill();

        pOwner = pHero->GetOwner();

        allHero[pHero->m_nSide][pHero->m_nPos] = pHero;
        allPlayer[pHero->m_nSide][pHero->m_nPos] = pOwner;
    }


    switch (m_nType)
    {
    case emtPVEMap:
        if (allPlayer[sidLeft][0])
        {
            PLAYER_LOG(
                allPlayer[sidLeft][0], 
                "pve,globalchallenge,%d,%d,%d,%d", 
                allPlayer[sidLeft][1] ? allPlayer[sidLeft][1]->m_dwID : 0,
                pMission ? pMission->eType : 0,
                pMission ? pMission->nStep : 0,
                pMission ? pMission->nLevel: 0
            );
        }
        else if (allPlayer[sidLeft][1])
        {
            PLAYER_LOG(
                allPlayer[sidLeft][1],
                "pve,globalchallenge,%d,%d,%d,%d",
                allPlayer[sidLeft][0] ? allPlayer[sidLeft][0]->m_dwID : 0,
                pMission ? pMission->eType : 0,
                pMission ? pMission->nStep : 0,
                pMission ? pMission->nLevel: 0
            );
        }   
        break;
    case emtContinuousChallengeMap:
        pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
        KGLOG_PROCESS_ERROR(pMission);
        
        g_LogClient.DoFlowRecord(
            frmtBattle, bfrChallengeStart, 
            "%s,%s,%d,%d,%d", 
            allPlayer[sidLeft][0] ? allPlayer[sidLeft][0]->m_szName : "",
            allPlayer[sidLeft][1] ? allPlayer[sidLeft][1]->m_szName : "", 
            pMission ? pMission->eType : 0,
            pMission ? pMission->nStep : 0,
            pMission ? pMission->nLevel: 0
        );

        if (allPlayer[sidLeft][0])
        {
            PLAYER_LOG(
                allPlayer[sidLeft][0], 
                "pve,continuouschallenge,%d,%d,%d,%d", 
                allPlayer[sidLeft][1] ? allPlayer[sidLeft][1]->m_dwID : 0,
                pMission ? pMission->eType : 0,
                pMission ? pMission->nStep : 0,
                pMission ? pMission->nLevel: 0
           );
        }
        else if (allPlayer[sidLeft][1])
        {
            PLAYER_LOG(
                allPlayer[sidLeft][1],
                "pve,continouschallenge,%d,%d,%d,%d",
                allPlayer[sidLeft][0] ? allPlayer[sidLeft][0]->m_dwID : 0,
                pMission ? pMission->eType : 0,
                pMission ? pMission->nStep : 0,
                pMission ? pMission->nLevel: 0
            );
        }

        break;
    case emtPVPMap:
        g_LogClient.DoFlowRecord(
            frmtBattle, bfrLadderPVPStart, 
            "%s,%s,%s,%s", 
            allHero[sidLeft][0]     ? allHero[sidLeft][0]->m_szName : "", 
            allHero[sidLeft][1]     ? allHero[sidLeft][1]->m_szName : "", 
            allHero[sidRight][0]    ? allHero[sidRight][0]->m_szName : "", 
            allHero[sidRight][1]    ? allHero[sidRight][1]->m_szName : ""
        );

        if (allPlayer[sidLeft][0])
        {
            PLAYER_LOG(
                allPlayer[sidLeft][0],
                "pvp,ladder,%d,%d,%d",
                allPlayer[sidLeft][1]     ? allPlayer[sidLeft][1]->m_dwID : 0,
                allPlayer[sidRight][0]    ? allPlayer[sidRight][0]->m_dwID : 0,
                allPlayer[sidRight][1]    ? allPlayer[sidRight][1]->m_dwID : 0                    
            );
        } 
        else if (allPlayer[sidLeft][1])
        {
            PLAYER_LOG(
                allPlayer[sidLeft][1],
                "pvp,ladder,%d,%d,%d",
                allPlayer[sidLeft][0]     ? allPlayer[sidLeft][0]->m_dwID  : 0,
                allPlayer[sidRight][0]    ? allPlayer[sidRight][0]->m_dwID : 0,
                allPlayer[sidRight][1]    ? allPlayer[sidRight][1]->m_dwID : 0
            );
        }
        else if (allPlayer[sidRight][0])
        {
            PLAYER_LOG(
                allPlayer[sidRight][0],
                "pvp,ladder,%d,%d,%d",
                allPlayer[sidRight][1]    ? allPlayer[sidRight][1]->m_dwID  : 0,
                allPlayer[sidLeft][0]     ? allPlayer[sidLeft][0]->m_dwID   : 0,
                allPlayer[sidLeft][1]     ? allPlayer[sidLeft][1]->m_dwID   : 0
            );
        }
        else if (allPlayer[sidRight][1])
        {
            PLAYER_LOG(
                allPlayer[sidRight][1],
                "pvp,ladder,%d,%d,%d",
                allPlayer[sidRight][0]    ? allPlayer[sidRight][0]->m_dwID  : 0,
                allPlayer[sidLeft][0]     ? allPlayer[sidLeft][0]->m_dwID   : 0,
                allPlayer[sidLeft][1]     ? allPlayer[sidLeft][1]->m_dwID   : 0
            );
        }
        break;
    default:
        break;
    }

Exit0:
    return;
}

void KScene::ProcessGameOverOnFreePVP()
{
    BOOL        bRetCode    = false;
    KHero*      pMember     = NULL;
    KPlayer*    pOwner      = NULL;
    KPlayer*    allPlayer[sidTotal][MAX_TEAM_MEMBER_COUNT] = {{NULL}};
    KHero*      allHero[sidTotal][MAX_TEAM_MEMBER_COUNT] = {{NULL}};
    int         nWinnerSide = GetWinnerSide();

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotHero)
            continue;
        
        pMember = (KHero*)pObj;
        KGLOG_PROCESS_ERROR(pMember->m_nSide == sidLeft || pMember->m_nSide == sidRight);
        KGLOG_PROCESS_ERROR(pMember->m_nPos >= 0 && pMember->m_nPos < MAX_TEAM_MEMBER_COUNT);

        pOwner = pMember->GetOwner();
        KGLOG_PROCESS_ERROR(pOwner);

        allHero[pMember->m_nSide][pMember->m_nPos] = pMember;

        bRetCode = pMember->IsMainHero();
        if (!bRetCode)
            continue;

        allPlayer[pMember->m_nSide][pMember->m_nPos] = pOwner;

        if (pMember->m_nSide == nWinnerSide)
            pOwner->OnEvent(peWinFreePVP);
    }
    
    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            KPlayer* pPlayer = allPlayer[i][j];
            if (!pPlayer)
                continue;
            
            g_PlayerServer.DoFreePVPBattleFinished(pPlayer->m_nConnIndex, i, m_nScore, allHero[i]);

            /*if (i == nWinnerSide && g_pSO3World->m_Settings.m_ConstList.nFreePvpAwardItemTabType > 0)
            {
            IItem* pGift = pPlayer->AddItem(
            g_pSO3World->m_Settings.m_ConstList.nFreePvpAwardItemTabType, 
            g_pSO3World->m_Settings.m_ConstList.nFreePvpAwardItemTabIndex,
            1, 0
            );

            if (pGift)
            {
            g_PlayerServer.DoAwardGift(
            pPlayer->m_nConnIndex, g_pSO3World->m_Settings.m_ConstList.nFreePvpAwardItemTabType, 
            g_pSO3World->m_Settings.m_ConstList.nFreePvpAwardItemTabIndex, 1
            );
            }
            }

            g_RelayClient.SaveRoleData(pPlayer);*/
        }
    }
Exit0:
    return;
}

void KScene::ProcessGameOverOnLadderPVP()
{
    BOOL                bRetCode    = false;
    KPlayer*            pOwner      = NULL;
    KHero*              pHero       = NULL;
    KSceneObject*       pObj        = NULL;
    int                 nWinnerSide = cdNoTeamWinID;
    int                 nMissionLevel = 0;
    KLADDER_AWARD_DATA* pAward      = NULL;
    KPvPAward*          pPvPAward   = NULL;
    int                 nAddedLadderExp     = 0;
    int                 nAddedMenterPoint   = 0;
    int                 nAddedNormalExp     = 0;
    KHero*              allHero[sidTotal][MAX_TEAM_MEMBER_COUNT]    = {{NULL,}};
    KPlayer*            allPlayer[sidTotal][MAX_TEAM_MEMBER_COUNT]  = {{NULL,}};
    KLADDER_AWARD_DATA  allAward[sidTotal][MAX_TEAM_MEMBER_COUNT]   = {{NULL,}};
    BOOL                bHasAwardCard[sidTotal][MAX_TEAM_MEMBER_COUNT][KAWARD_CARD_TYPE_TOTAL];
    int                 nDeltaScore = 0;
    BOOL                bWinGame = false;
    KRANK_EXP_SETTING_ITEM* pRankExpSettingItem = NULL;
    KHeroData*          pAssistHeroData = NULL;

    memset(allAward, 0, sizeof(allAward));
    memset(bHasAwardCard, 0, sizeof(bHasAwardCard));

    nDeltaScore = abs(m_nScore[sidLeft] - m_nScore[sidRight]);

    nWinnerSide = GetWinnerSide();
    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        pObj = it.GetValue();
        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;
        KGLOG_PROCESS_ERROR(pHero->m_nSide == sidLeft || pHero->m_nSide == sidRight);
        KGLOG_PROCESS_ERROR(pHero->m_nPos >= 0 && pHero->m_nPos < MAX_TEAM_MEMBER_COUNT);
        
        bWinGame = (pHero->m_nSide == nWinnerSide);

        pOwner = pHero->GetOwner();
        if (pOwner)
        {             
            if (bWinGame)
                pOwner->m_nLadderLosingStreakCount = 0;
            else            
                ++pOwner->m_nLadderLosingStreakCount;

            if (pHero->IsMainHero())
            {
                if (pHero->m_nSide == nWinnerSide)
                    pOwner->OnEvent(peWinLadderPVP, nDeltaScore);

                pOwner->OnEvent(peFinishLadderPVP);
            }

            bRetCode = ProcessLadderPVPAward(nWinnerSide, pHero, nAddedLadderExp, nAddedMenterPoint);
            KGLOG_PROCESS_ERROR(bRetCode);

            if (m_LadderPVPFatigueCosted[pHero->m_nSide][pHero->m_nPos])
            {
                bRetCode = g_pSO3World->m_Settings.m_RankExpSettings.GetExp(pHero->m_nLevel, m_nScore[pHero->m_nSide], bWinGame, nAddedNormalExp);
                KGLOG_PROCESS_ERROR(bRetCode);

                bRetCode = pOwner->AddHeroExpNoAdditional(pHero->m_dwTemplateID, nAddedNormalExp);
                KGLOG_PROCESS_ERROR(bRetCode);

                allAward[pHero->m_nSide][pHero->m_nPos].m_nHeroExp = nAddedNormalExp;

                if (pOwner->m_dwAssistHeroTemplateID)
                {
                    pAssistHeroData = pOwner->GetAssistHeroData();
                    KGLOG_PROCESS_ERROR(pAssistHeroData);

                    bRetCode = g_pSO3World->m_Settings.m_RankExpSettings.GetExp(pAssistHeroData->m_nLevel, m_nScore[pHero->m_nSide], bWinGame, nAddedNormalExp);
                    KGLOG_PROCESS_ERROR(bRetCode);

                    bRetCode = pOwner->AddHeroExpNoAdditional(pOwner->m_dwAssistHeroTemplateID, nAddedNormalExp);
                    KGLOG_PROCESS_ERROR(bRetCode);
                }
            }
            
            if (bWinGame && m_LadderPVPFatigueCosted[pHero->m_nSide][pHero->m_nPos])
            {
                pPvPAward = g_pSO3World->m_Settings.m_PvPAward.GetByID(pHero->GetNormalLevel());
                KGLOG_PROCESS_ERROR(pPvPAward);   
                pOwner->OnMissionSuccess(&(pPvPAward->stAwardData), bHasAwardCard[pHero->m_nSide][pHero->m_nPos]);
            }
        }
        else
        {
            bRetCode = g_pSO3World->m_Settings.m_RankExpSettings.GetExp(pHero->m_nLevel, m_nScore[pHero->m_nSide], bWinGame, nAddedNormalExp);
            KGLOG_PROCESS_ERROR(bRetCode);

            allAward[pHero->m_nSide][pHero->m_nPos].m_nHeroExp = nAddedNormalExp;

            bRetCode = GetLadderPVPAward(pHero, nAddedLadderExp, nAddedMenterPoint);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        allHero[pHero->m_nSide][pHero->m_nPos] = pHero;
        allPlayer[pHero->m_nSide][pHero->m_nPos] = pOwner;

        pAward = &allAward[pHero->m_nSide][pHero->m_nPos];
        pAward->m_nAwardedLadderExp    = nAddedLadderExp;
        pAward->m_nAwardedMenterPoint  = nAddedMenterPoint;
    }

    for (int i = 0; i < sidTotal; ++i)
    {
        //首次PvP的玩家 提示添加好友
        CheckFirstPvP(allPlayer[i][0], allPlayer[i][1]);
        CheckFirstPvP(allPlayer[i][1], allPlayer[i][0]);
 
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            KPlayer* pPlayer = allPlayer[i][j];
            KHero*   pHero   = allHero[i][j];

            if (pPlayer == NULL)
                continue;

            assert(pHero);
            if (!pHero->IsMainHero())
                continue;

            pPvPAward = g_pSO3World->m_Settings.m_PvPAward.GetByID(pHero->GetNormalLevel());
            KGLOG_PROCESS_ERROR(pPvPAward);   
            g_PlayerServer.DoLadderBattleFinished(pPlayer->m_nConnIndex, i, m_nScore, allAward[i], allHero[i], bHasAwardCard[i][j], pPvPAward->stAwardData.nGoldCostMoney);

            if (i == nWinnerSide && g_pSO3World->m_Settings.m_ConstList.nLadderPvpAwardItemTabType > 0)
            {
                IItem* pGift = pPlayer->AddItem(
                    g_pSO3World->m_Settings.m_ConstList.nLadderPvpAwardItemTabType, 
                    g_pSO3World->m_Settings.m_ConstList.nLadderPvpAwardItemTabIndex,
                    1, 0
                );

                if (pGift)
                {
                    g_PlayerServer.DoAwardGift(
                        pPlayer->m_nConnIndex, g_pSO3World->m_Settings.m_ConstList.nLadderPvpAwardItemTabType, 
                        g_pSO3World->m_Settings.m_ConstList.nLadderPvpAwardItemTabIndex, 1
                    );
                }
            }

           g_RelayClient.SaveRoleData(pPlayer);
        }
    }

    g_LogClient.DoFlowRecord(
        frmtBattle, bfrLadderPVPEnd, 
        "%s,%s,%s,%s,%d,%d", 
        allPlayer[sidLeft][0] ? allPlayer[sidLeft][0]->m_szName : "",
        allPlayer[sidLeft][1] ? allPlayer[sidLeft][1]->m_szName : "",
        allPlayer[sidRight][0] ? allPlayer[sidRight][0]->m_szName : "",
        allPlayer[sidRight][1] ? allPlayer[sidRight][1]->m_szName : "",
        m_nScore[sidLeft], m_nScore[sidRight]
    );
Exit0:
    return;
}

void KScene::ProcessGameOverOnMission()
{
    KMapParams* pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapInfo);

    if (pMapInfo->nType == emtContinuousChallengeMap)
    {
        ProcessGameOverOnChallengeMission();
    }
    else
    {
        ProcessGameOverOnNormalMission();
    }

Exit0:
    return;
}

void KScene::ProcessGameOverOnNormalMission()
{
    BOOL            bRetCode    = false;
    BOOL            bMainHero   = false;
    BOOL            bMissionSuccessed = false;
    int             nWinnerSide = cdNoTeamWinID;
    unsigned        uCoeExp     = 0;
    unsigned        uCoeMoney   = 0;
    int             nAwardedExp = 0;
    int             nAwardexMoney = 0;
    KMission*       pMission    = NULL;    
    KSceneObject*   pObj        = NULL;
    KHero*          pHero       = NULL;
    KPlayer*        pOwner      = NULL;
    KHero*          allHero[sidTotal][MAX_TEAM_MEMBER_COUNT] = {{NULL}};
    KPlayer*        allPlayer[sidTotal][MAX_TEAM_MEMBER_COUNT] = {{NULL}};
    KMISSION_AWARD_DATA allAward[sidTotal][MAX_TEAM_MEMBER_COUNT];
    BOOL bHasAward[sidTotal][MAX_TEAM_MEMBER_COUNT][KAWARD_CARD_TYPE_TOTAL];
    int nMaxMissionNpcAILevel = 0;

    memset(allAward, 0, sizeof(allAward));
    memset(bHasAward, 0, sizeof(bHasAward));

    pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMission);

    nWinnerSide = GetWinnerSide();

    bRetCode = GetAwardParam(uCoeExp, uCoeMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    nMaxMissionNpcAILevel = GetMaxMissionNpcAILevel();

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        pObj = it.GetValue();
        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;
        KGLOG_PROCESS_ERROR(pHero->m_nSide == sidLeft || pHero->m_nSide == sidRight);
        KGLOG_PROCESS_ERROR(pHero->m_nPos >= 0 && pHero->m_nPos < MAX_TEAM_MEMBER_COUNT);

        pOwner = pHero->GetOwner();
        if (!pOwner)
            continue;

        nAwardedExp     = 0;
        nAwardexMoney   = 0;
        
        bMainHero = pHero->IsMainHero();
        bMissionSuccessed = (pHero->m_nSide == nWinnerSide || nWinnerSide == cdNoTeamWinID);

        if (bMainHero)
        {
            if (pMission->bAffectAICoe)
            {
                int nMaxAILevel = 0;
                int nDeltaAICoe = 0;
                int nDeltaScore = abs(m_nScore[sidLeft] - m_nScore[sidRight]);

                bRetCode = g_pSO3World->m_Settings.m_BattleSetting.GetDeltaAICoe(nDeltaScore, bMissionSuccessed, nDeltaAICoe);
                KGLOG_PROCESS_ERROR(bRetCode);

                nMaxAILevel = g_pSO3World->m_AIManager.GetMaxAILevel();
                pOwner->m_nAILevelCoe += nDeltaAICoe;
                if (pOwner->m_nAILevelCoe > nMaxAILevel - 1)
                    pOwner->m_nAILevelCoe = nMaxAILevel - 1;
                else if (pOwner->m_nAILevelCoe < - (nMaxAILevel - 1) )
                    pOwner->m_nAILevelCoe = - (nMaxAILevel - 1);

                if (!bMissionSuccessed && nMaxMissionNpcAILevel > 0 && pOwner->m_nAILevelCoe >= nMaxMissionNpcAILevel)
                {
                    pOwner->m_nAILevelCoe = nMaxMissionNpcAILevel - 1;
                }
            }
        }

        if (bMainHero && bMissionSuccessed && pMission->eType != 0)
            pOwner->OnEvent(peWinPVE, pMission->eType, pMission->nStep, pMission->nLevel); 

        if (bMainHero && pMission->eType != 0)     
            pOwner->OnEvent(peFinishPVE, pMission->eType, pMission->nStep, pMission->nLevel);
        
        if (bMissionSuccessed)
        {
            bRetCode = ProcessMissionAwardExp(pHero, pMission, uCoeExp, nAwardedExp);
            KGLOG_PROCESS_ERROR(bRetCode);
            if (bMainHero)
            {
                nAwardexMoney = 0;         
                if (!pOwner->m_bIsLimited)
                {
                    nAwardexMoney = pMission->nBaseMoney * uCoeMoney / HUNDRED_NUM;
                    bRetCode = pOwner->m_MoneyMgr.AddMoney(emotMoney, nAwardexMoney);
                    KGLOG_CHECK_ERROR(bRetCode);
                }
         
                PLAYER_LOG(pOwner, "money,addmoney,%d,%s,%d-%d,%d,%d", emotMoney, "finishmissionaward", 0, 0, 0, nAwardexMoney);
                PLAYER_LOG(
                    pOwner, "pve_room,finish_mission_hero_info,%d,%d,%d,%d,%d,%d",
                    pMission->eType, pMission->nStep, pMission->nLevel,pOwner->GetMainHeroLevel(),pOwner->GetMainHeroEquipTotalValuePoint(),pOwner->m_nAILevelCoe);

                pOwner->OnMissionSuccess(&(pMission->stAwardData), bHasAward[pHero->m_nSide][pHero->m_nPos]);
            } 
        }

        if (bMainHero && pMission->dwScriptID)
        {
            bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pMission->dwScriptID);
            if (bRetCode && g_pSO3World->m_ScriptManager.IsFuncExist(pMission->dwScriptID, SCRIPT_ON_FINISH_MISSION))
            {
                int nTopIndex = 0;
                g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

                g_pSO3World->m_ScriptManager.Push(pOwner);
                g_pSO3World->m_ScriptManager.Push((int)pMission->eType);
                g_pSO3World->m_ScriptManager.Push(pMission->nStep);
                g_pSO3World->m_ScriptManager.Push(pMission->nLevel);
                g_pSO3World->m_ScriptManager.Push(bMissionSuccessed);

                g_pSO3World->m_ScriptManager.Call(pMission->dwScriptID, SCRIPT_ON_FINISH_MISSION, 0);
                g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);
            }
        }

        allAward[pHero->m_nSide][pHero->m_nPos].m_nHeroExp  = nAwardedExp;
        allAward[pHero->m_nSide][pHero->m_nPos].m_nMoney    = nAwardexMoney;
        allHero[pHero->m_nSide][pHero->m_nPos]              = pHero;
        allPlayer[pHero->m_nSide][pHero->m_nPos]            = (bMainHero ? pOwner : NULL);
        if (pOwner)
        {
            KHeroData*  pHeroData   = NULL;
            pHeroData = pOwner->m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
            allAward[pHero->m_nSide][pHero->m_nPos].m_nExpMultiple = pHeroData->m_nExpMultiple;
        }
    }

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (!allPlayer[i][j])
                continue;

            BOOL bAssistHeroFighting = false;
            for (int k = 0; k < MAX_TEAM_MEMBER_COUNT; ++k)
            {
                KHero* pMember = allHero[i][k];                
                if (!pMember)
                    continue;
                
                if (pMember->GetOwner() != allPlayer[i][j])
                    continue;

                if (!pMember->IsAssistHero())
                    continue;

                bAssistHeroFighting = true;
                break;                            
            }

            if (!bAssistHeroFighting)
            {
                allPlayer[i][j]->AddHeroExpNoAdditional(allPlayer[i][j]->m_dwAssistHeroTemplateID, allAward[i][j].m_nHeroExp);
            }
        }
    }

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (!allPlayer[i][j])
                continue;

            g_PlayerServer.DoMissionBattleFinished(
                allPlayer[i][j]->m_nConnIndex, i, m_nScore, allAward[i], allHero[i], bHasAward[i][j], pMission->stAwardData.nGoldCostMoney
            );
            g_RelayClient.SaveRoleData(allPlayer[i][j]);
        }  
    }

Exit0:
    return;
}

void KScene::ProcessGameOverOnChallengeMission()
{
    BOOL            bRetCode    = false;
    BOOL            bMainHero   = false;
    BOOL            bMissionSuccessed = false;
    int             nWinnerSide = cdNoTeamWinID;
    KMission*       pMission    = NULL;
    KSceneObject*   pObj        = NULL;
    KPlayer*        pOwner      = NULL;
    KHero*          allHero[sidTotal][MAX_TEAM_MEMBER_COUNT] = {{NULL}};
    KPlayer*        allPlayer[sidTotal][MAX_TEAM_MEMBER_COUNT] = {{NULL}};
    BOOL            aUseAssistHero[sidTotal][MAX_TEAM_MEMBER_COUNT];
    KMISSION_AWARD_DATA allAward[sidTotal][MAX_TEAM_MEMBER_COUNT];
    BOOL bHasAward[sidTotal][MAX_TEAM_MEMBER_COUNT][KAWARD_CARD_TYPE_TOTAL];
    DWORD           dwMapID = 0;

    memset(allAward, 0, sizeof(allAward));
	memset(bHasAward, 0, sizeof(bHasAward));
    memset(aUseAssistHero, 0, sizeof(aUseAssistHero));

    pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMission);

    nWinnerSide = GetWinnerSide();

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KHero*          pHero       = NULL;

        pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;
        KGLOG_PROCESS_ERROR(pHero->m_nSide == sidLeft || pHero->m_nSide == sidRight);
        KGLOG_PROCESS_ERROR(pHero->m_nPos >= 0 && pHero->m_nPos < MAX_TEAM_MEMBER_COUNT);

        pOwner = pHero->GetOwner();
        if (!pOwner)
            continue;

        bMainHero = pHero->IsMainHero();
        bMissionSuccessed = (pHero->m_nSide == nWinnerSide || nWinnerSide == cdNoTeamWinID);

        if (bMainHero && pMission->dwScriptID)
        {
            bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pMission->dwScriptID);
            if (bRetCode && g_pSO3World->m_ScriptManager.IsFuncExist(pMission->dwScriptID, SCRIPT_ON_FINISH_MISSION))
            {
                int nTopIndex = 0;
                g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

                g_pSO3World->m_ScriptManager.Push(pOwner);
                g_pSO3World->m_ScriptManager.Push((int)pMission->eType);
                g_pSO3World->m_ScriptManager.Push(pMission->nStep);
                g_pSO3World->m_ScriptManager.Push(pMission->nLevel);
                g_pSO3World->m_ScriptManager.Push(bMissionSuccessed);

                g_pSO3World->m_ScriptManager.Call(pMission->dwScriptID, SCRIPT_ON_FINISH_MISSION, 0);
                g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);
            }
        }

        allHero[pHero->m_nSide][pHero->m_nPos]      = pHero;
        allPlayer[pHero->m_nSide][pHero->m_nPos]    = (bMainHero ? pOwner : NULL);        
    }

    // 检查是否使用了AI队友
    for (int i = 0; i < sidTotal; ++i)
    {
        if (allHero[i][0] && allHero[i][1])
        {
            if (allPlayer[i][0] == NULL)
            {
                assert(allPlayer[i][1]);
                aUseAssistHero[i][1] = true;
            }
            else if (allPlayer[i][1] == NULL)
            {
                assert(allPlayer[i][0]);
                aUseAssistHero[i][0] = true;
            }
        }
    }

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            KPlayer* pPlayer = allPlayer[i][j];
            if (!pPlayer)
                continue;

            if (bMissionSuccessed)
            {
                pPlayer->OnEvent(peWinContinuousChallenge, pMission->eType, pMission->nStep, pMission->nLevel);
                pPlayer->SetLastChallengeStep(pPlayer->GetLastChallengeStep() + 1);

                if (pPlayer->GetLastChallengeStep() > g_pSO3World->m_Settings.m_ConstList.nChallengeMissionCount) // 通关
                {
                    PLAYER_LOG(pPlayer, "challenge,win,%d", pPlayer->GetLastChallengeStep());

                    KPlayer* pOtherPlayer = NULL;
                    if (allPlayer[i][0] && allPlayer[i][1] && !aUseAssistHero[i][j]) // 有其它玩家
                    {
                        int nOtherPlayerIndex = (j == 0 ? 1 : 0);
                        pOtherPlayer = allPlayer[i][nOtherPlayerIndex];
                    }
                    
                    pPlayer->ProcessChallengeAward(
                        catWinAtLastStep, pOtherPlayer, aUseAssistHero[i][j], pPlayer->GetStartChallengeStep(), 
                        g_pSO3World->m_Settings.m_ConstList.nChallengeMissionCount + 1
                    );
                    pPlayer->SetLastChallengeStep(1);
                    pPlayer->m_vecChallengeAwardItem.clear();

                    dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1);
                    KGLOG_PROCESS_ERROR(dwMapID);

                    // 修改关卡
                    g_RelayClient.DoPveTeamChangeMissionRequest(pPlayer->m_dwID, pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1, dwMapID);
                }
                else if ((pPlayer->GetLastChallengeStep() - 1) % cdChallengeSmallStepCount == 0) // 每大关发奖
                {
                    KPlayer* pOtherPlayer = NULL;
                    if (allPlayer[i][0] && allPlayer[i][1] && !aUseAssistHero[i][j]) // 有其它玩家
                    {
                        int nOtherPlayerIndex = (j == 0 ? 1 : 0);
                        pOtherPlayer = allPlayer[i][nOtherPlayerIndex];
                    }

                    pPlayer->ProcessChallengeAward(
                        catWin5Step, pOtherPlayer, aUseAssistHero[i][j], pPlayer->GetStartChallengeStep(), 
                        pPlayer->GetLastChallengeStep()
                    );

                    pPlayer->SetStartChallengeStep(pPlayer->GetLastChallengeStep());
                    pPlayer->m_bNeedCheckCanEnter = false;

                    dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1);
                    KGLOG_PROCESS_ERROR(dwMapID);

                    // 修改关卡
                    g_RelayClient.DoPveTeamChangeMissionRequest(pPlayer->m_dwID, pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1, dwMapID);
                }
                else
                {
                    // 连续挑战赛进入下一阶段
                    dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1);
                    KGLOG_PROCESS_ERROR(dwMapID);

                    // 修改关卡
                    g_RelayClient.DoPveTeamChangeMissionRequest(pPlayer->m_dwID, pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1, dwMapID);
                    g_RelayClient.DoPveTeamStartGameRequest(pPlayer, false, false);
                }
            }
            else
            {
                KPlayer* pOtherPlayer = NULL;
                PLAYER_LOG(pPlayer, "challenge,lose,%d", pPlayer->GetLastChallengeStep());

                if (allHero[i][0] && allHero[i][1] && !aUseAssistHero[i][j]) // 有其它玩家
                {
                    int nOtherPlayerIndex = (j == 0 ? 1 : 0);
                    pOtherPlayer = allPlayer[i][nOtherPlayerIndex];
                }

                pPlayer->ProcessChallengeAward(
                    catFailedStep, pOtherPlayer, aUseAssistHero[i][j], pPlayer->GetStartChallengeStep(), 
                    pPlayer->GetLastChallengeStep()
                );

                pPlayer->m_bUseAssistHero           = aUseAssistHero[i][j];
                pPlayer->m_nFailedChallengeStep     = pMission->nStep;
                pPlayer->SetLastChallengeStep((pPlayer->GetLastChallengeStep() - 1) / cdChallengeSmallStepCount * cdChallengeSmallStepCount + 1);
                
                dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1);
                KGLOG_PROCESS_ERROR(dwMapID);

                // 修改关卡
                g_RelayClient.DoPveTeamChangeMissionRequest(pPlayer->m_dwID, pPlayer->GetCurrentChallengeType(), pPlayer->GetLastChallengeStep(), 1, dwMapID);

                g_PlayerServer.DoFinishChallengeMissionNotify(
                    pPlayer->m_nConnIndex, bMissionSuccessed, pPlayer->GetStartChallengeStep(), pPlayer->m_nFailedChallengeStep
                );
            }
        }  
    }
    
    g_LogClient.DoFlowRecord(
        frmtBattle, bfrChallengeEnd, 
        "%s,%s,%s,%s,%d,%d", 
        allPlayer[sidLeft][0] ? allPlayer[sidLeft][0]->m_szName : "",
        allPlayer[sidLeft][1] ? allPlayer[sidLeft][1]->m_szName : "",
        allPlayer[sidRight][0] ? allPlayer[sidRight][0]->m_szName : "",
        allPlayer[sidRight][1] ? allPlayer[sidRight][1]->m_szName : "",
        m_nScore[sidLeft], m_nScore[sidRight]
    );
Exit0:
    return;
}

BOOL KScene::ProcessMissionAwardExp(KHero* pHero, KMission* pMission, unsigned uCoeExp, int& rAwardedExp)
{
	BOOL        bResult     = false;
	BOOL        bRetCode    = false;
    int         nExpToAdd   = 0;
    int         nLevelParam = 0;
    KPlayer*    pOwner      = NULL;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pMission);

    pOwner = pHero->GetOwner();
    KGLOG_PROCESS_ERROR(pOwner);

    nLevelParam  = GetLevelParam(pMission->nExpLevel, pHero->m_nLevel);

    nExpToAdd = pMission->nBaseExp * uCoeExp / HUNDRED_NUM;
    nExpToAdd = nExpToAdd * nLevelParam / HUNDRED_NUM;

    bRetCode = pOwner->AddHeroExp(pHero->m_dwTemplateID, nExpToAdd, rAwardedExp);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KScene::IsBattleFinished() const
{
    return m_Battle.IsFinished();
}

void KScene::GetAllEnemy(int nSelfSide, std::vector<KHero*>& vecEnemy)
{
    GetAllHero(vecEnemy);
    for(std::vector<KHero*>::iterator it = vecEnemy.begin(); it != vecEnemy.end();)
    {
        KHero* pHero = *it;
        if (pHero->m_nSide == nSelfSide)
        {
            it = vecEnemy.erase(it);
            continue;
        }
        
        ++it;
    }
}

KHero* KScene::GetBallTaker()
{
    KHero* pBallHandler = NULL;
    KBall* pBall = NULL;
    
    pBall = GetBall();
    KG_PROCESS_ERROR(pBall);
    
    pBallHandler = pBall->GetBallTaker();

 Exit0:   
    return pBallHandler;
}

BOOL KScene::GetAllTeammate(KHero* pSelf, std::vector<KHero*>& vecAllTeammate)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KSceneObject*   pObj        = NULL;
    KHero*          pHero       = NULL;

    KGLOG_PROCESS_ERROR(pSelf);

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        pObj = it.GetValue();
        assert(pObj);

        bRetCode = pObj->Is(sotHero);
        if (!bRetCode)
            continue;

        pHero = (KHero*)pObj;
        if (pHero->m_nSide != pSelf->m_nSide)
            continue;
        
        if (pHero->m_dwID == pSelf->m_dwID)
            continue;
        
        vecAllTeammate.push_back(pHero);
    }
    
    bResult = true;
Exit0:
    return bResult;
}

void KScene::GetAllHero(std::vector<KHero*>& rvecHeros)
{
    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotHero)
            continue;

        rvecHeros.push_back((KHero*)pObj);
    }

    return;
}

void KScene::GetAllBasket(std::vector<KBasket*>& rvecBaskets)
{
    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotBasket)
            continue;
        
        rvecBaskets.push_back((KBasket*)pObj);
    }

    return;
}

void KScene::GetAllBasketSocket(std::vector<KBasketSocket*>& rvecSockets)
{
    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotBasketSocket)
            continue;
        
        rvecSockets.push_back((KBasketSocket*)pObj);
    }

    return;
}

void KScene::OnBallToken(KHero* pOwner)
{
    FireEvent(pOwner, aevOnSelfTakeBall, aevOnTeammateTakeBall, aevOnEnemyTakeBall);

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        int nRelation = 0;
        KSceneObject* pObj = it.GetValue();

        if (!pObj->Is(sotHero))
            continue;

        KHero* pTrigger = (KHero*)pObj;

        nRelation = pTrigger->GetBallTakerRelation();

        pTrigger->TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_GET_BALL, 0, nRelation);
    }
}

void KScene::OnBallDropped(KHero* pOldOwner)
{
    FireEvent(pOldOwner, aevOnSelfDropBall, aevOnTeammateDropBall, aevOnEnemyDropBall);
}

void KScene::OnBallShot(KHero* pShooter, KShootType eType)
{
    KAI_EVENT eTeammate = aevInvalid;
    KAI_EVENT eEnemy = aevInvalid;
    switch(eType)
    {
    case stNormalShoot:
        eTeammate   = aevOnTeammateNormalShoot;
        eEnemy      = aevOnEnemyNormalShoot;
        break;
    case stSkillShoot:
    case stShootBySkill:    // TODO:tongxuehu
        eTeammate   = aevOnTeammateSkillShoot;
        eEnemy      = aevOnEnemySkillShoot;
        break;
    case stSkillSlam:
        eTeammate   = aevOnTeammateSkillSlam;
        eEnemy      = aevOnEnemySkillSlam;
        break;
    case stSlamDunk:
        eTeammate   = aevOnTeammateSlam;
        eEnemy      = aevOnEnemySlam;
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Unknown ShootType:%d", eType);
        break;
    }
    FireEvent(pShooter, aevInvalid, eTeammate, eEnemy);
}

void KScene::OnBallPassTo(KHero* pPasser, KHero* pReceiver)
{
    FireEvent(pPasser, aevInvalid, aevOnTeammatePassBall, aevOnEnemyPassBall);
}

void KScene::OnBasketDropped(KBasket* pBasket, KBasketSocket* pSocket)
{
    FireAIEventToAll(aevOnBasketDropped, pBasket->m_dwID, pSocket->m_dwID);
}

void KScene::OnBasketPlugin(KBasket* pBasket, KBasketSocket* pSocket)
{
    FireAIEventToAll(aevOnBasketPlugin, pBasket->m_dwID, pSocket->m_dwID);
}

void KScene::FireAIEventToAll(KAI_EVENT eAIEvent, DWORD dwSrcID, int nEventParam)
{
    std::vector<KHero*> vecAllHero;
    GetAllHero(vecAllHero);
    for(std::vector<KHero*>::iterator it = vecAllHero.begin(); it != vecAllHero.end(); ++it)
    {
        KHero* pHero = *it;
        assert(pHero);

        pHero->FireAIEvent(eAIEvent, 0U, 0);
    }
}

void KScene::FireEvent(KHero* pSelf, KAI_EVENT eSelf, KAI_EVENT eTeammate, KAI_EVENT eEnemy)
{
    std::vector<KHero*> vecAllHero;
    GetAllHero(vecAllHero);
    for(std::vector<KHero*>::iterator it = vecAllHero.begin(); it != vecAllHero.end(); ++it)
    {
        KHero* pHero = *it;
        assert(pHero);

        if (pHero == pSelf)
        {
             if(eSelf != aevInvalid)
             {
                 pHero->FireAIEvent(eSelf, 0U, 0);
             }
        }
        else if (pHero->m_nSide == pSelf->m_nSide)
        {
            if (eTeammate != aevInvalid)
            {
                pHero->FireAIEvent(eTeammate, 0U, 0);
            }
        }
        else
        {
            if (eEnemy != aevInvalid)
            {
                pHero->FireAIEvent(eEnemy, 0U, 0);
            }         
        }
    }
}

void KScene::CallPlayerEvent()
{  
    int nWinnerSide = 0;
    int nDifferenceScore = 0;
	std::vector<KHero*> vecAllHero;

    GetAllHero(vecAllHero);

    nWinnerSide = GetWinnerSide();

    for (std::vector<KHero*>::iterator it = vecAllHero.begin(); it != vecAllHero.end(); ++it)
    {
        KHero* pHero = *it;
        assert(pHero);

        if (!pHero->IsMainHero())
            continue;

        KPlayer* pPlayer = pHero->GetOwner();
        if (pPlayer == NULL)
            continue;

        nDifferenceScore = m_nScore[pHero->m_nSide] - GetEnemyScore(pHero->m_nSide);

		if (m_Param.m_eBattleType == KBATTLE_TYPE_MISSION)
		{
			KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
			if (!pMission)
				continue;

			pPlayer->OnEvent(peFinishBattle, m_Param.m_eBattleType, nDifferenceScore, pMission->eType, pMission->nStep, pMission->nLevel);
		}
		else
		{
			pPlayer->OnEvent(peFinishBattle, m_Param.m_eBattleType, nDifferenceScore, m_dwMapID);
		}
    }
}


void KScene::OnAttacked(KHero* pHero)
{
    pHero->FireAIEvent(aevOnAttacked, 0U, 0);
}

KBullet* KScene::GetBulletByID(DWORD dwID)
{
    KBullet* pResult = NULL;
    KSceneObject* pObj = NULL;
    
    pObj = m_pSceneObjMgr->Get(dwID);
    if (pObj && pObj->GetType() == sotBullet)
    {
        pResult = (KBullet*)pObj;
    }

    return pResult;
}

KHero* KScene::AddHero(DWORD dwTemplateID, int nLevel, int nSide, int nPos, int nFaceDir /*= csdInvalid*/, const char* pszName /*= NULL*/)
{
    BOOL        bRetCode   = false;
    KHero*      pResult    = NULL;
    KSIDE_TYPE  eSide      = (KSIDE_TYPE)nSide;
    KHeroTemplateInfo* pTemplateInfo = NULL;
    KVEC_DWORD vecSkills;

    KGLOG_PROCESS_ERROR(dwTemplateID > 0);
    KGLOG_PROCESS_ERROR(nLevel > 0);
    KGLOG_PROCESS_ERROR(eSide == sidLeft || eSide == sidRight);
    KGLOG_PROCESS_ERROR(nPos >= 0 && nPos < MAX_TEAM_MEMBER_COUNT);
     
    pTemplateInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
    KGLOG_PROCESS_ERROR(pTemplateInfo);

    pResult = (KHero*)m_pSceneObjMgr->AddByType(sotHero);
    assert(pResult);

    pResult->m_nSide  = nSide;
    pResult->m_nPos   = nPos;

    pResult->m_eFaceDir = (KTWO_DIRECTION)nFaceDir;
    if (nFaceDir == csdInvalid)
    {
        pResult->m_eFaceDir = csdLeft;
        if (pResult->m_nSide == sidLeft)
            pResult->m_eFaceDir = csdRight;
    }
    
    if (pszName)
        strncpy(pResult->m_szName, pszName, countof(pResult->m_szName));
    else
        strncpy(pResult->m_szName, pTemplateInfo->szName, countof(pResult->m_szName));
    
    pResult->m_szName[countof(pResult->m_szName) - 1] = '\0';
    pResult->m_nVirtualFrame = g_pSO3World->m_nGameLoop;
    pResult->m_dwTemplateID = dwTemplateID;
    
    if (pTemplateInfo->AllFashionID.size())
    {
        unsigned uRandomIndex = g_Random(pTemplateInfo->AllFashionID.size());
        pResult->m_wFashionID = (WORD)pTemplateInfo->AllFashionID[uRandomIndex];
    }

    pResult->SetLevel(nLevel);

    for (int i = 0; i < KACTIVE_SLOT_TYPE_TOTAL; ++i)
    {
        vecSkills.clear();

        for (KVEC_DWORD::iterator it = pTemplateInfo->AllActiveSkill[i].begin(); it != pTemplateInfo->AllActiveSkill[i].end(); ++it)
        {
            KSkill* pSkill = g_pSO3World->m_SkillManager.GetSkill(*it);
            KGLOG_PROCESS_ERROR(pSkill);

            if (pSkill->m_nRequireLevel > nLevel)
                continue;

            vecSkills.push_back(*it);
        }

        if (vecSkills.size() == 0)
        {
            if (i == KACTIVE_SLOT_TYPE_BASE_FIST ||  i == KACTIVE_SLOT_TYPE_PEDIMENT_FIST ||
                i == KACTIVE_SLOT_TYPE_AIR_FIST || i == KACTIVE_SLOT_TYPE_RISE_FIST)
            {
                vecSkills.push_back(pTemplateInfo->dwInitBaseFistSkill);
            }
            else
            {
                vecSkills.push_back(pTemplateInfo->dwInitBaseFootSkill);
            }
        }
        
        pResult->m_ActiveSkill[i] = GetRandomOne(vecSkills);
    }
    
    vecSkills.clear();
    for (KVEC_DWORD::iterator it = pTemplateInfo->AlllPassiveSkill.begin(); it != pTemplateInfo->AlllPassiveSkill.end(); ++it)
    {
        KPassiveSkillData* pPassiveSkillData = g_pSO3World->m_Settings.m_PassiveSkillData.GetByID(*it);
        KGLOG_PROCESS_ERROR(pPassiveSkillData);

        if (pPassiveSkillData->nRequireLevel > nLevel)
            continue;

        vecSkills.push_back(*it);
    }  
    for (int i = 0; i < cdPassiveSkillSlotCount; ++i)
    {
        pResult->m_PassiveSkill[i] = GetRandomOne(vecSkills);
    }

    vecSkills.clear();
    for (KVEC_DWORD::iterator it = pTemplateInfo->AllNirvanaSkill.begin(); it != pTemplateInfo->AllNirvanaSkill.end(); ++it)
    {
        KSkill* pSkill = g_pSO3World->m_SkillManager.GetSkill(*it);
        KGLOG_PROCESS_ERROR(pSkill);

        if (pSkill->m_nRequireLevel > nLevel)
            continue;

        vecSkills.push_back(*it);
    }  
    for (int i = 0; i < cdNirvanaSkillSlotCount; ++i)
    {
        pResult->m_NirvanaSkill[i] = GetRandomOne(vecSkills);
    }
    
    vecSkills.clear();
    for (KVEC_DWORD::iterator it = pTemplateInfo->AllSlamBallSkill.begin(); it != pTemplateInfo->AllSlamBallSkill.end(); ++it)
    {
        KSlamBallSkill* pSkill = g_pSO3World->m_Settings.m_SlamBallSkillSettings.Get(*it);
        KGLOG_PROCESS_ERROR(pSkill);

        if (pSkill->nRequireLevel > nLevel)
            continue;

        vecSkills.push_back(*it);
    }
    for (int i = 0; i < cdSlamBallSkillSlotCount; ++i)
    {
        pResult->m_SlamBallSkill[i] = GetRandomOne(vecSkills);
    }

    pResult->m_vecAllOtherActiveSkill = pTemplateInfo->AllOtherActiveSkill;

    CallEnterSceneScript(pResult);
    CallGlobalEnterSceneScript(pResult);
   
Exit0:
    return pResult;
}

KHero* KScene::AddPlayerHero(KPlayer* pPlayer, const KHERO_INFO& rHeroInfo)
{
    KHero*          pResult     = NULL;
    KHero*          pTemp       = NULL;
    BOOL            bRetCode    = false;
    KSceneObject*   pObj        = NULL;
    KHeroData*      pHeroData   = NULL;
    KHeroTemplateInfo* pHeroTemplateInfo = NULL;
    KTWO_DIRECTION  eFaceDir    = csdRight;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(rHeroInfo.dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(rHeroInfo.nSide == sidLeft || rHeroInfo.nSide == sidRight);
    KGLOG_PROCESS_ERROR(rHeroInfo.nPos >= 0 && rHeroInfo.nPos < MAX_TEAM_MEMBER_COUNT);

    pHeroTemplateInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(rHeroInfo.dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroTemplateInfo);

    if (rHeroInfo.nSide == sidRight)
        eFaceDir = csdLeft;

    pHeroData = pPlayer->m_HeroDataList.GetHeroData(rHeroInfo.dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    pObj = m_pSceneObjMgr->AddByType(sotHero);
    KGLOG_PROCESS_ERROR(pObj);
    KGLOG_PROCESS_ERROR(pObj->GetType() == sotHero);

    pTemp = (KHero*)pObj;
    KGLOG_PROCESS_ERROR(pTemp);

    pTemp->m_nCreateTime  = g_pSO3World->m_nGameLoop;
    pTemp->m_bAiMode      = rHeroInfo.bAiMode;
    pTemp->m_nSide        = rHeroInfo.nSide;
    pTemp->m_nPos         = rHeroInfo.nPos;
    pTemp->m_eFaceDir     = eFaceDir;
    pTemp->m_dwTemplateID = pHeroData->m_dwTemplateID;
    pTemp->m_wFashionID   = pHeroData->m_wFashionID;

    memcpy(pTemp->m_szName, pPlayer->m_szName, sizeof(pTemp->m_szName));

    if (pTemp->m_bAiMode)
    {
        pTemp->AttachAsAITeammate(pPlayer);
    }
    else
    {
        pTemp->AttachAsFightingHero(pPlayer);
    }

    pHeroData->OnEnterScene();
    
    pTemp->SetLevel(pHeroData->m_nLevel);
    
    bRetCode = pTemp->m_BuffList.Load(pHeroData->m_mapBuffNode);
    KGLOG_PROCESS_ERROR(bRetCode);
    
    // 球队buff
    bRetCode = pPlayer->ApplyPlayerBuff(pTemp);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->ApplyHeroEquip(pTemp);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->ApplyHeroTalent(pTemp);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->ApplyHeroFashionAttr(pTemp);
    KGLOG_PROCESS_ERROR(bRetCode);

    pTemp->m_nCurrentAngry        = 0;
    pTemp->m_nCurrentEndurance    = pTemp->m_nMaxEndurance;
    pTemp->m_nCurrentStamina      = pTemp->m_nMaxStamina;

    memcpy(pTemp->m_ActiveSkill   , pHeroData->m_SelectedActvieSkill,     sizeof(pTemp->m_ActiveSkill));
    memcpy(pTemp->m_PassiveSkill  , pHeroData->m_SelectedPassiveSkill,    sizeof(pTemp->m_PassiveSkill));
    memcpy(pTemp->m_NirvanaSkill  , pHeroData->m_SelectedNirvanaSkill,    sizeof(pTemp->m_NirvanaSkill));
    memcpy(pTemp->m_SlamBallSkill , pHeroData->m_SelectedSlamBallSkill,   sizeof(pTemp->m_SlamBallSkill));

    pTemp->m_vecAllOtherActiveSkill = pHeroTemplateInfo->AllOtherActiveSkill;

    pTemp->SetUpAVM();
    pTemp->OnEnterScene(this);

    pResult = pTemp;
Exit0:
    if (pResult == NULL && pTemp)
    {
        m_pSceneObjMgr->Delete(pTemp->m_dwID);
        pTemp = NULL;
    }
    
    return pResult;
}

BOOL KScene::GetAwardParam(unsigned &uExpParam, unsigned &uMoneyParam)
{
    BOOL                bResult                     = false;
    BOOL                bRetCode                    = false;
    const unsigned      RANGE_COUNT                 = 9;
    KRANGE<unsigned>    allRange[RANGE_COUNT]       = {{0, 5}, {6,10}, {11, 15}, {16,20}, {21, 25}, {26, 30}, {31, 35}, {36,50}, {51, UINT_MAX}};
    unsigned            allCoe[RANGE_COUNT]         = {100, 101, 102, 103, 104, 105, 106, 107, 108};
    unsigned            uDeltaScore                 = 0U;
    unsigned            uRangeIndex                 = 0U;

    uExpParam = 0;
    uMoneyParam = 0;

    if (m_nScore[sidLeft] > m_nScore[sidRight])
        uDeltaScore = m_nScore[sidLeft] - m_nScore[sidRight];
    else
        uDeltaScore = m_nScore[sidRight] - m_nScore[sidLeft];

    for (uRangeIndex = 0; uRangeIndex < countof(allRange); ++uRangeIndex)
    {
        if (uDeltaScore >= allRange[uRangeIndex].tLower && uDeltaScore <= allRange[uRangeIndex].tUpper)
        {
            KGLOG_PROCESS_ERROR(uRangeIndex < countof(allCoe));

            uExpParam = allCoe[uRangeIndex];
            uMoneyParam = allCoe[uRangeIndex];
            break;
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

int  KScene::GetLevelParam(int nMissionLevel, int nHeroLevel)
{
    int nDeltLevel = (nMissionLevel - nHeroLevel);
    if (nDeltLevel < 0)
        nDeltLevel = 0;

    if (nDeltLevel <= 4)
        return 100;
    else if (nDeltLevel <= 5)
        return 90;
    else if (nDeltLevel <= 6)
        return 80;
    else if (nDeltLevel <= 7)
        return 70;
    else if (nDeltLevel <= 8)
        return 60;
    else if (nDeltLevel <= 9)
        return 50;
    return 40;
}

// Ra"=Ra+K(Sa-Ea)
// Sa为一个固定常数，胜等于1，和等于0.5，负等于0
// Ea为胜利期望值
// K作为一个加权值, K=Ko*S*R
// Ko含义:初始K值，最大为30，最小为10，初步定为130-Ra/20
// S含义:稳定因素，默认=1，竞技次数低于20场的新玩家S=2
// R含义:规模效应，R = 1+(组队排的玩家人数-1)x0.1

BOOL KScene::ProcessLadderPVPAward(int nWinnerSide, KHero* pHero, int& rAddedLadderPVPExp, int& rAddedMenterPoint)
{
	BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    KPlayer*    pOwner      = NULL;
    KHeroData*  pHeroData   = NULL;
    int         nAddedEol   = 0;
    int         nLadderExp = 0;
    int         nTotalLadderGameCount = 0;
    int         nMentorPointPercent = 100;
    KLADDER_LEVELDATA_ITEM* pLadderLevelDataItem = NULL;
    double      allEA[sidTotal] = {0.0};
    int         nEol[sidTotal] = {0};
    int         nDeltaEol = 0;
    double k = 0.0;
	double sa = 0.0;
    double ea = 0.0;
    int k0 = 0;
    int s = 1;
    double r = 0;
    double dHigherEA = 0.0;
    double dLowerEA = 0.0;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_nSide >= 0 && pHero->m_nSide < sidTotal);
    KGLOG_PROCESS_ERROR(pHero->m_nPos >= 0 && pHero->m_nPos < MAX_TEAM_MEMBER_COUNT);

    pOwner = pHero->GetOwner();
    KGLOG_PROCESS_ERROR(pOwner);

    pHeroData = pOwner->m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    pLadderLevelDataItem = g_pSO3World->m_Settings.m_LadderLevelData.GetLadderLevelData(pHeroData->m_nLadderLevel);
    KGLOG_PROCESS_ERROR(pLadderLevelDataItem);

    if (nWinnerSide == pHero->m_nSide)
    {
        ++pHeroData->m_nLadderWinCount;
		if (!m_LadderPVPFatigueCosted[pHero->m_nSide][pHero->m_nPos])
        {
            int nNoFatigueWinCount;

            bRetCode = pOwner->GetNoFatigueWinRankCount(nNoFatigueWinCount);
            KGLOG_PROCESS_ERROR(bRetCode);

            ++nNoFatigueWinCount;
            bRetCode = pOwner->SetNoFatigueWinRankCount(nNoFatigueWinCount);
            KGLOG_PROCESS_ERROR(bRetCode);

            nMentorPointPercent = g_pSO3World->m_Settings.m_MentorCutSettings.GetPercent(nNoFatigueWinCount);
        }
		rAddedMenterPoint = pLadderLevelDataItem->m_nMenterPointOnWin * nMentorPointPercent / 100;
        sa = 1.0;
    }
    else
    {
        ++pHeroData->m_nLadderLoseCount;
        rAddedMenterPoint = pLadderLevelDataItem->m_nMenterPointOnLose;

        sa = (nWinnerSide == cdNoTeamWinID ? 0.5 : 0.0);
    }
    
    k0 = 130 - pHeroData->m_nEol / 20;
    MakeInRange<int>(k0, 10, 30);

    nTotalLadderGameCount = pHeroData->m_nLadderWinCount + pHeroData->m_nLadderLoseCount + pHeroData->m_nLadderDrawCount;
    if (nTotalLadderGameCount <= 20)
        s = 2;

    r = 1 + (m_Param.m_byTogetherCount[pHero->m_nSide][pHero->m_nPos] - 1) * 0.1;

    k = k0 * s * r;

    memcpy(nEol, m_Param.m_nEol, sizeof(nEol));
    nEol[pHero->m_nSide] = pHeroData->m_nEol;

    g_pSO3World->m_Settings.m_EAData.GetEA(nEol, allEA);
    ea = allEA[pHero->m_nSide];

    nAddedEol = (int)(k * (sa - ea));
    
    if (nAddedEol > 0 || !pLadderLevelDataItem->m_bNoDecrease)
    {
        pHeroData->m_nEol += nAddedEol;
        if (pHeroData->m_nEol < 0)
            pHeroData->m_nEol = 0;

        KGLogPrintf(KGLOG_INFO, "Player ID:%u Name:%s 's Hero TemplateID:%u Eol %d Add %d.\n", pOwner->m_dwID, pOwner->m_szName, pHero->m_dwTemplateID, pHeroData->m_nEol, nAddedEol);
    }

    if (pOwner->m_bIsLimited)
    {
        rAddedMenterPoint = 0;
    }
    else
    {
        pOwner->m_MoneyMgr.AddMoney(emotMenterPoint, rAddedMenterPoint);
    }

    PLAYER_LOG(pOwner, "money,addmoney,%d,%s,%d-%d,%d,%d", emotMenterPoint, "ladderaward", 0, 0, 0, rAddedMenterPoint);
    
    nLadderExp = nAddedEol * pLadderLevelDataItem->m_nExpCoe;
    if (pLadderLevelDataItem->m_bNoDecrease && nLadderExp < 0)
        nLadderExp = 0;
    
    bRetCode = pOwner->AddHeroLadderExp(pHeroData->m_dwTemplateID, nLadderExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    rAddedLadderPVPExp = nLadderExp;

    g_LSClient.DoLadderDataChangeNotify(pOwner, pHeroData->m_dwTemplateID);

    bResult = true;
Exit0:
	return bResult;
}

BOOL KScene::GetAddedEol(KHero* pHero, int& rAddedEol)
{
    BOOL bResult = false;
    int nLadderLevel = 0;
    KLADDER_LEVELDATA_ITEM* pLadderLevelDataItem = NULL;
    int nWinnerSide = cdNoTeamWinID;
    double      allEA[sidTotal] = {0.0};
    int         nDeltaEol = 0;
    double k = 0.0;
    double sa = 0.0;
    double ea = 0.0;
    int k0 = 0;
    int s = 1;
    double r = 0;
    double dHigherEA = 0.0;
    double dLowerEA = 0.0;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_nSide >= 0 && pHero->m_nSide < sidTotal);
    KGLOG_PROCESS_ERROR(pHero->m_nPos >= 0 && pHero->m_nPos < MAX_TEAM_MEMBER_COUNT);

    nLadderLevel = m_Param.m_byLadderLevel[pHero->m_nSide][pHero->m_nPos];
    pLadderLevelDataItem = g_pSO3World->m_Settings.m_LadderLevelData.GetLadderLevelData(nLadderLevel);
    KGLOG_PROCESS_ERROR(pLadderLevelDataItem);

    nWinnerSide = GetWinnerSide();
    if (nWinnerSide == cdNoTeamWinID)
    {
        sa = 0.5;
    }
    else if (nWinnerSide == pHero->m_nSide)
    {
        sa = 1.0;
    }
    else
    {
        sa = 0.0;
    }

    k0 = 130 - m_Param.m_MemberEol[pHero->m_nSide][pHero->m_nPos] / 20;
    MakeInRange<int>(k0, 10, 30);
    
    s = 2;

    r = 1 + (m_Param.m_byTogetherCount[pHero->m_nSide][pHero->m_nPos] - 1) * 0.1;

    k = k0 * s * r;

    g_pSO3World->m_Settings.m_EAData.GetEA(m_Param.m_nEol, allEA);
    ea = allEA[pHero->m_nSide];

    rAddedEol = (int)(k * (sa - ea));

    bResult = true;
Exit0:
    return bResult;
}

BOOL KScene::GetLadderPVPAward(KHero* pHero, int& rAddedLadderPVPExp, int& rAddedMenterPoint)
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	int nAddedEol = 0;
    int nLadderLevel = 0;
    KLADDER_LEVELDATA_ITEM* pLadderLevelDataItem = NULL;
    int nWinnerSide = cdNoTeamWinID;

    KGLOG_PROCESS_ERROR(pHero);

    bRetCode = GetAddedEol(pHero, nAddedEol);
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLOG_PROCESS_ERROR(pHero->m_nSide == sidLeft || pHero->m_nSide == sidRight);
    KGLOG_PROCESS_ERROR(pHero->m_nPos >= 0 && pHero->m_nPos < MAX_TEAM_MEMBER_COUNT);

    nLadderLevel = m_Param.m_byLadderLevel[pHero->m_nSide][pHero->m_nPos];
    pLadderLevelDataItem = g_pSO3World->m_Settings.m_LadderLevelData.GetLadderLevelData(nLadderLevel);
    KGLOG_PROCESS_ERROR(pLadderLevelDataItem);

    nWinnerSide = GetWinnerSide();
    if (nWinnerSide == cdNoTeamWinID)
        rAddedMenterPoint = pLadderLevelDataItem->m_nMenterPointOnDraw;
    else if (nWinnerSide == pHero->m_nSide)
        rAddedMenterPoint = pLadderLevelDataItem->m_nMenterPointOnWin;
    else
        rAddedMenterPoint = pLadderLevelDataItem->m_nMenterPointOnLose;

    rAddedLadderPVPExp = nAddedEol * pLadderLevelDataItem->m_nExpCoe;
    if (pLadderLevelDataItem->m_bNoDecrease && rAddedLadderPVPExp < 0)
        rAddedLadderPVPExp = 0;

	bResult = true;
Exit0:
	return bResult;
}

BOOL KScene::GetFreePos(int nSide, int& nPos)
{
	BOOL bResult = false;
    BOOL posInUse[MAX_TEAM_MEMBER_COUNT] = {false, };

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotHero)
            continue;

        KHero* pHero = (KHero*)pObj;
        if (pHero->m_nSide != nSide)
            continue;

        KGLOG_PROCESS_ERROR(pHero->m_nPos >= 0 && pHero->m_nPos < MAX_TEAM_MEMBER_COUNT);
        posInUse[pHero->m_nPos] = true;
    }

    for (int i = 0; i < countof(posInUse); ++i)
    {
        if (posInUse[i])
            continue;

        nPos = i;
        bResult = true;
        break;
    }
    
Exit0:
	return bResult;    
}

BOOL KScene::LoadAIHeroInLadder()
{
	BOOL bResult = false;
	BOOL bRetCode = false;
    KHero* pMember = NULL;
    KHeroTemplateInfo* pHeroTemplateInfo = NULL;
    int nAIType = 0;
	
    KGLOG_PROCESS_ERROR(m_Param.m_eBattleType == KBATTLE_TYPE_LADDER_PVP);

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (m_Param.m_dwMember[i][j])
                continue;

            KTWO_DIRECTION eFaceDir = csdRight;
            if (i == sidRight)
                eFaceDir = csdLeft;

            int nAINormalLevel = m_Param.m_nNormalLevel[i][j];
            MakeInRange(nAINormalLevel, 1, (int)cdMaxHeroLevel);

            pMember = AddHero(m_Param.m_wHeroTemplate[i][j], nAINormalLevel, i, j, eFaceDir, m_Param.m_Name[i][j]);
            KGLOG_PROCESS_ERROR(pMember);

            g_PlayerServer.DoBroadcastNewHero(pMember, ERROR_ID);

            pHeroTemplateInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(m_Param.m_wHeroTemplate[i][j]);
            KGLOG_PROCESS_ERROR(pHeroTemplateInfo);
            
            bRetCode = g_pSO3World->m_AIManager.GetRandomAIType(pHeroTemplateInfo->dwAIGroupID, m_Param.m_nAIType[i][j], nAIType);
            KGLOG_PROCESS_ERROR(bRetCode);

            pMember->m_bAiMode = true;
            pMember->m_AIData.nAIType = nAIType;
            pMember->SetUpAVM();
        }
    }
      
Exit1:
	bResult = true;
Exit0:
	return bResult;
}


BOOL KScene::PlayerSetReady(KPlayer* pPlayer)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pPlayer);
    
    KGLOG_PROCESS_ERROR(m_eSceneState == ssWaitingClientLoading);

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (m_Param.m_dwMember[i][j]  != pPlayer->m_dwID)
                continue;

            if (m_Param.m_bIsAIMode[i][j])
                continue;

            m_bReady[i][j] = true;

            bResult = true;
            goto Exit0;
        }
    }

Exit0:
    return bResult;
}

BOOL KScene::IsAllReady()
{
    BOOL bResult = false;
    KPlayer* pPlayer = NULL;

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {    
            if (!m_Param.m_dwMember[i][j])
                continue;

            if (m_Param.m_bIsAIMode[i][j])
                continue;

            pPlayer = g_pSO3World->m_PlayerSet.GetObj(m_Param.m_dwMember[i][j]);
            KG_PROCESS_ERROR(pPlayer);
            KG_PROCESS_ERROR(m_bReady[i][j]);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KScene::IsOnlineReady()
{
    BOOL bResult = false;
    KPlayer* pPlayer = NULL;

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {    
            if (!m_Param.m_dwMember[i][j])
                continue;

            if (m_Param.m_bIsAIMode[i][j])
                continue;

            pPlayer = g_pSO3World->m_PlayerSet.GetObj(m_Param.m_dwMember[i][j]);
            if (pPlayer == NULL)
                continue;

            KG_PROCESS_ERROR(m_bReady[i][j]);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

int KScene::GetCanEnterPlayerCount()
{
    int  nPlayerCount = 0;

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {    
            if (!m_Param.m_dwMember[i][j])
                continue;

            if (m_Param.m_bIsAIMode[i][j])
                continue;

            ++nPlayerCount;
        }
    }

    return nPlayerCount;
}

void KScene::BroadcastLoadingCompleted()
{
    KPlayer*    pPlayer         = NULL;
    KScene*     pScene          = NULL;
    int         nLeftFrame      = 0;

    nLeftFrame = m_nShouldStartFrame - g_pSO3World->m_nGameLoop;

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (m_Param.m_bIsAIMode[i][j])
                continue;

            if (!m_Param.m_dwMember[i][j])
                continue;

            pPlayer = g_pSO3World->m_PlayerSet.GetObj(m_Param.m_dwMember[i][j]);
            if (pPlayer == NULL)
                continue;

            KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForNewMapLoading);

            pScene = g_pSO3World->GetScene(pPlayer->m_dwMapID, pPlayer->m_nCopyIndex);
            KGLOG_PROCESS_ERROR(pScene);

            pPlayer->m_eGameStatus      = gsPlaying;
            pPlayer->m_nLastClientFrame = 0;
            g_PlayerServer.DoSyncStartedFrame(m_Battle.GetStartedFrame(), m_Battle.m_nFramePerBattle, nLeftFrame, pPlayer->m_nConnIndex, this);
            g_LSClient.DoRemoteCall("OnUpdateBattleStatus", (int)pPlayer->m_dwID, true);
        }
    }

Exit0:
    return;
}

BOOL KScene::GetAllHeroTemplate(unsigned int (&allHero)[sidTotal * MAX_TEAM_MEMBER_COUNT])
{
	BOOL bResult = false;
	
    memset(allHero, 0, sizeof(allHero));
    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            allHero[i * MAX_TEAM_MEMBER_COUNT + j] = m_Param.m_wHeroTemplate[i][j];
        }   
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KScene::GetAllHeroAIMode(bool (&aiMode)[sidTotal * MAX_TEAM_MEMBER_COUNT])
{
    BOOL bResult = false;

    memset(aiMode, 0, sizeof(aiMode));
    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            aiMode[i * MAX_TEAM_MEMBER_COUNT + j] = m_Param.m_bIsAIMode[i][j] > 0;
        }   
    }

    bResult = true;
Exit0:
    return bResult;
}

int KScene::GetPlayerPosInTeam(DWORD dwPlayerID)
{
    int nIndex = -1;
    for(int i = 0; i < sidTotal; ++i)
    {
        for(int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (dwPlayerID == m_Param.m_dwMember[i][j])
            {
                nIndex = i * MAX_TEAM_MEMBER_COUNT + j + 1;
                break;
            }
        }
    }
    return nIndex;
}

BOOL KScene::IsPlayerReady(DWORD dwPlayerID)
{
    BOOL bResult = false;

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (m_Param.m_dwMember[i][j] != dwPlayerID)
                continue;

            if (m_Param.m_bIsAIMode[i][j])
                continue;

            if (m_bReady[i][j])
                bResult = true;

            goto Exit0;
        }
    }

Exit0:
    return bResult;
}

void KScene::GetGrouppedHeroWithOwner(KMAP_SCENE_PLAYERS& rAllPlayer)
{
    rAllPlayer.clear();

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;
        
        KHero*      pHero   = (KHero*)pObj;
        KPlayer*    pPlayer = pHero->GetOwner();
        if (!pPlayer)
            continue;

        rAllPlayer[pPlayer].push_back(pHero);     
    }  
}

void KScene::OnClientLoading(int nCurLoop)
{
    BOOL bRetCode = false;
    KMission* pMission = NULL;

    if (nCurLoop > (m_nClientLoadingFrame + MAX_CLIENT_MAP_LOADING_FRAME))
    {
        CancelBattle();
        goto Exit0;
    }

    if (nCurLoop < (m_nClientLoadingFrame + MAX_DIRECT_ENTER_SCENE_FRAME))
    {
        bRetCode = IsAllReady();
        KG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        bRetCode = IsOnlineReady();
        KG_PROCESS_ERROR(bRetCode);
    }

    if (m_Param.m_eBattleType == KBATTLE_TYPE_MISSION && !m_bLoadNpcAfterAllPlayerEnter)
    {
        LoadNpcInMission();
        CallOnPlayersInMissionReadyScript();
    }

    for (KObjEnumerator it = m_pSceneObjMgr->GetEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if(!pObj)
            continue;

        if (pObj->Is(sotHero))
        {
            KHero* pHero = (KHero*)pObj;
            pHero->m_nVirtualFrame = g_pSO3World->m_nGameLoop;	
        }
    }

	pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
	if (pMission && pMission->nUnenableReady)
    {
        m_nShouldStartFrame = nCurLoop;
        BroadcastLoadingCompleted();
        OnCountDown(nCurLoop);
    }
	else
    {
		m_nShouldStartFrame = nCurLoop + BATTLE_START_COUNT_DOWN;
        m_eSceneState = ssCountDown;
		
        BroadcastLoadingCompleted();
    }

Exit0:
    return;
}

void KScene::ActivateHeroPassiveSkill()
{
    KHero*              pHero       = NULL;
    KSceneObject*       pObj        = NULL;

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        pObj = it.GetValue();
        assert(pObj);

        if (pObj->GetType() != sotHero)
            continue;

        pHero = (KHero*)pObj;

        pHero->ApplyPassiveSkill();
    }
}

void KScene::OnCountDown(int nCurLoop)
{
    m_pSceneObjMgr->ActivateHeroVirtualFrame();

    if (nCurLoop >= m_nShouldStartFrame)
    {
        m_Battle.Start(nCurLoop);
        m_eSceneState = ssFighting;
        FireAIEventToAll(aevOnGameStart, 0, 0);

        ActivateHeroPassiveSkill();

        ProcessFlowRecord();
    }
}

void KScene::OnFighting(int nCurLoop)
{
    if (m_bPause)
    {
        m_pSceneObjMgr->ActivateHeroVirtualFrame();
        return;
    }
    
    m_pSceneObjMgr->Activate(nCurLoop);
    m_pSceneObjMgr->SpecialActivate(nCurLoop);
    m_pSceneObjMgr->ProcessCollision();

    m_Battle.Activate(nCurLoop);

    m_ScriptTimerList.Activate();

    if (m_Battle.IsFinished())
        OnBattleFinished();

    m_DramaPlayer.Activate();
}

void KScene::CancelBattle()
{
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_eSceneState == ssWaitingClientLoading);

    bRetCode = ResetAllPlayer();
    KGLOG_PROCESS_ERROR(bRetCode);

    g_RelayClient.DoBattleFinishedNotify(m_dwMapID, m_nCopyIndex, BATTLE_CANCELED_WINNER_SIDE);
    m_eSceneState = ssWatingDelete;

Exit0:
    return;
}

BOOL KScene::ResetAllPlayer()
{
	BOOL bResult = false;
    KPlayer* pPlayer = NULL;
	
    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            if (!m_Param.m_dwMember[i][j])
                continue;

            if (m_Param.m_bIsAIMode[i][j])
                continue;

            pPlayer = g_pSO3World->m_PlayerSet.GetObj(m_Param.m_dwMember[i][j]);
            if (!pPlayer)
                continue;
            
            pPlayer->ReturnToHall();
        }
    }
    

	bResult = true;
Exit0:
	return bResult;
}

KHero* KScene::AddNpc(DWORD dwNpcTemplateID, int nSide, const KPOSITION& rPos, int nFaceDir, int nAIType)
{
    KHero*          pResult         = NULL;
    BOOL            bRetCode        = false;
    KNpcTemplate*   pNpcTemplate    = NULL;
    int             nPos            = 0;

    KGLOG_PROCESS_ERROR(dwNpcTemplateID);
    KGLOG_PROCESS_ERROR(nSide == sidRight || nSide == sidLeft);
    KGLOG_PROCESS_ERROR(rPos.nX >= 0 && rPos.nX <= GetLength());
    KGLOG_PROCESS_ERROR(rPos.nY >= 0 && rPos.nY <= GetWidth());
    KGLOG_PROCESS_ERROR(rPos.nZ >= 0);
    KGLOG_PROCESS_ERROR(nFaceDir == csdRight || nFaceDir == csdLeft);

    bRetCode = GetFreePos(nSide, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    pNpcTemplate = g_pSO3World->m_Settings.m_NpcTemplateMgr.GetTemplate(dwNpcTemplateID);
    KGLOG_PROCESS_ERROR(pNpcTemplate);

    pResult = (KHero*)m_pSceneObjMgr->AddByType(sotHero);
    KGLOG_PROCESS_ERROR(pResult);

    // nAIType 为负数表示不指定ai；为0表示随机一个ai；为正数表示指定特定ai
    if (nAIType == 0)
    {
        bRetCode = GetNpcAIType(pNpcTemplate, nAIType);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (nAIType > 0)
    {
        pResult->m_AIData.nAIType = nAIType;
        pResult->m_bAiMode        = true;
    }

    pResult->m_nSide          = nSide;
    pResult->m_nPos           = nPos;
    pResult->m_eFaceDir       = (KTWO_DIRECTION)nFaceDir;
    pResult->m_nVirtualFrame  = g_pSO3World->m_nGameLoop;
    
    memcpy(pResult->m_szName, pNpcTemplate->szName, sizeof(pResult->m_szName));
    pResult->m_pNpcTemplate     = pNpcTemplate;
    pResult->m_nLevel           = pNpcTemplate->nLevel;
    pResult->m_nLength          = pNpcTemplate->nLength;
    pResult->m_nWidth           = pNpcTemplate->nWidth;
    pResult->m_nHeight          = pNpcTemplate->nHeight;
    pResult->SetPosition(rPos);

    pResult->CallAttributeFunction(atMaxEndurance,                true,  pNpcTemplate->nMaxEndurance,                0);
    pResult->CallAttributeFunction(atEnduranceReplenish,          true,  pNpcTemplate->nEnduranceReplenish,          0);
    pResult->CallAttributeFunction(atEnduranceReplenishPercent,   true,  pNpcTemplate->nEnduranceReplenishPercent,   0);
    pResult->CallAttributeFunction(atMaxStamina,                  true,  pNpcTemplate->nMaxStamina,                  0);
    pResult->CallAttributeFunction(atStaminaReplenish,            true,  pNpcTemplate->nStaminaReplenish,            0);
    pResult->CallAttributeFunction(atStaminaReplenishPercent,     true,  pNpcTemplate->nStaminaReplenishPercent,     0);
    pResult->CallAttributeFunction(atMaxAngry,                    true,  pNpcTemplate->nMaxAngry,                    0);
    pResult->CallAttributeFunction(atAdditionalRecoveryAngry,     true,  pNpcTemplate->nAdditionalRecoveryAngry,     0);
    pResult->CallAttributeFunction(atWillPower,                   true,  pNpcTemplate->nWillPower,                   0);
    pResult->CallAttributeFunction(atInterference,                true,  pNpcTemplate->nInterference,                0);
    pResult->CallAttributeFunction(atInterferenceRange,           true,  pNpcTemplate->nInterferenceRange,           0);
    pResult->CallAttributeFunction(atAddInterferenceRangePercent, true,  pNpcTemplate->nAddInterferenceRangePercent, 0);
    pResult->CallAttributeFunction(atAttackPoint,                 true,  pNpcTemplate->nAttackPoint,                 0);
    pResult->CallAttributeFunction(atAttackPointPercent,          true,  pNpcTemplate->nAttackPointPercent,          0);
    pResult->CallAttributeFunction(atAgility,                     true,  pNpcTemplate->nAgility,                     0);
    pResult->CallAttributeFunction(atCritPoint,                   true,  pNpcTemplate->nCritPoint,                   0);
    pResult->CallAttributeFunction(atCritRate,                    true,  pNpcTemplate->nCritRate,                    0);
    pResult->CallAttributeFunction(atDefense,                     true,  pNpcTemplate->nDefense,                     0);
    pResult->CallAttributeFunction(atDefensePercent,              true,  pNpcTemplate->nDefensePercent,              0);
    pResult->CallAttributeFunction(atRunSpeedBase,                true,  pNpcTemplate->nMoveSpeedBase,               0);
    pResult->CallAttributeFunction(atAddMoveSpeedPercent,         true,  pNpcTemplate->nAddMoveSpeedPercent,         0);
    pResult->CallAttributeFunction(atJumpSpeedBase,               true,  pNpcTemplate->nJumpSpeedBase,               0);
    pResult->CallAttributeFunction(atJumpSpeedPercent,            true,  pNpcTemplate->nJumpSpeedPercent,            0);
    pResult->CallAttributeFunction(atShootBallHitRate,            true,  pNpcTemplate->nHitRate,                     0);
    pResult->CallAttributeFunction(atNomalShootForce,             true,  pNpcTemplate->nNormalShootForce,            0);
    pResult->CallAttributeFunction(atSkillShootForce,             true,  pNpcTemplate->nSkillShootForce,             0);
    pResult->CallAttributeFunction(atSlamDunkForce,               true,  pNpcTemplate->nSlamDunkForce,               0);

    pResult->m_nCurrentEndurance    = pResult->m_nMaxEndurance;
    pResult->m_nCurrentStamina      = pResult->m_nMaxStamina;
    pResult->m_nCurrentAngry        = 0;

    memcpy(pResult->m_ActiveSkill   ,pNpcTemplate->ActiveSkill,     sizeof(pResult->m_ActiveSkill));
    memcpy(pResult->m_PassiveSkill  ,pNpcTemplate->PassiveSkill,    sizeof(pResult->m_PassiveSkill));
    memcpy(pResult->m_NirvanaSkill  ,pNpcTemplate->NirvanaSkill,    sizeof(pResult->m_NirvanaSkill));
    memcpy(pResult->m_SlamBallSkill ,pNpcTemplate->SlamBallSkill,   sizeof(pResult->m_SlamBallSkill));

    pResult->m_vecAllOtherActiveSkill = pNpcTemplate->AllOtherActiveSkill;

    CallEnterSceneScript(pResult);
    CallGlobalEnterSceneScript(pResult);

    pResult->SetUpAVM();

    g_PlayerServer.DoSyncNewNpc(pResult);

Exit0:
    return pResult;
}

BOOL KScene::LoadNpcInMission()
{
	BOOL        bResult  = false;
	BOOL        bRetCode = false;
	KMission*   pMission = NULL;
    KHero*      pNpc     = NULL;   
    int         nFaceDir = 0;
    int         nAIType = 0;

	KGLOG_PROCESS_ERROR(m_Param.m_eBattleType == KBATTLE_TYPE_MISSION);

    pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMission);

    if (pMission->eType == emitPVE1ContinuousChallenge || pMission->eType == emitPVE2ContinuousChallenge) // 连续挑战赛随机npc
    {
        int nNpcID1 = 0;
        int nNpcID2 = 0;
        KPOSITION pos;

        bRetCode = g_pSO3World->m_Settings.m_ChallengeMissionNpc.GetTwoRandomNpc(pMission->eType, pMission->nStep, nNpcID1, nNpcID2);
        KGLOG_PROCESS_ERROR(bRetCode);

        if (nNpcID1 > 0)
        {
            pos.nX = 9344;
            pos.nY = 2560;
            pos.nZ = 0;

            pNpc = AddNpc(nNpcID1, sidRight, pos, csdLeft, 0);
            KGLOG_PROCESS_ERROR(pNpc);
        }
        
        if (nNpcID2 > 0)
        {
            pos.nX = 9344;
            pos.nY = 512;
            pos.nZ = 0;

            pNpc = AddNpc(nNpcID2, sidRight, pos, csdLeft, 0);
            KGLOG_PROCESS_ERROR(pNpc);
        }
        
        goto Exit1;
    }
    
    for (int i = 0; i < countof(pMission->Npc); ++i)
    {
        const KMission::KNpcInfo& cNpcInfo = pMission->Npc[i];
        if (!cNpcInfo.dwTemplateID)
            continue;

        nFaceDir = csdLeft;
        if (cNpcInfo.nSide == sidLeft)
            nFaceDir = csdRight;

        pNpc = AddNpc(cNpcInfo.dwTemplateID, cNpcInfo.nSide, cNpcInfo.cPos, nFaceDir, cNpcInfo.nAIType);
        KGLOG_PROCESS_ERROR(pNpc);
    }
    
Exit1:
	bResult = true;
Exit0:
	return bResult;
}

KBasketSocket* KScene::GetBasketSocketByID(DWORD dwBasketSocketID)
{
    KBasketSocket*  pResult = NULL;
    KSceneObject*   pObj    = NULL;

    pObj = GetSceneObjByID(dwBasketSocketID);
    if (pObj && pObj->Is(sotBasketSocket))
    {
        pResult = (KBasketSocket*)pObj;
    }
    
    return pResult;
}

KBasketSocket* KScene::GetSkillShootBallTargetBasketSocket(KTWO_DIRECTION eShooterFaceDir)
{
    KBasketSocket* pResult = NULL;
    KBasketSocket* pBasketSocket = NULL;
    KTWO_DIRECTION eDirFilter = csdRight;
   
    if (eShooterFaceDir == csdRight)
        eDirFilter = csdLeft;    

    pBasketSocket = GetBasketSocket(eDirFilter, bfSixth);
    KG_PROCESS_ERROR(pBasketSocket);

    if (pBasketSocket->IsEmpty())
    {
        pBasketSocket = GetBasketSocket(eDirFilter, bfFifth);
        KG_PROCESS_ERROR(pBasketSocket);
    }

    pResult = pBasketSocket;
Exit0:
    return pResult;    
}

KBasketSocket* KScene::GetSkillSlamBallTargetBasketSocket(KTWO_DIRECTION eShooterFaceDir, int nBackboradFloor)
{
    KBasketSocket* pResult = NULL;
    KBasketSocket* pBasketSocket = NULL;
    KTWO_DIRECTION eDirFilter = csdRight;
    KBasketSocketFloor eFloor[2];
    
    switch(nBackboradFloor)
    {
    case 1:
        eFloor[0] = bfFirst;
        eFloor[1] = bfSecond;
        break;
    case 2:
        eFloor[0] = bfThird;
        eFloor[1] = bfForth;
        break;
    case 3:
        eFloor[0] = bfFifth;
        eFloor[1] = bfSixth;
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
    }

    if (eShooterFaceDir == csdRight)
        eDirFilter = csdLeft;

    pBasketSocket = GetBasketSocket(eDirFilter, eFloor[1]);
    KG_PROCESS_ERROR(pBasketSocket);

    if (pBasketSocket->IsEmpty())
    {
        pBasketSocket = GetBasketSocket(eDirFilter, eFloor[0]);
        KG_PROCESS_ERROR(pBasketSocket);
    }

    pResult = pBasketSocket;
Exit0:
    return pResult;    
}

KBasket* KScene::GetBasket(int nSide, int nFloor)
{
    KBasket* pBasket = NULL;

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotBasketSocket))
            continue;

        KBasketSocket* pSocket = (KBasketSocket*)pObj;
        if (pSocket->m_eFloor != nFloor)
            continue;

        if (pSocket->m_nSide != nSide)
            continue;
        
        pBasket = pSocket->m_pBasket;
        break;
    }

    return pBasket;
}

BOOL KScene::GetTeamAILevelCoe(int& nAILevelCoe)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int  nPlayerIndex = 0;
    size_t uPlayerAILevelCount = 0;
    int  PlayerAILevelCoe[MAX_TEAM_MEMBER_COUNT] = {INT_MIN, INT_MIN};

	KG_PROCESS_ERROR(m_Param.m_eBattleType == KBATTLE_TYPE_MISSION);
    
    uPlayerAILevelCount = m_vecPlayerAILevelCoe.size();
    KGLOG_PROCESS_ERROR(uPlayerAILevelCount <= 2);

    switch(uPlayerAILevelCount)
    {
    case 0:
        nAILevelCoe = 0;                    
        break;
    case 1: 
        nAILevelCoe = m_vecPlayerAILevelCoe[0];
        break;
    case 2:
        if (m_vecPlayerAILevelCoe[0] > m_vecPlayerAILevelCoe[1])
            nAILevelCoe = (m_vecPlayerAILevelCoe[0] * 70 + m_vecPlayerAILevelCoe[1] * 30) / 100;
        else
            nAILevelCoe = (m_vecPlayerAILevelCoe[1] * 70 + m_vecPlayerAILevelCoe[0] * 30) / 100;
        break;
    //default:
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KScene::GetNpcAIType(KNpcTemplate* pNpcTemplate, int& nAIType)
{
	BOOL        bResult         = false;
	BOOL        bRetCode        = false;
	int         nAILevel        = 0;
    int         nTeamAILevelCoe = 0;
    KMission*   pMission        = NULL;

	KGLOG_PROCESS_ERROR(pNpcTemplate);

    pMission = g_pSO3World->m_MissionMgr.GetMission(m_dwMapID);
    KGLOG_PROCESS_ERROR(pMission);

    nAILevel = pNpcTemplate->nAILevel;
    if (pMission->bUseAICoe)
    {
        bRetCode = GetTeamAILevelCoe(nTeamAILevelCoe);
        KGLOG_PROCESS_ERROR(bRetCode);
        nAILevel += nTeamAILevelCoe;

        if (pNpcTemplate->nMinAILevel && nAILevel < pNpcTemplate->nMinAILevel)
            nAILevel = pNpcTemplate->nMinAILevel;

        if (pNpcTemplate->nMaxAILevel && nAILevel > pNpcTemplate->nMaxAILevel)
            nAILevel = pNpcTemplate->nMaxAILevel;
    }

    bRetCode = g_pSO3World->m_AIManager.GetRandomAIType(pNpcTemplate->dwAIGroupID, nAILevel, nAIType);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KScene::AddPlayer(KPlayer* pPlayer, int nSide)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	int nPos = 0;
    KHeroData* pHeroData = NULL;

	KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(nSide == sidRight || nSide == sidLeft);

    KGLOG_PROCESS_ERROR(m_eSceneState == ssFighting);

    pHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pHeroData);

    bRetCode = GetFreePos(nSide, nPos);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_Param.m_bIsAIMode[nSide][nPos]        = false;
    m_Param.m_byLadderLevel[nSide][nPos]    = (BYTE)pHeroData->m_nLadderLevel;
    m_Param.m_byTogetherCount[nSide][nPos]  = (BYTE)1;
    m_Param.m_byVIPLevel[nSide][nPos]       = (BYTE)pPlayer->m_nVIPLevel;
    m_Param.m_dwMember[nSide][nPos]         = pPlayer->m_dwID;
    m_Param.m_MemberEol[nSide][nPos]        = pHeroData->m_nEol;
    m_Param.m_nAIType[nSide][nPos]          = 0;
    memcpy(m_Param.m_Name[nSide][nPos], pPlayer->m_szName, sizeof(m_Param.m_Name[nSide][nPos]));
    m_Param.m_nNormalLevel[nSide][nPos]     = pHeroData->m_nLevel;
    m_Param.m_wHeroTemplate[nSide][nPos]    = (WORD)pHeroData->m_dwTemplateID;

	bResult = true;
Exit0:
	return bResult;
}

BOOL KScene::SyncCheerleadingsOnPlayerEnterScene(KPlayer* pPlayer, int nSide, int nPos)
{
    BOOL  bResult = false;
    BOOL  bRetCode = false;
   
    g_PlayerServer.DoSyncCheerleadingInfoInBattle(nSide, nPos, pPlayer, pPlayer->m_nConnIndex);

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;

        KHero* pHero = (KHero*)pObj;

        KPlayer* pOwner = pHero->GetOwner();
        if (!pOwner)
            continue;

        if (pOwner->m_pFightingHero != pHero)
            continue;

        if (pOwner == pPlayer)
            continue;
        
        g_PlayerServer.DoSyncCheerleadingInfoInBattle(pHero->m_nSide, pHero->m_nPos, pOwner, pPlayer->m_nConnIndex);
        g_PlayerServer.DoSyncCheerleadingInfoInBattle(nSide, nPos, pPlayer, pOwner->m_nConnIndex);
    }

    bResult = true;
Exit0:
    return bResult;
}

KCellEx* KScene::GetCellEx(int nX, int nY)
{
	KCellEx* pResult = NULL;
    int nCellX = nX / CELL_LENGTH;
    int nCellY = nY / CELL_LENGTH;

    KG_PROCESS_ERROR(nCellX >= 0);
    KG_PROCESS_ERROR(nCellX < m_nXGridCount);
    KG_PROCESS_ERROR(nCellY >= 0);
    KG_PROCESS_ERROR(nCellY < m_nYGridCount);
	
    pResult = m_PathFinder.GetCell(nCellX, nCellY);

Exit0:
	return pResult;
}

BOOL KScene::WillPassingCell(int nX, int nY, int nVelocityX, int nVelocityY, KCellEx* pCell)
{
    BOOL bResult = false;
    KCellEx* pCurrent = NULL;
    int nFrameEnterX = 0;
    int nFrameLeaveX = INT_MAX;
    int nFrameEnterY = 0;
    int nFrameLeaveY = INT_MAX;
    int nFrameEnter = 0;
    int nFrameLeave = 0;

    pCurrent = GetCellEx(nX, nY);
    KGLOG_PROCESS_ERROR(pCurrent);
    if (pCurrent == pCell)
        goto Exit1;

    if (nVelocityX == 0 && pCurrent->nX != pCell->nX)
        goto Exit0;

    if (nVelocityY == 0 && pCurrent->nY != pCell->nY)
        goto Exit0;    
      
    if (nVelocityX)
    {
        nFrameEnterX = (pCell->nX * CELL_LENGTH - nX) / nVelocityX;
        nFrameLeaveX = ((pCell->nX + 1) * CELL_LENGTH - 1 - nX) / nVelocityX;
        if (nFrameEnterX > nFrameLeaveX)
            swap(nFrameEnterX, nFrameLeaveX);

        if (nFrameLeaveX < 0)
            goto Exit0;

        if (nFrameEnterX < 0)
            nFrameEnterX = 0;
    }
  
    if (nVelocityY)
    {
        nFrameEnterY = (pCell->nY * CELL_LENGTH - nY) / nVelocityY;
        nFrameLeaveY = ((pCell->nY + 1) * CELL_LENGTH - 1 - nY) / nVelocityY;
        if (nFrameEnterY > nFrameLeaveY)
            swap(nFrameEnterY, nFrameLeaveY);

        if (nFrameLeaveY < 0)
            goto Exit0;

        if (nFrameEnterY < 0)
            nFrameEnterY = 0;
    }

    nFrameEnter = MAX(nFrameEnterX, nFrameEnterY);
    nFrameLeave = MIN(nFrameLeaveX, nFrameLeaveY);

    if (nFrameEnter > nFrameLeave)
        goto Exit0;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KScene::GetCellExRange(KSceneObject* pObj, KCellEx* &pLeftBottom, KCellEx* &pRightTop)
{
	BOOL bResult        = false;
	BOOL bRetCode       = false;   
    int nLeftBottomX    = 0;
    int nLeftBottomY    = 0;
    int nRightTopX      = 0;
    int nRightTopY      = 0;

    bRetCode = pObj->GetAttackRange(nLeftBottomX, nLeftBottomY, nRightTopX, nRightTopY);
    KGLOG_PROCESS_ERROR(bRetCode);

    MAKE_IN_RANGE(nLeftBottomX, 0, GetLength() - 1);
    MAKE_IN_RANGE(nLeftBottomY, 0, GetWidth() - 1);

    MAKE_IN_RANGE(nRightTopX, 0, GetLength() - 1);
    MAKE_IN_RANGE(nRightTopY, 0, GetWidth() - 1);

    pLeftBottom = GetCellEx(nLeftBottomX, nLeftBottomY);
    KGLOG_PROCESS_ERROR(pLeftBottom);

    pRightTop = GetCellEx(nRightTopX, nRightTopY);
    KGLOG_PROCESS_ERROR(pRightTop);

	bResult = true;
Exit0:
	return bResult;
}

void KScene::ApplyObstacle(KSceneObject* pObj, BOOL bApply)
{
	BOOL bRetCode = false;
	KCellEx* pLeftBottom = NULL;
    KCellEx* pRightTop = NULL;

	KGLOG_PROCESS_ERROR(pObj);

    m_PathFinder.UnApplyObstacle(pObj->m_dwID, pObj->m_bFullObstacle);

    if (bApply)
    {
        bRetCode = GetCellExRange(pObj, pLeftBottom, pRightTop);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = m_PathFinder.ApplyObstacle(pObj->m_dwID, pLeftBottom, pRightTop, pObj->m_bFullObstacle);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

Exit0:
	return;
}

KSceneObject* KScene::CreateGold(int nTemplateID, int nX, int nY, int nZ, int nMoney)
{
    BOOL    bResult        = false;
    BOOL    bRetCode       = false;
    KSceneObject* pObj     = NULL;
    KSceneObjectTemplate*   pTemplate = NULL;

    MAKE_IN_RANGE(nX, 0, GetLength() - 1);
    MAKE_IN_RANGE(nY, 0, GetWidth() - 1);

    assert(m_pSceneObjMgr);
    pTemplate = g_pSO3World->m_Settings.m_ObjTemplate.GetTemplate(nTemplateID);
    KGLOG_PROCESS_ERROR(pTemplate);
    KGLOG_PROCESS_ERROR(pTemplate->m_nType == sotGold);

    pObj = m_pSceneObjMgr->AddByTemplate(nTemplateID);
    KGLOG_PROCESS_ERROR(pObj);

    pObj->SetX(nX);
    pObj->SetY(nY);
    pObj->SetZ(nZ);

    if (pObj->m_pTemplate && pObj->m_pTemplate->m_Obstacle)
    {
        ApplyObstacle(pObj, true);    
    }
    
    ((KGold*)pObj)->SetMoney(nMoney);

    g_PlayerServer.DoSyncSceneObject(pObj);

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (pObj)
        {
            m_pSceneObjMgr->Delete(pObj->m_dwID);
            pObj = NULL;
        }
    }
    return pObj;
}

KSceneObject* KScene::CreateSceneObject(int nTemplateID, int nX, int nY, int nZ)
{
    BOOL    bResult        = false;
    BOOL    bRetCode       = false;
    KSceneObject* pObj     = NULL;

    MAKE_IN_RANGE(nX, 0, GetLength() - 1);
    MAKE_IN_RANGE(nY, 0, GetWidth() - 1);

    assert(m_pSceneObjMgr);

    pObj = m_pSceneObjMgr->AddByTemplate(nTemplateID);
    KGLOG_PROCESS_ERROR(pObj);

    pObj->SetX(nX);
    pObj->SetY(nY);
    pObj->SetZ(nZ);
    
    if (pObj->m_pTemplate && pObj->m_pTemplate->m_Obstacle)
    {
        ApplyObstacle(pObj, true);    
    }

    g_PlayerServer.DoSyncSceneObject(pObj);

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (pObj)
        {
            m_pSceneObjMgr->Delete(pObj->m_dwID);
            pObj = NULL;
        }
    }
    return pObj;
}

void KScene::RemoveSceneObject(DWORD dwObjID)
{
    KSceneObject*   pObj        = NULL;

    pObj = m_pSceneObjMgr->Get(dwObjID);
    assert(pObj);

    pObj->SetDeleteFlag();
    g_PlayerServer.DoRemoveSceneObj(m_dwID, dwObjID);

Exit0:
    return;
}

void KScene::RemoveSceneObject(KSceneObject* pObj)
{
    assert(pObj);

    pObj->SetDeleteFlag();
    g_PlayerServer.DoRemoveSceneObj(m_dwID, pObj->m_dwID);

Exit0:
    return;
}

BOOL KScene::ApplyAllObstacle()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	for(KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();

        if (!pObj)
            continue;        

        if (!pObj->m_pTemplate)
            continue;
        if (!pObj->m_pTemplate->m_Obstacle)
            continue;
        
        ApplyObstacle(pObj, true);        
    }

	bResult = true;
Exit0:
	return bResult;
}

KBasketSocket* KScene::GetSkillSlamBallTargetBasketSocketEx(DWORD dwSlamBallSkillID, KTWO_DIRECTION eShooterFaceDir)
{
    KBasketSocket* pResult = NULL;
    KBasketSocket* pBasketSocket = NULL;
    KTWO_DIRECTION eDirFilter = csdRight;
    KSlamBallSkill* pSlamBallSkill = NULL;

    KGLOG_PROCESS_ERROR(dwSlamBallSkillID);

    pSlamBallSkill = g_pSO3World->m_Settings.m_SlamBallSkillSettings.Get(dwSlamBallSkillID);
    KGLOG_PROCESS_ERROR(pSlamBallSkill);

    if (eShooterFaceDir == csdRight)
        eDirFilter = csdLeft; 

    KGLOG_PROCESS_ERROR(pSlamBallSkill->nHighestTargetBasket > 0 && pSlamBallSkill->nHighestTargetBasket <= 3);

    for (int i = pSlamBallSkill->nHighestTargetBasket * 2; i > 0; --i)
    {
        pBasketSocket = GetBasketSocket(eDirFilter, (KBasketSocketFloor)i);
        if (pBasketSocket->m_pBasket)
        {
            pResult = pBasketSocket;
            break;
        }
    }
    
Exit0:
    return pResult;
}

DWORD KScene::GetRandomOne(KVEC_DWORD& vecData)
{
    DWORD dwResult = 0;
    size_t uSize = 0;
    size_t uIndex = 0;

    uSize = vecData.size();
    KG_PROCESS_ERROR(uSize);
    
    uIndex = g_Random(uSize);
    KGLOG_PROCESS_ERROR(uIndex >= 0 && uIndex < uSize);

    dwResult = vecData[uIndex];
Exit0:
    return dwResult;
}

void KScene::BroadcastEvent(int nSide, int nEvent, int nParam1/*=0*/, int nParam2/*=0*/, int nParam3/*=0*/, int nParam4/*=0*/, int nParam5/*=0*/, int nParam6/*=0*/)
{
    BOOL bRetCode = false;
    KPlayer* pPlayer = NULL;

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pSceneObj = it.GetValue();
        bRetCode = pSceneObj->Is(sotHero);
        if (!bRetCode)
            continue;

        KHero* pHero = (KHero*)pSceneObj;
        bRetCode = pHero->IsMainHero();
        if (!bRetCode)
            continue;

        if (pHero->m_nSide != nSide)
            continue;        

        pPlayer = pHero->GetOwner();
        if (!pPlayer)
            continue;

        pPlayer->OnEvent(nEvent, nParam1, nParam2, nParam3, nParam4, nParam5, nParam6);
    }
}

int KScene::GetMaxMissionNpcAILevel()
{
    int nMaxMissionNpcAILevel = 0;
    BOOL bRetCode = false;

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pSceneObj = it.GetValue();
        bRetCode = pSceneObj->Is(sotHero);
        if (!bRetCode)
            continue;

        KHero* pHero = (KHero*)pSceneObj;
        if (!pHero->m_pNpcTemplate)
            continue;

        nMaxMissionNpcAILevel = MAX(nMaxMissionNpcAILevel, pHero->m_pNpcTemplate->nMaxAILevel);     
    }

    return nMaxMissionNpcAILevel;
}

KHero* KScene::GetHeroAt(int nSide, int nPos)
{
    KHero* pResult = NULL;
    BOOL bRetCode = false;

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pSceneObj = it.GetValue();
        bRetCode = pSceneObj->Is(sotHero);
        if (!bRetCode)
            continue;

        KHero* pHero = (KHero*)pSceneObj;
        if (pHero->m_nSide != nSide || pHero->m_nPos != nPos)
            continue;

        pResult = pHero;
        break;
    }
    
    return pResult;
}

void KScene::CheckFirstPvP(KPlayer* pPlayer, KPlayer* pOtherPlayer)
{
    KG_PROCESS_ERROR(pPlayer && pOtherPlayer);
     
    if (pPlayer->m_bFirstPvP)
    {
        pPlayer->m_bFirstPvP = false;
        g_PlayerServer.DoFirstPvPNotify(pPlayer->m_nConnIndex, pOtherPlayer->m_dwID, pOtherPlayer->m_szName);     
    }

Exit0:
    return;
}

void KScene::FireAIEventToSideInRange(int nSide, KAI_EVENT eEvent, const KBODY& cRange)
{
    BOOL bRetCode = false;
    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pSceneObj = it.GetValue();
        bRetCode = pSceneObj->Is(sotHero);
        if (!bRetCode)
            continue;

        KHero* pHero = (KHero*)pSceneObj;
        if (pHero->m_nSide != nSide)
            continue;

        if (!pHero->m_bAiMode)
            continue;

        bRetCode = g_IsCollide(cRange, pHero->GetBody());
        if (!bRetCode)
            continue;

        pHero->FireAIEvent(eEvent, 0, 0);
    }
}

KBasketSocket* KScene::GetSlamBallTargetSocket(DWORD dwSlamBallSkillID, KTWO_DIRECTION eShooterDir)
{
    KBasketSocket* pResult = NULL;
    KSlamBallSkill* pSkill = NULL;
    KTWO_DIRECTION eDirFilter = csdLeft;
    int nHeighestTargetFloor = bfInvalid;
    
    if (eShooterDir == csdLeft)
        eDirFilter = csdRight;
    
    pSkill = g_pSO3World->m_Settings.m_SlamBallSkillSettings.Get(dwSlamBallSkillID);
    KGLOG_PROCESS_ERROR(pSkill);

    nHeighestTargetFloor = pSkill->nHighestTargetBasket * 2;

    for (KObjEnumerator it = GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pSceneObj = it.GetValue();
        if (!pSceneObj->Is(sotBasketSocket))
            continue;
        
        KBasketSocket* pBasketSocket = (KBasketSocket*)pSceneObj;
        if (pBasketSocket->IsEmpty())
            continue;
        
        if (pBasketSocket->m_eDir != eDirFilter)
            continue;

        if (pBasketSocket->m_eFloor > nHeighestTargetFloor)
            continue;

        if (pResult && pBasketSocket->m_eFloor <= pResult->m_eFloor)
            continue;

        pResult = pBasketSocket;
    }

Exit0:
    return pResult;
}
