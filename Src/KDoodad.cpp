#include "stdafx.h"
#include "KDoodad.h"
#include "KHero.h"
#include "KHero.h"
#include "KScene.h"
#include "KPlayerServer.h"
#include "game_define.h"
#include "KSO3World.h"
#include "KParabola.h"
#include "KFix2PParabola.h"
#include <algorithm>

KDoodad::KDoodad()
{
    m_bCanAttach        = false;
    m_bCanThrown        = true;
    m_bCanToken         = true;
    m_bCanStood         = false;
    m_bCanShot          = true;

    m_bCanHit           = false;
    m_pTargetSocket     = NULL;
    m_dwShooterID       = ERROR_ID;
    m_dwThrowerID       = ERROR_ID;

    m_eActionType       = tmaNone;
    m_nTotalFrame       = 0;
    m_nCreateFrame      = g_pSO3World->m_nGameLoop;
    m_dwAttakerID       = ERROR_ID;
}

KDoodad::~KDoodad()
{

}

BOOL KDoodad::BeToken(KHero* pHero)
{
    assert(pHero);

    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = CanBeTokenBy(pHero);
    KG_PROCESS_ERROR(bRetCode);

    TurnToMoveType(mosToken);

    m_dwThrowerID   = ERROR_ID;
    m_dwShooterID   = ERROR_ID;
    m_pTargetSocket = NULL;

    ClearVelocity();
 
    pHero->AddFollower(this);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::BeUnTake(KHero* pTaker)
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(pTaker);
    KG_PROCESS_ERROR(IsTokenBy(pTaker));
    
    RemoveGuide();

    TurnToMoveType(mosFree);
    ClearVelocity();

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::BeThrown(KHero* pHero, KTWO_DIRECTION eDir)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KPOSITION cStartPos = pHero->GetTopCenter();

    assert(pHero);

    bRetCode = CanBeThrownBy(pHero);
    KG_PROCESS_ERROR(bRetCode);

    MakeInRange<int>(cStartPos.nX, 0, m_pScene->GetLength() - 1);
    MakeInRange<int>(cStartPos.nY, 0, m_pScene->GetWidth() - 1);

    RemoveGuide();
    TurnToMoveType(mosFree);

    m_nVelocityX = m_pTemplate ? m_pTemplate->m_nVXOnThrow : 0;
    m_nVelocityY = m_pTemplate ? m_pTemplate->m_nVYOnThrow : 0;
    m_nVelocityZ = m_pTemplate ? m_pTemplate->m_nVZOnThrow : 0;

    if (eDir == csdLeft)
    {
        m_nVelocityX = -m_nVelocityX;
    }

    SetPosition(cStartPos);
    m_dwThrowerID = pHero->m_dwID;
    m_dwShooterID = ERROR_ID;

    OnThrowOut(pHero);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::BeAttachTo(KHero* pHero)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = CanBeAttachedTo(pHero);
    KGLOG_PROCESS_ERROR(bRetCode);

    TurnToMoveType(mosAttached);

    m_dwThrowerID       = ERROR_ID;
    m_dwShooterID       = ERROR_ID;
    m_pTargetSocket     = NULL;
    ClearVelocity();

    pHero->AddFollower(this);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::BeDetach(KHero* pHero)
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(pHero);
    KG_PROCESS_ERROR(IsAttachedTo(pHero));

    RemoveGuide();
    TurnToMoveType(mosFree);
    ClearVelocity();

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::BeStandOn(KMovableObject* pObj)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pObj);

    bRetCode = CanBeStandOnBy(pObj);
    KGLOG_PROCESS_ERROR(bRetCode);

    pObj->TurnToMoveType(mosStandOn);
    pObj->ClearVelocity();

    AddFollower(pObj);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::BeShoot(KHero* pShooter, KBasketSocket* pTargetSocket)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KFix2PParabola cPath(GetCurrentGravity());

    KGLOG_PROCESS_ERROR(pShooter && pTargetSocket);

    bRetCode = CanBeShootBy(pShooter);
    KGLOG_PROCESS_ERROR(bRetCode);

    cPath = GetShootPath(pShooter, pTargetSocket);
    cPath.Start(g_pSO3World->m_nGameLoop);
    
    DoTrackMove(&cPath);

    m_nTotalFrame   = cPath.GetTotalFrame();
    m_bCanHit       = true;
    m_dwShooterID   = pShooter->m_dwID;
    m_dwThrowerID   = ERROR_ID;
    m_pTargetSocket = pTargetSocket;
    m_eActionType   = tmaShoot;

    bResult = true;
Exit0:
    return bResult;
}

int KDoodad::GetCustomData(int nIndex)
{
    return 0;
}


BOOL KDoodad::CanBeStandOnBy(KMovableObject* pObj) const
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(pObj);
    KG_PROCESS_ERROR(m_bCanStood);

    bRetCode = HasFollowRelationWith(pObj);
    KG_PROCESS_ERROR(!bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::CanBeShootBy(KHero* pShooter) const
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_bCanShot);

    bRetCode = IsTokenBy(pShooter);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::CanBeTokenBy(KHero* pHero)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KMoveType eMoveType = GetMoveType();

    KG_PROCESS_ERROR(pHero);
    KG_PROCESS_ERROR(m_bCanToken);

    KG_PROCESS_ERROR(eMoveType == mosIdle || eMoveType == mosTrackMove || eMoveType == mosFree);

    if (!Is(sotBall))
    {
        bRetCode = IsOnGround();
        KG_PROCESS_ERROR(bRetCode);
    }

    KG_PROCESS_ERROR(!m_bAttackState);

    bRetCode = pHero->HasFollowRelationWith(this);
    KG_PROCESS_ERROR(!bRetCode);

    bRetCode = g_IsCollide(pHero, this);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::CanBeThrownBy(KHero* pHero) const
{
    return m_bCanThrown && IsTokenBy(pHero);
}


BOOL KDoodad::CanBeAttachedTo(KHero* pHero) const
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(pHero);
    KG_PROCESS_ERROR(m_bCanAttach);
    KG_PROCESS_ERROR(!IsFollowingSomeone());

    bResult = true;
Exit0:
    return bResult;
}

KFix2PParabola KDoodad::GetShootPath(KHero* pShooter,KBasketSocket* pTargetSocket)
{
    assert(pShooter && pTargetSocket);

    KPOSITION   cStart  = pShooter->GetTopCenter();
    KPOSITION   cEnd    = pTargetSocket->GetPosition();
    int         nHeight = CalcProcessTrackMoveZmax(pShooter->m_nVirtualFrame, cStart, cEnd);
    
    return KFix2PParabola(cStart, cEnd, nHeight, GetCurrentGravity());
}

BOOL KDoodad::BeUseBy(KHero* pHero, KTWO_DIRECTION eDir)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(eDir == csdLeft || eDir == csdRight);

    bRetCode = BeThrown(pHero, eDir);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KDoodad::CanGetAward()
{
    return g_pSO3World->m_nGameLoop > m_nCreateFrame + DELAY_GET_AWARD;
}

BOOL KDoodad::GetAIEventRange(KHero* pUser, KBODY& rRetRange)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pUser);
    KGLOG_PROCESS_ERROR(m_pTemplate);
	KGLOG_PROCESS_ERROR(m_pTemplate->m_bFireAIEvent);

    rRetRange.nLength   = m_nLength * (m_pTemplate->m_nAIEventMaxX - m_pTemplate->m_nAIEventMinX) / HUNDRED_NUM;
    if (pUser->m_eFaceDir == csdRight)
        rRetRange.nX = m_nX + m_nLength * m_pTemplate->m_nAIEventMinX / HUNDRED_NUM + rRetRange.nLength / 2;
    else
        rRetRange.nX = m_nX - m_nLength * m_pTemplate->m_nAIEventMinX / HUNDRED_NUM - rRetRange.nLength / 2;

    rRetRange.nWidth    = m_nWidth * (m_pTemplate->m_nAIEventMaxY - m_pTemplate->m_nAIEventMinY) / HUNDRED_NUM;
    rRetRange.nY        = m_nY + m_nWidth * m_pTemplate->m_nAIEventMinY / HUNDRED_NUM + rRetRange.nWidth / 2;

    rRetRange.nHeight   = m_nHeight * (m_pTemplate->m_nAIEventMaxZ - m_pTemplate->m_nAIEventMinZ) / HUNDRED_NUM;
    rRetRange.nZ        = m_nZ + m_nHeight * m_pTemplate->m_nAIEventMinZ / HUNDRED_NUM;

    bResult = true;
Exit0:
    return bResult;
}
