#include "stdafx.h"
#include "KDramaPlayer.h"
#include "KSO3World.h"
#include "KScene.h"
#include "KPlayerServer.h"

KDramaPlayer::KDramaPlayer()
{
    m_pDramaFrameInfo = NULL;
    m_nNextFrame = 0;
    m_dwScriptID = 0;
    m_pScene = NULL;
    m_nLoop = 0;
    m_bPause = false;
    m_nDramaNumber = 0;
}

KDramaPlayer::~KDramaPlayer()
{

}

BOOL KDramaPlayer::Init(KScene* pScene)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

    m_pScene = pScene;

	bResult = true;
//Exit0:
	return bResult;
}

void KDramaPlayer::UnInit()
{
}

void KDramaPlayer::Activate()
{
    if (!m_pDramaFrameInfo)
        return;

    if (m_bPause)
        return;

    if (m_nNextFrame == -1) // 处理暂停后的结束
    {
        StopDrama();
        return;
    }

    ++m_nLoop;

    if (m_nNextFrame != m_nLoop)
        return;

    KMAP_DRAMAFRAMEINFO::iterator itLower = m_pDramaFrameInfo->lower_bound(m_nNextFrame);
    KMAP_DRAMAFRAMEINFO::iterator itUpper = m_pDramaFrameInfo->upper_bound(m_nNextFrame);

    KGLOG_PROCESS_ERROR(itLower != m_pDramaFrameInfo->end());

    while (itLower != itUpper)
    {
        KDramaFrameInfo* pFrameInfo = &itLower->second;
        if (pFrameInfo->szFuncName[0] != '\0')
        {
            int nTopIndex = 0;
            unsigned nOldSeed = g_GetRandomSeed();
            g_RandomSeed(m_nDramaNumber);

            //LogInfo("Drama time:(%d, %d), fun:%s", m_nLoop / GAME_FPS, m_nLoop % GAME_FPS, pFrameInfo->szFuncName);

            g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
            g_pSO3World->m_ScriptManager.Push(m_pScene);
            g_pSO3World->m_ScriptManager.Push(m_nDramaNumber);
            g_pSO3World->m_ScriptManager.Call(m_dwScriptID, pFrameInfo->szFuncName, 0);
            g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

            g_RandomSeed(nOldSeed);
        }

        if (pFrameInfo->nDialogID)
        {
            if (pFrameInfo->bPauseOnDialog)
                m_bPause = true; // 对话的时候剧情暂停，玩家关闭对话后剧情再恢复	
			
			// sync client
			g_PlayerServer.DoStartDialog(m_pScene, pFrameInfo->nDialogID, pFrameInfo->bPauseOnDialog);
        }

        ++itLower;
    }

    if (itUpper == m_pDramaFrameInfo->end()) // 已经结束
    {
        m_nNextFrame = -1;
        goto Exit0;
    }

    m_nNextFrame = itUpper->first;

Exit0:
    return;
}

BOOL KDramaPlayer::StartDrama(int nDramaID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KDramaConfig* pDramaConfig = NULL;
    KMAP_DRAMAFRAMEINFO* pDramaFrameInfo = NULL;
    int nTopIndex = 0;

    LogInfo("Start Drama %d", nDramaID);

    KGLOG_PROCESS_ERROR(m_pDramaFrameInfo == NULL);

    pDramaConfig = g_pSO3World->m_DramaMgr.GetDramaConfig(nDramaID);
    KGLOG_PROCESS_ERROR(pDramaConfig);
    KGLOG_PROCESS_ERROR(pDramaConfig->nMapID == (int)m_pScene->m_dwMapID);

    pDramaFrameInfo = g_pSO3World->m_DramaMgr.GetDrama(nDramaID);
    KGLOG_PROCESS_ERROR(pDramaFrameInfo);
    KGLOG_PROCESS_ERROR(!pDramaFrameInfo->empty());

    m_dwScriptID = g_pSO3World->m_DramaMgr.GetDramaScriptID(nDramaID);
    KGLOG_PROCESS_ERROR(m_dwScriptID);

    m_nDramaNumber = g_pSO3World->m_DramaMgr.GetNextDramaNumber();

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(m_pScene);
    g_pSO3World->m_ScriptManager.Push(m_nDramaNumber);
    g_pSO3World->m_ScriptManager.Call(m_dwScriptID, "OnStartDrama", 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);
    
    m_pDramaFrameInfo = pDramaFrameInfo;
    m_nNextFrame = pDramaFrameInfo->begin()->first;
    m_nLoop = 0;
    m_bPause = false;

    g_PlayerServer.DoStartDrama(m_pScene, nDramaID);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KDramaPlayer::StopDrama()
{
    int nTopIndex = 0;
    int nNumber = m_nDramaNumber;

    m_pDramaFrameInfo = NULL;
    m_nNextFrame = 0;
    m_nLoop = 0;
    m_bPause = false;
    m_nDramaNumber = 0;

    g_PlayerServer.DoStopDrama(m_pScene);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(m_pScene);
    g_pSO3World->m_ScriptManager.Push(nNumber);
    g_pSO3World->m_ScriptManager.Call(m_dwScriptID, "OnStopDrama", 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

    return true;
}

BOOL KDramaPlayer::ResumeDrama()
{
    m_bPause = false;
    return true;
}

