#include "stdafx.h"
#include <assert.h>
#include "KHeroDataList.h"
#include "KRoleDBDataDef.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KSO3World.h"
#include "KLSClient.h"
#include <algorithm>

int KHeroData::GetUsedTalentCount()
{
    int nCount = 0;
    for (int i = egymWeightLift; i < egymTotal; ++i)
    {
        nCount += m_wTalent[i];
    }
    return nCount;
}

// 精确到万分之一，不要轻易改
int KHeroData::GetLadderWinRate()
{
    int nWinRate = 0;
    int nToltalCount = m_nLadderWinCount + m_nLadderLoseCount;

    if (nToltalCount > 0)
    {
        nWinRate = 10000 * (int64_t)(m_nLadderWinCount) / nToltalCount;
    }
    
    MAKE_IN_RANGE(nWinRate, 0, 10000);

    return nWinRate;
}

void KHeroData::OnEnterScene()
{
    if (m_bEnterScene) // 上次进入场景没有调用OnLeaveScene
    {
        KGLogPrintf(KGLOG_ERR, "Should call OnLeaveScene!!!");
        OnLeaveScene();
    }

    m_bEnterScene = true;
    m_nOldExpMultiple = m_nExpMultiple;
}

void KHeroData::OnLeaveScene()
{
    m_bEnterScene = false;
    m_nExpMultiple = m_nOldExpMultiple;
}

//////////////////////////////////////////////////////////////////////////

KHeroDataList::KHeroDataList()
{
    m_nMaxHeroCount = 0;
    m_nFiredHeroCount = 0;
}

KHeroDataList::~KHeroDataList()
{
}

BOOL KHeroDataList::Init(KPlayer* pPlayer)
{
    m_pPlayer = pPlayer;
    return true;
}

void KHeroDataList::UnInit()
{
}

BOOL KHeroDataList::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult = false;
    T3DB::KPBHeroDataList* pHeroDataList = NULL;
    T3DB::KPB_HERO_DATA_LIST* pHeroList = NULL;
    T3DB::KPB_BUFF_DATA_LIST* pBuffDataListDB = NULL;
    KHeroData* pHeroData = NULL;
    KBUFF_NODE_MAP::iterator itBuffNode;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pHeroDataList = pSaveBuf->mutable_herodatalist();
    pHeroList = pHeroDataList->mutable_herodatalistdb();
    pBuffDataListDB = pHeroDataList->mutable_buffdatalistdb();

    pHeroList->set_maxherocount(m_nMaxHeroCount);

    for (KMAP_HERO_DATA::iterator it = m_mapHeroData.begin(); it != m_mapHeroData.end(); ++it)
    {
        pHeroData = &it->second;
        T3DB::KPB_HERO_DATA* pPBHeroData = pHeroDataList->add_herodata();
        
        pPBHeroData->set_index(it->first);
        pPBHeroData->set_level(pHeroData->m_nLevel);
        pPBHeroData->set_templateid(pHeroData->m_dwTemplateID);
        pPBHeroData->set_exp(pHeroData->m_nExp);
        pPBHeroData->set_eol(pHeroData->m_nEol);
        pPBHeroData->set_ladderlevel(pHeroData->m_nLadderLevel);
        pPBHeroData->set_ladderexp(pHeroData->m_nLadderExp);
        pPBHeroData->set_ladderwincount(pHeroData->m_nLadderWinCount);
        pPBHeroData->set_ladderlosecount(pHeroData->m_nLadderLoseCount);
        pPBHeroData->set_ladderdrawcount(pHeroData->m_nLadderDrawCount);
        pPBHeroData->set_freetalent(pHeroData->m_wFreeTalent);
        pPBHeroData->set_fashionid(pHeroData->m_wFashionID);
        pPBHeroData->set_fired(pHeroData->m_bFired);

        pPBHeroData->add_talent(pHeroData->m_wTalent[egymWeightLift]);
        pPBHeroData->add_talent(pHeroData->m_wTalent[egymRunning]);
        pPBHeroData->add_talent(pHeroData->m_wTalent[egymBoxing]);

        pPBHeroData->set_equipscore(m_pPlayer->m_ItemList.GetHeroEquipScore(pHeroData->m_dwTemplateID));

        for (int i = 0; i < KACTIVE_SLOT_TYPE_TOTAL; ++i)
        {
            pPBHeroData->add_selectedactiveskill(pHeroData->m_SelectedActvieSkill[i]);
        }

        for (int i = 0; i < cdPassiveSkillSlotCount; ++i)
        {
            pPBHeroData->add_selectedpassiveskill(pHeroData->m_SelectedPassiveSkill[i]);
        }

        for (int i = 0; i < cdNirvanaSkillSlotCount; ++i)
        {
            pPBHeroData->add_selectednirvanaskill(pHeroData->m_SelectedNirvanaSkill[i]);
        }

        for (int i = 0; i < cdSlamBallSkillSlotCount; ++i)
        {
            pPBHeroData->add_selectedslamballskill(pHeroData->m_SelectedSlamBallSkill[i]);
        }

        // 存储buff数据
        
        itBuffNode = pHeroData->m_mapBuffNode.begin();
        KGLOG_CHECK_ERROR(pHeroData->m_mapBuffNode.size() <= HUNDRED_NUM);
        while (itBuffNode != pHeroData->m_mapBuffNode.end())
        {
            T3DB::KPB_BUFF_DATA* pBuffData = pBuffDataListDB->add_buffdata();
            pBuffData->set_buffid(itBuffNode->first);
            pBuffData->set_leftframe(itBuffNode->second.m_nActiveCount);
            pBuffData->set_activecount(itBuffNode->second.m_nLeftFrame);
            ++itBuffNode;
        }
    }

    bResult = true;
Exit0:
    return bResult;
}
    
BOOL KHeroDataList::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL            bResult         = false;
    KHeroData*      pHeroData       = NULL;
    KHeroPackage*   pHeroPackage    = NULL;
    std::pair<KMAP_HERO_DATA::iterator, bool> InsRet;

    assert(pLoadBuf);

    const T3DB::KPBHeroDataList& HeroDataList = pLoadBuf->herodatalist();
    const T3DB::KPB_HERO_DATA_LIST& HeroList = HeroDataList.herodatalistdb();
    const T3DB::KPB_BUFF_DATA_LIST& BuffDataListDB = HeroDataList.buffdatalistdb();

    m_nMaxHeroCount = HeroList.maxherocount();

    if (m_pPlayer->m_eConnectType == eLogin)
    {
        g_PlayerServer.DoSyncMaxHeroCount(m_pPlayer->m_nConnIndex, m_nMaxHeroCount);
    }

    for (int i = 0; i < HeroDataList.herodata_size(); ++i)
    {
        KHeroData HeroData;
        const T3DB::KPB_HERO_DATA& PBHeroData = HeroDataList.herodata(i);

        HeroData.m_dwTemplateID = PBHeroData.templateid();
        HeroData.m_nLevel = PBHeroData.level();
        HeroData.m_nExp = PBHeroData.exp();
        HeroData.m_nExpMultiple = 1;
        HeroData.m_nEol = PBHeroData.eol();
        HeroData.m_nLadderLevel = PBHeroData.ladderlevel();
        HeroData.m_nLadderExp = PBHeroData.ladderexp();
        HeroData.m_nLadderWinCount = PBHeroData.ladderwincount();
        HeroData.m_nLadderLoseCount = PBHeroData.ladderlosecount();
        HeroData.m_nLadderDrawCount = PBHeroData.ladderdrawcount();
        HeroData.m_wFreeTalent = (WORD)PBHeroData.freetalent();
        HeroData.m_wFashionID = (WORD)PBHeroData.fashionid();
        HeroData.m_bFired = (BOOL)PBHeroData.fired();
        if (HeroData.m_bFired)
            m_nFiredHeroCount++;

        HeroData.m_nExpMultiple = 1;

        HeroData.m_wTalent[egymWeightLift] = (WORD)PBHeroData.talent(egymWeightLift);
        HeroData.m_wTalent[egymRunning] = (WORD)PBHeroData.talent(egymRunning);
        HeroData.m_wTalent[egymBoxing] = (WORD)PBHeroData.talent(egymBoxing);

        memset(HeroData.m_SelectedActvieSkill, 0, sizeof(HeroData.m_SelectedActvieSkill));
        for (int j = 0; j < PBHeroData.selectedactiveskill_size(); ++j)
        {
            if (j >= countof(HeroData.m_SelectedActvieSkill))
                break;

            HeroData.m_SelectedActvieSkill[j] = PBHeroData.selectedactiveskill(j);
        }

        memset(HeroData.m_SelectedPassiveSkill, 0, sizeof(HeroData.m_SelectedPassiveSkill));
        for (int j = 0; j < PBHeroData.selectedpassiveskill_size(); ++j)
        {
            if (j >= countof(HeroData.m_SelectedPassiveSkill))
                break;

            HeroData.m_SelectedPassiveSkill[j] = PBHeroData.selectedpassiveskill(j);
        }

        memset(HeroData.m_SelectedNirvanaSkill, 0, sizeof(HeroData.m_SelectedNirvanaSkill));
        for (int j = 0; j < PBHeroData.selectednirvanaskill_size(); ++j)
        {
            if (j >= countof(HeroData.m_SelectedNirvanaSkill))
                break;

            HeroData.m_SelectedNirvanaSkill[j] = PBHeroData.selectednirvanaskill(j);
        }

        memset(HeroData.m_SelectedSlamBallSkill, 0, sizeof(HeroData.m_SelectedSlamBallSkill));
        for (int j = 0; j < PBHeroData.selectedslamballskill_size(); ++j)
        {
            if (j >= countof(HeroData.m_SelectedSlamBallSkill))
                break;

            HeroData.m_SelectedSlamBallSkill[j] = PBHeroData.selectedslamballskill(j);
        }

        HeroData.m_bEnterScene = false;
        HeroData.m_nOldExpMultiple = 0;

        InsRet = m_mapHeroData.insert(make_pair(HeroData.m_dwTemplateID, HeroData));
        KGLOG_PROCESS_ERROR(InsRet.second);
        
        pHeroData = &InsRet.first->second;

        pHeroPackage = m_pPlayer->m_ItemList.AddHeroPackage(HeroData.m_dwTemplateID);
        KGLOG_PROCESS_ERROR(pHeroPackage);

        if (m_pPlayer->m_eConnectType == eLogin)
        {
            g_PlayerServer.DoSyncHeroData(m_pPlayer->m_nConnIndex, pHeroData);
        }

        for (int j = 0; j < BuffDataListDB.buffdata_size(); ++j)
        {
            KBuffNode Node;
            const T3DB::KPB_BUFF_DATA& PBBuffData = BuffDataListDB.buffdata(j);

            Node.m_nActiveCount = PBBuffData.activecount();
            Node.m_nLeftFrame    = PBBuffData.leftframe();

            pHeroData->m_mapBuffNode[PBBuffData.buffid()] = Node;
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

size_t KHeroDataList::GetHeroCount()
{
    return m_mapHeroData.size();
}

size_t KHeroDataList::GetActiveHeroCount()
{
    return m_mapHeroData.size() - m_nFiredHeroCount;
}

KHeroData* KHeroDataList::GetHeroData(DWORD dwTemplateID)
{
    KHeroData* pResult = NULL;
    KMAP_HERO_DATA::iterator it = m_mapHeroData.find(dwTemplateID);
    KG_PROCESS_ERROR(it != m_mapHeroData.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

BOOL KHeroDataList::CanAddHeroData(DWORD dwTemplateID)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    int  nCurHeroCount = (int)GetActiveHeroCount();
    KMAP_HERO_DATA::iterator it;

    KG_PROCESS_ERROR(nCurHeroCount + 1 <= m_nMaxHeroCount);

    it = m_mapHeroData.find(dwTemplateID);
    KG_PROCESS_ERROR(it == m_mapHeroData.end());

	bResult = true;
Exit0:
	return bResult;
}

BOOL KHeroDataList::AddHeroData(DWORD dwTemplateID, int nLevel)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KHeroData HeroData = {0};
    std::pair<KMAP_HERO_DATA::iterator, bool> InsRet;
    KHeroTemplateInfo* pHeroTemplate = NULL;

    KGLOG_PROCESS_ERROR(nLevel > 0);

    pHeroTemplate = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
	KGLOG_PROCESS_ERROR(pHeroTemplate);
	
    HeroData.m_dwTemplateID             = dwTemplateID;
    HeroData.m_nLevel                   = nLevel;
    HeroData.m_nExp                     = 0;
    HeroData.m_nEol                     = INIT_HERO_EOL;
    HeroData.m_nExpMultiple             = 1;
    HeroData.m_nLadderExp               = 0;
    HeroData.m_nLadderLevel             = 1;
    HeroData.m_nLadderWinCount          = 0;
    HeroData.m_nLadderLoseCount         = 0;
    HeroData.m_nLadderDrawCount         = 0;
    HeroData.m_wFreeTalent              = (WORD)nLevel;
    HeroData.m_wTalent[egymWeightLift]  = 0;
    HeroData.m_wTalent[egymRunning]     = 0;
    HeroData.m_wTalent[egymBoxing]      = 0;

    for (int i = 0; i < KACTIVE_SLOT_TYPE_TOTAL; ++i)
    {
        HeroData.m_SelectedActvieSkill[i] = pHeroTemplate->AllActiveSkill[i][0];
    }

    HeroData.m_nOldExpMultiple = 0;
    HeroData.m_bEnterScene = false;

    InsRet = m_mapHeroData.insert(make_pair(HeroData.m_dwTemplateID, HeroData));
    KGLOG_PROCESS_ERROR(InsRet.second);
    
    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroDataList::RemoveHeroData(DWORD dwTemplateID)
{
    m_mapHeroData.erase(dwTemplateID);
    return true;
}

BOOL KHeroDataList::HasHero(DWORD dwTemplateID) const
{
    BOOL bResult = false;
    KMAP_HERO_DATA::const_iterator constIt = m_mapHeroData.find(dwTemplateID);

    bResult = (constIt != m_mapHeroData.end());

    return bResult;
}

BOOL KHeroDataList::AddMaxHeroCount(int nAddCount)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;

    KGLOG_PROCESS_ERROR(nAddCount > 0);

    m_nMaxHeroCount += nAddCount;

    g_PlayerServer.DoSyncMaxHeroCount(m_pPlayer->m_nConnIndex, m_nMaxHeroCount);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KHeroDataList::SaveHeroData(KHero* pHero)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KHeroData* pHeroData = NULL;

    assert(pHero);

    pHeroData = GetHeroData(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    bRetCode = pHero->m_BuffList.Save(pHeroData->m_mapBuffNode);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KHeroDataList::DecreaseAllCompetitorHeroExp(int nPercent)
{
    BOOL                    bRetCode    = false;
    KHeroData*              pHeroData   = NULL;
    KLADDER_LEVELDATA_ITEM* pBase       = NULL;
    KLADDER_LEVELDATA_ITEM* pCurrent    = NULL;
    int                     nTotalLadderExp = 0;
    BOOL                    bDecreased  = false;

    pBase = g_pSO3World->m_Settings.m_LadderLevelData.GetLadderLevelData(COMPETITOR_LEVEL);
    KGLOG_PROCESS_ERROR(pBase);

    for (KMAP_HERO_DATA::iterator it = m_mapHeroData.begin(); it != m_mapHeroData.end(); ++it)
    {
        pHeroData = &it->second;
        if (pHeroData->m_nLadderLevel < COMPETITOR_LEVEL)
            continue;

        pCurrent = g_pSO3World->m_Settings.m_LadderLevelData.GetLadderLevelData(pHeroData->m_nLadderLevel);
        KGLOG_PROCESS_ERROR(pCurrent);

        nTotalLadderExp = pCurrent->m_nTotalExp - pBase->m_nTotalExp +  pHeroData->m_nLadderExp;
        bRetCode = m_pPlayer->AddHeroLadderExp(pHeroData->m_dwTemplateID, - nTotalLadderExp * nPercent / HUNDRED_NUM);
        KGLOG_PROCESS_ERROR(bRetCode);

        g_LSClient.DoLadderDataChangeNotify(m_pPlayer, pHeroData->m_dwTemplateID);
    }

Exit0:
    return;
}

KHeroData* KHeroDataList::FindFreeHero()
{
    KHeroData* pNewHero = NULL;
    for (KMAP_HERO_DATA::iterator it = m_mapHeroData.begin(); it != m_mapHeroData.end(); ++it)
    {
       pNewHero = &it->second;
       if (!pNewHero->m_bFired 
           && pNewHero->m_dwTemplateID != m_pPlayer->m_dwAssistHeroTemplateID 
           && pNewHero->m_dwTemplateID != m_pPlayer->m_dwMainHeroTemplateID)
        break;
    }
    return pNewHero;
}

BOOL KHeroDataList::Fire(const DWORD dwTemplateID)
{
	BOOL bResult 	= false;
    KHeroData* pHero = NULL;
    
    pHero = GetHeroData(dwTemplateID);
    KGLOG_PROCESS_ERROR(pHero && pHero->m_bFired == false);

    pHero->m_bFired = true; 
    m_nFiredHeroCount++;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroDataList::UnFire(const DWORD dwTemplateID)
{
	BOOL bResult 	= false;
    KHeroData* pHero = NULL;
    
    pHero = GetHeroData(dwTemplateID);
    KG_PROCESS_ERROR(pHero && pHero->m_bFired);
 
    pHero->m_bFired = false;
    m_nFiredHeroCount--;

    bResult = true;
Exit0:
    return bResult;
}

int KHeroDataList::GetHighestLadderLevel()
{
    int nResult = 0;

    KHeroData* pNewHero = NULL;
    for (KMAP_HERO_DATA::iterator it = m_mapHeroData.begin(); it != m_mapHeroData.end(); ++it)
    {
        pNewHero = &it->second;
        if (nResult < pNewHero->m_nLadderLevel)
        {
            nResult = pNewHero->m_nLadderLevel;
        }
    }
    return nResult;
}

BOOL KHeroDataList::SelectHeroSkill(DWORD dwHeroTemplateID, int nSkillGroup, int nSlotIndex, DWORD dwSkillID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KHeroData* pHeroData = NULL;
    int nOldSlotIndex = -1;
    KSkillSlotCount* pSlotCountInfo = NULL;

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);
    KGLOG_PROCESS_ERROR(!pHeroData->m_bFired);

    pSlotCountInfo = g_pSO3World->m_Settings.m_SkillSlotCountSettings.GetByID(pHeroData->m_nLevel);
    KGLOG_PROCESS_ERROR(pSlotCountInfo);

    switch(nSkillGroup)
    {
    case KSKILL_GROUP_ACTIVE:
        {
            KSkill* pSkill = NULL;

            KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHeroData->m_SelectedActvieSkill));
            
            bRetCode = IsHeroLearnedActiveSkill(dwHeroTemplateID, dwSkillID, 0);
            KGLOG_PROCESS_ERROR(bRetCode);

            pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID, 0);
            KGLOG_PROCESS_ERROR(pSkill);

            bRetCode = pSkill->CanPluginInSlot(nSlotIndex);
            KGLOG_PROCESS_ERROR(bRetCode);

            pHeroData->m_SelectedActvieSkill[nSlotIndex] = dwSkillID;

            if (dwSkillID == 79 || dwSkillID == 23)
            {
                m_pPlayer->OnEvent(peSelectQianChongQuan);
            }
            else if (dwSkillID == 82 || dwSkillID == 29)
            {
                m_pPlayer->OnEvent(peSelectKongZhongJiao);
            }

        }

        break;
    case KSKILL_GROUP_PASSIVE:
        {
            KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHeroData->m_SelectedPassiveSkill));
            KGLOG_PROCESS_ERROR(nSlotIndex < pSlotCountInfo->nPassiveSkillSlotCount);

            bRetCode = IsHeroLearnedPassiveSkill(dwHeroTemplateID, dwSkillID);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = GetPassiveSkillBelongToSlot(dwHeroTemplateID, dwSkillID, nOldSlotIndex);
            if (bRetCode)
            {
                KGLOG_PROCESS_ERROR(nOldSlotIndex >= 0 && nOldSlotIndex < countof(pHeroData->m_SelectedPassiveSkill));
                swap(pHeroData->m_SelectedPassiveSkill[nSlotIndex], pHeroData->m_SelectedPassiveSkill[nOldSlotIndex]);
            }
            else
            {
                pHeroData->m_SelectedPassiveSkill[nSlotIndex] = dwSkillID;
            }
        }
        break;
    case KSKILL_GROUP_NIRVANA:
        {
            KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHeroData->m_SelectedNirvanaSkill));
            KGLOG_PROCESS_ERROR(nSlotIndex < pSlotCountInfo->nNirvanaSkillSlotCount);

            bRetCode = IsHeroLearnedNirvanaSkill(dwHeroTemplateID, dwSkillID);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = GetNirvanaSkillBelongToSlot(dwHeroTemplateID, dwSkillID, nOldSlotIndex);
            if (bRetCode)
            {
                KGLOG_PROCESS_ERROR(nOldSlotIndex >= 0 && nOldSlotIndex < countof(pHeroData->m_SelectedNirvanaSkill));
                swap(pHeroData->m_SelectedNirvanaSkill[nSlotIndex], pHeroData->m_SelectedNirvanaSkill[nOldSlotIndex]);
            }
            else
            {
                pHeroData->m_SelectedNirvanaSkill[nSlotIndex] = dwSkillID;
            }
        }
        break;
    case KSKILL_GROUP_SLAMBALL:
        {
            KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHeroData->m_SelectedSlamBallSkill));
            KGLOG_PROCESS_ERROR(nSlotIndex < pSlotCountInfo->nSlamBallSlotCount);

            bRetCode = IsHeroLearnedSlamBallSkill(dwHeroTemplateID, dwSkillID);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = GetSlamBallSkillBelongToSlot(dwHeroTemplateID, dwSkillID, nOldSlotIndex);
            if (bRetCode)
            {
                KGLOG_PROCESS_ERROR(nOldSlotIndex >= 0 && nOldSlotIndex < countof(pHeroData->m_SelectedSlamBallSkill));
                swap(pHeroData->m_SelectedSlamBallSkill[nSlotIndex], pHeroData->m_SelectedSlamBallSkill[nOldSlotIndex]);
            }
            else
            {
                pHeroData->m_SelectedSlamBallSkill[nSlotIndex] = dwSkillID;
            }
        }
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    g_PlayerServer.DoSyncHeroData(m_pPlayer->m_nConnIndex, pHeroData);

    m_pPlayer->OnEvent(peSelectHeroSkill);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroDataList::IsHeroLearnedActiveSkill(DWORD dwHeroTemplateID, DWORD dwSkillID, DWORD dwStep)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	BOOL bHasThisSkill = false;
    KSkill* pSkill = NULL;
    KHeroData* pHeroData = NULL;
    KHeroTemplateInfo* pHeroTemplateInfo = NULL;
    KVEC_DWORD::iterator it;
    
    pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID, dwStep);
    KGLOG_PROCESS_ERROR(pSkill);

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_nLevel >= pSkill->m_nRequireLevel);

    pHeroTemplateInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroTemplateInfo);

    for (int i = 0; i < countof(pHeroTemplateInfo->AllActiveSkill); ++i)
    {
        it = std::find(pHeroTemplateInfo->AllActiveSkill[i].begin(), pHeroTemplateInfo->AllActiveSkill[i].end(), dwSkillID);
        if(it != pHeroTemplateInfo->AllActiveSkill[i].end())
        {
            bHasThisSkill = true;
            break;
        }
    }

    KGLOG_PROCESS_ERROR(bHasThisSkill);

    bResult = true;
Exit0:
	return bResult;
}

BOOL KHeroDataList::IsHeroLearnedPassiveSkill(DWORD dwHeroTemplateID, DWORD dwSkillID)
{
    BOOL bResult  = false;
    KPassiveSkillData* pSkill = NULL;
    KHeroData* pHeroData = NULL;
    KHeroTemplateInfo* pHeroTemplateInfo = NULL;
    KVEC_DWORD::iterator it;

    pSkill = g_pSO3World->m_Settings.m_PassiveSkillData.GetByID(dwSkillID);
    KGLOG_PROCESS_ERROR(pSkill);

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_nLevel >= pSkill->nRequireLevel);

    pHeroTemplateInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroTemplateInfo);

    it = std::find(pHeroTemplateInfo->AlllPassiveSkill.begin(), pHeroTemplateInfo->AlllPassiveSkill.end(), dwSkillID);
    if (it != pHeroTemplateInfo->AlllPassiveSkill.end())
        bResult = true;

Exit0:
    return bResult;
}

BOOL KHeroDataList::IsHeroLearnedNirvanaSkill(DWORD dwHeroTemplateID, DWORD dwSkillID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    BOOL bHasThisSkill = false;
    KSkill* pSkill = NULL;
    KHeroData* pHeroData = NULL;
    KHeroTemplateInfo* pHeroTemplateInfo = NULL;
    KVEC_DWORD::iterator it;

    pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID, 0);
    KGLOG_PROCESS_ERROR(pSkill);

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_nLevel >= pSkill->m_nRequireLevel);

    pHeroTemplateInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroTemplateInfo);

    for (int i = 0; i < countof(pHeroTemplateInfo->AllNirvanaSkill); ++i)
    {
        it = std::find(pHeroTemplateInfo->AllNirvanaSkill.begin(), pHeroTemplateInfo->AllNirvanaSkill.end(), dwSkillID);
        if(it != pHeroTemplateInfo->AllNirvanaSkill.end())
        {
            bHasThisSkill = true;
            break;
        }
    }

    KGLOG_PROCESS_ERROR(bHasThisSkill);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroDataList::IsHeroLearnedSlamBallSkill(DWORD dwHeroTemplateID, DWORD dwSkillID)
{
    BOOL bResult  = false;
    KSlamBallSkill* pSkill = NULL;
    KHeroData* pHeroData = NULL;
    KHeroTemplateInfo* pHeroTemplateInfo = NULL;
    KVEC_DWORD::iterator it;

    pSkill = g_pSO3World->m_Settings.m_SlamBallSkillSettings.Get(dwSkillID);
    KGLOG_PROCESS_ERROR(pSkill);

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_nLevel >= pSkill->nRequireLevel);

    pHeroTemplateInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroTemplateInfo);

    it = std::find(pHeroTemplateInfo->AllSlamBallSkill.begin(), pHeroTemplateInfo->AllSlamBallSkill.end(), dwSkillID);
    KGLOG_PROCESS_ERROR(it != pHeroTemplateInfo->AllSlamBallSkill.end());

	bResult = true;
Exit0:
    return bResult;
}

BOOL KHeroDataList::GetActiveSkillBelongToSlot(DWORD dwHeroTemplateID, DWORD dwSkillID, int& nSlotIndex)
{
	BOOL bResult = false;
    KHeroData* pHeroData = NULL;

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

	for (int i = 0; i < countof(pHeroData->m_SelectedActvieSkill); ++i)
	{
        if (pHeroData->m_SelectedActvieSkill[i] == dwSkillID)
        {
            nSlotIndex = i;
            bResult = true;
            break;
        }
	}

Exit0:
	return bResult;
}

BOOL KHeroDataList::GetPassiveSkillBelongToSlot(DWORD dwHeroTemplateID, DWORD dwSkillID, int& nSlotIndex)
{
    BOOL bResult = false;
    KHeroData* pHeroData = NULL;

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    for (int i = 0; i < countof(pHeroData->m_SelectedPassiveSkill); ++i)
    {
        if (pHeroData->m_SelectedPassiveSkill[i] == dwSkillID)
        {
            nSlotIndex = i;
            bResult = true;
            break;
        }
    }

Exit0:
    return bResult;
}

BOOL KHeroDataList::GetNirvanaSkillBelongToSlot(DWORD dwHeroTemplateID, DWORD dwSkillID, int& nSlotIndex)
{
    BOOL bResult = false;
    KHeroData* pHeroData = NULL;

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    for (int i = 0; i < countof(pHeroData->m_SelectedNirvanaSkill); ++i)
    {
        if (pHeroData->m_SelectedNirvanaSkill[i] == dwSkillID)
        {
            nSlotIndex = i;
            bResult = true;
            break;
        }
    }

Exit0:
    return bResult;
}

BOOL KHeroDataList::GetSlamBallSkillBelongToSlot(DWORD dwHeroTemplateID, DWORD dwSkillID, int& nSlotIndex)
{
    BOOL bResult = false;
    KHeroData* pHeroData = NULL;

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    for (int i = 0; i < countof(pHeroData->m_SelectedSlamBallSkill); ++i)
    {
        if (pHeroData->m_SelectedSlamBallSkill[i] == dwSkillID)
        {
            nSlotIndex = i;
            bResult = true;
            break;
        }
    }

Exit0:
    return bResult;
}

BOOL KHeroDataList::AddBuffTime(DWORD dwHeroTemplateID, DWORD dwBuffID, int nFrames)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KHeroData* pHeroData = NULL;
    KBuff* pBuff = NULL;
    KBUFF_NODE_MAP::iterator it;
    KBuffNode* pBuffNode = NULL;

    pHeroData = GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    pBuff = g_pSO3World->m_BuffManager.GetBuff(dwBuffID);
    KGLOG_PROCESS_ERROR(pBuff);
    KGLOG_PROCESS_ERROR(pBuff->m_nActiveCount == 1);

    KGLOG_PROCESS_ERROR(nFrames > 0);

    it = pHeroData->m_mapBuffNode.find(dwBuffID);
	if (it != pHeroData->m_mapBuffNode.end())
	{
        it->second.m_nLeftFrame += nFrames;
	}
    else
    {
        pBuffNode = &pHeroData->m_mapBuffNode[dwBuffID];
        pBuffNode->m_nActiveCount = 1;
        pBuffNode->m_nLeftFrame = nFrames;
    }

	bResult = true;
Exit0:
	return bResult;
}

KHeroData* KHeroDataList::GetHeroDataOfHighestLadder()
{
    KHeroData* pResult = NULL;
    KHeroData* pTemp   = NULL;

    for (KMAP_HERO_DATA::iterator it = m_mapHeroData.begin(); it != m_mapHeroData.end(); ++it)
    {
        if (it->second.m_bFired)
            continue;

        if (!pTemp)
        {
            pTemp = &it->second;
            continue;
        }

        if (GreaterThanByLadder(&it->second, pTemp))
        {
            pTemp = &it->second;
            continue;
        }
    }

    pResult = pTemp;
Exit0:
    return pResult;
}

BOOL KHeroDataList::GreaterThanByLadder(KHeroData* pLeft, KHeroData* pRight)
{
    int nLeftWinRate = 0;
    int nRightWinRate = 0;

    if (pLeft->m_nLadderLevel != pRight->m_nLadderLevel)
        return pLeft->m_nLadderLevel > pRight->m_nLadderLevel;
    
    if (pLeft->m_nLadderExp != pRight->m_nLadderExp)
        return pLeft->m_nLadderExp > pRight->m_nLadderExp;

    nLeftWinRate = pLeft->GetLadderWinRate();
    nRightWinRate = pRight->GetLadderWinRate();

    if (nLeftWinRate != nRightWinRate)
        return nLeftWinRate > nRightWinRate;

    return pLeft->m_nLevel > pRight->m_nLevel;
}

void KHeroDataList::GetHeroList(std::vector<DWORD>& vecHeroList)
{
    KHeroData* pHeroData = NULL;

    for (KMAP_HERO_DATA::iterator it = m_mapHeroData.begin(); it != m_mapHeroData.end(); ++it)
    {
        pHeroData = &it->second;

        if (!pHeroData->m_bFired)
            vecHeroList.push_back(pHeroData->m_dwTemplateID);
    }
}