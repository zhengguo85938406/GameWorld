#include "stdafx.h"
#include "KWorldSettings.h"

KWorldSettings::KWorldSettings()
{
}

KWorldSettings::~KWorldSettings()
{
}

BOOL KWorldSettings::Init(void)
{
    BOOL bResult                        = false;
	BOOL bRetCode                       = false;
    BOOL bConstListInitFlag             = false;
    BOOL bMapInit                       = false;
    BOOL bCoolDownInitFlag              = false;
    BOOL bQuestInfoListInitFlag         = false;
    BOOL bGMInfoListInitFlag            = false;
    BOOL bTrainingTemplateInitFlag      = false;
    BOOL bLadderLevelDataInitFlag       = false;
    BOOL bEADataInitFlag                = false;
    BOOL bNpcTemplateInitFlag           = false;
    BOOL bRecipeDataInitFlag            = false;
    BOOL bBattleSettingInitFlag         = false;
    BOOL bAffectSettingInitFlag         = false;
    BOOL bPlayerValueInfoListInitFlag   = false;
    BOOL bChallengeMissionNpcInitFlag   = false;
    BOOL bAchievementSettingsInitFlag   = false;
    BOOL bVitalitySettingsInitFlag      = false;
    BOOL bSystemMailMgrInitFlag         = false;
    BOOL bSlamBallSkillSettingsInitFlag = false;
    BOOL bComplexPathListInitFlag       = false;
    BOOL bPathPointList                 = false;
    BOOL bMentorCutInitFlag             = false;
    BOOL bRankExpSettingsInitFlag       = false;

	bRetCode = m_ConstList.Init();
	KGLOG_PROCESS_ERROR(bRetCode);
    bConstListInitFlag = true;

    bRetCode = m_HeroTemplate.Load();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_MapListFile.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bMapInit = true;

    bRetCode = m_ObjTemplate.LoadAll();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_CoolDownList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bCoolDownInitFlag = true;

    bRetCode = m_QuestInfoList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bQuestInfoListInitFlag = true;

    bRetCode = m_GMInfoList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bGMInfoListInitFlag = true;

    bRetCode = m_TrainingTemplateMgr.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bTrainingTemplateInitFlag = true;

    bRetCode = m_LadderLevelData.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bLadderLevelDataInitFlag = true;

    bRetCode = m_EAData.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bEADataInitFlag = true;

    bRetCode = m_NpcTemplateMgr.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bNpcTemplateInitFlag = true;
    
    bRetCode = m_RecipeData.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bRecipeDataInitFlag = true;

    bRetCode = m_BattleSetting.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bBattleSettingInitFlag = true;

    bRetCode = m_SystemMailMgr.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bSystemMailMgrInitFlag = true;

    bRetCode = m_PlayerLevelData.LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_AffectSetting.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bAffectSettingInitFlag = true;

    bRetCode = m_MentorCutSettings.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bMentorCutInitFlag = true;

    bRetCode = m_EquipLevelData.Load("Gym/EquipLevelData.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_HeroTalent.Load("Gym/HeroTalent.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_HeroTalentAttr.Load("Gym/HeroTalentAttr.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_TrainingTeacher.Load("Gym/TrainingTeacher.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_SafeBox.Load("Gym/SafeBox.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Store[estypeKFC].Load("Gym/Store-KFC.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Store[estypeClothing].Load("Gym/Store-Clothing.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Store[estypeSport].Load("Gym/Store-Sport.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_VIPLevelData.Load("Gym/VIP-LevelData.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_VIPConfig.Load("Gym/VIP-Config.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_VIPCharge.Load("Gym/VIP-Charge.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_FatiguePointPriceSettings.Load("FatiguePointPrice.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_PassiveSkillData.Load("PassiveSkillList.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_SkillSlotCountSettings.Load("SkillSlot.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_ActivePlayerAward.Load("ActivePlayerAward.tab");
    KGLOG_PROCESS_ERROR(bRetCode);
  
    bRetCode = m_PvPAward.Load("PVPAward.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_PlayerValueInfoList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bPlayerValueInfoListInitFlag = true;

    bRetCode = m_ChallengeMissionNpc.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bChallengeMissionNpcInitFlag = true;

    bRetCode = m_AchievementSettings.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bAchievementSettingsInitFlag = true;

    bRetCode = m_ActivePlayerSettings.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bVitalitySettingsInitFlag = true;

    bRetCode = m_SlamBallSkillSettings.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bSlamBallSkillSettingsInitFlag = true;

    bRetCode = m_ComplexPathList.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bComplexPathListInitFlag = true;

    bRetCode = m_PathPointData.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bPathPointList = true;

    bRetCode = m_DecomposeEquipLuckItem.Load("DecomposeEquipLuckItem.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_DecomposeQuality.Load("DecomposeEquipQuality.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_RankExpSettings.Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bRankExpSettingsInitFlag = true;

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bRankExpSettingsInitFlag)
        {
            m_RankExpSettings.UnInit();
            bRankExpSettingsInitFlag = false;
        }
        

        if (!bMentorCutInitFlag)
        {
            m_MentorCutSettings.UnInit();
            bMentorCutInitFlag = false;
        }
        
        if (bPathPointList)
        {
            m_PathPointData.UnInit();
            bPathPointList = false;
        }

        if (bComplexPathListInitFlag)
        {
            m_ComplexPathList.UnInit();
            bComplexPathListInitFlag = false;
        }

        if (bSlamBallSkillSettingsInitFlag)
        {
            m_SlamBallSkillSettings.UnInit();
            bSlamBallSkillSettingsInitFlag = false;
        }

        if (bAchievementSettingsInitFlag)
        {
            m_AchievementSettings.UnInit();
            bAchievementSettingsInitFlag = false;
        }
        

        if (bChallengeMissionNpcInitFlag)
        {
            m_ChallengeMissionNpc.UnInit();
            bChallengeMissionNpcInitFlag = false;
        }
        
        if (bPlayerValueInfoListInitFlag)
        {
            m_PlayerValueInfoList.UnInit();
            bPlayerValueInfoListInitFlag = false;
        }
        
        if (bAffectSettingInitFlag)
        {
            m_AffectSetting.UnInit();
            bAffectSettingInitFlag = false;
        }

        if (bBattleSettingInitFlag)
        {
            m_BattleSetting.UnInit();
            bBattleSettingInitFlag = false;
        }

        if (bSystemMailMgrInitFlag)
        {
            m_SystemMailMgr.UnInit();
            bSystemMailMgrInitFlag = false;
        }
        
        if (bRecipeDataInitFlag)
        {
            m_RecipeData.UnInit();
            bRecipeDataInitFlag = false;
        }
        
        if (bNpcTemplateInitFlag)
        {
            m_NpcTemplateMgr.UnInit();
            bNpcTemplateInitFlag = false;
        }
        
        if (bEADataInitFlag)
        {
            m_EAData.UnInit();
            bEADataInitFlag = false;
        }
        
        if (bLadderLevelDataInitFlag)
        {
            m_LadderLevelData.UnInit();
            bLadderLevelDataInitFlag = false;
        }
        
        if (bGMInfoListInitFlag)
        {
            m_GMInfoList.UnInit();
            bGMInfoListInitFlag = false;
        }

        if (bQuestInfoListInitFlag)
        {
            m_QuestInfoList.UnInit();
            bQuestInfoListInitFlag = false;
        }
        
        if (bCoolDownInitFlag)
        {
            m_CoolDownList.UnInit();
            bCoolDownInitFlag = false;
        }
        
        if (bMapInit)
        {
            m_MapListFile.UnInit();
            bMapInit = false;
        }
        
        if (bConstListInitFlag)
        {
            m_ConstList.UnInit();
            bConstListInitFlag = false;
        }

        if (bTrainingTemplateInitFlag)
        {
            m_TrainingTemplateMgr.UnInit();
            bTrainingTemplateInitFlag = false;
        }
        
        if (bVitalitySettingsInitFlag)
        {
            m_ActivePlayerSettings.UnInit();
            bVitalitySettingsInitFlag = false;
        }
    }
	return bResult;
}

void KWorldSettings::UnInit(void)
{
    m_MentorCutSettings.UnInit();
    m_PathPointData.UnInit();
    m_ComplexPathList.UnInit();
    m_SlamBallSkillSettings.UnInit();
    m_AchievementSettings.UnInit();
    m_ActivePlayerSettings.UnInit();
	m_ChallengeMissionNpc.UnInit();
    m_PlayerValueInfoList.UnInit();
    m_AffectSetting.UnInit();
    m_BattleSetting.UnInit();
    m_SystemMailMgr.UnInit();
    m_RecipeData.UnInit();
    m_NpcTemplateMgr.UnInit();
    m_EAData.UnInit();
	m_LadderLevelData.UnInit();
    m_QuestInfoList.UnInit();
    m_CoolDownList.UnInit();
    m_MapListFile.UnInit();
	m_ConstList.UnInit();
    m_GMInfoList.UnInit();

	return;
}

