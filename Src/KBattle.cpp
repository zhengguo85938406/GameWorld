#include "stdafx.h"
#include "KBattle.h"
#include "KPlayerServer.h"
#include "game_define.h"
#include "KHero.h"
#include "KScene.h"
#include "T3GameWorld/KSO3World.h"
#include <algorithm>


KBattle::KBattle(void)
{
    m_pScene = NULL;
    m_eMode  = ebtInvalid;
    m_nFramePerBattle = GAME_FPS * cdDefalutBattleTime;
    m_nCurFrame = INT_MAX;
    m_nStartFrame = INT_MAX;
    m_nTotalScore = 0;
    m_bUptoTotalScore = false;
}


KBattle::~KBattle(void)
{
}

void KBattle::Activate(int nCurLoop)
{
    m_nCurFrame = nCurLoop;
}


BOOL KBattle::Init(KScene* pScene)
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(pScene);

    m_pScene = pScene;

    bResult = true;
Exit0:
    return bResult;
}

void KBattle::UnInit()
{
    m_pScene        = NULL;
    m_nCurFrame     = INT_MAX;
    m_nStartFrame   = INT_MAX;
}

BOOL KBattle::IsFinished() const
{
    if (m_eMode == ebtTime)
    {
        if (m_nFramePerBattle == 0)
            return false;

        if (m_nCurFrame == INT_MAX || m_nStartFrame == INT_MAX)
            return false;

        return m_nCurFrame >= (m_nStartFrame + m_nFramePerBattle);
    }
    else if (m_eMode == ebtScore)
    {
        return m_bUptoTotalScore;
    }
    else
    {
        return false;
    }
}

void KBattle::Start(int nStartFrame)
{  
    m_nStartFrame = nStartFrame;
    m_nCurFrame = nStartFrame;
}

int KBattle::GetStartedFrame()
{
    if (m_eMode == ebtTime)
    {
        if (m_nFramePerBattle == 0)
            return 0;
    }
    
    if (m_nStartFrame == INT_MAX)
        return 0;

    if (m_nCurFrame == INT_MAX)
        return 0;
    
    return (m_nCurFrame - m_nStartFrame);
}

void KBattle::ApplyTemplate(DWORD dwTemplateID)
{
    KBattleTempalte* pTemplate = NULL;
    
    pTemplate = g_pSO3World->m_Settings.m_BattleSetting.GetBattleTemplate(dwTemplateID);
    KGLOG_PROCESS_ERROR(pTemplate);

    m_eMode           = (KBATTLE_MODE)pTemplate->nMode;
    m_nFramePerBattle = pTemplate->nTotleFrame;
    m_nTotalScore     = pTemplate->nWinScore;

Exit0:
    return;
}

BOOL KBattle::SetBattleTotalFrame(int nTotalFrame, BOOL bReTiming)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(m_eMode == ebtTime);
    KGLOG_PROCESS_ERROR(nTotalFrame > 0);
    m_nFramePerBattle = nTotalFrame;

    if (bReTiming)
    {
        m_nStartFrame = g_pSO3World->m_nGameLoop;
        m_nCurFrame = g_pSO3World->m_nGameLoop;
    }

    BroadcastTotalFrame();

    bResult = true;
Exit0:
    return bResult;
}

void KBattle::OnAddScore(int nNewScore)
{
    if (m_eMode == ebtScore)
    {
        if (nNewScore >= m_nTotalScore)
            m_bUptoTotalScore = true;
    }
    return;
}

void KBattle::BroadcastTotalFrame()
{
    int nStartedFrame = 0;
    int nLeftFrame = 0;

    KG_PROCESS_ERROR(m_pScene != NULL);
    KG_PROCESS_ERROR(m_nStartFrame != INT_MAX && m_nCurFrame != INT_MAX);

    nStartedFrame = GetStartedFrame();
    nLeftFrame = MAX(m_pScene->m_nShouldStartFrame - g_pSO3World->m_nGameLoop, 0);
    
    g_PlayerServer.DoSyncStartedFrame(nStartedFrame, m_nFramePerBattle, nLeftFrame, -1, m_pScene);

Exit0:
    return;
}
