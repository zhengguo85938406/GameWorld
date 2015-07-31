// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KPackage.h 
//  Creator 	: Xiayong  
//  Date		: 02/08/2012
//  Comment	: 
// ***************************************************************
#pragma once

struct IItem;
class KPlayer;
class KHero;
namespace T3DB
{
    class KPBPackage;
}

class KPackage
{
public:
    BOOL	Init(int nSize, int nMaxSize, KPlayer* pPlayer, int nPackageType, int nPackageIndex);
    void    UnInit();

    BOOL    LoadFromProtoBuf(const T3DB::KPBPackage* pPackage);
    BOOL    SaveToProtoBuf(T3DB::KPBPackage* pPackage);

    BOOL    SetSize(int nNewSize);
    int     GetSize() { return m_nSize; }
    int     GetMaxSize() { return m_nMaxSize; }
	IItem*  GetItem(int nPos);
	BOOL	PlaceItem(IItem* piItem, int nPos);
	IItem*	PickUpItem(int nPos);
    int     GetFreePos();
    int     GetCanStackPos(IItem* piSrcItem);
    int     CostItem(DWORD dwTabType, DWORD dwIndex, int nAmount);
    BOOL    CostItem(int nPos, int nCount);
    BOOL    DestroyItem(int nPos);
    int     GetItemCount(DWORD dwTabType, DWORD dwIndex);
    int     GetFreeRoomSize();

    BOOL    AbradeItemDurability();
    BOOL    RepairAllEquip();
    int     GetRepairPrice();
    BOOL    CheckItemValidity();
	int		GetMaxStrengthenLevel();
    int     GetAllEquipScore();

private:
	int	        m_nSize;
    int         m_nMaxSize;
	IItem**	    m_ppiItemArray;
    KPlayer*    m_pPlayer;
    int         m_nPackageType;
    int         m_nPackageIndex;
};

