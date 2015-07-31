// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KCondition.h
//  Creator		: hanruofei
//	Date		: 1/6/2013
//  Comment		: 
//	*********************************************************************
#pragma once
#include <vector>
#include <list>

#include "KTools.h"


class KCondition
{
public:
    KCondition(void);
    ~KCondition(void);

    BOOL InitFromString(char szConditonString[]);
    BOOL Check(int nValue);

private:
    typedef std::vector<int> KVEC_CTRLVALUES;
    typedef BOOL (KCondition::*FNCONDITON_CHECKER)(int, KVEC_CTRLVALUES&);


    struct KCONDITION_ITEM
    {
        KCONDITION_ITEM(FNCONDITON_CHECKER fnFunction, const KVEC_CTRLVALUES& vecCtrlValues)
            : m_fnFunction(fnFunction), m_vecCtrlValues(vecCtrlValues)
        {

        }
        FNCONDITON_CHECKER m_fnFunction;
        KVEC_CTRLVALUES m_vecCtrlValues;
    };

    typedef std::list<KCONDITION_ITEM> KLIST_CONDITIONS;

private:
    BOOL LoadConditionItem(char* pszValue);
    BOOL InsertItem(char cLogic, KVEC_CTRLVALUES& vecCtrlValues);
    BOOL Less    (int nValue, KVEC_CTRLVALUES& vecCtrlValues);
    BOOL Greater (int nValue, KVEC_CTRLVALUES& vecCtrlValues);
    BOOL Equal   (int nValue, KVEC_CTRLVALUES& vecCtrlValues);
    BOOL NotEqual(int nValue, KVEC_CTRLVALUES& vecCtrlValues);

private:
    KLIST_CONDITIONS m_lstConditions;
};

struct KEventCondition
{
    int nEvent;
    KCondition AllCondition[cdMaxEventParamCount];

    BOOL CheckCondition(const int (&nValue)[cdMaxEventParamCount])
    {
        BOOL  bResult = false;
        BOOL  bRetCode = false;

        for (int i = 0; i < countof(AllCondition); ++i)
        {
            bRetCode = AllCondition[i].Check(nValue[i]);
            KG_PROCESS_ERROR(bRetCode);
        }

        bResult = true;
    Exit0:
        return bResult;
    }

};