#include "stdafx.h"
#include "KParabola.h"
#include "KSO3World.h"

KParabola::KParabola()
{
    m_nVelocityX = 0;
    m_nVelocityY = 0;
    m_nVelocityZ = 0;
    m_nStartFrame = INT_MAX;
    m_nGravity = 0;
}

KParabola::KParabola(int nGravity)
{
    m_nVelocityX = 0;
    m_nVelocityY = 0;
    m_nVelocityZ = 0;
    m_nStartFrame = INT_MAX;
    m_nGravity = nGravity;
}

KParabola::KParabola(const KPOSITION& cStart, int nVelocityX, int nVelocityY, int nVelocityZ, int nGravity)
{
    m_cStart = cStart;
    m_nVelocityX = nVelocityX;
    m_nVelocityY = nVelocityY;
    m_nVelocityZ = nVelocityZ;
    m_nStartFrame = INT_MAX;
    m_nGravity = nGravity;

}

KParabola::~KParabola(void)
{
}

void KParabola::GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    KGLOG_PROCESS_ERROR(nFrame >= m_nStartFrame);
    GetVelocityByDeltaFrame(nFrame - m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
Exit0:
    return;
}

void KParabola::GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    KGLOG_PROCESS_ERROR(nDeltaFrame >= 0);

    nDeltaFrame = nDeltaFrame >= 0 ? nDeltaFrame : 0;

    nVelocityX = m_nVelocityX;
    nVelocityY = m_nVelocityY;
    nVelocityZ = m_nVelocityZ - nDeltaFrame * m_nGravity;

Exit0:
    return;
}

KPOSITION KParabola::GetPosAtFrame(int nFrame) const
{
    KPOSITION cResult;
    int nDeltaFrame = nFrame - m_nStartFrame;

    assert(nDeltaFrame >= 0);
    nDeltaFrame = nDeltaFrame >= 0 ? nDeltaFrame : 0;

    cResult.nX = m_cStart.nX + m_nVelocityX * nDeltaFrame;
    cResult.nY = m_cStart.nY + m_nVelocityY * nDeltaFrame;
    cResult.nZ = int(m_cStart.nZ + m_nVelocityZ * nDeltaFrame - 0.5 * m_nGravity * nDeltaFrame * nDeltaFrame);
    
    return cResult;
}

BOOL KParabola::IsFinished(int nFrame) const
{
    return false;
}

void KParabola::ReleaseClone()
{
    KMemory::Delete(this);
}

IPath* KParabola::GetClone() const
{
    KParabola* pResult = KMEMORY_NEW(KParabola);

    pResult->m_cStart = m_cStart;
    
    pResult->m_nVelocityX   = m_nVelocityX;
    pResult->m_nVelocityY   = m_nVelocityY;
    pResult->m_nVelocityZ   = m_nVelocityZ;
    pResult->m_nStartFrame  = m_nStartFrame;
    pResult->m_nGravity     = m_nGravity;

    return pResult;
}

void KParabola::Start(int nStartFrame)
{
    m_nStartFrame = nStartFrame;
}

KPOSITION KParabola::GetStartPos() const
{
    return m_cStart;
}

KPOSITION KParabola::GetFinnalPos() const
{
    return KPOSITION(0,0,0);
}

int KParabola::GetStartFrame() const
{
    return m_nStartFrame;
}

int KParabola::GetTotalFrame() const
{
    return 0;
}

void KParabola::SetCtrlParams(const KPOSITION& cStart, int nVelocityX, int nVelocityY, int nVelocityZ)
{
    m_cStart = cStart;
    m_nVelocityX = nVelocityX;
    m_nVelocityY = nVelocityY;
    m_nVelocityZ = nVelocityZ;
}

void KParabola::GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    GetVelocity(m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
}

