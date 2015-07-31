// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KPlayerServer.h 
//	Creator 	: Xiayong  
//  Date		: 08/09/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "Common/KG_Socket.h"
#include "Common/KG_Package.h"
#include <map>
#include <vector>
#include "KConnectionData.h"
#include "Protocol/cl2gs_protocol.h"
#include "Protocol/gs2cl_protocol.h"
#include "KBasketSocket.h"
#include "KFellowship.h"
#include "KMailDef.h"
#include "KRoleDBDataDef.h"
#include "KRPC.h"

class KSceneObject;
struct IKG_Buffer;
struct IItem;
class KScene;
class KDoodad;
class KBackboard;
class KClip;
class KHero;
class KMovableObject;
class KSkill;
class KBullet;
class KHeroPackage;
struct KHeroData;
struct KLADDER_AWARD_DATA;
struct KMISSION_AWARD_DATA;
struct KGFellowship;
struct KREPORT_ITEM;
struct KAWARD_ITEM;
namespace T3DB
{
    class KPB_MISSION_DATA;
}

class KPlayerServer
{
public:
	KPlayerServer(void);
	~KPlayerServer(void);

	BOOL Init();
	BOOL UnInit(void);

    void Activate();
	void ProcessNetwork(void);

    BOOL Attach(KPlayer* pPlayer, int nConnIndex);
    BOOL Detach(int nConnIndex);

	BOOL Send(int nConnIndex, void* pvData, size_t uDataLen);
	void FlushSend();

    KConnectionData* GetConnection(int nConnIndex)
    {
        KConnectionData* pConnection = NULL;
        if (nConnIndex >= 0 && nConnIndex < m_nMaxConnection)
        {
            pConnection = &m_ConnectionDataList[nConnIndex];
        }
        return pConnection;
    }

    int   GetConnectionCount()
    {
        return m_nMaxConnection - (int)m_ConnectionDataListFreeVector.size();
    }

    void ResetPakStat();
    BOOL DumpPakStat();
    
public:
	int					m_nListenPort;		
	DWORD				m_dwInternetAddr;
    char                m_szLocalIP[16];
    float               m_fS2CTraffic;
    float               m_fC2STraffic;

    BOOL                m_bCloseFlag[client_gs_connection_end];

private:
    int                 m_nS2CTraffic;
    int                 m_nC2STraffic;
    int                 m_nPingCycle;
	KConnectionData*	m_ConnectionDataList;
	int					m_nMaxConnection;
    int                 m_nMaxClientPackPerFrame;
    int                 m_nClientGM;
    int                 m_nNextCheckConnection;
    DWORD               m_dwTimeNow;

    KG_SocketServerAcceptor     m_SocketServerAcceptor;
    int                         m_nSocketEventCount;                            
    KG_SOCKET_EVENT*            m_pSocketEventArray;

    typedef vector<unsigned>                KG_CONNECTION_DATA_LIST_FREE_VECTOR;
    KG_CONNECTION_DATA_LIST_FREE_VECTOR     m_ConnectionDataListFreeVector;

	typedef void (KPlayerServer::*PROCESS_PROTOCOL_FUNC)(char*, size_t, int, int);
	PROCESS_PROTOCOL_FUNC	m_ProcessProtocolFuns[client_gs_connection_end]; 

	int m_nProtocolSize[client_gs_connection_end];

    struct KPROTOCOL_STAT_INFO
    {
        DWORD    dwPackCount;
        uint64_t uTotalSize;
    };
    KPROTOCOL_STAT_INFO m_S2CPakStat[gs_client_connection_end];
    KPROTOCOL_STAT_INFO m_C2SPakStat[client_gs_connection_end];
    BYTE                m_byTempData[MAX_EXTERNAL_PACKAGE_SIZE];

    int _Construct();
    int _Destroy();

    inline KPlayer* GetPlayerByConnection(int nConnIndex)
    {
        if (nConnIndex != -1)
        {
            assert(nConnIndex >= 0 && nConnIndex < m_nMaxConnection);
            return m_ConnectionDataList[nConnIndex].pPlayer;
        }
        return NULL;
    }

	BOOL Shutdown(int nConnIndex);

    BOOL ProcessNewConnection(IKG_SocketStream* piSocket);
    BOOL ProcessPackage(IKG_SocketStream* piSocket);
	BOOL CheckPackage(BYTE* pbyData, size_t uDataLen);
    void InitProcotolProcess();

    KHero* GetFightingHeroByConnection(int nConnIndex);

public:
	// 返回应答信息
    BOOL DoSyncPlayerBaseInfo(KPlayer* pPlayer);
    BOOL DoSyncPlayerStateInfo(KPlayer* pPlayer);
    BOOL DoSyncPlayerHeroInfo(KPlayer* pPlayer);
	// 通知客户端自动断线踢号
	BOOL DoAccountKickout(int nConnIndex);
    BOOL DoPlayerEnterHallNotify(int nConnIndex);
	// 通知客户端切换服务器
	BOOL DoSwitchGS(int nConnIndex, DWORD dwPlayerID, GUID& Guid, DWORD dwIPAddr, WORD wPort);
	// 通知客户端换地图
    BOOL DoSwitchMap(KPlayer* pPlayer, DWORD dwMapID, int nCopyIndex);

	BOOL DoLimitPlayInfoResponse(KPlayer* pPlayer, BYTE byResult);

    // ---------------- Player, Doodad 的出现/消失同步 --------------------------
    // 同步hero到单个客户端玩家
    BOOL DoSyncNewHero(KHero* pHero, int nConnIndex);

    // 广播hero
    BOOL DoBroadcastNewHero(KHero* pHero, DWORD dwHeroExceptID);

    BOOL DoSyncSceneObject(KSceneObject* pObj, int nConnIndex = -1);

    BOOL DoSyncCurrentEndurance(KHero* pHero);
    BOOL DoSyncCurrentStamina(KHero* pHero);
    BOOL DoSyncCurrentAngry(KHero* pHero);
    BOOL DoSyncCurrentMoneys(KPlayer* pPlayer);
    BOOL DoSyncMoneyChanged(int nConnIndex, ENUM_MONEY_TYPE eMoneyType, int nAmount);

	// 通知客户端某个角色被移除了
    BOOL DoBroadcastHeroRemove(KScene* pScene, DWORD dwHeroID, BOOL bKilled);
    
    // ----------------- 移动同步 --------------------------------------------
    BOOL DoSyncRunMode(KHero* pHero);
    BOOL DoSyncMoveState(KHero* pHero, int nVirtualFrame, BOOL bSyncSelf);
    BOOL DoAdjustHeroMove(KHero* pHero, int nReason);

    BOOL DoFrameSignal(int nConnIndex);
    BOOL DoSyncFaceDir(KHero* pHero, unsigned char direction);
    BOOL DoTakeBall(KHero* pHero, BOOL bNotifySelf);
    BOOL DoUnTakeBall(KHero* pHero);
    BOOL DoShootBasket(KHero* pHero, DWORD dwBasketID, KBasketSocket* pBasketSocket);
    BOOL BasketDrop(KBasket* pBasket);
    BOOL DoPassBallTo(KHero* ballTaker, KHero* teammate);
    BOOL DoAimAt(KHero* pHero, DWORD dwHoldingObjID, BOOL bSyncSelf);
    BOOL DoTakeObject(KHero* pHero, KDoodad* pObject);
    BOOL DoUseDoodad(KHero* pHero, KDoodad* pObject, unsigned char direction);
    BOOL DoDropObject(KHero* pHero, KDoodad* pObject);
    BOOL DoDetachObject(KHero* pHero, KDoodad* pObject);
    BOOL DoAttachObject(KHero* pHero, KDoodad* pObject);

    BOOL DoSyncScore(KScene* pScene, DWORD dwTeamID, DWORD dwScore);
    BOOL DoSyncScoreToOneHero(int nConnIndex, DWORD dwTeamID, DWORD dwScore);

    BOOL DoHeroSkill(KHero* pCaster, DWORD dwSkillID, DWORD dwStepID, BOOL bSyncSelf);
    BOOL DoHeroGrabSkill(KHero* pCaster, DWORD dwSkillID, DWORD dwStepID, DWORD dwTargetHeroID);
    BOOL DoSkillResult(KHero* pCaster, DWORD dwTargetID, KSkill* pSkill);

    BOOL DoHeroStandOn(KHero* pHero, KDoodad* pObject);

    BOOL DoBattleStart(KScene* pScene);
    BOOL DoMissionBattleFinished(int nConnIndex, int nSelfSide, int (&rTotalScores)[sidTotal], const KMISSION_AWARD_DATA(& rAwardData)[MAX_TEAM_MEMBER_COUNT], KHero*(& rTeammates)[MAX_TEAM_MEMBER_COUNT], BOOL (&bHasAward)[3], int nGoldCostMoney);
    BOOL DoFreePVPBattleFinished(int nConnIndex, int nSelfSide, int (&rTotalScores)[sidTotal], KHero* (& rTeammates)[MAX_TEAM_MEMBER_COUNT]);
    BOOL DoLadderBattleFinished(int nConnIndex, int nSelfSide, int (&rTotalScores)[sidTotal], const KLADDER_AWARD_DATA(& rAwardData)[MAX_TEAM_MEMBER_COUNT], KHero* (& rTeammates)[MAX_TEAM_MEMBER_COUNT], BOOL (&bHasAward)[3], int nGoldCostMoney);
    BOOL DoSyncStartedFrame(int nStartedFrame, int nTotalFrame, int nLeftFrame, int nConnIndex, KScene* pScene);

    BOOL DoShootObject(KHero* pHero, KDoodad* pObject, DWORD dwSocketID, BOOL bCanHit);

    BOOL DoObjStandObj(KDoodad* pSource, KDoodad* pDest, int nConnIndex = -1);

    // 房间机制
    BOOL DoSyncCreateRoomRespond(int nConnIndex, DWORD dwRoomID, DWORD dwMapID, BOOL bSuccess);
    BOOL DoSyncRoomInfoStart(int nConnIndex, int nPageIndex);
    BOOL DoSyncRoomBaseInfo(int nConnIndex, int nPageIndex, DWORD dwRoomID, const char szRoomName[], 
        DWORD dwMapID, int nMemberCount, bool bHasPassword, bool bFighting);
    BOOL DoSyncRoomInfoEnd(int nConnIndex, unsigned uTotalPage);
    BOOL DoSyncOneRoomMemberPosInfo(int nConnIndex, DWORD dwPlayerID, KSIDE_TYPE eSide, int nPos);
    BOOL DoApplyJoinRoomRespond(int nConnIndex, BOOL bSuccess, DWORD dwRoomID, int nTid, int nPosition,  KS2C_ROOM_MEMBER_INFO& rNewMember);
    BOOL DoApplyLeaveRoomRespond(int nConnIndex, DWORD dwRoomID, DWORD dwLeaverID, int nSide, int nPos);
    BOOL DoApplySwitchRoomHostRespond(int nConnIndex, BOOL bSuccess, DWORD dwRoomID, DWORD dwNewHostID);
    BOOL DoApplySwitchBattleMapRespond(int nConnIndex, BOOL bSuccess, DWORD dwNewMapID);
    BOOL DoApplySetRoomPassword(int nConnIndex, BOOL bSuccess, const char(& cszPassword)[cdRoomPasswordMaxLen]);
    BOOL DoApplySetReadyRespond(int nConnIndex, DWORD dwRoomID, DWORD dwReadyPlayerID, BOOL bReady);
    BOOL DoSyncRoomName(int nConnIndex, DWORD dwRoomID, const char szName[]);
    
    BOOL DoAutoMatchRespond(int nConnIndex, BYTE eRetCode, uint32_t dwPunishingMemberID, int nLeftSeconds);
    BOOL DoEnterAutoMatchNotify(int nConnIndex);
    BOOL DoLeaveAutoMatchNotify(int nConnIndex, BYTE byLeaveReason);
  
    BOOL DoLandMineBlowupBoardcast(KScene* pScene, DWORD dwLandMineID);

    BOOL DoSyncPlayerListStart(int nConnIndex);
    BOOL DoSyncPlayerInfo(int nConnIndex, KPlayer* pPlayer);
    BOOL DoSyncPlayerListEnd(int nConnIndex, int nPageIndex);
    BOOL DoDoodadStuckHero(KDoodad* pDoodad, KHero* pHero);
    BOOL DoDoodadRelaseHero(KScene* pScene, DWORD dwDoodadID, DWORD dwHeroID);
    BOOL DoUnTakeDoodad(DWORD dwTakerID, KMovableObject* pObj);
    BOOL DoRemoveGuide(KHero* pHero);
    BOOL DoSendToClient(int nConnIndex, BYTE* pbyData, size_t uDataLen);
    BOOL DoSendToPlayer(DWORD dwPlayerID, BYTE* pbyData, size_t uDataLen);
    BOOL DoRequestFailed(int nConnIndex, int nRequestID, DWORD dwErrorCode);

    BOOL DoAwardItemNotify(KPlayer* pPlayer, KAWARD_CARD_TYPE eType, KS2C_Award_Item awardList, unsigned uAwardedIndex);
    BOOL DoAwardItemListNotify(KPlayer* pPlayer, KAWARD_CARD_TYPE eType, KS2C_Award_Item arrAwardList[], unsigned uCount);
    BOOL DoAwardHeroExp(KPlayer* pPlayer, DWORD dwHeroTemplateID, int nExp);
    BOOL DoAwardMoney(KPlayer* pPlayer, ENUM_MONEY_TYPE eType, int nCount);
    BOOL DoBuyGoodsRespond(KPlayer* pPlayer, int nShopID, int nGoodsID, BOOL bSuccess);
    BOOL DoSellGoodsRespond(KPlayer* pPlayer, int nPackageType, int nPackageIndex, int nPos, DWORD dwTabType, DWORD dwTabIndex, int nGenTime, BOOL bSuccess);
    BOOL DoRepairSingleEquipRespond(KPlayer* pPlayer, BOOL bSuccess);
    BOOL DoRepairAllEquipRespond(KPlayer* pPlayer, BOOL bSuccess);
    BOOL DoIBActionResultNotify(KPlayer* pPlayer, BOOL bSuccessed);

    BOOL DoHeroLevelUp(int nConnIndex, DWORD dwHeroTemplateID, int nLevel);
    BOOL DoSyncHeroExp(int nConnIndex, DWORD dwHeroTemplateID, int nExp);

    BOOL DoSyncPlayerLevelExp(KPlayer* pPlayer, BOOL bLevelUp);

    // 聊天消息
    BOOL DoTalkMessage( int nMsgType, DWORD dwTalkerID, const char cszTalker[], DWORD dwTalkHeroID, int nTalkGroupID, DWORD dwReceiverID, const char cszReceiver[], size_t uDataLen, const char cszTalkData[] );

    BOOL DoSyncOneFellowship(DWORD dwPlayerID, DWORD dwAlliedPlayerID);
    BOOL DoSyncOneBlackList(DWORD dwPlayerID, DWORD dwAlliedPlayerID);
    BOOL DoSyncFellowshipList(DWORD dwPlayerID, int nCount, KFellowShipData* pData);
    BOOL DoSyncBlackList(DWORD dwPlayerID, int nCount, KBlackListData* pData);
    BOOL DoSyncFellowshipGroupName(DWORD dwPlayerID);
    BOOL DoDelFellowshipRespond(int nConnIndex, DWORD dwAlliedPlayerID, int nType, BOOL bSuccess);
    BOOL DoUpdateFellowshipInfo(int nConnIndex, DWORD dwAlliedPlayerID, KGFellowship* pFellowship);
    BOOL DoFirstPvPNotify(int nConnIndex, DWORD dwPlayerID, const char cszPlayerName[]);

    BOOL DoSyncItemData(int nConnIndex, DWORD dwPlayerID, IItem* piItem, int nPackageType, int nPackageIndex, int nPos);
    BOOL DoSyncQuestData(int nConnIndex, DWORD dwDestPlayerID, const BYTE* pbyData, size_t uDataLen, BYTE byDataType);
    BOOL DoSyncMaxHeroCount(int nConnIndex, int nMaxHeroCount);
    BOOL DoSyncHeroData(int nConnIndex, KHeroData* pHeroData);
    BOOL DoFireHeroRespond(int nConnIndex, DWORD dwHeroTemplateID);
    BOOL DoUnFireHeroRespond(int nConnIndex, DWORD dwHeroTemplateID);
    BOOL DoBuyHeroRespond(int nConnIndex, DWORD dwHeroTemplateID);
    BOOL DoSyncUserPreferences(int nConnIndex, int nOffset, int nLength, BYTE* pbyData);
    BOOL DoSyncPlayerPackageSize(int nConnIndex, int nPackageIndex, int nPackageSize);
    BOOL DoRenameRespond(int nConnIndex, BYTE byCode, DWORD dwPlayerID, char szNewName[]);
    BOOL DoSelectSkillRespond(int nConnIndex, BOOL bSuccess);

    // -------------------- 邮件系统 ----------------------------------->
    BOOL DoSendMailRespond(int nConnIndex, int nRespondID, int nResult);
    BOOL DoGetMailListRespond(int nConnIndex, KMailListInfo* pMailListInfo, int nMailCount, int nMailTotalCount);
    BOOL DoNewMailNotify(int nConnIndex, KMailListInfo* pMailListInfo);
    BOOL DoSyncMailContent(DWORD dwPlayerID, DWORD dwMailID, int nResult, KMailContent* pContent);
    BOOL DoMailGeneralRespond(DWORD dwPlayerID, DWORD dwMailID, int nResult);
    // <------------------- 邮件系统 ------------------------------------

    //--------------------------------------俱乐部------------------------>
    BOOL DoLeaveClubNotify(int nConnIndex);
    BOOL DoPostModify(int nConnIndex, DWORD dwPlayerID, int nPost);
    BOOL DoClubBaseInfo(int nConnIndex, int nPost, BOOL bSwitchApply, DWORD dwClubID, int nInfoType,
        const char cszClubName[], const char cszLeaderName[], int nCreateTime, int nMemberCount, 
        int nMaxMemberCount, size_t uNoticeLen, size_t uOnlineLen, size_t uIntroLen, BYTE* byData
    );
    BOOL DoApplyJoinClubRepsond(int nConnIndex, DWORD dwClubID, int nError);
    BOOL DoClubInviteNotify(int nConnIndex, DWORD dwClubID, DWORD dwLauncherID, const char cszClubName[], const char cszRoleName[]);

    //<-------------------------------------------------------------------
    BOOL DoSyncTrainingHeroList(KPlayer* pPlayer);
    BOOL DoBeginTrainingHeroRespond(int nConnIndex, DWORD dwHeroTemplateID, DWORD dwTotalTime, DWORD dwTrainingTemplateID, BOOL bSuccess);
    BOOL DoEndTrainingHeroRespond(int nConnIndex, DWORD dwHeroTemplateID, BOOL bSuccess);
    BOOL DoBuyTrainingSeatRespond(int nConnIndex, BOOL bSuccess, unsigned uCurSeatCount);

    BOOL DoDelHeroData(int nConnIndex, DWORD dwTemplateID);

    // <------------------- 背包部分 ------------------------------------>
    BOOL DoUpdateItemAmount(int nConnIndex, int nPackageType, int nPackageIndex, int nPos, int nStack);
    BOOL DoUpdateItemDurability(int nConnIndex, int nPackageType, int nPackageIndex, int nPos, int nDurability);
    BOOL DoDestroyItem(int nConnIndex, int nPackageType, int nPackageIndex, int nPos);
    // <------------------- 背包部分 ------------------------------------

    // 任务相关
    BOOL DoSyncQuestList(KPlayer* pPlayer);
    BOOL DoSetQuestValueRespond(int nConnIndex, int nQuestIndex, int nValueIndex, int nNewValue, QUEST_RESULT_CODE eRetCode);
    BOOL DoAcceptQuestRespond(int nConnIndex, DWORD dwQuestID, int nQuestIndex, QUEST_RESULT_CODE eRetCode);
    BOOL DoFinishQuestRespond(int nConnIndex, QUEST_RESULT_CODE eRetCode, int nQuestIndex);
    BOOL DoCancelQuestRespond(int nConnIndex, QUEST_RESULT_CODE eRetCode, int nQuestIndex);
    BOOL DoSyncQuestValue(int nConnIndex, int nQuestIndex, int nValueIndex, int nCurValue);
    BOOL DoSyncDailyQuestData(KPlayer* pPlayer);
    BOOL DoSyncOneDailyQuestData(int nConnIndex, DWORD dwQuestID, int nNextAcceptTime);
    BOOL DoClearQuest(int nConnIndex, DWORD dwQuestID);

    BOOL DoExchangeItemRespond(
        int nConnIndex, int nCode, int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
        int nDestPackageType, int nDestPackageIndex, int nDestPos, int nExChangeResult
    );

    BOOL DoUseItemRespond(
        int nConnIndex, int nCode, int nPackageType, int nPackageIndex, int nPos
    );

    BOOL DoAddBuffNotify(KHero* pHero, DWORD dwBuffID, int nLeftFrame);
    BOOL DoSetCoolDownNotify(int nConnIndex, DWORD dwTimerID, int nEndFrame, int nInterval);
    BOOL DoRemoveSceneObj(DWORD dwSceneID, DWORD dwObjID);
    BOOL DoSyncMissionData(int nConnIndex, const T3DB::KPB_MISSION_DATA* pMissionData);
    BOOL DoSyncMissionOpened(int nConnIndex, int nType, int nStep, int nLevel);
    BOOL DoSyncMissionFinished(int nConnIndex, int nType, int nStep, int nLevel);
    BOOL DoSyncRandomDailyQuest(int nConnIndex, std::vector<DWORD>& vecQuestID);
    BOOL DoSyncItemBind(int nConnIndex, int nPackageType, int nPackageIndex, int nPos, BOOL bBind);
    BOOL DoEnterMissionRespond(int nConnIndex, int nMissionType, int nMissionStep, int nMissionLevel, BOOL bSucceed);
    BOOL DoLeaveMissionRespond(int nConnIndex, int nMissionType, int nMissionStep, int nMissionLevel);
    BOOL DoBroadcastUseItem(KScene* pScene, DWORD dwUserID, DWORD dwTabType, DWORD dwIndex);
    BOOL DoUpdateMoveParam(KHero* pHero);
    BOOL DoFreePVPInviteNotify(KPlayer* pInvitee, const char(&cszInvitorName)[_NAME_LEN], DWORD dwRoomID, DWORD dwMapID);
    BOOL DoLadderPVPInviteNotify(KPlayer* pInvitee, const char(&cszInvitorName)[_NAME_LEN]);
    BOOL DoPVEInviteNotify(KPlayer* pInvitee, const char(&cszInvitorName)[_NAME_LEN], int nPVEMode, int nMissionType, int nMissionStep, int nMissionLevel);
    BOOL DoLoseBuffNotify(KHero* pHero, DWORD dwBuffID);
    BOOL DoActivityCanAwardNotify(KPlayer* pPlayer, DWORD dwActivityID);
    BOOL DoActivityAwardedNotify(KPlayer* pPlayer, DWORD dwActivityID, KACTIVIY_AWARD_RET eRetCode);
    BOOL DoPlayerDataLoadFinished(int nConnIndex);
    BOOL DoSyncPlayerLimitedFlag(int nConnIndex, BOOL bLimited);
    BOOL DoSyncOneQuestState(int nConnIndex, DWORD dwQuestID, QUEST_STATE eState);
    BOOL DoAwardGift(int nConnIndex, DWORD dwTabType, DWORD dwIndex, int nAmount);
    BOOL DoAddItemNotify(int nConnIndex, DWORD dwTabType, DWORD dwIndex, int nAmount);
    BOOL DoSyncObjEnd(int nConnIndex);

    BOOL DoSyncNewNpc(KHero* pNpc, int nConnIndex=-1);

    BOOL DoNormalShootBall(KHero* pShooter, KBasketSocket* pSocket, DWORD dwInterferenceHeroID);
    BOOL DoSkillShootBall(KHero* pShooter, KBasketSocket* pSocket);
    BOOL DoNormalSlamBall(KHero* pSlamer, KBasketSocket* pSocket);
    BOOL DoSkillSlamBall(KHero* pSlamer, KBasketSocket* pSocket);

    BOOL DoSyncMakingMachineInfo(KPlayer* pPlayer);
    BOOL DoLearnRecipeNotify(KPlayer* pPlayer, DWORD dwRecipeID);
    BOOL DoClearBuildCDRespond(KPlayer* pPlayer, int nCDType, BOOL bSuccess);
    BOOL DoDownwardNotify(KPlayer* pPlayer, DWORD dwMessageID, void* pData = NULL, size_t uDataLen = 0);
    BOOL DoProduceItemRespond(KPlayer* pPlayer, BOOL bSuccess, BOOL bCrit);
    BOOL DoSyncSceneGravity(KScene* pScene);
    BOOL DoSyncSceneObjPosition(KSceneObject* pObj);
    BOOL DoNewBulletNotify(KScene* pScene, KBullet* pBullet);
    BOOL DoStrengthenEquipNotify(int nConnIndex, KPACKAGE_TYPE epackageType, int nPackageIndex, int nPos, BOOL bSuccess, int nStrengthenLevel);
    BOOL DoSyncDamage(KHero* pTarget, BOOL bEnduranceCrited, int nEnduranceDamage, BOOL bStaminaCrited, int nStaminaDamage, int nBeAttackedType = 0); 
    BOOL DoSyncReportItem(KPlayer* pPlayer, KREPORT_ITEM* pReportItem);
    BOOL DoRemoveReportItem(KPlayer* pPlayer, KREPORT_ITEM* pReportItem);
    
    BOOL DoSyncWardrobeInfo(KPlayer* pPlayer);
    BOOL DoSyncFashionInfo(KPlayer* pPlayer, uint32_t dwFashionID, uint32_t dwOverduePoint);
    BOOL DoChangeFashionRespond(KPlayer* pPlayer, BOOL bSuccess, uint32_t dwHeroTemplateID, uint32_t dwTargetFashionID);
    BOOL DoChangeTeamLogoRespond(KPlayer* pPlayer, BOOL bSuccess, WORD wTeamLogoFrameID, WORD wTeamLogoEmblemID);
    BOOL DoSyncTeamLogoInfo(KPlayer* pPlayer);
    BOOL DoAddTeamLogoRespond(KPlayer* pPlayer, WORD wTeamLogoID);

    BOOL DoSyncCheerleadingSlotInfo(KPlayer* pPlayer);
    BOOL DoSyncCheerleadingItem(KPlayer* pPlayer, DWORD dwIndex, DWORD dwID, int nEndTime);
    BOOL DoRemoveCheerleadingItem(KPlayer* pPlayer, DWORD dwIndex);
    BOOL DoSyncCheerleadingInfoInBattle(int nSide, int nPos, KPlayer* pSrcPlayer, int nConnIndex);
    BOOL DoSyncGymInfo(KPlayer* pPlayer);
    BOOL DoUpgradeGymEquipLevel(int nConnIndex, int nNewLevel);
    BOOL DoSyncEquipCount(int nConnIndex, KGYM_TYPE eType, int nEquipCount);
    BOOL DoSyncHeroUseEquip(int nConnIndex, DWORD dwHeroTemplateID, KGYM_TYPE eType, int nEndTime, int nCount);
    BOOL DoFinishUseEquip(int nConnIndex, DWORD dwHeroTemplateID, int nFreeTalent, KGYM_TYPE eType, int nTalent);
    BOOL DoSyncUpgradeQueueData(KPlayer* pPlayer);
    BOOL DoAddUpgradeQueueNode(int nConnIndex, int nType, int nEndTime, uint32_t uHeroTemplateID);
    BOOL DoDelUpgradeQueueNode(int nConnIndex, int nType, uint32_t dwHeroTemplateID);
    BOOL DoSyncUpgradeSlotCount(int nConnIndex, int nSlotCount);
    BOOL DoSyncHeroTrainTeacherLevel(int nConnIndex, int nTeacherLevel);
    BOOL DoSyncHeroTrainSitCount(int nConnIndex, int nSitCount);
    BOOL DoSyncHeroTrainData(KPlayer* pPlayer);
    BOOL DoSyncSafeBoxLevel(int nConnIndex, int nSafeBoxLevel);
    BOOL DoBSSyncData(KPlayer* pPlayer);
    BOOL DoBSBuyLandResult(int nConnIndex, int nFreeLandCount, int nBuyLandTimes);
    BOOL DoBSSyncNewStore(int nConnIndex, int nFreeLandCount, uint32_t dwStoreID, int nType, int nLevel, int nGetMoneyTimes, int nNextGetMoneyTime);
    BOOL DoBSUpgradeStoreResult(int nConnIndex, uint32_t dwStoreID, int nLevel);
    BOOL DoBSSyncGetMoneyInfo(int nConnIndex, uint32_t dwStoreID, int nGetMoneyTimes, int nNextGetMoneyTime);
    BOOL DoBSSyncStoreNewType(int nConnIndex, uint32_t dwStoreID, int nType);
    BOOL DoSyncVIPLevel(int nConnIndex, int nVIPLevel, int nVIPExp);
    BOOL DoSyncVIPEndTime(int nConnIndex, int nVIPEndTime);
    BOOL DoSyncVIPAutoRepairFlag(KPlayer* pPlayer, BOOL bAutoRepairFlag);
    BOOL DoSyncMaxFatiguePoint(int nConnIndex, int nMaxFatiguePoint);
    BOOL DoSyncFreeTalent(int nConnIndex, DWORD dwHeroTemplateID, int nFreeTalent);
    BOOL DoSyncFreeClearCDTimes(int nConnIndex, int nFreeClearCDTimes);
    BOOL DoStartDrama(KScene* pScene, int nDramaID);
    BOOL DoStopDrama(KScene* pScene);
	BOOL DoStartDialog(KScene* pScene, int nDialogID, BOOL bNeedSendCompleted);
    BOOL DoSyncPlayerValueGroup(KPlayer* pPlayer, int nGroupID);
    BOOL DoSyncOnePlayerValue(KPlayer* pPlayer, DWORD dwID, int nCurValue);
    BOOL DoSyncChallengeAward(
        int nConnIndex, KCHALLENGE_AWARD_TYPE eAwardType, 
        KPlayer* pOtherPlayer, int nStartStep, int nEndStep,
        int nAwardMoney, int nAddMainHeroExp, int nAddAssistHeroExp, 
        std::vector<KCHALLENGEMISSION_AWARD>& rvecAwardItem
    );

    BOOL DoFinishChallengeMissionNotify(int nConnIndex, BOOL bSuccessed, int nStartStep, int nEndStep);

    BOOL DoSyncAchievementPoint(KPlayer* pPlayer, int nAchievementPoint);
    BOOL DoSyncAllAchievementState(KPlayer* pPlayer, void* pData, size_t uSize);
    BOOL DoSyncOneAchievementState(KPlayer* pPlayer, DWORD dwAchievementID, BOOL bFinished);
    BOOL DoSyncOneAchievementProgress(KPlayer* pPlayer, DWORD dwAchievementID, int nCurValue);

	BOOL DoSyncOneHeroUnlocktime(KPlayer* pPlayer, DWORD dwHeroTemplateID, time_t tUnlockTime);

    BOOL DoSyncGetTesterAward(int nConnIndex, int nGetTesterAwardLevel);
    BOOL DoSyncOnlineAwardInfo(int nConnIndex, int nOnlineAwardTimes, int nNextOnlineAwardTime);
    BOOL DoSyncOnlineAwardItems(int nConnIndex, int nRealAwardIndex, int nItemCount, KAWARD_ITEM* pItems[]);
    BOOL DoSyncAwardItemInfo(int nConnIndex, int nAwardTableID, int nItemCount, KAWARD_ITEM* pItems[]);
    BOOL DoDailySignAwardNotify(int nConnIndex, KAWARD_ITEM* pAwardItem);
    BOOL DoSyncRandomQuest(int nConnIndex, uint32_t dwQuestID);
    BOOL DoSyncHeroAddedExp(int nConnIndex, DWORD dwHeroTemplateID, int nAddedExp);
    BOOL DoSetBattleTemplate(KScene* pScene);
    BOOL DoModifyRemarkFail(KPlayer* pPlayer);

    BOOL DoCastSlamBallSkill(KHero* pSlamer, DWORD dwSlamBallSkillID, KBasketSocket* pSocket, DWORD dwInterferenceID);
    BOOL DoNormalSlamBallEx(KHero* pSlamer, DWORD dwSocketID, DWORD dwInterferenceID);
    
    BOOL DoClearActivePlayerData(int nConnIndex);
    BOOL DoSyncActiveProcess(int nConnIndex, int nCount, KActiveProcess* pActiveProcess);
    BOOL DoSyncActiveAward(int nConnIndex, int nCurActivePoint, int nOnlineTime, std::set<DWORD> setAwardAlreadyGet);
    BOOL DoUpdateActiveProcess(int nConnIndex, int nID, int nValue, BOOL bFinished);
    BOOL DoUpdateActivePoint(int nConnIndex, int nPoint);
    BOOL DoApplyActiveAwardRespond(int nConnIndex, DWORD dwAwardID, BOOL bSucc);
    BOOL DoSyncCostFatiguePoint(int nConnIndex, int nCostFatiguePoint);
    BOOL DoActiveLandMineNotify(KScene* pScene, DWORD dwLandMineID);
    BOOL DoSyncSceneHeroDataEnd(int nConnIndex);
    BOOL DoFinishCastSkillNotify(KScene* pScene, uint32_t dwHeroID);
    BOOL DoRemoteCall(int nConnIndex, const char* pcszFunName, KVARIABLE param1 = rpcInvalid, KVARIABLE param2 = rpcInvalid, KVARIABLE param3 = rpcInvalid, KVARIABLE param4 = rpcInvalid);
    BOOL DoSendFriendHeroList(DWORD dwPlayerID, DWORD dwFriendID, int nLevel, std::vector<DWORD>& vecHeroList, DWORD dwMainHeroID, DWORD dwAssistHeroID, const char cszName[]);
    BOOL DoSendFriendHeroInfo(DWORD dwPlayerID, DWORD dwFriendID, KHeroData* pHeroData, KHeroPackage* pHeroPackage);

	BOOL DoBroadcastScenePause(KScene* pScene, BOOL bPause); 
    BOOL DoSyncRandomPlayerList(int nConnIndex, std::vector<KPlayer*>& vecRandomPlayerList);
    BOOL DoGetEnterSceneHeroListRequest(int nConnIndex, KScene* pScene);
    BOOL DoSyncEnterSceneProgress(int nConnIndex, DWORD dwPlayerID, int nIndex, int nProgress, KScene* pScene);
    BOOL DoSyncEquipDecomposeResult(int nConnIndex, std::vector<IItem*>& vecItems);
    BOOL DoSyncPlayerBuff(int nConnIndex, DWORD dwBuffID, int nLeftFrame, int nActiveCount);
    BOOL DoDelPlayerBuff(int nConnIndex, DWORD dwBuffID);
    BOOL DoSyncCurrentCity(int nConnIndex, int nCity);

	BOOL DoNewMailNotify(int nConnIndex);

public:
	// 请求玩家数据
	void OnHandshakeRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // 请求加载场景物件
    void OnApplySceneObj(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplySceneHeroData(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // 通知服务器，资源已经加载完毕
    void OnLoadingComplete(char* pData, size_t nSize, int nConnIndex, int nFrame);

	// 请求进入场景
	void OnApplyEnterScene(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// 玩家登出
	void OnPlayerLogout(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // 上行Ping
    void OnPingSignal(char* pData, size_t nSize, int nConnIndex, int nFrame);

	void OnSubmitLimitPlayInfo(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
	void OnChangeFaceDirection(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // 角色移动指令
	void OnMoveAction(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // 角色站立指令
	void OnCharacterStand(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// 角色跳跃指令
	void OnHeroJump(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnOperateAITeammate(char* pData, size_t nSize, int nConnIndex, int nFrame);
	
	void OnHeroSkill(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnHeroSubSkill(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnHeroGrabSkill(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnPlayerTakeBall(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerShootBasket(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerAimAt(char* pData, size_t nSize,  int nConnIndex, int nFrame);
    void OnPlayerPickupObject(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerUseDoodad(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerDropObject(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnPlayerChangeRunMode(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnPlayerPassBall(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerShootObject(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerGetAwardRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerBuyGoodsRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerSellGoodsRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerRepairSingleEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerRepairAllEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerPresentGoodsRequeset(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPlayerChangeTeamLogoRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnPlayerClearTeamLogoNewFlag(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // 房间功能
    void OnApplyCreateRoom(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnInvitePlayerJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSwitchRoomHost(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSwtichBattleMap(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetRoomPassword(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyLeaveRoom(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyKickoutOther(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyRoomInfo(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplySetReady(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyStartGame(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRoomMemberChangePosRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnChangeRoomName(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAutoJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplySetRoomAIMode(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTryJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAcceptOrRefuseJoinRoom(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // 组队系统
    void OnCreateTeamRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTeamInvitePlayerRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTeamKickoutPlayerRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTeamLeaveRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTeamAcceptOrRefuseInvite(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTeamReadyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // PVP 匹配
    void OnAutoMatchRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnCancelAutoMatchRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
    void OnPlayerListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTalkMessage(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnClientLog(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGMCommand(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ---------------------- 好友系统 ----------------------->
    void OnGetFellowshipListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAddFellowshipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDelFellowshipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAddFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDelFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRenameFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetFellowshipRemarkRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
    void OnSyncUserPreferences(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRenameRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnGetTrainingHeroListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBeginTrainingHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnEndTrainingHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBuyTrainingSeatRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnUpgradeTeacherLevel(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ---------------------- 邮件系统 ----------------------->
    void OnSendMailRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetMaillistRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnQueryMailContent(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAcquireMailMoneyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAcquireMailItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetMailRead(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDeleteMail(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAcquireMailAllRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
    void OnBuyHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBuyHeroSoltRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnFireHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnSelectMainHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSelectAssistHeroRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ---------------------- VIP系统 ----------------------->
    void OnSetVIPAutoRepairFlag(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ---------------------- 任务系统 ----------------------->
    void OnGetQuestListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetQuestValueRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAcceptQuestRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnFinishQuestRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnCancelQuestRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDoneQuestDirectlyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnExchangeItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDestroyItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnUseItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnApplyEnterMissionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyLeaveMissionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetDailyQuestRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // pve 
    void OnCreatePveTeamRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyJoinPveTeamRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyPveTeamInfoRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPveTeamInvitePlayerRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPveTeamKickoutPlayerRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPveTeamLeaveRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPveTeamAcceptOrRefuseInvite(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPveTeamReadyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplySetPveTeamAiMode(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPveTeamStartGameRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPveTeamChangeMissionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTryJoinPveTeam(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAutoJoinPveTeam(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnQuickStartPveRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnApplyExtendPlayerPackage(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnCutStackItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSendNetworkDelay(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnActivityAwardRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnStatClientInfo(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnStatClientLoadingInfo(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnNormalShootBall(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSkillShootBall(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnNormalSlamBall(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSkillSlamBall(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnMakingMachineInfoRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnProduceItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnApplyUpgradeGymEquip(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyUseGymEquip(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyDirectEndUseGymEquip(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyBuyGymSlot(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyUpgradeSafeBox(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnStrengthenEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBuyFatiguePointRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnReportItemRead(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnBuyFashionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnChangeHeroFashionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnChargeFashionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnBuyCheerleaingSlot(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnUpdateCheerleadingSlotInfo(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRechareCheerleadingItem(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnClearUpgradeCDRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnUpgradeGymEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnHeroUseEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDirectFinishUseEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnBsBuyLandRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBsBuildStoreRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBsUpgradeStoreRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBsGetStoreProfit(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBsClearGetStoreProfitCD(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBsGetAllStoreProfit(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBsClearAllGetStoreProfitCD(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnBsChangeStoreType(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
	void OnDialogCompleted(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnScenePause(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnStartDramaRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // 俱乐部
    void OnCreateClubRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnGetMemberListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnApplyClubRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnAcceptAppRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnInviteJoinClub(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnAccetpInviteRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetClubPostRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnDelMemberRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnSetClubTxtRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnGetRandClubRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnGetApplyListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
    void OnSwitchApplyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
	void OnFindClubRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 
	void OnGetClubListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame); 

    void OnFinishAchievementRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnIncreaseAchievementValue(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnGetTesterAward(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetLevelupAward(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnViewOnlineAward(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetOnlineAward(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetAwardTableInfo(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnClientCallRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnVIPChargeSelf(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnVIPChargeFriend(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRequestPlayerValueGroup(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetRandomQuest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSkipRandomQuest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnSelectHeroSkill(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnCastSlamBallSkill(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnNormalSlamBallEx(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyActiveAward(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetClubApplyNumRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnFindRoleInTopListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRequestTopListPage(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRequestFriendHeroList(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRequestFriendHeroInfo(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetRefuseStranger(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnRequestRandomHallPlayerList(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetEnterSceneHeroListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSyncEnterSceneProgress(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnReqeustRecommendFriend(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDecomposeEquipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnRequestClubName(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnCloseBattleReport(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnRankQuickStartRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnChangeRoomHost(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnFreePVPQucikStart(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnChangeCurrentCity(char* pData, size_t nSize, int nConnIndex, int nFrame);
};

extern KPlayerServer g_PlayerServer;

struct KBroadcastFunc
{
    KBroadcastFunc();
	BOOL operator()(KPlayer* pPlayer);
    BOOL operator()(KHero* pHero);
	void*	        m_pvData;			
	size_t	        m_uSize;			
    DWORD           m_dwExcept;         // 广播时排除这个ID的玩家
    DWORD           m_dwExclusive;      // 广播时,只是形式上的广播,其实只发给这个ID的玩家
};

struct KGroupTalkFunc
{
    KGroupTalkFunc();
    BOOL operator()(KPlayer* pPlayer);
    void*       m_pvData;
    size_t      m_uSize;
    int         m_nGroupID;
};
