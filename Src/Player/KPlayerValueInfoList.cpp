#include "stdafx.h"
#include "KPlayerValueInfoList.h"
#include "KGEnumConvertor.h"

#include <limits.h>


KPlayerValueInfoList::KPlayerValueInfoList(void)
{
	m_uTotalBytes = 0;
}


KPlayerValueInfoList::~KPlayerValueInfoList(void)
{
}

BOOL KPlayerValueInfoList::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KPlayerValueInfoList::UnInit()
{
    m_vecMonthlyReset.clear();
    m_vecDailyReset.clear();
    m_vecPlayerValueInfo.clear();
}

KPlayerValueInfo* KPlayerValueInfoList::GetPlayerValue(DWORD dwID)
{
    KPlayerValueInfo* pResult = NULL;
    KPlayerValueInfo*       pTemp   = NULL;

    KGLOG_PROCESS_ERROR(dwID > 0 && dwID <= m_vecPlayerValueInfo.size());

    pTemp = &m_vecPlayerValueInfo[dwID - 1];
    KGLOG_PROCESS_ERROR(pTemp->bIsInUse);

    pResult = pTemp;
Exit0:
    return pResult;
}

size_t KPlayerValueInfoList::GetTotalBytes()
{
    return m_uTotalBytes;
}

BOOL KPlayerValueInfoList::LoadData()
{
	BOOL 		bResult 	= false;
	BOOL 		bRetCode 	= false;
	int  		nHeight 	= 0;
	ITabFile* 	piTabFile 	= NULL;
    DWORD       dwNextID    = 1;
    size_t      uBitCount       = 0;
    DWORD       dwID        = 0;
    size_t      uOffset     = 0;
    char        szTemp[_NAME_LEN];
    KPlayerValueInfo* pPlayerValueInfo = NULL;

	piTabFile = g_OpenTabFile(SETTING_DIR"/PlayerValue.tab");
	KGLOG_PROCESS_ERROR(piTabFile);

	nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 0);

    m_vecPlayerValueInfo.resize(nHeight - 1);
    m_vecDailyReset.reserve(nHeight - 1);
    m_vecDailyReset.reserve(nHeight - 1);
	for(int nRow = 2; nRow <= nHeight; ++nRow)
	{
		bRetCode = piTabFile->GetInteger(nRow, "ID", 0, (int*)&dwID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        KGLOG_PROCESS_ERROR(dwID == dwNextID);
        ++dwNextID;

        pPlayerValueInfo = &m_vecPlayerValueInfo[dwID - 1];
        memset(pPlayerValueInfo, 0, sizeof(KPlayerValueInfo));
        pPlayerValueInfo->dwID      = dwID;
        pPlayerValueInfo->uOffset   = uOffset;

        bRetCode = piTabFile->GetString(nRow, "Type", "", szTemp, sizeof(szTemp));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        szTemp[countof(szTemp) - 1] = '\0';
        bRetCode = ENUM_STR2INT(PLAYER_VALUE_TYPE, szTemp, pPlayerValueInfo->nType);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(pPlayerValueInfo->nType == KPLAYER_VALUE_TYPE_BOOLEAN || uOffset % UINT32_BIT_COUNT == 0 );

        bRetCode = piTabFile->GetInteger(nRow, "IsInUse", 0, &pPlayerValueInfo->bIsInUse);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetString(nRow, "SetType", "", szTemp, sizeof(szTemp));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        bRetCode = ENUM_STR2INT(KPLAYERVALUE_SET_TYPE, szTemp, pPlayerValueInfo->nSetType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRow, "ResetType", "", szTemp, sizeof(szTemp));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        bRetCode = ENUM_STR2INT(KRESET_TYPE, szTemp, pPlayerValueInfo->nResetType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = GetBitCountOfType(pPlayerValueInfo->nType, uBitCount);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRow, "ActivityGroup", "KACTIVITY_GROUP_INVALID", szTemp, sizeof(szTemp));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = ENUM_STR2INT(KACTIVITY_GROUP, szTemp, pPlayerValueInfo->nActivityGroup);
        KGLOG_PROCESS_ERROR(bRetCode);

        uOffset += uBitCount;

        switch(pPlayerValueInfo->nResetType)
        {
        case KRESET_TYPE_DAILY:
            m_vecDailyReset.push_back(pPlayerValueInfo);
            break;
        case KRESET_TYPE_WEEKLY:
            m_vecWeeklyReset.push_back(pPlayerValueInfo);
            break;
        case KRESET_TYPE_MONTHLY:
            m_vecMonthlyReset.push_back(pPlayerValueInfo);
            break;
        default:
            break;
        }

        m_PlayerValueGroup[pPlayerValueInfo->nActivityGroup].push_back(pPlayerValueInfo);
	}

    m_uTotalBytes = uOffset / 8 + 1;
    KGLOG_PROCESS_ERROR(m_uTotalBytes <= MAX_PLAYERVALUE_BYTES);

	bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(KGLOG_DEBUG, "ID:%u", dwID);
    }
	KG_COM_RELEASE(piTabFile);
	return bResult;
}

BOOL KPlayerValueInfoList::GetBitCountOfType(int nType, size_t& uBitCount)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

    switch(nType)
    {
    case KPLAYER_VALUE_TYPE_BOOLEAN:
        uBitCount = 1;
        break;

    case KPLAYER_VALUE_TYPE_INT32:
        uBitCount = 32;
        break;

    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    bResult = true;
Exit0:
    return bResult;
}

KVEC_DAILY_RESET& KPlayerValueInfoList::GetDailyResetList()
{
    return m_vecDailyReset;
}

KVEC_WEEKLY_RESET& KPlayerValueInfoList::GetWeeklyResetList()
{
    return m_vecWeeklyReset;
}

KVEC_MONTHLY_RESET& KPlayerValueInfoList::GetMonthlyResetList()
{
    return m_vecMonthlyReset;
}

KVEC_PLAYERVALUE_GROUP* KPlayerValueInfoList::GetGroup(int nGroupID)
{
    KVEC_PLAYERVALUE_GROUP* pResult = NULL;

    KGLOG_PROCESS_ERROR(nGroupID >= 0 && nGroupID < countof(m_PlayerValueGroup));

    pResult = &m_PlayerValueGroup[nGroupID];

Exit0:
    return pResult;
}

