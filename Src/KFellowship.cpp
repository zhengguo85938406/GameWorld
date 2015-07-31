#include "stdafx.h"
#include "KGPublic.h"
#include "KSO3World.h"
#include "KFellowship.h"
#include "KLSClient.h"
#include "KPlayerServer.h"
#include "KPlayer.h"

// CLASS: KGFellowshipMgr //////////////////////////////////////////////////
BOOL KGFellowshipMgr::Init()
{
    assert(m_FellowshipMap.empty());
    assert(m_ReverseFellowshipSet.empty());

    m_GroupNamesMap.clear();

    return true;
}

void KGFellowshipMgr::UnInit()
{
    m_FellowshipMap.clear();
    m_ReverseFellowshipSet.clear();

    m_GroupNamesMap.clear();
}

void KGFellowshipMgr::Reset()
{
    m_FellowshipMap.clear();
    m_ReverseFellowshipSet.clear();

    m_GroupNamesMap.clear();
}

struct _GetIDArrayFunc 
{
    _GetIDArrayFunc() { m_dwAlliedPlayerIDArray.reserve(32); };

    BOOL operator ()(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
    {
        m_dwAlliedPlayerIDArray.push_back(dwAlliedPlayerID);
        return true;
    };

    std::vector<DWORD>  m_dwAlliedPlayerIDArray;
};

BOOL KGFellowshipMgr::LoadFellowshipData(DWORD dwPlayerID)
{
    g_LSClient.DoApplyFellowshipDataRequest(dwPlayerID);
    return true;
}

BOOL KGFellowshipMgr::OnLoadFellowshipData(DWORD dwPlayerID, size_t uDataSize, BYTE byData[])
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    KG_FELLOWSHIP_DB*   pFellowshipDB   = NULL;
    KPlayer*            pPlayer         = NULL;

    UnloadPlayerFellowship(dwPlayerID);
    m_GroupNamesMap[dwPlayerID].nGroupCount = 0;

    KG_PROCESS_SUCCESS(!uDataSize); // This player do not have any fellowship data saved before. 

    KGLOG_PROCESS_ERROR(uDataSize > sizeof(KG_FELLOWSHIP_DB));
    pFellowshipDB = (KG_FELLOWSHIP_DB*)byData;

    switch(pFellowshipDB->nVersion)
    {
    case 1:
        bRetCode = OnLoadFellowshipDataV1(dwPlayerID, uDataSize, byData);
        KGLOG_PROCESS_ERROR(bRetCode);
        break;

    default:
        KGLogPrintf(KGLOG_ERR, "Unexcepted fellowship data type: %d.", pFellowshipDB->nVersion);
    }

Exit1:
    m_OnlineIDSet.insert(dwPlayerID);
    bResult = true;
Exit0:
    if (!bResult)
    {
        UnloadPlayerFellowship(dwPlayerID);
    }
    return bResult;
}

BOOL KGFellowshipMgr::OnLoadFellowshipDataV1(DWORD dwPlayerID, size_t uDataSize, BYTE byData[])
{
    BOOL                    bResult         = false;
    BOOL                    bRetCode        = false;
    KG_FELLOWSHIP_DB_V1*    pFellowshipDBv1 = NULL;
    size_t                  uExceptedSize   = 0;
    BYTE*                   pbyData         = NULL;
    KPlayer*                pSelf           = NULL;
    KG_FELLOWSHIP_GROUPNAMES_MAP::iterator it;

    KGLOG_PROCESS_ERROR(uDataSize >= sizeof(KG_FELLOWSHIP_DB_V1));

    pFellowshipDBv1 = (KG_FELLOWSHIP_DB_V1*)byData;

    uExceptedSize  = sizeof(KG_FELLOWSHIP_DB_V1);
    uExceptedSize += pFellowshipDBv1->nFriendCount * sizeof(KG_FELLOWSHIP_ENTRY_V1);
    uExceptedSize += pFellowshipDBv1->nBlackListCount * sizeof(KG_FELLOWSHIP_BLACKLIST_ENTRY_V1);

    KGLOG_PROCESS_ERROR(uDataSize == uExceptedSize);
    //KGLOG_PROCESS_ERROR(pFellowshipDBv1->nFriendCount <= KG_FELLOWSHIP_MAX_RECORD);
    //KGLOG_PROCESS_ERROR(pFellowshipDBv1->nFoeCount <= KG_FELLOWSHIP_MAX_FOE_RECORD);
    //KGLOG_PROCESS_ERROR(pFellowshipDBv1->nBlackListCount <= KG_FELLOWSHIP_MAX_BLACKLIST_RECORD);

    pSelf = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KGLOG_PROCESS_ERROR(pSelf);

    it = m_GroupNamesMap.find(dwPlayerID);
    assert(it != m_GroupNamesMap.end());
    for (int i = 0; i < KG_FELLOWSHIP_MAX_CUSTEM_GROUP; i++)
    {
        if (pFellowshipDBv1->szGroupName[i][0] == '\0')
            break;

        strncpy(it->second.szGroupName[i], pFellowshipDBv1->szGroupName[i], sizeof(it->second.szGroupName[i]));
        it->second.szGroupName[i][sizeof(it->second.szGroupName[i]) - sizeof('\0')] = '\0';

        it->second.nGroupCount++;
    }

    for (int i = it->second.nGroupCount; i < KG_FELLOWSHIP_MAX_CUSTEM_GROUP; i++)
        it->second.szGroupName[i][0] = '\0';

    pbyData = pFellowshipDBv1->byData;

    // Friend
    for (int i = 0; i < pFellowshipDBv1->nFriendCount; i++)
    {
        KGFellowship*           pFellowship = NULL;
        KPlayer*                pPlayer     = NULL;
        KG_FELLOWSHIP_ENTRY_V1* pFellowshipEntry = (KG_FELLOWSHIP_ENTRY_V1*)pbyData;

        pFellowship = AddFellowship(dwPlayerID, pFellowshipEntry->dwAlliedPlayerID, "----", false);
        KGLOG_PROCESS_ERROR(pFellowship);

        pFellowship->m_dwGroupID = pFellowshipEntry->byGroupID;

        strncpy(pFellowship->m_szRemark, pFellowshipEntry->szRemark, sizeof(pFellowship->m_szRemark));
        pFellowship->m_szRemark[sizeof(pFellowship->m_szRemark) - sizeof('\0')] = '\0';

        pbyData += sizeof(KG_FELLOWSHIP_ENTRY_V1);
    }

    // BlackList
    for (int i = 0; i < pFellowshipDBv1->nBlackListCount; i++)
    {
        KGBlackNode*                        pBlackNode      = NULL;
        KG_FELLOWSHIP_BLACKLIST_ENTRY_V1*   pBlackListEntry = (KG_FELLOWSHIP_BLACKLIST_ENTRY_V1*)pbyData;

        pBlackNode = AddBlackList(dwPlayerID, pBlackListEntry->dwAlliedPlayerID, "----", false);
        KGLOG_PROCESS_ERROR(pBlackNode);

        pbyData += sizeof(KG_FELLOWSHIP_BLACKLIST_ENTRY_V1);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL KGFellowshipMgr::SaveFellowshipData(DWORD dwPlayerID)
{
    BOOL                                    bResult         = false;
    BOOL                                    bRetCode        = false;
    KG_FELLOWSHIP_DB_V1*                    pFellowshipDBv1 = NULL;
    IKG_Buffer*                             piBuffer        = NULL;
    int                                     nFriendCount    = CountFellowship(dwPlayerID);
    int                                     nBlackListCount = CountBlackList(dwPlayerID);
    size_t                                  uSize           = 0;
    int                                     nGroupCount     = 0;
    BYTE*                                   pbyData         = NULL;
    _GetIDArrayFunc                         GetIDArrayFunc;
    KG_FELLOWSHIP_ONLINE_ID_SET::iterator   it;
    KG_FELLOWSHIP_GROUPNAMES_MAP::iterator  itGroup;

    it = m_OnlineIDSet.find(dwPlayerID);
    KG_PROCESS_SUCCESS(it == m_OnlineIDSet.end());

    uSize       = sizeof(KG_FELLOWSHIP_DB_V1);
    uSize      += nFriendCount * sizeof(KG_FELLOWSHIP_ENTRY_V1);
    uSize      += nBlackListCount * sizeof(KG_FELLOWSHIP_BLACKLIST_ENTRY_V1);
    piBuffer    = KG_MemoryCreateBuffer((unsigned)uSize);
    KGLOG_PROCESS_ERROR(piBuffer);

    pFellowshipDBv1 = (KG_FELLOWSHIP_DB_V1*)piBuffer->GetData();
    assert(pFellowshipDBv1);

    memset(pFellowshipDBv1, 0, uSize);

    pFellowshipDBv1->nVersion = KG_FELLOWSHIP_DB_CURRENT_DATA_VERSION;
    pFellowshipDBv1->nReserved = 0;

    // Group name
    itGroup = m_GroupNamesMap.find(dwPlayerID);
    if (itGroup != m_GroupNamesMap.end())
        nGroupCount = itGroup->second.nGroupCount;

    for (int i = 0; i < nGroupCount; i++)
    {
        strncpy(pFellowshipDBv1->szGroupName[i], itGroup->second.szGroupName[i], sizeof(pFellowshipDBv1->szGroupName[i]));
        pFellowshipDBv1->szGroupName[i][sizeof(pFellowshipDBv1->szGroupName[i]) - sizeof('\0')] = '\0';
    }

    for (int i = nGroupCount; i < KG_FELLOWSHIP_MAX_CUSTEM_GROUP; i++)
        pFellowshipDBv1->szGroupName[i][0] = '\0';

    pbyData = pFellowshipDBv1->byData;

    // Fellowship
    GetIDArrayFunc.m_dwAlliedPlayerIDArray.clear();
    TraverseFellowshipID(dwPlayerID, GetIDArrayFunc);
    KGLOG_PROCESS_ERROR((int)GetIDArrayFunc.m_dwAlliedPlayerIDArray.size() == nFriendCount);
    pFellowshipDBv1->nFriendCount = nFriendCount;

    for (int i = 0; i < (int)GetIDArrayFunc.m_dwAlliedPlayerIDArray.size(); i++)
    {
        DWORD                       dwAlliedPlayerID    = GetIDArrayFunc.m_dwAlliedPlayerIDArray[i];
        KGFellowship*               pFellowship         = NULL;
        KG_FELLOWSHIP_ENTRY_V1*     pFellowshipEntry    = (KG_FELLOWSHIP_ENTRY_V1*)pbyData;

        pFellowship = GetFellowship(dwPlayerID, dwAlliedPlayerID);
        KGLOG_PROCESS_ERROR(pFellowship);

        pFellowshipEntry->dwAlliedPlayerID  = dwAlliedPlayerID;
        pFellowshipEntry->byGroupID         = (BYTE)pFellowship->m_dwGroupID;

        strncpy(pFellowshipEntry->szRemark, pFellowship->m_szRemark, sizeof(pFellowshipEntry->szRemark));
        pFellowshipEntry->szRemark[sizeof(pFellowshipEntry->szRemark) - sizeof('\0')] = '\0';

        pbyData += sizeof(KG_FELLOWSHIP_ENTRY_V1);
    }

    // BlackList
    GetIDArrayFunc.m_dwAlliedPlayerIDArray.clear();
    TraverseBlackListID(dwPlayerID, GetIDArrayFunc);
    KGLOG_PROCESS_ERROR((int)GetIDArrayFunc.m_dwAlliedPlayerIDArray.size() == nBlackListCount);
    pFellowshipDBv1->nBlackListCount = nBlackListCount;

    for (int i = 0; i < (int)GetIDArrayFunc.m_dwAlliedPlayerIDArray.size(); i++)
    {
        DWORD                               dwAlliedPlayerID    = GetIDArrayFunc.m_dwAlliedPlayerIDArray[i];
        KGBlackNode*                        pBlackNode          = NULL;
        KG_FELLOWSHIP_BLACKLIST_ENTRY_V1*   pBlackListEntry     = (KG_FELLOWSHIP_BLACKLIST_ENTRY_V1*)pbyData;

        pBlackNode = GetBlackListNode(dwPlayerID, dwAlliedPlayerID);
        KGLOG_PROCESS_ERROR(pBlackNode);

        pBlackListEntry->dwAlliedPlayerID  = dwAlliedPlayerID;

        pbyData += sizeof(KG_FELLOWSHIP_BLACKLIST_ENTRY_V1);
    }

    g_LSClient.DoUpdateFellowshipData(dwPlayerID, uSize, (BYTE*)pFellowshipDBv1);

Exit1:
    bResult = true;
Exit0:
    KG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL KGFellowshipMgr::UnloadPlayerFellowship(DWORD dwPlayerID)
{
    BOOL bResult = false;

    assert(dwPlayerID);

    _GetIDArrayFunc UnLoadFunc;

    // ∫√”—
    UnLoadFunc.m_dwAlliedPlayerIDArray.clear();
    TraverseFellowshipID(dwPlayerID, UnLoadFunc);
    for (int i = 0; i < (int)UnLoadFunc.m_dwAlliedPlayerIDArray.size(); i++)
        DelFellowship(dwPlayerID, UnLoadFunc.m_dwAlliedPlayerIDArray[i]);

    // ∫⁄√˚µ•
    UnLoadFunc.m_dwAlliedPlayerIDArray.clear();
    TraverseBlackListID(dwPlayerID, UnLoadFunc);
    for (int i = 0; i < (int)UnLoadFunc.m_dwAlliedPlayerIDArray.size(); i++)
        DelBlackList(dwPlayerID, UnLoadFunc.m_dwAlliedPlayerIDArray[i]);

    m_GroupNamesMap.erase(dwPlayerID);
    m_OnlineIDSet.erase(dwPlayerID);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

KGFellowship* KGFellowshipMgr::GetFellowship(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
{
    KGFellowship* pResult = NULL;
    KG_FELLOWSHIP_MAP::iterator it;

    it = m_FellowshipMap.find(KG_ID_PAIR(dwPlayerID, dwAlliedPlayerID));
    KG_PROCESS_ERROR(it != m_FellowshipMap.end());

    pResult = &(it->second);
Exit0:
    return pResult;
}

KGFellowship* KGFellowshipMgr::AddFellowship(DWORD dwPlayerID, DWORD dwAlliedPlayerID, const char cszAlliedPlayerName[], BOOL bNotifyClient)
{
    int                                     nResult     = KMESSAGE_UNKNOW_ERROR;
    int                                     nRetCode    = false;
    KGFellowship*                           pResult     = NULL;
    KGFellowship*                           pFellowship = NULL;
    KPlayer*                                pPlayer     = NULL;
    std::pair<KG_FELLOWSHIP_MAP_IT, bool>   RetPairRM;
    std::pair<KG_ID_PAIR_SET_IT,  bool>     RetPairRRS;

    assert(dwPlayerID);

    KGLOG_PROCESS_ERROR(cszAlliedPlayerName);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    if (dwAlliedPlayerID == ERROR_ID)
    {
        if (bNotifyClient)
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_FELLOWSHIP_TARGET_NOT_EXIST);
        goto Exit0;
    }

    if (dwAlliedPlayerID == dwPlayerID)
    {
        if (bNotifyClient)
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_FELLOWSHIP_TARGET_CANNOT_BE_SELF);
        goto Exit0;
    }
    
    nRetCode = CountFellowship(dwPlayerID);
    if (nRetCode >= 300)
    {
        if (bNotifyClient)
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_FELLOWSHIP_LIST_IS_FULL);
        goto Exit0;
    }

    // Add relation link, dwPlayerID ---> dwAlliedPlayerID
    RetPairRM = m_FellowshipMap.insert(std::pair<const KG_ID_PAIR, KGFellowship>(KG_ID_PAIR(dwPlayerID, dwAlliedPlayerID), KGFellowship()));
    if (!RetPairRM.second)
    {
        if (bNotifyClient)
        {
            KMessage31_Para param;
            strncpy(param.targetName, cszAlliedPlayerName, sizeof(param.targetName));
            param.targetName[countof(param.targetName) - 1] = '\0';
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_FELLOWSHIP_TARGET_IS_YOUR_FRIEND, &param, sizeof(param));
        }
        goto Exit0;
    }

    // Add reverse relation link, dwPlayerID <--- dwAlliedPlayerID
    RetPairRRS = m_ReverseFellowshipSet.insert(KG_ID_PAIR(dwAlliedPlayerID, dwPlayerID));
    KGLOG_CHECK_ERROR(RetPairRRS.second);

    pFellowship = &((RetPairRM.first)->second);

    strncpy(pFellowship->m_szName, cszAlliedPlayerName, sizeof(pFellowship->m_szName));
    pFellowship->m_szName[sizeof(pFellowship->m_szName) - 1] = '\0';

    g_LSClient.DoGetFellowshipName(dwPlayerID, 1, &dwAlliedPlayerID);
    g_LSClient.DoApplyFellowshipPlayerFellowInfo(dwAlliedPlayerID, bNotifyClient);

    if (bNotifyClient)
    {
        KMessage32_Para param;
        strncpy(param.targetName, cszAlliedPlayerName, sizeof(param.targetName));
        param.targetName[countof(param.targetName) - 1] = '\0';
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_FELLOWSHIP_SUCCESS, &param, sizeof(param));
    }

    pResult = pFellowship;

    g_LSClient.DoRemoteCall("OnAddFriend", (int)dwPlayerID, (int)dwAlliedPlayerID);

Exit0:
    return pResult;
}

BOOL KGFellowshipMgr::DelFellowship(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
{
    BOOL bResult    = false;
    int  nRetCode   = 0;

    // Remove relation link, dwPlayerID -X-> dwAlliedPlayerID
    nRetCode = (int)m_FellowshipMap.erase(KG_ID_PAIR(dwPlayerID, dwAlliedPlayerID));
    KGLOG_PROCESS_ERROR(nRetCode == 1);

    // Remove reverse relation link, dwPlayerID <-X- dwAlliedPlayerID
    nRetCode = (int)m_ReverseFellowshipSet.erase(KG_ID_PAIR(dwAlliedPlayerID, dwPlayerID));
    KGLOG_CHECK_ERROR(nRetCode == 1);
    
Exit1:
    bResult = true;
    g_LSClient.DoRemoteCall("OnDelFriend", (int)dwPlayerID, (int)dwAlliedPlayerID);
Exit0:
    return bResult;
}

struct _FellowshipCount 
{
    _FellowshipCount()
    {
        nCount          = 0;
    }

    BOOL operator () (DWORD dwPlayerID, DWORD dwAlliedPlayerID)
    {
        nCount++; 
        return true; 
    }

    int                 nCount;
};

BOOL KGFellowshipMgr::CountFellowship(DWORD dwPlayerID)
{
    _FellowshipCount Count;

    assert(dwPlayerID);

    TraverseFellowshipID(dwPlayerID, Count);

Exit0:
    return Count.nCount;
}

BOOL KGFellowshipMgr::ClearFellowship(DWORD dwPlayerID)
{
    BOOL            bResult = false;
    _GetIDArrayFunc ClearFunc;

    assert(dwPlayerID);

    TraverseFellowshipID(dwPlayerID, ClearFunc);
    for (int i = 0; i < (int)ClearFunc.m_dwAlliedPlayerIDArray.size(); i++)
        DelFellowship(dwPlayerID, ClearFunc.m_dwAlliedPlayerIDArray[i]);

    m_GroupNamesMap.erase(dwPlayerID);

    bResult = true;
Exit0:
    return bResult;
}

KGBlackNode* KGFellowshipMgr::GetBlackListNode(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
{
    KGBlackNode*            pResult = NULL;
    KG_BLACK_LIST_MAP_IT    it;


    it = m_BlackListMap.find(KG_ID_PAIR(dwPlayerID, dwAlliedPlayerID));
    KG_PROCESS_ERROR(it != m_BlackListMap.end());

    pResult = &(it->second);
Exit0:
    return pResult;
}

KGBlackNode* KGFellowshipMgr::AddBlackList(DWORD dwPlayerID, DWORD dwAlliedPlayerID, const char cszAlliedPlayerName[], BOOL bNotifyClient)
{
    int                                     nRetCode    = false;
    KGBlackNode*                            pResult     = NULL;
    KGBlackNode*                            pBlackNode  = NULL;
    KPlayer*                                pPlayer     = NULL;
    std::pair<KG_BLACK_LIST_MAP_IT, bool>   RetPairRM;
    std::pair<KG_ID_PAIR_SET_IT,  bool>     RetPairRRS;

    assert(dwPlayerID);
    
    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    KGLOG_PROCESS_ERROR(cszAlliedPlayerName);

    if (!dwAlliedPlayerID)
    {
        if (bNotifyClient)
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_BLACKLIST_TARGET_NOT_EXIST);

        goto Exit0;
    }
    
    if (dwAlliedPlayerID == dwPlayerID)
    {
        if (bNotifyClient)
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_BLACKLIST_TARGET_CANNOT_BE_SELF);

        goto Exit0;
    }


    nRetCode = CountBlackList(dwPlayerID);
    if (nRetCode >= 100) 
    {
        if (bNotifyClient)
            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_BLACKLIST_LIST_IS_FULL);

        goto Exit0;
    }

    // Add relation link, dwPlayerID ---> dwAlliedPlayerID
    RetPairRM = m_BlackListMap.insert(std::pair<const KG_ID_PAIR, KGBlackNode>(KG_ID_PAIR(dwPlayerID, dwAlliedPlayerID), KGBlackNode()));
    if (!RetPairRM.second)
    {
        if (bNotifyClient)
        {
            KMessage36_Para param;
            strncpy(param.targetName, cszAlliedPlayerName, countof(param.targetName));
            param.targetName[countof(param.targetName) - 1] = '\0';

            g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_BLACKLIST_TARGET_IS_IN, &param, sizeof(param));
        }
        goto Exit0;
    }

    // Add reverse relation link, dwPlayerID <--- dwAlliedPlayerID
    RetPairRRS = m_ReverseBlackListSet.insert(KG_ID_PAIR(dwAlliedPlayerID, dwPlayerID));
    KGLOG_CHECK_ERROR(RetPairRRS.second);

    pBlackNode = &((RetPairRM.first)->second);

    strncpy(pBlackNode->m_szName, cszAlliedPlayerName, sizeof(pBlackNode->m_szName));
    pBlackNode->m_szName[sizeof(pBlackNode->m_szName) - 1] = '\0';

    g_LSClient.DoGetFellowshipName(dwPlayerID, 1, &dwAlliedPlayerID);

    if (bNotifyClient)
    {
        KMessage37_Para param;
        strncpy(param.targetName, cszAlliedPlayerName, countof(param.targetName));
        param.targetName[countof(param.targetName) - 1] = '\0';

        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_BLACKLIST_SUCCESS, &param, sizeof(param));
    }
    
    g_LSClient.DoRemoteCall("OnAddBlackList", (int)dwPlayerID, (int)dwAlliedPlayerID);

    pResult = pBlackNode;
Exit0:
    return pResult;
}

BOOL KGFellowshipMgr::DelBlackList(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
{
    BOOL bResult    = false;
    int  nRetCode   = 0;

    // Remove relation link, dwPlayerID -X-> dwAlliedPlayerID
    nRetCode = (int)m_BlackListMap.erase(KG_ID_PAIR(dwPlayerID, dwAlliedPlayerID));
    KG_PROCESS_ERROR(nRetCode == 1);

    // Remove reverse relation link, dwPlayerID <-X- dwAlliedPlayerID
    m_ReverseBlackListSet.erase(KG_ID_PAIR(dwAlliedPlayerID, dwPlayerID));

    g_LSClient.DoRemoteCall("OnDelBlackList", (int)dwPlayerID, (int)dwAlliedPlayerID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KGFellowshipMgr::CountBlackList(DWORD dwPlayerID)
{
    _FellowshipCount Count;

    assert(dwPlayerID);

    TraverseBlackListID(dwPlayerID, Count);
Exit0:
    return Count.nCount;
}

BOOL KGFellowshipMgr::ClearBlackList(DWORD dwPlayerID)
{
    BOOL            bResult = false;
    _GetIDArrayFunc ClearFunc;

    assert(dwPlayerID);

    TraverseBlackListID(dwPlayerID, ClearFunc);
    for (int i = 0; i < (int)ClearFunc.m_dwAlliedPlayerIDArray.size(); i++)
        DelBlackList(dwPlayerID, ClearFunc.m_dwAlliedPlayerIDArray[i]);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KGFellowshipMgr::AddFellowshipGroup(DWORD dwPlayerID, const char cszGroupName[])
{
    BOOL        bResult = false;
    KPlayer*    pPlayer = NULL;
    KG_FELLOWSHIP_GROUPNAMES_MAP::iterator  it;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    it = m_GroupNamesMap.find(dwPlayerID);
    if (it == m_GroupNamesMap.end())
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_INNER_ERROR);
        goto Exit0;
    }

    if (it->second.nGroupCount >= KG_FELLOWSHIP_MAX_CUSTEM_GROUP)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_GROUP_COUNT_OUTOF_LIMIT);
        goto Exit0;
    }

    strncpy(
        it->second.szGroupName[it->second.nGroupCount], 
        cszGroupName, 
        sizeof(it->second.szGroupName[it->second.nGroupCount])
    );
    it->second.szGroupName[it->second.nGroupCount][sizeof(it->second.szGroupName[it->second.nGroupCount]) - sizeof('\0')] = '\0';

    it->second.nGroupCount += 1;

    g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_ADD_GROUP_SUCCESS);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KGFellowshipMgr::DelFellowshipGroup(DWORD dwPlayerID, DWORD dwGroupID)
{
    BOOL                                    bResult     = false;
    KPlayer*                                pPlayer     = NULL;
    KGFellowship*                           pFellowship = NULL;
    GetFellowshipIDListFunc                 TraverseIDFunc;
    KG_FELLOWSHIP_GROUPNAMES_MAP::iterator  it;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    it = m_GroupNamesMap.find(dwPlayerID);
    if (it == m_GroupNamesMap.end())
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_INNER_ERROR);
        goto Exit0;
    }

    if (dwGroupID >= (DWORD)it->second.nGroupCount)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_DEL_GROUP_TARGET_NOT_FOUND);
        goto Exit0;
    }
    
    for (int i = (int)dwGroupID; i < it->second.nGroupCount - 1; i++)
    {
        strncpy(
            it->second.szGroupName[i], 
            it->second.szGroupName[i + 1], 
            sizeof(it->second.szGroupName[i])
        );
        it->second.szGroupName[i][sizeof(it->second.szGroupName[i]) - sizeof('\0')] = '\0';

        it->second.szGroupName[i + 1][0] = '\0';
    }

    TraverseFellowshipID(dwPlayerID, TraverseIDFunc);
    for (int nIndex = 0; nIndex < (int)TraverseIDFunc.m_dwIDList.size(); nIndex++)
    {
        pFellowship = GetFellowship(dwPlayerID, TraverseIDFunc.m_dwIDList[nIndex]);
        assert(pFellowship);

        if (pFellowship->m_dwGroupID == dwGroupID + 1)
        {
            pFellowship->m_dwGroupID = 0;
        }

        if (pFellowship->m_dwGroupID > dwGroupID + 1)
        {
            assert(pFellowship->m_dwGroupID != 0);
            pFellowship->m_dwGroupID--;
        }
    }

    it->second.nGroupCount--;

    g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_DEL_GROUP_SUCCESS);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KGFellowshipMgr::RenameFellowshipGroup(DWORD dwPlayerID, DWORD dwGroupID, const char cszGroupName[])
{
    BOOL                                    bResult = false;
    KPlayer*                                pPlayer = NULL;
    KG_FELLOWSHIP_GROUPNAMES_MAP::iterator  it;

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    it = m_GroupNamesMap.find(dwPlayerID);
    if (it == m_GroupNamesMap.end())
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_RENAME_GROUP_FAILED);
        goto Exit0;
    }

    if (dwGroupID >= (DWORD)it->second.nGroupCount)
    {
        g_PlayerServer.DoDownwardNotify(pPlayer, KMESSAGE_CANNOT_FIND_THE_GROUP);
        goto Exit0;
    }

    strncpy(it->second.szGroupName[dwGroupID], cszGroupName, sizeof(it->second.szGroupName[dwGroupID]));
    it->second.szGroupName[dwGroupID][sizeof(it->second.szGroupName[dwGroupID]) - sizeof('\0')] = '\0';

    bResult = true;
Exit0:
    return bResult;
}


BOOL GetFellowshipIDListFunc::operator()(DWORD dwPlayerID, DWORD dwAlliedPlayerID)
{
    m_dwIDList.push_back(dwAlliedPlayerID);

    return true;
}
