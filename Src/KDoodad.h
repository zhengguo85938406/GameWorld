// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KDoodad.h
//  Creator 	: Hanruofei  
//  Date		: 12/14/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "KMovableObject.h"
#include "game_define.h"
#include "GlobalEnum.h"

enum KTrackMoveAction
{
    tmaNone,
    tmaShoot,
    tmaBallSkillSlam,
};

class KHero;
class KBasketSocket;
class IPath;
class KFix2PParabola;

struct KPOSITION;
struct KVELOCITY;

class KDoodad : public KMovableObject
{
public:
    KDoodad();
    ~KDoodad();

    BOOL BeToken(KHero* pHero);
    BOOL BeUnTake(KHero* pTaker);
    BOOL BeThrown(KHero* pHero, KTWO_DIRECTION eDir);
    BOOL BeAttachTo(KHero* pHero);
    BOOL BeDetach(KHero* pHero);
    BOOL BeStandOn(KMovableObject* pObj);

    virtual BOOL BeShoot(KHero* pShooter, KBasketSocket* pTargetSocket);

    virtual int GetCustomData(int nIndex);
    
    virtual BOOL CanBeStandOnBy(KMovableObject* pObj) const;

    virtual BOOL BeUseBy(KHero* pHero, KTWO_DIRECTION eDir);

    virtual BOOL BeSpecialUsedBy(KHero* pHero) {return false;};
    
    virtual BOOL CanGetAward();

    BOOL GetAIEventRange(KHero* pUser, KBODY& rRetRange);

    BOOL CanBeTokenBy(KHero* pHero);
public:
    BOOL                m_bCanToken;
    BOOL                m_bCanThrown;
    BOOL                m_bCanAttach;
    BOOL                m_bCanStood;
    BOOL                m_bCanShot;

    DWORD               m_dwThrowerID;
    DWORD               m_dwShooterID;   

    BOOL                m_bCanHit;
    KBasketSocket*      m_pTargetSocket;

    KTrackMoveAction    m_eActionType;
    int                 m_nTotalFrame;
    int                 m_nCreateFrame;
    DWORD               m_dwAttakerID;

protected:
    BOOL CanBeShootBy(KHero* pShooter) const;

private:

    BOOL CanBeThrownBy(KHero* pHero) const;
    BOOL CanBeAttachedTo(KHero* pHero) const;

    virtual void OnThrowOut(KHero* pHero) {};

    KFix2PParabola GetShootPath(KHero* pShooter,KBasketSocket* pTargetSocket);

};
