#include "stdafx.h"
#include "KCollisionMgr.h"
#include "KBall.h"
#include "KBasket.h"
#include "KPlayerServer.h"
#include "KPlayer.h"
#include "KScene.h"
#include "KLadder.h"
#include "KSO3World.h"
#include "KAttribute.h"
#include "KSkill.h"
#include "KBattle.h"
#include "KChest.h"
#include "KBackboard.h"
#include "KBullet.h"
#include "KTrap.h"
#include "KLandMine.h"
#include "KBrick.h"
#include "KClip.h"
#include "KTyre.h"
#include "KCandyBag.h"
#include "KBaseball.h"
#include "KBananaPeel.h"
#include "KDummy.h"
#include "KBuffBox.h"
#include "KGift.h"
#include "KAwardMgr.h"
#include "KMissionMgr.h"
#include "KGold.h"
#include "KBuffObj.h"

#define REGISTER_COLLISION_FUN(srctype, desttype, beginfun, endfun) \
    llKey = MAKE_INT64(srctype, desttype);                          \
    m_mapToCollisionProcessor[llKey].BeginFun = beginfun;           \
    m_mapToCollisionProcessor[llKey].EndFun = endfun; 

KCollisionMgr::KCollisionMgr(void)
{
    m_nMinVelocityCollidedBasket = 8;
}

KCollisionMgr::~KCollisionMgr(void)
{
}

BOOL KCollisionMgr::Init(KScene* pScene)
{
    BOOL bResult = false;
    int64_t llKey = 0;

    KGLOG_PROCESS_ERROR(pScene);
    m_pScene = pScene;

    m_nMinVelocityCollidedBasket = g_pSO3World->m_Settings.m_ConstList.nMinVelocityCollidedBasket;

    REGISTER_COLLISION_FUN(sotBall, sotBasket, &KCollisionMgr::OnBallCollisionBasket, NULL);
    REGISTER_COLLISION_FUN(sotBall, sotBackboard, &KCollisionMgr::OnBallCollisionBackboard, NULL);
    REGISTER_COLLISION_FUN(sotBall, sotHero, &KCollisionMgr::OnBallCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotHero, sotBall, &KCollisionMgr::OnHeroCollisionBall, NULL);
    REGISTER_COLLISION_FUN(sotChest, sotHero, &KCollisionMgr::OnChestCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotLadder, sotHero, &KCollisionMgr::OnLadderCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotLadder, sotBasket, &KCollisionMgr::OnLadderCollisionBasket, NULL);
    REGISTER_COLLISION_FUN(sotChest, sotBasket, &KCollisionMgr::OnChestCollisionBasket, NULL);
    REGISTER_COLLISION_FUN(sotBall, sotChest, &KCollisionMgr::OnBallCollisionChest, NULL);
    REGISTER_COLLISION_FUN(sotBall, sotLadder, &KCollisionMgr::OnBallCollisionLadder, NULL);
    REGISTER_COLLISION_FUN(sotBullet, sotHero, &KCollisionMgr::OnBulletCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotHero, sotTrap, &KCollisionMgr::OnHeroCollisionTrapBegin, &KCollisionMgr::OnHeroCollisionTrapEnd);
    REGISTER_COLLISION_FUN(sotHero, sotLandMine, &KCollisionMgr::OnHeroCollisionLandMine, NULL);
    REGISTER_COLLISION_FUN(sotBrick, sotHero, &KCollisionMgr::OnBrickCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotHero, sotClip, &KCollisionMgr::OnHeroCollisionClip, NULL);
    REGISTER_COLLISION_FUN(sotClip, sotHero, &KCollisionMgr::OnClipCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotTyre, sotHero, &KCollisionMgr::OnTyreCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotLandMine, sotHero, &KCollisionMgr::OnLandMineCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotBall, sotCandyBag, &KCollisionMgr::OnBallCollisionCandyBag, NULL);
    REGISTER_COLLISION_FUN(sotBaseball, sotHero, &KCollisionMgr::OnBaseballCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotHero, sotBananaPeel, &KCollisionMgr::OnHeroCollisionBananaPeel, NULL);
    REGISTER_COLLISION_FUN(sotBananaPeel, sotHero, &KCollisionMgr::OnBananaPeelCollisionHero, NULL);
    REGISTER_COLLISION_FUN(sotLadder, sotDummy, &KCollisionMgr::OnAttackObjCollisionDummy, NULL);
    REGISTER_COLLISION_FUN(sotBrick, sotDummy, &KCollisionMgr::OnAttackObjCollisionDummy, NULL);
    REGISTER_COLLISION_FUN(sotTyre, sotDummy, &KCollisionMgr::OnAttackObjCollisionDummy, NULL);
    REGISTER_COLLISION_FUN(sotBall, sotDummy, &KCollisionMgr::OnBallCollisionDummy, NULL);

    REGISTER_COLLISION_FUN(sotBall, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotBasket, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotChest, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotLadder, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotLandMine, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotBrick, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotClip, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotTyre, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotBaseball, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);
    REGISTER_COLLISION_FUN(sotBananaPeel, sotTrap, &KCollisionMgr::OnObjCollisionTrap, NULL);

    REGISTER_COLLISION_FUN(sotHero, sotGift, &KCollisionMgr::OnHeroCollisionGift, NULL);
    REGISTER_COLLISION_FUN(sotGift, sotHero, &KCollisionMgr::OnGiftCollisionHero, NULL);

    REGISTER_COLLISION_FUN(sotLadder, sotBuffBox, &KCollisionMgr::OnAttackObjCollisionBuffBox, NULL);
    REGISTER_COLLISION_FUN(sotBrick, sotBuffBox, &KCollisionMgr::OnAttackObjCollisionBuffBox, NULL);
    REGISTER_COLLISION_FUN(sotTyre, sotBuffBox, &KCollisionMgr::OnAttackObjCollisionBuffBox, NULL);

    REGISTER_COLLISION_FUN(sotHero, sotBuffObj, &KCollisionMgr::OnHeroCollisionBuffObj, NULL);
    REGISTER_COLLISION_FUN(sotBuffObj, sotHero, &KCollisionMgr::OnBuffObjCollisionHero, NULL);

    REGISTER_COLLISION_FUN(sotHero, sotGold, &KCollisionMgr::OnHeroCollisionGold, NULL);
    REGISTER_COLLISION_FUN(sotGold, sotHero, &KCollisionMgr::OnGoldCollisionHero, NULL);

    bResult = true;
Exit0:
    return bResult;
}

void KCollisionMgr::UnInit()
{
}

void KCollisionMgr::OnCollisionBegin(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL            bRetCode = false;
    int64_t         llKey = 0;
    KMAP_TO_COLLISION_PROCESSOR::iterator it;
    KCollisionProcessor Func = NULL;

    llKey = MAKE_INT64(pSource->GetType(), pDest->GetType());

    it = m_mapToCollisionProcessor.find(llKey);
    KG_PROCESS_SUCCESS(it == m_mapToCollisionProcessor.end());

    Func = it->second.BeginFun;
    KG_PROCESS_SUCCESS(Func == NULL);

    (this->*Func)(pSource, pDest);

Exit1:
Exit0:
    return;
}

void KCollisionMgr::OnCollisionEnd(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL            bRetCode = false;
    int64_t         llKey = 0;
    KMAP_TO_COLLISION_PROCESSOR::iterator it;
    KCollisionProcessor Func = NULL;

    llKey = MAKE_INT64(pSource->GetType(), pDest->GetType());

    it = m_mapToCollisionProcessor.find(llKey);
    KG_PROCESS_SUCCESS(it == m_mapToCollisionProcessor.end());

    Func = it->second.EndFun;
    KG_PROCESS_SUCCESS(Func == NULL);

    (this->*Func)(pSource, pDest);

Exit1:
Exit0:
    return;
}

void KCollisionMgr::OnBallCollisionBasket(KSceneObject* pSource, KSceneObject* pDest)
{
    KBall* pBall = NULL;
    KBasket* pBasket = NULL;
    int nToLeftRightDistance = 0;
    int nToUpDownDistance = 0;
    int nBallRadius = 0;

    assert(pSource->GetType() == sotBall);
    pBall = (KBall*)pSource;

    assert(pDest->GetType() == sotBasket);
    pBasket = (KBasket*)pDest;

    KG_PROCESS_ERROR(pBasket->GetMoveType() == mosFixed);

    //LogInfo("Ball(%s) Collision Basket(%s).", pBall->ToStatusString().c_str(), pBasket->ToStatusString().c_str());

    nToLeftRightDistance = MIN(
        abs(pBall->m_nX - pBasket->m_nX + pBasket->m_nLength / 2),
        abs(pBall->m_nX - pBasket->m_nX - pBasket->m_nLength / 2));//到前后框较小距离
    nToUpDownDistance = MIN(
        abs(pBall->m_nY - pBasket->m_nY + pBasket->m_nWidth  / 2),
        abs(pBall->m_nY - pBasket->m_nY - pBasket->m_nWidth  / 2));//到上下框较小距离

    //在篮筐中可进范围
    assert(pBall->m_nHeight == pBall->m_nLength);
    assert(pBall->m_nLength == pBall->m_nWidth);
    nBallRadius = pBall->m_nHeight / 2;
    if (
        pBall->m_nX - nBallRadius > pBasket->m_nX - pBasket->m_nLength / 2 &&
        pBall->m_nX + nBallRadius < pBasket->m_nX + pBasket->m_nLength / 2 &&
        pBall->m_nY - nBallRadius > pBasket->m_nY - pBasket->m_nWidth  / 2 &&
        pBall->m_nY + nBallRadius < pBasket->m_nY + pBasket->m_nWidth  / 2
        )//篮筐内没挂到框
    {
        if (pBall->m_nVelocityZ <= 0 && pBasket->m_pPluginSocket != NULL ) //球必须向下飞，能够得分
        {
            pBall->m_pTargetSocket = pBasket->m_pPluginSocket;
            pBall->m_nVelocityX = 0;
            pBall->m_nVelocityY = 0;
            pBall->m_nVelocityZ = 0;
            if (!pBall->m_bSomeThingOnBasket)
            {
                pBall->m_bCurFrameState = true;
            }

            pBall->TurnToMoveType(mosFree);
        }   
    }
    //与前后框有碰撞
    else if (nToLeftRightDistance < nToUpDownDistance) //到前后框距离小于到上下框距离
    {
        if (pBall->m_nVelocityZ < 0) pBall->SetZ(pBasket->m_nZ + pBasket->m_nHeight);      //防止多重检测，进行坐标调整
        if (pBall->m_nVelocityZ > 0) pBall->SetZ(pBasket->m_nZ - pBall->m_nHeight);

        /*KGLogPrintf(KGLOG_INFO, "前后框碰撞！反弹前 Vx = %d  Vy = %d  Vz = %d\n",
        pBall->m_nVelocityX,pBall->m_nVelocityY,pBall->m_nVelocityZ);*/

        int nAfterVelocityX = (int)(-pBall->m_nVelocityX * 0.9);
        int nAfterVelocityY = (int)(pBall->m_nVelocityY * 0.9);

        if (abs(nAfterVelocityX) < m_nMinVelocityCollidedBasket && 
            abs(nAfterVelocityY) < m_nMinVelocityCollidedBasket)
        {
            if (pBall->m_nVelocityX > 0)
                nAfterVelocityX = m_nMinVelocityCollidedBasket;
            else 
                nAfterVelocityX = -m_nMinVelocityCollidedBasket;
            nAfterVelocityY = pBall->m_nVelocityY;
        }

        pBall->m_nVelocityX = nAfterVelocityX;
        pBall->m_nVelocityY = nAfterVelocityY;

        if (pBall->m_nVelocityX == 0 && pBall->m_nVelocityY == 0)
        {
            pBall->m_nVelocityY = -m_nMinVelocityCollidedBasket; // 防卡死
        }         

        pBall->m_nVelocityZ = (int)(-pBall->m_nVelocityZ * 0.9);                //客户端暂定所有衰减0.9

        pBall->TurnToMoveType(mosFree);

        /* KGLogPrintf(KGLOG_INFO, "前后框碰撞！反弹后 Vx = %d  Vy = %d  Vz = %d 弹后坐标 = %d,%d,%d  帧 = %d\n",
        pBall->m_nVelocityX,pBall->m_nVelocityY,pBall->m_nVelocityZ, 
        pBall->m_nX, pBall->m_nY, pBall->m_nZ, g_pSO3World->m_nGameLoop);*/
    }
    //与上下框有碰撞
    else
    {
        if (pBall->m_nVelocityZ < 0) pBall->SetZ(pBasket->m_nZ + pBasket->m_nHeight);     //防止多重检测，进行坐标调整
        if (pBall->m_nVelocityZ > 0) pBall->SetZ(pBasket->m_nZ - pBall->m_nHeight);

        /*KGLogPrintf(KGLOG_INFO, "上下框碰撞！反弹前 Vx = %d  Vy = %d  Vz = %d\n",
        pBall->m_nVelocityX,pBall->m_nVelocityY,pBall->m_nVelocityZ);*/

        int nAfterVelocityX = (int)(pBall->m_nVelocityX * 0.9);
        int nAfterVelocityY = (int)(-pBall->m_nVelocityY * 0.9);        

        if (abs(nAfterVelocityX) < m_nMinVelocityCollidedBasket && 
            abs(nAfterVelocityY) < m_nMinVelocityCollidedBasket)
        {
            nAfterVelocityX = pBall->m_nVelocityX;

            //球与篮筐的碰撞可以导致进球
            if (pBall->m_nVelocityY > 0)
                nAfterVelocityY = m_nMinVelocityCollidedBasket;
            else
                nAfterVelocityY = -m_nMinVelocityCollidedBasket;
        }

        pBall->m_nVelocityX = nAfterVelocityX;
        pBall->m_nVelocityY = nAfterVelocityY;

        if (pBall->m_nVelocityX == 0 && pBall->m_nVelocityY == 0)
        {
            pBall->m_nVelocityY = -m_nMinVelocityCollidedBasket;// 防卡死
        }        

        pBall->m_nVelocityZ = (int)(-pBall->m_nVelocityZ * 0.9);        

        pBall->TurnToMoveType(mosFree);

        /*KGLogPrintf(KGLOG_INFO, "上下框碰撞！反弹后 Vx = %d  Vy = %d  Vz = %d 弹后坐标 = %d,%d,%d  帧 = %d\n",
        pBall->m_nVelocityX,pBall->m_nVelocityY,pBall->m_nVelocityZ, 
        pBall->m_nX, pBall->m_nY, pBall->m_nZ, g_pSO3World->m_nGameLoop);*/
    }

    // 遇到碰撞有轨运动结束
    pBall->m_bCanHit = false;

Exit0:
    return;
}

void KCollisionMgr::OnBallCollisionBackboard(KSceneObject* pSource, KSceneObject* pDest)
{
    KBall* pBall = NULL;
    KBackboard* pBackboard = NULL;

    assert(pSource->GetType() == sotBall);
    pBall = (KBall*)(pSource);

    assert(pDest->GetType() == sotBackboard);
    pBackboard = (KBackboard*)(pDest);

    //LogInfo("篮板碰撞！反弹前 Ball(%s) Collision Backboard(%s).", pBall->ToStatusString().c_str(), pBackboard->ToStatusString().c_str());

    if (pBall->m_nVelocityX < 0) pBall->SetX(pBackboard->m_nX + pBackboard->m_nLength / 2 + pBall->m_nWidth / 2 + 1);
    if (pBall->m_nVelocityX > 0) pBall->SetX(pBackboard->m_nX - pBackboard->m_nLength / 2 - pBall->m_nWidth / 2 - 1);

    pBall->m_nVelocityX = (int)(-pBall->m_nVelocityX * 0.5);
    pBall->m_nVelocityY = (int)(pBall->m_nVelocityY * 0.5);
    pBall->m_nVelocityZ = (int)(pBall->m_nVelocityZ * 0.5);

    pBall->TurnToMoveType(mosFree);

    /*KGLogPrintf(KGLOG_INFO, "篮板碰撞！反弹后Vx = %d  Vy = %d  Vz = %d 弹后坐标 = %d,%d,%d  帧 = %d\n", 
    pBall->m_nVelocityX, pBall->m_nVelocityY, pBall->m_nVelocityZ, pBall->m_nX, pBall->m_nY, pBall->m_nZ,
    g_pSO3World->m_nGameLoop);*/

Exit0:
    return;
}

void KCollisionMgr::OnBallCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL            bRetCode    = false;
    KBall*          pBall       = NULL;
    KHero*          pHero  = NULL;

    assert(pSource->GetType() == sotBall);
    pBall = (KBall*)pSource;

    assert(pDest->GetType() == sotHero);
    pHero = (KHero*)pDest;

    if (pBall->m_bAttackState)
    {
        KSkill*     pSkill      = NULL;
        KHero*      pShooter    = NULL;

        KG_PROCESS_ERROR(pBall->m_dwShooterID != pHero->m_dwID);

        bRetCode = pHero->CanBeAttacked();
        KG_PROCESS_ERROR(bRetCode);

        pShooter = m_pScene->GetHeroById(pBall->m_dwShooterID);
        KGLOG_PROCESS_ERROR(pShooter);

        KG_PROCESS_ERROR(pShooter->m_nSide != pHero->m_nSide);

        bRetCode = pHero->ApplyGeneralAffect(pShooter->m_dwID, pBall, pBall->m_pTemplate->m_dwAttackAffectID1);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        KMoveType eType = pBall->GetMoveType();
        KG_PROCESS_ERROR(eType == mosFree || eType == mosTrackMove || eType == mosIdle);

        pHero->AutoTakeBall();
    }

Exit0:
    return;
}

void KCollisionMgr::OnHeroCollisionBall(KSceneObject* pSource, KSceneObject* pDest)
{
    OnBallCollisionHero(pDest, pSource);
}

void KCollisionMgr::OnChestCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    KChest* pChest  = NULL;
    KHero*  pHero   = NULL;
    KMoveType eMoveType = mosInvalid;

    assert(pSource->GetType() == sotChest);
    pChest = (KChest*)pSource;

    assert(pDest->GetType() == sotHero);
    pHero = (KHero*)pDest;

    //LogInfo("Chest(%s) Collision Hero(P:%d,%d,%d).", pChest->ToStatusString().c_str(), pHero->m_nX, pHero->m_nY, pHero->m_nZ);

    eMoveType = pChest->GetMoveType();

    KG_PROCESS_ERROR(eMoveType == mosFree || eMoveType == mosIdle || eMoveType == mosTrackMove);

    pHero->BeThrownChest(pChest);

Exit0:
    return;
}

void KCollisionMgr::OnBulletCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL        bRetCode    = false;
    KBullet*    pBullet     = NULL;
    KHero*      pHero       = NULL;

    assert(pSource->GetType() == sotBullet);
    pBullet = (KBullet*)pSource;

    assert(pDest->GetType() == sotHero);
    pHero = (KHero*)pDest;

    bRetCode = pBullet->CollideHero(pHero);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KCollisionMgr::OnLadderCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    KLadder* pLadder = NULL;
    KHero* pHero = NULL;
    KHero* pThrower = NULL;
    BOOL bRetCode = false;

    assert(pSource->GetType() == sotLadder);
    pLadder = (KLadder*)(pSource);

    assert(pDest->GetType() == sotHero);
    pHero = (KHero*)(pDest);

    KG_PROCESS_ERROR(pLadder->m_dwThrowerID != ERROR_ID);

    pThrower = m_pScene->GetHeroById(pLadder->m_dwThrowerID);
    KGLOG_PROCESS_ERROR(pThrower);

    KG_PROCESS_ERROR(pThrower->m_nSide != pHero->m_nSide);

    bRetCode = pLadder->IsFollowedBy(pHero);
    KG_PROCESS_ERROR(!bRetCode);

    bRetCode = pHero->CanBeAttacked();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pHero->ApplyGeneralAffect(pThrower->m_dwID, pLadder, pLadder->m_pTemplate->m_dwAttackAffectID1);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KCollisionMgr::OnLadderCollisionBasket(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL bRetCode = false;
    KLadder* pLadder = NULL;
    KBasket* pBasket = NULL;
    double dZSpeed = 0.0;
    KMoveType eType = mosIdle;

    assert(pSource->GetType() == sotLadder);
    pLadder = (KLadder*)(pSource);

    assert(pDest->GetType() == sotBasket);
    pBasket = (KBasket*)(pDest);

    eType = pLadder->GetMoveType();

    KG_PROCESS_ERROR(pBasket->m_pPluginSocket);
    KG_PROCESS_ERROR(eType == mosTrackMove || eType == mosFree);
    KG_PROCESS_ERROR(pLadder->m_dwShooterID != ERROR_ID);
    KG_PROCESS_ERROR(pLadder->GetMoveType() != mosStandOn);
    KG_PROCESS_ERROR(pLadder->m_nVelocityZ < 0);

    bRetCode = pLadder->HasFollower();
    KG_PROCESS_ERROR(!bRetCode);

    //LogInfo("Ladder(%s) Collision Basket(%s).", pLadder->ToStatusString().c_str(), pBasket->ToStatusString().c_str());

    pBasket->BeStandOn(pLadder);

    g_PlayerServer.DoObjStandObj(pLadder, pBasket);

Exit0:
    return;
}

void KCollisionMgr::OnLadderCollisionBackboard(KSceneObject* pSource, KSceneObject* pDest)
{
    KLadder* pLadder = NULL;
    KBackboard* pBackboard = NULL;

    assert(pSource->GetType() == sotLadder);
    pLadder = (KLadder*)(pSource);

    assert(pDest->GetType() == sotBackboard);
    pBackboard = (KBackboard*)(pDest);

    //LogInfo("Ladder(%s) Collision Backboard(%s).", pLadder->ToStatusString().c_str(), pBackboard->ToStatusString().c_str());

    if (pLadder->m_nVelocityX < 0) pLadder->SetX(pBackboard->m_nX + pBackboard->m_nLength / 2 + pLadder->m_nWidth / 2);
    if (pLadder->m_nVelocityX > 0) pLadder->SetX(pBackboard->m_nX - pBackboard->m_nLength / 2 - pLadder->m_nWidth / 2);

    pLadder->ClearVelocity();
    pLadder->TurnToMoveType(mosFree);
}

void KCollisionMgr::OnChestCollisionBasket(KSceneObject* pSource, KSceneObject* pDest)
{

}

void KCollisionMgr::OnChestCollisionBackboard(KSceneObject* pSource, KSceneObject* pDest)
{
    KChest* pChest = NULL;
    KBackboard* pBackboard = NULL;

    assert(pSource->GetType() == sotChest);
    pChest = (KChest*)(pSource);

    assert(pDest->GetType() == sotBackboard);
    pBackboard = (KBackboard*)(pDest);
    //LogInfo("Before Chest(%s) Collision Backboard(%s).", pChest->ToStatusString().c_str(), pBackboard->ToStatusString().c_str());

    if (pChest->m_nVelocityX < 0) pChest->SetX(pBackboard->m_nX + pBackboard->m_nLength / 2 + pChest->m_nWidth / 2);
    if (pChest->m_nVelocityX > 0) pChest->SetX(pBackboard->m_nX - pBackboard->m_nLength / 2 - pChest->m_nWidth / 2);

    pChest->ClearVelocity();
    pChest->TurnToMoveType(mosFree);

    //LogInfo("After Chest(%s) Collision Backboard(%s).", pChest->ToStatusString().c_str(), pBackboard->ToStatusString().c_str());
}

void KCollisionMgr::OnBallCollisionChest(KSceneObject* pSource, KSceneObject* pDest)
{

}

void KCollisionMgr::OnBallCollisionLadder(KSceneObject* pSource, KSceneObject* pDest)
{
    KBall* pBall = NULL;
    KLadder* pLadder = NULL;
    BOOL bRetCode = false;
    BOOL bFreeFall = false;
    int nVelocityX = 100;
    int nVelocityY = 100;
    int nVelocityZ = 50;

    assert(pSource->GetType() == sotBall);
    pBall = (KBall*)(pSource);

    assert(pDest->GetType() == sotLadder);
    pLadder = (KLadder*)(pDest);

    //LogInfo("Before Ball(%s) Collision Ladder(%s).", pBall->ToStatusString().c_str(), pLadder->ToStatusString().c_str());

    KG_PROCESS_ERROR(pBall->m_pTargetSocket);
    KG_PROCESS_ERROR(pLadder->IsStandOnObj());

    bFreeFall = (pBall->m_nVelocityX == 0 && pBall->m_nVelocityY == 0 && pBall->m_nVelocityZ < 0);

    if (pLadder->m_nX > m_pScene->m_nXGridCount * CELL_LENGTH / 2)
    {
        nVelocityX = -nVelocityX;
    }

    if (pBall->m_nVelocityY < 0)
    {
        nVelocityY = -nVelocityY;
    }

    pLadder->m_nVelocityX = nVelocityX;
    pLadder->m_nVelocityY = nVelocityY;
    pLadder->m_nVelocityZ = nVelocityZ;

    pBall->m_nVelocityX = (int)(- 0.65 * pBall->m_nVelocityX);
    pBall->m_nVelocityY = (int)(- 0.65 * pBall->m_nVelocityY);
    pBall->m_nVelocityZ = (int)(- 0.65 * pBall->m_nVelocityZ);

    if (bFreeFall)
    {
        pBall->m_nVelocityX = nVelocityX;
        pBall->m_nVelocityY = -nVelocityY;
    }

    pLadder->TurnToMoveType(mosFree);
    pBall->TurnToMoveType(mosFree);

    //LogInfo("After Ball(%s) Collision Ladder(%s).", pBall->ToStatusString().c_str(), pLadder->ToStatusString().c_str());

Exit0:
    return;
}

void KCollisionMgr::OnHeroCollisionTrapBegin(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL        bRetCode    = false;
    KHero*      pHero       = NULL;
    KTrap*      pTrap       = NULL;
    const char* pszFuncName = "OnEnterTrap";
    int         nTopIndex   = 0;

    assert(pSource->GetType() == sotHero);
    pHero = (KHero*)(pSource);

    assert(pDest->GetType() == sotTrap);
    pTrap = (KTrap*)(pDest);

    KG_PROCESS_ERROR(pTrap->m_dwScriptID);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pTrap->m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(pTrap->m_dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

    g_pSO3World->m_ScriptManager.Push(pHero);
    g_pSO3World->m_ScriptManager.Call(pTrap->m_dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KCollisionMgr::OnHeroCollisionTrapEnd(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL        bRetCode    = false;
    KHero*      pHero       = NULL;
    KTrap*      pTrap       = NULL;
    const char* pszFuncName = "OnLeaveTrap";
    int         nTopIndex   = 0;

    assert(pSource->GetType() == sotHero);
    pHero = (KHero*)(pSource);

    assert(pDest->GetType() == sotTrap);
    pTrap = (KTrap*)(pDest);

    KG_PROCESS_ERROR(pTrap->m_dwScriptID);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pTrap->m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(pTrap->m_dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

    g_pSO3World->m_ScriptManager.Push(pHero);
    g_pSO3World->m_ScriptManager.Call(pTrap->m_dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

BOOL KCheckHeroBlowupFunc::operator()(KHero* pHero)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KLandMine* pLandMine = (KLandMine*)m_pLandMine;
    KVELOCITY vBlowupVelocity = {0};
    KPOSITION Src;
    KPOSITION Dest;
    int nDamage = 0;

    assert(m_pLandMine);

    KGLOG_PROCESS_ERROR(pLandMine->m_nBlowupRange > 0);

    bRetCode = pHero->CanBeAttacked();
    KG_PROCESS_SUCCESS(!bRetCode);

    Src   = pLandMine->GetPosition();
    Dest  = pHero->GetPosition();
    Dest.nZ += pHero->m_nHeight / 2;

    bRetCode = g_InRange(Dest.nX, Dest.nY, Dest.nZ, Src.nX, Src.nY, Src.nZ, pLandMine->m_nBlowupRange);
    KG_PROCESS_SUCCESS(!bRetCode);

    bRetCode = pHero->ApplyGeneralAffect(pLandMine->m_dwAttakerID, pLandMine, pLandMine->m_pTemplate->m_dwAttackAffectID1);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KCollisionMgr::OnHeroCollisionLandMine(KSceneObject* pSource, KSceneObject* pDest)
{
    KHero*      pHero       = NULL;
    KLandMine*  pLandMine   = NULL;
    KCheckHeroBlowupFunc BlowupHero;

    assert(pSource->GetType() == sotHero);
    pHero = (KHero*)(pSource);

    assert(pDest->GetType() == sotLandMine);
    pLandMine = (KLandMine*)(pDest);

    //LogInfo("Hero (%s) Collision LandMine(%s).", pHero->ToStatusString().c_str(), pLandMine->ToStatusString().c_str());

    KG_PROCESS_ERROR(pLandMine->m_bAttackState);// 是否激活
    pLandMine->m_bAttackState = false;

    BlowupHero.m_pLandMine = pLandMine;

    pHero->m_pScene->TraverseHero(BlowupHero);

    pLandMine->SetDeleteFlag();

    g_PlayerServer.DoLandMineBlowupBoardcast(pLandMine->m_pScene, pLandMine->m_dwID);

    pLandMine->m_pScene->ApplyObstacle(pLandMine, false);

Exit0:
    return;
}

void KCollisionMgr::OnBrickCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    KBrick*     pBrick      = NULL;
    KHero*      pHero       = NULL;

    assert(pSource->GetType() == sotBrick);
    pBrick = (KBrick*)(pSource);

    assert(pDest->GetType() == sotHero);
    pHero = (KHero*)(pDest);

    pBrick->CollideHero(pHero);
}

void KCollisionMgr::OnHeroCollisionClip(KSceneObject* pSource, KSceneObject* pDest)
{
    assert(pSource->GetType() == sotHero);
    KHero* pHero = (KHero*)pSource;

    assert(pDest->GetType() == sotClip);
    KClip* pClip = (KClip*)pDest;

    pClip->Stampede(pHero);
}

void KCollisionMgr::OnClipCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    OnHeroCollisionClip(pDest, pSource);
}

void KCollisionMgr::OnTyreCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    assert(pSource->GetType() == sotTyre);
    KTyre* pTyre = (KTyre*)pSource;

    assert(pDest->GetType() == sotHero);
    KHero* pHero = (KHero*)pDest;

    pTyre->OnCollideHero(pHero);

}

void KCollisionMgr::OnLandMineCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    OnHeroCollisionLandMine(pDest, pSource);
}

void KCollisionMgr::OnBallCollisionCandyBag(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL        bRetCode    = false;
    KBall*      pBall       = NULL;
    KCandyBag*  pCandyBag   = NULL;

    assert(pSource->Is(sotBall));
    pBall = (KBall*)(pSource);

    assert(pDest->Is(sotCandyBag));
    pCandyBag = (KCandyBag*)(pDest);

    bRetCode = pCandyBag->CollidedByBall(pBall);
    KG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KCollisionMgr::OnBaseballCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL        bRetCode    = false;
    KBaseball*  pBaseball   = NULL;
    KHero*      pHero       = NULL;

    assert(pSource->Is(sotBaseball));
    pBaseball   = (KBaseball*)pSource;

    assert(pDest->Is(sotHero));
    pHero       = (KHero*)pDest;

    bRetCode = pBaseball->CollideHero(pHero);
    KG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KCollisionMgr::OnHeroCollisionBananaPeel(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL            bRetCode    = false;
    KHero*          pHero       = NULL;
    KBananaPeel*    pBananaPeel = NULL;

    assert(pSource->Is(sotHero));
    pHero       = (KHero*)pSource;

    assert(pDest->Is(sotBananaPeel));
    pBananaPeel = (KBananaPeel*)pDest;

    bRetCode = pBananaPeel->CollidedByHero(pHero);
    KG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KCollisionMgr::OnBananaPeelCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    OnHeroCollisionBananaPeel(pDest, pSource);
}

void KCollisionMgr::OnAttackObjCollisionDummy( KSceneObject* pSource, KSceneObject* pDest )
{
    KDummy*			pDummy		= NULL;

    assert(pDest->Is(sotDummy));
    pDummy = (KDummy*)pDest;
    pDummy->BeAttacked();
}

void KCollisionMgr::OnBallCollisionDummy(KSceneObject* pSource, KSceneObject* pDest)
{
    KBall*          pBall       = NULL;
    KDummy*			pDummy		= NULL;

    assert(pSource->GetType() == sotBall);
    pBall = (KBall*)pSource;

    if (pBall->m_bAttackState)
    {
        assert(pDest->Is(sotDummy));
        pDummy = (KDummy*)pDest;
        pDummy->BeAttacked();
    }

    return;
}

void KCollisionMgr::OnObjCollisionTrap(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL        bRetCode    = false;
    KTrap*      pTrap       = NULL;
    const char* pszFuncName = "OnObjEnterTrap";
    int         nTopIndex   = 0;

    assert(pDest->GetType() == sotTrap);
    pTrap = (KTrap*)(pDest);

    KG_PROCESS_ERROR(pTrap->m_dwScriptID);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pTrap->m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(pTrap->m_dwScriptID, pszFuncName);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(pSource->m_pScene);
    g_pSO3World->m_ScriptManager.Push(pSource->m_dwID);
    g_pSO3World->m_ScriptManager.Call(pTrap->m_dwScriptID, pszFuncName, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KCollisionMgr::OnHeroCollisionGift(KSceneObject* pSource, KSceneObject* pDest)
{
    return OnGiftCollisionHero(pDest,pSource);
}

void KCollisionMgr::OnGiftCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL            bRetCode        = false;
    KGift*          pGift            = NULL;
    KHero*          pHero           = NULL;
    KAwardTable*    pAwardTable     = NULL;
    KAWARD_ITEM*    pAwardItem      = NULL;
    KPlayer*        pPlayer         = NULL;
    IItem*          pItem           = NULL;
    KScene*         pScene          = NULL;
    KMapParams*     pMapParams              = NULL;
    std::vector<KHero*> vecAllTeammate;
    std::vector<KPlayer*> vecPlayer;
   
    KG_PROCESS_ERROR(!pSource->IsToBeDelete());
    assert(pSource->GetType() == sotGift);
    pGift = (KGift*)pSource;

    assert(pDest->GetType() == sotHero);
    pHero = (KHero*)pDest;

    KG_PROCESS_ERROR(pGift->CanGetAward());
    KG_PROCESS_ERROR(pHero->IsMainHero());

    //自己获得获得物品 队友获得物品
    pScene = pHero->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);
    
    vecPlayer.push_back(pHero->GetOwner());

    m_pScene->GetAllTeammate(pHero, vecAllTeammate);

    for (size_t i = 0; i < vecAllTeammate.size(); ++i)
    {
        pHero = vecAllTeammate[i];
        KGLOG_PROCESS_ERROR(pHero);
        if (pHero->IsMainHero())
            vecPlayer.push_back(pHero->GetOwner());
    }

    m_pScene->RemoveSceneObject(pGift);

    pMapParams = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(m_pScene->m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapParams);

    g_pSO3World->m_AwardMgr.ScenenItemAward(vecPlayer, pMapParams->dwItemAwardID);

Exit0:
    return;
}

void KCollisionMgr::OnHeroCollisionGold(KSceneObject* pSource, KSceneObject* pDest)
{
    return OnGoldCollisionHero(pDest,pSource);
}

void KCollisionMgr::OnGoldCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL            bRetCode        = false;
    KGold*          pGold           = NULL;
    KHero*          pHero           = NULL;
    KMission*       pMission        = NULL;    
    KPlayer*        pPlayer         = NULL;
    int             nMoney          = 0;
    std::vector<KHero*> vecAllTeammate;
    std::vector<KPlayer*> vecPlayer;
    
    KG_PROCESS_ERROR(!pSource->IsToBeDelete());
    assert(pSource->GetType() == sotGold);
    pGold = (KGold*)pSource;
    nMoney = pGold->GetMoney(); 
    assert(nMoney > 0);

    KG_PROCESS_ERROR(pGold->CanGetAward());

    assert(pDest->GetType() == sotHero);
    pHero = (KHero*)pDest;

    KG_PROCESS_ERROR(pHero->IsMainHero());

    vecPlayer.push_back(pHero->GetOwner());

    m_pScene->GetAllTeammate(pHero, vecAllTeammate);

    for (size_t i = 0; i < vecAllTeammate.size(); ++i)
    {
        pHero = vecAllTeammate[i];
        KGLOG_PROCESS_ERROR(pHero);
        if (pHero->IsMainHero())
            vecPlayer.push_back(pHero->GetOwner());
    }

    for (size_t i = 0; i < vecPlayer.size(); ++i)
    {
        pPlayer = vecPlayer[i];
        pPlayer->m_MoneyMgr.AddMoney(emotMoney,nMoney);
        PLAYER_LOG(pPlayer, "money,addmoney,%d,%s,%d-%d,%d,%d", emotMoney, "PickGold", 0, 0, 0, nMoney);
    }

    m_pScene->RemoveSceneObject(pGold);

Exit0:
    return;
}

void KCollisionMgr::OnAttackObjCollisionBuffBox(KSceneObject* pSource, KSceneObject* pDest)
{
    KBuffBox*			pBuffBox= NULL;

    assert(pDest->Is(sotBuffBox));
    pBuffBox = (KBuffBox*)pDest;
    pBuffBox->BeAttacked();
}
void KCollisionMgr::OnHeroCollisionBuffObj(KSceneObject* pSource, KSceneObject* pDest)
{
    return OnBuffObjCollisionHero(pDest,pSource);
}

void KCollisionMgr::OnBuffObjCollisionHero(KSceneObject* pSource, KSceneObject* pDest)
{
    BOOL            bRetCode        = false;
    KBuffObj*       pBuffObj        = NULL;
    KHero*          pHero           = NULL;
    DWORD           dwBuffID        = 0;
    
    KG_PROCESS_ERROR(!pSource->IsToBeDelete());
    assert(pSource->GetType() == sotBuffObj);
    pBuffObj = (KBuffObj*)pSource;

    dwBuffID = pBuffObj->GetCustomData(0);
    assert(pDest->GetType() == sotHero);

    pHero = (KHero*)pDest;
    
    KG_PROCESS_ERROR(pHero->IsMainHero());
    
    pHero->AddBuff(dwBuffID);
    m_pScene->RemoveSceneObject(pBuffObj);

Exit0:
    return;
}
