#include "stdafx.h"
#include "KHero.h"
#include "KScene.h"
#include "Luna.h"
#include "KPlayerServer.h"
#include "KPlayer.h"
#include "KBullet.h"

int KHero::LuaJump(Lua_State* L)
{
    BOOL        bRetCode        = false;
    int         nTopIndex       = 0;
    BOOL        bMove           = false;
    int         nDirection      = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);

    bMove = (BOOL)Lua_ValueToNumber(L, 1);
    nDirection = (int)Lua_ValueToNumber(L, 2);

    Jump(bMove, nDirection, true);

Exit0:
    return 0;
}

int KHero::LuaSetMoveTarget(Lua_State* L)
{
    BOOL        bRetCode        = false;
    int         nTopIndex       = 0;
    BOOL        bMove           = false;
    int         nDirection      = 0;

    nTopIndex = Lua_GetTopIndex(L);
    if (nTopIndex == 1)
    {
        m_AIData.eTargetType = KTARGET_OBJ;
        m_AIData.dwTargetObjID = (DWORD)Lua_ValueToNumber(L, 1);
    }
    else if(nTopIndex == 3)
    {
        m_AIData.eTargetType = KTARGET_POSITION;
        m_AIData.nTargetX = (DWORD)Lua_ValueToNumber(L, 1);
        m_AIData.nTargetY = (DWORD)Lua_ValueToNumber(L, 2);
        m_AIData.nTargetZ = (DWORD)Lua_ValueToNumber(L, 3);

        MAKE_IN_RANGE(m_AIData.nTargetX, 0, m_pScene->GetLength());
        MAKE_IN_RANGE(m_AIData.nTargetY, 0, m_pScene->GetWidth());
    }

    return 0;
}

int KHero::LuaMoveToTarget(Lua_State* L)
{
    BOOL bRetCode = false;
    int  nDestX = 0;
    int  nDestY = 0;
    int  nDestZ = 0;
    int  nDirection = 0;
    BOOL bWalk = false;
    int nTopIndex = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KG_PROCESS_ERROR(nTopIndex == 1);

    bWalk = Lua_ValueToBoolean(L, 1);

    bRetCode = GetAITargetPos(nDestX, nDestY, nDestZ);
    KG_PROCESS_ERROR(bRetCode);

    nDirection  = g_GetDirection(m_nX, m_nY, nDestX, nDestY);
    if (bWalk)
        bRetCode    = WalkTo(nDirection, true);   
    else
        bRetCode    = RunTo(nDirection, true);
Exit0:
    return 0;
}

int KHero::LuaSwitchAIMode(Lua_State* L)
{
    BOOL bSuccess = false;
    BOOL bAiMode = false;
    int nTopIndex = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KG_PROCESS_ERROR(nTopIndex == 1);

    bAiMode = Lua_ValueToBoolean(L, 1);

    bSuccess = bAiMode ? TurnOnAI() : TurnOffAI();
Exit0:
    Lua_PushBoolean(L, bSuccess);
    return 1;
}

int KHero::LuaGetScene(Lua_State* L)
{
    int nRet = 0;

    KG_PROCESS_ERROR(m_pScene != NULL);

    nRet = m_pScene->LuaGetObj(L);

Exit0:
    return nRet;
}

int KHero::LuaSetAIDebug(Lua_State* L)
{
    int nRetCode    = 0;
    int nDebugCount = 0;

    nRetCode = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nRetCode == 1);

    nDebugCount = (int)Lua_ValueToNumber(L, 1);

    m_AIVM.m_nDebugCount = nDebugCount;
    m_AIVM.DebugAICurrentStateInfo();

Exit0:
    return 0;
}

int KHero::LuaSetAIMode(Lua_State* L)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    BOOL            bAIMode     = false;
    int             nTopIndex   = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    bAIMode  = (BOOL)Lua_ValueToNumber(L, 1);

    if (bAIMode)
        TurnOnAI();
    else
        TurnOffAI();

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KHero::LuaAddBuff(Lua_State* L)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    DWORD           dwBuffID    = ERROR_ID;
    int             nTopIndex   = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwBuffID  = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = AddBuff(dwBuffID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KHero::LuaDelBuff(Lua_State* L)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    DWORD           dwBuffID    = ERROR_ID;
    int             nTopIndex   = Lua_GetTopIndex(L);

    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwBuffID  = (DWORD)Lua_ValueToNumber(L, 1);

    bRetCode = m_BuffList.DelBuff(dwBuffID);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KHero::LuaIsMainHero(Lua_State* L)
{
    BOOL bResult = IsMainHero();
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KHero::LuaIsAssistHero(Lua_State* L)
{
    BOOL bResult = IsAssistHero();
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KHero::LuaIsNpc(Lua_State* L)
{
    BOOL bResult = IsNpc();
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KHero::LuaGetOwner(Lua_State* L)
{
    KPlayer* pPlayer = GetOwner();
    if (pPlayer)
    {
        return pPlayer->LuaGetObj(L);
    }

    return 0;
}

int KHero::LuaIsTakingBall(Lua_State* L)
{
    BOOL bResult = false;
    bResult = HasFollowerWithStateAndType(mosToken, sotBall);
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KHero::LuaTakeBall(Lua_State* L)
{
    int nTopIndex = 0;
    int bNotifyClient = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    bNotifyClient = (BOOL)Lua_ValueToBoolean(L, 1);

    TakeBall(bNotifyClient);

Exit0:
    return 0;
}

int KHero::LuaUnTakeBall(Lua_State* L)
{
    int nTopIndex = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 0);
    EnsureUntakeBall();

Exit0:
    return 0;

}

int KHero::LuaPassBall(Lua_State* L)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    KHero*  pReceiver   = NULL;

    bRetCode = HasFollowerWithStateAndType(mosToken, sotBall);
    KG_PROCESS_ERROR(bRetCode);

    pReceiver = GetOneTeammate();
    KG_PROCESS_ERROR(pReceiver);

    bRetCode = PassBallTo(pReceiver, false);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KHero::LuaAimAt(Lua_State* L)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    int  nTopIndex  = 0;
    BOOL bAimAt     = false;
    int  nDirection = 0;
    KDoodad* pDoodad = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 2);
    
    bAimAt = (BOOL)Lua_ValueToNumber(L, 1);
    nDirection = (int)Lua_ValueToNumber(L, 2);

    pDoodad = GetHoldingDoodad();
    KGLOG_PROCESS_ERROR(pDoodad);

    bRetCode = AimAt(bAimAt, nDirection, pDoodad->m_dwID, true);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KHero::LuaShootBall(Lua_State* L)
{
    BOOL    bResult                 = false;
    BOOL    bRetCode                = false;
    DWORD   dwInterferenceHeroID    = ERROR_ID;
    int     nAimLoop                = 0;
    
    bRetCode = HasFollowerWithStateAndType(mosToken, sotBall);
    KGLOG_PROCESS_ERROR(bRetCode);

    dwInterferenceHeroID = GetInterferenceID();
    nAimLoop = m_nVirtualFrame - m_nLastAimTime;

    bRetCode = NormalShootBall(nAimLoop, dwInterferenceHeroID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KHero::LuaSlamBall(Lua_State* L)
{
    int  nTopIndex  = 0;
    int  nFloor     = 0;
    KBasketSocket*  pTargetBasketSocket = NULL;
    
    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nFloor = (int)Lua_ValueToNumber(L, 1);
    
    pTargetBasketSocket = m_pScene->GetSkillSlamBallTargetBasketSocket(m_eFaceDir, nFloor);
    KGLOG_PROCESS_ERROR(pTargetBasketSocket);
    
    CastSlamBallSkill(10001, pTargetBasketSocket->m_dwID, 0);

Exit0:
    return 0;
}

int KHero::LuaLoseStamina(Lua_State* L)
{
    BOOL bResult  = false;
    int nTopIndex = 0;
    int nLoseStamina = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KG_PROCESS_ERROR(nTopIndex == 1);

    nLoseStamina = (int)Lua_ValueToNumber(L, 1);

    LoseStamina(nLoseStamina);

    bResult = true;
Exit0:
    Lua_PushNumber(L, bResult);
    return 1;
}

int KHero::LuaReleaseRelationWithOther(Lua_State* L)
{
    EnsureUntakeBall();
    EnsureNoAttached();
    EnsureNoFollows();
    EnsureNoGuide();

    return 0;
}

int KHero::LuaStand(Lua_State* L)
{
    Stand(true);
    return 0;
}

int KHero::LuaSit(Lua_State* L)
{
    Sit();
    return 0;
}

int KHero::LuaFireBullet(Lua_State* L)
{
    int      nTopIndex    = 0;
    DWORD    dwTemplateID = ERROR_ID;
    KBullet* pBullet      = NULL;
    DWORD    dwBulletID   = ERROR_ID;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1 || nTopIndex == 4 || nTopIndex == 7);

    dwTemplateID = (DWORD)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(dwTemplateID);

    pBullet = FireBullet(dwTemplateID);
    KGLOG_PROCESS_ERROR(pBullet);

    if (nTopIndex >= 4)
    {
        int nX = (int)Lua_ValueToNumber(L, 2);
        int nY = (int)Lua_ValueToNumber(L, 3);
        int nZ = (int)Lua_ValueToNumber(L, 4);
        pBullet->SetPosition(nX, nY, nZ);
    }

    if (nTopIndex == 7)
    {
        pBullet->m_nVelocityX = (int)Lua_ValueToNumber(L, 5);
        pBullet->m_nVelocityY = (int)Lua_ValueToNumber(L, 6);
        pBullet->m_nVelocityZ = (int)Lua_ValueToNumber(L, 7);
    }

    g_PlayerServer.DoNewBulletNotify(m_pScene, pBullet);

    dwBulletID = pBullet->m_dwID;
Exit0:
    Lua_PushNumber(L, dwBulletID);
    return 1;
}

int KHero::LuaAISay(Lua_State* L)
{
	BOOL bResult 	    = false;
	BOOL bRetCode	    = false;
	int	 nTopIndex	    = 0;
    int  nTalkChannel   = 0;
    int  nMsgType       = trRoom;
    const char* szMessage = NULL;
    DWORD dwTalkID      = ERROR_ID;
    char* szTalkerName  = m_szName;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 2);

    nTalkChannel = (int)Lua_ValueToNumber(L, 1);
    KGLOG_PROCESS_ERROR(nTalkChannel == KAI_TALK_CHANNEL_MAP || nTalkChannel == KAI_TALK_CHANNEL_TEAM);

    szMessage = Lua_ValueToString(L, 2);
    KGLOG_PROCESS_ERROR(szMessage);

    if (nTalkChannel == KAI_TALK_CHANNEL_TEAM)
        nMsgType = trTeam;

    if (m_pOwner)
    {
        dwTalkID = m_pOwner->m_dwID;
        szTalkerName = m_pOwner->m_szName;
    }

    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        KSceneObject* pObj = it.GetValue();
        if (!pObj->Is(sotHero))
            continue;

        KHero* pHero = (KHero*)pObj;
        KPlayer* pOwner = pHero->GetOwner();
        if (!pOwner)
            continue;
        if (pOwner->m_pFightingHero != pHero)
            continue;

        if (nTalkChannel == KAI_TALK_CHANNEL_TEAM && pHero->m_nSide != m_nSide)
            continue;

        g_PlayerServer.DoTalkMessage(nMsgType, dwTalkID, szTalkerName, m_dwID,  ERROR_ID, pOwner->m_dwID, pOwner->m_szName, strlen(szMessage), szMessage);
    }

	bResult = true;
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KHero::LuaAutoMove(Lua_State* L)
{
	BOOL bResult 	= false;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
    BOOL bWalk = false;
    int nAroundX = 0;
    int nAroundY = 0;
    int nDestX = 0;
    int nDestY = 0;

	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 5);

    bWalk  = (int)Lua_ValueToNumber(L, 1);
    nAroundX = (int)Lua_ValueToNumber(L, 2);
    nAroundY = (int)Lua_ValueToNumber(L, 3);
    nDestX = (int)Lua_ValueToNumber(L, 4);
    nDestY = (int)Lua_ValueToNumber(L, 5);

    m_AIData.eTargetType = KTARGET_POSITION;
    m_AIData.nTargetX = nDestX;
    m_AIData.nTargetY = nDestY;
    m_AIData.nTargetZ = 0;

    bRetCode = UpdateAutoMoveParam(nAroundX, nAroundY, bWalk);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = DirectlyMoveToDest();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	Lua_PushBoolean(L, bResult);
	return 1;
}

int KHero::LuaCastSkill(Lua_State* L)
{
    BOOL        bRetCode        = false;
    int         nTopIndex       = 0;
    int         nSkillID        = 0;
    KSkill*     pSkill          = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nSkillID = (int)Lua_ValueToNumber(L, 1);

    pSkill = GetWillCastSkill(nSkillID);
    KGLOG_PROCESS_ERROR(pSkill);

    bRetCode = CanCastSkill(pSkill, m_eFaceDir, false);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = CastSkill(pSkill->m_dwID, pSkill->m_dwStepID, m_eFaceDir);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return 0;
}

int KHero::LuaChangeFaceDir(Lua_State* L)
{
    BOOL        bRetCode        = false;
    int         nTopIndex       = 0;
    int         nFaceDir        = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nFaceDir = (int)Lua_ValueToNumber(L, 1);

    SetFaceDir((KTWO_DIRECTION)nFaceDir, true);
    
Exit0:
    return 0;
}

int KHero::LuaSetAISelfDefData(Lua_State* L)
{
    BOOL bResult 	    = false;
    BOOL bRetCode	    = false;
    int	 nTopIndex	    = 0;
    const char* pszKey  = NULL;
    int nNewValue       = 0;
    BOOL bDirectlySet   = false;
    int nDefaultValue   = 0;
    int nCurValue 		= 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 4);

    pszKey        = Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszKey);

    nNewValue     = (int)Lua_ValueToNumber(L, 2);
    bDirectlySet  = (BOOL)Lua_ValueToBoolean(L, 3);
    nDefaultValue = (int)Lua_ValueToNumber(L, 4);

    if (bDirectlySet)
        m_dictAISelfDefData[pszKey] = nNewValue;
    else if (m_dictAISelfDefData.find(pszKey) == m_dictAISelfDefData.end())
        m_dictAISelfDefData[pszKey] = nDefaultValue + nNewValue;
    else
        m_dictAISelfDefData[pszKey] += nNewValue;
    
Exit0:
    return 0;
}

int KHero::LuaCheckAISelfDefData(Lua_State* L)
{
    int nRetCode = KAI_BRANCH_ERROR;
    BOOL bRetCode	= false;
    int	 nTopIndex	= 0;
    const char* pszKey = NULL;
    int nMinValue = 0;
    int nMaxValue = 0;
    int nCurValue = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    pszKey = Lua_ValueToString(L, 1);
    KGLOG_PROCESS_ERROR(pszKey);

    nMinValue = (int)Lua_ValueToNumber(L, 1);
    nMaxValue = (int)Lua_ValueToNumber(L, 2);

    if (m_dictAISelfDefData.find(pszKey) == m_dictAISelfDefData.end())
        goto Exit0;

    nCurValue = m_dictAISelfDefData[pszKey];
    if (nCurValue < nMinValue || nCurValue > nMaxValue)
    {
        nRetCode = KAI_BRANCH_FAILED;
        goto Exit0;
    }

    nRetCode = KAI_BRANCH_SUCCESS;
Exit0:
    Lua_PushNumber(L, nRetCode);
    return 1;
}

int KHero::LuaForbitAutoTakeBall(Lua_State* L)
{
    BOOL        bRetCode        = false;
    int         nTopIndex       = 0;
    int         nForbitFrame    = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nForbitFrame = (int)Lua_ValueToNumber(L, 1);

    ForbitAutoTakeBall(nForbitFrame);

Exit0:
    return 0;
}

int KHero::LuaLoseEndurance(Lua_State* L)
{
	int	nTopIndex	= 0;
    int nEndurance = 0;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 1);

    nEndurance = (int)Lua_ValueToNumber(L, 1);

    LoseEndurance(nEndurance);

Exit0:
	return 0;
}

int KHero::LuaFinishCastingSkill(Lua_State* L)
{
    BOOL bRetCode = false;

    bRetCode = EnsureNotCastingSkill();
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoFinishCastSkillNotify(m_pScene, m_dwID);
    
Exit0:
    return 0;
}

int KHero::LuaSpecialUseDoodad(Lua_State* L)
{
	int  nResult 	= 0;
	BOOL bRetCode	= false;
	int	 nTopIndex	= 0;
	
	nTopIndex = Lua_GetTopIndex(L);
	KGLOG_PROCESS_ERROR(nTopIndex == 0);

    bRetCode = SpecialUseDoodad();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
	return nResult;
}

int KHero::LuaChangeToDefenseAI(Lua_State* L)
{
    KHeroLevelInfo* pLevelInfo = NULL;

    pLevelInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroLevelInfo(m_dwTemplateID, m_nLevel);
    KGLOG_PROCESS_ERROR(pLevelInfo);

    TurnOffAI();
    m_AIData.nAIType    = pLevelInfo->nDefenseAIType;
    TurnOnAI();    

Exit0:
    return 0;
}

int KHero::LuaChangeToAttackAI(Lua_State* L)
{
    KHeroLevelInfo* pLevelInfo = NULL;

    pLevelInfo = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroLevelInfo(m_dwTemplateID, m_nLevel);
    KGLOG_PROCESS_ERROR(pLevelInfo);

    TurnOffAI();
    m_AIData.nAIType    = pLevelInfo->nAIType;
    TurnOnAI();    

Exit0:
	return 0;
}

DEFINE_LUA_CLASS_BEGIN(KHero)
    REGISTER_LUA_DWORD(KObject, ID)
    REGISTER_LUA_INTEGER(KSceneObject, X)
    REGISTER_LUA_INTEGER(KSceneObject, Y)
    REGISTER_LUA_INTEGER(KSceneObject, Z)
    REGISTER_LUA_INTEGER(KSceneObject, Length)
    REGISTER_LUA_INTEGER(KSceneObject, Width)
    REGISTER_LUA_INTEGER(KSceneObject, Height)

    REGISTER_LUA_STRING(KHero, Name)
    REGISTER_LUA_INTEGER(KHero, Level)
    REGISTER_LUA_INTEGER(KHero, Side)
    REGISTER_LUA_INTEGER(KHero, Pos)
    REGISTER_LUA_BOOL_READONLY(KHero, AiMode)
    REGISTER_LUA_INTEGER(KHero, CurrentEndurance)
    REGISTER_LUA_INTEGER(KHero, MaxEndurance)
    REGISTER_LUA_INTEGER(KHero, CurrentStamina)
    REGISTER_LUA_INTEGER(KHero, MaxStamina)
    REGISTER_LUA_INTEGER(KHero, CurrentAngry)
    REGISTER_LUA_INTEGER(KHero, MaxAngry)
    REGISTER_LUA_INTEGER(KHero, HitRate)

    REGISTER_LUA_FUNC(KHero, Jump)
    REGISTER_LUA_FUNC(KHero, MoveToTarget)
    REGISTER_LUA_FUNC(KHero, SetMoveTarget)
    REGISTER_LUA_FUNC(KHero, SwitchAIMode)
    REGISTER_LUA_FUNC(KHero, GetScene)
    REGISTER_LUA_FUNC(KSceneObject, GetPosition)
    REGISTER_LUA_FUNC(KSceneObject, SetPosition)
    REGISTER_LUA_FUNC(KHero, SetAIDebug)
    REGISTER_LUA_FUNC(KHero, SetAIMode)
    REGISTER_LUA_FUNC(KHero, AddBuff)
    REGISTER_LUA_FUNC(KHero, DelBuff)

    REGISTER_LUA_FUNC(KHero, IsMainHero)
    REGISTER_LUA_FUNC(KHero, IsAssistHero)
    REGISTER_LUA_FUNC(KHero, IsNpc)
    REGISTER_LUA_FUNC(KHero, GetOwner)
    REGISTER_LUA_FUNC(KHero, IsTakingBall)
    REGISTER_LUA_FUNC(KHero, TakeBall)
    REGISTER_LUA_FUNC(KHero, UnTakeBall)
    
    REGISTER_LUA_FUNC(KHero, PassBall)
    REGISTER_LUA_FUNC(KHero, AimAt)
    REGISTER_LUA_FUNC(KHero, ShootBall)
    REGISTER_LUA_FUNC(KHero, SlamBall)
    REGISTER_LUA_FUNC(KHero, LoseStamina)
    REGISTER_LUA_FUNC(KHero, ReleaseRelationWithOther)
    REGISTER_LUA_FUNC(KHero, Stand)
    REGISTER_LUA_FUNC(KHero, Sit)
    REGISTER_LUA_FUNC(KHero, FireBullet)
    REGISTER_LUA_FUNC(KHero, AISay)
    REGISTER_LUA_FUNC(KHero, AutoMove)
    REGISTER_LUA_FUNC(KHero, CastSkill)
    REGISTER_LUA_FUNC(KHero, ChangeFaceDir)
    REGISTER_LUA_FUNC(KHero, SetAISelfDefData)
    REGISTER_LUA_FUNC(KHero, CheckAISelfDefData)
    REGISTER_LUA_FUNC(KHero, ForbitAutoTakeBall)
    REGISTER_LUA_FUNC(KHero, LoseEndurance)
    REGISTER_LUA_FUNC(KHero, FinishCastingSkill)
    REGISTER_LUA_FUNC(KHero, SpecialUseDoodad)
    REGISTER_LUA_FUNC(KHero, ChangeToDefenseAI)
    REGISTER_LUA_FUNC(KHero, ChangeToAttackAI)
    
DEFINE_LUA_CLASS_END(KHero)


