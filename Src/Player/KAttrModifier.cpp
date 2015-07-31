#include "stdafx.h"
#include "KAttrModifier.h"
#include "KHero.h"
#include "KPlayer.h"
#include "KPlayerServer.h"

BOOL ProcessGravityBase(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult  = false;

    pDstHero->m_nAddGravityBase += (bApply ? nValue1 : -nValue1);

    pDstHero->UpdateMoveParameter();

    bResult = true;
//Exit0:
    return bResult;
}

BOOL ProcessGravityPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult  = false;

    pDstHero->m_nGravityPercent += (bApply ? nValue1 : -nValue1);

    pDstHero->UpdateMoveParameter();

    bResult = true;
//Exit0:
    return bResult;
}

BOOL ProcessSetAttackState(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult  = false;

    pDstHero->m_bAttackState = bApply;
    
    bResult = true;
//Exit0:
    return bResult;
}

BOOL ProcessAutoMove(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult     = false;

    KGLOG_PROCESS_ERROR(bApply && "Cann't unapply!");

    pDstHero->AutoMove(nValue1 / GAME_FPS, (KAutoMoveDirection)nValue2);
    
    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessGeneralDamage(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(bApply && "Cann't unapply!");
    pDstHero->LoseEndurance(nValue1);

    g_PlayerServer.DoSyncDamage(pDstHero, false, nValue1, false, 0, 0);
    
    KGLogPrintf(
        KGLOG_DEBUG, 
        "Character %d apply general damage %d, life %d/%d", 
        pDstHero->m_dwID, 
        nValue1, 
        pDstHero->m_nCurrentEndurance,
        pDstHero->m_nMaxEndurance
    );

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessKnockedOff(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;
    
    KGLOG_PROCESS_ERROR(bApply && "Cann't unapply!");

    pDstHero->KnockedOff(nValue1, nValue2);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessGrabKnockedOff(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(bApply && "Cann't unapply!");

    pDstHero->KnockedOff(nValue1, nValue2);

    bResult = true;
Exit0:
    return bResult;
}


BOOL ProcessGrabKnockedOffOffset(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    KPOSITION cPos;
    KTWO_DIRECTION eFaceDir = csdInvalid;

    KGLOG_PROCESS_ERROR(bApply && "Cann't unapply!");

    cPos = pDstHero->GetPosition();

    eFaceDir = pDstHero->GetFaceDir();
    if (eFaceDir != csdRight)
        nValue1 = -nValue1;

    pDstHero->SetPosition(cPos.nX + nValue1, cPos.nY, cPos.nZ + nValue2);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessCannotSkill(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    if (bApply)
    {
        pDstHero->ApplyCannotSkill();
    }
    else
    {
        pDstHero->UnApplyCannotSkill();
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessCannotPick(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    if (bApply)
    {
        pDstHero->ApplyCannotPick();
    }
    else
    {
        pDstHero->UnApplyCannotPick();
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessShootBallHitRate(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nHitRate += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessNormalShootForce(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nNormalShootForce += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessSkillShootForce(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nSkillShootForce += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessSlamDunkForce(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nSlamDunkForce += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessMoveSpeed(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nMoveSpeedBase += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nMoveSpeed = pDstHero->m_nMoveSpeedBase + pDstHero->m_nMoveSpeedBase * pDstHero->m_nAddMoveSpeedBasePercent / HUNDRED_NUM;

    pDstHero->UpdateMoveParameter();

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessJumpSpeed(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nJumpSpeedBase += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nJumpSpeed = pDstHero->m_nJumpSpeedBase + pDstHero->m_nJumpSpeedBase * pDstHero->m_nAddJumpSpeedBasePercent / HUNDRED_NUM;

    pDstHero->UpdateMoveParameter();

    bResult = true;
Exit0:
    return bResult;
}

////////////////////////////////////////////////////////////////////////// »ù´¡ÊôÐÔ
BOOL ProcessCurrentEndurance(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->LoseEndurance(bApply ? -nValue1 : nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessMaxEndurance(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nMaxEnduranceBase += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nMaxEndurance = pDstHero->m_nMaxEnduranceBase 
        + pDstHero->m_nMaxEnduranceBase * pDstHero->m_nAddMaxEndurancePercent / HUNDRED_NUM;
    return true;
}

BOOL ProcessCurrentEndurancePercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;
    int nEnduranceReplenish = 0;

    nEnduranceReplenish = pDstHero->m_nMaxEndurance * nValue1 / HUNDRED_NUM;

    pDstHero->LoseEndurance((bApply ? -nEnduranceReplenish : nEnduranceReplenish));

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessEnduranceReplenish(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nEnduranceReplenish += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessEnduranceReplenishPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nEnduranceReplenishPercent += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessCurrentStamina(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->LoseStamina(bApply ? -nValue1 : nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessMaxStamina(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nMaxStaminaBase += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nMaxStamina = pDstHero->m_nMaxStaminaBase 
        + pDstHero->m_nMaxStaminaBase * pDstHero->m_nAddMaxStaminaPercent / HUNDRED_NUM;
    return true;
}

BOOL ProcessCurrentStaminaPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;
    int nStaminaReplenish   = 0;

    nStaminaReplenish = pDstHero->m_nMaxStamina * nValue1 / HUNDRED_NUM;

    pDstHero->LoseStamina(bApply ? -nStaminaReplenish : nStaminaReplenish);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessStaminaReplenish(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult  = false;    

    pDstHero->m_nStaminaReplenish += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessStaminaReplenishPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult  = false;

    pDstHero->m_nStaminaReplenishPercent += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessCurrentAngry(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nCurrentAngry += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessMaxAngry(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nMaxAngry += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessCurrentAngryPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;
    int nAngryReplenish = 0;

    nAngryReplenish = pDstHero->m_nMaxAngry * nValue1 / HUNDRED_NUM;

    pDstHero->m_nCurrentAngry += (bApply ? nAngryReplenish : -nAngryReplenish);

    MAKE_IN_RANGE(pDstHero->m_nCurrentAngry, 0, pDstHero->m_nMaxAngry);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessAdditionalRecoveryAngry(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = true;

    pDstHero->m_nAdditionalAddAngryPercent += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessWillPower(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nWillPowerBase += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nWillPower = pDstHero->m_nWillPowerBase
        + pDstHero->m_nWillPowerBase * pDstHero->m_nAddWillPowerPercent / HUNDRED_NUM;
    return true;
}

BOOL ProcessInterference(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nInterferenceBase += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nInterference = pDstHero->m_nInterferenceBase 
        + pDstHero->m_nInterferenceBase * pDstHero->m_nAddInterferencePercent / HUNDRED_NUM;
    return true;
}

BOOL ProcessInterferenceRange(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nInterferenceRangeBase += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nInterferenceRange = pDstHero->m_nInterferenceRangeBase + 
        pDstHero->m_nInterferenceRangeBase * pDstHero->m_nAddInterferenceRangePercent / HUNDRED_NUM;

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessAttackPoint(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nAttackPoint += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessAttackPointPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nAttackPointPercent += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessAgility(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nAgility += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessCritPoint(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nCritPoint += (bApply ? nValue1 : -nValue1);

    return true;
}

BOOL ProcessCritRate(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nCritRate += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessDefense(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nDefense += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessDefensePointPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nDefensePointPercent += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessDefensePercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nDefensePercent += (bApply ? nValue1 : -nValue1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessInvincible(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_bInvincible = (bApply ? true : false);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessAddJumpSpeedPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nAddJumpSpeedBasePercent += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nJumpSpeed = pDstHero->m_nJumpSpeedBase + pDstHero->m_nJumpSpeedBase * pDstHero->m_nAddJumpSpeedBasePercent / HUNDRED_NUM;

    pDstHero->UpdateMoveParameter();

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessAddInterferenceRangePercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nAddInterferenceRangePercent += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nInterferenceRange = pDstHero->m_nInterferenceRangeBase + 
        pDstHero->m_nInterferenceRangeBase * pDstHero->m_nAddInterferenceRangePercent / HUNDRED_NUM;

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessAddMoveSpeedPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bResult = false;

    pDstHero->m_nAddMoveSpeedBasePercent += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nMoveSpeed = pDstHero->m_nMoveSpeedBase + pDstHero->m_nMoveSpeedBase * pDstHero->m_nAddMoveSpeedBasePercent / HUNDRED_NUM;

    pDstHero->UpdateMoveParameter();

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessExpMultiple(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL        bResult     = false;
    KHeroData*  pHeroData   = NULL;
    KPlayer*    pPlayer     = pDstHero->GetOwner();

    KGLOG_PROCESS_ERROR(pPlayer);

    pHeroData = pPlayer->m_HeroDataList.GetHeroData(pDstHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    pHeroData->m_nExpMultiple += (bApply ? nValue1 : -nValue1);

    LogInfo("ProcessExpMultiple:(player:%d,hero:%d,apply:%d,result:%d)", pPlayer->m_dwID, pDstHero->m_dwTemplateID, bApply, pHeroData->m_nExpMultiple);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessForbitAutoAction(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_bForbitAutoAction = bApply;
    return true;
}

BOOL ProcessCloseAiMode(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bRetCode = false;

    if (bApply)
    {
        bRetCode = pDstHero->TurnOffAI();
        if (bRetCode)
            pDstHero->m_bAiTurnOffFlag = true;
    }
    else
    {
        if (pDstHero->m_bAiTurnOffFlag)
        {
            pDstHero->TurnOnAI();
            pDstHero->m_bAiTurnOffFlag = false;
        }   
    }

    return true;
}

BOOL ProcessShootBall(KHero *pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL bRetCode = false;
    KGLOG_PROCESS_ERROR(bApply && "Cann't unapply");
    bRetCode = pDstHero->ShootBallBySkill((DWORD)nValue1, nValue2);
    KGLOG_PROCESS_ERROR(bRetCode);
Exit0:
    return true;
}

BOOL ProcessAddWillPowerPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nAddWillPowerPercent += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nWillPower = pDstHero->m_nWillPowerBase 
        + pDstHero->m_nWillPowerBase + pDstHero->m_nAddWillPowerPercent / HUNDRED_NUM;
    return true;
}

BOOL ProcessAddInterferencePercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nAddInterferencePercent += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nInterference = pDstHero->m_nInterferenceBase 
        + pDstHero->m_nInterferenceBase * pDstHero->m_nAddInterferencePercent / HUNDRED_NUM;
    return true;
}

BOOL ProcessAddMaxStaminaPercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nAddMaxStaminaPercent += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nMaxStamina = pDstHero->m_nMaxStaminaBase 
        + pDstHero->m_nMaxStaminaBase * pDstHero->m_nAddMaxStaminaPercent / HUNDRED_NUM;
    return true;
}

BOOL ProcessAddMaxEndurancePercent(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_nAddMaxEndurancePercent += (bApply ? nValue1 : -nValue1);
    pDstHero->m_nMaxEndurance = pDstHero->m_nMaxEnduranceBase 
        + pDstHero->m_nMaxEnduranceBase * pDstHero->m_nAddMaxEndurancePercent / HUNDRED_NUM;
    return true;
}

BOOL ProcessCallSkillScript(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KSkill* pSkill      = NULL;
    int     nTopIndex   = 0;

    pSkill = pDstHero->m_pCastingSkill;
    KGLOG_PROCESS_ERROR(pSkill);
    KGLOG_PROCESS_ERROR(pSkill->m_dwScriptID != 0);

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(pSkill->m_dwScriptID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(pSkill->m_dwScriptID, SCRIPT_ON_CASTING_SKILL);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);
    g_pSO3World->m_ScriptManager.Push(pDstHero);
    g_pSO3World->m_ScriptManager.Push(pDstHero->m_nVirtualFrame - pDstHero->m_nAttackSkillTime);
    g_pSO3World->m_ScriptManager.Push(nValue1);
    g_pSO3World->m_ScriptManager.Push(nValue2);
    g_pSO3World->m_ScriptManager.Call(pSkill->m_dwScriptID, SCRIPT_ON_CASTING_SKILL, 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

    bResult = true;
Exit0:
    return bResult;
}

BOOL ProcessSkillTrackMove(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->TrackMoveByCastingSkill(nValue1);
    return true;
}


BOOL ProcessWeakKnockDown(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->WeakKnockDown(nValue1, nValue2);
    return true;
}

BOOL ProcessBati(KHero* pDstHero, BOOL bApply, int nValue1, int nValue2)
{
    pDstHero->m_bBaTi = bApply;
    return true;
}

ATTR_PROCESS_FUN g_AttributeProcessFunctions[atTotal] = { NULL };

void InitAttributeFunctions()
{
    memset(g_AttributeProcessFunctions, 0, sizeof(g_AttributeProcessFunctions));

    g_AttributeProcessFunctions[atGravityBase]          = ProcessGravityBase;
    g_AttributeProcessFunctions[atGravityPercent]       = ProcessGravityPercent;

    g_AttributeProcessFunctions[atSetAttackState]       = ProcessSetAttackState;
    g_AttributeProcessFunctions[atAutoMove]             = ProcessAutoMove;

    g_AttributeProcessFunctions[atGeneralDamage]        = ProcessGeneralDamage;
    g_AttributeProcessFunctions[atKnockedoff]           = ProcessKnockedOff;
    g_AttributeProcessFunctions[atGrabKnockedOff]       = ProcessGrabKnockedOff;
    g_AttributeProcessFunctions[atGrabKnockedOffOffset] = ProcessGrabKnockedOffOffset;
    g_AttributeProcessFunctions[atCannotSkill]          = ProcessCannotSkill;
    g_AttributeProcessFunctions[atCannotPick]           = ProcessCannotPick;

    g_AttributeProcessFunctions[atRunSpeedBase]         = ProcessMoveSpeed;
    g_AttributeProcessFunctions[atJumpSpeedBase]        = ProcessJumpSpeed;
    g_AttributeProcessFunctions[atJumpSpeedPercent]     = ProcessAddJumpSpeedPercent;
    g_AttributeProcessFunctions[atCurrentEndurance]     = ProcessCurrentEndurance;
    g_AttributeProcessFunctions[atMaxEndurance]         = ProcessMaxEndurance;
    g_AttributeProcessFunctions[atCurrentEndurancePercent] = ProcessCurrentEndurancePercent;    
    g_AttributeProcessFunctions[atEnduranceReplenish]   = ProcessEnduranceReplenish;
    g_AttributeProcessFunctions[atEnduranceReplenishPercent] = ProcessEnduranceReplenishPercent;
    g_AttributeProcessFunctions[atCurrentStamina]       = ProcessCurrentStamina;
    g_AttributeProcessFunctions[atMaxStamina]           = ProcessMaxStamina;
    g_AttributeProcessFunctions[atCurrentStaminaPercent] = ProcessCurrentStaminaPercent;
    g_AttributeProcessFunctions[atStaminaReplenish]     = ProcessStaminaReplenish;
    g_AttributeProcessFunctions[atStaminaReplenishPercent] = ProcessStaminaReplenishPercent;
    g_AttributeProcessFunctions[atCurrentAngry]         = ProcessCurrentAngry;
    g_AttributeProcessFunctions[atMaxAngry]             = ProcessMaxAngry;
    g_AttributeProcessFunctions[atCurrentAngryPercent]  = ProcessCurrentAngryPercent;
    g_AttributeProcessFunctions[atAdditionalRecoveryAngry] = ProcessAdditionalRecoveryAngry;
    g_AttributeProcessFunctions[atWillPower]            = ProcessWillPower;
    g_AttributeProcessFunctions[atInterference]         = ProcessInterference;
    g_AttributeProcessFunctions[atInterferenceRange]    = ProcessInterferenceRange;
    g_AttributeProcessFunctions[atAttackPoint]          = ProcessAttackPoint;
    g_AttributeProcessFunctions[atAttackPointPercent]   = ProcessAttackPointPercent;
    g_AttributeProcessFunctions[atAgility]              = ProcessAgility;
    g_AttributeProcessFunctions[atCritPoint]            = ProcessCritPoint;
    g_AttributeProcessFunctions[atCritRate]             = ProcessCritRate;
    g_AttributeProcessFunctions[atDefense]              = ProcessDefense;
    g_AttributeProcessFunctions[atDefensePointPercent]  = ProcessDefensePointPercent;
    g_AttributeProcessFunctions[atDefensePercent]       = ProcessDefensePercent;
    g_AttributeProcessFunctions[atShootBallHitRate]     = ProcessShootBallHitRate;
    g_AttributeProcessFunctions[atNomalShootForce]      = ProcessNormalShootForce;
    g_AttributeProcessFunctions[atSkillShootForce]      = ProcessSkillShootForce;
    g_AttributeProcessFunctions[atSlamDunkForce]        = ProcessSlamDunkForce;
    g_AttributeProcessFunctions[atAddInterferenceRangePercent]  = ProcessAddInterferenceRangePercent;
    g_AttributeProcessFunctions[atAddMoveSpeedPercent]          = ProcessAddMoveSpeedPercent;
    g_AttributeProcessFunctions[atInvincible]           = ProcessInvincible;
    g_AttributeProcessFunctions[atExpMultiple]          = ProcessExpMultiple;
    g_AttributeProcessFunctions[atForbitAutoAction]     = ProcessForbitAutoAction;
    g_AttributeProcessFunctions[atCloseAiMode]          = ProcessCloseAiMode;
    g_AttributeProcessFunctions[atShootBall]            = ProcessShootBall;
    
    g_AttributeProcessFunctions[atAddWillPowerPercent]      = ProcessAddWillPowerPercent;
    g_AttributeProcessFunctions[atAddInterferencePercent]   = ProcessAddInterferencePercent;
    g_AttributeProcessFunctions[atAddMaxStaminaPercent]     = ProcessAddMaxStaminaPercent;
    g_AttributeProcessFunctions[atAddMaxEndurancePercent]   = ProcessAddMaxEndurancePercent;

    g_AttributeProcessFunctions[atCallSkillScript]          = ProcessCallSkillScript;
    g_AttributeProcessFunctions[atSkillTrackMove]           = ProcessSkillTrackMove;

    g_AttributeProcessFunctions[atWeakKnockDown]            = ProcessWeakKnockDown;
    g_AttributeProcessFunctions[atBati]                     = ProcessBati;
}
