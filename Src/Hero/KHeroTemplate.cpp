#include "stdafx.h"
#include "KHeroTemplate.h"
#include "KGEnumConvertor.h"
#include "KPlayer.h"

KHeroTemplate::KHeroTemplate()
{
}

KHeroTemplate::~KHeroTemplate()
{
}

BOOL KHeroTemplate::Load()
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    ITabFile*	piTabFile   = NULL;
    char        szLevelDataFileName[_NAME_LEN];
    DWORD       dwHeroTemplateID = 0;
    std::pair<KMAP_HERO_TEMPLATE::iterator, bool> InsRet;
    KHeroTemplateInfo* pTemplateInfo = NULL;
    char        szUnLockType[64] = {0};
    int         nUnlockType   = UNLOCK_TYPE_CANNOT;
    const char* AllSkillTypeName[KACTIVE_SLOT_TYPE_TOTAL] = 
    {
        "AllBaseFistSkill",
        "AllBaseFootSkill",
        "AllPedimentFistSkill",
        "AllPedimentFootSkill",
        "AllAirFistSkill",
        "AllAirFootSkill",
        "AllRiseFistSkill",
        "AllRiseFootSkill",
    };

    char        szValue[128];

    piTabFile = g_OpenTabFile(SETTING_DIR"/HeroTemplate.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
	{
		bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)&dwHeroTemplateID);
		KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(dwHeroTemplateID > 0);

        InsRet = m_mapHeroTemplate.insert(std::make_pair(dwHeroTemplateID, KHeroTemplateInfo()));
        KGLOG_PROCESS_ERROR(InsRet.second);

        pTemplateInfo = &InsRet.first->second;

        pTemplateInfo->dwTemplateID = dwHeroTemplateID;

        bRetCode = piTabFile->GetString(nRowIndex, "Name", "", &pTemplateInfo->szName[0], sizeof(pTemplateInfo->szName));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "LevelData", "", &szLevelDataFileName[0], sizeof(szLevelDataFileName));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = LoadHeroLevelData(szLevelDataFileName, pTemplateInfo->HeroLevelData);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "InitBaseFistSkill", 0, (int*)&pTemplateInfo->dwInitBaseFistSkill);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(pTemplateInfo->dwInitBaseFistSkill > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "InitBaseFootSkill", 0, (int*)&pTemplateInfo->dwInitBaseFootSkill);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(pTemplateInfo->dwInitBaseFootSkill > 0);

        for (int i = 0; i < countof(AllSkillTypeName); ++i)
        {
            bRetCode = piTabFile->GetString(nRowIndex, AllSkillTypeName[i], "", szValue, sizeof(szValue));
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = LoadSkillList(szValue, pTemplateInfo->AllActiveSkill[i]);
            KGLOG_PROCESS_ERROR(bRetCode);

            KGLOG_PROCESS_ERROR(pTemplateInfo->AllActiveSkill[i].size());
        }

        bRetCode = piTabFile->GetString(nRowIndex, "AllPassiveSkill", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadSkillList(szValue, pTemplateInfo->AlllPassiveSkill);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "AllNirvanaSkill", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadSkillList(szValue, pTemplateInfo->AllNirvanaSkill);
        KGLOG_PROCESS_ERROR(bRetCode); 

        bRetCode = piTabFile->GetString(nRowIndex, "AllSlamBallSkill", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadSkillList(szValue, pTemplateInfo->AllSlamBallSkill);
        KGLOG_PROCESS_ERROR(bRetCode);
        
        bRetCode = piTabFile->GetString(nRowIndex, "AllOtherActiveSkill", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadSkillList(szValue, pTemplateInfo->AllOtherActiveSkill);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(pTemplateInfo->AllOtherActiveSkill.size() <= cdMaxOtherActiveSkillCount);

        szUnLockType[0] = '\0';
        bRetCode = piTabFile->GetString(nRowIndex, "UnLockType", "", szUnLockType, sizeof(szUnLockType));
        KG_PROCESS_ERROR(bRetCode != 0);

        pTemplateInfo->eUnlockType = UNLOCK_TYPE_CANNOT;
        if (szUnLockType[0] != '\0')
        {
            bRetCode = ENUM_STR2INT(UNLOCK_TYPE, szUnLockType, nUnlockType);
            KG_PROCESS_ERROR(bRetCode);
            pTemplateInfo->eUnlockType = (UNLOCK_TYPE)nUnlockType;
        }

        bRetCode = piTabFile->GetInteger(nRowIndex, "LockData", 0, (int*)&pTemplateInfo->dwLockData);
		KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BuyMoneyType", 0, &pTemplateInfo->nBuyMoneyType);
		KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BuyPrice", 0, &pTemplateInfo->nBuyPrice);
		KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CanFire", 0, &pTemplateInfo->nCanFire);
        KGLOG_PROCESS_ERROR(bRetCode);

		bRetCode = piTabFile->GetInteger(nRowIndex, "DisCountTime", 0, &pTemplateInfo->nDisCountTime);
		KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIGroupID", 0, (int*)&pTemplateInfo->dwAIGroupID);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "FashionIDs", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadFashionIDList(szValue, pTemplateInfo->AllFashionID);
        KGLOG_PROCESS_ERROR(bRetCode);

    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KHeroTemplate::LoadHeroLevelData(const char cszFileName[], KMAP_HERO_LEVEL_INFO& rHeroLevelInfo)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    ITabFile*	piTabFile   = NULL;
    char        szFilePath[MAX_PATH];

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s/%s", SETTING_DIR, "LevelUpData", cszFileName);
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        KHeroLevelInfo Info;

        bRetCode = piTabFile->GetInteger(nRowIndex, "Level", 0, (int*)&Info.nLevel);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Experience", 0, (int*)&Info.nExp);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MaxEndurance", 0, (int*)&Info.nMaxEndurance);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(Info.nMaxEndurance > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "EnduranceReplenish", 0, (int*)&Info.nEnduranceReplenish);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "EnduranceReplenishPercent", 0, (int*)&Info.nEndurancereplenishPercent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MaxStamina", 0, (int*)&Info.nMaxStamina);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(Info.nMaxStamina > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "StaminaReplenish", 0, (int*)&Info.nStaminaReplenish);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "StaminaReplenishPercent", 0, (int*)&Info.nStaminaReplenishPercent);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MaxAngry", 0, &Info.nMaxAngry);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(Info.nMaxAngry >= 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AdditionalAddAngry", 0, &Info.nAdditionalAddAngry);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "WillPower", 0, &Info.nWillPower);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Interference", 0, &Info.nInterference);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "InterferenceRange", 0, &Info.nInterferenceRange);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AddInterferenceRangePercent", 0, &Info.nAddInterferenceRangePercent);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AttackPoint", 0, &Info.nAttackPoint);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AttackPointPercent", 0, &Info.nAttackPointPercent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Agility", 0, &Info.nAgility);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CritPoint", 0, &Info.nCritPoint);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CritRate", 150, &Info.nCritRate);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Defense", 0, &Info.nDefense);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "DefensePercent", 0, &Info.nDefensePercent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MoveSpeed", 0, &Info.nMoveSpeed);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        Info.nMoveSpeed /= GAME_FPS;

        bRetCode = piTabFile->GetInteger(nRowIndex, "AddMoveSpeedPercent", 0, &Info.nAddMoveSpeedPercent);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "JumpingAbility", 0, &Info.nJumpingAbility);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        Info.nJumpingAbility /= GAME_FPS;

        bRetCode = piTabFile->GetInteger(nRowIndex, "AddJumpingAbilityPercent", 0, &Info.nJumpingAbilityPercent);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "ShootBallHitRate", 0, &Info.nShootBallHitRate);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "NormalShootForce", 0, &Info.nNormalShootForce);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(Info.nNormalShootForce >= 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "SkillShootForce", 0, &Info.nSkillShootForce);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(Info.nSkillShootForce > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "SlamDunkForce", 0, &Info.nSlamDunkForce);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(Info.nSlamDunkForce > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIType", 0, &Info.nAIType);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "DenAIType", 0, &Info.nDefenseAIType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Lenth", 512,  &Info.nLenth);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Width", 512,  &Info.nWidth);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Height", 1074,  &Info.nHeight);
        KGLOG_PROCESS_ERROR(bRetCode);

        rHeroLevelInfo[Info.nLevel] = Info;
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;    
}

KHeroTemplateInfo* KHeroTemplate::GetHeroTemplate(DWORD dwHeroTemplateID)
{
    KHeroTemplateInfo* pResult = NULL;
    KMAP_HERO_TEMPLATE::iterator itHeroTemplate;

    itHeroTemplate = m_mapHeroTemplate.find(dwHeroTemplateID);
    KG_PROCESS_ERROR(itHeroTemplate != m_mapHeroTemplate.end());

    pResult = &itHeroTemplate->second;
Exit0:
	return pResult;
}

KHeroLevelInfo* KHeroTemplate::GetHeroLevelInfo(DWORD dwHeroTemplateID, int nLevel)
{
    KHeroLevelInfo* pResult = NULL;
    KMAP_HERO_TEMPLATE::iterator itHeroTemplate;
    KMAP_HERO_LEVEL_INFO::iterator itHeroLevelInfo;

    itHeroTemplate = m_mapHeroTemplate.find(dwHeroTemplateID);
    KG_PROCESS_ERROR(itHeroTemplate != m_mapHeroTemplate.end());

    itHeroLevelInfo = itHeroTemplate->second.HeroLevelData.find(nLevel);
    KG_PROCESS_ERROR(itHeroLevelInfo != itHeroTemplate->second.HeroLevelData.end());

    pResult = &itHeroLevelInfo->second;
Exit0:
	return pResult;
}

const KMAP_HERO_TEMPLATE&	KHeroTemplate::GetHeroTemplateMap()
{
	return m_mapHeroTemplate;
}

BOOL KHeroTemplate::LoadSkillList(char cszSkills[], KVEC_DWORD& vecSkill)
{
	BOOL bResult  = false;
    int nRetCount = 0;
    DWORD dwSkillID = ERROR_ID;
	char* pSkilID = NULL;

	KGLOG_PROCESS_ERROR(cszSkills);

    pSkilID = strtok(cszSkills, ";");
    while(pSkilID)
    {
        nRetCount = sscanf(pSkilID, "%u", &dwSkillID);
        KGLOG_PROCESS_ERROR(nRetCount == 1);

        vecSkill.push_back(dwSkillID);

        pSkilID = strtok(NULL, ";");
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KHeroTemplate::LoadFashionIDList(char cszFashionIDs[], KVEC_DWORD& vecFashionIDs)
{
    BOOL bResult  = false;
    int nRetCount = 0;
    DWORD dwFashionID = ERROR_ID;
    char* pFashionID = NULL;

    KGLOG_PROCESS_ERROR(cszFashionIDs);

    pFashionID = strtok(cszFashionIDs, ";");
    while(pFashionID)
    {
        nRetCount = sscanf(pFashionID, "%u", &dwFashionID);
        KGLOG_PROCESS_ERROR(nRetCount == 1);

        vecFashionIDs.push_back(dwFashionID);

        pFashionID = strtok(NULL, ";");
    }

    bResult = true;
Exit0:
    return bResult;
}

void KHeroTemplate::ReportUnLockHero(KPlayer* pPlayer, int eUnlockType, DWORD dwLockData)
{
    KHeroTemplateInfo* pHeroTemplate = NULL;
    KMAP_HERO_TEMPLATE::iterator itHeroTemplate;
    KReport_Hero_UnLock param;
    BOOL    bRetCode = false;

    for (itHeroTemplate = m_mapHeroTemplate.begin(); itHeroTemplate != m_mapHeroTemplate.end(); ++itHeroTemplate)
    {
        pHeroTemplate = &itHeroTemplate->second;
        KGLOG_PROCESS_ERROR(pHeroTemplate);

        if (eUnlockType != pHeroTemplate->eUnlockType)
            continue;

        switch (pHeroTemplate->eUnlockType)
        {
        case UNLOCK_TYPE_QUEST:
        case UNLOCK_TYPE_ACHIEVEMENT:
            if (pHeroTemplate->dwLockData == dwLockData)
            {
                bRetCode = pPlayer->CheckUnLockHero(pHeroTemplate->dwTemplateID);
                if (bRetCode)
                {
                    param.heroTemplateID = pHeroTemplate->dwTemplateID;
                    pPlayer->m_Secretary.AddReport(KREPORT_EVENT_HERO_UNLOCK, (BYTE*)&param, sizeof(param));
                }
            }
            break;
        default:
            break;
        }
    }

Exit0:
    return;
}
