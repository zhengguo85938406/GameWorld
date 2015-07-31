// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KPlayerValue.h
//  Creator		: hanruofei
//	Date		: 10/28/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include "GlobalEnum.h"
#include "KSO3World.h"

namespace T3DB
{
    class KPB_SAVE_DATA;
}

class KPlayer;

class KPlayerValue
{
public:
    KPlayerValue(void);
    ~KPlayerValue(void);

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL GetValue(DWORD dwID, int& nValue);
    BOOL SetValue(DWORD dwID, int nNewValue);

    void Refresh(time_t nLastDailyRefreshTime);
private:
    BOOL DailyReset();
    BOOL WeeklyReset();
    BOOL MonthlyReset();

    BOOL SetValue(KPlayerValueInfo* pPlayerValueInfo, int nNewValue);

private:
    uint32_t m_Data[MAX_PLAYERVALUE_BYTES / sizeof(uint32_t) + 1];
    KPlayer* m_pPlayer;
};

