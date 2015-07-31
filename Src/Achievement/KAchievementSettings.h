// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KAchievementSettings.h
//  Creator		: hanruofei
//	Date		: 11/19/2012
//  Comment		: 
//	*********************************************************************

#pragma once
#include "KCondition.h"
#include "game_define.h"

typedef KMaterial KAwardItem;

struct KAchievementSettingsItem
{
    DWORD dwID;
    BOOL bExpired;
    int nAchievementPoint;
    BOOL bAutoFinish;
    int nAchievementType;
    DWORD dwAwardItemType;
    int dwAwardItemIndex;
    int nAwardItemCount;
    int nAwardItemValuePoint;
    KEventCondition cEventCondition;
    int nValueUpdateType;
    int nValueUpdateParam;
    int nTargetValue;
    BOOL bMessageToFriend;
    BOOL bMessageToServer;
    BOOL bMessageToClub;
};

typedef std::vector<KAchievementSettingsItem> KVEC_ACHIEVEMENTS;
typedef std::vector<KAchievementSettingsItem*> KVEC_DAILY_ACHIEVEMENTS;
typedef KVEC_DAILY_ACHIEVEMENTS KVEC_WEEKLY_ACHIEVEMENTS;

class KAchievementSettings
{
public:
    KAchievementSettings(void);
    ~KAchievementSettings(void);

    BOOL Init();
    void UnInit();

    KAchievementSettingsItem* GetItem(DWORD dwID);
    KVEC_DAILY_ACHIEVEMENTS* GetDailyAchievement();
    KVEC_WEEKLY_ACHIEVEMENTS* GetWeeklyAchievement();
    int GetAchievementCount();
    DWORD GetMaxAchievementID();
    BOOL IsValidAchievement(DWORD dwID);

private:
    BOOL LoadData();
    BOOL LoadUpdator(char szValue[], KAchievementSettingsItem* pItem);

private:
    KVEC_ACHIEVEMENTS m_vecAchievement;
    KVEC_DAILY_ACHIEVEMENTS m_vecDailyAchievement;
    KVEC_WEEKLY_ACHIEVEMENTS m_vecWeekyAchievement;
};

