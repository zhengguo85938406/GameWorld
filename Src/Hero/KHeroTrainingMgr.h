// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KHeroTrainingMgr.h
//  Creator 	: hanruofei 
//  Date		: 2012-4-10
//  Comment	: 
// ***************************************************************
#pragma once

#include <vector>

struct KHERO_TRAINING_ITEM
{
    DWORD   dwHeroTemplateID;
    int     nEndTime;
    DWORD   dwTrainingID;
};

typedef std::vector<KHERO_TRAINING_ITEM> KVEC_TRAINING_ITEMS;

class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

class KHeroTrainingMgr
{
public:
    KHeroTrainingMgr();
    ~KHeroTrainingMgr();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    void Activate();

    BOOL UpgradeTeacherLevel();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL BeginTrainingHero(uint32_t dwHeroTemplateID,  uint32_t dwTrainingID);
    BOOL EndTrainingHero(uint32_t dwHeroTemplateID, int nCostMoney);

    const KVEC_TRAINING_ITEMS* GetAllTrainingItems() const;
    int      GetTeacherLevel() const {return m_nTeacherLevel;};
    
    BOOL IsTraining(uint32_t dwHeroTemplateID) const;
    KUPGRADE_TYPE GetHeroUpgradeType(uint32_t dwHeroTemplateID);
    BOOL GetSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize);
    void OnUpgradeQueueFinished(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID, int nCostMoney);

private:
    BOOL GetTrainingItem(uint32_t dwTemplateID, KHERO_TRAINING_ITEM& rTrainingItem) const;
    BOOL RemoveTrainingItem(uint32_t dwTemplateID, KHERO_TRAINING_ITEM& rRemoved);
    BOOL AddTrainingItem(const KHERO_TRAINING_ITEM& rNewItem);

private:
    int                 m_nTeacherLevel;
    KPlayer*            m_pPlayer;
    KVEC_TRAINING_ITEMS m_vecTrainingItems;
};

