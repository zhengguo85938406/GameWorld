#include "stdafx.h"
#include "KPassiveSkill.h"
#include "KHero.h"
#include "KAttribute.h"
#include "Protocol/gs2cl_protocol.h"
#include "T3GameWorld/KSO3World.h"

KPassiveSkill::KPassiveSkill()
{
    m_pPassiveSkillData = NULL;
    m_nActiveFrame = 0;
}

KPassiveSkill::~KPassiveSkill()
{
}

BOOL KPassiveSkill::Init(KPassiveSkillData* pData, KHero* pHero)
{
    assert(pData);
    assert(pHero);

    m_pPassiveSkillData = pData;
    m_pOwner = pHero;
    m_nActiveFrame = 0;
    return true;
}

void KPassiveSkill::UnInit()
{
    m_pPassiveSkillData = NULL;
    m_pOwner = NULL;
    return;
}

BOOL KPassiveSkill::IsEmpty()
{
    return (m_pPassiveSkillData == NULL);
}

BOOL KPassiveSkill::IsAddBuff()
{
    KGLOG_PROCESS_ERROR(m_pPassiveSkillData);

    return (m_pPassiveSkillData->nFoveverBuffID != ERROR_ID);
Exit0:
    return false;
}

BOOL KPassiveSkill::AddBuff()
{
    if (m_pPassiveSkillData->nFoveverBuffID != ERROR_ID)
    {
        m_pOwner->AddBuff(m_pPassiveSkillData->nFoveverBuffID);
    }

    return true;
}

BOOL KPassiveSkill::CanTrigger(int nEvent, int nOldValue, int nNewValue)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

    KG_PROCESS_ERROR(m_pPassiveSkillData);
    KG_PROCESS_ERROR(m_pPassiveSkillData->nTriggerEvent == nEvent);

    bRetCode = CheckCoolDown();
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = CheckEventValue(nOldValue, nNewValue);
    KG_PROCESS_ERROR(bRetCode);
    
    bRetCode = CheckCondition();
    KG_PROCESS_ERROR(bRetCode);
    
    bRetCode = g_RandPercent(m_pPassiveSkillData->nTriggerRate);
    KG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPassiveSkill::CheckCondition()
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(m_pPassiveSkillData);

    switch (m_pPassiveSkillData->nTriggerCondition)
    {
        case PASSIVE_SKILL_TRIGGER_CONDITION_GET_BALL:
            {
                int nRelation = 0;
                nRelation = m_pOwner->GetBallTakerRelation(); 
                if (nRelation == m_pPassiveSkillData->nTriggerConditionValue)
                    bResult = true;
            }
            break;
        default:
            bResult = true;
            break;
    }

Exit0:
    return bResult;
}

BOOL KPassiveSkill::CheckEventValue(int nOldValue, int nNewValue)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(m_pPassiveSkillData);

    switch (m_pPassiveSkillData->nTriggerEvent)
    {
        case PASSIVE_SKILL_TRIGGER_EVENT_LEAD:
            if (m_pPassiveSkillData->nTriggerEventValue > nOldValue && m_pPassiveSkillData->nTriggerEventValue <= nNewValue)
                bResult = true;
            break;
        case PASSIVE_SKILL_TRIGGER_EVENT_HP:
        case PASSIVE_SKILL_TRIGGER_EVENT_ENEMY_NEAR:
        case PASSIVE_SKILL_TRIGGER_EVENT_ENEMY_BALL_NEAR:
            if (m_pPassiveSkillData->nTriggerEventValue < nOldValue && m_pPassiveSkillData->nTriggerEventValue >= nNewValue)
                bResult = true;
            break;
        case PASSIVE_SKILL_TRIGGER_EVENT_GET_BALL:
            if (m_pPassiveSkillData->nTriggerEventValue == nNewValue)
                bResult = true;
            break;
        default:
            bResult = true;
            break;
    }

Exit0:
    return bResult;
}

BOOL KPassiveSkill::CheckCoolDown()
{
    BOOL bResult = false;
    KGLOG_PROCESS_ERROR(m_pPassiveSkillData);
    
    KG_PROCESS_SUCCESS(m_nActiveFrame == 0);
    KG_PROCESS_ERROR(m_nActiveFrame + m_pPassiveSkillData->nCDFrame < g_pSO3World->m_nGameLoop);
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KPassiveSkill::Action()
{
    KGLOG_PROCESS_ERROR(m_pPassiveSkillData);
    m_nActiveFrame = g_pSO3World->m_nGameLoop;
    switch (m_pPassiveSkillData->nTriggerAction)
    {
    case PASSIVE_SKILL_ACTION_SKILL:
        m_pOwner->CastSkill(m_pPassiveSkillData->nActionValue, 0 , m_pOwner->GetFaceDir(), false, true);
        break;
    case PASSIVE_SKILL_ACTION_BUFF:
        m_pOwner->AddBuff(m_pPassiveSkillData->nActionValue);
        break;
    }
Exit0:
    return;
}