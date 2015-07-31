#include "stdafx.h"
#include "KAchievementSettings.h"
#include "KGEnumConvertor.h"


KAchievementSettings::KAchievementSettings(void)
{
}


KAchievementSettings::~KAchievementSettings(void)
{
}

BOOL KAchievementSettings::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KAchievementSettings::UnInit()
{
    m_vecWeekyAchievement.clear();
    m_vecDailyAchievement.clear();
    m_vecAchievement.clear();
}

KAchievementSettingsItem* KAchievementSettings::GetItem(DWORD dwID)
{
    KAchievementSettingsItem* pResult = NULL;
    KAchievementSettingsItem* pTemp = NULL;

    KGLOG_PROCESS_ERROR(dwID > 0 && dwID <= m_vecAchievement.size());

    pTemp = &m_vecAchievement[dwID - 1];
    KG_PROCESS_ERROR(!pTemp->bExpired);

    pResult = pTemp;
Exit0:
    return pResult;
}

BOOL KAchievementSettings::LoadData()
{
	BOOL 		bResult 	= false;
	BOOL 		bRetCode 	= false;
	int  		nHeight 	= 0;
	DWORD       dwID        = ERROR_ID;
    DWORD       dwNextID    = 1;
    ITabFile* 	piTabFile 	= NULL;
    KAchievementSettingsItem* pAchievementInfo = NULL;
    char        szKey[64];
    char        szValue[256];

	piTabFile = g_OpenTabFile(SETTING_DIR"/Achievement.tab");
	KGLOG_PROCESS_ERROR(piTabFile);
    
	nHeight = piTabFile->GetHeight();    
    if (nHeight > 1 && nHeight - 1 <= cdMaxAchievenmentCount)
        m_vecAchievement.resize(nHeight - 1);

    for(int nRow = 2; nRow <= nHeight; ++nRow)
	{
        bRetCode = piTabFile->GetInteger(nRow, "ID", 0, (int*)&dwID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        KGLOG_PROCESS_ERROR(dwID == dwNextID);
        ++dwNextID;

        pAchievementInfo = &m_vecAchievement[dwID - 1];
        pAchievementInfo->dwID = dwID;

        bRetCode = piTabFile->GetInteger(nRow, "Expired", 0, &pAchievementInfo->bExpired);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "AchievementPoint", 0, &pAchievementInfo->nAchievementPoint);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(pAchievementInfo->nAchievementPoint >= 0);

        bRetCode = piTabFile->GetInteger(nRow, "AutoFinish", 0, &pAchievementInfo->bAutoFinish);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "MessageToFriend", 0, &pAchievementInfo->bMessageToFriend);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "MessageToServer", 0, &pAchievementInfo->bMessageToServer);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "MessageToClub", 0, &pAchievementInfo->bMessageToClub);
        KGLOG_PROCESS_ERROR(bRetCode);
        
        bRetCode = piTabFile->GetString(nRow, "AwardItem", "0,0,0,0",  szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = sscanf(szValue, "%u,%u,%d,%d", &pAchievementInfo->dwAwardItemType, &pAchievementInfo->dwAwardItemIndex, &pAchievementInfo->nAwardItemCount, &pAchievementInfo->nAwardItemValuePoint);
        KGLOG_PROCESS_ERROR(bRetCode == 4);

        bRetCode = piTabFile->GetString(nRow, "Event", "peInvalid", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = ENUM_STR2INT(PLAYER_EVENT_TYPE, szValue, pAchievementInfo->cEventCondition.nEvent);
        KGLOG_PROCESS_ERROR(bRetCode);

        for (int i = 0; i < countof(pAchievementInfo->cEventCondition.AllCondition); ++i)
        {
            sprintf(szKey, "Param%dConditon", i + 1);

            bRetCode = piTabFile->GetString(nRow, szKey, "", szValue, sizeof(szValue));
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = pAchievementInfo->cEventCondition.AllCondition[i].InitFromString(szValue);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        bRetCode = piTabFile->GetString(nRow, "ValueUpdateType", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = LoadUpdator(szValue, pAchievementInfo);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "TargetValue", 0, &pAchievementInfo->nTargetValue);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRow, "Type", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        bRetCode = ENUM_STR2INT(KACHIEVEMENT_TYPE, szValue, pAchievementInfo->nAchievementType);
        KGLOG_PROCESS_ERROR(bRetCode);

        switch(pAchievementInfo->nAchievementType)
        {
        case KACHIEVEMENT_TYPE_DAILY:
            m_vecDailyAchievement.push_back(pAchievementInfo);
            break;
        case KACHIEVEMENT_TYPE_WEEKLY:
            m_vecWeekyAchievement.push_back(pAchievementInfo);
            break;
        default:
            break;
        }
	}

Exit1:
	bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}


BOOL KAchievementSettings::LoadUpdator(char szValue[], KAchievementSettingsItem* pItem)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nCur = 0;
    int  nUpdateType = KEVENT_VALUE_UPDATE_ADDV;
    int  nUpdateParam = 0;

    KGLOG_PROCESS_ERROR(szValue);

    if (!szValue[0])
    {
        pItem->nValueUpdateType = KEVENT_VALUE_UPDATE_ADDV;
        pItem->nValueUpdateParam = 1;

        goto Exit1;
    }

    if (szValue[nCur] == 'P')
    {
        nUpdateType = KEVENT_VALUE_UPDATE_ADDP;
        ++nCur;
    }

    bRetCode = g_CStringToInteger(szValue + nCur, nUpdateParam);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (nUpdateType == KEVENT_VALUE_UPDATE_ADDP)
    {
        nUpdateParam -= 1;
        KGLOG_PROCESS_ERROR(nUpdateParam >= 0 && nUpdateParam < cdMaxEventParamCount);
    }

    pItem->nValueUpdateType  = nUpdateType;
    pItem->nValueUpdateParam = nUpdateParam;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

KVEC_DAILY_ACHIEVEMENTS* KAchievementSettings::GetDailyAchievement()
{
    return &m_vecDailyAchievement;
}

KVEC_WEEKLY_ACHIEVEMENTS* KAchievementSettings::GetWeeklyAchievement()
{
    return &m_vecWeekyAchievement;
}

int KAchievementSettings::GetAchievementCount()
{
    return (int)m_vecAchievement.size();
}

DWORD KAchievementSettings::GetMaxAchievementID()
{
    return (DWORD)m_vecAchievement.size();
}

BOOL KAchievementSettings::IsValidAchievement(DWORD dwID)
{
    return dwID > 0 && dwID <= m_vecAchievement.size();
}
