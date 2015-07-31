// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KRelayClient.h 
//	Creator 	: Xiayong  
//  Date		: 08/08/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "Common/KG_Socket.h"
#include "Relay_GS_Protocol.h"
#include <list>

class KPlayer;
class KScene;
struct KHeroData;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

class KRelayClient
{
public:
	KRelayClient();

	BOOL Init();
	void UnInit();

    void Activate();
    BOOL ProcessPackage();

    int  GetWorldIndex() { return m_nWorldIndex; }

    BOOL SaveRoleData(KPlayer* pPlayer);
    //BOOL SaveRoleDataToProtoBuf(KPlayer* pPlayer);

    void PrepareToQuit() { m_bQuiting = true; m_nNextQuitingSaveTime = 0; }

    void ResetPakStat();
    BOOL DumpPakStat();

private:
    IKG_SocketStream*	m_piSocketStream;
    BOOL                m_bSocketError;
    BOOL                m_bQuiting;
    time_t              m_nNextQuitingSaveTime;

	typedef void (KRelayClient::*PROCESS_PROTOCOL_FUNC)(BYTE* pbyData, size_t uDataLen);
	PROCESS_PROTOCOL_FUNC	m_ProcessProtocolFuns[r2s_protocol_end];
    size_t                  m_uProtocolSize[r2s_protocol_end];

    struct KPROTOCOL_STAT_INFO
    {
        DWORD    dwPackCount;
        uint64_t uTotalSize;
    };
    KPROTOCOL_STAT_INFO m_S2RPakStat[s2r_protocol_end];
    KPROTOCOL_STAT_INFO m_R2SPakStat[r2s_protocol_end];

    BOOL Send(IKG_Buffer* piBuffer);

    BOOL RecvConfigFromRelay();

public:
    float  m_fRecvPakSpeed;
    float  m_fSendPakSpeed;
    float  m_fUpTraffic;
    float  m_fDownTraffic;

private:
	int    m_nPingCycle;
    time_t m_nLastSendPacketTime;
    int    m_nWorldIndex;

    DWORD  m_dwSyncRoleID;
    BYTE*  m_pbySyncRoleBuffer;
    T3DB::KPB_SAVE_DATA* m_pLoadBuf;

    size_t m_uSyncRoleOffset;
    BYTE   m_byTempData[MAX_EXTERNAL_PACKAGE_SIZE];

    BYTE*  m_pbySaveRoleBuffer;
    T3DB::KPB_SAVE_DATA* m_pSaveBuf;

    int    m_nRecvPakSpeed;
    int    m_nSendPakSpeed;
    int    m_nUpTraffic;
    int    m_nDownTraffic;

public:
    BOOL DoApplyServerConfigRequest();

	BOOL DoHandshakeRequest();
	BOOL DoPingSignal();

    BOOL DoUpdatePerformance();

	BOOL DoPlayerLoginRespond(
        DWORD dwPlayerID, BOOL bPermit, GUID Guid, DWORD dwPacketIdentity
    );

	BOOL DoConfirmPlayerLoginRequest(DWORD dwPlayerID, DWORD dwClientIP);

    BOOL DoPlayerLeaveGS(DWORD dwPlayerID);

	BOOL DoSubmitLimitPlayInfo(DWORD dwGroupID, DWORD dwIP, const char cszAccount[], const char cszName[], const char cszID[], const char cszEmail[]);

    BOOL DoCreateMapRespond(DWORD dwMapID, int nMapCopyIndex, BOOL bSucceed);

    BOOL DoSearchMapRequest(KPlayer* pPlayer, DWORD dwMapID, int nMapCopyIndex);

    BOOL DoTransferPlayerRequest(KPlayer* pPlayer);
    BOOL DoTransferPlayerRespond(DWORD dwPlayerID, BOOL bSucceed, GUID Guid);

    BOOL DoCoinShopBuyItemRequest(
        KPlayer* pPlayer, DWORD dwTabType, DWORD dwTabIndex, int nGoodsID, int nCount, 
        int nCoinPrice, KCUSTOM_CONSUME_INFO* pCCInfo = NULL
    );

    // ----------------------- 角色相关操作 -------------------------------------    
    BOOL DoLoadRoleDataRequest(DWORD dwRoleID);
    BOOL DoSyncRoleData(DWORD dwID, BYTE* pbyData, size_t uOffset, size_t uDataLen);
    BOOL DoSaveRoleData(KPlayer* pPlayer, size_t uRoleDataLen);
    BOOL DoPlayerEnterSceneNotify(DWORD dwPlayerID, DWORD dwMapID, int nMapCopyIndex);
    BOOL DoPlayerEnterHallNotify(DWORD dwPlayerID);
    BOOL DoBattleFinishedNotify(DWORD dwMapID, int nMapCopyIndex, int nWinnerSide);
    BOOL DoApplyCreateRoom(KPlayer* pPlayer, DWORD dwMapID, const char cszRoomName[], const char cszPassword[]);
    BOOL DoApplyJoinRoom(KPlayer* pPlayer, DWORD dwRoomID, const char cszPassword[]);
    BOOL DoInvitePlayerJoinRoom(DWORD dwInviterID, const char szDestPlayerName[]);
    BOOL DoSwitchRoomHost(DWORD dwCurHostID, DWORD dwNewHostID);
    BOOL DoSwitchBattleMap(DWORD dwHostID, DWORD dwNewMapID);
    BOOL DoSetRoomPassword(DWORD dwHostID, const char szNewPassword[]);
    BOOL DoApplyLeaveRoom(DWORD dwPlayerID);
    BOOL DoApplyKickoutOther(DWORD dwPlayerID, DWORD dwOtherPlayerID);
    BOOL DoApplySetReady(DWORD dwPlayerID, BOOL bReady);
    BOOL DoApplyStartGame(DWORD dwPlayerID);
    BOOL DoRoomMemberChangePosRequest(DWORD dwPlayerID, KSIDE_TYPE eNewSide, int nNewPos);
    BOOL DoChangeRoomName(DWORD dwPlayerID, char szNewName[]);
    BOOL DoSelectHeroRequest(KPlayer* pPlayer);
    BOOL DoAutoJoinRoom(KPlayer* pPlayer);
    BOOL DoApplySetRoomAIMode(DWORD dwPlayerID, KSIDE_TYPE eSide, int nPos, BOOL bAIMode);
    BOOL DoTryJoinRoom(KPlayer* pPlayer, DWORD dwRoomID);
    BOOL DoAcceptOrRefuseJoinRoom(KPlayer* pPlayer, int nAcceptCode, DWORD dwRoomID);

    // 组队
    BOOL DoCreateTeamRequest(KPlayer* pPlayer);
    BOOL DoTeamInvitePlayerRequest(DWORD dwPlayerID, const char cszTargetName[]);
    BOOL DoTeamKickoutPlayerRequest(DWORD dwPlayerID, DWORD dwBeKickedPlayerID);
    BOOL DoTeamLeaveRequest(DWORD dwPlayerID);
    BOOL DoTeamAcceptOrRefuseInvite(KPlayer* pTarget, int nAcceptCode, const char (&szInviterName)[_NAME_LEN]);
    BOOL DoTeamReadyRequest(uint32_t dwPlayerID, BOOL bReady);

    // PVP匹配
    BOOL DoAutoMatchRequest(KPlayer* pRequestor);
    BOOL DoCancelAutoMatchRequest(KPlayer* pPlayer);
    
    BOOL DoRenameRequest(DWORD dwPlayerID, char szNewName[]);
    BOOL DoUpdateRoleLevel(uint32_t dwPlayerID, int nNewLevel);
    BOOL DoApplyRoomBaseInfoForCache(DWORD dwLastRoomID);

    // PVE组队
    BOOL DoCreatePveTeamRequest( KPlayer* pPlayer, int nPveMode, int nMissionType, int nMissionStep, int nMissionLevel, uint32_t dwMapID, const char (&cszTeamName)[_NAME_LEN], const char (&cszPassword)[cdRoomPasswordMaxLen] );
	BOOL DoQuickStartPveRequest(KPlayer* pPlayer, int nPveMode, int nMissionType, int nMissionStep, int nMissionLevel, 
		uint32_t dwMapID, char szTeamName[], char szPassword[] 
	);
    BOOL DoApplyJoinPveTeamRequest(KPlayer* pPlayer, uint32_t dwTeamID, char szPassword[]);
    BOOL DoApplyPveTeamInfoRequest( uint32_t dwPlayerID, int nPage, int nPveMode, int nMissionType, int nMissiongStep);
    BOOL DoPveTeamInvitePlayerRequest(DWORD dwPlayerID, const char cszTargetName[]);
    BOOL DoPveTeamKickoutPlayerRequest(DWORD dwPlayerID, DWORD dwBeKickedPlayerID);
    BOOL DoPveTeamLeaveRequest(DWORD dwPlayerID);
    BOOL DoPveTeamAcceptOrRefuseInvite(KPlayer* pPlayer, int nAccpetCode, const char cszInviterName[]);
    BOOL DoPveTeamReadyRequest(KPlayer* pPlayer, BOOL bReady);
    BOOL DoPveTeamSetAiMode(KPlayer* pPlayer, BOOL bIsAiMode);
    BOOL DoPveTeamStartGameRequest(KPlayer* pPlayer, BOOL bNeedCostChallengeItem, BOOL bNeedCheckAllReady);
    BOOL DoPveTeamChangeMissionRequest(uint32_t dwPlayerID, int nMissionType, int nMissiontStep, int nMissionLevel, uint32_t dwMapID);
    BOOL DoPveTeamTryJoin(KPlayer* pPlayer, int nPveMode, uint32_t dwTeamID);
    BOOL DoPveTeamAutoJoin(KPlayer* pPlayer, int nPveMode, int nMissionType, int nMissionStep, int nOpenLevel, unsigned uValidLevel);
    BOOL DoPveTeamReset(uint32_t dwPlayerID);

    BOOL DoCanEnterMissionRespond(uint32_t dwPlayerID, uint32_t dwHeroTemplateID,  int nMissionType, int nMissionStep, int nMissionLevel, BOOL bSuccess);
    BOOL DoUpdatePlayerGender(KPlayer* pPlayer);
    BOOL DoChangeExtPointRequest(DWORD dwPlayerID, unsigned uExtPointIndex, int nChangeValue);
    BOOL DoUpdateHeroLevel(KPlayer* pPlayer, DWORD dwHeroTemplateID);
    BOOL DoSyncMainHero(uint32_t dwPlayerID, uint32_t dwHeroTemplateID);
    BOOL DoKickAccountRespond(const char cszAccount[], int nPlayerIndex);

    BOOL DoUpdateVIPInfo(KPlayer* pPlayer);
    BOOL DoUpdateClubID(KPlayer* pPlayer);

    BOOL DoRankQuickStart(KPlayer* pPlayer);
    BOOL DoChangeRoomHost(DWORD dwPlayerID, int nRoomType, DWORD dwNewHostID);

    BOOL DoFreePVPQuickStart(KPlayer* pPlayer, DWORD dwMapID, const char cszRoomName[], const char cszPassword[]);

    BOOL DoUpdatePlayerTeamLogo(KPlayer* pPlayer);

    //AutoCode:-发送协议函数结束-
public:
    void OnApplyServerConfigRespond(BYTE* pbyData, size_t uDataLen);
	void OnHandshakeRespond(BYTE* pbyData, size_t uDataLen);
    void OnQuitNotify(BYTE* pbyData, size_t uDataLen);

	void OnCreateMapNotify(BYTE* pbyData, size_t uDataLen);
    
    void OnFinishCreateMapNotify(BYTE* pbyData, size_t uDataLen);

	void OnDeleteMapNotify(BYTE* pbyData, size_t uDataLen);

	void OnSearchMapRespond(BYTE* pbyData, size_t uDataLen);

	void OnTransferPlayerRequest(BYTE* pbyData, size_t uDataLen);
	void OnTransferPlayerRespond(BYTE* pbyData, size_t uDataLen);
    void OnCoinShopBuyItemRespond(BYTE* pbyData, size_t uDataLen);

	void OnPlayerLoginRequest(BYTE* pbyData, size_t uDataLen);

	void OnLimitPlayInfoResponse(BYTE* pbyData, size_t uDataLen);
	void OnConfirmPlayerLoginRespond(BYTE* pbyData, size_t uDataLen);

    void OnKickAccountNotify(BYTE* pbyData, size_t uDataLen);
    void OnSaveRoleDataRespond(BYTE* pbyData, size_t uDataLen);
    void OnSyncRoleData(BYTE* pbyData, size_t uDataLen);
    void OnLoadRoleData(BYTE* pbyData, size_t uDataLen);

    // 房间机制
    void OnSyncCreateRoomRespond(BYTE* pbyData, size_t uDataLen);
    void OnSyncOneRoomMemberPosInfo(BYTE* pbyData, size_t uDataLen);
    void OnApplyJoinRoomRespond(BYTE* pbyData, size_t uDataLen);
    void OnApplyLeaveRoomRespond(BYTE* pbyData, size_t uDataLen);
    void OnApplySwitchRoomHostRespond(BYTE* pbyData, size_t uDataLen);
    void OnApplySwitchBattleMapRespond(BYTE* pbyData, size_t uDataLen);
    void OnApplySetRoomPasswordRespond(BYTE* pbyData, size_t uDataLen);
    void OnApplySetReadyRespond(BYTE* pbyData, size_t uDataLen);   
    void OnApplyStartGameRespond(BYTE* pbyData, size_t uDataLen);
    void OnSyncRoomName(BYTE* pbyData, size_t uDataLen);

    // 房间缓存
    void OnAddRoomCache(BYTE* pbyData, size_t uDataLen);
    void OnDelRoomCache(BYTE* pbyData, size_t uDataLen);
    void OnUpdateRoomNameInCache(BYTE* pbyData, size_t uDataLen);
    void OnUpdateMapIDInCache(BYTE* pbyData, size_t uDataLen);
    void OnUpdateMemberCountInCache(BYTE* pbyData, size_t uDataLen);
    void OnUpdateFightingStateInCache(BYTE* pbyData, size_t uDataLen);
    void OnUpdateHasPasswordInCache(BYTE* pbyData, size_t uDataLen);
    void OnSyncRoomBaseInfoForCache(BYTE* pbyData, size_t uDataLen);

    // 自动匹配组队相关
    void OnAutoMatchRespond(BYTE* pbyData, size_t uDataLen);
    void OnLeaveAutoMatchNotify(BYTE* pbyData, size_t uDataLen);

    void OnSendToClient(BYTE* pbyData, size_t uDataLen);
    void OnRenameRespond(BYTE* pbyData, size_t uDataLen);
    void OnResetMap(BYTE* pbyData, size_t uDataLen);

    void OnCanEnterMission(BYTE* pbyData, size_t uDataLen);

    void OnFreePVPInvite(BYTE* pbyData, size_t uDataLen);
    void OnLadderPVPInvite(BYTE* pbyData, size_t uDataLen);
    void OnPVEInvite(BYTE* pbyData, size_t uDataLen);
    void OnChangeExtPointRespond(BYTE* pbyData, size_t uDataLen);

    void OnForbidPlayerTalk(BYTE* pbyData, size_t uDataLen);
    void OnForbidIPTalk(BYTE* pbyData, size_t uDataLen);
    void OnFreezeRole(BYTE* pbyData, size_t uDataLen);
    void OnFreezeIP(BYTE* pbyData, size_t uDataLen);
    void OnAccountNewCoinNotify(BYTE* pbyData, size_t uDataLen);
    void OnSendItemMailNotify(BYTE* pbyData, size_t uDataLen);

    void OnRequestPlayerTeamLogo(BYTE* pbyData, size_t uDataLen);

    //AutoCode:-处理协议函数结束-
};

extern KRelayClient g_RelayClient;
