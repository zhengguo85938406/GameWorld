#include "stdafx.h"
#include "KBall.h"
#include "KHero.h"
#include "KScene.h"
#include "KPlayerServer.h"
#include "KBasketSocket.h"
#include "KSO3World.h"
#include "KBasket.h"
#include "KAttribute.h"
#include "KFix2PParabola.h"
#include "KSkillShootPath.h"
#include "KPlayer.h"
#include "KComplexPath.h"
#include "KComplexPathData.h"
#include "GlobalEnum.h"

KBall::KBall()
{
    m_fXYReboundCoeX    = 0.65;
    m_fXYReboundCoeY    = 0.65;
    m_fXYReboundCoeZ    = 0.65;

    m_fYZReboundCoeX    = 0.5;	             
    m_fYZReboundCoeY    = 0.5;            
    m_fYZReboundCoeZ    = 0.5;

    m_bCanHit           = false;
    m_pTargetSocket     = NULL;

    m_bRestAfterTrackMove    = false;
    m_bLastFrameState = false;
    m_bCurFrameState  = false;

    m_eLastBeShootType = stNormalShoot;
    m_bShootFlagForCandyBag = false;
    m_bCanBeAttack      = true;
    m_nFreezeReleaseFrame = 0;
    m_nBaseRandomSeed   = 0;
    m_nHitRate          = 0;
    m_bDoDropRebound    = false;
    memset(&m_DelayAction, 0, sizeof(m_DelayAction));
}

KBall::~KBall()
{
}

void KBall::Activate(int nCurLoop)
{
    if (nCurLoop <= m_nFreezeReleaseFrame)
        goto Exit0;

    KDoodad::Activate(nCurLoop);
    if (m_DelayAction.nFrame > 0 && nCurLoop >= m_DelayAction.nFrame)
    {
        m_DelayAction.nFrame = 0;
        m_nX = m_DelayAction.nX;
        m_nY = m_DelayAction.nY;
        m_nZ = m_DelayAction.nZ;
        m_nVelocityX = m_DelayAction.nVX;
        m_nVelocityY = m_DelayAction.nVY;
        m_nVelocityZ = m_DelayAction.nVZ;
    }

Exit0:
    return;
}

void KBall::ProcessCollision()
{
    m_bLastFrameState = m_bCurFrameState;
    m_bCurFrameState  = false;

    KMovableObject::ProcessCollision();
}

void KBall::ProcessAfterCollision()
{
    KDoodad::ProcessAfterCollision();

    CheckWhetherScored();

    DoDropRebound();

Exit0:
    return;
}

BOOL KBall::BeShoot(KHero* pShooter, KBasketSocket* pTargetSocket, int nInterference)
{
    BOOL            bResult                 = false;
    BOOL            bRetCode                = false;
    int             nNowHitRate             = 0;
    int             nNowAttenuationTimes    = 0;
    KFix2PParabola  cPath(GetCurrentGravity());
    int             nZmax                   = 0;
    KPOSITION       cStart;
    KPOSITION       cEnd;

    KGLOG_PROCESS_ERROR(pShooter && pTargetSocket);

    bRetCode = CanBeShootBy(pShooter);
    KGLOG_PROCESS_ERROR(bRetCode);

    cStart = GetShootStartPos(pShooter);
    cEnd =  GetShootEndPos(pTargetSocket);

    pShooter->CalcHitRateParams(cStart, cEnd, nNowAttenuationTimes, nNowHitRate, nInterference);
    m_bCanHit = pShooter->RandomHitted(nNowHitRate);

    LogInfo(
        "Canhit:%d, hitrate:%d, virtualframe:%d, nInterference:%d", 
        m_bCanHit, nNowHitRate, pShooter->m_nVirtualFrame, nInterference
    );

    if (!m_bCanHit)
    {
        int nFlag = 1;
        if (nNowAttenuationTimes % 2 == 0)
            nFlag = -1;
        cEnd.nX += nFlag * pShooter->m_nMaxHitOffset * (HUNDRED_NUM - nNowHitRate) / HUNDRED_NUM;
        cEnd.nY += nFlag * pShooter->m_nMaxHitOffset * (HUNDRED_NUM - nNowHitRate) / HUNDRED_NUM;
    }

    nZmax = CalcProcessTrackMoveZmax(pShooter->m_nVirtualFrame, cStart, cEnd);

    cPath.SetCtrlParams(cStart, cEnd, nZmax);
    cPath.Start(g_pSO3World->m_nGameLoop);
    DoTrackMove(&cPath);

    m_nTotalFrame   = cPath.GetTotalFrame();
    m_pTargetSocket = pTargetSocket;
    m_dwShooterID   = pShooter->m_dwID;
    m_dwThrowerID   = ERROR_ID;

    bResult = true;
Exit0:
    return bResult;
}

KSceneObjectType KBall::GetType() const
{
    return sotBall;
}

BOOL KBall::SlamDunk(KHero* pSlamer, KBasketSocket* pTargetSocket)
{
    BOOL        bResult            = false;
    BOOL        bRetCode           = false;
    KBasket*    pBasket             = NULL;
    KBODY       cSocketBody;
    KBODY       cSlamerBody;
    KPOSITION   cSlamerDstPos;
    KPOSITION   cBallDstPos;
   
    KGLOG_PROCESS_ERROR(pSlamer && pTargetSocket);

    pBasket = pTargetSocket->m_pBasket; // Ã»Àº¿ð
    KGLOG_PROCESS_ERROR(pBasket);

    bRetCode = BeUnTake(pSlamer);
    KGLOG_PROCESS_ERROR(bRetCode);

    cSocketBody = pTargetSocket->GetBody();
    cSlamerBody = pSlamer->GetBody();

    if (pTargetSocket->m_eDir == csdRight)
        cSlamerDstPos.nX = cSocketBody.nX + cSocketBody.nLength / 2 + cSlamerBody.nLength;
    else
        cSlamerDstPos.nX = cSocketBody.nX - cSocketBody.nLength / 2 - cSlamerBody.nLength;
    cSlamerDstPos.nY = cSocketBody.nY;
    cSlamerDstPos.nZ = cSocketBody.nZ - CELL_LENGTH * 3;

    pSlamer->EnsureNotStandOnObject();
    pSlamer->SetPosition(cSlamerDstPos);
    pSlamer->ClearVelocity();
    pSlamer->LoseControlByCounter(cdSlamDunkFrameCount);

    cBallDstPos = pTargetSocket->GetPosition();
    SetPosition(cBallDstPos);
    m_dwShooterID   = pSlamer->m_dwID;
    m_dwThrowerID   = ERROR_ID;
    m_pTargetSocket = pTargetSocket;

    Freeze(g_pSO3World->m_Settings.m_ConstList.nSlamBallReboundFrame);

    bResult = true;
Exit0:
    return bRetCode;
}


BOOL KBall::SkillShoot(KHero* pShooter, KBasketSocket* pSocket)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    int             nVelocity   = CELL_LENGTH;
    KSkillShootPath cPath((GetCurrentGravity()));
    KPOSITION       cSocketPos;
    KPOSITION       cSrc;
    KPOSITION       cDst;
    KPOSITION       cCtrl;
    
    KGLOG_PROCESS_ERROR(pShooter && pSocket);

    bRetCode = CanBeShootBy(pShooter);
    KG_PROCESS_ERROR(bRetCode);

    cSocketPos = pSocket->GetPosition();

    cSrc = GetShootStartPos(pShooter);

    cDst = cSocketPos;
    cDst.nZ += CELL_LENGTH * 3;

    cCtrl.nX = (pSocket->m_eDir == csdLeft) ? cSocketPos.nX - (CELL_LENGTH * 12) : cSocketPos.nX + (CELL_LENGTH * 12);
    cCtrl.nY = cSocketPos.nY;
    cCtrl.nZ = CELL_LENGTH * 4;

    cPath.SetCtrlParams(cSrc, cDst, cCtrl, nVelocity);
    cPath.Start(g_pSO3World->m_nGameLoop);

    DoTrackMove(&cPath);

    m_bAttackState = true;
    m_dwShooterID = pShooter->m_dwID;
    m_dwThrowerID = ERROR_ID;
    m_pTargetSocket = pSocket;
    m_bRestAfterTrackMove = true;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBall::SkillSlam(KHero* pSlamer, KBasketSocket* pTargetSocket)
{
    BOOL            bResult                 = false;
    BOOL            bRetCode                = false;
    int             nNowHitRate             = 0;
    int             nNowAttenuationTimes    = 0;
    int             nZmax                   = 0;
    KFix2PParabola  cPath(GetCurrentGravity());
    KPOSITION       cSrc;
    KPOSITION       cDst;
    int             nTotalFrame             = 0;

    KG_PROCESS_ERROR(pSlamer && pTargetSocket);

    bRetCode = CanSkillSlam(pSlamer, pTargetSocket);
    KGLOG_PROCESS_ERROR(bRetCode);

    cSrc = GetShootStartPos(pSlamer);
    cDst = GetSkillSlamDstPos(pSlamer, pTargetSocket);

    nZmax = CalcProcessTrackMoveZmax(pSlamer->m_nVirtualFrame, cSrc, cDst);

    cPath.SetCtrlParams(cSrc, cDst, nZmax);
    cPath.Start(pSlamer->m_nVirtualFrame);
    pSlamer->DoTrackMove(&cPath);
    nTotalFrame = cPath.GetTotalFrame();
    pSlamer->LoseControlByCounter(nTotalFrame);

    m_bCanHit = true;
    m_nTotalFrame = cPath.GetTotalFrame();
    m_pTargetSocket = pTargetSocket;
    m_dwShooterID = pSlamer->m_dwID;
    m_dwThrowerID = ERROR_ID;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBall::SkillSlamEx(KHero* pSlamer, KSlamBallSkill* pSkill, KBasketSocket* pTargetSocket)
{
    BOOL            bResult                 = false;
    BOOL            bRetCode                = false;
    int             nNowHitRate             = 0;
    int             nNowAttenuationTimes    = 0;
    int             nZmax                   = 0;
    KFix2PParabola  cPath(GetCurrentGravity());
    KPOSITION       cSrc;
    KPOSITION       cDst;
    int             nTotalFrame             = 0;

    KG_PROCESS_ERROR(pSlamer && pTargetSocket);

    bRetCode = CanSkillSlam(pSlamer, pTargetSocket);
    KGLOG_PROCESS_ERROR(bRetCode);

    cSrc = GetShootStartPos(pSlamer);
    cDst = GetSkillSlamDstPos(pSlamer, pTargetSocket);

    nZmax = CalcProcessTrackMoveZmax(pSlamer->m_nVirtualFrame, cSrc, cDst);

    cPath.SetCtrlParams(cSrc, cDst, nZmax);
    cPath.Start(pSlamer->m_nVirtualFrame);
    pSlamer->DoTrackMove(&cPath);
    nTotalFrame = cPath.GetTotalFrame();
    pSlamer->LoseControlByCounter(nTotalFrame);

    m_bCanHit = true;
    m_nTotalFrame = cPath.GetTotalFrame();
    m_pTargetSocket = pTargetSocket;
    m_dwShooterID = pSlamer->m_dwID;
    m_dwThrowerID = ERROR_ID;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBall::ShootWithComplexPath(KHero* pShooter, DWORD dwPathID)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;

    KComplexPathInfo*   pPath           = NULL;
    KBasketSocket*      pTargetSocket   = NULL;

    KComplexPath    cPath;
    KPOSITION       cSocketPos;
    KPOSITION       cSrc;
    KPOSITION       cDst;
    KTWO_DIRECTION  eFaceDir        = csdInvalid;

    KGLOG_PROCESS_ERROR(pShooter);
    KGLOG_PROCESS_ERROR(pShooter->m_pScene);
    
    pPath = g_pSO3World->m_Settings.m_ComplexPathList.GetPath(dwPathID);
    KGLOG_PROCESS_ERROR(pPath);

    m_eLastBeShootType = stShootBySkill;
    m_bRestAfterTrackMove = true;

    eFaceDir = pShooter->GetFaceDir();
    pTargetSocket = pShooter->m_pScene->GetBasketSocket(
        eFaceDir == csdRight ? csdLeft : csdRight, 
        pPath->eTargetFloor
    );
    KGLOG_PROCESS_ERROR(pTargetSocket);

    cSocketPos = pTargetSocket->GetPosition();
    cSrc = GetShootStartPos(pShooter);

    if (eFaceDir == csdRight)
    {
        cSrc.nX += pPath->nOffsetX;
    }
    else
    {
        cSrc.nX -= pPath->nOffsetX;
    }
    cSrc.nY += pPath->nOffsetY;
    cSrc.nZ += pPath->nOffsetZ;

    cDst = cSocketPos;
    cDst.nZ += CELL_LENGTH * 3;
    
    bRetCode = cPath.LoadComplexPath(
        &cSrc, 
        &cDst, 
        pPath->cPoints,
        pShooter, 
        pPath->eEndPathType
    );
    KGLOG_PROCESS_ERROR(bRetCode);

    cPath.Start(g_pSO3World->m_nGameLoop);
    DoTrackMove(&cPath);

    m_bAttackState = true;
    m_dwShooterID = pShooter->m_dwID;
    m_dwThrowerID = ERROR_ID;
    m_pTargetSocket = pTargetSocket;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBall::CanSkillSlam(KHero* pSlamer, KBasketSocket* pTargetSocket)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(pSlamer);
    KG_PROCESS_ERROR(pTargetSocket);
    
    bRetCode = IsTokenBy(pSlamer);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

KPOSITION KBall::GetSkillSlamDstPos(KHero* pSlamer, KBasketSocket* pTargetSocket)
{
    assert(pSlamer && pTargetSocket);
    KPOSITION cResult = pTargetSocket->GetPosition();
    KBODY cSocketBody = pTargetSocket->GetBody();
    KBODY pSlamerBody = pSlamer->GetBody();

    if (pTargetSocket->m_eDir == csdRight)
        cResult.nX += cSocketBody.nLength / 2 + pSlamerBody.nLength;
    else
        cResult.nX -= cSocketBody.nLength / 2 + pSlamerBody.nLength;

    return cResult;
}

BOOL KBall::PassTo(KHero* pPasser, KHero* pReceiver)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KPOSITION cSrcPos;
    KPOSITION cDstPos;
    KFix2PParabola cPath(GetCurrentGravity());
    int       nZMax = 0;

    KGLOG_PROCESS_ERROR(pPasser && pReceiver);
    KGLOG_PROCESS_ERROR(pPasser->m_dwID != pReceiver->m_dwID);

    bRetCode = IsTokenBy(pPasser);
    KG_PROCESS_ERROR(bRetCode);

    cSrcPos = GetPassSrcPos(pPasser, pReceiver);
    cDstPos = GetPassDstPos(pPasser, pReceiver);
    nZMax = GetPassZMax(cSrcPos, cDstPos);

    cPath.SetCtrlParams(cSrcPos, cDstPos, nZMax);
    cPath.Start(g_pSO3World->m_nGameLoop);

    DoTrackMove(&cPath);

    pPasser->ForbitAutoTakeBall(GAME_FPS);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBall::CanSlam(KHero* pSlamer, KBasketSocket* pTargetSocket)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(pSlamer);
    KG_PROCESS_ERROR(pTargetSocket);

    bRetCode = IsTokenBy(pSlamer);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pTargetSocket->IsEmpty();
    KG_PROCESS_ERROR(!bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

KPOSITION KBall::GetPassSrcPos(KHero* pPasser, KHero* pReceiver) const
{
    assert(pPasser && pReceiver);

    int nX = 0;
    int nY = 0;
    int nZ = 0;

    KBODY cBody = pPasser->GetBody();

    switch (pPasser->GetFaceDir())
    {
    case csdLeft:
        nX = cBody.nX - cBody.nWidth;
        break;
    case csdRight:
        nX = cBody.nX + cBody.nWidth;
        break;
    default:
        assert(false);
    }

    nY = cBody.nY;
    nZ = cBody.nHeight * 3 / 2 + cBody.nZ; 

    MakeInRange<int>(nX, 0, m_pScene->m_nXGridCount * CELL_LENGTH - 1);
    MakeInRange<int>(nY, 0, m_pScene->m_nYGridCount * CELL_LENGTH - 1);

    return KPOSITION(nX, nY, nZ);
}

KPOSITION KBall::GetPassDstPos(KHero* pPasser, KHero* pReceiver) const
{
    int nX = 0;
    int nY = 0;
    int nZ = 0;
    KPOSITION cReceiverPos = pReceiver->GetPosition();

    nX = cReceiverPos.nX;
    nY = cReceiverPos.nY;
    nZ = cReceiverPos.nZ + pReceiver->m_nHeight / 2;

    return KPOSITION(nX, nY, nZ);
}

int KBall::GetPassZMax(const KPOSITION& cSrc, const KPOSITION& cDst) const
{
    int nZOffset = 40;
    return MAX(cSrc.nZ, cDst.nZ) + nZOffset;
}

KPOSITION KBall::GetShootStartPos(KHero* pShooter) const
{
    KPOSITION       cResult;
    KBODY           cSelfBody       = GetBody();
    KBODY           cShooterBody    = pShooter->GetBody();
    KTWO_DIRECTION  eDir            = pShooter->GetFaceDir();
    int             nOffsetX        = 0;

    switch(eDir)
    {
    case csdLeft:
        nOffsetX = - cSelfBody.nWidth;
        break;
    case csdRight:
        nOffsetX = cSelfBody.nWidth;
        break;
    default:
        assert(false);
    }

    cResult.nX = cShooterBody.nX + nOffsetX;
    cResult.nY = cShooterBody.nY;
    cResult.nZ = cShooterBody.nZ + cShooterBody.nHeight;

    int nSceneXLength = m_pScene->m_nXGridCount * CELL_LENGTH;
    int nSceneYLength = m_pScene->m_nYGridCount * CELL_LENGTH;

    MakeInRange<int>(cResult.nX, 0, nSceneXLength - 1);
    MakeInRange<int>(cResult.nY, 0, nSceneYLength - 1);

    return cResult;
}

KPOSITION KBall::GetShootEndPos(KBasketSocket* pTargetSocket) const
{
    assert(pTargetSocket);
    return pTargetSocket->GetPosition();
}

void KBall::OnTrackMoveFinished()
{
    if (m_bRestAfterTrackMove)
    {
        ClearVelocity();
        m_bRestAfterTrackMove = false;
    }
    KMovableObject::OnTrackMoveFinished();
}

void KBall::OnTrackMoveInterrupted()
{
    m_bRestAfterTrackMove = false;
}

KHero* KBall::GetBallTaker()
{
    KHero*          pHandler    = NULL;
    KMovableObject* pGuide      = NULL;
    
    pGuide = GetGuide();
    KG_PROCESS_ERROR(pGuide);

    KGLOG_PROCESS_ERROR(pGuide->Is(sotHero));
    pHandler = (KHero*)pGuide;        
    
Exit0:
    return pHandler;
}

void KBall::CheckWhetherScored()
{
    KG_PROCESS_ERROR(m_bLastFrameState);
    KG_PROCESS_ERROR(!m_bCurFrameState);
    KG_PROCESS_ERROR(m_nVelocityZ < 0);

    //m_nVelocityZ = 0;
    m_bLastFrameState = false;
    m_bCurFrameState  = false;

    {
        BOOL bRetCode = false;
        KHero* pShooter = NULL;
        KPlayer* pPlayer = NULL;

        KGLOG_PROCESS_ERROR(m_pTargetSocket);

        pShooter = m_pScene->GetHeroById(m_dwShooterID);
        if (pShooter && pShooter->IsMainHero())
            pPlayer = pShooter->GetOwner();
        
        if (pPlayer)
        {
            KPLAYER_EVENT eEvent = peInvalid;

            switch (m_pTargetSocket->m_eFloor)
            {
            case bfFirst:
            case bfSecond:
                eEvent = peShootFirstSuccess;
                break;
            case bfThird:
            case bfForth:
                eEvent = peShootSecondSuccess;
                break;
            case bfFifth:
            case bfSixth:
                eEvent = peShootThirdSuccess;
                break;
            default:
                assert(false);
            }

            pPlayer->OnEvent(eEvent);
			pPlayer->OnEvent(peShootSuccess);
        }

        if (m_pTargetSocket->m_eFloor == bfFifth || m_pTargetSocket->m_eFloor == bfThird)
        {
            m_bDoDropRebound = true;
        }
    }
    
    m_pScene->OnShootBallSuccess(this, m_pTargetSocket);

Exit0:
    return;
}

void KBall::Freeze(int nFrame)
{
    m_nFreezeReleaseFrame = g_pSO3World->m_nGameLoop + nFrame;
}

void KBall::DoDropRebound()
{
    int nHitRate = 0;
    BOOL bHit = false;
    int nRandomSeed = 0;

    KG_PROCESS_ERROR(m_bDoDropRebound);
    KGLOG_PROCESS_ERROR(m_pTargetSocket);

    KG_PROCESS_ERROR(m_nZ + m_nHeight <= m_pTargetSocket->m_nZ);

    m_bDoDropRebound = false;

    nRandomSeed = m_nBaseRandomSeed * m_pTargetSocket->m_eFloor;
    nHitRate = m_nHitRate + g_pSO3World->m_Settings.m_ConstList.nAdditionalHitRateOnDrop;
    bHit = (g_RandomBySeed(nRandomSeed, 0, 100) < nHitRate);
    if (!bHit)
    {
        m_nVelocityY = g_RandomBySeed(nRandomSeed, g_pSO3World->m_Settings.m_ConstList.nRandomMinVYOnDrop, g_pSO3World->m_Settings.m_ConstList.nRandomMaxVYOnDrop);
        m_nVelocityX = g_RandomBySeed(nRandomSeed, g_pSO3World->m_Settings.m_ConstList.nRandomMinVXOnDrop, g_pSO3World->m_Settings.m_ConstList.nRandomMaxVXOnDrop);
        if (m_pTargetSocket->m_eDir == csdLeft)
            m_nVelocityX = -m_nVelocityX;

        LogInfo("In %s, Floor:%d, BaseRandomSeed:%d RandomSeed:%d, HitRate:%d XYZ:%d,%d,%d VXYZ:%d,%d,%d", 
            __FUNCTION__, m_pTargetSocket->m_eFloor, m_nBaseRandomSeed, nRandomSeed, nHitRate, m_nX, m_nY, m_nZ, m_nVelocityX, m_nVelocityY, m_nVelocityZ);
    }
    else
    {
        m_nVelocityZ = 0;
    }

Exit0:
    return;
}

int KBall::LuaGetTaker(Lua_State* L)
{
    KHero* pTaker = GetBallTaker();
    KG_PROCESS_ERROR(pTaker);
    return pTaker->LuaGetObj(L);
Exit0:
    return 0;
}

//////////////////////////////////////////////////////////////////////////

DEFINE_LUA_CLASS_BEGIN(KBall)
    REGISTER_LUA_DWORD(KObject, ID)
    REGISTER_LUA_INTEGER(KSceneObject, X)
    REGISTER_LUA_INTEGER(KSceneObject, Y)
    REGISTER_LUA_INTEGER(KSceneObject, Z)
    REGISTER_LUA_FUNC(KSceneObject, GetPosition)
    REGISTER_LUA_FUNC(KSceneObject, SetPosition)
    REGISTER_LUA_FUNC(KMovableObject, ClearVelocity)
    REGISTER_LUA_FUNC(KBall, GetTaker)
DEFINE_LUA_CLASS_END(KBall)

