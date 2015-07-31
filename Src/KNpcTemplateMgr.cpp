#include "stdafx.h"
#include "KNpcTemplateMgr.h"


KNpcTemplateMgr::KNpcTemplateMgr(void)
{
}


KNpcTemplateMgr::~KNpcTemplateMgr(void)
{
}

BOOL KNpcTemplateMgr::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
    bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KNpcTemplateMgr::UnInit()
{
    m_Data.clear();
}

KNpcTemplate* KNpcTemplateMgr::GetTemplate(DWORD dwTemplateID)
{
    KNpcTemplate* pResult = NULL;
    std::map<DWORD, KNpcTemplate>::iterator it = m_Data.find(dwTemplateID);

    KG_PROCESS_ERROR(it != m_Data.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

BOOL KNpcTemplateMgr::LoadData()
{
	BOOL        bResult     = false;
	BOOL        bRetCode    = false;
	ITabFile*   piTableFile = NULL;
    int         nHeight     = 0;
    KNpcTemplate* pDataItem = NULL;
    DWORD       dwID        = ERROR_ID;
    char        szTemp[MAX_PATH] = "";
    char        szValue[128];
    const char* pszActiveSkillColumnName[KACTIVE_SLOT_TYPE_TOTAL] = 
    {
        "BaseFistSkill",
        "BaseFootSkill",
        "PedimentFistSkill",
        "PedimentFootSkill",
        "AirFistSkill",
        "AirFootSkill",
        "RiseFistSkill",
        "RiseFootSkill",
    };
	
    piTableFile = g_OpenTabFile(SETTING_DIR"/NpcList.tab");
    KGLOG_PROCESS_ERROR(piTableFile);

    nHeight = piTableFile->GetHeight();
    for (int nRow = 2; nRow <= nHeight; ++nRow)
    {
        bRetCode = piTableFile->GetInteger(nRow, "ID", ERROR_ID, (int*)&dwID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        pDataItem = &m_Data[dwID];
        memset(pDataItem, 0, sizeof(*pDataItem));

        pDataItem->dwID = dwID;

        TABLEFILE_GET_STRING(Name);
        TABLEFILE_GET_DWORD(RepresentID);
        TABLEFILE_GET_DWORD(SkinID);
        TABLEFILE_GET_INT(Level);
        TABLEFILE_GET_INT(AILevel);
        TABLEFILE_GET_INT(MaxAILevel);
        TABLEFILE_GET_INT(MinAILevel);
        TABLEFILE_GET_INT(MaxEndurance);
        TABLEFILE_GET_INT(EnduranceReplenish);
        TABLEFILE_GET_INT(EnduranceReplenishPercent);
        TABLEFILE_GET_INT(MaxStamina);
        TABLEFILE_GET_INT(StaminaReplenish);
        TABLEFILE_GET_INT(StaminaReplenishPercent);
        TABLEFILE_GET_INT(MaxAngry);
        TABLEFILE_GET_INT(AdditionalRecoveryAngry);
        TABLEFILE_GET_INT(WillPower);
        TABLEFILE_GET_INT(Interference);
        TABLEFILE_GET_INT(InterferenceRange);
        TABLEFILE_GET_INT(AddInterferenceRangePercent);    
        TABLEFILE_GET_INT(AttackPoint);
        TABLEFILE_GET_INT(AttackPointPercent);
        TABLEFILE_GET_INT(Agility);
        TABLEFILE_GET_INT(CritPoint);
        TABLEFILE_GET_INT(CritRate);
        TABLEFILE_GET_INT(Defense);
        TABLEFILE_GET_INT(DefensePercent);

        TABLEFILE_GET_INT(MoveSpeedBase);
        pDataItem->nMoveSpeedBase /= GAME_FPS;

        TABLEFILE_GET_INT(AddMoveSpeedPercent);

        TABLEFILE_GET_INT(JumpSpeedBase);
        pDataItem->nJumpSpeedBase /= GAME_FPS;

        TABLEFILE_GET_INT(JumpSpeedPercent);
        
        TABLEFILE_GET_INT(HitRate);
        TABLEFILE_GET_INT(NormalShootForce);
        TABLEFILE_GET_INT(SkillShootForce);
        TABLEFILE_GET_INT(SlamDunkForce);
        TABLEFILE_GET_INT(Length);
        TABLEFILE_GET_INT(Width);
        TABLEFILE_GET_INT(Height);

        TABLEFILE_GET_DWORD(AIGroupID);

        bRetCode = piTableFile->GetInteger(nRow, "EventLabel", 0, &pDataItem->nEventLabel);
        KGLOG_PROCESS_ERROR(bRetCode);

        for (int i = 0; i < MAX_ATTACK_AWARD; ++i)
        {
            sprintf(szTemp, "AttackAwardHP%d", i + 1);
            bRetCode = piTableFile->GetInteger(nRow, szTemp, 0, &pDataItem->nAttackAwardHP[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
            
            sprintf(szTemp, "AttackAwardMoney%d", i + 1);
            bRetCode = piTableFile->GetInteger(nRow, szTemp, 0, &pDataItem->nAttackAwardMoney[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        for (int i = 0; i < countof(pszActiveSkillColumnName); ++i)
        {
            bRetCode = piTableFile->GetInteger(nRow, pszActiveSkillColumnName[i], 0,  (int*)&pDataItem->ActiveSkill[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        for (int i = 0; i < countof(pDataItem->PassiveSkill); ++i)
        {
            sprintf(szTemp, "PassiveSkill%d", i + 1);
            bRetCode = piTableFile->GetInteger(nRow, szTemp, 0, (int*)&pDataItem->PassiveSkill[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        for (int i = 0; i < countof(pDataItem->NirvanaSkill); ++i)
        {
            sprintf(szTemp, "NirvanaSkill%d", i + 1);
            bRetCode = piTableFile->GetInteger(nRow, szTemp, 0, (int*)&pDataItem->NirvanaSkill[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        for (int i = 0; i < countof(pDataItem->SlamBallSkill); ++i)
        {
            sprintf(szTemp, "SlamBallSkill%d", i + 1);
            bRetCode = piTableFile->GetInteger(nRow, szTemp, 0, (int*)&pDataItem->SlamBallSkill[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        bRetCode = piTableFile->GetString(nRow, "AllOtherActiveSkill", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadSkillList(szValue, pDataItem->AllOtherActiveSkill);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(pDataItem->AllOtherActiveSkill.size() <= cdMaxOtherActiveSkillCount);
    }

	bResult = true;
Exit0:
    KG_COM_RELEASE(piTableFile);
	return bResult;
}

BOOL KNpcTemplateMgr::LoadSkillList(char cszSkills[], KVEC_DWORD& vecSkill)
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