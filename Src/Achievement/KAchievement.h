// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KAchievement.h
//  Creator		: hanruofei
//	Date		: 11/19/2012
//  Comment		: 
//	*********************************************************************

#pragma once
#include "Engine/KMemory.h"
#include "game_define.h"
#include <vector>
#include <map>

class KPlayer;
struct KAchievementSettingsItem;

typedef std::vector<DWORD, KMemory::KAllocator<DWORD> > KVEC_ACHIEVEMENT_UPDATOR;
typedef std::map<int, KVEC_ACHIEVEMENT_UPDATOR, std::less<int>, KMemory::KAllocator<std::pair<int, KVEC_ACHIEVEMENT_UPDATOR> > > KMAP_ACHIEVEMENT_UPDATOR;

typedef std::map<DWORD, int, std::less<DWORD>, KMemory::KAllocator<std::pair<DWORD, int> > > KMAP_PROGRESSING_ACHIEVEMENT;

class KAchievement
{
public:
    KAchievement(void);
    ~KAchievement(void);

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    void OnEvent(int nEvent, int (&Params)[cdMaxEventParamCount]);
    
    void Refresh(time_t nLastDailyRefreshTime);
    BOOL ClientIncreaseValue(DWORD dwAchievementID);
    BOOL FinishAchievement(DWORD dwAchievementID);
    BOOL UpdateAchievementValue(DWORD dwAchievementID, int nEvent, int (&Params)[cdMaxEventParamCount]);
    BOOL IsAchievementFinished(DWORD dwAchievementID);
    BOOL SetAchievementValue(DWORD dwAchievementID, int nNewValue);
    int GetAchievenmentValue(DWORD dwAchievementID);
    BOOL EnsureEventMapBuild(DWORD dwAchievementID);
    BOOL ResetOne(DWORD dwAchievementID);

    BOOL ProcessAfterDataLoaded();

private:
    BOOL DoDailyReset();
    BOOL DoWeeklyReset();
    BOOL BuildEventMap();
    BOOL BuildEventMapOf(KAchievementSettingsItem* pAchievementInfo);
    BOOL DoSyncAchievementData();
    BOOL DoUpdate(KVEC_ACHIEVEMENT_UPDATOR vecUpdator, int nEvent, int (&Params)[cdMaxEventParamCount]);
    BOOL ResetState(DWORD dwAchievementID);
    BOOL ResetProgressing(DWORD dwAchievementID);
    BOOL EventCallbackInScript(int nEvent, int (&Params)[cdMaxEventParamCount]);
    BOOL IsSpecifiedEvent(DWORD dwAchievementID, int nEvent, int (&Params)[cdMaxEventParamCount]);
    BOOL IsAchievementValueFinished(DWORD dwAchievementID);
 
private:
    uint32_t m_Achievement[cdMaxAchievenmentCount / UINT32_BIT_COUNT + 1];
    KPlayer* m_pPlayer;
    int m_nAchievementPoint;
    KMAP_PROGRESSING_ACHIEVEMENT m_mapProgressingAchievement;
    KMAP_ACHIEVEMENT_UPDATOR m_mapValueUpdator;
};

