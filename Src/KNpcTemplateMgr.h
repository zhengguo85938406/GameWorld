// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KNpcTemplateMgr.h
//  Creator		: Hanruofei
//	Date		: 7/27/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include <map>
#include <vector>

typedef std::vector<DWORD> KVEC_DWORD;

enum KNpcEventLabel
{
    KNpcEventLabelLiZheng   = 1,
    KNpcEventLabelSaMa      = 2,
    KNpcEventLabelSiMaTe    = 3,
    KNpcEventLabelNuoCong   = 4,
    KNpcEventLabelHengLi    = 5,
    KNpcEventLabelSiTa      = 6,
    KNpcEventLabelShanXia   = 7,
    KNpcEventLabelCanDi     = 8,
};

struct KNpcTemplate
{
    DWORD   dwID;
    char    szName[_NAME_LEN];
    int     nEventLabel;
    DWORD   dwRepresentID;
    DWORD   dwSkinID;
    int     nLevel;
    int     nMaxEndurance;
    int     nEnduranceReplenish;
    int     nEnduranceReplenishPercent;
    int     nMaxStamina;
    int     nStaminaReplenish;
    int     nStaminaReplenishPercent;
    int     nMaxAngry;
    int     nAdditionalRecoveryAngry;
    int     nWillPower;
    int     nInterference;
    int     nInterferenceRange;
    int     nAddInterferenceRangePercent;
    int     nAttackPoint;
    int     nAttackPointPercent;
    int     nAgility;
    int     nCritPoint;
    int     nCritRate;
    int     nDefense;
    int     nDefensePercent;
    int     nMoveSpeedBase;
    int     nAddMoveSpeedPercent;
    int     nJumpSpeedBase;
    int     nJumpSpeedPercent;
    int     nHitRate;
    int     nNormalShootForce;
    int     nSkillShootForce;
    int     nSlamDunkForce;
    int     nAILevel;
    int     nMaxAILevel;
    int     nMinAILevel;
    int     nLength;
    int     nWidth;
    int     nHeight;
    DWORD   dwAIGroupID;
    int     nAttackAwardHP[MAX_ATTACK_AWARD];
    int     nAttackAwardMoney[MAX_ATTACK_AWARD];
    DWORD   ActiveSkill[KACTIVE_SLOT_TYPE_TOTAL];
    DWORD   PassiveSkill[cdPassiveSkillSlotCount];
    DWORD   NirvanaSkill[cdNirvanaSkillSlotCount];
    DWORD   SlamBallSkill[cdSlamBallSkillSlotCount];
    KVEC_DWORD AllOtherActiveSkill;
};

class KNpcTemplateMgr
{
public:
    KNpcTemplateMgr(void);
    ~KNpcTemplateMgr(void);

    BOOL Init();
    void UnInit();

    KNpcTemplate* GetTemplate(DWORD dwTemplateID);

private:
    BOOL LoadData();

    BOOL LoadSkillList(char cszSkills[], KVEC_DWORD& vecSkill);

private:
    std::map<DWORD, KNpcTemplate> m_Data;
};

