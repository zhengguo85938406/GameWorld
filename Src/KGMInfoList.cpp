#include "stdafx.h"
#include "KGMInfoList.h"

KGMInfoList::KGMInfoList()
{

}

KGMInfoList::~KGMInfoList()
{

}

BOOL KGMInfoList::Init()
{
    BOOL        bResult     = false;
    int         nRetCode    = false;
    ITabFile*   piTabFile   = NULL;
    char        szFilePath[MAX_PATH];

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, GMACCOUNT_LIST_FILE_NAME);
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        GMAccountInfo NewGMAccount;

        piTabFile->GetString(nRowIndex, "szAccount", "", NewGMAccount.szGMAccount, sizeof(NewGMAccount.szGMAccount));
        NewGMAccount.szGMAccount[countof(NewGMAccount.szGMAccount) - 1] = '\0';
        KGLOG_PROCESS_ERROR(NewGMAccount.szGMAccount != '\0');

        m_GMAccountInfoList.push_back(NewGMAccount);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

void KGMInfoList::UnInit()
{
    m_GMAccountInfoList.clear();
}

BOOL KGMInfoList::CheckIsPermission(char* pszAccount)
{
    BOOL bResult    = false;
    int  nRetCode   = false;

    vector<GMAccountInfo>::iterator it = m_GMAccountInfoList.begin();

    while (it != m_GMAccountInfoList.end())
    {
        nRetCode = strcmp(pszAccount, it->szGMAccount);
        if (nRetCode == 0)
        {
            bResult = true;
            break;
        }

        ++it;
    }

Exit0:
    return bResult;
}
