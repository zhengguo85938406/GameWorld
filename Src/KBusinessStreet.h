// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBusinessStreet.h 
//  Creator 	: Xiayong  
//  Date		: 09/13/2012
//  Comment	: 
// ***************************************************************
#pragma once

class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

struct KStoreInfo 
{
    uint32_t    dwID;
    KSTORE_TYPE eType;
    int         nLevel;
    int         nGetMoneyTimes;
    time_t      nNextGetMoneyTime;
    int         nState;

    KUPGRADE_TYPE   GetUpgradeNodeEnum(){return (KUPGRADE_TYPE)(eutUpgradeStoreBegin + dwID);};
    bool            CanGetMoney();
};

typedef std::map<uint32_t, KStoreInfo> KMAP_STORE;

class KBusinessStreet
{
public:
    KBusinessStreet();
    ~KBusinessStreet();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    KStoreInfo* GetStore(uint32_t dwID);
    void Activate();

    BOOL BuyLandRequest(int nLandID);
    BOOL BuyLandResult(int nLandID);
    BOOL BuildStoreRequest(int nLandID, KSTORE_TYPE eTargetType);
    BOOL BuildStoreResult(int nLandID, KSTORE_TYPE eTargetType);
    BOOL UpgradeStoreRequest(uint32_t dwStoreID);
    BOOL UpgradeStoreResult(uint32_t dwStoreID);
    BOOL CanGetMoney(uint32_t dwStoreID);
    BOOL GetMoney(uint32_t dwStoreID);
    BOOL GetAllMoney(BOOL bAuto);
    BOOL NotifyGetMoney();

    BOOL ClearGetMoneyCDRequest(uint32_t dwStoreID);
    BOOL ClearGetMoneyCDResult(uint32_t dwStoreID);

    BOOL ClearAllGetMoneyCDRequest();
    BOOL ClearAllGetMoneyCDResult();

    void RefreshGetMoneyTimes();
    BOOL ChangeStoreTypeRequest(uint32_t dwStoreID, KSTORE_TYPE eTargetType);
    BOOL ChangeStoreTypeRespond(uint32_t dwStoreID, KSTORE_TYPE eTargetType);
    BOOL GetLandSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize);
    BOOL GetStoreSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize);
    void OnUpgradeQueueFinished(KUPGRADE_TYPE eType);
    void OnFirstLogin();
    int  GetStoreLevel(uint32_t dwID);
    int  GetStoreCount();

private:
    void CalcNextGetMoneyTime(KStoreInfo* pInfo);
    int  GetNeedCoin(KSTORE_TYPE eType, int nLevel);

private:
    KPlayer*    m_pPlayer;
    std::vector<int> m_vecFreeLandID; 
    int         m_nBuyLandTimes;
    KMAP_STORE  m_mapStore;
    BOOL        m_bNeedNotify;
};
