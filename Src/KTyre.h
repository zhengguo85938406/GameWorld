// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KTyre.h
//  Creator 	: hanruofei 
//  Date		: 2012-2-13
//  Comment	: 
// ***************************************************************
#pragma once
#include "KDoodad.h"

enum KTYPE_ROLLING_STEP
{
    trsNone,
    trsThrowing,
    trsRolling,
};

class KTyre :  public KDoodad
{
public:
    KTyre(void);
    virtual ~KTyre(void);

    void Activate(int nCurLoop);
    KSceneObjectType GetType() const {return sotTyre;};

    void OnCollideHero(KHero* pHero);

private:
    void BeginRolling(int nCurLoop);
    void FinishRolling();
    void ProcessRolling(int nCurLoop);

    virtual void OnThrowOut(KHero* pHero);

    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    virtual void OnTurnedToIdle();

    virtual void ProcessXYRebound();
    virtual void ProcessXZRebound();
    virtual void ProcessYZRebound();

    int m_nStartRollingFrame;
    int m_nTotalRollingFrame;

    KTYPE_ROLLING_STEP m_eStep;
};

