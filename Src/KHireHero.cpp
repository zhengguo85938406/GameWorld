// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KHireHero.cpp
//  Creator		: yangkaibo
//	Date		: 2/4/2013
//  Comment		: 球员招募解雇
//	*********************************************************************
#include "stdafx.h"
#include "KHireHero.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"

KHireHero::KHireHero(void)
{
	m_pPlayer = NULL;
}

KHireHero::~KHireHero(void)
{
}

BOOL KHireHero::Init(KPlayer* pPlayer)
{
	BOOL bResult  = false;

	assert(pPlayer);
	assert(!m_pPlayer);

	m_pPlayer = pPlayer;	

	bResult = true;
Exit0:
	return bResult;
}

void KHireHero::UnInit()
{
	m_pPlayer = NULL;
}

BOOL KHireHero::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	DWORD   dwHeroTemplateID = 0;
	time_t  unlockTime = 0;

	assert(pLoadBuf);
	
	const T3DB::KPB_HERO_UNLOCK_DATA_LIST& heroUnlockList = pLoadBuf->herounlockdatalist();

	for (int i = 0; i < heroUnlockList.herounlockdata_size(); ++i)
	{
		const T3DB::KPB_HERO_UNLOCK_DATA& heroData = heroUnlockList.herounlockdata(i);
		dwHeroTemplateID = heroData.herotemplateid();
		unlockTime       = heroData.unlocktime();
		m_mapUnlockTime.insert(KMAP_HERO_UNLOCK_TIME::value_type(dwHeroTemplateID, unlockTime));
	}

	bResult = true;
Exit0:
	return bResult;
}

BOOL KHireHero::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
	BOOL bResult  = false;
 	BOOL bRetCode = false;
 	T3DB::KPB_HERO_UNLOCK_DATA_LIST* pData = NULL;
	T3DB::KPB_HERO_UNLOCK_DATA* pHeroUnlock = NULL;
	KMAP_HERO_UNLOCK_TIME::iterator it;
 
 	KGLOG_PROCESS_ERROR(pSaveBuf);
 
 	pData = pSaveBuf->mutable_herounlockdatalist();
	KGLOG_PROCESS_ERROR(pData);

	for (it = m_mapUnlockTime.begin(); it != m_mapUnlockTime.end(); ++it)
	{
		pHeroUnlock = pData->add_herounlockdata();
		pHeroUnlock->set_herotemplateid(it->first);
		pHeroUnlock->set_unlocktime(it->second);
	}

	bResult = true;
Exit0:
	return bResult;
}

BOOL KHireHero::BuildEventMap()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

	KMAP_HERO_UNLOCK_EVENT::iterator itEvent;
	DWORD event = peInvalid;

	const KMAP_HERO_TEMPLATE& HeroTemplateMap = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplateMap();

	for (KMAP_HERO_TEMPLATE::const_iterator it = HeroTemplateMap.begin(); it != HeroTemplateMap.end(); ++it)
	{
		const KHeroTemplateInfo& HeroTemplate = it->second;

		event = HeroTemplate.eUnlockType; //如增加解锁类型，此处需增加相应解锁类型到事件的转化
		if (event == UNLOCK_TYPE_QUEST)
		{
			event = peFinishQuest;
		}
		else if (event == UNLOCK_TYPE_ACHIEVEMENT)
		{
			event = peFinishAchievement;
		}
		else
		{
			event = peInvalid;
		}
			
		m_mapUnlockEvent[event].push_back(HeroTemplate.dwTemplateID);
	}

	bResult = true;
Exit0:
	return bResult;
}

BOOL KHireHero::DoSyncUnlockData()
{	
	BOOL bResult  = false;	
	
	for (KMAP_HERO_UNLOCK_TIME::iterator it = m_mapUnlockTime.begin(); it != m_mapUnlockTime.end(); ++it)
	{
		g_PlayerServer.DoSyncOneHeroUnlocktime(m_pPlayer, it->first, it->second);
	}
	
	bResult = true;
Exit0:
	return bResult;
}

BOOL KHireHero::ProcessAfterDataLoaded()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

	bRetCode = BuildEventMap();
	KGLOG_PROCESS_ERROR(bRetCode);

	if (m_pPlayer->m_eConnectType == eLogin)
	{
		bRetCode = DoSyncUnlockData();
		KGLOG_PROCESS_ERROR(bRetCode);
	}

	bResult = true;
Exit0:
	return bResult;
}

void KHireHero::OnEvent(int nEvent, int (&Params)[cdMaxEventParamCount])
{
	BOOL bRetCode = false;
	KMAP_HERO_UNLOCK_EVENT::iterator it;

	KGLOG_PROCESS_ERROR(nEvent > peInvalid && nEvent < peTotal);

	it = m_mapUnlockEvent.find(nEvent);
	if (it != m_mapUnlockEvent.end())
	{
		bRetCode = DoUpdate(it->second, nEvent, Params);
		KGLOG_PROCESS_ERROR(bRetCode);
	}

Exit0:
	return;
}

BOOL KHireHero::DoUpdate(KVECT_HERO_TEMPLATEID& heroVect, int nEvent, int (&Params)[cdMaxEventParamCount])
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
	KHeroTemplateInfo* pHeroTemplate = NULL;
	KHeroData* pHero = NULL;
	DWORD dwTemplateID = 0;
	KVECT_HERO_TEMPLATEID::iterator it;

	for (it = heroVect.begin(); it != heroVect.end(); ++it)
	{
		dwTemplateID = *it;
		pHeroTemplate = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
		KGLOG_PROCESS_ERROR(pHeroTemplate);

		// 打折仅限点券
		if (pHeroTemplate->nBuyMoneyType != emotCoin)
			continue;

		switch (pHeroTemplate->eUnlockType)
		{
		case UNLOCK_TYPE_CANNOT:
			KG_PROCESS_ERROR(false);
			break;
		case UNLOCK_TYPE_NONE:
			break;
		case UNLOCK_TYPE_QUEST:
			if (pHeroTemplate->dwLockData == (DWORD)Params[0])
			{
				m_mapUnlockTime.insert(KMAP_HERO_UNLOCK_TIME::value_type(dwTemplateID, g_pSO3World->m_nCurrentTime));
				g_PlayerServer.DoSyncOneHeroUnlocktime(m_pPlayer, dwTemplateID, g_pSO3World->m_nCurrentTime);
			}
			break;
		case UNLOCK_TYPE_ACHIEVEMENT:
			if (pHeroTemplate->dwLockData == (DWORD)Params[1])
			{
				m_mapUnlockTime.insert(KMAP_HERO_UNLOCK_TIME::value_type(dwTemplateID, g_pSO3World->m_nCurrentTime));
				g_PlayerServer.DoSyncOneHeroUnlocktime(m_pPlayer, dwTemplateID, g_pSO3World->m_nCurrentTime);
			}
			break;
		default:
			break;
		}
	}

	bResult = true;
Exit0:
	return bResult;
}

int KHireHero::BuyHeroPrice(DWORD dwTemplateID)
{
	int nResult 	= -1;
	KHeroTemplateInfo* pHeroTemplate = NULL;
	KMAP_HERO_UNLOCK_TIME::iterator it;
	float fDiscount = 1.0f;
	time_t tTime = 0;
	time_t tDiscountTime = 0;
	time_t SecPerHour = 3600;
	
	pHeroTemplate = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
	KGLOG_PROCESS_ERROR(pHeroTemplate);

	it = m_mapUnlockTime.find(dwTemplateID);
	if (it != m_mapUnlockTime.end())
	{
		tTime = it->second;
		tDiscountTime = pHeroTemplate->nDisCountTime * SecPerHour;

		if (g_pSO3World->m_nCurrentTime < tTime + tDiscountTime)
			fDiscount = 0.8f; // 八折
	}
	
	nResult = int(pHeroTemplate->nBuyPrice * fDiscount);
Exit0:
	return nResult;
}

BOOL KHireHero::RemoveHeroUnlockTime(DWORD dwTemplateID)
{
	KMAP_HERO_UNLOCK_TIME::iterator it;	
	it = m_mapUnlockTime.find(dwTemplateID);
	if (it != m_mapUnlockTime.end())
	{
		m_mapUnlockTime.erase(it);
		g_PlayerServer.DoSyncOneHeroUnlocktime(m_pPlayer, dwTemplateID, 0); // 通知客户端打折时间没有了	
		return true;
	}	
	return false;
}
