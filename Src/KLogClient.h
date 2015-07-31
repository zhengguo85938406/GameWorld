#ifndef __K_LOG_CLIENT_H_
#define __K_LOG_CLIENT_H_

#include "GS_LG_Protocol.h"

class KPlayer;
class KLogClient
{
public:
    KLogClient();
    ~KLogClient();

    BOOL                    Init();
    void                    UnInit();

    void                    Activate();
    BOOL                    ProcessPackage();

    BOOL                    DoPingSignal();
    BOOL                    DoRecordLog(char* pszLog, size_t uDataLen );
    BOOL                    DoFlowRecord(int nFlowRecordMainType, int nFlowRecordAssistType, const char cszRecordInfomation[], ...);
    BOOL                    DoPlayerLog(const char cszPlayerLogInfo[], ...);
private:
    BOOL                    Send(IKG_Buffer* piBuffer);
    int                     ThreadFunction();
    static void             WorkThreadFunction(void* pvParam); 
    BOOL                    Connect();

    void                    OnHandshakeRespond(BYTE* pbyData, size_t uDataLen);
private:
    IKG_SocketStream*	    m_piSocketStream;
    BOOL                    m_bSocketError;

    typedef void (KLogClient::*PROCESS_PROTOCOL_FUNC)(BYTE* pbyData, size_t uDataLen);
    PROCESS_PROTOCOL_FUNC	m_ProcessProtocolFuns[l2g_protocol_end];
    size_t                  m_uProtocolSize[l2g_protocol_end];

    int                     m_nRelayPort;
    DWORD                   m_dwNextPingTime;
    int                     m_nPingCycle;
    char                    m_szLogServerAddr[16];

    BOOL                    m_bRunFlag;
    KThread                 m_WorkThread;

    time_t                  m_nLastSendPacketTime;
};

extern KLogClient g_LogClient;

#endif
