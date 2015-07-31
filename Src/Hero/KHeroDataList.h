// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KHeroDataList.h 
//  Creator 	: Xiayong  
//  Date		: 02/05/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include <map>
#include "Engine/KMemory.h"
#include "Ksdef/TypeDef.h"

struct KBuffNode
{
    int    m_nLeftFrame;
    int    m_nActiveCount;
};
typedef std::map<DWORD, KBuffNode, std::less<DWORD>, KMemory::KAllocator<std::pair<DWORD, KBuffNode> > > KBUFF_NODE_MAP;
typedef std::vector<DWORD, KMemory::KAllocator<DWORD> > KVEC_SKILL_LIST;
struct KHeroData
{
    DWORD       m_dwTemplateID; // 英雄模板id
    int         m_nLevel;
    int         m_nExp;
    int         m_nExpMultiple; // 增加经验时的倍率,由buff控制存盘
    int         m_nEol;
    int         m_nLadderLevel;
    int         m_nLadderExp;
    int         m_nLadderWinCount;
    int         m_nLadderLoseCount;
    int         m_nLadderDrawCount;
    WORD        m_wFreeTalent;
    WORD        m_wTalent[egymTotal];
    WORD        m_wFashionID;
    BOOL        m_bFired;       //是否是解雇状态
    DWORD       m_SelectedActvieSkill[KACTIVE_SLOT_TYPE_TOTAL];
    DWORD       m_SelectedPassiveSkill[cdPassiveSkillSlotCount];
    DWORD       m_SelectedNirvanaSkill[cdNirvanaSkillSlotCount];
    DWORD       m_SelectedSlamBallSkill[cdSlamBallSkillSlotCount];

    KBUFF_NODE_MAP m_mapBuffNode;

    int GetUsedTalentCount();
    int GetLadderWinRate();

    BOOL        m_bEnterScene;
    int         m_nOldExpMultiple;
    void        OnEnterScene();
    void        OnLeaveScene();

};

class KPlayer;
class KHero;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

class KHeroDataList
{
public:
    KHeroDataList();
    ~KHeroDataList();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    size_t      GetHeroCount();
    size_t      GetActiveHeroCount();
    KHeroData*  GetHeroData(DWORD dwTemplateID);
    BOOL        CanAddHeroData(DWORD dwTemplateID);
    BOOL        AddHeroData(DWORD dwTemplateID, int nLevel);
    BOOL        RemoveHeroData(DWORD dwTemplateID);
    BOOL        HasHero(DWORD dwTemplateID) const;
    int 		GetMaxHeroCount(){return m_nMaxHeroCount;}
    BOOL 		AddMaxHeroCount(int nAddCount);
    BOOL        SaveHeroData(KHero* pHero);
    BOOL        AddPlayerBuff(DWORD dwBuffID, int nFrames);
    BOOL        AddBuffTime(DWORD dwHeroTemplateID, DWORD dwBuffID, int nFrames);
    void        DecreaseAllCompetitorHeroExp(int nPercent);
    KHeroData*  FindFreeHero();
    BOOL        Fire(const DWORD dwTemplateID);
    BOOL        UnFire(const DWORD dwTemplateID);
    int         GetHighestLadderLevel();
    BOOL        SelectHeroSkill(DWORD dwHeroTemplateID, int nSkillGroup, int nSlotIndex, DWORD dwSkillID);
    BOOL 		IsHeroLearnedActiveSkill(DWORD dwHeroTemplateID, DWORD dwSkillID, DWORD dwStep);
    BOOL        IsHeroLearnedPassiveSkill(DWORD dwHeroTemplateID, DWORD dwSkillID);
    BOOL        IsHeroLearnedNirvanaSkill(DWORD dwHeroTemplateID, DWORD dwSkillID);
    BOOL        IsHeroLearnedSlamBallSkill(DWORD dwHeroTemplateID, DWORD dwSkillID);
    KHeroData*  GetHeroDataOfHighestLadder();
    BOOL        GreaterThanByLadder(KHeroData* pLeft, KHeroData* pRight);
    void        GetHeroList(std::vector<DWORD>& vecHeroList);
private:
    BOOL         GetActiveSkillBelongToSlot(DWORD dwHeroTemplateID, DWORD dwSkillID, int& nSlotIndex);
    BOOL         GetPassiveSkillBelongToSlot(DWORD dwHeroTemplateID, DWORD dwSkillID, int& nSlotIndex);
    BOOL         GetNirvanaSkillBelongToSlot(DWORD dwHeroTemplateID, DWORD dwSkillID, int& nSlotIndex);
    BOOL         GetSlamBallSkillBelongToSlot(DWORD dwHeroTemplateID, DWORD dwSkillID, int& nSlotIndex);

private:
    typedef std::map<DWORD, KHeroData, std::less<DWORD>, KMemory::KAllocator<std::pair<DWORD, KHeroData> > > KMAP_HERO_DATA;
    KMAP_HERO_DATA m_mapHeroData;

    int      m_nMaxHeroCount;
    int      m_nFiredHeroCount;
    KPlayer* m_pPlayer;
};


