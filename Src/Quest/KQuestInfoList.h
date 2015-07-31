/************************************************************************/
/* �������ñ�															*/
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

    BOOL            bRepeat;									// �Ƿ���ظ�
    BOOL            bAccept;									// �Ƿ���Ҫ���˲��ܽ�

    BYTE            byLevel;                                    // ����ȼ�
    BYTE            byStartItemType;							// ������Item����
    WORD			wStartItemIndex;							// ������Item����			

    BYTE            byMinLevel;									// ��������С�ȼ�����
    BYTE            byMaxLevel;									// ���������ȼ�����
    BYTE            byRequireGender;								// ��������Ա�����
	BYTE			byRequireSingle;							//���������Ҫ�ǵ���

    DWORD           dwOfferItemType[QUEST_OFFER_ITEM_COUNT];			// ������ʱ�ṩ���ߵ�����
    int             nOfferItemIndex[QUEST_OFFER_ITEM_COUNT];		    // ������ʱ�ṩ���ߵ�����
    int             nOfferItemAmount[QUEST_OFFER_ITEM_COUNT];		// ������ʱ�ṩ���ߵ�����

    BYTE            byRepeatCutPercent;							// �ظ�������ʱ�����������ٷֱ�

    BYTE            byEndRequireItemType[QUEST_END_ITEM_COUNT];	//������ʱ������ߵ�����
    WORD			wEndRequireItemIndex[QUEST_END_ITEM_COUNT];	//������ʱ������ߵ�����
    WORD            wEndRequireItemAmount[QUEST_END_ITEM_COUNT];	//������ʱ������ߵ�����
    bool			bIsDeleteEndRequireItem[QUEST_END_ITEM_COUNT];  // ���������ɾ�������ʱ���Ƿ�ɾ���������

    BOOL			bPresentAll[2];								// ��һ��(1-4)�����Ƿ�ȫ������ֻ��һ��
    BYTE			byPresentItemType[cdQuestParamCount];	// ������ʱ�������ߵ�����
    WORD			wPresentItemIndex[cdQuestParamCount];	// ������ʱ�������ߵ�����
    WORD            wPresentItemAmount[cdQuestParamCount];	// ������ʱ�������ߵ�����
    WORD            wPresentItemValuePoint[cdQuestParamCount];	// ������ʱ�������ߵ�����

    DWORD           dwAwardHeroTemplateID[QUEST_AWARD_HERO_COUNT];
    int             nAwardHeroLevel[QUEST_AWARD_HERO_COUNT];
	    
	DWORD			dwCoolDownID;								// CoolDown��ʱ��ID
	
	DWORD			dwPrequestID;			                    // ǰ������ID
	DWORD			dwSubsequenceID;							// ֱ�Ӻ������ID
	int				nStartTime;									// ��ʼʱ��
	int				nEndTime;									// ����ʱ��
        
	int				nCostMoney;									//����������Ľ�Ǯ

    BOOL            bEventOrder;                                // ��������Ƿ�˳�����

    KQuestEvent     AllEvent[cdMaxQuestEventCount];             // �����¼�

    int				nPresentExp;								// ������ʱ�����ľ���
    int				nPresentMoney;								// ������ʱ�����Ľ�Ǯ����

    DWORD           dwScriptID;
    BOOL            bAutoFinish;                                // �Ƿ��Զ����
    BOOL            bClientCanFinish;                           // �ͻ����ܷ�ǿ���������
    BOOL            bDaily;                                     // �Ƿ��ճ�����

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

    DECLARE_LUA_STRUCT_STRING(QuestName, _NAME_LEN, szQuestName);          // ��������
    DECLARE_LUA_STRUCT_BOOL(Repeat, bRepeat);					// �Ƿ���ظ�
    DECLARE_LUA_STRUCT_BOOL(Accept, bAccept);					// �Ƿ��ȽӲ��ܽ�

    DECLARE_LUA_STRUCT_DWORD(CoolDownID, dwCoolDownID);		//CoolDown��ʱ��ID
    DECLARE_LUA_STRUCT_DWORD(PrequestID, dwPrequestID);			//ǰ������ID
    DECLARE_LUA_STRUCT_INTEGER(StartTime, nStartTime);			//��ʼʱ��
    DECLARE_LUA_STRUCT_INTEGER(EndTime, nEndTime);				//����ʱ��
    DECLARE_LUA_STRUCT_DWORD(SubsequenceID, dwSubsequenceID);	//ֱ�Ӻ������ID   
    DECLARE_LUA_STRUCT_INTEGER(CostMoney, nCostMoney);							//���ĵĽ�Ǯ

    DECLARE_LUA_STRUCT_INTEGER(QuestValue1, AllEvent[0].nQuestValue); //�������
    DECLARE_LUA_STRUCT_INTEGER(QuestValue2, AllEvent[1].nQuestValue); //�������
    DECLARE_LUA_STRUCT_INTEGER(QuestValue3, AllEvent[2].nQuestValue); //�������
    DECLARE_LUA_STRUCT_INTEGER(QuestValue4, AllEvent[3].nQuestValue); //�������

    DECLARE_LUA_STRUCT_INTEGER(QuestEvent1, AllEvent[0].nEvent); //�����¼�
    DECLARE_LUA_STRUCT_INTEGER(QuestEvent2, AllEvent[1].nEvent); //�����¼�
    DECLARE_LUA_STRUCT_INTEGER(QuestEvent3, AllEvent[2].nEvent); //�����¼�
    DECLARE_LUA_STRUCT_INTEGER(QuestEvent4, AllEvent[3].nEvent); //�����¼�

    DECLARE_LUA_STRUCT_INTEGER(PresentExp, nPresentExp);					//������ʱ�����ľ���
    DECLARE_LUA_STRUCT_INTEGER(PresentMoney, nPresentMoney);				//������ʱ�����Ľ�Ǯ����
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

    KQuestInfo	m_DefaultQuestInfo;		//Ĭ���趨

    BOOL LoadQuestInfo(ITabFile* piTabFile, int nIndex, KQuestInfo* pQuestInfo);

    typedef std::vector<DWORD> KQUEST_GROUP;
    typedef std::map<int, KQUEST_GROUP> KMAP_LEVEL2QUESTGROUP;

    KMAP_LEVEL2QUESTGROUP   m_RandomQuestGroup;
    std::vector<DWORD>      m_DailyQuest;

    BOOL LoadRandomQuestGroup();
};

#endif	//_KQUEST_INFO_LIST_H_
