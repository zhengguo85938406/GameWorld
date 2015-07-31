#include "stdafx.h"
#include "KBullet.h"
#include "KScene.h"
#include "KHero.h"
#include "KSO3World.h"

KBullet::KBullet()
{
    m_pSkill            = NULL;
    m_dwCasterID        = ERROR_ID;
    m_nStartFrame       = 0;
    m_nGravity          = 0;
    m_nLifeTime         = 0;
    m_pBulletTemplate   = NULL;
    m_bAttackState      = false;
    m_nSide             = 0;
    m_bBlastedOnCurFrame          = false;
}

KBullet::~KBullet()
{
}

KSceneObjectType KBullet::GetType() const
{
    return sotBullet;
}

void KBullet::ProcessRebounds()
{
    if (m_bXYRebound || m_bXZRebound || m_bYZRebound)
    {
        m_bXYRebound = false;
        m_bXZRebound = false;
        m_bYZRebound = false;

        SetDeleteFlag();
    }
}

void KBullet::Activate(int nCurLoop)
{
    BOOL bRetCode = false;
    KHero* pCaster = NULL;
    KMoveType eMoveType;

    m_bBlastedOnCurFrame = false;

    KG_PROCESS_ERROR(!m_bToBeDelete);

    pCaster = m_pScene->GetHeroById(m_dwCasterID);
    if (pCaster == NULL)
    {
        SetDeleteFlag();
        goto Exit0;
    }

    if (g_pSO3World->m_nGameLoop - m_nStartFrame > m_nLifeTime)
    {
        SetDeleteFlag();
        goto Exit0;
    }
    
    KMovableObject::Activate(nCurLoop);

    eMoveType = GetMoveType();
    if (eMoveType == mosIdle && m_bAttackState)
        KMovableObject::CheckCollisionObj();

Exit0:
    return;
}

void KBullet::ApplyBulletInfo(KBulletInfo* pInfo)
{
    m_nLength           = pInfo->nLengthX;
    m_nWidth            = pInfo->nLengthY;
    m_nHeight           = pInfo->nLengthZ;
    m_nLifeTime         = pInfo->nLifeTime;
    m_nGravity          = pInfo->nGravity;
    m_bAttackState      = pInfo->bAttackOnFly;
    m_pBulletTemplate   = pInfo;
}

BOOL KBullet::CollideHero(KHero* pHero)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KHero* pCaster = NULL;

    KGLOG_PROCESS_ERROR(pHero);

    KG_PROCESS_SUCCESS(m_bToBeDelete);
    KG_PROCESS_SUCCESS(!m_bAttackState);
    KG_PROCESS_SUCCESS(m_nSide == pHero->m_nSide);

    bRetCode = pHero->CanBeAttacked();
    KG_PROCESS_SUCCESS(!bRetCode);

    bRetCode = IsInAttackInterval(pHero->m_dwID);
    KG_PROCESS_SUCCESS(bRetCode);

    if (m_pBulletTemplate->dwCollideAffectID)
    {
        bRetCode = pHero->ApplyGeneralAffect(m_dwCasterID, this, m_pBulletTemplate->dwCollideAffectID);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = RecordAttackedHero(pHero->m_dwID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (m_pBulletTemplate->bBlastOnCollide)
    {
        bRetCode = Blast();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (!m_pBulletTemplate->bPenetrate)
    {
        SetDeleteFlag();
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

int KBullet::GetCurrentGravity()
{
    return m_nGravity;
}

void KBullet::OnDeleteFlagSetted()
{
    BOOL bRetCode = false;

    KG_PROCESS_ERROR(m_pBulletTemplate->bBlastOnDisappear);

    bRetCode = Blast();
    KGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

BOOL KBullet::Blast()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KVEC_OBJ vecObj; 
    KBODY cBlastBody(m_nX, m_nY, m_nZ, m_pBulletTemplate->nBlastRadiusX * 2, m_pBulletTemplate->nBlastRadiusY * 2, m_pBulletTemplate->nBlastRadiusZ * 2);

    KGLOG_PROCESS_ERROR(m_pBulletTemplate);

    KG_PROCESS_SUCCESS(m_bBlastedOnCurFrame);
    KG_PROCESS_SUCCESS(!m_pBulletTemplate->dwBlastAffectID);

    m_pScene->GetBeAttackedObj(cBlastBody, vecObj);
    for (KVEC_OBJ::iterator it = vecObj.begin(); it != vecObj.end(); ++it)
    {
        KSceneObject* pObj = *it;
        if (!pObj->Is(sotHero))
            continue;

        KHero* pHero = (KHero*)pObj;
        if (m_nSide == pHero->m_nSide)
            continue;

        bRetCode = pHero->CanBeAttacked();
        if (!bRetCode)
            continue;

        bRetCode = IsInAttackInterval(pHero->m_dwID);
        if (bRetCode)
            continue;

        bRetCode = pHero->ApplyGeneralAffect(m_dwCasterID, this, m_pBulletTemplate->dwBlastAffectID);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = RecordAttackedHero(pHero->m_dwID);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

Exit1:
    m_bBlastedOnCurFrame = true;
	bResult = true;
Exit0:
	return bResult;
}

BOOL KBullet::IsInAttackInterval(DWORD dwHeroID)
{
    BOOL bResult = false;
    KATTACKED_OBJ_LIST::iterator it;

    KG_PROCESS_ERROR(m_pBulletTemplate->nAttackIntervalFrame > 0);

    it = m_AttackedObjList.find(dwHeroID);
    if (it != m_AttackedObjList.end() && g_pSO3World->m_nGameLoop <= it->second)
    {
        bResult = true;
    }

Exit0:
    return bResult;
}

BOOL KBullet::RecordAttackedHero(DWORD dwHeroID)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(m_pBulletTemplate);

    if (m_pBulletTemplate->nAttackIntervalFrame > 0)
        m_AttackedObjList[dwHeroID] = g_pSO3World->m_nGameLoop + m_pBulletTemplate->nAttackIntervalFrame;

    bResult = true;
Exit0:
    return bResult;
}
