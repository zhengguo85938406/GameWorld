// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KRoomCache.h 
//  Creator 	: Xiayong  
//  Date		: 04/12/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include <map>
#include "Engine/KMemory.h"

struct KRoomBaseInfo 
{
    DWORD       dwRoomID;
    char        szRoomName[_NAME_LEN];
    DWORD       dwMapID;
    BYTE        byMemberCount;
    bool        bFighting;
    bool        bHasPassword;
};

typedef KMemory::KAllocator<std::pair<DWORD, KRoomBaseInfo> > KROOM_BASEINFO_ALLOCTOR;
typedef std::map<DWORD, KRoomBaseInfo, std::less<DWORD>, KROOM_BASEINFO_ALLOCTOR> KMAP_ROOM_BASEINFO;

class KRoomCache
{
public:
    KRoomCache();
    ~KRoomCache();

    BOOL Init();
    void UnInit();

    KRoomBaseInfo* GetRoomBaseInfo(DWORD dwRoomID);
    BOOL AddRoomBaseInfo(const KRoomBaseInfo& rRoomBaseInfo);
    void DelRoomBaseInfo(DWORD dwRoomID);

    BOOL DoSyncRoomInfo(int nConnIndex, int nPageIndex);

private:
    KMAP_ROOM_BASEINFO m_mapRoomBaseInfo;
};
