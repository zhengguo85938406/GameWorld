#include "stdafx.h"
#include "KQuestInfoList.h"
#include "KSO3World.h"

DEFINE_LUA_CLASS_BEGIN(KQuestInfo)

    REGISTER_LUA_STRING_READONLY(KQuestInfo, QuestName)     // ��������
	REGISTER_LUA_BOOL(KQuestInfo, Repeat)					//�Ƿ���ظ�
	REGISTER_LUA_BOOL(KQuestInfo, Accept)					//�Ƿ��ȽӲ��ܽ�

	REGISTER_LUA_DWORD(KQuestInfo, CoolDownID)			//CoolDown��ʱ��ID
	REGISTER_LUA_DWORD(KQuestInfo, PrequestID)			//ǰ������ID
	REGISTER_LUA_INTEGER(KQuestInfo, StartTime)			//��ʼʱ��
	REGISTER_LUA_INTEGER(KQuestInfo, EndTime)			//����ʱ��
	REGISTER_LUA_DWORD(KQuestInfo, SubsequenceID)		//ֱ�Ӻ������ID   

	REGISTER_LUA_INTEGER(KQuestInfo, QuestValue1)			//�������
	REGISTER_LUA_INTEGER(KQuestInfo, QuestValue2)			//�������
	REGISTER_LUA_INTEGER(KQuestInfo, QuestValue3)			//�������
	REGISTER_LUA_INTEGER(KQuestInfo, QuestValue4)			//�������

    REGISTER_LUA_INTEGER(KQuestInfo, QuestEvent1)			//�����¼�
    REGISTER_LUA_INTEGER(KQuestInfo, QuestEvent2)			//�����¼�
    REGISTER_LUA_INTEGER(KQuestInfo, QuestEvent3)			//�����¼�
    REGISTER_LUA_INTEGER(KQuestInfo, QuestEvent4)			//�����¼�

	REGISTER_LUA_INTEGER(KQuestInfo, PresentExp)			//������ʱ�����ľ���
	REGISTER_LUA_INTEGER(KQuestInfo, PresentMoney)			//������ʱ�����Ľ�Ǯ����

DEFINE_LUA_CLASS_END(KQuestInfo)
