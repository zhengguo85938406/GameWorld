#include "stdafx.h"
#include "GS_LG_Protocol.h"
#include "KLogClient.h"
#include "KSO3World.h"
#include "KPlayer.h"

#define LOG_SERVER_SECTION_GAMESERVER   "GS-LOG"

KLogClient g_LogClient;

KLogClient::KLogClient()
{
    m_piSocketStream        = NULL;
    m_bSocketError          = false;
    m_nPingCycle            = 0;
    m_dwNextPingTime        = 0;

    memset(m_ProcessProtocolFuns, 0, sizeof(m_ProcessProtocolFuns));
    memset(m_uProtocolSize, 0, sizeof(m_uProtocolSize));

    REGISTER_INTERNAL_FUNC(l2g_handshake_respond, &KLogClient::OnHandshakeRespond, sizeof(L2G_HANDSHAKE_RESPOND));
}

KLogClient::~KLogClient()
{
}

BOOL KLogClient::Init()
{
    BOOL               bResult          = false;
    BOOL               bRetCode         = false;
    IIniFile*	       piIniFile        = NULL;

    piIniFile = g_OpenIniFile(GS_SETTINGS_FILENAME);
    KGLOG_PROCESS_ERROR(piIniFile);

    bRetCode = piIniFile->GetString(LOG_SERVER_SECTION_GAMESERVER, "IP", "127.0.0.1", m_szLogServerAddr, sizeof(m_szLogServerAddr));
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piIniFile->GetInteger(LOG_SERVER_SECTION_GAMESERVER, "Port", 5004, &m_nRelayPort);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = piIniFile->GetInteger(LOG_SERVER_SECTION_GAMESERVER, "PingCycle", 20, &m_nPingCycle);
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

void KLogClient::WorkThreadFunction(void* pvParam)
{
    KLogClient* pThis = (KLogClient*)pvParam;

    assert(pThis);

    pThis->ThreadFunction();
}

BOOL KLogClient::Connect()
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    KG_SocketConnector Connector;
    struct timeval     TimeVal;
    IKG_SocketStream*	    piSocketStream = NULL;
    IKG_Buffer*             piPackage       = NULL;
    G2L_HANDSHAKE_REQUEST*  pHandshake      = NULL;

    piSocketStream = Connector.Connect(m_szLogServerAddr, m_nRelayPort);
    KG_PROCESS_ERROR(piSocketStream);

    TimeVal.tv_sec  = 0;
    TimeVal.tv_usec = 10000;

    bRetCode = piSocketStream->SetTimeout(&TimeVal);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_bSocketError        = false;
    //m_nWorldIndex         = 0;

    m_nLastSendPacketTime = g_pSO3World->m_nCurrentTime;

    // 初始化的一些操作，注意多线程
    piPackage = KG_MemoryCreateBuffer((unsigned)sizeof(G2L_HANDSHAKE_REQUEST));
    KGLOG_PROCESS_ERROR(piPackage);

    pHandshake = (G2L_HANDSHAKE_REQUEST*)piPackage->GetData();
    KGLOG_PROCESS_ERROR(pHandshake);

    pHandshake->wProtocolID         = g2l_handshake_request;
    pHandshake->nServerTime         = (int)time(NULL);
    pHandshake->nServerIndexInGC    = g_pSO3World->m_nServerIndexInGC;

    bRetCode = piSocketStream->Send(piPackage);
    KGLOG_PROCESS_ERROR(bRetCode == 1);

    // 小心: 这里不能写成 "m_piSocket = piSocket; m_piSocket->AddRef(); ", 那样会导致线程安全隐患
    piSocketStream->AddRef();
    m_piSocketStream = piSocketStream;

    KGLogPrintf( KGLOG_INFO, "Connect to log server %s:%d ... ... [OK]", m_szLogServerAddr, m_nRelayPort );

    bResult = true;
Exit0:
    KG_COM_RELEASE(piPackage);
    KG_COM_RELEASE(piSocketStream);
    return bResult;
}

void KLogClient::OnHandshakeRespond(BYTE* pbyData, size_t uDataLen)
{
    return;
}

BOOL KLogClient::DoPingSignal()
{
    BOOL				bResult			= false;
    BOOL				bRetCode		= false;
    DWORD               dwNowTime       = 0;
    IKG_Buffer*         piSendBuffer    = NULL;
    G2L_PING_SIGNAL*    pPingSingal		= NULL;

    dwNowTime = KG_GetTickCount();

    KG_PROCESS_ERROR(dwNowTime >= m_dwNextPingTime);

    m_dwNextPingTime = dwNowTime + 1000 * m_nPingCycle;

    piSendBuffer = KG_MemoryCreateBuffer(sizeof(G2L_PING_SIGNAL));
    KGLOG_PROCESS_ERROR(piSendBuffer);

    pPingSingal = (G2L_PING_SIGNAL*)piSendBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPingSingal);

    pPingSingal->wProtocolID = g2l_ping_signal;

    bRetCode = Send(piSendBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);   

    bResult = true;
Exit0:
    KG_COM_RELEASE(piSendBuffer);
    return bResult;
}

BOOL KLogClient::DoFlowRecord(int nFlowRecordMainType, int nFlowRecordAssistType, const char cszRecordInfomation[], ...)
{
    BOOL    bResult	        = false;
    BOOL    bRetCode        = false;
    int     nBufferLen      = 0;
    char    szBuffer[1024]  = {'\0'};
    IKG_Buffer* piBuffer    = NULL;
    G2L_FLOW_RECORD* pPak   = NULL;

    va_list marker;
    va_start(marker, cszRecordInfomation);
    bRetCode = vsnprintf(szBuffer + nBufferLen, sizeof(szBuffer) - 1 - nBufferLen, cszRecordInfomation, marker);
    va_end(marker);
    KGLOG_PROCESS_ERROR(bRetCode != -1);
    szBuffer[sizeof(szBuffer) - 1] = '\0';
    nBufferLen += bRetCode;
	
	KGLOG_PROCESS_ERROR(nBufferLen < sizeof(szBuffer));
	szBuffer[nBufferLen] = '\0';
    ++nBufferLen;

    piBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(G2L_FLOW_RECORD) + nBufferLen);
    KGLOG_PROCESS_ERROR(piBuffer);
 
    pPak = (G2L_FLOW_RECORD*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);
 
    pPak->wProtocolID           = g2l_flow_record;
    pPak->wFlowRecordMainType   = (WORD)nFlowRecordMainType;
    pPak->wFlowRecordAssistType = (WORD)nFlowRecordAssistType;
    pPak->wInfomationLength     = (WORD)nBufferLen;
 
    memcpy(pPak->byInfomation, szBuffer, nBufferLen);
 
    bRetCode = Send(piBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL KLogClient::DoPlayerLog(const char cszPlayerLogInfo[], ...)
{
    BOOL    bResult	        = false;
    BOOL    bRetCode        = false;
    int     nBufferLen      = 0;
    char    szBuffer[1024]  = {'\0'};
    IKG_Buffer* piBuffer    = NULL;
    G2L_OLD_PLAYER_LOG* pPak   = NULL;

    va_list marker;
    va_start(marker, cszPlayerLogInfo);
    bRetCode = vsnprintf(szBuffer + nBufferLen, sizeof(szBuffer) - 1 - nBufferLen, cszPlayerLogInfo, marker);
    va_end(marker);
    KGLOG_PROCESS_ERROR(bRetCode != -1);
    szBuffer[sizeof(szBuffer) - 1] = '\0';
    nBufferLen += bRetCode;

    KGLOG_PROCESS_ERROR(nBufferLen < sizeof(szBuffer));
    szBuffer[nBufferLen] = '\0';
    ++nBufferLen;

    piBuffer = KG_MemoryCreateBuffer((unsigned)sizeof(G2L_OLD_PLAYER_LOG) + nBufferLen);
    KGLOG_PROCESS_ERROR(piBuffer);

    pPak = (G2L_OLD_PLAYER_LOG*)piBuffer->GetData();
    KGLOG_PROCESS_ERROR(pPak);

    pPak->wProtocolID           = g2l_player_log;
    pPak->wInfomationLength     = (WORD)nBufferLen;

    memcpy(pPak->byInfomation, szBuffer, nBufferLen);

    bRetCode = Send(piBuffer);
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult	= true;
Exit0:
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

int KLogClient::ThreadFunction()
{
    while (m_bRunFlag)
    {
        KGThread_Sleep(1000);

        if (m_piSocketStream != NULL)
            continue;

        Connect();
    }

    return 0;
}


void KLogClient::UnInit(void)
{
    m_bRunFlag = false;
    m_WorkThread.Destroy();
    KG_COM_RELEASE(m_piSocketStream);
}

void KLogClient::Activate()
{
    // 临时测试用的代码，如果见到了，请勿删除
    // ...
    //static DWORD s_dwLastRecordTime = 0;
    //DWORD   dwCurrentTime   = KG_GetTickCount();

    //if( s_dwLastRecordTime > dwCurrentTime )
    //    return;

    //char    szLog[512];
    //sprintf( szLog, "KLogClient::Activate\n" );
    //size_t uDataLen    = strlen(szLog);
    //g_LogClient.DoRecordLog( szLog, uDataLen );

    //s_dwLastRecordTime  = dwCurrentTime + 2000;
}

BOOL KLogClient::ProcessPackage()
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

        KGLOG_PROCESS_ERROR(pHeader->wProtocolID < l2g_protocol_end);

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
        KGLogPrintf(KGLOG_ERR, "Log server connection lost!\n");
        KG_COM_RELEASE(m_piSocketStream);
    }
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL KLogClient::Send(IKG_Buffer* piBuffer)
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

    assert(pHeader->wProtocolID > g2l_protocol_begin);
    assert(pHeader->wProtocolID < g2l_protocol_end);

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
