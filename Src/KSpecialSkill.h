// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KSpecialSkill.h 
//  Creator 	: Xiayong  
//  Date		: 05/30/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include <map>
#include "Engine/KMemory.h"

struct KSpecialSkillInfo
{
    uint32_t    dwID;
    int         nCostAngry;
    int         nCostStamina;
    int         nCostAngryPercent;
    int         nCostStaminaPercent;
    int         nFlyTrack;
    int         nDamage;
    int         nDamageRange;
    int         nRequireLevel;
    uint32_t    dwAddBuff;
    uint32_t    dwCastSkill;

    int GetCostAngry(int nCurrentAngry){return nCostAngry + nCostAngryPercent * nCurrentAngry / 1000;}
    int GetCostStamina(int nCurrentStamina){return nCostStamina + nCostStaminaPercent * nCurrentStamina / 1000;}
};

typedef KMemory::KAllocator<std::pair<uint32_t, KSpecialSkillInfo> > KSPECIAL_SKILL_MAP_ALLOCTOR;
typedef std::map<uint32_t, KSpecialSkillInfo, std::less<uint32_t>, KSPECIAL_SKILL_MAP_ALLOCTOR> KSPECIAL_SKILL_MAP;

class KSpecialSkill
{
public:
    BOOL                Load();
    KSpecialSkillInfo*  Get(uint32_t dwID);

private:
    KSPECIAL_SKILL_MAP m_SpecialSkill;
};
