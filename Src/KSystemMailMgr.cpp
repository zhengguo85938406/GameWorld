#include "stdafx.h"
#include "KSystemMailMgr.h"


KSystemMailMgr::KSystemMailMgr(void)
{
}


KSystemMailMgr::~KSystemMailMgr(void)
{
}

BOOL KSystemMailMgr::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = LoadSystemMailTxt();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KSystemMailMgr::UnInit()
{
    m_mapSystemMail.clear();
}

KSystemMailTxt* KSystemMailMgr::GetSystemMailTxt(DWORD dwSystemMailTemplateID)
{
    KSystemMailTxt* pResult = NULL;
    KMAP_SYSMAIL::iterator it;

    KG_PROCESS_ERROR(dwSystemMailTemplateID);

    it = m_mapSystemMail.find(dwSystemMailTemplateID);
    KG_PROCESS_ERROR(it != m_mapSystemMail.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

BOOL KSystemMailMgr::LoadSystemMailTxt()
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    ITabFile*       piTabFile   = NULL;
    DWORD           dwID        = ERROR_ID;
    KSystemMailTxt* pSystemMailTxt = NULL;
    pair<std::map<DWORD, KSystemMailTxt>::iterator, bool> InsRet;

    piTabFile = g_OpenTabFile(SETTING_DIR"/SysMail.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); ++nRowIndex)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", ERROR_ID, (int*)(&dwID));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(dwID > 0);

        InsRet = m_mapSystemMail.insert(std::make_pair(dwID, KSystemMailTxt()));
        KGLOG_PROCESS_ERROR(InsRet.second);

        pSystemMailTxt = &InsRet.first->second;

        bRetCode = piTabFile->GetInteger(nRowIndex, "Type", 0, &pSystemMailTxt->m_nMailType);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetString(nRowIndex, "Sender", "System", pSystemMailTxt->m_szSendName, sizeof(pSystemMailTxt->m_szSendName));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "Title", "System Mail", pSystemMailTxt->m_szMailTitl, sizeof(pSystemMailTxt->m_szMailTitl));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "Message", "", pSystemMailTxt->m_szMailTxt, sizeof(pSystemMailTxt->m_szMailTxt));
        KGLOG_PROCESS_ERROR(bRetCode);

    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}
