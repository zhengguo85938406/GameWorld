#include "stdafx.h"
#include "KSceneLoader.h"
#include "KScene.h"
#include "KSO3World.h"

BOOL KSceneLoader::Init()
{
    BOOL bResult  = false;
    int  nRetCode = false;

    nRetCode = m_Thread.Create(WorkThreadFunction, this);
    KGLOG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KSceneLoader::UnInit()
{
    m_bQuit = true;
    m_Thread.Destroy();
}

BOOL KSceneLoader::PostLoadingScene(DWORD dwSceneID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = m_LoadingList.Push(dwSceneID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

DWORD KSceneLoader::GetCompletedScene()
{
    DWORD   dwSceneID       = ERROR_ID;

    m_LoadedList.Pop(&dwSceneID);

    return dwSceneID;
}

void KSceneLoader::WorkThreadFunction(void *pvParam)
{
    KSceneLoader* pThis = (KSceneLoader*)pvParam;

    assert(pThis);

    pThis->ThreadFunction();
}

void KSceneLoader::ThreadFunction()
{
    BOOL    bRetCode    = false;
    DWORD   dwSceneID   = ERROR_ID;
    KScene* pScene      = NULL;

    while(!m_bQuit)
	{
        KGThread_Sleep(10);

		bRetCode = m_LoadingList.Pop(&dwSceneID);
		if (!bRetCode)
			continue;
        
        pScene = g_pSO3World->m_SceneSet.GetObj(dwSceneID);
        if (!pScene)
        {
            KGLogPrintf(KGLOG_DEBUG,"Error SceneID(%d) at line %d in %s\n", dwSceneID, __LINE__, KG_FUNCTION);
            continue;
        }

        KGLOG_PROCESS_ERROR(pScene->m_eSceneState == ssLoading);

        bRetCode = pScene->ServerLoad();
        
        pScene->m_eSceneState = bRetCode ? ssCompleteLoading : ssFailedLoading;

        m_LoadedList.Push(dwSceneID);
    }

Exit0:
    return;
}
