/************************************************************************/
/* 任务配置表															*/
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2005.11.01	Create												*/
/************************************************************************/
#ifndef _KQUEST_INFO_LIST_H_
#define _KQUEST_INFO_LIST_H_

#include <map>
#include <vector>
#include "SO3ProtocolBasic.h"
#include "Luna.h"
#include "Engine/KMemory.h"
#include "game_define.h"
#include "KCondition.h"

#define RANDOM_QUEST_COUNT_PER_GROUP    16

struct KQuestEvent : KEventCondition
{
    int nQuestValue;
};

struct KQuestInfo
{
    DWORD			dwQuestID;
    char            szQuestName[_NAME_LEN];
    int             nQuestType;

    BOOL            bRepeat;									// 是否可重复
    BOOL            bAccept;									// 是否需要接了才能交

    BYTE            byLevel;                                    // 任务等级
    BYTE            byStartItemType;							// 接任务Item类型
    WORD			wStartItemIndex;							// 接任务Item类型			

    BYTE            byMinLevel;									// 接任务最小等级需求
    BYTE            byMaxLevel;									// 接任务最大等级需求
    BYTE            byRequireGender;								// 接任务的性别需求
	BYTE			byRequireSingle;							//完成任务需要是单人

    DWORD           dwOfferItemType[QUEST_OFFER_ITEM_COUNT];			// 接任务时提供道具的类型
    int             nOfferItemIndex[QUEST_OFFER_ITEM_COUNT];		    // 接任务时提供道具的类型
    int             nOfferItemAmount[QUEST_OFFER_ITEM_COUNT];		// 接任务时提供道具的数量

    BYTE            byRepeatCutPercent;							// 重复做任务时奖励的削减百分比

    BYTE            byEndRequireItemType[QUEST_END_ITEM_COUNT];	//交任务时所需道具的类型
    WORD			wEndRequireItemIndex[QUEST_END_ITEM_COUNT];	//交任务时所需道具的类型
    WORD            wEndRequireItemAmount[QUEST_END_ITEM_COUNT];	//交任务时所需道具的数量
    bool			bIsDeleteEndRequireItem[QUEST_END_ITEM_COUNT];  // 交任务或者删除任务的时候是否删除这个道具

    BOOL			bPresentAll[2];								// 第一组(1-4)道具是否全给或者只给一件
    BYTE			byPresentItemType[cdQuestParamCount];	// 交任务时奖励道具的类型
    WORD			wPresentItemIndex[cdQuestParamCount];	// 交任务时奖励道具的类型
    WORD            wPresentItemAmount[cdQuestParamCount];	// 交任务时奖励道具的数量
    WORD            wPresentItemValuePoint[cdQuestParamCount];	// 交任务时奖励道具的数量

    DWORD           dwAwardHeroTemplateID[QUEST_AWARD_HERO_COUNT];
    int             nAwardHeroLevel[QUEST_AWARD_HERO_COUNT];
	    
	DWORD			dwCoolDownID;								// CoolDown计时器ID
	
	DWORD			dwPrequestID;			                    // 前置任务ID
	DWORD			dwSubsequenceID;							// 直接后继任务ID
	int				nStartTime;									// 开始时间
	int				nEndTime;									// 结束时间
        
	int				nCostMoney;									//交任务需求的金钱

    BOOL            bEventOrder;                                // 任务变量是否按顺序完成

    KQuestEvent     AllEvent[cdMaxQuestEventCount];             // 任务事件

    int				nPresentExp;								// 交任务时奖励的经验
    int				nPresentMoney;								// 交任务时奖励的金钱数量

    DWORD           dwScriptID;
    BOOL            bAutoFinish;                                // 是否自动完成
    BOOL            bClientCanFinish;                           // 客户端能否强制完成任务
    BOOL            bDaily;                                     // 是否日常任务

    int             nRequireMissionType;
    int             nRequireMissionStep;    
    int             nRequireMissionLevel;
    int             nRequireMapType;

    KQuestInfo()
    {
        dwQuestID       = 0;
        szQuestName[0]  = '\0';
        nQuestType      = 0;

        bRepeat         = false;
        bAccept         = false;

        byLevel         = 0;
        byStartItemType = 0;
        wStartItemIndex = 0;

        byMinLevel      = 0;
        byMaxLevel      = 0;
        byRequireGender = 0;

        memset(dwOfferItemType,         0,  sizeof(dwOfferItemType));
        memset(nOfferItemIndex,         0,  sizeof(nOfferItemIndex));
        memset(nOfferItemAmount,        0,  sizeof(nOfferItemAmount));

        byRepeatCutPercent  = 0;

        memset(byEndRequireItemType,    0,  sizeof(byEndRequireItemType));
        memset(wEndRequireItemIndex,    0,  sizeof(wEndRequireItemIndex));
        memset(wEndRequireItemAmount,   0,  sizeof(wEndRequireItemAmount));
        memset(bIsDeleteEndRequireItem, 0,  sizeof(bIsDeleteEndRequireItem));

        memset(bPresentAll,             0,  sizeof(bPresentAll));
        memset(byPresentItemType,       0,  sizeof(byPresentItemType));
        memset(wPresentItemIndex,       0,  sizeof(wPresentItemIndex));
        memset(wPresentItemAmount,      0,  sizeof(wPresentItemAmount));
        memset(wPresentItemValuePoint,  0,  sizeof(wPresentItemValuePoint));

        memset(dwAwardHeroTemplateID,   0,  sizeof(dwAwardHeroTemplateID));
        memset(nAwardHeroLevel,         0,  sizeof(nAwardHeroLevel));
        dwCoolDownID            = 0;

        dwPrequestID            = 0;
        dwSubsequenceID         = 0;
        nStartTime              = 0;
        nEndTime                = 0;

        nCostMoney              = 0;

        bEventOrder             = 0;

        nPresentExp             = 0;
        nPresentMoney           = 0;

        dwScriptID              = 0;
        bAutoFinish             = 0;
        bClientCanFinish        = 0;
        bDaily                  = 0;
        nRequireMissionType     = 0;
        nRequireMissionStep     = 0;
        nRequireMissionLevel    = 0;
        nRequireMapType         = 0;
    };

    DECLARE_LUA_CLASS(KQuestInfo);

    DECLARE_LUA_STRUCT_STRING(QuestName, _NAME_LEN, szQuestName);          // 任务名称
    DECLARE_LUA_STRUCT_BOOL(Repeat, bRepeat);					// 是否可重复
    DECLARE_LUA_STRUCT_BOOL(Accept, bAccept);					// 是否先接才能交

    DECLARE_LUA_STRUCT_DWORD(CoolDownID, dwCoolDownID);		//CoolDown计时器ID
    DECLARE_LUA_STRUCT_DWORD(PrequestID, dwPrequestID);			//前置任务ID
    DECLARE_LUA_STRUCT_INTEGER(StartTime, nStartTime);			//开始时间
    DECLARE_LUA_STRUCT_INTEGER(EndTime, nEndTime);				//结束时间
    DECLARE_LUA_STRUCT_DWORD(SubsequenceID, dwSubsequenceID);	//直接后继任务ID   
    DECLARE_LUA_STRUCT_INTEGER(CostMoney, nCostMoney);							//消耗的金钱

    DECLARE_LUA_STRUCT_INTEGER(QuestValue1, AllEvent[0].nQuestValue); //任务变量
    DECLARE_LUA_STRUCT_INTEGER(QuestValue2, AllEvent[1].nQuestValue); //任务变量
    DECLARE_LUA_STRUCT_INTEGER(QuestValue3, AllEvent[2].nQuestValue); //任务变量
    DECLARE_LUA_STRUCT_INTEGER(QuestValue4, AllEvent[3].nQuestValue); //任务变量

    DECLARE_LUA_STRUCT_INTEGER(QuestEvent1, AllEvent[0].nEvent); //任务事件
    DECLARE_LUA_STRUCT_INTEGER(QuestEvent2, AllEvent[1].nEvent); //任务事件
    DECLARE_LUA_STRUCT_INTEGER(QuestEvent3, AllEvent[2].nEvent); //任务事件
    DECLARE_LUA_STRUCT_INTEGER(QuestEvent4, AllEvent[3].nEvent); //任务事件

    DECLARE_LUA_STRUCT_INTEGER(PresentExp, nPresentExp);					//交任务时奖励的经验
    DECLARE_LUA_STRUCT_INTEGER(PresentMoney, nPresentMoney);				//交任务时奖励的金钱数量
};

class KQuestInfoList
{
public:
    BOOL Init();
    void UnInit();
    BOOL Reload();

    KQuestInfo* GetQuestInfo(DWORD dwQuestID);
    void        GetDailyQuest(std::vector<DWORD>& vecQuest);
    DWORD       GetRandomQuest(int nHeroLevel, int nRandomQuestIndex);
private:
    typedef std::map<DWORD, KQuestInfo> MAP_ID_2_QUEST_INFO;
    MAP_ID_2_QUEST_INFO					m_mapID2QuestInfo;

    KQuestInfo	m_DefaultQuestInfo;		//默认设定

    BOOL LoadQuestInfo(ITabFile* piTabFile, int nIndex, KQuestInfo* pQuestInfo);

    typedef std::vector<DWORD> KQUEST_GROUP;
    typedef std::map<int, KQUEST_GROUP> KMAP_LEVEL2QUESTGROUP;

    KMAP_LEVEL2QUESTGROUP   m_RandomQuestGroup;
    std::vector<DWORD>      m_DailyQuest;

    BOOL LoadRandomQuestGroup();
};

#endif	//_KQUEST_INFO_LIST_H_
