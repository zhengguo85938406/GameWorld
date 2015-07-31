#include "stdafx.h"
#include "KMissionMgr.h"
#include "KPlayer.h"

KMissionMgr::KMissionMgr()
{
}

KMissionMgr::~KMissionMgr()
{
}

BOOL KMissionMgr::Init()
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;

    bRetCode = Load();
    KGLOG_PROCESS_ERROR(bRetCode);
    
    bResult = true;
Exit0:
    return bResult;
}

void KMissionMgr::UnInit()
{
}

BOOL KMissionMgr::CanEnterMission(KPlayer* pPlayer, int eType, int nStep, int nLevel)
{
    BOOL    bResult 	    = false;
    BOOL    bRetCode 	    = false;
    int     nMainHeroLevel  = 0;
    KMission* pMission = NULL;

    assert(pPlayer);

    KGLOG_PROCESS_ERROR(!pPlayer->m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(!pPlayer->m_bIsMatching);
    
    bRetCode = pPlayer->m_MissionData.IsMissionOpened(eType, nStep, nLevel);
    KGLOG_PROCESS_ERROR(bRetCode);

    pMission = GetMission(eType, nStep, nLevel);
    KGLOG_PROCESS_ERROR(pMission);

    nMainHeroLevel = pPlayer->GetMainHeroLevel();
    KGLOG_PROCESS_ERROR(nMainHeroLevel >= pMission->nRequireLevel);
    
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

KMission* KMissionMgr::GetMission(int eType, int nStep, int nLevel)
{
    KMIOSSION_TABLE::iterator it = m_mapMissions.find(GetMissionKey(eType, nStep, nLevel));
    KG_PROCESS_ERROR(it != m_mapMissions.end());

    return &it->second;
Exit0:
    return NULL;
}

uint32_t KMissionMgr::GetMissionMap(int eType, int nStep, int nLevel)
{
    uint32_t dwResult = 0;
    KMIOSSION_TABLE::iterator it = m_mapMissions.find(GetMissionKey(eType, nStep, nLevel));
    KG_PROCESS_ERROR(it != m_mapMissions.end());

    dwResult = it->second.dwMapID;
Exit0:
    return dwResult;
}

KMission* KMissionMgr::GetMission(uint32_t dwMapID)
{
    KMIOSSION_TABLE::iterator it = m_mapMissions.begin();
    for (;it != m_mapMissions.end(); ++it)
    {
        if (it->second.dwMapID == dwMapID)
            return &it->second;
    }

    return NULL;
}

uint64_t KMissionMgr::GetMissionKey(int eType, int nStep, int nLevel)
{
    return MAKE_INT64(MAKE_INT32(eType, nStep), nLevel);
}

BOOL KMissionMgr::IsTrainingMission(uint32_t dwMapID)
{
    BOOL        bResult     = false;
    KMission*   pMission    = NULL;

    pMission = GetMission(dwMapID);
    KG_PROCESS_ERROR(pMission);
    
    KG_PROCESS_ERROR(pMission->eType == emitNewbie || pMission->eType == emitTrain);
	
	bResult = true;
Exit0:
    return bResult;
}

BOOL KMissionMgr::Load()
{
	BOOL        bResult     = false;
    BOOL        bRetCode    = false;
	ITabFile*	piTabFile   = NULL;
    KMission    Mission;
    char        szScript[MAX_PATH];
    char        szColumnName[MAX_PATH] = "";
    char        szValue[MAX_PATH] = "";

	piTabFile = g_OpenTabFile(SETTING_DIR"/Mission.tab");
	KGLOG_PROCESS_ERROR(piTabFile);

	for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
	{
        memset(&Mission, 0, sizeof(Mission));
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "Type", 0, (int*)&Mission.eType);   
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Step", 0, (int*)&Mission.nStep);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Level", 0, &Mission.nLevel);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

		bRetCode = piTabFile->GetString(nRowIndex, "Name", "", Mission.szName, sizeof(Mission.szName));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
		KGLOG_PROCESS_ERROR(Mission.szName[0] != '\0');

        bRetCode = piTabFile->GetInteger(nRowIndex, "MapID", 0, (int*)&Mission.dwMapID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "RequireLevel", 0, (int*)&Mission.nRequireLevel);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "ExpLevel", 0, (int*)&Mission.nExpLevel);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AwardList", ERROR_ID, (int*)&Mission.stAwardData.dwAwardListID[KAWARD_CARD_TYPE_NORMAL]);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "GoldList", ERROR_ID, (int*)&Mission.stAwardData.dwAwardListID[KAWARD_CARD_TYPE_GOLD]);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "VipList", ERROR_ID, (int*)&Mission.stAwardData.dwAwardListID[KAWARD_CARD_TYPE_VIP]);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "GoldCostMoney", 0, (int *)&Mission.stAwardData.nGoldCostMoney);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(Mission.stAwardData.nGoldCostMoney >= 0);
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "BaseExp", 0, (int*)&Mission.nBaseExp);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(Mission.nBaseExp >= 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BaseMoney", 0, (int*)&Mission.nBaseMoney);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(Mission.nBaseMoney >= 0);

        for (int i = 0; i < countof(Mission.Npc); ++i)
        {
            sprintf(szColumnName, "Npc%dID", i + 1);
            bRetCode = piTabFile->GetInteger(nRowIndex, szColumnName, ERROR_ID, (int*)&Mission.Npc[i].dwTemplateID);
            KGLOG_PROCESS_ERROR(bRetCode);

            sprintf(szColumnName, "Npc%dAIType", i + 1);
            bRetCode = piTabFile->GetInteger(nRowIndex, szColumnName, ERROR_ID, &Mission.Npc[i].nAIType);
            KGLOG_PROCESS_ERROR(bRetCode);

            sprintf(szColumnName, "Npc%dSide", i + 1);
            bRetCode = piTabFile->GetInteger(nRowIndex, szColumnName, 0, &Mission.Npc[i].nSide);
            KGLOG_PROCESS_ERROR(bRetCode);
            KGLOG_PROCESS_ERROR(Mission.Npc[i].nSide == sidRight || Mission.Npc[i].nSide == sidLeft);

            sprintf(szColumnName, "Npc%dPos", i + 1);
            bRetCode = piTabFile->GetString(nRowIndex, szColumnName, "0,0,0", szValue, countof(szValue));
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = g_StringToPosition(szValue, Mission.Npc[i].cPos);
            KGLOG_PROCESS_ERROR(bRetCode);
            KGLOG_PROCESS_ERROR(Mission.Npc[i].cPos.nX >= 0 && Mission.Npc[i].cPos.nY >= 0 && Mission.Npc[i].cPos.nZ >= 0);
        }

        bRetCode = piTabFile->GetString(nRowIndex, "Script", "", szScript, sizeof(szScript));
        KGLOG_PROCESS_ERROR(bRetCode);

        if (szScript[0] != '\0')
            Mission.dwScriptID = g_FileNameHash(szScript);

        bRetCode = piTabFile->GetInteger(nRowIndex, "UseAICoe", 0, &Mission.bUseAICoe);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AffectAICoe", 0, &Mission.bAffectAICoe);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CostFatiguePoint", 0, &Mission.nCostFatiguePoint);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "RequireQuest", 0, (int*)&Mission.dwRequireQuest);
        KGLOG_PROCESS_ERROR(bRetCode);

		bRetCode = piTabFile->GetInteger(nRowIndex, "UnenableReady", 0, &Mission.nUnenableReady);
		bRetCode = piTabFile->GetInteger(nRowIndex, "CanPause", 0, &Mission.nCanPause);

        m_mapMissions[MAKE_INT64(MAKE_INT32(Mission.eType, Mission.nStep), Mission.nLevel)] = Mission;
	}

    bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}
