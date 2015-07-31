#include "stdafx.h"
#include "KComplexPath.h"
#include "KSO3World.h"
#include "KLinePath.h"
#include "KFix2PParabola.h"
#include "KSceneGlobal.h"
#include "KHero.h"
#include "KScene.h"

KComplexPath::KComplexPath(void)
{
    m_nStartFrame = 0;
    m_nTotalFrame = 0;
}


KComplexPath::~KComplexPath(void)
{
    KCOMPLEX_SUB_PATHS::iterator itPaths;
    for (itPaths = m_cSubPaths.begin(); itPaths != m_cSubPaths.end(); ++itPaths)
    {
        (*itPaths)->ReleaseClone();
        (*itPaths) = NULL;
    }
    m_cSubPaths.clear();
}


KPOSITION KComplexPath::GetStartPos() const
{
    return m_cStart;
}

KPOSITION KComplexPath::GetFinnalPos() const
{
    return m_cEnd;
}

int KComplexPath::GetStartFrame() const
{
    return m_nStartFrame;
}

int KComplexPath::GetTotalFrame() const
{
    return m_nTotalFrame;
}

void KComplexPath::GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    GetVelocity(m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
}

void KComplexPath::Start(int nStartFrame)
{
    KCOMPLEX_SUB_PATHS::iterator itPaths;

    m_nStartFrame = nStartFrame;

    for (itPaths = m_cSubPaths.begin(); itPaths != m_cSubPaths.end(); ++itPaths)
    {
        IPath* pSubPath = (*itPaths);
        pSubPath->Start(nStartFrame);
        nStartFrame += pSubPath->GetTotalFrame();
    }
}

BOOL KComplexPath::IsFinished(int nFrame) const
{
    int nTotalFrame = GetTotalFrame();
    return nFrame - m_nStartFrame >= nTotalFrame;
}

IPath* KComplexPath::GetClone() const
{
    KCOMPLEX_SUB_PATHS::const_iterator itPaths;
    KComplexPath* pResult = NULL;
    KComplexPath* pClone = NULL;

    pClone = KMEMORY_NEW(KComplexPath);
    KGLOG_PROCESS_ERROR(pClone);

    pClone->m_cStart = m_cStart;
    pClone->m_cEnd = m_cEnd;
    pClone->m_nStartFrame = m_nStartFrame;
    pClone->m_nTotalFrame = m_nTotalFrame;

    for (itPaths = m_cSubPaths.begin(); itPaths != m_cSubPaths.end(); ++itPaths)
    {
        IPath* pSubPath = (*itPaths)->GetClone();
        KGLOG_PROCESS_ERROR(pSubPath);
        pClone->m_cSubPaths.push_back(pSubPath);
    }

    pResult = pClone;
Exit0:
    return pResult;
}

void KComplexPath::ReleaseClone()
{
    KMemory::Delete(this);
}

IPath* KComplexPath::GetPathByDeltaFrame(int nDeltaFrame, int* pRetDeltaFrameInPath) const
{
    IPath* pResultPath = NULL;
    KCOMPLEX_SUB_PATHS::const_iterator itPaths;
    int nCurrentFrame = 0;
	
    KGLOG_PROCESS_ERROR(nDeltaFrame >= 0);
    KGLOG_PROCESS_ERROR(nDeltaFrame <= m_nTotalFrame);
	
    for (itPaths = m_cSubPaths.begin(); itPaths != m_cSubPaths.end(); ++itPaths)
    {
        IPath* pPath = *itPaths;
        int nTotalFrame = pPath->GetTotalFrame();
        if (nDeltaFrame <= nCurrentFrame + nTotalFrame)
        {
            pResultPath = pPath;
            if (pRetDeltaFrameInPath)
                *pRetDeltaFrameInPath = nDeltaFrame - nCurrentFrame;
            break;
        }
        nCurrentFrame += nTotalFrame;
    }
Exit0:
    return pResultPath;
}

void KComplexPath::GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    KGLOG_PROCESS_ERROR(nFrame >= m_nStartFrame);
    GetVelocityByDeltaFrame(nFrame - m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
Exit0:
    return;
}

void KComplexPath::GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    IPath* pPath = NULL;
    int nDeltaFrameInPath = 0;
	
    nVelocityX = 0;
    nVelocityY = 0;
    nVelocityZ = 0;

    KGLOG_PROCESS_ERROR(nDeltaFrame >= 0);
    KGLOG_PROCESS_ERROR(nDeltaFrame <= m_nTotalFrame);
    KG_PROCESS_SUCCESS(m_nTotalFrame == 0);

    pPath = GetPathByDeltaFrame(nDeltaFrame, &nDeltaFrameInPath);
    KGLOG_PROCESS_ERROR(pPath);

    pPath->GetVelocityByDeltaFrame(nDeltaFrameInPath, nVelocityX, nVelocityY, nVelocityZ);

Exit1:
Exit0:
    return;
}


KPOSITION KComplexPath::GetPosAtFrame(int nFrame) const
{
    KPOSITION Pos;
    IPath* pPath = NULL;

	KGLOG_PROCESS_ERROR(nFrame >= m_nStartFrame);
    KGLOG_PROCESS_ERROR(nFrame <= m_nStartFrame + m_nTotalFrame);
    
    pPath = GetPathByDeltaFrame(nFrame - m_nStartFrame, NULL);
    KGLOG_PROCESS_ERROR(pPath);

    Pos = pPath->GetPosAtFrame(nFrame);

 Exit0:
    return Pos;
}

BOOL SetPointByPointInfo(KPOSITION& cPoint, KPathPointInfo* pInfo, KHero* pHero)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KTWO_DIRECTION eDir = csdInvalid;
    int nSceneLength = 0;
    int nSceneWidth = 0;

    KGLOG_PROCESS_ERROR(pInfo);
    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    switch (pInfo->ePointType)
    {
    case KPATH_POINT_TYPE_ABS:
        cPoint.nX = pInfo->nX;
        cPoint.nY = pInfo->nY;
        cPoint.nZ = pInfo->nZ;
        break;
    case KPATH_POINT_TYPE_PERCENT:
        eDir = pHero->GetFaceDir();
        nSceneLength = pHero->m_pScene->GetLength();
        nSceneWidth = pHero->m_pScene->GetWidth();
        if (eDir == csdRight)
            cPoint.nX = nSceneLength * pInfo->nX / HUNDRED_NUM;
        else
            cPoint.nX = nSceneLength * (HUNDRED_NUM - pInfo->nX) / HUNDRED_NUM;
        cPoint.nY = nSceneWidth * pInfo->nY / HUNDRED_NUM;
        cPoint.nZ = pInfo->nZ;
        break;
    case KPATH_POINT_TYPE_PLAYER:
        cPoint.nX = pHero->m_nX + pInfo->nX;
        cPoint.nY = pHero->m_nY + pInfo->nY;
        cPoint.nZ = pHero->m_nZ + pInfo->nZ;
        break;
    default:
        KGLOG_PROCESS_ERROR(false && "Error fly path point type!");
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KComplexPath::LoadComplexPath(
    const KPOSITION* pStart,  
    const KPOSITION* pEnd, 
    const KFLY_PATH_POINTS& cPoints, 
    KHero* pHero,
    KFLY_PATH_TYPE eEndPathType
)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KPOSITION       cSubStart;
    KPOSITION       cSubEnd;
    int             nEndVelocity = 0;
    KFLY_PATH_POINTS::const_iterator itPoints;

    KGLOG_PROCESS_ERROR(pStart);
    // KGLOG_PROCESS_ERROR(pEnd);
    KGLOG_PROCESS_ERROR(pHero);
    
    bRetCode = m_cSubPaths.empty();
    KGLOG_PROCESS_ERROR(bRetCode);

    m_cStart = *pStart;
    cSubStart = *pStart;
    cSubEnd = cSubStart;

    for (itPoints = cPoints.begin(); itPoints != cPoints.end(); ++itPoints)
    {
        KPathPointInfo* pPointInfo = g_pSO3World->m_Settings.m_PathPointData.GetPointInfo(*itPoints);
        KGLOG_PROCESS_ERROR(pPointInfo);

        bRetCode = SetPointByPointInfo(cSubEnd, pPointInfo, pHero);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = AddSubPathByType(
            pPointInfo->ePathType, 
            cSubStart, 
            cSubEnd, 
            pPointInfo->nGravity, 
            pPointInfo->nHeightOffset, 
            pPointInfo->nVelocity
        );
        KGLOG_PROCESS_ERROR(bRetCode);

        cSubStart = cSubEnd;
    }
    
    if (pEnd)
    {
        KBall* pBall = NULL;
        int nGravity = 0;

        KGLOG_PROCESS_ERROR(pHero->m_pScene);
        
        pBall = pHero->m_pScene->GetBall();
        KGLOG_PROCESS_ERROR(pBall);
        
        nGravity = pBall->GetCurrentGravity();
        KGLOG_PROCESS_ERROR(nGravity > 0);

        nEndVelocity = GetEndVelocity();
        KGLOG_PROCESS_ERROR(nEndVelocity > 0);

        bRetCode = AddSubPathByType(
            eEndPathType, 
            cSubEnd, 
            *pEnd, 
            nGravity, 
            CELL_LENGTH * 3, 
            nEndVelocity
        );
        KGLOG_PROCESS_ERROR(bRetCode);
        m_cEnd = *pEnd;
    }
    else
    {
        m_cEnd = cSubEnd;
    }
    
    bResult = true;
Exit0:
    return bResult;
}

int KComplexPath::GetEndVelocity()
{
    BOOL bRetCode = false;

    int nVelocityX   = 0;
    int nVelocityY   = 0;
    int nVelocityZ   = 0;
    int nTotalFrame  = 0;
    int nEndVelocity = 1000; // default if no pre path

    IPath* pPath = NULL;

    bRetCode = m_cSubPaths.empty();
    KG_PROCESS_SUCCESS(bRetCode);

    pPath = m_cSubPaths.back();
    KGLOG_PROCESS_ERROR(pPath);

    nTotalFrame = pPath->GetTotalFrame();
    pPath->GetVelocityByDeltaFrame(nTotalFrame, nVelocityX, nVelocityY, nVelocityZ);

    nEndVelocity = (int)sqrt((double)(nVelocityX * nVelocityX + nVelocityY * nVelocityY + nVelocityZ * nVelocityZ));

Exit1:
Exit0:
    return nEndVelocity;
}

BOOL KComplexPath::AddSubPathByType(
    KFLY_PATH_TYPE eType,
    const KPOSITION& cStart,  
    const KPOSITION& cEnd,
    int nGravity,
    int nHeightOffset,
    int nVelocity
 )
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    
    int nTotalFrame = 0;
    switch (eType)
    {
    case KFLY_PATH_TYPE_LINE:
        {
            KLinePath cLinePath;
            bRetCode = cLinePath.SetLine(cStart, cEnd, nVelocity);
            KGLOG_PROCESS_ERROR(bRetCode);
            nTotalFrame = cLinePath.GetTotalFrame();
            if (nTotalFrame > 0)
            {
                bRetCode = AddSubPath(&cLinePath);
                KGLOG_PROCESS_ERROR(bRetCode);
            }
            break;
        }
    case KFLY_PATH_TYPE_PARABOLA:
        {
            KFix2PParabola cParabola(nGravity);
            int nHeight = max(cStart.nZ, cEnd.nZ);
            nHeight += nHeightOffset;
            cParabola.SetCtrlParams(cStart, cEnd, nHeight);
            nTotalFrame = cParabola.GetTotalFrame();
            if (nTotalFrame > 0)
            {
                bRetCode = AddSubPath(&cParabola);
                KGLOG_PROCESS_ERROR(bRetCode);
            }
            break;
        }
    default:
        KGLOG_PROCESS_ERROR(false && "Error fly path type!");
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KComplexPath::AddSubPath(const IPath *pPath)
{
    BOOL    bResult     = false;

    IPath*  pSubPath    = NULL;
    int     nTotalFrame = 0;

    KGLOG_PROCESS_ERROR(pPath);
    
    nTotalFrame = pPath->GetTotalFrame();
    KGLOG_PROCESS_ERROR(nTotalFrame > 0);
    
    pSubPath = pPath->GetClone();
    KGLOG_PROCESS_ERROR(pSubPath);

    m_nTotalFrame += nTotalFrame;
    m_cSubPaths.push_back(pSubPath);

    bResult = true;
Exit0:
    return bResult;
}
