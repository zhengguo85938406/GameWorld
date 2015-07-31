// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KTrainingTemplateMgr.h
//  Creator 	: hanruofei 
//  Date		: 2012-4-12
//  Comment	: 
// ***************************************************************

#pragma once
#include "KTabConfig.h"
struct KTRAINING_TEMPLATE
{
    DWORD   m_dwID;
    int     m_nMiniutes;    
    int     m_nBaseExp;
    int     m_nCostMeony;
};

class KTrainingTemplateMgr
{
public:
    KTrainingTemplateMgr(void);
    ~KTrainingTemplateMgr(void);

    BOOL Init();
    void UnInit();

    BOOL HasTemplate(DWORD dwTemplateID);
    int  GetTrainSeconds(DWORD dwTemplateID);
    BOOL GetTemplate(DWORD dwTemplateID, int nLevel, KTRAINING_TEMPLATE& Exp);

private:
    KReadTab<KTraining>     m_TrainTime;
    KReadTab<KTrainingExp>  m_TrainExp;
};

