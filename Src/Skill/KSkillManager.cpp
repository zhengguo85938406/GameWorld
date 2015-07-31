#include "stdafx.h"
#include "KSkillManager.h"
#include "KAttribute.h"
#include "game_define.h"
#include "StrMap.h"
#include "KGEnumConvertor.h"
#include "skill_logic_file_analysis.h"


KSkillManager::KSkillManager()
{
}

KSkillManager::~KSkillManager()
{
}

BOOL KSkillManager::Init()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = LoadSkillTable();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_SpecSkill.Load();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        for (KSkillTable::iterator it = m_SkillTable.begin(); it != m_SkillTable.end(); ++it)
	    {
	        it->second.UnInit();
	    }
	    m_SkillTable.clear();
    }
    return bResult;
}

void KSkillManager::UnInit()
{
    for (KSkillTable::iterator it = m_SkillTable.begin(); it != m_SkillTable.end(); ++it)
    {
        it->second.UnInit();
    }
    m_SkillTable.clear();
}

KSkill* KSkillManager::GetSkill(DWORD dwSkillID, DWORD dwStepID/*=0U*/)
{
    KSkill* pResult = NULL;
    KSkillTable::iterator it;
    uint64_t uKey = MAKE_INT64(dwSkillID, dwStepID);

    KG_PROCESS_ERROR(dwSkillID != ERROR_ID);

    it = m_SkillTable.find(uKey);
    KG_PROCESS_ERROR(it != m_SkillTable.end());

    pResult = &(it->second);
Exit0:
    return pResult;
}

KSpecialSkillInfo* KSkillManager::GetSpecialSkill(KSPECIAL_SKILL eID)
{
    return m_SpecSkill.Get((uint32_t)eID);
}

BOOL KSkillManager::LoadSkillTable()
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    DWORD       dwSkillID   = 0;
    DWORD       dwStepID    = 0;  
    KSkill*     pSkill      = NULL;
    ITabFile*   piTabFile   = NULL;
    std::pair<KSkillTable::iterator, bool> InsRet;
    const int   MAX_COLUMN_NAME_LEN = 64;
    char        szMoveState[MAX_COLUMN_NAME_LEN] = "";
    KAttribData  AttributeData;
    KMAP_SKILL_DATA cSkillBinaryData;
    KMAP_SKILL_DATA::iterator it;
    char        szValue[64];
    char	    szScriptName[MAX_PATH];

    bRetCode = LoadSkillBinaryData(cSkillBinaryData);
    KGLOG_PROCESS_ERROR(bRetCode);

    piTabFile = g_OpenTabFile(SETTING_DIR"/"SKILL_LIST_FILE_NAME);
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)(&dwSkillID));
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Step", 0, (int*)(&dwStepID));
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(dwStepID >= 0);
          
        InsRet = m_SkillTable.insert(std::make_pair(MAKE_INT64(dwSkillID, dwStepID), KSkill()));
        KGLOG_PROCESS_ERROR(InsRet.second);

        pSkill = &(InsRet.first->second);

        pSkill->m_dwID = dwSkillID;
        pSkill->m_dwStepID = dwStepID;

        bRetCode = piTabFile->GetInteger(nRowIndex, "NirvanaSkill", 0, &pSkill->m_bNirvanaSkill);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "BelongToSlot", "", szValue, sizeof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = g_ParseMutiInteger(szValue, ";", pSkill->m_vecBelongToSlots);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "SrcBuffID", 0, (int*)(&pSkill->m_dwSrcBuffID));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "DestBuffID", 0, (int*)(&pSkill->m_dwDestBuffID));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AimType", 0, (int*)(&pSkill->m_dwAimType));
        KGLOG_PROCESS_ERROR(bRetCode);

        for (int i = 1; i <= 8; i++)
        {
            bRetCode = LoadAttribute(piTabFile, nRowIndex, "SrcAttribute", i, &AttributeData);
            KGLOG_PROCESS_ERROR(bRetCode);

            if (AttributeData.nKey != atInvalid)
            {
                APPEND_ATTRIB(pSkill->m_pSrcAttribute, AttributeData);
            }

            bRetCode = LoadAttribute(piTabFile, nRowIndex, "DestAttribute", i, &AttributeData);
            KGLOG_PROCESS_ERROR(bRetCode);

            if (AttributeData.nKey != atInvalid)
            {
                APPEND_ATTRIB(pSkill->m_pDestAttribute, AttributeData);
            }
        }

        bRetCode = piTabFile->GetInteger(nRowIndex, "CanCastWithBall", 0, &pSkill->m_bCanCastWithBall);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "SkillType", skilltypeInvalid, (int*)&pSkill->m_eSkillType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AttackIntervalFrame", 0, &pSkill->m_nAttackIntervalFrame);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TargetCostEndurance", 0, &pSkill->m_nTargetCostEndurance);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TargetCostStamina", 0, &pSkill->m_nTargetCostStamina);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TargetCostEndurancePercent", 0, &pSkill->m_nTargetCostEndurancePercent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TargetCostStaminaPercent", 0, &pSkill->m_nTargetCostStaminaPercent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "MoveState", "cmsInvalid", szMoveState, sizeof(szMoveState));
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = ENUM_STR2INT(HERO_MOVESTATE, szMoveState, pSkill->m_nMoveState);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "MoveStateFrame", 0, &pSkill->m_nMoveStateFrame);
        KGLOG_PROCESS_ERROR(bRetCode);
        pSkill->m_nMoveStateFrame = pSkill->m_nMoveStateFrame * GAME_FPS / cdTimeBase;

        bRetCode = piTabFile->GetInteger(nRowIndex, "StepMinTime", 0, &pSkill->m_nStepMinFrame);
        KGLOG_PROCESS_ERROR(bRetCode);
        pSkill->m_nStepMinFrame = pSkill->m_nStepMinFrame * GAME_FPS / cdTimeBase;

        bRetCode = piTabFile->GetInteger(nRowIndex, "StepMaxTime", 0, &pSkill->m_nStepMaxFrame);
        KGLOG_PROCESS_ERROR(bRetCode);
        pSkill->m_nStepMaxFrame = pSkill->m_nStepMaxFrame * GAME_FPS / cdTimeBase;

        bRetCode = piTabFile->GetInteger(nRowIndex, "EndType", 0, (int*)&pSkill->m_eEndType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "EndFrame", 0, &pSkill->m_nEndFrame);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "InterruptLevel", 0, &pSkill->m_nInterruptLevel);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "Script", "", szScriptName, sizeof(szScriptName));
        KGLOG_PROCESS_ERROR(bRetCode);
        szScriptName[sizeof(szScriptName) - 1] = '\0';
        
        if (szScriptName[0] != '\0')
            pSkill->m_dwScriptID = g_FileNameHash(szScriptName);
        else
            pSkill->m_dwScriptID = 0;

        for (int i = 0; i < countof(pSkill->m_SubSkill); ++i)
        {
            char szKey[32] = "";

            sprintf(szKey, "SubSkillID%d", i+1);
            bRetCode = piTabFile->GetInteger(nRowIndex, szKey, ERROR_ID, (int*)&pSkill->m_SubSkill[i].dwID);
            KGLOG_PROCESS_ERROR(bRetCode);

            sprintf(szKey, "SubSkillTriggerType%d", i+1);
            bRetCode = piTabFile->GetInteger(nRowIndex, szKey, KSKILL_TRIGGER_TYPE_INVALID, (int*)&pSkill->m_SubSkill[i].eType);
            KGLOG_PROCESS_ERROR(bRetCode);

            sprintf(szKey, "SubSkillTriggerTimer%d", i+1);
            bRetCode = piTabFile->GetInteger(nRowIndex, szKey, 0, &pSkill->m_SubSkill[i].nFrame);
            KGLOG_PROCESS_ERROR(bRetCode);      
            pSkill->m_SubSkill[i].nFrame = pSkill->m_SubSkill[i].nFrame * GAME_FPS / cdTimeBase;

            sprintf(szKey, "SubSkillRate%d", i+1);
            bRetCode = piTabFile->GetInteger(nRowIndex, szKey, 0, (int*)&pSkill->m_SubSkill[i].dwRate);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        bRetCode = piTabFile->GetInteger(nRowIndex, "SelfAddAngry", 0, &pSkill->m_nSelfAddAngry);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TargetAddAngry", 0, &pSkill->m_nTargetAddAngry);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "NeedAngry", 0, &pSkill->m_nNeedAngry);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CostAngry", 0, &pSkill->m_nCostAngry);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CostAngryPercent", 0, &pSkill->m_nCostAngryPercent);
        KGLOG_PROCESS_ERROR(bRetCode);
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "TargetBuffLimit", 0, (int*)&pSkill->m_dwTargetBuffLimit);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CanCastCountInAir", 0, &pSkill->m_nCanCastCountInAir);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "SpasticityTime", 0, &pSkill->m_nSpasticityFrame);
        KGLOG_PROCESS_ERROR(bRetCode);
        pSkill->m_nSpasticityFrame = pSkill->m_nSpasticityFrame * GAME_FPS / cdTimeBase;

        bRetCode = piTabFile->GetInteger(nRowIndex, "DelayEffect", 0, &pSkill->m_bDelayApplySkillEffect);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Invincible", 0, &pSkill->m_bInvincibleOnSpasticity);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "IsCommonSkill", 0, (int*)&pSkill->m_bIsCommonSkill);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "RequireLevel", 0, (int*)&pSkill->m_nRequireLevel);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "GravityPercent", 0, &pSkill->m_nGravityPercent);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "JudgeBoxMinX", 0, &pSkill->m_nJudgeBoxMinX);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "JudgeBoxMaxX", 0, &pSkill->m_nJudgeBoxMaxX);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "JudgeBoxMinY", 0, &pSkill->m_nJudgeBoxMinY);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "JudgeBoxMaxY", 0, &pSkill->m_nJudgeBoxMaxY);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "JudgeBoxMinZ", 0, &pSkill->m_nJudgeBoxMinZ);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "JudgeBoxMaxZ", 0, &pSkill->m_nJudgeBoxMaxZ);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CanSpecifyCastDir", 0, &pSkill->m_bCanSpecifyCastDir);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CanSpecifyCastingDir", 0, &pSkill->m_bCanSpecifyCastingDir);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetString(nRowIndex, "RestraintType", "",  szValue, countof(szValue));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        szValue[countof(szValue) - 1] = '\0';

        bRetCode = EnumStr2Int("RESTRAINT_TYPE", szValue, &pSkill->m_nRestraintType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "ClearVelocityAtEnd", 0, &pSkill->m_bClearVelocityAtEnd);
        KGLOG_PROCESS_ERROR(bRetCode > 0); 

        bRetCode = piTabFile->GetInteger(nRowIndex, "Series", 0, &pSkill->m_nSeries);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "FireAIEvent", 0, &pSkill->m_bFireAIEvent);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMinX", 0, &pSkill->m_nAIEventMinX);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMaxX", 0, &pSkill->m_nAIEventMaxX);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMinY", 0, &pSkill->m_nAIEventMinY);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMaxY", 0, &pSkill->m_nAIEventMaxY);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMinZ", 0, &pSkill->m_nAIEventMinZ);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AIEventMaxZ", 0, &pSkill->m_nAIEventMaxZ);
        KGLOG_PROCESS_ERROR(bRetCode);

        it = cSkillBinaryData.find(InsRet.first->first);
        if (it != cSkillBinaryData.end())
        {
            pSkill->SetBinaryData(it->second);
        }

        // 以下代码用于验证输入内容的合法性

        KGLOG_PROCESS_ERROR(pSkill->m_eSkillType > skilltypeInvalid && pSkill->m_eSkillType < skilltypeTotal);
        KGLOG_PROCESS_ERROR(pSkill->m_nMoveState >= cmsInvalid && pSkill->m_nMoveState < cmsTotal);

        KGLOG_PROCESS_ERROR(pSkill->m_eEndType >= KSKILL_END_TYPE_NONE && pSkill->m_eEndType < KSKILL_END_TYPE_TOTAL);

        // KGLOG_PROCESS_ERROR(pSkill->m_eSkillType != skilltypeGrab || pSkill->m_eEndType == KSKILL_END_TYPE_BY_TIME);

        KGLOG_PROCESS_ERROR(pSkill->m_eEndType != KSKILL_END_TYPE_BY_TIME || pSkill->m_nEndFrame >= 0);

        KGLOG_PROCESS_ERROR(pSkill->m_nSpasticityFrame >= 0);
        KGLOG_PROCESS_ERROR(pSkill->m_nSpasticityFrame != 0 || !pSkill->m_bDelayApplySkillEffect);
        KGLOG_PROCESS_ERROR(pSkill->m_nSpasticityFrame != 0 || !pSkill->m_bInvincibleOnSpasticity);

        for (int i = 0; i < countof(pSkill->m_SubSkill); ++i)
        {
            KGLOG_PROCESS_ERROR(pSkill->m_SubSkill[i].eType >= KSKILL_TRIGGER_TYPE_INVALID && pSkill->m_SubSkill[i].eType < KSKILL_TRIGGER_TYPE_TOTAL);
            KGLOG_PROCESS_ERROR(pSkill->m_SubSkill[i].dwRate >=0 && pSkill->m_SubSkill[i].dwRate <= SKILL_RATE_BASE);
            KGLOG_PROCESS_ERROR(pSkill->m_SubSkill[i].eType != KSKILL_TRIGGER_TYPE_TIMER || pSkill->m_SubSkill[i].nFrame > 0);
        }

        KGLOG_PROCESS_ERROR(pSkill->m_nSpasticityFrame >= 0);

        KGLOG_PROCESS_ERROR(pSkill->m_nAIEventMinX <= pSkill->m_nAIEventMaxX);
        KGLOG_PROCESS_ERROR(pSkill->m_nAIEventMinY <= pSkill->m_nAIEventMaxY);
        KGLOG_PROCESS_ERROR(pSkill->m_nAIEventMinZ <= pSkill->m_nAIEventMaxZ);
    }

    for (KSkillTable::iterator it = m_SkillTable.begin(); it != m_SkillTable.end(); ++it)
    {
        dwSkillID =  it->second.m_dwID;
        bRetCode = it->second.UpdateOnSkillLoaded();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    if (!bResult)
        KGLogPrintf(KGLOG_ERR, "ID = %u", dwSkillID);

    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KSkillManager::LoadSkillBinaryData(KMAP_SKILL_DATA& allBinarySkillData)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    IFile*              piFile          = NULL;
    BYTE*               pBuffer         = NULL;
    size_t              uBufferSize     = 0;
    uint64_t            uSkillKey       = 0;
    KSkillBinaryData*   pBinaryData     = NULL;
    KAnchor*            pAnchor         = NULL;
    KSkillSegmentInfo*  pSegmentInfo    = NULL;
    KSkillLogicInfo*    pLogicData      = NULL;
    unsigned int        uEnd            = 0;
    BYTE*               pBufferEnd      = NULL;

    piFile = g_OpenFile(SETTING_DIR"/SkillAction.skilllogic");
    KGLOG_PROCESS_ERROR(piFile);

    pBuffer = (BYTE*)piFile->GetBuffer();
    KGLOG_PROCESS_ERROR(pBuffer);

    uBufferSize = piFile->Size();
    KGLOG_PROCESS_ERROR(uBufferSize);

    bRetCode = g_DoBufferSkip(pBuffer, uBufferSize, sizeof(KSkillLogicFileHeader));
    KG_PROCESS_ERROR(bRetCode);

    while (uBufferSize)
    {
        bRetCode = g_ReadFromBufferTo(pBuffer, uBufferSize, uEnd);
        KGLOG_PROCESS_ERROR(bRetCode);

        pBufferEnd = pBuffer + uEnd;

        bRetCode = g_ReadFromBufferAs(pBuffer, uBufferSize, pLogicData);
        KGLOG_PROCESS_ERROR(bRetCode);

        uSkillKey = MAKE_INT64(pLogicData->skillID, pLogicData->skillStep);
        pBinaryData = &allBinarySkillData[uSkillKey];

        pBinaryData->dwID           = pLogicData->skillID;
        pBinaryData->dwStep         = pLogicData->skillStep;
        pBinaryData->nTotalFrame    = pLogicData->frameCount;

        bRetCode = LoadAnchors(pBuffer, uBufferSize, pBinaryData->Anchors);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = LoadSegments(pBuffer, uBufferSize, pBinaryData->Segments);
        KGLOG_PROCESS_ERROR(bRetCode);

        KGLOG_PROCESS_ERROR(pBufferEnd == pBuffer);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piFile);
    return bResult;
}

BOOL KSkillManager::LoadAnchors(BYTE*& pBuffer, size_t& uBufferSize, KMAP_ANCHOR& rMapAnchors)
{
	BOOL                bResult         = false;
	BOOL                bRetCode        = false;
    KAnchor*            pAnchor         = NULL;
	unsigned short      anchorCount     = 0;
    KSkillLogicAnchor*  pBinaryAnchor   = NULL;

    KGLOG_PROCESS_ERROR(pBuffer);
	
    bRetCode = g_ReadFromBufferTo(pBuffer, uBufferSize, anchorCount);
    KGLOG_PROCESS_ERROR(bRetCode);
    for (size_t i = 0; i < anchorCount; ++i)
    {
        bRetCode = g_ReadFromBufferAs(pBuffer, uBufferSize, pBinaryAnchor);
        KGLOG_PROCESS_ERROR(bRetCode);

        pAnchor = &rMapAnchors[pBinaryAnchor->frame];
        pAnchor->nFrame     = pBinaryAnchor->frame;
        pAnchor->nVelocityX = pBinaryAnchor->x;
        pAnchor->nVelocityY = pBinaryAnchor->y;
        pAnchor->nVelocityZ = pBinaryAnchor->z;
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KSkillManager::LoadSegments(BYTE*& pBuffer, size_t& uBufferSize, KMAP_SEGMENT& rMapSegments)
{
	BOOL                bResult             = false;
	BOOL                bRetCode            = false;
    KSkillSegmentInfo*  pSegmentInfo        = NULL;
    unsigned short      segmentCount        = 0;
    KSkillLogicSegment* pBinarySegmentInfo  = NULL;
	
	KGLOG_PROCESS_ERROR(pBuffer);

    bRetCode = g_ReadFromBufferTo(pBuffer, uBufferSize, segmentCount);
    for (size_t i = 0; i < segmentCount; ++i)
    {
        bRetCode = g_ReadFromBufferAs(pBuffer, uBufferSize, pBinarySegmentInfo);
        KGLOG_PROCESS_ERROR(bRetCode);

        pSegmentInfo = &rMapSegments[pBinarySegmentInfo->frame];
        pSegmentInfo->nFrame                = pBinarySegmentInfo->frame;
        pSegmentInfo->bAttackState          = pBinarySegmentInfo->attackState;
        pSegmentInfo->nBeAttackType         = pBinarySegmentInfo->beAttackType;
        pSegmentInfo->dwSelfBuffID          = pBinarySegmentInfo->srcBuffID;
        pSegmentInfo->dwTargetBuffID        = pBinarySegmentInfo->dstBuffID;
        pSegmentInfo->nAttackIntervalFrame  = pBinarySegmentInfo->attackIntervalFrame;
        pSegmentInfo->nStiffFrame           = pBinarySegmentInfo->stiffFrameCount;

        bRetCode = LoadAttributes(pBuffer, uBufferSize, pSegmentInfo->vecSelfAttributes);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = LoadAttributes(pBuffer, uBufferSize, pSegmentInfo->vecTargetAttributes);
        KGLOG_PROCESS_ERROR(bRetCode);
        
        bRetCode = LoadBullets(pBuffer, uBufferSize, pSegmentInfo->vecBullet);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KSkillManager::LoadAttributes(BYTE*& pBuffer, size_t& uBufferSize, std::vector<KAttribData>& rVecAttributes)
{
	BOOL            bResult             = false;
	BOOL            bRetCode            = false;
    KAttribData*    pAttribData         = NULL;
	unsigned short  attributeCount      = 0;
    const size_t    MAX_ATTRIBUTE_NAME_SIZE    = 32;
    char            szAttributeName[MAX_ATTRIBUTE_NAME_SIZE];

    bRetCode = g_ReadFromBufferTo(pBuffer, uBufferSize, attributeCount);
    KGLOG_PROCESS_ERROR(bRetCode);
	
    rVecAttributes.resize(attributeCount);
    for (size_t i = 0; i < attributeCount; ++i)
    {
        bRetCode = g_ReadStringFromBuffer(pBuffer, uBufferSize, szAttributeName, countof(szAttributeName));
        KGLOG_PROCESS_ERROR(bRetCode);
        szAttributeName[countof(szAttributeName) - 1] = '\0';

        bRetCode = ENUM_STR2INT(ATTRIBUTE_TYPE, szAttributeName,rVecAttributes[i].nKey);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = g_ReadFromBufferTo(pBuffer, uBufferSize, rVecAttributes[i].nValue1);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = g_ReadFromBufferTo(pBuffer, uBufferSize, rVecAttributes[i].nValue2);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KSkillManager::LoadBullets(BYTE*& pBuffer, size_t& uBufferSize, std::vector<DWORD>& rVecBullets)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	unsigned short bulletCount = 0;
    unsigned short bulletID = 0;

	KGLOG_PROCESS_ERROR(pBuffer);

    bRetCode = g_ReadFromBufferTo(pBuffer, uBufferSize, bulletCount);
    KGLOG_PROCESS_ERROR(bRetCode);
    
    rVecBullets.resize(bulletCount);
    for (size_t i = 0; i < bulletCount; ++i)
    {
        bRetCode = g_ReadFromBufferTo(pBuffer, uBufferSize, bulletID);
        KGLOG_PROCESS_ERROR(bRetCode);

        rVecBullets[i] = bulletID;
    }

	bResult = true;
Exit0:
	return bResult;
}

