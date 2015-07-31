#include "stdafx.h"
#include "KPlayerLevelData.h"

BOOL KPlayerLevelData::LoadData()
{
    BOOL        bResult     = false;
    int         nRetCode    = false;
    ITabFile*   piTabFile   = NULL;
    char        szFilePath[MAX_PATH];
    int         nLevel      = 0;
    int         nExp        = 0;

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, "TeamLevelData.tab");
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        nRetCode = piTabFile->GetInteger(nRowIndex, "Level", 0, &nLevel);
        KGLOG_PROCESS_ERROR(nRetCode > 0);

        nRetCode = piTabFile->GetInteger(nRowIndex, "Experience", 0, &nExp);
        KGLOG_PROCESS_ERROR(nRetCode > 0);

        m_mapLevelData[nLevel] = nExp;
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

int  KPlayerLevelData::GetLevelUpExp(int nLevel)
{
    int nExp = 0;
    std::map<int, int>::iterator it;

    it = m_mapLevelData.find(nLevel);
    KGLOG_PROCESS_ERROR(it != m_mapLevelData.end());

    nExp = it->second;
Exit0:
    return nExp;
}
