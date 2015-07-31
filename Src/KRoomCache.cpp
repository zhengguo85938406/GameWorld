#include "stdafx.h"
#include "KRoomCache.h"
#include "KPlayerServer.h"

KRoomCache::KRoomCache()
{
}

KRoomCache::~KRoomCache()
{
}

BOOL KRoomCache::Init()
{
    return true;
}

void KRoomCache::UnInit()
{
}

KRoomBaseInfo* KRoomCache::GetRoomBaseInfo(DWORD dwRoomID)
{
    KRoomBaseInfo* pResult = NULL;
    KMAP_ROOM_BASEINFO::iterator it;

    it = m_mapRoomBaseInfo.find(dwRoomID);
    KG_PROCESS_ERROR(it != m_mapRoomBaseInfo.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

BOOL KRoomCache::AddRoomBaseInfo(const KRoomBaseInfo& rRoomBaseInfo)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    std::pair<KMAP_ROOM_BASEINFO::iterator, bool> InsRet;

    InsRet = m_mapRoomBaseInfo.insert(std::make_pair(rRoomBaseInfo.dwRoomID, rRoomBaseInfo));
    KGLOG_PROCESS_ERROR(InsRet.second);

	bResult = true;
Exit0:
	return bResult;
}

void KRoomCache::DelRoomBaseInfo(DWORD dwRoomID)
{
    m_mapRoomBaseInfo.erase(dwRoomID);
}

BOOL KRoomCache::DoSyncRoomInfo(int nConnIndex, int nPageIndex)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false;
    int  nValidIndex = -1;
    unsigned  uTotalCount = 0;
    unsigned  uTotalPage = 0;

    g_PlayerServer.DoSyncRoomInfoStart(nConnIndex, nPageIndex);

    for (KMAP_ROOM_BASEINFO::iterator it = m_mapRoomBaseInfo.begin(); it != m_mapRoomBaseInfo.end(); ++it)
    {
        KRoomBaseInfo& rBaseInfo = it->second;

        ++nValidIndex;
        if (nValidIndex < (nPageIndex - 1) * cdRoomNumberPerPage)
            continue;

        if (nValidIndex >= nPageIndex * cdRoomNumberPerPage)
            break;

        g_PlayerServer.DoSyncRoomBaseInfo(
            nConnIndex, nPageIndex, rBaseInfo.dwRoomID, rBaseInfo.szRoomName,
            rBaseInfo.dwMapID, rBaseInfo.byMemberCount, rBaseInfo.bHasPassword, rBaseInfo.bFighting
        );
    }

    uTotalCount = (unsigned)m_mapRoomBaseInfo.size();
    uTotalPage = uTotalCount / cdRoomNumberPerPage;   
    if (uTotalCount % cdRoomNumberPerPage)
        ++uTotalPage;
   
    if (uTotalPage == 0)
        uTotalPage = 1;

    g_PlayerServer.DoSyncRoomInfoEnd(nConnIndex, uTotalPage);

    bResult = true;
//Exit0:
    return bResult;
}
