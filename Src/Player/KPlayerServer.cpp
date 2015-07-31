#include "stdafx.h"
#include "KPlayerServer.h"
#include "Engine/KG_Time.h"
#include "Engine/KG_CreateGUID.h"
#include "Common/CRC32.h"
#include "Common/KG_Socket.h"
#include "Common/KG_Package.h"
#include "KPlayer.h"
#include "KRelayClient.h"
#include "KMath.h"
#include "KScene.h"
#include "KSO3World.h"
#include "Engine/inoutmac.h"

#ifdef __GNUC__
#include <netinet/tcp.h>
#endif

#define KG_MAX_ACCEPT_EACH_WAIT     8

KPlayerServer g_PlayerServer;

int KPlayerServer::_Construct()
{
	m_nMaxConnection            = 0;
    m_nMaxClientPackPerFrame    = 0;
	m_ConnectionDataList        = NULL;

    m_nSocketEventCount         = 0;
    m_pSocketEventArray         = NULL;

    m_fS2CTraffic               = 0.0f;
    m_fC2STraffic               = 0.0f;
    m_nS2CTraffic               = 0;
    m_nC2STraffic               = 0;

    m_nListenPort               = 0;
    m_dwInternetAddr            = 0;
    m_szLocalIP[0]              = '\0';

    m_nPingCycle                = 0;
    m_nClientGM                 = 0;
    m_nNextCheckConnection      = 0;
    m_dwTimeNow                 = 0;

    memset(m_bCloseFlag, 0, sizeof(m_bCloseFlag));

    return true;
}

int KPlayerServer::_Destroy()
{
	assert(!m_pSocketEventArray);
    assert(!m_ConnectionDataList); 
    
    return true;
}

KPlayerServer::KPlayerServer(void)
{
    _Construct();
    InitProcotolProcess();
}

KPlayerServer::~KPlayerServer(void)
{
    _Destroy();
}

// 初始化面向客户端的连接
BOOL KPlayerServer::Init()
{
    BOOL            bResult                 = false;
    int             nRetCode                = false;
    BOOL            bSocketServerInit       = false;
    IIniFile*	    piIniFile               = NULL;
    unsigned long   ulInternalIPAddressMask = 0;
    unsigned long   ulExternalIPAddress     = INADDR_NONE;
    unsigned long   ulInternalIPAddress     = INADDR_NONE;
    //int				nListenPort             = 0;
    int             nNatPort                = 0;
    int 			nMaxConnection          = 0;
    int             nRecvBufferSize         = 0;
    int             nSendBufferSize         = 0;
    int             nClientGM               = 0;
    int             nMaxClientPackPerFrame  = 0;
    char            szNatIP[16];
    char            szInternalIPMask[16];
    unsigned char   byInternalMacAddress[6];
    unsigned char   byExternalMacAddress[6];
    struct          in_addr  InAddr;
    
    assert(g_pSO3World);
    
    piIniFile = g_OpenIniFile(GS_SETTINGS_FILENAME);
	KGLOG_PROCESS_ERROR(piIniFile);

    memset(szNatIP, 0, sizeof(szNatIP));

    piIniFile->GetString("GS-Player", "NatIP", "", szNatIP, sizeof(szNatIP));
    piIniFile->GetInteger("GS-Player", "NatPort", 0, &nNatPort);
    piIniFile->GetString("GS-Player", "InternalIPAddressMask", "192.168.0.0", szInternalIPMask, 16);
	piIniFile->GetInteger("GS-Player", "MaxConnection", 1024, &nMaxConnection);
	piIniFile->GetInteger("GS-Player", "RecvBufferSize", 8000, &nRecvBufferSize);
	piIniFile->GetInteger("GS-Player", "SendBufferSize", 64000, &nSendBufferSize); 
    piIniFile->GetInteger("GS-Player", "PingCycle", 0, &m_nPingCycle);
    piIniFile->GetInteger("GS-Player", "ClientGM", 0, &nClientGM);
    piIniFile->GetInteger("GS-Player", "MaxClientPackPerFrame", 256, &nMaxClientPackPerFrame);

    if (m_szLocalIP[0] != '\0')
    {
         ulExternalIPAddress = (unsigned long)inet_addr(m_szLocalIP);
         if (ulExternalIPAddress == INADDR_NONE)
         {
             KGLogPrintf(KGLOG_ERR, "[KPlayerServerBase] Invalid ip address or format.\n");
             goto Exit0;
         }    
    }
    else
    {
        char* pszInternetAddr = NULL;

        ulInternalIPAddressMask = inet_addr(szInternalIPMask);
        KGLOG_PROCESS_ERROR(ulInternalIPAddressMask != INADDR_NONE);

        nRetCode = gGetMacAndIPAddress(
            byInternalMacAddress, &ulInternalIPAddress, 
            byExternalMacAddress, &ulExternalIPAddress, ulInternalIPAddressMask
        );
        KGLOG_PROCESS_ERROR(nRetCode > 0);

        InAddr.s_addr = ulExternalIPAddress;
        pszInternetAddr = inet_ntoa(InAddr);
        KGLOG_PROCESS_ERROR(pszInternetAddr);

        strcpy(m_szLocalIP, pszInternetAddr);
    }
    
    KGLOG_PROCESS_ERROR(nMaxConnection > 0);
    
    m_dwTimeNow = KG_GetTickCount();
    m_nClientGM = nClientGM;

    ResetPakStat();

	m_nMaxConnection = nMaxConnection;
    m_nMaxClientPackPerFrame = nMaxClientPackPerFrame;

	m_ConnectionDataList = new KConnectionData[nMaxConnection];
	KGLOG_PROCESS_ERROR(m_ConnectionDataList);

    m_ConnectionDataListFreeVector.reserve(nMaxConnection);

    for (int i = 0; i < nMaxConnection; i++)
    {
        m_ConnectionDataListFreeVector.push_back(nMaxConnection - i - 1);
    }

    m_nSocketEventCount = nMaxConnection + KG_MAX_ACCEPT_EACH_WAIT;

    m_pSocketEventArray = new KG_SOCKET_EVENT[m_nSocketEventCount];
    KGLOG_PROCESS_ERROR(m_pSocketEventArray);

    nRetCode = m_SocketServerAcceptor.Init(
        m_szLocalIP, m_nListenPort, KG_MAX_ACCEPT_EACH_WAIT, 
        nRecvBufferSize, nSendBufferSize, KSG_ENCODE_DECODE_NONE, NULL
    );

    KGLogPrintf(
        KGLOG_INFO, "Start service at %s:%d ... ... [%s]",
        m_szLocalIP, m_nListenPort, nRetCode ? "OK" : "Failed"
    );
    
    KGLOG_PROCESS_ERROR(nRetCode);
    bSocketServerInit = true;

	m_dwInternetAddr    = ulExternalIPAddress;
    if (szNatIP[0] != '\0')
    {
        m_nListenPort       = nNatPort;
        m_dwInternetAddr    = (unsigned long)inet_addr(szNatIP);

        KGLogPrintf(KGLOG_INFO, "NAT at %s:%d", szNatIP, nNatPort);
    }

    m_nNextCheckConnection = 0;

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bSocketServerInit)
        {
            m_SocketServerAcceptor.UnInit(NULL);
            bSocketServerInit = false;
        }
        KG_DELETE_ARRAY(m_pSocketEventArray);
    	KG_DELETE_ARRAY(m_ConnectionDataList);
    }
    KG_COM_RELEASE(piIniFile);
	return bResult;
}

BOOL KPlayerServer::UnInit(void)
{
    for (int nConnIndex = 0; nConnIndex < m_nMaxConnection; nConnIndex++)
    {
        if (m_ConnectionDataList[nConnIndex].piSocketStream)
        {
            Shutdown(nConnIndex);
        }
    }

    DumpPakStat();
    
    m_SocketServerAcceptor.UnInit(NULL);

    KG_DELETE_ARRAY(m_pSocketEventArray);
    m_nSocketEventCount = 0;

    KG_DELETE_ARRAY(m_ConnectionDataList);
    m_nMaxConnection = 0;

	return true;
}

void KPlayerServer::Activate()
{
    assert(g_pSO3World);

    m_dwTimeNow = KG_GetTickCount();

    if (g_pSO3World->m_nGameLoop % GAME_FPS == 0)
    {
        m_fS2CTraffic = m_fS2CTraffic * 0.9f + (float)m_nS2CTraffic * 0.1f;
        m_fC2STraffic = m_fC2STraffic * 0.9f + (float)m_nC2STraffic * 0.1f;

        m_nS2CTraffic = 0;
        m_nC2STraffic = 0;
    }

    for (int i = 0; i < 8; i++)
    {
        int              nConnIndex  = m_nNextCheckConnection;
        KConnectionData* pConnection = m_ConnectionDataList + nConnIndex;

        m_nNextCheckConnection = (m_nNextCheckConnection + 1) % m_nMaxConnection;

        if (pConnection->piSocketStream == NULL)
            continue;

        if (m_dwTimeNow > pConnection->dwLastPingTime + m_nPingCycle * 1000)
        {
            KGLogPrintf(KGLOG_ERR, "Connection timeout: %d\n", nConnIndex);

            Detach(nConnIndex);
            Shutdown(nConnIndex);

            continue;
        }

        if (g_pSO3World->m_nGameLoop >= pConnection->nStatLastUpdateFrame + GAME_FPS)
        {
            int nDeltaFrame = g_pSO3World->m_nGameLoop - pConnection->nStatLastUpdateFrame;

            pConnection->fStatSendSpeed = pConnection->fStatSendSpeed * 0.8f + pConnection->uStatSendByteCount * 0.2f * GAME_FPS / nDeltaFrame;
            pConnection->uStatSendByteCount = 0;
            pConnection->nStatLastUpdateFrame = g_pSO3World->m_nGameLoop;
        }

        if (pConnection->bSendError)
        {
            Detach(nConnIndex);
            Shutdown(nConnIndex);
            pConnection->bSendError = false;
        }
    }
}

void KPlayerServer::ProcessNetwork(void)
{
	int             nRetCode    = false;

    while (true)
    {
        int                 nEventCount     = 0;
        KG_SOCKET_EVENT*    pSocketEvent    = NULL;
        KG_SOCKET_EVENT*    pSocketEventEnd = NULL;

        nRetCode = m_SocketServerAcceptor.Wait(m_nSocketEventCount, m_pSocketEventArray, &nEventCount);
        KGLOG_PROCESS_ERROR(nRetCode);

        if (nEventCount == 0)
            break;

        pSocketEventEnd = m_pSocketEventArray + nEventCount;
        for (pSocketEvent = m_pSocketEventArray; pSocketEvent < pSocketEventEnd; pSocketEvent++)
        {
            // Process new connection accept
            // KG_SOCKET_EVENT_ACCEPT event must be processed before any other event, 
            // because Maybe uEventFlag = KG_SOCKET_EVENT_ACCEPT | KG_SOCKET_EVENT_IN.
            if (pSocketEvent->uEventFlag & KG_SOCKET_EVENT_ACCEPT)
            {
                ProcessNewConnection(pSocketEvent->piSocket);
                KG_COM_RELEASE(pSocketEvent->piSocket);
                continue;
            }

            if (!(pSocketEvent->uEventFlag & KG_SOCKET_EVENT_IN))
            {
                KGLogPrintf(KGLOG_DEBUG, "Unexpected socket event: %u", pSocketEvent->uEventFlag);
                KG_COM_RELEASE(pSocketEvent->piSocket);
                continue;
            }

            ProcessPackage(pSocketEvent->piSocket);

            KG_COM_RELEASE(pSocketEvent->piSocket);
        }
	}

Exit0:
    return;
}

BOOL KPlayerServer::Attach(KPlayer* pPlayer, int nConnIndex)
{
    BOOL                    bResult         = false;
	BOOL                    bRetCode        = false;
    struct in_addr          RemoteAddr      = { 0 };
    u_short                 wRemotePortNet  = 0;
    char*                   pszRetString    = NULL;

    assert(nConnIndex >= 0 && nConnIndex < m_nMaxConnection);
    assert(m_ConnectionDataList[nConnIndex].piSocketStream);
    assert(m_ConnectionDataList[nConnIndex].pPlayer == NULL);

    assert(pPlayer->m_nConnIndex == -1);
    assert(pPlayer->m_eGameStatus == gsWaitForConnect);

	m_ConnectionDataList[nConnIndex].pPlayer = pPlayer;

	pPlayer->m_nConnIndex   = nConnIndex;

    m_ConnectionDataList[nConnIndex].piSocketStream->GetRemoteAddress(&RemoteAddr, &wRemotePortNet);

    pszRetString = inet_ntoa(RemoteAddr);
    KGLOG_PROCESS_ERROR(pszRetString);

    pPlayer->m_dwClientIP   = inet_addr( pszRetString );

    KGLogPrintf(
        KGLOG_INFO, "Bind %s:(%u) to connection(%d)\n", 
        pPlayer->m_szAccount, pPlayer->m_dwID, nConnIndex
    );

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::Detach(int nConnIndex)
{
    BOOL        bResult     = false;
    KPlayer*    pPlayer     = NULL;

    assert(nConnIndex >= 0 && nConnIndex < m_nMaxConnection);

    pPlayer = m_ConnectionDataList[nConnIndex].pPlayer;
    KG_PROCESS_ERROR(pPlayer);

    // 暂时这么处理，以后应该执行托管
    pPlayer->Logout();

	m_ConnectionDataList[nConnIndex].pPlayer = NULL;

	pPlayer->m_nConnIndex = -1;

	switch (pPlayer->m_eGameStatus)
	{
	case gsPlaying:
        pPlayer->RemoveHero();
        g_RelayClient.SaveRoleData(pPlayer);
        g_pSO3World->DelPlayer(pPlayer);
		break;

    case gsInHall:
    case gsSearchMap:
    case gsWaitForNewMapLoading:
        g_RelayClient.SaveRoleData(pPlayer);
        g_pSO3World->DelPlayer(pPlayer);
        break;

    //case gsWaitForConnect: 此状态下网关还未解锁,在连接超时处删除玩家
	case gsWaitForPermit:
	case gsWaitForRoleData:
    case gsWaitForTransmissionSave:
    case gsWaitForTransmissionGuid:
		g_pSO3World->DelPlayer(pPlayer);
		break;
	default:
        KGLogPrintf(
            KGLOG_ERR, "Unexpected GameStatus %d for player %s when connection close !",
            pPlayer->m_eGameStatus, pPlayer->m_szName
        );
        break;
	}

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerServer::Send(int nConnIndex, void* pvData, size_t uDataLen)
{
    int                         nResult     = false;
    int                         nRetCode    = false;
    KS2C_Header*                pPakHeader  = (KS2C_Header*)pvData;
    KConnectionData*            pConnection = NULL;

    assert(pPakHeader->protocolID > gs_client_connection_begin);
    assert(pPakHeader->protocolID < gs_client_connection_end);

    KG_PROCESS_ERROR(nConnIndex >= 0 && nConnIndex < m_nMaxConnection);

    pConnection = &m_ConnectionDataList[nConnIndex];
    KG_PROCESS_ERROR(pConnection->piSocketStream);
    KG_PROCESS_ERROR(!pConnection->bSendError);

    m_S2CPakStat[pPakHeader->protocolID].dwPackCount++;
    m_S2CPakStat[pPakHeader->protocolID].uTotalSize += uDataLen;

    m_nS2CTraffic += (int)uDataLen;
    pConnection->uStatSendByteCount += uDataLen;

    nRetCode = pConnection->SocketPacker.Send(
        pConnection->piSocketStream, (unsigned)uDataLen, (const unsigned char*)pvData
    );
    if (!nRetCode)
    {
        KGLogPrintf(KGLOG_ERR, "Send error: %d\n", nConnIndex);
        pConnection->bSendError = true;
        goto Exit0;
    }

	nResult = true;
Exit0:
	return nResult;
}

void KPlayerServer::FlushSend()
{
    int              nRetCode   = false;
    KConnectionData* pConn      = m_ConnectionDataList;
    KConnectionData* pConnEnd   = m_ConnectionDataList + m_nMaxConnection;

    while (pConn < pConnEnd)
    {
        if (pConn->piSocketStream && !pConn->bSendError)
        {
            nRetCode = pConn->SocketPacker.FlushSend(pConn->piSocketStream);
            if (!nRetCode)
            {
                KGLogPrintf(
                    KGLOG_ERR, "Flush send error: %d\n", 
                    (int)(pConn - m_ConnectionDataList)
                );
                pConn->bSendError = true;
            }
        }

        pConn++;
    }
}

void KPlayerServer::ResetPakStat()
{
    memset(m_S2CPakStat, 0, sizeof(m_S2CPakStat));
    memset(m_C2SPakStat, 0, sizeof(m_C2SPakStat));
}

BOOL KPlayerServer::DumpPakStat()
{
    BOOL  bResult   = false;
    FILE* pFile     = NULL;

    pFile = fopen("s2c_pak_stat.tab", "w");
    KG_PROCESS_ERROR(pFile);

    fprintf(pFile, "Protocol\tPackageCount\tTotalSize\n");

    for (int nProtocol = gs_client_connection_begin + 1; nProtocol < gs_client_connection_end; nProtocol++)
    {
        KPROTOCOL_STAT_INFO* pStatInfo = m_S2CPakStat + nProtocol;

        fprintf(pFile, "%d\t%u\t%llu\n", nProtocol, pStatInfo->dwPackCount, pStatInfo->uTotalSize);
    }

    fclose(pFile);
    pFile = NULL;

    pFile = fopen("c2s_pak_stat.tab", "w");
    KG_PROCESS_ERROR(pFile);

    fprintf(pFile, "Protocol\tPackageCount\tTotalSize\n");

    for (int nProtocol = client_gs_connection_begin + 1; nProtocol < client_gs_connection_end; nProtocol++)
    {
        KPROTOCOL_STAT_INFO* pStatInfo = m_C2SPakStat + nProtocol;

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

BOOL KPlayerServer::Shutdown(int nConnIndex)
{
    assert(nConnIndex >= 0 && nConnIndex < m_nMaxConnection);

    assert(m_ConnectionDataList[nConnIndex].piSocketStream);

    m_ConnectionDataList[nConnIndex].piSocketStream->SetUserData((void*)(ptrdiff_t)-1);

    KG_COM_RELEASE(m_ConnectionDataList[nConnIndex].piSocketStream);

    m_ConnectionDataListFreeVector.push_back(nConnIndex);

    KGLogPrintf(KGLOG_INFO, "Shutdown connection: %d\n", nConnIndex);

    return true;
}

struct KSOCKET_STREAM_BODY
{
	void* pVTable;
	unsigned long  m_ulRefCount;
	int   m_nSocketHandle;
};

BOOL KPlayerServer::ProcessNewConnection(IKG_SocketStream* piSocket)
{
    BOOL            bResult         = false;
    int             nRetCode        = 0;
    int             nConnIndex      = -1;
    u_short         uRemotePort     = 0;
    const char*     pcszIP          = NULL;
    struct in_addr  RemoteIP;
	KSOCKET_STREAM_BODY* 	pSocketBody = NULL;
	INT 					nNagle = 1;

    assert(piSocket);

    nRetCode = m_ConnectionDataListFreeVector.empty();
    KGLOG_PROCESS_ERROR(!nRetCode);

    nRetCode = piSocket->GetRemoteAddress(&RemoteIP, &uRemotePort);
    KGLOG_PROCESS_ERROR(nRetCode);

    pcszIP = inet_ntoa(RemoteIP);
    KGLOG_PROCESS_ERROR(pcszIP);

    nConnIndex = m_ConnectionDataListFreeVector.back();
    m_ConnectionDataListFreeVector.pop_back();

    assert(nConnIndex >= 0 && nConnIndex < m_nMaxConnection);

    nRetCode = piSocket->SetUserData((void*)(ptrdiff_t)nConnIndex);
    KGLOG_PROCESS_ERROR(nRetCode);

	pSocketBody = (KSOCKET_STREAM_BODY*)piSocket;
	nRetCode = setsockopt(pSocketBody->m_nSocketHandle, IPPROTO_TCP, TCP_NODELAY, (char*)&nNagle, sizeof(nNagle));
	KGLOG_CHECK_ERROR(nRetCode >= 0);

    m_ConnectionDataList[nConnIndex].piSocketStream = piSocket;
    m_ConnectionDataList[nConnIndex].piSocketStream->AddRef();

    m_ConnectionDataList[nConnIndex].SocketPacker.Reset();
    m_ConnectionDataList[nConnIndex].dwLastPingTime         = m_dwTimeNow;
    m_ConnectionDataList[nConnIndex].pPlayer                = NULL;
    m_ConnectionDataList[nConnIndex].bSendError             = false;
    m_ConnectionDataList[nConnIndex].uStatSendByteCount     = 0;
    m_ConnectionDataList[nConnIndex].fStatSendSpeed         = 0.0f;
    m_ConnectionDataList[nConnIndex].nStatLastUpdateFrame   = g_pSO3World->m_nGameLoop;

    KGLogPrintf(KGLOG_INFO, "New connection from %s:%u, index = %d\n", pcszIP, uRemotePort, nConnIndex);

    bResult = true;
Exit0:
    if (!bResult)
    {
        piSocket->SetUserData((void *)(ptrdiff_t)-1);
    }
    return bResult;
}

BOOL KPlayerServer::ProcessPackage(IKG_SocketStream* piSocket)
{
    BOOL                bResult         = false;
    int                 nRetCode        = 0;
    int                 nConnIndex      = -1;
    IKG_Buffer*         piBuffer        = NULL;
    BYTE*               pbyData         = NULL;
    unsigned            uDataLen        = 0;
    int                 nPackCount      = 0;
    BOOL                bShutDownFlag   = false;

    assert(piSocket);

    nConnIndex = (int)(ptrdiff_t)(piSocket->GetUserData());
    KGLOG_PROCESS_ERROR(nConnIndex >= 0 && nConnIndex < m_nMaxConnection);

    while (true)
    {
        KG_COM_RELEASE(piBuffer);

       /* if (nPackCount++ >= m_nMaxClientPackPerFrame)
        {
            KGLogPrintf(KGLOG_INFO, "Client send too many packs, connection: %d\n", nConnIndex);

            bShutDownFlag = true;
            break;
        }*/

        nRetCode = piSocket->Recv(&piBuffer);
        if (nRetCode == -2)
        {
            break;
        }

        if (nRetCode == -1)
        {
            KPlayer* pPlayer = GetPlayerByConnection(nConnIndex);
            int r = piSocket->GetLastError();

            if (pPlayer)
                KGLogPrintf(KGLOG_INFO, "Connection lost: %d, err:%d, player:%d\n", nConnIndex, r, pPlayer->m_dwID);
            else
                KGLogPrintf(KGLOG_INFO, "Connection lost: %d, err:%d\n", nConnIndex, r);

            bShutDownFlag = true;
            break;
        }

        KGLOG_PROCESS_ERROR(piBuffer);

        pbyData   = (BYTE*)piBuffer->GetData();
        KGLOG_PROCESS_ERROR(pbyData);

        uDataLen = piBuffer->GetSize();

        nRetCode  = CheckPackage(pbyData, uDataLen);
        if (!nRetCode)
        {
		    KGLogPrintf(KGLOG_INFO, "Pak error, connection: %d\n", nConnIndex);

            bShutDownFlag = true;
            break;
        }

        KPlayer*        pPlayer = GetPlayerByConnection(nConnIndex);
	    KC2S_Header*    pHeader = (KC2S_Header*)pbyData;

        if (m_bCloseFlag[pHeader->protocolID] != 0)
            continue;

        m_nC2STraffic += (int)uDataLen;

        m_C2SPakStat[pHeader->protocolID].dwPackCount++;
        m_C2SPakStat[pHeader->protocolID].uTotalSize += uDataLen;

        m_ConnectionDataList[nConnIndex].dwLastPingTime = m_dwTimeNow;

        if (pPlayer)
        {
            if (pPlayer->m_eGameStatus == gsPlaying)
            {
                // 帧数不可能比前面的还低, 帧数不可能超过服务端一定范围(客户端帧平衡算法决定的)
                if (
                    (pHeader->frame < pPlayer->m_nLastClientFrame) //|| 
                    //(pHeader->nFrame > g_pSO3World->m_nGameLoop + GAME_FPS / 2)
                )
                {
                    KGLogPrintf(
                        KGLOG_ERR, "Frame error(%d): last frame = %d, pak frame = %d , server frame = %d\n", 
                        nConnIndex, pPlayer->m_nLastClientFrame, pHeader->frame, g_pSO3World->m_nGameLoop
                    );

                    bShutDownFlag = true;
                    break;
                }

                KHero* pFightingHero = pPlayer->GetFightingHero();
                if (!pFightingHero)
                    continue;

                if (!pFightingHero->m_pScene)
                    continue;

                BOOL bShouldSkip = false;

                switch (pFightingHero->m_pScene->m_eSceneState)
                {
                case ssWaitingClientLoading:
                    if (pHeader->protocolID != c2s_ping_signal && pHeader->protocolID != c2s_loading_complete)
                        bShouldSkip = true;
                    break;
                case ssCountDown:
                    if (pHeader->protocolID != c2s_ping_signal && pHeader->protocolID != c2s_apply_leave_mission)
                        bShouldSkip = true;
                    break;
                case ssFighting:
                    break;
                default:
                    bShouldSkip = true;
                    break;
                }
                
                if (bShouldSkip)
                    continue;
                
            }
        }
        else
        {
            // 玩家指针为空,这时候只允许极少数几个协议上来
            if (pHeader->protocolID != c2s_handshake_request && pHeader->protocolID != c2s_ping_signal)
            {
                continue;
            }
        }

	    (this->*m_ProcessProtocolFuns[pHeader->protocolID])(
            (char*)pbyData, (int)uDataLen, nConnIndex, pHeader->frame
        );

        if (pPlayer && (pPlayer->m_nLastClientFrame < pHeader->frame))
        {
            nPackCount = 0;
            pPlayer->m_nLastClientFrame = pHeader->frame;
        }
    }

    bResult = true;
Exit0:
    if (bShutDownFlag)
    {
        Detach(nConnIndex);
        Shutdown(nConnIndex);
        bShutDownFlag = false;
    }
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL KPlayerServer::CheckPackage(BYTE* pbyData, size_t uDataLen)
{
    BOOL                bResult = false;
	KC2S_Header*        pHeader = (KC2S_Header*)pbyData;

    KGLOG_PROCESS_ERROR(uDataLen >= sizeof(KC2S_Header));

    KGLOG_PROCESS_ERROR(pHeader->protocolID > client_gs_connection_begin);

    KGLOG_PROCESS_ERROR(pHeader->protocolID < client_gs_connection_end);

	if (m_nProtocolSize[pHeader->protocolID] != UNDEFINED_PROTOCOL_SIZE)
	{
        KGLOG_PROCESS_ERROR(uDataLen == (size_t)m_nProtocolSize[pHeader->protocolID]);
    }
    
    KGLOG_PROCESS_ERROR(m_ProcessProtocolFuns[pHeader->protocolID]);

    bResult = true;
Exit0:
    if (!bResult)
    {
        KGLogPrintf(KGLOG_ERR, "Error protocol id = %d", pHeader->protocolID);
    }
	return bResult;
}

KBroadcastFunc::KBroadcastFunc()
{
    m_pvData            = NULL;
    m_uSize             = 0;
    m_dwExcept          = ERROR_ID;
    m_dwExclusive       = ERROR_ID;
}

BOOL KBroadcastFunc::operator()(KPlayer* pPlayer)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;

	assert(pPlayer);

    KG_PROCESS_SUCCESS(pPlayer->m_dwID == m_dwExcept);
	KG_PROCESS_SUCCESS(pPlayer->m_nConnIndex == -1);

    if (m_dwExclusive != ERROR_ID && pPlayer->m_dwID != m_dwExclusive)
    {
        goto Exit1;
    }

	g_PlayerServer.Send(pPlayer->m_nConnIndex, m_pvData, m_uSize);

Exit1:
    bResult = true;
Exit0:
	return bResult;
}

BOOL KBroadcastFunc::operator()(KHero* pHero)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int nConnIndex = -1;
    KPlayer* pPlayer = NULL;
   
    KG_PROCESS_SUCCESS(pHero->m_dwID == m_dwExcept);

    KG_PROCESS_SUCCESS(!pHero->IsMainHero());
    
    pPlayer = pHero->GetOwner();
    KG_PROCESS_SUCCESS(!pPlayer);

    nConnIndex = pPlayer->m_nConnIndex;
    KG_PROCESS_SUCCESS(nConnIndex == -1);

    if (m_dwExclusive != ERROR_ID && pHero->m_dwID != m_dwExclusive)
    {
        goto Exit1;
    }

    g_PlayerServer.Send(nConnIndex, m_pvData, m_uSize);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

KGroupTalkFunc::KGroupTalkFunc()
{
}

BOOL KGroupTalkFunc::operator()(KPlayer* pPlayer)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;

    assert(pPlayer);

    if (pPlayer->m_nGroupID == m_nGroupID)
        g_PlayerServer.Send(pPlayer->m_nConnIndex, m_pvData, m_uSize);

    bResult = true;
Exit0:
    return bResult;
}
