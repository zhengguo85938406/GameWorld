#include "stdafx.h"
#include "KMapListFile.h"
#include "KSO3World.h"
#include "KScene.h"

BOOL KMapListFile::Init(void)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    DWORD       dwMapID     = 0;
    KMapParams  MapParam;
	ITabFile*	piTabFile   = NULL;

	piTabFile = g_OpenTabFile(SETTING_DIR"/"MAP_LIST_FILE_NAME);
	KGLOG_PROCESS_ERROR(piTabFile);

    piTabFile->SetErrorLog(false);

	for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
	{
        memset(&MapParam, 0, sizeof(MapParam));

		bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)&dwMapID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
		KGLOG_PROCESS_ERROR(dwMapID > 0 && dwMapID <= MAX_MAP_ID);

		bRetCode = piTabFile->GetString(nRowIndex, "Name", "", MapParam.szMapName, sizeof(MapParam.szMapName));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
		KGLOG_PROCESS_ERROR(MapParam.szMapName[0] != '\0');

        bRetCode = piTabFile->GetInteger(nRowIndex, "Type", 1, &MapParam.nType);
        KGLOG_PROCESS_ERROR(bRetCode);

        MapParam.szResourceFilePath[0] = '\0';
        bRetCode = piTabFile->GetString(nRowIndex, "ResourcePath", "", MapParam.szResourceFilePath, sizeof(MapParam.szResourceFilePath));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BattleID", 0, (int*)&MapParam.dwBattleID);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "ServerResource", 0, &MapParam.nServerResourceID);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "ScriptDir", 0, &MapParam.nScriptDir);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BuffAwardID", 0, (int*)&MapParam.dwBuffAwardID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "ItemAwardID", 0, (int*)&MapParam.dwItemAwardID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        MapParam.dwMapID = dwMapID;
        m_MapParamTable[MapParam.dwMapID] = MapParam;
	}

    bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}

KMapParams*	KMapListFile::GetMapParamByID(DWORD dwMapID)
{
    KMAP_PARAM_TABLE::iterator it = m_MapParamTable.find(dwMapID);

    if (it != m_MapParamTable.end())
    {
        return &it->second;
    }

	return NULL;
}

KMapParams*	KMapListFile::GetMapParamByName(const char cszName[])
{
    int                        nRetCode = 0;
    KMAP_PARAM_TABLE::iterator it       = m_MapParamTable.begin();
    
    while (it != m_MapParamTable.end())
    {
        nRetCode = strcmp(it->second.szMapName, cszName);
        if (nRetCode == 0)
        {
            return &it->second;
        }
        ++it;
    }

    return NULL;
}

