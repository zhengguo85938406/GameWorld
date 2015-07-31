// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KHeroPackage.h 
//  Creator 	: Xiayong  
//  Date		: 03/14/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KPackage.h"
#include "game_define.h"
#include "Ksdef/TypeDef.h"

class KHero;
namespace T3DB
{
    class KPBHeroPackage;
}

class KHeroPackage
{
public:
    KHeroPackage();
    ~KHeroPackage();

    BOOL    Init(KPlayer* pPlayer, int nPackageIndex);
    void    UnInit();

    BOOL    LoadFromProtoBuf(const T3DB::KPBHeroPackage* pHeroPackage);
    BOOL    SaveToProtoBuf(T3DB::KPBHeroPackage* pHeroPackage);

    int     GetFreeRoomSize() {return m_Package.GetFreeRoomSize();}
    int     GetSize() { return m_Package.GetSize();}
    IItem*  GetItem(int nPos);

    BOOL    CanPlaceItem(IItem* piItem, int nPos);
    BOOL    CheckItemType(IItem* piItem, int nPos);
    BOOL	PlaceItem(IItem* piItem, int nPos);
    IItem*	PickUpItem(int nPos);

    int     GetItemPlacePos(IItem* piItem);
    BOOL    CostItem(DWORD dwTabType, DWORD dwIndex, int nAmount);
    BOOL    CostItem(int nPos, int nCount);

    BOOL    ApplyEquip(KHero* pHero);
    BOOL    UnApplyEquip(KHero* pHero);
    BOOL    AbradeEquip();
    BOOL    RepairAllEquip();
    int     GetRepairPrice();
    BOOL    CheckItemValidity();
	int		GetMaxStrengthenLevel() {return m_Package.GetMaxStrengthenLevel();}
    int     GetAllEquipScore(){return m_Package.GetAllEquipScore();}
    BOOL    GetBinaryDataForClient(uint16_t* pItemLen, BYTE* pbyData, size_t uMaxBufferSize);
private:
    KPackage m_Package;
};

