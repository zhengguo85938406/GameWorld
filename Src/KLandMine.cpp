#include "stdafx.h"
#include "KLandMine.h"
#include "KAllObjTemplate.h"
#include "KScene.h"
#include "KSO3World.h"
#include "KPlayerServer.h"

KLandMine::KLandMine()
{
    m_nActiveFrame      = 0;
    m_nWaitActiveFrame  = 0;
    m_bDistinguishForce = false;
    m_nBlowupVelocity   = 0;
    m_nBlowupRange      = 0;
    m_bCanThrown        = true;
    m_bCanShot          = false;
    m_bFullObstacle     = false;
}

KLandMine::~KLandMine()
{
}

KSceneObjectType KLandMine::GetType() const {return sotLandMine;}

void KLandMine::Activate(int nCurLoop)
{
    KDoodad::Activate(nCurLoop);

    if (m_nActiveFrame != 0 && nCurLoop >= m_nActiveFrame)
    {
        m_bAttackState      = true;
        m_nActiveFrame      = 0;
        m_dwAttakerID       = m_dwThrowerID;
        m_dwThrowerID       = ERROR_ID;

        m_pScene->ApplyObstacle(this, true);
        
        ClearCollision();
        CheckCollisionObj();
    }
}

void KLandMine::ActiveState()
{
    m_nActiveFrame = g_pSO3World->m_nGameLoop + m_nWaitActiveFrame;
    g_PlayerServer.DoActiveLandMineNotify(m_pScene, m_dwID);
    return;
}

void KLandMine::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    m_nWaitActiveFrame  = m_pTemplate->m_nCustomParam[0] * GAME_FPS;
    m_bDistinguishForce = m_pTemplate->m_nCustomParam[1];
    m_nBlowupVelocity   = m_pTemplate->m_nCustomParam[2] / GAME_FPS;
    m_nBlowupRange      = m_pTemplate->m_nCustomParam[3];
}

void KLandMine::OnTurnedToIdle()
{
    KG_PROCESS_ERROR(m_dwThrowerID != ERROR_ID);
    m_nActiveFrame  = g_pSO3World->m_nGameLoop + m_nWaitActiveFrame;

Exit0:
    return;
}

void KLandMine::OnThrowOut(KHero* pHero)
{
    m_bCanToken = false;
}


