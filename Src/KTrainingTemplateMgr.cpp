#include "stdafx.h"
#include "KTrainingTemplateMgr.h"
#include "KTabConfig.h"

KTrainingTemplateMgr::KTrainingTemplateMgr(void)
{
}


KTrainingTemplateMgr::~KTrainingTemplateMgr(void)
{
}

BOOL KTrainingTemplateMgr::Init()
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	
    bRetCode = m_TrainTime.Load("/Gym/Training.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_TrainExp.Load("/Gym/TrainingExp.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KTrainingTemplateMgr::UnInit()
{
}

BOOL KTrainingTemplateMgr::HasTemplate(DWORD dwTemplateID)
{
    return NULL != m_TrainTime.GetByID(dwTemplateID);
}

int KTrainingTemplateMgr::GetTrainSeconds(DWORD dwTemplateID)
{
    int nResult = 0;
    KTraining* pTrainingTime = NULL;

    KGLOG_PROCESS_ERROR(dwTemplateID != ERROR_ID);

    pTrainingTime = m_TrainTime.GetByID(dwTemplateID);
    KGLOG_PROCESS_ERROR(pTrainingTime);

    nResult = pTrainingTime->nMiniutes * SECONDS_PER_MINUTE;
Exit0:
    return nResult;
}

BOOL KTrainingTemplateMgr::GetTemplate(DWORD dwTemplateID, int nLevel, KTRAINING_TEMPLATE& Exp)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KTraining* pTrainingTime = NULL;
    KTrainingExp* pTrainingExp = NULL;

    KGLOG_PROCESS_ERROR(dwTemplateID != ERROR_ID);

    pTrainingTime = m_TrainTime.GetByID(dwTemplateID);
    KGLOG_PROCESS_ERROR(pTrainingTime);

    pTrainingExp = m_TrainExp.GetByID(nLevel);
    KGLOG_PROCESS_ERROR(pTrainingExp);

    Exp.m_dwID = dwTemplateID;
    Exp.m_nMiniutes = pTrainingTime->nMiniutes;
    Exp.m_nBaseExp = pTrainingExp->nExp * pTrainingTime->nMiniutes / MINUTES_PER_HOUR;
    Exp.m_nCostMeony = pTrainingExp->nCostMoney * pTrainingTime->nMiniutes / MINUTES_PER_HOUR;

    KGLOG_PROCESS_ERROR(Exp.m_nBaseExp > 0);
    KGLOG_PROCESS_ERROR(Exp.m_nCostMeony > 0);
    
    bResult = true;
Exit0:
    return bResult;
}

