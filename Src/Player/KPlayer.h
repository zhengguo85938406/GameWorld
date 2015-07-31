// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KPlayer.h 
//	Creator 	: Xiayong  
//  Date		: 08/07/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include <list>
#include "SO3ProtocolBasic.h"
#include "KHero.h"
#include "KCustomData.h"
#include "GlobalMacro.h"
#include "KRoleDBDataDef.h"
#include "KHeroDataList.h"
#include "KItemList.h"
#include "game_define.h"
#include "KMoneyMgr.h"
#include "KUserPreferences.h"
#include "KCDTimerList.h"
#include "KQuestList.h"
#include "KHeroTrainingMgr.h"
#include "KMissionData.h"
#include "KMakingMachine.h"
#include "KGym.h"
#include "KUpgradeQueue.h"
#include "KBusinessStreet.h"
#include "KSecretary.h"
#include "KWardrobe.h"
#include "KCheerleadingMgr.h"
#include "KPlayerValue.h"
#include "KAchievement.h"
#include "KActivePlayer.h"
#include "KBuffList.h"
#include "KHireHero.h"

enum GAME_STATUS
{
	gsInvalid = 0,			            // 无效状态

	gsWaitForConnect,			        // 正在等待客户端的连接
	gsWaitForPermit,		            // 登陆时正在等待GC的确认
    gsWaitForRoleData,		            // 登录时正在等待Goddess的回包
    gsInHall,                           // 在大厅中
	gsWaitForNewMapLoading,             // 切换地图过程中等待客户端载入

	gsPlaying,				            // 正在游戏中

	gsSearchMap,			            // 正在搜索地图,不一定需要跨服
    gsWaitForTransmissionSave,          // 保存数据准备跨服
	gsWaitForTransmissionGuid,	        // 跨服时正在等待目标服务器返回GUID

	gsTotal
};

enum KENUM_CONNECT_TYPE
{
    eInvalid,
    eLogin,
    eTransferGS
};

struct KMove_Critical_Param;

struct KAwardData;

class KHero;
namespace
{
    class KPB_SAVE_DATA;
}
namespace T3DB
{
    class KPB_SAVE_DATA;
}

#define PLAYER_EXT_POINT_SCRIPT         SCRIPT_DIR"/player/PlayerExtPoint.lua"

typedef std::map<std::string, int, std::less<std::string>, KMemory::KAllocator<std::pair<std::string, int> > > KMAP_DAILYRESET_USERDATA;

class KPlayer : public KObject
{
public:
	KPlayer();
	~KPlayer();

	BOOL Init(void);
	void UnInit(void);
    void Activate();

    void Award(KAWARD_CARD_TYPE eType, unsigned uChooseIndex=0);

    BOOL AddTeamExp(int nExp);
    BOOL AddHeroExp(DWORD dwHeroTemplateID, int nExp, int& nExpRealAdded);
    BOOL AddHeroExpNoAdditional(DWORD dwHeroTemplateID, int nExp);
    BOOL AddHeroLadderExp(uint32_t dwHeroTemplateID, int nExp);

    BOOL AddBuff(DWORD dwBuffID, int nLeftFrame = 0);
    BOOL DelBuff(DWORD dwBuffID);

    KHero* GetFightingHero() const;

    KHeroData* GetMainHeroData();
    KHeroData* GetAssistHeroData();

    BOOL LearnRecipe(DWORD dwRecipeID);
    BOOL ProduceItem(DWORD dwRecipeID);
    BOOL DoFatiguePointCost(int nCostFatiguePoint);
    BOOL ResetFatiguePoint(int nResetCount);
    BOOL SetVIPLevel(int nVIPLevel, int nVIPExp);
    BOOL AddVIPExp(int nVIPExp);
    BOOL ChargeVIPTimeRequest(int nChargeType);
    BOOL ChargeVIPTimeResult(int nChargeType, char* pszSrcName);
    BOOL ApplyVIP();
    BOOL UnApplyVIP(BOOL bFirstLogin);
    BOOL SetVIPAutoRepairFlag(BOOL bAutoRepairFlag);

    BOOL PresentVIPTimeToFriendRequest(uint32_t dwFriendID, int nChargeType);
    BOOL PresentVIPTimeToFriendResult(uint32_t dwFriendID, int nChargeType);

    BOOL StrengthenEquip(int nPackageTypeEquipIn, int nPackageIndexEquipIn, int nPosEquipIn, int nCurStrengthenLevel, BOOL bUseLuckyItem, int nPackageTypeLuckItemIn, int nPackageIndexLuckyItemIn, int nPosLuckyItemIn);
    BOOL BuyFatiguePoint(int nBuySN);
    BOOL OnBuyFatigueSuccess();
    void FillShortcutBar();
    
    BOOL SetPlayerValue(DWORD dwPlayerValueID, int nNewValue);
    KScene* GetCurrentScene();
    BOOL SetFatiguePointDefaultValue();

    BOOL SelectHeroSkill(DWORD dwHeroTemplateID, int nSkillGroup, int nSlotIndex, DWORD dwSkillID);
    BOOL DoLadderPVPFatigueCost();
    BOOL GetNoFatigueWinRankCount(int& nCount);
    BOOL SetNoFatigueWinRankCount(int nCount);

public:
	GUID				m_Guid;
    int                 m_nGroupID;             
    int                 m_nTimer;
	int					m_nConnIndex;
    DWORD               m_dwClientIP;
	char				m_szAccount[_NAME_LEN];
	tagExtPointInfo		m_ExtPointInfo;         // 扩展点
    BOOL                m_bExtPointLock;        // 扩展点操作锁
    int                 m_nLastExtPointIndex;   // 上一次操作扩展点索引
    int                 m_nLastExtPointValue;   // 上一次操作扩展点值
    time_t              m_nEndTimeOfFee;        // 收费总截止时间
    time_t              m_nNoFeeTime;           // 发现玩家余额不足的时间
    int                 m_nLastClientFrame;
    int                 m_nNextSaveFrame;
	time_t				m_nLastSaveTime; 
    
    time_t              m_nLastLoginTime;
    time_t              m_nCurrentLoginTime;
    time_t              m_nTotalGameTime; // 总在线秒数
    time_t              m_nCreateTime;
    time_t              m_nLastHeroExpDecreasedTime;
    int                 m_nCreateHeroTemplateID;

    int                 m_nLevel;
    int                 m_nExp;
    DWORD               m_dwClubID;
    DWORD               m_dwLastClubID;
    int                 m_nClubPost;

	KCustomData<PLAYER_CUSTOM_DATA_SIZE>    m_CustomData;

    KENUM_CONNECT_TYPE  m_eConnectType;
    GAME_STATUS         m_eGameStatus;

    DWORD               m_dwMapID;
    int                 m_nCopyIndex;

    char                m_szName[_NAME_LEN];

    KHeroDataList       m_HeroDataList;
    KItemList           m_ItemList;
    KGENDER             m_eGender;
    BOOL                m_bIsRenaming;

    KHero*              m_pFightingHero;
    KHero*              m_pAITeammate;

    KMoneyMgr           m_MoneyMgr;
    KUserPreferences	m_UserPreferences;	//  玩家界面数据存盘
    KCDTimerList        m_CDTimerList;
    KQuestList          m_QuestList;
    KMissionData        m_MissionData;

    DWORD               m_dwMissionAward[KAWARD_CARD_TYPE_TOTAL];
    DWORD               m_dwMissionAwardCount[KAWARD_CARD_TYPE_TOTAL];
    BOOL                m_bMissionAwardChooseIndex[KAWARD_CARD_TYPE_TOTAL][cdVipCardCount];
    BOOL                m_bLuckDrawing;
    int                 m_nGoldCostMoney;

    BOOL                m_bCanRename;
    KHeroTrainingMgr    m_HeroTrainingMgr;

    uint32_t            m_dwMainHeroTemplateID;     // 主英雄
    uint32_t            m_dwAssistHeroTemplateID;   // 副英雄

    int                 m_nTalkWorldDailyCount;
    BOOL                m_bInPVPRoom;
    BOOL                m_bIsMatching;
    int                 m_nHighestHeroLevel;
    int                 m_nHighestLadderLevel;
    int                 m_nLadderLosingStreakCount;

    int                 m_nCoin; // paysys 返回的金币数,会在角色加载完后设置到moneymgr中
	int                 m_nIBActionTime;         //  IB操作标识
	DWORD               m_dwLimitPlayTimeFlag;    //  防沉迷
    DWORD               m_dwLimitOnlineSecond;
    DWORD               m_dwLimitOfflineSecond;
    BOOL                m_bIsLimited;             // 是否处于防沉迷状态
    BOOL                m_bVIPAutoRepair;         // VIP自动修理标志
    int                 m_nVIPLevel;
    int                 m_nVIPExp;
    int                 m_nVIPEndTime;              // vip过期时间
    int                 m_nAILevelCoe;              // 修正玩家在关卡中遇到的AI的难度等级的系数
    int                 m_nCurFatiguePoint;          // 当前疲劳值
    int                 m_nMaxFatiguePoint;          // 最大疲劳值
    int                 m_nTodayBuyFatiguePointTimes;   // 今日购买疲劳值次数
    time_t              m_nForbidTalkTime;                 // 禁言时间
    int                 m_nLastDailyRefreshTime;        // 每天七点刷新的时间控制
    int                 m_nLastDailyRefreshTimeZero;    // 每天零点刷新的时间控制
    int                 m_nClubApplyNum;

    int                 m_nTeamLogoFrameID;         // 队徽边框ID
    int                 m_nTeamLogoEmblemID;        // 队徽徽饰ID
    std::vector<WORD>   m_vecOwnTeamLogo;		
	//OwnTeamLogo存储id第一位是新logo的标识

    KMakingMachine      m_MakingMachine;
    KGym                m_Gym;
    KUpgradeQueue       m_UpgradeQueue;
    KBusinessStreet     m_BusinessStreet;
    KSecretary          m_Secretary;
	KWardrobe           m_Wardrobe;
    KCheerleadingMgr    m_CheerleadingMgr;
    static BYTE         s_byTempData[MAX_EXTERNAL_PACKAGE_SIZE];
    KPlayerValue        m_PlayerValue;
    KAchievement        m_Achievement;
    KActivePlayer       m_ActivePlayer;
	KHireHero			m_HireHero;

    KMAP_DAILYRESET_USERDATA m_mapDailyResetUserData;

    int                 m_nCurrentCity;
    int                 m_nLastChallengeStep1; // 第一座城连续挑战赛的阶段,初始值为1表示从第一关开始
    int                 m_nStartChallengeStep1;

    int                 m_nLastChallengeStep2; // 第二座城连续挑战赛的阶段,初始值为1表示从第一关开始
    int                 m_nStartChallengeStep2;

    BOOL                m_bUseAssistHero;
    int                 m_nFailedChallengeStep;
    BOOL                m_bNeedCheckCanEnter;        // 是否需要检查进入关卡条件 
    std::vector<KCHALLENGEMISSION_AWARD> m_vecChallengeAwardItem;

    int                 m_nOnlineGetAwardTimes;
    int                 m_nReportOnlineGetAward;
    int                 m_nOnlineAwardTime;
    int                 m_nQuitClubTime;

    int                 m_nRandomQuestIndex;
    int                 m_nSkipRandomQuestTime;
    BOOL                m_bFirstPvP;
    BOOL                m_bRefuseStranger;

    int                 m_nLastBattleType;
	char				m_szLastMissionName[_NAME_LEN];
    int                 m_nLastMissionLevel;

    KBuffTable          m_BuffTable;
    
    BYTE* m_pbyPresentGoodInfo;

public:
    BOOL    LoadExtRoleData(BYTE* pbyData, size_t uDataLen);
    BOOL    LoadFromProtoBuf(BYTE* pbyData, T3DB::KPB_SAVE_DATA* pLoadBuf, size_t uDataLen);
    BOOL    LoadExtRoleDataFromProtoBuf(BYTE* pbyData, T3DB::KPB_SAVE_DATA* pLoadBuf, size_t uDataLen);
    BOOL    LoadStateInfoFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL    LoadTeamLogoInfoFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL    LoadCustomDataFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL    LoadPlayerBuffFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);

    BOOL    SaveToProtoBuf(size_t* puUsedSize, T3DB::KPB_SAVE_DATA* pSaveBuf, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL    SaveStateInfoToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);
    BOOL    SaveTeamLogoInfoToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);
    BOOL    SaveCustomDataToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);
    BOOL    SavePlayerBuffFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);

    // 调用这个函数之前应该确保m_SavePosition被正确的设置(比如通过SavePosition)
    BOOL    SaveBaseInfo(KRoleBaseInfo* pBaseInfo);
    BOOL    LoadBaseInfo(KRoleBaseInfo* pBaseInfo);

    void    Logout();
    int     GetShootBallRandomNum() const;
    BOOL    SwitchMap(DWORD dwMapID, int nCopyIndex);
    BOOL    RemoveHero();
    void    ReturnToHall(); // 通知gc玩家返回大厅,gc场景人数减一
    BOOL    CallLoginScript();

    BOOL    CanBuyHero(DWORD dwTemplateID);
    BOOL    CheckUnLockHero(DWORD dwTemplateID);
    BOOL    BuyHero(DWORD dwTemplateID);
	BOOL    DoBuyHero(DWORD dwTemplateID);


    BOOL    CanFireHero(DWORD dwTemplateID);
    BOOL    FireHero(DWORD dwTemplateID);
    
    BOOL    CanBuyHeroSlot();
    BOOL    BuyHeroSlot();

    BOOL    AddNewHero(DWORD dwTemplateID, int nLevel);
    BOOL    DelHero(DWORD dwTemplateID);
    void    UpdateHeroData(KHero* pHero);

    BOOL    ApplyPlayerBuff(KHero* pHero);
    void    UnApplyPlayerBuff(KHero* pHero);

    BOOL    ApplyHeroEquip(KHero* pHero);
    void    UnApplyHeroEquip(KHero* pHero);

    BOOL    ApplyHeroTalent(KHero* pHero);
    void    UnApplyHeroTalent(KHero* pHero);

    BOOL    ApplyHeroFashionAttr(KHero* pHero);
    void    UnApplyHeroFashionAttr(KHero* pHero);

    BOOL    SafeAddItem(DWORD dwTabType, DWORD dwIndex, int nStackNum, int nValuePoint, 
        const char cszTitle[], const char cszSender[], const char cszTxt[], DWORD dwRandSeed = 0
    );

    IItem*  AddItem(DWORD dwTabType, DWORD dwIndex, int nStackNum, int nValuePoint, DWORD dwRandSeed = 0);
    BOOL    AddItemByMail(DWORD dwTabType, DWORD dwIndex, int nStackNum, int nValuePoint, 
        const char cszTitle[], const char cszSender[], const char cszText[], DWORD dwRandSeed = 0
    );

    BOOL    AddItemByMail(IItem* piItem, const char cszTitle[], const char cszSender[], const char cszText[]);
    BOOL	CanAddTeamLogo(WORD wTeamLogoID);
	BOOL    AddTeamLogo(WORD wTeamLogoID);
    BOOL	SetTeamLogo(WORD wFrameID, WORD wEmblemID);
	BOOL    ChangeFashion(uint32_t dwHeroTemplateID, uint32_t dwTargetFashionID);
    BOOL    CanAddItem(DWORD dwTabType, DWORD dwIndex, int nCount);
    BOOL    HasFreePosAddItem(DWORD dwTabType, DWORD dwIndex);
    BOOL    AbradeEquip(KHero* pHero);
    BOOL    RepairEquip(int nPackageType, int nPackageIndex, int nPos);

    BOOL    BuyGoods(int nShopID, int nGoodsID, int nCount);
    BOOL    SellGoods(int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime, WORD wCount);

    BOOL    HasHero(DWORD dwHeroTemplateID) const;

    BOOL    CostGroupTalkItem();
    BOOL    CostGlobalTalkItem();
    BOOL    CheckTalkDailyCount(unsigned char cTalkType);

    void 	OnEvent(int nEvent, int nParam1=0, int nParam2=0, int nParam3=0, int nParam4=0, int nParam5=0, int nParam6=0);
    BOOL    SelectMainHero(DWORD dwHeroTemplateID);
    BOOL    SelectAssistHero(DWORD dwHeroTemplateID);
    BOOL 	RefreshDailyVariable();
    int     GetMainHeroLevel();
    int     GetAssistHeroLevel();
    int     GetMainHeroEquipTotalValuePoint();

    BOOL    CreateTeam();
    BOOL    LeaveTeam();
    BOOL    TeamInvitePlayer(const char cszTargetName[]);
    BOOL    TeamAcceptOrRefuseInvite(int nAcceptCode, const char (&szInviterName)[_NAME_LEN]);
    BOOL    TeamKickoutPlayer(uint32_t dwTargetID);
    BOOL    AutoMatch();
    BOOL    CancelAutoMatch();
    BOOL    TryJoinRoom(DWORD dwRoomID);
    BOOL    AcceptOrRefuseJoinRoom(int nAcceptCode, DWORD dwRoomID);

    void    UpdateLevel(int nNewLevel);
    void    DoHeroExpWeeklyDecrease();

    void    UpdateLadderLevel();

    void    OnMissionSuccess(KAwardData* pAwardData, BOOL(&bHasAward)[KAWARD_CARD_TYPE_TOTAL]);
    BOOL    UpdateVIPAwardCount();
    BOOL    IsVIP();

    BOOL    EnterMission(int eType, int nStep, int nLevel);

    BOOL    SetExtPoint(int nIndex, int nChangeValue);
    BOOL    GetExtPoint(int nIndex, int nBitIndex, int nBitLength, int& nGetValue);

    BOOL    CanStartContinuousChallenge();
    BOOL    CostContinuousChallengeItem();
    void    ProcessChallengeAward(
        KCHALLENGE_AWARD_TYPE eAwardType, KPlayer* pOtherPlayer, BOOL bUseAssistHero, int nStartStep, int nFinishStep
    );

    BOOL    GetTeamLogoSyncInfo(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize);
    int     GetNextOnlineAwardTime();

    void    ReportLevelUpAward();
    void    ReportOnlineAward();
    void    ReportBuffEnd();

    int     GetCurrentChallengeType();
    int     GetLastChallengeStep();
    int     GetStartChallengeStep();
    void    SetLastChallengeStep(int nLastChallengeType);
    void    SetStartChallengeStep(int nStartChallengeType);
    void    SetCurrentCityByMissionType(int nType);

private:
    void SetMainHero(uint32_t dwMainHeroTemplateID);
    void CheckLimited();
    void CheckItemValidity();
    void CheckVIP();
	BOOL CheckTeamLogoID(WORD wID, ETEAMLOGO_INFO_TYPE eType);

public:
    int LuaTestReport(Lua_State* L);
    int LuaAddNewHero(Lua_State* L);
    int LuaDelHero(Lua_State* L);
    int LuaAddItem(Lua_State* L);
    int LuaDestroyItem(Lua_State* L);
    int LuaCostItem(Lua_State* L);
    int LuaExchangeItem(Lua_State* L);
    int LuaAddMoney(Lua_State* L);
    int LuaGetFightingHero(Lua_State* L);
    int LuaRename(Lua_State* L);
    int LuaSwitchMap(Lua_State* L);
    int LuaSendSystemMail(Lua_State* L);
    int LuaSendMailToSelf(Lua_State* L);
    int LuaSendMail(Lua_State* L);
    int LuaAddMaxHeroCount(Lua_State* L);
    int LuaSelectMainHero(Lua_State* L);
    int LuaSelectAssistHero(Lua_State* L);
    int LuaEnterMission(Lua_State* L);
    int LuaLeaveMission(Lua_State* L);
    int LuaOpenMission(Lua_State* L);
    int LuaFinishMission(Lua_State* L);
    int LuaAcceptQuest(Lua_State* L);
    int LuaCancelQuest(Lua_State* L);
    int LuaClearQuest(Lua_State* L);
    int LuaSetQuestValue(Lua_State* L);
    int LuaFinishQuest(Lua_State* L);
    int LuaGetQuestPhase(Lua_State* L);
    int LuaCreateTeam(Lua_State* L);
    int LuaLeaveTeam(Lua_State* L);
    int LuaTeamInvitePlayer(Lua_State* L);
    int LuaTeamKickoutPlayer(Lua_State* L);
    int LuaAutoMatch(Lua_State* L);
    int LuaCancelAutoMatch(Lua_State* L);
    int LuaFireEvent(Lua_State* L);
    int LuaAddHeroExp(Lua_State* L);
    int LuaGetExtPoint(Lua_State* L);
    int LuaSetExtPoint(Lua_State* L);
    int LuaGetExtPointByBits(Lua_State* L);
    int LuaSetExtPointByBits(Lua_State* L);
    int LuaGetHeroLevel(Lua_State* L);
    int LuaGetAITeammate(Lua_State* L);
    int LuaCreateFreePVPRoom(Lua_State* L);
    int LuaProduceItem(Lua_State* L);
    int LuaLearnRecipe(Lua_State* L);
    int LuaRepairEquip(Lua_State* L);
    int LuaAbradeHeroEquip(Lua_State* L);
    int LuaDownwardNotify(Lua_State* L);
    int LuaClearBuildCD(Lua_State* L);
    int LuaSetFatiguePoint(Lua_State* L);
    int LuaAddTeamExp(Lua_State* L);
    int LuaBuyGoods(Lua_State* L);
    int LuaAddTeamLogo(Lua_State* L);
	int LuaSetTeamLogo(Lua_State* L);
    int LuaSwitchToExistScene(Lua_State* L);
    int LuaBuyFatiguePoint(Lua_State* L);
    int LuaSetFatigueValue(Lua_State* L);
    int LuaAddVIPExp(Lua_State* L);
    int LuaChargeVIPTime(Lua_State* L);
    int LuaBuyCheerleadingSlot(Lua_State* L);
    int LuaAddCheerleadingSlot(Lua_State* L);
    int LuaGetCheerleadingSlotCount(Lua_State* L);
    int LuaAddCheerleadingItem(Lua_State* L);
    int LuaResetFatiguePoint(Lua_State* L);
    int LuaGetMailList(Lua_State* L);
    int LuaSeeMail(Lua_State* L);
    int LuaGetMailAll(Lua_State* L);
    int LuaSetMailRead(Lua_State* L);
    int LuaDelMail(Lua_State* L);
    int LuaMailItem(Lua_State* L);
    int LuaCreateClub(Lua_State* L);
    int LuaMemberList(Lua_State* L);
    int LuaApply(Lua_State* L);
    int LuaAccept(Lua_State* L);
    int LuaInvite(Lua_State* L);
    int LuaSetPost(Lua_State* L);
    int LuaSetDescript(Lua_State* L);
    int LuaDelMember(Lua_State* L);
    int LuaRandomClub(Lua_State* L);
    int LuaAddActive(Lua_State* L);
    int LuaGetPlayerValue(Lua_State* L);
    int LuaSetPlayerValue(Lua_State* L);
    int LuaDoCustomConsumeRequest(Lua_State* L);
    int LuaGetDailyResetUserData(Lua_State* L);
    int LuaSetDailyResetUserData(Lua_State* L);
    int LuaAwardRandomOne(Lua_State* L);
    int LuaAwardAll(Lua_State* L);
    int LuaClientCall(Lua_State* L);
    int LuaWinGame(Lua_State* L);
    int LuaLoseGame(Lua_State* L);
    int LuaAddFreeCoin(Lua_State* L);
    int LuaIsAchievementFinished(Lua_State* L);
    int LuaSetAchievementValue(Lua_State* L);
    int LuaGetAchievementValue(Lua_State* L);
    int LuaResetAchievement(Lua_State* L);
    int LuaDailySignAward(Lua_State* L);
    int LuaEnterChallenge(Lua_State* L);
    int LuaAddLadderExp(Lua_State* L);
    int LuaPlayerLog(Lua_State* L);
	int LuaIsCheerleadingEmpty(Lua_State* L);
	int LuaGetHeroFashionID(Lua_State* L);
    int LuaTestFlowRecord(Lua_State* L);
    int LuaAddPlayerBuff(Lua_State* L);
    int LuaAddBuffTime(Lua_State* L);
    int LuaHasHero(Lua_State* L);
    int LuaS2CRemoteCall(Lua_State* L);
    
    int LuaUpgradeSafeBox(Lua_State* L);
    int LuaUpgradeEquip(Lua_State* L);
    int LuaUpgradeTrainTeacher(Lua_State* L);
    int LuaHeroUseEquip(Lua_State* L);
    int LuaHeroTrain(Lua_State* L);
    int LuaRecommendFriend(Lua_State* L);
    int LuaDecomposeEquip(Lua_State* L);
    int LuaUpgradeHeroLevel(Lua_State* L);
    int LuaSecretaryReport(Lua_State* L);
    int LuaCanAddItem(Lua_State* L);
    int LuaGetFreeRoomSize(Lua_State* L);
	int LuaGetHeroMaxLevel(Lua_State* L);
    int LuaGetStoreCount(Lua_State* L);

public:
    DECLARE_LUA_CLASS(KPlayer);
    DECLARE_LUA_STRING(Account, sizeof(m_szAccount));
    DECLARE_LUA_STRING(Name, sizeof(m_szName));
    DECLARE_LUA_TIME(CreateTime);
    DECLARE_LUA_TIME(LastLoginTime);
    DECLARE_LUA_TIME(LastSaveTime);
    DECLARE_LUA_DWORD(MainHeroTemplateID);
    DECLARE_LUA_DWORD(AssistHeroTemplateID);
    DECLARE_LUA_INTEGER(AILevelCoe);
    DECLARE_LUA_INTEGER(Level);
    DECLARE_LUA_DWORD(ClubID);
};
