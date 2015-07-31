////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KAIAction.cpp
//  Version     : 1.0
//  Creator     : Chen Jie, zhaochunfeng
//  Comment     : 定义所有的Action
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KAIAction.h"
#include "KAIManager.h"
#include "KMath.h"
#include "KPlayerServer.h"
#include "KHero.h"
#include "KScene.h"
#include "KBasket.h"
#include "KPlayer.h"

#define MIN_TIMER_INTERVAL  (GAME_FPS / GAME_FPS)

#define REGISTER_AI_ACTION_FUNC(__VALUE__)                          \
    do                                                              \
    {                                                               \
        m_ActionFunctionTable[eak##__VALUE__] = AI##__VALUE__;      \
    } while (0)


KAIAction::KAIAction()
{
    memset(m_nParam, 0, sizeof(m_nParam));
    
    for (int i = 0; i < KAI_ACTION_BRANCH_NUM; i++)
    {
        m_nBranch[i] = KAI_ACTION_ID_ERROR;
        m_pBranch[i] = NULL;
    }
}

int KAIAction::LuaSetParam(Lua_State* L)
{
    BOOL    bResult     = false;
    int     nParamCount = 0;
    int     nValue      = 0;

    nParamCount = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nParamCount <= KAI_ACTION_PARAM_NUM);

    for (int i = 0; i < nParamCount; i++)
    {
        nValue = (int)Lua_ValueToNumber(L, i + 1);
        m_nParam[i] = nValue;
    }

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KAIAction::LuaGetParam(Lua_State* L)
{
    for (int i = 0; i < KAI_ACTION_PARAM_NUM; i++)
    {
        Lua_PushNumber(L, m_nParam[i]);
    }

    return KAI_ACTION_PARAM_NUM;
}

int KAIAction::LuaSetBranch(Lua_State* L)
{
    BOOL    bResult     = false;
    int     nParamCount = 0;
    int     nBranch     = 0;

    nParamCount = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nParamCount <= KAI_ACTION_BRANCH_NUM);

    for (int i = 0; i < nParamCount; i++)
    {
        nBranch = (int)Lua_ValueToNumber(L, i + 1);
        KGLOG_PROCESS_ERROR(nBranch > KAI_ACTION_ID_NONE);
        m_nBranch[i] = nBranch;
    }

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

DEFINE_LUA_CLASS_BEGIN(KAIAction)
REGISTER_LUA_FUNC(KAIAction, SetParam)
REGISTER_LUA_FUNC(KAIAction, GetParam)
REGISTER_LUA_FUNC(KAIAction, SetBranch)
DEFINE_LUA_CLASS_END(KAIAction)

///////////////////////////////////////////////////////////////////////////////////////////////

// Function:    切换AI状态
// Param:       [1]StateID
// Branch:      N/A
// Remark:      这个Action之后不能跟任何的Action（没有分支）。
//              切换State后，等待下一个事件发生的时候，由相应的EventHandle决定AI虚拟机的行为。
int AISetState(KHero* pCharacter, KAIAction* pActionData)
{
    int nState = pActionData->m_nParam[0];

    pCharacter->m_AIVM.SetState(nState);

    return -1;
}

// Function:    设置主AI定时器
// Param:       [1]Frame
// Branch:      [1]Success
// Remark:      当设置的时间到之后，将触发OnPrimaryTimer事件。
//              当OnPrimaryTimer事件触发后，计数器自动清零，除非再次调用SetPrimaryTimer，否则不会再次触发OnPrimaryTimer事件。
int AISetPrimaryTimer(KHero* pCharacter, KAIAction* pActionData)
{
    int nBranchSuccess  = 1;
    int nFrame          = pActionData->m_nParam[0];

    if (nFrame < MIN_TIMER_INTERVAL)
        nFrame = MIN_TIMER_INTERVAL;

    pCharacter->m_AIVM.SetPrimaryTimer(nFrame);

    return nBranchSuccess;
}

// Function:    设置次AI定时器
// Param:       [1]Frame
// Branch:      [1]Success
// Remark:      当设置的时间到之后，将触发OnSecondaryTimer事件。
//              当OnSecondaryTimer事件触发后，计数器自动清零，除非再次调用SetSecondaryTimer，否则不会再次触发OnSecondaryTimer事件。
int AISetSecondaryTimer(KHero* pCharacter, KAIAction* pActionData)
{
    int nBranchSuccess  = 1;
    int nFrame          = pActionData->m_nParam[0];

    if (nFrame < MIN_TIMER_INTERVAL)
        nFrame = MIN_TIMER_INTERVAL;

    pCharacter->m_AIVM.SetSecondaryTimer(nFrame);

    return nBranchSuccess;
}

// Function:    设置第三AI定时器
// Param:       [1]Frame
// Branch:      [1]Success
// Remark:      当设置的时间到之后，将触发OnTertiaryTimer事件。
//              当OnTertiaryTimer事件触发后，计数器自动清零，除非再次调用OnTertiaryTimer，否则不会再次触发OnTertiaryTimer事件。
int AISetTertiaryTimer(KHero* pCharacter, KAIAction* pActionData)
{
    int nBranchSuccess  = 1;
    int nFrame          = pActionData->m_nParam[0];

    if (nFrame < MIN_TIMER_INTERVAL)
        nFrame = MIN_TIMER_INTERVAL;

    pCharacter->m_AIVM.SetTertiaryTimer(nFrame);

    return nBranchSuccess;
}

// Function:    随机二叉分支
// Param:       [1]Rate1 [2]Rate2
// Branch:      [1]Branch1 [2]Branch2
// Remark:      随机产生一个整数n，0 <= n < Rate1 + Rate2。
//              如果n属于[0, Rate1), 则走Branch1分支；
//              否则走Branch2分支。
int AIRandomBiBranch(KHero* pCharacter, KAIAction* pActionData)
{
    int nResult     = 0;
    int nBranch1    = 1;
    int nBranch2    = 2;
    int nRate1      = pActionData->m_nParam[0];
    int nRate2      = pActionData->m_nParam[1];
    int nTotalRate  = nRate1 + nRate2;
    int nRandom     = 0;
   
    KGLOG_PROCESS_ERROR(nRate1 >= 0);
    KGLOG_PROCESS_ERROR(nRate2 >= 0);
    KGLOG_PROCESS_ERROR(nTotalRate > 0);

    nRandom = g_Random(nTotalRate);

    if (nRandom < nRate1)
    {
        nResult = nBranch1;
        goto Exit0;
    }

    nResult = nBranch2;
Exit0:
    return nResult;
}

// Function:    随机三叉分支
// Param:       [1]Rate1 [2]Rate2 [3]Rate3
// Branch:      [1]Branch1 [2]Branch2 [3]Branch3
// Remark:      随机产生一个整数n，0 <= n < Rate1 + Rate2 + Rate3。
//              如果n属于[0, Rate1), 则走Branch1分支；
//              如果n属于[Rate1, Rate1 + Rate2)，则走Branch2分支；
//              否则走Branch3分支。
int AIRandomTriBranch(KHero* pCharacter, KAIAction* pActionData)
{
    int nResult     = 0;
    int nBranch1    = 1;
    int nBranch2    = 2;
    int nBranch3    = 3;
    int nRate1      = pActionData->m_nParam[0];
    int nRate2      = pActionData->m_nParam[1];
    int nRate3      = pActionData->m_nParam[2];
    int nTotalRate  = nRate1 + nRate2 + nRate3;
    int nRandom     = 0;

    KGLOG_PROCESS_ERROR(nRate1 >= 0);
    KGLOG_PROCESS_ERROR(nRate2 >= 0);
    KGLOG_PROCESS_ERROR(nRate3 >= 0);
    KGLOG_PROCESS_ERROR(nTotalRate > 0);

    nRandom = g_Random(nTotalRate);

    if (nRandom < nRate1)
    {
        nResult = nBranch1;
        goto Exit0;
    }

    if (nRandom < nRate1 + nRate2)
    {
        nResult = nBranch2;
        goto Exit0;
    }

    nResult = nBranch3;
Exit0:
    return nResult;
}

// Function:    控制角色站立
// Param:       N/A
// Branch:      [1]Success
// Remark:      
int AIStand(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult  = KAI_BRANCH_FAILED;
    BOOL bRetCode = false;

    bRetCode = pHero->Stand(true);
    if (bRetCode)
        nBranchResult = KAI_BRANCH_SUCCESS;

    return nBranchResult;
}

// Function: 控制角色跳跃
// Param: [0] nDirectionType
// Branch: [1] Success [2] Failed
int AIJump(KHero* pHero, KAIAction* pActionData)
{
    int nBranchSuccess  = 1;
    BOOL bMove = pActionData->m_nParam[0];
    int nDirectionType = pActionData->m_nParam[1];
    int nParam = pActionData->m_nParam[2];

    pHero->AIJump(bMove, nDirectionType, nParam);

    return nBranchSuccess;
}

int AISetMoveTarget(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AISetMoveTarget(pActionData->m_nParam);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AIMoveToTarget(KHero* pHero, KAIAction* pActionData)
{   
    BOOL bWalk = pActionData->m_nParam[0];
    int nBranchResult = pHero->AIMoveToTarget(bWalk);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

// 检查是否已经接近前面指定的目标
int AICheckNearTargetA(KHero* pHero, KAIAction* pActionData)
{
    int nDelta = pActionData->m_nParam[0];

    int nBranchResult = pHero->AICheckNearTargetA(nDelta);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AITakeBall(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AITakeBall();
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AIAimAt(KHero* pHero, KAIAction* pActionData)
{
    BOOL bSelf = pActionData->m_nParam[0];
    int nBranchResult = pHero->AIAimAt(bSelf);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AICheckAimFloor(KHero* pHero, KAIAction* pActionData)
{
    int nFloor = pActionData->m_nParam[0];
    int nBranchResult = pHero->AICheckAimFloor(nFloor);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AIShootBall(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AIShootBall();
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AICheckFollower(KHero* pHero, KAIAction* pActionData)
{
    int nObjType = pActionData->m_nParam[0];
    int nMoveType = pActionData->m_nParam[1];
    int nBranchResult = pHero->AICheckFollower(nObjType, nMoveType);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

// 检查球的持有者和本人的关系
// nType = 0:球无持有者， 1:自己  2:是自己这边的, 3:敌人
int AICheckBallTaker(KHero* pHero, KAIAction* pActionData)
{
    int nType = pActionData->m_nParam[0];
    int nBranchResult = pHero->AICheckBallTaker(nType);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

// 释放技能
// dwSkillID: 技能ID
int AICastSkill(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AICastSkill(pActionData->m_nParam[0]);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AINoneOp(KHero* pHero, KAIAction* pActionData)
{
    return KAI_BRANCH_SUCCESS;
}

int AICheckHeroState(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AICheckHeroState(pActionData->m_nParam);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AITakeDoodad(KHero* pHero, KAIAction* pActionData)
{
    int nDoodadType = pActionData->m_nParam[0];
    int nBranchResult = pHero->AITakeDoodad(nDoodadType);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AIUseDoodad(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AIUseDoodad();
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AIShootDoodad(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AIShootDoodad();
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AIDropDoodad(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AIDropDoodad();
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AIPassBall(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = pHero->AIPassBall();
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AIChangeDirection(KHero* pHero, KAIAction* pActionData)
{
    int nDirectionType  = pActionData->m_nParam[0];
    int nParam          = pActionData->m_nParam[1];
    int nBranchResult   = pHero->AIChangeDirection(nDirectionType, nParam);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AICheckBallMoveType(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult   = pHero->AICheckBallMoveType(pActionData->m_nParam);
    assert(nBranchResult > KAI_BRANCH_BEGIN && nBranchResult < KAI_BRANCH_END);

    return nBranchResult;
}

int AILog(KHero* pHero, KAIAction* pActionData)
{
    return KAI_BRANCH_SUCCESS;
}

int AIIsApproachingTarget(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_ERROR;
    BOOL bRetCode = false;
    int nX = 0;
    int nY = 0;
    int nZ = 0;
    int nDeltaX = 0;
    int nDeltaY = 0;
    int nDeltaZ = 0;
    BOOL bApproachingX = false;
    BOOL bApproachingY = false;
    BOOL bApproachingZ = false;
    BOOL bXFarAway = false;
    BOOL bYFarAway = false;
    BOOL bZFarAway = false;

    bRetCode = pHero->GetAITargetPos(nX, nY, nZ);
    KG_PROCESS_ERROR(bRetCode);

    nDeltaX = nX - pHero->m_nX;
    nDeltaY = nY - pHero->m_nY;
    nDeltaZ = nZ - pHero->m_nZ;

    bApproachingX = (nDeltaX == 0 || nDeltaX * pHero->m_nVelocityX > 0);
    bApproachingY = (nDeltaY == 0 || nDeltaY * pHero->m_nVelocityY > 0);
    bApproachingZ = (nDeltaZ == 0 || nDeltaZ * pHero->m_nVelocityZ > 0);

    bXFarAway = (nDeltaX * pHero->m_nVelocityX < 0);
    bYFarAway = (nDeltaY * pHero->m_nVelocityY < 0);
    bZFarAway = (nDeltaZ * pHero->m_nVelocityZ < 0);

    switch (pActionData->m_nParam[0])
    {
    case KAI_COORDINATE_GROUP_X:
        nBranchResult = bApproachingX ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED;
        break;
    case KAI_COORDINATE_GROUP_Y:
        nBranchResult = bApproachingY ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED;
        break;
    case KAI_COORDINATE_GROUP_Z:
        nBranchResult = bApproachingZ ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED;
        break;
    case KAI_COORDINATE_GROUP_XY:
        nBranchResult = bApproachingX && !bYFarAway || bApproachingY && !bXFarAway ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED;
        break;
    case KAI_COORDINATE_GROUP_XZ:
        nBranchResult = bApproachingX && !bZFarAway || bApproachingZ && !bXFarAway ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED;
        break;
    case KAI_COORDINATE_GROUP_YZ:
        nBranchResult = bApproachingY && !bZFarAway || bApproachingZ && !bYFarAway ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED;
        break;
    case KAI_COORDINATE_GROUP_XYZ:
        nBranchResult = bApproachingX && !bYFarAway && !bZFarAway || bApproachingY && !bXFarAway && !bZFarAway || bApproachingZ && !bXFarAway && !bYFarAway ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED;
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Invalid param(%d) in function %s", pActionData->m_nParam[0], __FUNCTION__);
        goto Exit0;
        break;
    }

Exit0:
    return nBranchResult;
}



int AIIsTargetInFront(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_ERROR;
    BOOL bRetCode = false;
    int nX = 0;
    int nY = 0;
    int nZ = 0;
    int nDeltaX = 0;

    bRetCode = pHero->GetAITargetPos(nX, nY, nZ);
    KGLOG_PROCESS_ERROR(bRetCode);

    nDeltaX = nX - pHero->m_nX;
    if (nDeltaX == 0 || nDeltaX > 0 && pHero->GetFaceDir() == csdRight || nDeltaX < 0 && pHero->GetFaceDir() == csdLeft)
    {
        nBranchResult = KAI_BRANCH_SUCCESS;
    }
    else
    {
        nBranchResult = KAI_BRANCH_FAILED;
    }

Exit0:
    return nBranchResult;
}


int AICheckDistanceToTarget(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_ERROR;
    BOOL bRetCode = false;
    int nX = 0;
    int nY = 0;
    int nZ = 0;
    int nDeltaX = 0;
    int nDeltaY = 0;
    int nDeltaZ = 0;
    KAI_COORDINATE_GROUP eType = (KAI_COORDINATE_GROUP)pActionData->m_nParam[0];
    int nDistanceCtrlValue  = abs(pActionData->m_nParam[1]);
    int nDistanceCtrlValueX = abs(pActionData->m_nParam[1]);
    int nDistanceCtrlValueY = abs(pActionData->m_nParam[2]);
    int nDistanceCtrlValueZ = abs(pActionData->m_nParam[3]);

    bRetCode = pHero->GetAITargetPos(nX, nY, nZ);
    KG_PROCESS_ERROR(bRetCode);

    nDeltaX = abs(nX - pHero->m_nX);
    nDeltaY = abs(nY - pHero->m_nY);
    nDeltaZ = abs(nZ - pHero->m_nZ);

    switch (eType)
    {
    case KAI_COORDINATE_GROUP_X:
        {
            int nCurDistance = nDeltaX;
            nBranchResult = ((nCurDistance <= nDistanceCtrlValue) ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);
        }
        break;
    case KAI_COORDINATE_GROUP_Y:
        {
            int nCurDistance = nDeltaY;
            nBranchResult = ((nCurDistance <= nDistanceCtrlValue) ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);
        }
        break;
    case KAI_COORDINATE_GROUP_Z:
        {
            int nCurDistance = nDeltaZ;
            nBranchResult = ((nCurDistance <= nDistanceCtrlValue) ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);
        }
        break;
    case KAI_COORDINATE_GROUP_XY:
        {
            int nCurDistance = nDeltaX * nDeltaX + nDeltaY * nDeltaY;
            nBranchResult = ((nCurDistance <= nDistanceCtrlValue * nDistanceCtrlValue) ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);
        }
        break;
    case KAI_COORDINATE_GROUP_XZ:
        {
            int nCurDistance = nDeltaX * nDeltaX + nDeltaZ * nDeltaZ;
            nBranchResult = ((nCurDistance <= nDistanceCtrlValue * nDistanceCtrlValue) ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);
        }
        break;
    case KAI_COORDINATE_GROUP_YZ:
        {
            int nCurDistance = nDeltaY * nDeltaY + nDeltaZ * nDeltaZ;
            nBranchResult = ((nCurDistance <= nDistanceCtrlValue * nDistanceCtrlValue) ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);
        }
        break;
    case KAI_COORDINATE_GROUP_XYZ:
        {
            int nCurDistance = nDeltaX * nDeltaX + nDeltaY * nDeltaY + nDeltaZ * nDeltaZ;
            nBranchResult = ((nCurDistance <= nDistanceCtrlValue * nDistanceCtrlValue) ? KAI_BRANCH_SUCCESS : KAI_BRANCH_FAILED);
        }
        break;
    case KAI_COORDINATE_GROUP_SEPERATEXYZ:
        {
            if (nDeltaX <= nDistanceCtrlValueX &&
                nDeltaY <= nDistanceCtrlValueY &&
                nDeltaZ <= nDistanceCtrlValueZ)
            {
                nBranchResult = KAI_BRANCH_SUCCESS;
            }
            else
            {
                nBranchResult = KAI_BRANCH_FAILED;
            }
            
        }
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Invalid param(%d) in %s", eType, __FUNCTION__);
        goto Exit0;
        break;
    }
    
Exit0:
    return nBranchResult;
}

int AISetRandomTimer(KHero* pHero, KAIAction* pActionData)
{
    KAI_TIMER eTimerType = KAI_TIMER_PRIMARY;
    int nMinInterval = 0;
    int nMaxInterval = 0;
    int nInvterval = 0;
   
    eTimerType = (KAI_TIMER)pActionData->m_nParam[0];
    if (eTimerType < KAI_TIMER_PRIMARY || eTimerType > KAI_TIMER_THIRD)
    {
        eTimerType = KAI_TIMER_PRIMARY;
    }    

    nMinInterval = abs(pActionData->m_nParam[1]);
    MakeInRange(nMinInterval, MIN_TIMER_INTERVAL, INT_MAX);
    
    nMaxInterval = abs(pActionData->m_nParam[2]);
    MakeInRange(nMaxInterval, MIN_TIMER_INTERVAL, INT_MAX);

    if (nMinInterval > nMaxInterval)
        swap(nMinInterval, nMaxInterval);

    nInvterval = nMinInterval + g_Random(nMaxInterval - nMinInterval);

    pHero->m_AIVM.SetPrimaryTimer(nInvterval);    

    return KAI_BRANCH_SUCCESS;
}

int AIHasEnemyNearBy(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    int nDeltaX = pActionData->m_nParam[0];
    int nDeltaY = pActionData->m_nParam[1];
    int nDeltaZ = pActionData->m_nParam[2];
    int nEnemyFilter = pActionData->m_nParam[3];

    for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;

        KHero* pTarget = (KHero*)pObj;
        if (pTarget->m_nSide == pHero->m_nSide)
            continue;

        if (nEnemyFilter == KAI_ENEMY_FILTER_ONLY_MOVABLE)
        {
            if (pTarget->m_eMoveState == cmsOnKnockedDown ||
                pTarget->m_eMoveState == cmsOnKnockedOff || 
                pTarget->m_eMoveState == cmsOnFreeze)
            {
                continue;
            }
        }

        if (nDeltaX > 0  && (pTarget->m_nX < pHero->m_nX - nDeltaX || pTarget->m_nX > pHero->m_nX + nDeltaX))
            continue;

        if (nDeltaY > 0  && (pTarget->m_nY < pHero->m_nY - nDeltaY || pTarget->m_nY > pHero->m_nY + nDeltaY))
            continue;
        
        if (nDeltaZ > 0  && (pTarget->m_nZ < pHero->m_nZ - nDeltaZ || pTarget->m_nZ > pHero->m_nZ + nDeltaZ))
            continue;
        
        nBranchResult = KAI_BRANCH_SUCCESS;
        break;
    }
    
    return nBranchResult;
}

int AICheckTargetHeroState(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_ERROR;
    KHero* pTarget = NULL;
    KMovableObject* pObj = NULL;

    pObj = pHero->GetAITarget();
    KG_PROCESS_ERROR(pObj);

    KG_PROCESS_ERROR(pObj->Is(sotHero));
    pTarget = (KHero*)pObj;
    
    nBranchResult = KAI_BRANCH_FAILED;

    for (int i = 0; i < KAI_ACTION_PARAM_NUM; ++i)
    {
        if (pTarget->m_eMoveState == pActionData->m_nParam[i])
        {
            nBranchResult = KAI_BRANCH_SUCCESS;
            break;
        }
    }

Exit0:
    return nBranchResult;    
}

int AICheckDeltaScore(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    int nMinScore = 0;
    int nMaxScore = 0;
    int nMyScore = 0;
    int nEnemyScore = 0;
    int nDeltaScore = 0;

    nMinScore = pActionData->m_nParam[0];
    nMaxScore = pActionData->m_nParam[1];
    if (nMinScore > nMaxScore)
    {
        swap(nMinScore, nMaxScore);
    }

    if (pHero->m_nSide == sidLeft)
    {
        nMyScore = pHero->m_pScene->m_nScore[sidLeft];
        nEnemyScore = pHero->m_pScene->m_nScore[sidRight];
    }
    else
    {
        nMyScore = pHero->m_pScene->m_nScore[sidRight];
        nEnemyScore = pHero->m_pScene->m_nScore[sidLeft];
    }
    
    nDeltaScore = nMyScore - nEnemyScore;

    if (nDeltaScore >= nMinScore && nDeltaScore <= nMaxScore)
    {
        nBranchResult = KAI_BRANCH_SUCCESS;
    }

    return nBranchResult;
}

int AICheckHeroProperty(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    int nPropertyValue = 0;
    KAI_HERO_PROPERTY   eProperty    = (KAI_HERO_PROPERTY)pActionData->m_nParam[0];
    KAI_COMPARE         eCompareType = (KAI_COMPARE)pActionData->m_nParam[1];
    int                 nCtrlValue   = pActionData->m_nParam[2];

    switch(eProperty)
    {
    case KAI_HERO_PROPERTY_ENDURANCE:
        nPropertyValue = pHero->m_nCurrentEndurance;
        break;
    case KAI_HERO_PROPERTY_STAMINA:
        nPropertyValue = pHero->m_nCurrentStamina;
        break;
    case KAI_HERO_PROPERTY_ANGRY:
        nPropertyValue = pHero->m_nCurrentAngry;
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    switch(eCompareType)
    {
    case KAI_COMPARE_LESSEQUAL:
        if (nPropertyValue <= nCtrlValue)
        {
            nBranchResult = KAI_BRANCH_SUCCESS;
        }
        
        break;
    case KAI_COMPARE_GREATEREQUAL:
        if (nPropertyValue >= nCtrlValue)
        {
            nBranchResult = KAI_BRANCH_SUCCESS;
        }
        
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }
    
Exit0:    
    return nBranchResult;
}

int AICheckDistanceTo(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    int nCurDistance = 0;
    int nDistanceCtrlValue = pActionData->m_nParam[0];
    KAI_SECOND_TARGET eTarget = (KAI_SECOND_TARGET)pActionData->m_nParam[1];
    
    switch(eTarget)
    {
    case KAI_SECOND_TARGET_BALL:
        {
            KBall* pTarget = pHero->m_pScene->GetBall();
            KG_PROCESS_ERROR(pTarget);
            KG_PROCESS_ERROR(!pTarget->HasGuide());

            nCurDistance = g_GetDistance(pHero, pTarget);
        }
        break;
    case KAI_SECOND_TARGET_NEARER_ENEMY:
        {
            KHero* pTarget = pHero->GetNearerEnemy(false);
            KG_PROCESS_ERROR(pTarget);
            nCurDistance = g_GetDistance(pHero, pTarget);
        }
        break;
    case KAI_SECOND_TARGET_FARER_ENMEY:
        {
            KHero* pTarget = pHero->GetFarerEnemy(false);
            KG_PROCESS_ERROR(pTarget);
            nCurDistance = g_GetDistance(pHero, pTarget);
        }
        break;
    case KAI_SECOND_TARGET_TEAMMATE:
        {
            KHero* pTarget = pHero->GetTeammate();
            KG_PROCESS_ERROR(pTarget);
            nCurDistance = g_GetDistance(pHero, pTarget);
        }
        break;
    case KAI_SECOND_TARGET_NEAREST_DOODAD:
        {
            KSceneObject* pTarget = NULL;
            KSceneObjectType eDoodadType = (KSceneObjectType)pActionData->m_nParam[2];
            int nMinDistance = INT_MAX;
            for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
            {
                KSceneObject* pObj = it.GetValue();
                if(g_IsDoodad(pObj))
                    continue;

                if (eDoodadType != sotInvalid && !pObj->Is(eDoodadType))
                    continue;

                int nDistance = g_GetDistance(pHero, pObj);
                if (nDistance < nMinDistance)
                {
                    pTarget = pObj;
                }
            }
            KG_PROCESS_ERROR(pTarget);
            nCurDistance = g_GetDistance(pHero, pTarget);
        }
        break;
    case KAI_SECOND_TARGET_FAREST_DOODAD:
        {
            KSceneObject* pTarget = NULL;
            KSceneObjectType eDoodadType = (KSceneObjectType)pActionData->m_nParam[2];
            int nMaxDistance = INT_MIN;
            for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
            {
                KSceneObject* pObj = it.GetValue();
                if(g_IsDoodad(pObj))
                    continue;

                if (eDoodadType != sotInvalid && !pObj->Is(eDoodadType))
                    continue;             

                int nDistance = g_GetDistance(pHero, pObj);
                if (nDistance > nMaxDistance)
                {
                    pTarget = pObj;
                }
            }
            KG_PROCESS_ERROR(pTarget);
            nCurDistance = g_GetDistance(pHero, pTarget);
        }
        break;
    case KAI_SECOND_TARGET_BALL_HOLDER:
        {
            KHero* pTarget = pHero->m_pScene->GetBallTaker();
            KG_PROCESS_ERROR(pTarget);
            nCurDistance = g_GetDistance(pHero, pTarget);
        }
        break;
    case KAI_SECOND_TARGET_POSITION:
        {
            int nTargetX = pActionData->m_nParam[2];
            int nTargetY = pActionData->m_nParam[3];
            int nTargetZ = pActionData->m_nParam[4];

            nCurDistance = g_GetDistance3(pHero->m_nX, pHero->m_nY, pHero->m_nZ, nTargetX, nTargetY, nTargetZ);
        }
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Invalid KAI_SECOND_TARGET %d in %s\n", eTarget, __FUNCTION__);
        KG_PROCESS_ERROR(false);
        break;
    }

    if (nCurDistance <= nDistanceCtrlValue)
    {
        nBranchResult = KAI_BRANCH_SUCCESS;
    }
    else
    {
        nBranchResult = KAI_BRANCH_FAILED;
    }
    

Exit0:    
    return nBranchResult;
}

int AICanJumpToTarget(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_ERROR;
    BOOL bRetCode = false;
    int nTargetX = 0;
    int nTargetY = 0;
    int nTargetZ = 0;
    int nFrames = 0;
    int nX = 0;
    int nY = 0;
    int nDeltaX = 0;
    int nDeltaY = 0;
    int nMinDeltaX = 0;
    int nMaxDeltaX = 0;
    int nMinDeltaY = 0;
    int nMaxDeltaY = 0;

    bRetCode = pHero->GetAITargetPos(nTargetX, nTargetY, nTargetZ);
    KG_PROCESS_ERROR(bRetCode);

    KG_PROCESS_ERROR(pHero->m_nJumpSpeed >= 0);

    nBranchResult = KAI_BRANCH_FAILED;

    nFrames = ((int)sqrt(pHero->m_nJumpSpeed * pHero->m_nJumpSpeed + 2.0 * pHero->m_nZ * GAME_FPS)  + pHero->m_nJumpSpeed) / GAME_FPS;

    nX = pHero->m_nX + pHero->m_nVelocityX * nFrames;
    nY = pHero->m_nY + pHero->m_nVelocityY * nFrames;
  
    nDeltaX = nTargetX - nX;
    nDeltaY = nTargetY - nY;

    if (nDeltaX * pHero->m_nVelocityX >= 0)
        nDeltaX = abs(nDeltaX);
    else
        nDeltaX = -abs(nDeltaX);

    if (nDeltaY * pHero->m_nVelocityY >= 0)
        nDeltaY = abs(nDeltaY);
    else
        nDeltaY = -abs(nDeltaY);

    nMinDeltaX = pActionData->m_nParam[0];
    nMaxDeltaX = pActionData->m_nParam[1];
    nMinDeltaY = pActionData->m_nParam[2];
    nMaxDeltaY = pActionData->m_nParam[3];

    KG_PROCESS_ERROR(nMinDeltaX <= nDeltaX && nMaxDeltaX >= nDeltaX);
    KG_PROCESS_ERROR(nMinDeltaY <= nDeltaY && nMaxDeltaY >= nDeltaY);
    
    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AIIsPlayerTeammate(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    KPlayer* pOwner = NULL;

    pOwner = pHero->GetOwner();
    if (pOwner)
        nBranchResult = KAI_BRANCH_SUCCESS;

    return nBranchResult;
}

int AICheckBasketState(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
  
    KAI_BASKET_STATE eState = (KAI_BASKET_STATE)pActionData->m_nParam[0];

    switch (eState)
    {
    case KAI_BASKET_STATE_ALL_IN_SOCKET:
        {
            nBranchResult = KAI_BRANCH_SUCCESS;
            for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
            {
                KSceneObject* pObj = it.GetValue();
                if (!pObj->Is(sotBasket))
                    continue;
                KBasket* pBasket = (KBasket*)pObj;
                if (pBasket->m_pPluginSocket)
                    continue;
                nBranchResult = KAI_BRANCH_FAILED;
                break;
            }      
        }
        break;
    case KAI_BASKET_STATE_HAS_ONE_FREE:
        {
            nBranchResult = KAI_BRANCH_FAILED;
            for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
            {
                KSceneObject* pObj = it.GetValue();
                if (!pObj->Is(sotBasket))
                    continue;
                KBasket* pBasket = (KBasket*)pObj;
                if (pBasket->m_pPluginSocket)
                    continue;
                KMovableObject* pGuide = pBasket->GetGuide();
                if (pGuide)
                    continue;
                nBranchResult = KAI_BRANCH_SUCCESS;
                break;
            }
        }
        break;
    case KAI_BASKET_STATE_HAS_ONE_TOKEN_BY_ENEMY:
        {
            nBranchResult = KAI_BRANCH_FAILED;
            for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
            {
                KSceneObject* pObj = it.GetValue();
                if (!pObj->Is(sotBasket))
                    continue;
                KBasket* pBasket = (KBasket*)pObj;
                if (pBasket->m_pPluginSocket)
                    continue;
                KMoveType eMoveType = pBasket->GetMoveType();
                if (eMoveType != mosToken)
                    continue;
                KMovableObject* pGuide = pBasket->GetGuide();
                if (!pGuide)
                {
                    KGLogPrintf(KGLOG_ERR, "Error, One Basket is in token state but no guide.");
                    continue;
                }
                
                if (!pGuide->Is(sotHero))
                {
                    KGLogPrintf(KGLOG_ERR, "Error, One taking basket is not hero.");
                    continue;
                }

                KHero* pHeroTakingBasket = (KHero*)pGuide;
                KRelationType eRelationType = pHero->GetRelationTypeWith(pHeroTakingBasket);
                if (eRelationType != rltEnemy)
                    continue;
                
                nBranchResult = KAI_BRANCH_SUCCESS;
                break;
            }
        }
        break;
    case KAI_BASKET_STATE_HAS_ONE_TOKEN_BY_TEAMMATE:
        {
            nBranchResult = KAI_BRANCH_FAILED;
            for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
            {
                KSceneObject* pObj = it.GetValue();
                if (!pObj->Is(sotBasket))
                    continue;
                KBasket* pBasket = (KBasket*)pObj;
                if (pBasket->m_pPluginSocket)
                    continue;
                KMoveType eMoveType = pBasket->GetMoveType();
                if (eMoveType != mosToken)
                    continue;
                KMovableObject* pGuide = pBasket->GetGuide();
                if (!pGuide)
                {
                    KGLogPrintf(KGLOG_ERR, "Error, One Basket is in token state but no guide.");
                    continue;
                }

                if (!pGuide->Is(sotHero))
                {
                    KGLogPrintf(KGLOG_ERR, "Error, One taking basket is not hero.");
                    continue;
                }

                KHero* pHeroTakingBasket = (KHero*)pGuide;
                KRelationType eRelationType = pHero->GetRelationTypeWith(pHeroTakingBasket);
                if (eRelationType != rltUs)
                    continue;

                nBranchResult = KAI_BRANCH_SUCCESS;
                break;
            }
        }
        break;
    case KAI_BASKET_STATE_HAS_ONE_TOKEN_BY_SELF:
        {
            nBranchResult = KAI_BRANCH_FAILED;
            for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
            {
                KSceneObject* pObj = it.GetValue();
                if (!pObj->Is(sotBasket))
                    continue;
                KBasket* pBasket = (KBasket*)pObj;
                if (pBasket->m_pPluginSocket)
                    continue;
                KMoveType eMoveType = pBasket->GetMoveType();
                if (eMoveType != mosToken)
                    continue;
                KMovableObject* pGuide = pBasket->GetGuide();
                if (!pGuide)
                {
                    KGLogPrintf(KGLOG_ERR, "Error, One Basket is in token state but no guide.");
                    continue;
                }

                if (!pGuide->Is(sotHero))
                {
                    KGLogPrintf(KGLOG_ERR, "Error, One taking basket is not hero.");
                    continue;
                }

                KHero* pHeroTakingBasket = (KHero*)pGuide;
                KRelationType eRelationType = pHero->GetRelationTypeWith(pHeroTakingBasket);
                if (eRelationType != rltMe)
                    continue;

                nBranchResult = KAI_BRANCH_SUCCESS;
                break;
            }
        }
        break;
    default:
        KGLogPrintf(KGLOG_ERR, "Not Specified AI_BASKET_STATE %d in %s.", eState, __FUNCTION__);
        break;
    }
    
    return nBranchResult;
}

int AICheckHeroCountBySide(KHero* pHero, KAIAction* pActionData)
{
    int     nBranchResult = KAI_BRANCH_FAILED;
    BOOL    bSelfSide   = pActionData->m_nParam[0];
    int     nCount  = pActionData->m_nParam[1];
    KHero*  pTemp   = NULL;
    BOOL    bRetCode = false;
    int     nCurCount = 0;
    
    for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        bRetCode = pObj->Is(sotHero);
        if (!bRetCode)
            continue;

        pTemp = (KHero*)pObj;
        if (!bSelfSide && pTemp->m_nSide != pHero->m_nSide)
            ++nCurCount;
        else if (bSelfSide && pTemp->m_nSide == pHero->m_nSide)        
            ++nCurCount;
    }

    if (nCurCount == nCount)
        nBranchResult = KAI_BRANCH_SUCCESS;
    
    return nBranchResult;
}

int AICheckBasketInBackboard(KHero* pHero, KAIAction* pActionData)
{
    int                 nBranchResult   = KAI_BRANCH_FAILED;
    BOOL                bRetCode        = false;
    BOOL                bSelfSide       = pActionData->m_nParam[0];
    int                 nBackboardFloor = pActionData->m_nParam[1];
    int                 nCurStatus      = 0;
    KBasketSocketFloor  validFloors[2]  = {bfInvalid};
    KBasketSocket*      validSocket[countof(validFloors)] = {NULL};

    validFloors[0] = (KBasketSocketFloor)(nBackboardFloor * 2 - 1);
    validFloors[1] = (KBasketSocketFloor)(nBackboardFloor * 2);

    for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        bRetCode = pObj->Is(sotBasketSocket);
        if (!bRetCode)
            continue;

        KBasketSocket* pSocket = (KBasketSocket*)pObj;
        if (!pSocket->m_pBasket)
            continue;

        KBasketSocketFloor* pFloor = std::find(validFloors, validFloors + countof(validFloors), pSocket->m_eFloor);
        if (pFloor == validFloors + countof(validFloors))
            continue;

        if (bSelfSide && pSocket->m_nSide == pHero->m_nSide)
            continue;

        if (!bSelfSide && pSocket->m_nSide != pHero->m_nSide)
            continue;
        
        validSocket[pFloor - validFloors] = pSocket;
    }
    
    for (int i = 0; i < countof(validSocket); ++i)
    {
        if (!validSocket[i])
            continue;
        
        nCurStatus += (1 << i);
    }

    for (int i = 2; i < countof(pActionData->m_nParam); ++i)
    {
        if (nCurStatus == pActionData->m_nParam[i])
        {
            nBranchResult = KAI_BRANCH_SUCCESS;
            break;
        }      
    }

    return nBranchResult;
}

int AICanSkillHit(KHero* pHero, KAIAction* pActionData)
{
    int                 nBranchResult       = KAI_BRANCH_FAILED;
    BOOL                bRetCode            = false;
    int                 nSlotIndex          = pActionData->m_nParam[0];
    DWORD               dwSkillID           = ERROR_ID;
    KSkill*             pSkill              = NULL;
    KMovableObject*     pTarget             = NULL;
    KBODY               cAttackBody;
    KBODY               cTargetBody;

    KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHero->m_ActiveSkill));

    dwSkillID = pHero->m_ActiveSkill[nSlotIndex];

    pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID);
    KGLOG_PROCESS_ERROR(pSkill);

    bRetCode = pSkill->GetJudageBox(pHero, cAttackBody);
    KGLOG_PROCESS_ERROR(bRetCode);

    pTarget = pHero->GetAITarget();
    KGLOG_PROCESS_ERROR(pTarget);
    cTargetBody = pTarget->GetBody();

    bRetCode = g_IsCollide(cAttackBody, cTargetBody);
    KG_PROCESS_ERROR(bRetCode);
    
    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AIUseItem(KHero* pHero, KAIAction* pActionData)
{
    int         nBranchResult  = KAI_BRANCH_FAILED;
    BOOL        bRetCode       = false;
    DWORD       dwTabType      = (DWORD)pActionData->m_nParam[0];
    DWORD       dwIndex        = (DWORD)pActionData->m_nParam[1];
    KGOtherExtInfo* pOtherExtInfo   = NULL;
    KGItemInfo*     pItemInfo       = NULL;
    KPlayer*        pPlayer         = NULL;
    
    pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(dwTabType, dwIndex);
    KGLOG_PROCESS_ERROR(pItemInfo);

    pOtherExtInfo = pItemInfo->pOtherExtInfo;
    KGLOG_PROCESS_ERROR(pOtherExtInfo);

    for (int i = 0; i < countof(pOtherExtInfo->dwHeroBuffID); ++i)
    {
        DWORD dwBuffID = pOtherExtInfo->dwHeroBuffID[i];

        if (!dwBuffID)
            continue;

        bRetCode = pHero->AddBuff(dwBuffID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    pPlayer = pHero->GetOwner();
    if (pPlayer)
    {
        for (int i = 0; i < countof(pOtherExtInfo->dwPlayerBuffID); ++i)
        {
            uint32_t dwBuffID = pOtherExtInfo->dwPlayerBuffID[i];

            if (dwBuffID == 0)
                continue;
            
            bRetCode = pPlayer->AddBuff(dwBuffID);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AICheckStandOn(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    BOOL bSelf = pActionData->m_nParam[0];
    KMovableObject* pCheckObj = NULL;

    if (bSelf)
        pCheckObj = pHero;
    else
        pCheckObj = pHero->GetAITarget();
    KG_PROCESS_ERROR(pCheckObj);

    if (pCheckObj->GetMoveType() == mosStandOn)
        nBranchResult = KAI_BRANCH_SUCCESS;
 
Exit0:
    return nBranchResult;
}

int AICheckHolding(KHero* pHero, KAIAction* pActionData)
{
    int         nBranchResult   = KAI_BRANCH_ERROR;
    BOOL        bRetCode        = false;
    KDoodad*    pDoodad         = NULL;
    int         nType           = 0;
    pDoodad = pHero->GetHoldingDoodad();
    KG_PROCESS_ERROR(pDoodad);

    nBranchResult = KAI_BRANCH_FAILED;

    nType = pDoodad->GetType();
    for (int i = 0; i < countof(pActionData->m_nParam); ++i)
    {
        if (pActionData->m_nParam[i] == -1)
            continue;
        
        if (pActionData->m_nParam[i] != 0 && nType != pActionData->m_nParam[i])
            continue;

        nBranchResult = KAI_BRANCH_SUCCESS;
        break;
    }
    
Exit0:
    return nBranchResult;
}

int AICheckTeammateType(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    KAI_TEAMMATE_TYPE eTeammateType = KAI_TEAMMATE_TYPE_INVALID;
    KHero* pTeammate = pHero->GetTeammate();

    if (!pTeammate)
        eTeammateType = KAI_TEAMMATE_TYPE_NOTEAMMATE;
    else if (pTeammate->IsMainHero())
        eTeammateType = KAI_TEAMMATE_TYPE_PLAYER;
    else if (pTeammate->IsAssistHero())
        eTeammateType = KAI_TEAMMATE_TYPE_AITEAMMATE;
    else
        eTeammateType = KAI_TEAMMATE_TYPE_NPC;
    
    for (int i = 0; i < countof(pActionData->m_nParam); ++i)
    {
        if (pActionData->m_nParam[i] != eTeammateType)
            continue;

        nBranchResult = KAI_BRANCH_SUCCESS;
        break;
    }
    
    return nBranchResult;
}

int AICheckCanSkillShoot(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    BOOL bRetCode = false;
    BOOL bSelf = pActionData->m_nParam[0];
    KPOSITION pos = pHero->GetPosition();

    if (bSelf && pHero->m_nSide == sidLeft ||
        !bSelf && pHero->m_nSide == sidRight)
    {
        if (pos.nX >= 12 * 256)
        {
            nBranchResult = KAI_BRANCH_SUCCESS;
        }
    }
    else
    {
        int nSceneXLength = 0;
        KG_PROCESS_ERROR(pHero->m_pScene);

        nSceneXLength = pHero->m_pScene->GetLength();

        if (pos.nX <= nSceneXLength - 12 * 256)
        {
            nBranchResult = KAI_BRANCH_SUCCESS;
        }
    }
    
Exit0:
    return nBranchResult;
}

int AICheckBattleStartTime(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult      = KAI_BRANCH_FAILED;
    int nBattleStartedTime = 0;
    int nLower             = pActionData->m_nParam[0] * GAME_FPS;
    int nUpper             = pActionData->m_nParam[1] * GAME_FPS;

    KG_PROCESS_ERROR(pHero->m_pScene);
    KG_PROCESS_ERROR(pHero->m_pScene->m_eSceneState == ssFighting);
    nBattleStartedTime = pHero->m_pScene->m_Battle.GetStartedFrame();

    if (nBattleStartedTime >= nLower && nBattleStartedTime <= nUpper)
        nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AIHasDoodadNearBy(KHero* pHero, KAIAction* pActionData)
{
    int  nBranchResult  = KAI_BRANCH_FAILED;
    BOOL bRetCode       = false;
    int  nRange         = pActionData->m_nParam[0];
    int  nDistance      = 0;

    for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        bRetCode = g_IsDoodad(pObj);
        if (!bRetCode)
            continue;

        KDoodad* pDoodad = (KDoodad*)pObj;
        if (pDoodad->GetMoveType() != mosIdle)
            continue;

        if (pDoodad->m_bAttackState)
            continue;

        nDistance = g_GetDistance3(pHero->m_nX, pHero->m_nY, pHero->m_nZ, pObj->m_nX, pObj->m_nY, pObj->m_nZ);
        if (nDistance > nRange * nRange)
            continue;

        int nType = pObj->GetType();
        for (int i = 1; i < countof(pActionData->m_nParam); ++i)
        {
            if (pActionData->m_nParam[i] == -1)
                continue;
            if (pActionData->m_nParam[i] != 0 && nType != pActionData->m_nParam[i])
                continue;

            nBranchResult = KAI_BRANCH_SUCCESS;
            goto Exit0;
        }
    }
    
Exit0:
    return nBranchResult;
}

int AIGoAround(KHero* pHero, KAIAction* pActionData)
{
    const static DWORD GO_AROUND_UP_SKILL_ID = 9;
    const static DWORD GO_AROUND_DOWN_SKILL_ID = 10;

    int  nBranchResult  = KAI_BRANCH_FAILED;
    BOOL bRetCode       = false;
    BOOL bGoAroundUp    = pActionData->m_nParam[0];
    DWORD dwSkillID     = GO_AROUND_DOWN_SKILL_ID;

    KG_PROCESS_ERROR(pHero->m_eMoveState == cmsOnRun);

    if (bGoAroundUp)
        dwSkillID = GO_AROUND_UP_SKILL_ID;

    bRetCode = pHero->CastSkill(dwSkillID, 0, pHero->m_eFaceDir);
    KG_PROCESS_ERROR(bRetCode);
    
    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AICheckPositionRelationXY(KHero* pHero, KAIAction* pActionData)
{
	int nBranchResult = KAI_BRANCH_FAILED;
	BOOL bRetCode = false;
    int nX = 0;
    int nY = 0;
    int nZ = 0;
    int nDeltaX = 0;
    int nDeltaY = 0;
    int nMinDeltaX = pActionData->m_nParam[0];
    int nMaxDeltaX = pActionData->m_nParam[1];
    int nMinDeltaY = pActionData->m_nParam[2];
    int nMaxDeltaY = pActionData->m_nParam[3];

    bRetCode = pHero->GetAITargetPos(nX, nY, nZ);
    KGLOG_PROCESS_ERROR(bRetCode);

    nDeltaX = nX - pHero->m_nX;
    nDeltaY = nY - pHero->m_nY;

    if (nDeltaX < nMinDeltaX || nDeltaX > nMaxDeltaX)
        goto Exit0;

    if (nDeltaY < nMinDeltaY || nDeltaY > nMaxDeltaY)
        goto Exit0;    


	nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
	return nBranchResult;
}

int AIIsAtEdge(KHero* pHero, KAIAction* pActionData)
{
	int nBranchResult   = KAI_BRANCH_FAILED;
	BOOL bRetCode       = false;
    int nDeltaXMinus    = pActionData->m_nParam[0];
    int nDeltaXPlus     = pActionData->m_nParam[1];
    int nDeltaYMinus    = pActionData->m_nParam[2];
    int nDeltaYPlus     = pActionData->m_nParam[3];

    if (nDeltaXMinus > 0 && pHero->m_nX <= nDeltaXMinus)
        goto Exit1;

    if (nDeltaXPlus > 0 && pHero->m_nX >= pHero->m_pScene->GetLength() - nDeltaXPlus)
        goto Exit1;

    if (nDeltaYMinus > 0 && pHero->m_nY <= nDeltaYMinus)
        goto Exit1;

    if (nDeltaYPlus > 0 && pHero->m_nY >= pHero->m_pScene->GetWidth() - nDeltaYPlus)
        goto Exit1;

    goto Exit0;

Exit1:
	nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
	return nBranchResult;
}

int AIAutoMove(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult   = KAI_BRANCH_FAILED;
    BOOL bRetCode = false;
    BOOL bWalk = pActionData->m_nParam[0];
    int nAroundX = pActionData->m_nParam[1];
    int nAroundY = pActionData->m_nParam[2];

    KGLOG_PROCESS_ERROR(nAroundX > 0);
    KGLOG_PROCESS_ERROR(nAroundY > 0);

    bRetCode = pHero->UpdateAutoMoveParam(nAroundX, nAroundY, bWalk);
    KG_PROCESS_ERROR(bRetCode);

    bRetCode = pHero->DirectlyMoveToDest();
    KG_PROCESS_ERROR(bRetCode);
   
    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AIStopAutoMove(KHero* pHero, KAIAction* pActionData)
{
    pHero->StopAutoMove();
    return KAI_BRANCH_SUCCESS;
}

int AIIsCastingSkill(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    
    if (pHero->m_pCastingSkill)
        nBranchResult = KAI_BRANCH_SUCCESS;

    return nBranchResult;
}

int AIMove(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    BOOL bRetCode = false;
    BOOL bWalk = false;
    int nDirectionType = 0;
    int nParam = 0;
    int nDirection = 0;

    bWalk = pActionData->m_nParam[0];
    nDirectionType = pActionData->m_nParam[1];
    nParam = pActionData->m_nParam[2];

    bRetCode = pHero->GetDirectionByType(nDirectionType, nDirection, nParam);
    KGLOG_PROCESS_ERROR(bRetCode);
    if (bWalk)
    {
        pHero->ApplyMoveAction((BYTE)cmsOnWalk, (BYTE)nDirection);
        KGLOG_PROCESS_ERROR(pHero->m_eMoveState == cmsOnWalk);
    }
    else
    {
        pHero->ApplyMoveAction((BYTE)cmsOnRun, (BYTE)nDirection);
        KGLOG_PROCESS_ERROR(pHero->m_eMoveState == cmsOnRun);
    }

    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AIHasSlamBallSkill(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    int nSlotIndex = pActionData->m_nParam[0];

    KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHero->m_SlamBallSkill));

    if (pHero->m_SlamBallSkill[nSlotIndex])
        nBranchResult = KAI_BRANCH_SUCCESS;

Exit0:
    return nBranchResult;
}

int AIHasNirvanaSkill(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    int nSlotIndex = pActionData->m_nParam[0];
    
    KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHero->m_NirvanaSkill));

    if (pHero->m_NirvanaSkill[nSlotIndex])
        nBranchResult = KAI_BRANCH_SUCCESS;

Exit0:
    return nBranchResult;
}

int AIHasTeammate(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;

    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    for (KObjEnumerator it = pHero->m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pSceneObj = it.GetValue();
        if (!pSceneObj->Is(sotHero))
            continue;

        if(pSceneObj->m_dwID == pHero->m_dwID)
            continue;

        KHero* pHeroObj = (KHero*)pSceneObj;
        if (pHeroObj->m_nSide != pHero->m_nSide)
            continue;

        nBranchResult = KAI_BRANCH_SUCCESS;
        break;
    }
    
Exit0:
    return nBranchResult;
}

int AICanSkillSlamBasketLevel(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    BOOL bRetCode = false;
    DWORD dwSkillID = 0;
    int nBasketLevel = pActionData->m_nParam[0];
    KSlamBallSkill* pSlamBallSkill  = NULL;

    KGLOG_PROCESS_ERROR(pHero->m_pScene);

    if (pHero->m_SlamBallSkill[0] == 0)
    {
        nBranchResult = KAI_BRANCH_ERROR;
        goto Exit0;
    }

    dwSkillID = pHero->m_SlamBallSkill[0];
    pSlamBallSkill = g_pSO3World->m_Settings.m_SlamBallSkillSettings.Get(dwSkillID);
    KGLOG_PROCESS_ERROR(pSlamBallSkill);

    if (nBasketLevel > pSlamBallSkill->nHighestTargetBasket)
    {
        nBranchResult = KAI_BRANCH_FAILED;
        goto Exit0;
    }
    
    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AISpecialUseDoodad(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_FAILED;
    BOOL bRetCode = false;

    bRetCode = pHero->SpecialUseDoodad();
    KGLOG_PROCESS_ERROR(bRetCode);

    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AICheckSkillSeries(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult = KAI_BRANCH_ERROR;
    int nSkillGroup = pActionData->m_nParam[0];
    int nSlotIndex = pActionData->m_nParam[1];
    int nSeries = pActionData->m_nParam[2];

    switch(nSkillGroup)
    {
    case KAI_SKILL_GROUP_ACTIVE:
        {     
            KSkill* pSkill = NULL;
            DWORD dwSkillID = ERROR_ID;

            KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHero->m_ActiveSkill));
            
            dwSkillID = pHero->m_ActiveSkill[nSlotIndex];
            KG_PROCESS_ERROR(dwSkillID);

            pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID);
            KGLOG_PROCESS_ERROR(pSkill);

            nBranchResult = KAI_BRANCH_FAILED;
            KG_PROCESS_ERROR (pSkill->m_nSeries == nSeries);
            nBranchResult = KAI_BRANCH_SUCCESS;
        }
        break;
    case KAI_SKILL_GROUP_PASSIVE:
        break;
    case KAI_SKILL_GROUP_NIRVANA:
        {
            KSkill* pSkill = NULL;
            DWORD dwSkillID = ERROR_ID;
     
            KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHero->m_NirvanaSkill));

            dwSkillID = pHero->m_NirvanaSkill[nSlotIndex];
            KG_PROCESS_ERROR(dwSkillID);

            pSkill = g_pSO3World->m_SkillManager.GetSkill(dwSkillID);
            KGLOG_PROCESS_ERROR(pSkill);

            nBranchResult = KAI_BRANCH_FAILED;
            KG_PROCESS_ERROR (pSkill->m_nSeries == nSeries);
            nBranchResult = KAI_BRANCH_SUCCESS;
        }
        break;
    case KAI_SKILL_GROUP_SLAMBALL:
        break;
    }

Exit0:
    return nBranchResult;
}

int AICastNirvanaSkill(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult   = KAI_BRANCH_FAILED;
    BOOL bRetCode       = false;
    DWORD dwSkillID     = ERROR_ID;
    int nSlotIndex      = pActionData->m_nParam[0];

    KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHero->m_NirvanaSkill));

    dwSkillID = pHero->m_NirvanaSkill[nSlotIndex];
    KGLOG_PROCESS_ERROR(dwSkillID);
    
    bRetCode = pHero->CastSkill(dwSkillID, 0, pHero->m_eFaceDir);
    KG_PROCESS_ERROR(bRetCode);

    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}

int AICastSlamBallSkill(KHero* pHero, KAIAction* pActionData)
{
    int nBranchResult   = KAI_BRANCH_FAILED;
    int nSlotIndex      = pActionData->m_nParam[0];
    BOOL bRetCode       = false;
    DWORD dwSkillID     = ERROR_ID;
    DWORD dwInterferenceID = ERROR_ID;
    KBasketSocket* pTargetSocket = NULL;

    KGLOG_PROCESS_ERROR(nSlotIndex >= 0 && nSlotIndex < countof(pHero->m_SlamBallSkill));

    dwSkillID = pHero->m_SlamBallSkill[nSlotIndex];
    KG_PROCESS_ERROR(dwSkillID);

    dwInterferenceID = pHero->GetInterferenceID();

    pTargetSocket = pHero->m_pScene->GetSlamBallTargetSocket(dwSkillID, pHero->m_eFaceDir);
    KG_PROCESS_ERROR(pTargetSocket);

    bRetCode = pHero->CastSlamBallSkill(dwSkillID, pTargetSocket->m_dwID, dwInterferenceID);
    KGLOG_PROCESS_ERROR(bRetCode);

    nBranchResult = KAI_BRANCH_SUCCESS;
Exit0:
    return nBranchResult;
}


void KAIManager::RegisterActionFunctions()
{
    memset(m_ActionFunctionTable, 0, sizeof(m_ActionFunctionTable));

    REGISTER_AI_ACTION_FUNC(SetState);
    REGISTER_AI_ACTION_FUNC(SetPrimaryTimer);    
    REGISTER_AI_ACTION_FUNC(SetSecondaryTimer);
    REGISTER_AI_ACTION_FUNC(SetTertiaryTimer);
    REGISTER_AI_ACTION_FUNC(RandomBiBranch);
    REGISTER_AI_ACTION_FUNC(RandomTriBranch);
    REGISTER_AI_ACTION_FUNC(Stand);
    REGISTER_AI_ACTION_FUNC(Jump);
    REGISTER_AI_ACTION_FUNC(SetMoveTarget);
    REGISTER_AI_ACTION_FUNC(MoveToTarget);
    REGISTER_AI_ACTION_FUNC(CheckNearTargetA);
    REGISTER_AI_ACTION_FUNC(TakeBall);
    REGISTER_AI_ACTION_FUNC(AimAt);
    REGISTER_AI_ACTION_FUNC(CheckAimFloor);
    REGISTER_AI_ACTION_FUNC(ShootBall);
    REGISTER_AI_ACTION_FUNC(CheckFollower);
    REGISTER_AI_ACTION_FUNC(CheckBallTaker);
    REGISTER_AI_ACTION_FUNC(CastSkill);
    REGISTER_AI_ACTION_FUNC(NoneOp);
    REGISTER_AI_ACTION_FUNC(CheckHeroState);
    REGISTER_AI_ACTION_FUNC(TakeDoodad);
    REGISTER_AI_ACTION_FUNC(UseDoodad);
    REGISTER_AI_ACTION_FUNC(ShootDoodad);
    REGISTER_AI_ACTION_FUNC(DropDoodad);
    REGISTER_AI_ACTION_FUNC(PassBall);
    REGISTER_AI_ACTION_FUNC(ChangeDirection);
    REGISTER_AI_ACTION_FUNC(CheckBallMoveType);
    REGISTER_AI_ACTION_FUNC(Log);
    REGISTER_AI_ACTION_FUNC(IsApproachingTarget);
    REGISTER_AI_ACTION_FUNC(IsTargetInFront);
    REGISTER_AI_ACTION_FUNC(CheckDistanceToTarget);
    REGISTER_AI_ACTION_FUNC(SetRandomTimer);
    REGISTER_AI_ACTION_FUNC(HasEnemyNearBy);
    REGISTER_AI_ACTION_FUNC(CheckTargetHeroState);
    REGISTER_AI_ACTION_FUNC(CheckDeltaScore);
    REGISTER_AI_ACTION_FUNC(CheckHeroProperty);
    REGISTER_AI_ACTION_FUNC(CanJumpToTarget);
    REGISTER_AI_ACTION_FUNC(IsPlayerTeammate);
    REGISTER_AI_ACTION_FUNC(CheckBasketState);
    REGISTER_AI_ACTION_FUNC(CheckHeroCountBySide);
    REGISTER_AI_ACTION_FUNC(CheckBasketInBackboard);
    REGISTER_AI_ACTION_FUNC(CanSkillHit);
    REGISTER_AI_ACTION_FUNC(UseItem);
    REGISTER_AI_ACTION_FUNC(CheckStandOn);
    REGISTER_AI_ACTION_FUNC(CheckHolding);
    REGISTER_AI_ACTION_FUNC(CheckTeammateType);
    REGISTER_AI_ACTION_FUNC(CheckCanSkillShoot);
    REGISTER_AI_ACTION_FUNC(CheckBattleStartTime);
    REGISTER_AI_ACTION_FUNC(HasDoodadNearBy);
    REGISTER_AI_ACTION_FUNC(GoAround);
    REGISTER_AI_ACTION_FUNC(CheckPositionRelationXY);
    REGISTER_AI_ACTION_FUNC(IsAtEdge);
    REGISTER_AI_ACTION_FUNC(AutoMove);
    REGISTER_AI_ACTION_FUNC(StopAutoMove);
    REGISTER_AI_ACTION_FUNC(IsCastingSkill);
    REGISTER_AI_ACTION_FUNC(Move);
    REGISTER_AI_ACTION_FUNC(HasSlamBallSkill);
    REGISTER_AI_ACTION_FUNC(HasNirvanaSkill);
    REGISTER_AI_ACTION_FUNC(HasTeammate);
    REGISTER_AI_ACTION_FUNC(CanSkillSlamBasketLevel);
    REGISTER_AI_ACTION_FUNC(SpecialUseDoodad);
    REGISTER_AI_ACTION_FUNC(CheckSkillSeries);
    REGISTER_AI_ACTION_FUNC(CastNirvanaSkill);
    REGISTER_AI_ACTION_FUNC(CastSlamBallSkill);
}

