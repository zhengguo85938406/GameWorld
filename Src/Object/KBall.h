// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBall.h 
//  Creator 	: Xiayong  
//  Date		: 09/20/2011
//  Comment	: 
// ***************************************************************
#pragma once
#include "KDoodad.h"
#include "GlobalStruct.h"

class KHero;
class KBasketSocket;
struct KSlamBallSkill;
class KBall : public KDoodad
{
public:
    KBall();
    ~KBall();

    KSceneObjectType GetType() const;

    void Activate(int nCurLoop);
    void ProcessCollision();
    void ProcessAfterCollision();

    BOOL BeShoot(KHero* pShooter, KBasketSocket* pTargetSocket, int nInterference);
    BOOL SkillShoot(KHero* pShooter, KBasketSocket* pSocket);
    BOOL SlamDunk(KHero* pSlamer, KBasketSocket* pTargetSocket);
    BOOL SkillSlam(KHero* pSlamer, KBasketSocket* pTargetSocket);
    BOOL SkillSlamEx(KHero* pSlamer, KSlamBallSkill* pSkill, KBasketSocket* pTargetSocket);
    BOOL ShootWithComplexPath(KHero* pShooter, DWORD dwPathID);
    BOOL PassTo(KHero* pPasser, KHero* pReceiver);
    BOOL CanSlam(KHero* pSlamer, KBasketSocket* pTargetSocket);

    KHero* GetBallTaker();

    void Freeze(int nFrame);
private:
    virtual void OnTrackMoveFinished();
    virtual void OnTrackMoveInterrupted();

    BOOL CanSkillSlam(KHero* pSlamer, KBasketSocket* pTargetSocket);
    KPOSITION GetSkillSlamDstPos(KHero* pSlamer, KBasketSocket* pTargetSocket);

    KPOSITION GetPassSrcPos(KHero* pPasser, KHero* pReceiver) const;
    KPOSITION GetPassDstPos(KHero* pPasser, KHero* pReceiver) const;
    int GetPassZMax(const KPOSITION& cSrc, const KPOSITION& cDst) const;

    KPOSITION GetShootStartPos(KHero* pShooter) const;
    KPOSITION GetShootEndPos(KBasketSocket* pTargetSocket) const;

    void CheckWhetherScored();

    void DoDropRebound();

private:
    int LuaGetTaker(Lua_State* L);

public:
    BOOL                m_bLastFrameState;
    BOOL                m_bCurFrameState;
    KShootType          m_eLastBeShootType;
    BOOL                m_bShootFlagForCandyBag;
    int                 m_nFreezeReleaseFrame;


    struct KDelayAction
    {
        int nFrame;
        int nX;
        int nY;
        int nZ;
        int nVX;
        int nVY;
        int nVZ;
    } m_DelayAction;

    int                 m_nBaseRandomSeed;
    int                 m_nHitRate;

private:
    BOOL                m_bRestAfterTrackMove;
    BOOL                m_bDoDropRebound;

public:
    DECLARE_LUA_CLASS(KBall);
};
