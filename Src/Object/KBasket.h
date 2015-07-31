#pragma once
#include "KDoodad.h"

class KBall;
class KBasketSocket;
class KHero;
class KBasket : public KDoodad
{
public:
    KBasket(void);
    virtual ~KBasket(void);

    virtual KSceneObjectType GetType() const;

    void CheckBasketHP(KHero* pHero);
    void CostHP(DWORD dwHeroID, KBall* pBall);

    virtual int GetCustomData(int nIndex);

    BOOL    IsInSocket() const;

    virtual BOOL CanBeStandOnBy(KMovableObject* pObj) const;

    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);

public:
    int            m_nBasketHP;                     //篮筐当前HP
    int            m_nDropBasketVelocityX;          //掉落篮筐水平偏移速度
    int            m_nDropBasketVelocityZ;          //掉落篮筐垂直偏移速度
    KBasketSocket* m_pPluginSocket;
    BOOL           m_bNotCostHP;

private:

    virtual void ApplySpecialInitInfo(const KSceneObjInitInfo& cTemplate);
    virtual void OnTrackMoveFinished();
    virtual void OnTrackMoveInterrupted();
    void    OnShootSuccess();

private:
    int            m_nBasketMaxHP;                  //篮筐最大HP
};

