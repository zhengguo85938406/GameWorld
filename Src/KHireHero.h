// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KHireHero.h
//  Creator		: yangkaibo
//	Date		: 2/4/2013
//  Comment		: 球员招募解雇
//	*********************************************************************
#include <map>
#include "game_define.h"
#include "Ksdef/TypeDef.h"
#pragma once

class KPlayer;

// <英雄模版ID, 解锁时间>
typedef std::map<DWORD, time_t> KMAP_HERO_UNLOCK_TIME;

typedef std::vector<DWORD> KVECT_HERO_TEMPLATEID;

// <事件, 英雄模版ID列表>表示列表中的英雄模版关心该事件 现在有peFinishAchievement, peFinishQuest
typedef std::map<DWORD, KVECT_HERO_TEMPLATEID> KMAP_HERO_UNLOCK_EVENT;

class KHireHero
{
public:
	KHireHero(void);
	~KHireHero(void);

	BOOL Init(KPlayer* pPlayer);
	void UnInit();

	BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
	BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);
	BOOL ProcessAfterDataLoaded();
	BOOL BuildEventMap();
	BOOL DoSyncUnlockData();

	BOOL DoUpdate(KVECT_HERO_TEMPLATEID& heroVect, int nEvent, int (&Params)[cdMaxEventParamCount]);

	void OnEvent(int nEvent, int (&Params)[cdMaxEventParamCount]);

	int  BuyHeroPrice(DWORD dwTemplateID); // 折后hero价格
	BOOL RemoveHeroUnlockTime(DWORD dwTemplateID);

private:
	KPlayer* m_pPlayer;
	KMAP_HERO_UNLOCK_TIME	m_mapUnlockTime;
	KMAP_HERO_UNLOCK_EVENT	m_mapUnlockEvent;
};

