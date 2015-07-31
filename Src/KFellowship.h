#ifndef _KFELLOWSHIP_H_
#define _KFELLOWSHIP_H_

#include <map>
#include <set>
#include <deque>
#include <vector>
#include <algorithm>
#include "KFellowShipDef.h"
#include "SO3GlobalDef.h"

#define KG_FELLOWSHIP_DB_CURRENT_DATA_VERSION   1

//////////////////////////////////////////////////////////////////////////

class KPlayer;

struct KGFellowship
{
    int                     m_nPlayerLevel;   // 好友的等级
    char                    m_szName[_NAME_LEN];
    char                    m_szRemark[_NAME_LEN];
    DWORD                   m_dwGroupID;
    BOOL                    m_bOnline;
    int                     m_nVIPLevel;
    KGENDER                 m_eGender;
    int                     m_nLadderLevel;
    int                     m_nTeamLogo;
    int                     m_nTeamLogoBg;
    BOOL                    m_bIsVIP;
    int                     m_nServerID;
    KGFellowship()
    {
        m_nServerID = 0;
        m_nPlayerLevel  = 0;
        m_szName[0]     = '\0';
        m_szRemark[0]   = '\0';
        m_dwGroupID     = cdInvalidFelllowshipGroupID;
        m_bOnline       = false;
        m_nVIPLevel     = 0;
        m_bIsVIP        = false;
        m_eGender       = gNone;
        m_nLadderLevel  =  0;
        m_nTeamLogo     = 1;
        m_nTeamLogoBg     = 1;
    };
};

struct KGBlackNode  
{
    char                    m_szName[_NAME_LEN];

    KGBlackNode()
    {
        m_szName[0]     = '\0';
    }
};

struct KGFellowshipGroupList 
{
    int     nGroupCount;
    char    szGroupName[KG_FELLOWSHIP_MAX_CUSTEM_GROUP][_NAME_LEN];
};

typedef std::pair<DWORD, DWORD>                                     KG_ID_PAIR;

typedef std::map<KG_ID_PAIR, KGFellowship, std::less<KG_ID_PAIR> >       KG_FELLOWSHIP_MAP;
typedef KG_FELLOWSHIP_MAP::iterator                                 KG_FELLOWSHIP_MAP_IT;

typedef std::map<KG_ID_PAIR, KGBlackNode, std::less<KG_ID_PAIR> >        KG_BLACK_LIST_MAP;
typedef KG_BLACK_LIST_MAP::iterator                                 KG_BLACK_LIST_MAP_IT;

typedef std::set<KG_ID_PAIR, std::less<KG_ID_PAIR> >                     KG_ID_PAIR_SET;
typedef KG_ID_PAIR_SET::iterator                                    KG_ID_PAIR_SET_IT;

typedef std::map<DWORD, KGFellowshipGroupList>                      KG_FELLOWSHIP_GROUPNAMES_MAP;
typedef std::set<DWORD>                                             KG_FELLOWSHIP_ONLINE_ID_SET;

class KGFellowshipMgr
{
public:
    KGFellowshipMgr() {};
    ~KGFellowshipMgr() {};

    BOOL            Init();
    void            UnInit();

    void            Reset();

    BOOL            LoadFellowshipData(DWORD dwPlayerID);   // 从 GameCenter 加载好友数据。
    BOOL            OnLoadFellowshipData(DWORD dwPlayerID, size_t uDataSize, BYTE byData[]);
    BOOL            OnLoadFellowshipDataV1(DWORD dwPlayerID, size_t uDataSize, BYTE byData[]);

    BOOL            SaveFellowshipData(DWORD dwPlayerID);   // 将好友数据交给 GameCenter 保存。

    BOOL            UnloadPlayerFellowship(DWORD dwPlayerID);

    KGFellowship*   GetFellowship(DWORD dwPlayerID, DWORD dwAlliedPlayerID);

    KGFellowship*   AddFellowship(DWORD dwPlayerID, DWORD dwAlliedPlayerID, const char cszAlliedPlayerName[], BOOL bNotifyClient);
    BOOL            DelFellowship(DWORD dwPlayerID, DWORD dwAlliedPlayerID);
    int             CountFellowship(DWORD dwPlayerID);
    BOOL            ClearFellowship(DWORD dwPlayerID);

    KGBlackNode*    GetBlackListNode(DWORD dwPlayerID, DWORD dwAlliedPlayerID);

    KGBlackNode*    AddBlackList(DWORD dwPlayerID, DWORD dwAlliedPlayerID, const char cszAlliedPlayerName[], BOOL bNotifyClient);
    BOOL            DelBlackList(DWORD dwPlayerID, DWORD dwAlliedPlayerID);
    int             CountBlackList(DWORD dwPlayerID);
    BOOL            ClearBlackList(DWORD dwPlayerID);

    BOOL            AddFellowshipGroup(DWORD dwPlayerID, const char cszGroupName[]);
    BOOL            DelFellowshipGroup(DWORD dwPlayerID, DWORD dwGroupID);
    BOOL            RenameFellowshipGroup(DWORD dwPlayerID, DWORD dwGroupID, const char cszGroupName[]);

    template <class TFunc>
    BOOL            TraverseFellowshipID(DWORD dwPlayerID, TFunc& Func);

    template <class TFunc>
    BOOL            TraverseBlackListID(DWORD dwPlayerID, TFunc& Func);

    template <class TFunc>
    BOOL            TraverseReverseFellowshipID(DWORD dwPlayerID, TFunc& Func);

    template <class TFunc>
    BOOL            TraverseReverseBlackListID(DWORD dwPlayerID, TFunc& Func);

public:
    KG_FELLOWSHIP_GROUPNAMES_MAP    m_GroupNamesMap;

private:
    KG_FELLOWSHIP_MAP               m_FellowshipMap;        // 在线玩家的好友列表(需要存盘)
    KG_ID_PAIR_SET                  m_ReverseFellowshipSet; // 反向索引表(不存盘，根据在线玩家的好友信息动态生成)\

    KG_BLACK_LIST_MAP               m_BlackListMap;         // 黑名单
    KG_ID_PAIR_SET                  m_ReverseBlackListSet;

    KG_FELLOWSHIP_ONLINE_ID_SET     m_OnlineIDSet;
};

struct GetFellowshipIDListFunc
{
    GetFellowshipIDListFunc() { m_dwIDList.reserve(32); };

    BOOL operator()(DWORD dwPlayerID, DWORD dwAlliedPlayerID);

    std::vector<DWORD>  m_dwIDList;
};

template <class TFunc>
BOOL KGFellowshipMgr::TraverseFellowshipID(DWORD dwPlayerID, TFunc& Func)
{
    BOOL                    bResult  = false;
    BOOL                    bRetCode = false;
    KG_FELLOWSHIP_MAP_IT    it;
    KG_FELLOWSHIP_MAP_IT    itEnd;

    assert(dwPlayerID != ERROR_ID);

    it = m_FellowshipMap.lower_bound(KG_ID_PAIR(dwPlayerID, 0));
    itEnd = m_FellowshipMap.lower_bound(KG_ID_PAIR(dwPlayerID + 1, 0));

    for (NULL; it != itEnd; ++it)
    {
        KG_ID_PAIR& rIDPair = (KG_ID_PAIR&)it->first;

        bRetCode = Func(rIDPair.first, rIDPair.second);
        if (!bRetCode)
            break;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

template <class TFunc>
BOOL KGFellowshipMgr::TraverseBlackListID(DWORD dwPlayerID, TFunc& Func)
{
    BOOL                    bResult  = false;
    BOOL                    bRetCode = false;
    KG_BLACK_LIST_MAP_IT    it;
    KG_BLACK_LIST_MAP_IT    itEnd;

    assert(dwPlayerID != ERROR_ID);

    it = m_BlackListMap.lower_bound(KG_ID_PAIR(dwPlayerID, 0));
    itEnd = m_BlackListMap.lower_bound(KG_ID_PAIR(dwPlayerID + 1, 0));

    for (NULL; it != itEnd; ++it)
    {
        KG_ID_PAIR& rIDPair = (KG_ID_PAIR&)it->first;

        bRetCode = Func(rIDPair.first, rIDPair.second);
        if (!bRetCode)
            break;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

template <class TFunc>
BOOL KGFellowshipMgr::TraverseReverseFellowshipID(DWORD dwPlayerID, TFunc& Func)
{
    BOOL                bResult  = false;
    BOOL                bRetCode = false;
    KG_ID_PAIR_SET_IT   it;
    KG_ID_PAIR_SET_IT   itEnd;

    assert(dwPlayerID != ERROR_ID);

    it = m_ReverseFellowshipSet.lower_bound(KG_ID_PAIR(dwPlayerID, 0));
    itEnd = m_ReverseFellowshipSet.lower_bound(KG_ID_PAIR(dwPlayerID + 1, 0));

    for (NULL; it != itEnd; ++it)
    {
        KG_ID_PAIR& rIDPair = (KG_ID_PAIR&)*it;

        bRetCode = Func(rIDPair.first, rIDPair.second);
        if (!bRetCode)
            break;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

template <class TFunc>
BOOL KGFellowshipMgr::TraverseReverseBlackListID(DWORD dwPlayerID, TFunc& Func)
{
    BOOL                bResult  = false;
    BOOL                bRetCode = false;
    KG_ID_PAIR_SET_IT   it;
    KG_ID_PAIR_SET_IT   itEnd;

    assert(dwPlayerID != ERROR_ID);

    it = m_ReverseBlackListSet.lower_bound(KG_ID_PAIR(dwPlayerID, 0));
    itEnd = m_ReverseBlackListSet.lower_bound(KG_ID_PAIR(dwPlayerID + 1, 0));

    for (NULL; it != itEnd; ++it)
    {
        KG_ID_PAIR& rIDPair = (KG_ID_PAIR&)*it;

        bRetCode = Func(rIDPair.first, rIDPair.second);
        if (!bRetCode)
            break;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

#endif // _KFELLOWSHIP_H_
