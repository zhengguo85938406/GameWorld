// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KFix2PParabola.h
//  Creator 	: Hanruofei  
//  Date		: 12/13/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "IPath.h"
#include "GlobalStruct.h"

class KFix2PParabola :  public IPath
{
public:
    KFix2PParabola();
    KFix2PParabola(int nGravity);
    KFix2PParabola(const KPOSITION& cStart, const KPOSITION& cEnd, int nHeight, int nGravity);

    void SetCtrlParams(const KPOSITION& cStart, const KPOSITION& cEnd, int nHeight);

    virtual ~KFix2PParabola(void);

    virtual KPOSITION GetStartPos() const;
    virtual KPOSITION GetFinnalPos() const;

    virtual void GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ);
    virtual void GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ);

    virtual KPOSITION GetPosAtFrame(int nFrame) const;

    virtual void Start(int nStartFrame);

    virtual BOOL IsFinished(int nFrame) const;

    virtual void ReleaseClone();

    virtual IPath* GetClone() const;

    virtual void GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ) ;

    virtual int GetStartFrame() const;
    virtual int GetTotalFrame() const;

    void CloneTo(KFix2PParabola& Parabola) const;

private:
    KPOSITION   m_cStart;
    KPOSITION   m_cEnd;
    int         m_nTotalFrame;
    int         m_nStartFrame;
    double      m_dInitVZ;
    int         m_nGravity;
};

