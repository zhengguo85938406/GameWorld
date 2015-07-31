// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KActivityCenter.h
//  Creator 	: hanruofei 
//  Date		: 2012-7-2
//  Comment	: 
// ***************************************************************

#pragma once
#include "Engine/KMemory.h"
#include <map>
#include "KReadTab.h"

struct KACTIVITY_ITEM
{
    DWORD   dwType;
    DWORD   dwIndex;
    int     nCount;
    int     nValuePoint;
};

struct KActivity
{
    DWORD dwActivityID;
    int   nBeginTime;
    int   nEndTime;
    DWORD dwQuestID;
    KACTIVITY_ITEM Award[MAX_ACTIVITY_AWARD];
};

struct KTesterPoint : public KLineData
{
    char szAccount[32];
    int  nPoint;

    KTesterPoint()
    {
        REGISTER_COL_INFO(szAccount, eStringCol);
        REGISTER_COL_INFO(nPoint, eIntCol);
    }
};

struct KTesterAwardConfig : public KLineData
{
    int nPoint;
    int nLevel;
    int nAwardItemTabType;
    int nAwardItemTabIndex;
    int nAwardItemCount;
    char szTitle[64];
    char szText[256];

    KTesterAwardConfig()
    {
        REGISTER_COL_INFO(nPoint, eIntCol);
        REGISTER_COL_INFO(nLevel, eIntCol);
        REGISTER_COL_INFO(nAwardItemTabType, eIntCol);
        REGISTER_COL_INFO(nAwardItemTabIndex, eIntCol);
        REGISTER_COL_INFO(nAwardItemCount, eIntCol);
        REGISTER_COL_INFO(szTitle, eStringCol);
        REGISTER_COL_INFO(szText, eStringCol);
    }
};

struct KLevelupAwardConfig : public KLineData
{
    int nLevel;
    int nAwardID;

    KLevelupAwardConfig()
    {
        REGISTER_COL_INFO(nLevel, eIntCol);
        REGISTER_COL_INFO(nAwardID, eIntCol);
    }
};


class KPlayer;

class KActivityCenter
{
public:
    BOOL Init();
    void UnInit();
    BOOL OnActivityFinished(KPlayer* pPlayer, DWORD dwActivityID, int nFinishType);
    BOOL OnCheckAndUpdateActivityStateRespond(KPlayer* pPlayer, DWORD dwActivityID, int nFinishType);
    BOOL EnsureQuestFinished(KPlayer* pPlayer, DWORD dwActivityID);
    BOOL GetTesterAwardLevel(KPlayer* pPlayer);
    BOOL GetTesterAward(KPlayer* pPlayer);
    BOOL GetLevelupAward(KPlayer* pPlayer, int nLevel);
    BOOL ViewOnlineAward(KPlayer* pPlayer);
    BOOL GetOnlineAward(KPlayer* pPlayer);

private:
    BOOL        LoadData();
    KACTIVITY_ITEM*  GetAwardItem(DWORD dwActivityID, int nFinishType);

private:
    typedef std::map<DWORD, KActivity, std::less<DWORD>, KMemory::KAllocator<std::pair<DWORD, KActivity> > > KMAP_ACTIVITY;
    KMAP_ACTIVITY m_ActivityData;

    KReadStringTab<KTesterPoint> m_TesterPoint;
    KReadTab<KTesterAwardConfig> m_TesterAwardConfig;
    KReadTab<KLevelupAwardConfig> m_LevelupAwardConfig;
};
