// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KHoist.h 
//  Creator 	: Xiayong  
//  Date		: 08/10/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KMovableObstacle.h"

class KHoist : public KMovableObstacle
{
public:
    KHoist();
    virtual ~KHoist();

    virtual BOOL Init();
    virtual KSceneObjectType GetType() const {return sotHoist;};
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    virtual int GetCustomData(int nIndex);
    virtual void Activate(int nCurLoop); 
    void ChangeHeight(int nCurLoop);
    
private:
    int m_nStartFrame;
    int m_nDelaySeconds;        // 比赛开始一段时间后开始执行,单位：s
    int m_nAddHeightVelocity;   // 每s增加的高度
    int m_nAddSeconds;          // 持续时间：单位s  

    int m_nIdleFrame;

    int m_nAddHeightPerFrame;
    int m_nLeftFrame;
};
