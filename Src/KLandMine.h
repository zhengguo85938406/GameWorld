// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KLandMine.h 
//  Creator 	: Xiayong  
//  Date		: 02/09/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KDoodad.h"
#include "game_define.h"

struct KSceneObjectTemplate;

class KLandMine : public KDoodad
{
public:
    KLandMine();
    virtual ~KLandMine();
    virtual KSceneObjectType GetType() const;

    virtual void Activate(int nCurLoop);

    int     m_nActiveFrame;  // 地雷激活的时间
    int     m_nWaitActiveFrame; // 放置后多少帧激活
    bool    m_bDistinguishForce; // 是否区分敌我
    int     m_nBlowupVelocity;  // 爆炸速度
    int     m_nBlowupRange;     // 爆炸距离

    void    ActiveState();

private:
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    virtual void OnTurnedToIdle();
    virtual void OnThrowOut(KHero* pHero);
};
