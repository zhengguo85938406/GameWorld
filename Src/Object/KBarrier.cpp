#include "stdafx.h"
#include "KBarrier.h"
#include "KAllObjTemplate.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KScene.h"

KBarrier::KBarrier()
{
    m_nLife = 0;
	m_nRepresentID = 0;
}

KBarrier::~KBarrier()
{
}

BOOL KBarrier::Init()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = KMovableObstacle::Init();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KBarrier::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    m_nLife = pTemplate->m_nCustomParam[0];
    MAKE_IN_RANGE(m_nLife, 0, INT_MAX);
    m_bCanBeAttack = pTemplate->m_nCustomParam[1];
	m_nRepresentID = pTemplate->m_nCustomParam[2];

Exit0:
    return;
}

int KBarrier::GetCustomData(int nIndex)
{
    if (nIndex == 0)
        return m_nLife;
	if (nIndex == 1)
		return m_nRepresentID;
    return 0;
}

void KBarrier::BeAttacked()
{
    if (!m_bCanBeAttack)
        return;
    
    --m_nLife;
    if (m_nLife == 0)
    {
        m_pScene->RemoveSceneObject(this);
        return;
    }

    g_PlayerServer.DoSyncSceneObject(this, -1);

    return;
}
