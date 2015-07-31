#include "stdafx.h"
#include "KSkill.h"
#include "KHero.h"
#include "KAttribute.h"
#include "Protocol/gs2cl_protocol.h"
#include "T3GameWorld/KSO3World.h"

KSkill::KSkill()
{
    m_dwID              = ERROR_ID;

    m_dwSrcBuffID       = ERROR_ID;
    m_dwDestBuffID      = ERROR_ID;
    m_pSrcAttribute     = NULL;
    m_pDestAttribute    = NULL;
	m_dwAimType         = 0;
    m_bCanCastWithBall  = false;
    m_eSkillType        = skilltypeInvalid;
    m_nAttackIntervalFrame = 0;

    m_nSelfAddAngry     = 0;      
    m_nTargetAddAngry   = 0;    
    m_nNeedAngry        = 0;         
    m_nCostAngry        = 0;         
    m_nCostAngryPercent = 0;

    m_eEndType          = KSKILL_END_TYPE_NONE;
    m_nEndFrame          = 0;
    m_nInterruptLevel   = 0;

    m_nMoveState        = 0;
    m_nMoveStateFrame   = 0;

    m_dwStepID          = 0;
    m_nStepMinFrame     = 0;
    m_nStepMaxFrame     = 0;

    memset(m_SubSkill, 0, sizeof(m_SubSkill));

    m_bCauseGrabSkill   = false;
    m_dwTargetBuffLimit = ERROR_ID;
    m_nCanCastCountInAir = 0;

    m_nSpasticityFrame = 0;
    m_bDelayApplySkillEffect = false;
    m_bInvincibleOnSpasticity = false;

    m_bIsCommonSkill    = false;

    m_nJudgeBoxMinX     = 0;
    m_nJudgeBoxMaxX     = 0;
    m_nJudgeBoxMinY     = 0;
    m_nJudgeBoxMaxY     = 0;
    m_nJudgeBoxMinZ     = 0;
    m_nJudgeBoxMaxZ     = 0;

    m_nRestraintType    = KRESTRAINT_TYPE_NORMAL;

    m_bCanSpecifyCastDir = false;
    m_bCanSpecifyCastingDir = false;
    m_bClearVelocityAtEnd = false;
    m_nGravityPercent   = 0;
    m_nSeries           = 0;

    m_bFireAIEvent      = false;
    m_nAIEventMinX      = 0;
    m_nAIEventMaxX      = 0;
    m_nAIEventMinY      = 0;
    m_nAIEventMaxY      = 0;
    m_nAIEventMinZ      = 0;
    m_nAIEventMaxZ      = 0;

    m_bNirvanaSkill = false;
    m_dwScriptID        = 0;
}

KSkill::~KSkill()
{
}

BOOL KSkill::Init()
{
    return true;
}

void KSkill::UnInit()
{
    FREE_ATTRIB_LIST(m_pSrcAttribute);
    FREE_ATTRIB_LIST(m_pDestAttribute);
}


BOOL KSkill::IsOverTimeSkill()
{
   return m_eEndType > KSKILL_END_TYPE_NONE && m_eEndType < KSKILL_END_TYPE_TOTAL;
}

BOOL KSkill::CanBeApplyOnEnemy()
{
    return m_dwAimType & rltEnemy;
}

BOOL KSkill::CanBeApplyOnMe()
{
    return m_dwAimType & rltMe;
}

BOOL KSkill::CanBeApplyOnUs()
{
    return m_dwAimType & rltUs;
}

BOOL KSkill::CanBeApplyOn(KRelationType eType)
{
    assert(eType >= rltNone && eType < rltTotal);
    return m_dwAimType & eType;
}

BOOL KSkill::CheckStateCondition(KHeroMoveState eMoveState, int nDeltaFrame)
{
    if (m_nMoveState == cmsInvalid)
        return true;

    if (m_nMoveState == eMoveState/* && nDeltaFrame < m_nMoveStateFrame*/) // 暂时不判时间
        return true;

    return false;
}


BOOL KSkill::IsType(KSkillType eType)
{
    return m_eSkillType == eType;
}

KMAP_ANCHOR* KSkill::GetAnchorData()
{
    return &m_mapAnchors;
}

void KSkill::SetBinaryData(const KSkillBinaryData& rBinaryData)
{
    m_mapAnchors        = rBinaryData.Anchors;
    m_mapSegments       = rBinaryData.Segments;
    if (m_nEndFrame == 0)
        m_nEndFrame     = rBinaryData.nTotalFrame;
}

BOOL KSkill::IsAttackStateAtFrame(int nFrameIndex)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KSkillSegmentInfo* pSegmentInfo = NULL;
	
	KG_PROCESS_ERROR(m_eSkillType == skilltypeCommon || m_eSkillType == skilltypeShootBall);
    KG_PROCESS_ERROR(!m_bCauseGrabSkill);

    if (m_eEndType == KSKILL_END_TYPE_ON_FLOOR)
    {
        bResult = true;
        goto Exit0;
    }

    pSegmentInfo = GetSegmentOfFrameBelongTo(nFrameIndex);
    KG_PROCESS_ERROR(pSegmentInfo);

    bResult = pSegmentInfo->bAttackState;
Exit0:
	return bResult;
}

BOOL KSkill::WouldGrabAtFrame(int nFrameIndex)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KSkillSegmentInfo* pSegmentInfo = NULL;

    //KG_PROCESS_ERROR(m_eSkillType == skilltypeGrab);
    KG_PROCESS_ERROR(m_bCauseGrabSkill);

    if (m_eEndType == KSKILL_END_TYPE_ON_FLOOR)
    {
        bResult = true;
        goto Exit0;
    }

    pSegmentInfo = GetSegmentOfFrameBelongTo(nFrameIndex);
    KG_PROCESS_ERROR(pSegmentInfo);

    bResult = pSegmentInfo->bAttackState;
Exit0:
    return bResult;
}

KSkillSegmentInfo* KSkill::GetSegmentOfFrameBelongTo(int nFrameIndex)
{
    KSkillSegmentInfo* pSegment = NULL;
    KMAP_SEGMENT::iterator it;

    KG_PROCESS_ERROR(!m_mapSegments.empty());

    it = m_mapSegments.begin();
    KG_PROCESS_ERROR(nFrameIndex >= it->first);

    it = m_mapSegments.upper_bound(nFrameIndex);
    --it;

    pSegment = &it->second;
Exit0:
    return pSegment;
}

KAnchor* KSkill::GetAnchorAtFrame(int nFrameIndex)
{
    KAnchor* pResult = NULL;
    KMAP_ANCHOR::iterator it;

    it = m_mapAnchors.find(nFrameIndex);
    if (it != m_mapAnchors.end())
        pResult = &it->second;

    return pResult;
}

BOOL KSkill::GetJudageBox(KHero* pAttcker, KBODY& rRetBody)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	KGLOG_PROCESS_ERROR(pAttcker);

    rRetBody.nLength   = pAttcker->m_nLength * (m_nJudgeBoxMaxX - m_nJudgeBoxMinX) / HUNDRED_NUM;
    if (pAttcker->m_eFaceDir == csdRight)
        rRetBody.nX = pAttcker->m_nX + pAttcker->m_nLength * m_nJudgeBoxMinX / HUNDRED_NUM + rRetBody.nLength / 2;
    else
        rRetBody.nX = pAttcker->m_nX - pAttcker->m_nLength * m_nJudgeBoxMinX / HUNDRED_NUM - rRetBody.nLength / 2;

    rRetBody.nWidth    = pAttcker->m_nWidth * (m_nJudgeBoxMaxY - m_nJudgeBoxMinY) / HUNDRED_NUM;
    rRetBody.nY        = pAttcker->m_nY + pAttcker->m_nWidth * m_nJudgeBoxMinY / HUNDRED_NUM + rRetBody.nWidth / 2;

    rRetBody.nHeight   = pAttcker->m_nHeight * (m_nJudgeBoxMaxZ - m_nJudgeBoxMinZ) / HUNDRED_NUM;
    rRetBody.nZ        = pAttcker->m_nZ + pAttcker->m_nHeight * m_nJudgeBoxMinZ / HUNDRED_NUM;

	bResult = true;
Exit0:
	return bResult;
}

KSkillSegmentInfo* KSkill::GetSegmentAtFrame(int nFrameIndex)
{
    KSkillSegmentInfo* pResult = NULL;
    KMAP_SEGMENT::iterator it;

    it = m_mapSegments.find(nFrameIndex);
    if (it != m_mapSegments.end())
        pResult = &it->second;

    return pResult;
}

BOOL KSkill::UpdateOnSkillLoaded()
{
    BOOL    bResult  = false;
    BOOL    bRetCode = false;
    KSkill* pSkill   = NULL;

    for (int i = 0; i < countof(m_SubSkill); ++i)
    {
        KSubSkillInfo& rSubSkill = m_SubSkill[i];
        if (!rSubSkill.dwID)
            continue;

         pSkill = g_pSO3World->m_SkillManager.GetSkill(rSubSkill.dwID);
         KGLOG_PROCESS_ERROR(pSkill);

         if (pSkill->m_eSkillType != skilltypeGrab)
             continue;

         m_bCauseGrabSkill = true;
         break;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KSkill::HasSubSkill(DWORD dwSkillID, DWORD dwSkillStep)
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(dwSkillStep == 0);

    for (int i = 0; i < countof(m_SubSkill); ++i)
    {
        KSubSkillInfo& rSubSkill = m_SubSkill[i];
        if (!rSubSkill.dwID)
            continue;

        KG_PROCESS_SUCCESS(rSubSkill.dwID == dwSkillID);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KSkill::CanPluginInSlot(int nSlotIndex)
{
	BOOL bResult = false;

	std::vector<int>::iterator it;

    it = std::find(m_vecBelongToSlots.begin(), m_vecBelongToSlots.end(), nSlotIndex);
    KG_PROCESS_ERROR(it != m_vecBelongToSlots.end());

	bResult = true;
Exit0:
	return bResult;
}

BOOL KSkill::GetAIEventRange(KHero* pCaster, KBODY& rRetRange)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pCaster);

    rRetRange.nLength   = pCaster->m_nLength * (m_nAIEventMaxX - m_nAIEventMinX) / HUNDRED_NUM;
    if (pCaster->m_eFaceDir == csdRight)
        rRetRange.nX = pCaster->m_nX + pCaster->m_nLength * m_nAIEventMinX / HUNDRED_NUM + rRetRange.nLength / 2;
    else
        rRetRange.nX = pCaster->m_nX - pCaster->m_nLength * m_nAIEventMinX / HUNDRED_NUM - rRetRange.nLength / 2;

    rRetRange.nWidth    = pCaster->m_nWidth * (m_nAIEventMaxY - m_nAIEventMinY) / HUNDRED_NUM;
    rRetRange.nY        = pCaster->m_nY + pCaster->m_nWidth * m_nAIEventMinY / HUNDRED_NUM + rRetRange.nWidth / 2;

    rRetRange.nHeight   = pCaster->m_nHeight * (m_nAIEventMaxZ - m_nAIEventMinZ) / HUNDRED_NUM;
    rRetRange.nZ        = pCaster->m_nZ + pCaster->m_nHeight * m_nAIEventMinZ / HUNDRED_NUM;

    bResult = true;
Exit0:
    return bResult;
}
