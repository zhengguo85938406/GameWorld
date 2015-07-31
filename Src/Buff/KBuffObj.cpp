#include "stdafx.h"
#include "KBuffObj.h"
#include "KAllObjTemplate.h"
#include "KSO3World.h"
#include "KScene.h"

KBuffObj::KBuffObj(void)
{
    m_bCanToken = false;
}


KBuffObj::~KBuffObj(void)
{
}

KSceneObjectType KBuffObj::GetType() const
{
    return sotBuffObj;
}

BOOL KBuffObj::HasGuide() const
{
    return false;
}

int KBuffObj::GetCustomData(int nIndex)
{
    return m_nBuffID;
}

void KBuffObj::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    KMapParams*             pMapParams              = NULL;

    assert(m_pScene);

    pMapParams = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(m_pScene->m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapParams);

    m_nBuffID = g_pSO3World->m_AwardMgr.ScenenBuffAward(pMapParams->dwBuffAwardID);

Exit0:
    return;
}


