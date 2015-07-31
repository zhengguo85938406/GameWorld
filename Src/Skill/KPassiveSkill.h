#pragma once
#include "GlobalMacro.h"
#include "GlobalStruct.h"
#include "Protocol/gs2cl_protocol.h"
#include "game_define.h"
#include "KTools.h"
#include "KGEnumConvertor.h"
#include "KAttribute.h"
#include "T3GameWorld/KTabConfig.h"


class KHero;
class KPassiveSkill
{
public:
    KPassiveSkill();
    ~KPassiveSkill();

    BOOL Init(KPassiveSkillData* pData, KHero* pHero);
    void UnInit();
    BOOL IsEmpty();

    BOOL IsAddBuff();
    BOOL AddBuff();

    BOOL CanTrigger(int nEvent, int nOldValue, int nNewValue);

    BOOL CheckEventValue(int nOldValue, int nNewValue);
    BOOL CheckCondition();
    BOOL CheckCoolDown();
    void Action();
    
private:
    KHero* m_pOwner;
    KPassiveSkillData* m_pPassiveSkillData;
    time_t m_nActiveFrame;
};
