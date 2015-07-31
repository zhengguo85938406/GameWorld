#include "stdafx.h"
#include "KHoist.h"
#include "KAllObjTemplate.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KScene.h"

KHoist::KHoist()
{
    m_nStartFrame = 0;
    m_nDelaySeconds = 0;
    m_nAddHeightVelocity = 0;
    m_nAddSeconds = 0;
    m_nAddHeightPerFrame = 0;
    m_nLeftFrame = 0;
    m_nIdleFrame = 1;
}

KHoist::~KHoist()
{
}

BOOL KHoist::Init()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

	m_fXYReboundCoeX = 1.0f;
    m_fXYReboundCoeY = 1.0f;
    m_fYZReboundCoeX = 1.0f;
    m_fYZReboundCoeY = 1.0f;

    bRetCode = KMovableObstacle::Init();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KHoist::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    assert(pTemplate);
    m_nDelaySeconds = pTemplate->m_nCustomParam[0];
    m_nAddHeightVelocity = pTemplate->m_nCustomParam[1];
    m_nAddSeconds = pTemplate->m_nCustomParam[2];

    m_nStartFrame = 0;
    m_nLeftFrame = m_nAddSeconds * GAME_FPS;
}

int KHoist::GetCustomData(int nIndex)
{
    if (nIndex == 0)
        return m_nAddHeightPerFrame;
    return 0;
}

void KHoist::Activate(int nCurLoop)
{
    KMovableObstacle::Activate(nCurLoop);

    if (m_nStartFrame == 0)
        m_nStartFrame = nCurLoop + m_nDelaySeconds * GAME_FPS;

    ChangeHeight(nCurLoop);

    return;
}

void KHoist::ChangeHeight(int nCurLoop)
{
    KG_PROCESS_ERROR(nCurLoop >= m_nStartFrame);

    if (m_nIdleFrame > 0)
    {
        --m_nIdleFrame;

        if (m_nIdleFrame > 0)
            goto Exit0;

        m_nAddHeightPerFrame = m_nAddHeightVelocity / GAME_FPS;
        g_PlayerServer.DoSyncSceneObject(this);
    }

    //if (nCurLoop == m_nStartFrame)
    //{
    //    m_nAddHeightPerFrame = m_nAddHeightVelocity / GAME_FPS;
    //    g_PlayerServer.DoSyncSceneObject(this);
    //    m_nStartFrame = -1;
    //}

    if (m_nLeftFrame > 0)
    {
        BOOL bOstacleChange = (m_nHeight * (m_nHeight + m_nAddHeightPerFrame) == 0);

        m_nHeight += m_nAddHeightPerFrame;
        --m_nLeftFrame;

        if (bOstacleChange)
            m_pScene->ApplyObstacle(this, m_nHeight != 0);
    }

    if (m_nLeftFrame == 0)
    {
        m_nLeftFrame = m_nAddSeconds * GAME_FPS;
        if (m_nAddHeightPerFrame > 0)
        {       
            m_nAddHeightPerFrame *= -1;
        }
        else
        {
            m_nIdleFrame = GAME_FPS * 15;
            m_nAddHeightPerFrame = 0;
        }

        g_PlayerServer.DoSyncSceneObject(this);
    }  

Exit0:
    return;
}
