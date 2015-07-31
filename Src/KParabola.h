#pragma once
#include "IPath.h"
class KParabola : public IPath
{
public:
    KParabola();
    KParabola(int nGravity);
    KParabola(const KPOSITION& cStart, int nVelocityX, int nVelocityY, int nVelocityZ, int nGravity);
    virtual ~KParabola(void);

    virtual void GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ);
    virtual void GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ);

    virtual KPOSITION GetPosAtFrame(int nFrame) const;

    virtual BOOL IsFinished(int nFrame) const;

    virtual void ReleaseClone();

    virtual IPath* GetClone() const;

    virtual void Start(int nStartFrame);

    virtual KPOSITION GetStartPos() const;
    virtual KPOSITION GetFinnalPos() const;

    virtual void GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ);
    virtual int GetStartFrame() const;
    virtual int GetTotalFrame() const;

    void SetCtrlParams(const KPOSITION& cStart, int nVelocityX, int nVelocityY, int nVelocityZ);

private:
    KPOSITION   m_cStart;
    int         m_nVelocityX;
    int         m_nVelocityY;
    int         m_nVelocityZ;

    int         m_nStartFrame;
    int         m_nGravity;

};

