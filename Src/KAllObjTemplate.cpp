#include "stdafx.h"
#include "KAllObjTemplate.h"

KAllObjTemplate::KAllObjTemplate(void)
{
}

KAllObjTemplate::~KAllObjTemplate(void)
{
}

BOOL KAllObjTemplate::LoadAll()
{
    BOOL                        bResult                 = false;
    BOOL                        bRetCode                = false;
    int                         nHeight                 = 0;
    ITabFile*                   piTabFile               = NULL;
    KSceneObjectTemplate        cTemplate;
    pair<KMAP_SCENEOBJ_TEMPLATE::iterator, bool> InsRet;
    char                        szScript[MAX_PATH];
    char                        szColName[32];

    piTabFile = g_OpenTabFile(SETTING_DIR"/sceneobj_template.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();

    for (int nRowIndex = 2; nRowIndex <= nHeight; ++nRowIndex)
    {
        memset(&cTemplate, 0, sizeof(cTemplate));

        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)&cTemplate.m_dwID);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "nLength", 0, &cTemplate.m_nLength);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "nWidth", 0, &cTemplate.m_nWidth);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "nHeight", 0, &cTemplate.m_nHeight);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "eType", 0, &cTemplate.m_nType);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(cTemplate.m_nType > sotInvalid && cTemplate.m_nType < sotTotal);

        bRetCode = piTabFile->GetString(nRowIndex, "Script", "", szScript, sizeof(szScript));
        KGLOG_PROCESS_ERROR(bRetCode);
        cTemplate.m_dwScriptID = ERROR_ID;
        if (szScript[0] != '\0')
            cTemplate.m_dwScriptID = g_FileNameHash(szScript);

        bRetCode = piTabFile->GetInteger(nRowIndex, "nVXOnThrow", 0, &cTemplate.m_nVXOnThrow);
        KGLOG_PROCESS_ERROR(bRetCode);
        cTemplate.m_nVXOnThrow /= GAME_FPS;

        bRetCode = piTabFile->GetInteger(nRowIndex, "nVYOnThrow", 0, &cTemplate.m_nVYOnThrow);
        KGLOG_PROCESS_ERROR(bRetCode);
        cTemplate.m_nVYOnThrow /= GAME_FPS;

        bRetCode = piTabFile->GetInteger(nRowIndex, "nVZOnThrow", 0, &cTemplate.m_nVZOnThrow);
        KGLOG_PROCESS_ERROR(bRetCode);
        cTemplate.m_nVZOnThrow /= GAME_FPS;

        bRetCode = piTabFile->GetInteger(nRowIndex, "AttackAffectID1", 0, (int*)&cTemplate.m_dwAttackAffectID1);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AttackAffectID2", 0, (int*)&cTemplate.m_dwAttackAffectID2);
        KGLOG_PROCESS_ERROR(bRetCode);

        for (int i = 0; i < countof(cTemplate.m_nCustomParam); ++i)
        {
            snprintf(szColName, countof(szColName), "nCustomParam%d", i + 1);
            szColName[countof(szColName) - 1] = '\0';

            bRetCode = piTabFile->GetInteger(nRowIndex, szColName, 0, &cTemplate.m_nCustomParam[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        bRetCode = piTabFile->GetInteger(nRowIndex, "Impregnability", 0, &cTemplate.m_bImpregnability);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BeAttackedType", DEFAULT_ATTACK_TYPE, &cTemplate.m_nBeAttackedType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Obstacle", 0, &cTemplate.m_Obstacle);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "FireAIEvent", 0, &cTemplate.m_bFireAIEvent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMinX", 0, &cTemplate.m_nAIEventMinX);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMaxX", 0, &cTemplate.m_nAIEventMaxX);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMinY", 0, &cTemplate.m_nAIEventMinY);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMaxY", 0, &cTemplate.m_nAIEventMaxY);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMinZ", 0, &cTemplate.m_nAIEventMinZ);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMaxZ", 0, &cTemplate.m_nAIEventMaxZ);
        KGLOG_PROCESS_ERROR(bRetCode);

        KGLOG_PROCESS_ERROR(cTemplate.m_nAIEventMinX <= cTemplate.m_nAIEventMaxX);
        KGLOG_PROCESS_ERROR(cTemplate.m_nAIEventMinY <= cTemplate.m_nAIEventMaxY);
        KGLOG_PROCESS_ERROR(cTemplate.m_nAIEventMinZ <= cTemplate.m_nAIEventMaxZ);

        InsRet = m_mapData.insert(std::make_pair(cTemplate.m_dwID, cTemplate));
        KGLOG_PROCESS_ERROR(InsRet.second);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

KSceneObjectTemplate* KAllObjTemplate::GetTemplate(DWORD dwID)
{
    KSceneObjectTemplate* pResult = NULL;
    KMAP_SCENEOBJ_TEMPLATE::iterator it = m_mapData.find(dwID);
    KGLOG_PROCESS_ERROR(it != m_mapData.end());

    pResult = &it->second;
Exit0:
    return pResult;
}
