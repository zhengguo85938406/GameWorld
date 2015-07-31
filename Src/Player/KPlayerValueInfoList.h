// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KPlayerValueInfoList.h
//  Creator		: hanruofei
//	Date		: 10/28/2012
//  Comment		: 
//	*********************************************************************
#pragma once

#include <vector>

struct KPlayerValueInfo
{
    DWORD 	dwID;
    int 	nType;
    BOOL 	bIsInUse;
    int 	nSetType;
    int 	nResetType;
    int     nActivityGroup;
    size_t 	uOffset;
};

typedef std::vector<KPlayerValueInfo> KVEC_PLAYERVALUE_INFO;
typedef std::vector<KPlayerValueInfo*> KVEC_DAILY_RESET;
typedef std::vector<KPlayerValueInfo*> KVEC_WEEKLY_RESET;
typedef std::vector<KPlayerValueInfo*> KVEC_MONTHLY_RESET;
typedef std::vector<KPlayerValueInfo*> KVEC_PLAYERVALUE_GROUP;

class KPlayerValueInfoList
{
public:
    KPlayerValueInfoList(void);
    ~KPlayerValueInfoList(void);

    BOOL Init();
    void UnInit();

    KPlayerValueInfo* GetPlayerValue(DWORD dwID);
    size_t GetTotalBytes();
    KVEC_DAILY_RESET& GetDailyResetList();
    KVEC_WEEKLY_RESET& GetWeeklyResetList();
    KVEC_MONTHLY_RESET& GetMonthlyResetList();
    KVEC_PLAYERVALUE_GROUP* GetGroup(int nGroupID);

private:
    BOOL LoadData();
    BOOL GetBitCountOfType(int nType, size_t& uBitCount);
private:
    KVEC_PLAYERVALUE_INFO   m_vecPlayerValueInfo;
    KVEC_DAILY_RESET        m_vecDailyReset;
    KVEC_WEEKLY_RESET       m_vecWeeklyReset;
    KVEC_MONTHLY_RESET      m_vecMonthlyReset;
    size_t                  m_uTotalBytes;
    KVEC_PLAYERVALUE_GROUP              m_PlayerValueGroup[KACTIVITY_GROUP_TOTAL];
};

