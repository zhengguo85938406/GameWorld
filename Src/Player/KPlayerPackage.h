// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KPlayerPackage.h 
//  Creator 	: Xiayong  
//  Date		: 03/31/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KPackage.h"
#include "game_define.h"

class KPlayer;
namespace T3DB
{
    class KPBPlayerPackage;
}

struct KITEM_TAB_INFO 
{
    int nTabType;
    int nTabIndex;
};


class KPlayerPackage
{
public:
    KPlayerPackage();
    ~KPlayerPackage();

    BOOL    Init(KPlayer* pPlayer);
    void    UnInit();

    BOOL    LoadFromProtoBuf(const T3DB::KPBPlayerPackage* pPlayerPackage);
    BOOL    SaveToProtoBuf(T3DB::KPBPlayerPackage* pPlayerPackage);

    IItem*  GetItem(int nPackageIndex, int nPos);
    BOOL    CostItem(int nPackageIndex, int nPos, int nCount);
    IItem*  PickUpItem(int nPackageIndex, int nPos);
    BOOL    PlaceItem(IItem* piItem, int nPackageIndex, int nPos);

    BOOL    CanAddItem(IItem* piItem);
    BOOL    CanAddItems(IItem** piItems, const int nItemNum);
    BOOL    CanAddItems(DWORD dwTabType[], int nTabIndex[], const int nItemNum);
    BOOL    AddItem(IItem* piItem);
    BOOL    CanPutInPackage(IItem* pItem, int nPackageIndex);

    KPackage* GetPackage(int nPackageIndex);
    BOOL    HasFreePos(int nPackageIndex);
    BOOL    CheckItemValidity();
    BOOL    FillShortcutBar();
    
    void    PushReloadInfo(int nPos, int nTabType, int nTabIndex);

private:
    KPackage m_Package[eppiTotal];
    KITEM_TAB_INFO m_ReloadInfo[cdShotcutSize];
    KPlayer* m_pPlayer;
};

