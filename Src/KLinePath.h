// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KLinePath.h
//  Creator 	: tongxuehu  
//  Date		: 01/21/2013
//  Comment		: 
// ***************************************************************

#pragma once

#include "IPath.h"
#include "KFix2PParabola.h"

class KLinePath : public IPath
{
public:
    KLinePath();
    virtual ~KLinePath(void);
    
    virtual KPOSITION GetStartPos() const;
    virtual KPOSITION GetFinnalPos() const;

    virtual void GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ);
    virtual void GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ);

    virtual KPOSITION GetPosAtFrame(int nFrame) const;

    virtual void Start(int nStartFrame);

    virtual BOOL IsFinished(int nFrame) const;

    virtual void ReleaseClone();

    virtual IPath* GetClone() const;

    virtual void GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ);
    virtual int GetStartFrame() const;
    virtual int GetTotalFrame() const;

    BOOL SetLine(const KPOSITION& cStart, const KPOSITION& cEnd, int nVelocity);

private:
    KPOSITION       m_cStart;
    KPOSITION       m_cEnd;

    int             m_nVelocityX;
    int             m_nVelocityY;
    int             m_nVelocityZ;
    int             m_nStartFrame;
    int             m_nTotalFrame;
};
