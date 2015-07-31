#include "stdafx.h"
#include "KSkillShootPath.h"
#include "KSO3World.h"
#include "KMath.h"

KSkillShootPath::KSkillShootPath()
{
    m_nStartFrame   = INT_MAX;
    m_nTotalFrame   = INT_MAX;
    m_nGravity      = 0;
}

KSkillShootPath::KSkillShootPath(int nGravity)
{
    m_nStartFrame   = INT_MAX;
    m_nTotalFrame   = INT_MAX;
    m_nGravity      = nGravity;
}

KSkillShootPath::KSkillShootPath(const KPOSITION& cSrc, const KPOSITION& cDst, const KPOSITION& cCtrl, int nVelocity, int nGravity)
{
    m_nStartFrame   = INT_MAX;
    m_nTotalFrame   = INT_MAX;
    m_nGravity      = nGravity;

    SetCtrlParams(cSrc, cDst, cCtrl, nVelocity);
}


KSkillShootPath::~KSkillShootPath(void)
{
}

KPOSITION KSkillShootPath::GetStartPos() const
{
    return m_cStart;
}

KPOSITION KSkillShootPath::GetFinnalPos() const
{
    return m_cFinal;
}

void KSkillShootPath::GetVelocity(int nFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    KGLOG_PROCESS_ERROR(nFrame >= m_nStartFrame);
    GetVelocityByDeltaFrame(nFrame - m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
Exit0:
    return;
}

void KSkillShootPath::GetVelocityByDeltaFrame(int nDeltaFrame, int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    nVelocityX = 0;
    nVelocityY = 0;
    nVelocityZ = 0;

    KGLOG_PROCESS_ERROR(nDeltaFrame >= 0);
    KGLOG_PROCESS_ERROR(nDeltaFrame <= m_nTotalFrame);
    KG_PROCESS_SUCCESS(m_nTotalFrame == 0);
    
    if (1.0 * nDeltaFrame / m_nTotalFrame < m_dPercentA)
    {
        nVelocityX = int((m_cCtrl.nX - m_cStart.nX) / (m_dPercentA * m_nTotalFrame));
        nVelocityY = int((m_cCtrl.nY - m_cStart.nY) / (m_dPercentA * m_nTotalFrame));
        nVelocityZ = int((m_cCtrl.nZ - m_cStart.nZ) / (m_dPercentA * m_nTotalFrame));
    }
    else
    {
        nVelocityX = int((m_cFinal.nX - m_cCtrl.nX) / (m_dPercentB * m_nTotalFrame));
        nVelocityY = int((m_cFinal.nY - m_cCtrl.nY) / (m_dPercentB * m_nTotalFrame));
        nVelocityZ = int((m_cFinal.nZ - m_cCtrl.nZ) / (m_dPercentB * m_nTotalFrame));
    }

Exit1:
Exit0:
    return;
}

KPOSITION KSkillShootPath::GetPosAtFrame(int nFrame) const
{
    KPOSITION cResult;
    int nDeltaFrame = nFrame - m_nStartFrame;
    
    MakeInRange<int>(nDeltaFrame, 0, m_nTotalFrame);

    double dPercent = 1.0 * nDeltaFrame / m_nTotalFrame;

    if (dPercent < m_dPercentA)
    {
        double dPPercent = dPercent / m_dPercentA;
        cResult = m_cStart.GetPosByPercent(m_cCtrl, dPPercent);
    }
    else
    {
        dPercent -= m_dPercentA;
        double dPPercent = dPercent / m_dPercentB;
        cResult = m_cCtrl.GetPosByPercent(m_cFinal, dPPercent);
    }

    return cResult;
}

void KSkillShootPath::Start(int nStartFrame)
{
    m_nStartFrame = nStartFrame;
}

BOOL KSkillShootPath::IsFinished(int nFrame) const
{
    return nFrame - m_nStartFrame >= m_nTotalFrame;
}

void KSkillShootPath::ReleaseClone()
{
    this->~KSkillShootPath();
}

IPath* KSkillShootPath::GetClone() const
{
    KSkillShootPath* pResult = KMEMORY_NEW(KSkillShootPath);

    pResult->m_cStart = m_cStart;
    pResult->m_cFinal = m_cFinal;
    pResult->m_cCtrl = m_cCtrl;
    pResult->m_nTotalFrame = m_nTotalFrame;
    pResult->m_nStartFrame = m_nStartFrame;
    pResult->m_nGravity = m_nGravity;
    pResult->m_dPercentA = m_dPercentA;
    pResult->m_dPercentB = m_dPercentB;

    return pResult;
}

void KSkillShootPath::SetCtrlParams(const KPOSITION& cSrc, const KPOSITION& cDst, const KPOSITION& cCtrl, int nVelocity)
{
    assert(nVelocity > 0);
    m_cStart = cSrc;
    m_cFinal = cDst;
    m_cCtrl = cCtrl;

    double dDistanceA = cSrc.GetDistanceTo(cCtrl);
    double dDistanceB = cCtrl.GetDistanceTo(cDst);
    double dTotalDistance = dDistanceA + dDistanceB;

    assert(dTotalDistance > 0);

    m_dPercentA = dDistanceA / dTotalDistance;
    m_dPercentB = dDistanceB / dTotalDistance;
    m_nTotalFrame = int(dTotalDistance / nVelocity);
}

void KSkillShootPath::GetStartVelocity(int& nVelocityX, int& nVelocityY, int& nVelocityZ)
{
    GetVelocity(m_nStartFrame, nVelocityX, nVelocityY, nVelocityZ);
}

int KSkillShootPath::GetStartFrame() const
{
    return m_nStartFrame;
}

int KSkillShootPath::GetTotalFrame() const
{
    return m_nTotalFrame;
}