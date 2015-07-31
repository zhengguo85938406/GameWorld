#include "stdafx.h"
#include "KScriptManager.h"
#include "KScriptFuncList.h"
#include "Engine/KG_Time.h"
#include "KSO3World.h"
#include "KHero.h"
#include "KPlayer.h"
#include "KScene.h"
#include "KGItemHouse.h"

#ifndef WINDOWS
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#else 
#include <io.h>
#include <direct.h>
#endif

extern KLuaConstList g_LuaServerConstList[];

KScriptManager::KScriptManager()
{
    m_piScript  = NULL;
    m_bStatFlag = false;
    m_bReload   = false;
    m_dwMainScriptID = g_FileNameHash(MAIN_SCRIPT);
}

BOOL KScriptManager::Init()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = Load();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = IsScriptExist(m_dwMainScriptID);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = IsFuncExist(m_dwMainScriptID, MAIN_SCRIPT_ACTIVATE_FUNCTION);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KScriptManager::UnInit()
{
    KG_COM_RELEASE(m_piScript);
}

void KScriptManager::Activate()
{
    if (m_bReload)
    {
        Load();
        m_bReload = false;
    }

    CallActivateScript();

    if (g_pSO3World->m_nGameLoop % (GAME_FPS * 8) == 0)
    {
        lua_gc(m_piScript->GetLuaState(), LUA_GCCOLLECT, 0);
    }
}

BOOL KScriptManager::ExecutePlayerScript(KPlayer* pPlayer, const char* pszScript)
{
    BOOL        bResult     = false;
	BOOL        bRetCode    = false;
	Lua_State*  L           = NULL;
    KHero*      pHero       = NULL;
    KScene*     pScene      = NULL;

	assert(pPlayer);
	assert(pszScript);
	assert(m_piScript);

	L = m_piScript->GetLuaState();
	KGLOG_PROCESS_ERROR(L);

	lua_pushstring(L, "player");
	pPlayer->LuaGetObj(L);
	lua_settable(L, LUA_GLOBALSINDEX);

    pHero = pPlayer->GetFightingHero();
    if (pHero)
    {
        lua_pushstring(L, "hero");
	    pHero->LuaGetObj(L);
	    lua_settable(L, LUA_GLOBALSINDEX);

        pScene = pHero->m_pScene;
        if (pScene)
        {
            lua_pushstring(L, "scene");
	        pScene->LuaGetObj(L);
	        lua_settable(L, LUA_GLOBALSINDEX);
        }
    }

	bRetCode = m_piScript->LoadFromBuffer(0, pPlayer->m_szName, pszScript, (DWORD)strlen(pszScript));
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    if (L)
    {
        lua_pushstring(L, "player");
        lua_pushnil(L);
        lua_settable(L, LUA_GLOBALSINDEX);
    }
	return bResult;
}

BOOL KScriptManager::ExecuteGMCScript(const char cszGmName[], const char cszScript[])
{
    BOOL        bResult     = false;
	BOOL        bRetCode    = false;
	Lua_State*  L           = NULL;
    size_t      uScriptLen  = 0;

    assert(cszGmName);
    assert(cszScript);
    assert(m_piScript);

	L = m_piScript->GetLuaState();
	KGLOG_PROCESS_ERROR(L);

    lua_pushstring(L, "gm_name");
    lua_pushstring(L, cszGmName);
    lua_settable(L, LUA_GLOBALSINDEX);

    uScriptLen = strlen(cszScript);
    KGLOG_PROCESS_ERROR(uScriptLen > 0);

	bRetCode = m_piScript->LoadFromBuffer(0, "_GMC_", cszScript, (DWORD)uScriptLen);
	KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
	return bResult;
}

void KScriptManager::SafeCallBegin(int* pIndex)
{
	assert(m_piScript);
	m_piScript->SafeCallBegin(pIndex);
}

void KScriptManager::SafeCallEnd(int nIndex)
{
	assert(m_piScript);
	m_piScript->SafeCallEnd(nIndex);
}

BOOL KScriptManager::GetValuesFromStack(const char * pszFormat , ...)
{
    BOOL bResult = false;

    assert(m_piScript);

    va_list vlist;
    va_start(vlist, pszFormat);

    bResult = m_piScript->GetValuesFromStack(pszFormat, vlist);

    va_end(vlist);

    return bResult;
}

BOOL KScriptManager::Call(DWORD dwScriptID, const char* pszFuncName, int nResults)
{
    BOOL        bResult  = false;
	BOOL        bRetCode = false;
    uint64_t    uTime    = 0;
    BOOL        bStat    = m_bStatFlag;  
	assert(nResults >= 0);
    assert(pszFuncName);

    if (bStat)
    {
        uTime = RDTSC();
    }

    bRetCode = m_piScript->CallFunction(dwScriptID, pszFuncName, nResults);
    KG_PROCESS_ERROR(bRetCode);

    if (bStat)
    {
        uTime = RDTSC() - uTime;

        UpdateStatInfo(dwScriptID, pszFuncName, uTime);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KScriptManager::Call(const char* pszScriptName, const char* pszFuncName, int nResults)
{
    BOOL    bResult     = false;
	BOOL    bRetCode    = false;
	DWORD   dwScriptID  = 0;
	
	assert(pszScriptName);
    assert(nResults >= 0);

	dwScriptID = g_FileNameHash(pszScriptName);

	bRetCode = Call(dwScriptID, pszFuncName, nResults);
	KG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KScriptManager::IsFuncExist(DWORD dwScriptID, const char* pszFuncName)
{
	assert(m_piScript);

	return m_piScript->IsFuncExist(dwScriptID, pszFuncName);
}

BOOL KScriptManager::IsFuncExist(const char* pszScriptName, const char* pszFuncName)
{
	DWORD dwScriptID = 0;

	assert(m_piScript);

	dwScriptID = g_FileNameHash(pszScriptName);

	return m_piScript->IsFuncExist(dwScriptID, pszFuncName);
}

BOOL KScriptManager::IsScriptExist(DWORD dwScriptID)
{
	return m_piScript->IsScriptExist(dwScriptID);
}

BOOL KScriptManager::IsScriptExist(const char cszScriptName[])
{
	BOOL    bResult     = false;
	DWORD   dwScriptID  = 0;

	assert(cszScriptName);

	dwScriptID = g_FileNameHash(cszScriptName);

	bResult = IsScriptExist(dwScriptID);
	KG_PROCESS_ERROR(bResult);

	bResult = true;
Exit0:
	return bResult;
}

void KScriptManager::StatBegin()
{
    m_bStatFlag = true;
    m_StatTable.clear();
    return;
}

void KScriptManager::StatEnd()
{
    if (!m_bStatFlag)
        return;

    OutputStatInfo();

    m_StatTable.clear();
    m_bStatFlag = false;

    return;
}

BOOL KScriptManager::Reload(const char* pszFileName)
{
    BOOL            bResult                 = false;
    BOOL            bRetCode                = false;
    IFile*          piFile                  = NULL;
    char*           pszBuffer               = NULL;
    DWORD           dwScriptID              = 0;
    DWORD           dwFileSize              = 0;
    DWORD           dwReadSize              = 0;
    KLuaScriptData* pScriptData             = NULL;
    char            szPathName[MAX_PATH];
    char            szFileName[MAX_PATH];

    assert(pszFileName);

    piFile = g_OpenFile(pszFileName);
    KG_PROCESS_ERROR(piFile);

    dwFileSize = piFile->Size();

    pszBuffer = new char[dwFileSize];
    KG_PROCESS_ERROR(pszBuffer);

    dwReadSize = piFile->Read(pszBuffer, dwFileSize);
    KG_PROCESS_ERROR(dwReadSize == dwFileSize);

    g_GetFullPath(szPathName, pszFileName);

    bRetCode = g_GetFilePathFromFullPath(szFileName, szPathName);
    KG_PROCESS_ERROR(bRetCode);

    dwScriptID = g_FileNameHash(szFileName);

    bRetCode = IsScriptExist(dwScriptID);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = m_piScript->LoadFromBuffer(dwScriptID, szPathName, pszBuffer, dwFileSize);
    KG_PROCESS_ERROR(bRetCode);

    pScriptData = m_piScript->GetScriptData(dwScriptID);
    if (pScriptData)
    {
        std::vector<DWORD>::iterator it = pScriptData->vecIncludeScriptID.begin();
        std::vector<DWORD>::iterator itEnd = pScriptData->vecIncludeScriptID.end();

        for (NULL; it != itEnd; ++it)
        {
            KLuaScriptData* pNewScriptData = NULL;

            pNewScriptData = m_piScript->GetScriptData(*it);
            if (pNewScriptData)
                Reload(pNewScriptData->szName);
        }
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(KGLOG_ERR, "[Lua] Failed to reload file \"%s\" !\n", pszFileName);
    }
    KG_DELETE_ARRAY(pszBuffer);
    KG_COM_RELEASE(piFile);
    return bResult;
}


BOOL KScriptManager::IsLuaScriptFile(const char cszFileName[])
{
    BOOL        bResult     = false;
    int         nRetCode    = 0;
    char        szExtName[] = ".lua";
    size_t      uExtNameLen = sizeof(szExtName) - 1;
    size_t      uLength     = 0;
    const char* pszPos      = NULL;

    uLength = strlen(cszFileName);
    KG_PROCESS_ERROR(uLength > uExtNameLen);

    pszPos = cszFileName + uLength - uExtNameLen;

    nRetCode = stricmp(pszPos, szExtName);
    KG_PROCESS_ERROR(nRetCode == 0);

    bResult = true;
Exit0:
    return bResult;
}

#if (defined(_MSC_VER) || defined(__ICL))   //WINDOWS
BOOL KScriptManager::LoadScripts(ILuaScriptEx* piScript, const char cszDir[])
{
    BOOL            bResult     = false;
	BOOL			bRetCode    = false;
    HANDLE          hFind       = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    char            szPath[MAX_PATH];

    snprintf(szPath, sizeof(szPath), "%s\\*", cszDir);
    szPath[sizeof(szPath) - 1] = '\0';

    hFind = FindFirstFile(szPath, &FindFileData);
    KGLOG_PROCESS_ERROR(hFind != INVALID_HANDLE_VALUE);

    do 
    {
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
            {
                snprintf(szPath, sizeof(szPath), "%s\\%s", cszDir, FindFileData.cFileName);
                szPath[sizeof(szPath) - 1] = '\0';

                LoadScripts(piScript, szPath);
            }
        }
        else
        {
            snprintf(szPath, sizeof(szPath), "%s\\%s", cszDir, FindFileData.cFileName);
            szPath[sizeof(szPath) - 1] = '\0';

            bRetCode = IsLuaScriptFile(szPath);
            if (bRetCode)
            {
                bRetCode = piScript->LoadFromFile(szPath);
                if (!bRetCode)
                {
                    KGLogPrintf(KGLOG_ERR, "[Lua] Failed to load file: %s\n", szPath);
                }
            }
        }

    } while (FindNextFile(hFind, &FindFileData));

    bResult = true;
Exit0:
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
    return bResult;
}
#else
BOOL KScriptManager::LoadScripts(ILuaScriptEx* piScript, const char cszDir[])
{
    BOOL            bResult     = false;
	BOOL			bRetCode    = false;
	DIR*            pDir        = NULL;
	struct dirent*  pDirNode    = NULL;
    char            szPath[MAX_PATH];

	pDir = opendir(cszDir);
    KGLOG_PROCESS_ERROR(pDir);

	while (pDirNode = readdir(pDir)) 
    {
		if(strcmp(pDirNode->d_name, ".") == 0 || strcmp(pDirNode->d_name, "..") == 0) 
            continue;

		if(pDirNode->d_type == DT_DIR) 
        {
            snprintf(szPath, sizeof(szPath), "%s/%s", cszDir, pDirNode->d_name);
            szPath[sizeof(szPath) - 1] = '\0';

			LoadScripts(piScript, szPath);

            continue;
		}

        snprintf(szPath, sizeof(szPath), "%s/%s", cszDir, pDirNode->d_name);
        szPath[sizeof(szPath) - 1] = '\0';

        bRetCode = IsLuaScriptFile(szPath);
        if (bRetCode)
        {
            bRetCode = piScript->LoadFromFile(szPath);
            if (!bRetCode)
            {
                KGLogPrintf(KGLOG_ERR, "[Lua] Failed to load file: %s\n", szPath);
            }
        }
	}
    
    bResult = true;
Exit0:
    if (pDir != NULL)
    {
        closedir(pDir);
        pDir = NULL;
    }
    return bResult;
}

#endif

BOOL KScriptManager::Load()
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    ILuaScriptEx*   piScript    = NULL;
    lua_State*      pLuaState   = NULL;

	piScript = CreateLuaInterface(&KMemory::LuaAllocator, NULL);
	KGLOG_PROCESS_ERROR(piScript);

    bRetCode = RegisterScriptFunctions(piScript);
    KGLOG_PROCESS_ERROR(bRetCode);

    piScript->RegisterConstList(g_LuaServerConstList);

    pLuaState = piScript->GetLuaState();
    KGLOG_PROCESS_ERROR(pLuaState);

    Luna<KScene>::Register(pLuaState);
    Luna<KHero>::Register(pLuaState);
    Luna<KPlayer>::Register(pLuaState);
    Luna<KAILogic>::Register(pLuaState);
    Luna<KAIState>::Register(pLuaState);
    Luna<KAIAction>::Register(pLuaState);
    Luna<KGItem>::Register(pLuaState);
    Luna<KGLuaItemInfo>::Register(pLuaState);
    Luna<KBall>::Register(pLuaState);

    bRetCode = LoadScripts(piScript, SERVER_SCRIPTS_DIR);
    KGLOG_PROCESS_ERROR(bRetCode);

    lua_gc(piScript->GetLuaState(), LUA_GCRESTART, 0);
    lua_gc(piScript->GetLuaState(), LUA_GCSETPAUSE, 100);
    lua_gc(piScript->GetLuaState(), LUA_GCSETSTEPMUL, 150);

    KG_COM_RELEASE(m_piScript);
    m_piScript = piScript;
    piScript = NULL;

    bResult = true;
Exit0:
    KG_COM_RELEASE(piScript);
    return bResult;
}

BOOL KScriptManager::UpdateStatInfo(DWORD dwScriptID, const char cszFuncName[], uint64_t uCostTime)
{
    BOOL                bResult         = false;
    KStatInfoNode*      pNode           = NULL;
    char                szKey[128];
    KFUNC_STAT_TABLE::iterator it;

    snprintf(szKey, sizeof(szKey), "%u:%s", dwScriptID, cszFuncName);
    szKey[sizeof(szKey) - 1] = '\0';

    it = m_StatTable.find(szKey);
    if (it == m_StatTable.end())
    {
        std::pair<KFUNC_STAT_TABLE::iterator, BOOL> InsRet = m_StatTable.insert(
            std::make_pair(kstring(szKey), KStatInfoNode())
            );
        KGLOG_PROCESS_ERROR(InsRet.second);

        it = InsRet.first;

        it->second.dwScriptID = dwScriptID;
        it->second.sFunction  = cszFuncName;
    }

    pNode = &it->second;

    pNode->dwCount++;
    pNode->uTime += uCostTime;

    bResult = true;
Exit0:
    return true;
}

void KScriptManager::OutputStatInfo()
{
    FILE* pFile = NULL;

    KGLOG_PROCESS_ERROR(m_piScript);  // here process "dwScriptID ---> FileName" needs it's present.

    pFile = fopen("center_script_stat.txt", "w");
    KGLOG_PROCESS_ERROR(pFile);

    fprintf(pFile, "Function\tTime\tCount\tFile\n");

    for (KFUNC_STAT_TABLE::iterator it = m_StatTable.begin(); it != m_StatTable.end(); ++it)
    {       
        KLuaScriptData* pScriptData = m_piScript->GetScriptData(it->second.dwScriptID);
        const char*     pszFunc     = it->second.sFunction.c_str();

        assert(pScriptData);
        assert(pszFunc);

        fprintf(pFile, "%s\t%llu\t%u\t%s\n", pszFunc, it->second.uTime, it->second.dwCount, pScriptData->szName);
    }

Exit0:
    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }
    return;
}

void KScriptManager::CallActivateScript()
{
    int nTopIndex = 0;

    SafeCallBegin(&nTopIndex);
    Push(g_pSO3World->m_nGameLoop);
    Call(m_dwMainScriptID, MAIN_SCRIPT_ACTIVATE_FUNCTION, 0);
    SafeCallEnd(nTopIndex);
}
