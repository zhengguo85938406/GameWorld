// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: IPath.h
//  Creator 	: Hanruofei  
//  Date		: 12/12/2011
//  Comment		: 
// ***************************************************************
#pragma once

struct KPOSITION;

class IPath
{
public:
    virtual IPath* GetClone() const = 0;
    virtual void ReleaseClone() = 0;

    virtual BOOL IsFinished(int nFrame) const = 0;
    virtual KPOSITION GetPosAtFrame(int nFrame) const = 0;

    virtual void GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ) = 0;
    virtual void GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ) = 0;

    virtual void Start(int nStartFrame) = 0;

    virtual KPOSITION GetStartPos() const = 0;
    virtual KPOSITION GetFinnalPos() const = 0;

    virtual int GetStartFrame() const = 0;
    virtual int GetTotalFrame() const = 0;

    virtual void GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ) = 0;
};

