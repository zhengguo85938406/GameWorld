#include "stdafx.h"
#include "KActiveSettings.h"
#include "KGEnumConvertor.h"


KActiveSettings::KActiveSettings(void)
{
}


KActiveSettings::~KActiveSettings(void)
{
}

BOOL KActiveSettings::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KActiveSettings::UnInit()
{
    KMAP_ACTIVE_UPDATOR::iterator it;

    m_vecActive.clear();
    for (it = m_mapValueUpdator.begin(); it != m_mapValueUpdator.end(); ++it)
    {
        KMEMORY_DELETE(it->second);
    }
    m_mapValueUpdator.clear();
}

KActiveSettingsItem* KActiveSettings::GetItem(DWORD dwID)
{
    KActiveSettingsItem* pResult = NULL;

    KGLOG_PROCESS_ERROR(dwID > 0 && dwID <= m_vecActive.size());

    pResult = &m_vecActive[dwID - 1];

Exit0:
    return pResult;
}

BOOL KActiveSettings::LoadData()
{
	BOOL 		bResult 	= false;
	BOOL 		bRetCode 	= false;
	int  		nHeight 	= 0;
    int         nEvent      = 0;
	DWORD       dwID        = ERROR_ID;
    DWORD       dwNextID    = 1;
    ITabFile* 	piTabFile 	= NULL;
    char        szEvent[64] = {0};
    KActiveSettingsItem* pActiveInfo = NULL;
    KVEC_ACTIVE_UPDATOR* pvecActive = NULL;
    KMAP_ACTIVE_UPDATOR::iterator it;

	piTabFile = g_OpenTabFile(SETTING_DIR"/ActivePlayer.tab");
	KGLOG_PROCESS_ERROR(piTabFile);
    
	nHeight = piTabFile->GetHeight();    
    m_vecActive.resize(nHeight - 1);

    for(int nRow = 2; nRow <= nHeight; ++nRow)
	{
        bRetCode = piTabFile->GetInteger(nRow, "ID", 0, (int*)&dwID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        KGLOG_PROCESS_ERROR(dwID == dwNextID);
        ++dwNextID;

        pActiveInfo = &m_vecActive[dwID - 1];
        pActiveInfo->dwID = dwID;

        szEvent[0] = '\0';
        bRetCode = piTabFile->GetString(nRow, "Event", "peInvalid", szEvent, sizeof(szEvent));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = ENUM_STR2INT(PLAYER_EVENT_TYPE, szEvent, pActiveInfo->nEvent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "FinishValue", 0, (int*)&(pActiveInfo->nFininshValue));
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRow, "UnLimitCount", 0, (BOOL*)&(pActiveInfo->bUnLimit));
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRow, "ActivePoint", 0, (int*)&(pActiveInfo->nAwardActive));
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        it = m_mapValueUpdator.find(pActiveInfo->nEvent);
        if (it == m_mapValueUpdator.end())
        {
            pvecActive = KMEMORY_NEW(KVEC_ACTIVE_UPDATOR);
            m_mapValueUpdator[pActiveInfo->nEvent] = pvecActive;
        }
        else
        {
            pvecActive = it->second;
        }

        pvecActive->push_back(dwID);
        pvecActive = NULL;
	}

Exit1:
	bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}

int KActiveSettings::GetActiveCount()
{
    return (int)m_vecActive.size();
}

DWORD KActiveSettings::GetMaxActiveID()
{
    return (DWORD)m_vecActive.size();
}

BOOL KActiveSettings::IsValidActive(DWORD dwID)
{
    return dwID > 0 && dwID <= m_vecActive.size();
}

KVEC_ACTIVE_UPDATOR* KActiveSettings::GetActiveByEvent(int nEvent)
{
    KVEC_ACTIVE_UPDATOR* pvecActive = NULL;
    KMAP_ACTIVE_UPDATOR::iterator it;

    KGLOG_PROCESS_ERROR(nEvent > peInvalid && nEvent < peTotal);

    it = m_mapValueUpdator.find(nEvent);
    if (it != m_mapValueUpdator.end())
    {
        pvecActive = it->second;
    }

Exit0:
    return pvecActive;
}