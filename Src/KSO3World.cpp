#include "stdafx.h"
#include <time.h>

#ifdef WINDOWS
#include <conio.h>
#endif

#include "KMath.h"
#include "Engine/KMemory.h"
#include "KSO3World.h"
#include "KScene.h"
#include "KTerrainData.h"
#include "KHero.h"
#include "Engine/KG_Time.h"
#include "KRelayClient.h"
#include "KPlayerServer.h"
#include "KAttrModifier.h"
#include "KPlayer.h"
#include "KAIManager.h"
#include "KMailDef.h"
#include "Engine/KTextFilter.h"

#define CIRCLE_LIST_SIZE				256

KSO3World* g_pSO3World = NULL;

KSO3World::KSO3World()
{
    m_piTextFilter      = NULL;
    m_piNameFilter      = NULL;
    m_bStatLogFlag      = false;
    m_dwR2SLogTime      = 20;
    m_dwC2SLogTime      = 20;
    m_dwStatLogTime		= 50;
}

KSO3World::~KSO3World(void)
{
}

BOOL KSO3World::Init()
{
    BOOL        bResult                     = false;
    BOOL        bRetCode                    = false;
    int         nRetCode                    = 0;
    BOOL        bSettingsInitFlag           = false;
    BOOL        bSceneLoaderInitFlag        = false;
    BOOL        bSkillManagerInitFlag       = false;
    BOOL        bBuffManagerInitFlag        = false;
    BOOL        bScriptManagerInitFlag      = false;
    BOOL        bBulletMgrInitFlag          = false;
    DWORD       dwRandSeed                  = 0;
    BOOL        bItemHouseInitFlag          = false;
    BOOL        bAIManageInitFlag           = false;
    BOOL        bRelationMgrInitFlag        = false;
    BOOL        bAwardInitFlag              = false;
    BOOL        bShopInitFlag               = false;
    BOOL        bStatDataServer             = false;
    BOOL        bRoomCacheInit              = false;
    BOOL        bMissionMgrInit             = false;
    BOOL        bActivityCenterInitFlag     = false;
    IIniFile*   piIniFile                   = NULL;
    char*       pszRetCWD                   = NULL;
    char        szCWD[MAX_PATH];

    m_nBaseTime             = 0;
    m_dwStartTime           = 0;
    m_nStartLoop            = 0;
    m_nGameLoop             = 0;
    m_nServerIndexInLS      = ERROR_ID; 
    m_nServerIndexInGC      = ERROR_ID;

    m_nCurrentTime  = time(NULL);
    m_dwStartTime   = KG_GetTickCount();

    g_RandomSeed((unsigned int)m_nCurrentTime);
    srand((unsigned int)m_nCurrentTime);

    bRetCode = m_Settings.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bSettingsInitFlag = true;

    InitAttributeFunctions();

    bRetCode = m_SceneLoader.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bSceneLoaderInitFlag = true;

    bRetCode = m_SkillManager.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bSkillManagerInitFlag = true;

    bRetCode = m_BuffManager.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bBuffManagerInitFlag = true;

    bRetCode = m_ScriptManager.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bScriptManagerInitFlag = true;

    bRetCode = m_BulletMgr.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bBulletMgrInitFlag = true;

    bRetCode = m_ItemHouse.Init(ITEM_SETTING_DIR);
    KGLOG_PROCESS_ERROR(bRetCode);
    bItemHouseInitFlag = true;

    bRetCode = m_AIManager.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bAIManageInitFlag = true;

    bRetCode = m_FellowshipMgr.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bRelationMgrInitFlag = true;

    m_piTextFilter = CreateTextFilterInterface(true);
    KGLOG_PROCESS_ERROR(m_piTextFilter);

    bRetCode = m_piTextFilter->LoadFilterFile(SETTING_DIR"/TextFilter.txt");
    KGLOG_PROCESS_ERROR(bRetCode);

    m_piNameFilter = CreateTextFilterInterface(true);
    KGLOG_PROCESS_ERROR(m_piNameFilter);

    bRetCode = m_piNameFilter->LoadFilterFile(SETTING_DIR"/NameFilter.txt");
    KGLOG_PROCESS_ERROR(bRetCode);


    bRetCode = m_AwardMgr.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bAwardInitFlag = true;

    bRetCode = m_ShopMgr.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bShopInitFlag = true;

    bRetCode = m_StatDataServer.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bStatDataServer = true;

    bRetCode = m_RoomCache.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bRoomCacheInit = true;

    bRetCode = m_MissionMgr.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bMissionMgrInit = true;

    bRetCode = m_ActivityCenter.Init(); // 这个需要在m_AwardMgr初始化之后
    KGLOG_PROCESS_ERROR(bRetCode);
    bActivityCenterInitFlag = true;

    bRetCode = m_DramaMgr.LoadDrama();
    KGLOG_PROCESS_ERROR(bRetCode);

    m_bRunFlag             = true;
    m_nIdleFrame           = 0;

    piIniFile = g_OpenIniFile(GS_SETTINGS_FILENAME);
    KGLOG_PROCESS_ERROR(piIniFile);

    piIniFile->GetInteger("Global", "StatLogFlag", 0, (int*)&m_bStatLogFlag);

    m_nMainStatID           = m_RTWorldMgr.Create("Main");
    m_nR2SStatID            = m_RTWorldMgr.Create("R2S");
    m_nC2SStatID            = m_RTWorldMgr.Create("C2S");
    m_nWorldStatID          = m_RTWorldMgr.Create("World");

    pszRetCWD = getcwd(szCWD, sizeof(szCWD));    
    KG_PROCESS_ERROR(pszRetCWD);

    bResult = true;
Exit0:
    if (!bResult)
    {   
        if (bActivityCenterInitFlag)
        {
            m_ActivityCenter.UnInit();
            bActivityCenterInitFlag = false;
        }


        if (bMissionMgrInit)
        {
            m_MissionMgr.UnInit();
            bMissionMgrInit = false;
        }

        if (bRoomCacheInit)
        {
            m_RoomCache.UnInit();
            bRoomCacheInit = false;
        }

        if (bStatDataServer)
        {
            m_StatDataServer.UnInit();
            bStatDataServer = false;
        }

        if (bShopInitFlag)
        {
            m_ShopMgr.UnInit();
            bShopInitFlag = false;
        }        

        if (bAwardInitFlag)
        {
            m_AwardMgr.UnInit();
            bAwardInitFlag = false;
        }

        KG_COM_RELEASE(m_piTextFilter);
        KG_COM_RELEASE(m_piNameFilter);

        if (bRelationMgrInitFlag)
        {
            m_FellowshipMgr.UnInit();
            bRelationMgrInitFlag = false;
        }

        if (bAIManageInitFlag)
        {
            m_AIManager.UnInit();
            bAIManageInitFlag = false;
        }

        if (bItemHouseInitFlag)
        {
            m_ItemHouse.UnInit();
            bItemHouseInitFlag = false;
        }        

        if (bBulletMgrInitFlag)
        {
            m_BulletMgr.UnInit();
            bBulletMgrInitFlag = false;
        }

        if (bScriptManagerInitFlag)
        {
            m_ScriptManager.UnInit();
            bScriptManagerInitFlag = false;
        }

        if (bBuffManagerInitFlag)
        {
            m_BuffManager.UnInit();
            bBuffManagerInitFlag = false;
        }

        if (bSkillManagerInitFlag)
        {
            m_SkillManager.UnInit();
            bSkillManagerInitFlag = false;
        }

        if (bSceneLoaderInitFlag)
        {
            m_SceneLoader.UnInit();
            bSceneLoaderInitFlag = false;
        }

        if (bSettingsInitFlag)
        {
            m_Settings.UnInit();
            bSettingsInitFlag = false;
        }
    }
    KG_COM_RELEASE(piIniFile);

    KGLogPrintf(KGLOG_INFO, "Load game settings ... ... [%s]", bResult ? "OK" : "Failed");

    return bResult;
}

void KSO3World::UnInit(void)
{
    BOOL                bRetCode         = false;
    KDeleteSceneFunc    DeleteSceneFunc;

    m_ActivityCenter.UnInit();
    m_MissionMgr.UnInit();
    m_RoomCache.UnInit();
    m_StatDataServer.UnInit();
    m_ShopMgr.UnInit();
    m_AwardMgr.UnInit();
    m_FellowshipMgr.UnInit();
    m_AIManager.UnInit();
    m_ItemHouse.UnInit();
    m_BulletMgr.UnInit();
    m_ScriptManager.UnInit();
    m_BuffManager.UnInit();
    m_SkillManager.UnInit();

    m_SceneSet.Traverse(DeleteSceneFunc);
    m_SceneLoader.UnInit();
    m_Settings.UnInit();
	
	KG_COM_RELEASE(m_piNameFilter);
	KG_COM_RELEASE(m_piTextFilter);
	
    return;
}

void KSO3World::Activate(void)
{
    KTraverseSceneFunc  TraverseSceneFunc;

    m_nGameLoop++;

    m_nCurrentTime = time(NULL);

    KTraversePlayerFunc TraversePlayerFunc;

    g_pSO3World->m_PlayerSet.Traverse(TraversePlayerFunc);

    ProcessLoadedScene();

    m_SceneSet.Traverse(TraverseSceneFunc);

    m_ScriptManager.Activate();
    m_StatDataServer.Activate();

    return;
}

void KSO3World::ProcessLoadedScene()
{
    BOOL    bRetCode            = false;
    DWORD   dwLoadedSceneID     = 0;
    KScene* pLoadedScene        = NULL;

    dwLoadedSceneID = m_SceneLoader.GetCompletedScene();
    KG_PROCESS_ERROR(dwLoadedSceneID != ERROR_ID);

    pLoadedScene = m_SceneSet.GetObj(dwLoadedSceneID);
    KGLOG_PROCESS_ERROR(pLoadedScene);

    if (pLoadedScene->m_eSceneState == ssCompleteLoading)
    {
        bRetCode = pLoadedScene->CompleteLoading();
        if (bRetCode)
        {
            pLoadedScene->m_eSceneState = ssWaitingClientLoading;
            pLoadedScene->m_nClientLoadingFrame = g_pSO3World->m_nGameLoop;

            pLoadedScene->CallInitScript();

            g_RelayClient.DoCreateMapRespond(pLoadedScene->m_dwMapID, pLoadedScene->m_nCopyIndex, true);
        }
        else
        {
            g_RelayClient.DoCreateMapRespond(pLoadedScene->m_dwMapID, pLoadedScene->m_nCopyIndex, false);			
            pLoadedScene->m_eSceneState = ssInvalid;
            g_pSO3World->DeleteScene(pLoadedScene);
        }
    }
    else
    {
        assert(pLoadedScene->m_eSceneState == ssFailedLoading);

        g_RelayClient.DoCreateMapRespond(pLoadedScene->m_dwMapID, pLoadedScene->m_nCopyIndex, false);			
        pLoadedScene->m_eSceneState = ssInvalid;
        g_pSO3World->DeleteScene(pLoadedScene);
    }

Exit0:
    return;
}

BOOL KSO3World::DeleteScene(KScene* pScene)
{
    BOOL    bResult  = false;

    pScene->UnInit();

    KMemory::Delete(pScene);
    pScene = NULL;

    bResult = true;
Exit0:
    return bResult;
}

KScene* KSO3World::GetScene(DWORD dwMapID, int nMapCopyIndex)
{
    KGetSceneFunc GetSceneFunc;

    GetSceneFunc.m_dwMapID          = dwMapID;
    GetSceneFunc.m_nMapCopyIndex    = nMapCopyIndex;
    GetSceneFunc.m_pScene           = NULL;

    g_pSO3World->m_SceneSet.Traverse(GetSceneFunc);

    return GetSceneFunc.m_pScene;
}

KPlayer* KSO3World::NewPlayer(DWORD dwPlayerID)
{
    KPlayer*    pResult         = NULL;
    BOOL        bRetCode        = false;
    KPlayer*    pPlayer         = NULL;
    BOOL        bRegisterFlag   = false;
    BOOL        bInitFlag       = false;

    assert(dwPlayerID != ERROR_ID);

    pPlayer = KMEMORY_NEW(KPlayer);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_PlayerSet.Register(pPlayer, dwPlayerID);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRegisterFlag = true;

    bRetCode = pPlayer->Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bInitFlag = true;

    pResult = pPlayer;
Exit0:
    if (pResult == NULL)
    {
        if (bInitFlag)
        {
            pPlayer->UnInit();
            bInitFlag = false;
        }

        if (bRegisterFlag)
        {
            g_pSO3World->m_PlayerSet.Unregister(pPlayer);
            bRegisterFlag = false;
        }

        if (pPlayer != NULL)
        {
            KMemory::Delete(pPlayer);
            pPlayer = NULL;
        }
    }
    return pResult;
}

BOOL KSO3World::DelPlayer(KPlayer* pPlayer)
{
    g_RelayClient.DoPlayerLeaveGS(pPlayer->m_dwID);
    m_FellowshipMgr.UnloadPlayerFellowship(pPlayer->m_dwID);

    pPlayer->RemoveHero();
    pPlayer->UnInit();

    assert(pPlayer->m_dwID != ERROR_ID);

    g_pSO3World->m_PlayerSet.Unregister(pPlayer);

    KMemory::Delete(pPlayer);
    pPlayer = NULL;

    return true;
}

BOOL KSO3World::FillMail(
    KMAIL_TYPE eMailType, const char szSenderName[], const char szTitle[], const char szText[], 
    int nTextLen, IItem** ppItem, int nItemSize, KMail* pMail, unsigned int& uMailLen
)
{
    BOOL                    bResult	                = false;
    BOOL                    bRetCode	            = false;
    IItem*                  piItem                  = NULL;
    BYTE*                   pbyOffset               = NULL;
    size_t                  uItemDataLen            = 0;
    T3DB::KPB_EQUI_DATA     EquiData;

    assert(pMail);
    KGLOG_PROCESS_ERROR(nTextLen <= g_pSO3World->m_Settings.m_ConstList.nMailTextSize);
 
    uMailLen = 0;
    pbyOffset = pMail->byData;
    uMailLen  = sizeof(KMail);

    pMail->bRead        = false;
    pMail->byType       = (BYTE)eMailType;
    pMail->nSendTime    = m_nCurrentTime;
    pMail->nRecvTime    = pMail->nSendTime;
    pMail->wTextLen     = (WORD)nTextLen;
    pMail->byItemType   = 0;
    pMail->wItemIndex   = 0;
    pMail->dwSenderID   = 0;
    pMail->dwMailID     = 0;
    pMail->nMoney       = 0;
    pMail->bCanAcquire  = false;
    pMail->nLockTime    = 0;

    strncpy(pMail->szTitle, szTitle, sizeof(pMail->szTitle));
    pMail->szTitle[sizeof(pMail->szTitle) - 1] = '\0';

    strncpy(pMail->szSenderName, szSenderName, sizeof(pMail->szSenderName));
    pMail->szSenderName[sizeof(pMail->szSenderName) - 1] = '\0';

    memcpy(pbyOffset, szText, nTextLen);
    //pbyOffset[nTextLen - 1] = '\0';
    pbyOffset += pMail->wTextLen;
    uMailLen += pMail->wTextLen;
    
    KG_PROCESS_ERROR(nItemSize <= KMAIL_MAX_ITEM_COUNT);
    for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
    {
        pMail->ItemDesc[i].bAcquired = false;
        pMail->ItemDesc[i].byDataLen = 0;
        pMail->ItemDesc[i].byQuality = 0;

        if (i >= nItemSize)
            continue;

        EquiData.Clear();
        piItem = ppItem[i];
        assert(piItem);
        if (pMail->wItemIndex == 0)
        {
            const KItemProperty* pItemProperty = piItem->GetProperty();
            pMail->wItemIndex = (WORD)pItemProperty->dwTabIndex;
            pMail->byItemType = (BYTE)pItemProperty->dwTabType;
        }
        bRetCode = piItem->GetProtoBufData(&EquiData);
        KGLOG_PROCESS_ERROR(bRetCode);

        uItemDataLen = EquiData.ByteSize();
        KG_PROCESS_ERROR(uItemDataLen <= MAX_EXTERNAL_PACKAGE_SIZE - uMailLen);

        EquiData.SerializeWithCachedSizesToArray((::google::protobuf::uint8*)pbyOffset);

        uMailLen  += uItemDataLen;
        pbyOffset += uItemDataLen;

        pMail->ItemDesc[i].byDataLen = (BYTE)uItemDataLen;
        pMail->ItemDesc[i].byQuality = (BYTE)piItem->GetQuality();
    }

    bResult	= true;
Exit0:
    return bResult;
}

struct KFindSpecifyPlayerTravers
{
    int         nPageIndex;
    KPlayer**   pResult;
    int         nCurIndex;
    int         nMaxCount;
    BOOL operator()(KPlayer* pPlayer)
    {
        BOOL bContinue = true;
        assert(pPlayer);

        switch(pPlayer->m_eGameStatus)
        {
        case gsInHall:
        case gsWaitForNewMapLoading:
        case gsPlaying:
        case gsSearchMap:
            break;
        default:
            goto Exit0;
        }

        ++nCurIndex;

        if (nCurIndex >= (nPageIndex - 1) * nMaxCount && nCurIndex < nPageIndex * nMaxCount)
        {
            *pResult++ = pPlayer;
        }
        else if(nCurIndex >= nPageIndex * nMaxCount)
        {
            bContinue = false;
        }

Exit0:       
        return bContinue;
    }
};

void KSO3World::FindPlayers(int nPageIndex, KPlayer* aPlayers[], int nCount)
{
    KFindSpecifyPlayerTravers cTraverse;
    cTraverse.nPageIndex    = nPageIndex;
    cTraverse.pResult       = aPlayers;
    cTraverse.nCurIndex     = -1;
    cTraverse.nMaxCount     = nCount;

    m_PlayerSet.Traverse(cTraverse);
}


BOOL KGetSceneFunc::operator()(KScene* pScene)
{   
    // CopyID为0表示任意Copy,只要MapID对即可
    if (pScene->m_dwMapID == m_dwMapID && (pScene->m_nCopyIndex == m_nMapCopyIndex || m_nMapCopyIndex == 0))
    {
        m_pScene = pScene;
        return false;
    }

    return true;
}

BOOL KDeleteSceneFunc::operator()(KScene* pScene)
{
    assert(pScene);

    pScene->UnInit();

    KMemory::Delete(pScene);
    pScene = NULL;

    return true;
}

BOOL KTraversePlayerFunc::operator()(KPlayer* pPlayer)
{
    assert(pPlayer);

    pPlayer->Activate();

    return true;
}

BOOL KTraverseSceneFunc::operator()(KScene* pScene)
{   
    assert(pScene);

    switch (pScene->m_eSceneState)
    {
    case ssWaitingClientLoading:
    case ssCountDown:
    case ssFighting:
        pScene->Activate(g_pSO3World->m_nGameLoop);
        break;
    }

    return true;
}

