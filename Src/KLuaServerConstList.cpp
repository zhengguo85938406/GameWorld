#include "stdafx.h"
#include "Luna.h"
#include "KSceneGlobal.h"
#include "KAIVM.h"
#include "KGItemDef.h"

#define REGISTER_CONST(__VALUE__)	{#__VALUE__, __VALUE__},
#define REGISTER_CONST_LIST(__VALUE__)	{#__VALUE__, LUA_CONST_##__VALUE__},

KLuaConst LUA_CONST_GLOBAL[] =
{
    {"GAME_FPS",			    GAME_FPS},
    {"DIRECTION_COUNT",         DIRECTION_COUNT},
    {"CELL_LENGTH",			    CELL_LENGTH},
    {"LOGICAL_CELL_CM_LENGTH",	LOGICAL_CELL_CM_LENGTH},
    {"INVALID_PARTY_ID",        ERROR_ID},
    {"INVALID_SKILL_ID",        INVALID_SKILL_ID},
    {"MOVE_DEST_RANGE",         MOVE_DEST_RANGE},

    {"AI_USER_EVENT",           KAI_USER_EVENT},
    {"AI_USER_ACTION",          KAI_USER_ACTION},
    {NULL,						0}
};


KLuaConst LUA_AI_ACTION_KEY[] = 
{
    {"INVALID",                                 eakInvalid},

    {"SET_STATE",                               eakSetState},
    {"NONE_OP",                                 eakNoneOp},
    {"SET_PRIMARY_TIMER",                       eakSetPrimaryTimer},
    {"SET_SECONDARY_TIMER",                     eakSetSecondaryTimer},
    {"SET_TERTIARY_TIMER",                      eakSetTertiaryTimer},
    {"RANDOM_BI_BRANCH",                        eakRandomBiBranch},
    {"RANDOM_TRI_BRANCH",                       eakRandomTriBranch},
    {"STAND",                                   eakStand},
    {"JUMP",                                    eakJump},
    {"SET_MOVE_TARGET",                         eakSetMoveTarget},
    {"MOVE_TO_TARGET",                          eakMoveToTarget},
    {"CHECK_NEAR_TARGETA",                      eakCheckNearTargetA},
    {"TAKE_BALL",                               eakTakeBall},
    {"AIM_AT",                                  eakAimAt},
    {"CHECK_AIM_FLOOR",                         eakCheckAimFloor},
    {"SHOOT_BALL",                              eakShootBall},
    {"CHECK_FOLLOWER",                          eakCheckFollower},
    {"CHECK_TALL_TAKER",                        eakCheckBallTaker},
    {"CAST_SKILL",                              eakCastSkill},
    {"CHECK_HERO_STATE",                        eakCheckHeroState},
    {"TAKE_DOODAD",                             eakTakeDoodad},
    {"USE_DOODAD",                              eakUseDoodad},
    {"SHOOT_DOODAD",                            eakShootDoodad},
    {"DROP_DOODAD",                             eakDropDoodad},
    {"PASS_BALL",                               eakPassBall},
    {"CHANGE_DIRECTION",                        eakChangeDirection},
    {"CHECK_BALL_MOVETYPE",                     eakCheckBallMoveType},
    {"LOG",                                     eakLog},
    {"IS_APPROACHING_TARGET",                   eakIsApproachingTarget},
    {"IS_TARGET_IN_FRONT",                      eakIsTargetInFront},    
    {"CHECK_DISTANCE_TO_TARGET",                eakCheckDistanceToTarget},      
    {"SET_RANDOM_TIMER",                        eakSetRandomTimer},
    {"HAS_ENEMY_NEARBY",                        eakHasEnemyNearBy},
    {"CHECK_TARGET_HERO_STATE",                 eakCheckTargetHeroState},
    {"CHECK_DELTA_SCORE",                       eakCheckDeltaScore},
    {"CHECK_HERO_PROPERTY",                     eakCheckHeroProperty},
    {"CHECK_DISTANCE_TO",                       eakCheckDistanceTo},
    {"CAN_JUMP_TO_TARGET",                      eakCanJumpToTarget},
    {"IS_PLAYER_TEAMMATE",                      eakIsPlayerTeammate},
    {"CHECK_BASKET_STATE",                      eakCheckBasketState},
    {"CHECK_HERO_COUNT_BY_SIDE",                eakCheckHeroCountBySide},
    {"CHECK_BASKET_IN_BACKBOARD",               eakCheckBasketInBackboard},
    {"CAN_SKILL_HIT",                           eakCanSkillHit},
    {"USE_ITEM",                                eakUseItem},
    {"CHECK_STAND_ON",                          eakCheckStandOn},
    {"CHECK_HOLDING",                           eakCheckHolding}, 
    {"CHECK_TEAMMATE_TYPE",                     eakCheckTeammateType}, 
    {"CHECK_CAN_SKILL_SHOOT",                   eakCheckCanSkillShoot}, 
    {"CHECK_BATTLE_START_TIME",                 eakCheckBattleStartTime},
    {"HAS_DOODAD_NEAR_BY",                      eakHasDoodadNearBy},
    {"GO_AROUND",                               eakGoAround},
    {"CHECK_POSITION_RELATIONXY",               eakCheckPositionRelationXY},
    {"IS_AT_EDGE",                              eakIsAtEdge},
    {"AUTO_MOVE",                               eakAutoMove},
    {"STOP_AUTO_MOVE",                          eakStopAutoMove},
    {"IS_CASTING_SKILL",                        eakIsCastingSkill},
    {"MOVE",                                    eakMove},
    {"HAS_SLAMBALL_SKILL",                      eakHasSlamBallSkill},
    {"HAS_NIRVANA_SKILL",                       eakHasNirvanaSkill},
    {"HAS_TEAMMATE",                            eakHasTeammate},
    {"CAN_SKILLSLAM_BASKETLEVEL",               eakCanSkillSlamBasketLevel},
    {"SPECIAL_USE_DOODAD",                      eakSpecialUseDoodad},
    {"CHECK_SKILL_SERIES",                      eakCheckSkillSeries},
    {"CAST_NIRVANA_SKILL",                      eakCastNirvanaSkill},
    {"CAST_SLAM_BALL_SKILL",                    eakCastSlamBallSkill},
        
    {NULL, 0}
};

KLuaConst LUA_AI_EVENT[] =
{
    {"INVALID",                         aevInvalid},

    {"ON_GAME_START",                   aevOnGameStart},
    {"ON_PRIMARY_TIMER",                aevOnPrimaryTimer},
    {"ON_SECONDARY_TIMER",              aevOnSecondaryTimer},
    {"ON_TERTIARY_TIMER",               aevOnTertiaryTimer},

    {"ON_SELF_TAKE_BALL",               aevOnSelfTakeBall},
    {"ON_TEAMMATE_TAKE_BALL",           aevOnTeammateTakeBall},
    {"ON_ENEMY_TAKE_BALL",              aevOnEnemyTakeBall},

    {"ON_SELF_DROP_BALL",               aevOnSelfDropBall},
    {"ON_TEAMMATE_DROP_BALL",           aevOnTeammateDropBall},
    {"ON_ENEMY_DROP_BALL",              aevOnEnemyDropBall},

    {"ON_TEAMMATE_NORMAL_SHOOT",        aevOnTeammateNormalShoot},
    {"ON_ENEMY_NORMAL_SHOOT",           aevOnEnemyNormalShoot},
    {"ON_TEAMMATE_SKILL_SHOOT",         aevOnTeammateSkillShoot},
    {"ON_ENEMY_SKILL_SHOOT",            aevOnEnemySkillShoot},
    {"ON_TEAMMATE_SKILL_SLAM",          aevOnTeammateSkillSlam},
    {"ON_ENEMY_SKILL_SLAM",             aevOnEnemySkillSlam},
    {"ON_TEAMMATE_SLAM",                aevOnTeammateSlam},
    {"ON_ENEMY_SLAM",                   aevOnEnemySlam},

    {"ON_TEAMMATE_PASS_BALL",           aevOnTeammatePassBall},
    {"ON_ENEMY_PASS_BALL",              aevOnEnemyPassBall},

    {"ON_BASKET_DROPPED",               aevOnBasketDropped},
    {"ON_BASKET_PLUGIN",                aevOnBasketPlugin},

    {"ON_ATTACKED",                     aevOnAttacked},

    {"ON_AUTOMOVE_FAILED",              aevOnAutoMoveFailed},
    {"ON_AUTOMOVE_SUCCESS",             aevOnAutoMoveSuccess},



    {"NEARBY_ENEMY_USE_DOODAD",         aevNearByEnemyUseDoodad},
    {"NEARBY_ENEMY_CAST_GRABSKILL",     aevNearByEnemyCastGrabSkill},
    {"NEARBY_ENEMY_CAST_NORMALSKILL",   aevNearByEnemyCastNormalSkill},
    {"NEARBY_ENEMY_CAST_BATISKILL",     aevNearByEnemyCastBaTiSkill},

    {"TOTAL",                           aevTotal},
    {NULL, 0}
};


KLuaConst LUA_CONST_MOVE_STATE[] = 
{
    {"INVALID",		    cmsInvalid},
    {"ON_STAND",        cmsOnStand},
    {"ON_WALK",	        cmsOnWalk},
    {"ON_RUN",		    cmsOnRun},
    {"ON_JUMP",         cmsOnJump},
    {"ON_SIT",	        cmsOnSit},

    {"ON_KNOCKED_DOWN", cmsOnKnockedDown},
    {"ON_KNOCKED_OFF",	cmsOnKnockedOff},
    {"ON_HALT",		    cmsOnHalt},
    {"ON_FREEZE",       cmsOnFreeze},

    {"ON_LOSECONTROL_BYCOUNTER", cmsOnLoseControlByCounter},
    {"ON_STIFF", cmsOnStiff},

    {NULL,			0}
};

KLuaConst LUA_SCENE_OBJ_TYPE[] = 
{
    {"sotInvalid",          sotInvalid},
    {"sotBackboard",        sotBackboard},
    {"sotBasketSocket",     sotBasketSocket},
    {"sotBasket",           sotBasket},
    {"sotBall",             sotBall},
    {"sotChest",            sotChest},
    {"sotLadder",           sotLadder},
    {"sotHero",             sotHero},
    {"sotBullet",           sotBullet},
    {"sotTrap",             sotTrap},
    {"sotLandMine",         sotLandMine},
    {"sotBrick",            sotBrick},
    {"sotClip",             sotClip},
    {"sotTyre",             sotTyre},
    {"sotBuffEffect",       sotBuffEffect},
    {"sotEffect",           sotEffect},
    {"sotHoist",            sotHoist},
    {"sotCandyBag",         sotCandyBag},
    {"sotBaseball",         sotBaseball},
    {"sotBananaPeel",       sotBananaPeel},
    {"sotBox",              sotGift},
    {"sotBarrier",          sotBarrier},
    {"sotTotal",            sotTotal},
     
    {NULL,			0}
};

KLuaConst LUA_SIDE_TYPE[] = 
{
    {"LEFT",    sidLeft},
    {"RIGHT",   sidRight},

    {NULL,			0}
};

KLuaConst LUA_ITEM_TAB_TYPE[] =
{
    {"INVALID",         ittInvalid},

    {"EQUIP",        ittEquip},
    {"OTHER",        ittOther},
    {"FASHION",      ittFashion},
    {"CHEERLEADING", ittCheerleading},
    {"TOTAL",        ittTotal},

    {NULL,			    0}
};

KLuaConst LUA_MONEY_ENUM_TYPE[] = 
{
    {"MONEY",       emotMoney},
    {"COIN",        emotCoin},
    {"MENTERPOINT", emotMenterPoint},

    {NULL,          0}
};

KLuaConst LUA_PLAYER_EVENT[] = 
{
    {"peInvalid",               peInvalid},
    {"peOpenBag",               peOpenBag},
    {"peClientEnd",             peClientEnd},
    {"peShootFirstSuccess",     peShootFirstSuccess},
    {"peShootSecondSuccess",    peShootSecondSuccess},
    {"peShootThirdSuccess",     peShootThirdSuccess},
    {"peNpcDie",                peNpcDie},
    {"peKillSaMa",              peKillSaMa},
    {"peKillLiZheng",           peKillLiZheng},
    {"peKillSaMaAndLiZheng",    peKillSaMaAndLiZheng},
    {"peFinishLadderPVP",       peFinishLadderPVP},
    {"peWinFreePVP",            peWinFreePVP},
    {"peWinLadderPVP",          peWinLadderPVP},
    {"peWinPVE",                peWinPVE},
    {"peDownTwoBasket",         peDownBasket},
    {"peTenScoreAt30Minutes",   peWin10ScoreAt30Second},
    {"peNotUseItemAtGame",      peNotUseItemAtGame},
    {"peUseTenSkillSlamAndWin",     peUseSkillSlam},
    {"peUseTenSceneObj",            peUseSceneObj},
    {"peWin40Score",                peWin40Score},
    {"peEnemy0Score",               peEnemy0Score},
    {"peNotDieAndWin",              peNotDieAndWin},

    {"peFinishEasyKanshousuo",      peFinishEasyKanshousuo},
    {"peFinishNormalKanshousuo",    peFinishNormalKanshousuo},
    {"peFinishHardKanshousuo",      peFinishHardKanshousuo},
    {"peFinishEasyTangRenJie",      peFinishEasyTangRenJie},
    {"peFinishNormalTangRenJie",    peFinishNormalTangRenJie},
    {"peFinishHardTangRenJie",      peFinishHardTangRenJie},
    {"peFinishEasyTangGuoDuShi",    peFinishEasyTangGuoDuShi},
    {"peFinishNormalTangGuoDuShi",  peFinishNormalTangGuoDuShi},
    {"peFinishHardTangGuoDuShi",    peFinishHardTangGuoDuShi},
    {"peFinishEasyYangguanghaitan", peFinishEasyYangguanghaitan},
    {"peFinishNormalYangguanghaitan",   peFinishNormalYangguanghaitan},
    {"peFinishHardlYangguanghaitan",    peFinishHardlYangguanghaitan},
    {"peFinishEasyWuLiYanJiuSuo",   peFinishEasyWuLiYanJiuSuo},
    {"peFinishNormalWuLiYanJiuSuo", peFinishNormalWuLiYanJiuSuo},
    {"peFinishHardlWuLiYanJiuSuo",  peFinishHardlWuLiYanJiuSuo},
    {"peSlamDunk",                  peSlamDunk},

    {"pePutOnEquipment",            pePutOnEquipment},
    {"peMoveItemToShortcutBar",     peMoveItemToShortcutBar},
    {"peEmployHero",                peEmployHero},
    {"peMakeEquipment",             peMakeEquipment},
    {"peFinishKanShouSuo",          peFinishKanShouSuo},
    {"peFinishBangQiuChang",        peFinishBangQiuChang},
    {"peFinishTangGuoDuShi",        peFinishTangGuoDuShi},
    {"peFinishWuLiYanJiuSuo",       peFinishWuLiYanJiuSuo},
    {"peFinishYangGuangHaiTan",     peFinishYangGuangHaiTan},
    {"peFinishTangRenJie",          peFinishTangRenJie},
    {"peFinishMoTianDaLou",         peFinishMoTianDaLou},
    {"peFinishHangKongMuJian",      peFinishHangKongMuJian},
    {"peFinishEasyBangQiuChang",    peFinishEasyBangQiuChang},
    {"peFinishNormalBangQiuChang",  peFinishNormalBangQiuChang},
    {"peFinishHardBangQiuChang",    peFinishHardBangQiuChang},
    {"peFinishEasyMoTianDaLou",     peFinishEasyMoTianDaLou},
    {"peFinishNormalMoTianDaLou",   peFinishNormalMoTianDaLou},
    {"peFinishHardMoTianDaLou",     peFinishHardMoTianDaLou},
    {"peFinishEasyHangKongMuJian",  peFinishEasyHangKongMuJian},
    {"peFinishNormalHangKongMuJian",    peFinishNormalHangKongMuJian},
    {"peFinishHardHangKongMuJian",  peFinishHardHangKongMuJian},
    {"peDailySign",                 peDailySign},
	{"peCastNirvanaSkill",			peCastNirvanaSkill},
	{"peDecomposeEquip",			peDecomposeEquip},
	{"peBeAttackBySceneObj",		peBeAttackBySceneObj},
	{"peEnemyBeAttackBySceneObj",	peEnemyBeAttackBySceneObj},
	{"peNpcStaminaEmpty",			peNpcStaminaEmpty},
	{"peShootSuccess",				peShootSuccess},

    {NULL,          0}
};

KLuaConst LUA_PLAYER_VALUE[] = 
{
    {"FAVORITE", KPLAYER_VALUE_ID_FAVORITE},
    {"DAY01", KPLAYER_VALUE_ID_DAY01 },
    {"DAY02", KPLAYER_VALUE_ID_DAY02 },
    {"DAY03", KPLAYER_VALUE_ID_DAY03 },
    {"DAY04", KPLAYER_VALUE_ID_DAY04 },
    {"DAY05", KPLAYER_VALUE_ID_DAY05 },
    {"DAY06", KPLAYER_VALUE_ID_DAY06 },
    {"DAY07", KPLAYER_VALUE_ID_DAY07 },
    {"DAY08", KPLAYER_VALUE_ID_DAY08 },
    {"DAY09", KPLAYER_VALUE_ID_DAY09 },
    {"DAY10", KPLAYER_VALUE_ID_DAY10 },
    {"DAY11", KPLAYER_VALUE_ID_DAY11 },
    {"DAY12", KPLAYER_VALUE_ID_DAY12 },
    {"DAY13", KPLAYER_VALUE_ID_DAY13 },
    {"DAY14", KPLAYER_VALUE_ID_DAY14 },
    {"DAY15", KPLAYER_VALUE_ID_DAY15 },
    {"DAY16", KPLAYER_VALUE_ID_DAY16 },
    {"DAY17", KPLAYER_VALUE_ID_DAY17 },
    {"DAY18", KPLAYER_VALUE_ID_DAY18 },
    {"DAY19", KPLAYER_VALUE_ID_DAY19 },
    {"DAY20", KPLAYER_VALUE_ID_DAY20 },
    {"DAY21", KPLAYER_VALUE_ID_DAY21 },
    {"DAY22", KPLAYER_VALUE_ID_DAY22 },
    {"DAY23", KPLAYER_VALUE_ID_DAY23 },
    {"DAY24", KPLAYER_VALUE_ID_DAY24 },
    {"DAY25", KPLAYER_VALUE_ID_DAY25 },
    {"DAY26", KPLAYER_VALUE_ID_DAY26 },
    {"DAY27", KPLAYER_VALUE_ID_DAY27 },
    {"DAY28", KPLAYER_VALUE_ID_DAY28 },
    {"DAY29", KPLAYER_VALUE_ID_DAY29 },
    {"DAY30", KPLAYER_VALUE_ID_DAY30 },
    {"DAY31", KPLAYER_VALUE_ID_DAY31 },
    {"TOTAL_DAYS5",         KPLAYER_VALUE_ID_TOTAL_5DAYS },
    {"TOTAL_DAYS10",        KPLAYER_VALUE_ID_TOTAL_10DAYS},
    {"TOTAL_DAYS21",        KPLAYER_VALUE_ID_TOTAL_21DAYS},
    {"CONTINUOUS_DAYS2",    KPLAYER_VALUE_ID_CONTINUOUS_2DAYS},
    {"CONTINUOUS_DAYS7",    KPLAYER_VALUE_ID_CONTINUOUS_7DAYS},
    {"CONTINUOUS_DAYS14",   KPLAYER_VALUE_ID_CONTINUOUS_14DAYS},
    {"FIRST_CHARGE",        KPLAYER_VALUE_ID_FIRST_CHARGE},
    {"FIRST_CHARGE_AWARD",  KPLAYER_VALUE_ID_FIRST_CHARGE_AWARD},
    {NULL, 0}
};

KLuaConst LUA_CUSTOM_CONSUME_TYPE[] = 
{
    {"SCRIPT_CONSUME", uctScriptConsume},
    {NULL, 0},
};

KLuaConst LUA_REPORT_EVENT[] = 
{
    {"KREPORT_EVNET_CAN_GET_SIGN_AWARD",    KREPORT_EVNET_CAN_GET_SIGN_AWARD},
    {"KREPORT_EVNET_NOT_SIGN",          KREPORT_EVNET_NOT_SIGN},

    {NULL, 0}
};

KLuaConst LUA_SIGN_TYPE[] = 
{
    {"CARD", KSIGN_TYPE_CARD},
    {"COIN", KSIGN_TYPE_COIN},
};

KLuaConst LUA_CLIENT_CALL[] =
{
    {"FAVORITE",    KCLIENT_CALL_FAVORITE},
    {"DAILY_SIGN",  KCLIENT_CALL_DAILY_SIGN},
    {"CARD_SIGN",   KCLIENT_CALL_CARD_SIGN},
    {"COIN_SIGN",   KCLIENT_CALL_COIN_SIGN},
    {"QUICK_SIGN",   KCLIENT_CALL_QUICK_SIGN},
    {"SIGN_AWARD_TOTAL_5DAYS",        KCLIENT_CALL_SIGN_AWARD_TOTAL_5DAYS},
    {"SIGN_AWARD_TOTAL_10DAYS",       KCLIENT_CALL_SIGN_AWARD_TOTAL_10DAYS},
    {"SIGN_AWARD_TOTAL_21DAYS",       KCLIENT_CALL_SIGN_AWARD_TOTAL_21DAYS},
    {"SIGN_AWARD_CONTINUOUS_2DAYS",   KCLIENT_CALL_SIGN_AWARD_CONTINUOUS_2DAYS},
    {"SIGN_AWARD_CONTINUOUS_7DAYS",   KCLIENT_CALL_SIGN_AWARD_CONTINUOUS_7DAYS},
    {"SIGN_AWARD_CONTINUOUS_14DAYS",  KCLIENT_CALL_SIGN_AWARD_CONTINUOUS_14DAYS},
    {"FIRST_CHARGE_AWARD",  KCLIENT_CALL_FIRST_CHARGE_AWARD},
    {NULL,  0},
};

KLuaConstList g_LuaServerConstList[] =
{   
    {"GLOBAL",                              LUA_CONST_GLOBAL},
    {"AI_ACTION",                           LUA_AI_ACTION_KEY},
    {"AI_EVENT",                            LUA_AI_EVENT},
    {"MOVE_STATE",                          LUA_CONST_MOVE_STATE},
    {"SCENE_OBJ_TYPE",                      LUA_SCENE_OBJ_TYPE},
    {"SIDE_TYPE",                           LUA_SIDE_TYPE},
    {"ITEM_TAB_TYPE",                       LUA_ITEM_TAB_TYPE},
    {"MONEY_TYPE",                          LUA_MONEY_ENUM_TYPE},
    {"PLAYER_EVENT",                        LUA_PLAYER_EVENT},
    {"PLAYER_VALUE",                        LUA_PLAYER_VALUE},
    {"CUSTOM_CONSUME_TYPE",                 LUA_CUSTOM_CONSUME_TYPE},
    {"SIGN_TYPE",                           LUA_SIGN_TYPE},
    {"CLIENT_CALL",                         LUA_CLIENT_CALL},
    {"REPORT",                              LUA_REPORT_EVENT},

    {NULL,                      			0}
};
