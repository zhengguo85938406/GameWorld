/************************************************************************/
/* ��Ϸ������ͻ��˵����ӵ�״̬����                                     */
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2005.01.26	Create												*/
/************************************************************************/
#ifndef _KCONNECTION_DATA_H_
#define _KCONNECTION_DATA_H_

class KPlayer;
#include "Common/KG_Socket.h"
#include "Common/KG_Package.h"

struct KConnectionData
{
    KConnectionData() : SocketPacker(3000)
	{
        piSocketStream          = NULL,
        pPlayer                 = NULL;
        dwLastPingTime          = 0;
        bSendError              = false;
        uStatSendByteCount      = 0;
        fStatSendSpeed          = 0.0f;
        nStatLastUpdateFrame    = 0;
    };

    KG_Packer               SocketPacker;
    IKG_SocketStream*       piSocketStream;
	KPlayer*			    pPlayer;
    DWORD                   dwLastPingTime;
    BOOL                    bSendError;
    size_t                  uStatSendByteCount;
    float                   fStatSendSpeed;
    int                     nStatLastUpdateFrame;
};

#endif	//_KCONNECTION_DATA_H_
