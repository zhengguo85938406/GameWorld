#include "stdafx.h"
#include "KQuestInfoList.h"
#include "KGEnumConvertor.h"
#include <algorithm>
#include "KTools.h"
#include <iterator>
#include "KSO3World.h"
#include "GlobalMacro.h"

#define QUEST_FILE_NAME	"Quests.tab"
#define RANDOM_QUEST_FILE_NAME	"RandomQuestGroup.tab"

BOOL KQuestInfoList::Init(void)
{
    BOOL        bResult     = false;
    int         nRetCode    = false;
    int         nHeight     = 0;
    ITabFile*   piTabFile   = NULL;
	char        szFilePath[MAX_PATH];
    std::pair<MAP_ID_2_QUEST_INFO::iterator, bool> InsRet;

    nRetCode = (int)snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, QUEST_FILE_NAME);
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFilePath));

	piTabFile = g_OpenTabFile(szFilePath);
	KGLOG_PROCESS_ERROR(piTabFile);

	nHeight = piTabFile->GetHeight();
	KGLOG_PROCESS_ERROR(nHeight > 1);

	nRetCode = LoadQuestInfo(piTabFile, 2, &m_DefaultQuestInfo);
	KGLOG_PROCESS_ERROR(nRetCode);

	for (int nIndex = 3; nIndex <= nHeight; nIndex++)
	{
		KQuestInfo	TempQuestInfo;

		nRetCode = LoadQuestInfo(piTabFile, nIndex, &TempQuestInfo);
		KGLOG_PROCESS_ERROR(nRetCode);

        InsRet = m_mapID2QuestInfo.insert(std::make_pair(TempQuestInfo.dwQuestID, TempQuestInfo));
        KGLOG_PROCESS_ERROR(InsRet.second);

        if (TempQuestInfo.bDaily)
            m_DailyQuest.push_back(TempQuestInfo.dwQuestID);
	}

    nRetCode = LoadRandomQuestGroup();
    KGLOG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}

void KQuestInfoList::UnInit()
{
}

KQuestInfo* KQuestInfoList::GetQuestInfo(DWORD dwQuestID)
{
    KQuestInfo* pResult = NULL;
	MAP_ID_2_QUEST_INFO::iterator it;

    KG_PROCESS_ERROR(dwQuestID > 0);

	it = m_mapID2QuestInfo.find(dwQuestID);
    KGLOG_PROCESS_ERROR(it != m_mapID2QuestInfo.end());

    pResult = &(it->second);
Exit0:
    return pResult;
}

void KQuestInfoList::GetDailyQuest(std::vector<DWORD>& vecQuest)
{
    std::copy(m_DailyQuest.begin(), m_DailyQuest.end(), back_inserter(vecQuest));
}

BOOL KQuestInfoList::LoadQuestInfo(ITabFile* piTabFile, int nIndex, KQuestInfo* pQuestInfo)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;
    int  nTabData = 0;
    char szColName[64];
    char szScriptName[MAX_PATH];
    char szValue[256] = "";

	bRetCode = piTabFile->GetInteger(nIndex, "QuestID", m_DefaultQuestInfo.dwQuestID, (int*)&(pQuestInfo->dwQuestID));
	KGLOG_PROCESS_ERROR(bRetCode);
    KGLOG_PROCESS_ERROR(pQuestInfo->dwQuestID < MAX_QUEST_COUNT);

	bRetCode = piTabFile->GetString(nIndex, "QuestName", m_DefaultQuestInfo.szQuestName, pQuestInfo->szQuestName, _NAME_LEN);
	KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(nIndex, "QuestType", m_DefaultQuestInfo.nQuestType, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->nQuestType = nTabData;
  
	bRetCode = piTabFile->GetInteger(nIndex, "CanRepeat", m_DefaultQuestInfo.bRepeat, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bRepeat = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "NeedAccept", m_DefaultQuestInfo.bAccept, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bAccept = (bool)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "QuestLevel", m_DefaultQuestInfo.byLevel, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "StartItemType", m_DefaultQuestInfo.byStartItemType, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byStartItemType = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "StartItemIndex", m_DefaultQuestInfo.wStartItemIndex, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->wStartItemIndex = (WORD)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MinLevel", m_DefaultQuestInfo.byMinLevel, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byMinLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "MaxLevel", m_DefaultQuestInfo.byMaxLevel, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byMaxLevel = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "RequireGender", m_DefaultQuestInfo.byRequireGender, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byRequireGender = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "RequireSingle", m_DefaultQuestInfo.byRequireSingle, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
	pQuestInfo->byRequireSingle = (BYTE)nTabData;

	bRetCode = piTabFile->GetInteger(
        nIndex, "CooldownID",
        m_DefaultQuestInfo.dwCoolDownID, (int*)&(pQuestInfo->dwCoolDownID)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piTabFile->GetInteger(
        nIndex, "PrequestID",
        m_DefaultQuestInfo.dwPrequestID, (int*)&pQuestInfo->dwPrequestID
    );
    KGLOG_PROCESS_ERROR(bRetCode);
    
    bRetCode = piTabFile->GetInteger(
        nIndex, "SubsequenceID", 
		m_DefaultQuestInfo.dwSubsequenceID, (int*)&(pQuestInfo->dwSubsequenceID)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "StartTime",
		m_DefaultQuestInfo.nStartTime, (int*)&(pQuestInfo->nStartTime)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "EndTime",
		m_DefaultQuestInfo.nEndTime, (int*)&(pQuestInfo->nEndTime)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

    for (int i = 0; i < QUEST_OFFER_ITEM_COUNT; ++i)
    {
        sprintf(szColName, "OfferItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.dwOfferItemType[i], (int*)&pQuestInfo->dwOfferItemType[i]);
	    KGLOG_PROCESS_ERROR(bRetCode);
        
        sprintf(szColName, "OfferItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.nOfferItemIndex[i], &pQuestInfo->nOfferItemIndex[i]);
	    KGLOG_PROCESS_ERROR(bRetCode);
        
        sprintf(szColName, "OfferItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.nOfferItemAmount[i], &pQuestInfo->nOfferItemAmount[i]);
	    KGLOG_PROCESS_ERROR(bRetCode);
    }
    
    for (int i = 0; i < QUEST_END_ITEM_COUNT; ++i)
    {
        sprintf(szColName, "EndRequireItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byEndRequireItemType[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byEndRequireItemType[i] = (BYTE)nTabData;
        
        sprintf(szColName, "EndRequireItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wEndRequireItemIndex[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wEndRequireItemIndex[i] = (WORD)nTabData;
        
        sprintf(szColName, "EndRequireItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wEndRequireItemAmount[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wEndRequireItemAmount[i] = (WORD)nTabData;
        
        sprintf(szColName, "DropDoodadTemplateID%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        sprintf(szColName, "IsDeleteEndRequireItem%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.bIsDeleteEndRequireItem[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->bIsDeleteEndRequireItem[i] = (bool)nTabData;
    }
    
    bRetCode = piTabFile->GetInteger(nIndex, "EventOrder", m_DefaultQuestInfo.bEventOrder, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bEventOrder = (bool)nTabData;

    for (int i = 0; i < countof(pQuestInfo->AllEvent); ++i)
    {
        sprintf(szColName, "QuestValue%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.AllEvent[i].nQuestValue, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->AllEvent[i].nQuestValue = nTabData;

        sprintf(szColName, "QuestEvent%d", i + 1);
        bRetCode = piTabFile->GetString(nIndex, szColName, "peInvalid", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = EnumStr2Int("PLAYER_EVENT_TYPE", szValue, &nTabData);
        KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->AllEvent[i].nEvent       = nTabData;

        for (int j = 0; j < countof(pQuestInfo->AllEvent[i].AllCondition); ++j)
        {
            sprintf(szColName, "Event%dParam%d", i + 1, j  + 1);
            szColName[sizeof(szColName) - 1] = '\0';

            bRetCode = piTabFile->GetString(nIndex, szColName, "", szValue, sizeof(szValue));
            KGLOG_PROCESS_ERROR(bRetCode);
            szValue[countof(szValue) - 1] = '\0';

            bRetCode = pQuestInfo->AllEvent[i].AllCondition[j].InitFromString(szValue);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }
    
	bRetCode = piTabFile->GetInteger(nIndex, "PresentAll1", m_DefaultQuestInfo.bPresentAll[0], &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bPresentAll[0] = (bool)nTabData;

	bRetCode = piTabFile->GetInteger(nIndex, "PresentAll2", m_DefaultQuestInfo.bPresentAll[1], &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bPresentAll[1] = (bool)nTabData;
    
    for (int i = 0; i < cdQuestParamCount; ++i)
    {
        sprintf(szColName, "PresentItemType%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.byPresentItemType[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->byPresentItemType[i] = (BYTE)nTabData;
        
        sprintf(szColName, "PresentItemIndex%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wPresentItemIndex[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wPresentItemIndex[i] = (WORD)nTabData;
        
        sprintf(szColName, "PresentItemAmount%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wPresentItemAmount[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wPresentItemAmount[i] = (WORD)nTabData;

        sprintf(szColName, "PresentItemValuePoint%d", i + 1);
        szColName[sizeof(szColName) - 1] = '\0';

	    bRetCode = piTabFile->GetInteger(nIndex, szColName, m_DefaultQuestInfo.wPresentItemValuePoint[i], &nTabData);
	    KGLOG_PROCESS_ERROR(bRetCode);
        pQuestInfo->wPresentItemValuePoint[i] = (WORD)nTabData;
    }  

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentExp",
		m_DefaultQuestInfo.nPresentExp, (int*)&(pQuestInfo->nPresentExp)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(
        nIndex, "PresentMoney",
		m_DefaultQuestInfo.nPresentMoney, (int*)&(pQuestInfo->nPresentMoney)
    );
	KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piTabFile->GetInteger(nIndex, "RepeatCutPercent", m_DefaultQuestInfo.byRepeatCutPercent, &nTabData);
	KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->byRepeatCutPercent = (BYTE)nTabData;
    
    pQuestInfo->dwScriptID = 0;
    bRetCode = piTabFile->GetString(nIndex, "ScriptName", "", szScriptName, MAX_PATH);
	KGLOG_PROCESS_ERROR(bRetCode);
    if (szScriptName[0] != '\0')
    {
        pQuestInfo->dwScriptID = g_FileNameHash(szScriptName);
    }

    bRetCode = piTabFile->GetInteger(nIndex, "CostMoney", m_DefaultQuestInfo.nCostMoney, (int*)&(pQuestInfo->nCostMoney));
	KGLOG_PROCESS_ERROR(bRetCode);
    KGLOG_PROCESS_ERROR(pQuestInfo->nCostMoney >= 0);

    bRetCode = piTabFile->GetInteger(nIndex, "AutoFinish", m_DefaultQuestInfo.bAutoFinish, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bAutoFinish = (BOOL)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "ClientCanFinish", m_DefaultQuestInfo.bClientCanFinish, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bClientCanFinish = (BOOL)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "Daily", m_DefaultQuestInfo.bDaily, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->bDaily = (BOOL)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "RequireMissionType", m_DefaultQuestInfo.nRequireMissionType, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->nRequireMissionType = (DWORD)nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "RequireMissionStep", m_DefaultQuestInfo.nRequireMissionStep, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->nRequireMissionStep = nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "RequireMissionLevel", m_DefaultQuestInfo.nRequireMissionLevel, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->nRequireMissionLevel = nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "RequireMapType", m_DefaultQuestInfo.nRequireMapType, &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->nRequireMapType = nTabData;
    bRetCode = piTabFile->GetInteger(nIndex, "AwardHero1TemplateID", m_DefaultQuestInfo.dwAwardHeroTemplateID[0], &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->dwAwardHeroTemplateID[0] = nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "AwardHero2TemplateID", m_DefaultQuestInfo.dwAwardHeroTemplateID[1], &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->dwAwardHeroTemplateID[1] = nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "AwardHero1Level", m_DefaultQuestInfo.nAwardHeroLevel[0], &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->nAwardHeroLevel[0] = nTabData;

    bRetCode = piTabFile->GetInteger(nIndex, "AwardHero2Level", m_DefaultQuestInfo.nAwardHeroLevel[1], &nTabData);
    KGLOG_PROCESS_ERROR(bRetCode);
    pQuestInfo->nAwardHeroLevel[1] = nTabData;

    bResult = true;
Exit0:
	return bResult;
}

BOOL KQuestInfoList::LoadRandomQuestGroup()
{
    BOOL        bResult         = false;
    int         nRetCode        = false;
    int         nHeight         = 0;
    ITabFile*   piTabFile       = NULL;
    DWORD       dwTemplateID    = 0;
    DWORD       dwQuestID       = 0;
    KQuestInfo* pQuestInfo      = NULL;
    char        szFilePath[MAX_PATH];
    char        szColName[64];
    std::set<DWORD>::iterator setIt;
    int         nID             = 0;

    nRetCode = (int)snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, RANDOM_QUEST_FILE_NAME);
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFilePath));

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();
    if (nHeight >= 2)
    {
        for (int i = 2; i <= nHeight; ++i)
        {
            KQUEST_GROUP QuestGroupInfo;

            nRetCode = piTabFile->GetInteger(i, "ID", 0, &nID);
            KGLOG_PROCESS_ERROR(nRetCode > 0);

            for (int j = 0; j < RANDOM_QUEST_COUNT_PER_GROUP; ++j)
            {
                snprintf(szColName, sizeof(szColName), "Quest%d", j + 1);
                szColName[sizeof(szColName) - 1] = '\0';

                nRetCode = piTabFile->GetInteger(i, szColName, 0, (int*)&dwQuestID);

                if (dwQuestID == 0)
                    break;

                pQuestInfo = GetQuestInfo(dwQuestID);
                KGLOG_PROCESS_ERROR(pQuestInfo);

                KGLOG_PROCESS_ERROR(pQuestInfo->bAccept);
                KGLOG_PROCESS_ERROR(pQuestInfo->bRepeat);

                QuestGroupInfo.push_back(dwQuestID);
            }
        
            m_RandomQuestGroup[nID] = QuestGroupInfo;
        }
    }
    
Exit1:
    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

DWORD KQuestInfoList::GetRandomQuest(int nHeroLevel, int nRandomQuestIndex)
{
    DWORD dwResult = 0;
    KMAP_LEVEL2QUESTGROUP::iterator it;
    KQUEST_GROUP* pQuestGroup = NULL;
    int nLevel = (nHeroLevel - 1) / RANDOM_QUEST_LEVEL + 1;
    unsigned int nOldSeed = g_GetRandomSeed();
    unsigned int nRandSeed = nRandomQuestIndex + g_pSO3World->m_nBaseTime;
    unsigned int nRandomQuest = 0;

    it = m_RandomQuestGroup.find(nLevel);
    KG_PROCESS_ERROR(it != m_RandomQuestGroup.end());

    pQuestGroup = &it->second;
    KGLOG_PROCESS_ERROR(pQuestGroup->size() > 0);

    g_RandomSeed(nRandSeed);
    nRandomQuest = g_Random(pQuestGroup->size());
    g_RandomSeed(nOldSeed);

    dwResult = pQuestGroup->at(nRandomQuest);
Exit0:
    return dwResult;
}

BOOL KQuestInfoList::Reload()
{
    BOOL        bResult     = false;
    int         nRetCode    = false;
    int         nHeight     = 0;
    ITabFile*   piTabFile   = NULL;
    char        szFilePath[MAX_PATH];
    DWORD       dwQuestID    = 0;
    KQuestInfo* pQuestInfo  = NULL;

    nRetCode = (int)snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, QUEST_FILE_NAME);
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(szFilePath));

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);

    nRetCode = LoadQuestInfo(piTabFile, 2, &m_DefaultQuestInfo);
    KGLOG_PROCESS_ERROR(nRetCode);

    m_DailyQuest.clear();
    m_RandomQuestGroup.clear();

    for (int nIndex = 3; nIndex <= nHeight; nIndex++)
    {
        KQuestInfo	TempQuestInfo;

        nRetCode = piTabFile->GetInteger(nIndex, "QuestID", m_DefaultQuestInfo.dwQuestID, (int*)&dwQuestID);
        KGLOG_PROCESS_ERROR(nRetCode);
        KGLOG_PROCESS_ERROR(dwQuestID < MAX_QUEST_COUNT);

        pQuestInfo = GetQuestInfo(dwQuestID);

        if (pQuestInfo == NULL)
            pQuestInfo = &TempQuestInfo;

        nRetCode = LoadQuestInfo(piTabFile, nIndex, pQuestInfo);
        KGLOG_PROCESS_ERROR(nRetCode);

        m_mapID2QuestInfo[dwQuestID] = *pQuestInfo;

        if (pQuestInfo->bDaily)
            m_DailyQuest.push_back(pQuestInfo->dwQuestID);
    }

    nRetCode = LoadRandomQuestGroup();
    KGLOG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}