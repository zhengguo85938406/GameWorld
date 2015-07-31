#include "stdafx.h"
#include "KAttribute.h"
#include "KGEnumConvertor.h"

BOOL LoadAttribute(ITabFile* piFile, int nRow, const char* pszKey, int nIndex, KAttribData *pRetData)
{
    BOOL bResult = FALSE;

    int nRetCode = false;
    int nKeyID   = atInvalid;
    char szKey[64];
    char szKAttributeType[64] = {0};

    ASSERT(piFile);
    ASSERT(pszKey);
    ASSERT(pRetData);
    
    memset(pRetData, 0, sizeof(KAttribData));

    nRetCode = snprintf(szKey, sizeof(szKey), "%s%dKey", pszKey, nIndex);
    KGLOG_PROCESS_ERROR(nRetCode < sizeof(szKey));
    szKey[sizeof(szKey) - 1] = '\0';
    
    szKAttributeType[0] = '\0';
    nRetCode = piFile->GetString(nRow, szKey, "", szKAttributeType, sizeof(szKAttributeType));
    KG_PROCESS_ERROR(nRetCode != 0);

    pRetData->nKey = atInvalid;
    if (szKAttributeType[0] != '\0')
    {
        nRetCode = ENUM_STR2INT(ATTRIBUTE_TYPE, szKAttributeType, nKeyID);
        KG_PROCESS_ERROR(nRetCode);
        pRetData->nKey = nKeyID;
    }
    
    nRetCode = snprintf(szKey, sizeof(szKey), "%s%dValue1", pszKey, nIndex);
    KGLOG_PROCESS_ERROR(nRetCode < sizeof(szKey));
    szKey[sizeof(szKey) - 1] = '\0';
    nRetCode = piFile->GetInteger(nRow, szKey, 0, &pRetData->nValue1);
    KGLOG_PROCESS_ERROR(nRetCode != 0);

    nRetCode = snprintf(szKey, sizeof(szKey), "%s%dValue2", pszKey, nIndex);
    KGLOG_PROCESS_ERROR(nRetCode < sizeof(szKey));
    szKey[sizeof(szKey) - 1] = '\0';
    nRetCode = piFile->GetInteger(nRow, szKey, 0, &pRetData->nValue2);
    KGLOG_PROCESS_ERROR(nRetCode != 0);

    bResult = TRUE;
Exit0:
    return bResult;
}


BOOL CloneAttribListToEnd(KAttribute** ppDestAttribList, KAttribute* pCloneAttribList)
{
    BOOL            bResult         = false;
    KAttribute*     pLoopAttribute  = NULL;
    KAttribute*     pTmpAttribList  = NULL;
    KAttribute**    ppLastAttribute = NULL;

    assert(ppDestAttribList);

    ppLastAttribute = &pTmpAttribList;
    for (pLoopAttribute = pCloneAttribList; pLoopAttribute != NULL; pLoopAttribute = pLoopAttribute->pNext)
    {
        KAttribute* pTmpAttrib = NULL;

        pTmpAttrib = KMEMORY_NEW(KAttribute);
        KGLOG_PROCESS_ERROR(pTmpAttrib);

        pTmpAttrib->nKey    = pLoopAttribute->nKey;
        pTmpAttrib->nValue1 = pLoopAttribute->nValue1;
        pTmpAttrib->nValue2 = pLoopAttribute->nValue2;
        pTmpAttrib->pNext   = NULL;

        *ppLastAttribute    = pTmpAttrib;
        ppLastAttribute     = &(pTmpAttrib->pNext);
        pTmpAttrib          = NULL;
    }

    if (!(*ppDestAttribList))
    {
        *ppDestAttribList    = pTmpAttribList;
        pTmpAttribList      = NULL;
        goto Exit1;
    }

    pLoopAttribute = *ppDestAttribList;
    while (pLoopAttribute->pNext)
    {
        pLoopAttribute = pLoopAttribute->pNext;
    }
    pLoopAttribute->pNext = pTmpAttribList;
    pTmpAttribList = NULL;

Exit1:
    bResult = true;
Exit0:
    FREE_ATTRIB_LIST(pTmpAttribList);
    return bResult;
}
