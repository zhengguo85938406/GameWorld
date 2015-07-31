#include "stdafx.h"
#include "KBrick.h"
#include "KHero.h"
#include "KPlayerServer.h"
#include "KAllObjTemplate.h"

KBrick::KBrick()
{
    m_bLoseGravity = false;
    m_bCanShot = false;

    m_fYZReboundCoeX    = 0.3;
    m_fYZReboundCoeY    = 0.0;
    m_fYZReboundCoeZ    = 0.0;
}

KBrick::~KBrick()
{
}

void KBrick::CollideHero(KHero* pHero)
{
    BOOL bRetCode = false;
    int nDamage = 0;
    int nKnockedVX = 0;
    int nKnockedVZ = 0;

    KGLOG_PROCESS_ERROR(pHero);
    KG_PROCESS_ERROR(m_bAttackState);// ÊÇ·ñ¼¤»î
    KG_PROCESS_ERROR(m_dwThrowerID != pHero->m_dwID);

    bRetCode = pHero->CanBeAttacked();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pHero->ApplyGeneralAffect(m_dwThrowerID, this, m_pTemplate->m_dwAttackAffectID1);
    KGLOG_PROCESS_ERROR(bRetCode);

    ChangeToNormalState();
    m_nVelocityX = -(int)(m_nVelocityX* m_fYZReboundCoeX);
    m_nVelocityY = 0;
    m_nVelocityZ = 0;

    g_PlayerServer.DoSyncSceneObject(this);

Exit0:
    return;
}


void KBrick::ProcessRebounds()
{
    if (m_bAttackState && (m_bXYRebound || m_bXZRebound || m_bYZRebound)) // ÓÐÅö×²
    {
        ChangeToNormalState();
    }

    KDoodad::ProcessRebounds();
}

void KBrick::ChangeToAttackState()
{
    m_bLoseGravity = true;
    m_bCanToken = false;
    m_bAttackState = true;
}

void KBrick::ChangeToNormalState()
{
    m_bLoseGravity      = false;
    m_bCanToken         = true;
    m_bAttackState      = false;
}

void KBrick::OnThrowOut(KHero* pHero)
{
    ChangeToAttackState();
}

int KBrick::GetCustomData(int nIndex)
{
    int nRet = 0;

    if (nIndex == 0)
    {
        nRet = m_bAttackState;
    }

    return nRet;
}

int KBrick::GetCurrentGravity()
{
    int nCurrentGravity = 0;
    if (!m_bLoseGravity)
        nCurrentGravity = KMovableObject::GetCurrentGravity();

    return nCurrentGravity;
}
