// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KSkillShootPath.h
//  Creator 	: Hanruofei  
//  Date		: 12/15/2011
//  Comment		: 折线轨迹，用于技能投篮
// ***************************************************************
#pragma once
#include "IPath.h"
class KSkillShootPath :  public IPath
{
public:
    KSkillShootPath();
    KSkillShootPath(int nGravity);
    KSkillShootPath(const KPOSITION& cSrc, const KPOSITION& cDst, const KPOSITION& cCtrl, int nVelocity, int nGravity);

    void SetCtrlParams(const KPOSITION& cSrc, const KPOSITION& cDst, const KPOSITION& cCtrl, int nVelocity);

    virtual ~KSkillShootPath(void);

    virtual KPOSITION GetStartPos() const;
    virtual KPOSITION GetFinnalPos() const;

    virtual void GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ);
    virtual void GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ);

    virtual KPOSITION GetPosAtFrame(int nFrame) const;

    virtual void Start( int nStartFrame );

    virtual BOOL IsFinished( int nFrame ) const;

    virtual void ReleaseClone();

    virtual IPath* GetClone() const;

    virtual void GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ);
    virtual int GetStartFrame() const;
    virtual int GetTotalFrame() const;

private:
    int m_nGravity;
    int m_nStartFrame;
    int m_nTotalFrame;
    double m_dPercentA;
    double m_dPercentB;
    KPOSITION m_cStart;
    KPOSITION m_cFinal;
    KPOSITION m_cCtrl;
};

