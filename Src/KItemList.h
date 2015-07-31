// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KItemList.h 
//  Creator 	: Xiayong  
//  Date		: 02/08/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KPackage.h"
#include "KHeroPackage.h"
#include "KPlayerPackage.h"
#include "Engine/KMemory.h"
#include <map>

struct KITEMPOS
{
    int nPackageType;
    int nPackageIndex;
    int nPos;
};

class KPlayer;
struct KGItemInfo;
struct KDecomposeItems;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

class KItemList
{
public:
    KItemList();
    ~KItemList();

    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL    CanAddPlayerItem(IItem* piItem);
    BOOL    CanAddPlayerItems(IItem** piItems, const int nItemNum);
    BOOL    AddPlayerItem(IItem* piItem);
    BOOL    AddPlayerItemToPos(IItem* piItem, int nPos);
    IItem*  PickUpPlayerItem(int nPackageIndex, int nPos);
    IItem*  GetPlayerItem(int nPackageIndex, int nPos);

    BOOL PlaceItemToHeroPackage(DWORD dwHeroTemplateID, IItem* piItem);
    IItem* PickUpHeroItem(DWORD dwTemplateID, int nPos);
    IItem* GetHeroItem(DWORD dwHeroTemplateID, int nPos);

    KHeroPackage* GetHeroPackage(DWORD dwHeroTemplateID);

    KHeroPackage* AddHeroPackage(DWORD dwHeroTemplateID);
    void RemoveHeroPackage(DWORD dwHeroTemplateID);

    BOOL    CostPlayerItem(DWORD dwTabtype, DWORD dwIndex, int nAmount);

    BOOL    CostItem(int nPackageType, int nPackageIndex, int nPos, int nCount);

    KPLAYER_PACKAGE_EXCHANGE_RESULT CanPlaceShortcutBarItem(
        int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
        int nDestPackageType, int nDestPackageIndex, int nDestPos
    );

    BOOL    PlaceShortcutBarItem(
        IItem* pSrcItem, int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
        IItem* pDestItem, int nDestPackageType, int nDestPackageIndex, int nDestPos
    );

    KPLAYER_PACKAGE_EXCHANGE_RESULT CanExchangeItem(
        int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
        int nDestPackageType, int nDestPackageIndex, int nDestPos
    );
    BOOL    ExchangeItem(     
        int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
        int nDestPackageType, int nDestPackageIndex, int nDestPos
    );

    IItem*  GetItem(int nPackageType, int nPackageIndex, int nPos);
    IItem*  PickUpItem(int nPackageType, int nPackageIndex, int nPos);
    BOOL    PlaceItem(IItem* pItem, int nPackageType, int nPackageIndex, int nPos);
    
    BOOL    CanUseItem(KGItemInfo* pItemInfo);
    BOOL    UseItem(int nPackageType, int nPackageIndex, int nPos);
    BOOL    DestroyItem(int nPackageType, int nPackageIndex, int nPos);
    BOOL    CostOneStackItem(int nPackageType, int nPackageIndex, int nPos);
    int     GetPlayerFreeRoomSize(int nPackageIndex);
    int     GetPlayerItemCount(DWORD dwTabType, DWORD dwIndex);

    KPackage* GetPlayerPackage(int nPackageIndex);

    BOOL    CutStackItem(
        int nSrcPackageType, int nSrcPackageIndex, int nSrcPos, 
        int nDestPackageType, int nDestPackageIndex, int nDestPos, int nCutNum
    );

    BOOL HasFreePosAddItem(DWORD dwTabType, DWORD dwIndex);
    BOOL AbradeEquip(KHero* pHero);
    BOOL RepairAllHeroEquip();
    int  GetRepairTotalPrice();

    BOOL CheckItemValidity();
    BOOL HasItem(DWORD dwTabType, DWORD dwTabIndex, int nCount);

    BOOL HasFreeRoom(int (&RoomSize)[eppiTotal]);
    BOOL DecomposeEquip(size_t nItemCount, KDecomposeItems* pItems);
    BOOL DecomposeOneEquip(int nPackageType, int nPackageIndex, int nPos, std::vector<IItem*>& vecDecomposeResult);
    BOOL IsValidPos(int nPackageType, int nPackageIndex, int nPos);

	int GetAllHeroEquipScore();
	int GetHeroEquipScore(DWORD dwHeroTemplateID);
	int GetMaxStrengthenLevel();

    KPlayerPackage m_PlayerPackage;

    typedef KMemory::KAllocator<std::pair<DWORD, KHeroPackage> > KHERO_PACKAGE_ALLOCTOR;
    typedef std::map<DWORD, KHeroPackage, std::less<DWORD>, KHERO_PACKAGE_ALLOCTOR> KMAP_HERO_PACKAGE;

    KMAP_HERO_PACKAGE m_mapHeroPackage;

    KPlayer* m_pPlayer;
};
