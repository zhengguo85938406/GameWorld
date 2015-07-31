#include "stdafx.h"
#include "KScene.h"
#include "KBananaPeel.h"
#include "KPlayerServer.h"

KBananaPeel::KBananaPeel(void)
{
    m_bAttackState = false;
    m_bOnlyAttackPlayer = false;
    m_bFullObstacle = false;
}


KBananaPeel::~KBananaPeel(void)
{
}

KSceneObjectType KBananaPeel::GetType() const
{
    return sotBananaPeel;
}

void KBananaPeel::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
	assert(pTemplate);

    m_bOnlyAttackPlayer = (BOOL)pTemplate->m_nCustomParam[0];
}

void KBananaPeel::OnTurnedToIdle()
{
    m_bAttackState = true;
    m_pScene->ApplyObstacle(this, true);
}

BOOL KBananaPeel::CollidedByHero(KHero* pHero)
{
	BOOL bResult =  false;
	BOOL bRetCode = false;
	KSkill* pSkill = NULL;

	assert(pHero);
	KG_PROCESS_ERROR(m_bAttackState);

    KGLOG_PROCESS_ERROR(m_pTemplate);

    if (m_bOnlyAttackPlayer)
    {
        bRetCode = pHero->IsNpc();
        KG_PROCESS_SUCCESS(bRetCode);
    }

	bRetCode = pHero->CanBeAttacked();
	KG_PROCESS_ERROR(bRetCode);

    bRetCode = pHero->ApplyGeneralAffect(ERROR_ID, this, m_pTemplate->m_dwAttackAffectID1);
    KGLOG_PROCESS_ERROR(bRetCode);

	m_bAttackState = false;
    m_pScene->RemoveSceneObject(this);
    m_pScene->ApplyObstacle(this, false);

Exit1:
	bResult = true;
Exit0:
	return bResult;
}
