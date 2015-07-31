#include "stdafx.h"
#include "KSO3World.h"
#include "KBuffList.h"
#include "KBuff.h"
#include "KHero.h"
#include "KAttribute.h"
#include "KPlayerServer.h"

KBuffList::KBuffList()   
{
    m_pSelf = NULL;
}

KBuffList::~KBuffList()
{
}

BOOL KBuffList::Init(KHero* pHero)
{
    m_pSelf = pHero;
    return true;
}

void KBuffList::UnInit()
{
}

BOOL KBuffList::AddBuff(DWORD dwBuffID)
{
    BOOL    bResult     = false;
    KBuff*  pBuff       = NULL;
    KBuffInfo BuffInfo;

    KG_PROCESS_ERROR(dwBuffID != ERROR_ID);

    DelBuff(dwBuffID); // 先删除同类buff

    pBuff = g_pSO3World->m_BuffManager.GetBuff(dwBuffID);
    KGLOG_PROCESS_ERROR(pBuff);

    m_pSelf->ApplyAttribute(pBuff->m_pRollBackAttr);
    m_pSelf->ApplyAttribute(pBuff->m_pActiveAttr);
    
    BuffInfo.pBuff = pBuff;
    BuffInfo.m_nEndFrame = -1;

    if (pBuff->m_nFrame > 0)
        BuffInfo.m_nEndFrame = g_pSO3World->m_nGameLoop + pBuff->m_nFrame;

    if (pBuff->m_nActiveCount > 0)
        BuffInfo.m_nActiveCount = pBuff->m_nActiveCount - 1; // add的时候算一跳

    m_BuffTable[pBuff->m_dwID] = BuffInfo;
    
    g_PlayerServer.DoAddBuffNotify(m_pSelf, pBuff->m_dwID, pBuff->m_nFrame);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBuffList::DelBuff(DWORD dwBuffID)
{
    BOOL    bResult     = false;
    KBuff*  pBuff       = NULL;
    KBuffTable::iterator it;

    KG_PROCESS_SUCCESS(!dwBuffID);

    it = m_BuffTable.find(dwBuffID);
    KG_PROCESS_SUCCESS(it == m_BuffTable.end());

    pBuff = g_pSO3World->m_BuffManager.GetBuff(dwBuffID);
    KGLOG_PROCESS_ERROR(pBuff);

    m_pSelf->UnApplyAttribute(pBuff->m_pRollBackAttr);

    m_BuffTable.erase(dwBuffID);

    g_PlayerServer.DoLoseBuffNotify(m_pSelf, dwBuffID);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void KBuffList::Activate()
{
    KBuffInfo* pBuffInfo = NULL;
    KBuffTable::iterator it = m_BuffTable.begin();
    while (it != m_BuffTable.end())
    {
        pBuffInfo = &it->second;
        assert(pBuffInfo->pBuff);

        if (pBuffInfo->m_nEndFrame > 0 && g_pSO3World->m_nGameLoop > pBuffInfo->m_nEndFrame) // 时间到
        {
            if (pBuffInfo->m_nActiveCount > 0)
            {
                m_pSelf->ApplyAttribute(pBuffInfo->pBuff->m_pActiveAttr);
                --pBuffInfo->m_nActiveCount;

                if (pBuffInfo->m_nActiveCount > 0)
                {
                    pBuffInfo->m_nEndFrame = g_pSO3World->m_nGameLoop + pBuffInfo->pBuff->m_nFrame;
                    ++it;
                    continue;
                }
            }

            m_pSelf->UnApplyAttribute(pBuffInfo->pBuff->m_pRollBackAttr);

            if (pBuffInfo->pBuff->m_bDelOnObjNoAttached)
            {
                m_pSelf->EnsureNoAttached();
            }

            m_BuffTable.erase(it++);
            continue;
        }
        
        if (pBuffInfo->pBuff->m_bDelOnFloor)
        {
            if (!m_pSelf->IsInAir()) // 已经落地
            {
                m_pSelf->UnApplyAttribute(pBuffInfo->pBuff->m_pRollBackAttr);
                m_BuffTable.erase(it++);
                continue;
            }
        }

        if (pBuffInfo->pBuff->m_bDelOnObjNoAttached)
        {
            if(!m_pSelf->HasAttachedByDoodad(m_pSelf->m_dwAttachedDoodadID))
            {
                m_pSelf->UnApplyAttribute(pBuffInfo->pBuff->m_pRollBackAttr);
                m_BuffTable.erase(it++);
                continue;
            }
        }

        ++it;
    }
}

BOOL KBuffList::Save(KBUFF_NODE_MAP& rmapBuffNode)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;

    KBuffInfo* pBuffInfo = NULL;
    KBuffTable::iterator it = m_BuffTable.begin();
    while (it != m_BuffTable.end())
    {
        KBuffNode Node;
        pBuffInfo = &it->second;
        assert(pBuffInfo->pBuff);

        if (!pBuffInfo->pBuff->m_bNeedSave)
        {
            ++it;
            continue;
        }
        
        if (pBuffInfo->m_nEndFrame <= g_pSO3World->m_nGameLoop)
        {
            ++it;
            continue;
        }

        Node.m_nActiveCount = pBuffInfo->m_nActiveCount;
        Node.m_nLeftFrame   = pBuffInfo->m_nEndFrame - g_pSO3World->m_nGameLoop;

        rmapBuffNode[it->first] = Node;

        ++it;
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KBuffList::Load(KBUFF_NODE_MAP& rmapBuffNode)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KBUFF_NODE_MAP::iterator it = rmapBuffNode.begin();
    
    while (it != rmapBuffNode.end())
    {
        KBuffNode& rNode = it->second;

        bRetCode = AddBuff(it->first);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = SetBuffParam(it->first, g_pSO3World->m_nGameLoop + rNode.m_nLeftFrame, rNode.m_nActiveCount);
        KGLOG_PROCESS_ERROR(bRetCode);

        ++it;
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KBuffList::GetAllBuff( KVEC_BUFF& vecRetObj )
{
    KBuffInfo* pBuffInfo = NULL;

    KBuffTable::iterator it = m_BuffTable.begin();
    while (it != m_BuffTable.end())
    {
        pBuffInfo = &it->second;
        assert(pBuffInfo->pBuff);

        vecRetObj.push_back(pBuffInfo);
        ++it;
    }

Exit0:
    return true;
}

BOOL KBuffList::SetBuffParam(DWORD dwBuffID, int nEndFrame, int nActiveCount)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KBuffTable::iterator it;

    it = m_BuffTable.find(dwBuffID);
    KGLOG_PROCESS_ERROR(it != m_BuffTable.end());
    
    it->second.m_nEndFrame      = nEndFrame;
    it->second.m_nActiveCount   = nActiveCount;

	bResult = true;
Exit0:
	return bResult;
}

BOOL KBuffList::HasBuff(DWORD dwBuffID) const
{
    KBuffTable::const_iterator constIt = m_BuffTable.find(dwBuffID);

    return constIt != m_BuffTable.end();
}
