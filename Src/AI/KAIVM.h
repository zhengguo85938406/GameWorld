////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KAIVM.h
//  Version     : 1.0
//  Creator     : Chen Jie, zhaochunfeng
//  Comment     : AI 状态机外壳
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _KAIVM_H_ 
#define _KAIVM_H_ 

#include "KAIState.h"
#include "KAIAction.h"

class KAIState;
class KHero;
class KAILogic;

enum KAI_EVENT
{
    aevInvalid  = 0,

    aevOnGameStart,

    aevOnPrimaryTimer,
    aevOnSecondaryTimer,
    aevOnTertiaryTimer,

    aevOnSelfTakeBall,
    aevOnTeammateTakeBall,
    aevOnEnemyTakeBall,

    aevOnSelfDropBall,
    aevOnTeammateDropBall,
    aevOnEnemyDropBall,

    aevOnTeammateNormalShoot,
    aevOnEnemyNormalShoot,
    aevOnTeammateSkillShoot,
    aevOnEnemySkillShoot,
    aevOnTeammateSkillSlam,
    aevOnEnemySkillSlam,
    aevOnTeammateSlam,
    aevOnEnemySlam,

    aevOnTeammatePassBall,
    aevOnEnemyPassBall,
    
    aevOnBasketDropped,
    aevOnBasketPlugin,

    aevOnAttacked,

    aevOnAutoMoveFailed,
    aevOnAutoMoveSuccess,
    aevNearByEnemyUseDoodad,
    aevNearByEnemyCastGrabSkill,
    aevNearByEnemyCastNormalSkill,
    aevNearByEnemyCastBaTiSkill,

    aevTotal,
};

// 用户自定义AI event从 KAI_USER_EVENT 开始
#define KAI_USER_EVENT 1000

class KAIVM
{
public:
    KAIVM();

    BOOL Setup(KHero* pCharacter, int nAIType);

    int  GetAIType();

    void Active();

    void FireEvent(int nEvent, DWORD dwEventSrc, int nEventParam);

    BOOL SetState(int nState);
    void SetPrimaryTimer(int nFrame);
    void SetSecondaryTimer(int nFrame);
    void SetTertiaryTimer(int nFrame);

    void DebugAICurrentStateInfo();

public:
    int                 m_nDebugCount;
    uint64_t            m_ullRunTimeStatistic;

    // 用于输出AI流程
    FILE*               m_pAIProcessLogFile;

private:
    KHero*              m_pOwner;
    int                 m_nAIType;
    KAILogic*           m_pAILogic;
    KAIState*           m_pState;
    int                 m_nStateID;
    int                 m_nActionID;
    int                 m_nPrimaryTimerFrame;
    int                 m_nSecondaryTimerFrame;
    int                 m_nTertiaryTimerFrame;
    int                 m_nCurrentEvent; // 标示"正在某个Event的处理流程中"
    int                 m_nPendingEvent;
    DWORD               m_dwPendingEventSrc;
    int                 m_nPendingEventParam;
};

#endif
