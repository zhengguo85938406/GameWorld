// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KQuestList.h 
//  Creator 	: Xiayong  
//  Date		: 03/20/2012
//  Comment	: 
// ***************************************************************
#pragma once

#include <list>
#include "SO3Result.h"
#include "KQuestInfoList.h"
#include "Engine/KMemory.h"
#include "game_define.h"
#include <vector>
#include <map>

class KPlayer;
class KHero;
namespace T3DB
{
    class KPB_SAVE_DATA;
    class KPBQuestList;
}

struct KQuest
{
	KQuestInfo* pQuestInfo;
	int			nQuestValue[cdMaxQuestEventCount];	// 任务变量
};

class KQuestList;


struct KQUESTVALUE_INCREASOR
{
    int nQuestIndex;
    int nValueIndex;
};

typedef KMemory::KAllocator<KQUESTVALUE_INCREASOR> KQUESTVALUE_INCREASOR_ALLOCTOR;
typedef std::vector<KQUESTVALUE_INCREASOR, KQUESTVALUE_INCREASOR_ALLOCTOR> KVEC_QUESTVALUE_INCREASOR;

typedef KMemory::KAllocator<std::pair<KPLAYER_EVENT, KVEC_QUESTVALUE_INCREASOR> > KMAP_QUESTVALUE_INCREASOR_ALLOCTOR;
typedef std::map<int, KVEC_QUESTVALUE_INCREASOR, std::less<int>, KMAP_QUESTVALUE_INCREASOR_ALLOCTOR> KMAP_QUESTVALUE_INCREASOR;


class KQuestList
{
public:
	BOOL                Init(KPlayer* pPlayer);
	void                UnInit();
    
    BOOL                LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL                LoadStateFromProtoBuf(const std::string*   pStrState);
    BOOL                LoadListFromProtoBuf(const T3DB::KPBQuestList*   pQuestList);
    BOOL                LoadDailyFromProtoBuf(const T3DB::KPBQuestList*   pQuestList);

    BOOL                SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);
    BOOL                SaveStateToProtoBuf(T3DB::KPBQuestList*   pQuestList);
    BOOL                SaveListToProtoBuf(T3DB::KPBQuestList*   pQuestList);
    BOOL                SaveDailyToProtoBuf(T3DB::KPBQuestList*   pQuestList);

    int                 GetDailyQuest(BYTE* pbyBuffer, size_t uBufferSize);

    QUEST_RESULT_CODE   CanAccept(KQuestInfo* pQuestInfo);
  
	QUEST_RESULT_CODE   Accept(DWORD dwQuestID, BOOL bIgnoreRules = false);
    QUEST_RESULT_CODE   Finish(DWORD dwQuestID, int nPresentChoice1, int nPresentChoice2, BOOL bIgnoreRules = false);

	QUEST_RESULT_CODE   Cancel(int nQuestIndex);
    BOOL                ClearQuest(DWORD dwQuestID);
	QUEST_RESULT_CODE   CanFinish(KQuestInfo* pQuestInfo);
	int                 GetAcceptedCount(void);
	QUEST_STATE	        GetQuestState(DWORD dwQuestID);
    BOOL    	        SetQuestState(DWORD dwQuestID, QUEST_STATE eState);

	int                 GetQuestPhase(DWORD dwQuestID);
	DWORD               GetQuestID(int nQuestIndex);
	int                 GetQuestIndex(DWORD dwQuestID);
	int                 GetQuestValue(int nQuestIndex, int nValueIndex);
    BOOL                SetQuestValue(int nQuestIndex, int nValueIndex, int nNewValue);

	DWORD               GetKillNpcCount(int nQuestIndex, int nCountIndex);
	BOOL                SetKillNpcCount(int nQuestIndex, int nCountIndex, int nNewCount);
	BOOL                SetQuestFailedFlag(int nQuestIndex, BOOL bFlag);
	BOOL                GetQuestFailedFlag(int nQuestIndex);
	int                 GetLastTime(int nQuestIndex);
	BOOL				GetQuestRepeatCutPercent(KQuestInfo* pQuestInfo, int* pnRepeatCutPercent);
    
    BOOL                IsQuestEndItemEnoughByIndex(KQuestInfo* pQuestInfo, int nIndex);
    
    void                ClearQuestList();

    void 				OnEvent(int nEvent, int (&Params)[cdMaxEventParamCount]);

    const KQuest*       GetQuestByIndex(int nQuestIndex) const;

    QUEST_RESULT_CODE   ClientSetQuestValue(int nQuestIndex, int nValueIndex, int nNewValue);
    void                CheckAutoFinish(DWORD dwQuestID);
    BOOL                CanClientForceFinish(DWORD dwQuestID);

    BOOL                CanAcceptRandomQuest(DWORD dwQuestID);
    void                ClearRandomQuest();

private:
    BOOL                IsQuestEndItemEnough(KQuestInfo* pQuestInfo);

    QUEST_RESULT_CODE   CommonCheck(KQuestInfo* pQuestInfo);

	QUEST_RESULT_CODE   Present(KQuestInfo* pQuestInfo, int nPresentChoice1, int nPresentChoice2);
    BOOL                CheckFreeRoomForPresent(KQuestInfo* pQuestInfo, int nPresentChoice1, int nPresentChoice2);
    QUEST_RESULT_CODE   CanPresentItems(KQuestInfo* pQuestInfo, int nPresentChoice1, int nPresentChoice2);
    QUEST_RESULT_CODE   CanPresentItem(DWORD dwTabType, DWORD dwIndex, int nAmount);
    QUEST_RESULT_CODE   PresentItems(KQuestInfo* pQuestInfo, int nPresentChoice1, int nPresentChoice2);
    QUEST_RESULT_CODE   PresentItem(DWORD dwTabType, DWORD dwIndex, int nAmount, int nValuePoint);
    BOOL                CostQuestRequireItem(KQuestInfo* pQuestInfo, BOOL bCancelQuest);
    BOOL                CostQuestOfferItem(KQuestInfo* pQuestInfo);

    void                RegisterQuestValueProccessor(int nQuestIndex);
    void                UnRegisterQuestValueProccessor(int nQuestIndex);

    QUEST_RESULT_CODE   CanAcceptDailyQuest(KQuestInfo* pQuestInfo);
    BOOL                AcceptDailyQuest(KQuestInfo* pQuestInfo);
    void                ClearDailyQuest(KQuestInfo* pQuestInfo);
    void                FinishDailyQuest(KQuestInfo* pQuestInfo);
    void                FinishRandomQuest(DWORD dwQuestID);

private:
	BYTE	    m_byQuestStateList[MAX_QUEST_COUNT];
	KPlayer*    m_pPlayer;
	KQuest	    m_AcceptedQuestList[cdMaxAcceptQuestCount];
	int		    m_nAcceptedCount;

    KMAP_QUESTVALUE_INCREASOR m_mapQuestValueIncreasor;

    struct KDAILY_QUEST
    {
       DWORD  dwQuestID;
       time_t nNextAcceptTime;
    };
       
    KDAILY_QUEST m_DailyQuest[MAX_DAILY_QUEST_COUNT];
};
