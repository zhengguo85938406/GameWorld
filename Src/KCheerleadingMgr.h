// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KCheerleading.h
//  Creator		: hanruofei
//	Date		: 9/21/2012
//  Comment		: 
//	*********************************************************************

#pragma once
#include "GlobalStruct.h"
#include "Protocol/gs2cl_protocol.h"
#include <map>

class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

struct KCHEERLEADING_ITEM
{
    DWORD   dwIndex;
    DWORD   dwID;
    int     nEndTime;
};

struct KCHEERLEADING_SLOTINFO
{
    DWORD dwCheerleadingIndex;
    int nOffsetX;
    int nFaceTo;
};

typedef std::map<DWORD, KCHEERLEADING_ITEM> KMAP_CHEERLEADING;

class KCheerleadingMgr
{
public:
    KCheerleadingMgr(void);
    ~KCheerleadingMgr(void);

    BOOL Init(KPlayer* pOwner);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);
    BOOL BuySlot(DWORD dwSlotIndex);
    BOOL AddSlot(DWORD dwSlotIndex);
    BOOL OnBuySlotSuccess(DWORD dwSlotIndex);
    BOOL UpdateCheerleadingSlotInfo(const KCHEERLEADING_SLOTINFO (&slotInfo)[cdMaxCheerleadingSlotCount]);
    BOOL AddCheerleading(DWORD dwCheerleadingID);
    BOOL OnRechargeCheerleadingItemRequest(DWORD dwCheerleadingItemIndex, int nLifeType);
    BOOL OnRechargeCheerleadingItemSuccess(DWORD dwCheerleadingItemIndex, int nLifeType);

    BOOL GetSlotInfoInBattle(KS2C_Cheerleading_SlotInfo_In_Battle (&slotInfoInBattle)[cdMaxCheerleadingSlotCount]);

    void Activate();

    DWORD                       m_uBuySlotTimes;
    DWORD                       m_uSlotCount;
    KCHEERLEADING_SLOTINFO      m_SlotInfo[cdMaxCheerleadingSlotCount];
    KMAP_CHEERLEADING           m_mapCheerleadingItems;

private:
    KCHEERLEADING_ITEM* GetCheerleadingItem(DWORD dwCheerleadingIndex);
    void ResetSlotWhichCheerleadingItemIn(DWORD dwCheerleadingItemIndex);

private:
    KPlayer* m_pOwner;
    DWORD    m_dwNextIndex;
};

