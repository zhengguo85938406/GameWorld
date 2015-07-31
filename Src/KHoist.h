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
    int m_nDelaySeconds;        // ������ʼһ��ʱ���ʼִ��,��λ��s
    int m_nAddHeightVelocity;   // ÿs���ӵĸ߶�
    int m_nAddSeconds;          // ����ʱ�䣺��λs  

    int m_nIdleFrame;

    int m_nAddHeightPerFrame;
    int m_nLeftFrame;
};
