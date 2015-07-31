#include "stdafx.h"
#include "KFix2PParabola.h"
#include "KSO3World.h"
#include "KMath.h"

KFix2PParabola::KFix2PParabola()
{
    m_nGravity = 0;
}

KFix2PParabola::KFix2PParabola(int nGravity) : m_nGravity(nGravity)
{
}

KFix2PParabola::KFix2PParabola(const KPOSITION& cStart, const KPOSITION& cEnd, int nHeight, int nGravity)
{
    m_nGravity = nGravity;
    SetCtrlParams(cStart, cEnd, nHeight);
}

KFix2PParabola::~KFix2PParabola(void)
{
}

KPOSITION KFix2PParabola::GetStartPos() const
{
    return m_cStart;
}

KPOSITION KFix2PParabola::GetFinnalPos() const
{
    return m_cEnd;
}


void KFix2PParabola::GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    KGLOG_PROCESS_ERROR(nFrame >= m_nStartFrame);
    GetVelocityByDeltaFrame(nFrame - m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
Exit0:
    return;
}

void KFix2PParabola::GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    KGLOG_PROCESS_ERROR(nDeltaFrame >= 0);
    KGLOG_PROCESS_ERROR(nDeltaFrame <= m_nTotalFrame);

    if(m_nTotalFrame == 0)
    {
        nVelocityX = 0;
        nVelocityY = 0;
        nVelocityZ = 0;
    }
    else
    {
        nVelocityX = (m_cEnd.nX - m_cStart.nX) / m_nTotalFrame;
        nVelocityY = (m_cEnd.nY - m_cStart.nY) / m_nTotalFrame;
        nVelocityZ = (int)(m_dInitVZ - m_nGravity * nDeltaFrame);
    } 
Exit0:
    return;
}

KPOSITION KFix2PParabola::GetPosAtFrame(int nFrame) const
{
    KPOSITION cResult;
    int nDeltaFrame = nFrame - m_nStartFrame;

    assert(nDeltaFrame >= 0);

    nDeltaFrame = nDeltaFrame >= 0 ? nDeltaFrame : 0;

    if (nDeltaFrame >= m_nTotalFrame)
    {
        cResult = m_cEnd;
    }
    else
    {
        cResult.nX = m_cStart.nX + (m_cEnd.nX - m_cStart.nX) * nDeltaFrame / m_nTotalFrame;
        cResult.nY = m_cStart.nY + (m_cEnd.nY - m_cStart.nY) * nDeltaFrame / m_nTotalFrame;
        cResult.nZ = int(m_cStart.nZ + m_dInitVZ * nDeltaFrame - 0.5 * m_nGravity * nDeltaFrame * nDeltaFrame);
    }

    return cResult;
}

void KFix2PParabola::Start(int nStartFrame)
{
    m_nStartFrame = nStartFrame;
}

BOOL KFix2PParabola::IsFinished(int nFrame) const
{
    return (nFrame - m_nStartFrame) >= m_nTotalFrame;
}

void KFix2PParabola::ReleaseClone()
{
    KMemory::Delete(this);
}

IPath* KFix2PParabola::GetClone() const
{
    KFix2PParabola* pResult = KMEMORY_NEW(KFix2PParabola);
    if (pResult)
    {
        CloneTo(*pResult);
    }
    return pResult;
}

void KFix2PParabola::CloneTo(KFix2PParabola& Parabola) const
{
    Parabola.m_cStart = m_cStart;
    Parabola.m_cEnd = m_cEnd;
    Parabola.m_nTotalFrame = m_nTotalFrame;
    Parabola.m_nStartFrame = m_nStartFrame;
    Parabola.m_dInitVZ = m_dInitVZ;
    Parabola.m_nGravity = m_nGravity;
}

void KFix2PParabola::SetCtrlParams(const KPOSITION& cStart, const KPOSITION& cEnd, int nHeight)
{
    m_cStart = cStart;
    m_cEnd = cEnd;

    int nStart = cStart.nZ;
    int nEnd = cEnd.nZ;

    assert(nHeight > nStart && nHeight > nEnd);

    double dUpTime = 0;
    double dDownTime = 0;

    dUpTime = sqrt((double)(nHeight - nStart) * 2 / m_nGravity);
    dDownTime = sqrt((double)(nHeight - nEnd) * 2 / m_nGravity);

    double dTotalTime = dUpTime + dDownTime;

    m_dInitVZ = m_nGravity * dTotalTime * 0.5 - (nStart - nEnd) / dTotalTime;
    m_nTotalFrame = int(dTotalTime);

    //gravity * _totalFrame * 0.5 - (_startPos.z - _finalPos.z) / totalFrameF;

    LogInfo(
        "Totalframe: %d, startpos:(%d,%d,%d), endpos:(%d,%d,%d), init Vz:%f, totaltimef:%f", 
        m_nTotalFrame, m_cStart.nX, m_cStart.nY, m_cStart.nZ, m_cEnd.nX, m_cEnd.nY, m_cEnd.nZ, m_dInitVZ, dTotalTime
    );
}

int KFix2PParabola::GetStartFrame() const
{
    return m_nStartFrame;
}

int KFix2PParabola::GetTotalFrame() const
{
    return m_nTotalFrame;
}

void KFix2PParabola::GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    GetVelocity(m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
}


