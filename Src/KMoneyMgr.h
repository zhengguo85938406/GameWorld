// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KMoneyMgr.h 
//  Creator 	: Xiayong  
//  Date		: 03/16/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "GlobalEnum.h"

class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

class KMoneyMgr
{
public:
    KMoneyMgr();
    ~KMoneyMgr();

    BOOL    Init(KPlayer* pPlayer);
    void    UnInit();

    BOOL    LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL    SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL    CanAddMoney(ENUM_MONEY_TYPE eMoneyType, int nAddMoney);
    BOOL    AddMoney(ENUM_MONEY_TYPE eMoneyType, int nAddMoney);
    int     GetMoney(ENUM_MONEY_TYPE eMoneyType);

    BOOL    UpgradeSafeBox();
    void    RefreshGameMoneyLimit();
    int     GetGameMoneyLimit();
    int     GetSafeBoxLevel(){return m_nSafeBoxLevel;};

    int     GetPayCoin(){return m_nMoneys[emotCoin];};
    int     GetFreeCoin(){return m_nFreeCoin;};
    int     AddFreeCoin(int nAddFreeCoin);
    
private:
    int         m_nMoneys[emotTotal];
    KPlayer*    m_pPlayer;

    int         m_nFreeCoin;    // 系统赠送的点卷
    int         m_nSafeBoxLevel;
};
