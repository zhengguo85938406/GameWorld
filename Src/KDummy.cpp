#include "stdafx.h"
#include "KDummy.h"
#include "KAllObjTemplate.h"
#include "KPlayerServer.h"
#include "KSO3World.h"
#include "KHero.h"
#include "KScene.h"
#include "KBall.h"

KDummy::KDummy()
{
    m_bCanBeAttack = true;
    m_nAttackFrames = 0;  
	m_nAttackRange	= 0;
    m_bOnlyAttackPlayer = false;       
    m_nLeftAttackFrames = 0;   
}

KDummy::~KDummy()
{
}

void KDummy::Activate(int nCurLoop)
{
    KMovableObstacle::Activate(nCurLoop);

    KG_PROCESS_ERROR(m_nLeftAttackFrames > 0);
	
	ProcessDummyAttack();
    
	--m_nLeftAttackFrames;
    if (m_nLeftAttackFrames <= 0)
    {
        m_bAttackState = false;
        m_pScene->ApplyObstacle(this, true);
    }

Exit0:
    return;
}

int KDummy::GetCustomData(int nIndex)
{
    if (nIndex == 0)
        return m_nLeftAttackFrames;
    return 0;
}

KSceneObjectType KDummy::GetType() const
{
    return sotDummy;
}

void KDummy::BeAttacked()
{
    m_nLeftAttackFrames = m_nAttackFrames;
    m_bAttackState = true;
    m_pScene->ApplyObstacle(this, true);
    g_PlayerServer.DoSyncSceneObject(this, -1);

Exit0:
    return;
}

BOOL KDummy::ProcessDummyAttack()
{
    BOOL bResult	= false;
    BOOL bRetCode	= false;
    KSkill* pSkill	= NULL;
    KHero* pHero	= NULL;
	KSceneObjectType eType = sotInvalid;
	KBODY		AttackBody;
	KVEC_OBJ	vecObj;

    KG_PROCESS_ERROR(m_nLeftAttackFrames > 0);
	KGLOG_PROCESS_ERROR(m_nHeight > 0);
	
	AttackBody.nX = m_nX;
	AttackBody.nY = m_nY;
	AttackBody.nZ = m_nZ;
	AttackBody.nLength = m_nAttackRange;
	AttackBody.nWidth = m_nAttackRange;
	AttackBody.nHeight = m_nHeight - 1;

	m_pScene->GetBeAttackedObj(AttackBody, vecObj);
	KG_PROCESS_ERROR(!vecObj.empty());

	for (KVEC_OBJ::iterator it = vecObj.begin(); it != vecObj.end(); it++)
	{
		KSceneObject* pSceneObj = *it;

		if (pSceneObj->m_dwID == m_dwID)
			continue;

		eType = pSceneObj->GetType();

        if  (eType == sotBall)
        {
            AttackBall((KBall*)pSceneObj);
            continue;
        }

		if (eType != sotHero)
			continue;

		pHero = (KHero*)pSceneObj;
		if (m_bOnlyAttackPlayer && pHero->IsNpc())
		{
            continue;
		}

        bRetCode = pHero->ApplyGeneralAffect(ERROR_ID, this, m_pTemplate->m_dwAttackAffectID1);
        KGLOG_PROCESS_ERROR(bRetCode);
	}

    bResult = true;
Exit0:
    return bResult;
}

void KDummy::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    assert(pTemplate);
    m_nAttackFrames		= pTemplate->m_nCustomParam[0] * GAME_FPS;
	m_nAttackRange		= pTemplate->m_nCustomParam[1];
    m_bOnlyAttackPlayer = pTemplate->m_nCustomParam[2];
}

BOOL KDummy::GetAttackRange(int& nLeftBottomX, int& nLeftBottomY, int& nRightTopX, int& nRightTopY)
{
    if (m_bAttackState)
    {
        nLeftBottomX = m_nX - m_nAttackRange / 2 - CELL_LENGTH;
        nLeftBottomY = m_nY - m_nAttackRange / 2 - CELL_LENGTH;

        nRightTopX = m_nX + m_nAttackRange / 2 + CELL_LENGTH;
        nRightTopY = m_nY + m_nAttackRange / 2 + CELL_LENGTH;
    }
    else
    {
        nLeftBottomX = m_nX - m_nLength / 2;
        nLeftBottomY = m_nY - m_nWidth / 2;

        nRightTopX = m_nX + m_nLength / 2;
        nRightTopY = m_nY + m_nWidth / 2;
    }

    return true;
}

void KDummy::AttackBall(KBall* pBall)
{
    int nDirection = 0;

    assert(pBall);

    nDirection = g_GetDirection(m_nX, m_nY, pBall->m_nX, pBall->m_nY);
    pBall->m_nVelocityX = g_pSO3World->m_Settings.m_ConstList.nDummyAttackBallVXY * g_Cos(nDirection) / SIN_COS_NUMBER;
    pBall->m_nVelocityY = g_pSO3World->m_Settings.m_ConstList.nDummyAttackBallVXY * g_Sin(nDirection) / SIN_COS_NUMBER;
    pBall->m_nVelocityZ = g_pSO3World->m_Settings.m_ConstList.nDummyAttackBallVZ;

    g_PlayerServer.DoSyncSceneObject(pBall, -1);
}
