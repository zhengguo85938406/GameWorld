#include "stdafx.h"
#include "KStatDataServer.h"
#include "KSO3World.h"
#include "KLSClient.h"

KStatDataServer::KStatDataServer()
{
}

KStatDataServer::~KStatDataServer()
{
}

BOOL KStatDataServer::Init()
{   
    return true;
}

void KStatDataServer::UnInit()
{
    KDATA_TABLE::iterator it;

    for (it = m_DataTable.begin(); it != m_DataTable.end(); ++it)
    {
        KMemory::Delete(it->second);
    }
    m_DataTable.clear();
}

void KStatDataServer::Activate()
{
    for (int i = 0; i < 16; i++)
    {
        if (m_Name2IDQueue.empty())
            break;

        const char* pszName = m_Name2IDQueue.begin()->c_str();

        g_LSClient.DoQueryStatIDRequest(pszName);

        m_Name2IDQueue.erase(m_Name2IDQueue.begin());
    }

    if (g_pSO3World->m_nGameLoop % GAME_FPS == 0 && !m_DataList.empty())
        g_LSClient.DoUpdateStatDataRequest();

    return;
}

void KStatDataServer::SendAllStatData()
{
    while (!m_DataList.empty())
    {
        g_LSClient.DoUpdateStatDataRequest();
    }

    return;
}

int KStatDataServer::GetUpdateStatData(KSTAT_DATA_MODIFY* pData, int nMaxCount)
{
    KStatData*          pStatData   = NULL;
    KSTAT_DATA_MODIFY*  pNode       = pData;
    KSTAT_DATA_MODIFY*  pTail       = pData + nMaxCount;

    while (pNode < pTail && !m_DataList.empty())
    {
        pStatData = m_DataList.front();

        assert(pStatData);

        pNode->nID    = pStatData->m_nID;
        pNode->nValue = pStatData->m_nValue;

        pStatData->m_nValue  = 0;
        pStatData->m_bInList = false;

        pNode++;

        m_DataList.pop_front();
    }

    return (int)(pNode - pData);
}

BOOL KStatDataServer::Update(const char szName[], int64_t nValue)
{
    BOOL                                    bResult   = false;
    size_t                                  uNameLen  = 0;
    KStatData*                              pStatData = NULL;
    KDATA_TABLE::iterator                   it;
    std::pair<KDATA_TABLE::iterator, BOOL>  InsRet;

    uNameLen = strlen(szName) + 1;
    KGLOG_PROCESS_ERROR(uNameLen <= STAT_DATA_NAME_LEN);

    it = m_DataTable.find((char*)szName);

    if (it == m_DataTable.end())
    {
        pStatData = KMEMORY_NEW(KStatData);
        KGLOG_PROCESS_ERROR(pStatData);

        strncpy(pStatData->m_szName, szName, sizeof(pStatData->m_szName));
        pStatData->m_szName[sizeof(pStatData->m_szName) - 1] = '\0';

        InsRet = m_DataTable.insert(std::make_pair(pStatData->m_szName, pStatData));
        KGLOG_PROCESS_ERROR(InsRet.second);

        it = InsRet.first;
    }
    
    pStatData = it->second;
    pStatData->m_nValue += nValue;

    if (pStatData->m_nID == 0)
    {
        m_Name2IDQueue.insert(szName);
    }
    else
    {
        // 只有已获得ID的数据才放入m_DataList中
        if (!pStatData->m_bInList)
        {
            // 不在队列里说明是已发送过的数据，此次再次修改
            m_DataList.push_back(pStatData);
            pStatData->m_bInList = true;
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KStatDataServer::SetNameID(const char szName[], int nID)
{
    BOOL                    bResult   = false;
    KStatData*              pStatData = NULL;
    KDATA_TABLE::iterator   it;
    
    it = m_DataTable.find((char*)szName);
    KG_PROCESS_ERROR(it != m_DataTable.end());

    pStatData = it->second;
    KGLOG_PROCESS_ERROR(pStatData);

    KG_PROCESS_SUCCESS(pStatData->m_nID != 0);
        
    pStatData->m_nID = nID;
    m_DataList.push_back(pStatData);
    pStatData->m_bInList = true;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KStatDataServer::UpdateMoneyStat(int nMoney, const char cszMethod[], const char* cpszSubMethod/* = NULL*/)
{
    const char* pszGain = (nMoney >= 0 ? "GAIN" : "COST");
    char        szVarName[STAT_DATA_NAME_LEN];

    KG_PROCESS_ERROR(nMoney != 0);

    if (cpszSubMethod)
    {
        snprintf(szVarName, sizeof(szVarName), "MONEY|%s|%s|%s", pszGain, cszMethod, cpszSubMethod);
        szVarName[sizeof(szVarName) - 1] = '\0';
    }
    else
    {
        snprintf(szVarName, sizeof(szVarName), "MONEY|%s|%s", pszGain, cszMethod);
        szVarName[sizeof(szVarName) - 1] = '\0';
    }

    if (nMoney < 0)
        nMoney = -nMoney;

    Update(szVarName, nMoney);

Exit0:
    return;
}
