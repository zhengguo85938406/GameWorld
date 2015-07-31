#ifndef _KSTAT_DATA_SERVER_H_
#define _KSTAT_DATA_SERVER_H_

#include "Engine/KMemory.h"
#include "KStatDataDef.h"
#include <list>

class KPlayer;
class KScene;

// Server自己维护的统计数据结构
struct KStatData
{
    KStatData() : m_nValue(0), m_nID(0), m_bInList(false) {}
    char    m_szName[STAT_DATA_NAME_LEN];
    int64_t m_nValue;
    int     m_nID;
    BOOL    m_bInList;
};

class KStatDataServer
{
public:
    KStatDataServer();
    ~KStatDataServer();

    BOOL Init();
    void UnInit();

    void Activate();
    void SendAllStatData();
    int  GetUpdateStatData(KSTAT_DATA_MODIFY* pStatData, int nMaxCount);
    BOOL Update(const char cszName[], int64_t nValue);
    BOOL SetNameID(const char cszName[], int nID);

public:
    void UpdateMoneyStat(int nMoney, const char cszMethod[], const char* cpszSubMethod = NULL);

private:
    struct KStatNameLess
    {
        bool operator()(const char* pszX, const char* pszY) const
        {
            return strcmp(pszX, pszY) < 0;
        }
    };

    typedef KMemory::KAllocator<std::pair<char*, KStatData*> > KDATA_TABLE_ALLOCATOR;
    typedef std::map<char*, KStatData*, KStatNameLess, KDATA_TABLE_ALLOCATOR> KDATA_TABLE;

    KDATA_TABLE     m_DataTable;

    typedef std::list<KStatData*, KMemory::KAllocator<KStatData*> > KDATA_QUEUE;
    KDATA_QUEUE     m_DataList;

    std::set<kstring, std::less<kstring>, KMemory::KAllocator<kstring> > m_Name2IDQueue;
};
#endif
