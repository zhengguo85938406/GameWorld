// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBrick.h 
//  Creator 	: Xiayong  
//  Date		: 02/10/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KDoodad.h"
#include "game_define.h"

class KHero;
class KBrick : public KDoodad
{
public:
    KBrick();   
    ~KBrick();

    void CollideHero(KHero* pHero);
    virtual int GetCustomData(int nIndex);
    virtual KSceneObjectType GetType() const {return sotBrick;};
    virtual int GetCurrentGravity();

private:
    virtual void OnThrowOut(KHero* pHero);
    void ChangeToAttackState();
    void ChangeToNormalState();
    virtual void ProcessRebounds();

private:
    BOOL m_bLoseGravity;
};
