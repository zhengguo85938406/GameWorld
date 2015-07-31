#include "stdafx.h"
#include "KBasket.h"
#include "KBasketSocket.h"
#include "GlobalMacro.h"
#include "KHero.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KScene.h"
#include "KFix2PParabola.h"
#include "KPlayer.h"
#include "KHero.h"

KBasket::KBasket(void)
{
    m_pPluginSocket         = NULL;
    m_nBasketHP             = 0;
    m_nBasketMaxHP          = 20;
    m_nDropBasketVelocityX  = 20;
    m_nDropBasketVelocityZ  = 10;
    m_bNotCostHP            = false;
}


KBasket::~KBasket(void)
{
    if (m_pPluginSocket)
    {
        m_pPluginSocket->m_pBasket = NULL;
        m_pPluginSocket = NULL;
    }
}


int KBasket::GetCustomData(int nIndex)
{
    int nData = 0;
    if (nIndex == 0)
    {
        if (m_pPluginSocket)
            nData = m_pPluginSocket->m_dwID;
    }

    return nData;
}

void KBasket::CheckBasketHP(KHero* pHero)
{
    KBasketSocket* pSocket = m_pPluginSocket;

    assert(pHero);
    KG_PROCESS_ERROR(m_nBasketHP > m_nBasketMaxHP);

    m_nBasketHP = 0;
    pSocket->DelBasket();
    g_PlayerServer.BasketDrop(this);

    KGLogPrintf(KGLOG_INFO,"%d ²à %d ²ã Àº¿ðµôÂä\n", pSocket->m_eDir, pSocket->m_eFloor);
    m_pScene->OnBasketDropped(this, pSocket);

    if (pHero->IsMainHero())
    {
        KPlayer* pPlayer = NULL;
        pPlayer = pHero->GetOwner();
        assert(pPlayer);

        pPlayer->OnEvent(peDownBasket);
    }

Exit0:
    return;
}

void KBasket::CostHP(DWORD dwHeroID, KBall* pBall)
{
    KHero* pHero = NULL;
	
	assert(pBall);

    KG_PROCESS_ERROR(!m_bNotCostHP);
		
	pHero = m_pScene->GetHeroById(dwHeroID);
    KGLOG_PROCESS_ERROR(pHero);

    switch (pBall->m_eLastBeShootType)
    {
    case stNormalShoot:
        m_nBasketHP += pHero->m_nNormalShootForce;
        break;
    case stSkillShoot:
    case stShootBySkill:    // TODO: tongxuehu
        m_nBasketHP += pHero->m_nSkillShootForce;
        pBall->m_eLastBeShootType = stNormalShoot;
        break;
    case stSlamDunk:
    case stSkillSlam:
        m_nBasketHP += pHero->m_nSlamDunkForce;
        pBall->m_eLastBeShootType = stNormalShoot;
        break;
    default:
        assert(false);
    }

    CheckBasketHP(pHero);

Exit0:
    return;
}

KSceneObjectType KBasket::GetType() const
{
    return sotBasket;
}

void KBasket::ApplySpecialInitInfo(const KSceneObjInitInfo& cTemplate)
{
    m_bCanStood = true;
}

BOOL KBasket::CanBeStandOnBy(KMovableObject* pObj) const
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(pObj);
    KG_PROCESS_ERROR(pObj->GetType() == sotLadder);

    KG_PROCESS_ERROR(m_pPluginSocket != NULL);

    KG_PROCESS_ERROR(KDoodad::CanBeStandOnBy(pObj));

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBasket::IsInSocket() const
{
    return m_pPluginSocket != NULL;
}

void KBasket::OnTrackMoveFinished()
{
    if (m_eActionType == tmaShoot)
    {
        OnShootSuccess();
    }
}

void KBasket::OnTrackMoveInterrupted()
{

}

void KBasket::OnShootSuccess()
{
    m_eActionType = tmaNone;
    if (m_pTargetSocket->IsEmpty())
    {
        m_pTargetSocket->AddBasket(this);
        KHero* pShooter = m_pScene->GetHeroById(m_dwShooterID);
        if (pShooter && pShooter->m_nSide == m_pTargetSocket->m_nSide)
        {
            pShooter->m_GameStat.m_nPluginBasket += 1;
        }
        m_pScene->OnBasketPlugin(this, m_pPluginSocket);
    }
}

void KBasket::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    m_nBasketMaxHP = pTemplate->m_nCustomParam[0];
}
