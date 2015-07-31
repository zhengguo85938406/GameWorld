// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KHeroTemplate.h 
//  Creator 	: Xiayong  
//  Date		: 10/20/2011
//  Comment	: 
// ***************************************************************
#pragma once
#include "Ksdef/TypeDef.h"
#include "GlobalMacro.h"
#include "game_define.h"

class KPlayer;

struct KHeroLevelInfo 
{
    int nLevel;
    int nExp;
    int nMaxEndurance;
    int nEnduranceReplenish;
    int nEndurancereplenishPercent;
    int nMaxStamina;
    int nStaminaReplenish;
    int nStaminaReplenishPercent;
    int nMaxAngry;
    int nAdditionalAddAngry;
    int nWillPower;
    int nInterference;
    int nInterferenceRange;
    int nAddInterferenceRangePercent;
    int nAttackPoint;
    int nAttackPointPercent;
    int nAgility;
    int nCritPoint;
    int nCritRate;
    int nDefense;
    int nDefensePercent;
    int nMoveSpeed;
    int nAddMoveSpeedPercent;
    int nJumpingAbility;
    int nJumpingAbilityPercent;
    int nShootBallHitRate;
    int nNormalShootForce;
    int nSkillShootForce;
    int nSlamDunkForce;
    int nAIType;
    int nDefenseAIType;
    int nLenth;
    int nWidth;
    int nHeight;
};
typedef std::map<int, KHeroLevelInfo> KMAP_HERO_LEVEL_INFO;
typedef std::vector<DWORD> KVEC_DWORD;

struct KHeroTemplateInfo
{
    DWORD                   dwTemplateID;
    char                    szName[_NAME_LEN];
    KMAP_HERO_LEVEL_INFO    HeroLevelData;
    UNLOCK_TYPE             eUnlockType;
    uint32_t                dwLockData;
    int                     nBuyMoneyType;
    int                     nBuyPrice;
    int                     nCanFire;
	int						nDisCountTime; //限时打折
    DWORD                   dwAIGroupID;
    KVEC_DWORD              AllFashionID;
    DWORD                   dwInitBaseFistSkill;
    DWORD                   dwInitBaseFootSkill;
    KVEC_DWORD              AllActiveSkill[KACTIVE_SLOT_TYPE_TOTAL];
    KVEC_DWORD              AlllPassiveSkill;
    KVEC_DWORD              AllNirvanaSkill;
    KVEC_DWORD              AllSlamBallSkill;  
    KVEC_DWORD              AllOtherActiveSkill; // 只要等级到了就可以使用
};
typedef std::map<DWORD, KHeroTemplateInfo> KMAP_HERO_TEMPLATE;

class KHeroTemplate
{
public:
    KHeroTemplate();
    ~KHeroTemplate();

    BOOL Load();

    KHeroTemplateInfo*	GetHeroTemplate(DWORD dwHeroTemplateID);
    KHeroLevelInfo*		GetHeroLevelInfo(DWORD dwHeroTemplateID, int nLevel);
	const KMAP_HERO_TEMPLATE&	GetHeroTemplateMap();
    void ReportUnLockHero(KPlayer* pPlayer, int eUnlockType, DWORD dwLockData);
private:
    BOOL LoadHeroLevelData(const char cszFileName[], KMAP_HERO_LEVEL_INFO& rHeroLevelInfo);
    BOOL LoadSkillList(char cszSkills[], KVEC_DWORD& vecSkill);
    BOOL LoadFashionIDList(char cszFashionIDs[], KVEC_DWORD& vecFashionIDs);

    KMAP_HERO_TEMPLATE m_mapHeroTemplate;
};
