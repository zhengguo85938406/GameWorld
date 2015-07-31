////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KAIVM.h
//  Version     : 1.0
//  Creator     : Chen Jie, zhaochunfeng
//  Comment     : AI ״̬�����
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

// �û��Զ���AI event�� KAI_USER_EVENT ��ʼ
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

    // �������AI����
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
    int                 m_nCurrentEvent; // ��ʾ"����ĳ��Event�Ĵ���������"
    int                 m_nPendingEvent;
    DWORD               m_dwPendingEventSrc;
    int                 m_nPendingEventParam;
};

#endif
