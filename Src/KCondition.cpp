#include "stdafx.h"
#include "KCondition.h"

#include <algorithm>


KCondition::KCondition(void)
{
}


KCondition::~KCondition(void)
{
}

BOOL KCondition::InitFromString(char szConditonString[])
{
	BOOL    bResult  = false;
    BOOL    bRetCode = false;
    char*   pszValue = NULL;

	KGLOG_PROCESS_ERROR(szConditonString);

    pszValue = strtok(szConditonString, ";");
    while(pszValue)
    {
        bRetCode = LoadConditionItem(pszValue);
        KGLOG_PROCESS_ERROR(bRetCode);

        pszValue = strtok(NULL, ";");
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCondition::Check(int nValue)
{
	BOOL bResult = false;
    BOOL bRetCode = false;

    for (KLIST_CONDITIONS::iterator it = m_lstConditions.begin(); it != m_lstConditions.end(); ++it)
    {
        bRetCode = (this->*(it->m_fnFunction))(nValue, it->m_vecCtrlValues);
        KG_PROCESS_ERROR(bRetCode);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCondition::Less(int nValue, KVEC_CTRLVALUES& vecCtrlValues)
{
	BOOL bResult = false;

	KGLOG_PROCESS_ERROR(vecCtrlValues.size() == 1);

    KG_PROCESS_ERROR(nValue < vecCtrlValues[0]);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCondition::Greater(int nValue, KVEC_CTRLVALUES& vecCtrlValues)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(vecCtrlValues.size() == 1);

    KG_PROCESS_ERROR(nValue > vecCtrlValues[0]);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KCondition::Equal(int nValue, KVEC_CTRLVALUES& vecCtrlValues)
{
    return !NotEqual(nValue, vecCtrlValues);
}

BOOL KCondition::NotEqual(int nValue, KVEC_CTRLVALUES& vecCtrlValues)
{
    BOOL bResult = false;
    KVEC_CTRLVALUES::iterator it;

    KGLOG_PROCESS_ERROR(vecCtrlValues.size());

    it = std::find(vecCtrlValues.begin(), vecCtrlValues.end(), nValue);
    KG_PROCESS_ERROR(it == vecCtrlValues.end());

    bResult = true;
Exit0:
    return bResult;
}

BOOL KCondition::LoadConditionItem(char* pszValue)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int  nValue   = 0;
	char cLogic   = '=';
    char cSeperator;
    KVEC_CTRLVALUES vecCtrlValues;

    KGLOG_PROCESS_ERROR(pszValue);
    KG_PROCESS_ERROR(pszValue[0]);

    if (pszValue[0] < '0' || pszValue[0] > '9')
    {
        bRetCode = g_ReadCharFromCString(pszValue, cLogic);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    while (true)
    {
        bRetCode = g_ReadIntegerFromCString(pszValue, nValue);
        KGLOG_PROCESS_ERROR(bRetCode);

        vecCtrlValues.push_back(nValue);

        if (pszValue[0] == NULL)
            break;

        bRetCode = g_ReadCharFromCString(pszValue, cSeperator);
        KGLOG_PROCESS_ERROR(cSeperator == ',');
    }
	
    bRetCode = InsertItem(cLogic, vecCtrlValues);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KCondition::InsertItem(char cLogic, KVEC_CTRLVALUES& vecCtrlValues)
{
	BOOL bResult = false;
    FNCONDITON_CHECKER fnFunction;

    switch(cLogic)
    {
    case '>':
        KGLOG_PROCESS_ERROR(vecCtrlValues.size() == 1);
        fnFunction = &KCondition::Greater;
        break;
    case '<':
        KGLOG_PROCESS_ERROR(vecCtrlValues.size() == 1);
        fnFunction = &KCondition::Less;
        break;
    case '=':
        KGLOG_PROCESS_ERROR(vecCtrlValues.size());
        fnFunction = &KCondition::Equal;
        break;
    case '!':
        KGLOG_PROCESS_ERROR(vecCtrlValues.size());
        fnFunction = &KCondition::NotEqual;
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }
	
    m_lstConditions.push_back(KCONDITION_ITEM(fnFunction, vecCtrlValues));

	bResult = true;
Exit0:
	return bResult;
}


