// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KComplexPath.h
//  Creator 	: tongxuehu  
//  Date		: 01/21/2013
//  Comment		: 
// ***************************************************************

#pragma once

#include "IPath.h"
#include "KComplexPathData.h"

typedef std::vector<IPath*> KCOMPLEX_SUB_PATHS;

class KHero;
class KComplexPath : public IPath
{
public:
    KComplexPath(void);
    virtual ~KComplexPath(void);
    
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

    BOOL LoadComplexPath(
        const KPOSITION* pStart,  
        const KPOSITION* pEnd, 
        const KFLY_PATH_POINTS& cPoints, 
        KHero* pHero,
        KFLY_PATH_TYPE eEndPathType
    );

protected:
    BOOL AddSubPathByType(
        KFLY_PATH_TYPE eType,
        const KPOSITION& cStart,  
        const KPOSITION& cEnd,
        int nGravity,
        int nHeightOffset,
        int nVelocity
    );
    int GetEndVelocity();
    BOOL AddSubPath(const IPath *pPath);
    IPath* GetPathByDeltaFrame(int nDeltaFrame, int* pRetDeltaFrameInPath) const;

private:
    KPOSITION   m_cStart;
    KPOSITION   m_cEnd;
    int         m_nStartFrame;
    int         m_nTotalFrame;

    KCOMPLEX_SUB_PATHS m_cSubPaths;
};

