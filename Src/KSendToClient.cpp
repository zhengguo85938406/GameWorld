#include "stdafx.h"
#include "KPlayerServer.h"
#include "Engine/KG_CreateGUID.h"
#include "Common/CRC32.h"
#include "KRelayClient.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KScene.h"
#include "Engine/KG_Time.h"
#include "Engine/KTextFilter.h"
#include "KPlayer.h"
#include "KBall.h"
#include "KBasket.h"
#include "KBasketSocket.h"
#include "KBackboard.h"
#include "KClip.h"
#include "KSkill.h"
#include "KBullet.h"
#include "KHeroTrainingMgr.h"
#include "KSecretary.h"
#include "game_define.h"
#include "KLSClient.h"
#include "KPlayerValueInfoList.h"

BOOL KPlayerServer::DoSyncPlayerBaseInfo(KPlayer* pPlayer)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_SyncPlayerBaseInfo SyncPlayerBaseInfo;

    SyncPlayerBaseInfo.protocolID       = s2c_sync_player_base_info;
    SyncPlayerBaseInfo.uPlayerID        = pPlayer->m_dwID;
    SyncPlayerBaseInfo.nCreateTime      = (int)pPlayer->m_nCreateTime;
    SyncPlayerBaseInfo.nTotalGameTime   = pPlayer->m_nTotalGameTime;
    SyncPlayerBaseInfo.nLastSaveTime    = (int)pPlayer->m_nLastSaveTime;
    SyncPlayerBaseInfo.nLastLoginTime   = (int)pPlayer->m_nLastLoginTime;
    SyncPlayerBaseInfo.byGender         = (BYTE)pPlayer->m_eGender;
    SyncPlayerBaseInfo.byCanRename      = (BYTE)pPlayer->m_bCanRename;
    SyncPlayerBaseInfo.nServerTime      = g_pSO3World->m_nCurrentTime;
    SyncPlayerBaseInfo.uClubID          = pPlayer->m_dwClubID;
    SyncPlayerBaseInfo.nGroupID         = pPlayer->m_nGroupID;

    SyncPlayerBaseInfo.byCreateHeroTemplateID = (BYTE)pPlayer->m_nCreateHeroTemplateID;

    strncpy(SyncPlayerBaseInfo.szAccountName, pPlayer->m_szAccount, countof(SyncPlayerBaseInfo.szAccountName));
    SyncPlayerBaseInfo.szAccountName[countof(SyncPlayerBaseInfo.szAccountName) - 1] = '\0';

    strncpy(SyncPlayerBaseInfo.szPlayerName, pPlayer->m_szName, countof(SyncPlayerBaseInfo.szPlayerName));
    SyncPlayerBaseInfo.szPlayerName[countof(SyncPlayerBaseInfo.szPlayerName) - 1] = '\0';

    bRetCode = Send(pPlayer->m_nConnIndex, &SyncPlayerBaseInfo, sizeof(SyncPlayerBaseInfo));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncPlayerStateInfo(KPlayer* pPlayer)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_SyncPlayerStateInfo SyncPlayerStateInfo;

    SyncPlayerStateInfo.protocolID                  = s2c_sync_player_state_info;
    SyncPlayerStateInfo.wHighestHeroLevel           = (unsigned short)pPlayer->m_nHighestHeroLevel; 
    SyncPlayerStateInfo.uMainHero                   = (unsigned short)pPlayer->m_dwMainHeroTemplateID;
    SyncPlayerStateInfo.uAssistHero                 = (unsigned short)pPlayer->m_dwAssistHeroTemplateID;
    SyncPlayerStateInfo.wCurFatiguePoint            = (unsigned short)pPlayer->m_nCurFatiguePoint;     
    SyncPlayerStateInfo.wMaxFatiguePoint            = (unsigned short)pPlayer->m_nMaxFatiguePoint;
    SyncPlayerStateInfo.byTodayBuyFatiguePointTimes = (BYTE)pPlayer->m_nTodayBuyFatiguePointTimes;
    SyncPlayerStateInfo.byLevel                     = (unsigned char)pPlayer->m_nLevel;
    SyncPlayerStateInfo.nExp                        = pPlayer->m_nExp;
    SyncPlayerStateInfo.byVipLevel                  = (unsigned char)pPlayer->m_nVIPLevel;
    SyncPlayerStateInfo.nVIPExp                     = pPlayer->m_nVIPExp;
    SyncPlayerStateInfo.nVIPEndTime                 = pPlayer->m_nVIPEndTime;
    SyncPlayerStateInfo.byCurrentCity               = (BYTE)pPlayer->m_nCurrentCity;
    SyncPlayerStateInfo.byLastChallengeStep1        = (BYTE)pPlayer->m_nLastChallengeStep1;
    SyncPlayerStateInfo.byLastChallengeStep2        = (BYTE)pPlayer->m_nLastChallengeStep2;
    SyncPlayerStateInfo.bySkipRandomQuestTime       = (BYTE)pPlayer->m_nSkipRandomQuestTime;
    SyncPlayerStateInfo.byRefuseStranger            = (BYTE)pPlayer->m_bRefuseStranger;

    bRetCode = Send(pPlayer->m_nConnIndex, &SyncPlayerStateInfo, sizeof(SyncPlayerStateInfo));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

// 同步玩家数据
BOOL KPlayerServer::DoSyncPlayerHeroInfo(KPlayer* pPlayer)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KHero*                  pHero       = NULL;
    KS2C_SyncPlayerHeroInfo SyncPlayerHeroInfo;
    KPOSITION pos;

    pHero = pPlayer->GetFightingHero();
    assert(pHero);

    pos = pHero->GetPosition();

    memset(&SyncPlayerHeroInfo, 0, sizeof(SyncPlayerHeroInfo));

    SyncPlayerHeroInfo.protocolID   = s2c_sync_player_hero_info;
    SyncPlayerHeroInfo.heroID       = pHero->m_dwID;
    SyncPlayerHeroInfo.mapID        = pPlayer->m_dwMapID;
    SyncPlayerHeroInfo.mapCopyIndex = pPlayer->m_nCopyIndex;
    SyncPlayerHeroInfo.x            = pos.nX;
    SyncPlayerHeroInfo.y            = pos.nY;
    SyncPlayerHeroInfo.z            = pos.nZ;
    SyncPlayerHeroInfo.faceDir      = (unsigned char)pHero->GetFaceDir();
    SyncPlayerHeroInfo.bySide       = (BYTE)pHero->m_nSide;
    SyncPlayerHeroInfo.byVIP        = (unsigned char)pHero->GetVIPLevel();
    SyncPlayerHeroInfo.normalLevel  = (short)pHero->GetNormalLevel();
    SyncPlayerHeroInfo.ladderLevel  = (short)pHero->GetLadderLevel();
    SyncPlayerHeroInfo.wHeroTemplateID = (WORD)pHero->m_dwTemplateID;
    SyncPlayerHeroInfo.heroData.nCurrentEndurance    = pHero->m_nCurrentEndurance;
    SyncPlayerHeroInfo.heroData.nMaxEndurance        = pHero->m_nMaxEndurance;
    SyncPlayerHeroInfo.heroData.nCurrentStamina      = pHero->m_nCurrentStamina;
    SyncPlayerHeroInfo.heroData.nMaxStamina          = pHero->m_nMaxStamina;
    SyncPlayerHeroInfo.heroData.wCurrentAngry        = (WORD)pHero->m_nCurrentAngry;
    SyncPlayerHeroInfo.heroData.wMaxAngry            = (WORD)pHero->m_nMaxAngry;
    SyncPlayerHeroInfo.heroData.wWillPower           = (WORD)pHero->m_nWillPower;
    SyncPlayerHeroInfo.heroData.wInterference        = (WORD)pHero->m_nInterference;
    SyncPlayerHeroInfo.heroData.wInterferenceRange   = (WORD)pHero->m_nInterferenceRange;
    SyncPlayerHeroInfo.heroData.wMoveSpeed           = (WORD)pHero->m_nMoveSpeed;
    SyncPlayerHeroInfo.heroData.wJumpSpeed           = (WORD)pHero->m_nJumpSpeed;
    SyncPlayerHeroInfo.heroData.wAddGravityBase      = (WORD)pHero->m_nAddGravityBase;
    SyncPlayerHeroInfo.heroData.wGravityPercent      = (WORD)pHero->m_nGravityPercent;
    SyncPlayerHeroInfo.heroData.wFashionID           = (WORD)pHero->m_wFashionID;

    for (int i = 0; i < KACTIVE_SLOT_TYPE_TOTAL; ++i)
    {
        SyncPlayerHeroInfo.heroData.wActiveSkill[i] = (WORD)pHero->m_ActiveSkill[i];
    }

    for (int i = 0; i < cdNirvanaSkillSlotCount; ++i)
    {
        SyncPlayerHeroInfo.heroData.wNirvanaSkill[i] = (WORD)pHero->m_NirvanaSkill[i];
    }

    SyncPlayerHeroInfo.heroData.wSlamBallSkill = (WORD)pHero->m_SlamBallSkill[0];

    for (size_t i = 0; i < countof(SyncPlayerHeroInfo.heroData.wOtherActiveSkill); ++i)
    {
        if (i < pHero->m_vecAllOtherActiveSkill.size())
            SyncPlayerHeroInfo.heroData.wOtherActiveSkill[i] = (WORD)pHero->m_vecAllOtherActiveSkill[i];
        else
            SyncPlayerHeroInfo.heroData.wOtherActiveSkill[i] = 0;
    }

    strncpy(SyncPlayerHeroInfo.roleName, pPlayer->m_szName, _NAME_LEN);
    SyncPlayerHeroInfo.roleName[_NAME_LEN - 1] = 0;

    bRetCode = Send(pPlayer->m_nConnIndex, &SyncPlayerHeroInfo, sizeof(SyncPlayerHeroInfo));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAccountKickout(int nConnIndex)
{
    BOOL                        bResult       = false;
    BOOL                        bRetCode      = false;
    KS2C_AccountKickout  Notify;

    Notify.protocolID = s2c_account_kickout;

    bRetCode = Send(nConnIndex, &Notify, sizeof(Notify));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoPlayerEnterHallNotify(int nConnIndex)
{
    BOOL                            bResult  = false;
    BOOL                            bRetCode = false;
    KS2C_PlayerEnterHallNotify      PlayerEnterHall;

    PlayerEnterHall.protocolID   = s2c_player_enter_hall_notify;

    bRetCode = Send(nConnIndex, &PlayerEnterHall, sizeof(PlayerEnterHall));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

// 通知客户端切换服务器
BOOL KPlayerServer::DoSwitchGS(int nConnIndex, DWORD dwPlayerID, GUID& Guid, DWORD dwIPAddr, WORD wPort)
{
    BOOL                    bResult             = false;
    BOOL                    bRetCode            = false;
    KS2C_SwitchGS           Notify;

    Notify.protocolID       = s2c_switch_gs;
    Notify.playerID         = dwPlayerID;
    Notify.IPAddr           = dwIPAddr;
    Notify.port             = wPort;
    memcpy(Notify.guid, &Guid, sizeof(Notify.guid));

    bRetCode = Send(nConnIndex, &Notify, sizeof(Notify));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoLimitPlayInfoResponse( KPlayer* pPlayer, BYTE byResult )
{
	BOOL            bResult     = false;
	BOOL            bRetCode    = false;
	KS2C_LimitplayInfoRespond  Pak;

	Pak.protocolID      = s2c_limitplay_info_respond;
	Pak.byResult		= byResult;

	bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
	KG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSwitchMap(KPlayer* pPlayer, DWORD dwMapID, int nCopyIndex)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KScene*         pScene      = NULL;
    KMission*       pMission    = NULL;
    KS2C_SwitchMap  Notify;

    Notify.protocolID       = s2c_switch_map;
    Notify.mapID            = dwMapID;
    Notify.copyIndex        = nCopyIndex;

    Notify.missionType      = 0;
    Notify.missionStep      = 0;   
    Notify.missionLevel     = 0;   

    pMission = g_pSO3World->m_MissionMgr.GetMission(dwMapID);
    if (pMission)
    {
        Notify.missionType      = pMission->eType;
        Notify.missionStep      = pMission->nStep;
        Notify.missionLevel     = pMission->nLevel;
    }
    
    pScene = g_pSO3World->GetScene(dwMapID, nCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    bRetCode = pScene->GetAllHeroTemplate(Notify.allHero);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pScene->GetAllHeroAIMode(Notify.allHeroAIMode);
    KGLOG_PROCESS_ERROR(bRetCode);

    Notify.battleType = pScene->m_Param.m_eBattleType;
    Notify.playerIndexInTeam = (char)pScene->GetPlayerPosInTeam(pPlayer->m_dwID);

    bRetCode = Send(pPlayer->m_nConnIndex, &Notify, sizeof(Notify));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void GetSyncNewHeroData(KHero* pHero, KS2C_SyncNewHero& Pak)
{
    assert(pHero);
    KPOSITION cPos = pHero->GetPosition();

    assert(cPos.nX >= 0 && cPos.nX <= MAX_X_COORDINATE);
    assert(cPos.nY >= 0 && cPos.nY <= MAX_Y_COORDINATE);

    memset(&Pak, 0, sizeof(Pak));

    Pak.protocolID      = s2c_sync_new_hero;
    Pak.id              = pHero->m_dwID;
    strncpy(Pak.roleName, pHero->m_szName, sizeof(Pak.roleName));
    Pak.roleName[sizeof(Pak.roleName) - 1] = '\0';

    Pak.moveState       = (BYTE)pHero->m_eMoveState;
    Pak.x               = cPos.nX;
    Pak.y               = cPos.nY;
    Pak.z               = cPos.nZ;
    Pak.velocityX       = pHero->m_nVelocityX;
    Pak.velocityY       = pHero->m_nVelocityY;
    Pak.velocityZ       = pHero->m_nVelocityZ;
    Pak.facedir         = (BYTE)pHero->m_eFaceDir;

    assert(pHero->m_nSide >= 0 && pHero->m_nSide <= 255);
    Pak.teamID          = (BYTE)pHero->m_nSide;
    Pak.isRunMode		= pHero->m_bIsRunMode;
    Pak.wTemplateID     = (unsigned short)pHero->m_dwTemplateID;
    Pak.isAiTeammate    = (unsigned char)pHero->IsAssistHero();
    Pak.byIsVIP         = (BYTE)pHero->IsVip();
    Pak.byVIPLevel      = (BYTE)pHero->GetVIPLevel();
    Pak.normalLevel     = (short)pHero->GetNormalLevel();
    Pak.ladderLevel     = (short)pHero->GetLadderLevel();

    Pak.heroData.nCurrentEndurance    = pHero->m_nCurrentEndurance;
    Pak.heroData.nMaxEndurance        = pHero->m_nMaxEndurance;
    Pak.heroData.nCurrentStamina      = pHero->m_nCurrentStamina;
    Pak.heroData.nMaxStamina          = pHero->m_nMaxStamina;
    Pak.heroData.wCurrentAngry        = (WORD)pHero->m_nCurrentAngry;
    Pak.heroData.wMaxAngry            = (WORD)pHero->m_nMaxAngry;
    Pak.heroData.wWillPower           = (WORD)pHero->m_nWillPower;
    Pak.heroData.wInterference        = (WORD)pHero->m_nInterference;
    Pak.heroData.wInterferenceRange   = (WORD)pHero->m_nInterferenceRange;
    Pak.heroData.wMoveSpeed           = (WORD)pHero->m_nMoveSpeed;
    Pak.heroData.wJumpSpeed           = (WORD)pHero->m_nJumpSpeed;
    Pak.heroData.wAddGravityBase      = (WORD)pHero->m_nAddGravityBase;
    Pak.heroData.wGravityPercent      = (WORD)pHero->m_nGravityPercent;
    Pak.heroData.wFashionID           = (WORD)pHero->m_wFashionID;

    for (int i = 0; i < KACTIVE_SLOT_TYPE_TOTAL; ++i)
    {
        Pak.heroData.wActiveSkill[i] = (WORD)pHero->m_ActiveSkill[i];
    }

    for (int i = 0; i < cdNirvanaSkillSlotCount; ++i)
    {
        Pak.heroData.wNirvanaSkill[i] = (WORD)pHero->m_NirvanaSkill[i];
    }

    Pak.heroData.wSlamBallSkill = (WORD)pHero->m_SlamBallSkill[0];

    for (size_t i = 0; i < countof(Pak.heroData.wOtherActiveSkill); ++i)
    {
        if (i < pHero->m_vecAllOtherActiveSkill.size())
            Pak.heroData.wOtherActiveSkill[i] = (WORD)pHero->m_vecAllOtherActiveSkill[i];
        else
            Pak.heroData.wOtherActiveSkill[i] = 0;
    }

    return;
}

BOOL KPlayerServer::DoSyncNewHero(KHero* pHero, int nConnIndex)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    KS2C_SyncNewHero            Pak;

    GetSyncNewHeroData(pHero, Pak);

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

// 广播hero
BOOL KPlayerServer::DoBroadcastNewHero(KHero* pHero, DWORD dwExceptHeroID)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    KS2C_SyncNewHero            Pak;
    KBroadcastFunc              BroadcastFunc;
    KScene*                     pScene      = NULL;

    assert(pHero);

    GetSyncNewHeroData(pHero, Pak);

    BroadcastFunc.m_pvData          = &Pak;
    BroadcastFunc.m_uSize           = sizeof(Pak);
    BroadcastFunc.m_dwExcept        = dwExceptHeroID;
    BroadcastFunc.m_dwExclusive     = 0;

    pScene = pHero->m_pScene;
    assert(pScene);

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncSceneObject(KSceneObject* pObject, int nConnIndex)
{
    BOOL                        bResult             = false;
    BOOL                        bRetCode            = false;
    KS2C_SyncSceneObj           Pak;
    KPOSITION                   cPos                = pObject->GetPosition();

    assert(cPos.nX >= 0 && cPos.nX <= MAX_X_COORDINATE);
    assert(cPos.nY >= 0 && cPos.nY <= MAX_Y_COORDINATE);

    KGLOG_PROCESS_ERROR(pObject->m_pTemplate);

    Pak.protocolID      = s2c_sync_scene_object;
    Pak.id              = pObject->m_dwID;
    Pak.state           = (BYTE)pObject->GetMoveType();
    Pak.templateid      = (BYTE)pObject->m_pTemplate->m_dwID;
    Pak.x               = cPos.nX;
    Pak.y               = cPos.nY;
    Pak.z               = cPos.nZ;
    Pak.velocityX       = pObject->GetVelocityX();
    Pak.velocityY       = pObject->GetVelocityY();
    Pak.velocityZ       = pObject->GetVelocityZ();
    Pak.gravity         = pObject->GetCurrentGravity();
    Pak.followerID      = pObject->GetGuideID();
    Pak.customData0     = pObject->GetCustomData(0);
    Pak.customData1     = pObject->GetCustomData(1);
    Pak.customData2     = pObject->GetCustomData(2);
    Pak.customData3     = pObject->GetCustomData(3);

    if (nConnIndex != -1)
    {
        bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
        KG_PROCESS_ERROR(bRetCode);
    }
    else // 广播
    {
        KBroadcastFunc      BroadcastFunc;
        KScene*             pScene      = NULL;

        BroadcastFunc.m_pvData          = &Pak;
        BroadcastFunc.m_uSize           = sizeof(Pak);
        BroadcastFunc.m_dwExclusive     = 0;

        pScene = pObject->m_pScene;
        assert(pScene);
        pScene->TraverseHero(BroadcastFunc);
    }

    bResult = true;
Exit0:
    return bResult;
}

// 通知客户端某个Hero被移除了
BOOL KPlayerServer::DoBroadcastHeroRemove(KScene* pScene, DWORD dwHeroID, BOOL bKilled)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    KBroadcastFunc                  BroadcastFunc;
    KS2C_RemoveHero            RemoveCharacter;

    assert(pScene);

    RemoveCharacter.protocolID  = s2c_remove_hero;
    RemoveCharacter.id          = dwHeroID;
    RemoveCharacter.killed      = (unsigned char)bKilled;

    BroadcastFunc.m_pvData            = &RemoveCharacter;
    BroadcastFunc.m_uSize             = sizeof(RemoveCharacter);
    BroadcastFunc.m_dwExclusive       = 0;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncCurrentEndurance(KHero* pHero)
{
    KS2C_SyncCurrentEndurance Pak;
    KBroadcastFunc       BroadcastFunc;

    assert(pHero);

    KG_PROCESS_ERROR(pHero->m_nCurrentEndurance != pHero->m_nLastSyncCurrentEndurance);
    pHero->m_nLastSyncCurrentEndurance = pHero->m_nCurrentEndurance;

    Pak.protocolID  = s2c_sync_current_endurance;
    Pak.id          = pHero->m_dwID;
    Pak.currentEndurance = pHero->m_nCurrentEndurance;

    BroadcastFunc.m_pvData            = &Pak;
    BroadcastFunc.m_uSize             = sizeof(Pak);
    BroadcastFunc.m_dwExclusive       = 0;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

Exit0:
    return true;
}

BOOL KPlayerServer::DoSyncCurrentStamina(KHero* pHero)
{
    KS2C_Sync_Current_Stamina Pak;
    KBroadcastFunc       BroadcastFunc;

    assert(pHero);

    KG_PROCESS_ERROR(pHero->m_nCurrentStamina != pHero->m_nLastSyncCurrentStamina);
    pHero->m_nLastSyncCurrentStamina = pHero->m_nCurrentStamina;

    Pak.protocolID  = s2c_sync_current_stamina;
    Pak.id          = pHero->m_dwID;
    Pak.currentStamina = pHero->m_nCurrentStamina;

    BroadcastFunc.m_pvData            = &Pak;
    BroadcastFunc.m_uSize             = sizeof(Pak);
    BroadcastFunc.m_dwExclusive       = 0;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

Exit0:
    return true;
}

BOOL KPlayerServer::DoSyncCurrentAngry(KHero* pHero)
{
    KS2C_Sync_Current_Angry Pak;
    KBroadcastFunc       BroadcastFunc;

    assert(pHero);

    KG_PROCESS_ERROR(pHero->m_nCurrentAngry != pHero->m_nLastSyncCurrentAngry);
    pHero->m_nLastSyncCurrentAngry = pHero->m_nCurrentAngry;

    Pak.protocolID  = s2c_sync_current_angry;
    Pak.id          = pHero->m_dwID;
    Pak.currentAngry = pHero->m_nCurrentAngry;

    BroadcastFunc.m_pvData            = &Pak;
    BroadcastFunc.m_uSize             = sizeof(Pak);
    BroadcastFunc.m_dwExclusive       = 0;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

Exit0:
    return true;
}

BOOL KPlayerServer::DoSyncCurrentMoneys(KPlayer* pPlayer)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Sync_Current_Moneys Pak;

    assert(pPlayer);

    Pak.protocolID = s2c_sync_current_moneys;

    Pak.nMoney          = pPlayer->m_MoneyMgr.GetMoney(emotMoney);
    Pak.nCoin           = pPlayer->m_MoneyMgr.GetMoney(emotCoin);
    Pak.nMenterPoint    = pPlayer->m_MoneyMgr.GetMoney(emotMenterPoint);
    Pak.bySafeBoxLevel  = (BYTE)pPlayer->m_MoneyMgr.GetSafeBoxLevel();

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMoneyChanged(int nConnIndex, ENUM_MONEY_TYPE eMoneyType, int nAmount)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Money_Changed Pak;

    KGLOG_PROCESS_ERROR(eMoneyType >= emotBegin && eMoneyType < emotTotal);

    Pak.protocolID      = s2c_money_changed;
    Pak.eMoneyType      = (char)eMoneyType;
    Pak.nChangedMoney   = nAmount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncRunMode(KHero* pHero)
{
    BOOL                bResult         = false;
    KBroadcastFunc      BroadcastFunc;
    KS2C_SyncRunMode    Pak;

    assert(pHero->m_pScene);

    Pak.protocolID  = s2c_sync_run_mode;
    Pak.id          = pHero->m_dwID;
    Pak.isRunMode	= pHero->m_bIsRunMode;

    BroadcastFunc.m_pvData            = &Pak;
    BroadcastFunc.m_uSize             = sizeof(Pak);
    BroadcastFunc.m_dwExclusive       = 0;
    BroadcastFunc.m_dwExcept          = ERROR_ID;
    pHero->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMoveState(KHero* pHero, int nVirtualFrame, BOOL bSyncSelf)
{
    BOOL                        bResult         = false;
    KBroadcastFunc              BroadcastFunc;
    KS2C_SyncMoveState          MoveState;
    KPOSITION       cPos;

    assert(pHero->m_pScene);

    cPos = pHero->GetPosition();

    MoveState.protocolID            = s2c_sync_move_state;
    MoveState.id                    = pHero->m_dwID;
    MoveState.virtualFrame          = nVirtualFrame;
    MoveState.nCurrentMoveCount     = pHero->m_MoveCount;

    MoveState.moveState             = (BYTE)pHero->m_eMoveState;
    MoveState.wMoveFrameCount       = (WORD)pHero->m_nMoveFrameCounter;
    MoveState.x                     = cPos.nX;
    MoveState.y                     = cPos.nY;
    MoveState.z                     = cPos.nZ;

    MoveState.velocityX             = pHero->m_nVelocityX;
    MoveState.velocityY             = pHero->m_nVelocityY;
    MoveState.velocityZ             = pHero->m_nVelocityZ;
    MoveState.jumpCount             = (BYTE)pHero->m_nJumpCount;
    MoveState.wAddGravityBase       = (short)pHero->m_nAddGravityBase;
    MoveState.wGravityPercent       = (short)pHero->m_nGravityPercent;
    MoveState.facedir               = (BYTE)pHero->m_eFaceDir;
    MoveState.idOfStandOnObj        = pHero->GetStandOnObjID();

    BroadcastFunc.m_pvData          = &MoveState;
    BroadcastFunc.m_uSize           = sizeof(MoveState);
    BroadcastFunc.m_dwExclusive     = 0;
    BroadcastFunc.m_dwExcept        = bSyncSelf ? ERROR_ID : pHero->m_dwID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);
    //KGLogPrintf(KGLOG_INFO, "Broadcast player %d move state %d", pHero->m_dwID, MoveState.moveState);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAdjustHeroMove(KHero* pHero, int nReason)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_AdjustHeroMove       Pak;
    KPOSITION                   cPos;    

    cPos                    = pHero->GetPosition();

    Pak.protocolID          = s2c_adjust_hero_move;

    Pak.nMoveCount          = pHero->m_MoveCount;
    Pak.nVirtualFrame       = pHero->m_nVirtualFrame;
    Pak.byMoveFrameCount    = (BYTE)pHero->m_nMoveFrameCounter;

    Pak.moveState           = (BYTE)pHero->m_eMoveState;
    Pak.x                   = cPos.nX;
    Pak.y                   = cPos.nY;
    Pak.z                   = cPos.nZ;

    Pak.velocityX           = pHero->m_nVelocityX;
    Pak.velocityY           = pHero->m_nVelocityY;
    Pak.velocityZ           = pHero->m_nVelocityZ;
    Pak.wAddGravityBase     = (short)pHero->m_nAddGravityBase;
    Pak.wGravityPercent     = (short)pHero->m_nGravityPercent;
    Pak.jumpCount           = (unsigned char)pHero->m_nJumpCount;
    Pak.facedir             = (BYTE)pHero->m_eFaceDir;
    Pak.takeobjid           = pHero->GetHoldingObjID();
    Pak.standingobjid       = pHero->GetStandOnObjID();
    Pak.adjustreason        = (unsigned char)nReason;

    KGLogPrintf(
        KGLOG_DEBUG, "[move] do adjust, count=%d,frame=%d<%d,%s>, velocity=<%d,%d,%d>, reason:%d", 
        Pak.nMoveCount, Pak.nVirtualFrame,
        pHero->m_eMoveState, pHero->GetPosition().ToString().c_str(),
        pHero->m_nVelocityX, pHero->m_nVelocityY, pHero->m_nVelocityZ, nReason
    );

    bRetCode = Send(pHero->GetConnIndex(), &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoFrameSignal(int nConnIndex)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_FrameSignal     FrameSignal;

    FrameSignal.protocolID  = s2c_frame_signal;
    FrameSignal.frame       = g_pSO3World->m_nGameLoop;

    bRetCode = Send(nConnIndex, &FrameSignal, sizeof(FrameSignal));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoTakeBall(KHero* pHero, BOOL bNotifySelf)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_TakeBall           Pak;
    KBroadcastFunc          BroadcastFunc;
    DWORD                   dwExceptID = ERROR_ID;

    assert(pHero);
    assert(pHero->m_pScene);

    if (!bNotifySelf)
        dwExceptID = pHero->m_dwID;

    Pak.protocolID    = s2c_take_ball;
    Pak.id   = pHero->m_dwID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = dwExceptID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoTakeObject(KHero* pHero, KDoodad* pObject)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_TakeDoodad         Pak;
    KBroadcastFunc          BroadcastFunc;

    assert(pHero);
    assert(pHero->m_pScene);
    assert(pObject);
    assert(pObject->m_pScene);

    Pak.protocolID  = s2c_take_doodad;
    Pak.idTaker     = pHero->m_dwID;
    Pak.idObject    = pObject->m_dwID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUseDoodad(KHero* pHero, KDoodad* pObject, unsigned char direction)
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    KS2C_UseDoodad  Pak;
    KBroadcastFunc  BroadcastFunc;
    KPOSITION       cPos;

    assert(pHero);
    assert(pHero->m_pScene);
    assert(pObject);
    assert(pObject->m_pScene);

    cPos = pHero->GetPosition();

    Pak.protocolID  = s2c_use_doodad;
    Pak.idUser   = pHero->m_dwID;
    Pak.idObject    = pObject->m_dwID;
    Pak.x = cPos.nX;
    Pak.y = cPos.nY;
    Pak.z = cPos.nZ;
    Pak.direction   = direction;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDropObject( KHero* pHero, KDoodad* pObject)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_DropDoodad         Pak;
    KBroadcastFunc          BroadcastFunc;
    KPOSITION               cPos;

    assert(pHero);
    assert(pHero->m_pScene);
    assert(pObject);
    assert(pObject->m_pScene);

    cPos = pObject->GetPosition();

    Pak.protocolID  = s2c_drop_doodad;
    Pak.idThrower   = pHero->m_dwID;
    Pak.idObject    = pObject->m_dwID;
    Pak.x           = cPos.nX;
    Pak.y           = cPos.nY;
    Pak.z           = cPos.nZ;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);
    //KGLogPrintf(KGLOG_INFO, "Broadcast player:%d drop object %d.", pHero->m_dwID, pObject->m_dwID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDetachObject( KHero* pHero, KDoodad* pObject)
{
    BOOL                bResult = false;
    KS2C_DetachDoodad   Pak;
    KBroadcastFunc      BroadcastFunc; 
    KPOSITION           cPos;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pObject);

    cPos = pObject->GetPosition();

    Pak.protocolID      = s2c_detach_doodad;
    Pak.idBeAttacher    = pHero->m_dwID;
    Pak.idObject        = pObject->m_dwID; 
    Pak.x               = cPos.nX;
    Pak.y               = cPos.nY;
    Pak.z               = cPos.nZ;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    //KGLogPrintf(KGLOG_INFO, "Broadcast player:%d detach %d.", pHero->m_dwID, pObject->m_dwID);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoUnTakeBall(KHero* pHero)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_UnTakeBall         Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;
    KBall*                  pBall       = NULL;
    KPOSITION               cPos;

    assert(pHero);
    assert(pHero->m_pScene);

    pScene = pHero->m_pScene;

    pBall = pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    cPos = pBall->GetPosition();

    Pak.protocolID  = s2c_untake_ball;
    Pak.id          = pHero->m_dwID;
    Pak.x           = cPos.nX;
    Pak.y           = cPos.nY;
    Pak.z           = cPos.nZ;
    Pak.velocityX   = pBall->m_nVelocityX;
    Pak.velocityY   = pBall->m_nVelocityY;
    Pak.velocityZ   = pBall->m_nVelocityZ;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);
    //KGLogPrintf(KGLOG_INFO, "Broadcast player:%d untake ball", pHero->m_dwID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoShootObject(KHero* pHero, KDoodad* pObject, DWORD dwSocketID, BOOL bCanHit)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_ShootDoodad        Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;
    KPOSITION               cPos;

    assert(pHero);
    assert(pObject);

    pScene = pHero->m_pScene;
    assert(pScene);

    cPos = pHero->GetPosition();

    Pak.protocolID      = s2c_shoot_doodad;
    Pak.id              = pHero->m_dwID;
    Pak.x               = cPos.nX;
    Pak.y               = cPos.nY;
    Pak.z               = cPos.nZ;
    Pak.objectID        = pObject->m_dwID;
    Pak.socketID        = dwSocketID;
    Pak.canHit          = BYTE(bCanHit);
    Pak.nVirtualFrame   = g_pSO3World->m_nGameLoop;
    Pak.aimatLoop		= pHero->m_nShootTime;
    Pak.nVirtualFrame   = pHero->m_nVirtualFrame;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = pHero->m_dwID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoShootBasket(KHero* pHero, DWORD dwBasketID, KBasketSocket* pBasketSocket)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_ShootBasket        Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;
    KBasket*                pBasket     = NULL;
    KPOSITION               cPos;

    assert(pBasketSocket);
    assert(pHero);
    assert(pHero->m_pScene);

    pScene = pHero->m_pScene;
    pBasket = pScene->GetBasketByID(dwBasketID);
    KGLOG_PROCESS_ERROR(pBasket);

    cPos = pHero->GetPosition();

    Pak.protocolID  = s2c_shoot_basket;
    Pak.id          = pHero->m_dwID;
    Pak.x           = cPos.nX;
    Pak.y           = cPos.nY;
    Pak.z           = cPos.nZ;
    Pak.moveFrame   = pBasket->m_nTotalFrame;
    Pak.basketID    = dwBasketID;
    Pak.basketSocketID  = pBasketSocket->m_dwID;
    Pak.aimatLoop       = pHero->m_nShootTime;
    Pak.nVirtualFrame   = pHero->m_nVirtualFrame;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = pHero->m_dwID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::BasketDrop(KBasket* pBasket)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_BasketDrop         Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;

    KGLOG_PROCESS_ERROR(pBasket);

    pScene = pBasket->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    Pak.protocolID          = s2c_basket_drop;
    Pak.basketID            = pBasket->m_dwID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncFaceDir(KHero* pHero, unsigned char direction)
{
    BOOL                    bResult     = false;
    KS2C_SyncFaceDir        Pak;
    KBroadcastFunc          BroadcastFunc;

    assert(pHero);
    assert(pHero->m_pScene);

    Pak.protocolID = s2c_sync_face_dir;

    Pak.id          = pHero->m_dwID;
    Pak.direction8  = direction;

    BroadcastFunc.m_pvData            = &Pak;
    BroadcastFunc.m_uSize             = sizeof(Pak);
    BroadcastFunc.m_dwExclusive       = 0;
    BroadcastFunc.m_dwExcept          = pHero->m_dwID;
    pHero->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoHeroSkill(KHero* pCaster, DWORD dwSkillID, DWORD dwStepID, BOOL bSyncSelf)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_CastSkill          Pak;
    KBroadcastFunc          BroadcastFunc;
    DWORD                   dwExcept  = ERROR_ID;

    assert(pCaster);
    assert(pCaster->m_pScene);

    Pak.protocolID  = s2c_cast_skill;
    Pak.casterID    = pCaster->m_dwID;
    Pak.x           = pCaster->m_nX;
    Pak.y           = pCaster->m_nY;
    Pak.z           = pCaster->m_nZ;
    Pak.skillID     = dwSkillID;
    Pak.step        = dwStepID;
    Pak.facedir     = (BYTE)pCaster->m_eFaceDir;

    if (!pCaster->m_bAiMode && !pCaster->m_pScene->IsDramaPlayering() && !bSyncSelf)
        dwExcept = pCaster->m_dwID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = dwExcept;

    pCaster->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoHeroGrabSkill(KHero* pCaster, DWORD dwSkillID, DWORD dwStepID, DWORD dwTargetHeroID)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_CastGrabSkill      Pak;
    KBroadcastFunc          BroadcastFunc;
    DWORD                   dwExcept    = ERROR_ID;

    assert(pCaster);
    assert(pCaster->m_pScene);

    Pak.protocolID      = s2c_cast_grabskill;
    Pak.casterID        = pCaster->m_dwID;
    Pak.x               = pCaster->m_nX;
    Pak.y               = pCaster->m_nY;
    Pak.z               = pCaster->m_nZ;
    Pak.skillID         = dwSkillID;
    Pak.step            = dwStepID;
    Pak.targetHeroID    = dwTargetHeroID;

    if (!pCaster->m_bAiMode && !pCaster->m_pScene->IsDramaPlayering())
        dwExcept = pCaster->m_dwID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = dwExcept;

    pCaster->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSkillResult(KHero* pCaster, DWORD dwTargetID, KSkill* pSkill)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_SkillResult        Pak;
    KBroadcastFunc          BroadcastFunc;

    assert(pCaster);
    assert(pCaster->m_pScene);

    Pak.protocolID      = s2c_skill_result;
    Pak.casterID        = pCaster->m_dwID;
    Pak.targetID        = dwTargetID;
    Pak.skillID         = pSkill->m_dwID;
    Pak.stepID          = pSkill->m_dwStepID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pCaster->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoPassBallTo(KHero* ballTaker, KHero* teammate)
{
    BOOL            bResult = false;
    BOOL            bRetCode = false;
    KBall*          pBall = NULL;
    KPOSITION       cStart;
    KPOSITION       cFinnal;
    KS2C_PassBall	Pak;
    KBroadcastFunc  BroadcastFunc; 

    KGLOG_PROCESS_ERROR(ballTaker);
    KGLOG_PROCESS_ERROR(teammate);

    pBall = ballTaker->m_pScene->GetBall();
    KGLOG_PROCESS_ERROR(pBall);

    bRetCode = pBall->GetStartPos(cStart);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pBall->GetFinnalPos(cFinnal);
    KGLOG_PROCESS_ERROR(bRetCode);

    Pak.protocolID  = s2c_pass_ball;
    Pak.takerID = ballTaker->m_dwID;
    Pak.teammateID = teammate->m_dwID;

    Pak.startX = cStart.nX;
    Pak.startY = cStart.nY;
    Pak.startZ = cStart.nZ;
    Pak.targetX = cFinnal.nX;
    Pak.targetY = cFinnal.nY;
    Pak.targetZ = cFinnal.nZ;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    ballTaker->m_pScene->TraverseHero(BroadcastFunc);
    KGLogPrintf(KGLOG_INFO, "DoPassBallTo Send\n");

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAimAt(KHero* pHero,DWORD dwHoldObjID, BOOL bSyncSelf)
{
    BOOL            bResult = false;
    KS2C_Aimat      Pak;
    KBroadcastFunc  BroadcastFunc; 

    KGLOG_PROCESS_ERROR(pHero);

    Pak.protocolID  = s2c_aimat;
    Pak.id          = pHero->m_dwID;
    Pak.aimat       = pHero->m_bAimAt;
    Pak.facedir     = (unsigned char)pHero->m_eFaceDir;
    Pak.holdingObjID = dwHoldObjID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = bSyncSelf ? ERROR_ID : pHero->m_dwID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoAttachObject(KHero* pHero, KDoodad* pObject)
{
    BOOL                bResult = false;
    KS2C_AttachDoodad   Pak;
    KBroadcastFunc      BroadcastFunc; 
    KPOSITION           cPos;

    KGLOG_PROCESS_ERROR(pHero);
    KGLOG_PROCESS_ERROR(pObject);

    cPos = pHero->GetPosition();

    Pak.protocolID      = s2c_attach_doodad;
    Pak.idBeAttacher    = pHero->m_dwID;
    Pak.beAttacherVirtualFrame  = pHero->m_nVirtualFrame;
    Pak.nCurrentMoveCount = pHero->m_MoveCount;
    Pak.idObject        = pObject->m_dwID; 
    Pak.attacherX       = cPos.nX;
    Pak.attacherY       = cPos.nY;
    Pak.attacherZ       = cPos.nZ;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    //KGLogPrintf(KGLOG_INFO, "Broadcast player:%d attach object %d.", pHero->m_dwID, pObject->m_dwID);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncScore(KScene* pScene, DWORD dwTeamID, DWORD dwScore)
{
    BOOL            bResult = false;
    KS2C_SyncScore  Pak;
    KBroadcastFunc  BroadcastFunc; 

    assert(pScene);

    Pak.protocolID      = s2c_sync_score;
    Pak.teamID          = BYTE(dwTeamID);
    Pak.score           = dwScore;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

    //KGLogPrintf(KGLOG_INFO, "Broadcast score, teamID %d, score %d.", dwTeamID, dwScore);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncScoreToOneHero(int nConnIndex, DWORD dwTeamID, DWORD dwScore)
{
    BOOL               bResult     = false;
    BOOL               bRetCode    = false;
    KS2C_SyncScore     Pak;

    Pak.protocolID  = s2c_sync_score;
    Pak.teamID = BYTE(dwTeamID);
    Pak.score = dwScore;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoHeroStandOn(KHero* pHero, KDoodad* pObject)
{
    BOOL                bResult     = false;
    KS2C_HeroStandOn    Pak;
    KBroadcastFunc      BroadcastFunc; 
    KPOSITION           cPos;

    assert(pHero);
    assert(pObject);
    assert(pHero->m_pScene);

    cPos = pObject->GetPosition();

    Pak.protocolID			= s2c_hero_standon;
    Pak.playerID			= pHero->m_dwID;
    Pak.nCurrentMoveCount   = pHero->m_MoveCount;
    Pak.objectID            = pObject->m_dwID;
    Pak.x                   = cPos.nX;
    Pak.y                   = cPos.nY;
    Pak.z                   = cPos.nZ;
    Pak.nPlayerVirtualFrame = pHero->m_nVirtualFrame;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    KGLogPrintf(KGLOG_INFO, "Broadcast character %d stand on object %d.", pHero->m_dwID, pObject->m_dwID);

    bResult = true;

    return bResult;
}

BOOL KPlayerServer::DoBattleStart(KScene* pScene)
{
    BOOL                   bResult     = false;
    KS2C_BattleStart       Pak;
    KBroadcastFunc         BroadcastFunc; 

    assert(pScene);

    Pak.protocolID = s2c_battle_start;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

    KGLogPrintf(KGLOG_INFO, "Broadcast battle start.");

    bResult = true;

    return bResult;
}

BOOL KPlayerServer::DoMissionBattleFinished(
    int nConnIndex, int nSelfSide, int (&rTotalScores)[sidTotal], 
    const KMISSION_AWARD_DATA(& rAwardData)[MAX_TEAM_MEMBER_COUNT], 
    KHero*(& rTeammates)[MAX_TEAM_MEMBER_COUNT], BOOL (&bHasAward)[3], int nGoldCostMoney
)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KS2C_Mission_Battle_Finished Pak;

    memset(&Pak, 0, sizeof(Pak));
    Pak.protocolID      = s2c_mission_battle_finished;
    Pak.bySelfSide      = (BYTE)nSelfSide;
    Pak.goldCostMoney   = (unsigned short)nGoldCostMoney;
    Pak.totalScores[0]  = (unsigned short)rTotalScores[0];
    Pak.totalScores[1]  = (unsigned short)rTotalScores[1];

    for (int i = 0; i < MAX_TEAM_MEMBER_COUNT; ++i)
    {
        KPlayer* pOwner = NULL;
        KHero* pHero = rTeammates[i];

        if (!pHero)
            continue;
		
		Pak.playerID[i] = ERROR_ID;
		
        pOwner = pHero->GetOwner();
        if (pOwner)
        {
            strncpy(Pak.playerName[i], pOwner->m_szName, countof(Pak.playerName[i]));
            Pak.playerName[i][countof(Pak.playerName[i]) - 1] = '\0';
            Pak.playerID[i] = pOwner->getID();;
        }

        Pak.awardedHeroExp[i]   = rAwardData[i].m_nHeroExp;
        Pak.expMultiple[i]      = rAwardData[i].m_nExpMultiple;
        Pak.awardedMoney[i]     = rAwardData[i].m_nMoney;
        Pak.heroTemplateID[i]   = pHero->m_dwTemplateID;
        Pak.isAI[i]             = (unsigned char)pHero->IsAssistHero();
    }

    for (int i = 0; i < countof(bHasAward); ++i)
        Pak.hasAward[i] = (BYTE)bHasAward[i];

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoFreePVPBattleFinished(int nConnIndex, int nSelfSide, int (&rTotalScores)[sidTotal], KHero* (& rTeammates)[MAX_TEAM_MEMBER_COUNT])
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    KHero*      pHero       = NULL;
    KPlayer*    pOwner      = NULL;
    KS2C_FreePVP_Battle_Finished Pak;

    assert(countof(Pak.heroTemplateID) == MAX_TEAM_MEMBER_COUNT);

    memset(&Pak, 0, sizeof(Pak));

    Pak.protocolID      = s2c_freepvp_battle_finished;
    Pak.bySelfSide      = (BYTE)nSelfSide;
    Pak.totalScores[0]  = (unsigned short)rTotalScores[0];
    Pak.totalScores[1]  = (unsigned short)rTotalScores[1]; 

    for (int i = 0; i < MAX_TEAM_MEMBER_COUNT; ++i)
    {
        pHero = rTeammates[i];
        if (!pHero)
            continue;

        pOwner = pHero->GetOwner();
        KGLOG_PROCESS_ERROR(pOwner);

        strncpy(Pak.playerName[i], pOwner->m_szName, countof(Pak.playerName[i]));
        Pak.playerName[i][countof(Pak.playerName[i]) - 1] = '\0';
        Pak.playerID[i] = pOwner->getID();

        Pak.heroTemplateID[i]       = pHero->m_dwTemplateID;
        Pak.memeberScores[i]        = (unsigned short)pHero->m_GameStat.m_nScore;
        Pak.knockDown[i]            = (unsigned short)pHero->m_GameStat.m_nKnockDownOther;
        Pak.normalShoot[i]          = (unsigned short)pHero->m_GameStat.m_nNormalShoot;
        Pak.slamBall[i]             = (unsigned short)pHero->m_GameStat.m_nSlamBall;
        Pak.passBall[i]             = (unsigned short)pHero->m_GameStat.m_nPassBall;
        Pak.skillShoot[i]           = (unsigned short)pHero->m_GameStat.m_nSkillShoot;
        Pak.useItem[i]              = (unsigned short)(pHero->m_GameStat.m_nUseSceneDoodad + pHero->m_GameStat.m_nUseItemCount);
        Pak.pluginBasket[i]         = (unsigned short)pHero->m_GameStat.m_nPluginBasket;
        Pak.isAI[i]                 = (unsigned char)pHero->IsAssistHero();
    }

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoLadderBattleFinished(
    int nConnIndex, int nSelfSide, int (&rTotalScores)[sidTotal], 
    const KLADDER_AWARD_DATA(& rAwardData)[MAX_TEAM_MEMBER_COUNT], 
    KHero* (& rTeammates)[MAX_TEAM_MEMBER_COUNT], BOOL (&bHasAward)[3], int nGoldCostMoney
)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Ladder_Battle_Finished Pak;

    memset(&Pak, 0, sizeof(KS2C_Ladder_Battle_Finished));

    Pak.protocolID      = s2c_ladder_battle_finished;
    Pak.bySelfSide      = (BYTE)nSelfSide;
    Pak.totalScores[0]  = (unsigned short)rTotalScores[0];
    Pak.totalScores[1]  = (unsigned short)rTotalScores[1];

    for (int i = 0; i < countof(bHasAward); ++i)
        Pak.hasAward[i] = (BYTE)bHasAward[i];

    for (int i = 0; i < MAX_TEAM_MEMBER_COUNT; ++i)
    {
        KPlayer* pOwner = NULL;
        KHero* pHero = rTeammates[i];

        if (!pHero)
            continue;

        pOwner = pHero->GetOwner();
        if (pOwner)
        {
            strncpy(Pak.playerName[i], pOwner->m_szName, countof(Pak.playerName[i]));
            Pak.playerName[i][countof(Pak.playerName[i]) - 1] = '\0';
            Pak.playerID[i] = pOwner->getID();;
        }
        else
        {
            strncpy(Pak.playerName[i], pHero->m_pScene->m_Param.m_Name[pHero->m_nSide][pHero->m_nPos], countof(Pak.playerName[i]));
            Pak.playerName[i][countof(Pak.playerName[i]) - 1] = '\0';
            Pak.playerID[i] = 0;
        }

        Pak.heroTemplateID[i]       = (unsigned short)pHero->m_dwTemplateID;
        Pak.awardedLadderExp[i]     = (unsigned short)rAwardData[i].m_nAwardedLadderExp;
        Pak.awardedMenterPoint[i]   = (unsigned short)rAwardData[i].m_nAwardedMenterPoint;
        Pak.awardedHeroExp[i]       = rAwardData[i].m_nHeroExp;
    }
    Pak.goldCostMoney = (unsigned short)nGoldCostMoney;

    bRetCode = Send(nConnIndex, &Pak, sizeof(KS2C_Ladder_Battle_Finished));
    KGLOG_PROCESS_ERROR(bRetCode);
    
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncStartedFrame(int nStartedFrame, int nTotalFrame, int nLeftFrame, int nConnIndex, KScene* pScene)
{
    BOOL                       bResult     = false;
    BOOL                       bRetCode    = false;
    KS2C_BattleStartedFrame    Pak;
    KBroadcastFunc             BroadcastFunc; 

    assert(nStartedFrame >= 0);
    assert(nTotalFrame >= 0);

    Pak.protocolID = s2c_battle_startedframe;
    Pak.startFrame = g_pSO3World->m_nGameLoop;
    Pak.startedFrame = nStartedFrame;
    Pak.totalFrame = nTotalFrame;
    Pak.leftFrame = nLeftFrame;

    if (nConnIndex != -1)
    {
        bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        assert(pScene);
        BroadcastFunc.m_pvData         = &Pak;
        BroadcastFunc.m_uSize          = sizeof(Pak);
        BroadcastFunc.m_dwExclusive    = 0;
        BroadcastFunc.m_dwExcept       = ERROR_ID;
        pScene->TraverseHero(BroadcastFunc);
    }

    KGLogPrintf(KGLOG_INFO, "Broadcast battle start frame %d.", nStartedFrame);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoObjStandObj(KDoodad* pSource, KDoodad* pDest, int nConnIndex)
{
    BOOL                       bResult     = false;
    BOOL                       bRetCode    = false;
    KS2C_ObjOnObj              Pak;
    KBroadcastFunc             BroadcastFunc; 
    KPOSITION                  cPos;

    assert(pSource && pDest);
    assert(pSource->m_pScene);

    cPos = pDest->GetPosition();

    Pak.protocolID = s2c_obj_on_obj;
    Pak.objBeStanddID = pDest->m_dwID;
    Pak.objStandID = pSource->m_dwID;
    Pak.objBeStandx = cPos.nX;
    Pak.objBeStandy = cPos.nY;
    Pak.objBeStandz = cPos.nZ;

    if (nConnIndex == -1)
    {
        BroadcastFunc.m_pvData         = &Pak;
        BroadcastFunc.m_uSize          = sizeof(Pak);
        BroadcastFunc.m_dwExclusive    = 0;
        BroadcastFunc.m_dwExcept       = ERROR_ID;
        pSource->m_pScene->TraverseHero(BroadcastFunc);
    }
    else
    {
        Send(nConnIndex, &Pak, sizeof(Pak));
    }

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncCreateRoomRespond(int nConnIndex, DWORD dwRoomID, DWORD dwMapID, BOOL bSuccess)
{
    BOOL               bResult     = false;
    BOOL               bRetCode    = false;
    KS2C_SyncCreateRoomRespond Pak;

    Pak.protocolID  = s2c_sync_create_room_respond;
    Pak.dwRoomID = dwRoomID;
    Pak.dwMapID = dwMapID;
    Pak.bSuccess = (BYTE)bSuccess;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncRoomInfoStart(int nConnIndex, int nPageIndex)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_SyncRoomInfoStart Pak;

    Pak.protocolID = s2c_sync_room_info_start;
    Pak.wPageIndex = (WORD)nPageIndex;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncRoomBaseInfo(int nConnIndex, int nPageIndex, DWORD dwRoomID, const char szRoomName[], DWORD dwMapID, 
    int nMemberCount, bool bHasPassword, bool bFighting)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_SyncRoomBaseInfo Pak;

    Pak.protocolID = s2c_sync_one_room_base_info;
    Pak.wPageIndex = (WORD)nPageIndex;
    Pak.dwRoomID = dwRoomID;
    Pak.dwMapID  = dwMapID;
    Pak.byMemberCount = (BYTE)nMemberCount;
    Pak.bHasPassword = bHasPassword;
    Pak.bFighting = bFighting;

    strncpy(Pak.szRoomName, szRoomName, countof(Pak.szRoomName));
    Pak.szRoomName[countof(Pak.szRoomName) - 1] = '\0';

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncRoomInfoEnd(int nConnIndex, unsigned uTotalPage)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_SyncRoomInfoEnd Pak;

    Pak.protocolID  = s2c_sync_room_info_end;
    Pak.totalPage   = uTotalPage;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncOneRoomMemberPosInfo(int nConnIndex, DWORD dwPlayerID, KSIDE_TYPE eSide, int nPos)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_SyncOneTeamMemberPosInfo Pak;

    Pak.protocolID  = s2c_sync_oneteammember_posinfo;
    Pak.dwPlayerID  = dwPlayerID;
    Pak.bySide      = (BYTE)eSide;
    Pak.nPos        = nPos;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoApplyJoinRoomRespond(int nConnIndex, BOOL bSuccess, DWORD dwRoomID, int nTid, int nPosition, KS2C_ROOM_MEMBER_INFO& rNewMember)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_ApplyJoinRoomRespond Pak;

    Pak.protocolID      = s2c_apply_join_room_respond;
    Pak.bySuccess       = (BYTE)bSuccess;
    Pak.dwRoomID        = dwRoomID;
    Pak.byTid           = (BYTE)nTid;
    Pak.byPosition      = (BYTE)nPosition;
    Pak.newMember       = rNewMember;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoApplyLeaveRoomRespond(int nConnIndex, DWORD dwRoomID, DWORD dwLeaverID, int nSide, int nPos)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_ApplyLeaveRoomRespond Pak;

    Pak.protocolID  = s2c_apply_leave_room_respond;
    Pak.dwLeaverID  = dwLeaverID;
    Pak.dwRoomID    = dwRoomID;
    Pak.bySide      = (BYTE)nSide;
    Pak.byPos       = (BYTE)nPos;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoApplySwitchRoomHostRespond(int nConnIndex, BOOL bSuccess, DWORD dwRoomID, DWORD dwNewHostID)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_ApplySwitchRoomHostRespond Pak;

    Pak.protocolID = s2c_apply_switch_room_host_respond;
    Pak.bySuccess = (BYTE)bSuccess;
    Pak.dwRoomID = dwRoomID;
    Pak.dwNewHostID = dwNewHostID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoApplySwitchBattleMapRespond(int nConnIndex, BOOL bSuccess, DWORD dwNewMapID)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_ApplySwitchBattleMapRespond    Pak;

    Pak.protocolID = s2c_apply_switch_battle_map_respond;
    Pak.bySuccess = (BYTE)bSuccess;
    Pak.dwNewMapID = dwNewMapID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoApplySetRoomPassword(int nConnIndex, BOOL bSuccess, const char(& cszPassword)[cdRoomPasswordMaxLen])
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_ApplySetRoomPasswordRespond    Pak;

    Pak.protocolID = s2c_apply_set_room_password_respond;
    Pak.bySuccess = (BYTE)bSuccess;
    memcpy(Pak.szPassword, cszPassword, sizeof(Pak.szPassword));

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoApplySetReadyRespond(int nConnIndex, DWORD dwRoomID, DWORD dwReadyPlayerID, BOOL bReady)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_ApplySetReadyRespond Pak;

    Pak.protocolID      = s2c_apply_set_ready_respond;
    Pak.dwRoomID        = dwRoomID;
    Pak.dwReadyPlayerID = dwReadyPlayerID;
    Pak.byReady         = (BYTE)bReady;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncRoomName(int nConnIndex, DWORD dwRoomID, const char szName[])
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_SyncRoomName Pak;

    Pak.protocolID = s2c_sync_roomname;
    Pak.dwRoomID = dwRoomID;
    strncpy(Pak.szRoomName, szName, countof(Pak.szRoomName));
    Pak.szRoomName[countof(Pak.szRoomName) - 1] = '\0';

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAutoMatchRespond(int nConnIndex, BYTE eRetCode, uint32_t dwPunishingMemberID, int nLeftSeconds)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_AutoMatchRespond Pak;

    Pak.protocolID          = s2c_automatch_respond;
    Pak.eRetCode            = eRetCode;
    Pak.punishingMemberID   = dwPunishingMemberID;
    Pak.wLeftSeconds        = (WORD)nLeftSeconds;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoEnterAutoMatchNotify(int nConnIndex)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_EnterAutoMatchNotify Pak;

    Pak.protocolID = s2c_enter_automatch_notify;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoLeaveAutoMatchNotify(int nConnIndex, BYTE byLeaveReason)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Leave_AutoMatch_Notify Pak;

    Pak.protocolID = s2c_leave_automatch_notify;
    Pak.byLeaveReason = byLeaveReason;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode); 

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoLandMineBlowupBoardcast(KScene* pScene, DWORD dwLandMineID)
{
    BOOL                   bResult = false;
    KS2C_LandMineBlowup    Pak;
    KBroadcastFunc         BroadcastFunc;

    assert(pScene);

    Pak.protocolID = s2c_landmine_blowup;
    Pak.dwLandMineID = dwLandMineID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
    //Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncPlayerListStart(int nConnIndex)
{
    BOOL                        bResult = false;
    BOOL                        bRetCode = false;
    KS2C_SyncPlayerList_Start   Pak;

    Pak.protocolID = s2c_sync_playerlist_start;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncPlayerInfo(int nConnIndex, KPlayer* pPlayer)
{
    BOOL                    bResult = false;
    BOOL                    bRetCode = false;
    KS2C_SyncPlayerInfo     Pak;
    int                     nMainHeroLevel = 0;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID  = s2c_sync_playerinfo;
    Pak.dwID        = pPlayer->m_dwID;

    nMainHeroLevel = pPlayer->GetMainHeroLevel();

    KGLOG_PROCESS_ERROR(nMainHeroLevel >= 0 && nMainHeroLevel <= UCHAR_MAX);
    Pak.level       = (BYTE)nMainHeroLevel;
    Pak.gender      = (BYTE)(pPlayer->m_eGender);
    Pak.isVIP       = pPlayer->IsVIP();
    Pak.vipLevel    = (BYTE)pPlayer->m_nVIPLevel;

    g_CStringCopy(Pak.playerName, pPlayer->m_szName);

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncPlayerListEnd(int nConnIndex, int nPageIndex)
{
    BOOL                        bResult = false;
    BOOL                        bRetCode = false;
    KS2C_SyncPlayerList_End     Pak;

    Pak.protocolID = s2c_sync_playerlist_end;
    Pak.pageIndex = nPageIndex;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoDoodadStuckHero(KDoodad* pDoodad, KHero* pHero)
{
    BOOL                bResult            = false;
    KS2C_StuckByClip    Pak;
    KBroadcastFunc      BroadcastFunc;

    KGLOG_PROCESS_ERROR(pDoodad);
    KGLOG_PROCESS_ERROR(pDoodad->m_pScene);

    Pak.protocolID              = s2c_stuck_by_doodad;
    Pak.idClip                  = pDoodad->m_dwID;
    if (pHero)
    {
        Pak.beStuckerVirtualFrame   = pHero->m_nVirtualFrame;
        Pak.idBeStucker             = pHero->m_dwID;
        Pak.nCurrentMoveCount       = pHero->m_MoveCount;
        Pak.stuckerX                = pHero->m_nX;
        Pak.stuckerY                = pHero->m_nY;
        Pak.stuckerZ                = pHero->m_nZ;
    }
    else
    {
        Pak.beStuckerVirtualFrame   = 0;
        Pak.idBeStucker             = 0;
        Pak.nCurrentMoveCount       = 0;
        Pak.stuckerX                = 0;
        Pak.stuckerY                = 0;
        Pak.stuckerZ                = 0;
    }

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pDoodad->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDoodadRelaseHero(KScene* pScene, DWORD dwDoodadID, DWORD dwHeroID)
{
    BOOL                    bResult            = false;
    KS2C_ReleaseFromClip    Pak;
    KBroadcastFunc          BroadcastFunc;

    assert(pScene);

    Pak.protocolID  = s2c_release_from_doodad;
    Pak.idClip      = dwDoodadID;
    Pak.idBeStucker = dwHeroID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;
    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUnTakeDoodad(DWORD dwTakerID, KMovableObject* pObj)
{
    BOOL            bResult      = false;
    BOOL            bRetCode     = false;
    KS2C_UnTakeDoodad Pak;
    KBroadcastFunc  BroadcastFunc;
    KPOSITION       cPos;

    assert(pObj && pObj->m_pScene);

    cPos = pObj->GetPosition();

    Pak.protocolID  = s2c_untake_doodad;
    Pak.idTaker     = dwTakerID;
    Pak.idObject    = pObj->m_dwID;
    Pak.x           = cPos.nX;
    Pak.y           = cPos.nY;
    Pak.z           = cPos.nZ;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pObj->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoRemoveGuide(KHero* pHero)
{
    BOOL            bResult      = false;
    BOOL            bRetCode     = false;
    KS2C_RemoveGuide Pak;
    KBroadcastFunc  BroadcastFunc;
    KPOSITION       cPos;

    assert(pHero && pHero->m_pScene);

    cPos = pHero->GetPosition();

    Pak.protocolID  = s2c_remove_guide;
    Pak.id          = pHero->m_dwID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSendToClient(int nConnIndex, BYTE* pbyData, size_t uDataLen)
{
    return Send(nConnIndex, pbyData, uDataLen);
}
    
BOOL KPlayerServer::DoSendToPlayer(DWORD dwPlayerID, BYTE* pbyData, size_t uDataLen)
{
    BOOL        bResult = false;
    KPlayer*    pPlayer = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    if (pPlayer)
    {
        bResult = Send(pPlayer->m_nConnIndex, pbyData, uDataLen);
        goto Exit0;
    }
    
    //通过logicserver转发
    g_LSClient.DoSendToPlayer(dwPlayerID, pbyData, uDataLen); 
	
	bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoRequestFailed(int nConnIndex, int nRequestID, DWORD dwErrorCode)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_RequestFailed Pak;

    Pak.protocolID  = s2c_request_failed;
    Pak.requestID   = nRequestID;
    Pak.errorCode   = dwErrorCode;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoTalkMessage(int nMsgType, DWORD dwTalkerID, const char cszTalker[], DWORD dwTalkHeroID, int nTalkGroupID, DWORD dwReceiverID, const char cszReceiver[], size_t uDataLen, const char cszTalkData[])
{
    BOOL                        bResult             = false;
    BOOL                        bRetCode            = false;
    size_t                      uNameLen            = 0;
    size_t                      uPakSize            = 0;
    KS2C_TalkMessage*           pTalkMsg            = (KS2C_TalkMessage*)m_byTempData;
    KPlayer*                    pTalker             = NULL;
    KPlayer*                    pReceiver           = NULL;
    KScene*                     pScene              = NULL;
    KBroadcastFunc              BroadcastFunc;
    KGroupTalkFunc              GroupTalkFunc;

    KGLOG_PROCESS_ERROR(nMsgType > trInvalid && nMsgType < trTotal);
    assert(uDataLen);

    KGLOG_PROCESS_ERROR(MAX_CHAT_TEXT_LEN <= MAX_EXTERNAL_PACKAGE_SIZE);

    pTalkMsg->protocolID    = s2c_talk_message;
    pTalkMsg->uTalkerID     = dwTalkerID;
	pTalkMsg->uHeroID		= (WORD)dwTalkHeroID;
    pTalkMsg->byMsgType     = (BYTE)nMsgType;

    switch (nMsgType)
    {
    case trFace:
        break;

    case trSmallHorn:
        {
            uNameLen = strlen(cszTalker);
            uPakSize = sizeof(KS2C_TalkMessage) + sizeof(BYTE) + uNameLen + uDataLen;
            KGLOG_PROCESS_ERROR(uPakSize <= MAX_CHAT_TEXT_LEN && uNameLen <= UCHAR_MAX);
            pTalkMsg->talkdataLength = (unsigned short)(sizeof(BYTE) + uNameLen + uDataLen);
            pTalkMsg->talkdata[0] = (BYTE)uNameLen;
            memcpy(pTalkMsg->talkdata + 1, cszTalker, uNameLen);
            memcpy(pTalkMsg->talkdata + 1 + uNameLen, cszTalkData, uDataLen);

            GroupTalkFunc.m_pvData         = pTalkMsg;
            GroupTalkFunc.m_uSize          = uPakSize;
            GroupTalkFunc.m_nGroupID       = nTalkGroupID;

            g_pSO3World->m_PlayerSet.Traverse(GroupTalkFunc);
        }
        break;

    case trGlobalSys:
    case trGmAnnounce:
    case trPhysicalServer:
    case trWorld:
    case trBigHorn:
        {
            uNameLen = strlen(cszTalker);
            uPakSize = sizeof(KS2C_TalkMessage) + sizeof(BYTE) + uNameLen + uDataLen;
            KGLOG_PROCESS_ERROR(uPakSize <= MAX_CHAT_TEXT_LEN && uNameLen <= UCHAR_MAX);
            pTalkMsg->talkdataLength = (unsigned short)(sizeof(BYTE) + uNameLen + uDataLen);
            pTalkMsg->talkdata[0] = (BYTE)uNameLen;
            memcpy(pTalkMsg->talkdata + 1, cszTalker, uNameLen);
            memcpy(pTalkMsg->talkdata + 1 + uNameLen, cszTalkData, uDataLen);

            BroadcastFunc.m_pvData         = pTalkMsg;
            BroadcastFunc.m_uSize          = uPakSize;
            BroadcastFunc.m_dwExclusive    = 0;
            BroadcastFunc.m_dwExcept       = ERROR_ID;

            g_pSO3World->m_PlayerSet.Traverse(BroadcastFunc);
        }
        break;

    case trRoom:
    case trTeam:
    case trWhisper:
        {
            KGBlackNode*    pBlackNode  = NULL;     

            //KGLOG_PROCESS_ERROR(dwTalkerID);
            KGLOG_PROCESS_ERROR(dwReceiverID);

            // 接收人在说话人的黑名单中
            pBlackNode = g_pSO3World->m_FellowshipMgr.GetBlackListNode(dwTalkerID, dwReceiverID);
            if (pBlackNode)
            {
                KMessage23_Para param;
                strncpy(param.targetName, cszReceiver, countof(param.targetName));
                param.targetName[countof(param.targetName) - 1] = '\0';

                pTalker = g_pSO3World->m_PlayerSet.GetObj(dwTalkerID);
                KGLOG_PROCESS_ERROR(pTalker);

                DoDownwardNotify(pTalker, KMESSAGE_TARGET_IN_YOUR_BLACKLIST, &param, sizeof(param));
                goto Exit1;
            }

            // 说话人在接收人的黑名单中
            pBlackNode = g_pSO3World->m_FellowshipMgr.GetBlackListNode(dwReceiverID, dwTalkerID);
            if (pBlackNode)
            {
                KMessage24_Para param;
                strncpy(param.targetName, cszReceiver, countof(param.targetName));
                param.targetName[countof(param.targetName) - 1] = '\0';

                pTalker = g_pSO3World->m_PlayerSet.GetObj(dwTalkerID);
                KGLOG_PROCESS_ERROR(pTalker);

                DoDownwardNotify(pTalker, KMESSAGE_YOU_IN_TARGET_BLACKLIST, &param, sizeof(param));
                goto Exit1;
            }

            pReceiver = g_pSO3World->m_PlayerSet.GetObj(dwReceiverID);
            KG_PROCESS_ERROR(pReceiver);

            uNameLen = strlen(cszTalker);
            uPakSize = sizeof(KS2C_TalkMessage) + sizeof(BYTE) + uNameLen + uDataLen;
            KGLOG_PROCESS_ERROR(uPakSize <= MAX_CHAT_TEXT_LEN && uNameLen <= UCHAR_MAX);
            pTalkMsg->talkdataLength = (unsigned short)(sizeof(BYTE) + uNameLen + uDataLen);
            pTalkMsg->talkdata[0] = (BYTE)uNameLen;
            memcpy(pTalkMsg->talkdata + 1, cszTalker, uNameLen);
            memcpy(pTalkMsg->talkdata + 1 + uNameLen, cszTalkData, uDataLen);
            Send(pReceiver->m_nConnIndex, pTalkMsg, uPakSize);

            if (nMsgType == trWhisper)
                g_LSClient.DoWhisperSuccess(nMsgType, dwTalkerID, cszTalker, dwReceiverID, cszReceiver, uDataLen, cszTalkData);
        }
        break;
    case trLocalSys:
    case trGmMessage:
    case trToPlayerGmAnnounce:
        {
            uPakSize = sizeof(KS2C_TalkMessage) + uDataLen;

            memcpy(pTalkMsg->talkdata, cszTalkData, uDataLen);

            pReceiver = g_pSO3World->m_PlayerSet.GetObj(dwReceiverID);
            KGLOG_PROCESS_ERROR(pReceiver);

            Send(pReceiver->m_nConnIndex, pTalkMsg, uPakSize);
        }
        break;

    default:
        KGLOG_PROCESS_ERROR(!"Undefined talk type !");
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncOneFellowship(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
{
    BOOL                bResult         = false;
    KGFellowship*       pFellowship     = NULL;
    KFellowShipData     Data;

    pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(dwPlayerID, dwAlliedPlayerID);
    KGLOG_PROCESS_ERROR(pFellowship);

    Data.dwAlliedPlayerID   = dwAlliedPlayerID;
    Data.nLevel             = pFellowship->m_nPlayerLevel;
    Data.byGroupID          = (BYTE)pFellowship->m_dwGroupID;
    Data.byOnline           = (BYTE)pFellowship->m_bOnline;
    Data.byVIPLevel         = (BYTE)pFellowship->m_nVIPLevel;
    Data.byIsVIP            = (BYTE)pFellowship->m_bIsVIP;
    Data.nServerID         = pFellowship->m_nServerID;
    Data.byGender           = (BYTE)pFellowship->m_eGender;
    Data.byLadderLevel      = (BYTE)pFellowship->m_nLadderLevel;
    Data.byTeamLogo         = (BYTE)pFellowship->m_nTeamLogo;
    Data.byTeamLogoBg       = (BYTE)pFellowship->m_nTeamLogoBg;

    strncpy(Data.szName, pFellowship->m_szName, sizeof(Data.szName));
    Data.szName[sizeof(Data.szName) - sizeof('\0')] = '\0';

    strncpy(Data.szRemark, pFellowship->m_szRemark, sizeof(Data.szRemark));
    Data.szRemark[sizeof(Data.szRemark) - sizeof('\0')] = '\0';

    g_PlayerServer.DoSyncFellowshipList(dwPlayerID, 1, &Data);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncOneBlackList(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
{
    BOOL                bResult     = false;
    KGBlackNode*        pBlackNode  = NULL;
    KBlackListData      Data;

    pBlackNode = g_pSO3World->m_FellowshipMgr.GetBlackListNode(dwPlayerID, dwAlliedPlayerID);
    KGLOG_PROCESS_ERROR(pBlackNode);

    Data.dwAlliedPlayerID = dwAlliedPlayerID;

    strncpy(Data.szName, pBlackNode->m_szName, sizeof(Data.szName));
    Data.szName[sizeof(Data.szName) - sizeof('\0')] = '\0';

    g_PlayerServer.DoSyncBlackList(dwPlayerID, 1, &Data);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncFellowshipList(DWORD dwPlayerID, int nCount, KFellowShipData* pData)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    KPlayer*                    pPlayer         = NULL;
    KS2C_Sync_Fellowship_List*  pSyncPak        = (KS2C_Sync_Fellowship_List*)m_byTempData;
    int                         nSendCount      = 0;
    int                         nTotalIndex     = 0;
    int                         nSendMax        = 0;
    KFellowShipData*            pFellowshipList = NULL;

    KGLOG_PROCESS_ERROR(sizeof(KS2C_Sync_Fellowship_List) <= MAX_EXTERNAL_PACKAGE_SIZE);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_SUCCESS(!pPlayer);

    pSyncPak->protocolID  = s2c_sync_fellowship_list;

    if (nCount == 0)
    {
        pSyncPak->fellowshipdataLength  = 0;

        bRetCode = Send(pPlayer->m_nConnIndex, pSyncPak, sizeof(KS2C_Sync_Fellowship_List));
        KG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    nSendMax = (MAX_EXTERNAL_PACKAGE_SIZE - sizeof(KS2C_Sync_Fellowship_List)) / sizeof(KFellowShipData);

    while (nCount > 0) // 分包发送客户端
    {
        nSendCount = nCount >= nSendMax ? nSendMax : nCount;
        nCount -= nSendCount;

        int nPackLength = (WORD)(sizeof(KS2C_Sync_Fellowship_List) + sizeof(KFellowShipData) * nSendCount);

        pSyncPak->fellowshipdataLength = (char)nSendCount;
        pFellowshipList = (KFellowShipData*)pSyncPak->fellowshipdata;

        for (int nIndex = 0; nIndex < nSendCount; nIndex++)
        {
            memcpy(pFellowshipList + nIndex, pData + nTotalIndex, sizeof(KFellowShipData));
            nTotalIndex++;
        }

        bRetCode = Send(pPlayer->m_nConnIndex, pSyncPak, nPackLength);
        KG_PROCESS_ERROR(bRetCode);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncBlackList(DWORD dwPlayerID, int nCount, KBlackListData* pData)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    KPlayer*                    pPlayer         = NULL;
    KS2C_Sync_Black_List*       pSyncPak        = (KS2C_Sync_Black_List*)m_byTempData;
    int                         nSendCount      = 0;
    int                         nTotalIndex     = 0;
    int                         nSendMax        = 0;
    KBlackListData*             pFellowshipList = NULL;

    KGLOG_PROCESS_ERROR(sizeof(KS2C_Sync_Black_List) <= MAX_EXTERNAL_PACKAGE_SIZE);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_SUCCESS(!pPlayer);

    pSyncPak->protocolID  = s2c_sync_black_list;

    if (nCount == 0)
    {
        pSyncPak->blacklistdataLength  = 0;

        bRetCode = Send(pPlayer->m_nConnIndex, pSyncPak, sizeof(KS2C_Sync_Black_List));
        KG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    nSendMax = (MAX_EXTERNAL_PACKAGE_SIZE - sizeof(KS2C_Sync_Black_List)) / sizeof(KBlackListData);

    while (nCount > 0) // 分包发送客户端
    {
        nSendCount = nCount >= nSendMax ? nSendMax : nCount;
        nCount -= nSendCount;

        int nPackLength = (WORD)(sizeof(KS2C_Sync_Fellowship_List) + sizeof(KBlackListData) * nSendCount);
        pSyncPak->blacklistdataLength = (unsigned short)nSendCount;
        pFellowshipList = pSyncPak->blacklistdata;

        for (int nIndex = 0; nIndex < nSendCount; nIndex++)
        {
            memcpy(pFellowshipList + nIndex, pData + nTotalIndex, sizeof(KBlackListData));
            nTotalIndex++;
        }

        bRetCode = Send(pPlayer->m_nConnIndex, pSyncPak, nPackLength);
        KG_PROCESS_ERROR(bRetCode);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncFellowshipGroupName(DWORD dwPlayerID)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KPlayer*                        pPlayer     = NULL;
    KS2C_Sync_Fellowship_Group_Name* pSyncPak   = (KS2C_Sync_Fellowship_Group_Name*)m_byTempData;
    size_t                          uPakLength  = 0;
    KG_FELLOWSHIP_GROUPNAMES_MAP::iterator  it;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    it = g_pSO3World->m_FellowshipMgr.m_GroupNamesMap.find(dwPlayerID);
    KG_PROCESS_ERROR(it != g_pSO3World->m_FellowshipMgr.m_GroupNamesMap.end());

    pSyncPak->protocolID = s2c_sync_fellowship_group_name;

    assert(it->second.nGroupCount <= KG_FELLOWSHIP_MAX_CUSTEM_GROUP);

    pSyncPak->groupnameLength = (unsigned short)it->second.nGroupCount;
    for (int i = 0; i < it->second.nGroupCount; i++)
    {
        strncpy(pSyncPak->groupname[i], it->second.szGroupName[i], _NAME_LEN);
        pSyncPak->groupname[i][_NAME_LEN - 1] = '\0';
    }

    uPakLength = sizeof(KS2C_Sync_Fellowship_Group_Name) + sizeof(pSyncPak->groupname[0]) * pSyncPak->groupnameLength;
    bRetCode = Send(pPlayer->m_nConnIndex, pSyncPak, uPakLength);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDelFellowshipRespond(int nConnIndex, DWORD dwAlliedPlayerID, int nType, BOOL bSuccess)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KS2C_Del_Fellowship_Respond Pak;

    Pak.protocolID = s2c_del_fellowship_respond;
    Pak.dwAlliedPlayerID = dwAlliedPlayerID;
    Pak.byType = (BYTE)nType;
    Pak.bySuccess = (BYTE)bSuccess;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUpdateFellowshipInfo(int nConnIndex, DWORD dwAlliedPlayerID, KGFellowship* pFellowship)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KS2C_Update_Fellowship_Info Pak;

    KGLOG_PROCESS_ERROR(pFellowship);

    Pak.protocolID          = s2c_update_fellowship_info;
    Pak.dwAlliedPlayerID    = dwAlliedPlayerID;
    Pak.nLevel              = pFellowship->m_nPlayerLevel;
    Pak.byOnline            = (BYTE)pFellowship->m_bOnline;
    Pak.byVIPLevel          = (BYTE)pFellowship->m_nVIPLevel;
    Pak.byIsVIP             = (BYTE)pFellowship->m_bIsVIP;
    Pak.nServerID          = pFellowship->m_nServerID;
    Pak.byGender            = (BYTE)pFellowship->m_eGender;
    Pak.byLadderLevel       = (BYTE)pFellowship->m_nLadderLevel;
    Pak.byTeamLogo          = (BYTE)pFellowship->m_nTeamLogo;
    Pak.byTeamLogoBg        = (BYTE)pFellowship->m_nTeamLogoBg;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoFirstPvPNotify(int nConnIndex, DWORD dwPlayerID, const char cszPlayerName[])
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KS2C_First_PvP_Notify Pak;

    Pak.protocolID  = s2c_first_pvp_notify;
    Pak.dwPlayerID  = dwPlayerID;
    COPY_SZ(Pak.szPlayerName, cszPlayerName);

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncItemData(
    int nConnIndex, DWORD dwPlayerID, IItem* piItem, int nPackageType, int nPackageIndex, int nPos
    )
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    size_t              	uBufferSize     = MAX_EXTERNAL_PACKAGE_SIZE - sizeof(KS2C_Sync_Item_Data);
    size_t              	uItemDataLen    = 0;
    KS2C_Sync_Item_Data* 	pSyncItemData   = (KS2C_Sync_Item_Data*)m_byTempData;

    KGLOG_PROCESS_ERROR(sizeof(KS2C_Sync_Item_Data) + sizeof(KEquipDataForClient) <= MAX_EXTERNAL_PACKAGE_SIZE);

    KGLOG_PROCESS_ERROR(piItem);

    pSyncItemData->protocolID       = s2c_sync_item_data;
    pSyncItemData->dwPlayerID       = dwPlayerID;
    pSyncItemData->nPackageType     = (char)nPackageType;
    pSyncItemData->nPackageIndex    = (char)nPackageIndex;
    pSyncItemData->nPos             = (char)nPos;

    bRetCode = piItem->GetBinaryDataForClient(&uItemDataLen, pSyncItemData->itemdata, uBufferSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    pSyncItemData->itemdataLength = (unsigned short)uItemDataLen;

    bRetCode = Send(nConnIndex, pSyncItemData, sizeof(KS2C_Sync_Item_Data) + uItemDataLen);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncQuestData(
    int nConnIndex, DWORD dwDestPlayerID,const BYTE* pbyData, size_t uDataLen, BYTE byDataType
    )
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    KS2C_Sync_Quest_Data*   pSyncQuestList  = (KS2C_Sync_Quest_Data*)m_byTempData;

    KGLOG_PROCESS_ERROR(sizeof(KS2C_Sync_Quest_Data) + uDataLen <= MAX_EXTERNAL_PACKAGE_SIZE);

    pSyncQuestList->protocolID      = s2c_sync_quest_data;
    pSyncQuestList->dwDestPlayerID  = dwDestPlayerID;
    pSyncQuestList->byQuestDataType = byDataType;
    pSyncQuestList->byVersion       = 0;
    pSyncQuestList->questdataLength = (unsigned short)uDataLen;

    memcpy(pSyncQuestList->questdata, pbyData, uDataLen);

    bRetCode = Send(nConnIndex, pSyncQuestList, sizeof(KS2C_Sync_Quest_Data) + uDataLen);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAwardItemNotify(KPlayer* pPlayer, KAWARD_CARD_TYPE eType, KS2C_Award_Item awardItem, unsigned uAwardedIndex)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Award_Item_Notify Pak;

    Pak.protocolID  = s2c_award_item_notify;
    Pak.awardCardType = (BYTE)eType;
    Pak.awardedIndex = (unsigned char)uAwardedIndex;
    memcpy(&Pak.awardItem, &awardItem, sizeof(KS2C_Award_Item));

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAwardItemListNotify(KPlayer* pPlayer, KAWARD_CARD_TYPE eType, KS2C_Award_Item arrAwardList[], unsigned uCount)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Unreal_Award_Item_List_Notify Pak;

    KGLOG_PROCESS_ERROR(uCount == countof(Pak.awardList));

    Pak.protocolID  = s2c_unreal_award_item_list_notify;
    Pak.awardCardType = (BYTE)eType;
    memcpy(Pak.awardList, arrAwardList, sizeof(Pak.awardList));

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAwardHeroExp(KPlayer* pPlayer, DWORD dwHeroTemplateID, int nExp)
{
    BOOL                bResult = false;
    BOOL                bRetCode = false;
    KS2C_Award_HeroExp  Pak;

    Pak.protocolID  = s2c_award_heroexp;
    Pak.templateID  = dwHeroTemplateID;
    Pak.exp         = nExp;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAwardMoney(KPlayer* pPlayer, ENUM_MONEY_TYPE eType, int nCount)
{
    BOOL                bResult = false;
    BOOL                bRetCode = false;
    KS2C_Award_Money  Pak;

    Pak.protocolID  = s2c_award_money;
    Pak.moneyType   = (unsigned char)eType;
    Pak.moneyCount  = nCount;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBuyGoodsRespond(KPlayer* pPlayer, int nShopID, int nGoodsID, BOOL bSuccess)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int     nGoodsStack = 0;
    KS2C_Buy_Goods_Respond Pak;

    nGoodsStack = g_pSO3World->m_ShopMgr.GetGoodsStack(nGoodsID);
    KGLOG_PROCESS_ERROR(nGoodsStack > 0);

    Pak.protocolID  = s2c_buy_goods_respond;
    Pak.ShopID      = nShopID;
    Pak.GoodsID     = nGoodsID;
    Pak.Stack       = nGoodsStack;
    Pak.Success     = bSuccess;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSellGoodsRespond(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime, BOOL bSuccess)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Sell_Goods_Respond Pak;

    Pak.protocolID      = s2c_sell_goods_respond;
    Pak.PackageType     = nPackageType;
    Pak.PackageIndex    = nPackageIndex;
    Pak.Pos             = nPos;
    Pak.TabType         = dwTabType;
    Pak.TabIndex        = dwTabIndex;
    Pak.GenTime         = nGenTime;
    Pak.Success         = bSuccess;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoRepairSingleEquipRespond(KPlayer* pPlayer,BOOL bSuccess)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Repair_Single_Equip_Respond Pak;

    Pak.protocolID  = s2c_repair_single_equip_respond;
    Pak.Success     = bSuccess;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoRepairAllEquipRespond(KPlayer* pPlayer, BOOL bSuccess)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Repair_All_Equip_Respond Pak;

    Pak.protocolID  = s2c_repair_all_equip_respond;
    Pak.Success     = bSuccess;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoIBActionResultNotify(KPlayer* pPlayer, BOOL bSuccessed)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    KS2C_IB_Action_Result_Notify Pak;

    Pak.protocolID  = s2c_ib_action_result_notify;
    Pak.bSuccessed  = bSuccessed;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBuyHeroRespond(int nConnIndex, DWORD dwHeroTemplateID)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_Buy_Hero_Respond	Pak;

    Pak.protocolID    		= s2c_buy_hero_respond;
    Pak.wHeroTemplateID 	= (WORD)dwHeroTemplateID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUnFireHeroRespond(int nConnIndex, DWORD dwHeroTemplateID)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_UnFire_Hero_Respond	Pak;

    Pak.protocolID    		= s2c_unfire_hero_respond;
    Pak.wHeroTemplateID 	= (WORD)dwHeroTemplateID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoFireHeroRespond(int nConnIndex, DWORD dwHeroTemplateID)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_Fire_Hero_Respond		Pak;

    Pak.protocolID    		= s2c_fire_hero_respond;
    Pak.wHeroTemplateID 	= (WORD)dwHeroTemplateID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMaxHeroCount(int nConnIndex, int nMaxHeroCount)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_Sync_Max_Hero_Count*   pPak        = (KS2C_Sync_Max_Hero_Count*)m_byTempData;

    pPak->protocolID    = s2c_sync_max_hero_count;
    pPak->wMaxHeroCount = (WORD)nMaxHeroCount;

    bRetCode = Send(nConnIndex, pPak, sizeof(KS2C_Sync_Max_Hero_Count));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncHeroData(int nConnIndex, KHeroData* pHeroData)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    uint32_t                uMsgSize        = sizeof(KS2C_Sync_Hero_Data); 
    uint32_t*               pPassiveSkill   = NULL;
    KS2C_Sync_Hero_Data*    pPak = (KS2C_Sync_Hero_Data*)m_byTempData;

    KGLOG_PROCESS_ERROR(pHeroData);

    pPak->protocolID   = s2c_sync_hero_data;
    pPak->wLevel       = (WORD)pHeroData->m_nLevel;
    pPak->wTemplateID  = (WORD)pHeroData->m_dwTemplateID;
    pPak->wFashionID   = pHeroData->m_wFashionID;
    pPak->nExp         = pHeroData->m_nExp;
    pPak->ladderLevel  = (WORD)pHeroData->m_nLadderLevel;
    pPak->ladderExp    = pHeroData->m_nLadderExp;
    pPak->wFreeTalent  = (WORD)pHeroData->m_wFreeTalent;
    pPak->wTalent[egymWeightLift] = pHeroData->m_wTalent[egymWeightLift];
    pPak->wTalent[egymRunning]    = pHeroData->m_wTalent[egymRunning];
    pPak->wTalent[egymBoxing]     = pHeroData->m_wTalent[egymBoxing];
    pPak->byFired = (BYTE)pHeroData->m_bFired;

    for (int i = 0; i < KACTIVE_SLOT_TYPE_TOTAL; ++i)
    {
        pPak->ActiveSkill[i] = (WORD)pHeroData->m_SelectedActvieSkill[i];
    }

    for (int i = 0; i < cdPassiveSkillSlotCount; ++i)
    {
        pPak->PassiveSkill[i] = (WORD)pHeroData->m_SelectedPassiveSkill[i];
    }

    for (int i = 0; i < cdNirvanaSkillSlotCount; ++i)
    {
        pPak->NirvanaSkill[i] = (WORD)pHeroData->m_SelectedNirvanaSkill[i];
    }

    assert(cdSlamBallSkillSlotCount == 1);
    pPak->SlamBallSkill = (WORD)pHeroData->m_SelectedSlamBallSkill[0];

    bRetCode = Send(nConnIndex, pPak, uMsgSize);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncUserPreferences(int nConnIndex, int nOffset, int nLength, BYTE* pbyData)
{
    BOOL                        bResult                 = false;
    BOOL                        bRetCode                = false;
    KS2C_Sync_User_Preferences*  pSyncUserPreferences    = (KS2C_Sync_User_Preferences*)m_byTempData;

    KGLOG_PROCESS_ERROR(nLength <= MAX_USER_PREFERENCES_LEN);
    KGLOG_PROCESS_ERROR(sizeof(KS2C_Sync_User_Preferences) + nLength <= MAX_EXTERNAL_PACKAGE_SIZE);

    pSyncUserPreferences->protocolID        = s2c_sync_user_preferences;
    pSyncUserPreferences->nOffset           = nOffset;
    pSyncUserPreferences->nLength           = nLength;
    pSyncUserPreferences->userdataLength    = (WORD)nLength;

    memcpy(pSyncUserPreferences->userdata, pbyData, (size_t)nLength);

    bRetCode = Send(nConnIndex, pSyncUserPreferences, sizeof(KS2C_Sync_User_Preferences) + nLength);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncPlayerPackageSize(int nConnIndex, int nPackageIndex, int nPackageSize)
{
    BOOL                                    bResult     = false;
    BOOL                                    bRetCode    = false;
    KS2C_Sync_Player_Package_Size           Sync;

    Sync.protocolID = s2c_sync_player_package_size;
    Sync.byIndex    = (BYTE)nPackageIndex;
    Sync.bySize     = (BYTE)nPackageSize;

    bRetCode = Send(nConnIndex, &Sync, sizeof(Sync));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoHeroLevelUp(int nConnIndex, DWORD dwHeroTemplateID, int nLevel)
{
    BOOL                bResult = false;
    BOOL                bRetCode = false;
    KS2C_Hero_LevelUp   Pak;

    Pak.protocolID      = s2c_hero_levelup;
    Pak.heroTemplateID  = dwHeroTemplateID;
    Pak.heroLevel       = nLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncHeroExp(int nConnIndex, DWORD dwHeroTemplateID, int nExp)
{
    BOOL                bResult = false;
    BOOL                bRetCode = false;
    KS2C_Sync_HeroExp   Pak;

    Pak.protocolID      = s2c_sync_heroexp;
    Pak.heroTemplateID  = dwHeroTemplateID;
    Pak.heroExp         = nExp;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncPlayerLevelExp(KPlayer* pPlayer, BOOL bLevelUp)
{
    BOOL                bResult = false;
    BOOL                bRetCode = false;
    KS2C_Sync_Player_Level_Exp   Pak;

    Pak.protocolID      = s2c_sync_player_level_exp;
    Pak.byLevel         = (unsigned char)pPlayer->m_nLevel;
    Pak.nExp            = pPlayer->m_nExp;
    Pak.byLevelUp       = (unsigned char)bLevelUp;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoRenameRespond(int nConnIndex, BYTE byCode, DWORD dwPlayerID, char szNewName[])
{
    BOOL                bResult     = false;
    BOOL                bRetCode    = false;
    KS2C_Rename_Respond Pak;

    Pak.protocolID  = s2c_rename_respond;
    Pak.byCode      = byCode;
    Pak.uPlayerID   = dwPlayerID;

    strncpy(Pak.szNewName, szNewName, countof(Pak.szNewName));
    Pak.szNewName[countof(Pak.szNewName) - 1] = '\0';

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSelectSkillRespond(int nConnIndex, BOOL bSuccess)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_Select_Skill_Respond   Pak;

    Pak.protocolID  = s2c_select_skill_respond;
    Pak.bSuccess    = bSuccess;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSendMailRespond(int nConnIndex, int nRespondID, int nResult)
{
    BOOL                    bResult  = false;
    BOOL                    bRetCode = false;
    KS2C_Send_Mail_Respond   Respond;

    Respond.protocolID = s2c_send_mail_respond;
    Respond.byRespondID  = (BYTE)nRespondID;
    Respond.byResult     = (BYTE)nResult;

    bRetCode = Send(nConnIndex, &Respond, sizeof(Respond));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoNewMailNotify(int nConnIndex)
{
	BOOL                        bResult     = false;
	BOOL                        bRetCode    = false;
	KS2C_New_Mail_Notify		Pak;

	Pak.protocolID            = s2c_new_mail_notify;	

	bRetCode = Send(nConnIndex, &Pak, sizeof(KS2C_New_Mail_Notify));
	KG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoGetMailListRespond(int nConnIndex, KMailListInfo* pMailListInfo, int nMailCount, int nMailTotalCount)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_Get_Mail_List_Respond*  pPak        = (KS2C_Get_Mail_List_Respond*)m_byTempData;
    size_t                      uSize       = 0;

    assert(pMailListInfo);
    assert(nMailCount >= 0);

    uSize = nMailCount * sizeof(KMailListInfo);

    KGLOG_PROCESS_ERROR(sizeof(KS2C_Get_Mail_List_Respond) + uSize < MAX_EXTERNAL_PACKAGE_SIZE);

    pPak->protocolID            = s2c_get_maillist_respond;
    pPak->byMailCount           = (BYTE)nMailCount;
    pPak->MailListInfoLength    = (unsigned short)nMailCount;
    pPak->wMailTotalCount       = (unsigned short)nMailTotalCount;

    if (nMailCount > 0)
    {
        memcpy(pPak->MailListInfo, pMailListInfo, uSize);
    }

    bRetCode = Send(nConnIndex, pPak, sizeof(KS2C_Get_Mail_List_Respond) + uSize);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMailContent(DWORD dwPlayerID, DWORD dwMailID, int nResult, KMailContent* pContentProtoBuf)
{
    BOOL                     bResult     = false;
    BOOL                     bRetCode    = false;
    KS2C_Sync_Mail_Content*   pRespond    = (KS2C_Sync_Mail_Content*)m_byTempData;
    KPlayer*                 pPlayer     = NULL;
    size_t                   uPakSize     = 0;
    size_t                   uContentLen = 0;
    KMailContent*            pContent    = NULL;
    BYTE*                    pbyItemDataSrc = NULL;
    BYTE*                    pbyItemDataDes = NULL;
    size_t                   uItemLen    = 0;
    T3DB::KPB_EQUI_DATA EquiData;

    KGLOG_PROCESS_ERROR(dwPlayerID);
    KGLOG_PROCESS_ERROR(dwMailID);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    pRespond->protocolID    = s2c_sync_mail_content;
    pRespond->dwMailID      = dwMailID;
    pRespond->byResult      = (BYTE)nResult;
    pRespond->contentLength = 0;
    pContent = (KMailContent*)pRespond->content;
    uPakSize = sizeof(KS2C_Sync_Mail_Content);
    if (nResult == mrcSucceed)
    {
        KGLOG_PROCESS_ERROR(pContentProtoBuf);
        pContent->wTextLen = pContentProtoBuf->wTextLen;
        pContent->nMoney = pContentProtoBuf->nMoney;

        //这里要把邮件中的protobuf数据 转成普通的物品数据
        pbyItemDataSrc = pContentProtoBuf->byData + pContentProtoBuf->wTextLen;
        pbyItemDataDes = pContent->byData + pContent->wTextLen;

        for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; i++)
        {
            uItemLen = 0;
            if (pContentProtoBuf->ItemDesc[i].byDataLen > 0 && !pContentProtoBuf->ItemDesc[i].bAcquired)
            {
                EquiData.Clear();
                bRetCode = EquiData.ParsePartialFromArray(pbyItemDataSrc,pContentProtoBuf->ItemDesc[i].byDataLen);
                KGLOG_PROCESS_ERROR(bRetCode);

                IItem* piItem = g_pSO3World->m_ItemHouse.CreateItemFromProtoBuf(&EquiData);
                KGLOG_PROCESS_ERROR(piItem);
                piItem->GetBinaryDataForClient(&uItemLen,pbyItemDataDes,MAX_EXTERNAL_PACKAGE_SIZE - uContentLen);
                g_pSO3World->m_ItemHouse.DestroyItem(piItem);

            }
            pContent->ItemDesc[i].bAcquired = pContentProtoBuf->ItemDesc[i].bAcquired;
            pContent->ItemDesc[i].byDataLen = (BYTE)uItemLen;
            pContent->ItemDesc[i].byQuality = pContentProtoBuf->ItemDesc[i].byQuality;

            pbyItemDataSrc += pContentProtoBuf->ItemDesc[i].byDataLen;
            pbyItemDataDes += pContent->ItemDesc[i].byDataLen;
            uContentLen += pContent->ItemDesc[i].byDataLen;
        }

        memcpy(pContent->byData, pContentProtoBuf->byData, pContent->wTextLen);

        uContentLen += sizeof(KMailContent) + pContent->wTextLen;

        uPakSize = sizeof(KS2C_Sync_Mail_Content) + uContentLen;
        KGLOG_PROCESS_ERROR(uPakSize <= MAX_EXTERNAL_PACKAGE_SIZE);

        pRespond->contentLength = (unsigned short)uContentLen;
    }

    bRetCode = Send(pPlayer->m_nConnIndex, pRespond, uPakSize);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoMailGeneralRespond(DWORD dwPlayerID, DWORD dwMailID, int nResult)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KS2C_Mail_General_Respond*   pRespond    = (KS2C_Mail_General_Respond*)m_byTempData;
    KPlayer*                    pPlayer     = NULL;
    unsigned                    uPakSize    = 0;

    KGLOG_PROCESS_ERROR(dwPlayerID);
    KGLOG_PROCESS_ERROR(dwMailID);

    uPakSize = sizeof(KS2C_Mail_General_Respond);
    KGLOG_PROCESS_ERROR(uPakSize <= MAX_EXTERNAL_PACKAGE_SIZE);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    pRespond->protocolID    = s2c_mail_general_respond;
    pRespond->dwMailID      = dwMailID;
    pRespond->byResult      = (BYTE)nResult;

    bRetCode = Send(pPlayer->m_nConnIndex, pRespond, uPakSize);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncTrainingHeroList(KPlayer* pPlayer)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    KS2C_Sync_TrainingHeroList* pPak            = NULL;
    size_t                      uTrainingHeroCount      = 0;
    size_t                      uSendDataLen    = 0;
    const KVEC_TRAINING_ITEMS*  pHeroTrainingItems = NULL;
    KTRAINING_TEMPLATE          TrainingTemplate = {0};
    DWORD                       dwTrainedTime = 0U;
    DWORD                       dwTotalTime = 0;

    assert(pPlayer);

    pHeroTrainingItems = pPlayer->m_HeroTrainingMgr.GetAllTrainingItems();
    assert(pHeroTrainingItems);

    uTrainingHeroCount = pHeroTrainingItems->size();
    uSendDataLen = sizeof(KS2C_Sync_TrainingHeroList) + sizeof(KS2C_TrainingHeroInfo) * uTrainingHeroCount;

    pPak = (KS2C_Sync_TrainingHeroList*)KMEMORY_ALLOC(uSendDataLen);
    KGLOG_PROCESS_ERROR(pPak);

    pPak->protocolID = s2c_sync_training_hero_list;
    pPak->uTeacherLevel = (unsigned char)pPlayer->m_HeroTrainingMgr.GetTeacherLevel();
    pPak->TrainingHeroListLength = (unsigned short)uTrainingHeroCount;

    for (size_t i = 0; i < uTrainingHeroCount;  ++i)
    {
        const KHERO_TRAINING_ITEM&  rTrainingItem = (*pHeroTrainingItems)[i];
        KS2C_TrainingHeroInfo&      rTrainingInfo = pPak->TrainingHeroList[i];

        dwTotalTime = g_pSO3World->m_Settings.m_TrainingTemplateMgr.GetTrainSeconds(rTrainingItem.dwTrainingID);

        rTrainingInfo.templateID = rTrainingItem.dwHeroTemplateID;

        KGLOG_PROCESS_ERROR(g_pSO3World->m_nCurrentTime >=  rTrainingItem.nEndTime);
        dwTrainedTime = g_pSO3World->m_nCurrentTime - rTrainingItem.nEndTime;

        rTrainingInfo.leftTime      = 0U;
        if (dwTrainedTime < dwTotalTime)
            rTrainingInfo.leftTime  = dwTotalTime - dwTrainedTime;

        rTrainingInfo.trainingID    = (WORD)rTrainingItem.dwTrainingID;
    }

    bRetCode = Send(pPlayer->m_nConnIndex, pPak, uSendDataLen);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KMEMORY_FREE(pPak);
    return bResult;
}

BOOL KPlayerServer::DoBeginTrainingHeroRespond(int nConnIndex, DWORD dwHeroTemplateID, DWORD dwTotalTime, DWORD dwTrainingTemplateID, BOOL bSuccess)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Begin_Training_Hero_Respond Pak;

    Pak.protocolID      = s2c_begin_training_hero_respond;
    Pak.heroTemplateID  = dwHeroTemplateID;
    Pak.leftTime        = dwTotalTime;
    Pak.trainingID      = (WORD)dwTrainingTemplateID;
    Pak.success         = !!bSuccess;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoEndTrainingHeroRespond(int nConnIndex, DWORD dwHeroTemplateID, BOOL bSuccess)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_End_Training_Hero_Respond Pak;

    Pak.protocolID      = s2c_end_training_hero_respond;
    Pak.heroTemplateID  = dwHeroTemplateID;
    Pak.success         = !!bSuccess;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoBuyTrainingSeatRespond(int nConnIndex, BOOL bSuccess, unsigned uCurSeatCount)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Buy_Training_Seat_Respond Pak;

    Pak.protocolID = s2c_buy_training_seat_respond;
    Pak.success = (bool)bSuccess;
    Pak.curSeatCount = (unsigned short)uCurSeatCount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDelHeroData(int nConnIndex, DWORD dwTemplateID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Del_Hero_Data Pak;

    Pak.protocolID      = s2c_del_hero_data;
    Pak.heroTemplateID  = dwTemplateID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUpdateItemAmount(int nConnIndex, int nPackageType, int nPackageIndex, int nPos, int nStack)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Update_Item_Amount Pak;

    Pak.protocolID      = s2c_update_item_amount;
    Pak.byPackageType   = (BYTE)nPackageType;
    Pak.byPackageIndex  = (BYTE)nPackageIndex;
    Pak.byPos           = (BYTE)nPos;
    Pak.wStack          = (WORD)nStack;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUpdateItemDurability(int nConnIndex, int nPackageType, int nPackageIndex, int nPos, int nDurability)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Update_Item_Durability Pak;

    Pak.protocolID      = s2c_update_item_durability;
    Pak.byPackageType   = (BYTE)nPackageType;
    Pak.byPackageIndex  = (BYTE)nPackageIndex;
    Pak.byPos           = (BYTE)nPos;
    Pak.wDurability     = (WORD)nDurability;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDestroyItem(int nConnIndex, int nPackageType, int nPackageIndex, int nPos)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Destroy_Item Pak;

    Pak.protocolID      = s2c_destroy_item;
    Pak.byPackageType   = (BYTE)nPackageType;
    Pak.byPackageIndex  = (BYTE)nPackageIndex;
    Pak.byPos           = (BYTE)nPos;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncQuestList(KPlayer* pPlayer)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    unsigned uAcceptCount = 0U;
    unsigned uSendDataLen = 0U;
    KS2C_Sync_Quest_List* pPak = NULL;
    const KQuest* cpQuest = NULL;
    size_t uIndex = 0;

    assert(pPlayer);

    uAcceptCount = (unsigned)pPlayer->m_QuestList.GetAcceptedCount();
    uSendDataLen = sizeof(KS2C_Sync_Quest_List) + uAcceptCount * sizeof(KS2C_QuestInfo);

    pPak = (KS2C_Sync_Quest_List*)KMEMORY_ALLOC(uSendDataLen);
    KGLOG_PROCESS_ERROR(pPak);

    pPak->protocolID = s2c_sync_quest_list;
    pPak->questListLength = (unsigned short)uAcceptCount;

    for (size_t i = 0; i < cdMaxAcceptQuestCount; ++i)
    {
        cpQuest = pPlayer->m_QuestList.GetQuestByIndex(i);
        assert(cpQuest);

        if (cpQuest->pQuestInfo == NULL)
            continue;

        assert(uIndex < uAcceptCount);

        pPak->questList[uIndex].questID = cpQuest->pQuestInfo->dwQuestID;
        pPak->questList[uIndex].questIndex = (unsigned char)i;

        assert(sizeof(pPak->questList[uIndex].questValue) == sizeof(cpQuest->nQuestValue));
        memcpy(pPak->questList[uIndex].questValue, cpQuest->nQuestValue, sizeof(pPak->questList[uIndex].questValue));

        ++uIndex;
    }

    bRetCode = Send(pPlayer->m_nConnIndex, pPak, uSendDataLen);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KMEMORY_FREE(pPak);
    return bResult;
}


BOOL KPlayerServer::DoSetQuestValueRespond(int nConnIndex, int nQuestIndex, int nValueIndex, int nNewValue, QUEST_RESULT_CODE eRetCode)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Set_Quest_Value_Respond Pak;

    Pak.protocolID = s2c_set_quest_value_respond;
    Pak.retCode    = (unsigned char)eRetCode;
    Pak.questIndex = (unsigned char)nQuestIndex;
    Pak.valueIndex = (unsigned char)nValueIndex;
    Pak.newValue   = nNewValue;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoAcceptQuestRespond(int nConnIndex, DWORD dwQuestID, int nQuestIndex, QUEST_RESULT_CODE eRetCode)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Accept_Quest_Respond Pak;

    Pak.protocolID = s2c_accept_quest_respond;
    Pak.retCode = (unsigned char)eRetCode;
    Pak.questID = dwQuestID;
    Pak.questIndex = (unsigned char)nQuestIndex;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoFinishQuestRespond(int nConnIndex, QUEST_RESULT_CODE eRetCode, int nQuestIndex)
{
    BOOL bResult = false;
    BOOL bRetCode = false;	
    KS2C_Finish_Quest_Respond Pak;

    Pak.protocolID  = s2c_finish_quest_respond;
    Pak.retCode     = (unsigned char)eRetCode;
    Pak.questIndex  = (unsigned char)nQuestIndex;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoCancelQuestRespond(int nConnIndex, QUEST_RESULT_CODE eRetCode, int nQuestIndex)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Cancel_Quest_Respond Pak;

    Pak.protocolID = s2c_cancel_quest_respond;
    Pak.retCode = (unsigned char)eRetCode;
    Pak.questIndex = (unsigned char)nQuestIndex;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncQuestValue(int nConnIndex, int nQuestIndex, int nValueIndex, int nCurValue)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Sync_Quest_Value Pak;

    Pak.protocolID = s2c_sync_quest_value;
    assert(nQuestIndex >= 0 && nQuestIndex <= CHAR_MAX);
    Pak.questIndex = (unsigned char)nQuestIndex;
    assert(nValueIndex >= 0 && nValueIndex <= CHAR_MAX);
    Pak.valueIndex = (unsigned char)nValueIndex;
    Pak.curValue = nCurValue;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncDailyQuestData(KPlayer* pPlayer)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    size_t uPakLength = 0;
    KS2C_Sync_Daily_Quest* pPak = (KS2C_Sync_Daily_Quest*)m_byTempData;
    int nCount = 0;

    assert(pPlayer);

    pPak->protocolID = s2c_sync_daily_quest;

    nCount = pPlayer->m_QuestList.GetDailyQuest(m_byTempData, sizeof(m_byTempData));
    if (nCount > 0)
    {
        pPak->dailyQuestLength = (unsigned short)nCount;
        uPakLength = sizeof(KS2C_Sync_Daily_Quest) + sizeof(KDailyQuest) * nCount;

        bRetCode = Send(pPlayer->m_nConnIndex, pPak, uPakLength);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncOneDailyQuestData(int nConnIndex, DWORD dwQuestID, int nNextAcceptTime)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    size_t uPakLength = sizeof(KS2C_Sync_Daily_Quest) + sizeof(KDailyQuest);
    KS2C_Sync_Daily_Quest* pPak = (KS2C_Sync_Daily_Quest*)m_byTempData;

    pPak->protocolID = s2c_sync_daily_quest;
    pPak->dailyQuestLength = 1;
    pPak->dailyQuest[0].wQuestID = (WORD)dwQuestID;
    pPak->dailyQuest[0].nNextAcceptTime = nNextAcceptTime;

    bRetCode = Send(nConnIndex, pPak, uPakLength);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoClearQuest(int nConnIndex, DWORD dwQuestID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KS2C_Clear_Quest Pak;

    Pak.protocolID = s2c_clear_quest;
    Pak.wQuestID   = (WORD)dwQuestID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoExchangeItemRespond(
    int nConnIndex,         int nCode,
    int nSrcPackageType,    int nSrcPackageIndex,   int nSrcPos, 
    int nDestPackageType,   int nDestPackageIndex,  int nDestPos, int nExChangeResult
    )
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Exchange_Item_Respond Pak;

    Pak.protocolID          = s2c_exchange_item_respond;
    Pak.eResultCode         = (BYTE)nCode;
    Pak.uSrcPackageType     = (BYTE)nSrcPackageType;
    Pak.uSrcPackageIndex    = (BYTE)nSrcPackageIndex;
    Pak.uSrcPos             = (BYTE)nSrcPos;
    Pak.uDestPackageType    = (BYTE)nDestPackageType;
    Pak.uDestPackageIndex   = (BYTE)nDestPackageIndex;
    Pak.uDestPos            = (BYTE)nDestPos;
    Pak.uExChangeResult     = (BYTE)nExChangeResult;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUseItemRespond(
    int nConnIndex, int nCode, int nPackageType, int nPackageIndex, int nPos
    )
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Use_Item_Respond Pak;

    Pak.protocolID      = s2c_use_item_respond;
    Pak.uCode           = (BYTE)nCode;
    Pak.uPackageType    = (BYTE)nPackageType;
    Pak.uPackageIndex   = (BYTE)nPackageIndex;
    Pak.uPos            = (BYTE)nPos;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAddBuffNotify(KHero* pHero, DWORD dwBuffID, int nLeftFrame)
{
    BOOL    bResult = false;
    BOOL    bRetCode = false;
    KScene* pScene = NULL;
    KS2C_Add_Buff_Notify Pak;
    KBroadcastFunc  BroadcastFunc;

    KGLOG_PROCESS_ERROR(pHero);

    pScene = pHero->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    Pak.protocolID  = s2c_add_buff_notify;
    Pak.dwHeroID    = pHero->m_dwID;
    Pak.dwBuffID    = dwBuffID;
    Pak.nLeftFrame   = nLeftFrame;

    BroadcastFunc.m_pvData          = &Pak;
    BroadcastFunc.m_uSize           = sizeof(Pak);
    BroadcastFunc.m_dwExclusive     = 0;
    BroadcastFunc.m_dwExcept        = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSetCoolDownNotify(int nConnIndex, DWORD dwTimerID, int nEndFrame, int nInterval)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Set_CoolDown_Notify Pak;

    Pak.protocolID      = s2c_set_cooldown_notify;
    Pak.uTimerID        = dwTimerID;
    Pak.uLeftFrame      = 0;
    Pak.uInterval       = (unsigned)nInterval;

    if (nEndFrame > g_pSO3World->m_nGameLoop)
        Pak.uLeftFrame = (unsigned)(nEndFrame - g_pSO3World->m_nGameLoop);

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoRemoveSceneObj(DWORD dwSceneID, DWORD dwObjID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Remove_Scene_Obj Pak;
    KBroadcastFunc  BroadcastFunc;
    KScene* pScene = NULL;

    Pak.protocolID  = s2c_remove_scene_obj;
    Pak.dwSceneID   = dwSceneID;
    Pak.dwObjID     = dwObjID;

    BroadcastFunc.m_pvData          = &Pak;
    BroadcastFunc.m_uSize           = sizeof(Pak);
    BroadcastFunc.m_dwExclusive     = 0;
    BroadcastFunc.m_dwExcept        = ERROR_ID;

    pScene = g_pSO3World->m_SceneSet.GetObj(dwSceneID);
    KGLOG_PROCESS_ERROR(pScene);

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMissionData(int nConnIndex, const T3DB::KPB_MISSION_DATA* pMissionData)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    size_t uPakLength = 0;
    KS2C_Sync_Mission_Data* pPak = (KS2C_Sync_Mission_Data*)m_byTempData;

    KGLOG_PROCESS_ERROR(pMissionData);

    pPak->protocolID = s2c_sync_mission_data;
    pPak->missionsLength = (unsigned short)pMissionData->missions_size();
    uPakLength = sizeof(KS2C_Sync_Mission_Data) + pPak->missionsLength * sizeof(KOneMissionData);

    for (int i = 0; i < pPak->missionsLength; ++i)
    {
        const T3DB::KPB_ONE_MISSION_DATA& OneMissionData = pMissionData->missions(i);
        pPak->missions[i].byMissionType = (BYTE)OneMissionData.type();
        pPak->missions[i].byMissionStep = (BYTE)OneMissionData.step();
        pPak->missions[i].byMissionOpenLevel = (BYTE)OneMissionData.openedlevel();
        pPak->missions[i].byMissionPassLevel = (BYTE)OneMissionData.finishedlevel();
    }

    bRetCode = Send(nConnIndex, pPak, uPakLength);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMissionOpened(int nConnIndex, int nType, int nStep, int nLevel)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KS2C_Sync_Mission_Opened Pak;

    Pak.protocolID = s2c_sync_mission_opened;
    Pak.byMissionType = (unsigned char)nType;
    Pak.byMissionStep = (unsigned char)nStep;
    Pak.byMissionOpenLevel = (unsigned char)nLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMissionFinished(int nConnIndex, int nType, int nStep, int nLevel)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KS2C_Sync_Mission_Passed Pak;

    Pak.protocolID = s2c_sync_mission_passed;
    Pak.byMissionType = (unsigned char)nType;
    Pak.byMissionStep = (unsigned char)nStep;
    Pak.byMissionPassLevel = (unsigned char)nLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncRandomDailyQuest(int nConnIndex, std::vector<DWORD>& vecQuestID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KS2C_Get_Daily_Quest_List_Respond* pPak = (KS2C_Get_Daily_Quest_List_Respond*)m_byTempData;
    size_t uCount = vecQuestID.size();
    size_t uPakSize = sizeof(KS2C_Get_Daily_Quest_List_Respond) + uCount * sizeof(unsigned int);

    KGLOG_PROCESS_ERROR(uPakSize < sizeof(m_byTempData));

    pPak->protocolID = s2c_get_daily_quest_list_respond;
    pPak->questIDListLength = (unsigned short)uCount;

    for (size_t i = 0; i < uCount; ++i)
        pPak->questIDList[i] = vecQuestID[i];

    bRetCode = Send(nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncItemBind(int nConnIndex, int nPackageType, int nPackageIndex, int nPos, BOOL bBind)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Sync_Item_Bind Pak;

    Pak.protocolID      = s2c_sync_item_bind;
    Pak.byPackageType   = (BYTE)nPackageType;
    Pak.byPackageIndex  = (BYTE)nPackageIndex;
    Pak.byPos           = (BYTE)nPos;
    Pak.byBind          = (BYTE)bBind;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoEnterMissionRespond(int nConnIndex, int nMissionType, int nMissionStep, int nMissionLevel, BOOL bSucceed)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Apply_Enter_Mission_Respond Pak;

    Pak.protocolID      = s2c_apply_enter_mission_respond;
    Pak.byMissionType   = (BYTE)nMissionType;
    Pak.byMissionStep   = (BYTE)nMissionStep;
    Pak.byMissionLevel  = (BYTE)nMissionLevel;
    Pak.isSuccess       = (bSucceed == 1);

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoLeaveMissionRespond(int nConnIndex, int nMissionType, int nMissionStep, int nMissionLevel)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Exit_Mission_Notify Pak;

    Pak.protocolID      = s2c_exit_mission_notify;
    Pak.byMissionType   = (BYTE)nMissionType;
    Pak.byMissionStep   = (BYTE)nMissionStep;
    Pak.byMissionLevel  = (BYTE)nMissionLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBroadcastUseItem(KScene* pScene, DWORD dwUserID, DWORD dwTabType, DWORD dwIndex)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Broadcast_Use_Item Pak;
    KBroadcastFunc  BroadcastFunc;

    assert(pScene);

    Pak.protocolID = s2c_broadcast_use_item;
    Pak.dwUserID = dwUserID;
    Pak.wTabType = (unsigned short)dwTabType;
    Pak.wIndex   = (unsigned short)dwIndex;

    BroadcastFunc.m_pvData          = &Pak;
    BroadcastFunc.m_uSize           = sizeof(Pak);
    BroadcastFunc.m_dwExclusive     = 0;
    BroadcastFunc.m_dwExcept        = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUpdateMoveParam(KHero* pHero)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Update_Move_Param Pak;
    KPlayer* pPlayer = NULL;

    assert(pHero);
    pPlayer = pHero->GetOwner();
    KG_PROCESS_SUCCESS(pPlayer == NULL);

    Pak.protocolID = s2c_update_move_param;
    Pak.dwHeroID  = pHero->m_dwID;
    Pak.wAddGravityBase = (WORD)pHero->m_nAddGravityBase;
    Pak.wGravityPercent = (WORD)pHero->m_nGravityPercent;
    Pak.wMoveSpeed = (WORD)pHero->m_nMoveSpeed;
    Pak.wJumpSpeed = (WORD)pHero->m_nJumpSpeed;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KG_PROCESS_ERROR(bRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoFreePVPInviteNotify(KPlayer* pInvitee, const char(&cszInvitorName)[_NAME_LEN], DWORD dwRoomID, DWORD dwMapID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Notify_Room_Invite Pak;

    KGLOG_PROCESS_ERROR(pInvitee);

    Pak.protocolID = s2c_notify_room_invite;
    Pak.roomID  = dwRoomID;
    Pak.mapID   = dwMapID;

    memcpy(Pak.invitorName, cszInvitorName, sizeof(Pak.invitorName));
    Pak.invitorName[countof(Pak.invitorName) - 1] = '\0';

    bRetCode = Send(pInvitee->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoLadderPVPInviteNotify(KPlayer* pInvitee, const char(&cszInvitorName)[_NAME_LEN])
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Sync_Team_Invite_Info Pak;

    KGLOG_PROCESS_ERROR(pInvitee);

    Pak.protocolID = s2c_sync_team_invite_info;

    memcpy(Pak.szInviterName, cszInvitorName, sizeof(Pak.szInviterName));
    Pak.szInviterName[countof(Pak.szInviterName) - 1] = '\0';

    bRetCode = Send(pInvitee->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoPVEInviteNotify(KPlayer* pInvitee, const char(&cszInvitorName)[_NAME_LEN], int nPVEMode, int nMissionType, int nMissionStep, int nMissionLevel)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Sync_Pve_Team_Invite_Info Pak;

    KGLOG_PROCESS_ERROR(pInvitee);

    Pak.protocolID      = s2c_sync_pve_team_invite_info;
    Pak.byPVEMode       = (BYTE)nPVEMode;
    Pak.byMissionType   = (BYTE)nMissionType;
    Pak.byMissionStep   = (BYTE)nMissionStep;
    Pak.byMissionLevel  = (BYTE)nMissionLevel;

    memcpy(Pak.szInviterName, cszInvitorName, sizeof(Pak.szInviterName));
    Pak.szInviterName[countof(Pak.szInviterName) - 1] = '\0';

    bRetCode = Send(pInvitee->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoLoseBuffNotify(KHero* pHero, DWORD dwBuffID)
{
    BOOL                bResult   = false;
    BOOL                bRetCode  = false;
    KBroadcastFunc      BroadcastFunc;
    KScene*             pScene    = NULL;
    KS2C_Lose_Buff_Notify Pak;

    KGLOG_PROCESS_ERROR(pHero);

    pScene = pHero->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    Pak.protocolID  = s2c_lose_buff_notify;
    Pak.dwHeroID    = pHero->m_dwID;
    Pak.dwBuffID    = dwBuffID;

    BroadcastFunc.m_pvData          = &Pak;
    BroadcastFunc.m_uSize           = sizeof(Pak);
    BroadcastFunc.m_dwExclusive     = 0;
    BroadcastFunc.m_dwExcept        = ERROR_ID;

    pHero->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoActivityCanAwardNotify(KPlayer* pPlayer, DWORD dwActivityID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Activity_Can_Award_Notify Pak;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID   = s2c_activity_can_award_notify;
    Pak.dwActivityID = dwActivityID;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoActivityAwardedNotify(KPlayer* pPlayer, DWORD dwActivityID, KACTIVIY_AWARD_RET eRetCode)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Activity_Awarded_Item_Notify Pak;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID   = s2c_activity_awarded_item_notify;
    Pak.dwActivityID = dwActivityID;
    Pak.byRetCode    = (BYTE)eRetCode;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoPlayerDataLoadFinished(int nConnIndex)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Load_Player_Data_Finished Pak;

    Pak.protocolID   = s2c_load_player_data_finished;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncPlayerLimitedFlag(int nConnIndex, BOOL bLimited)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Player_Limited_Notify Pak;

    Pak.protocolID   = s2c_player_limited_notify;
    Pak.bLimited     = (unsigned char)bLimited;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncOneQuestState(int nConnIndex, DWORD dwQuestID, QUEST_STATE eState)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Sync_One_Quest_State Pak;

    Pak.protocolID      = s2c_sync_one_quest_state;
    Pak.dwQuestID       = dwQuestID;
    Pak.byQuestState    = (BYTE)eState;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAwardGift(int nConnIndex, DWORD dwTabType, DWORD dwIndex, int nAmount)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_Award_Gift Pak;

    Pak.protocolID      = s2c_award_gift;
    Pak.wTabtype        = (WORD)dwTabType;
    Pak.wIndex          = (WORD)dwIndex;
    Pak.wAmount         = (WORD)nAmount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAddItemNotify(int nConnIndex, DWORD dwTabType, DWORD dwIndex, int nAmount)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_Add_Item_Notify    Pak;

    Pak.protocolID      = s2c_add_item_notify;
    Pak.wTabtype        = (WORD)dwTabType;
    Pak.wIndex          = (WORD)dwIndex;
    Pak.wAmount         = (WORD)nAmount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncObjEnd(int nConnIndex)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KS2C_SyncSceneObjEnd Pak;

    Pak.protocolID      = s2c_sync_scene_obj_end;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncNewNpc(KHero* pNpc, int nConnIndex/*=-1*/)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    KScene*                     pScene      = NULL;
    KS2C_New_Npc_Notify         Pak;
    KBroadcastFunc              BroadcastFunc;

    assert(pNpc);

    KGLOG_PROCESS_ERROR(pNpc->m_pNpcTemplate);

    memset(&Pak, 0, sizeof(Pak));

    Pak.protocolID = s2c_new_npc_notify;
    memcpy(Pak.roleName, pNpc->m_szName, sizeof(Pak.roleName));
    Pak.id              = pNpc->m_dwID;
    Pak.side            = (BYTE)pNpc->m_nSide;
    Pak.npcTemplateID   = pNpc->m_pNpcTemplate->dwID;
    Pak.representID     = pNpc->m_pNpcTemplate->dwRepresentID;
    Pak.level           = (short)pNpc->m_nLevel;
    Pak.moveState       = (BYTE)pNpc->m_eMoveState;
    Pak.facedir         = (BYTE)pNpc->m_eFaceDir;
    Pak.x               = pNpc->m_nX;
    Pak.y               = pNpc->m_nY;
    Pak.z               = pNpc->m_nZ;
    Pak.length          = pNpc->m_nLength;
    Pak.width           = pNpc->m_nWidth;
    Pak.height          = pNpc->m_nHeight;
    Pak.velocityX       = pNpc->m_nVelocityX;
    Pak.velocityY       = pNpc->m_nVelocityY;
    Pak.velocityZ       = pNpc->m_nVelocityZ;
    Pak.hitRate         = pNpc->m_nHitRate;

    Pak.heroData.nCurrentEndurance    = pNpc->m_nCurrentEndurance;
    Pak.heroData.nMaxEndurance        = pNpc->m_nMaxEndurance;
    Pak.heroData.nCurrentStamina      = pNpc->m_nCurrentStamina;
    Pak.heroData.nMaxStamina          = pNpc->m_nMaxStamina;
    Pak.heroData.wCurrentAngry        = (WORD)pNpc->m_nCurrentAngry;
    Pak.heroData.wMaxAngry            = (WORD)pNpc->m_nMaxAngry;
    Pak.heroData.wWillPower           = (WORD)pNpc->m_nWillPower;
    Pak.heroData.wInterference        = (WORD)pNpc->m_nInterference;
    Pak.heroData.wInterferenceRange   = (WORD)pNpc->m_nInterferenceRange;
    Pak.heroData.wMoveSpeed           = (WORD)pNpc->m_nMoveSpeed;
    Pak.heroData.wJumpSpeed           = (WORD)pNpc->m_nJumpSpeed;
    Pak.heroData.wAddGravityBase      = (WORD)pNpc->m_nAddGravityBase;
    Pak.heroData.wGravityPercent      = (WORD)pNpc->m_nGravityPercent;
	Pak.heroData.wFashionID           = (WORD)pNpc->m_pNpcTemplate->dwSkinID;
    
    for (int i = 0; i < KACTIVE_SLOT_TYPE_TOTAL; ++i)
    {
        Pak.heroData.wActiveSkill[i] = (WORD)pNpc->m_ActiveSkill[i];
    }

    for (int i = 0; i < cdNirvanaSkillSlotCount; ++i)
    {
        Pak.heroData.wNirvanaSkill[i] = (WORD)pNpc->m_NirvanaSkill[i];
    }

    Pak.heroData.wSlamBallSkill = (WORD)pNpc->m_SlamBallSkill[0];

    for (size_t i = 0; i < countof(Pak.heroData.wOtherActiveSkill); ++i)
    {
        if (i < pNpc->m_vecAllOtherActiveSkill.size())
            Pak.heroData.wOtherActiveSkill[i] = (WORD)pNpc->m_vecAllOtherActiveSkill[i];
        else
            Pak.heroData.wOtherActiveSkill[i] = 0;
    }

    if (nConnIndex != -1)
    {
        bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));    
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        KScene*        pScene      		= NULL;
        KBroadcastFunc BroadcastFunc;
        BroadcastFunc.m_pvData          = &Pak;
        BroadcastFunc.m_uSize           = sizeof(Pak);
        BroadcastFunc.m_dwExcept        = ERROR_ID;
        BroadcastFunc.m_dwExclusive     = 0;

        pScene = pNpc->m_pScene;
        KGLOG_PROCESS_ERROR(pScene);
        pScene->TraverseHero(BroadcastFunc);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoNormalShootBall(KHero* pShooter, KBasketSocket* pSocket, DWORD dwInterferenceHeroID)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_Normal_ShootBall   Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;

    assert(pSocket);
    assert(pShooter);
    assert(pShooter->m_pScene);

    pScene = pShooter->m_pScene;

    Pak.protocolID          = s2c_normal_shootball;
    Pak.id                  = pShooter->m_dwID;
    Pak.x                   = pShooter->m_nX;
    Pak.y                   = pShooter->m_nY;
    Pak.z                   = pShooter->m_nZ;
    Pak.aimLoop             = pShooter->m_nShootTime;
    Pak.basketSocketID      = pSocket->m_dwID;
    Pak.virtualFrame        = pShooter->m_nVirtualFrame;
    Pak.interferenceHeroID  = dwInterferenceHeroID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = pShooter->m_dwID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSkillShootBall(KHero* pShooter, KBasketSocket* pSocket)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_Skill_ShootBall    Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;

    assert(pSocket);
    assert(pShooter);
    assert(pShooter->m_pScene);

    pScene = pShooter->m_pScene;

    Pak.protocolID      = s2c_skill_shootball;
    Pak.id              = pShooter->m_dwID;
    Pak.x               = pShooter->m_nX;
    Pak.y               = pShooter->m_nY;
    Pak.z               = pShooter->m_nZ;
    Pak.basketSocketID  = pSocket->m_dwID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = pShooter->m_dwID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoNormalSlamBall(KHero* pSlamer, KBasketSocket* pSocket)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_Normal_SlamBall    Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;

    assert(pSocket);
    assert(pSlamer);
    assert(pSlamer->m_pScene);

    pScene = pSlamer->m_pScene;

    Pak.protocolID      = s2c_normal_slamball;
    Pak.id              = pSlamer->m_dwID;
    Pak.x               = pSlamer->m_nX;
    Pak.y               = pSlamer->m_nY;
    Pak.z               = pSlamer->m_nZ;
    Pak.basketSocketID  = pSocket->m_dwID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = pSlamer->m_dwID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSkillSlamBall(KHero* pSlamer, KBasketSocket* pSocket)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_Skill_SlamBall     Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;

    assert(pSocket);
    assert(pSlamer);
    assert(pSlamer->m_pScene);

    pScene = pSlamer->m_pScene;

    Pak.protocolID      = s2c_skill_slamball;
    Pak.id              = pSlamer->m_dwID;
    Pak.x               = pSlamer->m_nX;
    Pak.y               = pSlamer->m_nY;
    Pak.z               = pSlamer->m_nZ;
    Pak.basketSocketID  = pSocket->m_dwID;
    Pak.virtualFrame    = pSlamer->m_nVirtualFrame;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = pSlamer->m_dwID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMakingMachineInfo(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_MakingMachineInfo Pak;

    assert(pPlayer);

    Pak.protocolID          = s2c_sync_makingmachineinfo;

    pPlayer->m_MakingMachine.GetInfo(Pak.recipeState);

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoLearnRecipeNotify(KPlayer* pPlayer, DWORD dwRecipeID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Learn_Recipe_Notify Pak;

    assert(pPlayer);

    Pak.protocolID          = s2c_learn_recipe_notify;
    Pak.recipeID            = dwRecipeID;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoClearBuildCDRespond(KPlayer* pPlayer, int nCDType, BOOL bSuccess)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Clear_Build_CD_Respond Pak;

    assert(pPlayer);

    Pak.protocolID  = s2c_clear_build_cd_respond;
    Pak.cdType      = (BYTE)nCDType;
    Pak.bSuccess    = (BYTE)bSuccess;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoModifyRemarkFail(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Modify_Remark_Fail Pak;

    assert(pPlayer);
    Pak.protocolID  = s2c_modify_remark_fail;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDownwardNotify(KPlayer* pPlayer, DWORD dwMessageID, void* pData /*= NULL*/, size_t uDataLen /*= 0*/)
{
    BOOL                    bResult  = false;
    BOOL                    bRetCode = false;
    KS2C_Downward_Notify*   pPak     = (KS2C_Downward_Notify*)m_byTempData;
    size_t                  uPakSize = sizeof(*pPak) + uDataLen;

    KGLOG_PROCESS_ERROR(pPlayer);

    pPak->protocolID  = s2c_downward_notify;
    pPak->messageID   = dwMessageID;
    pPak->paramLength = (WORD)uDataLen;
    if (pData)
    {
        memcpy(pPak->param, pData, uDataLen);
    }
    bRetCode = Send(pPlayer->m_nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoProduceItemRespond(KPlayer* pPlayer, BOOL bSuccess, BOOL bCrit)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_ProduceItem_Respond Pak;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID  = s2c_produceitem_respond;
    Pak.bSuccess    = (BYTE)bSuccess;
    Pak.bCrit       = (BYTE)bCrit;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncSceneGravity(KScene* pScene)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_Sync_Scene_Gravity Pak;
    KBroadcastFunc          BroadcastFunc;

    assert(pScene);

    Pak.protocolID      = s2c_sync_scene_gravity;
    Pak.nGravity        = pScene->m_nGlobalGravity;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncSceneObjPosition(KSceneObject* pObj)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_Sync_Sceneobj_Position Pak;
    KBroadcastFunc          BroadcastFunc;

    assert(pObj);
    assert(pObj->m_pScene);

    Pak.protocolID      = s2c_sync_sceneobj_position;
    Pak.id              = pObj->m_dwID;
    Pak.x               = pObj->m_nX;
    Pak.y               = pObj->m_nY;
    Pak.z               = pObj->m_nZ;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pObj->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoNewBulletNotify(KScene* pScene, KBullet* pBullet)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_New_Bullet_Notify Pak;
    KBroadcastFunc BroadcastFunc;

    KGLOG_PROCESS_ERROR(pScene);
    KGLOG_PROCESS_ERROR(pBullet);
    KGLOG_PROCESS_ERROR(pBullet->m_pBulletTemplate);

    Pak.protocolID  = s2c_new_bullet_notify;
    Pak.templateID  = pBullet->m_pBulletTemplate->dwTemplateID;
    Pak.shooterID   = pBullet->m_dwCasterID;
    Pak.id          = pBullet->m_dwID;
    Pak.x           = pBullet->m_nX;
    Pak.y           = pBullet->m_nY;
    Pak.z           = pBullet->m_nZ;
    Pak.velocityX   = pBullet->m_nVelocityX;
    Pak.velocityY   = pBullet->m_nVelocityY;
    Pak.velocityZ   = pBullet->m_nVelocityZ;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoStrengthenEquipNotify(int nConnIndex, KPACKAGE_TYPE epackageType, int nPackageIndex, int nPos, BOOL bSuccess, int nStrengthenLevel)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Strengthen_Equip_Notify Pak;

    Pak.protocolID       = s2c_strengthen_equip_notify;
    Pak.uPackageType     = (BYTE)epackageType;
    Pak.uPackageIndex    = (BYTE)nPackageIndex;
    Pak.uPos             = (BYTE)nPos;
    Pak.bSuccess         = (BYTE)bSuccess;
    Pak.uStrengthenLevel = (BYTE)nStrengthenLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncDamage(KHero* pTarget, BOOL bEnduranceCrited, int nEnduranceDamage, BOOL bStaminaCrited, int nStaminaDamage, int nBeAttackedType /*= 0*/)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Damage    Pak;
    KBroadcastFunc      BroadcastFunc;
    KScene*             pScene = NULL;

    KGLOG_PROCESS_ERROR(pTarget);
    KGLOG_PROCESS_ERROR(pTarget->m_pScene);

    Pak.protocolID      = s2c_sync_damage;
    Pak.targetHeroID    = pTarget->m_dwID;
    Pak.enduranceCrited = (BYTE)bEnduranceCrited;
    Pak.enduranceDamage = nEnduranceDamage;
    Pak.staminaCrited   = (BYTE)bStaminaCrited;
    Pak.staminaDamage   = nStaminaDamage;
    Pak.beAttackedType  = (BYTE)nBeAttackedType;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pTarget->m_pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncReportItem(KPlayer* pPlayer, KREPORT_ITEM* pReportItem)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    size_t uPakSize = 0;
    KS2C_Sync_Report_Item* pPak = (KS2C_Sync_Report_Item*)m_byTempData;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pReportItem);

    uPakSize = sizeof(KS2C_Sync_Report_Item) + pReportItem->Param.size();
    KGLOG_PROCESS_ERROR(uPakSize <= sizeof(m_byTempData));

    pPak->protocolID 		= s2c_sync_report_item;
    pPak->reportIndex   	= (BYTE)pReportItem->nReportIndex;
    pPak->reportEvent 		= (BYTE)pReportItem->nReportEvent;
    pPak->reportTime 		= pReportItem->nReportTime;
    pPak->isNewReport       = (BYTE)pReportItem->bNewReport;
    pPak->reportParamLength = (unsigned short)pReportItem->Param.size();
    memcpy(pPak->reportParam, pReportItem->Param.data(), pReportItem->Param.size());

    bRetCode = Send(pPlayer->m_nConnIndex, pPak,  uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoRemoveReportItem(KPlayer* pPlayer, KREPORT_ITEM* pReportItem)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Remove_Report_Item Pak;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pReportItem);

    Pak.protocolID = s2c_remove_report_item;
    Pak.reportIndex = (BYTE)pReportItem->nReportIndex;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncWardrobeInfo(KPlayer* pPlayer)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    size_t  uUsedSize   = 0;

    assert(pPlayer);

    bRetCode = pPlayer->m_Wardrobe.GetSyncData(m_byTempData, sizeof(m_byTempData), uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);   

    bRetCode = Send(pPlayer->m_nConnIndex, m_byTempData, uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncFashionInfo(KPlayer* pPlayer, uint32_t dwFashionID, uint32_t dwOverduePoint)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Sync_Fashion_Info Pak;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID  = s2c_sync_fashion_info;
    Pak.id          = (WORD)dwFashionID;
    Pak.overduePoint= dwOverduePoint;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoChangeFashionRespond(
    KPlayer* pPlayer, BOOL bSuccess, uint32_t dwHeroTemplateID, uint32_t dwTargetFashionID
    )
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KS2C_Do_Change_Fashion_Respond Pak;

    Pak.protocolID      = s2c_change_fashion_respond;
    Pak.result          = bSuccess;
    Pak.herotemplateid  = dwHeroTemplateID;
    Pak.targetfashionid = dwTargetFashionID;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoChangeTeamLogoRespond(KPlayer* pPlayer, BOOL bSuccess, WORD wTeamLogoFrameID, WORD wTeamLogoEmblemID)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    KS2C_Change_TeamLogo_Respond Pak;

    Pak.protocolID  		= s2c_change_teamlogo_respond;
    Pak.byResult      		= bSuccess;
    Pak.wTeamLogoFrameID    = wTeamLogoFrameID;
    Pak.wTeamLogoEmblemID   = wTeamLogoEmblemID;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncTeamLogoInfo(KPlayer* pPlayer)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    size_t  uUsedSize   = 0;

    assert(pPlayer);

    bRetCode = pPlayer->GetTeamLogoSyncInfo(m_byTempData, sizeof(m_byTempData), uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(pPlayer->m_nConnIndex, m_byTempData, uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);    

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAddTeamLogoRespond(KPlayer* pPlayer, WORD wTeamLogoID)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    KS2C_Add_TeamLogo_Respond Pak;

    Pak.protocolID      = s2c_add_teamlogo_respond;
    Pak.wNewTeamLogoID  = wTeamLogoID;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncCheerleadingSlotInfo(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Cheerleading_Slot_Info Pak;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID = s2c_sync_cheerleading_slot_info;
    Pak.buyTimes   = (BYTE)pPlayer->m_CheerleadingMgr.m_uBuySlotTimes;
    Pak.validCount = (BYTE)pPlayer->m_CheerleadingMgr.m_uSlotCount;
    for (int i = 0; i < countof(Pak.slotInfo); ++i)
    {
        Pak.slotInfo[i].cheerleadingIndex = (WORD)pPlayer->m_CheerleadingMgr.m_SlotInfo[i].dwCheerleadingIndex;
        Pak.slotInfo[i].offsetX = (short)pPlayer->m_CheerleadingMgr.m_SlotInfo[i].nOffsetX;
        Pak.slotInfo[i].faceTo  = (short)pPlayer->m_CheerleadingMgr.m_SlotInfo[i].nFaceTo;
    }

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncCheerleadingItem(KPlayer* pPlayer, DWORD dwIndex, DWORD dwID, int nEndTime)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Cheerleading_Item Pak;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID = s2c_sync_cheerleading_item;
    Pak.idx     = (WORD)dwIndex;
    Pak.id      = (WORD)dwID;
    Pak.endTime = nEndTime;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoRemoveCheerleadingItem(KPlayer* pPlayer, DWORD dwIndex)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Remove_Cheerleading_Item Pak;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID = s2c_remove_cheerleading_item;
    Pak.idx     = (WORD)dwIndex;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncCheerleadingInfoInBattle(int nSide, int nPos, KPlayer* pSrcPlayer, int nConnIndex)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Cheerleadinginfo_In_Battle Pak;

    assert(pSrcPlayer);

    Pak.protocolID = s2c_sync_cheerleadinginfo_in_battle;
    Pak.side = (BYTE)nSide;
    Pak.pos  = (BYTE)nPos;

    bRetCode = pSrcPlayer->m_CheerleadingMgr.GetSlotInfoInBattle(Pak.slotInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncGymInfo(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    size_t uUsedSize = 0;

    assert(pPlayer);

    bRetCode = pPlayer->m_Gym.GetSyncData(m_byTempData, sizeof(m_byTempData), uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(pPlayer->m_nConnIndex, m_byTempData, uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUpgradeGymEquipLevel(int nConnIndex, int nNewLevel)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Upgrade_Equip_Level Pak;

    Pak.protocolID = s2c_upgrade_equip_level;
    Pak.byNewLevel = (BYTE)nNewLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncEquipCount(int nConnIndex, KGYM_TYPE eType, int nEquipCount)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Equip_Count Pak;

    Pak.protocolID = s2c_sync_equip_count;
    Pak.byType = (BYTE)eType;
    Pak.byEquipCount = (BYTE)nEquipCount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncHeroUseEquip(int nConnIndex, DWORD dwHeroTemplateID, KGYM_TYPE eType, int nEndTime, int nCount)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Hero_Use_Equip Pak;

    Pak.protocolID = s2c_sync_hero_use_equip;
    Pak.info.wHeroTemplateID = (WORD)dwHeroTemplateID;
    Pak.info.byType = (BYTE)eType;
    Pak.info.nEndTime = nEndTime;
    Pak.info.byCount = (BYTE)nCount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoFinishUseEquip(int nConnIndex, DWORD dwHeroTemplateID, int nFreeTalent, KGYM_TYPE eType, int nTalent)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Finished_Use_Equip Pak;

    Pak.protocolID = s2c_finished_use_equip;
    Pak.wHeroTemplateID = (WORD)dwHeroTemplateID;
    Pak.byFreeTalent = (BYTE)nFreeTalent;
    Pak.byType = (BYTE)eType;
    Pak.byTalent = (BYTE)nTalent;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncUpgradeQueueData(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    size_t uUsedSize = 0;

    assert(pPlayer);

    bRetCode = pPlayer->m_UpgradeQueue.GetSyncData(m_byTempData, sizeof(m_byTempData), uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(pPlayer->m_nConnIndex, m_byTempData, uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoAddUpgradeQueueNode(int nConnIndex, int nType, int nEndTime, uint32_t uHeroTemplateID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Add_Upgrade_Queue_Node Pak;

    Pak.protocolID = s2c_add_upgrade_queue_node;
    Pak.nEndTime = nEndTime;
    Pak.byUpgradeType = (BYTE)nType;
    Pak.uHeroTemplateID = uHeroTemplateID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDelUpgradeQueueNode(int nConnIndex, int nType, uint32_t dwHeroTemplateID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Del_Upgrade_Queue_Node Pak;

    Pak.protocolID = s2c_del_upgrade_queue_node;
    Pak.byUpgradeType = (BYTE)nType;
    Pak.uHeroTemplateID = dwHeroTemplateID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncUpgradeSlotCount(int nConnIndex, int nSlotCount)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Upgrade_Slot_Count Pak;

    Pak.protocolID = s2c_sync_upgrade_slot_count;
    Pak.byMaxSlotCount = (BYTE)nSlotCount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncHeroTrainTeacherLevel(int nConnIndex, int nTeacherLevel)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Herotrain_Teacher_Level Pak;

    Pak.protocolID = s2c_sync_herotrain_teacher_level;
    Pak.byTeachLevel = (BYTE)nTeacherLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncHeroTrainSitCount(int nConnIndex, int nSitCount)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Herotrain_Sit_Count Pak;

    Pak.protocolID = s2c_sync_herotrain_sit_count;
    Pak.bySitCount = (BYTE)nSitCount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncHeroTrainData(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    size_t uUsedSize = 0;

    bRetCode = pPlayer->m_HeroTrainingMgr.GetSyncData(m_byTempData, sizeof(m_byTempData), uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(pPlayer->m_nConnIndex, m_byTempData, uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncSafeBoxLevel(int nConnIndex, int nSafeBoxLevel)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Safebox_Level Pak;

    Pak.protocolID      = s2c_sync_safebox_level;
    Pak.bySafeBoxLevel  = (BYTE)nSafeBoxLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBSSyncData(KPlayer* pPlayer)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    size_t  uUsedSize   = 0;

    bRetCode = pPlayer->m_BusinessStreet.GetLandSyncData(m_byTempData, sizeof(m_byTempData), uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(pPlayer->m_nConnIndex, m_byTempData, uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->m_BusinessStreet.GetStoreSyncData(m_byTempData, sizeof(m_byTempData), uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(pPlayer->m_nConnIndex, m_byTempData, uUsedSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBSBuyLandResult(int nConnIndex, int nFreeLandid, int nBuyLandTimes)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Bs_Buy_Land_Result Pak;

    Pak.protocolID          = s2c_bs_buy_land_result;
    Pak.wID                 = (WORD)nFreeLandid;
    Pak.byBuyLandTimes      = (BYTE)nBuyLandTimes; 

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBSSyncNewStore(int nConnIndex, int nFreeLandCount, uint32_t dwStoreID, int nType, int nLevel, int nGetMoneyTimes, int nNextGetMoneyTime)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Bs_Sync_New_Store Pak;

    Pak.protocolID              = s2c_bs_sync_new_store;
    Pak.byFreeLandCount         = (BYTE)nFreeLandCount;
    Pak.store.wID               = (WORD)dwStoreID;
    Pak.store.byType            = (BYTE)nType;
    Pak.store.byLevel           = (BYTE)nLevel;
    Pak.store.byGetMoneyTimes   = (BYTE)nGetMoneyTimes;
    Pak.store.nNextGetMoneyTime = (unsigned int)nNextGetMoneyTime;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBSUpgradeStoreResult(int nConnIndex, uint32_t dwStoreID, int nLevel)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Bs_Upgrade_Store_Result Pak;

    Pak.protocolID      = s2c_bs_upgrade_store_result;
    Pak.wID             = (WORD)dwStoreID;
    Pak.byLevel         = (BYTE)nLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBSSyncGetMoneyInfo(int nConnIndex, uint32_t dwStoreID, int nGetMoneyTimes, int nNextGetMoneyTime)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Bs_Sync_Get_Money_Info Pak;

    Pak.protocolID          = s2c_bs_sync_get_money_info;
    Pak.wID                 = (WORD)dwStoreID;
    Pak.byGetMoneyTimes     = (BYTE)nGetMoneyTimes;
    Pak.nNextGetMoneyTime   = nNextGetMoneyTime;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBSSyncStoreNewType(int nConnIndex, uint32_t dwStoreID, int nType)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Bs_Sync_Store_New_Type Pak;

    Pak.protocolID      = s2c_bs_sync_store_new_type;
    Pak.wID             = (WORD)dwStoreID;
    Pak.byType          = (BYTE)nType;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncVIPLevel(int nConnIndex, int nVIPLevel, int nVIPExp)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Vip_Level Pak;

    Pak.protocolID = s2c_sync_vip_level;
    Pak.byVIPLevel = (BYTE)nVIPLevel;
    Pak.wVIPExp   = (WORD)nVIPExp;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncVIPEndTime(int nConnIndex, int nVIPEndTime)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Vip_End_Time Pak;

    Pak.protocolID = s2c_sync_vip_end_time;
    Pak.uVIPEndTime = (unsigned int)nVIPEndTime;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncVIPAutoRepairFlag(KPlayer* pPlayer, BOOL bAutoRepairFlag)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    KS2C_Sync_Vip_AutoRepair_Flag Pak;
    
    Pak.protocolID = s2c_sync_vip_autorepair_flag;
    Pak.bAutoRepairFlag = bAutoRepairFlag;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncMaxFatiguePoint(int nConnIndex, int nMaxFatiguePoint)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Max_Fatigue_Point Pak;

    Pak.protocolID = s2c_sync_max_fatigue_point;
    Pak.uMaxFatiguePoint = (unsigned int)nMaxFatiguePoint;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncFreeTalent(int nConnIndex, DWORD dwHeroTemplateID, int nFreeTalent)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Free_Talent Pak;

    Pak.protocolID = s2c_sync_free_talent;
    Pak.uHeroTemplateID = dwHeroTemplateID;
    Pak.uFreeTalent = (unsigned char)nFreeTalent;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncFreeClearCDTimes(int nConnIndex, int nFreeClearCDTimes)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Free_Clear_Cd_Times Pak;

    Pak.protocolID = s2c_sync_free_clear_cd_times;
    Pak.uFreeClearCDTimes = (unsigned char)nFreeClearCDTimes;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoStartDrama(KScene* pScene, int nDramaID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Start_Drama Pak;
    KBroadcastFunc BroadcastFunc;

    assert(pScene);

    Pak.protocolID = s2c_start_drama;
	Pak.wDramaID = (unsigned short)nDramaID;

    BroadcastFunc.m_pvData          = &Pak;
    BroadcastFunc.m_uSize           = sizeof(Pak);
    BroadcastFunc.m_dwExcept        = ERROR_ID;
    BroadcastFunc.m_dwExclusive     = 0;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoStopDrama(KScene* pScene)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Stop_Drama Pak;
    KBroadcastFunc BroadcastFunc;

    assert(pScene);

    Pak.protocolID = s2c_stop_drama;

    BroadcastFunc.m_pvData          = &Pak;
    BroadcastFunc.m_uSize           = sizeof(Pak);
    BroadcastFunc.m_dwExcept        = ERROR_ID;
    BroadcastFunc.m_dwExclusive     = 0;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoStartDialog(KScene* pScene, int nDialogID, BOOL bNeedSendCompleted)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Start_Dialog Pak;
	KBroadcastFunc BroadcastFunc;

	assert(pScene);

	Pak.protocolID = s2c_start_dialog;
	Pak.uDialogID = (unsigned short)nDialogID;
	Pak.byNeedSendCompleted = (BYTE)bNeedSendCompleted;

	BroadcastFunc.m_pvData          = &Pak;
	BroadcastFunc.m_uSize           = sizeof(Pak);
	BroadcastFunc.m_dwExcept        = ERROR_ID;
	BroadcastFunc.m_dwExclusive     = 0;
	
	pScene->TraverseHero(BroadcastFunc);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncPlayerValueGroup(KPlayer* pPlayer, int nGroupID)
{
	BOOL    bResult  = false;
	BOOL    bRetCode = false;
    size_t  uSize = 0;
    int     nValue = 0;
    KVEC_PLAYERVALUE_GROUP*         pGroup = NULL;
    KS2C_Sync_PlayerValue_Group*    pPak = (KS2C_Sync_PlayerValue_Group*)m_byTempData;
    KPLAYERVALUE_ITEM*              pPlayerValueItem = pPak->item;

    KGLOG_PROCESS_ERROR(pPlayer);

    pGroup = g_pSO3World->m_Settings.m_PlayerValueInfoList.GetGroup(nGroupID);
    KGLOG_PROCESS_ERROR(pGroup);

    pPak->protocolID    = s2c_sync_playervalue_group;
    pPak->groupID       = nGroupID;
    pPak->itemLength    = (unsigned short)pGroup->size();

    uSize = sizeof(*pPak) + pGroup->size() * sizeof(*pPlayerValueItem);
    KGLOG_PROCESS_ERROR(uSize <= sizeof(m_byTempData));

    for (KVEC_PLAYERVALUE_GROUP::iterator it = pGroup->begin(); it != pGroup->end(); ++it)
    {
        bRetCode = pPlayer->m_PlayerValue.GetValue((*it)->dwID, nValue);
        KGLOG_PROCESS_ERROR(bRetCode);

        pPlayerValueItem->id = (*it)->dwID;
        pPlayerValueItem->curValue = nValue;

        ++pPlayerValueItem;
    }

    bRetCode = Send(pPlayer->m_nConnIndex, pPak, uSize);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncOnePlayerValue(KPlayer* pPlayer, DWORD dwID, int nCurValue)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Sync_PlayerValue_Item Pak;

    Pak.protocolID = s2c_sync_playervalue_item;
    Pak.id = dwID;
    Pak.curValue = nCurValue;
	
    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncChallengeAward(
    int nConnIndex, KCHALLENGE_AWARD_TYPE eType, 
    KPlayer* pOtherPlayer, int nStartStep, int nEndStep,
    int nAwardMoney, int nAddMainHeroExp, int nAddAssistHeroExp, 
    std::vector<KCHALLENGEMISSION_AWARD>& rvecAwardItem
)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    size_t uItemCount = rvecAwardItem.size();
    KS2C_Sync_Challenge_Award* pPak = (KS2C_Sync_Challenge_Award*)&m_byTempData[0];
    KCHALLENGEMISSION_AWARD* pItem = NULL;
    size_t uPakSize = sizeof(KS2C_Sync_Challenge_Award) + sizeof(KCHALLENGEMISSION_AWARD) * uItemCount;
    KCHALLENGEMISSION_AWARD* pAwardItem = NULL;
    int nValuePoint = 0;

    KGLOG_PROCESS_ERROR(uPakSize < sizeof(m_byTempData));

    pPak->protocolID        = s2c_sync_challenge_award;
    
    pPak->uOtherPlayerID    = ERROR_ID;
    pPak->uOtherPlayerTemplateID = ERROR_ID;
    pPak->szOtherName[0] = '\0';

    if (pOtherPlayer)
    {
        pPak->uOtherPlayerID            = pOtherPlayer->m_dwID;
        pPak->uOtherPlayerTemplateID    = pOtherPlayer->m_dwMainHeroTemplateID;

        strncpy(pPak->szOtherName, pOtherPlayer->m_szName, countof(pPak->szOtherName));
        pPak->szOtherName[countof(pPak->szOtherName) - 1] = '\0';
    }

    pPak->byAwardType       = (BYTE)eType;
    pPak->uAddMainHeroExp   = (unsigned int)nAddMainHeroExp;
    pPak->uAddAssistHeroExp = (unsigned int)nAddAssistHeroExp;
    pPak->uAwardMoney       = (unsigned int)nAwardMoney;
    pPak->uStartStep        = (BYTE)nStartStep;
    pPak->uEndStep          = (BYTE)nEndStep;

    pPak->itemLength        = (unsigned short)uItemCount;

    for (size_t i = 0; i < uItemCount; ++i)
    {
        pItem = &pPak->item[i];
        pAwardItem = &rvecAwardItem[i];

        *pItem = *pAwardItem;
    }

    bRetCode = Send(nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoFinishChallengeMissionNotify(int nConnIndex, BOOL bSuccessed, int nStartStep, int nEndStep)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Finish_Challenge_Mission Pak;

    Pak.protocolID  = s2c_finish_challenge_mission;
    Pak.bSuccessed  = (BYTE)bSuccessed;
    Pak.byStartStep = (BYTE)nStartStep;
    Pak.byEndStep   = (BYTE)nEndStep;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncAchievementPoint(KPlayer* pPlayer, int nAchievementPoint)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Sync_Achievement_Point Pak;

    Pak.protocolID = s2c_sync_achievement_point;
    Pak.achievementPoint = nAchievementPoint;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoLeaveClubNotify(int nConnIndex)
{
    BOOL                        bResult  = false;
    BOOL                        bRetCode = false;
    KS2C_Leave_Club_Notify      Pak;

    Pak.protocolID = s2c_leave_club_notify;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoClubInviteNotify(int nConnIndex, DWORD dwClubID, DWORD dwLauncherID, const char cszClubName[], const char cszRoleName[])
{
    BOOL                        bResult  = false;
    BOOL                        bRetCode = false;
    KS2C_Invite_Join            sMsg; 

    sMsg.protocolID = s2c_invite_join;
    sMsg.dwClubID = dwClubID;
    sMsg.dwLauncherID = dwLauncherID;

    COPY_SZ(sMsg.szClubName, cszClubName);
    COPY_SZ(sMsg.szPlayerName, cszRoleName);

    bRetCode = Send(nConnIndex, &sMsg, sizeof(sMsg));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoApplyJoinClubRepsond(int nConnIndex, DWORD dwClubID, int nError)
{
    BOOL                         bResult  = false;
    BOOL                         bRetCode = false;
    KS2C_Apply_Join_Club_Respond Pak;

    Pak.protocolID = s2c_apply_join_club_respond;
    Pak.byError = (BYTE)nError;
    Pak.dwClubID = dwClubID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoPostModify(int nConnIndex, DWORD dwPlayerID, int nPost)
{
    BOOL                        bResult  = false;
    BOOL                        bRetCode = false;
    KS2C_Post_Modify            Pak;

    Pak.protocolID = s2c_post_modify;
    Pak.byPost     = (BYTE)nPost;
    Pak.dwPlayerID = dwPlayerID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
} 

BOOL KPlayerServer::DoClubBaseInfo(int nConnIndex, int nPost, BOOL bSwitchApply, DWORD dwClubID, int nInfoType, 
    const char cszClubName[], const char cszLeaderName[], int nCreateTime, int nMemberCount, 
    int nMaxMemberCount, size_t uNoticeLen, size_t uOnlineLen, size_t uIntroLen, BYTE* byData
)
{
    BOOL                        bResult  = false;
    BOOL                        bRetCode = false;
    KS2C_Club_BASE_Info*        pPak = (KS2C_Club_BASE_Info*)m_byTempData;;
    size_t                      uPakSize = sizeof(KS2C_Club_BASE_Info) + uNoticeLen + uOnlineLen + uIntroLen;

    KGLOG_PROCESS_ERROR(uPakSize <= MAX_EXTERNAL_PACKAGE_SIZE);

    pPak->protocolID = s2c_club_base_info;
    pPak->byPost = (BYTE)nPost;
    pPak->bySwitchApply = (BYTE)bSwitchApply;
    pPak->dwClubID = dwClubID;
    pPak->uIntroLen = (WORD)uIntroLen;
    pPak->uNoticeLen = (WORD)uNoticeLen;
    pPak->uOnlineLen = (WORD)uOnlineLen;
    pPak->nCreateTime = nCreateTime;
    pPak->byMaxMemberCount = (BYTE)nMaxMemberCount;
    pPak->byMemberCount = (BYTE)nMemberCount;
    pPak->byClubInfoType = (BYTE)nInfoType;

    strncpy(pPak->szName, cszClubName, sizeof(pPak->szName));
    pPak->szName[sizeof(pPak->szName) - 1] = '\0'; 
    strncpy(pPak->szLeader, cszLeaderName, sizeof(pPak->szLeader));
    pPak->szLeader[sizeof(pPak->szLeader) - 1] = '\0'; 

    memcpy(pPak->byData, byData, uOnlineLen + uNoticeLen + uIntroLen);
    pPak->byDataLength = pPak->uIntroLen + pPak->uNoticeLen + pPak->uOnlineLen;
    bRetCode = Send(nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncAllAchievementState(KPlayer* pPlayer, void* pData, size_t uSize)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    size_t uPakSize = 0;
	KS2C_Sync_All_Achievement_State* pPak = (KS2C_Sync_All_Achievement_State*)m_byTempData;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pData);

    uPakSize = uSize + sizeof(*pPak);

    KGLOG_PROCESS_ERROR(uPakSize <= sizeof(m_byTempData));
	
    pPak->protocolID = s2c_sync_all_achievement_state;
    pPak->dataLength = (unsigned short)uSize;
    memcpy(pPak->data, pData, uSize);

    bRetCode = Send(pPlayer->m_nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncOneAchievementState(KPlayer* pPlayer, DWORD dwAchievementID, BOOL bFinished)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Sync_One_Achievement_State Pak;

    KGLOG_PROCESS_ERROR(pPlayer);

    Pak.protocolID = s2c_sync_one_achievement_state;
    Pak.achievementID = dwAchievementID;
    Pak.finished = (BYTE)bFinished;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncOneAchievementProgress(KPlayer* pPlayer, DWORD dwAchievementID, int nCurValue)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Sync_One_Achievement_Progress Pak;

    Pak.protocolID = s2c_sync_one_achievement_progress;
    Pak.achievementID = dwAchievementID;
    Pak.curValue = nCurValue;

    bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncOneHeroUnlocktime(KPlayer* pPlayer, DWORD dwHeroTemplateID, time_t tUnlockTime)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Sync_One_Unlocktime Pak;

	Pak.protocolID = s2c_sync_one_unlocktime;
	Pak.heroTemplateID = dwHeroTemplateID;
	Pak.unlockTime = tUnlockTime;
	bRetCode = Send(pPlayer->m_nConnIndex, &Pak, sizeof(Pak));
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncGetTesterAward(int nConnIndex, int nGetTesterAwardLevel)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Tester_Award_Flag Pak;

    Pak.protocolID = s2c_sync_tester_award_flag;
    Pak.byTesterAwardLevel = (BYTE)nGetTesterAwardLevel;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncOnlineAwardInfo(int nConnIndex, int nOnlineAwardTimes, int nNextOnlineAwardTime)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Online_Award_Info Pak;

    int nLeftTime = nNextOnlineAwardTime - g_pSO3World->m_nCurrentTime;
    nLeftTime = MAX(0, nLeftTime);

    Pak.protocolID = s2c_sync_online_award_info;
    Pak.byOnlineAwardTimes = (BYTE)nOnlineAwardTimes;
    Pak.wAwardOnlineLeftTime = (unsigned short)nLeftTime;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncOnlineAwardItems(int nConnIndex, int nRealAwardIndex, int nItemCount, KAWARD_ITEM* pItems[])
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Online_Award_Items* pPak = (KS2C_Sync_Online_Award_Items*)m_byTempData;
    size_t uPakSize = sizeof(KS2C_Sync_Online_Award_Items) + sizeof(KAWARD_ITEM_INFO) * nItemCount;
    KAWARD_ITEM_INFO* pAwardItem = NULL;

    assert(pItems);

    KGLOG_PROCESS_ERROR(uPakSize <= MAX_EXTERNAL_PACKAGE_SIZE);

    pPak->protocolID = s2c_sync_online_award_items;
    pPak->byRealAwardItemIndex = (BYTE)nRealAwardIndex;
    pPak->itemsLength = (unsigned short)nItemCount;

    pAwardItem = &pPak->items[0];
    for (int i = 0; i < nItemCount; ++i, ++pAwardItem)
    {
        memset(pAwardItem, 0, sizeof(KAWARD_ITEM_INFO));

        if (pItems[i] == NULL)
            continue;

        pAwardItem->byTabType = (BYTE)pItems[i]->dwTabType;
        pAwardItem->wTabIndex = (WORD)pItems[i]->dwIndex;
        pAwardItem->wStackNum = (unsigned short)pItems[i]->nStackNum;
        pAwardItem->byMoneyType = (unsigned char)pItems[i]->eMoneyType;
        pAwardItem->wMoney = (unsigned short)pItems[i]->nMoney;
    }

    bRetCode = Send(nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncAwardItemInfo(int nConnIndex, int nAwardTableID, int nItemCount, KAWARD_ITEM* pItems[])
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Award_Table_Info* pPak = (KS2C_Sync_Award_Table_Info*)m_byTempData;
    size_t uPakSize = sizeof(KS2C_Sync_Award_Table_Info) + sizeof(KAWARD_ITEM_INFO) * nItemCount;
    KAWARD_ITEM_INFO* pAwardItem = NULL;

    assert(pItems);

    KGLOG_PROCESS_ERROR(uPakSize <= MAX_EXTERNAL_PACKAGE_SIZE);

    pPak->protocolID = s2c_sync_award_table_info;
    pPak->awardTableID = (unsigned short)nAwardTableID;
    pPak->infoLength = (unsigned short)nItemCount;

    pAwardItem = &pPak->info[0];
    for (int i = 0; i < nItemCount; ++i, ++pAwardItem)
    {
        memset(pAwardItem, 0, sizeof(KAWARD_ITEM_INFO));

        if (pItems[i] == NULL)
            continue;

        pAwardItem->byTabType = (BYTE)pItems[i]->dwTabType;
        pAwardItem->wTabIndex = (unsigned short)pItems[i]->dwIndex;
        pAwardItem->wStackNum = (unsigned short)pItems[i]->nStackNum;
        pAwardItem->byMoneyType = (BYTE)pItems[i]->eMoneyType;
        pAwardItem->wMoney = (unsigned short)pItems[i]->nMoney;
    }

    bRetCode = Send(nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDailySignAwardNotify(int nConnIndex, KAWARD_ITEM* pAwardItem)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Daily_Sign_Award_Notify Pak;

    assert(pAwardItem);

    Pak.protocolID = s2c_daily_sign_award_notify;
    Pak.awardItem.byTabType = (BYTE)pAwardItem->dwTabType;
    Pak.awardItem.wTabIndex = (unsigned short)pAwardItem->dwIndex;
    Pak.awardItem.wStackNum = (unsigned short)pAwardItem->nStackNum;
    Pak.awardItem.byMoneyType = (BYTE)pAwardItem->eMoneyType;
    Pak.awardItem.wMoney = (unsigned short)pAwardItem->nMoney;
	
    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncRandomQuest(int nConnIndex, uint32_t dwQuestID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Random_Quest Pak;

    Pak.protocolID  = s2c_sync_random_quest;
    Pak.dwQuestID   = dwQuestID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerServer::DoSyncHeroAddedExp(int nConnIndex, DWORD dwHeroTemplateID, int nAddedExp)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Sync_Hero_Added_Exp Pak;

    Pak.protocolID = s2c_sync_hero_added_exp;
    Pak.dwHeroTemplateID = dwHeroTemplateID;
    Pak.dwExp = nAddedExp;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSetBattleTemplate(KScene* pScene)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KS2C_Set_Battle_Template Pak;
    KBroadcastFunc BroadcastFunc;

    assert(pScene);

    Pak.protocolID  = s2c_set_battle_template;
    Pak.dwSceneID   = pScene->m_dwID;
    Pak.byMode      = (BYTE)pScene->m_Battle.m_eMode;
    Pak.wTotalScore = (WORD)pScene->m_Battle.m_nTotalScore;
    Pak.wTotalFrame = (WORD)pScene->m_Battle.m_nFramePerBattle;

    BroadcastFunc.m_pvData          = &Pak;
    BroadcastFunc.m_uSize           = sizeof(Pak);
    BroadcastFunc.m_dwExcept        = ERROR_ID;
    BroadcastFunc.m_dwExclusive     = 0;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoCastSlamBallSkill(KHero* pSlamer, DWORD dwSlamBallSkillID, KBasketSocket* pSocket, DWORD dwInterferenceID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Cast_Slamball_Skill Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;

    assert(pSlamer);
    assert(pSocket);

    pScene = pSlamer->m_pScene;
    assert(pScene);

    Pak.protocolID          = s2c_cast_slamball_skill;
    Pak.wHeroID             = (WORD)pSlamer->m_dwID;
	Pak.nX                  = pSlamer->m_nX;
    Pak.nY                  = pSlamer->m_nY;
    Pak.nZ                  = pSlamer->m_nZ;
    Pak.nRandomSeed         = pSlamer->m_nVirtualFrame;
    Pak.wSlamBallSkillID    = (WORD)dwSlamBallSkillID;
    Pak.wSocketID           = (WORD)pSocket->m_dwID;
    Pak.wInterferenceID     = (WORD)dwInterferenceID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    pScene->TraverseHero(BroadcastFunc);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoNormalSlamBallEx(KHero* pSlamer, DWORD dwSocketID, DWORD dwInterferenceID)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    KS2C_Normal_SlamBallEx  Pak;
    KBroadcastFunc          BroadcastFunc;
    KScene*                 pScene      = NULL;

    assert(pSlamer);
    assert(pSlamer->m_pScene);

    pScene = pSlamer->m_pScene;

    Pak.protocolID      = s2c_normal_slamballex;
    Pak.id              = pSlamer->m_dwID;
    Pak.x               = pSlamer->m_nX;
    Pak.y               = pSlamer->m_nY;
    Pak.z               = pSlamer->m_nZ;
    Pak.randomSeed      = pSlamer->m_nVirtualFrame;
    Pak.interferenceID  = dwInterferenceID;
    Pak.basketSocketID  = dwSocketID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = pSlamer->m_dwID;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoClearActivePlayerData(int nConnIndex)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Clear_Active_Player_Data   Pak;

    Pak.protocolID = s2c_clear_active_player_data;
    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncActiveProcess(int nConnIndex, int nCount, KActiveProcess* pActiveProcess)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Sync_Active_Process        *pPak       = (KS2C_Sync_Active_Process*)m_byTempData;
    size_t                          uSendSize   = sizeof(KS2C_Sync_Active_Process) + sizeof(KActiveProcess) * nCount;

    assert(pActiveProcess);

    pPak->protocolID = s2c_sync_active_process;
    pPak->activeProcessLength = (WORD)nCount;
    memcpy(pPak->activeProcess, pActiveProcess, sizeof(KActiveProcess) * nCount);

    bRetCode = Send(nConnIndex, pPak, uSendSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncActiveAward(int nConnIndex, int nCurActivePoint, int nOnlineTime, std::set<DWORD> setAwardAlreadyGet)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Sync_Active_Award*         pPak        = (KS2C_Sync_Active_Award*)m_byTempData;
    size_t                          uSendSize   = sizeof(KS2C_Sync_Active_Award);
    unsigned short*                 pData       = NULL; 
    int                             index       = 0;
    std::set<DWORD>::iterator       it;

    pPak->protocolID = s2c_sync_active_award;
    pPak->nCurActivePoint = nCurActivePoint;
    pPak->nOnlineTime = nOnlineTime;
    pPak->wAwardAlreadyGetLength = (WORD)setAwardAlreadyGet.size();
    pData = pPak->wAwardAlreadyGet;

    for (it = setAwardAlreadyGet.begin(); it != setAwardAlreadyGet.end(); ++it, ++index)
    {
        pData[index] = WORD(*it);
        uSendSize += sizeof(unsigned short);
    }

    bRetCode = Send(nConnIndex, pPak, uSendSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUpdateActiveProcess(int nConnIndex, int nID, int nValue, BOOL bFinished)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Update_Active_Process      Pak;

    Pak.protocolID = s2c_update_active_process;
    Pak.wId = (WORD)nID;
    Pak.nValue = nValue;
    Pak.byFinished = (BYTE)bFinished;

    bRetCode = Send(nConnIndex, &Pak, sizeof(KS2C_Update_Active_Process));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoUpdateActivePoint(int nConnIndex, int nPoint)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Update_Active_Point        Pak;

    Pak.protocolID = s2c_update_active_point;
    Pak.wActivePoint = (WORD)nPoint;

    bRetCode = Send(nConnIndex, &Pak, sizeof(KS2C_Update_Active_Point));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncCostFatiguePoint(int nConnIndex, int nCostFatiguePoint)
{
    BOOL                    bResult = false;
    BOOL                    bRetCode = false;
    KS2C_Cost_Fatigue_Point Pak;

    Pak.protocolID = s2c_cost_fatigue_point;
    Pak.wCostFatiguePoint = (WORD)nCostFatiguePoint;
    bRetCode = Send(nConnIndex, &Pak, sizeof(KS2C_Cost_Fatigue_Point));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoApplyActiveAwardRespond(int nConnIndex, DWORD dwAwardID, BOOL bSucc)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Apply_Active_Award_Respond Pak;

    Pak.protocolID = s2c_apply_active_award_respond;
    Pak.wActiveAwardID = (WORD)dwAwardID;
    Pak.bySucc = (BYTE)bSucc;

    bRetCode = Send(nConnIndex, &Pak, sizeof(KS2C_Apply_Active_Award_Respond));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoActiveLandMineNotify(KScene* pScene, DWORD dwLandMineID)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Active_Landmine_Notify     Pak;
    KBroadcastFunc          BroadcastFunc;

    Pak.protocolID = s2c_active_landmine_notify;
    Pak.dwLandMineID = dwLandMineID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = 0;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncSceneHeroDataEnd(int nConnIndex)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Sync_Scene_Hero_Data_End   Pak;

    Pak.protocolID      = s2c_sync_scene_hero_data_end;

    bRetCode = Send(nConnIndex, &Pak, sizeof(KS2C_Sync_Scene_Hero_Data_End));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoFinishCastSkillNotify(KScene* pScene, uint32_t dwHeroID)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    KS2C_Finish_Cast_Skill_Notify   Pak;
    
    KBroadcastFunc          BroadcastFunc;

    KGLOG_PROCESS_ERROR(pScene);

    Pak.protocolID      = s2c_finish_cast_skill_notify;
    Pak.dwHeroID        = dwHeroID;

    BroadcastFunc.m_pvData         = &Pak;
    BroadcastFunc.m_uSize          = sizeof(Pak);
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = 0;

    pScene->TraverseHero(BroadcastFunc);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoRemoteCall(int nConnIndex, const char* pcszFunName, KVARIABLE param1, KVARIABLE param2, KVARIABLE param3, KVARIABLE param4)
{
    BOOL						bResult				= false;
    BOOL						bRetCode			= false;
    IKG_Buffer*                 piSendBuffer        = NULL;
    KS2C_Remote_Call*			pPak		    	= (KS2C_Remote_Call*)m_byTempData;
    size_t                      uPakSize            = 0;
    size_t                      uUsedSize           = 0;
    BYTE*                       pbyOffset           = NULL;
    size_t                      uLeftSize           = 0;

    assert(pcszFunName);

    uPakSize = sizeof(KS2C_Remote_Call) + param1.GetPakSize() + param2.GetPakSize() + param3.GetPakSize() + param4.GetPakSize();
    KGLOG_PROCESS_ERROR(uPakSize <= sizeof(m_byTempData));
    
    pPak->protocolID = s2c_remote_call;

    strncpy(pPak->szFuncName, pcszFunName, countof(pPak->szFuncName));
    pPak->szFuncName[countof(pPak->szFuncName) - 1] = '\0';

    pPak->byParamDataLength = (unsigned short)(uPakSize - sizeof(KS2C_Remote_Call));

    pbyOffset = pPak->byParamData;
    uLeftSize = uPakSize;

    KG_PROCESS_SUCCESS(param1.t == rpcInvalid);
    uUsedSize = param1.PakData(pbyOffset, uLeftSize);
    pbyOffset += uUsedSize;
    uLeftSize -= uUsedSize;

    KG_PROCESS_SUCCESS(param2.t == rpcInvalid);
    uUsedSize = param2.PakData(pbyOffset, uLeftSize);
    pbyOffset += uUsedSize;
    uLeftSize -= uUsedSize;

    KG_PROCESS_SUCCESS(param3.t == rpcInvalid);
    uUsedSize = param3.PakData(pbyOffset, uLeftSize);
    pbyOffset += uUsedSize;
    uLeftSize -= uUsedSize;

    KG_PROCESS_SUCCESS(param4.t == rpcInvalid);
    uUsedSize = param4.PakData(pbyOffset, uLeftSize);
    pbyOffset += uUsedSize;
    uLeftSize -= uUsedSize;

Exit1:
    bRetCode = Send(nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSendFriendHeroList(DWORD dwPlayerID, DWORD dwFriendID, int nLevel, std::vector<DWORD>& vecHeroList, DWORD dwMainHeroID, DWORD dwAssistHeroID, const char cszName[])
{
    BOOL						   bResult				= false;
    BOOL						   bRetCode			= false;
    KS2C_Friend_Hero_List_Respond* pPak     = (KS2C_Friend_Hero_List_Respond*)m_byTempData;
    size_t                         uPakSize = sizeof(KS2C_Friend_Hero_List_Respond) + sizeof(DWORD) * vecHeroList.size(); 

    pPak->protocolID = s2c_friend_hero_list_respond;
    pPak->dwFriendID = dwFriendID;
    pPak->dwHeroListLength = (WORD)vecHeroList.size();
    pPak->dwMainHeroID = dwMainHeroID;
    pPak->dwAssistHeroID = dwAssistHeroID;
    pPak->byLevel = (BYTE)nLevel;
    COPY_SZ(pPak->szName, cszName);

    for (size_t i = 0; i < vecHeroList.size(); ++i)
    {
        pPak->dwHeroList[i] = vecHeroList[i];
    }
    
    bRetCode = DoSendToPlayer(dwPlayerID, (BYTE*)pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSendFriendHeroInfo(DWORD dwPlayerID, DWORD dwFriendID, KHeroData* pHeroData, KHeroPackage* pHeroPackage)
{
    BOOL						   bResult				= false;
    BOOL						   bRetCode			= false;
    KS2C_Friend_Hero_Info_Respond* pPak     = (KS2C_Friend_Hero_Info_Respond*)m_byTempData;
    size_t                         uPakSize = sizeof(KS2C_Friend_Hero_Info_Respond); 

    pPak->protocolID = s2c_friend_hero_info_respond;
    pPak->dwFriendID = dwFriendID;

    pPak->wLevel       = (WORD)pHeroData->m_nLevel;
    pPak->wTemplateID  = (WORD)pHeroData->m_dwTemplateID;
    pPak->wFashionID   = pHeroData->m_wFashionID;
    pPak->nExp         = pHeroData->m_nExp;
    pPak->ladderLevel  = (WORD)pHeroData->m_nLadderLevel;
    pPak->ladderExp    = pHeroData->m_nLadderExp;
    pPak->wFreeTalent  = (WORD)pHeroData->m_wFreeTalent;
    pPak->wTalent[egymWeightLift] = pHeroData->m_wTalent[egymWeightLift];
    pPak->wTalent[egymRunning]    = pHeroData->m_wTalent[egymRunning];
    pPak->wTalent[egymBoxing]     = pHeroData->m_wTalent[egymBoxing];
    
    assert(countof(pPak->itemdatalen) == eipTotal);

    bRetCode = pHeroPackage->GetBinaryDataForClient(pPak->itemdatalen, pPak->itemdata, MAX_EXTERNAL_PACKAGE_SIZE - uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPak->itemdataLength = 0;
    for (int i = 0; i < eipTotal; ++i)
        pPak->itemdataLength += pPak->itemdatalen[i];
    
    uPakSize += pPak->itemdataLength;
    bRetCode = DoSendToPlayer(dwPlayerID, (BYTE*)pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoBroadcastScenePause(KScene* pScene, BOOL bPause)
{
	BOOL bResult = false;
	BOOL bRetCode = false;

	KS2C_Notify_Scene_Pause Pak;
	KBroadcastFunc BroadcastFunc;

	Pak.protocolID  = s2c_notify_scene_pause;
	Pak.byPause = BYTE(bPause);

	BroadcastFunc.m_pvData            = &Pak;
	BroadcastFunc.m_uSize             = sizeof(Pak);
	BroadcastFunc.m_dwExclusive       = 0;

	bRetCode = pScene->TraverseHero(BroadcastFunc);
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoSyncRandomPlayerList(int nConnIndex, std::vector<KPlayer*>& vecRandomPlayerList)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	KS2C_Sync_Random_PlayerList Pak;
    KHeroData* pHeroData = NULL;

    memset(&Pak, 0, sizeof(Pak));

    Pak.protocolID = s2c_sync_random_playerlist;
    for (size_t i = 0; i < countof(Pak.randomPlayerList); ++i)
    {
        if (i >= vecRandomPlayerList.size())
            break;

        KS2C_PlayerItem& cItem = Pak.randomPlayerList[i];
        KPlayer* pPlayer = vecRandomPlayerList[i];
        KGLOG_PROCESS_ERROR(pPlayer);

        pHeroData = pPlayer->GetMainHeroData();
        KGLOG_PROCESS_ERROR(pHeroData);

        g_CStringCopy(cItem.name, pPlayer->m_szName);
        cItem.id            = pPlayer->m_dwID;
        cItem.vipLevel      = (BYTE)pPlayer->m_nVIPLevel;
        cItem.isVIP         = (BYTE)pPlayer->IsVIP();
        cItem.gender        = (BYTE)pPlayer->m_eGender;
        cItem.teamLogo      = (BYTE)pPlayer->m_nTeamLogoEmblemID;
        cItem.teamLogoBg    = (BYTE)pPlayer->m_nTeamLogoFrameID;
        cItem.level         = (BYTE)pHeroData->m_nLevel;
        cItem.ladderLevel   = (BYTE)pHeroData->m_nLadderLevel;
        cItem.clubID        = pPlayer->m_dwClubID;
    }

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerServer::DoGetEnterSceneHeroListRequest(int nConnIndex, KScene* pScene)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KPlayer* pPlayer = NULL;
    KHeroData* pHeroData = NULL;
    KS2C_Get_Enter_Scene_HeroList_Respond Pak;
    KCreateMapParam param;

    KGLOG_PROCESS_ERROR(pScene);

    Pak.protocolID = s2c_get_enter_scene_herolist_respond;
    Pak.playerCount = 0;

    param = pScene->m_Param;
    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0;  j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            strncpy(Pak.szPlayerName[Pak.playerCount], param.m_Name[i][j], _NAME_LEN);
            Pak.szPlayerName[Pak.playerCount][_NAME_LEN - 1] = '\0';
            Pak.level[Pak.playerCount] = (unsigned short)param.m_nNormalLevel[i][j];
            Pak.ladderLevel[Pak.playerCount] = (unsigned short)param.m_byLadderLevel[i][j];
            Pak.vipLevel[Pak.playerCount] = param.m_byVIPLevel[i][j];
            Pak.isVIP[Pak.playerCount] = param.m_bIsVIP[i][j] > 0;
            Pak.playerCount++;
        }
    }

    bRetCode = Send(nConnIndex, &Pak, sizeof(KS2C_Get_Enter_Scene_HeroList_Respond));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncEnterSceneProgress(int nConnIndex, DWORD dwPlayerID, int nIndex, int nProgress, KScene* pScene)
{
    BOOL bResult = false;
    KPlayer* pPlayer = NULL;
    KS2C_Sync_Enter_Scene_Progress Pak;
    
    KGLOG_PROCESS_ERROR(pScene);

    for (int i = 0; i < sidTotal; ++i)
    {
        for(int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            pPlayer = g_pSO3World->m_PlayerSet.GetObj(pScene->m_Param.m_dwMember[i][j]);
            if(!pPlayer || pPlayer->m_dwID == dwPlayerID)
                continue;

            Pak.protocolID = s2c_sync_enter_scene_progress;
            Pak.indexInTeam = (unsigned char)nIndex;
            Pak.progress = (unsigned char)nProgress;
            Send(pPlayer->m_nConnIndex, &Pak, sizeof(KS2C_Sync_Enter_Scene_Progress));
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncEquipDecomposeResult(int nConnIndex, std::vector<IItem*>& vecItems)
{
    BOOL						   bResult		= false;
    BOOL						   bRetCode		= false;
    KS2C_Sync_Equip_Decompose_Result* pPak      = (KS2C_Sync_Equip_Decompose_Result*)m_byTempData;
    size_t                         uPakSize     = 0 ; 
    int                            nItemCount   = 0;
    KItemTabInfo*                  pItemTabInfo = pPak->items;
    const KItemProperty*           pItemProperty = NULL;

    for (size_t i = 0; i < vecItems.size(); ++i)
    {
        if (vecItems[i] == NULL)
            continue;

        ++nItemCount;
    }

    uPakSize = sizeof(KS2C_Sync_Equip_Decompose_Result) + nItemCount * sizeof(KItemTabInfo);
    KGLOG_PROCESS_ERROR(uPakSize < MAX_EXTERNAL_PACKAGE_SIZE);

    pPak->protocolID = s2c_sync_equip_decompose_result;
    pPak->itemsLength = (unsigned short)nItemCount;

    for (size_t i = 0; i < vecItems.size(); ++i)
    {
        if (vecItems[i] == NULL)
            continue;

        pItemProperty = vecItems[i]->GetProperty();
        KGLOG_PROCESS_ERROR(pItemProperty);

        pItemTabInfo->byTabType = (BYTE)pItemProperty->dwTabType;
        pItemTabInfo->wTabIndex = (WORD)pItemProperty->dwTabIndex;
        pItemTabInfo->wStackNum = (WORD)pItemProperty->nStackNum;
        ++pItemTabInfo;
    }

    bRetCode = Send(nConnIndex, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncPlayerBuff(int nConnIndex, DWORD dwBuffID, int nLeftFrame, int nActiveCount)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    
    KS2C_Sync_Player_Buff Pak;

    Pak.protocolID = s2c_sync_player_buff;
    Pak.buffID = dwBuffID;
    Pak.leftFrame = nLeftFrame;
    Pak.activeCount = nActiveCount;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoDelPlayerBuff(int nConnIndex, DWORD dwBuffID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KS2C_Sync_Player_Buff Pak;

    Pak.protocolID = s2c_del_player_buff;
    Pak.buffID = dwBuffID;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::DoSyncCurrentCity(int nConnIndex, int nCity)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KS2C_Sync_Current_City Pak;

    Pak.protocolID = s2c_sync_current_city;
    Pak.city = (BYTE)nCity;

    bRetCode = Send(nConnIndex, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}
