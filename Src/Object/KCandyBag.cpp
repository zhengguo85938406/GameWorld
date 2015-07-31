#include "stdafx.h"
#include "KCandyBag.h"
#include "KScene.h"
#include "KPlayerServer.h"

KCandyBag::KCandyBag(void)
{
    m_bAttackState      = false;
    m_bCanAttach        = false;
    m_bCanThrown        = false;
    m_bCanStood         = false;
    m_bCanShot          = false;
    m_bCanToken         = false;
    m_nLifePoint        = 3;
}


KCandyBag::~KCandyBag(void)
{
}

KSceneObjectType KCandyBag::GetType() const
{
    return sotCandyBag;
}

void KCandyBag::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    assert(pTemplate);
    assert(pTemplate->m_nCustomParam[0] > 0);

    m_nLifePoint = pTemplate->m_nCustomParam[0];
}

BOOL KCandyBag::CollidedByBall(KBall* pBall)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    BOOL        bFreeFall   = false;
    int         nVelocityX  = 100;
    KMoveType   eMoveType   = mosInvalid;

    assert(pBall);

    KG_PROCESS_ERROR(!m_bToBeDelete);

    bFreeFall = (pBall->m_nVelocityX == 0 && pBall->m_nVelocityY == 0 && pBall->m_nVelocityZ <= 0);

    pBall->m_nVelocityZ = (int)(-0.65 * pBall->m_nVelocityZ);
    
    // 自由落体给个横向干扰
    if (bFreeFall)
    {
        if (m_nX > m_pScene->GetLength() / 2)
            nVelocityX = -nVelocityX;

        pBall->m_nVelocityX = nVelocityX;
    }

    if (pBall->m_bShootFlagForCandyBag)
    {
        pBall->m_bShootFlagForCandyBag = false;

        pBall->TurnToMoveType(mosFree);

        --m_nLifePoint;

        if (m_nLifePoint == 0)
            m_pScene->RemoveSceneObject(this);
    }

    bResult = true;
Exit0:
    return bResult;
}
