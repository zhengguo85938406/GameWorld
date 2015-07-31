#include "stdafx.h"
#include "KPlayer.h"
#include "Common/CRC32.h"
#include "KSO3World.h"
#include "KScene.h"
#include "KPlayerServer.h"
#include "KBall.h"
#include "KRelayClient.h"
#include "KMissionMgr.h"
#include "KEquipStrengthenSetting.h"
#include "KLSClient.h"
#include "KClubDef.h"

BYTE KPlayer::s_byTempData[MAX_EXTERNAL_PACKAGE_SIZE];

KPlayer::KPlayer()
{
    m_nLevel                        = 1;
    m_nExp                          = 0;
    m_nGroupID                      = ERROR_ID;
    m_nTimer                        = 0;
    m_nConnIndex                    = -1;
    m_dwClientIP                    = 0;
    m_dwMapID                       = ERROR_ID;
    m_nCopyIndex                    = 0;
    m_pFightingHero                 = NULL;
    m_pAITeammate                   = NULL;
    m_nLastClientFrame              = 0;
    m_eGender                       = gNone;
    m_bIsRenaming                   = false;
    m_eConnectType                  = eInvalid;
    m_bCanRename                    = false;
    m_dwMainHeroTemplateID          = 0;
    m_dwAssistHeroTemplateID        = 0;
    m_nTalkWorldDailyCount          = 0;
    m_bInPVPRoom                    = false;
    m_bIsMatching                   = false;
    m_nLastHeroExpDecreasedTime     = 0;
    m_nHighestHeroLevel             = 0;
    m_nLadderLosingStreakCount      = 0;
    m_nGoldCostMoney                = 0;
    m_nCoin                         = 0;
    m_dwLimitPlayTimeFlag           = 0;
    m_nIBActionTime                	= 0;
    m_dwLimitOnlineSecond           = 0;
    m_dwLimitOfflineSecond          = 0;
    m_bIsLimited                    = false;
    m_bVIPAutoRepair                = false;
    m_nVIPLevel                     = 0;
    m_nVIPExp                       = 0;
    m_nVIPEndTime                   = 0;
    m_bExtPointLock                 = false;
    m_nAILevelCoe                   = 0;
    m_nCurFatiguePoint              = 0;
    m_nMaxFatiguePoint              = 0;
    m_nTodayBuyFatiguePointTimes    = 0;
    m_nForbidTalkTime              = 0;
    m_nLastDailyRefreshTime         = 0;
    m_nLastDailyRefreshTimeZero     = 0;
    m_dwClubID                      = 0;
    m_dwLastClubID                  = 0;
    m_nLastSaveTime                 = 0;
    m_nClubApplyNum                 = 0;
    m_nClubPost                     = eClub_Post_Normal;
    m_nHighestLadderLevel           = 1;

	m_nLastChallengeStep1           = 1;
    m_nStartChallengeStep1          = 0;
    m_nLastChallengeStep2           = 1;
    m_nStartChallengeStep2          = 0;
    m_nCurrentCity                  = emitPVE1;
    m_nTeamLogoFrameID              = 0;
    m_nTeamLogoEmblemID             = 0;
    m_bUseAssistHero                = false;
    m_nFailedChallengeStep          = 0;
    m_bNeedCheckCanEnter            = true;
    m_nOnlineGetAwardTimes          = 0;
    m_nOnlineAwardTime              = 0;
    m_nQuitClubTime                 = 0;
    m_nRandomQuestIndex             = 0;
    m_nSkipRandomQuestTime          = 0;

    m_nTotalGameTime                = 0;
    m_nLastLoginTime                = 0;
    m_nCurrentLoginTime             = 0;
    m_nCreateTime                   = 0;
    m_bFirstPvP                     = true;
    m_nCreateHeroTemplateID         = 0;
    m_bRefuseStranger               = false;
    m_nLastBattleType               = 0;
    m_nLastMissionLevel             = 0;
    m_nReportOnlineGetAward         = -1;
    m_bLuckDrawing                  = false;

    memset(m_szLastMissionName, 0, sizeof(m_szLastMissionName));
    memset(m_dwMissionAward, 0, sizeof(m_dwMissionAward));
    memset(m_dwMissionAwardCount, 0, sizeof(m_dwMissionAwardCount));
    memset(m_bMissionAwardChooseIndex, 0, sizeof(m_bMissionAwardChooseIndex));

    m_pbyPresentGoodInfo = NULL;
}

KPlayer::~KPlayer()
{
}

BOOL KPlayer::Init(void)
{
    BOOL bResult                    = false;
    BOOL bRetCode                   = false;
    BOOL bItemListInitFlag          = false;
    BOOL bMoneyMgrInitFlag          = false;
    BOOL bUserPreferencesInitFlag   = false;
    BOOL bCDTimerListInitFlag       = false;
    BOOL bQuestListInitFlag         = false;
    BOOL bHeroDataListInitFlag      = false;
    BOOL bHeroTrainingMgrInitFlag   = false;
    BOOL bMissionDataInitFlag       = false;
    BOOL bMakingMachineInitFlag     = false;
    BOOL bGymInitFlag               = false;
    BOOL bUpgradeQueueInitFlag      = false;
    BOOL bBusinessInitFlag          = false;
    BOOL bSecretaryInitFlag         = false;
    BOOL bWardrobeInitFlag          = false;
    BOOL bCheerleadingInitFlag      = false;
    BOOL bPlayerValueInitFlag       = false;
    BOOL bAchievementInitFlag       = false;
    BOOL bActivePlayerInitFlag      = false;
    BOOL bHireHeroInitFlag          = false;

    m_pFightingHero     = NULL;
    m_eGameStatus       = gsInvalid;
    m_vecOwnTeamLogo.clear();

    m_nLastClientFrame  = 0;

    bRetCode = m_ItemList.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bItemListInitFlag = true;

    bRetCode = m_MoneyMgr.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bMoneyMgrInitFlag = true;

    bRetCode = m_UserPreferences.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bUserPreferencesInitFlag = true;

    bRetCode = m_CDTimerList.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bCDTimerListInitFlag = true;

    bRetCode = m_QuestList.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bQuestListInitFlag = true;

    bRetCode = m_HeroDataList.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bHeroDataListInitFlag = true;

    bRetCode = m_HeroTrainingMgr.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bHeroTrainingMgrInitFlag = true;

    bRetCode = m_MissionData.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bMissionDataInitFlag = true;

    bRetCode = m_MakingMachine.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bMakingMachineInitFlag = true;

    bRetCode = m_Gym.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bGymInitFlag = true;

    bRetCode = m_UpgradeQueue.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bUpgradeQueueInitFlag = true;

    bRetCode = m_BusinessStreet.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bBusinessInitFlag = true;

    bRetCode  = m_Secretary.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bSecretaryInitFlag = true;

    bRetCode = m_Wardrobe.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bWardrobeInitFlag = true;

    bRetCode = m_CheerleadingMgr.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bCheerleadingInitFlag = true;

    bRetCode = m_PlayerValue.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bPlayerValueInitFlag = true;

    bRetCode = m_Achievement.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bAchievementInitFlag = true;

    bRetCode = m_ActivePlayer.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bActivePlayerInitFlag = true;

    bRetCode = m_HireHero.Init(this);
    KGLOG_PROCESS_ERROR(bRetCode);
    bHireHeroInitFlag = true;

    bRetCode = SetFatiguePointDefaultValue();
    KGLOG_PROCESS_ERROR(bRetCode);


    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bActivePlayerInitFlag)
        {
            m_ActivePlayer.UnInit();
            bActivePlayerInitFlag = false;
        }

        if (bAchievementInitFlag)
        {
            m_Achievement.UnInit();
            bAchievementInitFlag = false;
        }

        if (bHireHeroInitFlag)
        {
            m_HireHero.UnInit();
            bHireHeroInitFlag = false;
        }

        if (bPlayerValueInitFlag)
        {
            m_PlayerValue.UnInit();
            bPlayerValueInitFlag = false;
        }

        if (bCheerleadingInitFlag)
        {
            m_CheerleadingMgr.UnInit();
            bCheerleadingInitFlag = false;
        }

        if (bWardrobeInitFlag)
        {
            m_Wardrobe.UnInit();
            bWardrobeInitFlag = false;
        }

        if (bSecretaryInitFlag)
        {
            m_Secretary.UnInit();
            bSecretaryInitFlag = false;
        }

        if (bBusinessInitFlag)
        {
            m_BusinessStreet.UnInit();
            bBusinessInitFlag = false;
        }

        if (bUpgradeQueueInitFlag)
        {
            m_UpgradeQueue.UnInit();
            bUpgradeQueueInitFlag = false;
        }

        if (bMakingMachineInitFlag)
        {
            m_MakingMachine.UnInit();
            bMakingMachineInitFlag = false;
        }
        if (bMissionDataInitFlag)
        {
            m_MissionData.UnInit();
            bMissionDataInitFlag = false;
        }

        if (bHeroTrainingMgrInitFlag)
        {
            m_HeroTrainingMgr.UnInit();
            bHeroTrainingMgrInitFlag = false;
        }

        if (bHeroDataListInitFlag)
        {
            m_HeroDataList.UnInit();
            bHeroDataListInitFlag = false;
        }

        if (bQuestListInitFlag)
        {
            m_QuestList.UnInit();
            bQuestListInitFlag = false;
        }

        if (bCDTimerListInitFlag)
        {
            m_CDTimerList.UnInit();
            bCDTimerListInitFlag = false;
        }

        if (bUserPreferencesInitFlag)
        {
            m_UserPreferences.UnInit();
            bUserPreferencesInitFlag = false;
        }

        if (bMoneyMgrInitFlag)
        {
            m_MoneyMgr.UnInit();
            bMoneyMgrInitFlag = false;
        }

        if (bItemListInitFlag)
        {
            m_ItemList.UnInit();
            bItemListInitFlag = false;
        }
    }

    return bResult;
}

void KPlayer::UnInit(void)
{
    KMEMORY_FREE(m_pbyPresentGoodInfo);

    m_ActivePlayer.UnInit();
    m_Achievement.UnInit();
    m_HireHero.UnInit();
    m_PlayerValue.UnInit();
    m_CheerleadingMgr.UnInit();
    m_Secretary.UnInit();
    m_BusinessStreet.UnInit();
    m_UpgradeQueue.UnInit();
    m_Gym.UnInit();
    m_MakingMachine.UnInit();
    m_MissionData.UnInit();
    m_HeroTrainingMgr.UnInit();
    m_HeroDataList.UnInit();
    m_QuestList.UnInit();
    m_CDTimerList.UnInit();
    m_UserPreferences.UnInit();
    m_MoneyMgr.UnInit();
    m_ItemList.UnInit();

    g_pSO3World->m_FellowshipMgr.UnloadPlayerFellowship(m_dwID);

    return;
}

KHero* KPlayer::GetFightingHero() const
{
    return m_pFightingHero;
}

BOOL KPlayer::LoadFromProtoBuf(BYTE* pbyData, T3DB::KPB_SAVE_DATA* pLoadBuf, size_t uDataLen)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;

    g_PlayerServer.DoSyncPlayerBaseInfo(this);

    if (uDataLen != 0)
    {
        KGLOG_PROCESS_ERROR(pLoadBuf);
        pLoadBuf->Clear();
        bRetCode = LoadExtRoleDataFromProtoBuf(pbyData, pLoadBuf, uDataLen);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (m_nLastSaveTime == 0) // 首次登录做的一些操作
    {
        m_BusinessStreet.OnFirstLogin();
        m_nLastDailyRefreshTime = g_pSO3World->m_nCurrentTime;
        m_nLastDailyRefreshTimeZero = g_pSO3World->m_nCurrentTime;

        m_Secretary.m_bLoadComplete = true;

        UnApplyVIP(true);
    }

    if (m_eConnectType == eLogin)
        m_nOnlineAwardTime = GetNextOnlineAwardTime();
    g_PlayerServer.DoSyncOnlineAwardInfo(m_nConnIndex, m_nOnlineGetAwardTimes, m_nOnlineAwardTime);

    RefreshDailyVariable();

    g_pSO3World->m_FellowshipMgr.LoadFellowshipData(m_dwID);

    DoHeroExpWeeklyDecrease();

    m_Achievement.ProcessAfterDataLoaded();

    m_HireHero.ProcessAfterDataLoaded();

    m_ActivePlayer.SyncActivePlayer();

    g_PlayerServer.DoPlayerDataLoadFinished(m_nConnIndex);

    g_RelayClient.DoSyncMainHero(m_dwID, m_dwMainHeroTemplateID);

    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(KGLOG_ERR, "Role data error, ID(%u), Name(%s)\n", m_dwID, m_szName);
    }
    return bResult;
}

BOOL KPlayer::LoadExtRoleDataFromProtoBuf(BYTE* pbyData, T3DB::KPB_SAVE_DATA* pLoadBuf, size_t uDataLen)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    KRoleDataHeader*    pGlobalHeader   = NULL;
    size_t              uLeftSize       = uDataLen;
    BYTE*               pbyOffset       = pbyData;
    DWORD               dwCRC           = 0;

    assert(pbyData);
    KGLOG_PROCESS_ERROR(pLoadBuf);

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KRoleDataHeader));
    pGlobalHeader = (KRoleDataHeader*)pbyOffset;

    uLeftSize -= sizeof(KRoleDataHeader);
    pbyOffset += sizeof(KRoleDataHeader);

    KGLOG_PROCESS_ERROR(pGlobalHeader->dwVer == 0);
    KGLOG_PROCESS_ERROR(pGlobalHeader->dwLen == uLeftSize);

    dwCRC = CRC32(0, pbyOffset, (DWORD)uLeftSize);
    KGLOG_PROCESS_ERROR(dwCRC == pGlobalHeader->dwCRC);
    if (uLeftSize > 0)
    {
        bRetCode = pLoadBuf->ParsePartialFromArray(pbyOffset,uLeftSize);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadStateInfoFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadTeamLogoInfoFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadCustomDataFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = LoadPlayerBuffFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_HeroDataList.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_ItemList.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_MoneyMgr.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_UserPreferences.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_CDTimerList.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_QuestList.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_HeroTrainingMgr.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_MissionData.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_MakingMachine.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_UpgradeQueue.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_Gym.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_BusinessStreet.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_Secretary.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_Wardrobe.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_CheerleadingMgr.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_PlayerValue.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_Achievement.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_ActivePlayer.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
        bRetCode = m_HireHero.LoadFromProtoBuf(pLoadBuf);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadStateInfoFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    assert(pLoadBuf);

    const T3DB::KPB_ROLE_STATE_INFO& RoleStateInfo = pLoadBuf->rolestateinfo();

    m_nTalkWorldDailyCount = RoleStateInfo.talkworlddailycount();
    m_nLastHeroExpDecreasedTime = RoleStateInfo.lastheroexpdecreasedtime();
    m_nLadderLosingStreakCount = RoleStateInfo.ladderlosingstreakcount();
    m_nAILevelCoe = RoleStateInfo.ailevelcoe();
    m_nCurFatiguePoint = RoleStateInfo.curfatiguepoint();
    m_nVIPEndTime = RoleStateInfo.vipendtime();
    m_nTodayBuyFatiguePointTimes = RoleStateInfo.todaybuyfatiguepointtimes();
    m_nLastDailyRefreshTime = RoleStateInfo.lastdailyrefreshtime();
    m_nLastDailyRefreshTimeZero = RoleStateInfo.lastdailyrefreshtimezero();
    m_bVIPAutoRepair = RoleStateInfo.byvipautorepairflag();
    m_nClubApplyNum = RoleStateInfo.clubapplynum();
    m_nLastChallengeStep1 = RoleStateInfo.nlastchallengestep1();
    m_nStartChallengeStep1 = RoleStateInfo.nstartchallengestep1();
    m_nLastChallengeStep2 = RoleStateInfo.nlastchallengestep2();
    m_nStartChallengeStep2 = RoleStateInfo.nstartchallengestep2();
    m_nCurrentCity = RoleStateInfo.ncurrentcity();
    m_nOnlineGetAwardTimes = RoleStateInfo.nonlinegetawardtimes();
    m_nQuitClubTime = RoleStateInfo.quitclubtime();
    m_nRandomQuestIndex = RoleStateInfo.nrandomquestindex();
    m_nSkipRandomQuestTime = RoleStateInfo.nskiprandomquesttime();
    m_bFirstPvP = RoleStateInfo.firstpvp();
    m_bRefuseStranger = RoleStateInfo.refusestranger();

    bRetCode = SetVIPLevel(m_nVIPLevel, m_nVIPExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (m_eConnectType == eLogin)
    {
        if (m_nCurrentCity == 0) // 默认选择第一个城
            m_nCurrentCity = emitPVE1;

        m_nLastChallengeStep1 = (m_nLastChallengeStep1 - 1) / cdChallengeSmallStepCount * cdChallengeSmallStepCount + 1;
        m_nLastChallengeStep2 = (m_nLastChallengeStep2 - 1) / cdChallengeSmallStepCount * cdChallengeSmallStepCount + 1;

        g_PlayerServer.DoSyncPlayerStateInfo(this);

        bRetCode = SetVIPAutoRepairFlag(m_bVIPAutoRepair);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;

Exit0:
    return bResult;
}

BOOL KPlayer::LoadTeamLogoInfoFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    size_t  uTeamLogoCounts = 0;

    const T3DB::KPB_TeamLogo_Info& TeamLogo_Info = pLoadBuf->teamlogoinfo();

    m_nTeamLogoFrameID  = (WORD)TeamLogo_Info.teamlogoframeid();
    m_nTeamLogoEmblemID = (WORD)TeamLogo_Info.teamlogoemblemid();

    uTeamLogoCounts     = TeamLogo_Info.teamlogodata_size();

    for (size_t i = 0; i < uTeamLogoCounts; ++i)
    {
        const T3DB::KPB_TEAMLOGO_DATA& PBTeamLogooData = TeamLogo_Info.teamlogodata(i);
        WORD wTeamLogoID = 0;

        wTeamLogoID = (WORD)PBTeamLogooData.ownteamlogoid();

        m_vecOwnTeamLogo.push_back(wTeamLogoID);
    }

    if (m_eConnectType == eLogin)
    {
        bRetCode = g_PlayerServer.DoSyncTeamLogoInfo(this);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadPlayerBuffFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KReport_Buff_End param;

    const T3DB::KPB_BUFF_DATA_LIST& BuffListData = pLoadBuf->buffdatalistdb();
    int nBuffCount = BuffListData.buffdata_size();

    for (int i = 0; i < nBuffCount; ++i)
    {
        KBuffInfo cBuffInfo;
        KBuff* pBuff = NULL;
        const T3DB::KPB_BUFF_DATA& BuffData = BuffListData.buffdata(i);

        DWORD dwBuffID = BuffData.buffid();
        int nLeftFrame = BuffData.leftframe();
        int nActiveCount = BuffData.activecount();

        pBuff = g_pSO3World->m_BuffManager.GetBuff(dwBuffID);
        if (!pBuff) // 可能Buff已经不存在
            continue;

        if (pBuff->m_bTimeLapseOffline)
        {
            int nDeltaTime = g_pSO3World->m_nCurrentTime - m_nLastSaveTime;
            KGLOG_PROCESS_ERROR(nDeltaTime >= 0);
            nLeftFrame -= nDeltaTime * GAME_FPS;
        }

        if (nLeftFrame <= 0)
        {
            param.buffID = dwBuffID;
            m_Secretary.AddReport(KREPORT_EVENT_BUFF_END, (BYTE*)&param, sizeof(param));
            continue;
        }
        cBuffInfo.pBuff = pBuff;
        cBuffInfo.m_nEndFrame = nLeftFrame + g_pSO3World->m_nGameLoop;
        cBuffInfo.m_nActiveCount = nActiveCount;

        m_BuffTable[dwBuffID] = cBuffInfo;

        if (m_eConnectType == eLogin)   // if not login??
        {
            bRetCode = g_PlayerServer.DoSyncPlayerBuff(m_nConnIndex, dwBuffID, nLeftFrame, nActiveCount);
            KGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadCustomDataFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL bResult = false;

    assert(pLoadBuf);

    const T3DB::KPBCustomData& CustomData = pLoadBuf->customdata();
    const std::string& strCustom = CustomData.byffer();

    m_CustomData.Load((BYTE*)strCustom.data(),strCustom.size());

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LoadBaseInfo(KRoleBaseInfo* pBaseInfo)
{
    BOOL        bResult         = false;
    BOOL        bRetCode        = false;

    assert(pBaseInfo);

    m_nLastLoginTime        = pBaseInfo->nLastLoginTime;
    m_nCurrentLoginTime     = g_pSO3World->m_nCurrentTime;
    m_nTotalGameTime        = pBaseInfo->nTotalGameTime;
    m_nCreateTime           = pBaseInfo->nCreateTime;
    m_dwMapID               = pBaseInfo->CurrentPos.dwMapID;
    m_nCopyIndex            = pBaseInfo->CurrentPos.nMapCopyIndex;
    m_eGender               = (KGENDER)pBaseInfo->DBBaseInfo.byGender;
    m_nLastSaveTime         = pBaseInfo->DBBaseInfo.nLastSaveTime;
    m_nHighestHeroLevel     = pBaseInfo->DBBaseInfo.wHighestLevel;
    m_dwClubID              = pBaseInfo->DBBaseInfo.dwClubID;
    m_nHighestLadderLevel   = pBaseInfo->DBBaseInfo.wLadderLevel;
    m_nLevel                = pBaseInfo->byLevel;
    m_nExp                  = pBaseInfo->nExp;
    m_dwMainHeroTemplateID  = pBaseInfo->wMainHeroTemplateID;
    m_dwAssistHeroTemplateID = pBaseInfo->wAssistHeroTemplateID;
    m_nCreateHeroTemplateID  = pBaseInfo->byCreateHeroTemplateID;

    bRetCode = SetVIPLevel(pBaseInfo->DBBaseInfo.byVIPLevel, pBaseInfo->nVIPExp);
    KGLOG_PROCESS_ERROR(bRetCode);


    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveToProtoBuf(size_t* puUsedSize,T3DB::KPB_SAVE_DATA* pSaveBuf, BYTE* pbyBuffer,size_t uBufferSize)
{
    BOOL                    bResult	        = false;
    BOOL                    bRetCode        = false;
    DWORD                   dwExtDataLen    = 0;
    KRoleDataHeader*        pGlobalHeader   = NULL;
    BYTE*                   pbyOffset       = pbyBuffer;

    KGLOG_PROCESS_ERROR(pSaveBuf);
    KGLOG_PROCESS_ERROR(uBufferSize >= sizeof(KRoleDataHeader));

    pGlobalHeader = (KRoleDataHeader*)pbyOffset;
    pbyOffset += sizeof(KRoleDataHeader);

    pSaveBuf->Clear();

    bRetCode = SaveStateInfoToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = SaveTeamLogoInfoToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = SaveCustomDataToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = SavePlayerBuffFromProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_HeroDataList.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_ItemList.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_MoneyMgr.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_UserPreferences.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_CDTimerList.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_QuestList.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_HeroTrainingMgr.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_MissionData.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_MakingMachine.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_UpgradeQueue.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_Gym.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_BusinessStreet.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_Secretary.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_Wardrobe.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_CheerleadingMgr.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_PlayerValue.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_Achievement.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_ActivePlayer.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);
    bRetCode = m_HireHero.SaveToProtoBuf(pSaveBuf);
    KGLOG_PROCESS_ERROR(bRetCode);

    dwExtDataLen = pSaveBuf->ByteSize();

    KGLOG_PROCESS_ERROR(uBufferSize >= sizeof(KRoleDataHeader) + dwExtDataLen);

    pSaveBuf->SerializeWithCachedSizesToArray((::google::protobuf::uint8*)pbyOffset);

    *puUsedSize = sizeof(KRoleDataHeader) + dwExtDataLen;

    pGlobalHeader->dwVer = 0;
    pGlobalHeader->dwCRC = CRC32(0, pbyOffset, dwExtDataLen);
    pGlobalHeader->dwLen = dwExtDataLen;

    m_nLastSaveTime = g_pSO3World->m_nCurrentTime;

    m_nNextSaveFrame = g_pSO3World->m_nGameLoop + g_pSO3World->m_Settings.m_ConstList.nSaveInterval * GAME_FPS;

    g_pSO3World->m_FellowshipMgr.SaveFellowshipData(m_dwID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveBaseInfo(KRoleBaseInfo* pBaseInfo)
{
    BOOL    bResult = false;

    pBaseInfo->nVersion                     = CURRENT_BASEINFO_VERSION;
    pBaseInfo->CurrentPos.dwMapID           = m_dwMapID;
    pBaseInfo->CurrentPos.nMapCopyIndex     = m_nCopyIndex;
    pBaseInfo->DBBaseInfo.nLastSaveTime                = (int32_t)g_pSO3World->m_nCurrentTime;
    pBaseInfo->nLastLoginTime               = (int32_t)m_nCurrentLoginTime;
    pBaseInfo->nCreateTime                  = (int32_t)m_nCreateTime;
    pBaseInfo->nTotalGameTime               = (int)(m_nTotalGameTime + (g_pSO3World->m_nCurrentTime - m_nCurrentLoginTime));
    pBaseInfo->DBBaseInfo.byGender          = (BYTE)m_eGender;
    pBaseInfo->DBBaseInfo.wHighestLevel     = (WORD)m_nHighestHeroLevel;
    pBaseInfo->DBBaseInfo.byVIPLevel        = (BYTE)m_nVIPLevel;
    pBaseInfo->DBBaseInfo.byIsVIP           = (BYTE)IsVIP();
    pBaseInfo->DBBaseInfo.byTeamLogoBg        = (BYTE)m_nTeamLogoFrameID;
    pBaseInfo->DBBaseInfo.byTeamLogo      = (BYTE)m_nTeamLogoEmblemID;
    pBaseInfo->nVIPExp                      = m_nVIPExp;
    pBaseInfo->byLevel						= (BYTE)m_nLevel;
    pBaseInfo->nExp 						    = m_nExp;
    pBaseInfo->DBBaseInfo.dwClubID          = m_dwClubID;
    pBaseInfo->DBBaseInfo.wLadderLevel      = (WORD)m_nHighestLadderLevel;
    pBaseInfo->DBBaseInfo.byHeroCount       = (BYTE)m_HeroDataList.GetActiveHeroCount();
    pBaseInfo->wMainHeroTemplateID          = (WORD)m_dwMainHeroTemplateID;
    pBaseInfo->wAssistHeroTemplateID        = (WORD)m_dwAssistHeroTemplateID;
    pBaseInfo->byCreateHeroTemplateID       = (BYTE)m_nCreateHeroTemplateID;

    memset(pBaseInfo->byReserved, 0, sizeof(pBaseInfo->byReserved));

    g_LSClient.DoRemoteCall("OnUpdateLastSaveTime", (int)m_dwID, (int)m_nCurrentLoginTime);

    bResult = true;
Exit0:
    return bResult;
}
BOOL KPlayer::SaveCustomDataToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult = false;
    T3DB::KPBCustomData* pCustomData = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pCustomData = pSaveBuf->mutable_customdata();
    pCustomData->set_byffer(m_CustomData.GetBuf(),m_CustomData.GetSize());

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveStateInfoToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                    bResult                 = false;
    T3DB::KPB_ROLE_STATE_INFO* pRoleStateInfo          = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pRoleStateInfo = pSaveBuf->mutable_rolestateinfo();

    pRoleStateInfo->set_talkworlddailycount(m_nTalkWorldDailyCount);
    pRoleStateInfo->set_lastheroexpdecreasedtime(m_nLastHeroExpDecreasedTime);
    pRoleStateInfo->set_ladderlosingstreakcount(m_nLadderLosingStreakCount);
    pRoleStateInfo->set_ailevelcoe(m_nAILevelCoe);
    pRoleStateInfo->set_curfatiguepoint(m_nCurFatiguePoint);
    pRoleStateInfo->set_vipendtime(m_nVIPEndTime);
    pRoleStateInfo->set_todaybuyfatiguepointtimes(m_nTodayBuyFatiguePointTimes);
    pRoleStateInfo->set_lastdailyrefreshtime(m_nLastDailyRefreshTime);
    pRoleStateInfo->set_lastdailyrefreshtimezero(m_nLastDailyRefreshTimeZero);
    pRoleStateInfo->set_byvipautorepairflag(m_bVIPAutoRepair);
    pRoleStateInfo->set_clubapplynum(m_nClubApplyNum);
    pRoleStateInfo->set_nlastchallengestep1(m_nLastChallengeStep1);
    pRoleStateInfo->set_nstartchallengestep1(m_nStartChallengeStep1);
    pRoleStateInfo->set_nlastchallengestep2(m_nLastChallengeStep2);
    pRoleStateInfo->set_nstartchallengestep2(m_nStartChallengeStep2);
    pRoleStateInfo->set_ncurrentcity(m_nCurrentCity);
    pRoleStateInfo->set_nonlinegetawardtimes(m_nOnlineGetAwardTimes);
    pRoleStateInfo->set_quitclubtime(m_nQuitClubTime);
    pRoleStateInfo->set_nrandomquestindex(m_nRandomQuestIndex);
    pRoleStateInfo->set_nskiprandomquesttime(m_nSkipRandomQuestTime);
    pRoleStateInfo->set_firstpvp(m_bFirstPvP);
    pRoleStateInfo->set_refusestranger(m_bRefuseStranger);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SaveTeamLogoInfoToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                        bResult = false;
    T3DB::KPB_TeamLogo_Info*    pTeamLogoInfo   = NULL;
    T3DB::KPB_TEAMLOGO_DATA*    pTeamLogoData   = NULL;

    pTeamLogoInfo = pSaveBuf->mutable_teamlogoinfo();

    pTeamLogoInfo->set_teamlogoframeid(m_nTeamLogoFrameID);
    pTeamLogoInfo->set_teamlogoemblemid(m_nTeamLogoEmblemID);

    for (std::vector<WORD>::iterator it = m_vecOwnTeamLogo.begin(); it != m_vecOwnTeamLogo.end(); ++it)
    {
        pTeamLogoData = pTeamLogoInfo->add_teamlogodata();
        pTeamLogoData->set_ownteamlogoid(*it);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SavePlayerBuffFromProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult = false;

    KBuffTable::const_iterator  itBuff;
    T3DB::KPB_BUFF_DATA_LIST*   pBuffListData   = NULL;

    pBuffListData = pSaveBuf->mutable_buffdatalistdb();
    KGLOG_PROCESS_ERROR(pBuffListData);

    for (itBuff = m_BuffTable.begin(); itBuff != m_BuffTable.end(); ++itBuff)
    {
        const KBuffInfo* pBuffInfo = &itBuff->second;
        int nLeftFrameCount = pBuffInfo->m_nEndFrame - g_pSO3World->m_nGameLoop;
        if (nLeftFrameCount > 0 && pBuffInfo->pBuff && pBuffInfo->pBuff->m_bNeedSave)
        {
            T3DB::KPB_BUFF_DATA* pBuffData = pBuffListData->add_buffdata();
            KGLOG_PROCESS_ERROR(pBuffData);
            pBuffData->set_buffid(pBuffInfo->pBuff->m_dwID);
            pBuffData->set_leftframe(nLeftFrameCount);
            pBuffData->set_activecount(pBuffInfo->m_nActiveCount);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::Logout()
{
    BOOL        bRetCode    = false;
    KScene*     pScene      = NULL;
    DWORD       dwHeroID    = ERROR_ID;
    DWORD       uChooseIndex = 0;


    while (m_dwMissionAwardCount[KAWARD_CARD_TYPE_NORMAL])
    {
        Award(KAWARD_CARD_TYPE_NORMAL, uChooseIndex++);
        if (uChooseIndex > cdCardCount)
            break;
    }

    uChooseIndex = 0;
    while (m_dwMissionAwardCount[KAWARD_CARD_TYPE_VIP])
    {
        Award(KAWARD_CARD_TYPE_VIP, uChooseIndex++);
        if (uChooseIndex > cdVipCardCount)
            break;
    }

    memset(m_dwMissionAward, 0, sizeof(m_dwMissionAward));
    memset(m_dwMissionAwardCount, 0, sizeof(m_dwMissionAwardCount));
    memset(m_bMissionAwardChooseIndex, 0, sizeof(m_bMissionAwardChooseIndex));

    KG_PROCESS_ERROR(m_pFightingHero);

    pScene = m_pFightingHero->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    if (pScene->m_nType == emtContinuousChallengeMap)
    {
        KMission* pMission = NULL;

        pMission = g_pSO3World->m_MissionMgr.GetMission(pScene->m_dwMapID);
        KGLOG_PROCESS_ERROR(pMission);

        g_LogClient.DoFlowRecord(
            frmtBattle, bfrChallengeDirectEnd, "%s,%d,%d,%d", m_szName, pMission->eType, pMission->nStep, pMission->nLevel
        );

        PLAYER_LOG(this, "challenge,exception,%d", pMission->nStep);
    }

    if (pScene->m_nType == emtPVPMap)
    {
        g_LogClient.DoFlowRecord(frmtBattle, bfrLadderPVPDirectEnd, "%s", m_szName);
    }

    pScene->RemoveHero(m_pFightingHero);
    m_pFightingHero = NULL;

    if (m_pAITeammate)
    {
        pScene->RemoveHero(m_pAITeammate);
        m_pAITeammate = NULL;
    }

Exit0:
    return;
}

BOOL KPlayer::SwitchMap(DWORD dwMapID, int nCopyIndex)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    if (m_eGameStatus == gsPlaying) // 直接场景中切换到另一场景
    {
        KHero* pHero = GetFightingHero();
        assert(pHero);
        assert(pHero->m_pScene);

        pHero->m_pScene->RemoveHero(pHero);
        if (m_pAITeammate)
            pHero->m_pScene->RemoveHero(m_pAITeammate);

        ReturnToHall();
    }

    KG_PROCESS_ERROR(m_eGameStatus == gsInHall);

    //g_RelayClient.SaveRoleData(this);
    g_RelayClient.DoSearchMapRequest(this, dwMapID, nCopyIndex);

    KGLogPrintf(KGLOG_INFO, "player %d search map %d request!", m_dwID, dwMapID);

    m_eGameStatus = gsSearchMap;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::RemoveHero()
{
    KHero* pHero = GetFightingHero();
    if (pHero)
    {
        assert(pHero->m_pScene);
        pHero->m_pScene->RemoveHero(pHero);

        if (m_pAITeammate)
            pHero->m_pScene->RemoveHero(m_pAITeammate);
    }

Exit0:
    return true;
}

void KPlayer::ReturnToHall()
{
    m_eGameStatus   = gsInHall;
    m_dwMapID       = ERROR_ID;
    m_nCopyIndex    = 0;
    g_RelayClient.DoPlayerEnterHallNotify(m_dwID);
    g_LSClient.DoRemoteCall("OnUpdateBattleStatus", (int)m_dwID, FALSE);
}

BOOL KPlayer::CallLoginScript()
{
    BOOL    bResult         = false;
    BOOL    bRetCode        = false;
    int     nLuaTopIndex    = 0;

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(PLAYER_SCRIPT);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nLuaTopIndex);
    g_pSO3World->m_ScriptManager.Push(this);
    g_pSO3World->m_ScriptManager.Call(PLAYER_SCRIPT, "OnPlayerLogin", 0);
    g_pSO3World->m_ScriptManager.SafeCallEnd(nLuaTopIndex);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CanBuyHeroSlot()
{
    BOOL    bResult 	= false;
    BOOL    bRetCode 	= false;
    int     nPayMoney   = 0;
    int     nCurMoney   = 0;

    nPayMoney = g_pSO3World->m_Settings.m_ConstList.GetHeroSlotPrice(m_HeroDataList.GetMaxHeroCount() + 1);
    KGLOG_PROCESS_ERROR(nPayMoney > 0);
    nCurMoney = m_MoneyMgr.GetMoney(emotCoin);
    KGLOG_PROCESS_ERROR(nCurMoney >= nPayMoney);

    KG_PROCESS_ERROR(m_HeroDataList.GetMaxHeroCount() < g_pSO3World->m_Settings.m_ConstList.nMaxHeroSlot);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::BuyHeroSlot()
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    int  nPayMoney = 0;

    bRetCode = CanBuyHeroSlot();
    KGLOG_PROCESS_ERROR(bRetCode);

    nPayMoney = g_pSO3World->m_Settings.m_ConstList.GetHeroSlotPrice(m_HeroDataList.GetMaxHeroCount() + 1);
    KGLOG_PROCESS_ERROR(nPayMoney > 0);

    bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(this, uctBuyHeroSlot, nPayMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CanFireHero(DWORD dwTemplateID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KHeroData* pHero = NULL;
    KHeroTemplateInfo* pHeroTemplate = NULL;
    KHeroPackage* pPackage = NULL;
    KUSING_EQUIP_INFO* pInfo = NULL;

    pHeroTemplate = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroTemplate);
    KGLOG_PROCESS_ERROR(pHeroTemplate->nCanFire);

    KGLOG_PROCESS_ERROR(m_HeroDataList.GetActiveHeroCount() > 2);
    pHero = m_HeroDataList.GetHeroData(dwTemplateID);
    KGLOG_PROCESS_ERROR(pHero && pHero->m_bFired == false);

    pPackage = m_ItemList.GetHeroPackage(dwTemplateID);
    KGLOG_PROCESS_ERROR(pPackage && pPackage->GetSize() == pPackage->GetFreeRoomSize());

    pInfo = m_Gym.GetHeroUsingEquipInfo(dwTemplateID);
    KGLOG_PROCESS_ERROR(pInfo == NULL);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::FireHero(DWORD dwTemplateID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KHeroData* pHero = NULL;
    KHeroData* pNewHero = NULL;

    bRetCode = CanFireHero(dwTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pHero = m_HeroDataList.GetHeroData(dwTemplateID);
    KGLOG_PROCESS_ERROR(pHero && pHero->m_bFired == false);

    bRetCode = m_HeroDataList.Fire(dwTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_PlayerServer.DoFireHeroRespond(m_nConnIndex, dwTemplateID);

    KG_PROCESS_SUCCESS(dwTemplateID != m_dwMainHeroTemplateID && dwTemplateID != m_dwAssistHeroTemplateID);

    pNewHero = m_HeroDataList.FindFreeHero();

    assert(pNewHero);

    if (dwTemplateID == m_dwMainHeroTemplateID)
    {
        SelectMainHero(pNewHero->m_dwTemplateID);
        goto Exit1;
    }

    if (dwTemplateID == m_dwAssistHeroTemplateID)
    {
        SelectAssistHero(pNewHero->m_dwTemplateID);
        goto Exit1;
    }

Exit1:
    g_LogClient.DoFlowRecord(frmtHeroBuyAndFire, hbaffrFireHero, "%s,%u", m_szName, dwTemplateID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CheckUnLockHero(DWORD dwTemplateID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KHeroTemplateInfo* pHeroTemplate = NULL;

    pHeroTemplate = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
    KG_PROCESS_ERROR(pHeroTemplate);

    switch (pHeroTemplate->eUnlockType)
    {
    case UNLOCK_TYPE_CANNOT:
        KG_PROCESS_ERROR(0);
        break;
    case UNLOCK_TYPE_NONE:
        break;
    case UNLOCK_TYPE_QUEST:
        if (pHeroTemplate->dwLockData > 0)
        {
            QUEST_STATE eState = m_QuestList.GetQuestState(pHeroTemplate->dwLockData);
            KG_PROCESS_ERROR(eState == qsFinished);
        }
        break;
    case UNLOCK_TYPE_ACHIEVEMENT:
        if (pHeroTemplate->dwLockData > 0)
        {
            bRetCode = m_Achievement.IsAchievementFinished(pHeroTemplate->dwLockData);
            KG_PROCESS_ERROR(bRetCode);
        }
        break;
    default:
        break;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CanBuyHero(DWORD dwTemplateID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KHeroTemplateInfo* pHeroTemplate = NULL;
    KHeroData* pHero = NULL;
    int nActiveHeroCount = 0;
    int nMaxHeroCount = 0;

    nActiveHeroCount    = (int)m_HeroDataList.GetActiveHeroCount();
    nMaxHeroCount       =      m_HeroDataList.GetMaxHeroCount();

    KG_PROCESS_ERROR(nActiveHeroCount < nMaxHeroCount);

    pHeroTemplate = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
    KG_PROCESS_ERROR(pHeroTemplate);

    bRetCode = CheckUnLockHero(dwTemplateID);
    KG_PROCESS_ERROR(bRetCode);

    //检查是否已经有该英雄
    pHero = m_HeroDataList.GetHeroData(pHeroTemplate->dwTemplateID);
    KG_PROCESS_ERROR(pHero == NULL || pHero->m_bFired);

    if (pHeroTemplate->nBuyPrice > 0)
    {
        int nCurMoney = m_MoneyMgr.GetMoney((ENUM_MONEY_TYPE)pHeroTemplate->nBuyMoneyType);
        KG_PROCESS_ERROR(nCurMoney >= pHeroTemplate->nBuyPrice);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::BuyHero(DWORD dwTemplateID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KHeroTemplateInfo* pHeroTemplate = NULL;    
    int nBuyHeroPrice = -1;
    int nMoneyType    = -1;
    KCUSTOM_CONSUME_INFO CCinfo = {0};

    bRetCode = CanBuyHero(dwTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    pHeroTemplate = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroTemplate);

    if (pHeroTemplate->nBuyPrice > 0)
    {
        // 计算打折
        nBuyHeroPrice = m_HireHero.BuyHeroPrice(dwTemplateID);
        KGLOG_PROCESS_ERROR(nBuyHeroPrice != -1);

        nMoneyType = pHeroTemplate->nBuyMoneyType;
        bRetCode = m_MoneyMgr.CanAddMoney((ENUM_MONEY_TYPE)nMoneyType, -nBuyHeroPrice);
        KGLOG_PROCESS_ERROR(bRetCode);

        if (nMoneyType == emotCoin)
        {
            CCinfo.nValue1 = dwTemplateID;
            bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(this, uctBuyHero, nBuyHeroPrice, &CCinfo);
            KGLOG_PROCESS_ERROR(bRetCode);
            goto Exit1;
        }

        bRetCode = m_MoneyMgr.AddMoney((ENUM_MONEY_TYPE)nMoneyType, -nBuyHeroPrice);
        KGLOG_PROCESS_ERROR(bRetCode);
        PLAYER_LOG(this, "money,costmoney,%d,%s,%d-%d,%d,%d", nMoneyType, "buyhero", 0, 0, 0, nBuyHeroPrice);
    }

    DoBuyHero(dwTemplateID);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayer::DoBuyHero(DWORD dwTemplateID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KHeroData* pHero = NULL;
    KHeroTemplateInfo* pHeroTemplate = NULL;    
    int nBuyHeroPrice = -1;

    pHeroTemplate = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroTemplate(dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroTemplate);

    nBuyHeroPrice = pHeroTemplate->nBuyPrice;

    pHero = m_HeroDataList.GetHeroData(pHeroTemplate->dwTemplateID);
    if (pHero == NULL)
    {
        bRetCode = AddNewHero(dwTemplateID, 1);
        KGLOG_PROCESS_ERROR(bRetCode);

        g_PlayerServer.DoBuyHeroRespond(m_nConnIndex, dwTemplateID);
        goto Exit1;
    }

    assert(pHero->m_bFired);
    bRetCode = m_HeroDataList.UnFire(dwTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);
    // sync
    g_PlayerServer.DoUnFireHeroRespond(m_nConnIndex,dwTemplateID);
    g_LSClient.DoRemoteCall("OnUpdateHeroCount", (int)m_dwID, (int)m_HeroDataList.GetActiveHeroCount());

Exit1:
    g_LogClient.DoFlowRecord(
        frmtHeroBuyAndFire, hbaffrBuyHero,
        "%s,%d,%u",
        m_szName,
        pHeroTemplate ? nBuyHeroPrice : 0,
        dwTemplateID
        );

    m_HireHero.RemoveHeroUnlockTime(dwTemplateID);
	OnEvent(peEmployHero, dwTemplateID);
    bResult = true;
Exit0:
    return bResult;

}

BOOL KPlayer::AddNewHero(DWORD dwTemplateID, int nLevel)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KHeroPackage* pPackage = NULL;
    KHeroData* pHeroData = NULL;
    KReport_New_Hero param;

    bRetCode = m_HeroDataList.CanAddHeroData(dwTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_HeroDataList.AddHeroData(dwTemplateID, nLevel);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPackage = m_ItemList.AddHeroPackage(dwTemplateID);
    KGLOG_PROCESS_ERROR(pPackage);

    pHeroData = m_HeroDataList.GetHeroData(dwTemplateID);
    assert(pHeroData);

    UpdateLevel(pHeroData->m_nLevel);

    bRetCode = g_PlayerServer.DoSyncHeroData(m_nConnIndex, pHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    param.heroTemplateID = dwTemplateID;
    bRetCode = m_Secretary.AddReport(KREPORT_EVENT_NEW_HERO, (BYTE*)&param, sizeof(param));
    KGLOG_PROCESS_ERROR(bRetCode);

    g_LSClient.DoRemoteCall("OnUpdateHeroCount", (int)m_dwID, (int)m_HeroDataList.GetActiveHeroCount());
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::DelHero(DWORD dwTemplateID)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = m_HeroDataList.RemoveHeroData(dwTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_ItemList.RemoveHeroPackage(dwTemplateID);

    g_PlayerServer.DoDelHeroData(m_nConnIndex, dwTemplateID);

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::UpdateHeroData(KHero* pHero)
{
    /*KHeroData* pHeroData = NULL;

    assert(pHero);

    pHeroData = m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    Exit0:*/
    return;
}

BOOL KPlayer::ApplyPlayerBuff(KHero* pHero)
{
    BOOL bResult    = false;
    KBuffTable::const_iterator  itBuff;

    assert(pHero);

    for (itBuff = m_BuffTable.begin(); itBuff != m_BuffTable.end(); ++itBuff)
    {
        const KBuffInfo* pBuffInfo = &itBuff->second;
        if (pBuffInfo->pBuff && pBuffInfo->m_nEndFrame > pHero->m_nCreateTime)
        {
            pHero->ApplyAttribute(pBuffInfo->pBuff->m_pRollBackAttr);
            // pResult->ApplyAttribute(pBuffInfo->pBuff->m_pActiveAttr);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::UnApplyPlayerBuff(KHero* pHero)
{
    KBuffTable::const_iterator  itBuff;

    assert(pHero);

    for (itBuff = m_BuffTable.begin(); itBuff != m_BuffTable.end(); ++itBuff)
    {
        const KBuffInfo* pBuffInfo = &itBuff->second;
        if (pBuffInfo->pBuff && pBuffInfo->m_nEndFrame > pHero->m_nCreateTime)
        {
            pHero->UnApplyAttribute(pBuffInfo->pBuff->m_pRollBackAttr);
            // pResult->ApplyAttribute(pBuffInfo->pBuff->m_pActiveAttr);
        }
    }

Exit0:
    return;
}

BOOL KPlayer::ApplyHeroEquip(KHero* pHero)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KHeroPackage* pHeroPackage = NULL;

    assert(pHero);
    pHeroPackage = m_ItemList.GetHeroPackage(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroPackage);

    bRetCode = pHeroPackage->ApplyEquip(pHero);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::UnApplyHeroEquip(KHero* pHero)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KHeroPackage* pHeroPackage = NULL;

    assert(pHero);
    pHeroPackage = m_ItemList.GetHeroPackage(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroPackage);

    bRetCode = pHeroPackage->UnApplyEquip(pHero);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

BOOL KPlayer::ApplyHeroTalent(KHero* pHero)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KHeroData* pData = NULL;
    KHeroTalentAttr* pAttrInfo = NULL;
    KAttribute attr;

    assert(pHero);

    pData = m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pData);

    for (int i = egymWeightLift; i < egymTotal; ++i)
    {
        if (pData->m_wTalent[i] == 0)
            continue;

        pAttrInfo = g_pSO3World->m_Settings.m_HeroTalentAttr.GetByID(pData->m_wTalent[i]);
        KGLOG_PROCESS_ERROR(pAttrInfo);

        for (int j = 0; j < 4; ++j)
        {
            attr.nKey = pAttrInfo->nAttrType[i][j];
            attr.nValue1 = pAttrInfo->nAttrValue1[i][j];
            attr.nValue2 = pAttrInfo->nAttrValue2[i][j];
            attr.pNext = NULL;

            if (attr.nKey == atInvalid)
                break;

            pHero->ApplyAttribute(&attr);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::UnApplyHeroTalent(KHero* pHero)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KHeroData* pData = NULL;
    KHeroTalentAttr* pAttrInfo = NULL;
    KAttribute attr;

    assert(pHero);

    pData = m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pData);

    for (int i = egymWeightLift; i < egymTotal; ++i)
    {
        pAttrInfo = g_pSO3World->m_Settings.m_HeroTalentAttr.GetByID(pData->m_wTalent[i]);
        if (pAttrInfo == NULL)
            continue;

        for (int j = 0; j < 4; ++j)
        {
            attr.nKey = pAttrInfo->nAttrType[i][j];
            attr.nValue1 = pAttrInfo->nAttrValue1[i][j];
            attr.nValue2 = pAttrInfo->nAttrValue2[i][j];
            attr.pNext = NULL;

            if (attr.nKey == atInvalid)
                break;

            pHero->UnApplyAttribute(&attr);
        }
    }

    bResult = true;
Exit0:
    return;
}

BOOL KPlayer::ApplyHeroFashionAttr(KHero* pHero)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KHeroData* pData = NULL;
    KFashionAttr* pAttrInfo = NULL;
    KAttribute attr;

    assert(pHero);

    pData = m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pData);
    KG_PROCESS_SUCCESS(pData->m_wFashionID == 0);

    pAttrInfo = g_pSO3World->m_ItemHouse.GetFashionAttr(pData->m_wFashionID);
    KGLOG_PROCESS_ERROR(pAttrInfo);

    for (int i = 0; i < MAX_FASHION_ATTR_NUM; ++i)
    {
        attr.nKey = pAttrInfo->nAttrKey[i];
        attr.nValue1 = pAttrInfo->nValue1[i];
        attr.nValue2 = pAttrInfo->nValue2[i];
        attr.pNext = NULL;

        if (attr.nKey == atInvalid)
            break;

        pHero->ApplyAttribute(&attr);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::UnApplyHeroFashionAttr(KHero* pHero)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KHeroData* pData = NULL;
    KFashionAttr* pAttrInfo = NULL;
    KAttribute attr;

    assert(pHero);

    pData = m_HeroDataList.GetHeroData(pHero->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pData);
    KG_PROCESS_ERROR(pData->m_wFashionID > 0);

    pAttrInfo = g_pSO3World->m_ItemHouse.GetFashionAttr(pData->m_wFashionID);
    KGLOG_PROCESS_ERROR(pAttrInfo);

    for (int i = 0; i < MAX_FASHION_ATTR_NUM; ++i)
    {
        attr.nKey       = pAttrInfo->nAttrKey[i];
        attr.nValue1    = pAttrInfo->nValue1[i];
        attr.nValue2    = pAttrInfo->nValue2[i];
        attr.pNext      = NULL;

        if (attr.nKey == atInvalid)
            break;

        pHero->UnApplyAttribute(&attr);
    }

    bResult = true;
Exit0:
    return;
}

BOOL KPlayer::AddItemByMail(IItem* piItem, const char cszTitle[], const char cszSender[], const char cszText[])
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    const KItemProperty*	pItemProperty	= NULL;
    int						nMaxStackNum	= 0; 
    uint32_t                uMailLen        = 0;
    KMail*                  pMail           = NULL;

    KG_PROCESS_ERROR(!m_bIsLimited);

    KGLOG_PROCESS_ERROR(piItem);

    pMail = (KMail*)s_byTempData;
    memset(pMail, 0, sizeof(KMail));

    bRetCode = g_pSO3World->FillMail(eMailType_System, cszSender, cszTitle, cszText, strlen(cszText), &piItem, 1, pMail, uMailLen);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_LSClient.DoSendMailRequest(0, m_szName, pMail, uMailLen);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CanAddTeamLogo( WORD wTeamLogoID )
{
	BOOL bResult	= false;
	BOOL bRetCode	= false;
	std::vector<WORD>::iterator RetIter;
	KTEAM_LOGO_INFO* pTeamLogoInfo   = NULL;
	WORD wNewID = 0;

	KGLOG_PROCESS_ERROR(wTeamLogoID);
	KGLOG_PROCESS_ERROR(m_vecOwnTeamLogo.size() <= HUNDRED_NUM);

	wNewID = wTeamLogoID | 0x8000; //新Logo标识

	pTeamLogoInfo = g_pSO3World->m_ItemHouse.GetTeamLogoInfo(wTeamLogoID);
	KGLOG_PROCESS_ERROR(pTeamLogoInfo);

	RetIter = std::find(m_vecOwnTeamLogo.begin(), m_vecOwnTeamLogo.end(), wTeamLogoID);
	KG_PROCESS_ERROR(RetIter == m_vecOwnTeamLogo.end());

	RetIter = std::find(m_vecOwnTeamLogo.begin(), m_vecOwnTeamLogo.end(), wNewID);
	KG_PROCESS_ERROR(RetIter == m_vecOwnTeamLogo.end());

	bResult	= true;
Exit0:
	return bResult;
}

BOOL KPlayer::AddTeamLogo(WORD wTeamLogoID)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    std::vector<WORD>::iterator RetIter;
	KTEAM_LOGO_INFO* pTeamLogoInfo   = NULL;
    WORD wNewID = 0;

	wNewID = wTeamLogoID | 0x8000; //新Logo标识

	bRetCode = CanAddTeamLogo(wTeamLogoID);
	KG_PROCESS_ERROR(bRetCode);

    m_vecOwnTeamLogo.push_back(wNewID);
    g_PlayerServer.DoAddTeamLogoRespond(this, wTeamLogoID);
	g_PlayerServer.DoDownwardNotify(this, KMESSAGE_TEAMLOGO_ADD);
    bResult	= true;
Exit0:
    return bResult;
}

BOOL KPlayer::SetTeamLogo(WORD wFrameID, WORD wEmblemID)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    WORD wID = 0;
    KTEAM_LOGO_INFO*                pTeamLogoInfo   = NULL;
    std::vector<WORD>::iterator     RetIter;

    KGLOG_PROCESS_ERROR(wFrameID != 0 || wEmblemID != 0);
    KGLOG_PROCESS_ERROR(!wFrameID || CheckTeamLogoID(wFrameID, etiTeamLogoFrame));
    KGLOG_PROCESS_ERROR(!wEmblemID || CheckTeamLogoID(wEmblemID, etiTeamLogoEmblem));

    m_nTeamLogoFrameID     = (int)wFrameID;
    m_nTeamLogoEmblemID    = (int)wEmblemID;

    g_RelayClient.DoUpdatePlayerTeamLogo(this);

    g_LSClient.DoRemoteCall("OnUpdateTeamLogo", (int)m_dwID, m_nTeamLogoEmblemID, m_nTeamLogoFrameID);
   
    bResult	= true;
Exit0:
    g_PlayerServer.DoChangeTeamLogoRespond(
        this, bResult, (WORD)m_nTeamLogoFrameID, (WORD)m_nTeamLogoEmblemID
        );
    return bResult;
}

BOOL KPlayer::AddItemByMail(DWORD dwTabType, DWORD dwIndex, int nStackNum, int nValuePoint, 
    const char cszTitle[], const char cszSender[], const char cszText[], DWORD dwRandSeed /*=0*/
)
{
    BOOL                    bResult         = NULL;
    BOOL                    bRetCode        = false;
    IItem*                  piItem          = NULL;
    const KItemProperty*	pItemProperty	= NULL;
    int						nMaxStackNum	= 0; 
    uint32_t                uMailLen        = 0;
    KMail*                  pMail           = NULL;
    int                     nParam3         = 0;

    KG_PROCESS_ERROR(!m_bIsLimited);
    KGLOG_PROCESS_ERROR(dwTabType == ittOther || dwTabType == ittEquip);

    piItem = g_pSO3World->m_ItemHouse.CreateItem(dwTabType, dwIndex, 0, dwRandSeed, nValuePoint);
    KGLOG_PROCESS_ERROR(piItem);

    pItemProperty = piItem->GetProperty();
    if (piItem->CanStack())
    {
        nMaxStackNum = piItem->GetMaxStackNum();

        if (nStackNum > 0)
            nStackNum = MIN(nStackNum, nMaxStackNum);
        else
            nStackNum = nMaxStackNum;

        piItem->SetStackNum(nStackNum);
        nParam3 = nStackNum;
    }
    else
    {
        piItem->SetCurrentDurability(pItemProperty->pItemInfo->nMaxDurability);
        nStackNum = 1;
        nParam3 = nValuePoint;
    }

    bRetCode = AddItemByMail(piItem, cszTitle, cszSender, cszText);
    KGLOG_PROCESS_ERROR(bRetCode);

    OnEvent(peObtainItem, dwTabType, dwIndex, nParam3);

    bResult = true;
Exit0:
    if (piItem)
    {
        g_pSO3World->m_ItemHouse.DestroyItem(piItem);
        piItem = NULL;
    }

    return bResult;
}

BOOL KPlayer::SafeAddItem(
    DWORD dwTabType, DWORD dwIndex, int nStackNum, int nValuePoint, 
    const char cszTitle[], const char cszSender[], const char cszTxt[], DWORD dwRandSeed /*=0*/
)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    IItem* pItem = NULL;

    KGLOG_PROCESS_ERROR(nStackNum > 0);

    bRetCode = CanAddItem(dwTabType, dwIndex, nStackNum);
    if (bRetCode)
    {
        pItem = AddItem(dwTabType, dwIndex, nStackNum, nValuePoint, dwRandSeed);
        KGLOG_PROCESS_ERROR(pItem);
    }
    else
    {
        bRetCode = AddItemByMail(dwTabType, dwIndex, nStackNum, nValuePoint, cszTitle, cszSender, cszTxt, dwRandSeed);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

IItem* KPlayer::AddItem(DWORD dwTabType, DWORD dwIndex, int nStackNum, int nValuePoint, DWORD dwRandSeed /*=0*/)
{
    IItem*                  pResult         = NULL;
    BOOL                    bRetCode        = false;
    IItem*                  piItem          = NULL;
    const KItemProperty*	pItemProperty	= NULL;
    int						nMaxStackNum	= 0;
    int                     nParam3         = 0;

    KG_PROCESS_ERROR(!m_bIsLimited);
    KGLOG_PROCESS_ERROR(dwTabType == ittOther || dwTabType == ittEquip);

    piItem = g_pSO3World->m_ItemHouse.CreateItem(dwTabType, dwIndex, 0, dwRandSeed, nValuePoint);
    KGLOG_PROCESS_ERROR(piItem);

    pItemProperty = piItem->GetProperty();

    if (piItem->CanStack())
    {
        nMaxStackNum = piItem->GetMaxStackNum();

        if (nStackNum > 0)
            nStackNum = MIN(nStackNum, nMaxStackNum);
        else
            nStackNum = nMaxStackNum;

        piItem->SetStackNum(nStackNum);
        nParam3 = nStackNum;
    }
    else
    {
        piItem->SetCurrentDurability(pItemProperty->pItemInfo->nMaxDurability);
        nStackNum = 1;
        nParam3 = nValuePoint;
    }

    bRetCode = m_ItemList.AddPlayerItem(piItem);
    KGLOG_PROCESS_ERROR(bRetCode);

    OnEvent(peObtainItem, dwTabType, dwIndex, nParam3);

Exit1:
    pResult = piItem;
Exit0:
    if (pResult == NULL)
    {
        if (piItem)
        {
            g_pSO3World->m_ItemHouse.DestroyItem(piItem);
            piItem = NULL;
        }

        g_PlayerServer.DoDownwardNotify(this, KMESSAGE_ADD_ITEM_FAILED);
    }

    return pResult;
}

BOOL KPlayer::ChangeFashion(uint32_t dwHeroTemplateID, uint32_t dwTargetFashionID)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = m_Wardrobe.ChangeFashion(dwHeroTemplateID, dwTargetFashionID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KPlayer::CanAddItem(DWORD dwTabType, DWORD dwIndex, int nCount)
{
    BOOL            bResult         = false;
    int             nFreeRoomSize   = 0;
    KGItemInfo*     pItemInfo       = NULL;
    int             nPackageIndex   = -1;
    int             nNeedRoomSize   = 0;

    KG_PROCESS_ERROR(!m_bIsLimited);

    KG_PROCESS_ERROR(dwTabType == ittEquip || dwTabType == ittOther);

    pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(dwTabType, dwIndex);
    KGLOG_PROCESS_ERROR(pItemInfo);

    nFreeRoomSize = m_ItemList.GetPlayerFreeRoomSize(eppiPlayerItemBox);

    nNeedRoomSize = nCount;
    if (dwTabType == ittOther && pItemInfo->bCanStack)
    {
        KGLOG_PROCESS_ERROR(pItemInfo->nMaxStackNum > 0);
        nNeedRoomSize = ((nCount + pItemInfo->nMaxStackNum - 1) / pItemInfo->nMaxStackNum); // 向上取整
    }

    KG_PROCESS_ERROR(nFreeRoomSize >= nNeedRoomSize);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::HasFreePosAddItem(DWORD dwTabType, DWORD dwIndex)
{
    return m_ItemList.HasFreePosAddItem(dwTabType, dwIndex);
}

BOOL KPlayer::AbradeEquip(KHero* pHero)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    assert(pHero);

    bRetCode = m_ItemList.AbradeEquip(pHero);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::RepairEquip(int nPackageType, int nPackageIndex, int nPos)
{
    BOOL                    bResult             = false;
    BOOL                    bRetCode            = false;
    IItem*                  piItem              = NULL;
    int                     nCurrentDurability  = ERROR_ID;
    const KItemProperty*    pItemProperty       = NULL;

    KGLOG_PROCESS_ERROR(nPackageType == ePlayerPackage || nPackageType == eHeroPackage);

    piItem = m_ItemList.GetItem(nPackageType, nPackageIndex, nPos);
    KGLOG_PROCESS_ERROR(piItem);

    bRetCode = piItem->CanRepair();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piItem->Repair();
    KGLOG_PROCESS_ERROR(bRetCode);

    pItemProperty = piItem->GetProperty();
    KGLOG_PROCESS_ERROR(pItemProperty);

    nCurrentDurability = piItem->GetCurrentDurability();
    KGLOG_PROCESS_ERROR(nCurrentDurability == pItemProperty->pItemInfo->nMaxDurability);

    bRetCode = g_PlayerServer.DoUpdateItemDurability(m_nConnIndex, nPackageType, nPackageIndex, nPos, nCurrentDurability);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::BuyGoods(int nShopID, int nGoodsID, int nCount)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);
    //KG_PROCESS_ERROR(!m_bIsLimited);

    bRetCode = g_pSO3World->m_ShopMgr.OnPlayerBuyGoods(this, nShopID, nGoodsID, nCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SellGoods(int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime, WORD wCount)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);

    bRetCode = g_pSO3World->m_ShopMgr.OnPlayerSellGoods(this, nPackageType, nPackageIndex, nPos, dwTabType, dwTabIndex, nGenTime, wCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::Activate()
{
    switch (m_eGameStatus)
    {
    case gsWaitForConnect:
        // 等待登陆倒计时
        if (m_nTimer++ >= LOGIN_TIMEOUT)
        {
            KGLogPrintf(KGLOG_INFO, "Login timeout: acc = %s, role = %s\n", m_szAccount, m_szName);
            g_pSO3World->DelPlayer(this);
            return;
        }
        break;

    case gsWaitForPermit:
        // 等待登陆倒计时
        if (m_nTimer++ >= LOGIN_TIMEOUT)
        {
            KGLogPrintf(KGLOG_INFO, "Relay permission timeout,shutdown(%s, %d)\n", m_szName, m_nConnIndex);
            g_PlayerServer.Detach(m_nConnIndex);
            return;
        }
        break;
    case gsWaitForRoleData:
        // 等待角色数据倒计时
        if (m_nTimer++ >= LOGIN_TIMEOUT)
        {
            KGLogPrintf(KGLOG_INFO, "Relay query role data time out,shutdown(%s, %d)\n", m_szName, m_nConnIndex);
            g_PlayerServer.Detach(m_nConnIndex);
            return;
        }
        break;

    case gsWaitForNewMapLoading:
        return;
    case gsPlaying:
        g_PlayerServer.DoFrameSignal(m_nConnIndex);

        if (g_pSO3World->m_nGameLoop >= m_nNextSaveFrame)
        {
            g_RelayClient.SaveRoleData(this);
        }

        break;
    case gsInHall:
        CheckItemValidity();//检查IB道具有效期
        CheckVIP();         //检查VIP有效期
        ReportBuffEnd();
        m_CheerleadingMgr.Activate();

        if ((g_pSO3World->m_nGameLoop - (int)m_dwID)  % (GAME_FPS * 60) == 0)
        {
            RefreshDailyVariable();
        }

        if (g_pSO3World->m_nGameLoop >= m_nNextSaveFrame)
        {
            g_RelayClient.SaveRoleData(this);
        }

        break;

    default:
        break;
    }

    if (m_eGameStatus == gsPlaying || m_eGameStatus == gsInHall)
    {
        CheckLimited(); // 检查防沉迷
        m_UpgradeQueue.Activate();
        m_CheerleadingMgr.Activate();
        m_BusinessStreet.Activate();
        m_ActivePlayer.Activate();
        m_HeroTrainingMgr.Activate();
        m_Gym.Activate();
    }

Exit0:
    return;
}

void KPlayer::Award(KAWARD_CARD_TYPE eType, unsigned uChooseIndex/*=0*/)
{
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(m_dwMissionAward[eType]);
    KG_PROCESS_ERROR(m_dwMissionAwardCount[eType] > 0);
    KGLOG_PROCESS_ERROR(uChooseIndex < countof(m_bMissionAwardChooseIndex[eType]));
    KG_PROCESS_ERROR(m_bMissionAwardChooseIndex[eType][uChooseIndex] == false);

    switch (eType)
    {
    case KAWARD_CARD_TYPE_NORMAL:
        break;
    case KAWARD_CARD_TYPE_GOLD:
        bRetCode = m_MoneyMgr.AddMoney(emotMoney, -m_nGoldCostMoney);
        KGLOG_PROCESS_ERROR(bRetCode);
        PLAYER_LOG(this, "money,costmoney,%d,%s,%d-%d,%d,%d", emotMoney, "goldcardaward", 0, 0, 0, m_nGoldCostMoney);

        break;
    case KAWARD_CARD_TYPE_VIP:
        {
            KVIPConfig* pConfig = NULL;
            KGLOG_PROCESS_ERROR(IsVIP());

            pConfig = g_pSO3World->m_Settings.m_VIPConfig.GetByID(m_nVIPLevel);
            KGLOG_PROCESS_ERROR(pConfig);
            KGLOG_PROCESS_ERROR(pConfig->nVIPAward);
        }
        break;
    }

    m_dwMissionAwardCount[eType]--;

    g_pSO3World->m_AwardMgr.Award(this, eType, m_dwMissionAward[eType], uChooseIndex, m_dwMissionAwardCount[eType] == 0);

    if (m_dwMissionAwardCount[eType] == 0)
        m_dwMissionAward[eType] = ERROR_ID;

    m_bMissionAwardChooseIndex[eType][uChooseIndex] = true;
Exit0:
    return;
}

BOOL KPlayer::AddTeamExp(int nExp)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    BOOL bLevelUp = false;
    int  nLevelUpExp = 0;
    KReport_Team_LevelUp param;

    m_nExp += nExp;

    nLevelUpExp = g_pSO3World->m_Settings.m_PlayerLevelData.GetLevelUpExp(m_nLevel);
    KGLOG_PROCESS_ERROR(nLevelUpExp > 0);

    while (m_nExp >= nLevelUpExp && m_nLevel < cdMaxHeroLevel)
    {
        m_nExp -= nLevelUpExp;
        ++m_nLevel;

        nLevelUpExp = g_pSO3World->m_Settings.m_PlayerLevelData.GetLevelUpExp(m_nLevel);
        KGLOG_PROCESS_ERROR(nLevelUpExp > 0);

        bLevelUp = true;
    }

    if (m_nLevel == cdMaxHeroLevel)
    {
        nLevelUpExp = g_pSO3World->m_Settings.m_PlayerLevelData.GetLevelUpExp(cdMaxHeroLevel);
        KGLOG_PROCESS_ERROR(nLevelUpExp > 0);

        MAKE_IN_RANGE(m_nExp, 0, nLevelUpExp);
    }

    g_PlayerServer.DoSyncPlayerLevelExp(this, bLevelUp);

    if (bLevelUp)
    {   
        int nRoleTime = g_pSO3World->m_nCurrentTime - m_nCreateTime;
        PLAYER_LOG(this, "levelup,teamlevelup,%d,%u,%u", m_nLevel, m_nTotalGameTime, nRoleTime);

        param.level = (unsigned short)m_nLevel;
        bRetCode = m_Secretary.AddReport(KREPORT_EVENT_TEAM_LEVELUP, (BYTE*)&param, sizeof(param));
        KGLOG_PROCESS_ERROR(bRetCode);

        if (
            (m_nLevel < RECOMMEND_FRIEND_LEVEL_CHANGE && m_nLevel > RECOMMEND_FRIEND_LEVEL_MIN && m_nLevel % RECOMMEND_FRIEND__LOW_LEVEL_BREAK == 0) ||
            (m_nLevel > RECOMMEND_FRIEND_LEVEL_CHANGE && m_nLevel % RECOMMEND_FRIEND__HIGH_LEVEL_BREAK == 0)
        )
        {
            bRetCode = g_LSClient.DoApplyRecommendFriend(m_dwID, m_nHighestHeroLevel);
            KGLOG_PROCESS_ERROR(bRetCode);
        }

        ReportLevelUpAward();
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::AddHeroExp(DWORD dwHeroTemplateID, int nExp, int& nExpRealAdded)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    KHeroData*  pHeroData   = NULL;

    KG_PROCESS_SUCCESS(nExp == 0);
    KGLOG_PROCESS_ERROR(nExp > 0);

    if (m_bIsLimited)
    {
        nExpRealAdded = 0;
        goto Exit1;
    }

    pHeroData = m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KGLOG_PROCESS_ERROR(pHeroData->m_nExpMultiple > 0);

    nExp = nExp * pHeroData->m_nExpMultiple;

    bRetCode = AddHeroExpNoAdditional(dwHeroTemplateID, nExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    nExpRealAdded = nExp;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::AddHeroExpNoAdditional(DWORD dwHeroTemplateID, int nExp)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    KHeroData*  pHeroData   = NULL;
    KHeroLevelInfo* pLevelUpData = NULL;
    BOOL        bLevelChanged = false;
    int         nLastLevel    = 0;
    KReport_Hero_LevelUP param;

    KG_PROCESS_SUCCESS(nExp == 0);
    KGLOG_PROCESS_ERROR(nExp > 0);

    KG_PROCESS_SUCCESS(m_bIsLimited);

    bRetCode = AddTeamExp(nExp);
    KGLOG_CHECK_ERROR(bRetCode);

    pHeroData = m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    KG_PROCESS_SUCCESS(pHeroData->m_nLevel >= cdMaxHeroLevel);

    nLastLevel = pHeroData->m_nLevel;
    pLevelUpData = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroLevelInfo(pHeroData->m_dwTemplateID, pHeroData->m_nLevel);
    KGLOG_PROCESS_ERROR(pLevelUpData);

    pHeroData->m_nExp += nExp;

    while (pHeroData->m_nExp >= pLevelUpData->nExp && pHeroData->m_nLevel < cdMaxHeroLevel)
    {
        pHeroData->m_nExp -= pLevelUpData->nExp;
        ++pHeroData->m_nLevel;

        OnEvent(peHeroLevelup, pHeroData->m_dwTemplateID, pHeroData->m_nLevel);

        if (pHeroData->m_nLevel % 5 == 0)
        {
            int nRoleTime = g_pSO3World->m_nCurrentTime - m_nCreateTime;
            PLAYER_LOG(this, "levelup,herolevelup,%u,%u,%u,%u", pHeroData->m_dwTemplateID, pHeroData->m_nLevel, m_nTotalGameTime, nRoleTime);
        }

        ++pHeroData->m_wFreeTalent; // 升一级增加一点可用天赋

        bLevelChanged = true;

        pLevelUpData = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroLevelInfo(pHeroData->m_dwTemplateID, pHeroData->m_nLevel);
        KGLOG_PROCESS_ERROR(pLevelUpData);
    }

    if (pHeroData->m_nLevel == cdMaxHeroLevel)
    {
        pLevelUpData = g_pSO3World->m_Settings.m_HeroTemplate.GetHeroLevelInfo(pHeroData->m_dwTemplateID, pHeroData->m_nLevel);
        KGLOG_PROCESS_ERROR(pLevelUpData);

        MAKE_IN_RANGE(pHeroData->m_nExp, 0, pLevelUpData->nExp);
    }

    g_PlayerServer.DoSyncHeroAddedExp(m_nConnIndex, pHeroData->m_dwTemplateID, nExp); // 通知获得经验

    g_PlayerServer.DoSyncHeroExp(m_nConnIndex, pHeroData->m_dwTemplateID, pHeroData->m_nExp);

    UpdateLevel(pHeroData->m_nLevel);

    if (bLevelChanged)
    {
        g_PlayerServer.DoHeroLevelUp(m_nConnIndex, pHeroData->m_dwTemplateID, pHeroData->m_nLevel);
        g_PlayerServer.DoSyncFreeTalent(m_nConnIndex, pHeroData->m_dwTemplateID, pHeroData->m_wFreeTalent);

        g_RelayClient.DoUpdateHeroLevel(this, dwHeroTemplateID);

        param.heroTemplateID = dwHeroTemplateID;
        param.level          = (unsigned short)pHeroData->m_nLevel;

        bRetCode = m_Secretary.AddReport(KREPORT_EVENT_HERO_LEVELUP, (BYTE*)&param, sizeof(param));
        KGLOG_PROCESS_ERROR(bRetCode);

        g_LSClient.DoLadderDataChangeNotify(this, dwHeroTemplateID);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::HasHero(DWORD dwHeroTemplateID) const
{
    return m_HeroDataList.HasHero(dwHeroTemplateID);
}

BOOL KPlayer::CostGroupTalkItem()
{
    BOOL bResult            = false;
    BOOL bRetCode           = false;
    int  nTabType           = 0;
    int  nTabIndex          = 0;

    nTabType = g_pSO3World->m_Settings.m_ConstList.nGroupMessageItemTabType;
    KGLOG_PROCESS_ERROR(nTabType >= 0);

    nTabIndex = g_pSO3World->m_Settings.m_ConstList.nGroupMessageItemTabIndex;
    KGLOG_PROCESS_ERROR(nTabIndex >= 0);

    bRetCode = m_ItemList.CostPlayerItem(nTabType, nTabIndex, 1);
    KGLOG_PROCESS_ERROR(bRetCode);

    PLAYER_LOG(this, "item,useitem,%d-%d", nTabType, nTabIndex);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CostGlobalTalkItem()
{
    BOOL bResult            = false;
    BOOL bRetCode           = false;
    int  nTabType           = 0;
    int  nTabIndex          = 0;

    nTabType = g_pSO3World->m_Settings.m_ConstList.nGlobalMessageItemTabType;
    KGLOG_PROCESS_ERROR(nTabType >= 0);

    nTabIndex = g_pSO3World->m_Settings.m_ConstList.nGlobalMessageItemTabIndex;
    KGLOG_PROCESS_ERROR(nTabIndex >= 0);

    bRetCode = m_ItemList.CostPlayerItem(nTabType, nTabIndex, 1);
    KGLOG_PROCESS_ERROR(bRetCode);

    PLAYER_LOG(this, "item,useitem,%d-%d", nTabType, nTabIndex);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CheckTalkDailyCount(unsigned char cTalkType)
{
    BOOL    bResult                 = false;
    int     nTalkWorldDailyCount    = g_pSO3World->m_Settings.m_ConstList.nWorldChannelDailyCount;

    switch (cTalkType)
    {
    case trWorld:
        KG_PROCESS_ERROR(m_nTalkWorldDailyCount < nTalkWorldDailyCount);
        ++m_nTalkWorldDailyCount;
        break;

    default:
        break;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::RefreshDailyVariable()
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    time_t  nOldDay     = 0;
    time_t  nNewDay     = 0;
    int     nDays       = 0;

    nOldDay     = (m_nLastDailyRefreshTime - timezone - DAILY_VARIABLE_REFRESH_TIME) / (24 * 3600);
    nNewDay     = (g_pSO3World->m_nCurrentTime - timezone - DAILY_VARIABLE_REFRESH_TIME) / (24 * 3600);
    nDays       = (int)(nNewDay - nOldDay);

    if (nDays > 0)
    {
        m_nTalkWorldDailyCount = 0;
        ResetFatiguePoint(nDays);
        m_Achievement.Refresh(m_nLastDailyRefreshTime);

        m_MoneyMgr.RefreshGameMoneyLimit();
        m_UpgradeQueue.ResetFreeClearCDTimes();
        m_BusinessStreet.RefreshGetMoneyTimes();
        m_mapDailyResetUserData.clear();

        m_nClubApplyNum = 0;

        m_nRandomQuestIndex             = 0;
        m_nSkipRandomQuestTime          = 0;

        m_nLastDailyRefreshTime = g_pSO3World->m_nCurrentTime;
    }

    nOldDay     = (m_nLastDailyRefreshTimeZero - timezone) / (24 * 3600);
    nNewDay     = (g_pSO3World->m_nCurrentTime - timezone) / (24 * 3600);
    nDays       = (int)(nNewDay - nOldDay);

    if (nDays > 0)
    {
        m_PlayerValue.Refresh(m_nLastDailyRefreshTimeZero);
        m_ActivePlayer.Refresh();
        m_nOnlineGetAwardTimes  = 0;
        m_nOnlineAwardTime      = GetNextOnlineAwardTime();
        g_PlayerServer.DoSyncOnlineAwardInfo(m_nConnIndex, m_nOnlineGetAwardTimes, m_nOnlineAwardTime);

        m_nLastDailyRefreshTimeZero = g_pSO3World->m_nCurrentTime;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SelectHeroSkill(DWORD dwHeroTemplateID, int nSkillGroup, int nSlotIndex, DWORD dwSkillID)
{
    return m_HeroDataList.SelectHeroSkill(dwHeroTemplateID, nSkillGroup, nSlotIndex, dwSkillID);
}

void KPlayer::OnEvent(int nEvent, int nParam1/*=0*/, int nParam2/*=0*/, int nParam3/*=0*/, int nParam4/*=0*/, int nParam5/*=0*/, int nParam6/*=0*/)
{
    int Params[cdMaxEventParamCount] = {nParam1, nParam2, nParam3, nParam4, nParam5, nParam6};

    m_QuestList.OnEvent(nEvent, Params);
    m_Achievement.OnEvent(nEvent, Params);
    m_ActivePlayer.OnEvent(nEvent);
    m_HireHero.OnEvent(nEvent, Params); // 解锁球员
}

BOOL KPlayer::SelectMainHero(DWORD dwHeroTemplateID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    DWORD dwOldMainHero = 0;

    bRetCode = HasHero(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    dwOldMainHero = m_dwMainHeroTemplateID;
    SetMainHero(dwHeroTemplateID);
    //m_dwMainHeroTemplateID = dwHeroTemplateID;

    if (m_dwAssistHeroTemplateID == m_dwMainHeroTemplateID)
        m_dwAssistHeroTemplateID = dwOldMainHero;

    g_PlayerServer.DoSyncPlayerStateInfo(this);

    g_RelayClient.DoSyncMainHero(m_dwID, m_dwMainHeroTemplateID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SelectAssistHero(DWORD dwHeroTemplateID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    DWORD dwOldAssistHero = 0;

    bRetCode = HasHero(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(bRetCode);

    dwOldAssistHero = m_dwAssistHeroTemplateID;
    m_dwAssistHeroTemplateID = dwHeroTemplateID;

    if (m_dwMainHeroTemplateID == m_dwAssistHeroTemplateID)
        SetMainHero(dwOldAssistHero);
    //m_dwMainHeroTemplateID = dwOldAssistHero;

    g_PlayerServer.DoSyncPlayerStateInfo(this);

    bResult = true;
Exit0:
    return bResult;
}

int KPlayer::GetMainHeroLevel()
{
    int         nMainHeroLevel = 0;
    KHeroData*  pHeroData = NULL;

    if (m_dwMainHeroTemplateID == ERROR_ID)
        KGLogPrintf(KGLOG_ERR, "Player(ID:%u, Name:%s)'s MainHeroTemplateID is 0 in %s.\n", m_dwID, m_szName, __FUNCTION__);

    pHeroData = m_HeroDataList.GetHeroData(m_dwMainHeroTemplateID);
    if (!pHeroData)
        KGLogPrintf(KGLOG_ERR, "Player(ID:%u, Name:%s)'s Mainhero(TempalteID:%u) not Exist in %s.\n", m_dwID, m_szName, m_dwMainHeroTemplateID, __FUNCTION__);

    KGLOG_PROCESS_ERROR(pHeroData);

    nMainHeroLevel = pHeroData->m_nLevel;
Exit0:
    return nMainHeroLevel;
}

int KPlayer::GetAssistHeroLevel()
{
    int         nAssistHeroLevel = 0;
    KHeroData*  pHeroData = NULL;

    KG_PROCESS_ERROR(m_dwAssistHeroTemplateID);

    pHeroData = m_HeroDataList.GetHeroData(m_dwAssistHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    nAssistHeroLevel = pHeroData->m_nLevel;
Exit0:
    return nAssistHeroLevel;
}

int KPlayer::GetMainHeroEquipTotalValuePoint()
{
    KHeroPackage* pPackage = NULL;

    pPackage = m_ItemList.GetHeroPackage(m_dwMainHeroTemplateID);
    KGLOG_PROCESS_ERROR(pPackage);

    return pPackage->GetAllEquipScore();
Exit0:
    return 0;
}

void KPlayer::SetMainHero(uint32_t dwMainHeroTemplateID)
{
    BOOL bRetCode = false;

    if (dwMainHeroTemplateID == ERROR_ID)
        KGLogPrintf(KGLOG_ERR, "New Main Hero TemplateID is 0.\n");

    bRetCode = HasHero(dwMainHeroTemplateID);
    if (!bRetCode)
        KGLogPrintf(KGLOG_ERR, "Player(ID:%u, Name:%s) has not hero(TempalteID:%u)\n", m_dwID, m_szName, dwMainHeroTemplateID);

    m_dwMainHeroTemplateID = dwMainHeroTemplateID;

    OnEvent(peSelectMainHero);
}

void KPlayer::CheckLimited()
{
    /*防沉迷标示；
    0表示未填写实名信息，
    1表示填写了实名信息且已经成年，
    2表示填写了实名但未成年。*/

    KG_PROCESS_ERROR(!m_bIsLimited);
    KG_PROCESS_ERROR(m_dwLimitPlayTimeFlag != 1); 

    if (m_dwLimitOnlineSecond + (g_pSO3World->m_nCurrentTime - m_nCurrentLoginTime) >= 3 * 3600)
    {
        m_bIsLimited = true;
        g_PlayerServer.DoSyncPlayerLimitedFlag(m_nConnIndex, m_bIsLimited);
        LogInfo("%s is limited!", m_szName);
    }

Exit0:
    return;
}

void KPlayer::CheckItemValidity()
{
    BOOL bRetCode   = false;
    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);

    KG_PROCESS_ERROR((g_pSO3World->m_nGameLoop - m_dwID) % (SECONDS_PER_MINUTE * GAME_FPS) == 0);

    bRetCode = m_ItemList.CheckItemValidity();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Wardrobe.CheckValidity();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KPlayer::CheckVIP()
{
    BOOL bRetCode   = false;
    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);

    KG_PROCESS_ERROR(m_nVIPEndTime > 0);
    KG_PROCESS_ERROR((g_pSO3World->m_nGameLoop - m_dwID) % (SECONDS_PER_MINUTE * GAME_FPS) == 0);

    if (m_nVIPEndTime < g_pSO3World->m_nCurrentTime)
    {
        UnApplyVIP(false);
        m_nVIPEndTime = 0;
    }

Exit0:
    return;
}

BOOL KPlayer::CheckTeamLogoID( WORD wID, ETEAMLOGO_INFO_TYPE eType )
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    BOOL wNewID	= 0;
    KTEAM_LOGO_INFO*                pTeamLogoInfo   = NULL;
    std::vector<WORD>::iterator     IDIter;
    std::vector<WORD>::iterator     NewIDIter;

    wNewID = wID | 0x8000;
    if (!wID)
    {
        KGLOG_PROCESS_ERROR(eType == etiTeamLogoInvaild);
    }
    else
    {
        IDIter = std::find(m_vecOwnTeamLogo.begin(), m_vecOwnTeamLogo.end(), wID);
        NewIDIter = std::find(m_vecOwnTeamLogo.begin(), m_vecOwnTeamLogo.end(), wNewID);
        KGLOG_PROCESS_ERROR(IDIter != m_vecOwnTeamLogo.end() || NewIDIter != m_vecOwnTeamLogo.end());

        pTeamLogoInfo = g_pSO3World->m_ItemHouse.GetTeamLogoInfo(wID);
        KGLOG_PROCESS_ERROR(pTeamLogoInfo && pTeamLogoInfo->eType == eType);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::AddHeroLadderExp(uint32_t dwHeroTemplateID, int nExpToAdd)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    KHeroData* pHeroData = NULL;
    KLADDER_LEVELDATA_ITEM* pLadderLevelDataItem = NULL;
    BOOL bLadderLevelChanged = false;

    if (nExpToAdd > 0)
        KG_PROCESS_SUCCESS(m_bIsLimited);

    pHeroData = m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);
    KGLOG_PROCESS_ERROR(pHeroData->m_nLadderLevel >= 1 && pHeroData->m_nLadderLevel <= MAX_LADDER_LEVEL);

    pHeroData->m_nLadderExp += nExpToAdd;
    while (pHeroData->m_nLadderLevel >= 1 && pHeroData->m_nLadderLevel <= MAX_LADDER_LEVEL)
    {
        pLadderLevelDataItem = g_pSO3World->m_Settings.m_LadderLevelData.GetLadderLevelData(pHeroData->m_nLadderLevel);
        KGLOG_PROCESS_ERROR(pLadderLevelDataItem);

        if (pHeroData->m_nLadderExp < 0)
        {
            if (pHeroData->m_nLadderLevel == 1)
            {
                pHeroData->m_nLadderExp = 0;
                break;
            }

            --pHeroData->m_nLadderLevel;
            bLadderLevelChanged = true;
            pHeroData->m_nLadderExp += pLadderLevelDataItem->m_nExp;
            continue;
        }

        if (pHeroData->m_nLadderExp >= pLadderLevelDataItem->m_nExp)
        {
            if (pHeroData->m_nLadderLevel == MAX_LADDER_LEVEL)
            {
                pHeroData->m_nLadderExp = pLadderLevelDataItem->m_nExp;
                break;
            }

            ++pHeroData->m_nLadderLevel;
            bLadderLevelChanged = true;
            pHeroData->m_nLadderExp -= pLadderLevelDataItem->m_nExp;
            continue;
        }

        break;
    }

    g_PlayerServer.DoSyncHeroData(m_nConnIndex, pHeroData);

    if (bLadderLevelChanged)
    {
        m_nHighestLadderLevel = m_HeroDataList.GetHighestLadderLevel();
        g_LSClient.DoRemoteCall("OnUpdateLadderLevel", (int)m_dwID, m_nHighestLadderLevel);
        g_RelayClient.DoUpdateHeroLevel(this, dwHeroTemplateID);

        OnEvent(peLadderLevelup, pHeroData->m_nLadderLevel);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::AddBuff(DWORD dwBuffID, int nLeftFrame /*= 0*/)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    int nActiveCount = 0;
    KBuff* pBuff = NULL;
    KBuffInfo *pBuffInfo = NULL;
    KBuffTable::iterator itBuff;

    pBuff = g_pSO3World->m_BuffManager.GetBuff(dwBuffID);
    KGLOG_PROCESS_ERROR(pBuff);

    if (nLeftFrame == 0)
        nLeftFrame = pBuff->m_nFrame;    
    nActiveCount = pBuff->m_nActiveCount;

    itBuff = m_BuffTable.find(dwBuffID);
    if (itBuff != m_BuffTable.end())
    {
        pBuffInfo = &itBuff->second;
        if (pBuffInfo->m_nEndFrame > g_pSO3World->m_nGameLoop)
        {
            nLeftFrame += pBuffInfo->m_nEndFrame - g_pSO3World->m_nGameLoop;
            nActiveCount += pBuffInfo->m_nActiveCount;
        }
    }

    pBuffInfo = &m_BuffTable[dwBuffID];
    KGLOG_PROCESS_ERROR(pBuffInfo);

    pBuffInfo->pBuff = pBuff;
    pBuffInfo->m_nEndFrame = nLeftFrame + g_pSO3World->m_nGameLoop;
    pBuffInfo->m_nActiveCount = nActiveCount;

    bRetCode = g_PlayerServer.DoSyncPlayerBuff(m_nConnIndex, dwBuffID, nLeftFrame, nActiveCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::DelBuff(DWORD dwBuffID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KBuffTable::iterator itBuff = m_BuffTable.find(dwBuffID);
    if (itBuff != m_BuffTable.end())
    {
        m_BuffTable.erase(itBuff);
    }

    g_PlayerServer.DoDelPlayerBuff(m_nConnIndex, dwBuffID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CreateTeam()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);

    bRetCode = g_RelayClient.DoCreateTeamRequest(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::LeaveTeam()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);

    bRetCode = g_RelayClient.DoTeamLeaveRequest(m_dwID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::TeamInvitePlayer(const char cszTargetName[])
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(cszTargetName);

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);

    bRetCode = g_RelayClient.DoTeamInvitePlayerRequest(m_dwID, cszTargetName);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::TeamAcceptOrRefuseInvite(int nAcceptCode, const char (&szInviterName)[_NAME_LEN])
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(szInviterName);

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);

    bRetCode = g_RelayClient.DoTeamAcceptOrRefuseInvite(this, nAcceptCode, szInviterName);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::TeamKickoutPlayer(uint32_t dwTargetID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);

    bRetCode = g_RelayClient.DoTeamKickoutPlayerRequest(m_dwID, dwTargetID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::AutoMatch()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);

    bRetCode = g_RelayClient.DoAutoMatchRequest(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CancelAutoMatch()
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(m_bIsMatching);

    bRetCode = g_RelayClient.DoCancelAutoMatchRequest(this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::TryJoinRoom(DWORD dwRoomID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);

    bRetCode = g_RelayClient.DoTryJoinRoom(this, dwRoomID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::AcceptOrRefuseJoinRoom(int nAcceptCode, DWORD dwRoomID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!m_bInPVPRoom);
    KGLOG_PROCESS_ERROR(!m_bIsMatching);

    bRetCode = g_RelayClient.DoAcceptOrRefuseJoinRoom(this, nAcceptCode, dwRoomID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::UpdateLevel(int nNewLevel)
{
    if (m_nHighestHeroLevel < nNewLevel)
    {
        m_nHighestHeroLevel = nNewLevel;
        SetExtPoint(HIGHEST_LEVEL_INDEX_IN_EXTPOINT, m_nHighestHeroLevel);
        g_RelayClient.DoUpdateRoleLevel(m_dwID, nNewLevel);
    }
}

void KPlayer::DoHeroExpWeeklyDecrease()
{
    int nTimeNow = g_pSO3World->m_nCurrentTime;
    int nDeltaTime = nTimeNow - m_nLastHeroExpDecreasedTime;
    int nWeeks = 0;

    nWeeks = nDeltaTime / SECONDS_PER_WEEK;
    KG_PROCESS_ERROR(nWeeks > 0);

    for (int i = 0; i < nWeeks; ++i)
    {
        m_HeroDataList.DecreaseAllCompetitorHeroExp(g_pSO3World->m_Settings.m_ConstList.nHeroExpWeeklyDecreasePercent);
    }

    m_nLastHeroExpDecreasedTime = nTimeNow;

Exit0:
    return;
}

BOOL KPlayer::UpdateVIPAwardCount()
{
    BOOL bResult = false;
    KVIPConfig* pVipConfig = NULL;
    int nUsedAward = 0;

    KG_PROCESS_ERROR(IsVIP());
    KG_PROCESS_ERROR(m_bLuckDrawing);

    pVipConfig = g_pSO3World->m_Settings.m_VIPConfig.GetByID(m_nVIPLevel);
    KGLOG_PROCESS_ERROR(pVipConfig);

    if (pVipConfig->nVIPAward && m_dwMissionAward[KAWARD_CARD_TYPE_VIP])
    {
        for (size_t i = cdCardCount; i < cdVipCardCount; ++i)
        {
            if (m_bMissionAwardChooseIndex[KAWARD_CARD_TYPE_VIP][i])
                ++nUsedAward;
        }

        m_dwMissionAwardCount[KAWARD_CARD_TYPE_VIP] = pVipConfig->nVIPAward - nUsedAward;
    }

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::OnMissionSuccess(KAwardData* pAwardData, BOOL(&bHasAward)[KAWARD_CARD_TYPE_TOTAL])
{
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pAwardData);

    memset(bHasAward, 0, sizeof(bHasAward));
    memset(m_dwMissionAward, 0, sizeof(m_dwMissionAward));
    memset(m_dwMissionAwardCount, 0, sizeof(m_dwMissionAwardCount));
    memset(m_bMissionAwardChooseIndex, 0, sizeof(m_bMissionAwardChooseIndex));
    m_bLuckDrawing = true;

    for (size_t i = cdCardCount; i < cdVipCardCount; ++i)
    {
        m_bMissionAwardChooseIndex[KAWARD_CARD_TYPE_NORMAL][i] = true;
        m_bMissionAwardChooseIndex[KAWARD_CARD_TYPE_GOLD][i] = true;
    }

    KG_PROCESS_ERROR(!m_bIsLimited);

    if (pAwardData->dwAwardListID[KAWARD_CARD_TYPE_NORMAL])
    {
        m_dwMissionAward[KAWARD_CARD_TYPE_NORMAL]   = pAwardData->dwAwardListID[KAWARD_CARD_TYPE_NORMAL];
        m_dwMissionAwardCount[KAWARD_CARD_TYPE_NORMAL] = 1;
        bHasAward[KAWARD_CARD_TYPE_NORMAL] = true;
    }

    bRetCode = m_MoneyMgr.CanAddMoney(emotMoney, -pAwardData->nGoldCostMoney);
    if (pAwardData->dwAwardListID[KAWARD_CARD_TYPE_GOLD] && bRetCode)
    {
        m_dwMissionAward[KAWARD_CARD_TYPE_GOLD] = pAwardData->dwAwardListID[KAWARD_CARD_TYPE_GOLD];
        m_dwMissionAwardCount[KAWARD_CARD_TYPE_GOLD] = 1;
        m_nGoldCostMoney = pAwardData->nGoldCostMoney;
        bHasAward[KAWARD_CARD_TYPE_GOLD] = true;
    }

    m_dwMissionAward[KAWARD_CARD_TYPE_VIP]  = pAwardData->dwAwardListID[KAWARD_CARD_TYPE_VIP];
    if (IsVIP())
    {
        bRetCode = UpdateVIPAwardCount();
        if (bRetCode)
        {
            bHasAward[KAWARD_CARD_TYPE_VIP]         = true;
        }
    }

Exit0:
    return;
}

BOOL KPlayer::IsVIP()
{
    return (m_nVIPEndTime > g_pSO3World->m_nCurrentTime);
}

BOOL KPlayer::EnterMission(int eType, int nStep, int nLevel)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    DWORD dwMapID = ERROR_ID;

    bRetCode = g_pSO3World->m_MissionMgr.CanEnterMission(this, eType, nStep, nLevel);
    KGLOG_PROCESS_ERROR(bRetCode);

    dwMapID = g_pSO3World->m_MissionMgr.GetMissionMap(eType, nStep, nLevel);
    KGLOG_PROCESS_ERROR(dwMapID);

    bRetCode = SwitchMap(dwMapID, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SetExtPoint(int nIndex, int nChangeValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    int  nOldValue  = 0;   

    KGLOG_PROCESS_ERROR(nIndex >= 0 && nIndex < MAX_EXT_POINT_COUNT);
    KGLOG_PROCESS_ERROR(m_ExtPointInfo.nExtPoint[nIndex] != nChangeValue);

    if (m_bExtPointLock)
    {
        KGLogPrintf(
            KGLOG_INFO, "ChangeExtPoint locked. PlayerID = %d, ExtIndex = %d, ChangeValue = %d",
            m_dwID, nIndex, nChangeValue
            );
        goto Exit0;
    }

    m_bExtPointLock         = true;
    m_nLastExtPointIndex    = nIndex;
    m_nLastExtPointValue    = m_ExtPointInfo.nExtPoint[nIndex];

    nOldValue = m_ExtPointInfo.nExtPoint[nIndex];

    m_ExtPointInfo.nExtPoint[nIndex] = nChangeValue;

    bRetCode = g_RelayClient.DoChangeExtPointRequest(m_dwID, (unsigned)nIndex, nChangeValue);
    KGLOG_PROCESS_ERROR(bRetCode);

    KGLogPrintf(
        KGLOG_INFO, "Apply Change ExtPoint. PlayerID = %d, ExtIndex = %d, ChangeValue = %d",
        m_dwID, nIndex, nChangeValue
        );

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::GetExtPoint(int nIndex, int nBitIndex, int nBitLength, int& nGetValue)
{
    BOOL		bResult		= false;
    int			nOldValue	= 0;
    const int   cMaxBit		= CHAR_BIT * sizeof(int);

    nGetValue = 0;

    KGLOG_PROCESS_ERROR(nIndex >= 0 && nIndex < MAX_EXT_POINT_COUNT);
    KGLOG_PROCESS_ERROR(nBitIndex >= 0 && nBitIndex < cMaxBit);
    KGLOG_PROCESS_ERROR(nBitLength > 0 && nBitIndex + nBitLength <= cMaxBit);

    nOldValue = m_ExtPointInfo.nExtPoint[nIndex] >> nBitIndex;

    for (int i = 0; i < nBitLength; i++)
        nGetValue |= nOldValue & (1 << i);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::CanStartContinuousChallenge()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nItemAmount = 0;

    nItemAmount = m_ItemList.GetPlayerItemCount(
        g_pSO3World->m_Settings.m_ConstList.nTiaoZhanShuTabType, g_pSO3World->m_Settings.m_ConstList.nTiaoZhanShuTabIndex
    );
    KG_PROCESS_ERROR(nItemAmount > 0);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::GetTeamLogoSyncInfo(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize)
{
    BOOL                        bResult	        = false;
    BOOL                        bRetCode	    = false;
    BYTE*                       pbyOffset       = pbyBuffer;
    size_t                      uLeftSize       = uBufferSize;
    WORD*                       pwTeamLogoID    = NULL;
    KS2C_Sync_TeamLogo_Info*    pPak = (KS2C_Sync_TeamLogo_Info*)pbyBuffer;

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(KS2C_Sync_TeamLogo_Info));
    pbyOffset += sizeof(KS2C_Sync_TeamLogo_Info);
    uLeftSize -= sizeof(KS2C_Sync_TeamLogo_Info);

    pPak->protocolID                = s2c_sync_teamlogo_info;
    pPak->wCurrentTeamLogoFrameID   = (WORD)m_nTeamLogoFrameID;
    pPak->wCurrentTeamLogoEmblemID  = (WORD)m_nTeamLogoEmblemID;
    pPak->wOwnLogoIDLength          = (WORD)m_vecOwnTeamLogo.size();

    KGLOG_PROCESS_ERROR(uLeftSize >= sizeof(WORD) * pPak->wOwnLogoIDLength);

    pwTeamLogoID = &pPak->wOwnLogoID[0];
    for (vector<WORD>::iterator it = m_vecOwnTeamLogo.begin(); it != m_vecOwnTeamLogo.end(); ++it)
    {
        *pwTeamLogoID = *it;
        ++pwTeamLogoID;
    }

    pbyOffset += sizeof(WORD) * pPak->wOwnLogoIDLength;
    uLeftSize -= sizeof(WORD) * pPak->wOwnLogoIDLength;

    uUsedSize = uBufferSize - uLeftSize;

    bResult	= true;
Exit0:
    return bResult;
}

int KPlayer::GetNextOnlineAwardTime()
{
    if (m_nOnlineGetAwardTimes >= cdOnlineAwardTimes)
        return 0;

    int nCDTime[cdOnlineAwardTimes] = {5*60, 10*60, 15*60, 20*60, 30*60};
    KGLOG_PROCESS_ERROR(m_nOnlineGetAwardTimes >= 0 && m_nOnlineGetAwardTimes < cdOnlineAwardTimes);
    return g_pSO3World->m_nCurrentTime + nCDTime[m_nOnlineGetAwardTimes];
Exit0:
    return 0;
}

BOOL KPlayer::CostContinuousChallengeItem()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = m_ItemList.CostPlayerItem(g_pSO3World->m_Settings.m_ConstList.nTiaoZhanShuTabType, g_pSO3World->m_Settings.m_ConstList.nTiaoZhanShuTabIndex, 1);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

// 处理从nStartStep到nFinishStep连续挑战赛的奖励，注意不包括nFinishStep
void KPlayer::ProcessChallengeAward(
    KCHALLENGE_AWARD_TYPE eAwardType, KPlayer* pOtherPlayer, BOOL bUseAssistHero, int nStartStep, int nFinishStep
)
{
    BOOL            bRetCode    = false;
    KMission*       pMission    = NULL;
    int             nAwardMoney = 0;
    int             nAwardExp   = 0;
    IItem*          pItem       = NULL;
    KAWARD_ITEM*    pAwardItem  = NULL;
    int             nSmallStepCount = 0; 
    KSystemMailTxt* pSysMailTxt = NULL;
    std::vector<KAWARD_ITEM*> vecAwardItem;
    int             nValuePoint = 0;
    char*           pszOtherPlayerName = NULL;
    KHero*          pFightHero      = NULL;
    KScene*         pScene          = NULL;
    int             nRealAwardExp   = 0;

    KCHALLENGEMISSION_AWARD award = {0};

    KGLOG_PROCESS_ERROR(nStartStep > 0);
    KGLOG_PROCESS_ERROR(nFinishStep > 0);
    KGLOG_PROCESS_ERROR(nStartStep <= nFinishStep);

    pFightHero = GetFightingHero();
    KGLOG_PROCESS_ERROR(pFightHero);

    pScene = pFightHero->m_pScene;
    assert(pScene);

    for (int i = nStartStep; i < nFinishStep; ++i)
    {
        pMission = g_pSO3World->m_MissionMgr.GetMission(GetCurrentChallengeType(), i, 1);
        KGLOG_PROCESS_ERROR(pMission);
		
        nAwardExp += pMission->nBaseExp;
        nAwardMoney += pMission->nBaseMoney;

        if (i % cdChallengeSmallStepCount == 0)
        {
            pAwardItem = g_pSO3World->m_AwardMgr.AwardRandomOne(pMission->stAwardData.dwAwardListID[KAWARD_CARD_TYPE_NORMAL]);
            vecAwardItem.push_back(pAwardItem); // pAwardItem 可能为空

            memset(&award, 0, sizeof(award));

            award.byStep = (BYTE)(i - 1) / cdChallengeSmallStepCount + 1;
            if (pAwardItem)
            {
                award.byTabType = (BYTE)pAwardItem->dwTabType;
                award.wTabIndex = (WORD)pAwardItem->dwIndex;
                award.wStackNum = (WORD)pAwardItem->nStackNum;
                award.byMoneyType = (BYTE)pAwardItem->eMoneyType;
                award.nMoney    = pAwardItem->nMoney;
                
                bRetCode = g_pSO3World->m_AwardMgr.GetAwardItemValuePoint(pAwardItem, nValuePoint);
                KGLOG_PROCESS_ERROR(bRetCode);
                award.wValuePoint = (WORD)nValuePoint;
                m_vecChallengeAwardItem.push_back(award);
            }
        }
    }

    bRetCode = m_MoneyMgr.AddMoney(emotMoney, nAwardMoney);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = AddHeroExpNoAdditional(m_dwMainHeroTemplateID, nAwardExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bUseAssistHero)
    {
        bRetCode = AddHeroExpNoAdditional(m_dwAssistHeroTemplateID, nAwardExp);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    pSysMailTxt = g_pSO3World->m_Settings.m_SystemMailMgr.GetSystemMailTxt(KMAIL_CHALLENGE);
    KGLOG_PROCESS_ERROR(pSysMailTxt); 

    for (size_t i = 0; i < vecAwardItem.size(); ++i)
    {
        pAwardItem = vecAwardItem[i];

        if (pAwardItem == NULL)
            continue;

        if (pAwardItem->dwTabType > 0)
        {
            bRetCode = g_pSO3World->m_AwardMgr.GetAwardItemValuePoint(pAwardItem, nValuePoint);
            KGLOG_PROCESS_ERROR(bRetCode);

            bRetCode = SafeAddItem(
                pAwardItem->dwTabType, pAwardItem->dwIndex, pAwardItem->nStackNum, nValuePoint,
                pSysMailTxt->m_szMailTitl, pSysMailTxt->m_szSendName, pSysMailTxt->m_szMailTxt
            );
            KGLOG_CHECK_ERROR(bRetCode);
        }
        else if (pAwardItem->nMoney > 0)
        {
            bRetCode = m_MoneyMgr.AddMoney(pAwardItem->eMoneyType, pAwardItem->nMoney);
            KGLOG_CHECK_ERROR(bRetCode);
        }
    }

    g_PlayerServer.DoSyncChallengeAward(
        m_nConnIndex, eAwardType, pOtherPlayer, nStartStep, nFinishStep, nAwardMoney,
        nAwardExp, bUseAssistHero ? nAwardExp : 0, m_vecChallengeAwardItem
    );

Exit0:
    return;
}

KHeroData* KPlayer::GetMainHeroData()
{
    return m_HeroDataList.GetHeroData(m_dwMainHeroTemplateID);
}

KHeroData* KPlayer::GetAssistHeroData()
{
    return m_HeroDataList.GetHeroData(m_dwAssistHeroTemplateID);
}

BOOL KPlayer::LearnRecipe(DWORD dwRecipeID)
{
    return m_MakingMachine.LearnRecipe(dwRecipeID);
}

BOOL KPlayer::ProduceItem(DWORD dwRecipeID)
{
    return m_MakingMachine.ProduceItem(dwRecipeID);
}

BOOL KPlayer::DoFatiguePointCost(int nCostFatiguePoint)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(nCostFatiguePoint > 0);

    KG_PROCESS_ERROR(m_nCurFatiguePoint > 0);

    m_nCurFatiguePoint -= nCostFatiguePoint;
    if (m_nCurFatiguePoint < 0)
        m_nCurFatiguePoint = 0;

    m_ActivePlayer.AddCostFatiguePoint(nCostFatiguePoint);

    g_PlayerServer.DoSyncPlayerStateInfo(this);

    if (m_nCurFatiguePoint == 0)
    {
        bRetCode = m_Secretary.AddReport(KREPORT_EVENT_ZERO_FATIGUE_POINT, NULL, 0);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::ResetFatiguePoint(int nResetCount)
{
    BOOL bResult = false;
    KVIPConfig* pVipInfo = NULL;
    int nVIPLevel = 0;
    int nDeltFatiguePoint = 0;

    KGLOG_PROCESS_ERROR(nResetCount > 0);

    if (IsVIP())
        nVIPLevel = m_nVIPLevel;

    pVipInfo = g_pSO3World->m_Settings.m_VIPConfig.GetByID(nVIPLevel);
    KGLOG_PROCESS_ERROR(pVipInfo);

    m_nMaxFatiguePoint = pVipInfo->nMaxFatiguePoint;

    nDeltFatiguePoint = pVipInfo->nMaxFatiguePoint - m_nCurFatiguePoint;
    m_nCurFatiguePoint = pVipInfo->nMaxFatiguePoint;

    PLAYER_LOG(this, "fatigue,add,reset,%d", nDeltFatiguePoint);

    m_nTodayBuyFatiguePointTimes = 0;

    g_PlayerServer.DoSyncPlayerStateInfo(this);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SetVIPLevel(int nVIPLevel, int nVIPExp)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    m_nVIPLevel = nVIPLevel;
    m_nVIPExp = nVIPExp;

    if (m_nConnIndex >= 0)    
        g_PlayerServer.DoSyncVIPLevel(m_nConnIndex, nVIPLevel, nVIPExp);

    if (IsVIP())
        ApplyVIP();

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::AddVIPExp(int nVIPExp)
{
    BOOL bResult     = false;
    BOOL bRetCode    = false;
    int  nNewExp     = 0;
    int  nNewLevel   = 0;
    BOOL bVIPLevelUp = false;    
    KVIPLevelData* pLevelData = NULL;
    KReport_VIP_LevelUp param;

    KGLOG_PROCESS_ERROR(nVIPExp > 0);
    KGLOG_PROCESS_ERROR(m_nVIPLevel < cdMaxVIPLevel);

    nNewExp = m_nVIPExp + nVIPExp;
    nNewLevel = m_nVIPLevel;

    pLevelData = g_pSO3World->m_Settings.m_VIPLevelData.GetByID(nNewLevel);
    while (nNewExp >= pLevelData->nExp)
    {
        ++nNewLevel;
        nNewExp -= pLevelData->nExp;

        OnEvent(peVIPLevelup, nNewLevel);

        if (nNewLevel == cdMaxVIPLevel)
            break;

        pLevelData = g_pSO3World->m_Settings.m_VIPLevelData.GetByID(nNewLevel);
        KGLOG_PROCESS_ERROR(pLevelData);
    }

    if (nNewLevel > m_nVIPLevel)
        bVIPLevelUp = true;

    bRetCode = SetVIPLevel(nNewLevel, nNewExp);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bVIPLevelUp)
    {
        param.level = (unsigned short)m_nVIPLevel;
        bRetCode = m_Secretary.AddReport(KREPORT_EVENT_VIP_LEVELUP, (BYTE*)&param, sizeof(param));
        KGLOG_PROCESS_ERROR(bRetCode);
        g_LSClient.DoRemoteCall("OnUpdateVIPLevel", (int)m_dwID, m_nVIPLevel);
        g_RelayClient.DoUpdateVIPInfo(this);
    }

    OnEvent(peAddVIPExp, nVIPExp, m_nVIPLevel);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::ChargeVIPTimeRequest(int nChargeType)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KVIPCharge* pCharge = NULL;
    KCUSTOM_CONSUME_INFO info = {0};

    pCharge = g_pSO3World->m_Settings.m_VIPCharge.GetByID(nChargeType);
    KGLOG_PROCESS_ERROR(pCharge);

    info.nValue1 = nChargeType;
    g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(this, uctChargeVIP, pCharge->nNeedCoin, &info);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::ChargeVIPTimeResult(int nChargeType, char* pszSrcName)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KVIPCharge* pCharge = NULL;
    KReport_Open_Or_Continue_VIP param1;
    KReport_Receive_Presentation_VIP param2;

    pCharge = g_pSO3World->m_Settings.m_VIPCharge.GetByID(nChargeType);
    KGLOG_PROCESS_ERROR(pCharge);

    param1.isOpen = (BYTE)(m_nVIPEndTime == 0);
    param2.isOpen = param1.isOpen;

    if (IsVIP())
    {
        m_nVIPEndTime += pCharge->nSeconds;
    }
    else
    {
        m_nVIPEndTime = g_pSO3World->m_nCurrentTime + pCharge->nSeconds;
        g_RelayClient.DoUpdateVIPInfo(this);
    }

    ApplyVIP();

    g_PlayerServer.DoSyncVIPEndTime(m_nConnIndex, m_nVIPEndTime);

    g_LSClient.DoRemoteCall("OnUpdateVIPEndTime", (int)m_dwID, (int)m_dwClubID, m_nVIPEndTime);

    if (pszSrcName)
    {
        memcpy(param2.srcName, pszSrcName, sizeof(param2.srcName));
        param2.vipChargeType = (BYTE)nChargeType;
        bRetCode = m_Secretary.AddReport(KREPORT_EVENT_RECEIVE_PRESENTATION_VIP, (BYTE*)&param2, sizeof(param2));
        KGLOG_PROCESS_ERROR(bRetCode);
        UpdateVIPAwardCount();

        goto Exit1;
    }

    param1.vipChargeType = (BYTE)nChargeType;
    bRetCode = m_Secretary.AddReport(KREPORT_EVENT_OPEN_OR_CONTINUE_VIP, (BYTE*)&param1, sizeof(param1)); 
    UpdateVIPAwardCount();

    KGLOG_PROCESS_ERROR(bRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::ApplyVIP()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KVIPConfig* pVIPInfo = NULL;

    pVIPInfo = g_pSO3World->m_Settings.m_VIPConfig.GetByID(m_nVIPLevel);
    KGLOG_PROCESS_ERROR(pVIPInfo);

    m_nMaxFatiguePoint = pVIPInfo->nMaxFatiguePoint;
    g_PlayerServer.DoSyncMaxFatiguePoint(m_nConnIndex, m_nMaxFatiguePoint);

    m_UpgradeQueue.SetMaxUpgradeCount(pVIPInfo->nUpgradeSlotCount);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::UnApplyVIP(BOOL bFirstLogin)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KVIPConfig* pVIPInfo = NULL;

    pVIPInfo = g_pSO3World->m_Settings.m_VIPConfig.GetByID(0);// 0表示没有vip
    KGLOG_PROCESS_ERROR(pVIPInfo);

    m_nMaxFatiguePoint = pVIPInfo->nMaxFatiguePoint;
    g_PlayerServer.DoSyncMaxFatiguePoint(m_nConnIndex, m_nMaxFatiguePoint);

    m_UpgradeQueue.SetMaxUpgradeCount(pVIPInfo->nUpgradeSlotCount);

    m_bVIPAutoRepair = false;
    g_PlayerServer.DoSyncVIPAutoRepairFlag(this, m_bVIPAutoRepair);

    KG_PROCESS_SUCCESS(bFirstLogin);

    bRetCode = m_Secretary.AddReport(KREPORT_EVENT_VIP_TIMEOUT, NULL, 0);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::SetVIPAutoRepairFlag(BOOL bAutoRepairFlag)
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    if (IsVIP() && m_nVIPLevel >= 3)
        m_bVIPAutoRepair = bAutoRepairFlag;

    if (m_nConnIndex >= 0)
        g_PlayerServer.DoSyncVIPAutoRepairFlag(this, m_bVIPAutoRepair);

    bResult	= true;
Exit0:
    return bResult;
}

BOOL KPlayer::PresentVIPTimeToFriendRequest(uint32_t dwFriendID, int nChargeType)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KGFellowship* pFellowship = NULL;
    KVIPCharge* pCharge = NULL;
    KCUSTOM_CONSUME_INFO info = {0};

    pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(m_dwID, dwFriendID);
    KGLOG_PROCESS_ERROR(pFellowship);

    pCharge = g_pSO3World->m_Settings.m_VIPCharge.GetByID(nChargeType);
    KGLOG_PROCESS_ERROR(pCharge);

    // 请求paysys扣费
    info.nValue1 = dwFriendID;
    info.nValue2 = nChargeType;
    g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(this, uctPresentVIP, pCharge->nNeedCoin, &info);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::PresentVIPTimeToFriendResult(uint32_t dwFriendID, int nChargeType)
{
    g_LSClient.DoPresentVIPTimeToFriendRequest(m_dwID, dwFriendID, nChargeType);
    LogInfo("%u present vip %d to friend %u, send to logicserver", m_dwID, nChargeType, dwFriendID);
    PLAYER_LOG(this, "item,present_vip,%d", dwFriendID);
    return true;
}

BOOL KPlayer::StrengthenEquip(
    int nPackageTypeEquipIn, int nPackageIndexEquipIn, int nPosEquipIn, int nCurStrengthenLevel, 
    BOOL bUseLuckyItem, int nPackageTypeLuckItemIn, int nPackageIndexLuckyItemIn, int nPosLuckyItemIn
    )
{
    BOOL        bResult             = false;
    BOOL        bRetCode            = false;
    IItem*      pEquip              = NULL;
    IItem*      pLuckyItem          = NULL;
    int         nEquipLevel         = 0;
    int         nSuccessRate        = 0;
    int         nAddedSuccessRate   = 0;
    unsigned    uRandomValue        = 0;
    int         nStrengthenLevel    = 0;
    KMaterial*  pMaterial           = NULL;
    int         nDecreaseLevel      = 0;
    const KItemProperty*         pEquipProperty      = NULL;
    const KItemProperty*         pLuckyItemProperty  = NULL;
    KEQUIPSTRENGTHEN_LOGICDATA*  pLogicData          = NULL;

    if (bUseLuckyItem)
    {
        KGLOG_PROCESS_ERROR(
            nPackageTypeEquipIn != nPackageTypeLuckItemIn || 
            nPackageIndexEquipIn != nPackageIndexLuckyItemIn || 
            nPosEquipIn != nPosLuckyItemIn
        );
    }

    pEquip = m_ItemList.GetItem(nPackageTypeEquipIn, nPackageIndexEquipIn, nPosEquipIn);
    KGLOG_PROCESS_ERROR(pEquip);

    pEquipProperty = pEquip->GetProperty();
    KGLOG_PROCESS_ERROR(pEquipProperty);

    KGLOG_PROCESS_ERROR(pEquipProperty->pItemInfo->nGenre == igEquipment);
    KG_PROCESS_ERROR(pEquipProperty->nStrengthenLevel == nCurStrengthenLevel);

    if (pEquipProperty->nStrengthenLevel >= cdMaxEquipStrengthenLevel)
    {
        g_PlayerServer.DoDownwardNotify(this, KMESSAGE_STRENGTHEN_EQUIP_MAX_STRENGTHENLEVEL);
        goto Exit0;
    }

    pLogicData = g_pSO3World->m_ItemHouse.GetStrengthLogicData(pEquipProperty->nStrengthenLevel);
    KGLOG_PROCESS_ERROR(pLogicData);

    nEquipLevel = pEquipProperty->pItemInfo->nLevel;

    KGLOG_PROCESS_ERROR(nEquipLevel >= 1 && nEquipLevel <= (int)countof(pLogicData->Fee));  
    bRetCode = m_MoneyMgr.CanAddMoney(emotMoney, -pLogicData->Fee[nEquipLevel - 1]);
    if (!bRetCode)
    {
        g_PlayerServer.DoDownwardNotify(this, KMESSAGE_STRENGTHEN_EQUIP_NOT_ENOUGH_MONEY);
        goto Exit0;
    }

    KGLOG_PROCESS_ERROR(nEquipLevel >= 1 && nEquipLevel <= (int)countof(pLogicData->Materials));
    pMaterial = &pLogicData->Materials[nEquipLevel - 1];

    bRetCode = m_ItemList.HasItem(pMaterial->dwItemType, pMaterial->dwItemIndex,  pMaterial->nCount);
    if (!bRetCode)
    {
        g_PlayerServer.DoDownwardNotify(this, KMESSAGE_STRENGTHEN_EQUIP_NOT_ENOUGH_MATERIAL);
        goto Exit0;
    }

    if (bUseLuckyItem)
    {
        pLuckyItem = m_ItemList.GetItem(nPackageTypeLuckItemIn, nPackageIndexLuckyItemIn, nPosLuckyItemIn);
        KGLOG_PROCESS_ERROR(pLuckyItem);

        pLuckyItemProperty = pLuckyItem->GetProperty();
        KGLOG_PROCESS_ERROR(pLuckyItemProperty);

        bRetCode = g_pSO3World->m_ItemHouse.GetAddedSuccessRate(pLuckyItem, nAddedSuccessRate);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    // 开始强化
    bRetCode = m_MoneyMgr.AddMoney(emotMoney, -pLogicData->Fee[nEquipLevel - 1]);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_ItemList.CostPlayerItem(pMaterial->dwItemType, pMaterial->dwItemIndex, pMaterial->nCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    if (bUseLuckyItem)
    {
        bRetCode = m_ItemList.CostItem(nPackageTypeLuckItemIn, nPackageIndexLuckyItemIn, nPosLuckyItemIn, 1);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    nSuccessRate = pLogicData->nSuccessRate + nAddedSuccessRate;
    MAKE_IN_RANGE(nSuccessRate, 0, HUNDRED_NUM);

    bRetCode = g_RandPercent(nSuccessRate);
    if (bRetCode)
    {
        bRetCode = g_pSO3World->m_ItemHouse.SetEquipStrengthenLevel(pEquip, pEquipProperty->nStrengthenLevel + 1);
        KGLOG_PROCESS_ERROR(bRetCode);

        g_PlayerServer.DoSyncItemData(m_nConnIndex, m_dwID, pEquip, nPackageTypeEquipIn, nPackageIndexEquipIn, nPosEquipIn);
        g_PlayerServer.DoStrengthenEquipNotify(m_nConnIndex, (KPACKAGE_TYPE)nPackageTypeEquipIn, nPackageIndexEquipIn, nPosEquipIn, true, pEquipProperty->nStrengthenLevel);

        {
            KMessage68_Para cParam;
            cParam.nStrengthenLevel = pEquipProperty->nStrengthenLevel;
            memcpy(cParam.szEquipName, pEquipProperty->pItemInfo->szName, sizeof(cParam.szEquipName));
            g_PlayerServer.DoDownwardNotify(this, KMESSAGE_STRENGTHEN_EQUIP_SUCCESS, &cParam, sizeof(cParam));
        }

        if (pLogicData->bMessageOnSuccess)
        {
            KMessage72_Para cParam;
            cParam.dwPlayerID = m_dwID;
            memcpy(cParam.szPlayerName, m_szName, sizeof(cParam.szPlayerName));
            memcpy(cParam.szEquipName, pEquipProperty->pItemInfo->szName, sizeof(cParam.szEquipName));
            cParam.nStrengthenLevel = pEquipProperty->nStrengthenLevel;

            g_LSClient.DoBroadcastMessage(KMESSAGE_STRENGTHEN_EQUIP_SUCCESS_NEWS, &cParam, sizeof(cParam));
        }

        OnEvent(peStrengthenEquipSuccess, nSuccessRate, pEquipProperty->nStrengthenLevel);

        goto Exit1;
    }

    bRetCode = g_GetRandomOne(&pLogicData->FailResult[0], pLogicData->FailResult + countof(pLogicData->FailResult), nDecreaseLevel);
    if (bRetCode && nDecreaseLevel > 0)
    {
        nStrengthenLevel = pEquipProperty->nStrengthenLevel - nDecreaseLevel;
        MAKE_IN_RANGE(nStrengthenLevel, 0, cdMaxEquipStrengthenLevel);

        bRetCode = g_pSO3World->m_ItemHouse.SetEquipStrengthenLevel(pEquip, nStrengthenLevel);
        KGLOG_PROCESS_ERROR(bRetCode);

        {
            KMessage71_Para cParam;
            cParam.nStrengthenLevel = pEquipProperty->nStrengthenLevel;
            memcpy(cParam.szEquipName, pEquipProperty->pItemInfo->szName, sizeof(cParam.szEquipName));
            g_PlayerServer.DoDownwardNotify(this, KMESSAGE_STRENGTHEN_EQUIP_FAILED_DECREASE, &cParam, sizeof(cParam));
        }

        if (nStrengthenLevel == 0 && pLogicData->bMessageOnFailedToLevel0)
        {
            KMessage73_Para cParam;
            cParam.dwPlayerID = m_dwID;
            memcpy(cParam.szPlayerName, m_szName, sizeof(cParam.szPlayerName));
            memcpy(cParam.szEquipName, pEquipProperty->pItemInfo->szName, sizeof(cParam.szEquipName));
            cParam.nStrengthenLevel = nCurStrengthenLevel + 1;
            g_LSClient.DoBroadcastMessage(KMESSAGE_STRENGTHEN_EQUIP_FAILEDTOZERO_NEWS, &cParam, sizeof(cParam));
        }
    }
    else
    {
        KMessage69_Para cParam;
        memcpy(cParam.szEquipName, pEquipProperty->pItemInfo->szName, sizeof(cParam.szEquipName));
        g_PlayerServer.DoDownwardNotify(this, KMESSAGE_STRENGTHEN_EQUIP_FAILED_NODECREASE, &cParam, sizeof(cParam));
    }

    g_PlayerServer.DoSyncItemData(m_nConnIndex, m_dwID, pEquip, nPackageTypeEquipIn, nPackageIndexEquipIn, nPosEquipIn);
    g_PlayerServer.DoStrengthenEquipNotify(m_nConnIndex, (KPACKAGE_TYPE)nPackageTypeEquipIn, nPackageIndexEquipIn, nPosEquipIn, false, pEquipProperty->nStrengthenLevel);

    OnEvent(peStrengthenEquipFailed, nSuccessRate, pEquipProperty->nStrengthenLevel);


    if (pEquipProperty)
    {
        g_LogClient.DoFlowRecord(
            frmtMakingAndStrengthen, masfrStrengthenFailed, 
            "%s,%d,%u,%u,%d,%s",
            m_szName, 
            pLogicData ? pLogicData->Fee[nEquipLevel - 1] : 0,
            pMaterial ? pMaterial->dwItemType : 0,
            pMaterial ? pMaterial->dwItemIndex : 0,
            pMaterial ? pMaterial->nCount : 0,
            pEquipProperty->pItemInfo ? pEquipProperty->pItemInfo->szName : ""
            );
    }

Exit1:
	if (nPackageTypeEquipIn == eHeroPackage)
	{
		g_LSClient.DoHeroEquipScoreChangeNotify(this, (DWORD)nPackageIndexEquipIn);
		g_LSClient.DoTeamEquipScoreChangeNotify(this);
	}

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::BuyFatiguePoint(int nBuySN)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nCoinCost = 0;
    KFatiguePointPrice* pPrice = NULL;

    KGLOG_PROCESS_ERROR(nBuySN == m_nTodayBuyFatiguePointTimes + 1);

    pPrice = g_pSO3World->m_Settings.m_FatiguePointPriceSettings.GetByID(nBuySN);
    if(!pPrice)
        pPrice = g_pSO3World->m_Settings.m_FatiguePointPriceSettings.GetLast();
    KGLOG_PROCESS_ERROR(pPrice);

    if(pPrice->nMoneyType == emotCoin)
    {
        bRetCode = g_pSO3World->m_ShopMgr.DoCustomConsumeRequest(this, uctBuyFatiguePoint, pPrice->nMoneyCount);
        KGLOG_PROCESS_ERROR(bRetCode);
        goto Exit1;
    }

    bRetCode = m_MoneyMgr.AddMoney((ENUM_MONEY_TYPE)pPrice->nMoneyType, -pPrice->nMoneyCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_nCurFatiguePoint += g_pSO3World->m_Settings.m_ConstList.nBuyFatigueValue;
    ++m_nTodayBuyFatiguePointTimes;

    g_PlayerServer.DoSyncPlayerStateInfo(this);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayer::OnBuyFatigueSuccess()
{
    m_nCurFatiguePoint += g_pSO3World->m_Settings.m_ConstList.nBuyFatigueValue;
    ++m_nTodayBuyFatiguePointTimes;

    g_PlayerServer.DoSyncPlayerStateInfo(this);

    return true;
}

void KPlayer::FillShortcutBar()
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;

    bRetCode = m_ItemList.m_PlayerPackage.FillShortcutBar();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    return;
}

BOOL KPlayer::SetPlayerValue(DWORD dwPlayerValueID, int nNewValue)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KPlayerValueInfo* pPlayerValueInfo = NULL;

    pPlayerValueInfo = g_pSO3World->m_Settings.m_PlayerValueInfoList.GetPlayerValue(dwPlayerValueID);
    KGLOG_PROCESS_ERROR(pPlayerValueInfo);

    if (pPlayerValueInfo->nType == KPLAYER_VALUE_TYPE_BOOLEAN)
        nNewValue = nNewValue ? 1 : 0;

    bRetCode = m_PlayerValue.SetValue(dwPlayerValueID, nNewValue);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

KScene* KPlayer::GetCurrentScene()
{
    KScene* pResult = NULL;
    KHero* pFightingHero = NULL;

    pFightingHero = GetFightingHero();
    KGLOG_PROCESS_ERROR(pFightingHero);

    pResult = pFightingHero->m_pScene;
Exit0:
    return pResult;
}

BOOL KPlayer::SetFatiguePointDefaultValue()
{
    BOOL bResult = false;
    KVIPConfig* pVipInfo = NULL;

    pVipInfo = g_pSO3World->m_Settings.m_VIPConfig.GetByID(m_nVIPLevel);
    KGLOG_PROCESS_ERROR(pVipInfo);

    m_nMaxFatiguePoint              = pVipInfo->nMaxFatiguePoint;
    m_nCurFatiguePoint              = pVipInfo->nMaxFatiguePoint;
    m_nTodayBuyFatiguePointTimes    = 0;

    bResult = true;
Exit0:
    return bResult;
}

void KPlayer::ReportLevelUpAward()
{
    BOOL bRetCode = false;
    int nPlayerValueIndex = 0;
    int nGetAwardFlag = 0;
    int nModLevel = m_nLevel / 10;
    KReport_LevelUp_Award param;

    KG_PROCESS_ERROR(m_nLevel % 10 == 0);
    KG_PROCESS_ERROR(nModLevel > 0);

    nPlayerValueIndex = KPLAYER_VALUE_ID_LEVEL_AWARD_10 + nModLevel - 1;
    KGLOG_PROCESS_ERROR(nPlayerValueIndex <= KPLAYER_VALUE_ID_LEVEL_AWARD_80);

    bRetCode = m_PlayerValue.GetValue(nPlayerValueIndex, nGetAwardFlag);
    KGLOG_PROCESS_ERROR(bRetCode);
    KGLOG_PROCESS_ERROR(nGetAwardFlag == 0); // 已经领过

    param.wlevel = (WORD)m_nLevel;
    m_Secretary.AddReport(KREPORT_EVENT_LEVELUP_AWARD, (BYTE*)&param, sizeof(param));

Exit0:
    return;
}


void KPlayer::ReportOnlineAward()
{
    KG_PROCESS_ERROR(m_nReportOnlineGetAward != m_nOnlineGetAwardTimes);
    KG_PROCESS_ERROR(g_pSO3World->m_nCurrentTime >= m_nOnlineAwardTime);
    KG_PROCESS_ERROR(m_nOnlineGetAwardTimes < cdOnlineAwardTimes);

    m_nReportOnlineGetAward = m_nOnlineGetAwardTimes;
    m_Secretary.AddReport(KREPORT_EVENT_ONLINE_AWARD, NULL, 0);

Exit0:
    return;
}

void KPlayer::ReportBuffEnd()
{ 
    KReport_Buff_End            param;
    KBuffTable::const_iterator  itBuff; 

    KG_PROCESS_ERROR((g_pSO3World->m_nGameLoop - m_dwID) % (SECONDS_PER_MINUTE * GAME_FPS) == 0);

    for (itBuff = m_BuffTable.begin(); itBuff != m_BuffTable.end(); ++itBuff)
    {
        const KBuffInfo* pBuffInfo = &itBuff->second;
        int nPassTime = pBuffInfo->m_nEndFrame - g_pSO3World->m_nGameLoop;
        if (pBuffInfo->pBuff &&  nPassTime <= SECONDS_PER_MINUTE * GAME_FPS && nPassTime > 0)
        {
            param.buffID = pBuffInfo->pBuff->m_dwID;
            m_Secretary.AddReport(KREPORT_EVENT_BUFF_END, (BYTE*)&param, sizeof(param));
        }
    }

Exit0:
    return;
}

int KPlayer::GetCurrentChallengeType()
{
    if (m_nCurrentCity == emitPVE1)
        return emitPVE1ContinuousChallenge;
    else if (m_nCurrentCity == emitPVE2)
        return emitPVE2ContinuousChallenge;

    return emitPVE1ContinuousChallenge;
}

int KPlayer::GetLastChallengeStep()
{
    if (m_nCurrentCity == emitPVE1)
        return m_nLastChallengeStep1;
    else if (m_nCurrentCity == emitPVE2)
        return m_nLastChallengeStep2;

    return m_nLastChallengeStep1;
}

int KPlayer::GetStartChallengeStep()
{
    if (m_nCurrentCity == emitPVE1)
        return m_nStartChallengeStep1;
    else if (m_nCurrentCity == emitPVE2)
        return m_nStartChallengeStep2;

    return m_nStartChallengeStep1;
}

void KPlayer::SetLastChallengeStep(int nLastChallengeType)
{
    if (m_nCurrentCity == emitPVE1)
        m_nLastChallengeStep1 = nLastChallengeType;
    else if (m_nCurrentCity == emitPVE2)
        m_nLastChallengeStep2 = nLastChallengeType;
    return;
}

void KPlayer::SetStartChallengeStep(int nStartChallengeType)
{
    if (m_nCurrentCity == emitPVE1)
        m_nStartChallengeStep1 = nStartChallengeType;
    else if (m_nCurrentCity == emitPVE2)
        m_nStartChallengeStep2 = nStartChallengeType;
    else
        m_nStartChallengeStep1 = nStartChallengeType;
    return;
}

void KPlayer::SetCurrentCityByMissionType(int nType)
{
    int nCity = emitPVE1;
    if (nType == emitPVE1ContinuousChallenge)
        nCity = emitPVE1;
    else if (nType == emitPVE2ContinuousChallenge)
        nCity = emitPVE2;

    KG_PROCESS_ERROR(m_nCurrentCity != nCity);

    m_nCurrentCity = nCity;

    g_PlayerServer.DoSyncCurrentCity(m_nConnIndex, nCity);

Exit0:
    return;
}

BOOL KPlayer::DoLadderPVPFatigueCost()
{
    return DoFatiguePointCost(g_pSO3World->m_Settings.m_ConstList.nRankConstFatiguePoint);
}

BOOL KPlayer::GetNoFatigueWinRankCount(int& nCount)
{
    return m_PlayerValue.GetValue(KPLAYER_VALUE_ID_NO_FATIGUE_WIN_RANK_COUNT, nCount);
}

BOOL KPlayer::SetNoFatigueWinRankCount(int nCount)
{
    return m_PlayerValue.SetValue(KPLAYER_VALUE_ID_NO_FATIGUE_WIN_RANK_COUNT, nCount);
}
