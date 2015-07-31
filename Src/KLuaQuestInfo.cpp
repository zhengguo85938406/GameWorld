#include "stdafx.h"
#include "KQuestInfoList.h"
#include "KSO3World.h"

DEFINE_LUA_CLASS_BEGIN(KQuestInfo)

    REGISTER_LUA_STRING_READONLY(KQuestInfo, QuestName)     // 任务名称
	REGISTER_LUA_BOOL(KQuestInfo, Repeat)					//是否可重复
	REGISTER_LUA_BOOL(KQuestInfo, Accept)					//是否先接才能交

	REGISTER_LUA_DWORD(KQuestInfo, CoolDownID)			//CoolDown计时器ID
	REGISTER_LUA_DWORD(KQuestInfo, PrequestID)			//前置任务ID
	REGISTER_LUA_INTEGER(KQuestInfo, StartTime)			//开始时间
	REGISTER_LUA_INTEGER(KQuestInfo, EndTime)			//结束时间
	REGISTER_LUA_DWORD(KQuestInfo, SubsequenceID)		//直接后继任务ID   

	REGISTER_LUA_INTEGER(KQuestInfo, QuestValue1)			//任务变量
	REGISTER_LUA_INTEGER(KQuestInfo, QuestValue2)			//任务变量
	REGISTER_LUA_INTEGER(KQuestInfo, QuestValue3)			//任务变量
	REGISTER_LUA_INTEGER(KQuestInfo, QuestValue4)			//任务变量

    REGISTER_LUA_INTEGER(KQuestInfo, QuestEvent1)			//任务事件
    REGISTER_LUA_INTEGER(KQuestInfo, QuestEvent2)			//任务事件
    REGISTER_LUA_INTEGER(KQuestInfo, QuestEvent3)			//任务事件
    REGISTER_LUA_INTEGER(KQuestInfo, QuestEvent4)			//任务事件

	REGISTER_LUA_INTEGER(KQuestInfo, PresentExp)			//交任务时奖励的经验
	REGISTER_LUA_INTEGER(KQuestInfo, PresentMoney)			//交任务时奖励的金钱数量

DEFINE_LUA_CLASS_END(KQuestInfo)
