#include "stdafx.h"
#include "KSO3World.h"
#include "KSceneObjMgr.h"
#include "KChest.h"
#include "KBall.h"
#include "KBasket.h"
#include "KBasketSocket.h"
#include "KBackboard.h"
#include "KLadder.h"
#include "KHero.h"
#include "KBullet.h"
#include "KTrap.h"
#include "KLandMine.h"
#include "KBrick.h"
#include "KTyre.h"
#include "KClip.h"
#include "KHoist.h"
#include "KCandyBag.h"
#include "KBaseball.h"
#include "KBananaPeel.h"
#include "KDummy.h"
#include "KBuffBox.h"
#include "KGift.h"
#include "KGold.h"
#include "KBuffBox.h"
#include "KBuffObj.h"
#include "KBarrier.h"
#include <algorithm>
#include "KScene.h"

KSceneObjMgr::KSceneObjMgr(void) : m_pScene(NULL)
{
}


KSceneObjMgr::~KSceneObjMgr(void)
{
}

BOOL KSceneObjMgr::Init(KScene* pScene)
{
    assert(pScene);

    m_dwCurID = 1;
    m_pScene = pScene;

    return true;
}

void KSceneObjMgr::UnInit()
{
    ClearAll();
    m_pScene = NULL;
}

KSceneObject* KSceneObjMgr::AddByType(KSceneObjectType eType)
{
    KSceneObject* pObj = CreateObj(eType);
    assert(pObj);

    pObj->m_dwID = m_dwCurID++;
    pObj->m_pScene = m_pScene;

    m_mapAllSceneObject[pObj->m_dwID] = pObj;

    return pObj;
}

KSceneObject* KSceneObjMgr::AddByTemplate(DWORD dwTemplateID)
{
    KSceneObject* pObj = NULL;
    KSceneObjectTemplate* pTemplate = g_pSO3World->m_Settings.m_ObjTemplate.GetTemplate(dwTemplateID);
    KGLOG_PROCESS_ERROR(pTemplate);

    pObj = AddByType((KSceneObjectType)pTemplate->m_nType);
    
    pObj->ApplyTemplateInfo(pTemplate);

Exit0:
    return pObj;
}

BOOL KSceneObjMgr::RemoveByTemplate(DWORD dwTemplateID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

    for (std::map<DWORD, KSceneObject*>::const_iterator constIt = m_mapAllSceneObject.begin(); constIt != m_mapAllSceneObject.end(); ++constIt)
    {
        KSceneObject* pObj = constIt->second;
        assert(pObj);

        if (!pObj->m_pTemplate)
            continue;

        if (pObj->m_pTemplate->m_dwID == dwTemplateID)
        {
            m_pScene->RemoveSceneObject(pObj);
		}
    }

	bResult = true;
Exit0:
	return bResult;
}

KSceneObject* KSceneObjMgr::Get(DWORD dwID) const
{
    KSceneObject* pResult = NULL;
    std::map<DWORD, KSceneObject*>::const_iterator constIt;

    KG_PROCESS_ERROR(dwID != ERROR_ID);

    constIt = m_mapAllSceneObject.find(dwID);
    KG_PROCESS_ERROR(constIt != m_mapAllSceneObject.end());

    pResult = constIt->second;
Exit0:
    return pResult;
}

void KSceneObjMgr::Delete(DWORD dwID)
{
    std::map<DWORD, KSceneObject*>::iterator it = m_mapAllSceneObject.find(dwID);
    assert(it != m_mapAllSceneObject.end());
    KSceneObject* pObj = it->second;

    pObj->UnInit();
    KMEMORY_DELETE(pObj);
    m_mapAllSceneObject.erase(it);
}

void KSceneObjMgr::Activate(int nCurLoop)
{
    // 在物件的activate前后都清除待删除物件，避免已经删除的物件继续参与逻辑处理
    for (std::vector<DWORD>::iterator it = m_vecNeedDelObj.begin(); it != m_vecNeedDelObj.end(); ++it)
    {
        KSceneObject* pObj = Get(*it);
        if (pObj && pObj->IsToBeDelete())
        {
            pObj->UnInit();
            KMEMORY_DELETE(pObj);

            m_mapAllSceneObject.erase(*it);
        }
    }

    m_vecNeedDelObj.clear();

    for (std::map<DWORD, KSceneObject*>::const_iterator constIt = m_mapAllSceneObject.begin(); constIt != m_mapAllSceneObject.end(); ++constIt)
    {
        KSceneObject* pObj = constIt->second;
        assert(pObj);
        pObj->Activate(nCurLoop);
    }

    for (std::vector<DWORD>::iterator it = m_vecNeedDelObj.begin(); it != m_vecNeedDelObj.end(); ++it)
    {
        KSceneObject* pObj = Get(*it);
        if (pObj && pObj->IsToBeDelete())
        {
            pObj->UnInit();
            KMEMORY_DELETE(pObj);

            m_mapAllSceneObject.erase(*it);
        }
    }

    m_vecNeedDelObj.clear();
}

void KSceneObjMgr::SpecialActivate(int nCurLoop)
{
    for (std::map<DWORD, KSceneObject*>::const_iterator constIt = m_mapAllSceneObject.begin(); constIt != m_mapAllSceneObject.end(); ++constIt)
    {
        KSceneObject* pObj = constIt->second;
        assert(pObj);
        pObj->SpecialActivate(nCurLoop);
    }
}

void KSceneObjMgr::ActivateHeroVirtualFrame()
{
    for (std::map<DWORD, KSceneObject*>::const_iterator constIt = m_mapAllSceneObject.begin(); constIt != m_mapAllSceneObject.end(); ++constIt)
    {
        KSceneObject* pObj = constIt->second;
        assert(pObj);

        if (pObj->GetType() == sotHero)
        {
            KHero* pHero = (KHero*)pObj;
            ++pHero->m_nVirtualFrame;
        }
    }
}

void KSceneObjMgr::ProcessCollision()
{
    for (std::map<DWORD, KSceneObject*>::const_iterator constIt = m_mapAllSceneObject.begin(); constIt != m_mapAllSceneObject.end(); ++constIt)
    {
        KSceneObject* pObj = constIt->second;
        assert(pObj);
        pObj->ProcessCollision();
        pObj->ProcessAfterCollision();
    }
}

KObjEnumerator KSceneObjMgr::GetEnumerator() const
{
    return KObjEnumerator(m_mapAllSceneObject.begin(), m_mapAllSceneObject.end());
}

void KSceneObjMgr::ClearAll()
{
    for (std::map<DWORD, KSceneObject*>::iterator it = m_mapAllSceneObject.begin(); it != m_mapAllSceneObject.end();)
    {
        KSceneObject* pObj = it->second;

        pObj->UnInit();
        KMemory::Delete(pObj);

        m_mapAllSceneObject.erase(it++);
    }
}

void KSceneObjMgr::DelayDelObj(DWORD dwID)
{
    m_vecNeedDelObj.push_back(dwID);
}

KSceneObject* KSceneObjMgr::CreateObj(KSceneObjectType eType) const
{
    KSceneObject*   pResult     = NULL;
    KSceneObject*   pRetObj     = NULL;
    BOOL            bRetCode    = false;

    switch(eType)
    {
    case sotChest:
        pRetObj = KMEMORY_NEW(KChest);
        break;
    case sotBall:
        pRetObj = KMEMORY_NEW(KBall);
        break;
    case sotLadder:
        pRetObj = KMEMORY_NEW(KLadder);
        break;
    case sotBackboard:
        pRetObj = KMEMORY_NEW(KBackboard);
        break;
    case sotBasket:
        pRetObj = KMEMORY_NEW(KBasket);
        break;
    case sotBasketSocket:
        pRetObj = KMEMORY_NEW(KBasketSocket);
        break;
    case sotHero:
        pRetObj = KMEMORY_NEW(KHero);
        break;
    case sotBullet:
        pRetObj = KMEMORY_NEW(KBullet);
        break;
    case sotTrap:
        pRetObj = KMEMORY_NEW(KTrap);
        break;
    case sotLandMine:
        pRetObj = KMEMORY_NEW(KLandMine);
        break;
    case sotBrick:
        pRetObj = KMEMORY_NEW(KBrick);
        break;
    case sotTyre:
        pRetObj = KMEMORY_NEW(KTyre);
        break;
    case sotClip:
        pRetObj = KMEMORY_NEW(KClip);
        break;
    case sotHoist:
        pRetObj = KMEMORY_NEW(KHoist);
        break;
    case sotCandyBag:
        pRetObj = KMEMORY_NEW(KCandyBag);
        break;
    case sotBaseball:
        pRetObj = KMEMORY_NEW(KBaseball);
        break;
    case sotBananaPeel:
        pRetObj = KMEMORY_NEW(KBananaPeel);
        break;
    case sotDummy:
        pRetObj = KMEMORY_NEW(KDummy);
        break;
    case sotGift:
        pRetObj = KMEMORY_NEW(KGift);
        break;
    case sotBuffBox:
        pRetObj = KMEMORY_NEW(KBuffBox);
        break;
    case sotBuffObj:
        pRetObj = KMEMORY_NEW(KBuffObj);
        break;
    case sotGold:
        pRetObj = KMEMORY_NEW(KGold);
        break;
    case sotBarrier:
        pRetObj = KMEMORY_NEW(KBarrier);
        break;
    default:
        assert(false);
        break;
    }

    assert(pRetObj);
    bRetCode = pRetObj->Init();
    KGLOG_PROCESS_ERROR(bRetCode);
    
    pResult = pRetObj;
Exit0:
    return pResult;
}

