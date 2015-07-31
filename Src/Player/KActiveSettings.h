// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KActiveSettings.h
//  Creator		: zhoukezhen 
//	Date		: 12/11/2012
//  Comment		: 
//	*********************************************************************

#pragma once

#include "Engine/KMemory.h"
#include <vector>
#include <map>

struct KActiveSettingsItem
{
    DWORD dwID;
    int nEvent;
    int nFininshValue;
    BOOL bUnLimit;
    int nAwardActive;
};

typedef std::vector<KActiveSettingsItem> KVEC_ACTIVE;
typedef std::vector<DWORD, KMemory::KAllocator<DWORD> > KVEC_ACTIVE_UPDATOR;
typedef std::map<int, KVEC_ACTIVE_UPDATOR*, std::less<int>, KMemory::KAllocator<std::pair<int, KVEC_ACTIVE_UPDATOR*> > > KMAP_ACTIVE_UPDATOR;

class KActiveSettings
{
public:
    KActiveSettings(void);
    ~KActiveSettings(void);

    BOOL Init();
    void UnInit();

    KActiveSettingsItem* GetItem(DWORD dwID);
    int GetActiveCount();
    DWORD GetMaxActiveID();
    BOOL IsValidActive(DWORD dwID);
    KVEC_ACTIVE_UPDATOR* GetActiveByEvent(int nEvent);

private:
    BOOL LoadData();

private:
    KVEC_ACTIVE m_vecActive;
    KMAP_ACTIVE_UPDATOR m_mapValueUpdator;
};

