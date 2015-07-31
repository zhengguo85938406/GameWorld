#include "stdafx.h"
#include "KRecipeData.h"
#include "KGEnumConvertor.h"


KRecipeData::KRecipeData(void)
{
    memset(m_Data, 0, sizeof(m_Data));
}


KRecipeData::~KRecipeData(void)
{
}

BOOL KRecipeData::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KRecipeData::UnInit()
{
    memset(m_Data, 0, sizeof(m_Data));
}

KRecipe* KRecipeData::GetRecipe(DWORD dwRecipeID)
{
    KRecipe* pResult = NULL;
    KRecipe* pRecipe = NULL;

    KGLOG_PROCESS_ERROR(dwRecipeID > 0 && dwRecipeID <= countof(m_Data));
    
    pRecipe = &m_Data[dwRecipeID - 1];
    if (pRecipe->dwID)
        pResult = pRecipe;

Exit0:
    return pResult;
}

BOOL KRecipeData::LoadData()
{
	BOOL        bResult     = false;
	BOOL        bRetCode    = false;
	ITabFile*   piTableFile = NULL;
    int         nHeight     = 0;
    KRecipe*    pRecipe     = NULL;
    DWORD       dwID        = ERROR_ID;
    char        szColumnName[260] = "";
    char        szValue[260] = "";

    memset(m_Data, 0, sizeof(m_Data));

    piTableFile = g_OpenTabFile(SETTING_DIR"/Recipe.tab");
	KGLOG_PROCESS_ERROR(piTableFile);

    nHeight = piTableFile->GetHeight();
    for (int nRowIndex = 2; nRowIndex <= nHeight; ++nRowIndex)
    {
        bRetCode = piTableFile->GetInteger(nRowIndex, "ID", 0, (int*)&dwID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(dwID > 0 && dwID <= countof(m_Data));

        pRecipe         = &m_Data[dwID - 1];
        pRecipe->dwID   = dwID;

        bRetCode = piTableFile->GetString(nRowIndex, "Type", "", szValue, countof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        bRetCode = EnumStr2Int("RECIPE_TYPE", szValue, &pRecipe->nType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTableFile->GetInteger(nRowIndex, "RequireLevel", 0, &pRecipe->nRequireLevel);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(pRecipe->nRequireLevel > 0);

        bRetCode = piTableFile->GetInteger(nRowIndex, "AutoLearn", 0, &pRecipe->bAutoLearn);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTableFile->GetInteger(nRowIndex, "CostGoldCount", 0, &pRecipe->nCostGoldCount);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(pRecipe->nCostGoldCount >= 0);

        for (int i = 0; i < countof(pRecipe->Materials); ++i)
        {
            sprintf(szColumnName, "Material%d", i + 1);
            bRetCode = piTableFile->GetString(nRowIndex, szColumnName, "", szValue, countof(szValue));
            KGLOG_PROCESS_ERROR(bRetCode);

            if (bRetCode > 0)
            {
                bRetCode = g_StringToItemInfo(szValue, pRecipe->Materials[i].dwItemType, pRecipe->Materials[i].dwItemIndex, pRecipe->Materials[i].nCount);
                KGLOG_PROCESS_ERROR(bRetCode);
            }
        }

        for (int i = 0; i < countof(pRecipe->Materials); ++i)
        {
            if (!pRecipe->Materials[i].dwItemType)
                continue;

            for (int j = i + 1; j < countof(pRecipe->Materials); ++j)
            {
                if (pRecipe->Materials[i].dwItemType == pRecipe->Materials[j].dwItemType &&
                    pRecipe->Materials[i].dwItemIndex == pRecipe->Materials[j].dwItemIndex)
                {
                    pRecipe->Materials[i].nCount += pRecipe->Materials[j].nCount;
                    memset(&pRecipe->Materials[j], 0, sizeof(pRecipe->Materials[j]));
                }
            }
        }

        bRetCode = piTableFile->GetString(nRowIndex, "ProductItem", "", szValue, countof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        bRetCode = g_StringToItemInfo(szValue, pRecipe->Product.dwItemType, pRecipe->Product.dwItemIndex, pRecipe->Product.nCount);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTableFile->GetInteger(nRowIndex, "LowerValuePoint", 0, &pRecipe->nLowerValuePoint);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTableFile->GetInteger(nRowIndex, "UpperValuePoint", 0, &pRecipe->nUpperValuePoint);
        KGLOG_PROCESS_ERROR(bRetCode);

        KGLOG_PROCESS_ERROR(pRecipe->nUpperValuePoint >= pRecipe->nLowerValuePoint && pRecipe->nLowerValuePoint >= 0);
    }

	bResult = true;
Exit0:
    KG_COM_RELEASE(piTableFile);
	return bResult;
}

