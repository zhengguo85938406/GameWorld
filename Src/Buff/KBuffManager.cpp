#include "stdafx.h"
#include "KBuffManager.h"
#include "Engine/KMemory.h"
#include "KAttribute.h"

KBuffManager::KBuffManager()
{

}

KBuffManager::~KBuffManager()
{

}

BOOL KBuffManager::Init()
{
	BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = LoadBuff("Buff.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KBuffManager::UnInit()
{
    KBUFF_INFO_MAP::iterator it = m_mapBuff.begin();
    for (;it != m_mapBuff.end(); ++it)
    {
        it->second.UnInit();
    }
    m_mapBuff.clear();
}

KBuff* KBuffManager::GetBuff(DWORD dwID)
{
    KBuff* pResult = NULL;
    KBUFF_INFO_MAP::iterator it;
    
    it = m_mapBuff.find(dwID);
    KGLOG_PROCESS_ERROR(it != m_mapBuff.end());

    pResult = &(it->second);
Exit0:
    return pResult;
}

BOOL KBuffManager::LoadBuff(const char* szTabFile)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    ITabFile*   piTabFile   = NULL;
    KBuff*      pBuff       = NULL;         
    DWORD       dwBuffID    = ERROR_ID;
    KAttribData AttribData;
    pair<KBUFF_INFO_MAP::iterator, bool> InsRet;
    int         nTabData = 0;

    char        szFilePath[MAX_PATH];

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, BUFF_LIST_FILE_NAME);
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    piTabFile->SetErrorLog(false);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        InsRet = m_mapBuff.insert(std::make_pair(++dwBuffID, KBuff()));
        KGLOG_PROCESS_ERROR(InsRet.second);
        pBuff = &(InsRet.first->second);

        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, &nTabData);
        pBuff->m_dwID = (DWORD)nTabData;
        KGLOG_PROCESS_ERROR(pBuff->m_dwID > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Frame", 0, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pBuff->m_nFrame = nTabData;

        bRetCode = piTabFile->GetInteger(nRowIndex, "TimeLapseOffline", 0, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pBuff->m_bTimeLapseOffline = nTabData;

        bRetCode = piTabFile->GetInteger(nRowIndex, "DelOnFloor", 0, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pBuff->m_bDelOnFloor = nTabData;

        bRetCode = piTabFile->GetInteger(nRowIndex, "DelOnObjNoAttached", 0, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pBuff->m_bDelOnObjNoAttached = nTabData;

        bRetCode = piTabFile->GetInteger(nRowIndex, "ActiveCount", 0, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pBuff->m_nActiveCount = nTabData;

        bRetCode = piTabFile->GetInteger(nRowIndex, "NeedSave", 0, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pBuff->m_bNeedSave = nTabData;
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "Share", 0, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pBuff->m_bShare = nTabData;

        for (int i = 1; i <= 8; i++)
        {
            bRetCode = LoadAttribute(piTabFile, nRowIndex, "Attribute", i, &AttribData);
            KGLOG_PROCESS_ERROR(bRetCode);

            if (AttribData.nKey != atInvalid)
            {
                APPEND_ATTRIB(pBuff->m_pRollBackAttr, AttribData);
            }
        }

        for (int i = 1; i <= 4; i++)
        {
            bRetCode = LoadAttribute(piTabFile, nRowIndex, "ActiveAttr", i, &AttribData);
            KGLOG_PROCESS_ERROR(bRetCode);

            if (AttribData.nKey != atInvalid)
            {
                APPEND_ATTRIB(pBuff->m_pActiveAttr, AttribData);
            }
        }
    }
    
    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);

    if (!bResult)
    {
        KBUFF_INFO_MAP::iterator it = m_mapBuff.begin();
        for (;it != m_mapBuff.end(); ++it)
        {
            it->second.UnInit();
        }
        m_mapBuff.clear();
    }
    return bResult;
}
