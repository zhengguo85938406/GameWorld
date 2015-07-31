////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KAIManager.cpp 
//  Version     : 1.0
//  Creator     : Chen Jie, zhaochunfeng
//  Comment     : 管理各种AI类型数据
//
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "KAIAction.h"
#include "KAIManager.h"
#include "KSO3World.h"
#include "KAILogic.h"
#include "KMath.h"

static const DWORD DEFAULT_AI_GROUP_ID = 0;

BOOL KAIManager::Init()
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    IIniFile*   piIniFile   = NULL;

    RegisterActionFunctions();

    KGLogPrintf(KGLOG_INFO, "[AI] loading ... ...");

    bRetCode = LoadAITabFile();
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLogPrintf(KGLOG_INFO, "[AI] %u AI loaded !", m_AITable.size());

    memset(m_ActionRunTimeStatisticsArray, 0, sizeof(m_ActionRunTimeStatisticsArray));
    m_NpcAIRunTimeStatisticsMap.clear();

    m_bLogAIRuntimeStat = 0;

    piIniFile = g_OpenIniFile(GS_SETTINGS_FILENAME);
    KGLOG_PROCESS_ERROR(piIniFile);

    bRetCode = piIniFile->GetInteger("AI", "LogAIRuntimeStat", 0, &m_bLogAIRuntimeStat);
    //KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piIniFile);
    return bResult;
}

BOOL AIRunTimeStatisticsComp(const std::pair<DWORD, KAI_RUNTIME_STATISTICS_NODE>& crLeft, const std::pair<DWORD, KAI_RUNTIME_STATISTICS_NODE>& crRight)
{
    if (crLeft.second.ullTime == crRight.second.ullTime)
        return crLeft.second.uCount < crRight.second.uCount;

    return crLeft.second.ullTime > crRight.second.ullTime;
}

BOOL KAIManager::LogAIRuntimeStat()
{
    BOOL        bResult         = false;
    time_t      nTime           = g_pSO3World->m_nCurrentTime;
    FILE*       pNpcStatFile    = NULL;
    FILE*       pActionStatFile = NULL;
    char        szFileName[_MAX_PATH];
    struct tm   tmNow; 
    std::map<DWORD, KAI_RUNTIME_STATISTICS_NODE>::iterator  it;
    std::map<DWORD, KAI_RUNTIME_STATISTICS_NODE>::iterator  itEnd;

    localtime_r(&nTime, &tmNow);

    if (!pNpcStatFile)
    {
        snprintf(
            szFileName, sizeof(szFileName), 
            "AIRuntimeStat/%d%2.2d%2.2d-%2.2d%2.2d%2.2d_Npc.tab",
            tmNow.tm_year + 1900,
            tmNow.tm_mon + 1,
            tmNow.tm_mday,
            tmNow.tm_hour,
            tmNow.tm_min,
            tmNow.tm_sec
        );
        szFileName[sizeof(szFileName) - 1] = '\0';

        KG_mkdir("AIRuntimeStat");

        pNpcStatFile = fopen(szFileName, "w");
        KG_PROCESS_ERROR(pNpcStatFile);

        fprintf(pNpcStatFile, "NpcTemp\tTime\tCount\t\n");
    }

    for (it = m_NpcAIRunTimeStatisticsMap.begin(), itEnd = m_NpcAIRunTimeStatisticsMap.end(); it != itEnd; ++it)
        fprintf(pNpcStatFile, "%u\t%llu\t%u\n", it->first, it->second.ullTime, it->second.uCount);

    if (!pActionStatFile)
    {
        snprintf(
            szFileName, sizeof(szFileName), 
            "AIRuntimeStat/%d%2.2d%2.2d-%2.2d%2.2d%2.2d_Action.tab",
            tmNow.tm_year + 1900,
            tmNow.tm_mon + 1,
            tmNow.tm_mday,
            tmNow.tm_hour,
            tmNow.tm_min,
            tmNow.tm_sec
        );
        szFileName[sizeof(szFileName) - 1] = '\0';

        KG_mkdir("AIRuntimeStat");

        pActionStatFile = fopen(szFileName, "w");
        KG_PROCESS_ERROR(pActionStatFile);

        fprintf(pActionStatFile, "Action\tTime\tCount\t\n");
    }

    for (int i = 0; i < sizeof(m_ActionRunTimeStatisticsArray) / sizeof(m_ActionRunTimeStatisticsArray[0]); i++)
        fprintf(pActionStatFile, "%d\t%llu\t%u\n", i, m_ActionRunTimeStatisticsArray[i].ullTime, m_ActionRunTimeStatisticsArray[i].uCount);

    bResult = true;
Exit0:
    if (pNpcStatFile)
    {
        fclose(pNpcStatFile);
        pNpcStatFile = NULL;
    }

    if (pActionStatFile)
    {
        fclose(pActionStatFile);
        pActionStatFile = NULL;
    }

    return bResult;
}

void KAIManager::UnInit()
{
    for (KAI_TABLE::iterator it = m_AITable.begin(); it != m_AITable.end(); ++it)
    {
        if (it->second.pLogic)
        {
            KMemory::Delete(it->second.pLogic);
            it->second.pLogic = NULL;
        }
    }

    m_AITable.clear();

    if (m_bLogAIRuntimeStat)
        LogAIRuntimeStat();
}

BOOL KAIManager::LoadAITabFile()
{
    BOOL        bResult                 = false;
    int         nRetCode                = false;
    ITabFile*   piAITabFile             = NULL;
    char        szFilePath[]            = SETTING_DIR"/AIType.tab";
    int         nHeight                 = 0;
    char        szColumnName[MAX_PATH]  = "";
    BOOL        bAILevelValid           = false;
    uint64_t    u64Key                  = 0;
    std::pair<KAI_TABLE::iterator, bool> RetPair;

    piAITabFile = g_OpenTabFile(szFilePath);
    if (!piAITabFile)
    {
        KGLogPrintf(KGLOG_ERR, "[AI] Unable to open table file \"%s\"\n", szFilePath);
        goto Exit0;
    }

    nHeight = piAITabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);

    m_nMaxAILevel = 0;
    for (int nRow = 2; nRow <= nHeight; nRow++)
    {
        int                 nAIType         = 0;
        DWORD               dwScriptID      = 0;
        char                szScriptPath[MAX_PATH];
        KAIInfo             AIInfo;

        nRetCode = piAITabFile->GetInteger(nRow, "AIType", 0, &nAIType);
        KGLOG_PROCESS_ERROR(nRetCode);
        KGLOG_PROCESS_ERROR(nAIType >= 0);

        nRetCode = piAITabFile->GetString(nRow, "ScriptFile", "", szScriptPath, sizeof(szScriptPath));
        KGLOG_PROCESS_ERROR(nRetCode);

        dwScriptID = g_FileNameHash(szScriptPath);
        KGLOG_PROCESS_ERROR(dwScriptID);
        AIInfo.dwScriptID = dwScriptID;

        for (int i = 0; i < countof(AIInfo.nAIOP); ++i)
        {
            sprintf(szColumnName, "AIOperation%02d", i + 1);
            nRetCode = piAITabFile->GetInteger(nRow, szColumnName, 0, &AIInfo.nAIOP[i]);
            KGLOG_PROCESS_ERROR(nRetCode);
        }
       
        nRetCode = piAITabFile->GetInteger(nRow, "AILevelValid", 0, &bAILevelValid);
        KGLOG_PROCESS_ERROR(nRetCode > 0);
        AIInfo.bAILevelValid    = bAILevelValid;

        AIInfo.nAILevel         = 0;
        AIInfo.dwAIGroupID      = 0;
        if (bAILevelValid)
        {
            nRetCode = piAITabFile->GetInteger(nRow, "AILevel", 0, &AIInfo.nAILevel);
            KGLOG_PROCESS_ERROR(nRetCode > 0);

            nRetCode = piAITabFile->GetInteger(nRow, "AIGroupID", DEFAULT_AI_GROUP_ID, (int*)&AIInfo.dwAIGroupID);
            KGLOG_PROCESS_ERROR(nRetCode);

            u64Key = MAKE_INT64(AIInfo.dwAIGroupID, AIInfo.nAILevel);
            m_AILevelTable[u64Key].push_back(nAIType);

            if (AIInfo.nAILevel > m_nMaxAILevel)
                m_nMaxAILevel = AIInfo.nAILevel;
        }

        AIInfo.pLogic           = NULL;

        m_AITable[nAIType] = AIInfo;
    }

    {
        int nCount = (int)m_AITable.size();
        int nIndex = 1;
        for (KAI_TABLE::iterator it = m_AITable.begin();it != m_AITable.end(); ++it, ++nIndex)
        {
            #if (defined(_MSC_VER) || defined(__ICL))   //WINDOWS
            cprintf("******************************>: Setup AI scripts : %d/%d\r", nIndex, nCount);
            #endif
            it->second.pLogic   = CreateAI(it->first, it->second.dwScriptID);
        }
        
        #if (defined(_MSC_VER) || defined(__ICL))   //WINDOWS
        cprintf("\n");
        #endif
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_AITable.clear();
    }
    KG_COM_RELEASE(piAITabFile);
    return bResult;
}

KAILogic* KAIManager::GetAILogic(int nAIType)
{
    KAILogic*           pLogic  = NULL;
    KAIInfo*            pInfo   = NULL;
    KAI_TABLE::iterator it      = m_AITable.find(nAIType);

    KGLOG_PROCESS_ERROR(it != m_AITable.end());

    pInfo = &it->second;

    if (!pInfo->pLogic)
    {
        pInfo->pLogic = CreateAI(nAIType, pInfo->dwScriptID);
    }

    pLogic = pInfo->pLogic;
Exit0:
    return pLogic;
}

KAI_ACTION_FUNC KAIManager::GetActionFunction(int nKey)
{
    if (nKey > eakInvalid && nKey < eakTotal)
    {
        return m_ActionFunctionTable[nKey];
    }

    return NULL;
}

KAILogic* KAIManager::CreateAI(int nType, DWORD dwScriptID)
{
    KAILogic* pResult       = NULL;
    BOOL      bRetCode      = false;
    KAILogic* pAI           = NULL;

    pAI = KMEMORY_NEW(KAILogic);
    KGLOG_PROCESS_ERROR(pAI);

    bRetCode = pAI->Setup(nType, dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pResult = pAI;
Exit0:
    if (!pResult)
    {
        if (pAI)
        {
            KMemory::Delete(pAI);
            pAI = NULL;
        }

        KGLogPrintf(KGLOG_ERR, "[AI] Setup AI failed, AIType: %d", nType);
    }
    return pResult;
}

BOOL KAIManager::GetRandomAIType(DWORD dwAIGroupID, int nAILevel, int& nAIType)
{
	BOOL    bResult     = false;
	BOOL    bRetCode    = false;	
    size_t  uIndex      = 0;
    int     nMaxAILevel = 0;
    uint64_t uKey       = 0;
    KAI_LEVEL_TABLE::iterator it;

    nMaxAILevel = GetMaxAILevel();
    KGLOG_PROCESS_ERROR(nMaxAILevel > 0);

    MakeInRange(nAILevel, 1, nMaxAILevel);

    uKey = MAKE_INT64(dwAIGroupID, nAILevel);

    it = m_AILevelTable.find(uKey);
    if (it == m_AILevelTable.end())
    {
        uKey = MAKE_INT64(DEFAULT_AI_GROUP_ID, nAILevel);
        it = m_AILevelTable.find(uKey);   
    }   
    KGLOG_PROCESS_ERROR(it != m_AILevelTable.end());

    KGLOG_PROCESS_ERROR(it->second.size());

    uIndex = g_Random(it->second.size());
    nAIType = it->second[uIndex];

	bResult = true;
Exit0:
	return bResult;
}

int KAIManager::GetMaxAILevel()
{
    return m_nMaxAILevel;
}

BOOL KAIManager::GetAILevel(int nAIType, int& nAILevel)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KAI_TABLE::iterator it;

    it = m_AITable.find(nAIType);
    KG_PROCESS_ERROR(it != m_AITable.end());

    nAILevel = it->second.nAILevel;

	bResult = true;
Exit0:
	return bResult;
}
