#include "stdafx.h"
#include "KSlamBallSkillSettings.h"


KSlamBallSkillSettings::KSlamBallSkillSettings(void)
{
}


KSlamBallSkillSettings::~KSlamBallSkillSettings(void)
{
}

BOOL KSlamBallSkillSettings::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KSlamBallSkillSettings::UnInit()
{
    m_mapSlamBallSkill.clear();
}

KSlamBallSkill* KSlamBallSkillSettings::Get(DWORD dwSlamBallSkillID)
{
    KSlamBallSkill* pResult = NULL;
    KMAP_SLAMBALL_SKILL::iterator it;

    it = m_mapSlamBallSkill.find(dwSlamBallSkillID);
    KGLOG_PROCESS_ERROR(it != m_mapSlamBallSkill.end());

    pResult = &it->second;

Exit0:
    return pResult;
}

BOOL KSlamBallSkillSettings::LoadData()
{
	BOOL 		bResult 	= false;
	BOOL 		bRetCode 	= false;
	int  		nHeight 	= 0;
	ITabFile* 	piTabFile 	= NULL;
    DWORD       dwID        = ERROR_ID;
    KSlamBallSkill* pSlamBallSkill = NULL;

	piTabFile = g_OpenTabFile(SETTING_DIR"/SlamBallSkill.tab");
	KGLOG_PROCESS_ERROR(piTabFile);

	nHeight = piTabFile->GetHeight();
	for(int nRow = 2; nRow <= nHeight; ++nRow)
	{
		bRetCode = piTabFile->GetInteger(nRow, "ID", 0, (int*)&dwID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(dwID > 0);

        pSlamBallSkill = &m_mapSlamBallSkill[dwID];

        memset(pSlamBallSkill, 0, sizeof(*pSlamBallSkill));
        pSlamBallSkill->dwID = dwID;

        bRetCode = piTabFile->GetInteger(nRow, "RequireLevel", 0, &pSlamBallSkill->nRequireLevel);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "HighestTargetBasket", 0,  &pSlamBallSkill->nHighestTargetBasket);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(pSlamBallSkill->nHighestTargetBasket > 0 && pSlamBallSkill->nHighestTargetBasket <= 3);

        bRetCode = piTabFile->GetInteger(nRow, "CostBasketHP", 0, &pSlamBallSkill->nCostBasketHP);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "AddAngry", 0, &pSlamBallSkill->nAddAngry);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "AdditionalHitRate", 0, &pSlamBallSkill->nAdditionalHitRate);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "IgnoreInterference", 0, &pSlamBallSkill->bIgnoreInterference);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "BaTi", 0, &pSlamBallSkill->bBaTi);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "RandomMinVX", 0, &pSlamBallSkill->nRandomMinVX);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "RandomMaxVX", 0, &pSlamBallSkill->nRandomMaxVX);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "RandomMinVY", 0, &pSlamBallSkill->nRandomMinVY);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "RandomMaxVY", 0, &pSlamBallSkill->nRandomMaxVY);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "RandomMinVZ", 0, &pSlamBallSkill->nRandomMinVZ);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRow, "RandomMaxVZ", 0, &pSlamBallSkill->nRandomMaxVZ);
        KGLOG_PROCESS_ERROR(bRetCode);

        KGLOG_PROCESS_ERROR(pSlamBallSkill->nRandomMinVX >= 0 && pSlamBallSkill->nRandomMinVX <= pSlamBallSkill->nRandomMaxVX);
        KGLOG_PROCESS_ERROR(pSlamBallSkill->nRandomMinVY <= pSlamBallSkill->nRandomMaxVY);
        KGLOG_PROCESS_ERROR(pSlamBallSkill->nRandomMinVZ >= 0 && pSlamBallSkill->nRandomMinVZ <= pSlamBallSkill->nRandomMaxVZ);
	}

	bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}
