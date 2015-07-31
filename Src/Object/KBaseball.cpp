#include "stdafx.h"
#include "KBaseball.h"
#include "KScene.h"
#include "KHero.h"
#include "KPlayerServer.h"


KBaseball::KBaseball(void)
{
    m_nCurAttackPoint   = 1;
    m_bAttackState      = true;

	m_fXYReboundCoeX    = 0.65;
	m_fXYReboundCoeY    = 0.65;
	m_fXYReboundCoeZ    = 0.65;

	m_fYZReboundCoeX    = 0.5;	             
	m_fYZReboundCoeY    = 0.5;
	m_fYZReboundCoeZ    = 0.5;
}


KBaseball::~KBaseball(void)
{
}

KSceneObjectType KBaseball::GetType() const
{
    return sotBaseball;
}

void KBaseball::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
}

void KBaseball::ProcessRebounds()
{
	if (m_bXYRebound || m_bXZRebound || m_bYZRebound)
	{
		++m_nCurAttackPoint;
		if (m_nCurAttackPoint > 2)
			m_nCurAttackPoint = 2;
	}
    KMovableObject::ProcessRebounds();
}

BOOL KBaseball::CollideHero(KHero* pHero)
{
    BOOL bResult =  false;
    BOOL bRetCode = false;
    KSkill* pSkill = NULL;

    assert(pHero);
    KG_PROCESS_ERROR(m_bAttackState);
    KG_PROCESS_ERROR(m_pTemplate);

    bRetCode = pHero->CanBeAttacked();
    KG_PROCESS_ERROR(bRetCode);

    if (m_nCurAttackPoint == 1)
    {
        bRetCode = pHero->ApplyGeneralAffect(ERROR_ID, this, m_pTemplate->m_dwAttackAffectID1);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else if (m_nCurAttackPoint == 2)
    {
        bRetCode = pHero->ApplyGeneralAffect(ERROR_ID, this, m_pTemplate->m_dwAttackAffectID2);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        KGLOG_PROCESS_ERROR(false);
    }

    m_nVelocityX = - (int)(m_nVelocityX * m_fYZReboundCoeX);
    m_nVelocityY = (int)(m_nVelocityY * m_fYZReboundCoeY);
    m_nVelocityZ = (int)(m_fYZReboundCoeZ * m_nVelocityZ);

    m_bAttackState = false;

    g_PlayerServer.DoSyncSceneObject(this);

    bResult = true;
Exit0:
    return bResult;
}

void KBaseball::OnTurnedToIdle()
{
    if (!m_bToBeDelete)
        m_pScene->RemoveSceneObject(this);
}
