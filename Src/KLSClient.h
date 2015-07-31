// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KRelayClient.h 
//	Creator 	: Xiayong  
//  Date		: 08/08/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "Common/KG_Socket.h"
#include "LS_GS_Protocol.h"
#include <list>
#include "KRPC.h"

class KPlayer;
class KScene;

class KLSClient
{
public:
	KLSClient();

	BOOL Init();
	void UnInit();

    void Activate();
    BOOL ProcessPackage();

    int  GetWorldIndex() { return m_nWorldIndex; }

private:
    BOOL        m_bRunFlag;
    KThread     m_WorkThread;
    int         ThreadFunction();
    static void WorkThreadFunction(void* pvParam); 
    BOOL        Connect();

protected:
    IKG_SocketStream*	m_piSocketStream;
    BOOL                m_bSocketError;
    time_t              m_nNextQuitingSaveTime;

	typedef void (KLSClient::*PROCESS_PROTOCOL_FUNC)(BYTE* pbyData, size_t uDataLen);
	PROCESS_PROTOCOL_FUNC	m_ProcessProtocolFuns[l2s_protocol_end];
    size_t                  m_uProtocolSize[l2s_protocol_end];

    BOOL Send(IKG_Buffer* piBuffer);

private:
    time_t m_nLastSendPacketTime;
    int    m_nWorldIndex;

    int    m_nRelayPort;
    int    m_nPingCycle;
    char   m_szLogicAddr[16];

    KRPCProcesser<KLSClient>* m_pRpcProcess;

public:
    // 通用RPC机制
    void DoRemoteCall(const char* pcszFunName, KVARIABLE param1 = rpcInvalid, KVARIABLE param2 = rpcInvalid, KVARIABLE param3 = rpcInvalid, KVARIABLE param4 = rpcInvalid);

public:
	BOOL DoPingSignal();
    BOOL DoTalkMessage(
        int nMsgType, DWORD dwTalkerID, const char cszTalker[],
        DWORD dwReceiverID, const char cszReceiver[], 
        size_t uDataLen, const char cszTalkData[]
    );

    // --------------------- 好友相关 --------------------------------------

    BOOL DoApplyFellowshipDataRequest(DWORD dwPlayerID);
    BOOL DoUpdateFellowshipData(DWORD dwPlayerID, size_t uDataSize, BYTE byData[]);
    BOOL DoPlayerAddFellowshipRequest(DWORD dwPlayerID, char szAlliedPlayerName[], int nType);
    BOOL DoGetFellowshipName(DWORD dwPlayerID, int nArrayLength, DWORD dwAlliedPlayerIDArray[]);
    BOOL DoApplyFellowshipPlayerFellowInfo(DWORD dwPlayerID, BOOL bNotifyClient);
    BOOL DoApplyRecommendFriend(DWORD dwPlayerID, int nLevel);
    BOOL DoRegFriendRecommend(DWORD dwPlayerID);
    BOOL DoRequestFriendHeroList(DWORD dwRequesterID, DWORD dwFriendID);
    BOOL DoRequestFriendHeroInfo(DWORD dwRequesterID, DWORD dwFriendID, DWORD dwHeroTemplateID);
    // ----------------------- 统计数据相关 --------------------------------
    BOOL DoUpdateStatDataRequest();
    BOOL DoQueryStatIDRequest(const char cszName[]);
    //-------------------------俱乐部-----------------------------------------
    BOOL DoApplyClubBaseInfo(DWORD dwPlayerID, BOOL bIsLogin);
    BOOL DoCreateClubRequest(DWORD dwPlayerID, const char cszClubName[]);
    BOOL DoRandomClubRequest(DWORD dwPlayerID);
    BOOL DoGetApplyListRequest(DWORD dwPlayerID);
    BOOL DoGetApplyNumRequest(DWORD dwPlayerID);
    BOOL DoSwitchApplyRequest(DWORD dwPlayerID, DWORD dwSwitch);
    BOOL DoFindClubRequest(DWORD dwPlayerID, const char cszFindName[]);
	BOOL DoGetClubList(DWORD dwPlayerID, DWORD dwPageIndex);
    BOOL DoAddMemberRequest(DWORD dwPlayerID, DWORD dwClubID);
    BOOL DoDelMemberRequest(DWORD dwPlayerID, DWORD dwClubID, DWORD dwTargetID);
    BOOL DoApplyClubMemberList(DWORD dwPlayerID, DWORD dwClubID, DWORD dwVersion);
    BOOL DoApplyJoinClubRequest(DWORD dwPlayerID, DWORD dwClubID);
    BOOL DoClubAcceptApplyRequest(DWORD dwPlayerID, DWORD dwClubID, DWORD dwTargetID, BOOL bAccept);
    BOOL DoClubInviteRequest(DWORD dwPlayerID, DWORD dwClubID, const char cszClubName[]);
    BOOL DoAcceptClubInviteRequest(DWORD dwPlayerID, DWORD dwClubID, DWORD dwLauncherID, int nRepsond);
    BOOL DoClubSetPostRequest(DWORD dwPlayerID, DWORD dwClubID, DWORD dwUnderID, int nPost);
    BOOL DoSetDescriptRequest(DWORD dwPlayerID, DWORD dwClubID, int nTypel, int nTxtLen, const char cszText[]);

    // --------------------- 邮件系统 ---------------------------------------
    BOOL DoSendMailRequest(int nRequestID, const char cszDstName[], KMail* pMail, size_t uMailLen);
    BOOL DoSendGlobalMailRequest(KMail* pMail, size_t uMailLen, int nLifeTime);
    BOOL DoGetMailListRequest(DWORD dwPlayerID, DWORD dwStartID);
    BOOL DoQueryMailContent(DWORD dwPlayer, DWORD dwMailID, int nMailType);
    BOOL DoAcquireMailMoneyRequest(DWORD dwPlayer, DWORD dwMailID);
    BOOL DoAcquireMailItemRequest(DWORD dwPlayer, DWORD dwMailID, int nIndex);
    BOOL DoAcquireMailAllRequest(DWORD dwPlayer, DWORD dwMailID);
    BOOL DoGiveMailMoneyToPlayerRespond(DWORD dwPlayerID, DWORD dwMailID, KMAIL_RESULT_CODE eResult);
    BOOL DoGiveMailItemToPlayerRespond(DWORD dwPlayerID, DWORD dwMailID, int nIndex, KMAIL_RESULT_CODE eResult);
    BOOL DoGiveMailAllToPlayerRespond(DWORD dwPlayerID, DWORD dwMailID, KMAIL_RESULT_CODE eResult);
    BOOL DoSetMailRead(DWORD dwSrcID, DWORD dwMailID);
    BOOL DoDeleteMail(DWORD dwSrcID, DWORD dwMailID);
    BOOL DoWhisperSuccess(
        int nMsgType, uint32_t dwTalkerID, const char szTalker[], uint32_t dwReceiverID, 
        const char szReceiver[], size_t uDataLen, const char byTalkData[]
    );

    BOOL DoUpdatePlayerGender(DWORD dwPlayerID, KGENDER eGender);
    BOOL DoCheckAndUpdateActivityStateRequest(DWORD dwPlayerID, DWORD dwActivityID, int nCurState, int nNewState);

    BOOL DoPresentVIPTimeToFriendRequest(DWORD dwSrcPlayerID, DWORD dwFirendID, int nChargeType);
    BOOL DoGetVIPPresentNotify(uint32_t dwSNID);

    BOOL DoBroadcastMessage(DWORD dwMessageID, void* pData /*= NULL*/, size_t uDataLen /*= 0*/);
    BOOL DoBroadcast(void* pData, size_t uSize);

    BOOL DoSendMessageToAllFriend(DWORD dwPlayerID, DWORD dwMessageID, void* pData /*= NULL*/, size_t uDataLen /*= 0*/);
    BOOL DoSendToPlayer(DWORD dwTargetID, void* pData, size_t uSize);
    BOOL DoSendToPlayerServer(DWORD dwTargetID, DWORD dwRespondID, void* pData, size_t uSize);
    BOOL DoSendMessageToClub(DWORD dwClubID, DWORD dwMessageID, void* pData /*= NULL*/, size_t uDataLen /*= 0*/);
    BOOL DoBroadcastInClub(DWORD dwClubID, DWORD dwExceptID, void* pData, size_t uSize);

    BOOL DoRoleLoginHallNotify(DWORD dwPlayerID);

    BOOL DoBeAddedFriendNotify(DWORD dwPlayerID, KPlayer* pSrcPlayer);

    BOOL DoFindRoleInTopListRequest(DWORD dwRoleID, int nTopListType, int nGroup, char (&szTargetRoleName)[_NAME_LEN]);
    BOOL DoRequestTopListPage(DWORD dwRoleID, int nTopListType, int nGroup, int nPageIndex);

    BOOL DoLadderDataChangeNotify(KPlayer* pPlayer, DWORD dwHeroTemplateID);
	BOOL DoHeroEquipScoreChangeNotify(KPlayer* pPlayer, DWORD dwHeroTemplateID);
	BOOL DoTeamEquipScoreChangeNotify(KPlayer* pPlayer);
	BOOL DoClearTopList(int nTopListType, int nGroup);
    BOOL DoClubInfosRequest(DWORD dwRoleID, KScene* pScene);
    BOOL DoRequestClubName(DWORD dwRoleID, DWORD dwClubID);
    // ----------------------------------------------------------------------

    //AutoCode:-发送协议函数结束-
public:
    void OnRemoteCall(BYTE* pbyData, size_t uDataLen);
	void OnHandshakeRespond(BYTE* pbyData, size_t uDataLen);
    void OnTalkMessage(BYTE* pbyData, size_t uDataLen);

    // ------------------ 好友相关 ------------------------------------------
    void OnApplyFellowshipDataRespond(BYTE* pbyData, size_t uDataLen);
    void OnAddFellowshipRespond(BYTE* pbyData, size_t uDataLen);
    void OnGetFellowshipNameRespond(BYTE* pbyData, size_t uDataLen);
    void OnSyncFellowshipPlayerInfo(BYTE* pbyData, size_t uDataLen);
    void OnQueryStatIDRespond(BYTE* pbyData, size_t uDataLen);
    void OnRequestFriendHeroList(BYTE* pbyData, size_t uDataLen);
    void OnRequestFriendHeroInfo(BYTE* pbyData, size_t uDataLen);
    void OnBeAddedFriendNotify(BYTE* pbyData, size_t uDataLen);

    // 网页上的活动
    void OnActivityFinishNotify(BYTE* pbyData, size_t uDataLen);
    void OnCheckUpdateActivityRespond(BYTE* pbyData, size_t uDataLen);

    // ------------------ 邮件 ----------------------------------------------
    void OnSendMailRespond(BYTE* pbyData, size_t uDataLen);
    void OnNewMailNotify(BYTE* pbyData, size_t uDataLen);
    void OnMailTooManyNotify(BYTE* pbyData, size_t uDataLen);
    void OnGetMailListRespond(BYTE* pbyData, size_t uDataLen);
    void OnSyncMailContent(BYTE* pbyData, size_t uDataLen);
    void OnGiveMailMoneyToPlayer(BYTE* pbyData, size_t uDataLen);
    void OnGiveMailAllToPlayer(BYTE* pbyData, size_t uDataLen);
    void OnGiveMailItemToPlayer(BYTE* pbyData, size_t uDataLen);
    void OnMailGeneralRespond(BYTE* pbyData, size_t uDataLen);

    void OnSendToClient(BYTE* pbyData, size_t uDataLen);
    void OnVIPPresentNotify(BYTE* pbyData, size_t uDataLen);

    void OnForbidRole(BYTE* pbyData, size_t uDataLen);
    void OnKickRole(BYTE* pbyData, size_t uDataLen);

    //---------------------------------俱乐部-----------------------------
    void OnLeaveClubNotify(BYTE* pbyData, size_t uDataLen);
    void OnEnterClubNotify(BYTE* pbyData, size_t uDataLen);
    void OnNewMemberNotify(BYTE* pbyData, size_t uDataLen);
    void OnApplyJoinClubRespond(BYTE* pbyData, size_t uDataLen);
    void OnPostModify(BYTE* pbyData, size_t uDataLen);
    void OnClubBaseInfo(BYTE* pbyData, size_t uDataLen);
    void OnCreateClubRespond(BYTE* pbyData, size_t uDataLen);
    void OnClubInviteNotify(BYTE* pbyData, size_t uDataLen);

    void OnBroadcastToClient(BYTE* pbyData, size_t uDataLen);
    
    //AutoCode:-处理协议函数结束-

private:
    void OnCheckCanPresentResult(int nParamCount, KVARIABLE m_Param[]);
};

extern KLSClient g_LSClient;
