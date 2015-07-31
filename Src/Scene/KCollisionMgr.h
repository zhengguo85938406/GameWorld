#pragma once
#include "KSceneGlobal.h"
#include "Protocol/gs2cl_protocol.h"

class KScene;
class KSceneObject;
class KHero;

class KCollisionMgr
{
public:
    KCollisionMgr(void);
    virtual ~KCollisionMgr(void);

    BOOL Init(KScene* pScene);
    void UnInit();

    void OnCollisionBegin(KSceneObject* pSource, KSceneObject* pDest);
    void OnCollisionEnd(KSceneObject* pSource, KSceneObject* pDest);

protected:
    typedef void (KCollisionMgr::*KCollisionProcessor)(KSceneObject* pSource, KSceneObject* pDest);

    struct KCollisionFun 
    {
        KCollisionProcessor BeginFun;
        KCollisionProcessor EndFun;
    };

    typedef std::map<int64_t, KCollisionFun> KMAP_TO_COLLISION_PROCESSOR;

protected:
    KScene* m_pScene;
    KMAP_TO_COLLISION_PROCESSOR     m_mapToCollisionProcessor;
    int                             m_nMinVelocityCollidedBasket; // 篮球碰到篮筐后应该具有的最小Y方向速度，这是为了保证篮球不停留在篮筐上
private:
    void OnBallCollisionBasket(KSceneObject* pSource, KSceneObject* pDest);
    void OnBallCollisionBackboard(KSceneObject* pSource, KSceneObject* pDest);
    void OnBallCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
    void OnHeroCollisionBall(KSceneObject* pSource, KSceneObject* pDest);
    void OnChestCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
    void OnBulletCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
    void OnLadderCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
    void OnLadderCollisionBasket(KSceneObject* pSource, KSceneObject* pDest);
    void OnLadderCollisionBackboard(KSceneObject* pSource, KSceneObject* pDest);
    void OnChestCollisionBasket(KSceneObject* pSource, KSceneObject* pDest);
    void OnChestCollisionBackboard(KSceneObject* pSource, KSceneObject* pDest);
    void OnBallCollisionChest(KSceneObject* pSource, KSceneObject* pDest);
    void OnBallCollisionLadder(KSceneObject* pSource, KSceneObject* pDest);
    void OnHeroCollisionTrapBegin(KSceneObject* pSource, KSceneObject* pDest);
    void OnHeroCollisionTrapEnd(KSceneObject* pSource, KSceneObject* pDest);
    void OnHeroCollisionLandMine(KSceneObject* pSource, KSceneObject* pDest);
    void OnBrickCollisionHero(KSceneObject* pSource, KSceneObject* pDest);

    void OnHeroCollisionClip(KSceneObject* pSource, KSceneObject* pDest);
    void OnClipCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
    void OnTyreCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
    void OnLandMineCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
    void OnBallCollisionCandyBag(KSceneObject* pSource, KSceneObject* pDest);
    void OnBaseballCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
    void OnHeroCollisionBananaPeel(KSceneObject* pSource, KSceneObject* pDest);
    void OnBananaPeelCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
	void OnAttackObjCollisionDummy(KSceneObject* pSource, KSceneObject* pDest);
    void OnBallCollisionDummy(KSceneObject* pSource, KSceneObject* pDest);
    void OnObjCollisionTrap(KSceneObject* pSource, KSceneObject* pDest);

    void OnHeroCollisionGift(KSceneObject* pSource, KSceneObject* pDest);
    void OnGiftCollisionHero(KSceneObject* pSource, KSceneObject* pDest);

    void OnHeroCollisionGold(KSceneObject* pSource, KSceneObject* pDest);
    void OnGoldCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
	void OnAttackObjCollisionBuffBox(KSceneObject* pSource, KSceneObject* pDest);
	void OnHeroCollisionBuffObj(KSceneObject* pSource, KSceneObject* pDest);
	void OnBuffObjCollisionHero(KSceneObject* pSource, KSceneObject* pDest);
};

struct KCheckHeroBlowupFunc
{
    KSceneObject* m_pLandMine;
    BOOL operator()(KHero* pHero);
};
