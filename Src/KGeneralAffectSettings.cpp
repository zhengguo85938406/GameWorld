#include "stdafx.h"
#include "KGeneralAffectSettings.h"
#include "KGEnumConvertor.h"


KGeneralAffectSettings::KGeneralAffectSettings(void)
{
}


KGeneralAffectSettings::~KGeneralAffectSettings(void)
{
}


BOOL KGeneralAffectSettings::Init()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KGeneralAffectSettings::UnInit()
{
    m_mapGeneralAffect.clear();
}

KGeneralAffect* KGeneralAffectSettings::GetGeneralAffect(DWORD dwID)
{
    KGeneralAffect* pResult = NULL;
    KMAP_GENERALAFFECT::iterator it;

    it = m_mapGeneralAffect.find(dwID);
    if (it != m_mapGeneralAffect.end())
        pResult = &it->second;

    return pResult;
}

BOOL KGeneralAffectSettings::LoadData()
{
    BOOL			bResult		= false;
    BOOL			bRetCode	= false;
    ITabFile*		piTabFile	= NULL;
    int				nHeight		= 0;
    DWORD			dwID		= ERROR_ID;
    int				MAX_ATTRIBUTE_COUNT = 8;
    char			szAttributeKeyName[64];
    char			szColumnName[64];
    KAttribData	cAttribute;
    KGeneralAffect* pDataItem = NULL;

    piTabFile = g_OpenTabFile(SETTING_DIR"/GeneralAffect.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();
    for(int nRow = 2; nRow <= nHeight; ++nRow)
    {
        bRetCode = piTabFile->GetInteger(nRow, "ID", ERROR_ID, (int*)&dwID);
        KGLOG_PROCESS_ERROR(bRetCode>0);
        KGLOG_PROCESS_ERROR(dwID);

        pDataItem = &m_mapGeneralAffect[dwID];
        pDataItem->dwID = dwID;

        bRetCode = piTabFile->GetInteger(nRow, "BuffID", 0, (int*)&pDataItem->dwBuffID);
        KGLOG_PROCESS_ERROR(bRetCode>0);

        bRetCode = piTabFile->GetInteger(nRow, "CostEndurance", 0, &pDataItem->nCostEndurance);
        KGLOG_PROCESS_ERROR(bRetCode>0);

        bRetCode = piTabFile->GetInteger(nRow, "CostEndurancePercent", 0, &pDataItem->nCostEndurancePercent);
        KGLOG_PROCESS_ERROR(bRetCode>0);

        bRetCode = piTabFile->GetInteger(nRow, "CostStamina", 0, &pDataItem->nCostStamina);
        KGLOG_PROCESS_ERROR(bRetCode>0);

        bRetCode = piTabFile->GetInteger(nRow, "CostStaminaPercent", 0, &pDataItem->nCostStaminaPercent);
        KGLOG_PROCESS_ERROR(bRetCode>0);

        pDataItem->vecAttributes.reserve(MAX_ATTRIBUTE_COUNT);
        for(int i = 0; i < MAX_ATTRIBUTE_COUNT; ++i)
        {
            sprintf(szColumnName, "Attribute%dKey", i + 1);
            bRetCode = piTabFile->GetString(nRow, szColumnName, "", szAttributeKeyName, countof(szAttributeKeyName));
            KGLOG_PROCESS_ERROR(bRetCode);

            if (bRetCode < 0)
                continue;

            bRetCode = ENUM_STR2INT(ATTRIBUTE_TYPE, szAttributeKeyName, cAttribute.nKey);
            KGLOG_PROCESS_ERROR(bRetCode);

            sprintf(szColumnName, "Attribute%dValue1", i + 1);
            bRetCode = piTabFile->GetInteger(nRow, szColumnName, 0, &cAttribute.nValue1);
            KGLOG_PROCESS_ERROR(bRetCode);

            sprintf(szColumnName, "Attribute%dValue2", i + 1);
            bRetCode = piTabFile->GetInteger(nRow, szColumnName, 0, &cAttribute.nValue2);
            KGLOG_PROCESS_ERROR(bRetCode);

            pDataItem->vecAttributes.push_back(cAttribute);
        }

        bRetCode = piTabFile->GetInteger(nRow, "StiffFrame", 0, &pDataItem->nStiffFrame);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}
