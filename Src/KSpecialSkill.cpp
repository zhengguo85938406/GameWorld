#include "stdafx.h"
#include "KSpecialSkill.h"

BOOL KSpecialSkill::Load()
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KSpecialSkillInfo Info;
    ITabFile* piTabFile = NULL;
    char szFilePath[MAX_PATH];
    int nHeight = 0;
    std::pair<KSPECIAL_SKILL_MAP::iterator, bool> InsRet;

    snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, "special_skill.tab");
    szFilePath[sizeof(szFilePath) - 1] = '\0';

    piTabFile = g_OpenTabFile(szFilePath);
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();
    for (int nRowIndex = 2; nRowIndex <= nHeight; nRowIndex++)
    {
        memset(&Info, 0, sizeof(Info));

        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)(&Info.dwID));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CostAngry", 0, &Info.nCostAngry);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CostStamina", 0, &Info.nCostStamina);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CostAngryPercent", 0, &Info.nCostAngryPercent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CostStaminaPercent", 0, &Info.nCostStaminaPercent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "FlyTrack", 0, &Info.nFlyTrack);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Damage", 0, &Info.nDamage);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "DamageRange", 0, &Info.nDamageRange);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "RequireLevel", 0, &Info.nRequireLevel);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AddBuff", 0, (int*)&Info.dwAddBuff);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CastSkill", 0, (int*)&Info.dwCastSkill);
        KGLOG_PROCESS_ERROR(bRetCode);

        InsRet = m_SpecialSkill.insert(std::make_pair(Info.dwID, Info));
        KGLOG_PROCESS_ERROR(InsRet.second);
    }

	bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
	return bResult;
}

KSpecialSkillInfo*  KSpecialSkill::Get(uint32_t dwID)
{
    KSPECIAL_SKILL_MAP::iterator it = m_SpecialSkill.find(dwID);
    if (it != m_SpecialSkill.end())
        return &it->second;
    return NULL;
}
