// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KAwardMgr.h 
//  Creator 	: Xiayong  
//  Date		: 03/19/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "Ksdef/TypeDef.h"
#include "Engine/KMemory.h"
#include <map>

struct KAWARD_ITEM 
{
    DWORD dwID;
    DWORD dwTabType;
    DWORD dwIndex;
    int   nStackNum;
    ENUM_MONEY_TYPE   eMoneyType;
    int   nMoney;
    int   nAwardItemRate;
    int   nQualityLevel;
    int   nMinValuePoint;
    int   nMaxValuePoint;
    DWORD dwBuffID;
    BOOL  bIsBroadcast;
};

typedef KMemory::KAllocator<std::pair<DWORD, KAWARD_ITEM> > KAWARD_ITEM_MAP_ALLOCTOR;
typedef std::map<DWORD, KAWARD_ITEM, std::less<DWORD>, KAWARD_ITEM_MAP_ALLOCTOR> KAWARD_ITEM_MAP;

typedef KMemory::KAllocator<std::pair<int, DWORD> > KAWARD_ITEM_RATE_ALLOCTOR;
typedef std::map<int, DWORD, std::less<int>, KAWARD_ITEM_RATE_ALLOCTOR> KAWARD_ITEM_RATE_MAP;

struct KAwardTable
{
    void            RatePreProcess();
    KAWARD_ITEM*    GetRandomAward();
    KAWARD_ITEM*    GetAwardItem(int nIndex);
    KAWARD_ITEM*    GetOneAwardItemByEqualRate();

    KAWARD_ITEM_MAP         m_mapAwardItem;
    KAWARD_ITEM_RATE_MAP    m_mapAwardItemRate;
    char                    m_szAwardTableName[MAX_PATH];
};

typedef KMemory::KAllocator<std::pair<DWORD, KAwardTable> > KAWARD_TABLE_MAP_ALLOCTOR;
typedef std::map<DWORD, KAwardTable, std::less<DWORD>, KAWARD_TABLE_MAP_ALLOCTOR> KAWARD_TABLE_MAP;

class KPlayer;
class KAwardMgr
{
public:
    KAwardMgr();
    ~KAwardMgr();

    BOOL Init();
    void UnInit();
    BOOL Reload();
    BOOL Award(KPlayer* pPlayer, KAWARD_CARD_TYPE eType, DWORD dwAwardID, unsigned uChooseIndex, BOOL bEndChoose);
    BOOL ScenenItemAward(std::vector<KPlayer*>& vecPlayer, DWORD dwAwardID);
    DWORD ScenenBuffAward(DWORD dwAwardID);
    DWORD GetAwardTableIDByName(const char(&cszFileName)[MAX_PATH]);
    BOOL CanAwardAll(KPlayer* pPlayer, DWORD dwAwardTableID);
    BOOL AwardAll(KPlayer* pPlayer, DWORD dwAwardTableID, const char szWayID[]);
    KAWARD_ITEM*  AwardRandomOne(KPlayer* pPlayer, DWORD dwAwardTableID);
    KAWARD_ITEM* AwardRandomOne(DWORD dwAwardTableID);
    KAWARD_ITEM* GetAwardItem(DWORD dwAwardTableID, int nItemIndex);
    BOOL GetAwardItems(DWORD dwAwardTableID, int& nItemCount, KAWARD_ITEM* pAwardItem[]);

    BOOL HasEnoughBagToAwardAll(KPlayer* pPlayer, DWORD dwAwardTableID);
    BOOL HasEnoughBagToAwardRandomOne(KPlayer* pPlayer, DWORD dwAwardTableID);
    BOOL GetRoomSizeToAwardRandomOne(DWORD dwAwardTableID, int(&RoomSize)[eppiTotal]);
    BOOL GetRoomSizeToAwardAll(DWORD dwAwardTableID, int(&RoomSize)[eppiTotal]);
    BOOL GetAwardItemValuePoint(KAWARD_ITEM* pAwardItem, int& nValuePoint);
    void BroadCastAwardItem(KPlayer* pPlayer, KAWARD_ITEM* pAwardItem);
    void BroadCastAwardMoney(KPlayer* pPlayer, KAWARD_ITEM* pAwardItem);
private:
    BOOL LoadAllAward();
    BOOL LoadOneAward(const char cszAwardFile[], KAwardTable* pAwardTable);
    KAwardTable* GetAwardTable(DWORD dwAwardID);
    KAWARD_TABLE_MAP m_AwardTable;
};
