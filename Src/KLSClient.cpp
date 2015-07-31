#include "stdafx.h"
#include "LS_GS_Protocol.h"
#include "KLSClient.h"
#include "SO3Result.h"
#include "Engine/KG_CreateGUID.h"
#include "Engine/KG_Time.h"
#include "KRoleDBDataDef.h"
#include "KPlayer.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KScene.h"
#include "KTerrainData.h"
#include "KClubDef.h"
#include "KRelayClient.h"

KLSClient g_LSClient;
extern KRelayClient g_RelayClient;

KLSClient::KLSClient(void)
{
    m_piSocketStream        = NULL;
    m_bSocketError          = false;
    m_nNextQuitingSaveTime  = 0;
	m_nPingCycle            = 0;
	m_nLastSendPacketTime   = 0;
    m_szLogicAddr[0]        = '\0';
    m_nRelayPort            = 0;

    memset(m_ProcessProtocolFuns, 0, sizeof(m_ProcessProtocolFuns));
    memset(m_uProtocolSize, 0, sizeof(m_uProtocolSize));

    // ------------------ 协议注册 --------------------------------------------------------------------
    REGISTER_INTERNAL_FUNC(l2s_remote_call, &KLSClient::OnRemoteCall, sizeof(L2S_REMOTE_CALL));
    REGISTER_INTERNAL_FUNC(l2s_handshake_respond, &KLSClient::OnHandshakeRespond, sizeof(L2S_HANDSHAKE_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_talk_message, &KLSClient::OnTalkMessage, sizeof(L2S_TALK_MESSAGE));
    REGISTER_INTERNAL_FUNC(l2s_apply_fellowship_data_respond, &KLSClient::OnApplyFellowshipDataRespond, sizeof(L2S_APPLY_FELLOWSHIP_DATA_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_add_fellowship_respond, &KLSClient::OnAddFellowshipRespond, sizeof(L2S_ADD_FELLOWSHIP_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_get_fellowship_name_respond, &KLSClient::OnGetFellowshipNameRespond, sizeof(L2S_GET_FELLOWSHIP_NAME_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_sync_fellowship_player_info, &KLSClient::OnSyncFellowshipPlayerInfo, sizeof(L2S_SYNC_FELLOWSHIP_PLAYER_INFO));
    REGISTER_INTERNAL_FUNC(l2s_query_stat_id_respond, &KLSClient::OnQueryStatIDRespond, sizeof(L2S_QUERY_STAT_ID_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_request_friend_hero_list, &KLSClient::OnRequestFriendHeroList, sizeof(L2S_REQUEST_FRIEND_HERO_LIST));
    REGISTER_INTERNAL_FUNC(l2s_request_friend_hero_info, &KLSClient::OnRequestFriendHeroInfo, sizeof(L2S_REQUEST_FRIEND_HERO_INFO));
    REGISTER_INTERNAL_FUNC(l2s_beadded_friend_notify, &KLSClient::OnBeAddedFriendNotify, sizeof(L2S_BEADDED_FRIEND_NOTIFY));
    REGISTER_INTERNAL_FUNC(l2s_activity_finish_notify, &KLSClient::OnActivityFinishNotify, sizeof(L2S_ACTIVITY_FINISH_NOTIFY));
    REGISTER_INTERNAL_FUNC(l2s_activity_check_update_respond, &KLSClient::OnCheckUpdateActivityRespond, sizeof(L2S_ACTIVITY_CHECK_UPDATE_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_send_mail_respond, &KLSClient::OnSendMailRespond, sizeof(L2S_SEND_MAIL_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_get_maillist_respond, &KLSClient::OnGetMailListRespond, sizeof(L2S_GET_MAILLIST_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_sync_mail_content, &KLSClient::OnSyncMailContent, sizeof(L2S_SYNC_MAIL_CONTENT));
    REGISTER_INTERNAL_FUNC(l2s_give_mail_money_to_player, &KLSClient::OnGiveMailMoneyToPlayer, sizeof(L2S_GIVE_MAIL_MONEY_TO_PLAYER));
    REGISTER_INTERNAL_FUNC(l2s_give_mail_all_to_player, &KLSClient::OnGiveMailAllToPlayer, sizeof(L2S_GIVE_MAIL_MONEY_TO_PLAYER));
    REGISTER_INTERNAL_FUNC(l2s_give_mail_item_to_player, &KLSClient::OnGiveMailItemToPlayer, sizeof(L2S_GIVE_MAIL_ITEM_TO_PLAYER));
    REGISTER_INTERNAL_FUNC(l2s_new_mail_notify, &KLSClient::OnNewMailNotify, sizeof(L2S_NEW_MAIL_NOTIFY));
    REGISTER_INTERNAL_FUNC(l2s_mail_too_many_notify, &KLSClient::OnMailTooManyNotify, sizeof(L2S_MAIL_TOO_MANG_NOTIFY));
    REGISTER_INTERNAL_FUNC(l2s_mail_general_respond, &KLSClient::OnMailGeneralRespond, sizeof(L2S_MAIL_GENERAL_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_send_to_client, &KLSClient::OnSendToClient, sizeof(L2S_SEND_TO_CLIENT));
    REGISTER_INTERNAL_FUNC(l2s_present_vip_notify, &KLSClient::OnVIPPresentNotify, sizeof(L2S_PRESENT_VIP_NOTIFY));
    REGISTER_INTERNAL_FUNC(l2s_forbid_role, &KLSClient::OnForbidRole, sizeof(L2S_FORBID_ROLE));
    REGISTER_INTERNAL_FUNC(l2s_kick_role, &KLSClient::OnKickRole, sizeof(L2S_KICK_ROLE));
    REGISTER_INTERNAL_FUNC(l2s_leave_club_notify, &KLSClient::OnLeaveClubNotify, sizeof(L2S_LEAVE_CLUB_NOTIFY));
    REGISTER_INTERNAL_FUNC(l2s_enter_club_notify, &KLSClient::OnEnterClubNotify, sizeof(L2S_ENTER_CLUB_NOTIFY));
    REGISTER_INTERNAL_FUNC(l2s_apply_join_club_respond, &KLSClient::OnApplyJoinClubRespond, sizeof(L2S_APPLY_JOIN_CLUB_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_club_invite_notify, &KLSClient::OnClubInviteNotify, sizeof(L2S_CLUB_INVITE_NOTIFY));
    REGISTER_INTERNAL_FUNC(l2s_post_modify, &KLSClient::OnPostModify, sizeof(L2S_POST_MODIFY));
    REGISTER_INTERNAL_FUNC(l2s_club_base_info, &KLSClient::OnClubBaseInfo, sizeof(L2S_CLUB_BASE_INFO));
    REGISTER_INTERNAL_FUNC(l2s_create_club_respond, &KLSClient::OnCreateClubRespond, sizeof(L2S_CREATE_CLUB_RESPOND));
    REGISTER_INTERNAL_FUNC(l2s_broadcast_to_client, &KLSClient::OnBroadcastToClient, sizeof(L2S_BROADCAST_TO_CLIENT));
    //AutoCode:注册协议
}

#define REGISTER_RPC_FUN(Func) \
    bRetCode = m_pRpcProcess->Register(#Func, &KLSClient::Func); \
    KGLOG_PROCESS_ERROR(bRetCode);

BOOL KLSClient::Init()
{
    BOOL               bResult          = false;
    BOOL               bRetCode         = false;
    IIniFile*	       piIniFile        = NULL;
    
    m_pRpcProcess = new KRPCProcesser<KLSClient>(this);

    REGISTER_RPC_FUN(OnCheckCanPresentResult);
	
    piIniFile = g_OpenIniFile(GS_SETTINGS_FILENAME);
	KGLOG_PROCESS_ERROR(piIniFile);

    bRetCode = piIniFile->GetString("GS-LS", "IP", "127.0.0.1", m_szLogicAddr, sizeof(m_szLogicAddr));
    KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piIniFile->GetInteger("GS-LS", "Port", 5004, &m_nRelayPort);
    KGLOG_PROCESS_ERROR(bRetCode);

	bRetCode = piIniFile->GetInteger("GS-LS", "PingCycle", 20, &m_nPingCycle);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_bRunFlag = true;

    bRetCode = m_WorkThread.Create(WorkThreadFunction, this);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        KG_COM_RELEASE(m_piSocketStream);
    }
    
    KG_COM_RELEASE(piIniFile);
    return bResult;
}

void KLSClient::WorkThreadFunction(void* pvParam)
{
    KLSClient* pThis = (KLSClient*)pvParam;

    assert(pThis);

    pThis->ThreadFunction();
}

BOOL KLSClient::Connect()
{
	BOOL bResult    = false;
    BOOL bRetCode   = false;
    KG_SocketConnector Connector;
    struct timeval     TimeVal;
    IKG_SocketStream*	    piSocketStream = NULL;
    IKG_Buffer*             piPackage       = NULL;
    S2L_HANDSHAKE_REQUEST*  pHandshake      = NULL;

    piSocketStream = Connector.Connect(m_szLogicAddr, m_nRelayPort);
    KG_PROCESS_ERROR(piSocketStream);

    TimeVal.tv_sec  = 1;
    TimeVal.tv_usec = 0;

    bRetCode = piSocketStream->SetTimeout(&TimeVal);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_bSocketError        = false;
    m_nWorldIndex         = 0;

    // 初始化的一些操作，注意多线程
    piPackage = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_HANDSHAKE_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pHandshake = (S2L_HANDSHAKE_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pHandshake);

    pHandshake->wProtocolID         = s2l_handshake_request;
    pHandshake->nServerTime         = (int)time(NULL);

    bRetCode = piSocketStream->Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode == 1);

    // 小心: 这里不能写成 "m_piSocket = piSocket; m_piSocket->AddRef(); ", 那样会导致线程安全隐患
    piSocketStream->AddRef();
    m_piSocketStream = piSocketStream;

    KGLogPrintf(
        KGLOG_INFO, "Connect to logic server %s:%d ... ... [OK]", m_szLogicAddr, m_nRelayPort
    );

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    KG_COM_RELEASE(piSocketStream);
    return bResult;
}

int KLSClient::ThreadFunction()
{
    while (m_bRunFlag)
    {
        KGThread_Sleep(10000);

        if (m_piSocketStream != NULL)
            continue;

        Connect();
    }

    return 0;
}


void KLSClient::UnInit(void)
{
    m_bRunFlag = false;
    m_WorkThread.Destroy();
    KG_COM_RELEASE(m_piSocketStream);
    SAFE_DELETE(m_pRpcProcess);
}

void KLSClient::Activate()
{
}

BOOL KLSClient::ProcessPackage()
{
    BOOL                    bResult             = false;
	int                     nRetCode            = false;
    IKG_Buffer*             piBuffer            = NULL;

    KG_PROCESS_ERROR(m_piSocketStream);

	while (true)
	{
	    const struct timeval        TimeVal     = {0, 0};
        INTERNAL_PROTOCOL_HEADER*   pHeader     = NULL;
        size_t                      uPakSize    = 0;
        PROCESS_PROTOCOL_FUNC       pFunc       = NULL;
        
		if (g_pSO3World->m_nCurrentTime - m_nLastSendPacketTime > m_nPingCycle)
		{
			DoPingSignal();
		}

	    nRetCode = m_piSocketStream->CheckCanRecv(&TimeVal);
        if (nRetCode == -1)
        {
            m_bSocketError = true;
            goto Exit0;
        }
        if (nRetCode == 0)
        {
            break;
        }

        KGLOG_PROCESS_ERROR(nRetCode == 1);

        KG_COM_RELEASE(piBuffer);

		nRetCode = m_piSocketStream->Recv(&piBuffer);
        if (nRetCode == -1)
        {
            m_bSocketError = true;
            goto Exit0;
        }
		KGLOG_PROCESS_ERROR(nRetCode == 1);

		pHeader = (INTERNAL_PROTOCOL_HEADER*)piBuffer->GetData();
        KGLOG_PROCESS_ERROR(pHeader);
          
        KGLOG_PROCESS_ERROR(pHeader->wProtocolID < l2s_protocol_end);
        
        uPakSize = piBuffer->GetSize();
        KGLOG_PROCESS_ERROR(uPakSize >= m_uProtocolSize[pHeader->wProtocolID]);

        pFunc = m_ProcessProtocolFuns[pHeader->wProtocolID];
        if (pFunc == NULL)
        {
            KGLogPrintf(KGLOG_INFO, "Protocol %d not process!", pHeader->wProtocolID);
            goto Exit0;
        }

		(this->*pFunc)((BYTE*)pHeader, uPakSize);
	}
     
    bResult = true;
Exit0:
    if (m_piSocketStream && m_bSocketError)
    {
        KGLogPrintf(KGLOG_ERR, "Logic server connection lost!\n");
        KG_COM_RELEASE(m_piSocketStream);
    }
    KG_COM_RELEASE(piBuffer);
	return bResult;
}

BOOL KLSClient::Send(IKG_Buffer* piBuffer)
{
    BOOL                        bResult      = false;
    int                         nRetCode     = false;
    INTERNAL_PROTOCOL_HEADER*   pHeader      = NULL;
    size_t                      uPakSize     = 0;

    assert(piBuffer);
    KG_PROCESS_SUCCESS(m_piSocketStream == NULL);
    KG_PROCESS_ERROR(!m_bSocketError);

    uPakSize = piBuffer->GetSize();
    assert(uPakSize >= sizeof(INTERNAL_PROTOCOL_HEADER));

    pHeader = (INTERNAL_PROTOCOL_HEADER*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pHeader);

    assert(pHeader->wProtocolID > s2l_protocol_begin);
    assert(pHeader->wProtocolID < s2l_protocol_end);

	nRetCode = m_piSocketStream->Send(piBuffer);
	if (nRetCode != 1)
    {
        m_bSocketError = true;
        goto Exit0;
    }

    m_nLastSendPacketTime = g_pSO3World->m_nCurrentTime;

Exit1:
    bResult = true;
Exit0:
	return bResult;
}

void KLSClient::DoRemoteCall(const char* pcszFunName, KVARIABLE param1, KVARIABLE param2, KVARIABLE param3, KVARIABLE param4)
{
    BOOL						bResult				= false;
    BOOL						bRetCode			= false;
    IKG_Buffer*                 piSendBuffer        = NULL;
    S2L_REMOTE_CALL*			pPak		    	= NULL;
    size_t                      uParamSize          = 0;
    size_t                      uUsedSize           = 0;
    BYTE*                       pbyOffset           = NULL;
    size_t                      uLeftSize           = 0;

    assert(pcszFunName);

    uParamSize = param1.GetPakSize() + param2.GetPakSize() + param3.GetPakSize() + param4.GetPakSize();

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_REMOTE_CALL) + uParamSize);
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_REMOTE_CALL*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID = s2l_remote_call;
    pPak->dwFunNameHashID = g_StringHash(pcszFunName);

    pbyOffset = pPak->byParamData;
    uLeftSize = uParamSize;

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
    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);   

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return;
}

// 发送协议
BOOL KLSClient::DoPingSignal()
{
    BOOL						bResult				= false;
    BOOL						bRetCode			= false;
    IKG_Buffer*                 piSendBuffer        = NULL;
    S2L_PING_SIGNAL*			pPingSingal			= NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_PING_SIGNAL));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPingSingal = (S2L_PING_SIGNAL*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPingSingal);

    pPingSingal->wProtocolID = s2l_ping_signal;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);   

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoTalkMessage(
    int nMsgType, DWORD dwTalkerID, const char cszTalker[],
    DWORD dwReceiverID, const char cszReceiver[], 
    size_t uDataLen, const char cszTalkData[]
)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    IKG_Buffer*         piSendBuffer    = NULL;
    S2L_TALK_MESSAGE*   pTalkMsg        = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)(sizeof(S2L_TALK_MESSAGE) + uDataLen));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pTalkMsg = (S2L_TALK_MESSAGE*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pTalkMsg);

    pTalkMsg->wProtocolID       = s2l_talk_message;
    pTalkMsg->byMsgType         = (BYTE)nMsgType;
    pTalkMsg->dwTalkerID        = dwTalkerID;
    pTalkMsg->dwReceiverID      = dwReceiverID;

    pTalkMsg->szTalker[0] = '\0';
    if (cszTalker)
    {
        strncpy(pTalkMsg->szTalker, cszTalker, sizeof(pTalkMsg->szTalker));
        pTalkMsg->szTalker[sizeof(pTalkMsg->szTalker) - 1] = '\0';
    }

    pTalkMsg->szReceiver[0] = '\0';
    if (cszReceiver)
    {
        strncpy(pTalkMsg->szReceiver, cszReceiver, sizeof(pTalkMsg->szReceiver));
        pTalkMsg->szReceiver[sizeof(pTalkMsg->szReceiver) - 1] = '\0';
    }

    memcpy(pTalkMsg->byTalkData, cszTalkData, uDataLen);

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}


BOOL KLSClient::DoApplyFellowshipDataRequest(DWORD dwPlayerID)
{
    BOOL                                bResult         = false;
    BOOL                                bRetCode        = false;
    IKG_Buffer*                         piSendBuffer    = NULL;
    S2L_APPLY_FELLOWSHIP_DATA_REQUEST*  pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)(sizeof(S2L_APPLY_FELLOWSHIP_DATA_REQUEST)));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_APPLY_FELLOWSHIP_DATA_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID                = s2l_apply_fellowship_data_request;
    pPak->dwPlayerID                 = dwPlayerID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoUpdateFellowshipData(DWORD dwPlayerID, size_t uDataSize, BYTE byData[])
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piSendBuffer    = NULL;
    S2L_UPDATE_FELLOWSHIP_DATA* pPak            = NULL;

    assert(dwPlayerID != ERROR_ID);

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)(sizeof(S2L_UPDATE_FELLOWSHIP_DATA) + uDataSize));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_UPDATE_FELLOWSHIP_DATA*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_update_fellowship_data;
    pPak->dwPlayerID        = dwPlayerID;
    memcpy(pPak->byData, byData, uDataSize);

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoPlayerAddFellowshipRequest(DWORD dwPlayerID, char szAlliedPlayerName[], int nType)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piSendBuffer    = NULL;
    S2L_ADD_FELLOWSHIP_REQUEST* pPak            = NULL;

    assert(dwPlayerID != ERROR_ID);

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_ADD_FELLOWSHIP_REQUEST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_ADD_FELLOWSHIP_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID   = s2l_add_fellowship_request;
    pPak->dwPlayerID    = dwPlayerID;
    pPak->byType        = (BYTE)nType;

    strncpy(pPak->szAlliedPlayerName, szAlliedPlayerName, sizeof(pPak->szAlliedPlayerName));
    pPak->szAlliedPlayerName[sizeof(pPak->szAlliedPlayerName) - sizeof('\0')] = '\0';

    bRetCode = Send(piSendBuffer);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoGetFellowshipName(DWORD dwPlayerID, int nArrayLength, DWORD dwAlliedPlayerIDArray[])
{
    BOOL                                bResult         = false;
    BOOL                                bRetCode        = false;
    IKG_Buffer*                         piSendBuffer    = NULL;
    S2L_GET_FELLOWSHIP_NAME_REQUEST*    pPak            = NULL;

    assert(dwPlayerID != ERROR_ID);
    assert(nArrayLength >= 0);
    assert(nArrayLength <= UCHAR_MAX);

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_GET_FELLOWSHIP_NAME_REQUEST) + sizeof(DWORD) * nArrayLength);
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_GET_FELLOWSHIP_NAME_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID   = s2l_get_fellowship_name_request;
    pPak->dwPlayerID    = dwPlayerID;
    pPak->byCount       = (BYTE)nArrayLength;

    for (int i = 0; i < nArrayLength; i++)
    {
        pPak->dwAlliedPlayerIDArray[i] = dwAlliedPlayerIDArray[i];
    }

    bRetCode = Send(piSendBuffer);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoApplyRecommendFriend(DWORD dwPlayerID, int nLevel)
{
    BOOL                        	bResult         = false;
    BOOL                        	bRetCode        = false;
    IKG_Buffer*                 	piSendBuffer    = NULL;
    S2L_APPLY_RECOMMEND_FRIEND*     pPak            = NULL;

    assert(dwPlayerID != ERROR_ID);

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_APPLY_RECOMMEND_FRIEND));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_APPLY_RECOMMEND_FRIEND*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_apply_recommend_friend;
    pPak->dwPlayerID        = dwPlayerID;
    pPak->wLevel            = (WORD)nLevel;

    bRetCode = Send(piSendBuffer);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoRegFriendRecommend(DWORD dwPlayerID)
{
    BOOL                        	bResult         = false;
    BOOL                        	bRetCode        = false;
    IKG_Buffer*                 	piSendBuffer    = NULL;
    S2L_REGISTER_RECOMMEND_FRIEND*  pPak            = NULL;

    assert(dwPlayerID != ERROR_ID);

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_REGISTER_RECOMMEND_FRIEND));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_REGISTER_RECOMMEND_FRIEND*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_register_recommend_friend;
    pPak->dwPlayerID        = dwPlayerID;

    bRetCode = Send(piSendBuffer);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoRequestFriendHeroList(DWORD dwRequesterID, DWORD dwFriendID)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    L2S_REQUEST_FRIEND_HERO_LIST    Pak;
    
    Pak.wProtocolID = l2s_request_friend_hero_list;
    Pak.dwRequesterID = dwRequesterID;
    Pak.dwFriendID = dwFriendID; 

    bRetCode = DoSendToPlayerServer(dwFriendID, dwRequesterID, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KLSClient::DoRequestFriendHeroInfo(DWORD dwRequesterID, DWORD dwFriendID, DWORD dwHeroTemplateID)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    L2S_REQUEST_FRIEND_HERO_INFO    Pak;
    
    Pak.wProtocolID = l2s_request_friend_hero_info;
    Pak.dwRequesterID = dwRequesterID;
    Pak.dwFriendID = dwFriendID; 
    Pak.dwHeroTemplateID = dwHeroTemplateID;

    bRetCode = DoSendToPlayerServer(dwFriendID, dwRequesterID, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}



BOOL KLSClient::DoApplyFellowshipPlayerFellowInfo(DWORD dwPlayerID, BOOL bNotifyClient)
{
    BOOL                                        bResult         = false;
    BOOL                                        bRetCode        = false;
    IKG_Buffer*                                 piSendBuffer    = NULL;
    S2L_APPLY_FELLOWSHIP_PLAYER_FELLOW_INFO*    pPak            = NULL;

    assert(dwPlayerID != ERROR_ID);

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_APPLY_FELLOWSHIP_PLAYER_FELLOW_INFO));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_APPLY_FELLOWSHIP_PLAYER_FELLOW_INFO*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_apply_fellowship_player_fellow_info;
    pPak->dwPlayerID        = dwPlayerID;
    pPak->byNotifyClient    = (BYTE)bNotifyClient;

    bRetCode = Send(piSendBuffer);
    KG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoUpdateStatDataRequest()
{
    BOOL                            	bResult  = false;
    BOOL                            	bRetCode = false;
    HRESULT                             hRetCode = E_FAIL;
    IKG_Buffer*                     	piBuffer = NULL;
    IKG_Buffer_ReSize*                  piResize = NULL;
    S2L_UPDATE_STAT_DATA_REQUEST*	    pRequest = NULL;
    int                                 nCount   = 0;
    
    piBuffer = KG_MemoryCreateBuffer((unsigned)(sizeof(S2L_UPDATE_STAT_DATA_REQUEST) + sizeof(KSTAT_DATA_MODIFY) * MAX_UPDATE_STAT_COUNT));
    KGLOG_PROCESS_ERROR(piBuffer);

    pRequest = (S2L_UPDATE_STAT_DATA_REQUEST*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pRequest);
    
    pRequest->wProtocolID  = s2l_update_stat_data_request;

    nCount = g_pSO3World->m_StatDataServer.GetUpdateStatData(pRequest->Values, MAX_UPDATE_STAT_COUNT);
    KG_PROCESS_ERROR(nCount > 0);

    pRequest->nCount = nCount;

    hRetCode = piBuffer->QueryInterface(IID_IKG_Buffer_ReSize, (void**)&piResize);
    KGLOG_COM_PROCESS_ERROR(hRetCode);

    bRetCode = piResize->SetSmallerSize((unsigned)(sizeof(S2L_UPDATE_STAT_DATA_REQUEST) + sizeof(KSTAT_DATA_MODIFY) * nCount));
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piResize);
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL KLSClient::DoQueryStatIDRequest(const char cszName[])
{
    BOOL                                bResult  = false;
    BOOL                                bRetCode = false;
    IKG_Buffer*                         piBuffer = NULL;
    S2L_QUERY_STAT_ID_REQUEST*          pRequest = NULL;

    piBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_QUERY_STAT_ID_REQUEST));
    KGLOG_PROCESS_ERROR(piBuffer);

    pRequest = (S2L_QUERY_STAT_ID_REQUEST*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID = s2l_query_stat_id_request;

    strncpy(pRequest->szName, cszName, sizeof(pRequest->szName));
    pRequest->szName[sizeof(pRequest->szName) - 1] = '\0';

    bRetCode = Send(piBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL KLSClient::DoClubInviteRequest(DWORD dwPlayerID, DWORD dwClubID, const char cszTargetName[])
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_CLUB_INVITE*            pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_CLUB_INVITE));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_CLUB_INVITE*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_club_invite;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwClubID         = dwClubID;
	
    strncpy(pPak->szName, cszTargetName, sizeof(pPak->szName));
    pPak->szName[sizeof(pPak->szName) - 1] = '\0';

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}


BOOL KLSClient::DoClubAcceptApplyRequest(DWORD dwPlayerID, DWORD dwClubID, DWORD dwTargetID, BOOL bAccept)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_ACCEPT_APPLY*             pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_ACCEPT_APPLY));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_ACCEPT_APPLY*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_accept_apply;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwClubID         = dwClubID;
    pPak->dwTargetID       = dwTargetID;
    pPak->byType           = (BYTE)bAccept;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoApplyJoinClubRequest(DWORD dwPlayerID, DWORD dwClubID)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_APPLY_JOIN_CLUB*             pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_APPLY_JOIN_CLUB));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_APPLY_JOIN_CLUB*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_apply_join_club;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwClubID         = dwClubID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoAcceptClubInviteRequest(DWORD dwPlayerID, DWORD dwClubID, DWORD dwLauncherID, int nRepsond)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_ACCEPT_INVITE*              pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_ACCEPT_INVITE));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_ACCEPT_INVITE*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_accept_invite;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwClubID         = dwClubID;
    pPak->dwLauncherID     = dwLauncherID;
    pPak->byRespond        = (BYTE)nRepsond;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoSetDescriptRequest(DWORD dwPlayerID, DWORD dwClubID, int nType, int nTxtLen, const char cszText[])
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_SET_CLUB_TXT*           pPak         = NULL;

    nTxtLen = min(nTxtLen, 360);

    if (nType == eClub_Txt_Remark)
        nTxtLen = min(nTxtLen, 75);

    if (nType == eClub_Txt_Intro)
        nTxtLen = min(nTxtLen, 150);

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_SET_CLUB_TXT) + nTxtLen + 1);
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_SET_CLUB_TXT*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_set_club_txt;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwClubID         = dwClubID;
    pPak->wTxtType         = (WORD)nType;
    pPak->wTxtLen          = (WORD)nTxtLen;
	
    memcpy(pPak->szText, cszText, nTxtLen);
    pPak->szText[nTxtLen] = '\0';

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoClubSetPostRequest(DWORD dwPlayerID, DWORD dwClubID, DWORD dwUnderID, int nPost)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_SET_POST*               pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_SET_POST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_SET_POST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_set_club_post;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwClubID         = dwClubID;
    pPak->dwUnderID        = dwUnderID;
    pPak->byPost           = (BYTE)nPost;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}


BOOL KLSClient::DoApplyClubMemberList(DWORD dwPlayerID, DWORD dwClubID, DWORD dwVersion)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_GET_MEMBER_LIST*            pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_GET_MEMBER_LIST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_GET_MEMBER_LIST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_get_member_list;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwClubID         = dwClubID;
    pPak->dwVersion        = dwVersion;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}


BOOL KLSClient::DoApplyClubBaseInfo(DWORD dwPlayerID, BOOL bIsLogin)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_CLUB_BASE_INFO*         pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_CLUB_BASE_INFO));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_CLUB_BASE_INFO*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_club_base_info;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->bIsLogin         = (BYTE)bIsLogin;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoCreateClubRequest(DWORD dwPlayerID, const char cszClubName[])
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_CREATE_CLUB*            pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_CREATE_CLUB));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_CREATE_CLUB*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_create_club;
    pPak->dwPlayerID       = dwPlayerID;

    strncpy(pPak->szClubName, cszClubName, sizeof(pPak->szClubName));
    pPak->szClubName[sizeof(pPak->szClubName) - 1] = '\0';

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoSwitchApplyRequest(DWORD dwPlayerID, DWORD dwSwitch)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_SWITCH_APPLY*           pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_SWITCH_APPLY));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_SWITCH_APPLY*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID  = s2l_switch_apply;
    pPak->bySwitch     = (BYTE)dwSwitch;
    pPak->dwPlayerID   = dwPlayerID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoFindClubRequest(DWORD dwPlayerID, const char cszFindName[])
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_FIND_CLUB*              pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_FIND_CLUB));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_FIND_CLUB*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID  = s2l_find_club;
    pPak->dwPlayerID   = dwPlayerID;
    strncpy(pPak->szFindName, cszFindName, _NAME_LEN);
    pPak->szFindName[_NAME_LEN - 1] = '\0';

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoGetApplyNumRequest(DWORD dwPlayerID)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_APPLY_NUM*              pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_APPLY_NUM));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_APPLY_NUM*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_get_apply_num;
    pPak->dwPlayerID       = dwPlayerID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoGetApplyListRequest(DWORD dwPlayerID)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_APPLY_LIST*             pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_APPLY_LIST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_APPLY_LIST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_get_apply_list;
    pPak->dwPlayerID       = dwPlayerID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoRandomClubRequest(DWORD dwPlayerID)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_RANDOM_CLUB*            pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_RANDOM_CLUB));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_RANDOM_CLUB*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_random_club;
    pPak->dwPlayerID       = dwPlayerID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoGetClubList(DWORD dwPlayerID, DWORD dwPageIndex)
{
	BOOL                        bResult      = false;
	BOOL                        bRetCode     = false;
	IKG_Buffer*                 piSendBuffer = NULL;
	S2L_GET_CLUB_LIST*          pPak         = NULL;

	piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_GET_CLUB_LIST));
	KGLOG_PROCESS_ERROR(piSendBuffer);

	pPak = (S2L_GET_CLUB_LIST*)piSendBuffer->GetData();
	KGLOG_PROCESS_ERROR(pPak);

	pPak->wProtocolID      = s2l_get_club_list;
	pPak->dwPageIndex      = dwPageIndex;

	bRetCode = Send(piSendBuffer);
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	KG_COM_RELEASE(piSendBuffer);
	return bResult;
}

BOOL KLSClient::DoAddMemberRequest(DWORD dwPlayerID, DWORD dwClubID)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_ADD_MEMBER*             pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_ADD_MEMBER));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_ADD_MEMBER*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_add_member;
    pPak->dwClubID         = dwClubID;
    pPak->dwPlayerID       = dwPlayerID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoDelMemberRequest(DWORD dwPlayerID, DWORD dwClubID, DWORD dwTargetID)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_DEL_MEMBER*             pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_DEL_MEMBER));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_DEL_MEMBER*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_del_member;
    pPak->dwClubID         = dwClubID;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwTargetID       = dwTargetID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}
 
// ------------------- 邮件系统 ------------------------------->

BOOL KLSClient::DoSendMailRequest(int nRequestID, const char cszDstName[], KMail* pMail, size_t uMailLen)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piSendBuffer    = NULL;
    S2L_SEND_MAIL_REQUEST*  pPak            = NULL;

    assert(cszDstName);
    assert(pMail);

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)(sizeof(S2L_SEND_MAIL_REQUEST) + uMailLen));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_SEND_MAIL_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID   = s2l_send_mail_request;
    pPak->byRequestID   = (BYTE)nRequestID;

    strncpy(pPak->szReceiverName, cszDstName, sizeof(pPak->szReceiverName));
    pPak->szReceiverName[sizeof(pPak->szReceiverName) - 1] = '\0';

    memcpy(pPak->byData, pMail, uMailLen);

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoSendGlobalMailRequest(
    KMail* pMail, size_t uMailLen, int nLifeTime
)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    IKG_Buffer*                     piSendBuffer    = NULL;
    S2L_SEND_GLOBAL_MAIL_REQUEST*   pPak            = NULL;

    assert(pMail);

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)(sizeof(S2L_SEND_GLOBAL_MAIL_REQUEST) + uMailLen));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_SEND_GLOBAL_MAIL_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_send_global_mail_request;
    pPak->nLifeTime 		= nLifeTime;

    memcpy(pPak->byData, pMail, uMailLen);

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoGetMailListRequest(DWORD dwPlayerID, DWORD dwStartID)
{
    BOOL                        bResult      = false;
    BOOL                        bRetCode     = false;
    IKG_Buffer*                 piSendBuffer = NULL;
    S2L_GET_MAILLIST_REQUEST*   pPak         = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_GET_MAILLIST_REQUEST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_GET_MAILLIST_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID      = s2l_get_maillist_request;
    pPak->dwPlayerID       = dwPlayerID;
    pPak->dwStartID        = dwStartID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoQueryMailContent(DWORD dwPlayerID, DWORD dwMailID, int nMailType)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piSendBuffer    = NULL;
    S2L_QUERY_MAIL_CONTENT* pPak            = NULL;
    
    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_QUERY_MAIL_CONTENT));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_QUERY_MAIL_CONTENT*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_query_mail_content;
    pPak->dwPlayerID        = dwPlayerID;
    pPak->dwMailID          = dwMailID;
    pPak->byMailType        = (BYTE)nMailType;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoAcquireMailMoneyRequest(DWORD dwPlayerID, DWORD dwMailID)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    IKG_Buffer*                     piSendBuffer    = NULL;
    S2L_ACQUIRE_MAIL_MONEY_REQUEST* pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_ACQUIRE_MAIL_MONEY_REQUEST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_ACQUIRE_MAIL_MONEY_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID   = s2l_acquire_mail_money_request;
    pPak->dwPlayerID    = dwPlayerID;
    pPak->dwMailID      = dwMailID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoAcquireMailAllRequest(DWORD dwPlayerID, DWORD dwMailID)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    IKG_Buffer*                     piSendBuffer    = NULL;
    S2L_ACQUIRE_ALL_MONEY_REQUEST*   pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2L_ACQUIRE_ALL_MONEY_REQUEST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_ACQUIRE_ALL_MONEY_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID   = s2l_acquire_mail_all_request;
    pPak->dwPlayerID    = dwPlayerID;
    pPak->dwMailID      = dwMailID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoAcquireMailItemRequest(DWORD dwPlayerID, DWORD dwMailID, int nIndex)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    IKG_Buffer*                     piSendBuffer    = NULL;
    S2L_ACQUIRE_MAIL_ITEM_REQUEST*  pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_ACQUIRE_MAIL_ITEM_REQUEST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_ACQUIRE_MAIL_ITEM_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID   = s2l_acquire_mail_item_request;
    pPak->dwPlayerID    = dwPlayerID;
    pPak->dwMailID      = dwMailID;
    pPak->byIndex       = (BYTE)nIndex;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoGiveMailAllToPlayerRespond(DWORD dwPlayerID, DWORD dwMailID, KMAIL_RESULT_CODE eResult)
{
    BOOL                                    bResult         = false;
    BOOL                                    bRetCode        = false;
    IKG_Buffer*                             piSendBuffer    = NULL;
    S2L_GIVE_MAIL_ALL_TO_PLAYER_RESPOND*    pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_GIVE_MAIL_ALL_TO_PLAYER_RESPOND));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_GIVE_MAIL_ALL_TO_PLAYER_RESPOND*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_give_mail_all_to_player;
    pPak->dwPlayerID        = dwPlayerID;
    pPak->dwMailID          = dwMailID;
    pPak->byResult          = (BYTE)eResult;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoGiveMailMoneyToPlayerRespond(DWORD dwPlayerID, DWORD dwMailID, KMAIL_RESULT_CODE eResult)
{
    BOOL                                    bResult         = false;
    BOOL                                    bRetCode        = false;
    IKG_Buffer*                             piSendBuffer    = NULL;
    S2L_GIVE_MAIL_MONEY_TO_PLAYER_RESPOND*  pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_GIVE_MAIL_MONEY_TO_PLAYER_RESPOND));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_GIVE_MAIL_MONEY_TO_PLAYER_RESPOND*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_give_mail_money_to_player;
    pPak->dwPlayerID        = dwPlayerID;
    pPak->dwMailID          = dwMailID;
    pPak->byResult          = (BYTE)eResult;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoGiveMailItemToPlayerRespond(DWORD dwPlayerID, DWORD dwMailID, int nIndex, KMAIL_RESULT_CODE eResult)
{
    BOOL                                    bResult         = false;
    BOOL                                    bRetCode        = false;
    IKG_Buffer*                             piSendBuffer    = NULL;
    S2L_GIVE_MAIL_ITEM_TO_PLAYER_RESPOND*   pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_GIVE_MAIL_ITEM_TO_PLAYER_RESPOND));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_GIVE_MAIL_ITEM_TO_PLAYER_RESPOND*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_give_mail_item_to_player;
    pPak->dwPlayerID        = dwPlayerID;
    pPak->dwMailID          = dwMailID;
    pPak->byIndex           = (BYTE)nIndex;
    pPak->byResult          = (BYTE)eResult;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoSetMailRead(DWORD dwPlayer, DWORD dwMailID)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piSendBuffer    = NULL;
    S2L_SET_MAIL_READ*      pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_SET_MAIL_READ));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_SET_MAIL_READ*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_set_mail_read;
    pPak->dwPlayerID        = dwPlayer;
    pPak->dwMailID          = dwMailID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoDeleteMail(DWORD dwPlayer, DWORD dwMailID)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piSendBuffer    = NULL;
    S2L_DELETE_MAIL*        pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2L_DELETE_MAIL));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_DELETE_MAIL*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID       = s2l_delete_mail;
    pPak->dwPlayerID        = dwPlayer;
    pPak->dwMailID          = dwMailID;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoWhisperSuccess(
    int nMsgType, uint32_t dwTalkerID, const char szTalker[], 
    uint32_t dwReceiverID, const char szReceiver[], size_t uDataLen, const char byTalkData[]
)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piSendBuffer    = NULL;
    S2L_WHISPER_SUCCESS*    pPak            = NULL;

    piSendBuffer = KG_MemoryCreateBuffer((unsigned)(sizeof(S2L_WHISPER_SUCCESS)) + uDataLen);
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPak = (S2L_WHISPER_SUCCESS*)piSendBuffer->GetData();

    pPak->wProtocolID       = s2l_whisper_success;
    pPak->byMsgType         = (BYTE)nMsgType;
    pPak->dwTalkerID        = dwTalkerID;
    pPak->dwReceiverID      = dwReceiverID;

    strncpy(pPak->szTalker, szTalker, countof(pPak->szTalker));
    pPak->szTalker[countof(pPak->szTalker) - 1] = '\0';

    strncpy(pPak->szReceiver, szReceiver, countof(pPak->szReceiver));
    pPak->szReceiver[countof(pPak->szReceiver) - 1] = '\0';

    memcpy(pPak->byTalkData, byTalkData, uDataLen);

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLSClient::DoUpdatePlayerGender(DWORD dwPlayerID, KGENDER eGender)
{
	BOOL bResult = false;
	BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_UPDATE_PLAYER_GENDER* pNotify = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_UPDATE_PLAYER_GENDER));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2L_UPDATE_PLAYER_GENDER*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID = s2l_update_player_gender;
    pNotify->dwPlayerID  = dwPlayerID;
    pNotify->byGender    = (BYTE)eGender;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}


BOOL KLSClient::DoCheckAndUpdateActivityStateRequest(DWORD dwPlayerID, DWORD dwActivityID, int nCurState, int nNewState)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_CHECK_UPDATE_ACTIVITY_STATE_REQUEST* pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_CHECK_UPDATE_ACTIVITY_STATE_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_CHECK_UPDATE_ACTIVITY_STATE_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID = s2l_check_update_activity_state_request;
    pRequest->dwPlayerID = dwPlayerID;
    pRequest->dwActivityID = dwActivityID;
    pRequest->nCurState = nCurState;
    pRequest->nNewState = nNewState;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoPresentVIPTimeToFriendRequest(DWORD dwSrcPlayerID, DWORD dwFirendID, int nChargeType)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_PRESENT_VIP_TIME_TO_FRIEND_REQUEST* pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_PRESENT_VIP_TIME_TO_FRIEND_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_PRESENT_VIP_TIME_TO_FRIEND_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID       = s2l_present_vip_time_to_friend_request;
    pRequest->dwSrcPlayerID     = dwSrcPlayerID;
    pRequest->byChargeType      = (BYTE)nChargeType;
    pRequest->dwDestPlayerID    = dwFirendID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoGetVIPPresentNotify(uint32_t dwSNID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_GET_VIP_PRESENT* pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_GET_VIP_PRESENT));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_GET_VIP_PRESENT*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_get_vip_present;
    pRequest->dwSNID        = dwSNID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}


BOOL KLSClient::DoBroadcastMessage(DWORD dwMessageID, void* pData /*= NULL*/, size_t uDataLen /*= 0*/)
{
    BOOL                    bResult  = false;
    BOOL                    bRetCode = false;
    IKG_Buffer*             piPackage = NULL;
    KS2C_Downward_Notify*   pPak     = NULL;
    size_t                  uPakSize = sizeof(KS2C_Downward_Notify) + uDataLen;

    piPackage = KG_MemoryCreateBuffer(uPakSize);
    KGLOG_PROCESS_ERROR(piPackage);

    pPak = (KS2C_Downward_Notify*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->protocolID  = s2c_downward_notify;
    pPak->messageID   = dwMessageID;
    pPak->paramLength = (WORD)uDataLen;
    if (pData)
    {
        memcpy(pPak->param, pData, uDataLen);
    }

    bRetCode = DoBroadcast(pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult; 
}

BOOL KLSClient::DoBroadcast(void* pData, size_t uSize)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_BROADCAST* pRequest = NULL;
    size_t uPakSize = 0;

    KGLOG_PROCESS_ERROR(pData);
    KGLOG_PROCESS_ERROR(uSize > 0);

    uPakSize = sizeof(S2L_BROADCAST) + uSize;

    piPackage = KG_MemoryCreateBuffer(uPakSize);
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_BROADCAST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID       = s2l_broadcast;
    memcpy(pRequest->pData, pData, uSize);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);
	
	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}


struct BroadcastToFriend
{
    BroadcastToFriend()
    {
        pData = NULL;
        uSize = 0;
    }

    bool operator () (DWORD dwPlayerID, DWORD dwAlliedPlayerID)
    {
        KGFellowship*   pFellowship     = NULL;

        pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(dwPlayerID, dwAlliedPlayerID);
        if (!pFellowship)
            goto Exit1;

        pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(dwAlliedPlayerID, dwPlayerID);
        if (!pFellowship)
            goto Exit1;

        g_LSClient.DoSendToPlayer(dwAlliedPlayerID, pData, uSize);

    Exit1:
        return true;
    }
    void* pData;
    size_t uSize;
};

BOOL KLSClient::DoSendMessageToAllFriend(DWORD dwPlayerID, DWORD dwMessageID, void* pData /*= NULL*/, size_t uDataLen /*= 0*/)
{
    BOOL                    bResult  = false;
    BOOL                    bRetCode = false;
    IKG_Buffer*             piPackage = NULL;
    KS2C_Downward_Notify*   pPak     = NULL;
    size_t                  uPakSize = sizeof(KS2C_Downward_Notify) + uDataLen;
    BroadcastToFriend       BroadcastFunc;

    if (uDataLen > 0)
    {
        KGLOG_PROCESS_ERROR(pData);
    }

    piPackage = KG_MemoryCreateBuffer(uPakSize);
    KGLOG_PROCESS_ERROR(piPackage);

    pPak = (KS2C_Downward_Notify*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->protocolID  = s2c_downward_notify;
    pPak->messageID   = dwMessageID;
    pPak->paramLength = (WORD)uDataLen;
    if (pData)
    {
        memcpy(pPak->param, pData, uDataLen);
    }

    BroadcastFunc.pData = pPak;
    BroadcastFunc.uSize = uPakSize;

    g_pSO3World->m_FellowshipMgr.TraverseFellowshipID(dwPlayerID, BroadcastFunc);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult; 
}

BOOL KLSClient::DoSendToPlayerServer(DWORD dwTargetID, DWORD dwRespondID, void* pData, size_t uSize)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_SEND_TO_PLAYER_SERVER* pRequest = NULL;
    size_t uPakSize = 0;

    KGLOG_PROCESS_ERROR(pData);
    KGLOG_PROCESS_ERROR(uSize > 0);

    uPakSize = sizeof(S2L_SEND_TO_PLAYER_SERVER) + uSize;

    piPackage = KG_MemoryCreateBuffer(uPakSize);
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_SEND_TO_PLAYER_SERVER*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_send_to_player_server;
    pRequest->dwTargetID    = dwTargetID;
    pRequest->dwRespondID   = dwRespondID;
    memcpy(pRequest->pData, pData, uSize);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}
BOOL KLSClient::DoSendToPlayer(DWORD dwTargetID, void* pData, size_t uSize)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_SEND_TO_PLAYER* pRequest = NULL;
    size_t uPakSize = 0;

    KGLOG_PROCESS_ERROR(pData);
    KGLOG_PROCESS_ERROR(uSize > 0);

    uPakSize = sizeof(S2L_SEND_TO_PLAYER) + uSize;

    piPackage = KG_MemoryCreateBuffer(uPakSize);
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_SEND_TO_PLAYER*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_send_to_player;
    pRequest->dwTargetID    = dwTargetID;
    memcpy(pRequest->pData, pData, uSize);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoSendMessageToClub(DWORD dwClubID, DWORD dwMessageID, void* pData /*= NULL*/, size_t uDataLen /*= 0*/)
{
    BOOL                    bResult  = false;
    BOOL                    bRetCode = false;
    IKG_Buffer*             piPackage = NULL;
    KS2C_Downward_Notify*   pPak     = NULL;
    size_t                  uPakSize = sizeof(KS2C_Downward_Notify) + uDataLen;

    if (uDataLen > 0)
    {
        KGLOG_PROCESS_ERROR(pData);
    }

    piPackage = KG_MemoryCreateBuffer(uPakSize);
    KGLOG_PROCESS_ERROR(piPackage);

    pPak = (KS2C_Downward_Notify*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->protocolID  = s2c_downward_notify;
    pPak->messageID   = dwMessageID;
    pPak->paramLength = (WORD)uDataLen;
    if (pData)
        memcpy(pPak->param, pData, uDataLen);

    bRetCode = DoBroadcastInClub(dwClubID, ERROR_ID, pPak, uPakSize);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult; 
}

BOOL KLSClient::DoBroadcastInClub(DWORD dwClubID, DWORD dwExceptID, void* pData, size_t uSize)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_BROADCAST_IN_CLUB* pRequest = NULL;
    size_t uPakSize = 0;

    KGLOG_PROCESS_ERROR(pData);
    KGLOG_PROCESS_ERROR(uSize > 0);

    uPakSize = sizeof(S2L_BROADCAST_IN_CLUB) + uSize;

    piPackage = KG_MemoryCreateBuffer(uPakSize);
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_BROADCAST_IN_CLUB*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_broadcast_in_club;
    pRequest->dwClubID      = dwClubID;
    pRequest->dwExceptID    = dwExceptID;
    memcpy(pRequest->pData, pData, uSize);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoRoleLoginHallNotify(DWORD dwPlayerID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_ROLE_LOGIN_HALL_NOTIFY* pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_ROLE_LOGIN_HALL_NOTIFY));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_ROLE_LOGIN_HALL_NOTIFY*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_role_login_hall_notify;
    pRequest->dwPlayerID    = dwPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoBeAddedFriendNotify(DWORD dwPlayerID, KPlayer* pSrcPlayer)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    L2S_BEADDED_FRIEND_NOTIFY Pak;

	KGLOG_PROCESS_ERROR(pSrcPlayer);

    Pak.wProtocolID = l2s_beadded_friend_notify;
    Pak.dwPlayerID = dwPlayerID;
    strncpy(Pak.szPlayerName, pSrcPlayer->m_szName, countof(Pak.szPlayerName));
    Pak.szPlayerName[countof(Pak.szPlayerName) - 1] = '\0';

    bRetCode = DoSendToPlayerServer(dwPlayerID, 0, &Pak, sizeof(Pak));
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KLSClient::DoFindRoleInTopListRequest(DWORD dwRoleID, int nTopListType, int nGroup, char (&szTargetRoleName)[_NAME_LEN])
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_FIND_ROLE_IN_TOPLIST* pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_FIND_ROLE_IN_TOPLIST));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_FIND_ROLE_IN_TOPLIST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_find_role_in_toplist;
    pRequest->dwRoleID      = dwRoleID;
	pRequest->nGroup		= nGroup;
    pRequest->wTopListType  = (uint16_t)nTopListType;
	
    strncpy(pRequest->szTargetRoleName, szTargetRoleName, countof(pRequest->szTargetRoleName));
    pRequest->szTargetRoleName[countof(pRequest->szTargetRoleName) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoRequestTopListPage(DWORD dwRoleID, int nTopListType, int nGroup, int nPageIndex)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_REQUEST_TOPLIST_PAGE* pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_REQUEST_TOPLIST_PAGE));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_REQUEST_TOPLIST_PAGE*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_request_toplist_page;
    pRequest->dwRoleID      = dwRoleID;
	pRequest->nGroup		= nGroup;
    pRequest->wTopListType  = (uint16_t)nTopListType;
    pRequest->wPageIndex    = (uint16_t)nPageIndex;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoLadderDataChangeNotify(KPlayer* pPlayer, DWORD dwHeroTemplateID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_LADDERDATA_CHANGED_NOTIFY* pNotify = NULL;
    KHeroData* pHeroData = NULL;
    int nTotalExp = 0;

    KGLOG_PROCESS_ERROR(pPlayer);

    pHeroData = pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_LADDERDATA_CHANGED_NOTIFY));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2L_LADDERDATA_CHANGED_NOTIFY*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID        = s2l_ladderdata_changed_notify;
    pNotify->dwRoleID           = pPlayer->m_dwID;
    pNotify->wHeroTemplateID    = (uint16_t)pHeroData->m_dwTemplateID;
    pNotify->nVIPEndTime        = pPlayer->m_nVIPEndTime;

    bRetCode = g_pSO3World->m_Settings.m_LadderLevelData.GetTotalLadderExp(pHeroData->m_nLadderLevel, nTotalExp);
    KGLOG_PROCESS_ERROR(bRetCode);
    nTotalExp += pHeroData->m_nLadderExp;

    pNotify->nLadderExp         = nTotalExp;
    pNotify->wWinRate           = (uint16_t)pHeroData->GetLadderWinRate();
    pNotify->wHeroNormalLevel   = (uint16_t)pHeroData->m_nLevel;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoHeroEquipScoreChangeNotify( KPlayer* pPlayer, DWORD dwHeroTemplateID )
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	IKG_Buffer* piPackage = NULL;
	S2L_HERO_EQUIP_SCORE_CHANGED_NOTIFY* pNotify = NULL;
	KHeroData* pHeroData = NULL;
	KHeroPackage*	pHeroPackage = NULL;

	KG_PROCESS_ERROR(pPlayer);

	pHeroData = pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
	KGLOG_PROCESS_ERROR(pHeroData);

	pHeroPackage = pPlayer->m_ItemList.GetHeroPackage(dwHeroTemplateID);
	KGLOG_PROCESS_ERROR(pHeroPackage);

	piPackage = KG_MemoryCreateBuffer(sizeof(S2L_HERO_EQUIP_SCORE_CHANGED_NOTIFY));
	KGLOG_PROCESS_ERROR(piPackage);

	pNotify = (S2L_HERO_EQUIP_SCORE_CHANGED_NOTIFY*)piPackage->GetData();
	KGLOG_PROCESS_ERROR(pNotify);

	pNotify->wProtocolID = s2l_hero_equip_score_changed_notify;
	pNotify->dwRoleID = pPlayer->m_dwID;
	pNotify->wHeroID = (uint16_t)dwHeroTemplateID;
	pNotify->wHeroLevel = (uint16_t)pHeroData->m_nLevel;
	pNotify->nHeroSocre = pHeroPackage->GetAllEquipScore();
	pNotify->nTeamScore = pPlayer->m_ItemList.GetAllHeroEquipScore();
	pNotify->nMaxStrengthen = pHeroPackage->GetMaxStrengthenLevel();
	pNotify->nVIPEndTime = pPlayer->m_nVIPEndTime;

	bRetCode = Send(piPackage);
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	KG_COM_RELEASE(piPackage);
	return bResult;
}


BOOL KLSClient::DoTeamEquipScoreChangeNotify( KPlayer* pPlayer )
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	IKG_Buffer* piPackage = NULL;
	S2L_TEAM_EQUIP_SCORE_CHANGED_NOTIFY* pNotify = NULL;

	KG_PROCESS_ERROR(pPlayer);

	piPackage = KG_MemoryCreateBuffer(sizeof(S2L_TEAM_EQUIP_SCORE_CHANGED_NOTIFY));
	KGLOG_PROCESS_ERROR(piPackage);

	pNotify = (S2L_TEAM_EQUIP_SCORE_CHANGED_NOTIFY*)piPackage->GetData();
	KGLOG_PROCESS_ERROR(pNotify);

	pNotify->wProtocolID = s2l_team_equip_score_changed_notify;
	pNotify->dwRoleID = pPlayer->m_dwID;
	pNotify->nTeamScore = pPlayer->m_ItemList.GetAllHeroEquipScore();
	pNotify->nMaxStrengthen = pPlayer->m_ItemList.GetMaxStrengthenLevel();
	pNotify->nVIPEndTime = pPlayer->m_nVIPEndTime;

	bRetCode = Send(piPackage);
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	KG_COM_RELEASE(piPackage);
	return bResult;
}


BOOL KLSClient::DoClearTopList( int nTopListType, int nGroup )
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	IKG_Buffer* piPackage = NULL;
	S2L_CLEAR_TOPLIST_NOTIFY* pNotify = NULL;

	piPackage = KG_MemoryCreateBuffer(sizeof(S2L_CLEAR_TOPLIST_NOTIFY));
	KGLOG_PROCESS_ERROR(piPackage);

	pNotify = (S2L_CLEAR_TOPLIST_NOTIFY*)piPackage->GetData();
	KGLOG_PROCESS_ERROR(pNotify);

	pNotify->wProtocolID = s2l_clear_toplist_nitify;
	pNotify->nTopListType = nTopListType;
	pNotify->nGroup = nGroup;

	bRetCode = Send(piPackage);
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KLSClient::DoClubInfosRequest(DWORD dwRoleID, KScene* pScene)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    DWORD clubIds[MAX_ROOM_MEMBER_COUNT];
    KPlayer* pPlayer = NULL;
    int nCount = 0;
    IKG_Buffer* piPackage = NULL;
    S2L_CLUB_INFOS_REQUEST* pRequest = NULL;
    size_t uDataSize = 0;
    size_t uPakSize = 0;

    KGLOG_PROCESS_ERROR(pScene);

    for (int i = 0; i < sidTotal; ++i)
    {
        for (int j = 0; j < MAX_TEAM_MEMBER_COUNT; ++j)
        {
            pPlayer = g_pSO3World->m_PlayerSet.GetObj(pScene->m_Param.m_dwMember[i][j]);
            clubIds[nCount] = 0;
            if (pPlayer)
            {
                clubIds[nCount] = pPlayer->m_dwClubID;
            }
            nCount++;
        }
    }

    uDataSize = nCount * sizeof(DWORD);
    uPakSize = sizeof(S2L_CLUB_INFOS_REQUEST) + uDataSize;

    piPackage = KG_MemoryCreateBuffer(uPakSize);
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_CLUB_INFOS_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_club_infos_request;
    pRequest->dwRoleID      = dwRoleID;
    memcpy(pRequest->pData, clubIds, uDataSize);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KLSClient::DoRequestClubName(DWORD dwRoleID, DWORD dwClubID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2L_REQUEST_CLUB_NAME* pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2L_REQUEST_CLUB_NAME));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2L_REQUEST_CLUB_NAME*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2l_request_club_name;
    pRequest->dwRoleID      = dwRoleID;
    pRequest->dwClubID      = dwClubID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

// <------------------- 邮件系统 -------------------------------

////////////////////////////////////////////////////////////////////////// 接收协议

void KLSClient::OnRemoteCall(BYTE* pbyData, size_t uDataLen)
{
    L2S_REMOTE_CALL* pPak = (L2S_REMOTE_CALL*)pbyData;
    KGLOG_PROCESS_ERROR(uDataLen >= sizeof(L2S_REMOTE_CALL));

    m_pRpcProcess->ProcessFun(pPak->dwFunNameHashID, pPak->byParamData, uDataLen - sizeof(L2S_REMOTE_CALL));

Exit0:
    return;
}

void KLSClient::OnHandshakeRespond(BYTE* pbyData, size_t uDataLen)
{
    L2S_HANDSHAKE_RESPOND* pPak = (L2S_HANDSHAKE_RESPOND*)pbyData;
    g_pSO3World->m_nServerIndexInLS = pPak->nServerIndex;
    return;
}

void KLSClient::OnTalkMessage(BYTE* pbyData, size_t uDataLen)
{
    L2S_TALK_MESSAGE*   pTalkMsg        = (L2S_TALK_MESSAGE*)pbyData;
    size_t              uTalkDataLen    = 0;

    uTalkDataLen = uDataLen - sizeof(L2S_TALK_MESSAGE);

    g_PlayerServer.DoTalkMessage(
        pTalkMsg->byMsgType,
        pTalkMsg->dwTalkerID, pTalkMsg->szTalker, 0, pTalkMsg->nTalkerGroupID,
        pTalkMsg->dwReceiverID, pTalkMsg->szReceiver,
        uTalkDataLen, (char *)pTalkMsg->byTalkData
    );

    return;
}

void KLSClient::OnApplyFellowshipDataRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                                bRetCode    = false;
    KPlayer*                            pPlayer     = NULL;
    L2S_APPLY_FELLOWSHIP_DATA_RESPOND*  pPak        = (L2S_APPLY_FELLOWSHIP_DATA_RESPOND*)pbyData;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_FellowshipMgr.OnLoadFellowshipData(pPak->dwPlayerID, uDataLen - sizeof(L2S_APPLY_FELLOWSHIP_DATA_RESPOND), pPak->byData);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KLSClient::OnAddFellowshipRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                        bRetCode    = false;
    L2S_ADD_FELLOWSHIP_RESPOND* pPak        = (L2S_ADD_FELLOWSHIP_RESPOND*)pbyData;
    KPlayer*                    pPlayer     = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KGFellowship*               pFellowship = NULL;
    KGBlackNode*                pBlackNode  = NULL;
    KGENDER                     eGender      = (KGENDER)pPak->byGender;

    KGLOG_PROCESS_ERROR(pPlayer);

    switch (pPak->byType)
    {
    case KFELLOWSHIP_REQUEST_FRIEND:
        pFellowship = g_pSO3World->m_FellowshipMgr.AddFellowship(
            pPak->dwPlayerID, pPak->dwAlliedPlayerID, pPak->szAlliedPlayerName, true
        );
        if (pFellowship)
        {
            pFellowship->m_nVIPLevel    = pPak->byVIPLevel;
            pFellowship->m_eGender      = eGender;
            pFellowship->m_bIsVIP       = pPak->byIsVIP;
            pFellowship->m_nTeamLogo    = pPak->byTeamLogo;
            pFellowship->m_nTeamLogoBg  = pPak->byTeamLogoBg;
            pFellowship->m_nServerID    = pPak->nServerID;
            pFellowship->m_nLadderLevel = pPak->byLadderLevel;

            bRetCode = g_pSO3World->m_FellowshipMgr.DelBlackList(pPak->dwPlayerID, pPak->dwAlliedPlayerID);
            if (bRetCode)
                g_PlayerServer.DoDelFellowshipRespond(pPlayer->m_nConnIndex, pPak->dwAlliedPlayerID, KFELLOWSHIP_REQUEST_BLACKLIST, true);

            g_PlayerServer.DoSyncOneFellowship(pPak->dwPlayerID, pPak->dwAlliedPlayerID);
            g_PlayerServer.DoSyncFellowshipList(pPlayer->m_dwID, 0, NULL);

            PLAYER_LOG(pPlayer, "friends,add_friend");

            pPlayer->OnEvent(peAddFriend);

            g_LSClient.DoBeAddedFriendNotify(pPak->dwAlliedPlayerID, pPlayer);
            
        }
        break;

    case KFELLOWSHIP_REQUEST_BLACKLIST:
        pBlackNode = g_pSO3World->m_FellowshipMgr.AddBlackList(pPak->dwPlayerID, pPak->dwAlliedPlayerID, pPak->szAlliedPlayerName, true);
        if (pBlackNode)
        {
            bRetCode = g_pSO3World->m_FellowshipMgr.DelFellowship(pPak->dwPlayerID, pPak->dwAlliedPlayerID);
            if (bRetCode)
                g_PlayerServer.DoDelFellowshipRespond(pPlayer->m_nConnIndex, pPak->dwAlliedPlayerID, KFELLOWSHIP_REQUEST_FRIEND, true);

            g_PlayerServer.DoSyncOneBlackList(pPak->dwPlayerID, pPak->dwAlliedPlayerID);
            g_PlayerServer.DoSyncBlackList(pPlayer->m_dwID, 0, NULL);

            PLAYER_LOG(pPlayer, "friends,add_black");
        }
        break;

    default:
        break;
    }

Exit0:
    return;
}

void KLSClient::OnGetFellowshipNameRespond(BYTE* pbyData, size_t uDataLen)
{
    L2S_GET_FELLOWSHIP_NAME_RESPOND*    pPak    = (L2S_GET_FELLOWSHIP_NAME_RESPOND*)pbyData;
    KPlayer*                            pPlayer = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    for (int i = 0; i < (int)pPak->byCount; i++)
    {
        KGFellowship*   pFellowship = NULL;
        KGBlackNode*    pBlackNode  = NULL;

        pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(pPak->dwPlayerID, pPak->AlliedPlayerInfo[i].dwID);
        pBlackNode  = g_pSO3World->m_FellowshipMgr.GetBlackListNode(pPak->dwPlayerID, pPak->AlliedPlayerInfo[i].dwID);

        if (pPak->AlliedPlayerInfo[i].szName[0] == '\0')
        {
            KGLogPrintf(
                KGLOG_ERR, 
                "In Player(%u)'s fellowship list, player(%u) can't find in game center. This fellowship is removed.", 
                pPak->dwPlayerID, pPak->AlliedPlayerInfo[i].dwID 
            );

            if (pFellowship)
            {
                g_pSO3World->m_FellowshipMgr.DelFellowship(pPak->dwPlayerID, pPak->AlliedPlayerInfo[i].dwID);
                pFellowship = NULL;
            }

            if (pBlackNode)
            {
                g_pSO3World->m_FellowshipMgr.DelBlackList(pPak->dwPlayerID, pPak->AlliedPlayerInfo[i].dwID);
                pBlackNode = NULL;
            }

            continue;
        }

        if (pFellowship)
        {
            strncpy(pFellowship->m_szName, pPak->AlliedPlayerInfo[i].szName, sizeof(pFellowship->m_szName));
            pFellowship->m_szName[sizeof(pFellowship->m_szName) - sizeof('\0')] = '\0';
        }

        if (pBlackNode)
        {
            strncpy(pBlackNode->m_szName, pPak->AlliedPlayerInfo[i].szName, sizeof(pBlackNode->m_szName));
            pBlackNode->m_szName[sizeof(pBlackNode->m_szName) - sizeof('\0')] = '\0';
        }
    }

Exit0:
    return;
}

struct _UpdateFellowshipInfo
{
    bool operator () (DWORD dwAlliedPlayerID, DWORD dwPlayerID)
    {
        KGFellowship*   pFellowship     = NULL;
        int             nOldLevel       = 0;
        KPlayer*        pPlayer         = NULL;

        pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(dwPlayerID, dwAlliedPlayerID);
        if (!pFellowship)
            goto Exit1;

        pFellowship->m_nPlayerLevel = nLevel;
        pFellowship->m_bOnline      = bOnline;
        pFellowship->m_nVIPLevel    = nVIPLevel;
        pFellowship->m_bIsVIP       = bIsVIP;
        pFellowship->m_eGender      = eGender;
        pFellowship->m_nLadderLevel = nLadderLevel;
        pFellowship->m_nTeamLogo    = nTeamLogo;
        pFellowship->m_nTeamLogoBg  = nTeamLogoBg;
        pFellowship->m_nServerID   = nServerID;

        if (!bNotifyClient)
            goto Exit1;

        pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
        if (pPlayer)
        {
            g_PlayerServer.DoUpdateFellowshipInfo(pPlayer->m_nConnIndex, dwAlliedPlayerID, pFellowship);
        }
Exit1:
        return true;
    }

    int     nLevel;
    BOOL    bOnline;
    BOOL    bNotifyClient;
    KGENDER eGender;
    int     nVIPLevel;
    int     nLadderLevel;
    int     nTeamLogo;
    int     nTeamLogoBg;
    BOOL    bIsVIP;
    int     nServerID;
};

void KLSClient::OnSyncFellowshipPlayerInfo(BYTE* pbyData, size_t uDataLen)
{
    L2S_SYNC_FELLOWSHIP_PLAYER_INFO*    pPak                    = (L2S_SYNC_FELLOWSHIP_PLAYER_INFO*)pbyData;
    _UpdateFellowshipInfo               UpdateFellowshipInfo;
    KGENDER                             eGender                 = (KGENDER)pPak->byGender;

    UpdateFellowshipInfo.nLevel        = pPak->byLevel;
    UpdateFellowshipInfo.bOnline       = pPak->byStatus;
    UpdateFellowshipInfo.bNotifyClient = pPak->bNotifyClient;
    UpdateFellowshipInfo.eGender       = eGender;
    UpdateFellowshipInfo.nVIPLevel     = pPak->byVIPLevel;
    UpdateFellowshipInfo.nLadderLevel  = pPak->byLadderLevel;
    UpdateFellowshipInfo.nTeamLogo     = pPak->byTeamLogo;
    UpdateFellowshipInfo.nTeamLogoBg   = pPak->byTeamLogoBg;
    UpdateFellowshipInfo.bIsVIP        = pPak->byIsVIP;
    UpdateFellowshipInfo.nServerID    = pPak->nServerID;

    g_pSO3World->m_FellowshipMgr.TraverseReverseFellowshipID(pPak->dwPlayerID, UpdateFellowshipInfo);

Exit0:
    return;
}

void KLSClient::OnQueryStatIDRespond(BYTE* pbyData, size_t uDataLen)
{
    L2S_QUERY_STAT_ID_RESPOND* pRespond = (L2S_QUERY_STAT_ID_RESPOND*)pbyData;

    g_pSO3World->m_StatDataServer.SetNameID(pRespond->szName, pRespond->nID);
}

void KLSClient::OnRequestFriendHeroList(BYTE* pbyData, size_t uDataLen)
{
    BOOL bRetCode = false;
    L2S_REQUEST_FRIEND_HERO_LIST* pMsg= (L2S_REQUEST_FRIEND_HERO_LIST*)pbyData;
    KPlayer* pPlayer = NULL;
    KHeroData* pMainHeroData = NULL; 
    KHeroData* pAssistHeroData = NULL;
    KHeroPackage* pMainHeroPackage = NULL;
    std::vector<DWORD> vecHeroList;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pMsg->dwFriendID);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_HeroDataList.GetHeroList(vecHeroList);

    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    pAssistHeroData = pPlayer->GetAssistHeroData();
    KGLOG_PROCESS_ERROR(pAssistHeroData);

    bRetCode = g_PlayerServer.DoSendFriendHeroList(pMsg->dwRequesterID, pMsg->dwFriendID, pPlayer->m_nLevel, vecHeroList, pMainHeroData->m_dwTemplateID, pAssistHeroData->m_dwTemplateID, pPlayer->m_szName);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KLSClient::OnBeAddedFriendNotify(BYTE* pbyData, size_t uDataLen)
{
    BOOL bRetCode = false;
    L2S_BEADDED_FRIEND_NOTIFY * pMsg= (L2S_BEADDED_FRIEND_NOTIFY*)pbyData;
    KPlayer* pPlayer = NULL;
    KReport_BeAdded_Friend param;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pMsg->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    memcpy(param.srcName, pMsg->szPlayerName, sizeof(param.srcName));
    pPlayer->m_Secretary.AddReport(KREPORT_EVENT_BEADDED_FRIEND, (BYTE*)&param, sizeof(param));

Exit0:
    return;
}

void KLSClient::OnRequestFriendHeroInfo(BYTE* pbyData, size_t uDataLen)
{
    BOOL bRetCode = false;
    L2S_REQUEST_FRIEND_HERO_INFO * pMsg= (L2S_REQUEST_FRIEND_HERO_INFO*)pbyData;
    KPlayer* pPlayer = NULL;
    KHeroData* pHeroData = NULL; 
    KHeroPackage* pHeroPackage = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pMsg->dwFriendID);
    KGLOG_PROCESS_ERROR(pPlayer);

    pHeroData = pPlayer->m_HeroDataList.GetHeroData(pMsg->dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    pHeroPackage = pPlayer->m_ItemList.GetHeroPackage(pHeroData->m_dwTemplateID);
    KGLOG_PROCESS_ERROR(pHeroPackage);

    bRetCode = g_PlayerServer.DoSendFriendHeroInfo(pMsg->dwRequesterID, pMsg->dwFriendID, pHeroData, pHeroPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}


void KLSClient::OnActivityFinishNotify(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    L2S_ACTIVITY_FINISH_NOTIFY* pNotify = (L2S_ACTIVITY_FINISH_NOTIFY*)pbyData;
    KPlayer* pPlayer = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = g_pSO3World->m_ActivityCenter.OnActivityFinished(pPlayer, pNotify->dwActivityID, pNotify->nFinishType);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return;
}

void KLSClient::OnCheckUpdateActivityRespond(BYTE* pbyData, size_t uDataLen)
{
	BOOL bResult = false;
	BOOL bRetCode = false;
    KPlayer* pPlayer = NULL;
    L2S_ACTIVITY_CHECK_UPDATE_RESPOND* pResond = (L2S_ACTIVITY_CHECK_UPDATE_RESPOND*)pbyData;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pResond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);
	
    bRetCode = g_pSO3World->m_ActivityCenter.OnCheckAndUpdateActivityStateRespond(pPlayer, pResond->dwActivityID, pResond->nFinishType);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return;
}

void KLSClient::OnSendMailRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                    bRetCode     	= 0;
    L2S_SEND_MAIL_RESPOND*  pPak         	= (L2S_SEND_MAIL_RESPOND*)pbyData;
    KPlayer*                pPlayer      	= NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwSrcID);
    KG_PROCESS_ERROR(pPlayer);
    KG_PROCESS_ERROR(pPak->byResult == mrcSucceed);
    //手续费
    if (pPlayer->m_MoneyMgr.CanAddMoney(emotMoney,-g_pSO3World->m_Settings.m_ConstList.nMailCostMoney))
    {
        bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotMoney,-g_pSO3World->m_Settings.m_ConstList.nMailCostMoney);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

Exit0:
    if (pPlayer)
    {
        g_PlayerServer.DoSendMailRespond(pPlayer->m_nConnIndex, pPak->byRespondID, pPak->byResult);
    }

    return;
}

void KLSClient::OnGetMailListRespond(BYTE* pbyData, size_t uDataLen)
{
    KPlayer*                    pPlayer         = NULL;
    L2S_GET_MAILLIST_RESPOND*   pPak            = (L2S_GET_MAILLIST_RESPOND*)pbyData;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    KG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);

    g_PlayerServer.DoGetMailListRespond(pPlayer->m_nConnIndex, pPak->MailList, pPak->nMailCount, pPak->nMailTotalCount);

Exit0:
    return;
}

void KLSClient::OnSyncMailContent(BYTE* pbyData, size_t uDataLen)
{
    KPlayer*                pPlayer     = NULL;
    L2S_SYNC_MAIL_CONTENT*  pPak        = (L2S_SYNC_MAIL_CONTENT*)pbyData;
    KMailContent*           pContent    = NULL;
    size_t                  uContentLen = 0;

    if (pPak->byResult == mrcSucceed)
    {
        pContent = (KMailContent*)pPak->byData;

        KGLOG_PROCESS_ERROR(uDataLen >= sizeof(L2S_SYNC_MAIL_CONTENT) + sizeof(KMailContent));
        uContentLen = sizeof(KMailContent) + pContent->wTextLen;
        for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; i++)
        {
            uContentLen += pContent->ItemDesc[i].byDataLen;
        }
    }
    KGLOG_PROCESS_ERROR(uContentLen + sizeof(L2S_SYNC_MAIL_CONTENT) == uDataLen);

    g_PlayerServer.DoSyncMailContent(
        pPak->dwPlayerID, pPak->dwMailID, pPak->byResult, pContent
    );

Exit0:
    return;
}

void KLSClient::OnGiveMailAllToPlayer(BYTE* pbyData, size_t uDataLen)
{
    KMAIL_RESULT_CODE               nResult       = mrcFailed;
    int                             nRetCode      = 0;
    KPlayer*                        pPlayer       = NULL;
    L2S_GIVE_MAIL_ALL_TO_PLAYER*  	pPak          = (L2S_GIVE_MAIL_ALL_TO_PLAYER*)pbyData;
    int                             nPlayerMoney  = 0;
    int                             nMoneyLimit   = 0;
    IItem*                          piItem[KMAIL_MAX_ITEM_COUNT] = {0};
    BYTE*                           pbyItemData = NULL;
    T3DB::KPB_EQUI_DATA             EquiData;
    BOOL                            bHaveMoney  = false;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    KG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);
	
    pbyItemData = pPak->byData;
    for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
    {
        if (pPak->ItemDesc[i].byDataLen > 0 && !pPak->ItemDesc[i].bAcquired)
        {
            EquiData.Clear();
            nRetCode = EquiData.ParsePartialFromArray(pbyItemData,pPak->ItemDesc[i].byDataLen);
            KGLOG_PROCESS_ERROR(nRetCode);
            piItem[i] = g_pSO3World->m_ItemHouse.CreateItemFromProtoBuf(&EquiData);
            KGLOG_PROCESS_ERROR(piItem[i]);
        }
        pbyItemData += pPak->ItemDesc[i].byDataLen;
    }
	
    if (pPak->nMoney > 0)
    {
        bHaveMoney = true;
        nRetCode = pPlayer->m_MoneyMgr.AddMoney(emotMoney, pPak->nMoney);
        KGLOG_PROCESS_ERROR(nRetCode);
        nResult = mrcOnlyMoneyOk;
    } 
    
    nRetCode = pPlayer->m_ItemList.CanAddPlayerItems(piItem, KMAIL_MAX_ITEM_COUNT); 
	nResult = mrcNotEnoughRoom;
    if (bHaveMoney) 
        nResult = mrcOnlyMoneyOk;
		
    KG_PROCESS_ERROR(nRetCode);

    
    for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
    {
        if (piItem[i] == NULL)
            continue;

        nRetCode = pPlayer->m_ItemList.AddPlayerItem(piItem[i]);
        assert(nRetCode);
        piItem[i] = NULL;
    }

    nResult = mrcSucceed;
Exit0:
    
    DoGiveMailAllToPlayerRespond(pPak->dwPlayerID, pPak->dwMailID, nResult);
    if (nResult != mrcSucceed)
    {
        for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
        {
            if (piItem[i])
            {
                g_pSO3World->m_ItemHouse.DestroyItem(piItem[i]);
                piItem[i] = NULL;
            }
        }
    }
    return;
}

void KLSClient::OnGiveMailMoneyToPlayer(BYTE* pbyData, size_t uDataLen)
{
    KMAIL_RESULT_CODE               eResult       = mrcFailed;
    int                             nRetCode      = 0;
    KPlayer*                        pPlayer       = NULL;
    L2S_GIVE_MAIL_MONEY_TO_PLAYER*  pPak          = (L2S_GIVE_MAIL_MONEY_TO_PLAYER*)pbyData;
    int                             nPlayerMoney  = 0;
    int                             nMoneyLimit   = 0;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    KG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);

    KGLOG_PROCESS_ERROR(pPak->nMoney > 0);

    nRetCode = pPlayer->m_MoneyMgr.AddMoney(emotMoney, pPak->nMoney);
    KGLOG_PROCESS_ERROR(nRetCode);

    eResult = mrcSucceed;
Exit0:
    DoGiveMailMoneyToPlayerRespond(pPak->dwPlayerID, pPak->dwMailID, eResult);
    return;
}

void KLSClient::OnGiveMailItemToPlayer(BYTE* pbyData, size_t uDataLen)
{
    int                             nRetCode        = 0;
    KPlayer*                        pPlayer         = NULL;
    L2S_GIVE_MAIL_ITEM_TO_PLAYER*   pPak            = (L2S_GIVE_MAIL_ITEM_TO_PLAYER*)pbyData;
    BOOL                            bGotItem        = false;
    IItem*                          piItem          = NULL;
    BOOL                            bMentorItem     = false;
    int                             nPlayerMoney    = 0;
    KMAIL_RESULT_CODE               eResult         = mrcFailed;
    T3DB::KPB_EQUI_DATA             EquiData;

    KGLOG_PROCESS_ERROR(pPak->byItemLen > 0);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    KG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);

    KG_PROCESS_ERROR(pPak->byResult == mrcSucceed);

    KG_PROCESS_ERROR(pPak->nItemPrice >= 0);

    nPlayerMoney    = pPlayer->m_MoneyMgr.GetMoney(emotMoney);

    if(nPlayerMoney < pPak->nItemPrice)
    {
        eResult = mrcNotEnoughMoney;
        goto Exit0;
    }

    EquiData.Clear();
    nRetCode = EquiData.ParsePartialFromArray(pPak->byData,pPak->byItemLen);
    KGLOG_PROCESS_ERROR(nRetCode);
    piItem = g_pSO3World->m_ItemHouse.CreateItemFromProtoBuf(&EquiData);
    KGLOG_PROCESS_ERROR(piItem);

    nRetCode = pPlayer->m_ItemList.CanAddPlayerItem(piItem);
    if (nRetCode == aircSuccess)
    {
        int                     nEquipScore     = 0;
        const KItemProperty*    pItemProperty   = NULL;

        nRetCode = pPlayer->m_ItemList.AddPlayerItem(piItem);
        KGLOG_PROCESS_ERROR(nRetCode == aircSuccess);
        piItem = NULL;

        nRetCode = pPlayer->m_MoneyMgr.AddMoney(emotMoney, -pPak->nItemPrice);
        KGLOG_PROCESS_ERROR(nRetCode);

        eResult  = mrcSucceed;
    }
    else if (nRetCode == aircNotEnoughFreeRoom)
    {
        eResult = mrcNotEnoughRoom;
    }
    else if (nRetCode == aircItemAmountLimited)
    {
        eResult = mrcItemAmountLimit;
    }

Exit0:
    DoGiveMailItemToPlayerRespond(pPak->dwPlayerID, pPak->dwMailID, pPak->byItemIndex, eResult);
    if (piItem)
    {
        g_pSO3World->m_ItemHouse.DestroyItem(piItem);
        piItem = NULL;
    }
    return;
}

void KLSClient::OnMailGeneralRespond(BYTE* pbyData, size_t uDataLen)
{
    L2S_MAIL_GENERAL_RESPOND*   pRespond    = (L2S_MAIL_GENERAL_RESPOND*)pbyData;

    g_PlayerServer.DoMailGeneralRespond(pRespond->dwPlayerID, pRespond->dwMailID, pRespond->byRespond);

    return;
}

void KLSClient::OnSendToClient(BYTE* pbyData, size_t uDataLen)
{
    KPlayer*            pPlayer     = NULL;
    size_t              uLen        = 0;
    L2S_SEND_TO_CLIENT* pPak        = (L2S_SEND_TO_CLIENT*)pbyData;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    uLen = uDataLen - sizeof(L2S_SEND_TO_CLIENT);

    g_PlayerServer.DoSendToClient(pPlayer->m_nConnIndex, pPak->pData, uLen);

Exit0:
    return;
}

void KLSClient::OnVIPPresentNotify(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bRetCode    = false;
    KPlayer*            pPlayer     = NULL;
    size_t              uLen        = 0;
    L2S_PRESENT_VIP_NOTIFY* pPak    = (L2S_PRESENT_VIP_NOTIFY*)pbyData;
    

    pPak->szSrcName[countof(pPak->szSrcName) - 1] = '\0';

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwDestID);
    KGLOG_PROCESS_ERROR(pPlayer);
    
    // 通知ls删除赠送记录
    DoGetVIPPresentNotify(pPak->dwSNID);

    bRetCode = pPlayer->ChargeVIPTimeResult(pPak->nChargeType, pPak->szSrcName);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KLSClient::OnForbidRole(BYTE* pbyData, size_t uDataLen)
{
    BOOL             bRetCode       = false;
    L2S_FORBID_ROLE* pForbidInfo    = (L2S_FORBID_ROLE*)pbyData;
    KPlayer*         pPlayer        = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pForbidInfo->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_nForbidTalkTime  = pForbidInfo->dwTime;
Exit0:
    return;
}

void KLSClient::OnKickRole(BYTE* pbyData, size_t uDataLen)
{
    BOOL            bRetCode    = false;
    L2S_KICK_ROLE*  pNotify     = (L2S_KICK_ROLE*)pbyData;
    KPlayer*        pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    KGLogPrintf(KGLOG_INFO, "Kick account(%s,%s) from game center, shutdown(%d)\n", pPlayer->m_szAccount, pPlayer->m_szName, pPlayer->m_nConnIndex);
    g_PlayerServer.DoAccountKickout(pPlayer->m_nConnIndex);

    if (pPlayer->m_nConnIndex >= 0)
    {
        g_PlayerServer.Detach(pPlayer->m_nConnIndex);
    }

Exit0:
    return;
}

void KLSClient::OnLeaveClubNotify(BYTE* pbyData, size_t uDataLen)
{
    BOOL                        bRetCode = false;
    L2S_LEAVE_CLUB_NOTIFY*      pNotify  = (L2S_LEAVE_CLUB_NOTIFY*)pbyData;
    KPlayer*                    pPlayer  = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);
    //修改变量
    pPlayer->m_dwClubID = 0;

    g_RelayClient.DoUpdateClubID(pPlayer);

    if (pPlayer->m_dwClubID != pPlayer->m_dwLastClubID)
    {
        pPlayer->m_nQuitClubTime = g_pSO3World->m_nCurrentTime;
        g_PlayerServer.DoLeaveClubNotify(pPlayer->m_nConnIndex); 
        g_RelayClient.SaveRoleData(pPlayer);
        pPlayer->m_Secretary.AddReport(KREPORT_EVENT_LEAVE_CLUB, NULL, 0);
    }

    pPlayer->m_dwLastClubID = pPlayer->m_dwClubID;
Exit0:
    return;
}

void KLSClient::OnEnterClubNotify(BYTE* pbyData, size_t uDataLen)
{
    L2S_ENTER_CLUB_NOTIFY*      pNotify  = (L2S_ENTER_CLUB_NOTIFY*)pbyData;
    KPlayer*                    pPlayer  = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);
    pPlayer->m_dwClubID = pNotify->dwClubID;
    g_RelayClient.SaveRoleData(pPlayer);

    g_RelayClient.DoUpdateClubID(pPlayer);

Exit0:
    return;
}

void KLSClient::OnApplyJoinClubRespond(BYTE* pbyData, size_t uDataLen)
{
    L2S_APPLY_JOIN_CLUB_RESPOND*        pNotify  = (L2S_APPLY_JOIN_CLUB_RESPOND*)pbyData;
    KPlayer*                            pPlayer  = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    if (pNotify->byFailedType != KAPPLY_JOIN_CLUB_NOERROR)
        --pPlayer->m_nClubApplyNum;
	else
    {
        PLAYER_LOG(pPlayer, "club,join,%d,%d,%d", pPlayer->m_nLevel, pPlayer->m_nTotalGameTime, (g_pSO3World->m_nCurrentTime - pPlayer->m_nCreateTime));
    }

    //nFailedType
    //根据失败类型提示客户端
    g_PlayerServer.DoApplyJoinClubRepsond(pPlayer->m_nConnIndex, pNotify->dwClubID, pNotify->byFailedType);
    
Exit0:
    return;
}

void KLSClient::OnPostModify(BYTE* pbyData, size_t uDataLen)
{
    L2S_POST_MODIFY*      pNotify  = (L2S_POST_MODIFY*)pbyData;
    KPlayer*              pPlayer  = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    if (pPlayer->m_dwID == pNotify->dwModerID)
        pPlayer->m_nClubPost = (int)pNotify->byPost;

    g_PlayerServer.DoPostModify(pPlayer->m_nConnIndex, pNotify->dwModerID, pNotify->byPost); 
Exit0:
    return;
}

void KLSClient::OnClubBaseInfo(BYTE* pbyData, size_t uDataLen)
{
    BOOL                  bRetCode = false;
    L2S_CLUB_BASE_INFO*   pNotify  = (L2S_CLUB_BASE_INFO*)pbyData;
    KPlayer*              pPlayer  = NULL;
    KCLUB_INFO_TYPE       eInfoType = eInfo_Normal;
    KReport_Enter_Club    param;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    pPlayer->m_dwClubID = pNotify->dwClubID;   
    pPlayer->m_nClubPost = (int)pNotify->byPost;

    g_RelayClient.DoUpdateClubID(pPlayer);

    if (pPlayer->m_dwClubID != pPlayer->m_dwLastClubID)
    {
        eInfoType = eInfo_NewMember;
        pPlayer->m_dwLastClubID = pPlayer->m_dwClubID;
        memcpy(param.strClubName, pNotify->szClubName, sizeof(param.strClubName));
        pPlayer->m_Secretary.AddReport(KREPORT_EVENT_ENTER_CLUB, (BYTE*)&param, sizeof(param));
    }

    if (pNotify->byIsNewClub)
        eInfoType = eInfo_Create;

    if (eInfoType == eInfo_NewMember || eInfoType == eInfo_Create)
    {
        pPlayer->OnEvent(peInjoinPart);
    }
      
    g_PlayerServer.DoClubBaseInfo(
        pPlayer->m_nConnIndex, pNotify->byPost, pNotify->byApplySwitch, pNotify->dwClubID, eInfoType,
        pNotify->szClubName, pNotify->szLeaderName, pNotify->nCreateTime, pNotify->byMemberCount,
        pNotify->byMaxMemberCount, pNotify->uNoticeLen, pNotify->uOnlineLen, pNotify->uIntroLen, pNotify->byData
        ); 

Exit0:
    return;
}
void KLSClient::OnClubInviteNotify(BYTE* pbyData, size_t uDataLen)
{
    L2S_CLUB_INVITE_NOTIFY* pNotify = (L2S_CLUB_INVITE_NOTIFY*)pbyData;
    KPlayer*                pPlayer = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    if (g_pSO3World->m_nCurrentTime < pPlayer->m_nQuitClubTime + g_pSO3World->m_Settings.m_ConstList.nAddClubCoolDown)
    {
        DoAcceptClubInviteRequest(pPlayer->m_dwID, pNotify->dwClubID, pNotify->dwLauncherID, KACCEPT_INVITE_COOL_DOWN);
        goto Exit0;
    }

    g_PlayerServer.DoClubInviteNotify(pPlayer->m_nConnIndex, pNotify->dwClubID, pNotify->dwLauncherID, pNotify->szClubName, pNotify->szPlayerName);
    
Exit0:
    return;
}

void KLSClient::OnCreateClubRespond(BYTE* pbyData, size_t uDataLen)
{
    L2S_CREATE_CLUB_RESPOND*   pNotify  = (L2S_CREATE_CLUB_RESPOND*)pbyData;
    KPlayer*                   pPlayer  = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    //扣钱
    if (pNotify->bySucc)
        pPlayer->m_MoneyMgr.AddMoney(emotMoney, -g_pSO3World->m_Settings.m_ConstList.nMoneyForCreateClub);

    PLAYER_LOG(pPlayer, "club,create,%d,%d,%d", pPlayer->m_nLevel, pPlayer->m_nTotalGameTime, (g_pSO3World->m_nCurrentTime - pPlayer->m_nCreateTime));

Exit0:
    return;
}

void KLSClient::OnBroadcastToClient(BYTE* pbyData, size_t uDataLen)
{
    L2S_BROADCAST_TO_CLIENT* pPak = (L2S_BROADCAST_TO_CLIENT*)pbyData;
    size_t uPakSize = 0;
    KBroadcastFunc BroadcastFunc;

    uPakSize = uDataLen - sizeof(L2S_BROADCAST_TO_CLIENT);

    BroadcastFunc.m_pvData         = pPak->pData;
    BroadcastFunc.m_uSize          = uPakSize;
    BroadcastFunc.m_dwExclusive    = 0;
    BroadcastFunc.m_dwExcept       = ERROR_ID;

    g_pSO3World->m_PlayerSet.Traverse(BroadcastFunc);
}

void KLSClient::OnNewMailNotify(BYTE* pbyData, size_t uDataLen)
{
    L2S_NEW_MAIL_NOTIFY* pPak = (L2S_NEW_MAIL_NOTIFY*)pbyData;
    KPlayer*             pPlayer  = NULL;
    KReport_New_Gift     param;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

	g_PlayerServer.DoNewMailNotify(pPlayer->m_nConnIndex);

    if (pPak->byMailType == eMailType_Shop)
    {
        memcpy(param.srcName, pPak->szSender, sizeof(param.srcName));
        pPlayer->m_Secretary.AddReport(KREPORT_EVENT_NEW_GIFT, (BYTE*)&param, sizeof(param));
    }
    else
    {
        pPlayer->m_Secretary.AddReport(KREPORT_EVENT_NEW_MAIL, NULL, 0);
    }

Exit0:
    return;
}

void KLSClient::OnMailTooManyNotify(BYTE* pbyData, size_t uDataLen)
{
    L2S_MAIL_TOO_MANG_NOTIFY* pPak = (L2S_MAIL_TOO_MANG_NOTIFY*)pbyData;
    KPlayer*             pPlayer  = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    pPlayer->m_Secretary.AddReport(KREPORT_EVENT_MAIL_TOO_MANY, NULL, 0);

Exit0:
    return;
}

void KLSClient::OnCheckCanPresentResult(int nParamCount, KVARIABLE m_Param[])
{
    BOOL        bRetCode        = false;
    int         nSrcPlayerID    = 0;
    int         nParam          = 0;
    int         nCanPresent     = 0;
    KPlayer*    pSrcPlayer      = NULL;

    KGLOG_PROCESS_ERROR(nParamCount == 2);

    nSrcPlayerID        = m_Param[0];
    nCanPresent         = m_Param[1];

    pSrcPlayer = g_pSO3World->m_PlayerSet.GetObj((DWORD)nSrcPlayerID);
    KGLOG_PROCESS_ERROR(pSrcPlayer);

    if (nCanPresent == 0) // 无法赠送
    {
        KMEMORY_FREE(pSrcPlayer->m_pbyPresentGoodInfo);
        // 通知客户端
        g_PlayerServer.DoDownwardNotify(pSrcPlayer, KMESSAGE_DEST_PLAYER_NOT_EXIST);
        goto Exit0;
    }
    
    bRetCode = g_pSO3World->m_ShopMgr.OnPlayerPresentGoods(pSrcPlayer);
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}
