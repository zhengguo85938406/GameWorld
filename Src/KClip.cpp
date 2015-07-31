#include "stdafx.h"
#include "KClip.h"
#include "KHero.h"
#include "KScene.h"
#include "KPlayerServer.h"


KClip::KClip(void)
{
    m_bAttackState      = false;

    m_bCanAttach        = false;
    m_bCanThrown        = true;
    m_bCanStood         = false;
    m_bCanShot          = false;

    m_bCanToken         = true;

    m_nTotalFrameCount      = 16;
    m_nCurTotalFrameCount   = 0;
    m_nFrameStart           = 0;
    m_dwStampeID            = ERROR_ID;
    m_bFullObstacle         = false;
}


KClip::~KClip(void)
{
}

void KClip::Stampede(KHero* pHero)
{
    BOOL bRetCode = false;
    int nDamage = 0;
    DWORD dwStampededID = ERROR_ID;
    KHero* pStampeded = NULL;

    KGLOG_PROCESS_ERROR(pHero);
    KG_PROCESS_ERROR(m_bAttackState);

    bRetCode = pHero->CanBeAttacked();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pHero->ApplyGeneralAffect(m_dwAttakerID, this, m_pTemplate->m_dwAttackAffectID1);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (!pHero->m_bBaTi)
    {
        pHero->SetPosition(m_nX, m_nY, m_nZ);

        // 为了解决夹完人adjust的问题，先减少服务端freeze时间,先用halt的时间
        pHero->FreezeByFrame(g_pSO3World->m_Settings.m_ConstList.nPlayerHaltTime);
        dwStampededID  = pHero->m_dwID;
        pStampeded    = pHero;
    }

    m_pScene->ApplyObstacle(this, false);

    m_nCurTotalFrameCount   = m_nTotalFrameCount;
    m_nFrameStart           = m_nGameLoop;
    m_bAttackState          = false;
    m_dwAttakerID           = ERROR_ID;
    m_dwStampeID            = dwStampededID;

    g_PlayerServer.DoDoodadStuckHero(this, pStampeded);

Exit0:
    return;
}

void KClip::Activate(int nCurLoop)
{
    KDoodad::Activate(nCurLoop);
    ProcessStampede(nCurLoop);
}

void KClip::ProcessStampede(int nCurLoop)
{
    KG_PROCESS_ERROR(m_nCurTotalFrameCount > 0);
    if (nCurLoop - m_nFrameStart >= m_nCurTotalFrameCount)
        FinishStampe();

Exit0:
    return;
}


KSceneObjectType KClip::GetType() const
{
    return sotClip;
}

void KClip::OnDeleteFlagSetted()
{
    KMovableObject::OnDeleteFlagSetted();

    if (m_dwStampeID != ERROR_ID)
    {
        KHero* pHero = NULL;
        assert(m_pScene);

        pHero = m_pScene->GetHeroById(m_dwStampeID);
        if (pHero)
            pHero->UnFreeze();

        FinishStampe();
    }

    if (m_bAttackState)
    {
        m_pScene->ApplyObstacle(this, false);
        m_bAttackState = false;
    }

    return;
}

void KClip::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    m_nTotalFrameCount = pTemplate->m_nCustomParam[0] * GAME_FPS;
}

void KClip::OnTurnedToIdle()
{
    if (m_dwThrowerID != ERROR_ID)
    {
        m_dwAttakerID = m_dwThrowerID;
        m_bAttackState = true;
        m_dwThrowerID = ERROR_ID;
        ClearCollision();
        CheckCollisionObj();

        m_pScene->ApplyObstacle(this, true);
    }
}

void KClip::OnThrowOut(KHero* pHero)
{
    m_bCanToken = false;
}

void KClip::FinishStampe()
{
    g_PlayerServer.DoDoodadRelaseHero(m_pScene, m_dwID, m_dwStampeID);

    m_bCanToken             = true;
    m_bAttackState          = false;
    m_dwStampeID            = ERROR_ID;
    m_nFrameStart           = 0;
    m_nCurTotalFrameCount   = 0;

    m_pScene->ApplyObstacle(this, false);
}


