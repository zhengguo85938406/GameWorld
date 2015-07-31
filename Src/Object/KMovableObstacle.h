// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KMovableObstacle.h 
//  Creator 	: Xiayong  
//  Date		: 08/08/2012
//  Comment	: 
// ***************************************************************
#pragma once

#include "KMovableObject.h"

class KMovableObstacle : public KMovableObject
{
public:
    KMovableObstacle();
    virtual ~KMovableObstacle();

    void GetMyAreaCells(const KPOSITION& pos, std::vector<uint64_t>& vecCells);

    BOOL GetObstacleFlag(){return m_bObstacle;};
    void SetObstacleFlag(BOOL bObstacleFlag);

    virtual void Activate(int nCurLoop); 

protected:
    virtual void OnPosChanged(const KPOSITION& cOld, const KPOSITION& cNew);
    virtual void OnDeleteFlagSetted();

private:
    BOOL m_bObstacle;
};
