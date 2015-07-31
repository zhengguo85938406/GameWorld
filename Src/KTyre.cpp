#include "stdafx.h"
#include "KTyre.h"
#include "KAllObjTemplate.h"
#include "KHero.h"


KTyre::KTyre(void)
{
    m_nStartRollingFrame = 0;
    m_nTotalRollingFrame = 0;

    m_bCanAttach    = false;
    m_bCanThrown    = true;
    m_bCanToken     = true;
    m_bCanStood     = false;
    m_bCanShot      = false;
    m_eStep         = trsNone;
}


KTyre::~KTyre(void)
{
}

void KTyre::Activate(int nCurLoop)
{
    KDoodad::Activate(nCurLoop);

    ProcessRolling(nCurLoop);
}

void KTyre::OnCollideHero(KHero* pHero)
{
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pHero);

    KG_PROCESS_ERROR(m_bAttackState);
    KG_PROCESS_ERROR(m_dwThrowerID != pHero->m_dwID);

    bRetCode = pHero->CanBeAttacked();
    KG_PROCESS_ERROR(bRetCode);

    switch(m_eStep)
    {
    case trsThrowing:
        if (m_pTemplate)
        {
            bRetCode = pHero->ApplyGeneralAffect(m_dwThrowerID, this, m_pTemplate->m_dwAttackAffectID1);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        break;
    case trsRolling:
        if (m_pTemplate)
        {
            bRetCode = pHero->ApplyGeneralAffect(m_dwThrowerID, this, m_pTemplate->m_dwAttackAffectID2);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        break;
    }

Exit0:
    return;
}

void KTyre::BeginRolling(int nCurLoop)
{
    if(m_eStep == trsThrowing)
    {
        m_nVelocityZ            = 0;
        m_nStartRollingFrame    = m_nGameLoop;
        m_eStep                 = trsRolling;
    }
}

void KTyre::FinishRolling()
{  
    assert(m_eStep == trsRolling || m_eStep == trsThrowing);

    ClearVelocity();
    m_bCanToken             = true;
    m_eStep                 = trsNone;
    m_bAttackState          = false;
    m_nStartRollingFrame    = 0;
}


void KTyre::ProcessRolling(int nCurLoop)
{
    if ((m_eStep == trsRolling) && (nCurLoop - m_nStartRollingFrame >= m_nTotalRollingFrame))
    {
        FinishRolling();
    }
}

void KTyre::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    m_nTotalRollingFrame = pTemplate->m_nCustomParam[0] * GAME_FPS;
}

void KTyre::OnThrowOut(KHero* pHero)
{
    assert(m_eStep == trsNone);

    m_bCanToken     = false;
    m_bAttackState  = true;
    m_eStep         = trsThrowing;
}

void KTyre::OnTurnedToIdle()
{
    if (m_eStep == trsRolling)
    {
        FinishRolling();
    }
}

void KTyre::ProcessXYRebound()
{
    if (m_eStep == trsThrowing)
    {
        BeginRolling(m_nGameLoop);
    }
}

void KTyre::ProcessXZRebound()
{
    if (m_eStep == trsRolling || m_eStep == trsThrowing)
    {
        FinishRolling();
    }
}

void KTyre::ProcessYZRebound()
{
    if (m_eStep == trsRolling || m_eStep == trsThrowing)
    {
        FinishRolling();
    }
}








