#include "stdafx.h"
#include "KGWConstList.h"

#define SECTION_GLOBAL                  "GLOBAL"
#define SECTION_STAMINA                 "STAMINA"
#define SECTION_ANGRY                   "ANGRY"
#define SECTION_MAIL                    "MAIL"
#define SECTION_CHAT                    "CHAT"
#define SECTION_TRAININGSEAT            "TRAININGSEAT"

//#define   DEFAULT_GRAVITY     (10 * 64 / GAME_FPS / GAME_FPS)	//单位：点/帧^2
#define     GRAVITY_DEFAULT     8192 //调整fps为32后的测试值 //单位：点/秒^2

BOOL KGWConstList::Init()
{
    BOOL        bResult     = false;
	int         nRetCode    = false;
	IIniFile*   piIniFile   = NULL;
	char        szFilePath[MAX_PATH];

	nRetCode = snprintf(szFilePath, sizeof(szFilePath), "%s/%s", SETTING_DIR, GWINI_FILE_NAME);
    KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < sizeof(szFilePath));

	piIniFile = g_OpenIniFile(szFilePath);
	KGLOG_PROCESS_ERROR(piIniFile);

	nRetCode = LoadData(piIniFile);
	KGLOG_PROCESS_ERROR(nRetCode);

	bResult = true;
Exit0:
	KG_COM_RELEASE(piIniFile);
	return bResult;
}

void KGWConstList::UnInit()
{
    m_mapTrainingSeatPrice.clear();
}

BOOL KGWConstList::LoadData(IIniFile* piIniFile)
{
    BOOL        bResult             = false;
	int         nRetCode            = 0;
    int         AwardItem[3];
    char szKey[] = "SlotPrice000";
    char szFirstChargeKey[] = "FirstChargeAward0";

	assert(piIniFile);

	nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "Gravity_Default", GRAVITY_DEFAULT, &nGravityDefault);
    KGLOG_PROCESS_ERROR(nRetCode > 0);
    // 转换为点每帧
    nGravityDefault /= GAME_FPS * GAME_FPS;

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "PlayerHaltTime", 6 * GAME_FPS, &nPlayerHaltTime);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "PlayerKnockDownTime", 3 * GAME_FPS, &nPlayerKnockDownTime);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "PlayerWeakKnockDownTime", GAME_FPS, &nPlayerWeakKnockDownTime);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "KnockOffSpeedPercentOnDead", 2 * HUNDRED_NUM, &nKnockOffSpeedPercentOnDead);
    KGLOG_PROCESS_ERROR(nRetCode);
    
	nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "MaxMemberCount", 4, &nMaxMemberCount);
    KGLOG_PROCESS_ERROR(nRetCode);

	nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "SaveInterval", 120, &nSaveInterval);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "HeroExpWeeklyDecreasePercent", 0, &nHeroExpWeeklyDecreasePercent);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "MinVelocityCollidedBasket", 8, &nMinVelocityCollidedBasket);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "InitGymCount", 2, &nInitGymCount);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "AwardGymCountPerGymLevel", 1, &nAwardGymCountPerGymLevel);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetMultiInteger(SECTION_GLOBAL, "GymCountPrice", nGymCountPrice, countof(nGymCountPrice));
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetMultiInteger(SECTION_GLOBAL, "ExtendPackageSlotPrice", nExtendPackageSlotPrice, countof(nExtendPackageSlotPrice));
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "InitUpgradeCount", 2, &nInitUpgradeCount);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "ClearCDCoinPerMinute", 3, &nClearCDCoinPerMinute);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "InitTeacherLevel", 1, &nInitTeacherLevel);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "InitMakingMachineLevel", 1, &nInitMakingMachineLevel);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "InitHeroTrainingSitCount", 1, &nInitHeroTrainingSitCount);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "AwardTrainingSitPerTeacherLevel", 1, &nAwardTrainingSitPerTeacherLevel);
    KGLOG_PROCESS_ERROR(nRetCode);
    KGLOG_PROCESS_ERROR(nAwardTrainingSitPerTeacherLevel > 0);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "TrainCoinPerMinute", 1, &nTrainCoinPerMinute);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "GymCoinPerMinute", 1, &nGymCoinPerMinute);
    KGLOG_PROCESS_ERROR(nRetCode);
   
    piIniFile->GetInteger(SECTION_GLOBAL, "GetUpProtectFrame", 1, &nGetUpProtectFrame);
    KGLOG_PROCESS_ERROR(nGetUpProtectFrame > 0);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "DummyAttackBallVXY", 1000, &nDummyAttackBallVXY);
    KGLOG_PROCESS_ERROR(nRetCode);
    KGLOG_PROCESS_ERROR(nDummyAttackBallVXY >= 0);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "DummyAttackBallVZ", 500, &nDummyAttackBallVZ);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "MaxSkipRandomQuestTime", 6, &nMaxSkipRandomQuestTime);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "EnduranceCoeInAir", HUNDRED_NUM, &nEnduranceCoeInAir);
    KGLOG_PROCESS_ERROR(nRetCode);
 
    nRetCode = piIniFile->GetInteger(SECTION_ANGRY, "SelfHit", 0, &nSelfHitAddAngry);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_ANGRY, "OtherHit", 0, &nOtherHitAddAngry);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_CHAT, "SmallHornItemTabType", 0, &nGroupMessageItemTabType);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_CHAT, "SmallHornItemTabIndex", 0, &nGroupMessageItemTabIndex);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_CHAT, "BigHornItemTabType", 0, &nGlobalMessageItemTabType);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_CHAT, "BigHornItemTabIndex", 0, &nGlobalMessageItemTabIndex);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_CHAT, "RoomChannelCoolDownID", 0, &nRoomChannelCoolDownID);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_CHAT, "WorldChannelCoolDownID", 0, &nWorldChannelCoolDownID);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_CHAT, "WorldChannelDailyCount", 0, &nWorldChannelDailyCount);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("FREEPVP", "AwardItemTabType", 0, &nFreePvpAwardItemTabType);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("FREEPVP", "AwardItemTabIndex", 0, &nFreePvpAwardItemTabIndex);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("LADDERPVP", "AwardItemTabType", 0, &nLadderPvpAwardItemTabType);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("LADDERPVP", "AwardItemTabIndex", 0, &nLadderPvpAwardItemTabIndex);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetMultiInteger("KBUSINESSSTREET", "LandCost", nLandCost, countof(nLandCost));
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("KBUSINESSSTREET", "DirectGetMoneyNeedCoinPerMinute", 3, &nDirectGetMoneyNeedCoinPerMinute);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("KBUSINESSSTREET", "ChangeStoreTypeCDTime", 10800, &nChangeStoreTypeCDTime);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("FatiguePoint", "BuyFatigueValue", 0, &nBuyFatigueValue);
    KGLOG_PROCESS_ERROR(nRetCode);
    KGLOG_PROCESS_ERROR(nBuyFatigueValue > 0);

    nRetCode = piIniFile->GetInteger("CHEERLEADING", "DefaultSlotCount", 0, (int*)&uDefaultCheerleaindgSlotCount);
    KGLOG_PROCESS_ERROR(nRetCode);
    
    nRetCode = piIniFile->GetInteger("HEROSLOT", "MaxSlotCount", 0 ,&nMaxHeroSlot);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("SCENEOBJID", "BuffObjID", 0 ,&nSceneBuffObjID);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("SCENEOBJID", "BigGoldObjID", 0 ,&nScenenBigGoldID);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("SCENEOBJID", "SmallGoldObjID", 0 ,&nScenenSmallGoldID);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_MAIL, "CostMoney", 0 ,&nMailCostMoney);
    KGLOG_PROCESS_ERROR(nRetCode);
    
    nRetCode = piIniFile->GetInteger(SECTION_MAIL, "MailText", 0 ,&nMailTextSize);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("CLUB", "PlayerApplyCount", 0 ,&nPlayerApplyLimit);
    KGLOG_PROCESS_ERROR(nRetCode);
	
    nRetCode = piIniFile->GetInteger("CLUB", "CreateClubMoney", 0 ,&nMoneyForCreateClub);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("CLUB", "CreateClubLevel", 0 ,&nLevelForCreateClub);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("CLUB", "AddClubCoolDown", 0 ,&nAddClubCoolDown);
    KGLOG_PROCESS_ERROR(nRetCode);

    m_mapHeroSlotPrice.clear();
    for (int i = 1; i <= nMaxHeroSlot; ++i)
    {
        int nPrice = 0;

        nRetCode = snprintf(szKey, sizeof(szKey), "SlotPrice%d", i);
        KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < sizeof(szKey));
        
        nRetCode = piIniFile->GetInteger("HEROSLOT", szKey, 0, &nPrice);
        m_mapHeroSlotPrice[i] = nPrice;
        KGLOG_PROCESS_ERROR(nRetCode > 0);
        KGLOG_PROCESS_ERROR(m_mapHeroSlotPrice[i] >= 0);
    }

    memset(CheerleadingSlotPrice, 0, sizeof(CheerleadingSlotPrice));
    nRetCode = piIniFile->GetMultiInteger("CHEERLEADING", "SlotPrice", CheerleadingSlotPrice, countof(CheerleadingSlotPrice));
    KGLOG_PROCESS_ERROR(nRetCode + uDefaultCheerleaindgSlotCount == cdMaxCheerleadingSlotCount);

    nRetCode = piIniFile->GetInteger("CHEERLEADING", "MaxCheerleadingCount", 0, &nMaxCheerleadingCount);
    KGLOG_PROCESS_ERROR(nRetCode);
    KGLOG_PROCESS_ERROR(nMaxCheerleadingCount > 0);

    nRetCode = piIniFile->GetInteger("CHEERLEADING", "MaxFashionCount", 0, &nMaxFashionCount);
    KGLOG_PROCESS_ERROR(nRetCode);
    KGLOG_PROCESS_ERROR(nMaxFashionCount > 0);

    for (int i = 0; i < nRetCode; ++i)
        KGLOG_PROCESS_ERROR(CheerleadingSlotPrice[i] > 0);

    nRetCode = LoadTrainingSeatPrice(piIniFile);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("CONTINUOUS_CHALLENGE", "TiaoZhanShuTabType", 0, &nTiaoZhanShuTabType);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("CONTINUOUS_CHALLENGE", "TiaoZhanShuTabIndex", 0, &nTiaoZhanShuTabIndex);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("CONTINUOUS_CHALLENGE", "MissionCount", 0, &nChallengeMissionCount);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("ONLINE_AWARD", "RealAwardTableID", 0, &nRealAwardTableID);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("ONLINE_AWARD", "UnRealAwardTableID", 0, &nUnRealAwardTableID);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("ACTIVITY", "SignCoinCost", 500, &nSignCoinCost);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetMultiInteger("ACTIVITY", "FavoriteAward", AwardItem, countof(AwardItem));
    KGLOG_PROCESS_ERROR(nRetCode);
    cFavoriteAward.dwItemType  = (DWORD)AwardItem[0];
    cFavoriteAward.dwItemIndex = (DWORD)AwardItem[1];
    cFavoriteAward.nCount      = AwardItem[2];
    

    nRetCode = piIniFile->GetMultiInteger("ACTIVITY", "SignCard", AwardItem, countof(AwardItem));
    KGLOG_PROCESS_ERROR(nRetCode);
    cSignCard.dwItemType  = (DWORD)AwardItem[0];
    cSignCard.dwItemIndex = (DWORD)AwardItem[1];
    cSignCard.nCount      = AwardItem[2];

    nRetCode = piIniFile->GetMultiInteger("ACTIVITY", "5DaysAwardTotal", AwardItem, countof(AwardItem));
    KGLOG_PROCESS_ERROR(nRetCode);
    cSign5DaysAwardTotal.dwItemType  = (DWORD)AwardItem[0];
    cSign5DaysAwardTotal.dwItemIndex = (DWORD)AwardItem[1];
    cSign5DaysAwardTotal.nCount      = AwardItem[2];

    nRetCode = piIniFile->GetMultiInteger("ACTIVITY", "10DaysAwardTotal", AwardItem, countof(AwardItem));
    KGLOG_PROCESS_ERROR(nRetCode);
    cSign10DaysAwardTotal.dwItemType  = (DWORD)AwardItem[0];
    cSign10DaysAwardTotal.dwItemIndex = (DWORD)AwardItem[1];
    cSign10DaysAwardTotal.nCount      = AwardItem[2];

    nRetCode = piIniFile->GetMultiInteger("ACTIVITY", "21DaysAwardTotal", AwardItem, countof(AwardItem));
    KGLOG_PROCESS_ERROR(nRetCode);
    cSign21DaysAwardTotal.dwItemType  = (DWORD)AwardItem[0];
    cSign21DaysAwardTotal.dwItemIndex = (DWORD)AwardItem[1];
    cSign21DaysAwardTotal.nCount      = AwardItem[2];

    nRetCode = piIniFile->GetMultiInteger("ACTIVITY", "2DaysAwardContinuous", AwardItem, countof(AwardItem));
    KGLOG_PROCESS_ERROR(nRetCode);
    cSign2DaysAwardContinuous.dwItemType  = (DWORD)AwardItem[0];
    cSign2DaysAwardContinuous.dwItemIndex = (DWORD)AwardItem[1];
    cSign2DaysAwardContinuous.nCount      = AwardItem[2];

    nRetCode = piIniFile->GetMultiInteger("ACTIVITY", "7DaysAwardContinuous", AwardItem, countof(AwardItem));
    KGLOG_PROCESS_ERROR(nRetCode);
    cSign7DaysAwardContinuous.dwItemType  = (DWORD)AwardItem[0];
    cSign7DaysAwardContinuous.dwItemIndex = (DWORD)AwardItem[1];
    cSign7DaysAwardContinuous.nCount      = AwardItem[2];

    nRetCode = piIniFile->GetMultiInteger("ACTIVITY", "14DaysAwardContinuous", AwardItem, countof(AwardItem));
    KGLOG_PROCESS_ERROR(nRetCode);
    cSign14DaysAwardContinuous.dwItemType  = (DWORD)AwardItem[0];
    cSign14DaysAwardContinuous.dwItemIndex = (DWORD)AwardItem[1];
    cSign14DaysAwardContinuous.nCount      = AwardItem[2];

    for (int i = 1; i <= FIRST_CHARGE_AWARD_COUNT; ++i)
    {
        nRetCode = snprintf(szFirstChargeKey, sizeof(szFirstChargeKey), "FirstChargeAward%d", i);
        KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < sizeof(szFirstChargeKey));

        nRetCode = piIniFile->GetMultiInteger("ACTIVITY", szFirstChargeKey, AwardItem, countof(AwardItem));
        KGLOG_PROCESS_ERROR(nRetCode);
        cFirstChargeAward[i - 1].dwItemType = (DWORD)AwardItem[0];
        cFirstChargeAward[i - 1].dwItemIndex = (DWORD)AwardItem[1];
        cFirstChargeAward[i - 1].nCount = AwardItem[2];
    }

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "AdditionalHitRateOnDrop", 0, &nAdditionalHitRateOnDrop);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "RandomMinVXOnDrop", 0, &nRandomMinVXOnDrop);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "RandomMaxVXOnDrop", 0, &nRandomMaxVXOnDrop);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "RandomMinVYOnDrop", 0, &nRandomMinVYOnDrop);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "RandomMaxVYOnDrop", 0, &nRandomMaxVYOnDrop);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger(SECTION_GLOBAL, "SlamBallReboundFrame", 0, &nSlamBallReboundFrame);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("NORMALSLAMBALL", "AdditionalHitRate", 0, &nAdditionalNormalSlamBallHitRate);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("NORMALSLAMBALL", "RandomMinVX", 0, &nNormalSlamBallRandomMinVX);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("NORMALSLAMBALL", "RandomMaxVX", 0, &nNormalSlamBallRandomMaxVX);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("NORMALSLAMBALL", "RandomMinVY", 0, &nNormalSlamBallRandomMinVY);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("NORMALSLAMBALL", "RandomMaxVY", 0, &nNormalSlamBallRandomMaxVY);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("NORMALSLAMBALL", "RandomMinVZ", 0, &nNormalSlamBallRandomMinVZ);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("NORMALSLAMBALL", "RandomMaxVZ", 0, &nNormalSlamBallRandomMaxVZ);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("MakingMachine", "CritRate", 0, &nMKCritRate);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("MakingMachine", "CritValuePointPercent", 0, &nMKCritValuePointPercent);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("Decompose", "DecomposeSecondItemRate", 0, &nDecomposeSecondItemRate);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("Decompose", "CritRatePercent", 0, &nCECritRatePercent);
    KGLOG_PROCESS_ERROR(nRetCode);
    nRetCode = piIniFile->GetInteger("Decompose", "CritMultiple", 0, &nCECritMultiple);
    KGLOG_PROCESS_ERROR(nRetCode);

    nRetCode = piIniFile->GetInteger("Rank", "CostFatiguePoint", 0, &nRankConstFatiguePoint);
    KGLOG_PROCESS_ERROR(nRetCode);
    KGLOG_PROCESS_ERROR(nRankConstFatiguePoint > 0);

	nRetCode = piIniFile->GetInteger("LimitPlayInfoAward", "ItemType", 0, &nLimitPlayInfoAwardItemType);
	KGLOG_PROCESS_ERROR(nRetCode);
	nRetCode = piIniFile->GetInteger("LimitPlayInfoAward", "ItemIndex", 0, &nLimitPlayInfoAwardItemIndex);
	KGLOG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
	return bResult;
}

BOOL KGWConstList::LoadTrainingSeatPrice(IIniFile* piIniFile)
{
	BOOL bResult = false;
	int nRetCode = false;
    int nMaxSeatCount = 0;
    char szKey[] = "SeatCost000";

    assert(piIniFile);
    
    m_mapTrainingSeatPrice.clear();
    nRetCode = piIniFile->GetInteger(SECTION_TRAININGSEAT, "MaxCount", 0, &nMaxSeatCount);
    KGLOG_PROCESS_ERROR(nRetCode > 0);
    KGLOG_PROCESS_ERROR(nMaxSeatCount > 0);

    m_uMaxTrainingSeatCount = (unsigned)nMaxSeatCount;

    for (unsigned i = 1; i <= m_uMaxTrainingSeatCount; ++i)
    {
        nRetCode = snprintf(szKey, sizeof(szKey), "SeatCost%03d", i);
        KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < sizeof(szKey));

        m_mapTrainingSeatPrice[i].m_uPos        = i;
        m_mapTrainingSeatPrice[i].m_nMoneyType  = emotCoin;
        
        nRetCode = piIniFile->GetInteger(SECTION_TRAININGSEAT, szKey, 0, &m_mapTrainingSeatPrice[i].m_nPrice);
        KGLOG_PROCESS_ERROR(nRetCode > 0);
        KGLOG_PROCESS_ERROR(m_mapTrainingSeatPrice[i].m_nPrice >= 0);
    }
    
	bResult = true;
Exit0:
	return bResult;
}

const KTrainingSeatPrice* KGWConstList::GetTrainingSeatPrice(unsigned uPos) const
{
    const KTrainingSeatPrice* pResult = NULL;
    KMAP_TRAINING_SEAT_PRICE::const_iterator constIt;

    KGLOG_PROCESS_ERROR(uPos <= m_uMaxTrainingSeatCount);

    constIt = m_mapTrainingSeatPrice.find(uPos);
    assert(constIt != m_mapTrainingSeatPrice.end());

    pResult = &constIt->second;

Exit0:
    return pResult;
}

int KGWConstList::GetHeroSlotPrice(const unsigned uPos) const
{
    int nRetPrice = 0;
    KMAP_HERO_SLOT_PRICE::const_iterator constIt;

    KGLOG_PROCESS_ERROR(uPos <= (unsigned)nMaxHeroSlot);
    constIt = m_mapHeroSlotPrice.find(uPos);
     
    assert(constIt != m_mapHeroSlotPrice.end());

    nRetPrice = constIt->second;
Exit0:
    return nRetPrice;
}