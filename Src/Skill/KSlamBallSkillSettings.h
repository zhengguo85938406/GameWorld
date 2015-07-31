// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KSlamBallSkillSettings.h
//  Creator		: hanruofei
//	Date		: 12/7/2012
//  Comment		: 
//	*********************************************************************
#pragma once

struct KSlamBallSkill
{
    DWORD dwID;
    int nRequireLevel;
    int nHighestTargetBasket;
    int nCostBasketHP;
    int nAddAngry;
    int nAdditionalHitRate;
    BOOL bIgnoreInterference;
    BOOL bBaTi;
    int  nRandomMinVX;
    int  nRandomMaxVX;
    int  nRandomMinVY;
    int  nRandomMaxVY;
    int  nRandomMinVZ;
    int  nRandomMaxVZ;
};

typedef std::map<DWORD, KSlamBallSkill> KMAP_SLAMBALL_SKILL;

class KSlamBallSkillSettings
{
public:
    KSlamBallSkillSettings(void);
    ~KSlamBallSkillSettings(void);

    BOOL Init();
    void UnInit();
    
    KSlamBallSkill* Get(DWORD dwSlamBallSkillID);

private:
    BOOL LoadData();

private:
    KMAP_SLAMBALL_SKILL m_mapSlamBallSkill;
};

