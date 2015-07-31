#include "stdafx.h"
#include "Relay_GS_Protocol.h"
#include "KRelayClient.h"
#include "SO3Result.h"
#include "Engine/KG_CreateGUID.h"
#include "Engine/KG_Time.h"
#include "KRoleDBDataDef.h"
#include "KPlayer.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KScene.h"
#include "KTerrainData.h"
#include "KLSClient.h"
#include "GatewayDef.h"
#include "KShopMgr.h"

KRelayClient g_RelayClient;

KRelayClient::KRelayClient(void)
{
    m_piSocketStream        = NULL;
    m_bSocketError          = false;
    m_bQuiting              = false;
    m_nNextQuitingSaveTime  = 0;
	m_nPingCycle            = 0;
	m_nLastSendPacketTime   = 0;

    m_dwSyncRoleID          = ERROR_ID;
    m_pbySyncRoleBuffer     = NULL;
    m_uSyncRoleOffset       = 0;

    m_pbySaveRoleBuffer     = NULL;

    memset(m_ProcessProtocolFuns, 0, sizeof(m_ProcessProtocolFuns));
    memset(m_uProtocolSize, 0, sizeof(m_uProtocolSize));

    // ------------------ 协议注册 --------------------------------------------------------------------
    REGISTER_INTERNAL_FUNC(r2s_apply_server_config_respond, &KRelayClient::OnApplyServerConfigRespond, sizeof(R2S_APPLY_SERVER_CONFIG_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_handshake_respond, &KRelayClient::OnHandshakeRespond, sizeof(R2S_HANDSHAKE_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_quit_notify, &KRelayClient::OnQuitNotify, sizeof(R2S_QUIT_NOTIFY));
	REGISTER_INTERNAL_FUNC(r2s_limitplay_info_respond, &KRelayClient::OnLimitPlayInfoResponse, sizeof(R2S_LIMITPLAY_INFO_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_confirm_player_login_respond, &KRelayClient::OnConfirmPlayerLoginRespond, sizeof(R2S_CONFIRM_PLAYER_LOGIN_RESPOND));

    REGISTER_INTERNAL_FUNC(r2s_create_map_notify, &KRelayClient::OnCreateMapNotify, sizeof(R2S_CREATE_MAP_NOTIFY));
    REGISTER_INTERNAL_FUNC(r2s_finish_create_map_notify, &KRelayClient::OnFinishCreateMapNotify, sizeof(R2S_FINISH_CREATE_MAP_NOTIFY));
    REGISTER_INTERNAL_FUNC(r2s_delete_map_notify, &KRelayClient::OnDeleteMapNotify, sizeof(R2S_DELETE_MAP_NOTIFY));

    REGISTER_INTERNAL_FUNC(r2s_player_login_request, &KRelayClient::OnPlayerLoginRequest, sizeof(R2S_PLAYER_LOGIN_REQUEST));
    REGISTER_INTERNAL_FUNC(r2s_search_map_respond, &KRelayClient::OnSearchMapRespond, sizeof(R2S_SEARCH_MAP_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_transfer_player_request, &KRelayClient::OnTransferPlayerRequest, sizeof(R2S_TRANSFER_PLAYER_REQUEST));
    REGISTER_INTERNAL_FUNC(r2s_transfer_player_respond, &KRelayClient::OnTransferPlayerRespond, sizeof(R2S_TRANSFER_PLAYER_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_coin_shop_buy_item_respond, &KRelayClient::OnCoinShopBuyItemRespond, sizeof(R2S_COIN_SHOP_BUY_ITEM_RESPOND));

    REGISTER_INTERNAL_FUNC(r2s_kick_account_notify, &KRelayClient::OnKickAccountNotify, sizeof(R2S_KICK_ACCOUNT_NOTIFY));
    REGISTER_INTERNAL_FUNC(r2s_sync_role_data, &KRelayClient::OnSyncRoleData, sizeof(R2S_SYNC_ROLE_DATA));
    REGISTER_INTERNAL_FUNC(r2s_load_role_data, &KRelayClient::OnLoadRoleData, sizeof(R2S_LOAD_ROLE_DATA));
    REGISTER_INTERNAL_FUNC(r2s_save_role_data_respond, &KRelayClient::OnSaveRoleDataRespond, sizeof(R2S_SAVE_ROLE_DATA_RESPOND));
    
    REGISTER_INTERNAL_FUNC(r2s_sync_create_room_respond, &KRelayClient::OnSyncCreateRoomRespond, sizeof(R2S_SYNC_CREATE_ROOM_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_sync_oneroommember_posinfo, &KRelayClient::OnSyncOneRoomMemberPosInfo, sizeof(R2S_SYNC_ONEROOMMEMBER_POSINFO));

    REGISTER_INTERNAL_FUNC(r2s_add_room_cache, &KRelayClient::OnAddRoomCache, sizeof(R2S_ADD_ROOM_CACHE));
    REGISTER_INTERNAL_FUNC(r2s_del_room_cache, &KRelayClient::OnDelRoomCache, sizeof(R2S_DEL_ROOM_CACHE));
    REGISTER_INTERNAL_FUNC(r2s_update_room_name_in_cache, &KRelayClient::OnUpdateRoomNameInCache, sizeof(R2S_UPDATE_ROOM_NAME_IN_CACHE));
    REGISTER_INTERNAL_FUNC(r2s_update_mapid_in_cache, &KRelayClient::OnUpdateMapIDInCache, sizeof(R2S_UPDATE_MAPID_IN_CACHE));
    REGISTER_INTERNAL_FUNC(r2s_update_membercount_in_cache, &KRelayClient::OnUpdateMemberCountInCache, sizeof(R2S_UPDATE_MEMBERCOUNT_IN_CACHE));
    REGISTER_INTERNAL_FUNC(r2s_update_fightingstate_in_cache, &KRelayClient::OnUpdateFightingStateInCache, sizeof(R2S_UPDATE_FIGHTINGSTATE_IN_CACHE));
    REGISTER_INTERNAL_FUNC(r2s_update_haspassword_in_cache, &KRelayClient::OnUpdateHasPasswordInCache, sizeof(R2S_UPDATE_HASPASSWORD_IN_CACHE));
    REGISTER_INTERNAL_FUNC(r2s_sync_room_baseinfo_for_cache, &KRelayClient::OnSyncRoomBaseInfoForCache, sizeof(R2S_SYNC_ROOM_BASEINFO_FOR_CACHE));

    REGISTER_INTERNAL_FUNC(r2s_apply_join_room_respond, &KRelayClient::OnApplyJoinRoomRespond, sizeof(R2S_APPLY_JOIN_ROOM_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_apply_leave_room_respond, &KRelayClient::OnApplyLeaveRoomRespond, sizeof(R2S_APPLY_LEAVE_ROOM_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_apply_switch_battle_map_respond, &KRelayClient::OnApplySwitchBattleMapRespond, sizeof(R2S_APPLY_SWITCH_BATTLE_MAP_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_apply_switch_room_host_respond, &KRelayClient::OnApplySwitchRoomHostRespond, sizeof(R2S_APPLY_SWITCH_ROOM_HOST_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_apply_set_room_password_respond, &KRelayClient::OnApplySetRoomPasswordRespond, sizeof(R2S_APPLY_SET_ROOM_PASSWORD_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_sync_roomname, &KRelayClient::OnSyncRoomName, sizeof(R2S_SYNC_ROOMNAME));

    REGISTER_INTERNAL_FUNC(r2s_apply_set_ready_respond, &KRelayClient::OnApplySetReadyRespond, sizeof(R2S_APPLY_SET_READY_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_apply_start_game_respond, &KRelayClient::OnApplyStartGameRespond, sizeof(R2S_APPLY_START_GAME_RESPOND));

    // 自动组队匹配相关
    REGISTER_INTERNAL_FUNC(r2s_automatch_respond, &KRelayClient::OnAutoMatchRespond, sizeof(R2S_AUTOMATCH_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_leave_automatch_notify, &KRelayClient::OnLeaveAutoMatchNotify, sizeof(R2S_LEAVE_AUTOMATCH_NOTIFY));

    REGISTER_INTERNAL_FUNC(r2s_sendto_client, &KRelayClient::OnSendToClient, sizeof(R2S_SENDTO_CLIENT));
    REGISTER_INTERNAL_FUNC(r2s_rename_respond, &KRelayClient::OnRenameRespond, sizeof(R2S_RENAME_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_reset_map, &KRelayClient::OnResetMap, sizeof(R2S_RESET_MAP));

    REGISTER_INTERNAL_FUNC(r2s_can_enter_mission, &KRelayClient::OnCanEnterMission, sizeof(R2S_CAN_ENTER_MISSION));

    REGISTER_INTERNAL_FUNC(r2s_notify_freepvp_invite, &KRelayClient::OnFreePVPInvite, sizeof(R2S_NOTIFY_FREEPVP_INVITE));
    REGISTER_INTERNAL_FUNC(r2s_notify_ladderpvp_invite, &KRelayClient::OnLadderPVPInvite, sizeof(R2S_NOTIFY_LADDERPVP_INVITE));
    REGISTER_INTERNAL_FUNC(r2s_notify_pve_invite, &KRelayClient::OnPVEInvite, sizeof(R2S_NOTIFY_PVE_INVITE));
    REGISTER_INTERNAL_FUNC(r2s_change_ext_point_respond, &KRelayClient::OnChangeExtPointRespond, sizeof(R2S_CHANGE_EXT_POINT_RESPOND));
    REGISTER_INTERNAL_FUNC(r2s_forbid_player_talk, &KRelayClient::OnForbidPlayerTalk, sizeof(R2S_FORBID_PLAYER_TALK));
    REGISTER_INTERNAL_FUNC(r2s_forbid_ip_talk, &KRelayClient::OnForbidIPTalk, sizeof(R2S_FORBID_PLAYER_TALK));
    REGISTER_INTERNAL_FUNC(r2s_freeze_role, &KRelayClient::OnFreezeRole, sizeof(R2S_FREEZE_NOTIFY));
    REGISTER_INTERNAL_FUNC(r2s_freeze_ip, &KRelayClient::OnFreezeIP, sizeof(R2S_FREEZE_NOTIFY));
    REGISTER_INTERNAL_FUNC(r2s_account_new_coin_notify, &KRelayClient::OnAccountNewCoinNotify, sizeof(R2S_ACCOUNT_NEW_COIN_NOTIFY));
    REGISTER_INTERNAL_FUNC(r2s_send_item_mail_notify, &KRelayClient::OnSendItemMailNotify, sizeof(R2S_SEND_ITEM_MAIL_NOTIFY));

    //AutoCode:注册协议
}

BOOL KRelayClient::Init()
{
    BOOL               bResult          = false;
    BOOL               bRetCode         = false;
    IIniFile*	       piIniFile        = NULL;
	int                nRelayPort       = 0;
	int                nPingCycle       = 0;
    char               szRelayAddr[16];
    KG_SocketConnector Connector;
    struct timeval     TimeVal;
    
    ResetPakStat();
    
    m_pbySyncRoleBuffer = new BYTE[MAX_ROLE_DATA_SIZE];
    KGLOG_PROCESS_ERROR(m_pbySyncRoleBuffer);

    m_uSyncRoleOffset = 0;
    m_dwSyncRoleID    = ERROR_ID;

    m_pbySaveRoleBuffer = new BYTE[MAX_ROLE_DATA_SIZE];
    KGLOG_PROCESS_ERROR(m_pbySaveRoleBuffer);

    m_pSaveBuf = new T3DB::KPB_SAVE_DATA;
    KGLOG_PROCESS_ERROR(m_pSaveBuf);

    m_pLoadBuf = new T3DB::KPB_SAVE_DATA;
    KGLOG_PROCESS_ERROR(m_pLoadBuf);

    piIniFile = g_OpenIniFile(GS_SETTINGS_FILENAME);
	KGLOG_PROCESS_ERROR(piIniFile);

    piIniFile->GetString("GC-GS", "IP", "127.0.0.1", szRelayAddr, 16);
	piIniFile->GetInteger("GC-GS", "Port", 5001, &nRelayPort);
	piIniFile->GetInteger("GC-GS", "PingCycle", 20, &nPingCycle);
    
	m_piSocketStream = Connector.Connect(szRelayAddr, nRelayPort);
	KGLOG_PROCESS_ERROR(m_piSocketStream);

	TimeVal.tv_sec  = 1;
	TimeVal.tv_usec = 0;

	bRetCode = m_piSocketStream->SetTimeout(&TimeVal);
	KGLOG_PROCESS_ERROR(bRetCode);
    
    m_bSocketError        = false;
	m_nPingCycle          = nPingCycle;
    m_nWorldIndex         = 0;

    m_nRecvPakSpeed       = 0;
    m_nSendPakSpeed       = 0;
    m_fRecvPakSpeed       = 0.0f;
    m_fSendPakSpeed       = 0.0f;
    m_nUpTraffic          = 0;
    m_nDownTraffic        = 0;
    m_fUpTraffic          = 0.0f;
    m_fDownTraffic        = 0.0f;

    bRetCode = DoApplyServerConfigRequest();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = RecvConfigFromRelay();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = DoHandshakeRequest();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        KG_COM_RELEASE(m_piSocketStream);
        KG_DELETE_ARRAY(m_pbySaveRoleBuffer);
        KG_DELETE_ARRAY(m_pbySyncRoleBuffer);
        KG_DELETE(m_pSaveBuf);
        KG_DELETE(m_pLoadBuf);
   }

    KGLogPrintf(
        KGLOG_INFO, "Connect to center server %s:%d ... ... [%s]",
        szRelayAddr, nRelayPort, bResult ? "OK" : "Failed"
    );
    
    KG_COM_RELEASE(piIniFile);
    return bResult;
}

void KRelayClient::UnInit(void)
{
    KG_COM_RELEASE(m_piSocketStream);
    KG_DELETE_ARRAY(m_pbySaveRoleBuffer);
    KG_DELETE_ARRAY(m_pbySyncRoleBuffer);
    KG_DELETE(m_pSaveBuf);
    KG_DELETE(m_pLoadBuf);

}

struct KSearchPlayerForExitingSave
{
    KSearchPlayerForExitingSave()
    {
        m_pPlayer = NULL;
    }

    BOOL operator()(KPlayer* pPlayer)
    {
        assert(pPlayer);
        if (pPlayer->m_nConnIndex >= 0)
        {
            m_pPlayer = pPlayer;
            return false;
        }

        return true;
    }

    KPlayer* m_pPlayer;
};

void KRelayClient::Activate()
{
    if (m_bQuiting)
    {
        int nPlayerCount = (int)g_pSO3World->m_PlayerSet.GetObjCount();
        if (nPlayerCount == 0)
        {
            g_pSO3World->m_bRunFlag = false;
        }

        if (g_pSO3World->m_nCurrentTime > m_nNextQuitingSaveTime)
        {
            KSearchPlayerForExitingSave TraverseFunc;

            g_pSO3World->m_PlayerSet.Traverse(TraverseFunc);

            if (TraverseFunc.m_pPlayer)
            {
                g_PlayerServer.Detach(TraverseFunc.m_pPlayer->m_nConnIndex);
            }

            m_nNextQuitingSaveTime = g_pSO3World->m_nCurrentTime + 1;
        }
    }

    DoUpdatePerformance();

    if (g_pSO3World->m_nGameLoop % GAME_FPS == 0)
    {
        m_fRecvPakSpeed = m_fRecvPakSpeed * 0.9f + (float)m_nRecvPakSpeed * 0.1f;
        m_fSendPakSpeed = m_fSendPakSpeed * 0.9f + (float)m_nSendPakSpeed * 0.1f;

        m_nRecvPakSpeed = 0;
        m_nSendPakSpeed = 0;

        m_fUpTraffic    = m_fUpTraffic * 0.9f + (float)m_nUpTraffic * 0.1f;
        m_fDownTraffic  = m_fDownTraffic * 0.9f + (float)m_nDownTraffic * 0.1f;

        m_nUpTraffic    = 0;
        m_nDownTraffic  = 0;
    }
}

BOOL KRelayClient::ProcessPackage()
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
          
        KGLOG_PROCESS_ERROR(pHeader->wProtocolID < r2s_protocol_end);
        
        uPakSize = piBuffer->GetSize();
        KGLOG_PROCESS_ERROR(uPakSize >= m_uProtocolSize[pHeader->wProtocolID]);

        m_nDownTraffic += (int)uPakSize;
        m_nRecvPakSpeed++;

        m_R2SPakStat[pHeader->wProtocolID].dwPackCount++;
        m_R2SPakStat[pHeader->wProtocolID].uTotalSize += uPakSize;

        pFunc = m_ProcessProtocolFuns[pHeader->wProtocolID];
        if (pFunc == NULL)
        {
            LogInfo("Procotol %d not process!", pHeader->wProtocolID);
            goto Exit0;
        }

		(this->*pFunc)((BYTE*)pHeader, uPakSize);
	}
     
    bResult = true;
Exit0:
    if (m_piSocketStream && m_bSocketError)
    {
        KGLogPrintf(KGLOG_DEBUG, "[Relay] LastPakTime = %u\n", m_nLastSendPacketTime);
        KGLogPrintf(KGLOG_DEBUG, "[Relay] CurrentTime = %u\n", g_pSO3World->m_nCurrentTime);

        KGLogPrintf(KGLOG_ERR, "Game center lost, shutdown !\n");

        m_bQuiting = true;
        KG_COM_RELEASE(m_piSocketStream);
    }
    KG_COM_RELEASE(piBuffer);
	return bResult;
}

BOOL KRelayClient::SaveRoleData(KPlayer* pPlayer)
{
    BOOL            bResult      = false;
    BOOL            bRetCode     = false;
    size_t          uRoleDataLen = 0;
    BYTE*           pbyPos       = m_pbySaveRoleBuffer;
    BYTE*           pbyTail      = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    m_pSaveBuf->Clear();
    bRetCode = pPlayer->SaveToProtoBuf(&uRoleDataLen, m_pSaveBuf, m_pbySaveRoleBuffer, MAX_ROLE_DATA_SIZE);

    //bRetCode = pPlayer->LoadFromProtoBuf(m_pbySaveRoleBuffer, m_pLoadBuf, uRoleDataLen);

    //bRetCode = pPlayer->Save(&uRoleDataLen, m_pbySaveRoleBuffer, MAX_ROLE_DATA_SIZE);

    KGLOG_PROCESS_ERROR(bRetCode);

    pbyTail = m_pbySaveRoleBuffer + uRoleDataLen;

    while (pbyPos < pbyTail)
    {
        size_t uSubPakSize = MIN((int)(pbyTail - pbyPos), MAX_ROLE_DATA_PAK_SIZE);

        bRetCode = DoSyncRoleData(pPlayer->m_dwID, pbyPos, pbyPos - m_pbySaveRoleBuffer, uSubPakSize);
        KGLOG_PROCESS_ERROR(bRetCode);

        pbyPos += uSubPakSize;
    }

    bRetCode = DoSaveRoleData(pPlayer, uRoleDataLen);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KRelayClient::ResetPakStat()
{
    memset(m_S2RPakStat, 0, sizeof(m_S2RPakStat));
    memset(m_R2SPakStat, 0, sizeof(m_R2SPakStat));
}

BOOL KRelayClient::DumpPakStat()
{
    BOOL  bResult   = false;
    FILE* pFile     = NULL;

    pFile = fopen("s2r_pak_stat.tab", "w");
    KG_PROCESS_ERROR(pFile);

    fprintf(pFile, "Protocol\tPackageCount\tTotalSize\n");

    for (int nProtocol = s2r_protocol_begin + 1; nProtocol < s2r_protocol_end; nProtocol++)
    {
        KPROTOCOL_STAT_INFO* pStatInfo = m_S2RPakStat + nProtocol;

        fprintf(pFile, "%d\t%u\t%llu\n", nProtocol, pStatInfo->dwPackCount, pStatInfo->uTotalSize);
    }

    fclose(pFile);
    pFile = NULL;

    pFile = fopen("r2s_pak_stat.tab", "w");
    KG_PROCESS_ERROR(pFile);

    fprintf(pFile, "Protocol\tPackageCount\tTotalSize\n");

    for (int nProtocol = r2s_protocol_begin + 1; nProtocol < r2s_protocol_end; nProtocol++)
    {
        KPROTOCOL_STAT_INFO* pStatInfo = m_R2SPakStat + nProtocol;

        fprintf(pFile, "%d\t%u\t%llu\n", nProtocol, pStatInfo->dwPackCount, pStatInfo->uTotalSize);
    }

    bResult = true;
Exit0:
    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }
    return bResult;
}

BOOL KRelayClient::Send(IKG_Buffer* piBuffer)
{
    BOOL                        bResult      = false;
    int                         nRetCode     = false;
    INTERNAL_PROTOCOL_HEADER*   pHeader      = NULL;
    size_t                      uPakSize     = 0;

    assert(piBuffer);
    KGLOG_PROCESS_ERROR(m_piSocketStream);
    KG_PROCESS_ERROR(!m_bSocketError);

    uPakSize = piBuffer->GetSize();
    assert(uPakSize >= sizeof(INTERNAL_PROTOCOL_HEADER));

    pHeader = (INTERNAL_PROTOCOL_HEADER*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pHeader);

    assert(pHeader->wProtocolID > s2r_protocol_begin);
    assert(pHeader->wProtocolID < s2r_protocol_end);

    m_S2RPakStat[pHeader->wProtocolID].dwPackCount++;
    m_S2RPakStat[pHeader->wProtocolID].uTotalSize += uPakSize;

    m_nSendPakSpeed++;

	nRetCode = m_piSocketStream->Send(piBuffer);
	if (nRetCode != 1)
    {
        m_bSocketError = true;
        goto Exit0;
    }

    m_nLastSendPacketTime = g_pSO3World->m_nCurrentTime;

    m_nUpTraffic += piBuffer->GetSize();

    bResult = true;
Exit0:
	return bResult;
}

BOOL KRelayClient::RecvConfigFromRelay()
{
	BOOL                        bResult 	= false;
	int                         nRetCode 	= false;
    IKG_Buffer*                 piBuffer    = NULL;
    const struct timeval        TimeVal     = {10, 0};
    INTERNAL_PROTOCOL_HEADER*   pHeader     = NULL;
    size_t                      uPakSize    = 0;
    PROCESS_PROTOCOL_FUNC       pFunc       = NULL;
        
	nRetCode = m_piSocketStream->CheckCanRecv(&TimeVal);
    KGLOG_PROCESS_ERROR(nRetCode == 1);

	nRetCode = m_piSocketStream->Recv(&piBuffer);
	KGLOG_PROCESS_ERROR(nRetCode == 1);

	pHeader = (INTERNAL_PROTOCOL_HEADER*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pHeader);
    
    KGLOG_PROCESS_ERROR(pHeader->wProtocolID == r2s_apply_server_config_respond);
        
    uPakSize = piBuffer->GetSize();
    KGLOG_PROCESS_ERROR(uPakSize == m_uProtocolSize[pHeader->wProtocolID]);

    OnApplyServerConfigRespond((BYTE*)pHeader, uPakSize);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piBuffer);
	return bResult;
}

// 发送协议
static BOOL gs_CopyRoomHeroInfo(S2R_ROOM_HERO_INFO* pRoomHeroInfo, KHeroData* pHeroData)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
    KGLOG_PROCESS_ERROR(pRoomHeroInfo);
    KGLOG_PROCESS_ERROR(pHeroData);

    pRoomHeroInfo->dwTempalteID = pHeroData->m_dwTemplateID;
    pRoomHeroInfo->nLadderLevel = pHeroData->m_nLadderLevel;
    pRoomHeroInfo->nNormalLevel = pHeroData->m_nLevel;
	
	bResult = true;
Exit0:
	return bResult;
}

BOOL KRelayClient::DoApplyServerConfigRequest()
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_APPLY_SERVER_CONFIG_REQUEST*  pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer((unsigned)sizeof(S2R_APPLY_SERVER_CONFIG_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_APPLY_SERVER_CONFIG_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID = s2r_apply_server_config_request;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoHandshakeRequest()
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_HANDSHAKE_REQUEST*  pHandshake      = NULL;

    piPackage = KG_MemoryCreateBuffer((unsigned)sizeof(S2R_HANDSHAKE_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pHandshake = (S2R_HANDSHAKE_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pHandshake);

    pHandshake->wProtocolID = s2r_handshake_request;
    pHandshake->nServerTime = (int)g_pSO3World->m_nCurrentTime;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoPingSignal()
{
    BOOL						bResult				= false;
    BOOL						bRetCode			= false;
    IKG_Buffer*                 piSendBuffer        = NULL;
    S2R_PING_SIGNAL*			pPingSingal			= NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2R_PING_SIGNAL));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPingSingal = (S2R_PING_SIGNAL*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPingSingal);

    pPingSingal->wProtocolID       = s2r_ping_signal;
    pPingSingal->dwReserved        = 0;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);   

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KRelayClient::DoUpdatePerformance()
{
    BOOL						bResult				= false;
    BOOL						bRetCode			= false;
    IKG_Buffer*                 piSendBuffer        = NULL;
    S2R_UPDATE_PERFORMANCE*	    pPerform			= NULL;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2R_UPDATE_PERFORMANCE));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPerform = (S2R_UPDATE_PERFORMANCE*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPerform);

    pPerform->wProtocolID       = s2r_update_performance;
    pPerform->nIdleFrame        = (unsigned int)g_pSO3World->m_nIdleFrame;
    pPerform->uMemory           = (unsigned int)KMemory::GetUsedSize();
    pPerform->nPlayerCount      = (int)g_pSO3World->m_PlayerSet.GetObjCount();
    pPerform->nConnectionCount  = 0;
    pPerform->fNetworkFlux      = 0;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);   

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KRelayClient::DoPlayerLoginRespond(DWORD dwPlayerID, BOOL bPermit, GUID Guid, DWORD dwPacketIdentity)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    IKG_Buffer*                 piPackage   = NULL;
    S2R_PLAYER_LOGIN_RESPOND*   pRespond    = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PLAYER_LOGIN_RESPOND));
    KGLOG_PROCESS_ERROR(piPackage);

    pRespond = (S2R_PLAYER_LOGIN_RESPOND*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRespond);

    pRespond->wProtocolID       = s2r_player_login_respond;
    pRespond->dwPacketIdentity  = dwPacketIdentity;
    pRespond->bPermit           = bPermit;
    pRespond->dwPlayerID        = dwPlayerID;
    pRespond->Guid              = Guid;
    pRespond->dwGSAddr          = g_PlayerServer.m_dwInternetAddr;
    pRespond->nGSPort           = g_PlayerServer.m_nListenPort;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoConfirmPlayerLoginRequest(DWORD dwPlayerID, DWORD dwClientIP)
{
    BOOL                                bResult     = false;
    BOOL                                bRetCode    = false;
    IKG_Buffer*                         piPackage   = NULL;
    S2R_CONFIRM_PLAYER_LOGIN_REQUEST*   pRequest    = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_CONFIRM_PLAYER_LOGIN_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_CONFIRM_PLAYER_LOGIN_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID         = s2r_confirm_player_login_request;
    pRequest->dwPlayerID          = dwPlayerID;
    pRequest->dwClientIP          = dwClientIP;
    pRequest->nServerIndexInLS    = g_pSO3World->m_nServerIndexInLS;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoPlayerLeaveGS(DWORD dwPlayerID)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    IKG_Buffer*             piPackage   = NULL;
    S2R_PLAYER_LEAVE_GS*    pLeaveGS    = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PLAYER_LEAVE_GS));
    KGLOG_PROCESS_ERROR(piPackage);

    pLeaveGS = (S2R_PLAYER_LEAVE_GS*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pLeaveGS);

    pLeaveGS->wProtocolID         = s2r_player_leave_gs;
    pLeaveGS->dwPlayerID          = dwPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoSubmitLimitPlayInfo( DWORD dwGroupID, DWORD dwIP, const char cszAccount[], const char cszName[], const char cszID[], const char cszEmail[] )
{
	BOOL						bResult     = false;
	BOOL						bRetCode    = false;
	IKG_Buffer*					piPackage   = NULL;
	S2R_SUBMIT_LIMITPLAY_INFO*  pPak    = NULL;

	piPackage = KG_MemoryCreateBuffer(sizeof(S2R_SUBMIT_LIMITPLAY_INFO));
	KGLOG_PROCESS_ERROR(piPackage);

	pPak = (S2R_SUBMIT_LIMITPLAY_INFO*)piPackage->GetData();
	KGLOG_PROCESS_ERROR(pPak);

	pPak->wProtocolID			= s2r_submit_limitplay_info;

	pPak->dwIP = dwIP;
	pPak->dwGroupID = dwGroupID;

	strncpy(pPak->szAccount, cszAccount, countof(pPak->szAccount));
	pPak->szAccount[countof(pPak->szAccount) -1] = '\0';

	strncpy(pPak->szName, cszName, countof(pPak->szName));
	pPak->szName[countof(pPak->szName) -1] = '\0';

	strncpy(pPak->szID, cszID, countof(pPak->szID));
	pPak->szID[countof(pPak->szID) -1] = '\0';

	strncpy(pPak->szEmail, cszEmail, countof(pPak->szEmail));
	pPak->szEmail[countof(pPak->szEmail) -1] = '\0';

	bRetCode = Send(piPackage);
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoCreateMapRespond(DWORD dwMapID, int nMapCopyIndex, BOOL bSucceed)
{
    BOOL                    bResult     = false;
    BOOL                    bRetCode    = false;
    IKG_Buffer*             piPackage   = NULL;
    S2R_CREATE_MAP_RESPOND* pRespond    = NULL;
    KMapParams*             pMapParam   = NULL;

    pMapParam = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(dwMapID);
    KGLOG_PROCESS_ERROR(pMapParam);

    KGLogPrintf(
        KGLOG_INFO, "Create map (%.2u, %d) ... ... [%s]\n", 
        dwMapID, nMapCopyIndex, bSucceed ? "OK" : "Failed"
        );

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_CREATE_MAP_RESPOND));
    KGLOG_PROCESS_ERROR(piPackage);

    pRespond = (S2R_CREATE_MAP_RESPOND*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRespond);

    pRespond->wProtocolID        = s2r_create_map_respond;
    pRespond->dwMapID            = dwMapID;
    pRespond->nMapCopyIndex      = nMapCopyIndex;
    pRespond->bResult            = bSucceed;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoSearchMapRequest(KPlayer* pPlayer, DWORD dwMapID, int nMapCopyIndex)
{
    BOOL                        bResult             = false;
    BOOL                        bRetCode            = false;
    IKG_Buffer*                 piSendBuffer        = NULL;
    S2R_SEARCH_MAP_REQUEST*     pSearchMapRequest   = NULL;
    KHeroData*                  pMainHeroData       = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(S2R_SEARCH_MAP_REQUEST));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pSearchMapRequest = (S2R_SEARCH_MAP_REQUEST*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pSearchMapRequest);

    pSearchMapRequest->wProtocolID       = s2r_search_map_request;
    pSearchMapRequest->dwPlayerID        = pPlayer->m_dwID;
    
    bRetCode = gs_CopyRoomHeroInfo(&pSearchMapRequest->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    pSearchMapRequest->dwMapID           = dwMapID;
    pSearchMapRequest->nMapCopyIndex     = nMapCopyIndex;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KRelayClient::DoTransferPlayerRequest(KPlayer* pPlayer)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    IKG_Buffer*                     piPackage       = NULL;
    S2R_TRANSFER_PLAYER_REQUEST*    pRequest        = NULL;
    size_t                          uUsedSize       = 0;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_TRANSFER_PLAYER_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_TRANSFER_PLAYER_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID           = s2r_transfer_player_request;
    pRequest->dwRoleID              = pPlayer->m_dwID;
    pRequest->ExtPointInfo          = pPlayer->m_ExtPointInfo;
    pRequest->bExtPointLock         = pPlayer->m_bExtPointLock;
    pRequest->nLastExtPointIndex    = pPlayer->m_nLastExtPointIndex;
    pRequest->nLastExtPointValue    = pPlayer->m_nLastExtPointValue;
    pRequest->nEndTimeOfFee         = (int)pPlayer->m_nEndTimeOfFee;
    pRequest->dwLimitPlayTimeFlag   = pPlayer->m_dwLimitPlayTimeFlag;
    pRequest->nIBActionTime         = pPlayer->m_nIBActionTime;
    pRequest->dwLimitOnlineSecond   = pPlayer->m_dwLimitOnlineSecond + (g_pSO3World->m_nCurrentTime - pPlayer->m_nCurrentLoginTime);
    pRequest->dwLimitOfflineSecond  = pPlayer->m_dwLimitOfflineSecond;
    pRequest->nOnlineTime           = pPlayer->m_ActivePlayer.GetOnlineTime();
    pRequest->nNextOnlineAwardTime  = pPlayer->m_nOnlineAwardTime;
    pRequest->nReportOnlineGetAward = pPlayer->m_nReportOnlineGetAward;

    bRetCode  = pPlayer->SaveBaseInfo(&pRequest->RoleBaseInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoTransferPlayerRespond(DWORD dwPlayerID , BOOL bSucceed, GUID Guid)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    IKG_Buffer*                     piPackage   = NULL;
    S2R_TRANSFER_PLAYER_RESPOND*    pRespond    = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_TRANSFER_PLAYER_RESPOND));
    KGLOG_PROCESS_ERROR(piPackage);

    pRespond = (S2R_TRANSFER_PLAYER_RESPOND*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRespond);

    pRespond->wProtocolID           = s2r_transfer_player_respond;
    pRespond->dwPlayerID            = dwPlayerID;
    pRespond->bSucceed              = bSucceed;
    pRespond->Guid                  = Guid;
    pRespond->dwAddress             = g_PlayerServer.m_dwInternetAddr;
    pRespond->wPort                 = (WORD)g_PlayerServer.m_nListenPort;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoCoinShopBuyItemRequest(
    KPlayer* pPlayer, DWORD dwTabType, DWORD dwTabIndex, int nGoodsID, 
    int nCount, int nCoinPrice, KCUSTOM_CONSUME_INFO* pCCInfo
)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    S2R_COIN_SHOP_BUY_ITEM_REQUEST* pRequest    = NULL;
    IKG_Buffer*                     piBuffer    = NULL;

    piBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2R_COIN_SHOP_BUY_ITEM_REQUEST));
    KGLOG_PROCESS_ERROR(piBuffer);

    pRequest = (S2R_COIN_SHOP_BUY_ITEM_REQUEST*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    memset(pRequest, 0, sizeof(S2R_COIN_SHOP_BUY_ITEM_REQUEST));

    pRequest->wProtocolID               = s2r_coin_shop_buy_item_request;
    pRequest->dwPlayerID                = pPlayer->m_dwID;
    pRequest->dwTabType                 = dwTabType;
    pRequest->dwTabIndex                = dwTabIndex;
    pRequest->nGoodsID                  = nGoodsID;
    pRequest->nCount                    = nCount;
    pRequest->nCoinPrice                = nCoinPrice;
    
    if (pCCInfo)
    {
        pRequest->CustomConsumeInfo.nValue1 = pCCInfo->nValue1;
        pRequest->CustomConsumeInfo.nValue2 = pCCInfo->nValue2;
        pRequest->CustomConsumeInfo.nValue3 = pCCInfo->nValue3;
        pRequest->CustomConsumeInfo.nValue4 = pCCInfo->nValue4;
		
        strncpy(pRequest->CustomConsumeInfo.szString1, pCCInfo->szString1, sizeof(pRequest->CustomConsumeInfo.szString1));
        pRequest->CustomConsumeInfo.szString1[sizeof(pRequest->CustomConsumeInfo.szString1) - 1] = '\0';
    }   

    bRetCode = Send(piBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->m_nIBActionTime = g_pSO3World->m_nCurrentTime;   // IB操作开始标志

    bResult = true;
Exit0:
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL KRelayClient::DoLoadRoleDataRequest(DWORD dwRoleID)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_LOAD_ROLE_DATA_REQUEST* pLoadRoleData   = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_LOAD_ROLE_DATA_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pLoadRoleData = (S2R_LOAD_ROLE_DATA_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pLoadRoleData);

    pLoadRoleData->wProtocolID  = s2r_load_role_data_request;
    pLoadRoleData->dwPlayerID   = dwRoleID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoSyncRoleData(DWORD dwID, BYTE* pbyData, size_t uOffset, size_t uDataLen)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    IKG_Buffer*         piPackage       = NULL;
    S2R_SYNC_ROLE_DATA* pSyncRoleData   = NULL;

    piPackage = KG_MemoryCreateBuffer((unsigned)(sizeof(S2R_SYNC_ROLE_DATA) + uDataLen));
    KGLOG_PROCESS_ERROR(piPackage);

    pSyncRoleData = (S2R_SYNC_ROLE_DATA*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSyncRoleData);

    pSyncRoleData->wProtocolID  = s2r_sync_role_data;
    pSyncRoleData->dwRoleID     = dwID;
    pSyncRoleData->uOffset      = (unsigned int)uOffset;

    memcpy(pSyncRoleData->byData, pbyData, uDataLen);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoSaveRoleData(KPlayer* pPlayer, size_t uRoleDataLen)
{
    BOOL                bResult       = false;
    BOOL                bRetCode      = false;
    IKG_Buffer*         piPackage     = NULL;
    S2R_SAVE_ROLE_DATA* pSaveRoleData = NULL;

    piPackage = KG_MemoryCreateBuffer((unsigned)sizeof(S2R_SAVE_ROLE_DATA));
    KGLOG_PROCESS_ERROR(piPackage);

    pSaveRoleData = (S2R_SAVE_ROLE_DATA*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSaveRoleData);

    pSaveRoleData->wProtocolID      = s2r_save_role_data;
    pSaveRoleData->dwRoleID         = pPlayer->m_dwID;
    pSaveRoleData->bIsTransmission  = (pPlayer->m_eGameStatus == gsWaitForTransmissionSave);
    pSaveRoleData->uRoleDataLen     = (unsigned int)uRoleDataLen;

    bRetCode = pPlayer->SaveBaseInfo(&pSaveRoleData->BaseInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoPlayerEnterSceneNotify(DWORD dwPlayerID, DWORD dwMapID, int nMapCopyIndex)
{
    BOOL                                bResult         = false;
    BOOL                                bRetCode        = false;
    IKG_Buffer*                         piPackage       = NULL;
    S2R_PLAYER_ENTER_SCENE_NOTIFY*      pNotify         = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PLAYER_ENTER_SCENE_NOTIFY));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2R_PLAYER_ENTER_SCENE_NOTIFY*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID   = s2r_player_enter_scene_notify;
    pNotify->dwPlayerID    = dwPlayerID;
    pNotify->dwMapID       = dwMapID;
    pNotify->nMapCopyIndex = nMapCopyIndex;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoPlayerEnterHallNotify(DWORD dwPlayerID)
{
    BOOL                                bResult         = false;
    BOOL                                bRetCode        = false;
    IKG_Buffer*                         piPackage       = NULL;
    S2R_PLAYER_ENTER_HALL_NOTIFY*      pNotify         = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PLAYER_ENTER_HALL_NOTIFY));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2R_PLAYER_ENTER_HALL_NOTIFY*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID   = s2r_player_enter_hall_notify;
    pNotify->dwPlayerID    = dwPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoBattleFinishedNotify(DWORD dwMapID, int nMapCopyIndex, int nWinnerSide)
{
    BOOL                                bResult         = false;
    BOOL                                bRetCode        = false;
    IKG_Buffer*                         piPackage       = NULL;
    S2R_BATTLE_FINISHED_NOTIFY*           pRequest         = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_BATTLE_FINISHED_NOTIFY));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_BATTLE_FINISHED_NOTIFY*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2r_battle_finished_notify;
    pRequest->dwMapID       = dwMapID;
    pRequest->nMapCopyIndex = nMapCopyIndex;
    assert(nWinnerSide >= 0 && nWinnerSide <= 0xFFU);
    pRequest->byWinnerSideID = (BYTE)nWinnerSide;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoApplyCreateRoom(KPlayer* pPlayer, DWORD dwMapID, const char cszRoomName[], const char cszPassword[])
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_APPLY_CREATE_ROOM*      pApply          = NULL;
    KHeroData*                  pMainHeroData   = NULL;
    KHeroData*                  pAssistHeroData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData   = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    pAssistHeroData = pPlayer->GetAssistHeroData();
    KGLOG_PROCESS_ERROR(pAssistHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_CREATE_ROOM));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_APPLY_CREATE_ROOM*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID             = s2r_apply_create_room;
    pApply->dwPlayerID              = pPlayer->m_dwID;
    pApply->dwMapID                 = dwMapID;

    bRetCode = gs_CopyRoomHeroInfo(&pApply->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = gs_CopyRoomHeroInfo(&pApply->assistHeroInfo, pAssistHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    strncpy(pApply->szName, cszRoomName, countof(pApply->szName));
    pApply->szName[countof(pApply->szName) - 1] = '\0';

    strncpy(pApply->szPassword, cszPassword, countof(pApply->szPassword));
    pApply->szPassword[countof(pApply->szPassword) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoApplyJoinRoom(KPlayer* pPlayer, DWORD dwRoomID, const char cszPassword[])
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_APPLY_JOIN_ROOM*    pApply          = NULL;
    KHeroData*              pMainHeroData   = NULL;
    KHeroData*              pAssistHeroData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData   = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    pAssistHeroData = pPlayer->GetAssistHeroData();
    KGLOG_PROCESS_ERROR(pAssistHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_JOIN_ROOM));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_APPLY_JOIN_ROOM*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID             = s2r_apply_join_room;
    pApply->dwPlayerID              = pPlayer->m_dwID;
    pApply->dwRoomID                = dwRoomID;

    bRetCode = gs_CopyRoomHeroInfo(&pApply->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = gs_CopyRoomHeroInfo(&pApply->assistHeroInfo, pAssistHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

	strncpy(pApply->szPassword, cszPassword, sizeof(pApply->szPassword));
    pApply->szPassword[sizeof(pApply->szPassword) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoInvitePlayerJoinRoom(DWORD dwInviterID, const char szDestPlayerName[])
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_INVITE_PLAYER_JOIN_ROOM*    pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_INVITE_PLAYER_JOIN_ROOM));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_INVITE_PLAYER_JOIN_ROOM*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID   = s2r_invite_player_join_room;
    pApply->dwInviterID   = dwInviterID;

    strncpy(pApply->szDestName, szDestPlayerName, countof(pApply->szDestName));
    pApply->szDestName[countof(pApply->szDestName) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoSwitchRoomHost(DWORD dwCurHostID, DWORD dwNewHostID)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_SWITCH_ROOM_HOST*   pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_SWITCH_ROOM_HOST));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_SWITCH_ROOM_HOST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID  = s2r_switch_room_host;
    pApply->dwCurHostID  = dwCurHostID;
    pApply->dwNewHostID  = dwNewHostID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoSwitchBattleMap(DWORD dwHostID, DWORD dwNewMapID)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_SWITCH_BATTLE_MAP*  pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_SWITCH_BATTLE_MAP));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_SWITCH_BATTLE_MAP*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID  = s2r_switch_battle_map;
    pApply->dwHostID     = dwHostID;
    pApply->dwNewMapID   = dwNewMapID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoSetRoomPassword(DWORD dwHostID, const char szNewPassword[])
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_SET_ROOM_PASSWORD*  pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_SET_ROOM_PASSWORD));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_SET_ROOM_PASSWORD*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID  = s2r_set_room_password;
    pApply->dwHostID     = dwHostID;
    memcpy(pApply->szNewPassword,  szNewPassword, sizeof(char) * cdRoomPasswordMaxLen);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoApplyLeaveRoom(DWORD dwPlayerID)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_APPLY_LEAVE_ROOM*    pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_LEAVE_ROOM));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_APPLY_LEAVE_ROOM*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID   = s2r_apply_leave_room;
    pApply->dwPlayerID    = dwPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoApplyKickoutOther(DWORD dwPlayerID, DWORD dwOtherPlayerID)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_APPLY_KICKOUT_OTHER*    pApply          = NULL;

    KGLOG_PROCESS_ERROR(dwPlayerID != dwOtherPlayerID);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_KICKOUT_OTHER));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_APPLY_KICKOUT_OTHER*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID   = s2r_apply_kickout_other;
    pApply->dwPlayerID    = dwPlayerID;
    pApply->dwOtherPlayerID = dwOtherPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoApplySetReady(DWORD dwPlayerID, BOOL bReady)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_APPLY_SET_READY*    pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_SET_READY));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_APPLY_SET_READY*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID   = s2r_apply_set_ready;
    pApply->dwPlayerID    = dwPlayerID;
    pApply->bReady        = (BYTE)bReady;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoApplyStartGame(DWORD dwPlayerID)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_APPLY_START_GAME*    pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_START_GAME));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_APPLY_START_GAME*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID   = s2r_apply_start_game;
    pApply->dwPlayerID    = dwPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}


BOOL KRelayClient::DoRoomMemberChangePosRequest(DWORD dwPlayerID, KSIDE_TYPE eNewSide, int nNewPos)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_ROOMMEMBER_CHANGE_POS_REQUEST*    pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_ROOMMEMBER_CHANGE_POS_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_ROOMMEMBER_CHANGE_POS_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID   = s2r_roommember_change_pos_request;
    pApply->dwPlayerID    = dwPlayerID;
    assert(eNewSide >= 0 && eNewSide <= 0xFFU);
    pApply->bySide        = (BYTE)eNewSide;

    KGLOG_PROCESS_ERROR(nNewPos >= 0 && nNewPos < MAX_TEAM_MEMBER_COUNT);
    pApply->nPos = nNewPos;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoChangeRoomName(DWORD dwPlayerID, char szNewName[])
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_CHANGE_ROOMNAME*    pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_CHANGE_ROOMNAME));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_CHANGE_ROOMNAME*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID   = s2r_change_roomname;
    pApply->dwPlayerID = dwPlayerID;
    strncpy(pApply->szNewName, szNewName, countof(pApply->szNewName));
    pApply->szNewName[countof(pApply->szNewName) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoSelectHeroRequest(KPlayer* pPlayer)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_SELECT_HERO_REQUEST*    pApply          = NULL;
    KHeroData*                  pMainHeroData   = NULL;
    KHeroData*                  pAssistHeroData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    pAssistHeroData = pPlayer->GetAssistHeroData();
    KG_PROCESS_ERROR(pAssistHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_SELECT_HERO_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_SELECT_HERO_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID             = s2r_select_hero_request;
    pApply->dwPlayerID              = pPlayer->m_dwID;

    pApply->wMainHeroTemplateID     = (WORD)pMainHeroData->m_dwTemplateID;
    pApply->nMainHeroNormalLevel    = pMainHeroData->m_nLevel;
    pApply->nMainHeroLadderLevel    = pMainHeroData->m_nLadderLevel;

    pApply->wAssistHeroTemplateID   = (WORD)pAssistHeroData->m_dwTemplateID;
    pApply->nAssistHeroNormalLevel  = pAssistHeroData->m_nLevel;
    pApply->nAssistHeroLadderLevel  = pAssistHeroData->m_nLadderLevel;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}


BOOL KRelayClient::DoAutoJoinRoom(KPlayer* pPlayer)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_AUTO_JOINROOM*          pApply          = NULL;
    KHeroData*                  pMainHeroData   = NULL;
    KHeroData*                  pAssistHeroData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    pAssistHeroData = pPlayer->GetAssistHeroData();
    KGLOG_PROCESS_ERROR(pAssistHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_AUTO_JOINROOM));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_AUTO_JOINROOM*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID             = s2r_auto_joinroom;
    pApply->dwPlayerID              = pPlayer->m_dwID;

    bRetCode = gs_CopyRoomHeroInfo(&pApply->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = gs_CopyRoomHeroInfo(&pApply->assistHeroInfo, pAssistHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoApplySetRoomAIMode(DWORD dwPlayerID, KSIDE_TYPE eSide, int nPos, BOOL bAIMode)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_APPLY_SET_ROOM_AIMODE*  pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_SET_ROOM_AIMODE));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_APPLY_SET_ROOM_AIMODE*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID     = s2r_apply_set_room_aimode;
    pApply->dwRoleID        = dwPlayerID;
    pApply->bySide          = (BYTE)eSide;
    pApply->byPos           = (BYTE)nPos;
    pApply->byAIMode        = (BYTE)bAIMode;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoTryJoinRoom(KPlayer* pPlayer, DWORD dwRoomID)
{
	BOOL bResult = false;
	BOOL bRetCode = false;
    IKG_Buffer*           piPackage       = NULL;
    S2R_TRY_JOIN_ROOM*    pRequest        = NULL;
    KHeroData*            pMainHeroData   = NULL;
    KHeroData*            pAssistHeroData = NULL;

	KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    pAssistHeroData = pPlayer->GetAssistHeroData();
    KGLOG_PROCESS_ERROR(pAssistHeroData);
	
    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_TRY_JOIN_ROOM));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_TRY_JOIN_ROOM*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID               = s2r_try_join_room;
    pRequest->dwPlayerID                = pPlayer->m_dwID;
    pRequest->dwRoomID                  = dwRoomID;

    bRetCode = gs_CopyRoomHeroInfo(&pRequest->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = gs_CopyRoomHeroInfo(&pRequest->assistHeroInfo, pAssistHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoAcceptOrRefuseJoinRoom(KPlayer* pPlayer, int nAcceptCode, DWORD dwRoomID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer*                         piPackage       = NULL;
    S2R_ACCEPT_OR_REFUSE_ROOM_INVITE*   pRequest        = NULL;
    KHeroData*                          pMainHeroData   = NULL;
    KHeroData*                          pAssistHeroData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    pAssistHeroData = pPlayer->GetAssistHeroData();
    KGLOG_PROCESS_ERROR(pAssistHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_ACCEPT_OR_REFUSE_ROOM_INVITE));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_ACCEPT_OR_REFUSE_ROOM_INVITE*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID            = s2r_accept_or_refuse_room_invite;
    pRequest->byAcceptCode           = (BYTE)nAcceptCode;
    pRequest->dwPlayerID             = pPlayer->m_dwID;
    pRequest->dwRoomID               = dwRoomID;

    bRetCode = gs_CopyRoomHeroInfo(&pRequest->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = gs_CopyRoomHeroInfo(&pRequest->assistHeroInfo, pAssistHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

// 组队
BOOL KRelayClient::DoCreateTeamRequest(KPlayer* pPlayer)
{
	BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_CREATE_TEAM_REQUEST*    pApply          = NULL;
    KHeroData*                  pHeroData       = NULL;

	assert(pPlayer);

    pHeroData = pPlayer->m_HeroDataList.GetHeroData(pPlayer->m_dwMainHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_CREATE_TEAM_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_CREATE_TEAM_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID         = s2r_create_team_request;
    pApply->dwPlayerID          = pPlayer->m_dwID;
    pApply->byGender            = (BYTE)pPlayer->m_eGender;
    pApply->byVIPLevel          = 0;
    pApply->dwHeroTemplateID    = pHeroData->m_dwTemplateID;
    pApply->nEol                = pHeroData->m_nEol;
    pApply->nLevel        		= pHeroData->m_nLevel;
    pApply->nLadderLevel        = pHeroData->m_nLadderLevel;
    pApply->nLadderBattleCount  = pHeroData->m_nLadderWinCount + pHeroData->m_nLadderLoseCount;
    pApply->nLadderLosingStreakCount = pPlayer->m_nLadderLosingStreakCount;

    strncpy(pApply->szName, pPlayer->m_szName, countof(pApply->szName));
    pApply->szName[countof(pApply->szName) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoTeamInvitePlayerRequest(DWORD dwPlayerID, const char cszTargetName[])
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_TEAM_INVITE_PLAYER_REQUEST*    pApply   = NULL;

    assert(cszTargetName);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_TEAM_INVITE_PLAYER_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_TEAM_INVITE_PLAYER_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID     = s2r_team_invite_player_request;
    pApply->dwPlayerID      = dwPlayerID;

    strncpy(pApply->szTargetName, cszTargetName, countof(pApply->szTargetName));
    pApply->szTargetName[countof(pApply->szTargetName) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoTeamKickoutPlayerRequest(DWORD dwPlayerID, DWORD dwBeKickedPlayerID)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_TEAM_KICKOUT_PLAYER_REQUEST*    pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_TEAM_KICKOUT_PLAYER_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_TEAM_KICKOUT_PLAYER_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID         = s2r_team_kickout_player_request;
    pApply->dwPlayerID          = dwPlayerID;
    pApply->dwBeKickedPlayerID  = dwBeKickedPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoTeamLeaveRequest(DWORD dwPlayerID)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_TEAM_LEAVE_REQUEST*    pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_TEAM_LEAVE_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_TEAM_LEAVE_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID     = s2r_team_leave_request;
    pApply->dwLeaverID      = dwPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoTeamAcceptOrRefuseInvite(KPlayer* pTarget, int nAcceptCode, const char (&szInviterName)[_NAME_LEN])
{
    BOOL                                bResult         = false;
    BOOL                                bRetCode        = false;
    IKG_Buffer*                         piPackage       = NULL;
    S2R_TEAM_ACCEPT_OR_REFUSE_INVITE*   pApply          = NULL;
    KHeroData*                          pHeroData       = NULL;
    size_t                              uNameLen        = 0U;
	
	assert(pTarget);	
    assert(szInviterName);

    pHeroData = pTarget->m_HeroDataList.GetHeroData(pTarget->m_dwMainHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_TEAM_ACCEPT_OR_REFUSE_INVITE));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_TEAM_ACCEPT_OR_REFUSE_INVITE*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID         = s2r_team_accept_or_refuse_invite;
    pApply->byAcceptCode        = (BYTE)nAcceptCode;
    pApply->dwTargetID          = pTarget->m_dwID;
    pApply->byGender            = (BYTE)pTarget->m_eGender;
    pApply->byVIPLevel          = 0;
    pApply->dwHeroTemplateID    = pHeroData->m_dwTemplateID;
    pApply->nEol                = pHeroData->m_nEol;
    pApply->nLevel              = pHeroData->m_nLevel;
    pApply->nLadderLevel        = pHeroData->m_nLadderLevel;
    pApply->nLadderLosingStreakCount = pTarget->m_nLadderLosingStreakCount;    

    g_CStringCopy(pApply->szInviterName, szInviterName);
    g_CStringCopy(pApply->szName, pTarget->m_szName);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoTeamReadyRequest(uint32_t dwPlayerID, BOOL bReady)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_TEAM_READY_REQUEST* pApply          = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_TEAM_READY_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pApply = (S2R_TEAM_READY_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pApply);

    pApply->wProtocolID = s2r_team_ready_request;
    pApply->dwPlayerID  = dwPlayerID;
    pApply->byReady     = (BYTE)bReady;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

// PVP匹配
BOOL KRelayClient::DoAutoMatchRequest(KPlayer* pRequestor)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_AUTOMATCH_REQUEST*      pSendData       = NULL;
    
    KGLOG_PROCESS_ERROR(pRequestor);

    KGLOG_PROCESS_ERROR(pRequestor->m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!pRequestor->m_bIsMatching);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_AUTOMATCH_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_AUTOMATCH_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID  = s2r_automatch_request;
    pSendData->dwPlayerID   = pRequestor->m_dwID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoCancelAutoMatchRequest(KPlayer* pPlayer)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    IKG_Buffer*                     piPackage       = NULL;
    S2R_CANCEL_AUTOMATCH_REQUEST*   pSendData       = NULL;

    assert(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_bIsMatching);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_CANCEL_AUTOMATCH_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_CANCEL_AUTOMATCH_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID  = s2r_cancel_automatch_request;
    pSendData->dwPlayerID   = pPlayer->m_dwID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoRenameRequest(DWORD dwPlayerID, char szNewName[])
{
	BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_RENAME_REQUEST*         pSendData       = NULL;

    assert(szNewName);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_RENAME_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_RENAME_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID  = s2r_rename_request;
    pSendData->dwRoleID     = dwPlayerID;

    strncpy(pSendData->szNewName, szNewName, countof(pSendData->szNewName));
    pSendData->szNewName[countof(pSendData->szNewName) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoUpdateRoleLevel(uint32_t dwPlayerID, int nNewLevel)
{
	BOOL                bResult     = false;
	BOOL                bRetCode    = false;
    IKG_Buffer*         piPackage   = NULL;
    S2R_UPDATE_ROLE_LEVEL*   pSendData   = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_UPDATE_ROLE_LEVEL));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_UPDATE_ROLE_LEVEL*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID  = s2r_update_role_level;
    pSendData->dwRoleID     = dwPlayerID;
    pSendData->nNewLevel    = nNewLevel;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoApplyRoomBaseInfoForCache(DWORD dwLastRoomID)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_APPLY_ROOM_BASEINFO_FOR_CACHE*         pSendData       = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_ROOM_BASEINFO_FOR_CACHE));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_APPLY_ROOM_BASEINFO_FOR_CACHE*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID  = s2r_apply_room_baseinfo_for_cache;
    pSendData->dwRoomID     = dwLastRoomID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoCreatePveTeamRequest(KPlayer* pPlayer, int nPveMode, int nMissionType, int nMissionStep, int nMissionLevel, uint32_t dwMapID, const char (&cszTeamName)[_NAME_LEN], const char (&cszPassword)[cdRoomPasswordMaxLen])
{
    BOOL                            bResult         = false;
	BOOL                            bRetCode        = false;
    IKG_Buffer*                     piPackage       = NULL;
    KHeroData*                      pMainHeroData   = NULL;
    S2R_CREATE_PVE_TEAM_REQUEST*    pSendData       = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);
    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_CREATE_PVE_TEAM_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_CREATE_PVE_TEAM_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_create_pve_team_request;
    pSendData->dwPlayerID       = pPlayer->m_dwID;
    pSendData->byPVEMode        = (BYTE)nPveMode;
    pSendData->byType           = (BYTE)nMissionType;
    pSendData->byStep           = (BYTE)nMissionStep;
    pSendData->byLevel          = (BYTE)nMissionLevel;
    pSendData->dwMapID          = dwMapID;

    bRetCode = gs_CopyRoomHeroInfo(&pSendData->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    g_CStringCopy(pSendData->szTeamName,  cszTeamName);
    g_CStringCopy(pSendData->szPassword, cszPassword);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoQuickStartPveRequest(
	KPlayer* pPlayer, int nPveMode, int nMissionType, int nMissionStep, int nMissionLevel, 
	uint32_t dwMapID, char szTeamName[], char szPassword[]
)
{
	BOOL                            bResult         = false;
	BOOL                            bRetCode        = false;
	IKG_Buffer*                     piPackage       = NULL;
	KHeroData*                      pMainHeroData   = NULL;
	S2R_PVE_QUICK_START_REQUEST*    pSendData       = NULL;

	KGLOG_PROCESS_ERROR(pPlayer);
	pMainHeroData = pPlayer->GetMainHeroData();
	KGLOG_PROCESS_ERROR(pMainHeroData);

	piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_QUICK_START_REQUEST));
	KGLOG_PROCESS_ERROR(piPackage);

	pSendData = (S2R_PVE_QUICK_START_REQUEST*)piPackage->GetData();
	KGLOG_PROCESS_ERROR(pSendData);

	pSendData->wProtocolID      = s2r_pve_quick_start_request;
	pSendData->dwPlayerID       = pPlayer->m_dwID;
	pSendData->byPVEMode        = (BYTE)nPveMode;
	pSendData->byType           = (BYTE)nMissionType;
	pSendData->byStep           = (BYTE)nMissionStep;
	pSendData->byLevel          = (BYTE)nMissionLevel;
	pSendData->dwMapID          = dwMapID;

    bRetCode = gs_CopyRoomHeroInfo(&pSendData->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

	strncpy(pSendData->szTeamName, szTeamName, countof(pSendData->szTeamName));
	pSendData->szTeamName[countof(pSendData->szTeamName) - 1] = '\0';

	memcpy(pSendData->szPassword, szPassword, countof(pSendData->szPassword));

	bRetCode = Send(piPackage);
	KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoApplyJoinPveTeamRequest(KPlayer* pPlayer, uint32_t dwTeamID, char szPassword[])
{
    BOOL                                bResult         = false;
	BOOL                                bRetCode        = false;
    IKG_Buffer*                         piPackage       = NULL;
    KHeroData*                          pMainHeroData   = NULL;
    S2R_APPLY_JOIN_PVE_TEAM_REQUEST*    pSendData       = NULL;

    assert(szPassword);
    KGLOG_PROCESS_ERROR(pPlayer);
    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_JOIN_PVE_TEAM_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_APPLY_JOIN_PVE_TEAM_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_apply_join_pve_team_request;
    pSendData->dwPlayerID       = pPlayer->m_dwID;
    pSendData->dwTeamID         = dwTeamID;

    bRetCode = gs_CopyRoomHeroInfo(&pSendData->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    memcpy(pSendData->szPassword, szPassword, countof(pSendData->szPassword));

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoApplyPveTeamInfoRequest(uint32_t dwPlayerID, int nPage, int nPveMode, int nMissionType, int nMissiongStep)
{
    BOOL                bResult     = false;
	BOOL                bRetCode    = false;
    IKG_Buffer*         piPackage   = NULL;
    S2R_APPLY_PVE_TEAM_INFO_REQUEST*   pSendData   = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_APPLY_PVE_TEAM_INFO_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_APPLY_PVE_TEAM_INFO_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_apply_pve_team_info_request;
    pSendData->dwPlayerID       = dwPlayerID;
    pSendData->byPage           = (BYTE)nPage;
    pSendData->byPVEMode        = (BYTE)nPveMode;
    pSendData->byMissionType    = (BYTE)nMissionType;
    pSendData->byMissionStep    = (BYTE)nMissiongStep;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamInvitePlayerRequest(DWORD dwPlayerID, const char cszTargetName[])
{
    BOOL                bResult     = false;
	BOOL                bRetCode    = false;
    IKG_Buffer*         piPackage   = NULL;
    S2R_PVE_TEAM_INVITE_PLAYER_REQUEST*   pSendData   = NULL;

    assert(cszTargetName);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_INVITE_PLAYER_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_INVITE_PLAYER_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID  = s2r_pve_team_invite_player_request;
    pSendData->dwPlayerID   = dwPlayerID;

    strncpy(pSendData->szTargetName, cszTargetName, countof(pSendData->szTargetName));
    pSendData->szTargetName[countof(pSendData->szTargetName) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamKickoutPlayerRequest(DWORD dwPlayerID, DWORD dwBeKickedPlayerID)
{
    BOOL                bResult     = false;
	BOOL                bRetCode    = false;
    IKG_Buffer*         piPackage   = NULL;
    S2R_PVE_TEAM_KICKOUT_PLAYER_REQUEST*   pSendData = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_KICKOUT_PLAYER_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_KICKOUT_PLAYER_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_kickout_player_request;
    pSendData->dwPlayerID       = dwPlayerID;
    pSendData->dwDestPlayerID   = dwBeKickedPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamLeaveRequest(DWORD dwPlayerID)
{
    BOOL                bResult     = false;
	BOOL                bRetCode    = false;
    IKG_Buffer*         piPackage   = NULL;
    S2R_PVE_TEAM_LEAVE_REQUEST*   pSendData = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_LEAVE_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_LEAVE_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_leave_request;
    pSendData->dwPlayerID       = dwPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamAcceptOrRefuseInvite(KPlayer* pPlayer, int nAccpetCode, const char cszInviterName[])
{
    BOOL                                    bResult         = false;
	BOOL                                    bRetCode        = false;
    IKG_Buffer*                             piPackage       = NULL;
    KHeroData*                              pMainHeroData   = NULL;
    S2R_PVE_TEAM_ACCEPT_OR_REFUSE_INVITE*   pSendData       = NULL;

    assert(cszInviterName);

    KGLOG_PROCESS_ERROR(pPlayer);
    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_ACCEPT_OR_REFUSE_INVITE));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_ACCEPT_OR_REFUSE_INVITE*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_accept_or_refuse_invite;
    pSendData->dwPlayerID       = pPlayer->m_dwID;
    pSendData->byAcceptCode     = (BYTE)nAccpetCode;

    bRetCode = gs_CopyRoomHeroInfo(&pSendData->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    strncpy(pSendData->szInviter, cszInviterName, countof(pSendData->szInviter));
    pSendData->szInviter[countof(pSendData->szInviter) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamReadyRequest(KPlayer* pPlayer, BOOL bReady)
{
    BOOL                        bResult     = false;
    BOOL                        bRetCode    = false;
    IKG_Buffer*                 piPackage   = NULL;
    S2R_PVE_TEAM_READY_REQUEST* pSendData   = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_READY_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_READY_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_ready_request;
    pSendData->dwPlayerID       = pPlayer->m_dwID;
    pSendData->byReady          = (BYTE)bReady;
    pSendData->wCurFatiguePoint = (uint16_t)pPlayer->m_nCurFatiguePoint;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamSetAiMode(KPlayer* pPlayer, BOOL bIsAiMode)
{
    BOOL                        bResult         = false;
	BOOL                        bRetCode        = false;
    IKG_Buffer*                 piPackage       = NULL;
    S2R_PVE_TEAM_SET_AI_MODE*   pSendData       = NULL;
    KHeroData*                  pAssistHeroData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);
    pAssistHeroData = pPlayer->GetAssistHeroData();
    KGLOG_PROCESS_ERROR(pAssistHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_SET_AI_MODE));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_SET_AI_MODE*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_set_ai_mode;
    pSendData->dwPlayerID       = pPlayer->m_dwID;
    pSendData->byAiMode         = (BYTE)bIsAiMode;

    bRetCode = gs_CopyRoomHeroInfo(&pSendData->assistHeroInfo, pAssistHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamStartGameRequest(KPlayer* pPlayer, BOOL bNeedCostItem, BOOL bNeedCheckAllReady)
{
    BOOL                                bResult     = false;
    BOOL                                bRetCode    = false;
    IKG_Buffer*                         piPackage   = NULL;
    S2R_PVE_TEAM_START_GAME_REQUEST*    pSendData   = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_START_GAME_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_START_GAME_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID              = s2r_pve_team_start_game_request;
    pSendData->dwPlayerID               = pPlayer->m_dwID;
    pSendData->wCurFatiguePoint         = (uint16_t)pPlayer->m_nCurFatiguePoint;
    pSendData->byNeedCostItem           = (BYTE)bNeedCostItem;
    pSendData->byNeedCheckAllReady      = (BYTE)bNeedCheckAllReady;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamChangeMissionRequest(uint32_t dwPlayerID, int nMissionType, int nMissiontStep, int nMissionLevel, uint32_t dwMapID)
{
    BOOL                                    bResult     = false;
    BOOL                                    bRetCode    = false;
    IKG_Buffer*                             piPackage   = NULL;
    S2R_PVE_TEAM_CHANGE_MISSION_REQUEST*    pSendData   = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_CHANGE_MISSION_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_CHANGE_MISSION_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_change_mission_request;
    pSendData->dwPlayerID       = dwPlayerID;
    pSendData->byMissionType    = (BYTE)nMissionType;
    pSendData->byMissionStep    = (BYTE)nMissiontStep;
    pSendData->byMissionLevel   = (BYTE)nMissionLevel;
    pSendData->dwMapID          = dwMapID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoPveTeamTryJoin(KPlayer* pPlayer, int nPveMode, uint32_t dwTeamID)
{
    BOOL                            bResult         = false;
    BOOL                            bRetCode        = false;
    IKG_Buffer*                     piPackage       = NULL;
    S2R_PVE_TEAM_TRY_JOIN*          pSendData       = NULL;
    KHeroData*                      pMainHeroData   = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);
    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_TRY_JOIN));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_TRY_JOIN*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_try_join;
    pSendData->dwPlayerID       = pPlayer->m_dwID;
    pSendData->dwTeamID         = dwTeamID;
    pSendData->byPveMode        = (BYTE)nPveMode;

    bRetCode = gs_CopyRoomHeroInfo(&pSendData->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoPveTeamAutoJoin(KPlayer* pPlayer, int nPveMode, int nMissionType, int nMissionStep, int nOpenLevel, unsigned uValidLevel)
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    IKG_Buffer*             piPackage       = NULL;
    S2R_PVE_TEAM_AUTO_JOIN* pSendData       = NULL;
    KMission*               pMission        = NULL;
    KHeroData*              pMainHeroData   = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_AUTO_JOIN));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_AUTO_JOIN*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_auto_join;
    pSendData->dwPlayerID       = pPlayer->m_dwID;
    pSendData->byPveMode        = (BYTE)nPveMode;
    pSendData->byMissionType    = (BYTE)nMissionType;
    pSendData->byMissionStep    = (BYTE)nMissionStep;
    pSendData->byOpenLevel      = (BYTE)nOpenLevel;
    pSendData->byValidLevel     = (BYTE)uValidLevel;

    bRetCode = gs_CopyRoomHeroInfo(&pSendData->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoPveTeamReset(uint32_t dwPlayerID)
{
    BOOL                                    bResult     = false;
    BOOL                                    bRetCode    = false;
    IKG_Buffer*                             piPackage   = NULL;
    S2R_PVE_TEAM_RESET*                     pSendData   = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_PVE_TEAM_RESET));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_PVE_TEAM_RESET*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_pve_team_reset;
    pSendData->dwPlayerID       = dwPlayerID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoCanEnterMissionRespond(uint32_t dwPlayerID, uint32_t dwHeroTemplateID, int nMissionType, int nMissionStep, int nMissionLevel, BOOL bSuccess)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    IKG_Buffer*                     piPackage   = NULL;
    S2R_CAN_ENTER_MISSION_RESPOND*  pSendData   = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_CAN_ENTER_MISSION_RESPOND));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_CAN_ENTER_MISSION_RESPOND*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID      = s2r_can_enter_mission_respond;
    pSendData->dwPlayerID       = dwPlayerID;
    pSendData->dwHeroTemplateID = dwHeroTemplateID;
    pSendData->byMissionType    = (BYTE)nMissionType;
    pSendData->byMissionStep    = (BYTE)nMissionStep;
    pSendData->byMissionLevel   = (BYTE)nMissionLevel;
    pSendData->bySuccess        = (BYTE)bSuccess;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoUpdatePlayerGender(KPlayer* pPlayer)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2R_UPDATE_PLAYER_GENDER* pSendData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_UPDATE_PLAYER_GENDER));
    KGLOG_PROCESS_ERROR(piPackage);

    pSendData = (S2R_UPDATE_PLAYER_GENDER*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pSendData);

    pSendData->wProtocolID  = s2r_update_player_gender;
    pSendData->dwPlayerID   = pPlayer->m_dwID;
    pSendData->byGender     = (BYTE)pPlayer->m_eGender;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}


BOOL KRelayClient::DoChangeExtPointRequest(DWORD dwPlayerID, unsigned uExtPointIndex, int nChangeValue)
{
    BOOL                            bResult  = false;
    BOOL                            bRetCode = false;
    IKG_Buffer*                     piBuffer = NULL;
    S2R_CHANGE_EXT_POINT_REQUEST*   pRequest = NULL;

    piBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(S2R_CHANGE_EXT_POINT_REQUEST));
    KGLOG_PROCESS_ERROR(piBuffer);

    pRequest = (S2R_CHANGE_EXT_POINT_REQUEST*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID       = s2r_change_ext_point_request;
    pRequest->dwPlayerID        = dwPlayerID;
    pRequest->uExtPointIndex    = uExtPointIndex;
    pRequest->nChangeValue      = nChangeValue;

    bRetCode = Send(piBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL KRelayClient::DoUpdateHeroLevel(KPlayer* pPlayer, DWORD dwHeroTemplateID)
{
	BOOL                   bResult   = false;
	BOOL                   bRetCode  = false;
	KHeroData*             pHeroData = NULL;
    IKG_Buffer*            piPackage = NULL;
    S2R_UPDATE_HERO_LEVEL* pNotify   = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(dwHeroTemplateID);

    pHeroData = pPlayer->m_HeroDataList.GetHeroData(dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_UPDATE_HERO_LEVEL));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2R_UPDATE_HERO_LEVEL*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID        = s2r_update_hero_level;
    pNotify->dwPlayerID         = pPlayer->m_dwID;
    pNotify->dwHeroTemplateID   = pHeroData->m_dwTemplateID;
    pNotify->nLadderLevel       = pHeroData->m_nLadderLevel;
    pNotify->nNormalLevel       = pHeroData->m_nLevel;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
	return bResult;
}

BOOL KRelayClient::DoSyncMainHero(uint32_t dwPlayerID, uint32_t dwHeroTemplateID)
{
    BOOL                   bResult   = false;
    BOOL                   bRetCode  = false;
    IKG_Buffer*            piPackage = NULL;
    S2R_SYNC_MAIN_HERO*    pNotify   = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_SYNC_MAIN_HERO));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2R_SYNC_MAIN_HERO*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID        = s2r_sync_main_hero;
    pNotify->dwPlayerID         = dwPlayerID;
    pNotify->dwHeroTemplateID   = dwHeroTemplateID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoKickAccountRespond(const char cszAccount[], int nPlayerIndex)
{
    BOOL                   bResult   = false;
    BOOL                   bRetCode  = false;
    IKG_Buffer*            piPackage = NULL;
    S2R_KICKACCOUNT_NOTIFY*    pNotify   = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_KICKACCOUNT_NOTIFY));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2R_KICKACCOUNT_NOTIFY*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID = s2r_kickaccount_notify;
    pNotify->nPlayerIndex = nPlayerIndex;
    
    strncpy(pNotify->szAccount, cszAccount, countof(pNotify->szAccount));
    pNotify->szAccount[countof(pNotify->szAccount) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoUpdateVIPInfo(KPlayer* pPlayer)
{
    BOOL                   bResult   = false;
    BOOL                   bRetCode  = false;
    IKG_Buffer*            piPackage = NULL;
    S2R_UPDATE_VIP_INFO*   pNotify   = NULL;

	KGLOG_PROCESS_ERROR(pPlayer);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_UPDATE_VIP_INFO));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2R_UPDATE_VIP_INFO*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID    = s2r_update_vip_info;
    pNotify->dwPlayerID     = pPlayer->m_dwID;
    pNotify->byIsVIP        = (BYTE)pPlayer->IsVIP();
    pNotify->byVIPLevel     = (BYTE)pPlayer->m_nVIPLevel;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoUpdateClubID(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    IKG_Buffer*            piPackage = NULL;
    S2R_UPDATE_CLUB_ID* pNotify = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_UPDATE_CLUB_ID));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2R_UPDATE_CLUB_ID*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID    = s2r_update_club_id;
    pNotify->dwPlayerID     = pPlayer->m_dwID;
    pNotify->dwClubID       = pPlayer->m_dwClubID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoRankQuickStart(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2R_RANK_QUICK_START_REQUEST* pRequest = NULL;
    KHeroData* pHeroData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_RANK_QUICK_START_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_RANK_QUICK_START_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID               = s2r_rank_quick_start_request;
    pRequest->dwPlayerID                = pPlayer->m_dwID;
    pRequest->nLadderLosingStreakCount  = pPlayer->m_nLadderLosingStreakCount;
    pRequest->dwHeroTemplateID          = pHeroData->m_dwTemplateID;
    pRequest->nEol                      = pHeroData->m_nEol;
    pRequest->nLevel                    = pHeroData->m_nLevel;
    pRequest->nLadderLevel              = pHeroData->m_nLadderLevel;
    pRequest->nLadderBattleCount        = pHeroData->m_nLadderWinCount + pHeroData->m_nLadderLoseCount;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoChangeRoomHost(DWORD dwPlayerID, int nRoomType, DWORD dwNewHostID)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2R_CHANGE_ROOM_HOST* pRequest = NULL;

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_CHANGE_ROOM_HOST));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_CHANGE_ROOM_HOST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID = s2r_change_room_host;
    pRequest->dwPlayerID = dwPlayerID;
    pRequest->dwNewHostID = dwNewHostID;
    pRequest->byRoomType = (BYTE)nRoomType;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoFreePVPQuickStart(KPlayer* pPlayer, DWORD dwMapID, const char cszRoomName[], const char cszPassword[])
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    IKG_Buffer* piPackage = NULL;
    S2R_FREEPVP_QUICK_START* pRequest = NULL;
    KHeroData* pMainHeroData = NULL;
    KHeroData* pAssistHeroData = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    pMainHeroData = pPlayer->GetMainHeroData();
    KGLOG_PROCESS_ERROR(pMainHeroData);

    pAssistHeroData = pPlayer->GetAssistHeroData();
    KGLOG_PROCESS_ERROR(pAssistHeroData);

    piPackage = KG_MemoryCreateBuffer(sizeof(S2R_FREEPVP_QUICK_START));
    KGLOG_PROCESS_ERROR(piPackage);

    pRequest = (S2R_FREEPVP_QUICK_START*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pRequest);

    pRequest->wProtocolID   = s2r_freepvp_quick_start; 
    pRequest->dwPlayerID    = pPlayer->m_dwID;
    pRequest->dwMapID       = dwMapID;
    
    bRetCode = gs_CopyRoomHeroInfo(&pRequest->mainHeroInfo, pMainHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = gs_CopyRoomHeroInfo(&pRequest->assistHeroInfo, pAssistHeroData);
    KGLOG_PROCESS_ERROR(bRetCode);

    strncpy(pRequest->szName, cszRoomName, countof(pRequest->szName));
    pRequest->szName[countof(pRequest->szName) - 1] = '\0';
    strncpy(pRequest->szPassword, cszPassword, countof(pRequest->szPassword));
    pRequest->szPassword[countof(pRequest->szPassword) - 1] = '\0';

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

BOOL KRelayClient::DoUpdatePlayerTeamLogo(KPlayer* pPlayer)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    IKG_Buffer*         piPackage       = NULL;
    S2R_UPDATE_PLAYER_TEAM_LOGO* pNotify   = NULL;

    KGLOG_PROCESS_ERROR(pPlayer);

    piPackage = KG_MemoryCreateBuffer((unsigned)sizeof(S2R_UPDATE_PLAYER_TEAM_LOGO));
    KGLOG_PROCESS_ERROR(piPackage);

    pNotify = (S2R_UPDATE_PLAYER_TEAM_LOGO*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pNotify);

    pNotify->wProtocolID    = s2r_update_player_team_logo;
    pNotify->dwPlayerID     = pPlayer->m_dwID;
    pNotify->byTeamLogoID    = (BYTE)pPlayer->m_nTeamLogoEmblemID;
    pNotify->byTeamLogoBgID  = (BYTE)pPlayer->m_nTeamLogoFrameID;

    bRetCode = Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    return bResult;
}

//////////////////////////////////////////////////////////////////////////

void KRelayClient::OnApplyServerConfigRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                                bRetCode    = false;
    R2S_APPLY_SERVER_CONFIG_RESPOND*    pRespond    = (R2S_APPLY_SERVER_CONFIG_RESPOND*)pbyData;

    pRespond->szExternalIPAddress[sizeof(pRespond->szExternalIPAddress) - 1] = '\0';
    
    strncpy(g_PlayerServer.m_szLocalIP, pRespond->szExternalIPAddress, sizeof(pRespond->szExternalIPAddress));
    g_PlayerServer.m_szLocalIP[sizeof(g_PlayerServer.m_szLocalIP) - 1] = '\0';

    g_PlayerServer.m_nListenPort = pRespond->nGS2PlayerPort;
    g_pSO3World->m_nServerIndexInGC = pRespond->nServerIndex;

    KGLogPrintf(KGLOG_INFO, "Set world index = %d\n", g_pSO3World->m_nServerIndexInGC);

Exit0:
    return;
}

void KRelayClient::OnHandshakeRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                    bRetCode   = false;
    R2S_HANDSHAKE_RESPOND*  pHandshake = (R2S_HANDSHAKE_RESPOND*)pbyData;

    g_pSO3World->m_nBaseTime    = pHandshake->nBaseTime;
    g_pSO3World->m_dwStartTime  = KG_GetTickCount();
    g_pSO3World->m_nStartLoop   = pHandshake->nStartFrame;
    g_pSO3World->m_nGameLoop    = pHandshake->nStartFrame;

    DoApplyRoomBaseInfoForCache(ERROR_ID);

Exit0:
    return;
}

void KRelayClient::OnQuitNotify(BYTE* pbyData, size_t uDataLen)
{
    KGLogPrintf(KGLOG_INFO, "Quit signal from center server !");
    PrepareToQuit();
}


void KRelayClient::OnCreateMapNotify(BYTE* pbyData, size_t uDataLen)
{
    BOOL                    bResult                 = false;
    BOOL                    bRetCode                = false;
    R2S_CREATE_MAP_NOTIFY*  pNotify                 = (R2S_CREATE_MAP_NOTIFY*)pbyData;
    KScene*                 pScene                  = NULL;
    BOOL                    bSceneInitFlag          = false;
    KMapParams*             pMapParams              = NULL;
    KScene*                 pCopyScene              = NULL;
    DWORD                   dwBattleID              = 0;
    char                    szFileName[PATH_MAX];

    pMapParams = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(pNotify->dwMapID);
    KGLOG_PROCESS_ERROR(pMapParams);

    KGLogPrintf(
        KGLOG_INFO, "Map(%.2u, %d) loading ... ...", 
        pNotify->dwMapID, pNotify->nMapCopyIndex
    );

    pScene = KMEMORY_NEW(KScene);
    KGLOG_PROCESS_ERROR(pScene);

    bRetCode = pScene->Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    bSceneInitFlag = true;

    pCopyScene = g_pSO3World->GetScene(pNotify->dwMapID, 0);
    if (pCopyScene)
    {
        assert(pCopyScene->m_pTerrainScene);
        pScene->m_pTerrainScene = pCopyScene->m_pTerrainScene;
        pScene->m_pTerrainScene->AddRef();
    }
    else
    {
        pScene->m_pTerrainScene = KMEMORY_NEW(KTerrainScene);
        KGLOG_PROCESS_ERROR(pScene->m_pTerrainScene);
    }

    pScene->m_pCells = pScene->m_pTerrainScene->GetTerrainData();
    KGLOG_PROCESS_ERROR(pScene->m_pCells);

    pScene->m_dwMapID    = pNotify->dwMapID;
    pScene->m_nCopyIndex = pNotify->nMapCopyIndex;
    pScene->m_nType      = pMapParams->nType;
    pScene->m_Param      = pNotify->param;

    snprintf(szFileName, sizeof(szFileName), "scripts/Map/%u/scene.lua", pMapParams->nScriptDir);
    szFileName[sizeof(szFileName) - 1] = '\0';

    pScene->m_dwScriptID = g_FileNameHash(szFileName);

    dwBattleID = pScene->m_Param.m_dwBattleTemplateID;
    if (dwBattleID == 0)
        dwBattleID = pMapParams->dwBattleID;
    
    pScene->m_Battle.ApplyTemplate(dwBattleID);

    pScene->m_eSceneState = ssLoading;

    bRetCode = g_pSO3World->m_SceneLoader.PostLoadingScene(pScene->m_dwID);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bSceneInitFlag)
        {
            pScene->UnInit();
            bSceneInitFlag = false;
        }

        if (pScene)
        {
            KMemory::Delete(pScene);
            pScene = NULL;
        }

        DoCreateMapRespond(pNotify->dwMapID, pNotify->nMapCopyIndex, false);
    }
    return;
}

void KRelayClient::OnFinishCreateMapNotify(BYTE* pbyData, size_t uDataLen)
{
    R2S_FINISH_CREATE_MAP_NOTIFY* pNotify = (R2S_FINISH_CREATE_MAP_NOTIFY*)pbyData;
    KPlayer*                      pPlayer = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);


Exit0:
    return;
}

void KRelayClient::OnDeleteMapNotify(BYTE* pbyData, size_t uDataLen)
{
    R2S_DELETE_MAP_NOTIFY* pNotify  = (R2S_DELETE_MAP_NOTIFY*)pbyData;
    KScene*                pScene   = NULL;
    BOOL				   bRetCode = false; 

    pScene = g_pSO3World->GetScene(pNotify->dwMapID, pNotify->nMapCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    KGLogPrintf(
        KGLOG_INFO, "Delete map (%u, %d) !\n", pScene->m_dwMapID, pScene->m_nCopyIndex
    );

    g_pSO3World->DeleteScene(pScene);

Exit0:
    return;
}

void KRelayClient::OnSearchMapRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                        bRetCode            = false;
    R2S_SEARCH_MAP_RESPOND*     pRespond            = (R2S_SEARCH_MAP_RESPOND*)pbyData;
    KPlayer*                    pPlayer             = NULL;
    KScene*                     pScene              = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsSearchMap);

    if (pRespond->nResultCode != smrcSuccess)
    {
        KMission* pMission = NULL;
        LogInfo("Search map faild %d!", pRespond->nResultCode);
        pPlayer->m_eGameStatus  = gsInHall;

        pMission = g_pSO3World->m_MissionMgr.GetMission(pRespond->dwMapID);
        if (pMission)
        {
            g_PlayerServer.DoEnterMissionRespond(
                pPlayer->m_nConnIndex, pMission->eType, pMission->nStep, pMission->nLevel, false
            );
        }
        
        goto Exit0;
    }

    if (pRespond->bSwitchServer)
    {
        // 将角色数据存盘,位置保存为跨服的目标点
        pPlayer->m_dwMapID          = pRespond->dwMapID;
        pPlayer->m_nCopyIndex       = pRespond->nMapCopyIndex;
        pPlayer->m_eGameStatus      = gsWaitForTransmissionSave;

        bRetCode = SaveRoleData(pPlayer);
        KGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        pPlayer->m_dwMapID      = pRespond->dwMapID;
        pPlayer->m_nCopyIndex   = pRespond->nMapCopyIndex;

        g_PlayerServer.DoSwitchMap(
            pPlayer, pRespond->dwMapID, pRespond->nMapCopyIndex
        );
        pPlayer->m_eGameStatus = gsWaitForNewMapLoading; //后续应该等待客户端加载完申请进入场景
        
        {
            KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(pRespond->dwMapID);
            if (pMission)
            {
                g_PlayerServer.DoEnterMissionRespond(
                    pPlayer->m_nConnIndex, pMission->eType, pMission->nStep, pMission->nLevel, true
                );
            }
        }    
    }

Exit0:
    return;
}

void KRelayClient::OnTransferPlayerRequest(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    R2S_TRANSFER_PLAYER_REQUEST*    pRequest    = (R2S_TRANSFER_PLAYER_REQUEST*)pbyData;
    KPlayer*                        pPlayer     = NULL;
    GUID                            Guid        = {0, 0, 0, 0};

    KG_PROCESS_ERROR(!m_bQuiting);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRequest->dwRoleID);
    KGLOG_PROCESS_ERROR(pPlayer == NULL);

    pPlayer = g_pSO3World->NewPlayer(pRequest->dwRoleID);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->LoadBaseInfo(&pRequest->BaseInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->m_ExtPointInfo         = pRequest->ExtPointInfo;
    pPlayer->m_bExtPointLock        = pRequest->bExtPointLock;
    pPlayer->m_nLastExtPointIndex   = pRequest->nLastExtPointIndex;
    pPlayer->m_nLastExtPointValue   = pRequest->nLastExtPointValue;
    pPlayer->m_nTimer               = 0;
    pPlayer->m_bCanRename           = pRequest->byCanRename;
    pPlayer->m_nIBActionTime        = pRequest->nIBActionTime;
    pPlayer->m_dwLimitPlayTimeFlag  = pRequest->dwLimitPlayTimeFlag;
    pPlayer->m_dwLimitOnlineSecond  = pRequest->dwLimitOnlineSecond;
    pPlayer->m_dwLimitOfflineSecond = pRequest->dwLimitOfflineSecond;
    pPlayer->m_ActivePlayer.SetOnlineTime(pRequest->nOnlineTime);
    pPlayer->m_nOnlineAwardTime     = pRequest->nNextOnlineAwardTime;

    pPlayer->m_eGameStatus          = gsWaitForConnect;

    strncpy(pPlayer->m_szAccount, pRequest->szAccount, sizeof(pPlayer->m_szAccount));
    pPlayer->m_szAccount[sizeof(pPlayer->m_szAccount) - 1] = '\0';

    strncpy(pPlayer->m_szName, pRequest->szRoleName, sizeof(pPlayer->m_szName));
    pPlayer->m_szName[sizeof(pPlayer->m_szName) - 1] = '\0';

    bRetCode = KG_CreateGUID(&Guid);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->m_Guid = Guid;
    pPlayer->m_nGroupID = pRequest->nGroupID;

    pPlayer->m_eConnectType = eTransferGS;

    bResult = true;
Exit0:
    DoTransferPlayerRespond(pRequest->dwRoleID, bResult, Guid);

    if (!bResult)
    {
        if (pPlayer)
        {
            g_pSO3World->DelPlayer(pPlayer);
        }
    }

    return;
}

void KRelayClient::OnTransferPlayerRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bRetCode    = false;
    R2S_TRANSFER_PLAYER_RESPOND*    pRespond    = (R2S_TRANSFER_PLAYER_RESPOND*)pbyData;
    KPlayer*                        pPlayer     = NULL;

    // 跨服中断线就可能找不到
    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForTransmissionGuid);

    if (!pRespond->bSucceed)
    {
        KGLogPrintf(KGLOG_INFO, "Switch GS failed, shutdown(%d)\n", pPlayer->m_nConnIndex);

        g_PlayerServer.Detach(pPlayer->m_nConnIndex);
        goto Exit0;
    }

    g_PlayerServer.DoSwitchGS(
        pPlayer->m_nConnIndex, pRespond->dwTargetPlayerID,
        pRespond->Guid, pRespond->dwAddress, pRespond->wPort
    );

    g_PlayerServer.Detach(pPlayer->m_nConnIndex);

    KGLogPrintf(KGLOG_INFO, "player %d transfer to server(%d, %d)", pRespond->dwPlayerID, pRespond->dwAddress, pRespond->wPort);

Exit0:
    return;
}

void KRelayClient::OnCoinShopBuyItemRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bResult     = false;
    BOOL                            bRetCode    = false;
    R2S_COIN_SHOP_BUY_ITEM_RESPOND* pRespond    = (R2S_COIN_SHOP_BUY_ITEM_RESPOND*)pbyData;
    KPlayer*                        pPlayer     = NULL;
    GOODSINFO*                      pGoodsInfo  = NULL;
    int                             nValuePoint = -1;
    IItem*                          pResult     = NULL;
    DWORD                           dwRandSeed  = 0;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);
    pPlayer->m_nIBActionTime = 0;

    if (pRespond->dwTabType == 0)      //dwTabType == 0 --> Undefition Consume
    {
        bRetCode = g_pSO3World->m_ShopMgr.OnCustomConsumeRespond(
            pPlayer, pRespond->bSucceed, (CUSTOM_CONSUME_TYPE)pRespond->dwTabIndex, 
			pRespond->nCoinPrice, &pRespond->CustomConsumeInfo
		);
        KGLOG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    pGoodsInfo = g_pSO3World->m_ShopMgr.GetGoodsInfo(pRespond->nGoodsID);
    KGLOG_PROCESS_ERROR(pGoodsInfo);
    KGLOG_PROCESS_ERROR(pGoodsInfo->eMoneyType == emotCoin);
    
    PLAYER_LOG(pPlayer, "buygoods, usecoinrespond, %d, %d, %d", pRespond->bSucceed, pRespond->nCoinPrice, pRespond->nGoodsID);

    if (!pRespond->bSucceed)
    {
        bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotCoin, pRespond->nCoinPrice);
        KGLOG_PROCESS_ERROR(bRetCode);

        KGLogPrintf(KGLOG_INFO, "Paysys respond Failed!:\nInfo --> player:%d try to spend:%d MoneyType:%d on shopping for Number:%d GoodsID:%d", pRespond->dwPlayerID, pRespond->nCoinPrice, pGoodsInfo->eMoneyType, pRespond->nCount, pRespond->nGoodsID);

        goto Exit1;
    }

    KGLOG_PROCESS_ERROR(pRespond->dwTabType == (DWORD)pGoodsInfo->eTabType);
    KGLOG_PROCESS_ERROR(pRespond->dwTabIndex == (DWORD)pGoodsInfo->nTabIndex);

    nValuePoint = pGoodsInfo->nValuePoint;
    KGLOG_PROCESS_ERROR(nValuePoint >= 0); 
    
    switch(pGoodsInfo->eTabType)
    {
    case ittEquip:
    case ittOther:
        {
            IItem*      pItem   = NULL;
            int         nCount  = pRespond->nCount;
            KGItemInfo* pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(pGoodsInfo->eTabType, pGoodsInfo->nTabIndex);
            KGLOG_PROCESS_ERROR(pItemInfo);

            dwRandSeed = KG_GetTickCount();

            if (pItemInfo->bCanStack)
            {
                int nTotalCount = 0;
                int nAddTime = 0;
                int nLeftCount = 0;

                KGLOG_PROCESS_ERROR(pItemInfo->nMaxStackNum > 0);

                nTotalCount = pGoodsInfo->nStack * nCount;
                nAddTime   = nTotalCount / pItemInfo->nMaxStackNum;

                for (int i = 0; i < nAddTime; ++i)
                {
                    pItem = pPlayer->AddItem(pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, pItemInfo->nMaxStackNum, pGoodsInfo->nValuePoint, dwRandSeed);
                    KGLOG_PROCESS_ERROR(pItem);
                    dwRandSeed *= (i + 2);
                }

                nLeftCount = nTotalCount - nAddTime * pItemInfo->nMaxStackNum;
                if (nLeftCount > 0)
                {
                    pItem = pPlayer->AddItem(
                        pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, nLeftCount, pGoodsInfo->nValuePoint, dwRandSeed
                    );
                    KGLOG_PROCESS_ERROR(pItem);
                }
            }
            else
            {
                for (int i = 0; i < nCount; ++i)
                {
                    pItem = pPlayer->AddItem(pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, pGoodsInfo->nStack, pGoodsInfo->nValuePoint, dwRandSeed);
                    KGLOG_PROCESS_ERROR(pItem);
                    dwRandSeed *= (i + 2);
                }
            }
        }
        break;
    case ittFashion:
        bRetCode = pPlayer->m_Wardrobe.AddFashion(pRespond->dwTabIndex);
        KGLOG_PROCESS_ERROR(bRetCode);
        g_LogClient.DoFlowRecord(
            frmtCheerleadingAndWardrobe, cawfrBuyFashion, 
            "%s,%d,%u",
            pPlayer->m_szName,
            pRespond->nCoinPrice,
            pRespond->dwTabIndex
        );
        break;
    case ittCheerleading:
        bRetCode = pPlayer->m_CheerleadingMgr.AddCheerleading(pRespond->dwTabIndex);
        KGLOG_PROCESS_ERROR(bRetCode);
        g_LogClient.DoFlowRecord(
            frmtCheerleadingAndWardrobe, cawfrBuyCheerleading, 
            "%s,%d,%u",
            pPlayer->m_szName,
            pRespond->nCoinPrice,
            pRespond->dwTabIndex
        );
        break;
    case ittTeamLogo:
        bRetCode = pPlayer->AddTeamLogo((WORD)pRespond->dwTabIndex);
        KGLOG_PROCESS_ERROR(bRetCode);
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }
	
	pPlayer->OnEvent(peBuyItem, pGoodsInfo->eTabType, pGoodsInfo->nTabIndex, pGoodsInfo->nID);

    KGLogPrintf(KGLOG_INFO, "Paysys respond Successed!:\nInfo --> player:%d try to spend:%d MoneyType:%d on shopping for Number:%d GoodsID:%d", pRespond->dwPlayerID, pRespond->nCoinPrice, pGoodsInfo->eMoneyType, pRespond->nCount, pRespond->nGoodsID);

Exit1:
    bResult = true;
Exit0:
	if (pPlayer)
		g_PlayerServer.DoIBActionResultNotify(pPlayer, pRespond->bSucceed);

    if (!bResult)
    {
        assert(pRespond);
        KGLogPrintf(KGLOG_INFO, "Paysys respond successed! But there is some unknow wrong for additem failed !\nInfo --> player:%d try to spend:%d on shopping for Number:%d GoodsID:%d", pRespond->dwPlayerID, pRespond->nCoinPrice, pRespond->nCount, pRespond->nGoodsID);
    }
    return;
}

void KRelayClient::OnPlayerLoginRequest(BYTE* pbyData, size_t uDataLen)
{
    BOOL                        bRetCode            = false;
    BOOL                        bResult             = false;
    KPlayer*                    pPlayer             = NULL;
    R2S_PLAYER_LOGIN_REQUEST*   pRequest            = (R2S_PLAYER_LOGIN_REQUEST*)pbyData;
    GUID                        guid                = {0, 0, 0, 0};

    KG_PROCESS_ERROR(!m_bQuiting);

    pPlayer = g_pSO3World->NewPlayer(pRequest->dwRoleID);
    KGLOG_PROCESS_ERROR(pPlayer);

    bRetCode = pPlayer->LoadBaseInfo(&pRequest->BaseInfo);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->m_nTimer               = 0;
    pPlayer->m_eGameStatus          = gsWaitForConnect;
    pPlayer->m_dwMapID              = ERROR_ID;
    pPlayer->m_nCopyIndex           = 0;
    pPlayer->m_bCanRename           = pRequest->byCanRename;
    pPlayer->m_ExtPointInfo         = pRequest->ExtPointInfo;
    pPlayer->m_nCoin                = pRequest->nCoin;
    pPlayer->m_dwLimitPlayTimeFlag  = pRequest->dwLimitPlayTimeFlag;
    pPlayer->m_dwLimitOnlineSecond  = pRequest->dwLimitOnlineSecond;
    pPlayer->m_dwLimitOfflineSecond = pRequest->dwLimitOfflineSecond;

    strncpy(pPlayer->m_szAccount, pRequest->szAccount, sizeof(pPlayer->m_szAccount));
    pPlayer->m_szAccount[sizeof(pPlayer->m_szAccount) - 1] = '\0';

    strncpy(pPlayer->m_szName, pRequest->szRoleName, sizeof(pPlayer->m_szName));
    pPlayer->m_szName[sizeof(pPlayer->m_szName) - 1] = '\0';

    bRetCode = KG_CreateGUID(&guid);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->m_Guid = guid;
    pPlayer->m_nGroupID = pRequest->nGroupID;

    pPlayer->m_eConnectType = eLogin;

    bResult = true;
Exit0:
    DoPlayerLoginRespond(pRequest->dwRoleID, bResult, guid, pRequest->dwPacketIdentity);

    if (!bResult)
    {
        if (pPlayer)
        {
            g_pSO3World->DelPlayer(pPlayer);
            pPlayer = NULL;
        }
    }

    return;
}

void KRelayClient::OnLimitPlayInfoResponse(BYTE* pbyData, size_t uDataLen)
{
	R2S_LIMITPLAY_INFO_RESPOND*   pRespond = (R2S_LIMITPLAY_INFO_RESPOND*)pbyData;
	BOOL				bRetCode = false;
	KPlayer*            pPlayer  = NULL;
	KGItemInfo*         pItemInfo = NULL;
	KSystemMailTxt*		pSysMailTxt = NULL;
	DWORD				dwItemType = 0;
	DWORD				dwItemIndex = 0;
	int					nAwardFlag = 0;

	pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
	KG_PROCESS_ERROR(pPlayer);
	
	g_PlayerServer.DoLimitPlayInfoResponse(pPlayer, pRespond->byResult);

	KG_PROCESS_ERROR(pRespond->byResult);

	pPlayer->m_bIsLimited = false;
	pPlayer->m_dwLimitPlayTimeFlag = 1;

	dwItemType = (DWORD)g_pSO3World->m_Settings.m_ConstList.nLimitPlayInfoAwardItemType;
	dwItemIndex = (DWORD)g_pSO3World->m_Settings.m_ConstList.nLimitPlayInfoAwardItemIndex;

	bRetCode = pPlayer->m_PlayerValue.GetValue(KPLAYER_VALUE_ID_LIMITPLAY_INFO_AWARD, nAwardFlag);
	KGLOG_PROCESS_ERROR(bRetCode);
	KGLOG_PROCESS_ERROR(!nAwardFlag);

	bRetCode = pPlayer->m_PlayerValue.SetValue(KPLAYER_VALUE_ID_LIMITPLAY_INFO_AWARD, 1);
	KGLOG_PROCESS_ERROR(bRetCode);

	pItemInfo = g_pSO3World->m_ItemHouse.GetItemInfo(dwItemType, dwItemIndex);
	KGLOG_PROCESS_ERROR(pItemInfo);

	pSysMailTxt = g_pSO3World->m_Settings.m_SystemMailMgr.GetSystemMailTxt(KMAIL_QUESTIONNAIRE);
	KGLOG_PROCESS_ERROR(pSysMailTxt);

	bRetCode = pPlayer->SafeAddItem(
		dwItemType, dwItemIndex, pItemInfo->nMaxStackNum, pItemInfo->nValuePoint, 
		pSysMailTxt->m_szMailTitl, pSysMailTxt->m_szSendName, pSysMailTxt->m_szMailTxt
		);
	KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
	return;
}

void KRelayClient::OnConfirmPlayerLoginRespond(BYTE* pbyData, size_t uDataLen)
{
    R2S_CONFIRM_PLAYER_LOGIN_RESPOND*   pRespond = (R2S_CONFIRM_PLAYER_LOGIN_RESPOND*)pbyData;
    KPlayer*                            pPlayer  = NULL;

    // 登陆期间，客户端断线，就有可能在这里找不到
    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForPermit);

    if (!pRespond->bPermit)
    {
        KGLogPrintf(KGLOG_INFO, "Login denied by game center, shutdown(%d)\n", pPlayer->m_nConnIndex);

        g_PlayerServer.Detach(pPlayer->m_nConnIndex);
        goto Exit0;
    }
    
    // 加载角色数据
    g_RelayClient.DoLoadRoleDataRequest(pPlayer->m_dwID);
    pPlayer->m_nTimer           = 0;
    pPlayer->m_eGameStatus      = gsWaitForRoleData;

Exit0:
    return;
}

void KRelayClient::OnKickAccountNotify(BYTE* pbyData, size_t uDataLen)
{
    BOOL                        bRetCode    = false;
    R2S_KICK_ACCOUNT_NOTIFY*    pNotify     = (R2S_KICK_ACCOUNT_NOTIFY*)pbyData;
    KPlayer*                    pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoAccountKickout(pPlayer->m_nConnIndex);

    if (pPlayer->m_nConnIndex >= 0)
    {
        KGLogPrintf(KGLOG_INFO, "Kick account(%s,%s) from game center, shutdown(%d)\n", pPlayer->m_szAccount, pPlayer->m_szName, pPlayer->m_nConnIndex);

        g_PlayerServer.Detach(pPlayer->m_nConnIndex);
    }

    DoKickAccountRespond(pPlayer->m_szAccount, pNotify->nIndex);

Exit0:
    return;
}

void KRelayClient::OnSaveRoleDataRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bRetCode        = false;
    int                             nStateForSaving = 0;
    KPlayer*                        pPlayer         = NULL;
    R2S_SAVE_ROLE_DATA_RESPOND*     pRespond        = (R2S_SAVE_ROLE_DATA_RESPOND*)pbyData;

    m_nNextQuitingSaveTime = 0;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    if (pRespond->bIsTransmission)
    {
        KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForTransmissionSave);

        if (!pRespond->bSucceed)
        {
            KGLogPrintf(
                KGLOG_ERR, "Transmission save failed: ID = %u, Name = %s\n",
                pPlayer->m_dwID, pPlayer->m_szName
            );
            g_PlayerServer.Detach(pPlayer->m_nConnIndex);
            goto Exit0;
        }

        g_RelayClient.DoTransferPlayerRequest(pPlayer);
        pPlayer->m_eGameStatus = gsWaitForTransmissionGuid;

        KGLogPrintf(KGLOG_INFO, "player %d start transmission", pPlayer->m_dwID);
    }

Exit0:
    return;
}

void KRelayClient::OnSyncRoleData(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    R2S_SYNC_ROLE_DATA* pSyncData       = (R2S_SYNC_ROLE_DATA*)pbyData;
    BYTE*               pbyOffset       = m_pbySyncRoleBuffer + m_uSyncRoleOffset;
    size_t              uRoleDataLen    = uDataLen - sizeof(R2S_SYNC_ROLE_DATA);

    KGLOG_PROCESS_ERROR(pSyncData->uOffset == m_uSyncRoleOffset);
    KGLOG_PROCESS_ERROR(MAX_ROLE_DATA_SIZE - m_uSyncRoleOffset >= uRoleDataLen);
    KGLOG_PROCESS_ERROR(pSyncData->uOffset == 0 || pSyncData->dwRoleID == m_dwSyncRoleID);

    memcpy(pbyOffset, pSyncData->byData, uRoleDataLen);

    m_uSyncRoleOffset += uRoleDataLen;
    m_dwSyncRoleID     = pSyncData->dwRoleID;

    bResult = true;    
Exit0:
    if(!bResult)
    {
        m_dwSyncRoleID    = ERROR_ID;
        m_uSyncRoleOffset = 0;
    }
    return;
}

void KRelayClient::OnLoadRoleData(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bResult   = false;
    BOOL                bRetCode  = false;
    KPlayer*            pPlayer   = NULL;
    R2S_LOAD_ROLE_DATA* pNotify   = (R2S_LOAD_ROLE_DATA*)pbyData;
    KScene*             pScene    = NULL;

    KGLOG_PROCESS_ERROR(pNotify->uRoleDataLen == m_uSyncRoleOffset);
    KGLOG_PROCESS_ERROR(pNotify->dwRoleID == m_dwSyncRoleID || m_dwSyncRoleID == ERROR_ID);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwRoleID);
    KGLOG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(pNotify->bSucceed);

    // 注意: 由于取角色数据是异步的,所以,有可能回包到达时已经是玩家断线后的下一次登陆了
    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsWaitForRoleData);

    g_LSClient.DoRoleLoginHallNotify(pPlayer->m_dwID); // 在load之前通知ls玩家的gs链接号，否则load中发往ls的包可能发不回来

    bRetCode = pPlayer->LoadFromProtoBuf(m_pbySyncRoleBuffer, m_pLoadBuf, pNotify->uRoleDataLen);
    KGLOG_PROCESS_ERROR(bRetCode);

    pPlayer->m_eGameStatus = gsInHall;
    KGLogPrintf(KGLOG_INFO, "player (%s:%u) login to hall!", pPlayer->m_szAccount, pPlayer->m_dwID);

    if (pPlayer->m_eConnectType == eLogin)
    {
        bRetCode = pPlayer->CallLoginScript();
        KGLOG_PROCESS_ERROR(bRetCode);

        // 通知GC玩家进入大厅
        g_RelayClient.DoPlayerEnterHallNotify(pNotify->dwRoleID);
        g_PlayerServer.DoPlayerEnterHallNotify(pPlayer->m_nConnIndex);
    
        //注册推荐好友列表
        g_LSClient.DoRegFriendRecommend(pPlayer->m_dwID);

        // 首测回馈
        g_pSO3World->m_ActivityCenter.GetTesterAward(pPlayer);
    }

    pPlayer->m_dwLastClubID = pPlayer->m_dwClubID;
    pPlayer->m_dwClubID = 0;
    //去LS取俱乐部的一些必要数据

    g_LSClient.DoApplyClubBaseInfo(pPlayer->m_dwID, pPlayer->m_eConnectType == eLogin);
    g_LSClient.DoRemoteCall("OnCheckNewMail", (int)pPlayer->m_dwID);

    pScene = g_pSO3World->GetScene(pPlayer->m_dwMapID, pPlayer->m_nCopyIndex); 
    if (pScene) // 跨服过去的时候可以找到场景
    {
        KGLOG_PROCESS_ERROR(pPlayer->m_eConnectType == eTransferGS);
        g_PlayerServer.DoSwitchMap(
            pPlayer, pPlayer->m_dwMapID, pPlayer->m_nCopyIndex
        );
        pPlayer->m_eGameStatus = gsWaitForNewMapLoading; //后续应该等待客户端加载完申请进入场景

        {
            KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(pPlayer->m_dwMapID);
            if (pMission)
            {
                g_PlayerServer.DoEnterMissionRespond(
                    pPlayer->m_nConnIndex, pMission->eType, pMission->nStep, pMission->nLevel, true
                );
            }
        }    
    }

    bResult = true;
Exit0:
    m_uSyncRoleOffset = 0;
    m_dwSyncRoleID    = ERROR_ID;

    // 这里失败不调用detach是因为玩家可能是第二次登录,玩家状态是waitforconnect,调用detach会宕机
    return;
}

void KRelayClient::OnSyncCreateRoomRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bRetCode    = false;
    R2S_SYNC_CREATE_ROOM_RESPOND*   pPak        = (R2S_SYNC_CREATE_ROOM_RESPOND*)pbyData;
    KPlayer*                        pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    if (pPak->bSuccess)    
        pPlayer->m_bInPVPRoom = true;

    g_PlayerServer.DoSyncCreateRoomRespond(pPlayer->m_nConnIndex, pPak->dwRoomID, pPak->dwMapID, pPak->bSuccess);

Exit0:
    return;
}

void KRelayClient::OnSyncOneRoomMemberPosInfo(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bRetCode = false;
    R2S_SYNC_ONEROOMMEMBER_POSINFO* pPak     = (R2S_SYNC_ONEROOMMEMBER_POSINFO*)pbyData;
    KPlayer*                        pSrc     = NULL;
    KSIDE_TYPE                      eSide    = (KSIDE_TYPE)pPak->byDstSide;

    pSrc = g_pSO3World->m_PlayerSet.GetObj(pPak->dwSrcID);
    KGLOG_PROCESS_ERROR(pSrc);

    KGLOG_PROCESS_ERROR(eSide >= sidBegin && eSide < sidTotal);

    g_PlayerServer.DoSyncOneRoomMemberPosInfo(pSrc->m_nConnIndex, pPak->dwDstID, eSide, pPak->nDstPos);

Exit0:
    return;
}

void KRelayClient::OnApplyJoinRoomRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bRetCode    = false;
    R2S_APPLY_JOIN_ROOM_RESPOND*    pPak        = (R2S_APPLY_JOIN_ROOM_RESPOND*)pbyData;
    KPlayer*                        pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoApplyJoinRoomRespond(pPlayer->m_nConnIndex, pPak->bySuccess, pPak->dwRoomID, pPak->byTid, pPak->byPosition, pPak->newMember);

    if (pPak->bySuccess && pPlayer->m_dwID == pPak->newMember.id)
        pPlayer->m_bInPVPRoom = true;

Exit0:
    return;
}

void KRelayClient::OnApplyLeaveRoomRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bRetCode    = false;
    R2S_APPLY_LEAVE_ROOM_RESPOND*   pPak        = (R2S_APPLY_LEAVE_ROOM_RESPOND*)pbyData;
    KPlayer*                        pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoApplyLeaveRoomRespond(
        pPlayer->m_nConnIndex, pPak->dwRoomID, pPak->dwLeaverID, pPak->bySide, pPak->byPos
    );

    if (pPlayer->m_dwID == pPak->dwLeaverID)
        pPlayer->m_bInPVPRoom = false;

Exit0:
    return;
}

void KRelayClient::OnApplySwitchRoomHostRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                                    bRetCode    = false;
    R2S_APPLY_SWITCH_ROOM_HOST_RESPOND*     pPak        = (R2S_APPLY_SWITCH_ROOM_HOST_RESPOND*)pbyData;
    KPlayer*                                pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoApplySwitchRoomHostRespond(pPlayer->m_nConnIndex, pPak->bSuccess, pPak->dwRoomID, pPak->dwNewHostID);

Exit0:
    return;
}

void KRelayClient::OnApplySwitchBattleMapRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                                    bRetCode    = false;
    R2S_APPLY_SWITCH_BATTLE_MAP_RESPOND*    pPak        = (R2S_APPLY_SWITCH_BATTLE_MAP_RESPOND*)pbyData;
    KPlayer*                                pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoApplySwitchBattleMapRespond(pPlayer->m_nConnIndex, pPak->bSuccess, pPak->dwNewMapID);

Exit0:
    return;
}

void KRelayClient::OnApplySetRoomPasswordRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                                    bRetCode    = false;
    R2S_APPLY_SET_ROOM_PASSWORD_RESPOND*    pPak        = (R2S_APPLY_SET_ROOM_PASSWORD_RESPOND*)pbyData;
    KPlayer*                                pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoApplySetRoomPassword(pPlayer->m_nConnIndex, pPak->bSuccess, pPak->szPassword);

Exit0:
    return;
}


void KRelayClient::OnApplySetReadyRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bRetCode    = false;
    R2S_APPLY_SET_READY_RESPOND*    pPak        = (R2S_APPLY_SET_READY_RESPOND*)pbyData;
    KPlayer*                        pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pPak->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoApplySetReadyRespond(pPlayer->m_nConnIndex, pPak->dwRoomID, pPak->dwReadyPlayerID, pPak->byReady);

Exit0:
    return;
}

void KRelayClient::OnApplyStartGameRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL bRetCode = false;
    KPlayer* pPlayer = NULL;
    R2S_APPLY_START_GAME_RESPOND* pRespond = (R2S_APPLY_START_GAME_RESPOND*)pbyData;
    KMapParams* pMapInfo = NULL;

    if (pRespond->bySuccess)
    {
        pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
        KG_PROCESS_ERROR(pPlayer);

        KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);

        pMapInfo = g_pSO3World->m_Settings.m_MapListFile.GetMapParamByID(pRespond->dwMapID);
        if (pMapInfo)
        {
            if (pMapInfo->nType == emtContinuousChallengeMap)
            {
                if (pRespond->byNeedCheckCanEnter)
                {
                    KMission* pMission = g_pSO3World->m_MissionMgr.GetMission(pRespond->dwMapID);
                    KGLOG_PROCESS_ERROR(pMission);

                    bRetCode = pPlayer->CostContinuousChallengeItem();
                    KGLOG_PROCESS_ERROR(bRetCode);

                    pPlayer->SetCurrentCityByMissionType(pMission->eType);
                    pPlayer->SetLastChallengeStep(pRespond->byChallengeStartStep);
                    pPlayer->SetStartChallengeStep(pRespond->byChallengeStartStep);

                    PLAYER_LOG(pPlayer, "challenge,start,%d", pPlayer->GetStartChallengeStep());
                }

                pPlayer->m_bNeedCheckCanEnter = true;
            }
        }

        pPlayer->m_dwMapID      = pRespond->dwMapID;
        pPlayer->m_nCopyIndex   = pRespond->nMapCopyIndex;

        pPlayer->SwitchMap(pRespond->dwMapID, pRespond->nMapCopyIndex);
    }
    else
    {
        // 通知客户端开始游戏失败
    }
    
Exit0:
    return;
}

void KRelayClient::OnSyncRoomName(BYTE* pbyData, size_t uDataLen)
{
    R2S_SYNC_ROOMNAME* pRespond = (R2S_SYNC_ROOMNAME*)pbyData;
    KPlayer* pPlayer = NULL;
    
    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoSyncRoomName(pPlayer->m_nConnIndex,pRespond->dwRoomID, pRespond->szName);

Exit0:
    return;
}

void KRelayClient::OnAddRoomCache(BYTE* pbyData, size_t uDataLen)
{
    R2S_ADD_ROOM_CACHE* pNotify = (R2S_ADD_ROOM_CACHE*)pbyData;
    KRoomBaseInfo       baseinfo;

    baseinfo.dwRoomID       = pNotify->dwRoomID;
    baseinfo.dwMapID        = pNotify->dwMapID;
    baseinfo.byMemberCount  = pNotify->byMemberCount;
    baseinfo.bFighting      = pNotify->bFighting;
    baseinfo.bHasPassword   = pNotify->bHasPassword;

    strncpy(baseinfo.szRoomName, pNotify->szRoomName, countof(baseinfo.szRoomName));
    baseinfo.szRoomName[countof(baseinfo.szRoomName) - 1] = '\0';

    g_pSO3World->m_RoomCache.AddRoomBaseInfo(baseinfo);

Exit0:
    return;
}

void KRelayClient::OnDelRoomCache(BYTE* pbyData, size_t uDataLen)
{
    R2S_DEL_ROOM_CACHE* pNotify = (R2S_DEL_ROOM_CACHE*)pbyData;
    g_pSO3World->m_RoomCache.DelRoomBaseInfo(pNotify->dwRoomID);
    return;
}

void KRelayClient::OnUpdateRoomNameInCache(BYTE* pbyData, size_t uDataLen)
{
    KRoomBaseInfo*                  pRoomBaseInfo   = NULL;
    R2S_UPDATE_ROOM_NAME_IN_CACHE*  pNotify         = (R2S_UPDATE_ROOM_NAME_IN_CACHE*)pbyData;

    pNotify->szName[countof(pNotify->szName) - 1] = '\0';

    pRoomBaseInfo = g_pSO3World->m_RoomCache.GetRoomBaseInfo(pNotify->dwRoomID);
    KG_PROCESS_ERROR(pRoomBaseInfo);

    strncpy(pRoomBaseInfo->szRoomName, pNotify->szName, countof(pRoomBaseInfo->szRoomName));
    pRoomBaseInfo->szRoomName[countof(pRoomBaseInfo->szRoomName) - 1] = '\0';

Exit0:
    return;
}

void KRelayClient::OnUpdateMapIDInCache(BYTE* pbyData, size_t uDataLen)
{
    KRoomBaseInfo*              pRoomBaseInfo   = NULL;
    R2S_UPDATE_MAPID_IN_CACHE*  pNotify         = (R2S_UPDATE_MAPID_IN_CACHE*)pbyData;

    pRoomBaseInfo = g_pSO3World->m_RoomCache.GetRoomBaseInfo(pNotify->dwRoomID);
    KG_PROCESS_ERROR(pRoomBaseInfo);

    pRoomBaseInfo->dwMapID = pNotify->dwMapID;

Exit0:
    return;
}

void KRelayClient::OnUpdateMemberCountInCache(BYTE* pbyData, size_t uDataLen)
{
    KRoomBaseInfo*              pRoomBaseInfo   = NULL;
    R2S_UPDATE_MEMBERCOUNT_IN_CACHE*  pNotify   = (R2S_UPDATE_MEMBERCOUNT_IN_CACHE*)pbyData;

    pRoomBaseInfo = g_pSO3World->m_RoomCache.GetRoomBaseInfo(pNotify->dwRoomID);
    KG_PROCESS_ERROR(pRoomBaseInfo);

    pRoomBaseInfo->byMemberCount = pNotify->byMemberCount;

Exit0:
    return;
}

void KRelayClient::OnUpdateFightingStateInCache(BYTE* pbyData, size_t uDataLen)
{
    KRoomBaseInfo*                      pRoomBaseInfo   = NULL;
    R2S_UPDATE_FIGHTINGSTATE_IN_CACHE*  pNotify         = (R2S_UPDATE_FIGHTINGSTATE_IN_CACHE*)pbyData;

    pRoomBaseInfo = g_pSO3World->m_RoomCache.GetRoomBaseInfo(pNotify->dwRoomID);
    KG_PROCESS_ERROR(pRoomBaseInfo);

    pRoomBaseInfo->bFighting = pNotify->byFightState;

Exit0:
    return;
}

void KRelayClient::OnUpdateHasPasswordInCache(BYTE* pbyData, size_t uDataLen)
{
    KRoomBaseInfo*              pRoomBaseInfo   = NULL;
    R2S_UPDATE_HASPASSWORD_IN_CACHE*  pNotify   = (R2S_UPDATE_HASPASSWORD_IN_CACHE*)pbyData;

    pRoomBaseInfo = g_pSO3World->m_RoomCache.GetRoomBaseInfo(pNotify->dwRoomID);
    KG_PROCESS_ERROR(pRoomBaseInfo);

    pRoomBaseInfo->bHasPassword = pNotify->byHasPassword;

Exit0:
    return;
}

void KRelayClient::OnSyncRoomBaseInfoForCache(BYTE* pbyData, size_t uDataLen)
{
    R2S_SYNC_ROOM_BASEINFO_FOR_CACHE* pNotify = (R2S_SYNC_ROOM_BASEINFO_FOR_CACHE*)pbyData;
    KRoomBaseInfo baseinfo;
    DWORD dwLastRoomID = 0;

    for (int i = 0; i < pNotify->byCount; ++i)
    {
        R2S_ADD_ROOM_CACHE& rRoomBaseInfo = pNotify->RoomBaseInfo[i];

        g_pSO3World->m_RoomCache.DelRoomBaseInfo(rRoomBaseInfo.dwRoomID);

        baseinfo.dwRoomID       = rRoomBaseInfo.dwRoomID;
        baseinfo.dwMapID        = rRoomBaseInfo.dwMapID;
        baseinfo.byMemberCount  = rRoomBaseInfo.byMemberCount;
        baseinfo.bFighting      = rRoomBaseInfo.bFighting;
        baseinfo.bHasPassword   = rRoomBaseInfo.bHasPassword;

        strncpy(baseinfo.szRoomName, rRoomBaseInfo.szRoomName, countof(baseinfo.szRoomName));
        baseinfo.szRoomName[countof(baseinfo.szRoomName) - 1] = '\0';

        g_pSO3World->m_RoomCache.AddRoomBaseInfo(baseinfo);

        dwLastRoomID = rRoomBaseInfo.dwRoomID;
    }

    if (pNotify->byCount == MAX_ROOM_BASE_INFO_SYNC_COUNT)
        DoApplyRoomBaseInfoForCache(dwLastRoomID);
}

// 自动组队匹配相关
void KRelayClient::OnAutoMatchRespond(BYTE* pbyData, size_t uDataLen)
{
	BOOL 				   bRetCode = false;
    R2S_AUTOMATCH_RESPOND* pRespond = (R2S_AUTOMATCH_RESPOND*)pbyData;
    KPlayer*               pPlayer  = NULL;
    
    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(!pPlayer->m_bIsMatching);

    if (pRespond->eRetCode == KAUTOMATCH_RESOND_CODE_SUCCESS)
        pPlayer->m_bIsMatching = true;

    g_PlayerServer.DoAutoMatchRespond(pPlayer->m_nConnIndex, pRespond->eRetCode, pRespond->dwPunishingMemberID, pRespond->wLeftSeconds);
Exit0:
    return;
}

void KRelayClient::OnLeaveAutoMatchNotify(BYTE* pbyData, size_t uDataLen)
{
	BOOL						bRetCode = false;
    R2S_LEAVE_AUTOMATCH_NOTIFY* pRespond = (R2S_LEAVE_AUTOMATCH_NOTIFY*)pbyData;
    KPlayer*               pPlayer  = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);

    KG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(pPlayer->m_eGameStatus == gsInHall);
    KGLOG_PROCESS_ERROR(pPlayer->m_bIsMatching);
    
    pPlayer->m_bIsMatching = false;

    g_PlayerServer.DoLeaveAutoMatchNotify(pPlayer->m_nConnIndex, pRespond->byLeaveReason);

Exit0:
    return;
}

void KRelayClient::OnSendToClient(BYTE* pbyData, size_t uDataLen)
{
    R2S_SENDTO_CLIENT* pRespond = (R2S_SENDTO_CLIENT*)pbyData;
    KPlayer*           pPlayer  = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(uDataLen == sizeof(R2S_SENDTO_CLIENT) + pRespond->uDataLen);

    g_PlayerServer.DoSendToClient(pPlayer->m_nConnIndex, pRespond->byData, pRespond->uDataLen);

Exit0:
    return;
}

void KRelayClient::OnRenameRespond(BYTE* pbyData, size_t uDataLen)
{
    R2S_RENAME_RESPOND* pRespond = (R2S_RENAME_RESPOND*)pbyData;
    KPlayer*            pPlayer  = NULL;

    pPlayer  = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    pRespond->szNewName[countof(pRespond->szNewName) - 1] = '\0';

    if (pRespond->byCode == eRenameSucceed)
    {
        strncpy(pPlayer->m_szName, pRespond->szNewName, countof(pPlayer->m_szName));
        pPlayer->m_szName[countof(pPlayer->m_szName) - 1] = '\0';

        pPlayer->m_bCanRename = false;
    }
    
    pPlayer->m_bIsRenaming = false;
    g_PlayerServer.DoRenameRespond(pPlayer->m_nConnIndex, pRespond->byCode, pRespond->dwPlayerID, pRespond->szNewName);

Exit0:
    return;
}

void KRelayClient::OnResetMap(BYTE* pbyData, size_t uDataLen)
{
    R2S_RESET_MAP*    pNotify     = (R2S_RESET_MAP*)pbyData;
    KScene*           pScene      = NULL;

    pScene = g_pSO3World->GetScene(pNotify->dwMapID, pNotify->nMapCopyIndex);
    KGLOG_PROCESS_ERROR(pScene);

    pScene->m_Param = pNotify->param;
    
    pScene->m_Battle.ApplyTemplate(pScene->m_Param.m_dwBattleTemplateID);
    pScene->m_Battle.Start(g_pSO3World->m_nGameLoop);

    memset(pScene->m_nScore, 0, sizeof(pScene->m_nScore));

    pScene->m_pSceneObjMgr->ClearAll();
    pScene->LoadBaseInfo();

    pScene->m_eSceneState = ssWaitingClientLoading;

Exit0:
    return;
}

void KRelayClient::OnCanEnterMission(BYTE* pbyData, size_t uDataLen)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    R2S_CAN_ENTER_MISSION* pNotify = (R2S_CAN_ENTER_MISSION*)pbyData;
    KPlayer* pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KMission* pMission = NULL;
    KHeroData* pHeroData = NULL;

    KG_PROCESS_ERROR(pPlayer);

    pHeroData = pPlayer->m_HeroDataList.GetHeroData(pNotify->dwHeroTemplateID);
    KGLOG_PROCESS_ERROR(pHeroData);

    pMission = g_pSO3World->m_MissionMgr.GetMission(pNotify->byMissionType, pNotify->byMissionStep, pNotify->byMissionLevel);
    KGLOG_PROCESS_ERROR(pMission);

    KG_PROCESS_ERROR(pHeroData->m_nLevel >= pMission->nRequireLevel);

    if (pNotify->byPveMode == KPVE_MISSION)
    {
        bRetCode = pPlayer->m_MissionData.IsMissionOpened(pNotify->byMissionType, pNotify->byMissionStep, pNotify->byMissionLevel);
        KG_PROCESS_ERROR(bRetCode);

        if (pMission->dwRequireQuest)
        {
            QUEST_STATE eState = pPlayer->m_QuestList.GetQuestState(pMission->dwRequireQuest);
            KG_PROCESS_ERROR(eState == qsFinished);
        }

    }
    else if (pNotify->byPveMode == KPVE_CONTINUOUS_CHALLENGE)
    {
        bRetCode = pPlayer->CanStartContinuousChallenge();
        KG_PROCESS_ERROR(bRetCode);
    }
    
    bResult = true;
Exit0:
    DoCanEnterMissionRespond(pNotify->dwPlayerID, pNotify->dwHeroTemplateID, pNotify->byMissionType, pNotify->byMissionStep, pNotify->byMissionLevel, bResult);
    return;
}

void KRelayClient::OnFreePVPInvite(BYTE* pbyData, size_t uDataLen)
{
    BOOL            bResult     = false;
    KGBlackNode*    pNode       = NULL;
    KGFellowship*   pFellowship = NULL;
    KPlayer*        pInvitee    = NULL;
    R2S_NOTIFY_FREEPVP_INVITE* pNotify = (R2S_NOTIFY_FREEPVP_INVITE*)pbyData;

    pNode = g_pSO3World->m_FellowshipMgr.GetBlackListNode(pNotify->dwInviteeID, pNotify->dwInvitorID);
    KG_PROCESS_ERROR(!pNode);

    pInvitee = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwInviteeID);
    KGLOG_PROCESS_ERROR(pInvitee);

    pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(pNotify->dwInviteeID, pNotify->dwInvitorID);
    if (pInvitee->m_bRefuseStranger && !pFellowship)
    {
        g_LSClient.DoRemoteCall("OnRefuseStrangerNotify", (int)pNotify->dwInvitorID);
        goto Exit0;
    }
    
    if (pInvitee->m_eGameStatus != gsInHall)
    {
        DoAcceptOrRefuseJoinRoom(pInvitee, false, pNotify->dwRoomID);
        goto Exit0;
    }
    
    g_PlayerServer.DoFreePVPInviteNotify(pInvitee, pNotify->szInvitorName, pNotify->dwRoomID, pNotify->dwMapID);

    bResult = true;
Exit0:
    return;
}

void KRelayClient::OnLadderPVPInvite(BYTE* pbyData, size_t uDataLen)
{
    BOOL            bResult     = false;
    KGBlackNode*    pNode       = NULL;
    KGFellowship*   pFellowship = NULL;
    KPlayer*        pInvitee    = NULL;
    R2S_NOTIFY_LADDERPVP_INVITE* pNotify = (R2S_NOTIFY_LADDERPVP_INVITE*)pbyData;

    pNode = g_pSO3World->m_FellowshipMgr.GetBlackListNode(pNotify->dwInviteeID, pNotify->dwInvitorID);
    KG_PROCESS_ERROR(!pNode);

    pInvitee = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwInviteeID);
    KGLOG_PROCESS_ERROR(pInvitee);

    pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(pNotify->dwInviteeID, pNotify->dwInvitorID);
    if (pInvitee->m_bRefuseStranger && !pFellowship)
    {
        g_LSClient.DoRemoteCall("OnRefuseStrangerNotify", (int)pNotify->dwInvitorID);
        goto Exit0;
    }

    if (pInvitee->m_eGameStatus != gsInHall)
    {
        DoTeamAcceptOrRefuseInvite(pInvitee, false, pNotify->szInvitorName);
        goto Exit0;
    }

    g_PlayerServer.DoLadderPVPInviteNotify(pInvitee, pNotify->szInvitorName);

    bResult = true;
Exit0:
    return;
}

void KRelayClient::OnPVEInvite(BYTE* pbyData, size_t uDataLen)
{
    BOOL            bResult     = false;
    KGBlackNode*    pNode       = NULL;
    KGFellowship*   pFellowship = NULL;
    KPlayer*        pInvitee    = NULL;
    R2S_NOTIFY_PVE_INVITE* pNotify = (R2S_NOTIFY_PVE_INVITE*)pbyData;

    pNode = g_pSO3World->m_FellowshipMgr.GetBlackListNode(pNotify->dwInviteeID, pNotify->dwInvitorID);
    KG_PROCESS_ERROR(!pNode);

    pInvitee = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwInviteeID);
    KGLOG_PROCESS_ERROR(pInvitee);
     
    pFellowship = g_pSO3World->m_FellowshipMgr.GetFellowship(pNotify->dwInviteeID, pNotify->dwInvitorID);
    if (pInvitee->m_bRefuseStranger && !pFellowship)
    {
        g_LSClient.DoRemoteCall("OnRefuseStrangerNotify", (int)pNotify->dwInvitorID);
        goto Exit0;
    }


    if (pInvitee->m_eGameStatus != gsInHall)
    {
        DoPveTeamAcceptOrRefuseInvite(pInvitee, false, pNotify->szInvitorName);
        goto Exit0;
    }

    g_PlayerServer.DoPVEInviteNotify(pInvitee, pNotify->szInvitorName, pNotify->byMode, 
        pNotify->byMissionType, pNotify->byMissionStep, pNotify->byMissionLevel);

    bResult = true;
Exit0:
    return;
}

void KRelayClient::OnChangeExtPointRespond(BYTE* pbyData, size_t uDataLen)
{
    BOOL                            bRetCode    = false;
    R2S_CHANGE_EXT_POINT_RESPOND*   pRespond    = (R2S_CHANGE_EXT_POINT_RESPOND*)pbyData;
    KPlayer*                        pPlayer     = NULL;
    int                             nTopIndex   = 0;
    const char*                     pszFuncName = "OnSetExtPointSucceed";

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    KGLogPrintf(
        KGLOG_INFO, "Change ExtPoint %s: Acc = %s, Role = %s, ID = %u, ExtPointIndex = %u, nChangeValue = %d",
        pRespond->bSucceed ? "succeed" : "failed", 
        pPlayer->m_szAccount, pPlayer->m_szName, pPlayer->m_dwID,
        pRespond->uExtPointIndex, pRespond->nChangeValue
    );

    pPlayer->m_bExtPointLock = false;

    if (!pRespond->bSucceed)
    {
        pPlayer->m_ExtPointInfo.nExtPoint[pPlayer->m_nLastExtPointIndex] = pPlayer->m_nLastExtPointValue;
        goto Exit0;
    }

    bRetCode = g_pSO3World->m_ScriptManager.IsScriptExist(PLAYER_EXT_POINT_SCRIPT);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_pSO3World->m_ScriptManager.IsFuncExist(PLAYER_EXT_POINT_SCRIPT, pszFuncName);
    KG_PROCESS_ERROR(bRetCode);

    g_pSO3World->m_ScriptManager.SafeCallBegin(&nTopIndex);

    g_pSO3World->m_ScriptManager.Push(pPlayer);
    g_pSO3World->m_ScriptManager.Push(pPlayer->m_nLastExtPointIndex);
    g_pSO3World->m_ScriptManager.Push((int)(pPlayer->m_nLastExtPointValue));
    g_pSO3World->m_ScriptManager.Call(PLAYER_EXT_POINT_SCRIPT, pszFuncName, 0);

    g_pSO3World->m_ScriptManager.SafeCallEnd(nTopIndex);

Exit0:
    return;
}

void KRelayClient::OnForbidPlayerTalk(BYTE* pbyData, size_t uDataLen)
{
    BOOL                    bRetCode    = false;
    R2S_FORBID_PLAYER_TALK* pRespond    = (R2S_FORBID_PLAYER_TALK*)pbyData;
    KPlayer*                pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_nForbidTalkTime  = g_pSO3World->m_nCurrentTime + pRespond->dwTime;
Exit0:
    return;
}

void KRelayClient::OnForbidIPTalk(BYTE* pbyData, size_t uDataLen)
{
    BOOL                    bRetCode    = false;
    R2S_FORBID_PLAYER_TALK* pRespond    = (R2S_FORBID_PLAYER_TALK*)pbyData;
    KPlayer*                pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pRespond->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    pPlayer->m_nForbidTalkTime  = g_pSO3World->m_nCurrentTime + pRespond->dwTime;
Exit0:
    return;
}

void KRelayClient::OnFreezeRole(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bRetCode    = false;
    R2S_FREEZE_NOTIFY*  pNotify     = (R2S_FREEZE_NOTIFY*)pbyData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoAccountKickout(pPlayer->m_nConnIndex);

    if (pPlayer->m_nConnIndex >= 0)
    {
        KGLogPrintf(KGLOG_INFO, "Kick account(%s,%s) from game center, shutdown(%d)\n", pPlayer->m_szAccount, pPlayer->m_szName, pPlayer->m_nConnIndex);

        g_PlayerServer.Detach(pPlayer->m_nConnIndex);
    }

Exit0:
    return;
}

void KRelayClient::OnFreezeIP(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bRetCode    = false;
    R2S_FREEZE_NOTIFY*  pNotify     = (R2S_FREEZE_NOTIFY*)pbyData;
    KPlayer*            pPlayer     = NULL;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    g_PlayerServer.DoAccountKickout(pPlayer->m_nConnIndex);

    if (pPlayer->m_nConnIndex >= 0)
    {
        KGLogPrintf(KGLOG_INFO, "Kick account(%s,%s) from game center, shutdown(%d)\n", pPlayer->m_szAccount, pPlayer->m_szName, pPlayer->m_nConnIndex);

        g_PlayerServer.Detach(pPlayer->m_nConnIndex);
    }

Exit0:
    return;
}

void KRelayClient::OnAccountNewCoinNotify(BYTE* pbyData, size_t uDataLen)
{
	BOOL        bResult 	= false;
	BOOL        bRetCode 	= false;
    KPlayer*    pPlayer     = NULL;
    R2S_ACCOUNT_NEW_COIN_NOTIFY* pNotify = (R2S_ACCOUNT_NEW_COIN_NOTIFY*)pbyData;
    int         nChargeCoin = 0;
    int         nCurrentCoin = 0;
    int         nHaveCharged = 0;
    KReport_Recharge param;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(pNotify->dwPlayerID);
    KGLOG_PROCESS_ERROR(pPlayer);

    nCurrentCoin = pPlayer->m_MoneyMgr.GetPayCoin();

    nChargeCoin = pNotify->nNewCoin - nCurrentCoin;
    KGLOG_PROCESS_ERROR(nChargeCoin > 0);

    bRetCode = pPlayer->m_MoneyMgr.AddMoney(emotCoin, nChargeCoin);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = pPlayer->AddVIPExp(nChargeCoin / 100);
    KGLOG_PROCESS_ERROR(bRetCode);
	
    param.coinCount = nChargeCoin;
    pPlayer->m_Secretary.AddReport(KREPORT_EVENT_RECHARGE, (BYTE*)&param, sizeof(param));

    bRetCode = pPlayer->SetPlayerValue(KPLAYER_VALUE_ID_FIRST_CHARGE, 1);
    KGLOG_PROCESS_ERROR(bRetCode);
    
	bResult = true;
Exit0:
	return;
}

void KRelayClient::OnSendItemMailNotify(BYTE* pbyData, size_t uDataLen)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    R2S_SEND_ITEM_MAIL_NOTIFY* pNotify = (R2S_SEND_ITEM_MAIL_NOTIFY*)pbyData;
    int         nItemInfoCount = 0;
    IItem*      piItem      = NULL;
    IItem*      pItem[KMAIL_MAX_ITEM_COUNT] = {0};
    const KItemProperty*    pItemProperty = NULL;
    uint32_t    dwTabType   = 0;
    uint32_t    dwTabIndex  = 0;
    uint32_t    dwItemStack = 0;
    uint32_t    uMailLen    = 0;
    KMail*      pMail       = NULL;

    KGLOG_PROCESS_ERROR(pNotify->byItemCount <= KMAIL_MAX_ITEM_COUNT);

    nItemInfoCount = pNotify->byItemCount * 3;
    KGLOG_PROCESS_ERROR(uDataLen == sizeof(R2S_SEND_ITEM_MAIL_NOTIFY) + nItemInfoCount * sizeof(int));

    for (int i = 0; i < pNotify->byItemCount; ++i)
    {
        dwTabType   = pNotify->nTabInfo[i * 3];
        dwTabIndex  = pNotify->nTabInfo[i * 3 + 1];
        dwItemStack = pNotify->nTabInfo[i * 3 + 2];

        if (dwTabType != 0)
        {
            piItem = g_pSO3World->m_ItemHouse.CreateItem(dwTabType, dwTabIndex, 0, 0, 0);
            KGLOG_PROCESS_ERROR(piItem);

            pItemProperty = piItem->GetProperty();
            if (piItem->CanStack())
                piItem->SetStackNum(dwItemStack);
            else
                piItem->SetCurrentDurability(pItemProperty->pItemInfo->nMaxDurability);

            pItem[i] = piItem;
            piItem   = NULL;
            pItemProperty = NULL;
        }
    }
        
    pMail = (KMail*)m_byTempData;
    bRetCode = g_pSO3World->FillMail(
        eMailType_System, "", pNotify->szTitle, pNotify->szText, 
        strlen(pNotify->szText), pItem, (int)pNotify->byItemCount, pMail, uMailLen
    );
    pMail->nMoney = pNotify->nMoneyNum;
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = g_LSClient.DoSendMailRequest(0, pNotify->szReceiverName, pMail, uMailLen);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (piItem)
        g_pSO3World->m_ItemHouse.DestroyItem(piItem);

    for (int i = 0; i < KMAIL_MAX_ITEM_COUNT; ++i)
    {
        if (pItem[i])
            g_pSO3World->m_ItemHouse.DestroyItem(pItem[i]);
    }

    return;
}

