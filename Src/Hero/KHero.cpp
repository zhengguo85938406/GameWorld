#include "stdafx.h"
#include "KHero.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KScene.h"
#include "KBasketSocket.h"
#include "KAttribute.h"
#include "KAttrModifier.h"
#include "KDoodad.h"
#include "KBall.h"
#include "KChest.h"
#include "KBasket.h"
#include "KBasketSocket.h"
#include "KPlayer.h"
#include "Engine/KGLog.h"
#include "KParabola.h"
#include "KSceneObject.h"
#include "KFix2PParabola.h"
#include "game_define.h"
#include "KBullet.h"
#include "KDummy.h"
#include "KBarrier.h"
#include "KGold.h"
#include "KBuffBox.h"

KHero::KHero()
{
    m_szName[0]     = '\0';		
    m_eGender       = gMale;
    m_nLevel        = 0;

    m_nCurrentEndurance     = INT_MAX / 2;
    m_nMaxEnduranceBase     = 0;
    m_nAddMaxEndurancePercent   = 0;
    m_nMaxEndurance         = 0;
    m_nEnduranceReplenish   = 0;
    m_nEnduranceReplenishPercent = 0;

    m_nCurrentStamina       = INT_MAX / 2;
    m_nMaxStaminaBase       = 0;
    m_nAddMaxStaminaPercent = 0;
    m_nMaxStamina           = 0;
    m_nStaminaReplenish     = 0;
    m_nStaminaReplenishPercent = 0;

    m_nCurrentAngry         = 0;
    m_nMaxAngry             = 0;
    m_nAdditionalAddAngryPercent   = 0;

    m_nWillPowerBase = 0;
    m_nAddWillPowerPercent = 0;
    m_nWillPower    = 0;

    m_nInterferenceBase     = 0;
    m_nAddInterferencePercent = 0;
    m_nInterference         = 0;
    m_nInterferenceRange    = 0;
    m_nInterferenceRangeBase = 0;
    m_nAddInterferenceRangePercent = 0;

    m_nAttackPoint  = 0;
    m_nAttackPointPercent   = 0;
    m_nAgility      = 0;
    m_nCritPoint    = 0;
    m_nCritRate     = 0;
    m_nDefense      = 0;
    m_nDefensePointPercent  = 0;
    m_nDefensePercent       = 0;

    m_nMoveSpeed            = 0;
    m_nMoveSpeedBase        = 0;
    m_nAddMoveSpeedBasePercent  = 0;

    m_nJumpSpeed    = 0;
    m_nJumpSpeedBase = 0;
    m_nAddJumpSpeedBasePercent  = 0;
    m_nJumpCount    = 0;

    m_nHitRate      = 0;
    m_nMaxHitOffset = CELL_LENGTH * 5 / 2;
    m_nHitRateAttenuation = CELL_LENGTH / 2;

    m_nNormalShootForce = 0;
    m_nSkillShootForce  = 0;
    m_nSlamDunkForce    = 0;

    m_nNextReplenishFrame = 0;
    m_dwTemplateID  = 0;

    m_nLastAimTime = 0;
    m_nShootTime   = 0;

    m_nAddGravityBase       = 0;
    m_nGravityPercent       = 0;

    m_dwAttachedDoodadID    = ERROR_ID;
    m_eMoveState            = cmsOnStand;
    m_nMoveStateStartFrame  = 0;
    m_eLastMoveState        = cmsOnStand;
    m_nMoveFrameCounter     = 0;
    m_nMoveSyncFrame        = 0;
    m_bAimAt                = false;

    m_pCastingSkill         = NULL;
    m_dwAttackerID          = ERROR_ID;
    m_eBeAttackedToDir      = csdLeft;
    m_bCastingSkillTrackMoved = false;
    m_bWeakKnockDown        = false;

    m_eFaceDir              = csdRight;
    m_nVelocityDir          = 0;
    m_nVirtualFrame         = 0;
    m_nCreateTime           = 0;

    m_nCopyIndex            = 0;
    m_bIsRunMode            = false;
    m_MoveCount             = 0;

    m_nSide                 = 0;
    m_nPos                  = 0;
    m_pTemplate             = NULL;
    m_bAiMode               = false;
    m_bAiTurnOffFlag        = false;
    memset(&m_AIData, 0, sizeof(m_AIData));

    m_bForbitAutoAction     = false;
    m_wFashionID            = 0;
    m_nForbitAutoTakeBall   = 0;
    m_bSkillSlaming = false;
    m_bForbitSkill  = false;
    m_bForbitPick   = false;
    m_pOwner        = NULL;

    m_nWidth = CELL_LENGTH * 2;
    m_nLength = CELL_LENGTH * 2;
    m_nHeight = CELL_LENGTH * 4 + 50;

    m_pPreSkill  = NULL;
    m_nPreSkillCastTime = 0;
    m_nPreSkillEndTime = 0;
    m_dwPerSkillTargetID = 0;
    m_nLastSyncCurrentEndurance = -1;
    m_nLastSyncCurrentStamina   = -1;
    m_nLastSyncCurrentAngry     = -1;

    m_nTriggerDepth = 0;
    m_bInvincible = false;
    m_nFreezeFrame = 0;

    m_nFrameToApplySkillEffect = 0;
    m_pSkillDelayApply = NULL;

    m_bExhausted = false;

    m_nAttackSkillTime = 0;

    m_pNpcTemplate = NULL;

    memset(&m_GameStat, 0, sizeof(m_GameStat));

    m_bCanBeAttack              = true;
    m_dwGrabHeroID              = ERROR_ID;
    m_bBaTi                     = false;

    m_bWalkAutoMove             = false;
    m_eAutoMoveStatus           = KAUTOMOVE_STATUS_NONE;
    m_nLastCalcStaminaPercent   = 100;
    m_nAutoMoveAroundX          = 0;
    m_nAutoMoveAroundY          = 0;
    m_nDestX                    = 0;
    m_nDestY                    = 0;
    m_bSkillSlamingHit          = false;
    m_pCastingSlamBallSkill     = NULL;

    memset(m_PassiveSkillUsed, 0, sizeof(m_PassiveSkillUsed));
    memset(m_ActiveSkill, 0, sizeof(m_ActiveSkill));
    memset(m_PassiveSkill, 0, sizeof(m_PassiveSkill));
    memset(m_NirvanaSkill, 0, sizeof(m_NirvanaSkill));
    memset(m_SlamBallSkill, 0, sizeof(m_SlamBallSkill));

    m_nKnockoffVelocityX = 0;
    m_nKnockoffVelocityY = 0;
    m_nKnockoffVelocityZ = 0;
    m_nAttackCountOnStiff = 0;

    m_nProtectEndFrame = 0;
}

KHero::~KHero()
{
}    

BOOL KHero::Init()
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    BOOL bSceneInitFlag = false;

    bRetCode = KMovableObject::Init();
    KGLOG_PROCESS_ERROR(bRetCode);

    m_BuffList.Init(this);

    m_bForbitSkill = false;

    m_bForbitPick = false;

    InitAIFunctions();

    bResult = true;
Exit0:    
    return bResult;
}

void KHero::UnInit()
{
    assert(m_pScene);

    EnsureUntakeBall();
    EnsureNoAttached();
    EnsureNoFollows();
    EnsureNoGuide();

    Detach();
    m_BuffList.UnInit();
    KMovableObject::UnInit();
}

void KHero::Activate(int nCurLoop)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KPOSITION cPos = GetPosition();

    bRetCode = CheckGameLoop(nCurLoop);
    KG_PROCESS_SUCCESS(!bRetCode);

    RecordStatus();
    //if (IsMainHero())
    //    KGLogPrintf(KGLOG_INFO, "ID:%u F:%d M:%d %s movecounter:%d", m_dwID, m_nVirtualFrame, m_eMoveState, ToStatusString().c_str(), m_nMoveFrameCounter);

    ++m_nVirtualFrame;

    KMovableObject::Activate(nCurLoop);

    MoveTriggerPassive();


    m_BuffList.Activate();

    CheckObjStandOn(cPos);
    RunStateMachine();

    if (m_nForbitAutoTakeBall > 0)
    {
        --m_nForbitAutoTakeBall;
    }

    DoCycleRevise();

    DoDelayAction();

    DoCycleSynchronous();

    if (m_nMoveSyncFrame != 0 && nCurLoop >= m_nMoveSyncFrame)
    {
        g_PlayerServer.DoSyncMoveState(this, m_nVirtualFrame, false);
        m_nMoveSyncFrame = 0;
    }

    FixAutoTakeBallBug();

Exit1:
Exit0:
    return;
}

void KHero::SpecialActivate(int nCurLoop)
{
    OverTimeSkillProcess();
    TryFinishCastingSkill();
    TryTriggerGrabSkill();
    ProcessAI();
    ProcessAutoMove();
}

void KHero::SetMoveState(int nState)
{
    assert(nState > cmsInvalid && nState < cmsTotal);

    m_eLastMoveState = m_eMoveState;
    m_eMoveState = (KHeroMoveState)nState;
    m_nMoveStateStartFrame = m_nVirtualFrame;
    //KGLogPrintf(KGLOG_DEBUG, "ID:%u F:%d, UpdateMoveState(%d,%d).", m_dwID, m_nVirtualFrame, m_eLastMoveState, m_eMoveState);
}


BOOL KHero::UpdateMoveParameter()
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KPlayer* pOwner = NULL;

    if (m_eMoveState == cmsOnWalk)
    {
        UpdateVelocity(m_nMoveSpeed, m_nVelocityDir);
    }
    else if (m_eMoveState == cmsOnRun)
    {
        UpdateVelocity((int)(m_nMoveSpeed * 1.5), m_nVelocityDir);
    }

    pOwner = GetOwner();
    if (pOwner && pOwner->m_eGameStatus == gsPlaying)
    {
        bRetCode = IsMainHero();
        if (bRetCode)
            g_PlayerServer.DoUpdateMoveParam(this);

        g_PlayerServer.DoSyncMoveState(this, m_nVirtualFrame, true);
    }

    bResult = true;
Exit0:
    return bResult;
}

void KHero::InvalidateMoveState(int nDelayFrame, BOOL bSyncSelf)
{
    if (bSyncSelf)
    {
        g_PlayerServer.DoSyncMoveState(this, m_nVirtualFrame, true);
        goto Exit0;
    }

    if (m_nMoveSyncFrame == 0 || (g_pSO3World->m_nGameLoop + nDelayFrame) < m_nMoveSyncFrame)
        m_nMoveSyncFrame = g_pSO3World->m_nGameLoop + nDelayFrame;

Exit0:
    return;
}

BOOL KHero::Stand(BOOL bSyncSelf)
{
    BOOL bResult            = false;

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    if (m_eMoveState != cmsOnStand && m_eMoveState != cmsOnWalk &&
        m_eMoveState != cmsOnRun   && m_eMoveState != cmsOnAimat)
    {
        goto Exit0;
    }

    ClearVelocity();
    SetMoveState(cmsOnStand);
    InvalidateMoveState(0, bSyncSelf);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::WalkTo(int nDirection, BOOL bSyncSelf)
{
    BOOL bResult            = false;
    BOOL bRetCode           = false;

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    if (m_eMoveState != cmsOnWalk && m_eMoveState != cmsOnRun && m_eMoveState != cmsOnStand)
    {
        goto Exit0;
    }

    SetMoveState(cmsOnWalk);

    UpdateVelocity(m_nMoveSpeed, nDirection);

    UpdateFaceDir(nDirection);

    InvalidateMoveState(1, bSyncSelf);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::RunTo(int nDirection, BOOL bSyncSelf)
{
    BOOL bResult            = false;
    BOOL bRetCode           = false;

    KG_PROCESS_ERROR(!m_pCastingSkill);

    if (m_eMoveState != cmsOnWalk && m_eMoveState != cmsOnRun && m_eMoveState != cmsOnStand)
    {
        goto Exit0;
    }

    SetMoveState(cmsOnRun);

    UpdateVelocity((int)(m_nMoveSpeed * 1.5), nDirection);

    UpdateFaceDir(nDirection);

    InvalidateMoveState(1, bSyncSelf);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::Jump(BOOL bMove, int nDirection, BOOL bSyncSelf)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    if (
        m_eMoveState != cmsOnJump && m_eMoveState != cmsOnStand &&
        m_eMoveState != cmsOnWalk && m_eMoveState != cmsOnRun
        ) 
    {
        goto Exit0;
    }

    KGLOG_PROCESS_ERROR(m_nJumpCount < cdMaxJumpCount);
    ++m_nJumpCount;

    m_nVelocityZ = m_nJumpSpeed;

    if (m_nJumpCount > 1 || IsStandOnObj())
    {
        if (bMove)
        {
            UpdateVelocity(m_nMoveSpeed, nDirection);
            UpdateFaceDir(nDirection);
        }
        else
        {
            m_nVelocityX = 0;
            m_nVelocityY = 0;
        }
    }

    SetMoveState(cmsOnJump);

    InvalidateMoveState(0, bSyncSelf);

    EnsureNotStandOnObject();

    m_dwObstacleID = ERROR_ID;
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::TakeBall(BOOL bNotifySelf)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KBall*  pBall       = NULL;

    KG_PROCESS_ERROR(!m_pCastingSkill);

    assert(m_pScene);
    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    bRetCode = CanTakeDoodad();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pBall->BeToken(this);
    KG_PROCESS_ERROR(bRetCode);

    // 广播球被谁take
    g_PlayerServer.DoTakeBall(this, bNotifySelf);

    LogInfo("Ball take by player:%d", m_dwID);

    m_pScene->OnBallToken(this);

    pBall->m_bShootFlagForCandyBag = true;

    bResult = true;
Exit0:
    return bResult;
}

void KHero::PassBallTo(DWORD dwID)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KHero* pReceiver = NULL;

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    pReceiver = m_pScene->GetHeroById(dwID);
    KGLOG_PROCESS_ERROR(pReceiver);

    bRetCode = PassBallTo(pReceiver, true);
    KG_PROCESS_ERROR(bRetCode);

    m_GameStat.m_nPassBall += 1;

    bResult = true;
Exit0:
    return;
}

BOOL KHero::AimAt(BOOL bAimAt, int nDirection, DWORD dwHoldingObjID, BOOL bSyncSelf)
{ 
    BOOL bResult = false;
    BOOL bTakeSomething = HasFollowerWithState(mosToken);

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    KG_PROCESS_ERROR(bTakeSomething);
    KGLOG_PROCESS_ERROR(!m_bSkillSlaming);

    if (!IsInAir())
    {
        ClearVelocity();
        InvalidateMoveState(0, false);
    }

    m_bAimAt = bAimAt;

    if (m_bAimAt)
    {
        if (m_eMoveState != cmsOnAimat)
            SetMoveState(cmsOnAimat);
    }
    else
    {
        if (IsInAir())
            SetMoveState(cmsOnJump);
        else
            SetMoveState(cmsOnStand);
    }

    UpdateFaceDir(nDirection);

    m_nLastAimTime = m_nVirtualFrame; 

    g_PlayerServer.DoAimAt(this, dwHoldingObjID, bSyncSelf);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::ShootBasket(DWORD dwBasketID)
{
    BOOL bResult = false;
    KBasket* pBasket = NULL;
    KTWO_DIRECTION eDir = csdInvalid;
    KBasketSocket* pBasketSocket = NULL;
    KDoodad* pObject = NULL;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    pObject = m_pScene->GetDoodadById(dwBasketID);
    KG_PROCESS_ERROR(pObject);

    KGLOG_PROCESS_ERROR(pObject->GetType() == sotBasket);
    pBasket = (KBasket*)(pObject);

    eDir = GetFaceDir();
    pBasketSocket = m_pScene->GetShootBasketBasketSocket(eDir, m_nShootTime);
    KGLOG_PROCESS_ERROR(pBasketSocket);

    pBasket->BeShoot(this, pBasketSocket);    

    g_PlayerServer.DoShootBasket(this, dwBasketID, pBasketSocket);

    if (IsInAir())
    {
        SetMoveState(cmsOnJump);
    }
    else
    {
        Stand(false);
    }

    bResult = true;
Exit0:
    return bResult;
}

void KHero::Sit()
{
    ClearVelocity();

    SetMoveState(cmsOnSit);
    m_nMoveFrameCounter = 8;

Exit0:
    return;
}

void KHero::BeAttacked(KMovableObject* pAttacker)
{
    BOOL bRetCode = false;
    BOOL bIsHero  = false;
	int nSide = sidLeft;
	KMission* pMission = NULL;
    assert(pAttacker);

    m_dwAttackerID      = pAttacker->m_dwID;
    m_eBeAttackedToDir  = pAttacker->GetAttackingDir();

    bIsHero = pAttacker->Is(sotHero);
    OnAttacked(!bIsHero);

    // 受伤后1s才能开始回复
    m_nNextReplenishFrame = g_pSO3World->m_nGameLoop + GAME_FPS;

	KG_PROCESS_ERROR(pAttacker->m_pTemplate);

	if (m_nSide == sidLeft)
		nSide = sidRight;

	m_pScene->BroadcastEvent(nSide, peEnemyBeAttackBySceneObj, pAttacker->m_pTemplate->m_dwID);
	m_pScene->BroadcastEvent(m_nSide, peBeAttackBySceneObj, pAttacker->m_pTemplate->m_dwID);

Exit0:
    return;
}

void KHero::BeAttackedByHero(KHero* pAttacker, KSkill* pSkill, KSkillSegmentInfo* pSegment /*= NULL*/)
{
    BOOL    bRetCode            = false;
    KBall*  pBall               = NULL;
    int     nRandom             = 0;
    int     nAgility            = 0;
    int     nAddAngry           = 0;
    BOOL    bEnduranceCrited    = false;
    int     nEnduranceDamage    = 0;
    BOOL    bStaminaCrited      = false;
    int     nStaminaDamage      = 0;
    int     nBeAttackedType     = 0;

    assert(pAttacker);
    assert(pSkill);

    assert(m_nLevel > 0);

    nAgility = (int)(pAttacker->m_nCritPoint * (0.61 * m_nLevel * m_nLevel - 72.5 * m_nLevel - 0.45* MAX(m_nLevel - 50, 0) * MAX(m_nLevel - 50, 0) - 
        0.085 * MAX(m_nLevel - 70, 0) * MAX(m_nLevel - 70, 0) + 2450) / 10000);
    if (nAgility < 0)
        nAgility = 0;

    nAgility += pAttacker->m_nAgility;
    if (nAgility > cdMaxAgility)
        nAgility = cdMaxAgility;

    // 耐力伤害
    nEnduranceDamage = 
        (pAttacker->m_nAttackPoint * (HUNDRED_NUM + pAttacker->m_nAttackPointPercent) * pSkill->m_nTargetCostEndurancePercent) / (HUNDRED_NUM * KILO_NUM)
        + pSkill->m_nTargetCostEndurance;

    nRandom = g_Random(KILO_NUM);
    if (nRandom < nAgility) // 暴击
    {
        bEnduranceCrited = true;
        nEnduranceDamage = nEnduranceDamage * m_nCritRate / HUNDRED_NUM;
    }
    nEnduranceDamage = ProcessEnduranceRemission(nEnduranceDamage, pAttacker);
    KGLOG_PROCESS_ERROR(nEnduranceDamage >= 0);

    if (IsInAir())
    {
        nEnduranceDamage = nEnduranceDamage * g_pSO3World->m_Settings.m_ConstList.nEnduranceCoeInAir / HUNDRED_NUM;
    }

    LoseEndurance(nEnduranceDamage);

    // 怒气
    if (m_nMaxEndurance > 0)
    {
        nAddAngry = 1000 * nEnduranceDamage / m_nMaxEndurance / 12;
        bRetCode = AddAngry(nAddAngry);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    // 体力伤害
    nStaminaDamage = (pAttacker->m_nAttackPoint * (HUNDRED_NUM + pAttacker->m_nAttackPointPercent) * pSkill->m_nTargetCostStaminaPercent / (HUNDRED_NUM * KILO_NUM)
        + pSkill->m_nTargetCostStamina);
    nRandom = g_Random(KILO_NUM);
    if (nRandom < nAgility) // 暴击
    {
        bStaminaCrited = true;
        nStaminaDamage = nStaminaDamage * m_nCritRate / HUNDRED_NUM;
    }
    nStaminaDamage = ProcessStaminaRemission(nStaminaDamage, pAttacker);
    KGLOG_PROCESS_ERROR(nStaminaDamage >= 0);

    LoseStamina(nStaminaDamage);

    if (pSegment)
        nBeAttackedType = pSegment->nBeAttackType;

    g_PlayerServer.DoSyncDamage(this, bEnduranceCrited, nEnduranceDamage, bStaminaCrited, nStaminaDamage, nBeAttackedType);

    BeAttacked(pAttacker);

    // 检查是否要进入僵直
    if (pSegment)
    {
        StartStiff(pSegment->nStiffFrame);

        bRetCode = ApplyBuffAndAttributes(pSegment->dwTargetBuffID, pSegment->vecTargetAttributes);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        ApplySkillEffect(pSkill);
    }

    m_pScene->OnAttacked(pAttacker);

Exit0:
    return;
}

void KHero::Halt()
{
    m_nCurrentEndurance = 0;

    KG_PROCESS_ERROR(m_eMoveState != cmsOnKnockedOff);
    KG_PROCESS_ERROR(m_eMoveState != cmsOnKnockedDown);

    ClearVelocity();

    KG_PROCESS_ERROR(m_eMoveState != cmsOnHalt);

    SetMoveState(cmsOnHalt);
    m_nMoveFrameCounter = g_pSO3World->m_Settings.m_ConstList.nPlayerHaltTime;
    InvalidateMoveState(0, true);

    m_cRecordedStatus.ClearRecord();
Exit0:
    return;
}

void KHero::WeakKnockDown(int nVelocityXY, int nVelocityZ)
{
    KnockedOff(nVelocityXY, nVelocityZ);
    m_bWeakKnockDown = true;
}

void KHero::KnockedOff(int nVelocityXY, int nVelocityZ)
{
    int nDirection = GetAutoMoveDir(amdAttackDirectionForward);
    int nVelocityX = nVelocityXY * g_Cos(nDirection) / SIN_COS_NUMBER;
    int nVelocityY = nVelocityXY * g_Sin(nDirection) / SIN_COS_NUMBER;

    KnockedOff(nVelocityX, m_nVelocityY, nVelocityZ);

    return;
}

void KHero::KnockedOff(int nVelocityX, int nVelocityY, int nVelocityZ)
{
    if (m_eMoveState == cmsOnStiff) // 僵直
    {
        m_nKnockoffVelocityX = nVelocityX;
        m_nKnockoffVelocityY = nVelocityY;
        m_nKnockoffVelocityZ = nVelocityZ;
        goto Exit0;
    }

    if (m_eMoveState == cmsOnKnockedDown)
        goto Exit0;

    if (m_bBaTi)
        goto Exit0;

    /*
    if (m_eMoveState == cmsOnHalt && m_nMoveFrameCounter == g_pSO3World->m_Settings.m_ConstList.nPlayerHaltTime) // 在开始喘气时会被击飞
    {
        m_nVelocityX = nVelocityX;
        m_nVelocityY = nVelocityY;
        m_nVelocityZ = nVelocityZ;
        InvalidateMoveState(0, true);
        goto Exit0;
    }
    */

    if (m_eMoveState == cmsOnHalt) // 在喘气会被击飞
    {
        m_nCurrentEndurance = 0;
    }

    m_nVelocityX = nVelocityX;
    m_nVelocityY = nVelocityY;
    m_nVelocityZ = nVelocityZ;

    if (!IsAlive())
    {
        int nFactor = g_pSO3World->m_Settings.m_ConstList.nKnockOffSpeedPercentOnDead / HUNDRED_NUM;
        m_nVelocityX *= nFactor;
        m_nVelocityY *= nFactor;
        m_nVelocityZ *= nFactor;
    }

    SetMoveState(cmsOnKnockedOff);
    m_bWeakKnockDown = false;

    InvalidateMoveState(0, true);

    m_cRecordedStatus.ClearRecord();
Exit0:
    //KGLogPrintf(KGLOG_DEBUG, "KnockedOff p=%d,%d,%d v=%d,%d,%d!", m_nX, m_nY, m_nZ, m_nVelocityX, m_nVelocityY, m_nVelocityZ);
    return;
}

void KHero::KnockedDown()
{
    if (m_eMoveState == cmsOnKnockedDown)
        goto Exit0;

    if (m_bBaTi)
        goto Exit0;

    ClearVelocity();

    if (IsAlive() && m_bWeakKnockDown)
        m_nMoveFrameCounter = g_pSO3World->m_Settings.m_ConstList.nPlayerWeakKnockDownTime;
    else
        m_nMoveFrameCounter = g_pSO3World->m_Settings.m_ConstList.nPlayerKnockDownTime;
    SetMoveState(cmsOnKnockedDown);

    // npc死亡事件
    {
        std::vector<KHero*> vecHeros;
        assert(m_pScene);
        m_pScene->GetAllHero(vecHeros);
        for (size_t i = 0; i < vecHeros.size(); ++i)
        {
            KPlayer* pOwner = NULL;
            KHero* pHero = vecHeros[i];
            if (!pHero->IsMainHero()) // 只对主英雄的玩家发事件
                continue;

            if (pHero == this) // 不能是自己
                continue;

            if (pHero->m_nSide == m_nSide) // 同一队的npc死亡不处理
                continue;

            pOwner = pHero->GetOwner();
            assert(pOwner);
            pOwner->OnEvent(peNpcDie, m_dwID);
        }

        ++m_GameStat.m_nDieTimes;
    }

    // 中间的事件处理可能会改变移动参数，所以要放最后同步
    InvalidateMoveState(0, true);

    m_cRecordedStatus.ClearRecord();
Exit0:
    return;   
}

int  KHero::ProcessEnduranceRemission(int nDamage, KHero* pAttacker)
{
    int nRemissionDamage = 0;
    int nPercent = 0;
    int nDefense = 0;

    assert(pAttacker);

    KGLOG_PROCESS_ERROR(pAttacker->m_nLevel > 0);

    nDefense = m_nDefense * (HUNDRED_NUM + m_nDefensePointPercent) / HUNDRED_NUM;
    if (nDefense < 0)
        nDefense = 0;

    nPercent = HUNDRED_NUM * nDefense / (pAttacker->m_nLevel * 50 + nDefense);
    if (nPercent > 75)
        nPercent = 75;

    nRemissionDamage = nDamage * ( HUNDRED_NUM - nPercent) / HUNDRED_NUM;

    if (nRemissionDamage < 0)
        nRemissionDamage = 0;

Exit0:
    return nRemissionDamage;
}

int  KHero::ProcessStaminaRemission(int nDamage, KHero* pAttacker)
{
    int nRemissionDamage = 0;
    int nPercent = 0;
    int nDefense = 0;

    assert(pAttacker);

    KGLOG_PROCESS_ERROR(pAttacker->m_nLevel > 0);
    nDefense = m_nDefense * (HUNDRED_NUM + m_nDefensePointPercent) / HUNDRED_NUM;
    if (nDefense < 0)
        nDefense = 0;

    nPercent = HUNDRED_NUM * nDefense / (pAttacker->m_nLevel * 50 + nDefense);
    nPercent += m_nDefensePercent;
    if (nPercent > 75)
        nPercent = 75;

    nRemissionDamage = nDamage * ( HUNDRED_NUM - nPercent) / HUNDRED_NUM;

    if (nRemissionDamage < 0)
        nRemissionDamage = 0;

Exit0:
    return nRemissionDamage;
}

BOOL KHero::AddAngry(int nShouldAddAngry)
{
    m_nCurrentAngry += nShouldAddAngry + m_nAdditionalAddAngryPercent * nShouldAddAngry / HUNDRED_NUM;
    MAKE_IN_RANGE(m_nCurrentAngry, 0, m_nMaxAngry);

    return true;
}

BOOL KHero::AutoMove(int nVelocity, KAutoMoveDirection eDirType)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nDirection  = 0;

    switch (eDirType)
    {
    case amdUp:
        {
            m_nVelocityZ = nVelocity;
        }
        break;
    case amdLeft:
        {
            UpdateVelocity(nVelocity,  DIRECTION_COUNT / 2);
            UpdateFaceDir(DIRECTION_COUNT / 2);
        }
        break;
    case amdRight:
        {
            UpdateVelocity(nVelocity,  0);
            UpdateFaceDir(0);
        }
        break;
    case amdIn:
        {
            UpdateVelocity(nVelocity,  DIRECTION_COUNT * 1 / 4);
        }
        break;
    case amdOut:
        {
            UpdateVelocity(nVelocity,  DIRECTION_COUNT * 3 / 4);
        }
        break;
    default:
        {
            nDirection = GetAutoMoveDir(eDirType);
            UpdateVelocity(nVelocity, nDirection);
        }
        break;
    }
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::LoseControlByCounter(int nFrame)
{
    if (m_bBaTi)
        return true;

    SetMoveState(cmsOnLoseControlByCounter);
    m_nMoveFrameCounter = nFrame;
    m_cRecordedStatus.ClearRecord();

    return true;
}

void KHero::StopLoseControlByCounter()
{
    if (m_eMoveState != cmsOnLoseControlByCounter)
        return;

    ClearVelocity();

    if (IsInAir())
    {
        SetMoveState(cmsOnJump);
        if (m_nJumpCount == 0) // 已经在天上将跳跃次数置为1
            m_nJumpCount = 1;
    }
    else
    {
        SetMoveState(cmsOnStand); 
        m_nJumpCount = 0;
    }
}

void KHero::StartStiff(int nStiffFrame)
{
    KG_PROCESS_ERROR(nStiffFrame > 0);
    KG_PROCESS_ERROR(!m_pCastingSkill);
    KG_PROCESS_ERROR(!m_bBaTi);

    if (m_eMoveState == cmsOnStiff)
    {
        KG_PROCESS_ERROR(m_nAttackCountOnStiff <= 10);
        ++m_nAttackCountOnStiff;
    }

    ClearVelocity();

    m_nMoveFrameCounter = nStiffFrame;
    SetMoveState(cmsOnStiff);

    InvalidateMoveState(0, true);

Exit0:
    return;
}

void KHero::StopStiff()
{
    if (m_eMoveState != cmsOnStiff)
        return;

    m_nAttackCountOnStiff = 0;

    SetMoveState(cmsOnStand);

    if (m_nKnockoffVelocityX != 0 || m_nKnockoffVelocityY != 0 || m_nKnockoffVelocityZ != 0)
    {
        KnockedOff(m_nKnockoffVelocityX, m_nKnockoffVelocityY, m_nKnockoffVelocityZ);
        m_nKnockoffVelocityX = 0;
        m_nKnockoffVelocityY = 0;
        m_nKnockoffVelocityZ = 0;
    }
}

BOOL KHero::CanCastSkill(KSkill* pSkill, KTWO_DIRECTION eFaceDir, BOOL bSubSkill)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KDoodad* pDoodad    = NULL;
    KCAST_SKILL_COUNT_INAIR_LIST::iterator it;
    int nFrameCount = 0;

    KGLOG_PROCESS_ERROR(pSkill);

    if (bSubSkill)
    {
        // 子技能由客户端触发，因网络延时不稳定可能存在触发时父技能已经结束
        if (m_pCastingSkill)
        {
            bRetCode = m_pCastingSkill->HasSubSkill(pSkill->m_dwID, pSkill->m_dwStepID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
        else
        {
            KG_PROCESS_ERROR(m_pPreSkill);

            if (IsMainHero())
                KGLOG_PROCESS_ERROR(m_nVirtualFrame - m_nPreSkillEndTime < SKILL_CAST_MAX_DELAY);
            
            bRetCode = m_pPreSkill->HasSubSkill(pSkill->m_dwID, pSkill->m_dwStepID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }
    else
    {
        bRetCode = HasSkill(pSkill->m_dwID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    KG_PROCESS_ERROR(
        m_eMoveState == cmsOnStand || m_eMoveState == cmsOnWalk     || 
        m_eMoveState == cmsOnRun   || m_eMoveState == cmsOnJump     || 
        m_eMoveState == cmsOnSit   || m_eMoveState == cmsOnAimat
    );

    pDoodad = GetHoldingDoodad();
    switch (pSkill->m_eSkillType)
    {
    case skilltypeCommon:
    case skilltypeGrab:
        if (pDoodad)
        {
            KG_PROCESS_ERROR(pSkill->m_bCanCastWithBall);
            KG_PROCESS_ERROR(pDoodad->Is(sotBall));
        }
        break;
    case skilltypeShootBall:
        KG_PROCESS_ERROR(pDoodad && pDoodad->Is(sotBall));
        break;
    case skilltypeSight:
        break;
    default:
        KG_PROCESS_ERROR(false);
    }

    bRetCode = IsForbitCastSkill();
    KG_PROCESS_ERROR(!bRetCode);
    
    bRetCode = pSkill->CheckStateCondition(m_eMoveState, m_nVirtualFrame - m_nMoveStateStartFrame);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pCastingSkill && !bSubSkill)
    {
        KGLOG_PROCESS_ERROR(pSkill->m_nInterruptLevel > m_pCastingSkill->m_nInterruptLevel);
    }

    KGLOG_PROCESS_ERROR(m_nCurrentAngry >= pSkill->m_nNeedAngry);

    if (pSkill->m_nCostAngry > 0) // 优先扣点数
    {
        KGLOG_PROCESS_ERROR(m_nCurrentAngry >= pSkill->m_nCostAngry);
    }
    else if (pSkill->m_nCostAngryPercent > 0)
    {
        int nCostAngry = m_nCurrentAngry * pSkill->m_nCostAngryPercent / HUNDRED_NUM;
        KGLOG_PROCESS_ERROR(m_nCurrentAngry >= nCostAngry);
    }

    bRetCode = IsInAir();
    if (bRetCode)
    {
        if (pSkill->m_nCanCastCountInAir < 0)
            goto Exit0;

        if (pSkill->m_nCanCastCountInAir > 0)
        {
            int nCastCountInAir = 0;
            uint64_t ullKey = MAKE_INT64(pSkill->m_dwID, pSkill->m_dwStepID);
            it = m_mapCastSkillCountInAir.find(ullKey);
            if (it != m_mapCastSkillCountInAir.end())
                nCastCountInAir = it->second;

            KGLOG_PROCESS_ERROR(nCastCountInAir < pSkill->m_nCanCastCountInAir);
        }
    }

    if (m_pOwner)
    {
        KGLOG_PROCESS_ERROR(m_nLevel >= pSkill->m_nRequireLevel);
    }

    KGLOG_PROCESS_ERROR(eFaceDir == csdRight || eFaceDir == csdLeft);
    KGLOG_PROCESS_ERROR(pSkill->m_bCanSpecifyCastDir || eFaceDir == m_eFaceDir);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::DoSkillCost(KSkill* pSkill)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;

    assert(pSkill);

    if (pSkill->m_nCostAngry > 0) // 优先扣点数
    {
        KGLOG_PROCESS_ERROR(m_nCurrentAngry >= pSkill->m_nCostAngry);
        bRetCode = AddAngry(-pSkill->m_nCostAngry);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else if (pSkill->m_nCostAngryPercent > 0)
    {
        int nCostAngry = m_nCurrentAngry * pSkill->m_nCostAngryPercent / HUNDRED_NUM;
        KGLOG_PROCESS_ERROR(m_nCurrentAngry >= nCostAngry);

        bRetCode = AddAngry(-nCostAngry);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::CastSkill(DWORD dwSkillID, DWORD dwStep, KTWO_DIRECTION eFaceDir, BOOL bSubSkill /*= false*/, BOOL bSyncSelf /*= false*/)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KSkill* pSkill      = NULL;
    KSkillSegmentInfo* pSegmentInfo = NULL;

    pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID, dwStep);
    KGLOG_PROCESS_ERROR(pSkill);

    bRetCode = CanCastSkill(pSkill, eFaceDir, bSubSkill);
    KG_PROCESS_ERROR(bRetCode);

    if (m_pOwner && IsMainHero() && dwStep == 0)
    {
        if (dwSkillID == 23 || dwSkillID == 79)
        {
            m_pOwner->OnEvent(peFinishQianChongQuan);
        }
        else if (dwSkillID == 29 || dwSkillID == 82)
        {
            m_pOwner->OnEvent(peFinishKongZhongJiao); 
        }
        else if (dwSkillID == 30 || dwSkillID == 83)
        {
            m_pOwner->OnEvent(peFinishQiShenQuan);   
        }
    }

    bRetCode = DoSkillCost(pSkill);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = PrepareToCastSkill();
    KGLOG_PROCESS_ERROR(bRetCode);

    m_pCastingSkill     = pSkill;
    m_nAttackSkillTime  = m_nVirtualFrame;
    m_pPreSkill         = pSkill;
    m_nPreSkillCastTime     = m_nVirtualFrame;
    m_dwPerSkillTargetID = 0;
    m_nPreSkillEndTime  = 0;
    m_bCastingSkillTrackMoved = false;

    m_eFaceDir = eFaceDir;
    m_nVelocityDir = eFaceDir == csdRight ? 0 : DIRECTION_COUNT / 2;

    bRetCode = ProcessCastSkillInAir();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = ApplyBuffAndAttributes(m_pCastingSkill->m_dwSrcBuffID, m_pCastingSkill->m_pSrcAttribute);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = UpdateVelocityByAnchors();
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pCastingSkill->m_nRestraintType == KRESTRAINT_TYPE_BATI)
        m_bBaTi = true;

    g_PlayerServer.DoHeroSkill(this, m_pCastingSkill->m_dwID, m_pCastingSkill->m_dwStepID, bSyncSelf);

    bRetCode = FireCastSkillAIEvent(pSkill);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = ProcessSkillSegment();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = ProcessSkillAttack();
    KGLOG_PROCESS_ERROR(bRetCode);

	if (pSkill->m_bNirvanaSkill)
	{
		m_bInvincible = true;
		if (m_pOwner)
		{
			m_pOwner->OnEvent(peCastNirvanaSkill);
		}
	}

    LogInfo("Hero id=%d cast skill id=%d at frame=%d!", m_dwID, dwSkillID, m_nVirtualFrame);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::CanCastSpecialSkill(KSPECIAL_SKILL eSkillID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KSpecialSkillInfo* pInfo = NULL;
    int nCost = 0;

    pInfo = g_pSO3World->m_SkillManager.GetSpecialSkill(eSkillID);
    KGLOG_PROCESS_ERROR(pInfo);

    nCost = pInfo->GetCostAngry(m_nCurrentAngry);
    KG_PROCESS_ERROR(m_nCurrentAngry >= nCost);

    nCost = pInfo->GetCostStamina(m_nCurrentStamina);
    KG_PROCESS_ERROR(m_nCurrentStamina >= nCost);

    KG_PROCESS_ERROR(m_nLevel >= pInfo->nRequireLevel);

    bResult = true;
Exit0:
    return bResult;
}

void KHero::OnCastSpecialSkill(KSPECIAL_SKILL eSkillID)
{
    KSpecialSkillInfo* pInfo = NULL;
    int nCost = 0;

    pInfo = g_pSO3World->m_SkillManager.GetSpecialSkill(eSkillID);
    KGLOG_PROCESS_ERROR(pInfo);

    nCost = pInfo->GetCostAngry(m_nCurrentAngry);
    KGLOG_PROCESS_ERROR(m_nCurrentAngry >= nCost);
    m_nCurrentAngry -= nCost;

    nCost = pInfo->GetCostStamina(m_nCurrentStamina);
    KGLOG_PROCESS_ERROR(m_nCurrentStamina >= nCost);
    LoseStamina(nCost);

Exit0:
    return;
}

BOOL KHero::CanCastGrabSkill(KSkill* pSkill, KHero* pTargetHero)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    BOOL bMeAttackTarget = false;
    BOOL bTargetAttackMe = false;

    KGLOG_PROCESS_ERROR(pSkill);
    KGLOG_PROCESS_ERROR(pSkill->m_eSkillType == skilltypeGrab);

    KGLOG_PROCESS_ERROR(pTargetHero);

    bRetCode = CanCastSkill(pSkill, m_eFaceDir, true);
    KGLOG_PROCESS_ERROR(bRetCode);
    
    if (pTargetHero->m_pCastingSkill && pTargetHero->m_pCastingSkill->m_nRestraintType == KRESTRAINT_TYPE_NORMAL)
    {
        bRetCode = DoRestraintJudge(pTargetHero, bMeAttackTarget, bTargetAttackMe);
        KGLOG_PROCESS_ERROR(bRetCode);
        KG_PROCESS_ERROR(!bTargetAttackMe);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::CastGrabSkill(DWORD dwSkillID, DWORD dwStep, DWORD dwTargetHeroID)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KSkill* pSkill      = NULL;
    KHero*  pTargetHero = NULL;
    KSkillSegmentInfo* pSegmentInfo = NULL;

    pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID, dwStep);
    KGLOG_PROCESS_ERROR(pSkill);

    pTargetHero = m_pScene->GetHeroById(dwTargetHeroID);
    KGLOG_PROCESS_ERROR(pTargetHero);

    bRetCode = CanCastGrabSkill(pSkill, pTargetHero);
    KG_PROCESS_ERROR(bRetCode);

    if (
        m_pPreSkill &&
        m_pPreSkill->m_eSkillType == skilltypeGrab &&
        m_dwPerSkillTargetID == dwTargetHeroID
    )
    {
        pTargetHero->m_bInvincible = false; // 上一个抓投技使目标进入无敌状态
    }

    bRetCode = pTargetHero->CanBeAttacked();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = PrepareToCastSkill();
    KGLOG_PROCESS_ERROR(bRetCode);

    m_pCastingSkill     = pSkill;
    m_nAttackSkillTime  = m_nVirtualFrame;
    m_pPreSkill         = pSkill;
    m_nPreSkillCastTime = m_nVirtualFrame;
    m_dwGrabHeroID      = dwTargetHeroID;
    m_dwPerSkillTargetID = dwTargetHeroID;
    m_nPreSkillEndTime  = 0;
    m_bCastingSkillTrackMoved = false;

    bRetCode = ApplyBuffAndAttributes(m_pCastingSkill->m_dwSrcBuffID, m_pCastingSkill->m_pSrcAttribute);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pCastingSkill->m_nRestraintType == KRESTRAINT_TYPE_BATI)
        m_bBaTi = true;

    bRetCode = ProcessSkillSegment();
    KGLOG_PROCESS_ERROR(bRetCode);

    //bRetCode = ProcessSkillAttack();
    //KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = UpdateVelocityByAnchors();
    KGLOG_PROCESS_ERROR(bRetCode);

    m_bInvincible = true;

    AddFollower(pTargetHero);
    pTargetHero->TurnToMoveType(mosToken);

    pTargetHero->OnAttacked(true);
    bRetCode = pTargetHero->LoseControlByCounter(pSkill->m_nEndFrame);
    KGLOG_PROCESS_ERROR(bRetCode);

    pTargetHero->m_bInvincible = true;

    g_PlayerServer.DoHeroGrabSkill(this, dwSkillID, dwStep, dwTargetHeroID);

    KGLogPrintf(KGLOG_DEBUG, "Hero id=%d cast grab skill id=%d at frame=%d!", m_dwID, dwSkillID, m_nVirtualFrame);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::TrackMoveByCastingSkill(DWORD dwPathID)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;

    KComplexPathInfo*   pPath           = NULL;
    

    KComplexPath    cPath;
    KPOSITION       cSrc;
    KPOSITION       cDst;
	KPOSITION*      pDst = NULL;

    KGLOG_PROCESS_ERROR(m_pScene);
    KGLOG_PROCESS_ERROR(m_pCastingSkill);

    pPath = g_pSO3World->m_Settings.m_ComplexPathList.GetPath(dwPathID);
    KGLOG_PROCESS_ERROR(pPath);

    if (pPath->eTargetFloor != bfInvalid)
    {
        KBasketSocket* pTargetSocket = m_pScene->GetBasketSocket(
            m_eFaceDir == csdRight ? csdLeft : csdRight, 
            pPath->eTargetFloor
        );
        KGLOG_PROCESS_ERROR(pTargetSocket);
		
        cDst = pTargetSocket->GetPosition();
        cDst.nZ += CELL_LENGTH * 3;
		
		pDst = &cDst;
    }

    cSrc = GetPosition();

    bRetCode = cPath.LoadComplexPath(
        &cSrc, 
        pDst, 
        pPath->cPoints, 
        this,
        pPath->eEndPathType
    );
    KGLOG_PROCESS_ERROR(bRetCode);

    cPath.Start(m_nVirtualFrame);
    DoTrackMove(&cPath);
    m_bCastingSkillTrackMoved = true;

    bResult = true;
Exit0:
    return bResult;
}

// 处理角色的状态机
void KHero::RunStateMachine()
{
    switch (m_eMoveState)
    {
    case cmsOnStand:
        break;
    case cmsOnWalk:
        break;
    case cmsOnRun:
        break;
    case cmsOnJump:
        break;
    case cmsOnSit:
        OnSit();
        break;
    case cmsOnAimat:
        break;
    case cmsOnKnockedDown:
        OnKnockedDown();
        break;
    case cmsOnKnockedOff:
        OnKnockedOff();
        break;
    case cmsOnHalt:
        OnHalt();
        break;
    case cmsOnFreeze:
        OnFreeze();
        break;
    case cmsOnLoseControlByCounter:
        OnLoseControlByCounter();
        break;
    case cmsOnStiff:
        OnStiff();
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Hero:%u, Invalid MoveState:%d!", m_dwID, m_eMoveState);
        break;
    }
}

void KHero::OnHalt()
{
    if (m_nVelocityX != 0 || m_nVelocityY != 0 || m_nVelocityZ != 0)
    {
        if (!IsInAir())
        {
            ClearVelocity();
        }
    }

    if (m_nMoveFrameCounter > 0)
        --m_nMoveFrameCounter;

    if (m_nMoveFrameCounter == 0)
    {
        LogInfo("Character %u revived!", m_dwID);
        m_nCurrentEndurance = m_nMaxEndurance;

        ClearVelocity();
        SetMoveState(cmsOnStand);
    }
}

void KHero::OnKnockedOff()
{
    if (!IsInAir())
    {
        if (!IsAlive() || m_bWeakKnockDown)
        {
            KnockedDown();
        }
        else
        {
            ClearVelocity();
            SetMoveState(cmsOnStand);
        }

        LogInfo("Character %u on knocked off at %s, v=%d,%d,%d", m_dwID, GetPosition().ToString().c_str(), m_nVelocityX, m_nVelocityY, m_nVelocityZ);
    }
}

void KHero::OnKnockedDown()
{
    if (m_nMoveFrameCounter > 0)
        --m_nMoveFrameCounter;

    if (m_nMoveFrameCounter == 0)
    {
        LogInfo("Character %u getup!", m_dwID);

        Sit();

        if (!IsAlive() || !m_bWeakKnockDown)
        {
            m_nCurrentEndurance = m_nMaxEndurance;
            m_nProtectEndFrame = g_pSO3World->m_nGameLoop + g_pSO3World->m_Settings.m_ConstList.nGetUpProtectFrame;
        }
        m_bWeakKnockDown = false;
        
        TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_STANDUP);
    }
}

void KHero::OnLoseControlByCounter()
{
    if (m_nMoveFrameCounter > 0)
        --m_nMoveFrameCounter;

    if (m_nMoveFrameCounter == 0)
        StopLoseControlByCounter();
}

void KHero::OnStiff()
{
    if (m_nMoveFrameCounter > 0)
        --m_nMoveFrameCounter;

    if (m_nMoveFrameCounter == 0)
        StopStiff();
}

void KHero::OnSit()
{
    if (m_nMoveFrameCounter > 0)
        --m_nMoveFrameCounter;

    if(m_nMoveFrameCounter <= 0)
    {
        ClearVelocity();
        SetMoveState(cmsOnStand);            
    }
}

KTWO_DIRECTION KHero::GetNextFaceDir(int nDirection)
{
    KTWO_DIRECTION eResult = m_eFaceDir;

    nDirection = nDirection % DIRECTION_COUNT;

    if (nDirection < 0)
        nDirection += DIRECTION_COUNT;

    if (nDirection == DIRECTION_COUNT / 4 || nDirection == DIRECTION_COUNT * 3 / 4)
        goto Exit0;

    if (nDirection < DIRECTION_COUNT / 4)
        eResult = csdRight;
    else if (nDirection < DIRECTION_COUNT * 3 / 4)
        eResult = csdLeft;
    else
        eResult = csdRight;

Exit0:
    return eResult;
}

void KHero::UpdateFaceDir(int nDirection)
{
    m_eFaceDir = GetNextFaceDir(nDirection);

    if (m_nVelocityDir != nDirection && m_pCastingSkill && m_pCastingSkill->m_bCanSpecifyCastingDir)
    {
        int nVelocity = (int)sqrt((double)(m_nVelocityX * m_nVelocityX + m_nVelocityY * m_nVelocityY));
        UpdateVelocity(nVelocity, nDirection);
        m_nVelocityDir = nDirection;
    }
}

KTWO_DIRECTION KHero::GetFaceDir() const
{
    return m_eFaceDir;
}

void KHero::SetFaceDir(KTWO_DIRECTION eNewDir, BOOL bSyncSelf)
{
    m_eFaceDir = eNewDir;
    InvalidateMoveState(0, bSyncSelf);
}

BOOL KHero::ProcessSkillAttack()
{
    BOOL             bResult  = false;
    BOOL             bRetCode = false;
    KSceneObject*    pObj     = NULL;
    KSceneObjectType eType    = sotInvalid;
    KBODY            JudgeBox;
    KVEC_OBJ         vecObj; 

    KGLOG_PROCESS_ERROR(m_pCastingSkill);
    KG_PROCESS_SUCCESS(m_pCastingSkill->m_eSkillType != skilltypeCommon && m_pCastingSkill->m_eSkillType != skilltypeShootBall);

    bRetCode = m_pCastingSkill->IsAttackStateAtFrame(m_nVirtualFrame - m_nAttackSkillTime);
    KG_PROCESS_SUCCESS(!bRetCode);

    bRetCode = m_pCastingSkill->GetJudageBox(this, JudgeBox);
    KGLOG_PROCESS_ERROR(bRetCode);

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        pObj = it.GetValue();
        if (!pObj->m_bCanBeAttack)
            continue;

        if (pObj->m_dwID == m_dwID)
            continue;

        KBODY cTargetBody = pObj->GetBody();

        bRetCode = g_IsCollide(JudgeBox, cTargetBody);
        if (!bRetCode)
            continue;

        eType = pObj->GetType();      
        switch(eType)
        {
        case sotHero:
            AttackHero(m_pCastingSkill, (KHero*)pObj);
            break;
        case sotDummy:
            AttackDummy(m_pCastingSkill, (KDummy*)pObj);
            break;
        case sotBarrier:
            AttackBarrier(m_pCastingSkill, (KBarrier*)pObj);
            break;
        case sotBuffBox:
            AttackBuffBox(m_pCastingSkill, (KBuffBox*)pObj);
            break;
        default:
            break;
        }

        if (!m_pCastingSkill)
            break;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KHero::AttackHero(KSkill* pSkill, KHero* pHero)
{
    BOOL bRetCode = false;
    KRelationType eRelation;
    KATTACKED_OBJ_LIST::iterator itAttack;
    KSkillSegmentInfo* pSegmentInfo = NULL;
    BOOL bMeAttackTarget = false;
    BOOL bTargetAttackMe = false;
    KSkill* pTargetCastingSkill = NULL;

    assert(pSkill);
    assert(pHero);

    eRelation = GetRelationTypeWith(pHero);
    bRetCode = pSkill->CanBeApplyOn(eRelation);
    KG_PROCESS_ERROR(bRetCode);


    if (pSkill->m_dwTargetBuffLimit != ERROR_ID)
    {
        bRetCode = pHero->HasBuff(pSkill->m_dwTargetBuffLimit);
        KG_PROCESS_ERROR(bRetCode);
    }

    itAttack = m_AttackObjList.find(pHero->m_dwID);
    if (itAttack != m_AttackObjList.end())
        KG_PROCESS_ERROR(g_pSO3World->m_nGameLoop > itAttack->second);

    bRetCode = DoRestraintJudge(pHero, bMeAttackTarget, bTargetAttackMe);
    KGLOG_PROCESS_ERROR(bRetCode);

    pTargetCastingSkill = pHero->m_pCastingSkill;

    if (bMeAttackTarget)
    {
        bRetCode = pHero->CanBeAttacked();
        KG_PROCESS_ERROR(bRetCode);

        g_PlayerServer.DoSkillResult(this, pHero->m_dwID, pSkill);

        pSegmentInfo = pSkill->GetSegmentOfFrameBelongTo(m_nVirtualFrame - m_nAttackSkillTime);
        pHero->BeAttackedByHero(this, pSkill, pSegmentInfo);

        if (pSegmentInfo)
        {
            m_AttackObjList[pHero->m_dwID] = g_pSO3World->m_nGameLoop + pSegmentInfo->nAttackIntervalFrame;
        }
        else
        {
            m_AttackObjList[pHero->m_dwID] = g_pSO3World->m_nGameLoop + pSkill->m_nAttackIntervalFrame;
        }
    }

    if (bTargetAttackMe)
    {
        KGLOG_PROCESS_ERROR(pTargetCastingSkill);

        bRetCode = CanBeAttacked();
        KG_PROCESS_ERROR(bRetCode);

        g_PlayerServer.DoSkillResult(pHero, m_dwID, pTargetCastingSkill);

        pSegmentInfo = pSkill->GetSegmentOfFrameBelongTo(pHero->m_nVirtualFrame - pHero->m_nAttackSkillTime); 
        BeAttackedByHero(pHero, pTargetCastingSkill, pSegmentInfo);

        if (pSegmentInfo)
        {
            pHero->m_AttackObjList[m_dwID] = g_pSO3World->m_nGameLoop + pSegmentInfo->nAttackIntervalFrame;
        }
        else
        {
            pHero->m_AttackObjList[m_dwID] = g_pSO3World->m_nGameLoop + pSkill->m_nAttackIntervalFrame;
        }
    }

Exit0:
    return;
}

void KHero::AttackBuffBox(KSkill* pSkill, KBuffBox* pObj)
{
    KATTACKED_OBJ_LIST::iterator itAttack;
    KSkillSegmentInfo* pSegmentInfo = NULL;

    assert(pSkill);
    assert(pObj);

    itAttack = m_AttackObjList.find(pObj->m_dwID);
    if (itAttack != m_AttackObjList.end())
        KG_PROCESS_ERROR(g_pSO3World->m_nGameLoop > itAttack->second);

    pObj->BeAttacked();

    pSegmentInfo = pSkill->GetSegmentOfFrameBelongTo(m_nVirtualFrame - m_nAttackSkillTime);
    if (pSegmentInfo)
    {
        m_AttackObjList[pObj->m_dwID] = g_pSO3World->m_nGameLoop + pSegmentInfo->nAttackIntervalFrame;
    }
    else
    {
        m_AttackObjList[pObj->m_dwID] = g_pSO3World->m_nGameLoop + pSkill->m_nAttackIntervalFrame;
    }

Exit0:
    return;
}

void KHero::AttackDummy(KSkill* pSkill, KDummy* pDummy)
{
    KATTACKED_OBJ_LIST::iterator itAttack;
    KSkillSegmentInfo* pSegmentInfo = NULL;

    assert(pSkill);
    assert(pDummy);

    itAttack = m_AttackObjList.find(pDummy->m_dwID);
    if (itAttack != m_AttackObjList.end())
        KG_PROCESS_ERROR(g_pSO3World->m_nGameLoop > itAttack->second);

    pDummy->BeAttacked();

    pSegmentInfo = pSkill->GetSegmentOfFrameBelongTo(m_nVirtualFrame - m_nAttackSkillTime);
    if (pSegmentInfo)
    {
        m_AttackObjList[pDummy->m_dwID] = g_pSO3World->m_nGameLoop + pSegmentInfo->nAttackIntervalFrame;
    }
    else
    {
        m_AttackObjList[pDummy->m_dwID] = g_pSO3World->m_nGameLoop + pSkill->m_nAttackIntervalFrame;
    }

Exit0:
    return;
}

void KHero::AttackBarrier(KSkill* pSkill, KBarrier* pBarrier)
{
    KATTACKED_OBJ_LIST::iterator itAttack;
    KSkillSegmentInfo* pSegmentInfo = NULL;

    assert(pSkill);
    assert(pBarrier);

    itAttack = m_AttackObjList.find(pBarrier->m_dwID);
    if (itAttack != m_AttackObjList.end())
        KG_PROCESS_ERROR(g_pSO3World->m_nGameLoop > itAttack->second);

    pBarrier->BeAttacked();

    pSegmentInfo = pSkill->GetSegmentOfFrameBelongTo(m_nVirtualFrame - m_nAttackSkillTime);
    if (pSegmentInfo)
    {
        m_AttackObjList[pBarrier->m_dwID] = g_pSO3World->m_nGameLoop + pSegmentInfo->nAttackIntervalFrame;
    }
    else
    {
        m_AttackObjList[pBarrier->m_dwID] = g_pSO3World->m_nGameLoop + pSkill->m_nAttackIntervalFrame;
    }

Exit0:
    return;
}

int KHero::GetAutoMoveDir(KAutoMoveDirection eType)
{
    KSceneObject* pObject = NULL;
    if (eType == amdSelfForward)
    {
        KTWO_DIRECTION eDir = GetFaceDir();
        return (eDir == csdRight) ? 0 : DIRECTION_COUNT / 2;
    }
    else if (eType == amdSelfBack)
    {
        KTWO_DIRECTION eDir = GetFaceDir();
        return (eDir == csdRight) ? DIRECTION_COUNT / 2 : 0;
    }
    else if (eType == amdAttackerForward)
    {
        KTWO_DIRECTION eDir = m_eBeAttackedToDir;
        return (eDir == csdRight) ? 0 : DIRECTION_COUNT / 2;
    }
    else if (eType == amdAttackerBack)
    {
        KTWO_DIRECTION eDir = m_eBeAttackedToDir;
        return (eDir == csdRight) ? DIRECTION_COUNT / 2 : 0;
    }
    else if (eType == amdAttackDirectionForward)
    {
        pObject = GetAttacker();
        KGLOG_PROCESS_ERROR(pObject);

        KPOSITION cPosAttacker = pObject->GetPosition();
        KPOSITION cPosBeAttacked = GetPosition();

        int nDir = g_GetDirection(cPosAttacker.nX, cPosAttacker.nY, cPosBeAttacked.nX, cPosBeAttacked.nY);
        if (nDir == 0 && m_eBeAttackedToDir == csdLeft)
        {
            nDir = 128;
        }
        return nDir;    
    }
    else if (eType == amdAttackDirectionBack)
    {
        pObject = GetAttacker();
        KGLOG_PROCESS_ERROR(pObject);

        KPOSITION cPosAttacker = pObject->GetPosition();
        KPOSITION cPosBeAttacked = GetPosition();

        return g_GetDirection(cPosBeAttacked.nX, cPosBeAttacked.nY, cPosAttacker.nX, cPosAttacker.nY);
    }
    else if (eType == amdSelfForwardUp)
    {
        KTWO_DIRECTION eDir = GetFaceDir();
        return (eDir == csdRight) ? (DIRECTION_COUNT / 8) : (DIRECTION_COUNT * 3 / 8);
    }
    else if (eType == amdSelfForwardDown)
    {
        KTWO_DIRECTION eDir = GetFaceDir();
        return (eDir == csdRight) ? (DIRECTION_COUNT * 7 / 8) : (DIRECTION_COUNT * 5 / 8);
    }

Exit0:
    return 0;
}

void KHero::ApplyAttribute(KAttribute* pAttr)
{
    while (pAttr)
    {
        CallAttributeFunction(pAttr->nKey, true, pAttr->nValue1, pAttr->nValue2);

        pAttr = pAttr->pNext;
    }
}

void KHero::UnApplyAttribute(KAttribute* pAttr)
{
    while (pAttr)
    {
        CallAttributeFunction(pAttr->nKey, false, pAttr->nValue1, pAttr->nValue2);

        pAttr = pAttr->pNext;
    }
}

BOOL KHero::CallAttributeFunction(int nKey, BOOL bApply, int nValue1, int nValue2)
{
    BOOL                bResult = false;
    ATTR_PROCESS_FUN    PFunc   = NULL;

    if (nKey <= atInvalid || nKey >= atTotal)
    {
        KGLogPrintf(KGLOG_ERR, "Unexpected attribute key %d !\n", nKey);
        goto Exit0;
    }

    PFunc = g_AttributeProcessFunctions[nKey];
    KG_PROCESS_ERROR(PFunc);

    PFunc(this, bApply, nValue1, nValue2);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::TakeObject(DWORD dwID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KDoodad *pDoodad = NULL;

    KG_PROCESS_ERROR(m_pScene);

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    pDoodad = m_pScene->GetDoodadById(dwID);
    KGLOG_PROCESS_ERROR(pDoodad);

    bRetCode = CanTakeDoodad();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = TakeDoodad(pDoodad, true);
    KG_PROCESS_ERROR(bRetCode);

    LogInfo("Ojb:%d take by player:%d", pDoodad->m_dwID, m_dwID);

    bResult = true;
Exit0:
    return bResult;
}

DWORD KHero::GetHoldingObjID()
{
    DWORD dwRetObjID = ERROR_ID;

    KMovableObject* pObject = GetFirstFollowerWithState(mosToken);
    if (pObject)
    {
        dwRetObjID = pObject->m_dwID;
    }

    return dwRetObjID;
}

BOOL KHero::CanTakeDoodad() const
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(!m_pCastingSkill);

    bRetCode = IsForbitPick();
    KG_PROCESS_ERROR(!bRetCode);

    bRetCode = HasFollower();
    KG_PROCESS_ERROR(!bRetCode);

    KG_PROCESS_ERROR(
        m_eMoveState == cmsOnStand || m_eMoveState == cmsOnWalk || m_eMoveState == cmsOnRun || m_eMoveState == cmsOnJump || m_eMoveState == cmsOnSit
        );

    KG_PROCESS_ERROR(!m_bAttackState); // 攻击状态不能拿物件

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::DropObject(DWORD dwID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KDoodad* pDoodad = NULL;

    KG_PROCESS_ERROR(m_pScene);

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    pDoodad = m_pScene->GetDoodadById(dwID);
    KGLOG_PROCESS_ERROR(pDoodad);

    bRetCode = DropDoodad(pDoodad, true);
    KG_PROCESS_ERROR(bRetCode);

    LogInfo("Ojb:%d untake by player:%d", pDoodad->m_dwID, m_dwID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::DetachObject(DWORD dwID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KDoodad* pDoodad = NULL;

    KG_PROCESS_ERROR(m_pScene);

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    pDoodad = m_pScene->GetDoodadById(dwID);
    KGLOG_PROCESS_ERROR(pDoodad);

    bRetCode = pDoodad->BeDetach(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoDetachObject(this, pDoodad);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::CanDropDoodad() const
{
    return true;
}

BOOL KHero::CanThrowDoodad() const
{
    return true;
}

BOOL KHero::HasAttachedByDoodad(DWORD dwID)
{
    BOOL bResult            = false;
    BOOL bRetCode           = false;
    KDoodad* pObject        = NULL;

    KG_PROCESS_ERROR(m_pScene);

    KG_PROCESS_ERROR(dwID != ERROR_ID);

    pObject = m_pScene->GetDoodadById(dwID);
    KGLOG_PROCESS_ERROR(pObject);

    bRetCode = pObject->IsAttachedTo(this);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::HasSkill(DWORD dwSkillID)
{
    BOOL        bResult     = false;
    BOOL		bRetCode	= false;

    for ( int i = 0; i < countof(m_ActiveSkill); ++i)
    {
        if (m_ActiveSkill[i] == dwSkillID)
        {
            bResult = true;
            goto Exit1;
        }
    }

    for ( int i = 0; i < countof(m_NirvanaSkill); ++i)
    {
        if (m_NirvanaSkill[i] == dwSkillID)
        {
            bResult = true;
            goto Exit1;
        }
    }

    bRetCode = HasOtherActiveSkill(dwSkillID);
    KG_PROCESS_ERROR(bRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::HasSlamBallSkill(DWORD dwSkillID)
{
    BOOL        bResult     = false;

    for ( int i = 0; i < countof(m_SlamBallSkill); ++i)
    {
        if (m_SlamBallSkill[i] == dwSkillID)
        {
            bResult = true;
            break;
        }
    }

    return bResult;
}

BOOL KHero::HasOtherActiveSkill(DWORD dwSkillID)
{
    BOOL bResult = false;
    for (KVEC_DWORD::iterator it = m_vecAllOtherActiveSkill.begin(); it != m_vecAllOtherActiveSkill.end(); ++it)
    {
        if (*it == dwSkillID)
        {
            bResult = true;
            break;
        }
    }

    return bResult;
}

BOOL KHero::EnsureUntakeBall()
{
    BOOL bResult = false;
    KBall* pBall = NULL;
    BOOL bWithBall = HasFollowerWithStateAndType(mosToken, sotBall);

    KG_PROCESS_SUCCESS(!bWithBall);

    pBall = m_pScene->GetBall();
    KG_PROCESS_ERROR(pBall);

    pBall->BeUnTake(this);
    // 广播球被释放
    g_PlayerServer.DoUnTakeBall(this);

    m_pScene->OnBallDropped(this);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::EnsureNoAttached()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KG_PROCESS_SUCCESS(m_dwAttachedDoodadID == ERROR_ID);

    bRetCode = DetachObject(m_dwAttachedDoodadID);
    KGLOG_CHECK_ERROR(bRetCode);

    m_dwAttachedDoodadID = ERROR_ID;
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::AddBuff(DWORD dwBuffID)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KBuff*  pBuff       = NULL;
    std::vector<KHero*> vecAllTeammate;

    KG_PROCESS_ERROR(dwBuffID != ERROR_ID);
    KGLOG_PROCESS_ERROR(m_pScene);

    pBuff = g_pSO3World->m_BuffManager.GetBuff(dwBuffID);
    KGLOG_PROCESS_ERROR(pBuff);

    bRetCode = m_BuffList.AddBuff(dwBuffID);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (pBuff->m_bShare)//共享给友军
    {
        m_pScene->GetAllTeammate(this, vecAllTeammate); 
        for (size_t i = 0; i < vecAllTeammate.size(); ++i)
        {
            if (this == vecAllTeammate[i])
                continue;

            bRetCode = vecAllTeammate[i]->m_BuffList.AddBuff(dwBuffID);   
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::CanBeAttacked()
{
    BOOL bResult = false;

    if (m_eMoveState == cmsOnKnockedOff)
    {
        //KG_PROCESS_ERROR(m_nAttackedCountOnKnockOff < 2);
        KG_PROCESS_ERROR(m_nCurrentEndurance > 0);
    }

    KG_PROCESS_ERROR(m_eMoveState != cmsOnKnockedDown);

    KG_PROCESS_ERROR(!m_bInvincible);

    KG_PROCESS_ERROR(g_pSO3World->m_nGameLoop >= m_nProtectEndFrame);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::EnsureNoFollows()
{
    ClearFollower();
    return true;
}

BOOL KHero::EnsureNoGuide()
{
    DWORD dwGuideID = GetGuideID();

    if (dwGuideID != ERROR_ID)
    {
        RemoveGuide();
        TurnToMoveType(mosFree);
        Stand(false);
        g_PlayerServer.DoRemoveGuide(this);
    }

    return true;
}


void KHero::CheckObjStandOn(const KPOSITION& rPos)
{
    BOOL        bRetCode = false;
    KDoodad*    pChair = NULL;
    KBODY       cBody;

    KGLOG_CHECK_ERROR(m_pScene);

    KG_PROCESS_ERROR(m_eMoveState != cmsOnHalt && m_eMoveState != cmsOnKnockedOff && m_eMoveState != cmsOnKnockedDown);

    bRetCode = IsStandOnObj();
    if (bRetCode)
    {
        KMovableObject* pObj = GetGuide();
        KGLOG_PROCESS_ERROR(pObj);
        SetPosition(pObj->GetTopCenter());
        goto Exit0;
    }

    cBody = GetBody();
    KG_PROCESS_ERROR(cBody.nZ < rPos.nZ);

    for(KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!g_IsDoodad(pObj))
            continue;

        KDoodad* pDoodad = (KDoodad*)pObj;
        if (!pDoodad->CanBeStandOnBy(this))
            continue;

        KBODY cDoodadBody = pDoodad->GetBody();
        int nTop = cDoodadBody.nZ + cDoodadBody.nHeight;

        if (nTop > rPos.nZ || nTop < cBody.nZ)
            continue;

        if (g_IsCollide(cBody, cDoodadBody))
        {
            pChair = pDoodad;
            break;
        }
    }

    KG_PROCESS_ERROR(pChair);

    bRetCode = pChair->BeStandOn(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_nJumpCount = 0;

    if (m_eMoveState == cmsOnJump || m_eMoveState == cmsOnWalk || m_eMoveState == cmsOnRun || m_eMoveState == cmsOnKnockedOff)
    {
        SetMoveState(cmsOnStand);
    }

    m_mapCastSkillCountInAir.clear();

    g_PlayerServer.DoHeroStandOn(this, pChair);

Exit0:
    return;
}

BOOL KHero::SlamBall(KBall* pBall, KBasketSocket* pBasketSocket)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    assert(pBall && pBasketSocket);

    bRetCode = pBall->CanSlam(this, pBasketSocket);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pBall->SlamDunk(this, pBasketSocket);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

KRelationType KHero::GetRelationTypeWith(KHero* pOtherOne) const
{
    KRelationType eResult = rltNone;

    if (m_dwID == pOtherOne->m_dwID)
    {
        eResult = rltMe;
    }
    else if (m_nSide == pOtherOne->m_nSide)
    {
        eResult = rltUs;
    }
    else
    {
        eResult = rltEnemy;
    }

Exit0:
    return eResult;
}

KSceneObject* KHero::GetAttacker()
{
    assert(m_pScene);

    return m_pScene->GetSceneObjByID(m_dwAttackerID);
}

BOOL KHero::ShootDoodad(DWORD dwID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    BOOL bCanHit = false;
    KDoodad* pDoodad = NULL;
    KPlayer* pPlayer = NULL;

    KGLOG_PROCESS_ERROR(m_pScene);

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    pDoodad = m_pScene->GetDoodadById(dwID);
    KGLOG_PROCESS_ERROR(pDoodad);

    bRetCode = ShootDoodad(pDoodad, true);
    KG_PROCESS_ERROR(bRetCode);

    pPlayer = GetOwner();
    KGLOG_PROCESS_ERROR(pPlayer);

    if (pDoodad->GetType() == sotLadder)
        pPlayer->OnEvent(peUseLadder);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KHero::CanShootDoodad() const
{
    return true;
}

BOOL KHero::RandomHitted(int nNowHitRate) const
{
    int nRandomValue = GetRandomNum(0, 100);
    return (nRandomValue < nNowHitRate);
}

KBasketSocket* KHero::GetShootSocket()
{
    KBasketSocket* pSocket = NULL;
    KTWO_DIRECTION eDir = csdInvalid;

    eDir = GetFaceDir();

    pSocket = m_pScene->GetShootObjectSocket(eDir, GetPosition().nZ);

    return pSocket;
}

void KHero::LoseEndurance(int nEndurance)
{
    if (m_bBaTi && nEndurance > 0)
        return;

    int nEnduranceBeforeLose = m_nCurrentEndurance;

    m_nCurrentEndurance -= nEndurance;
    if (m_nCurrentEndurance <= 0)
    {
        Halt();
    }
    MAKE_IN_RANGE(m_nCurrentEndurance, 0, m_nMaxEndurance);

    if (nEnduranceBeforeLose > 0 && m_nCurrentEndurance <= 0)
    {
        OnEnduranceDownToZero();
    }
}

void KHero::OnEnduranceDownToZero()
{
    BOOL bRetCode = false;
    int nSide = sidLeft;
	KMission* pMission = NULL;

    KG_PROCESS_ERROR(m_nCurrentEndurance == 0);
    KG_PROCESS_ERROR(m_pNpcTemplate);

    if (m_nSide == sidLeft)
        nSide = sidRight;

	pMission = g_pSO3World->m_MissionMgr.GetMission(m_pScene->m_dwMapID);
	if (pMission)
		m_pScene->BroadcastEvent(nSide, peFightDownNpc, m_pNpcTemplate->nEventLabel, pMission->eType, pMission->nStep, pMission->nLevel);

Exit0:
    return;
}

void KHero::onStaminaDownToZero()
{
	BOOL bRetCode = false;
	int nSide = sidLeft;
	KMission* pMission = NULL;

	KG_PROCESS_ERROR(m_nCurrentStamina == 0);
	KG_PROCESS_ERROR(m_pNpcTemplate);

	if (m_nSide == sidLeft)
		nSide = sidRight;

	pMission = g_pSO3World->m_MissionMgr.GetMission(m_pScene->m_dwMapID);
	if (pMission)
		m_pScene->BroadcastEvent(nSide, peNpcStaminaEmpty, m_pNpcTemplate->nEventLabel, pMission->eType, pMission->nStep, pMission->nLevel);

Exit0:
	return;
}

void KHero::LoseStamina(int nStamina)
{
    BOOL bRetCode = false;
    KBuff* pBuff = NULL;

    m_nCurrentStamina -= nStamina;
    MakeInRange<int>(m_nCurrentStamina, 0, m_nMaxStamina);
    if (m_nCurrentStamina == 0)
    {
        KG_PROCESS_SUCCESS(m_bExhausted);   
        m_bExhausted = true;

        bRetCode = m_BuffList.HasBuff(EXHAUSTEDEX_BUFF_ID);
        if (bRetCode)
        {
            bRetCode = m_BuffList.DelBuff(EXHAUSTEDEX_BUFF_ID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        pBuff = g_pSO3World->m_BuffManager.GetBuff(EXHAUSTED_BUFF_ID);
        KGLOG_PROCESS_ERROR(pBuff);

        bRetCode = AddBuff(pBuff->m_dwID);
        KGLOG_PROCESS_ERROR(bRetCode);

		onStaminaDownToZero();
    }
    else
    {
        KG_PROCESS_SUCCESS(!m_bExhausted);
        m_bExhausted = false;

        bRetCode = m_BuffList.DelBuff(EXHAUSTED_BUFF_ID);
        KGLOG_PROCESS_ERROR(bRetCode);

        pBuff = g_pSO3World->m_BuffManager.GetBuff(EXHAUSTEDEX_BUFF_ID);
        KGLOG_PROCESS_ERROR(pBuff);

        bRetCode = AddBuff(pBuff->m_dwID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

Exit1:
Exit0:
    return;
}


void KHero::ApplyCannotSkill()
{
    m_bForbitSkill= true;
}

void KHero::UnApplyCannotSkill()
{
    m_bForbitSkill = false;
}


BOOL KHero::IsForbitCastSkill() const
{
    return m_bForbitSkill;
}

void KHero::ApplyCannotPick()
{
    m_bForbitPick = true;
}

void KHero::UnApplyCannotPick()
{
    m_bForbitPick = false;
}

BOOL KHero::IsForbitPick() const
{
    return m_bForbitPick;
}

BOOL KHero::IsAlive() const
{
    return m_nCurrentEndurance > 0;
}

void KHero::CalcHitRateParams(const KPOSITION& cSrc, const KPOSITION& cDst, int& nNowAttenuationTimes, int& nNowHitRate, int nInterference) const
{
    double dDistance = sqrt(g_GetDistance2<double>(cSrc.nX, cSrc.nY, cDst.nX, cDst.nY));
    nNowAttenuationTimes = int(dDistance / m_nHitRateAttenuation);
    nNowHitRate = m_nHitRate - nNowAttenuationTimes;

    if (nInterference > 0)
    {
        if (nInterference > m_nWillPower)
            nNowHitRate -= (nInterference - m_nWillPower) / 10;
    }

    if (nNowHitRate < 0)
        nNowHitRate = 0;
}

KSceneObjectType KHero::GetType() const
{
    return sotHero;
}

KPlayer* KHero::GetOwner() const
{
    return m_pOwner;
}

int KHero::GetConnIndex() const
{
    KPlayer* pPlayer = GetOwner();
    if (pPlayer)
        return pPlayer->m_nConnIndex;
    return -1;
}

void KHero::ApplyMoveAction(BYTE byMoveState, BYTE byDirection)
{
    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    switch (byMoveState)
    {
    case cmsOnStand:
        Stand(false);
        break;
    case cmsOnRun:
        RunTo(byDirection, false);
        break;
    case cmsOnWalk:
        WalkTo(byDirection, false);
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

Exit0:
    return;
}

BOOL KHero::CheckMoveAdjust(int nClientFrame, const KMove_Critical_Param& crParam, BOOL& rbReverseFailed)
{
    BOOL bResult    = true;
    BOOL bRetCode   = false;
    KPOSITION cPos;

    bRetCode = AdjustVirtualFrame(nClientFrame, rbReverseFailed);
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLOG_PROCESS_ERROR(m_eMoveState == crParam.byMoveState);

    KGLOG_PROCESS_ERROR(m_eFaceDir == (KTWO_DIRECTION)crParam.byFacedir);

    cPos = GetPosition();
    KGLOG_PROCESS_ERROR(cPos.nX == crParam.nX);
    KGLOG_PROCESS_ERROR(cPos.nY == crParam.nY);
    KGLOG_PROCESS_ERROR(cPos.nZ == crParam.nZ);

Exit1:
    bResult = false; // not to adjust
Exit0:
    if (bResult)
    {
        cPos = GetPosition();
        KGLogPrintf(KGLOG_DEBUG, "CF:%d, (Server,Client),MoveState(%d,%d), FaceDir(%d,%d), Pos(%d,%d,%d %d,%d,%d).\n", nClientFrame, m_eMoveState, crParam.byMoveState,
            m_eFaceDir, crParam.byFacedir, cPos.nX, cPos.nY, cPos.nZ, crParam.nX, crParam.nY, crParam.nZ);
    }
    return bResult;
}

BOOL KHero::AdjustVirtualFrame(int nClientFrame, BOOL& rbReverseFailed)
{
    BOOL bResult        = false;
    BOOL bRetCode       = false;

    // 如果延迟是稳定的，则m_nVirtualFrame等于nClientFrame
    if (m_nVirtualFrame > nClientFrame)
    {
        bRetCode = ReverseFrame(m_nVirtualFrame - nClientFrame);
        if (!bRetCode)
        {
            rbReverseFailed = true;
            KGLogPrintf(KGLOG_INFO, "%d ReverseFrame failed!", m_dwID);
            goto Exit0;
        }
    }
    else if (m_nVirtualFrame < nClientFrame)
    {
        int nDeltaFrame = nClientFrame - m_nVirtualFrame;
        //KGLOG_PROCESS_ERROR(nDeltaFrame < GAME_FPS * 5); // 防止外挂

        bRetCode = ForwardFrame(nDeltaFrame);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true; 
Exit0:
    return bResult;
}

void KHero::SetRunMode(bool bIsRunMode)
{
    m_bIsRunMode = bIsRunMode;
}

void KHero::EnsureNotStandOnObject()
{
    if (IsStandOnObj())
    {
        RemoveGuide();
        TurnToMoveType(mosFree);
    }
}

BOOL KHero::ReverseFrame(int nFrame)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    int                 nRecordIndex    = 0;

    assert(nFrame > 0);
    KGLOG_PROCESS_ERROR(nFrame <= m_cRecordedStatus.GetRecordedCount());

    nRecordIndex = (m_nVirtualFrame - nFrame) % m_cRecordedStatus.GetMaxRecordCount();

    { // linux去警告
        const KPOSITION_RECORD& rRecord = m_cRecordedStatus.GetRecord(nRecordIndex);
        RestoreStatus(rRecord);
    }

    m_nVirtualFrame -= nFrame;

    //KGLogPrintf(KGLOG_INFO, "hero %d reverse %d frame to %d", m_dwID, nFrame, m_nVirtualFrame);

    bResult = true;
Exit0:
    m_cRecordedStatus.ClearRecord();
    return bResult;
}

BOOL KHero::ForwardFrame(int nFrame)
{
    BOOL bResult = false;
    int nForwardFrame = nFrame;

    while (nFrame-- > 0)
    {
        KPOSITION cPos = GetPosition();
        ++m_nVirtualFrame;

        KMovableObject::Activate(g_pSO3World->m_nGameLoop);

        CheckObjStandOn(cPos);
        RunStateMachine();
        DoDelayAction();

        SpecialActivate(m_nGameLoop);
    }

    //KGLogPrintf(KGLOG_INFO, "hero %d forward %d frame to %d", m_dwID, nFrame, m_nVirtualFrame);

    bResult = true;
Exit0:
    m_cRecordedStatus.ClearRecord();
    return bResult;
}

void KHero::RecordStatus()
{
    KPOSITION cPos = GetPosition();
    KPOSITION_RECORD cCurItem = 
    {
        m_eMoveState,
        m_eLastMoveState,
        cPos.nX,
        cPos.nY,
        cPos.nZ,
        m_nVelocityX,
        m_nVelocityY,
        m_nVelocityZ,
        m_eFaceDir,
        m_nJumpCount,
        m_nMoveFrameCounter,
    };

    m_cRecordedStatus.Record(m_nVirtualFrame, cCurItem);
}

void KHero::RestoreStatus(const KPOSITION_RECORD& rRecord)
{
    SetPosition(rRecord.nX, rRecord.nY, rRecord.nZ);
    m_eMoveState          =   rRecord.eMoveState;
    m_eLastMoveState      =   rRecord.eLastMoveState;
    m_nVelocityX          =   rRecord.nVelocityX;
    m_nVelocityY          =   rRecord.nVelocityY;
    m_nVelocityZ          =   rRecord.nVelocityZ;
    m_eFaceDir            =   (KTWO_DIRECTION)rRecord.nFaceDir;
    m_nVelocityZ          =   rRecord.nVelocityZ;
    m_nJumpCount          =	  rRecord.nJumpCount;   
    m_nMoveFrameCounter   =   rRecord.nMoveFrameCount;
}

void KHero::AttachAsFightingHero(KPlayer* pOwner)
{
    assert(pOwner);
    m_pOwner = pOwner;
    m_pOwner->m_pFightingHero = this;
}


void KHero::AttachAsAITeammate(KPlayer* pOwner)
{
    assert(pOwner);
    m_pOwner = pOwner;
    m_pOwner->m_pAITeammate = this;
}


int KHero::GetRandomNum(int nMin, int nMax) const
{
    assert(nMin <= nMax);

    int nRandomNum = 0;
    int nNums = nMax - nMin;
    int nTempRandomSeed = 0;

    if (nNums)
    {
        nTempRandomSeed = g_GetRandomSeed();

        g_RandomSeed(m_nVirtualFrame);
        nRandomNum  = g_Random(nNums);
        g_RandomSeed(nTempRandomSeed);
    }

    nRandomNum += nMin;

    return nRandomNum;
}

void KHero::BeThrownChest(KChest* pObject)
{
    BOOL bRetCode = false;
    KHero* pThrower = NULL;

    KG_PROCESS_ERROR(pObject);
    KG_PROCESS_ERROR(pObject->m_dwThrowerID != ERROR_ID);

    pThrower = m_pScene->GetHeroById(pObject->m_dwThrowerID);
    KGLOG_PROCESS_ERROR(pThrower);

    KG_PROCESS_ERROR(m_nSide != pThrower->m_nSide);

    bRetCode = CanBeAttached(pObject);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = ApplyGeneralAffect(pThrower->m_dwID, pObject, pObject->m_pTemplate->m_dwAttackAffectID1);
    KGLOG_PROCESS_ERROR(bRetCode);

    pObject->m_dwThrowerID = ERROR_ID;
    pObject->BeAttachTo(this);

    m_dwAttachedDoodadID = pObject->m_dwID;

    m_cRecordedStatus.ClearRecord();

    g_PlayerServer.DoAttachObject(this, pObject);

Exit0:
    return;
}

BOOL KHero::CanBeAttached(KChest* pObject)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KMoveType eMoveType = mosInvalid;

    eMoveType = pObject->GetMoveType();
    KG_PROCESS_ERROR(eMoveType == mosFree);

    bRetCode = CanBeAttacked();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = IsFollowedBy(pObject);
    KG_PROCESS_ERROR(!bRetCode);

    bResult = TRUE;
Exit0:
    return bResult;
}

void KHero::ProcessAI()
{
    KG_PROCESS_ERROR(m_bAiMode);

    m_AIVM.Active();

Exit0:
    return;
}

void KHero::Detach()
{
    BOOL bRetCode = false;
    KPlayer* pOwner = m_pOwner;

    KG_PROCESS_ERROR(pOwner);

    m_pOwner = NULL;

    if (pOwner->m_pFightingHero == this)
        pOwner->m_pFightingHero = NULL;
    else if(pOwner->m_pAITeammate == this)
        pOwner->m_pAITeammate = NULL;

    bRetCode = pOwner->m_HeroDataList.SaveHeroData(this);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KHero::ProcessRebounds()
{
    KG_PROCESS_SUCCESS(!m_bXYRebound);

    m_mapCastSkillCountInAir.clear();

    m_nJumpCount = 0;
    if (m_eMoveState == cmsOnJump)
    {        
        Sit();
    }
    else if (m_eMoveState == cmsOnAimat)
    {
        m_nVelocityX = 0;
        m_nVelocityY = 0;
        m_nVelocityZ = 0;
    }

Exit1:
    m_bXYRebound = false;
    m_bXZRebound = false;
    m_bYZRebound = false;
    return;
}

void KHero::AfterProcessMoveZ()
{
    BOOL bRetCode = false;
    int nGravity = GetCurrentGravity();

    bRetCode = IsInAir();
    KG_PROCESS_ERROR(bRetCode);

    if (m_pCastingSkill)
    {
        nGravity = nGravity * m_pCastingSkill->m_nGravityPercent / HUNDRED_NUM;
    }

    switch(m_eMoveState)
    {
    case cmsOnStand:
        SetMoveState(cmsOnJump);
        m_nJumpCount++;
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnWalk:
        SetMoveState(cmsOnJump);
        m_nJumpCount++;
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnRun:
        SetMoveState(cmsOnJump);  
        m_nJumpCount++;     
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnJump:      
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnSit:   
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnAimat:
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnKnockedDown:
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnKnockedOff:     
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnHalt:
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnFreeze:
        m_nVelocityZ -= nGravity;
        break;
    case cmsOnLoseControlByCounter:
        break;
    default:
        break;
    }

Exit0:
    return;
}

void KHero::UpdateVelocity(int nSpeed, int nDirection)
{
    m_nVelocityX = nSpeed * g_Cos(nDirection) / SIN_COS_NUMBER;
    m_nVelocityY = nSpeed * g_Sin(nDirection) / SIN_COS_NUMBER;
    m_nVelocityDir = nDirection;
}

KTWO_DIRECTION KHero::GetAttackingDir() const
{
    return GetFaceDir();
}

void KHero::OnTrackMove(int nCurLoop)
{
    KMovableObject::OnTrackMove(m_nVirtualFrame);
}

void KHero::OnTrackMoveFinished()
{
    BOOL        bRetCode = false;
    KBall*      pBall = NULL;
    BOOL        bHit = false;
    KBasket*    pBasket = NULL;
    KSlamBallSkill* pSlamBallSkill = NULL;
    KBasketSocket* pSocket = NULL;

    KMovableObject::OnTrackMoveFinished();

    KG_PROCESS_ERROR(m_bSkillSlaming);

    pSlamBallSkill  = m_pCastingSlamBallSkill;
    bHit            = m_bSkillSlamingHit;

    m_bSkillSlaming = false;
    m_bSkillSlamingHit = false;
    m_pCastingSlamBallSkill = NULL;
    m_bBaTi = false;

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    bRetCode = pBall->IsTokenBy(this);
    KG_PROCESS_ERROR(bRetCode);

    pSocket = pBall->m_pTargetSocket;
    KG_PROCESS_ERROR(pSocket);

    pBasket = pSocket->m_pBasket;
    KGLOG_PROCESS_ERROR(pBasket);

    bRetCode = SlamBall(pBall, pBall->m_pTargetSocket);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bHit)
    {
        m_nSlamDunkForce = pSlamBallSkill->nCostBasketHP;

        m_pScene->OnBallShot(this, stSlamDunk);
    }
    else
    {
        pBall->m_DelayAction.nFrame = g_pSO3World->m_nGameLoop + g_pSO3World->m_Settings.m_ConstList.nSlamBallReboundFrame;
    }
Exit0:
    return;
}

void KHero::OnTrackMoveInterrupted()
{
    KG_PROCESS_ERROR(m_bSkillSlaming);

    m_bSkillSlaming = false;
    m_bBaTi = false;
    m_pCastingSlamBallSkill = NULL;
    m_bSkillSlamingHit = false;

    ClearVelocity();

Exit0:
    return;
}

void KHero::OnAttacked(BOOL bIgnoreBaTi/*=false*/)
{
    BOOL bRetCode = false;

    if (m_bBaTi && !bIgnoreBaTi)
        goto Exit0;

    bRetCode = EnsureNotCastingSkill();
    KGLOG_CHECK_ERROR(bRetCode);

    bRetCode = EnsureUntakeBall();
    KGLOG_CHECK_ERROR(bRetCode);

    bRetCode = EnsureNoAttached();
    KGLOG_CHECK_ERROR(bRetCode);

    bRetCode = EnsureNoFollows();
    KGLOG_CHECK_ERROR(bRetCode);

    bRetCode = EnsureNoGuide();
    KGLOG_CHECK_ERROR(bRetCode);

    //触发被动技能
    TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_ATTACK);
    OnHpDecrease();

    m_pPreSkill = NULL;
    m_nPreSkillCastTime = 0;

Exit0:
    return;
}

BOOL KHero::UseDoodad(DWORD dwID, unsigned char direction)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KDoodad*        pDoodad     = NULL;
    KTWO_DIRECTION  eDir        = csdInvalid;
    KPlayer*        pPlayer     = NULL;

    pDoodad = m_pScene->GetDoodadById(dwID);
    KGLOG_PROCESS_ERROR(pDoodad);

    bRetCode = UseDoodad(pDoodad, direction, true);
    KG_PROCESS_ERROR(bRetCode);

    if (IsMainHero())
    {
        pPlayer = GetOwner();
        assert(pPlayer);

        if (pDoodad->GetType() == sotTyre)
            pPlayer->OnEvent(peUseTire);
    }

    LogInfo("Doodad %d Used By Hero %d\n", dwID, m_dwID);

    m_GameStat.m_nUseSceneDoodad += 1;

    bResult = true;
Exit0:
    return bResult;
}

void KHero::SetLevel(int nNewLevel)
{
    KHeroLevelInfo* pLevelUpData = NULL;

    KGLOG_PROCESS_ERROR(nNewLevel > 0);

    if (m_nLevel > 0)
    {
        pLevelUpData = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroLevelInfo(m_dwTemplateID, m_nLevel);   
        KGLOG_PROCESS_ERROR(pLevelUpData);

        CallAttributeFunction(atMaxEndurance, false, pLevelUpData->nMaxEndurance, 0);
        CallAttributeFunction(atEnduranceReplenish, false, pLevelUpData->nEnduranceReplenish, 0);
        CallAttributeFunction(atEnduranceReplenishPercent, false, pLevelUpData->nEndurancereplenishPercent, 0);
        CallAttributeFunction(atMaxStamina, false, pLevelUpData->nMaxStamina, 0);
        CallAttributeFunction(atStaminaReplenish, false, pLevelUpData->nStaminaReplenish, 0);
        CallAttributeFunction(atStaminaReplenishPercent, false, pLevelUpData->nStaminaReplenishPercent, 0);
        CallAttributeFunction(atMaxAngry, false, pLevelUpData->nMaxAngry, 0);
        CallAttributeFunction(atWillPower, false, pLevelUpData->nWillPower, 0);
        CallAttributeFunction(atAdditionalRecoveryAngry, false, pLevelUpData->nAdditionalAddAngry, 0);
        CallAttributeFunction(atInterference, false, pLevelUpData->nInterference, 0);
        CallAttributeFunction(atInterferenceRange, false, pLevelUpData->nInterferenceRange, 0);
        CallAttributeFunction(atAttackPoint, false, pLevelUpData->nAttackPoint, 0);
        CallAttributeFunction(atAttackPointPercent, false, pLevelUpData->nAttackPointPercent, 0);
        CallAttributeFunction(atAgility, false, pLevelUpData->nAgility, 0);
        CallAttributeFunction(atCritPoint, false, pLevelUpData->nCritPoint, 0);
        CallAttributeFunction(atCritRate, false, pLevelUpData->nCritRate, 0);
        CallAttributeFunction(atDefense, false, pLevelUpData->nDefense, 0);
        CallAttributeFunction(atDefensePercent, false, pLevelUpData->nDefensePercent, 0);
        CallAttributeFunction(atRunSpeedBase, false, pLevelUpData->nMoveSpeed, 0);
        CallAttributeFunction(atJumpSpeedBase, false, pLevelUpData->nJumpingAbility, 0);
        CallAttributeFunction(atJumpSpeedPercent, false, pLevelUpData->nJumpingAbilityPercent, 0);
        CallAttributeFunction(atShootBallHitRate, false, pLevelUpData->nShootBallHitRate, 0);
        CallAttributeFunction(atNomalShootForce, false, pLevelUpData->nNormalShootForce, 0);
        CallAttributeFunction(atSkillShootForce, false, pLevelUpData->nSkillShootForce, 0);
        CallAttributeFunction(atSlamDunkForce, false, pLevelUpData->nSlamDunkForce, 0);
        CallAttributeFunction(atAddInterferenceRangePercent, false, pLevelUpData->nAddInterferenceRangePercent, 0);
        CallAttributeFunction(atAddMoveSpeedPercent, false, pLevelUpData->nAddMoveSpeedPercent, 0);
    }

    pLevelUpData = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroLevelInfo(m_dwTemplateID, nNewLevel);
    KGLOG_PROCESS_ERROR(pLevelUpData);

    CallAttributeFunction(atMaxEndurance, true, pLevelUpData->nMaxEndurance, 0);
    CallAttributeFunction(atEnduranceReplenish, true, pLevelUpData->nEnduranceReplenish, 0);
    CallAttributeFunction(atEnduranceReplenishPercent, true, pLevelUpData->nEndurancereplenishPercent, 0);
    CallAttributeFunction(atMaxStamina, true, pLevelUpData->nMaxStamina, 0);
    CallAttributeFunction(atStaminaReplenish, true, pLevelUpData->nStaminaReplenish, 0);
    CallAttributeFunction(atStaminaReplenishPercent, true, pLevelUpData->nStaminaReplenishPercent, 0);
    CallAttributeFunction(atMaxAngry, true, pLevelUpData->nMaxAngry, 0);
    CallAttributeFunction(atAdditionalRecoveryAngry, true, pLevelUpData->nAdditionalAddAngry, 0);
    CallAttributeFunction(atWillPower, true, pLevelUpData->nWillPower, 0);
    CallAttributeFunction(atInterference, true, pLevelUpData->nInterference, 0);
    CallAttributeFunction(atInterferenceRange, true, pLevelUpData->nInterferenceRange, 0);
    CallAttributeFunction(atAttackPoint, true, pLevelUpData->nAttackPoint, 0);
    CallAttributeFunction(atAttackPointPercent, true, pLevelUpData->nAttackPointPercent, 0);
    CallAttributeFunction(atAgility, true, pLevelUpData->nAgility, 0);
    CallAttributeFunction(atCritPoint, true, pLevelUpData->nCritPoint, 0);
    CallAttributeFunction(atCritRate, true, pLevelUpData->nCritRate, 0);
    CallAttributeFunction(atDefense, true, pLevelUpData->nDefense, 0);
    CallAttributeFunction(atDefensePercent, true, pLevelUpData->nDefensePercent, 0);
    CallAttributeFunction(atRunSpeedBase, true, pLevelUpData->nMoveSpeed, 0);
    CallAttributeFunction(atJumpSpeedBase, true, pLevelUpData->nJumpingAbility, 0);
    CallAttributeFunction(atJumpSpeedPercent, true, pLevelUpData->nJumpingAbilityPercent, 0);
    CallAttributeFunction(atShootBallHitRate, true, pLevelUpData->nShootBallHitRate, 0);
    CallAttributeFunction(atNomalShootForce, true, pLevelUpData->nNormalShootForce, 0);
    CallAttributeFunction(atSkillShootForce, true, pLevelUpData->nSkillShootForce, 0);
    CallAttributeFunction(atSlamDunkForce, true, pLevelUpData->nSlamDunkForce, 0);
    CallAttributeFunction(atAddInterferenceRangePercent, true, pLevelUpData->nAddInterferenceRangePercent, 0);
    CallAttributeFunction(atAddMoveSpeedPercent, true, pLevelUpData->nAddMoveSpeedPercent, 0);

    m_nCurrentEndurance     = m_nMaxEndurance;
    m_nCurrentStamina       = m_nMaxStamina;
    m_nCurrentAngry         = 0;

    m_nLevel = nNewLevel;

    m_nLength   = pLevelUpData->nLenth;
    m_nWidth    = pLevelUpData->nWidth;
    m_nHeight   = pLevelUpData->nHeight;

    m_AIData.nAIType = pLevelUpData->nAIType;

Exit0:
    return;
}

void KHero::DoCycleRevise()
{
    int nEnduranceReplenish = 0;
    int nStaminaReplenish   = 0;

    if (g_pSO3World->m_nGameLoop < m_nNextReplenishFrame)
        return;

    if (m_eMoveState != cmsOnKnockedOff) // 击飞的时候不回复
    {
        nEnduranceReplenish += m_nEnduranceReplenish;
        nEnduranceReplenish += m_nMaxEndurance * m_nEnduranceReplenishPercent / HUNDRED_NUM;
        m_nCurrentEndurance += nEnduranceReplenish;
        MAKE_IN_RANGE(m_nCurrentEndurance, 0, m_nMaxEndurance);
    }

    nStaminaReplenish += m_nStaminaReplenish;
    nStaminaReplenish += m_nMaxStamina * m_nStaminaReplenishPercent / HUNDRED_NUM;
    m_nCurrentStamina += nStaminaReplenish;
    MAKE_IN_RANGE(m_nCurrentStamina, 0, m_nMaxStamina);

    MAKE_IN_RANGE(m_nCurrentAngry, 0, m_nMaxAngry);

    m_nNextReplenishFrame = g_pSO3World->m_nGameLoop + GAME_FPS;
}

void KHero::DoCycleSynchronous()
{
    int nFrame = g_pSO3World->m_nGameLoop - (int)m_dwID;

    // 向自己同步当前耐力
    if (nFrame % 5 == 0)
    {
        g_PlayerServer.DoSyncCurrentEndurance(this);
        g_PlayerServer.DoSyncCurrentAngry(this);
        g_PlayerServer.DoSyncCurrentStamina(this);
    }
}

KDoodad* KHero::GetHoldingDoodad()
{
    KDoodad*        pResult  = NULL;
    BOOL            bRetCode = false;
    KMovableObject* pObj     = NULL;

    pObj = GetFirstFollowerWithState(mosToken);
    KG_PROCESS_ERROR(pObj);

    bRetCode = g_IsDoodad(pObj);
    KGLOG_PROCESS_ERROR(bRetCode);

    pResult = (KDoodad*)pObj;

Exit0:
    return pResult;
}

void KHero::ForbitAutoTakeBall(int nFrames)
{
    assert(nFrames > 0);
    m_nForbitAutoTakeBall = nFrames;
}

void KHero::AutoTakeBall()
{
    if (m_nForbitAutoTakeBall == 0 && !m_bForbitAutoAction)
    {
        TakeBall(true);
    }
}

// AI相关
KAI_BRANCH KHero::AISetMoveTarget(int nParams[])
{
    KAI_BRANCH  eRet    = KAI_BRANCH_FAILED;
    std::map<int, FUNC_SETTARGET>::iterator it = m_SetTargetFuncs.find(nParams[0]);
    if(it != m_SetTargetFuncs.end())
    {
        FUNC_SETTARGET Func = it->second;
        BOOL bRetCode = (this->*Func)(nParams);
        eRet = (bRetCode ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);  
    }
    else
    {
        KGLogPrintf(KGLOG_ERR, "Invalid MoveTargetType :%d\n", nParams[0]);
    }

    return eRet;
}

KAI_BRANCH KHero::AIMoveToTarget(BOOL bWalk)
{
    KAI_BRANCH eRet = KAI_BRANCH_FAILED;
    BOOL bRetCode = false;
    int  nDestX = 0;
    int  nDestY = 0;
    int  nDestZ = 0;
    int  nDirection = 0;

    bRetCode = GetAITargetPos(nDestX, nDestY, nDestZ);
    KG_PROCESS_ERROR(bRetCode);

    nDirection  = g_GetDirection(m_nX, m_nY, nDestX, nDestY);
    if (bWalk)
        bRetCode    = WalkTo(nDirection, true);   
    else
        bRetCode    = RunTo(nDirection, true);

    eRet = (bRetCode ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);

Exit0:
    return eRet;
}

KAI_BRANCH KHero::AITakeBall()
{
    KAI_BRANCH eRet = KAI_BRANCH_ERROR;
    BOOL bRetCode = TakeBall(false);
    eRet = (bRetCode ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);

    return eRet;
}

KAI_BRANCH KHero::AICheckNearTargetA(int nDelta)
{
    KAI_BRANCH eRet = KAI_BRANCH_ERROR;
    BOOL bRetCode = false;
    int  nDestX   = 0;
    int  nDestY   = 0;
    int  nDestZ   = 0;
    int  nDeltaX  = 0;
    int  nDeltaY  = 0;
    int  nDeltaZ  = 0;

    bRetCode = GetAITargetPos(nDestX, nDestY, nDestZ);
    KG_PROCESS_ERROR(bRetCode);

    nDeltaX     = abs(nDestX - m_nX);
    nDeltaY     = abs(nDestY - m_nY);
    nDeltaZ     = abs(nDestZ - m_nZ);

    if ( nDeltaX < nDelta && nDeltaY < nDelta && nDeltaZ < nDelta )
    {
        eRet = KAI_BRANCH_SUCCESS;
    }
    else
    {
        eRet = KAI_BRANCH_FAILED;
    }

Exit0:
    return eRet;
}

KAI_BRANCH KHero::AIAimAt(BOOL bSelf)
{
    KAI_BRANCH      eRet        = KAI_BRANCH_FAILED;
    BOOL            bRetCode    = false;
    int             nDirection  = 0;
    KMovableObject* pObj        = NULL;

    pObj = GetFirstFollowerWithState(mosToken);
    KG_PROCESS_ERROR(pObj);

    nDirection = GetAIAimAtDirection(bSelf);

    bRetCode = AimAt(true, nDirection, pObj->m_dwID, true);
    if (bRetCode)
    {
        m_nLastAimTime      = m_nVirtualFrame;
        eRet                = KAI_BRANCH_SUCCESS;
    }
    else
    {
        eRet = KAI_BRANCH_FAILED;
    }

Exit0:
    return eRet;
}

KAI_BRANCH KHero::AICheckAimFloor(int nFloor)
{
    KAI_BRANCH  eRet        = KAI_BRANCH_ERROR;
    BOOL        bRetCode    = false;
    int         nAfterTime  = 0;
    KBasketSocketFloor eCurFloor = bfInvalid;

    bRetCode = IsValidAimFloor(nFloor);
    KG_PROCESS_ERROR(bRetCode);

    KG_PROCESS_ERROR(m_eMoveState == cmsOnAimat);

    nAfterTime = (m_nVirtualFrame - m_nLastAimTime);
    eCurFloor  = m_pScene->GetAimFloor(nAfterTime);

    eRet = (eCurFloor == nFloor ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);

Exit0:
    return eRet;
}

KAI_BRANCH KHero::AIShootBall()
{
    KAI_BRANCH      eRet                = KAI_BRANCH_FAILED;
    BOOL            bRetCode            = false;
    int             nAimLoop            = 0;
    DWORD           dwInterferenceID    = ERROR_ID;

    bRetCode = HasFollowerWithStateAndType(mosToken, sotBall);
    KG_PROCESS_ERROR(bRetCode);

    KG_PROCESS_ERROR(m_eMoveState == cmsOnAimat);
    
    dwInterferenceID = GetInterferenceID();
    nAimLoop = m_nVirtualFrame - m_nLastAimTime;
    bRetCode = NormalShootBall(nAimLoop, dwInterferenceID);
    KG_PROCESS_ERROR(bRetCode);

    eRet = KAI_BRANCH_SUCCESS;
Exit0:
    return eRet;
}

KAI_BRANCH KHero::AICheckFollower(int nObjType, int nMoveType)
{
    KAI_BRANCH          eRet      = KAI_BRANCH_FAILED;
    BOOL                bRetCode  = false;
    KSceneObjectType    eObjType  = (KSceneObjectType)nObjType;
    KMoveType           eMoveType = (KMoveType)nMoveType;

    if (eObjType == sotInvalid)
    {
        if (eMoveType == mosInvalid)
        {
            bRetCode = HasFollower();
        }
        else
        {
            bRetCode = HasFollowerWithState(eMoveType);
        }
    }
    else
    {
        if (eMoveType == mosInvalid)
        {
            bRetCode = HasFollowerWithType(eObjType);
        }
        else
        {
            bRetCode = HasFollowerWithStateAndType(eMoveType, eObjType);
        }
    }
    KG_PROCESS_ERROR(bRetCode);

    eRet = KAI_BRANCH_SUCCESS;
Exit0:
    return eRet;
}

KAI_BRANCH KHero::AICheckBallTaker(int nType)
{
    KAI_BRANCH      eRet                = KAI_BRANCH_FAILED;
    KRelationType   eRelationType       = (KRelationType)nType;
    KRelationType   eCurRelationType    = rltNone;
    KBall*          pBall               = NULL; 
    KHero*          pBallTaker        = NULL;

    pBall = m_pScene->GetBall();
    KG_PROCESS_ERROR(pBall);

    pBallTaker = pBall->GetBallTaker();
    if (pBallTaker)
        eCurRelationType = GetRelationTypeWith(pBallTaker);

    if (eRelationType == eCurRelationType)
        eRet = KAI_BRANCH_SUCCESS;

Exit0:
    return eRet;
}


KAI_BRANCH KHero::AICastSkill(int nSlotIndex)
{
    KAI_BRANCH eRet = KAI_BRANCH_FAILED;
    BOOL bRetCode = false;
    uint32_t dwSkillID = ERROR_ID;
    KSkill* pSkill = NULL;

    KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < KACTIVE_SLOT_TYPE_TOTAL);

    dwSkillID = m_ActiveSkill[nSlotIndex];

    pSkill = GetWillCastSkill(dwSkillID);
    KG_PROCESS_ERROR(pSkill);

    bRetCode = CanCastSkill(pSkill, m_eFaceDir, false);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = CastSkill(pSkill->m_dwID, pSkill->m_dwStepID, m_eFaceDir);
    KGLOG_PROCESS_ERROR(bRetCode);

    eRet = KAI_BRANCH_SUCCESS;    
Exit0:
    return eRet;
}

KAI_BRANCH KHero::AIJump(BOOL bMove, int nDirectionType, int nParam)
{
    KAI_BRANCH eRet = KAI_BRANCH_FAILED;
    BOOL bRetCode   = false;
    int nDirection  = 0;

    bRetCode = GetDirectionByType(nDirectionType, nDirection, nParam);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Jump(bMove, nDirection, true);
    KG_PROCESS_ERROR(bRetCode);

    eRet = KAI_BRANCH_SUCCESS;
Exit0:
    return eRet;
}


KAI_BRANCH KHero::AICheckHeroState(int nParams[])
{
    KAI_BRANCH eRet = KAI_BRANCH_FAILED;

    for (int i = 0; i < KAI_ACTION_PARAM_NUM; ++i)
    {
        if (m_eMoveState == nParams[i])
        {
            eRet = KAI_BRANCH_SUCCESS;
            break;
        }
    }

    return eRet;
}

KAI_BRANCH KHero::AITakeDoodad(int nDoodadType)
{
    KAI_BRANCH  eRet        = KAI_BRANCH_FAILED;
    BOOL        bRetCode    = false;

    bRetCode = CanTakeDoodad();
    KG_PROCESS_ERROR(bRetCode);

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!g_IsDoodad(pObj))
            continue;
        if (nDoodadType != 0 && pObj->GetType() != nDoodadType)
            continue;

        KDoodad* pDoodad = (KDoodad*)pObj;
        if (!pDoodad->CanBeTokenBy(this))
            continue;

        bRetCode = TakeDoodad(pDoodad, false);
        if (bRetCode)
        {
            eRet = KAI_BRANCH_SUCCESS;
        }

        break;
    }

Exit0:
    return eRet;
}


KAI_BRANCH KHero::AIUseDoodad()
{
    KAI_BRANCH      eRet        = KAI_BRANCH_FAILED;
    BOOL            bRetCode    = false;
    KDoodad*        pDoodad     = NULL;
    unsigned char   nDirection  = 0;

    pDoodad = GetHoldingDoodad();
    KG_PROCESS_ERROR(pDoodad);

    nDirection = (m_eFaceDir == csdLeft ? DIRECTION_COUNT / 2 : 0);

    bRetCode = UseDoodad(pDoodad, nDirection, false);
    KG_PROCESS_ERROR(bRetCode);

    eRet = KAI_BRANCH_SUCCESS;
Exit0:
    return eRet;
}

KAI_BRANCH KHero::AIShootDoodad()
{
    KAI_BRANCH      eRet        = KAI_BRANCH_FAILED;
    BOOL            bRetCode    = false;
    KDoodad*        pDoodad     = NULL;

    pDoodad = GetHoldingDoodad();
    KG_PROCESS_ERROR(pDoodad);

    if (pDoodad->Is(sotBasket))
    {
        m_nShootTime = m_nVirtualFrame - m_nLastAimTime;
        bRetCode = ShootBasket(pDoodad->m_dwID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        bRetCode = ShootDoodad(pDoodad, false);
        KG_PROCESS_ERROR(bRetCode);
    }

    eRet = KAI_BRANCH_SUCCESS;
Exit0:
    return eRet;
}

KAI_BRANCH KHero::AIDropDoodad()
{
    KAI_BRANCH  eRet        = KAI_BRANCH_FAILED;
    BOOL        bRetCode    = false;
    KDoodad*    pDoodad     = NULL;

    pDoodad = GetHoldingDoodad();
    KG_PROCESS_ERROR(pDoodad);

    bRetCode = DropDoodad(pDoodad, false);
    KG_PROCESS_ERROR(bRetCode);

    eRet = KAI_BRANCH_SUCCESS;
Exit0:
    return eRet;
}

KAI_BRANCH KHero::AIPassBall()
{
    KAI_BRANCH  eRet        = KAI_BRANCH_FAILED;
    BOOL        bRetCode    = false;
    KHero*      pReceiver   = NULL;

    pReceiver = GetOneTeammate();
    KG_PROCESS_ERROR(pReceiver);

    bRetCode = PassBallTo(pReceiver, false);
    KG_PROCESS_ERROR(bRetCode);

    eRet = KAI_BRANCH_SUCCESS;
Exit0:
    return eRet;
}

KAI_BRANCH KHero::AIChangeDirection(int nDirectionType, int nParam)
{
    KAI_BRANCH  eRet        = KAI_BRANCH_FAILED;
    BOOL        bRetCode    = false;
    int         nDirection  = 0;
    KTWO_DIRECTION eFaceDirBeforeChange = csdInvalid;
    KTWO_DIRECTION eFaceDirAfterChange = csdInvalid;
    KBasketSocket* pSocket = NULL;

    if (
        m_eMoveState != cmsOnJump && m_eMoveState != cmsOnStand &&
        m_eMoveState != cmsOnWalk && m_eMoveState != cmsOnRun && m_eMoveState != cmsOnAimat
        ) 
    {
        goto Exit0;
    }

    bRetCode = GetDirectionByType(nDirectionType, nDirection, nParam);
    KG_PROCESS_ERROR(bRetCode);

    if (nDirectionType == KAI_DIRECTION_SELF_BASKET)
    {
        pSocket = GetOneSelfBaksetSocket();
        KGLOG_PROCESS_ERROR(pSocket);
    }
    else if (nDirectionType == KAI_DIRECTION_ENEMY_BASKET)
    {
        pSocket = GetOneEnemyBaksetSocket();
        KGLOG_PROCESS_ERROR(pSocket);
    }

    if (pSocket)
    {
        nDirection = 0;
        if (pSocket->m_eDir == csdRight)
            nDirection = DIRECTION_COUNT / 2;
    }

    eFaceDirBeforeChange = GetFaceDir();
    UpdateFaceDir(nDirection);
    eFaceDirAfterChange = GetFaceDir();

    if (eFaceDirAfterChange != eFaceDirBeforeChange)
        g_PlayerServer.DoSyncFaceDir(this, eFaceDirAfterChange == csdRight ? 0 : 4);

    if (!IsInAir())
    {
        m_nVelocityX = 0;
        m_nVelocityY = 0;
    }

    eRet = KAI_BRANCH_SUCCESS;
Exit0:
    return eRet;
}


KAI_BRANCH KHero::AICheckBallMoveType(int nParam[])
{
    KAI_BRANCH  eRet    = KAI_BRANCH_FAILED;
    KBall*      pBall   = NULL;
    KMoveType   eMoveType = mosInvalid;

    pBall = m_pScene->GetBall();
    KG_PROCESS_ERROR(pBall);

    eMoveType = pBall->GetMoveType();
    for (int i = 0; i < KAI_ACTION_PARAM_NUM; ++i)
    {
        if (eMoveType == nParam[i])
        {
            eRet = KAI_BRANCH_SUCCESS;
            break;
        }
    }
Exit0:
    return eRet;
}


// AI用辅助函数

void KHero::InitAIFunctions()
{
    m_SetTargetFuncs[KMOVE_TARGET_POSITION] = &KHero::SetTarget_Position;
    m_SetTargetFuncs[KMOVE_TARGET_BALL]     = &KHero::SetTarget_Ball;
    m_SetTargetFuncs[KMOVE_TARGET_HERO]     = &KHero::SetTarget_Hero;
    m_SetTargetFuncs[KMOVE_TARGET_SCENEOBJ] = &KHero::SetTarget_SceneObj;
}

BOOL KHero::GetAITargetPos(int& nX, int& nY, int& nZ)
{
    BOOL bResult = false;

    switch(m_AIData.eTargetType)
    {
    case KTARGET_OBJ:
        {
            KMovableObject* pTarget = GetAITarget();
            if (pTarget)
            {
                nX = pTarget->m_nX;
                nY = pTarget->m_nY;
                nZ = pTarget->m_nZ;
                bResult = true;
            }
        }
        break;
    case KTARGET_POSITION:
        {
            nX = m_AIData.nTargetX;
            nY = m_AIData.nTargetY;
            nZ = m_AIData.nTargetZ;
            bResult = true;
        }
        break;
    default:
        break;
    }

Exit0:
    return bResult;
}

int KHero::GetAIAimAtDirection(BOOL bSelf)
{
    int nRetDirection = 0;

    if ( m_nSide == sidLeft && bSelf 
        || (m_nSide == sidRight && !bSelf) )
    {
        nRetDirection = DIRECTION_COUNT / 2;
    }
    else
    {
        nRetDirection = 0;
    }

    return nRetDirection;
}

// 需要和KScene::GetAimFloor保持一致
BOOL KHero::IsValidAimFloor(int nFloor)
{
    BOOL bResult = false;
    KBasketSocketFloor eFloor = (KBasketSocketFloor)nFloor;

    if (eFloor == bfFirst || eFloor == bfThird || eFloor == bfFifth)
    {
        bResult = true;
    }

    return bResult;
}

void KHero::FireAIEvent(KAI_EVENT eAIEvent, DWORD dwSrcID, int nEventParam)
{
    if (m_bAiMode)
    {
        m_AIVM.FireEvent(eAIEvent, dwSrcID, nEventParam);
    }
}

BOOL KHero::SetTarget_Position(int nParams[])
{
    BOOL bResult = false;

    switch(nParams[1])
    {
    case KMOVE_TARGET_FILTER_SCENE_CENTER:
        {
            m_AIData.eTargetType = KTARGET_POSITION;
            m_AIData.nTargetX = (m_pScene->GetLength() - 1) / 2;
            m_AIData.nTargetY = (m_pScene->GetWidth() - 1) / 2;
            m_AIData.nTargetZ = 0;

            bResult = true;
        }
        break;
    case KMOVE_TARGET_FILTER_POSITION_XY:
        {
            m_AIData.eTargetType = KTARGET_POSITION;
            m_AIData.nTargetX = m_nX + nParams[2];
            m_AIData.nTargetY = m_nY + nParams[3];
            m_AIData.nTargetZ = 0;

            MakeInRange<int>(m_AIData.nTargetX, 0, m_pScene->GetLength() - 1);
            MakeInRange<int>(m_AIData.nTargetY, 0, m_pScene->GetWidth() - 1);

            bResult = true;
        }
        break;
    case KMOVE_TARGET_FILTER_RANDOM_X:
        {
            int nDelta = 0;
            KGLOG_PROCESS_ERROR(nParams[3] > nParams[2]);
            nDelta = g_Random(nParams[3] - nParams[2]);

            m_AIData.eTargetType = KTARGET_POSITION;
            m_AIData.nTargetX = m_nX + nParams[2] + nDelta;
            m_AIData.nTargetY = m_nY;
            m_AIData.nTargetZ = 0;

            MakeInRange<int>(m_AIData.nTargetX, 0, m_pScene->GetLength() - 1);
            MakeInRange<int>(m_AIData.nTargetY, 0, m_pScene->GetWidth() - 1);

            bResult = true;
        }
        break;
    case KMOVE_TARGET_FILTER_RANDOM_Y:
        {
            int nDelta = 0;
            KGLOG_PROCESS_ERROR(nParams[3] > nParams[2]);
            nDelta = g_Random(nParams[3] - nParams[2]);

            m_AIData.eTargetType = KTARGET_POSITION;
            m_AIData.nTargetX = m_nX ;
            m_AIData.nTargetY = m_nY + nParams[2] + nDelta;
            m_AIData.nTargetZ = 0;
            MakeInRange<int>(m_AIData.nTargetX, 0, m_pScene->GetLength() - 1);
            MakeInRange<int>(m_AIData.nTargetY, 0, m_pScene->GetWidth() - 1);

            bResult = true;
        }
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Invalid Position Target Type:%d\n", nParams[1]);
    }

Exit0:
    return bResult;
}

BOOL KHero::SetTarget_Hero(int nParams[])
{
    BOOL bResult = false;
    KHero* pTarget = GetHeroByFilter(nParams);
    if (pTarget)
    {
        m_AIData.eTargetType    = KTARGET_OBJ;
        m_AIData.dwTargetObjID  = pTarget->m_dwID;
        bResult                 = true;
    }

    return bResult;
}

BOOL KHero::SetTarget_SceneObj(int nParams[])
{
    BOOL bResult = false;
    KSceneObject* pTarget= GetSceneObjByFilter(nParams);
    if (pTarget)
    {
        m_AIData.eTargetType = KTARGET_OBJ;
        m_AIData.dwTargetObjID = pTarget->m_dwID;
        bResult = true;
    }

    return bResult;
}

BOOL KHero::SetTarget_Ball(int nParams[])
{
    BOOL bResult = false;
    KBall* pBall = m_pScene->GetBall();
    KG_PROCESS_ERROR(pBall);
    if (!pBall->HasGuide())
    {
        m_AIData.eTargetType = KTARGET_OBJ;
        m_AIData.dwTargetObjID = pBall->m_dwID;
        bResult = true;
    }

Exit0:
    return bResult;
}

KHero* KHero::GetHeroByFilter(int nParams[])
{
    KHero*  pResult = NULL;
    int nFilter = nParams[1];
    BOOL bExcludeCannotBeAttacked = nParams[2];

    switch(nFilter)
    {
    case KMOVE_TARGET_FILTER_BALL_TAKER:
        pResult = GetBallTaker();
        break;
    case KMOVE_TARGET_FILTER_RANDOM_ENEMY:
        pResult = GetRandomEnemy(bExcludeCannotBeAttacked);
        break;
    case KMOVE_TARGET_FILTER_FARER_ENEMY:
        pResult = GetFarerEnemy(bExcludeCannotBeAttacked);
        break;
    case KMOVE_TARGET_FILTER_NEARER_ENEMY:
        pResult = GetNearerEnemy(bExcludeCannotBeAttacked);
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Invalid Hero Filter Type:%d\n", nFilter);
    }
    return pResult;
}

KHero* KHero::GetBallTaker()
{
    return m_pScene->GetBallTaker();
}

KHero* KHero::GetRandomEnemy(BOOL bExcludeCannotBeAttacked)
{
    KHero*              pResult  = NULL;
    BOOL                bRetCode = false;
    std::vector<KHero*> vecAllEnemy;

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;

        KHero* pHero = (KHero*)pObj;
        if (pHero->m_nSide == m_nSide)
            continue;

        if (bExcludeCannotBeAttacked && !pHero->CanBeAttacked())
            continue;

        vecAllEnemy.push_back(pHero);
    }

    if (vecAllEnemy.size() > 0)
    {
        unsigned uIndex = g_Random((unsigned int)vecAllEnemy.size());
        pResult = vecAllEnemy[uIndex];
    }

    return pResult;
}

KHero* KHero::GetFarerEnemy(BOOL bExcludeCannotBeAttacked)
{
    KHero*              pResult = NULL;
    BOOL                bRetCode = false;
    int                 nMaxDistance = -1;
    std::vector<KHero*> vecAllEnemy;

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;

        KHero* pHero = (KHero*)pObj;
        if (pHero->m_nSide == m_nSide)
            continue;

        if (bExcludeCannotBeAttacked && !pHero->CanBeAttacked())
            continue;

        int nCurDistance = g_GetDistance3(m_nX, m_nY, m_nZ, pHero->m_nX, pHero->m_nY, pHero->m_nZ);
        if (nCurDistance > nMaxDistance)
        {
            nMaxDistance = nCurDistance;
            pResult = pHero;
        }
    }

    return pResult;
}

KHero* KHero::GetNearerEnemy(BOOL bExcludeCannotBeAttacked)
{
    KHero*              pResult = NULL;
    int                 nMinDistance = INT_MAX;
    std::vector<KHero*> vecAllEnemy;

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;

        KHero* pHero = (KHero*)pObj;
        if (pHero->m_nSide == m_nSide)
            continue;

        if (bExcludeCannotBeAttacked && !pHero->CanBeAttacked())
            continue;

        int nCurDistance = g_GetDistance3(m_nX, m_nY, m_nZ, pHero->m_nX, pHero->m_nY, pHero->m_nZ);
        if (nCurDistance < nMinDistance)
        {
            nMinDistance    = nCurDistance;
            pResult         = pHero;
        }
    }

    return pResult;
}

KSceneObject* KHero::GetSceneObjByFilter(int nParams[])
{
    KSceneObject* pResult = NULL;
    int nFilter = nParams[1];

    switch(nFilter)
    {
    case KMOVE_TARGET_FILTER_SCENEOBJ_FARER:
        {
            pResult = GetFarerSceneObj(nParams[2]);
        }
        break;
    case KMOVE_TARGET_FILTER_SCENEOBJ_NEARER:
        {
            pResult = GetNearerSceneObj(nParams[2]);
        }
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Invalid SceneObj Filter:%d\n", nFilter);
        break;
    }

    return pResult;
}

KSceneObject* KHero::GetFarerSceneObj(int nObjType)
{
    KSceneObject* pResult = NULL;
    int nMaxDistance = -1;

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != nObjType || pObj->m_dwID == m_dwID || pObj->HasGuide())
            continue;

        if (!g_IsDoodad(pObj))
            continue;
        KMovableObject* pMovableObj = (KMovableObject*)pObj;
        KMoveType eMoveType = pMovableObj->GetMoveType();
        if (eMoveType != mosFree && eMoveType != mosIdle && eMoveType != mosTrackMove)
            continue;

        int nCurDistance = g_GetDistance3<int>(m_nX, m_nY, m_nZ, pObj->m_nX, pObj->m_nY, pObj->m_nZ);

        if(nCurDistance > nMaxDistance)
        {
            nMaxDistance = nCurDistance;
            pResult = pObj;
        }
    }

    return pResult;
}

KSceneObject* KHero::GetNearerSceneObj(int nObjType)
{
    KSceneObject* pResult = NULL;
    int nMinDistance = INT_MAX;

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != nObjType || pObj->m_dwID == m_dwID || pObj->HasGuide())
            continue;

        if (!g_IsDoodad(pObj))
            continue;

        KMovableObject* pMovableObj = (KMovableObject*)pObj;
        KMoveType eMoveType = pMovableObj->GetMoveType();
        if (eMoveType != mosFree && eMoveType != mosIdle && eMoveType != mosTrackMove)
            continue;

        int nCurDistance = g_GetDistance3<int>(m_nX, m_nY, m_nZ, pObj->m_nX, pObj->m_nY, pObj->m_nZ);

        if(nCurDistance < nMinDistance)
        {
            nMinDistance = nCurDistance;
            pResult = pObj;
        }
    }

    return pResult;
}

BOOL KHero::GetDirectionByType(int nDirectionType, int& rDirection, int nParam)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    switch(nDirectionType)
    {
    case KAI_DIRECTION_VALUE:
        {
            rDirection = nParam;
            bResult = true;
        }
        break;
    case KAI_DIRECTION_BALL:
        {
            KBall* pBall = m_pScene->GetBall();
            KG_PROCESS_ERROR(pBall);
            rDirection = g_GetDirection(m_nX, m_nY, pBall->m_nX, pBall->m_nY);
            bResult = true;
        }
        break;
    case KAI_DIRECTION_NEARER_ENEMY:
        {
            KHero* pEnemy = GetNearerEnemy(false);
            if (pEnemy)
            {
                rDirection = g_GetDirection(m_nX, m_nY, pEnemy->m_nX, pEnemy->m_nY);
                bResult = true;
            }
        }
        break;
    case KAI_DIRECTION_NEARER_TEAMMATE:
        {
            KHero* pTeammate = GetNearerTeammate();
            if (pTeammate)
            {
                rDirection = g_GetDirection(m_nX, m_nY, pTeammate->m_nX, pTeammate->m_nY);
                bResult = true;
            }       
        }
        break;
    case KAI_DIRECTION_SELF_BASKET:
        {
            KBasketSocket* pSocket = GetOneSelfBaksetSocket();
            if (pSocket)
            {
                rDirection = g_GetDirection(m_nX, m_nY, pSocket->m_nX, pSocket->m_nY);
                bResult = true;
            }
        }
        break;
    case KAI_DIRECTION_ENEMY_BASKET:
        {
            KBasketSocket* pSocket = GetOneEnemyBaksetSocket();
            if (pSocket)
            {
                rDirection = g_GetDirection(m_nX, m_nY, pSocket->m_nX, pSocket->m_nY);
                bResult = true;
            }
        }
        break;
    case KAI_DIRECTION_SPECIFIED_TARGET:
        {
            KPOSITION cTarget;
            bRetCode = GetAITargetPos(cTarget.nX, cTarget.nY, cTarget.nZ);
            if (bRetCode)
            {
                rDirection = g_GetDirection(m_nX, m_nY, cTarget.nX, cTarget.nY);
                bResult = true;
            }
        }
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Invalid Direction Type:%d\n", nDirectionType);
        break;
    }

Exit0:
    return bResult;
}

KHero* KHero::GetNearerTeammate()
{
    KHero* pResult = NULL;
    int nMinDistance = INT_MAX;

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotHero || pObj->m_dwID == m_dwID)
            continue;

        KHero* pHero = (KHero*)pObj;
        if (pHero->m_nSide != m_nSide)
            continue;

        int nCurDistance = g_GetDistance3<int>(m_nX, m_nY, m_nZ, pHero->m_nX, pHero->m_nY, pHero->m_nZ);
        if (nCurDistance < nMinDistance)
        {
            nMinDistance = nCurDistance;
            pResult = pHero;
        }

    }

    return pResult;
}

KBasketSocket* KHero::GetOneSelfBaksetSocket()
{
    KBasketSocket* pResult = NULL;
    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotBasketSocket)
            continue;

        KBasketSocket* pSocket = (KBasketSocket*)pObj;
        if ( (m_nSide == sidLeft && pSocket->m_eDir == csdRight)
            || (m_nSide == sidRight && pSocket->m_eDir == csdLeft) )
        {
            pResult = pSocket;
            break;
        }

    }

    return pResult;
}

KBasketSocket* KHero::GetOneEnemyBaksetSocket()
{
    KBasketSocket* pResult = NULL;
    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotBasketSocket)
            continue;

        KBasketSocket* pSocket = (KBasketSocket*)pObj;
        if ( (m_nSide == sidLeft && pSocket->m_eDir == csdLeft)
            || (m_nSide == sidRight && pSocket->m_eDir == csdRight) )
        {
            pResult = pSocket;
            break;
        }

    }

    return pResult;
}

BOOL KHero::TakeDoodad(KDoodad * pObject, BOOL bAdjust)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = pObject->BeToken(this);
    if (!bRetCode && bAdjust)
    {
        LogInfo("TakeDoodad error, sync doodad state!");
        g_PlayerServer.DoSyncSceneObject(pObject);
    }
    KG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoTakeObject(this, pObject);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KHero::UseDoodad(KDoodad* pDoodad, unsigned char direction, BOOL bAdjust)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KTWO_DIRECTION eDir = csdInvalid;
    DWORD dwDoodadTemplateID = ERROR_ID;

    eDir = GetNextFaceDir(direction);
    KGLOG_PROCESS_ERROR(eDir == csdLeft || eDir == csdRight);

    bRetCode = pDoodad->BeUseBy(this, eDir);
    if (!bRetCode && bAdjust)
    {
        LogInfo("UseDoodad error, sync doodad state!");
        g_PlayerServer.DoSyncSceneObject(pDoodad);
    }
    KG_PROCESS_ERROR(bRetCode);

    if (IsMainHero())
    {
        if (pDoodad->m_pTemplate)
        {
            dwDoodadTemplateID = pDoodad->m_pTemplate->m_dwID;
        }
        m_pOwner->OnEvent(peUseSceneObj, pDoodad->GetType(), dwDoodadTemplateID);
    }

    bRetCode = FireUseDoodadAIEvent(pDoodad);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoUseDoodad(this, pDoodad, direction);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::ShootDoodad(KDoodad* pDoodad, BOOL bAdjust)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KBasketSocket* pTargetSocket = NULL;

    bRetCode = CanShootDoodad();
    KGLOG_PROCESS_ERROR(bRetCode);

    if(pDoodad->GetType() == sotBasket)
    {
        KTWO_DIRECTION eDir = GetFaceDir();
        KG_PROCESS_ERROR(m_eMoveState == cmsOnAimat);
        m_nShootTime = m_nVirtualFrame - m_nLastAimTime;
        pTargetSocket = m_pScene->GetShootBasketBasketSocket(eDir, m_nShootTime);
    }
    else
    {
        pTargetSocket = GetShootSocket();
    }  
    KG_PROCESS_ERROR(pTargetSocket);

    bRetCode = pDoodad->BeShoot(this, pTargetSocket);
    if(!bRetCode && bAdjust)
    {
        LogInfo("ShootDoodad error, sync doodad state!");
        g_PlayerServer.DoSyncSceneObject(pDoodad);
    }
    KG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoShootObject(this, pDoodad, pTargetSocket->m_dwID, pDoodad->m_bCanHit);

    if (IsInAir())
    {
        SetMoveState(cmsOnJump);
    }
    else
    {
        Stand(false);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::DropDoodad(KDoodad* pDoodad, BOOL bAdjust)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = CanDropDoodad();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pDoodad->BeUnTake(this);
    if(!bRetCode && bAdjust)
    {
        LogInfo("DropDoodad error, sync doodad state!");
        g_PlayerServer.DoSyncSceneObject(pDoodad);
    }
    KG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoDropObject(this, pDoodad);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::PassBallTo(KHero* pReceiver, BOOL bAdjust)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KBall*  pBall       = NULL;

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    KG_PROCESS_ERROR(!m_bSkillSlaming);

    bRetCode = pBall->PassTo(this, pReceiver);
    if (!bRetCode && bAdjust)
    {
        LogInfo("PassBall error, sync ball state!");
        g_PlayerServer.DoSyncSceneObject(pBall);
    }
    KG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoPassBallTo(this, pReceiver);

    m_pScene->OnBallPassTo(this, pReceiver);

    bResult = true;
Exit0:
    return bResult;
}

KHero* KHero::GetOneTeammate()
{
    KHero*              pResult         = NULL;
    std::vector<KHero*> vecAllTeammate;

    m_pScene->GetAllTeammate(this, vecAllTeammate);
    if (vecAllTeammate.size() > 0)
    {
        pResult = vecAllTeammate[0];
    }

    return pResult;
}

void KHero::TryFinishCastingSkill()
{
    BOOL bShouldFinish   = false;
    BOOL bTimeEnd = false;
    BOOL bOnFloor = false;
    KSkill* pSkill = NULL;
    KMoveType eMoveType = GetMoveType();

    KG_PROCESS_ERROR(m_pCastingSkill);

    pSkill = m_pCastingSkill;

    if (m_pCastingSkill->m_nEndFrame <= m_nVirtualFrame - m_nAttackSkillTime)
        bTimeEnd = true;

    if (!IsInAir())
        bOnFloor = true;

    switch(m_pCastingSkill->m_eEndType)
    {
    case KSKILL_END_TYPE_BY_TIME:
        bShouldFinish = bTimeEnd;
        break;
    case KSKILL_END_TYPE_ON_FLOOR:
        bShouldFinish = bOnFloor;
        break;
    case KSKILL_END_TYPE_BYTIME_OR_ONFLOOR:
        bShouldFinish = (bTimeEnd || bOnFloor);
        break;
    case KSKILL_END_TYPE_TRACK_MOVE:
        if (m_bCastingSkillTrackMoved)
        {
            bShouldFinish = (eMoveType != mosTrackMove);
        }
        else
        {
            bShouldFinish = bTimeEnd;   // 为了保证技能没有触发有轨运动时能正常结束
            if (bShouldFinish)
            {
                KGLogPrintf(
                    KGLOG_ERR, 
                    "Track move skill id=%d, step=%d has no track!!", 
                    m_pCastingSkill->m_dwID, 
                    m_pCastingSkill->m_dwStepID
                );
            }
        }
        break;
    default:
        bShouldFinish = true;
        KGLogPrintf(KGLOG_INFO, "Invalid Skill End Type :%d", m_pCastingSkill->m_eEndType);
        break;
    }

    if (bShouldFinish)
    {
        EnsureNotCastingSkill(); 
        TriggerOnSkillFinished(pSkill);
        m_nPreSkillEndTime = m_nVirtualFrame;

        LogInfo("Hero id=%d skill id=%d finished at frame=%d!", m_dwID, pSkill->m_dwID, m_nVirtualFrame);
    }

Exit0:
    return;
}

BOOL KHero::EnsureNotCastingSkill()
{
    BOOL    bResult  = false;
    BOOL    bRetCode = false;

    KG_PROCESS_SUCCESS(!m_pCastingSkill);

    if (m_pCastingSkill->m_eSkillType == skilltypeGrab && m_dwGrabHeroID)
    {
        KHero* pTarget = m_pScene->GetHeroById(m_dwGrabHeroID);
        if (pTarget)
        {
            pTarget->m_bInvincible = false;
            pTarget->TurnToMoveType(mosFree);
        }
    }

    bRetCode = m_BuffList.DelBuff(m_pCastingSkill->m_dwSrcBuffID);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pCastingSkill->m_bClearVelocityAtEnd)
        ClearVelocity();

    m_dwGrabHeroID           = ERROR_ID;
    m_bInvincible            = false;
    m_nAttackSkillTime       = 0;
    m_pCastingSkill          = NULL;
    m_bBaTi                  = false;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}


void KHero::TriggerOnSkillFinished(KSkill* pSkill)
{
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(pSkill);

    bRetCode = IsMainHero();
    KG_PROCESS_ERROR(!bRetCode);

    for (int i = 0; i < countof(pSkill->m_SubSkill); ++i)
    {
        KSubSkillInfo& subSkill = pSkill->m_SubSkill[i];
        KSkill* pSkill = NULL;

        if (subSkill.dwID == ERROR_ID)
            continue;

        if (subSkill.eType != KSKILL_TRIGGER_TYPE_DISAPPEAR)
            continue;

        pSkill = g_pSO3World->m_SkillManager.GetSkill(subSkill.dwID, 0);
        if (!pSkill)
            continue;

        switch (pSkill->m_eSkillType)
        {
        case skilltypeGrab:
            bRetCode = CastGrabSkill(subSkill.dwID, 0, m_dwPerSkillTargetID);
            KG_PROCESS_ERROR(bRetCode);
            break;
        default:
            bRetCode = CastSkill(subSkill.dwID, 0, m_eFaceDir, true);
            KGLOG_PROCESS_ERROR(bRetCode);
            break;
        }
    }

Exit0:
    return;
}


void KHero::OverTimeSkillProcess()
{
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(m_pCastingSkill);

    bRetCode = ProcessSkillSegment();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = ProcessSkillAttack();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = UpdateVelocityByAnchors();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

KSkill* KHero::GetWillCastSkill(DWORD dwSkillID)
{
    KSkill* pSkill = NULL;

    KG_PROCESS_ERROR(dwSkillID != ERROR_ID);

    if (m_pPreSkill && m_pPreSkill->m_dwID == dwSkillID)
    {
        pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID, m_pPreSkill->m_dwStepID + 1);
        if (!pSkill)
        {
            pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID);
        }
        else if (m_nVirtualFrame - m_nPreSkillCastTime > pSkill->m_nStepMaxFrame || m_nVirtualFrame - m_nPreSkillCastTime < pSkill->m_nStepMinFrame ||
            pSkill->m_nRequireLevel > m_nLevel)
        {
            pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID);         
        }
    }
    else
    {
        pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID);
    }
Exit0:
    return pSkill;
}

BOOL KHero::FreezeByFrame(int nFrameCount)
{
    BOOL bResult = false;

    if (m_bBaTi)
        return true;

    KGLOG_PROCESS_ERROR(nFrameCount > 0);

    SetMoveState(cmsOnFreeze);
    ClearVelocity();

    m_nFreezeFrame = g_pSO3World->m_nGameLoop + nFrameCount;

    m_cRecordedStatus.ClearRecord();

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::UnFreeze()
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(m_eMoveState == cmsOnFreeze);

    m_nFreezeFrame = 0;

    if (!IsInAir())
    {
        SetMoveState(cmsOnStand);
    }
    else
    {
        SetMoveState(cmsOnJump);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::TurnOnAI()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KG_PROCESS_SUCCESS(m_bAiMode);

    KG_PROCESS_ERROR(m_AIData.nAIType > 0);

    bRetCode = m_AIVM.Setup(this, m_AIData.nAIType);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_bAiMode = true;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::TurnOffAI()
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(m_bAiMode);

    m_AIVM.Setup(this, 0);
    m_bAiMode = false;

    bResult = true;
Exit0:
    return bResult;
}

void KHero::OnEnterScene(KScene* pScene)
{
    KMapParams* pMapInfo = NULL;
    KPlayer* pOwner = NULL;
    KHeroData* pHeroData = NULL;
    DWORD dwTabType[cdShotcutSize] = {0};
    DWORD dwTabIndex[cdShotcutSize] = {0};
    KHero* pTeamHero = NULL;

    assert(pScene);

    pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(pScene->m_dwMapID);
    KGLOG_PROCESS_ERROR(pMapInfo);

    pOwner = GetOwner();
    KG_PROCESS_ERROR(pOwner);

    // 记录携带的道具和技能
    KG_PROCESS_ERROR(IsMainHero()); // 只有玩家才记录

    for (int i = 0; i < cdShotcutSize; ++i)
    {
        IItem* pItem = pOwner->m_ItemList.GetPlayerItem(eppiPlayerShotcutBar, i);
        if (pItem)
        {
            const KItemProperty* pItemPro = pItem->GetProperty();
            assert(pItemPro);
            dwTabType[i] = pItemPro->dwTabType;
            dwTabIndex[i] = pItemPro->dwTabIndex;
        }
    }

    pHeroData = pOwner->m_HeroDataList.GetHeroData(m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    if (pMapInfo->nType == emtPVEMap)
    {
        PLAYER_LOG(
            pOwner, "fight,pve_skill,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", 
            pHeroData->m_SelectedActvieSkill[0],
            pHeroData->m_SelectedActvieSkill[1],
            pHeroData->m_SelectedActvieSkill[2],
            pHeroData->m_SelectedActvieSkill[3],
            pHeroData->m_SelectedActvieSkill[4],
            pHeroData->m_SelectedActvieSkill[5],
            pHeroData->m_SelectedActvieSkill[6],
            pHeroData->m_SelectedActvieSkill[7],
            pHeroData->m_SelectedPassiveSkill[0],
            pHeroData->m_SelectedPassiveSkill[1],
            pHeroData->m_SelectedPassiveSkill[2],
            pHeroData->m_SelectedNirvanaSkill[0],
            pHeroData->m_SelectedNirvanaSkill[1],
            pHeroData->m_SelectedSlamBallSkill[0]
        );

        PLAYER_LOG(
            pOwner, "fight,pve_item,%u-%u,%u-%u,%u-%u", 
            dwTabType[0], dwTabIndex[0],
            dwTabType[1], dwTabIndex[1],
            dwTabType[2], dwTabIndex[2]
        );
    }
    else if (pMapInfo->nType == emtPVPMap)
    {
        PLAYER_LOG(
            pOwner, "fight,pvp_skill,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", 
            pHeroData->m_SelectedActvieSkill[0],
            pHeroData->m_SelectedActvieSkill[1],
            pHeroData->m_SelectedActvieSkill[2],
            pHeroData->m_SelectedActvieSkill[3],
            pHeroData->m_SelectedActvieSkill[4],
            pHeroData->m_SelectedActvieSkill[5],
            pHeroData->m_SelectedActvieSkill[6],
            pHeroData->m_SelectedActvieSkill[7],
            pHeroData->m_SelectedPassiveSkill[0],
            pHeroData->m_SelectedPassiveSkill[1],
            pHeroData->m_SelectedPassiveSkill[2],
            pHeroData->m_SelectedNirvanaSkill[0],
            pHeroData->m_SelectedNirvanaSkill[1],
            pHeroData->m_SelectedSlamBallSkill[0]
        );

        PLAYER_LOG(
            pOwner, "fight,pvp_item,%u-%u,%u-%u,%u-%u", 
            dwTabType[0], dwTabIndex[0],
            dwTabType[1], dwTabIndex[1],
            dwTabType[2], dwTabIndex[2]
        );
    }

Exit0:
    return;
}

void KHero::OnFreeze()
{
    if (g_pSO3World->m_nGameLoop > m_nFreezeFrame)
    {
        if (!IsInAir())
        {
            SetMoveState(cmsOnStand);
        }
        else
        {
            SetMoveState(cmsOnJump);
        }
    }
}

BOOL KHero::ProcessCastSkillInAir()
{
    BOOL       bResult       = false;
    BOOL       bRetCode      = false;
    uint64_t   ullSkillKey   = 0;
    int        nCastCount    = 0;
    KCAST_SKILL_COUNT_INAIR_LIST::iterator it;

    KGLOG_PROCESS_ERROR(m_pCastingSkill);

    if (m_pCastingSkill->m_nCanCastCountInAir <= 0)
        goto Exit1;

    bRetCode = IsInAir();
    KG_PROCESS_SUCCESS(!bRetCode);

    ullSkillKey = MAKE_INT64(m_pCastingSkill->m_dwID, m_pCastingSkill->m_dwStepID);

    it = m_mapCastSkillCountInAir.find(ullSkillKey);
    if (it != m_mapCastSkillCountInAir.end())
        nCastCount = it->second;

    m_mapCastSkillCountInAir[ullSkillKey] = nCastCount + 1;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::SetUpAVM()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(m_bAiMode);
    KG_PROCESS_ERROR(m_AIData.nAIType > 0);

    bRetCode = m_AIVM.Setup(this, m_AIData.nAIType);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::HasBuff(DWORD dwBuffID) const
{
    return m_BuffList.HasBuff(dwBuffID);
}

void KHero::DoDelayAction()
{
    DelayApplySkillEffect();
}

void KHero::DelayApplySkillEffect()
{
    KSkill* pSkillDelayApply = NULL;

    if (m_nFrameToApplySkillEffect != 0 && m_nVirtualFrame >= m_nFrameToApplySkillEffect)
    {
        pSkillDelayApply = m_pSkillDelayApply;

        m_bInvincible = false;
        StopLoseControlByCounter();

        ApplyAttribute(pSkillDelayApply->m_pDestAttribute);
        AddBuff(pSkillDelayApply->m_dwDestBuffID);

        m_nFrameToApplySkillEffect = 0;
        m_pSkillDelayApply = NULL;

        if (m_eMoveState == cmsOnKnockedDown)
        {
            KHero* pAttacker = m_pScene->GetHeroById(m_dwAttackerID);
            if (pAttacker)
                pAttacker->m_GameStat.m_nKnockDownOther += 1;
        }
    }
}

void KHero::ApplySkillEffect(KSkill* pSkill)
{
    if (pSkill->m_bDelayApplySkillEffect)
    {
        m_nFrameToApplySkillEffect = m_nVirtualFrame + pSkill->m_nSpasticityFrame;
        m_pSkillDelayApply = pSkill;
    }
    else
    {
        ApplyAttribute(pSkill->m_pDestAttribute);
        AddBuff(pSkill->m_dwDestBuffID);

        if (m_eMoveState == cmsOnKnockedDown)
        {
            KHero* pAttacker = m_pScene->GetHeroById(m_dwAttackerID);
            if (pAttacker)
                pAttacker->m_GameStat.m_nKnockDownOther += 1;
        }
    }

    if (pSkill->m_nSpasticityFrame > 0 )
    {
        if (m_eMoveState != cmsOnHalt && m_eMoveState != cmsOnKnockedDown && 
            m_eMoveState != cmsOnKnockedOff)
        {
            LoseControlByCounter(pSkill->m_nSpasticityFrame);
            InvalidateMoveState(0, true);
        }
    }

    if (pSkill->m_bInvincibleOnSpasticity)
    {
        m_bInvincible = true;
    }
}

KMovableObject* KHero::GetAITarget()
{
    KMovableObject* pResult = NULL;
    KSceneObject* pObj = NULL;

    KG_PROCESS_ERROR(m_AIData.eTargetType == KTARGET_OBJ);

    pObj = m_pScene->GetSceneObjByID(m_AIData.dwTargetObjID);
    KG_PROCESS_ERROR(pObj);

    KG_PROCESS_ERROR(g_IsDoodad(pObj) || pObj->Is(sotHero));

    pResult = (KMovableObject*)pObj;

Exit0:
    return pResult;
}

KHero* KHero::GetTeammate()
{
    KHero* pResult = NULL;

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (pObj->GetType() != sotHero || pObj->m_dwID == m_dwID)
            continue;

        KHero* pHero = (KHero*)pObj;
        if (pHero->m_nSide != m_nSide)
            continue;

        pResult = pHero;
        break;
    }

    return pResult;
}

BOOL KHero::IsMainHero()
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;

    KG_PROCESS_ERROR(m_pOwner);

    KG_PROCESS_ERROR(m_pOwner->m_pFightingHero == this);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::IsAssistHero()
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;

    KG_PROCESS_ERROR(m_pOwner);

    KG_PROCESS_ERROR(m_pOwner->m_pAITeammate == this);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::IsNpc()
{
    return (m_pOwner == NULL);
}

BOOL KHero::UpdateVelocityByAnchors()
{
    BOOL     bResult = false;
    KAnchor* pAnchor = NULL;

    if(!m_pCastingSkill)
        goto Exit1;

    pAnchor = m_pCastingSkill->GetAnchorAtFrame(m_nVirtualFrame - m_nAttackSkillTime);
    KG_PROCESS_SUCCESS(!pAnchor);

    m_nVelocityX = m_eFaceDir == csdRight ? pAnchor->nVelocityX : -pAnchor->nVelocityX;
    m_nVelocityY = pAnchor->nVelocityY;
    m_nVelocityZ = pAnchor->nVelocityZ;
    
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::IsVip()
{
    if (m_pOwner)
        return m_pOwner->IsVIP();
    return false;
}

int KHero::GetVIPLevel()
{
    int nVIPLevel = 0;
    if (m_pOwner)
    {
        nVIPLevel = m_pOwner->m_nVIPLevel;
        goto Exit0;
    }

    if (m_pScene->m_Param.m_eBattleType == KBATTLE_TYPE_LADDER_PVP)
    {
        KGLOG_PROCESS_ERROR(m_nSide >= 0 && m_nSide < sidTotal);
        KGLOG_PROCESS_ERROR(m_nPos >= 0 && m_nPos < MAX_TEAM_MEMBER_COUNT);

        nVIPLevel = m_pScene->m_Param.m_byVIPLevel[m_nSide][m_nPos];
    }

Exit0:
    return nVIPLevel;
}

int KHero::GetNormalLevel()
{
    return m_nLevel;
}

int KHero::GetLadderLevel()
{
    int nLadderLevel = 0;
    if (m_pOwner)
    {
        KHeroData* pHeroData = m_pOwner->m_HeroDataList.GetHeroData(m_dwTemplateID);
        KGLOG_PROCESS_ERROR(pHeroData);
        nLadderLevel = pHeroData->m_nLadderLevel;
        goto Exit0;
    }

    KG_PROCESS_ERROR(m_pScene->m_Param.m_eBattleType == KBATTLE_TYPE_LADDER_PVP);
    KGLOG_PROCESS_ERROR(m_nSide >= 0 && m_nSide < sidTotal);
    KGLOG_PROCESS_ERROR(m_nPos >= 0 && m_nPos < MAX_TEAM_MEMBER_COUNT);

    nLadderLevel = m_pScene->m_Param.m_byLadderLevel[m_nSide][m_nPos];
Exit0:
    return nLadderLevel;
}

int KHero::GetCurrentGravity()
{
    if (m_eMoveState == cmsOnStiff)
        return 0;

    int nGravityBase = 0;
    assert(m_pScene);

    nGravityBase = m_pScene->m_nGlobalGravity + m_nAddGravityBase;
    return nGravityBase * (KILO_NUM + m_nGravityPercent) / KILO_NUM;
}

BOOL KHero::OwnerOperateAITeammate(int nOPIdx)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    int nTopIndex = 0;

    KGLOG_PROCESS_ERROR(m_bAiMode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(this);
    g_pSO3World->m_ScriptManager.Push(nOPIdx);
    g_pSO3World->m_ScriptManager.Call(AI_TEAMMATE_OPERATION_FILE, "CallOperation", 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

    bResult = true;
Exit0:
    return bResult;
}

DWORD KHero::GetInterferenceID()
{
    DWORD dwResult = ERROR_ID;
    int nInterference = 0;

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;

        KHero* pHero = (KHero*)pObj;
        int nDistance = g_GetDistance2(m_nX, m_nY, pHero->m_nX, pHero->m_nY);
        if (nDistance > pHero->m_nInterferenceRange * pHero->m_nInterferenceRange)
            continue;

        if (pHero->m_nInterference <= nInterference)
            continue;

        dwResult = pHero->m_dwID;
        nInterference = pHero->m_nInterference;
    }
    return dwResult;
}

BOOL KHero::NormalShootBall(int nAimLoop, DWORD dwInterferenceHeroID)
{
    BOOL            bResult             = false;
    BOOL            bRetCode            = false;
    KBasketSocket*  pBasketSocket       = NULL;
    KBall*          pBall               = NULL;
    KHero*          pInterferencer      = NULL;
    int             nInterference       = 0;

    KGLOG_PROCESS_ERROR(m_eMoveState == cmsOnAimat);

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    bRetCode = pBall->IsTokenBy(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    StopMove(false);

    m_nShootTime = nAimLoop;

    pBall->m_eLastBeShootType = stNormalShoot;

    pBasketSocket = m_pScene->GetShootBallBasketSocket(m_eFaceDir, nAimLoop);
    KGLOG_PROCESS_ERROR(pBasketSocket);

    if (dwInterferenceHeroID)
    {
        pInterferencer = m_pScene->GetHeroById(dwInterferenceHeroID);
        KGLOG_PROCESS_ERROR(pInterferencer);

        nInterference = pInterferencer->m_nInterference;
    }

    bRetCode = pBall->BeShoot(this, pBasketSocket, nInterference);
    if(!bRetCode)
    {
        g_PlayerServer.DoSyncSceneObject(pBall);
        KGLOG_PROCESS_ERROR(false);
    }
    pBall->m_nBaseRandomSeed = m_nVirtualFrame;
    pBall->m_nHitRate   = m_nHitRate;

    ForbitAutoTakeBall(2);

    g_PlayerServer.DoNormalShootBall(this, pBasketSocket, dwInterferenceHeroID);

    if (pBasketSocket->m_nSide == m_nSide)
    {
        m_GameStat.m_nNormalShoot += 1;
    }

    m_pScene->OnBallShot(this, stNormalShoot);

	if (m_pOwner)
	{
		if (m_pScene->m_Param.m_eBattleType == KBATTLE_TYPE_MISSION)
		{
			KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(m_pScene->m_dwMapID);
			KGLOG_PROCESS_ERROR(pMission);
			m_pOwner->OnEvent(peUseNormalShoot, pBasketSocket->m_nSide == m_nSide, pBasketSocket->m_eFloor, m_pScene->m_Param.m_eBattleType, pMission->eType, pMission->nStep, pMission->nLevel);
		}
		else
		{
			m_pOwner->OnEvent(peUseNormalShoot, pBasketSocket->m_nSide == m_nSide, pBasketSocket->m_eFloor, m_pScene->m_Param.m_eBattleType, m_pScene->m_dwMapID);
		}
	}

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::SkillShootBall(DWORD dwTargetBasketSocketID)
{
    BOOL            bResult  = false;
    BOOL            bRetCode = false;
    KBall*          pBall    = NULL;
    KBasketSocket*  pSocket  = NULL;

    pSocket = m_pScene->GetBasketSocketByID(dwTargetBasketSocketID);
    KGLOG_PROCESS_ERROR(pSocket);

    KGLOG_PROCESS_ERROR(m_eMoveState == cmsOnAimat || m_eMoveState == cmsOnStand || m_eMoveState == cmsOnWalk || m_eMoveState == cmsOnJump || m_eMoveState == cmsOnSit);

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    bRetCode = CanCastSpecialSkill(essSkillShoot);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = IsStandOnObj() || !IsInAir();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pBall->IsTokenBy(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    StopMove(false);

    pBall->m_eLastBeShootType = stSkillShoot;
    bRetCode = pBall->SkillShoot(this, pSocket);
    KGLOG_PROCESS_ERROR(bRetCode);
    pBall->m_nBaseRandomSeed = m_nVirtualFrame;
    pBall->m_nHitRate = m_nHitRate;

    g_PlayerServer.DoSkillShootBall(this, pSocket);

    if (pSocket->m_nSide == m_nSide)
    {
        m_GameStat.m_nSkillShoot += 1;
    }

    m_pScene->OnBallShot(this, stSkillShoot);
    OnCastSpecialSkill(essSkillShoot);

	if (m_pOwner)
	{
		if (m_pScene->m_Param.m_eBattleType == KBATTLE_TYPE_MISSION)
		{
			KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(m_pScene->m_dwMapID);
			KGLOG_PROCESS_ERROR(pMission);
			m_pOwner->OnEvent(peUseSkillShoot, pSocket->m_nSide == m_nSide, pSocket->m_eFloor, m_pScene->m_Param.m_eBattleType, pMission->eType, pMission->nStep, pMission->nLevel);
		}
		else
		{
			m_pOwner->OnEvent(peUseSkillShoot, pSocket->m_nSide == m_nSide, pSocket->m_eFloor, m_pScene->m_Param.m_eBattleType, m_pScene->m_dwMapID);
		}
	}

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::SkillSlamBall(DWORD dwTargetBasketSocketID)
{
    BOOL            bResult  = false;
    BOOL            bRetCode = false;
    KBall*          pBall    = NULL;
    KBasketSocket*  pSocket  = NULL;

    pSocket = m_pScene->GetBasketSocketByID(dwTargetBasketSocketID);
    KGLOG_PROCESS_ERROR(dwTargetBasketSocketID);

    KGLOG_PROCESS_ERROR(m_eMoveState == cmsOnAimat || m_eMoveState == cmsOnSit);

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    bRetCode = pBall->IsTokenBy(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    StopMove(false);

    g_PlayerServer.DoSkillSlamBall(this, pSocket);

    pBall->m_eLastBeShootType = stSkillSlam;
    bRetCode = pBall->SkillSlam(this, pSocket);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_bSkillSlaming = true;

    if (pSocket->m_nSide == m_nSide)
    {
        m_GameStat.m_nSkillShoot += 1;
    }

    m_pScene->OnBallShot(this, stSkillSlam);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::ShootBallBySkill(DWORD dwPathID, BOOL bFollowBall)
{
    BOOL            bResult  = false;
    BOOL            bRetCode = false;
    KBall*          pBall    = NULL;

    KGLOG_PROCESS_ERROR(m_pCastingSkill);
    KGLOG_PROCESS_ERROR(m_pCastingSkill->m_bCanCastWithBall);

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    bRetCode = pBall->IsTokenBy(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pBall->ShootWithComplexPath(this, dwPathID);
    KGLOG_PROCESS_ERROR(bRetCode);

    ForbitAutoTakeBall(GAME_FPS);

    pBall->m_nBaseRandomSeed = m_nVirtualFrame;
    pBall->m_nHitRate   = m_nHitRate;
    
    m_pScene->OnBallShot(this, stShootBySkill);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::NormalSlamBall()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KTWO_DIRECTION eDir = csdInvalid;
    KBasketSocket* pSocket = NULL;
    KBasket* pBasket = NULL;
    KBall* pBall = NULL;

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    KGLOG_PROCESS_ERROR(m_eMoveState == cmsOnAimat || m_eMoveState == cmsOnJump || m_eMoveState == cmsOnStand);

    StopMove(false);

    m_nShootTime = 0;

    pSocket = m_pScene->GetSlamDunkBasketSokcet(this);
    KGLOG_PROCESS_ERROR(pSocket);

    bRetCode = pBall->CanSlam(this, pSocket);
    KG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoNormalSlamBall(this, pSocket);

    pBall->m_eLastBeShootType = stSlamDunk;
    bRetCode = pBall->SlamDunk(this, pSocket);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (pSocket->m_nSide == m_nSide)
    {
        m_GameStat.m_nSlamBall += 1;
    }

    m_pScene->OnBallShot(this, stSlamDunk);

    bResult = true;
Exit0:
    return bResult;
}

void KHero::StopMove(BOOL bSyncSelf)
{
    if (IsInAir())
    {
        SetMoveState(cmsOnJump);
    }
    else
    {
        Stand(bSyncSelf);
    }
}

BOOL KHero::ApplyGeneralAffect(DWORD dwCasterID, KMovableObject* pAttackerObj, DWORD dwAffectID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nAgility = 0;
    int  nRandom  = 0;
    int  nAddAngry = 0;
    KGeneralAffect* pAffect = NULL;
    KHero* pCaster = NULL;
    int nBeAttackedType = DEFAULT_ATTACK_TYPE;

    KG_PROCESS_SUCCESS(dwAffectID == ERROR_ID);

    KGLOG_PROCESS_ERROR(pAttackerObj);

    pAffect = g_pSO3World->m_Settings.m_AffectSetting.GetGeneralAffect(dwAffectID);
    KGLOG_PROCESS_ERROR(pAffect);

    if (dwCasterID)
    {
        BOOL    bEnduranceCrited    = false;
        int     nEnduranceDamage    = 0;
        BOOL    bStaminaCrited      = false;
        int     nStaminaDamage      = 0;

        pCaster = m_pScene->GetHeroById(dwCasterID);
        KGLOG_PROCESS_ERROR(pCaster);

        assert(m_nLevel > 0);

        nAgility = (int)(pCaster->m_nCritPoint * (0.61 * m_nLevel * m_nLevel - 72.5 * m_nLevel - 0.45 * MAX(m_nLevel - 50, 0) * MAX(m_nLevel - 50, 0) - 
            0.085 * MAX(m_nLevel - 70, 0) * MAX(m_nLevel - 70, 0) + 2450) / 1000);
        if (nAgility < 0)
            nAgility = 0;

        nAgility += pCaster->m_nAgility;
        if (nAgility > cdMaxAgility)
            nAgility = cdMaxAgility;

        nEnduranceDamage = (pCaster->m_nAttackPoint * (HUNDRED_NUM + pCaster->m_nAttackPointPercent) * pAffect->nCostEndurancePercent) / (HUNDRED_NUM * KILO_NUM) 
            + pAffect->nCostEndurance;

        nRandom = g_Random(KILO_NUM);
        if (nRandom < nAgility) // 暴击
        {
            bEnduranceCrited = true;
            nEnduranceDamage = nEnduranceDamage * m_nCritRate / HUNDRED_NUM;
        }

        nEnduranceDamage = ProcessEnduranceRemission(nEnduranceDamage, pCaster);
        KGLOG_PROCESS_ERROR(nEnduranceDamage >= 0);

        if (IsInAir())
        {
            nEnduranceDamage = nEnduranceDamage * g_pSO3World->m_Settings.m_ConstList.nEnduranceCoeInAir / HUNDRED_NUM;
        }

        LoseEndurance(nEnduranceDamage);

        if (m_nMaxEndurance > 0)
        {
            nAddAngry = 1000 * nEnduranceDamage / m_nMaxEndurance / 12;
            bRetCode = AddAngry(nAddAngry);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        // 体力伤害
        nStaminaDamage = (pCaster->m_nAttackPoint * (HUNDRED_NUM + pCaster->m_nAttackPointPercent)  * pAffect->nCostStaminaPercent) / (HUNDRED_NUM * KILO_NUM)
            + pAffect->nCostStamina;
        nRandom = g_Random(KILO_NUM);
        if (nRandom < nAgility) // 暴击
        {
            bStaminaCrited = true;
            nStaminaDamage = nStaminaDamage * m_nCritRate / HUNDRED_NUM;
        }
        nStaminaDamage = ProcessStaminaRemission(nStaminaDamage, pCaster);
        KGLOG_PROCESS_ERROR(nStaminaDamage >= 0);

        LoseStamina(nStaminaDamage);

		if (pAttackerObj)
		{
			if (g_IsDoodad(pAttackerObj) && pAttackerObj->m_pTemplate)
			{
				nBeAttackedType = pAttackerObj->m_pTemplate->m_nBeAttackedType;
			}
			else if (pAttackerObj->GetType() == sotBullet)
			{
				KBullet* pBullet = (KBullet*)pAttackerObj;
				if (pBullet->m_pBulletTemplate)
					nBeAttackedType = pBullet->m_pBulletTemplate->nBeAttackType;
			}
		}

        g_PlayerServer.DoSyncDamage(this, bEnduranceCrited, nEnduranceDamage, bStaminaCrited, nStaminaDamage, nBeAttackedType);
    }

    BeAttacked(pAttackerObj);

    // 检查是否要进入僵直
    StartStiff(pAffect->nStiffFrame);

    AddBuff(pAffect->dwBuffID);
    for(KVEC_ATTRIBUTES::const_iterator constIt = pAffect->vecAttributes.begin(); constIt != pAffect->vecAttributes.end(); ++constIt)
    {
        CallAttributeFunction(constIt->nKey, true, constIt->nValue1, constIt->nValue2);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::PrepareToCastSkill()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = EnsureNotCastingSkill();
    KGLOG_PROCESS_ERROR(bRetCode);

    m_AttackObjList.clear();

    bRetCode = IsInAir();
    if (bRetCode)
    {
        SetMoveState(cmsOnJump);
        m_nJumpCount = 1;
    }
    else
    {
        ClearVelocity();
        SetMoveState(cmsOnStand);
        m_nMoveFrameCounter = 0;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::ProcessSkillSegment()
{
    BOOL bResult     = false;
    BOOL bRetCode    = false;
    KSkillSegmentInfo* pSegmentInfo = NULL;

    KGLOG_PROCESS_ERROR(m_pCastingSkill);

    pSegmentInfo = m_pCastingSkill->GetSegmentAtFrame(m_nVirtualFrame - m_nAttackSkillTime);
    KG_PROCESS_SUCCESS(!pSegmentInfo);

    m_AttackObjList.clear();

    bRetCode = FireBullets(pSegmentInfo->vecBullet);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = ApplyBuffAndAttributes(pSegmentInfo->dwSelfBuffID, pSegmentInfo->vecSelfAttributes);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_pCastingSkill->m_eSkillType == skilltypeGrab && m_dwGrabHeroID)
    {
        KHero* pGrabHero = m_pScene->GetHeroById(m_dwGrabHeroID);
        if (pGrabHero)
        {
            pGrabHero->m_bInvincible = false;
            pGrabHero->SetPosition(m_nX, m_nY, m_nZ);
            pGrabHero->BeAttackedByHero(this, m_pCastingSkill, pSegmentInfo);
        }
        m_dwGrabHeroID = ERROR_ID;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::ApplyBuffAndAttributes(DWORD dwBuffID, const KVEC_ATTRIBUTES& rvecAttributes)
{
    AddBuff(dwBuffID);
    for (KVEC_ATTRIBUTES::const_iterator cosntIt = rvecAttributes.begin(); cosntIt != rvecAttributes.end(); ++cosntIt)
    {
        CallAttributeFunction(cosntIt->nKey, true, cosntIt->nValue1, cosntIt->nValue2);
    }

    return true;
}

BOOL KHero::ApplyBuffAndAttributes(DWORD dwBuffID, const KAttribute* pListHead)
{
    AddBuff(dwBuffID);
    for (; pListHead; pListHead = pListHead->pNext)
    {
        CallAttributeFunction(pListHead->nKey, true, pListHead->nValue1, pListHead->nValue2);
    }

    return true;
}

KBullet* KHero::FireBullet(DWORD dwBulletTemplateID)
{
    KBullet*        pResult     = NULL;
    BOOL            bRetCode    = false;
    KBullet*        pNewBullet  = NULL;
    KBulletInfo*    pBulletInfo = NULL;
    KVELOCITY       cVelocity = {0, 0, 0};
    KPOSITION       cPos        = GetBodyCenter();

    KG_PROCESS_SUCCESS(!dwBulletTemplateID);

    pBulletInfo = g_pSO3World->m_BulletMgr.GetBulletInfo(dwBulletTemplateID);
    KGLOG_PROCESS_ERROR(pBulletInfo);

    pNewBullet = (KBullet*)m_pScene->m_pSceneObjMgr->AddByType(sotBullet);
    KGLOG_PROCESS_ERROR(pNewBullet);

    pNewBullet->ApplyBulletInfo(pBulletInfo);

    cPos.nX += (m_eFaceDir == csdRight ? pBulletInfo->nOffsetX : -pBulletInfo->nOffsetX);
    cPos.nY += pBulletInfo->nOffsetY;
    cPos.nZ += pBulletInfo->nOffsetZ;

    cVelocity.nVelocityX = (m_eFaceDir == csdRight ? pBulletInfo->nVelocityX : - pBulletInfo->nVelocityX);
    cVelocity.nVelocityY = pBulletInfo->nVelocityY;
    cVelocity.nVelocityZ = pBulletInfo->nVelocityZ;

    pNewBullet->SetPosition(cPos);

    pNewBullet->m_nVelocityX = cVelocity.nVelocityX;
    pNewBullet->m_nVelocityY = cVelocity.nVelocityY;
    pNewBullet->m_nVelocityZ = cVelocity.nVelocityZ;

    pNewBullet->m_nSide        = m_nSide;
    pNewBullet->m_dwCasterID   = m_dwID;
    pNewBullet->m_pSkill       = NULL;
    pNewBullet->m_nStartFrame  = g_pSO3World->m_nGameLoop;

Exit1:
    pResult = pNewBullet;
Exit0:
    if (!pResult && pNewBullet)
    {
        m_pScene->m_pSceneObjMgr->Delete(pNewBullet->m_dwID);
        pNewBullet = NULL;
    }
    return pResult;
}

BOOL KHero::FireBullets(const KVEC_BULLETS& rvecBullets)
{
    BOOL bResult  = false;
    KBullet* pBullet = NULL;

    for (KVEC_BULLETS::const_iterator constIt = rvecBullets.begin(); constIt != rvecBullets.end(); ++constIt)
    {
        pBullet = FireBullet(*constIt);
        KGLOG_PROCESS_ERROR(pBullet);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::DoRestraintJudge(KHero* pTarget, BOOL& bMeAttackTarget, BOOL& bTargetAttackMe)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KBODY cJudgeBox;
    KBODY cSelfBody;

    KGLOG_PROCESS_ERROR(pTarget);
    KGLOG_PROCESS_ERROR(m_pCastingSkill);

    if (!pTarget->m_pCastingSkill)
    {
        bMeAttackTarget = true;
        bTargetAttackMe = false;
        goto Exit1;
    }

    if (m_pCastingSkill->m_nRestraintType + 1 % KRESTRAINT_TYPE_TOTAL == pTarget->m_pCastingSkill->m_nRestraintType)
    {
        bMeAttackTarget = true;
        bTargetAttackMe = false;
        goto Exit1;
    }

    bRetCode = pTarget->m_pCastingSkill->IsAttackStateAtFrame(pTarget->m_nVirtualFrame - pTarget->m_nAttackSkillTime);
    if (!bRetCode)
    {
        bMeAttackTarget = true;
        bTargetAttackMe = false;
        goto Exit1;
    }

    bRetCode = pTarget->m_pCastingSkill->GetJudageBox(pTarget, cJudgeBox);
    KGLOG_PROCESS_ERROR(bRetCode);

    cSelfBody = GetBody();

    bRetCode = g_IsCollide(cJudgeBox, cSelfBody);
    if (!bRetCode)
    {
        bMeAttackTarget = true;
        bTargetAttackMe = false;
        goto Exit1;
    }

    if (pTarget->m_pCastingSkill->m_nRestraintType + 1 % KRESTRAINT_TYPE_TOTAL == m_pCastingSkill->m_nRestraintType)
    {
        bMeAttackTarget = false;
        bTargetAttackMe = (pTarget->m_pCastingSkill->m_nRestraintType != KRESTRAINT_TYPE_GRAB);
        goto Exit1;
    }

    bMeAttackTarget = true;
    bTargetAttackMe = (pTarget->m_pCastingSkill->m_nRestraintType != KRESTRAINT_TYPE_GRAB);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KHero::ProcessAutoMove()
{
    BOOL bRetCode = false;

    if(m_eAutoMoveStatus == KAUTOMOVE_STATUS_NONE)
        goto Exit0;

    if (m_pCastingSkill)
    {
        m_eAutoMoveStatus = KAUTOMOVE_STATUS_WAITING_PATH;
        goto Exit0;
    }

    if (m_eMoveState != cmsOnStand && m_eMoveState != cmsOnWalk && m_eMoveState != cmsOnRun)
    {
        m_eAutoMoveStatus = KAUTOMOVE_STATUS_WAITING_PATH;
        goto Exit0;
    }

    bRetCode = IsAroundOfAutoMovingTarget();
    if (bRetCode)
    {
        FinishAutoMove();
        FireAIEvent(aevOnAutoMoveSuccess, 0, 0);
        goto Exit0;
    }

    if(m_eAutoMoveStatus == KAUTOMOVE_STATUS_WAITING_PATH)
    {
        bRetCode = DirectlyMoveToDest();
        if (!bRetCode)
            FireAIEvent(aevOnAutoMoveFailed, 0, 0);

        goto Exit0;
    }

    if (m_eAutoMoveStatus == KAUTOMOVE_STATUS_DIRECTMOVING)
    {
        bRetCode = IsTargetMoveFarway();
        if (bRetCode)
        {
            bRetCode = DirectlyMoveToDest();
            if (!bRetCode)
                FireAIEvent(aevOnAutoMoveFailed, 0, 0);

            goto Exit0;
        }

        bRetCode = CanCellAtNextFrameReachable();
        if (!bRetCode)
        {
            bRetCode = StartAutoMove();
            if (!bRetCode)
            {
                Stand(true);
                FireAIEvent(aevOnAutoMoveFailed, 0, 0);
            }
        }

        goto Exit0;
    }

    if (m_eAutoMoveStatus == KAUTOMOVE_STATUS_AUTOMOVING)
    {
        KCellEx* pCurrent = NULL;
        KCellEx* pNext = NULL;
        KCellEx* pDest = NULL;

        bRetCode = IsTargetMoveFarway();
        if (bRetCode)
        {
            bRetCode = DirectlyMoveToDest();
            if (!bRetCode)
                FireAIEvent(aevOnAutoMoveFailed, 0, 0);

            goto Exit0;
        }

        bRetCode = IsReachedNextCell();
        if (bRetCode)
        {
            pCurrent = m_lstAutoMovePath.front();
            m_lstAutoMovePath.pop_front();

            if (m_lstAutoMovePath.size())
                bRetCode = MoveToNextCell(pCurrent);
            else
                bRetCode = DirectlyMoveToDest();

            if (!bRetCode)
                FireAIEvent(aevOnAutoMoveFailed, 0, 0);

            goto Exit0;
        }

        bRetCode = CanCellAtNextFrameReachable();
        if (bRetCode)
            goto Exit0;

        bRetCode = StartAutoMove();
        if (!bRetCode)
            FireAIEvent(aevOnAutoMoveFailed, 0, 0);

        goto Exit0;
    }

Exit0:
    return;
}

void KHero::FinishAutoMove()
{
    if (m_eAutoMoveStatus != KAUTOMOVE_STATUS_NONE)
    {
        m_eAutoMoveStatus = KAUTOMOVE_STATUS_NONE;
        m_lstAutoMovePath.clear();
        StopMove(true);
    }
}

BOOL KHero::CanCellAtNextFrameReachable()
{
    BOOL bResult  = false;
    KCellEx* pCurrentCell = NULL;
    KCellEx* pNextFrameCell = NULL;
    KCellEx* pCellOnPath = NULL;
    int nSrcHeight = 0;
    int nDstHeight = 0;
    int nStepX = 1;
    int nStepY = 1;

    pCurrentCell = m_pScene->GetCellEx(m_nX, m_nY);
    KGLOG_PROCESS_ERROR(pCurrentCell);

    pNextFrameCell = m_pScene->GetCellEx(m_nX + m_nVelocityX, m_nY + m_nVelocityY);
    KG_PROCESS_ERROR(pNextFrameCell);

    if (pCurrentCell == pNextFrameCell)
        goto Exit1;

    nSrcHeight = m_pScene->GetPosHeight(m_nX, m_nY);

    if (pNextFrameCell->nX - pCurrentCell->nX < 0)
        nStepX = -1;

    if (pNextFrameCell->nY - pCurrentCell->nY < 0)
        nStepY = -1;

    for (int nCellX = pCurrentCell->nX; nStepX * nCellX <= nStepX * pNextFrameCell->nX; nCellX += nStepX)
    {
        for (int nCellY = pCurrentCell->nY; nStepY * nCellY <= nStepY * pNextFrameCell->nY; nCellY += nStepY)
        {
            if (nCellX == pCurrentCell->nX && nCellY == pCurrentCell->nY)
                continue;

            pCellOnPath = m_pScene->m_PathFinder.GetCell(nCellX, nCellY);
            KGLOG_PROCESS_ERROR(pCellOnPath);

            if (pCellOnPath->bInitObstacle)
                goto Exit0;

            if (pCurrentCell->bObstacle)
            {
                if (pCellOnPath->bObstacle)
                {
                    nDstHeight = m_pScene->GetPosHeight(nCellX * CELL_LENGTH + CELL_LENGTH / 2, nCellY * CELL_LENGTH + CELL_LENGTH / 2);
                    if (nSrcHeight < nDstHeight)
                        goto Exit0;
                }
            }
            else
            {
                if (pCellOnPath->bObstacle)
                    goto Exit0;
            }
        }
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::StartAutoMove()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KCellEx* pCurrent = NULL;
    KCellEx* pDest = NULL;
    int nDestX = 0;
    int nDestY = 0;
    int nDestZ = 0;

    m_eAutoMoveStatus = KAUTOMOVE_STATUS_NONE;

    bRetCode = GetAITargetPos(nDestX, nDestY, nDestZ);
    if (!bRetCode)
        goto Exit0;

    pDest = m_pScene->GetCellEx(nDestX, nDestY);
    KGLOG_PROCESS_ERROR(pDest);

    pCurrent = m_pScene->GetCellEx(m_nX, m_nY);
    KGLOG_PROCESS_ERROR(pCurrent);

    if (pDest == pCurrent)
    {
        bRetCode = DirectlyMoveToDest();
        KGLOG_PROCESS_ERROR(bRetCode);

        m_eAutoMoveStatus = KAUTOMOVE_STATUS_DIRECTMOVING;
        goto Exit1;
    }

    bRetCode = m_pScene->m_PathFinder.FindPath(pCurrent, pDest, m_lstAutoMovePath, true);
    if (!bRetCode)
    {
        bRetCode = m_pScene->m_PathFinder.FindPath(pCurrent, pDest, m_lstAutoMovePath, false);
    }
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = MoveToNextCell(pCurrent);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_nDestX = nDestX;
    m_nDestY = nDestY;
    m_eAutoMoveStatus = KAUTOMOVE_STATUS_AUTOMOVING;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::MoveToNextCell(KCellEx* pCurrent)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KCellEx* pNext = NULL;
    int nDirection = 0;

    KGLOG_PROCESS_ERROR(m_lstAutoMovePath.size());

    pNext = m_lstAutoMovePath.front();

    bRetCode = m_pScene->WillPassingCell(m_nX, m_nY, m_nVelocityX, m_nVelocityY, pNext);
    KG_PROCESS_SUCCESS(bRetCode);

    nDirection = g_GetDirection(pCurrent->nX, pCurrent->nY, pNext->nX, pNext->nY);
    if (m_bWalkAutoMove)
        bRetCode = WalkTo(nDirection, true);
    else
        bRetCode = RunTo(nDirection, true);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_eAutoMoveStatus = KAUTOMOVE_STATUS_AUTOMOVING;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::UpdateAutoMoveParam(int nAutoMoveAroundX, int nAutoMoveAroundY, BOOL bWalk)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nDestX   = 0;
    int  nDestY   = 0;
    int  nDestZ   = 0;

    bRetCode = GetAITargetPos(nDestX, nDestY, nDestZ);
    KG_PROCESS_ERROR(bRetCode);

    m_nAutoMoveAroundX  = nAutoMoveAroundX;
    m_nAutoMoveAroundY  = nAutoMoveAroundY;
    m_bWalkAutoMove     = bWalk;	

    bResult = true;
Exit0:
    return bResult;

}

void KHero::OnHpDecrease()
{
    int                     nStaminaPercent = 0;
    int                     nLeftHP = 0;

    //计算当前体力百分比
    nStaminaPercent = (m_nCurrentStamina * 100) / m_nMaxStamina;
    KG_PROCESS_ERROR(nStaminaPercent < m_nLastCalcStaminaPercent);

    DropGold(m_nLastCalcStaminaPercent, nStaminaPercent);
    TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_HP, m_nLastCalcStaminaPercent, nStaminaPercent);

    m_nLastCalcStaminaPercent = nStaminaPercent;

Exit0:
    return;
}

void KHero::DropGold(int nOldStaminaPercent, int nNewStaminaPercent)
{
    KSceneObject*           pObj = NULL;
    KSceneObjectTemplate*   pTemplate = NULL;
    DWORD                   dwGoldID = 0;
    int                     nGold = 0;
    int                     nGoldChange = 0;
    int                     nGoldValue = 0;
    int                     nLeftHP = 0;

    KG_PROCESS_ERROR(IsNpc());
    KG_PROCESS_ERROR(m_pNpcTemplate);
    KG_PROCESS_ERROR(m_pScene->m_Param.m_eBattleType == KBATTLE_TYPE_MISSION);
    KG_PROCESS_ERROR(m_pNpcTemplate->nAttackAwardHP[0] > 0);

    //计算当前体力百分比
    KG_PROCESS_ERROR(nOldStaminaPercent > nNewStaminaPercent);

    for (int i = 0; i < MAX_ATTACK_AWARD; ++i)
    {
        nLeftHP = 100 - m_pNpcTemplate->nAttackAwardHP[i];
        if (nLeftHP >= nNewStaminaPercent && nLeftHP < nOldStaminaPercent)
        {
            //丢钱出来
            if (m_pNpcTemplate->nAttackAwardMoney[i] > 0)
                nGold += m_pNpcTemplate->nAttackAwardMoney[i];
        }
    }

    //上下浮动10%
    nGoldChange = (nGold * g_Random(11)) / 100;
    if (g_RandPercent(50))
        nGoldChange *= -1;

    nGold += nGoldChange;
    KG_PROCESS_ERROR(nGold > 0);

    nGoldValue = BIG_GOLD_MONEY;
    dwGoldID = g_pSO3World->m_Settings.m_ConstList.nScenenBigGoldID;
    pTemplate = g_pSO3World->m_Settings.m_ObjTemplate.GetTemplate(dwGoldID);

    if (nGold < BIG_GOLD_MONEY)
    {
        nGoldValue = SML_GOLD_MONEY;
        dwGoldID = g_pSO3World->m_Settings.m_ConstList.nScenenSmallGoldID;
        pTemplate = g_pSO3World->m_Settings.m_ObjTemplate.GetTemplate(dwGoldID);
    }
    KGLOG_PROCESS_ERROR(pTemplate);
    KGLOG_PROCESS_ERROR(pTemplate->m_nType == sotGold);

    while(nGold >= nGoldValue)
    {
        nGold -= nGoldValue;
        pObj = m_pScene->CreateGold(dwGoldID, m_nX, m_nY, m_nZ, nGoldValue);
        KG_PROCESS_ERROR(pObj);
    }
    KG_PROCESS_ERROR(nGold > 0);
    pObj = m_pScene->CreateGold(dwGoldID ,m_nX, m_nY, m_nZ, nGoldValue);
    KG_PROCESS_ERROR(pObj);

Exit0:
    return;
}


BOOL KHero::MoveTo(int nDestX, int nDestY, BOOL bWalk)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nDirection = 0;

    nDirection = g_GetDirection(m_nX, m_nY, nDestX, nDestY);
    if (bWalk)
        bRetCode = WalkTo(nDirection, true);
    else
        bRetCode = RunTo(nDirection, true);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::IsAroundOfAutoMovingTarget()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nDestX = 0;
    int nDestY = 0;
    int nDestZ = 0;

    bRetCode = GetAITargetPos(nDestX, nDestY, nDestZ);
    KG_PROCESS_ERROR(bRetCode);

    if (abs(nDestX - m_nX) <= m_nAutoMoveAroundX && abs(nDestY - m_nY) <= m_nAutoMoveAroundY)
        bResult = true;

Exit0:
    return bResult;
}

BOOL KHero::DirectlyMoveToDest()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nDestX = 0;
    int nDestY = 0;
    int nDestZ = 0;
    int nDirection = 0;

    m_eAutoMoveStatus = KAUTOMOVE_STATUS_NONE;

    bRetCode = GetAITargetPos(nDestX, nDestY, nDestZ);
    KG_PROCESS_ERROR(bRetCode);

    nDirection = g_GetDirection(m_nX, m_nY, nDestX, nDestY);
    if (m_bWalkAutoMove)
        bRetCode = WalkTo(nDirection, true);
    else
        bRetCode = RunTo(nDirection, true);
    KG_PROCESS_ERROR(bRetCode);

    m_nDestX = nDestX;
    m_nDestY = nDestY;

    m_eAutoMoveStatus = KAUTOMOVE_STATUS_DIRECTMOVING;
    bResult = true;
Exit0:
    return bResult;
} 

BOOL KHero::IsReachedNextCell()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KCellEx* pCurrent = NULL;
    KCellEx* pNext    = NULL;

    pCurrent = m_pScene->GetCellEx(m_nX, m_nY);
    KGLOG_PROCESS_ERROR(pCurrent);

    KGLOG_PROCESS_ERROR(m_lstAutoMovePath.size());
    pNext = m_lstAutoMovePath.front();

    if (pCurrent == pNext)
        bResult = true;

Exit0:
    return bResult;
}

BOOL KHero::IsTargetMoveFarway()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nDestX = 0;
    int nDestY = 0;
    int nDestZ = 0;

    bRetCode = GetAITargetPos(nDestX, nDestY, nDestZ);
    if (!bRetCode)
        goto Exit1;

    if (abs(m_nDestX - nDestX) > KAUTOMOVE_MAX_DELTAX)
        goto Exit1;

    if (abs(m_nDestY - nDestY) > KAUTOMOVE_MAX_DELTAY)
        goto Exit1;

    goto Exit0;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KHero::StopAutoMove()
{
    m_eAutoMoveStatus = KAUTOMOVE_STATUS_NONE;
    StopMove(true);
}

BOOL KHero::FixAutoTakeBallBug()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KBall* pBall = NULL;

    bRetCode = CanTakeDoodad();
    KG_PROCESS_ERROR(bRetCode);

    pBall = m_pScene->GetBall();
    KG_PROCESS_ERROR(pBall);

    bRetCode = pBall->CanBeTokenBy(this);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = IsSceneObjInCollidedList(pBall->m_dwID);
    KG_PROCESS_ERROR(!bRetCode);

    bRetCode = InsertSceneObjInCollidedList(pBall->m_dwID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::TriggerPassiveSkill(int nEvent, int nOldValue/* =0 */, int nNewValue/* =0 */)
{
    BOOL bTriggerFlag = false;
    KPassiveSkill* pPassiveSkill = NULL;

    for (int i = 0; i < cdPassiveSkillSlotCount; ++i)
    {
        pPassiveSkill = &m_PassiveSkillUsed[i];

        if (pPassiveSkill->CanTrigger(nEvent, nOldValue, nNewValue))
        {
            pPassiveSkill->Action();
            bTriggerFlag = true;
        }
    }

    return bTriggerFlag;
}


BOOL KHero::ApplyPassiveSkill()
{
    BOOL                bResult  = false;
    BOOL                bRetCode = false;
    KPassiveSkillData*  pPassiveSkillData = NULL;
    DWORD               dwBuffID = 0;
    //创建被动技能
    for (int i = 0; i < cdPassiveSkillSlotCount; ++i)
    {
        m_PassiveSkillUsed[i].UnInit();

        if (m_PassiveSkill[i] == 0)
            continue;

        pPassiveSkillData = g_pSO3World->m_Settings.m_PassiveSkillData.GetByID(m_PassiveSkill[i]);
        if (pPassiveSkillData == NULL)
            continue;

        m_PassiveSkillUsed[i].Init(pPassiveSkillData, this);
    }

    //加永久buf
    for (int i = 0; i < cdPassiveSkillSlotCount; ++i)
    {
        if (m_PassiveSkillUsed[i].IsEmpty())
            continue;

        bRetCode = m_PassiveSkillUsed[i].IsAddBuff();
        if (bRetCode)
            m_PassiveSkillUsed[i].AddBuff();
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::CanCastSlamBallSkill(KSlamBallSkill* pSkill, KBasketSocket* pSocket)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    assert(pSkill);
    assert(pSocket);

    KGLOG_PROCESS_ERROR(m_eMoveState == cmsOnAimat || m_eMoveState == cmsOnSit);

    KGLOG_PROCESS_ERROR(!m_pCastingSkill);

    bRetCode = HasSlamBallSkill(pSkill->dwID);
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLOG_PROCESS_ERROR(pSocket->m_eFloor <= pSkill->nHighestTargetBasket * 2); // 每层有两个篮筐插槽

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::CastSlamBallSkill(DWORD dwSlamBallSkillID, DWORD dwTargetSocketID, DWORD dwInterferenceID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KSlamBallSkill* pSlamBallSkill  = NULL;
    KBasketSocket*  pSocket         = NULL;
    KBall*          pBall           = NULL;
    BOOL            bHit            = false;

    KGLOG_PROCESS_ERROR(dwSlamBallSkillID > 0);

    pSlamBallSkill = g_pSO3World->m_Settings.m_SlamBallSkillSettings.Get(dwSlamBallSkillID);
    KGLOG_PROCESS_ERROR(pSlamBallSkill);

    pSocket = m_pScene->GetBasketSocketByID(dwTargetSocketID);
    KGLOG_PROCESS_ERROR(pSocket);

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    bRetCode = CanCastSlamBallSkill(pSlamBallSkill, pSocket);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pBall->IsTokenBy(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (pSlamBallSkill->bIgnoreInterference)
        dwInterferenceID = ERROR_ID;

    bHit = IsShootBallHitted(dwInterferenceID, pSlamBallSkill->nAdditionalHitRate);
    if (!bHit)
    {
        pBall->m_DelayAction.nFrame = 0;
        pBall->m_DelayAction.nX = pSocket->m_nX;
        pBall->m_DelayAction.nY = pSocket->m_nY;
        pBall->m_DelayAction.nZ = pSocket->m_nZ;
        pBall->m_DelayAction.nVX = GetRandomNum(pSlamBallSkill->nRandomMinVX,   pSlamBallSkill->nRandomMaxVX);
        pBall->m_DelayAction.nVY = GetRandomNum(pSlamBallSkill->nRandomMinVY,   pSlamBallSkill->nRandomMaxVY);
        pBall->m_DelayAction.nVZ = GetRandomNum(pSlamBallSkill->nRandomMinVZ,   pSlamBallSkill->nRandomMaxVZ);

        if (pSocket->m_eDir == csdLeft)
            pBall->m_DelayAction.nVX = -pBall->m_DelayAction.nVX;
    }

    g_PlayerServer.DoCastSlamBallSkill(this, dwSlamBallSkillID, pSocket, dwInterferenceID);

    StopMove(false);

    pBall->m_eLastBeShootType = stSkillSlam;
    bRetCode = pBall->SkillSlamEx(this, pSlamBallSkill, pSocket);
    KGLOG_PROCESS_ERROR(bRetCode);
    pBall->m_nBaseRandomSeed = m_nVirtualFrame;
    pBall->m_nHitRate = m_nHitRate;

    m_bSkillSlaming = true;
    m_bSkillSlamingHit = bHit;
    m_pCastingSlamBallSkill = pSlamBallSkill;
    m_bBaTi = pSlamBallSkill->bBaTi;

    AddAngry(pSlamBallSkill->nAddAngry);

    if (pSocket->m_nSide == m_nSide)
    {
        m_GameStat.m_nSkillShoot += 1;
    }

    m_pScene->OnBallShot(this, stSkillSlam);

	if (m_pOwner)
	{
		if (m_pScene->m_Param.m_eBattleType == KBATTLE_TYPE_MISSION)
		{
			KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(m_pScene->m_dwMapID);
			KGLOG_PROCESS_ERROR(pMission);
			m_pOwner->OnEvent(peUseSkillSlam, pSocket->m_nSide == m_nSide, pSocket->m_eFloor, m_pScene->m_Param.m_eBattleType, pMission->eType, pMission->nStep, pMission->nLevel);
		}
		else
		{
			m_pOwner->OnEvent(peUseSkillSlam, pSocket->m_nSide == m_nSide, pSocket->m_eFloor, m_pScene->m_Param.m_eBattleType, m_pScene->m_dwMapID);
		}
	}

    bResult = true;
Exit0:
    return bResult;
}

int KHero::GetDecreasedHitRate(DWORD dwInterferenceID)
{
    int nResultHitRate = 0;
    KHero* pHero = NULL;

    pHero = m_pScene->GetHeroById(dwInterferenceID);
    KGLOG_PROCESS_ERROR(pHero);

    KG_PROCESS_SUCCESS(pHero->m_nInterference <= m_nWillPower);

    nResultHitRate = (pHero->m_nInterference - m_nWillPower) / 10;
Exit1:
Exit0:
    return nResultHitRate;
}


BOOL KHero::NormalSlamBallEx(DWORD dwSocketID, DWORD dwInterferenceID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KBasketSocket* pSocket = NULL;
    KBall* pBall = NULL;
    int nAddtionalHitRate = 0;
    BOOL bHit = false;

    pBall = m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    pSocket = m_pScene->GetBasketSocketByID(dwSocketID);
    KGLOG_PROCESS_ERROR(pSocket);

    nAddtionalHitRate = g_pSO3World->m_Settings.m_ConstList.nAdditionalNormalSlamBallHitRate;
    bHit = IsShootBallHitted(dwInterferenceID, nAddtionalHitRate);

    bRetCode = pBall->SlamDunk(this, pSocket);
    KGLOG_PROCESS_ERROR(bRetCode);
    pBall->m_nBaseRandomSeed = m_nVirtualFrame;
    pBall->m_nHitRate = m_nHitRate;

    g_PlayerServer.DoNormalSlamBallEx(this, pSocket->m_dwID, dwInterferenceID);

    if (!bHit)
    {
        pBall->m_DelayAction.nFrame = g_pSO3World->m_nGameLoop + g_pSO3World->m_Settings.m_ConstList.nSlamBallReboundFrame;

        pBall->m_DelayAction.nX = pSocket->m_nX;
        pBall->m_DelayAction.nY = pSocket->m_nY;
        pBall->m_DelayAction.nZ = pSocket->m_nZ;
        pBall->m_DelayAction.nVX = GetRandomNum(g_pSO3World->m_Settings.m_ConstList.nNormalSlamBallRandomMinVX,
            g_pSO3World->m_Settings.m_ConstList.nNormalSlamBallRandomMaxVX);
        pBall->m_DelayAction.nVY = GetRandomNum(g_pSO3World->m_Settings.m_ConstList.nNormalSlamBallRandomMinVY,
            g_pSO3World->m_Settings.m_ConstList.nNormalSlamBallRandomMaxVY);
        pBall->m_DelayAction.nVZ = GetRandomNum(g_pSO3World->m_Settings.m_ConstList.nNormalSlamBallRandomMinVZ,
            g_pSO3World->m_Settings.m_ConstList.nNormalSlamBallRandomMaxVZ);

        if (pSocket->m_eDir == csdLeft)
            pBall->m_DelayAction.nVX = -pBall->m_DelayAction.nVX;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::IsShootBallHitted(DWORD dwInterferenceID, int nAddtionalHitRate)
{
    BOOL bResult = false;
    int nNowHitRate = 0;
    KHero* pHero = NULL;

    nNowHitRate = m_nHitRate + nAddtionalHitRate;
    if (dwInterferenceID)
    {
        pHero = m_pScene->GetHeroById(dwInterferenceID);
        if(pHero && pHero->m_nInterference > m_nWillPower)
        {
            nNowHitRate -=  (pHero->m_nInterference - m_nWillPower) / 10;
        }
    }

    if (nNowHitRate < 0)
        nNowHitRate = 0;

    bResult = RandomHitted(nNowHitRate);

    LogInfo("HitRate:%d Frame:%d, Hit:%d", nNowHitRate, m_nVirtualFrame, bResult);
    return bResult;
}

void KHero::TryTriggerGrabSkill()
{
    BOOL bRetCode = false;
    KSkill* pSkill = NULL;
    KHero* pTarget = NULL;
    DWORD dwGrabSkillID = ERROR_ID;
    KBODY cAttackBox;
    KBODY cTargetBoby;

    KG_PROCESS_ERROR(m_bAiMode);

    pSkill = m_pCastingSkill;
    KG_PROCESS_ERROR(pSkill);
    KG_PROCESS_ERROR(pSkill->m_bCauseGrabSkill);

    bRetCode = pSkill->WouldGrabAtFrame(m_nVirtualFrame - m_nAttackSkillTime);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pSkill->GetJudageBox(this, cAttackBox);
    KGLOG_PROCESS_ERROR(bRetCode);

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pSceneObj = it.GetValue();
        if (!pSceneObj->Is(sotHero))
            continue;

        if (pSceneObj->m_dwID == m_dwID)
            continue;

        cTargetBoby = pSceneObj->GetBody();
        bRetCode = g_IsCollide(cAttackBox, cTargetBoby);
        if (!bRetCode)
            continue;

        KHero* pHero = (KHero*)pSceneObj;
        if (pHero->m_nSide == m_nSide)
            continue;

        bRetCode = pHero->CanBeAttacked();
        if (!bRetCode)
            continue;

        pTarget = pHero;
        break;
    }

    KG_PROCESS_ERROR(pTarget);

    for (int i = 0; i < countof(pSkill->m_SubSkill); ++i)
    {
        KSubSkillInfo& rSubSkill = pSkill->m_SubSkill[i];
        if (!rSubSkill.dwID)
            continue;

        CastGrabSkill(rSubSkill.dwID, 0, pTarget->m_dwID);
        break;
    }

Exit0:
    return;
}

BOOL KHero::SpecialUseDoodad()
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KDoodad* pDoodad = NULL;

    pDoodad = GetHoldingDoodad();
    KGLOG_PROCESS_ERROR(pDoodad);

    bRetCode = pDoodad->BeSpecialUsedBy(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

int KHero::GetBallTakerRelation()
{
    int nRetRelation = rltNone;
    KHero* pBallTaker = NULL;

    pBallTaker = m_pScene->GetBallTaker();
    KG_PROCESS_ERROR(pBallTaker);

    nRetRelation = GetRelationTypeWith(pBallTaker);

Exit0:
    return nRetRelation;
}

BOOL KHero::FireCastSkillAIEvent(KSkill* pSkill)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    int nEnemySide = sidRight;
    KAI_EVENT eEvent = aevNearByEnemyCastNormalSkill;
    KBODY cRange;

    KGLOG_PROCESS_ERROR(pSkill);

    KG_PROCESS_SUCCESS(!pSkill->m_bFireAIEvent);

    if (m_nSide == sidRight)
        nEnemySide = sidLeft;

    bRetCode = pSkill->GetAIEventRange(this, cRange);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (pSkill->m_nRestraintType == KRESTRAINT_TYPE_NORMAL)
    {
        eEvent = aevNearByEnemyCastNormalSkill;
    }
    else if (pSkill->m_nRestraintType == KRESTRAINT_TYPE_BATI)
    {
        eEvent = aevNearByEnemyCastBaTiSkill;
    }
    else if (pSkill->m_nRestraintType == KRESTRAINT_TYPE_GRAB)
    {
        eEvent = aevNearByEnemyCastGrabSkill;
    }
    else
    {
        KGLOG_PROCESS_ERROR(false);
    }

    m_pScene->FireAIEventToSideInRange(nEnemySide, eEvent, cRange);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHero::FireUseDoodadAIEvent(KDoodad* pDoodad)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    int nEnemySide = sidRight;
    KBODY cRange;

    KGLOG_PROCESS_ERROR(pDoodad);

    KGLOG_PROCESS_ERROR(pDoodad->m_pTemplate);

    KG_PROCESS_SUCCESS(!pDoodad->m_pTemplate->m_bFireAIEvent);

    if (m_nSide == sidRight)
        nEnemySide = sidLeft;

    bRetCode = pDoodad->GetAIEventRange(this, cRange);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_pScene->FireAIEventToSideInRange(nEnemySide, aevNearByEnemyUseDoodad, cRange);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KHero::MoveTriggerPassive()
{
    KHero* pBallTaker = NULL;


    if ((g_pSO3World->m_nGameLoop - m_dwID) % GAME_FPS != 0)
        goto Exit0;

    pBallTaker = m_pScene->GetBallTaker();//这里不判断空指针

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        int nDistance = 0;

        KSceneObject* pObj = it.GetValue();

        if (!pObj->Is(sotHero))
            continue;

        KHero* pTrigger = (KHero*)pObj;

        if(pTrigger->m_nSide == m_nSide)
            continue;

        nDistance = (int)GetPosition().GetDistanceTo(pTrigger->GetPosition());

        pTrigger->TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_ENEMY_NEAR, INT_MAX, nDistance);
        TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_ENEMY_NEAR, INT_MAX, nDistance);

        if (pTrigger == pBallTaker)
            TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_ENEMY_BALL_NEAR, INT_MAX, nDistance);

        if (this == pBallTaker)
            pTrigger->TriggerPassiveSkill(PASSIVE_SKILL_TRIGGER_EVENT_ENEMY_BALL_NEAR, INT_MAX, nDistance);
    }
Exit0:
    return;
}
