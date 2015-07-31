#include "stdafx.h"
#include "KLinePath.h"
#include "KSO3World.h"
#include "KMath.h"

KLinePath::KLinePath(void)
{
    m_nStartFrame = INT_MAX;
    m_nTotalFrame = INT_MAX;
    m_nVelocityX = 0;
    m_nVelocityY = 0;
    m_nVelocityZ = 0;
}


KLinePath::~KLinePath(void)
{
}


BOOL KLinePath::SetLine(const KPOSITION& cStart, const KPOSITION& cEnd, int nVelocity)
{
    BOOL bResult = false;

    double  dLineDistance   = 0;

    KGLOG_PROCESS_ERROR(nVelocity > 0);

    m_cStart = cStart;
    m_cEnd = cEnd;

    dLineDistance = cStart.GetDistanceTo(m_cEnd);
    m_nTotalFrame = int(dLineDistance / nVelocity);
    if (m_nTotalFrame == 0 && dLineDistance != 0)
        m_nTotalFrame = 1;

    if (m_nTotalFrame > 0)
    {
        m_nVelocityX = int((m_cEnd.nX - m_cStart.nX) / m_nTotalFrame);
        m_nVelocityY = int((m_cEnd.nY - m_cStart.nY) / m_nTotalFrame);
        m_nVelocityZ = int((m_cEnd.nZ - m_cStart.nZ) / m_nTotalFrame);
    }
    
    bResult = true;
Exit0:
    return bResult;
}

KPOSITION KLinePath::GetStartPos() const
{
    return m_cStart;
}

KPOSITION KLinePath::GetFinnalPos() const
{
    return m_cEnd;
}

int KLinePath::GetStartFrame() const
{
    return m_nStartFrame;
}

int KLinePath::GetTotalFrame() const
{
    return m_nTotalFrame;
}

void KLinePath::Start(int nStartFrame)
{
    m_nStartFrame = nStartFrame;
}

BOOL KLinePath::IsFinished(int nFrame) const
{
    int nTotalFrame = GetTotalFrame();
    return nFrame - m_nStartFrame >= nTotalFrame;
}

IPath* KLinePath::GetClone() const
{
    KLinePath* pResult = KMEMORY_NEW(KLinePath);
    KGLOG_PROCESS_ERROR(pResult);

    pResult->m_cStart = m_cStart;
    pResult->m_cEnd = m_cEnd;

    pResult->m_nStartFrame = m_nStartFrame;
    pResult->m_nTotalFrame = m_nTotalFrame;
    pResult->m_nVelocityX = m_nVelocityX;
    pResult->m_nVelocityY = m_nVelocityY;
    pResult->m_nVelocityZ = m_nVelocityZ;

Exit0:
    return pResult;
}

void KLinePath::ReleaseClone()
{
    KMemory::Delete(this);
}

void KLinePath::GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    KGLOG_PROCESS_ERROR(nFrame >= m_nStartFrame);
    GetVelocityByDeltaFrame(nFrame - m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
Exit0:
    return;
}

void KLinePath::GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    KGLOG_PROCESS_ERROR(nDeltaFrame >= 0);
    KGLOG_PROCESS_ERROR(nDeltaFrame <= m_nTotalFrame);

    nVelocityX = 0;
    nVelocityY = 0;
    nVelocityZ = 0;

    KG_PROCESS_SUCCESS(m_nTotalFrame == 0);

    nVelocityX = m_nVelocityX;
    nVelocityY = m_nVelocityY;
    nVelocityZ = m_nVelocityZ;

Exit1:
Exit0:
    return;
}

void KLinePath::GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    GetVelocity(m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
}

KPOSITION KLinePath::GetPosAtFrame(int nFrame) const
{
    KPOSITION Pos;
    if (m_nTotalFrame > 0)
    {
        int nDeltaFrame = nFrame - m_nStartFrame;
        MakeInRange<int>(nDeltaFrame, 0, m_nTotalFrame);
        Pos = m_cStart.GetPosByPercent(m_cEnd, (double)nDeltaFrame / m_nTotalFrame);
    }
    return Pos;
}
