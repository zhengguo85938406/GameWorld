#include "stdafx.h"
#include "KDramaManager.h"
#include "KSO3World.h"

BOOL KDramaConfig::CheckData()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int nTopIndex = 0;

    dwScriptID = g_FileNameHash(szScriptName);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(nID);
    g_pSO3World->m_ScriptManager.Call(dwScriptID, "OnInitDramaConfig", 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

	bResult = true;
Exit0:
	return bResult;
}

//////////////////////////////////////////////////////////////////////////

KDramaManager::KDramaManager()
{
    m_nNextDramaNumber = 0;
}

KDramaManager::~KDramaManager()
{
}

BOOL KDramaManager::LoadDrama()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = m_DramaConfig.Load("Drama.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

// 如果有正在播放的剧情，调用此函数会导致宕机
BOOL KDramaManager::ReloadDrama()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    m_DramaConfig.m_Data.clear();
    m_mapDramas.clear();

    bRetCode = LoadDrama();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

KDramaConfig* KDramaManager::GetDramaConfig(int nID)
{
    return m_DramaConfig.GetByID(nID);
}

KMAP_DRAMAFRAMEINFO* KDramaManager::GetDrama(int nID)
{
    KMAP_DRAMAFRAMEINFO* pResult = NULL;
    KMAP_DRAMA::iterator it = m_mapDramas.find(nID);
    KGLOG_PROCESS_ERROR(it != m_mapDramas.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

BOOL KDramaManager::AddDramaFrameInfo(int nDramaID, int nFrame, const char szFunName[], int nDialogID, bool bPauseOnDialog)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KDramaFrameInfo FrameInfo;
    KMAP_DRAMAFRAMEINFO* pDramaFrameInfo = NULL;
    KMAP_DRAMA::iterator it = m_mapDramas.find(nDramaID);
    if (it == m_mapDramas.end())
    {
        std::pair<KMAP_DRAMA::iterator, bool> InsRet;
        InsRet = m_mapDramas.insert(std::make_pair(nDramaID, KMAP_DRAMAFRAMEINFO()));
        KGLOG_PROCESS_ERROR(InsRet.second);
        it = InsRet.first;
    }

    assert(it != m_mapDramas.end());

    pDramaFrameInfo = &it->second;

    FrameInfo.nFrame = nFrame;
    FrameInfo.nDialogID = nDialogID;
    FrameInfo.bPauseOnDialog = bPauseOnDialog;

    strncpy(FrameInfo.szFuncName, szFunName, countof(FrameInfo.szFuncName));
    FrameInfo.szFuncName[countof(FrameInfo.szFuncName) - 1] = '\0';

    pDramaFrameInfo->insert(std::make_pair(FrameInfo.nFrame, FrameInfo));

	bResult = true;
Exit0:
	return bResult;
}

uint32_t KDramaManager::GetDramaScriptID(int nID)
{
    uint32_t dwResult = 0;
    KDramaConfig* pConfig = NULL;
    pConfig = m_DramaConfig.GetByID(nID);
    KGLOG_PROCESS_ERROR(pConfig);

    dwResult = pConfig->dwScriptID;
Exit0:
    return dwResult;
}
