////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KAIAction.h
//  Version     : 1.0
//  Creator     : Chen Jie, zhaochunfeng
//  Comment     : 定义所有的AI Action
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _KAIACTION_H_
#define _KAIACTION_H_

#include "Luna.h"


#define KAI_ACTION_PARAM_NUM    5 // 修改这个要万分注意，必须检查到所有的ACTION
#define KAI_ACTION_BRANCH_NUM   3 // 修改这个要万分注意，必须检查到所有的ACTION

#define KAI_ACTION_ID_ERROR     (-1)
#define KAI_ACTION_ID_NONE      0

class KHero;

class KAIAction 
{
public:
    KAIAction();

    int         m_nKey;

    int         m_nParam[KAI_ACTION_PARAM_NUM];
    int         m_nBranch[KAI_ACTION_BRANCH_NUM];
    KAIAction*  m_pBranch[KAI_ACTION_BRANCH_NUM];

public:
    DECLARE_LUA_CLASS(KAIAction);
    int LuaSetParam(Lua_State* L);
    int LuaGetParam(Lua_State* L);
    int LuaSetBranch(Lua_State* L);
};

struct KAIActionHandle 
{
    KAIActionHandle() : nAIActionID(KAI_ACTION_ID_ERROR), pAIAction(NULL) {};

    int         nAIActionID;
    KAIAction*  pAIAction;
};


enum KAI_FILTER_TYPE
{
    airtInvalid =-1,

    airtBaseTarget,
    airtRangeTarget,
    airtNearestTarget,

    airtTotal
};

enum KAI_ACTION_KEY
{
    eakInvalid = 0,

    eakSetState,
    eakSetPrimaryTimer,    
    eakSetSecondaryTimer,
    eakSetTertiaryTimer,
    eakRandomBiBranch,
    eakRandomTriBranch,
    eakStand,
    eakJump,
    eakSetMoveTarget,
    eakMoveToTarget, 
    eakCheckNearTargetA,
    eakTakeBall,
    eakAimAt,
    eakCheckAimFloor,
    eakShootBall,
    eakCheckFollower,
    eakCheckBallTaker,
    eakCastSkill,
    eakNoneOp,
    eakCheckHeroState,
    eakTakeDoodad,
    eakUseDoodad,
    eakShootDoodad,
    eakDropDoodad,
    eakPassBall,
    eakChangeDirection,
    eakCheckBallMoveType,
    eakLog,
    eakIsApproachingTarget,
    eakIsTargetInFront,
    eakCheckDistanceToTarget,
    eakSetRandomTimer,
    eakHasEnemyNearBy,
    eakCheckTargetHeroState,
    eakCheckDeltaScore,
    eakCheckHeroProperty,
    eakCheckDistanceTo,
    eakCanJumpToTarget,
    eakIsPlayerTeammate,
    eakCheckBasketState,
    eakCheckHeroCountBySide,
    eakCheckBasketInBackboard,
    eakCanSkillHit,
    eakUseItem,
    eakCheckStandOn,
    eakCheckHolding,
    eakCheckTeammateType,
    eakCheckCanSkillShoot,
    eakCheckBattleStartTime,
    eakHasDoodadNearBy,
    eakGoAround,
    eakCheckPositionRelationXY,
    eakIsAtEdge,
    eakAutoMove,
    eakStopAutoMove,
    eakIsCastingSkill,
    eakMove,
    eakHasSlamBallSkill,
    eakHasNirvanaSkill,
    eakHasTeammate,
    eakCanSkillSlamBasketLevel,
    eakSpecialUseDoodad,
    eakCheckSkillSeries,
    eakCastNirvanaSkill,
    eakCastSlamBallSkill,
    eakTotal
};

// 用户自定义 ai action key 从 KAI_USER_ACTION 开始
#define KAI_USER_ACTION 1000

typedef int (*KAI_ACTION_FUNC)(KHero* pCharacter, KAIAction* pActionData);

#endif  // _KAIACTION_H_
