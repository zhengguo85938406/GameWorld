// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBarrier.h 
//  Creator 	: Xiayong  
//  Date		: 11/06/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KMovableObstacle.h"

class KBarrier : public KMovableObstacle
{
public:
    KBarrier();
    virtual ~KBarrier();

    virtual BOOL Init();
    virtual KSceneObjectType GetType() const {return sotBarrier;};
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    virtual int GetCustomData(int nIndex);

    void BeAttacked();

private:
    int     m_nLife;
	int		m_nRepresentID;
};
