#include "stdafx.h"
#include "KMovableObject.h"
#include "KDoodad.h"
#include "KParabola.h"
#include "KScene.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include <algorithm>
#include "KMovableObstacle.h"

KMovableObject::KMovableObject()
{
    m_pPath             = NULL;
    m_dwGuide           = ERROR_ID;
    m_eMoveType         = mosFree;
    m_nVelocityX        = 0;
    m_nVelocityY        = 0;
    m_nVelocityZ        = 0;
    m_nGameLoop         = 0;

    m_bXZRebound        = false;
    m_bYZRebound        = false;
    m_bXYRebound        = false;

    m_fXYReboundCoeX    = 0.0;
    m_fXYReboundCoeY    = 0.0;
    m_fXYReboundCoeZ    = 0.0;
                        
    m_fYZReboundCoeX    = 0.0;
    m_fYZReboundCoeY    = 0.0;
    m_fYZReboundCoeZ    = 0.0;
                    
    m_fXZReboundCoeX    = 0.0;
    m_fXZReboundCoeY    = 0.0;
    m_fXZReboundCoeZ    = 0.0;

    m_bSomeThingOnBasket = false;

    m_eTrackMoveState   = tmsNone;
    m_dwObstacleID      = ERROR_ID;
    memset(&m_moveOffset, 0, sizeof(m_moveOffset));
}

KMovableObject::~KMovableObject(void)
{ 
}

BOOL KMovableObject::Init()
{
    return KSceneObject::Init();
}

void KMovableObject::UnInit()
{
    ClearPath();
    ClearFollower();
    RemoveGuide();
    KSceneObject::UnInit();
}

void KMovableObject::Activate(int nCurLoop)
{
    m_nGameLoop = nCurLoop;

	m_moveOffset.nX = m_nX;
    m_moveOffset.nY = m_nY;
    m_moveOffset.nZ = m_nZ;

	if (m_dwObstacleID)
	{
		KMovableObstacle* pObstacle = NULL;
        KSceneObject* pSceneObj = m_pScene->GetSceneObjByID(m_dwObstacleID);
        if (pSceneObj)
        {
            assert(g_IsObstacle(pSceneObj->GetType()));
            pObstacle = (KMovableObstacle*)pSceneObj;
            KPOSITION cPos = GetPosition();

            if (pObstacle->GetType() == sotHoist)
            {
                cPos = GetPosition();
                cPos.nZ = pObstacle->m_nZ + pObstacle->m_nHeight;
                SetPosition(cPos);
            }
        }
	}
    switch (m_eMoveType)
    {
    case mosFree:
        OnFree(nCurLoop);
        break;
    case mosIdle:
        OnIdle(nCurLoop);
        break;
    case mosTrackMove:
        OnTrackMove(nCurLoop);
        break;
    default:
        break;
    }

    if (m_dwObstacleID)
    {
        KPOSITION cPos;
        KMovableObstacle* pObstacle = NULL;
        KSceneObject* pSceneObj = m_pScene->GetSceneObjByID(m_dwObstacleID);
        if (pSceneObj)
        {
            int nZ = 0;
            assert(g_IsObstacle(pSceneObj->GetType()));
            pObstacle = (KMovableObstacle*)pSceneObj;
            cPos = GetPosition();
            nZ = cPos.nZ + pObstacle->m_moveOffset.nZ;

            if (pObstacle->m_nZ + pObstacle->m_nHeight > nZ)
                nZ = pObstacle->m_nZ + pObstacle->m_nHeight;

            SetPosition(cPos.nX + pObstacle->m_moveOffset.nX, cPos.nY + pObstacle->m_moveOffset.nY, nZ);
        }
    }

	m_moveOffset.nX = m_nX - m_moveOffset.nX;
	m_moveOffset.nY = m_nY - m_moveOffset.nY;
	m_moveOffset.nZ = m_nZ - m_moveOffset.nZ;
}

void KMovableObject::AddFollower(KMovableObject* pFollower)
{
    assert(pFollower);
    
    assert(!IsAncestorOf(pFollower) && !pFollower->IsAncestorOf(this));

    pFollower->RemoveGuide();// 先删除之前的follower
    pFollower->SetPosition(GetTopCenter());

    m_vecFollower.push_back(pFollower->m_dwID);

    pFollower->m_dwGuide = m_dwID;
	pFollower->m_dwObstacleID = ERROR_ID;
}

void KMovableObject::RemoveFollower(KMovableObject* pFollower)
{
    assert(pFollower);
    std::vector<DWORD>::iterator it = std::find(m_vecFollower.begin(), m_vecFollower.end(), pFollower->m_dwID);
    assert(it != m_vecFollower.end());

    m_vecFollower.erase(it);
    pFollower->m_dwGuide = ERROR_ID;
}

void KMovableObject::ClearFollower()
{
    KMovableObject* pMoveObj = NULL;
    std::vector<DWORD> vecTemp(m_vecFollower);
    for (std::vector<DWORD>::const_iterator constIt = vecTemp.begin(); constIt != vecTemp.end(); ++constIt)
    {
        KSceneObject* pObj = (KSceneObject*)m_pScene->GetSceneObjByID(*constIt);
        assert(pObj);
        pMoveObj = dynamic_cast<KMovableObject*>(pObj);
        KGLOG_CHECK_ERROR(pMoveObj);

        pMoveObj->TurnToMoveType(mosFree);
        pMoveObj->RemoveGuide();

        if (!m_pScene->IsBattleFinished() && GetType() == sotHero)
        {
            g_PlayerServer.DoUnTakeDoodad(m_dwID, pMoveObj);
        }
    }

    m_vecFollower.clear();
}

void KMovableObject::RemoveGuide()
{
    if (m_dwGuide)
    {
        KSceneObject* pObj = (KSceneObject*)m_pScene->GetSceneObjByID(m_dwGuide);
        KGLOG_PROCESS_ERROR(pObj);
        KMovableObject* pMoveObj = dynamic_cast<KMovableObject*>(pObj);
        KGLOG_PROCESS_ERROR(pMoveObj);

        pMoveObj->RemoveFollower(this);
    }

Exit0:
    m_dwGuide = ERROR_ID;
    return;
}

void KMovableObject::TurnToMoveType(KMoveType eMoveType)
{
    KMoveType eOldType = m_eMoveType;
    m_eMoveType = eMoveType;

    if (eOldType == mosAttached || eOldType == mosStandOn || eOldType == mosToken)
    {
        RemoveGuide();
    }
    else if(eOldType == mosTrackMove)
    {
        KTrackMoveState eState = m_eTrackMoveState;

        m_eTrackMoveState = tmsNone;
        ClearPath();
        m_bAttackState = false;

        if (eState == tmsMoving)
        {
            OnTrackMoveInterrupted();
        }
        else if(eState == tmsFinished)
        {
            OnTrackMoveFinished();
        }
        else
        {
            assert(false);
        }
    }

    if (m_eMoveType == mosIdle)
    {
        m_bAttackState = false;
    }
}

DWORD KMovableObject::GetGuideID()
{
    return m_dwGuide;
}

BOOL KMovableObject::IsInAir() const
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(m_eMoveType == mosFree || m_eMoveType == mosTrackMove || m_eMoveType == mosIdle);
    KG_PROCESS_ERROR(KSceneObject::IsInAir() && m_dwObstacleID == ERROR_ID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMovableObject::IsOnGround() const
{
	return (KSceneObject::IsOnGround() || m_dwObstacleID != ERROR_ID);
}

BOOL KMovableObject::IsAncestorOf(const KMovableObject* pOther) const
{
    BOOL bResult = false;
    DWORD dwFatherID = ERROR_ID;
    KSceneObject* pFather = NULL;

    if (pOther)
        dwFatherID = pOther->m_dwID;

    while (dwFatherID != ERROR_ID)
    {
        if (dwFatherID == m_dwID)
        {
            bResult = true;
            break;
        }

        pFather = m_pScene->GetSceneObjByID(dwFatherID);
        dwFatherID = ERROR_ID;        
        if (pFather)
            dwFatherID = pFather->GetGuideID();
    }

Exit0:
    return bResult;
}


BOOL KMovableObject::HasFollowRelationWith(const KMovableObject* pOther) const
{
    BOOL bResult = false;
    KG_PROCESS_ERROR(pOther);

    bResult = IsAncestorOf(pOther) || pOther->IsAncestorOf(this);

Exit0:
    return bResult;
}


BOOL KMovableObject::IsTokenBy(KMovableObject* pHost) const
{
    return pHost && m_eMoveType == mosToken && pHost->m_dwID == m_dwGuide;
}

BOOL KMovableObject::IsAttachedTo(KMovableObject* pHost) const
{
    return pHost && m_eMoveType == mosAttached && m_dwGuide == pHost->m_dwID;
}

BOOL KMovableObject::IsStandOnObj() const
{
    return m_eMoveType == mosStandOn;
}

BOOL KMovableObject::IsFollowing(const KMovableObject* pObject) const
{
    return pObject && m_dwGuide == pObject->m_dwID;
}

BOOL KMovableObject::IsFollowing(DWORD dwID) const
{
    return m_dwGuide && m_dwGuide == dwID;
}

BOOL KMovableObject::IsFollowingSomeone() const
{
    return m_dwGuide != ERROR_ID ? true: false;
}

BOOL KMovableObject::IsFollowedBy(KMovableObject* pObject) const
{
    std::vector<DWORD>::const_iterator constIt = std::find(m_vecFollower.begin(), m_vecFollower.end(), pObject->m_dwID);
    return constIt != m_vecFollower.end();
}

BOOL KMovableObject::HasFollower() const
{
    return m_vecFollower.size() != 0;
}

BOOL KMovableObject::HasFollowerWithState(KMoveType eState) const
{
    return GetFirstFollowerWithState(eState) != NULL;
}

BOOL KMovableObject::HasFollowerWithType(KSceneObjectType eType) const
{
    return GetFirstFollowerWithType(eType) != NULL;
}

BOOL KMovableObject::HasFollowerWithStateAndType(KMoveType eState, KSceneObjectType eType) const
{
    BOOL bResult = false;

    for (std::vector<DWORD>::const_iterator constIt = m_vecFollower.begin(); constIt != m_vecFollower.end(); ++constIt)
    {
        KMovableObject* pObj = dynamic_cast<KMovableObject*>(m_pScene->GetSceneObjByID(*constIt));
        assert(pObj);
        if (pObj->GetType() == eType && pObj->GetMoveType() == eState)
        {
            bResult = true;
            break;
        }
    }

    return bResult;
}

KMovableObject* KMovableObject::GetFirstFollowerWithType(KSceneObjectType eType) const
{
    KMovableObject* pResult = NULL;
    KSceneObject*   pObj = NULL;

    for (std::vector<DWORD>::const_iterator constIt = m_vecFollower.begin(); constIt != m_vecFollower.end(); ++constIt)
    {
        pObj = m_pScene->GetSceneObjByID(*constIt);
        if (pObj == NULL)
            continue;
        
        if (pObj->GetType() == eType)
        {
            pResult = dynamic_cast<KMovableObject*>(pObj);
            break;
        }
    }

    return pResult;
}

KMovableObject* KMovableObject::GetFirstFollowerWithState(KMoveType eState) const
{
    KMovableObject* pResult = NULL;
    KSceneObject*   pObj = NULL;
    KMovableObject* pMoveObj = NULL;

    for (std::vector<DWORD>::const_iterator constIt = m_vecFollower.begin(); constIt != m_vecFollower.end(); ++constIt)
    {
        pObj = m_pScene->GetSceneObjByID(*constIt);
        if (pObj == NULL)
            continue;

        pMoveObj = dynamic_cast<KMovableObject*>(pObj);
        assert(pMoveObj);

        if (pMoveObj->m_eMoveType == eState)
        {
            pResult = pMoveObj;
            break;
        }
    }

    return pResult;
}

KMoveType KMovableObject::GetMoveType()
{
    return m_eMoveType;
}

void KMovableObject::ClearVelocity()
{
    m_nVelocityX = 0;
    m_nVelocityY = 0;
    m_nVelocityZ = 0;
}

void KMovableObject::ProcessCollision()
{
    KSceneObject* pObj = NULL;
    KMAP_COLLISIONOBJINFO::iterator itMap = m_mapCollisionObjInfo.begin();
    while (itMap != m_mapCollisionObjInfo.end())
    {
        if (itMap->second) // 已经处理过
        {
            ++itMap;
            continue;
        }

        pObj = m_pScene->GetSceneObjByID(itMap->first);
        if (pObj == NULL)
        {
            m_mapCollisionObjInfo.erase(itMap++);
            continue;
        }

        // 处理碰撞
        m_pScene->m_cCollisionMgr.OnCollisionBegin(this, pObj);

        itMap->second = true;
        ++itMap;
    }
}

std::string KMovableObject::ToStatusString() const
{
    char szTemp[128] = "\0";
    sprintf(szTemp, "P:%s V:%d,%d,%d", GetPosition().ToString().c_str(), m_nVelocityX, m_nVelocityY, m_nVelocityZ);
    return std::string(szTemp);
}

void KMovableObject::ProcessMove()
{
    if (m_eMoveType == mosFree)
    {
        MoveXY(m_nVelocityX, m_nVelocityY);

        MoveZ(m_nVelocityZ);
        AfterProcessMoveZ();

        CheckCollisionObj();

        ProcessRebounds();
        assert(!m_bXYRebound && !m_bXZRebound && !m_bYZRebound);
    }
}

BOOL KMovableObject::CheckGameLoop(int nCurLoop)
{
    if (m_nGameLoop < nCurLoop)
    {
        m_nGameLoop = nCurLoop;
        return true;
    }

    return false;
}

void KMovableObject::DoTrackMove(const IPath* pPath)
{
    int nTotalFrame = 0;
    assert(pPath);

    nTotalFrame = pPath->GetTotalFrame();
    KGLOG_PROCESS_ERROR(nTotalFrame > 0);

    TurnToMoveType(mosTrackMove);

    m_pPath = pPath->GetClone();

    SetPosition(m_pPath->GetStartPos());
    m_pPath->GetStartVelocity(m_nVelocityX, m_nVelocityY, m_nVelocityZ);

    m_eTrackMoveState = tmsMoving;

Exit0:
    return;
}

void KMovableObject::OnPosChanged(const KPOSITION& cOld, const KPOSITION& cNew)
{
    for (std::vector<DWORD>::const_iterator constIt = m_vecFollower.begin(); constIt != m_vecFollower.end(); ++constIt)
    {
        KMovableObject* pObj = dynamic_cast<KMovableObject*>(m_pScene->GetSceneObjByID(*constIt));
        assert(pObj);
        pObj->OnGuidePosChanged(cOld, cNew);
    }

	KMovableObstacle* pObstacle = NULL;
    pObstacle = m_pScene->GetTopObstacle(m_nX, m_nY);
    if (pObstacle)
    {
        int nObstacleTop = pObstacle->GetLocationZ();
        if (nObstacleTop >= m_nZ) // 站在障碍上面
        {
			if (m_nVelocityZ <= 0)
			{
				m_dwObstacleID = pObstacle->m_dwID;
				//LogInfo("%d follow Obstacle", m_dwID);
			}
			else
			{
				m_dwObstacleID = ERROR_ID;
			}
        }
        else
        {
            // 当在升降机上空的时候不能清
			if (m_dwObstacleID && pObstacle->GetType() != sotHoist)
            {
                m_dwObstacleID = ERROR_ID;
                //LogInfo("%d not follow Obstacle1", m_dwID);
            }
        }
    }
    else
    {
        if (m_dwObstacleID)
        {
            m_dwObstacleID = ERROR_ID;
            //LogInfo("%d not follow Obstacle2", m_dwID);
        }
    }
}

void KMovableObject::OnGuidePosChanged(const KPOSITION& cOld, const KPOSITION& cNew)
{
    KPOSITION selfNew;
    assert(m_dwGuide);

    selfNew.nX = m_nX + cNew.nX - cOld.nX;
    selfNew.nY = m_nY + cNew.nY - cOld.nY;
    selfNew.nZ = m_nZ + cNew.nZ - cOld.nZ;

    SetPosition(selfNew.nX, selfNew.nY, selfNew.nZ);

Exit0:
    return;
}

void KMovableObject::AfterProcessMoveZ()
{
    if (IsInAir())
    {
        // 悬空状态向下加速
        m_nVelocityZ -= GetCurrentGravity();
    }
}

void KMovableObject::OnTrackMoveFinished()
{
    KGLogPrintf(
        KGLOG_DEBUG, 
        "Trace move end at frame=%d, pos=%d,%d,%d, v=%d,%d,%d", 
        g_pSO3World->m_nGameLoop, 
        m_nX, m_nY, m_nZ, 
        m_nVelocityX, m_nVelocityY, m_nVelocityZ
    );
}

void KMovableObject::OnTrackMoveInterrupted()
{

}

void KMovableObject::OnFree(int nCurLoop)
{
    assert(m_eMoveType == mosFree);
    if (m_nVelocityX == 0 && m_nVelocityY == 0 && m_nVelocityZ == 0 && !IsInAir())
    {
        TurnToMoveType(mosIdle);
        OnTurnedToIdle();
        goto Exit0;
    }

    ProcessMove();

Exit0:
    return;
}

void KMovableObject::OnTrackMove(int nCurLoop)
{   
    BOOL        bMoveFinished   = false;
    KPOSITION   cCur = GetPosition();
    KPOSITION   cDst;

    assert(m_pPath);
    assert(m_eMoveType == mosTrackMove);
    
    KGLOG_PROCESS_ERROR(nCurLoop >= m_pPath->GetStartFrame());

    cDst = m_pPath->GetPosAtFrame(nCurLoop);

    m_pPath->GetVelocity(nCurLoop, m_nVelocityX, m_nVelocityY, m_nVelocityZ);
   
    MoveXY(cDst.nX - cCur.nX, cDst.nY - cCur.nY);
    MoveZ(cDst.nZ - cCur.nZ);

    if (m_bXZRebound || m_bYZRebound || m_bXYRebound)
    {
        ProcessRebounds();
        TurnToMoveType(mosFree);
        goto Exit0;
    }

    CheckCollisionObj();

    bMoveFinished = m_pPath->IsFinished(nCurLoop);
    if (bMoveFinished)
    {
        m_eTrackMoveState = tmsFinished;
        TurnToMoveType(mosFree);
    }

Exit0:
    return;
}

void KMovableObject::OnIdle(int nCurLoop)
{
    if(IsInAir() || m_nVelocityX != 0 || m_nVelocityY != 0 || m_nVelocityZ != 0)
    {
        TurnToMoveType(mosFree);
        OnFree(nCurLoop);
    }

Exit1:
    return;
}

void KMovableObject::ClearPath()
{
    if(m_pPath)
    {
        m_pPath->ReleaseClone();
        m_pPath = NULL;
    }
}

// 返回0:没碰撞,1:与X碰撞,2:与Y碰撞,3:与XY都碰撞
int KMovableObject::GetCollisionResult(int nSrcX, int nSrcY, int nDestX, int nDestY)
{
    BOOL XDown = false;
    BOOL XUp = false;
    BOOL YLeft = false;
    BOOL YRight = false;

    int nXDistance = -1;
    int nYDistance = -1;

    // 需要检测cell与轨迹的交面
    POINT ObjStart  = {nSrcX, nSrcY};
    POINT ObjEnd    = {nDestX, nDestY};

    POINT CellLeftDown;
    POINT CellLeftUp;
    POINT CellRightDown;
    POINT CellRightUp;

    CellLeftDown.x = nDestX - nDestX % CELL_LENGTH;
    CellLeftDown.y = nDestY - nDestY % CELL_LENGTH;

    CellLeftUp.x = nDestX - nDestX % CELL_LENGTH;
    CellLeftUp.y = nDestY - nDestY % CELL_LENGTH + CELL_LENGTH;

    CellRightDown.x = nDestX - nDestX % CELL_LENGTH + CELL_LENGTH;
    CellRightDown.y = nDestY - nDestY % CELL_LENGTH;

    CellRightUp.x = nDestX - nDestX % CELL_LENGTH + CELL_LENGTH;
    CellRightUp.y = nDestY - nDestY % CELL_LENGTH + CELL_LENGTH;

    // XDown
    if (IsLineSegmentCross(ObjStart, ObjEnd, CellLeftDown, CellRightDown))
    {
        POINT XDownIntersection;

        XDown = true;
        XDownIntersection = GetCrossPoint(ObjStart, ObjEnd, CellLeftDown, CellRightDown);
        nXDistance = g_GetDistance2((int)nSrcX, (int)nSrcY, (int)XDownIntersection.x, (int)XDownIntersection.y);
    }

    // XUp
    if (IsLineSegmentCross(ObjStart, ObjEnd, CellLeftUp, CellRightUp))
    {
        POINT XUpIntersection;

        XUp = true;
        XUpIntersection = GetCrossPoint(ObjStart, ObjEnd, CellLeftUp, CellRightUp);

        int nXUpDistance = g_GetDistance2((int)nSrcX, (int)nSrcY, (int)XUpIntersection.x, (int)XUpIntersection.y);

        if (XDown)
            nXDistance = MIN(nXUpDistance, nXDistance);
        else
            nXDistance = nXUpDistance;
    }

    // YLeft
    if (IsLineSegmentCross(ObjStart, ObjEnd, CellLeftDown, CellLeftUp))
    {
        POINT YLeftIntersection;

        YLeft = true;
        YLeftIntersection = GetCrossPoint(ObjStart, ObjEnd, CellLeftDown, CellLeftUp);
        nYDistance = g_GetDistance2((int)nSrcX, (int)nSrcY, (int)YLeftIntersection.x, (int)YLeftIntersection.y);
    }

    // YRight
    if (IsLineSegmentCross(ObjStart, ObjEnd, CellRightDown, CellRightUp))
    {
        POINT YRightIntersection;

        YRight = true;
        YRightIntersection = GetCrossPoint(ObjStart, ObjEnd, CellRightDown, CellRightUp);

        int nYRightDistance = g_GetDistance2((int)nSrcX, (int)nSrcY, (int)YRightIntersection.x, (int)YRightIntersection.y);

        if (YLeft)
            nYDistance = MIN(nYRightDistance, nYDistance);
        else
            nYDistance = nYRightDistance;
    }

    if (nXDistance >= 0 && nYDistance >= 0)
    {
        if (nXDistance < nYDistance)
            return 1;
        else if (nXDistance > nYDistance)
            return 2;
        else
            return 3;
    }
    else if (nXDistance >= 0)
        return 1;
    else if (nYDistance >= 0)
        return 2;
    else
        return 0;
}   

void KMovableObject::ProcessRebounds()
{
    KG_PROCESS_ERROR(m_bXYRebound || m_bXZRebound || m_bYZRebound);

    if (m_bXYRebound)
    {
        ProcessXYRebound();
    }

    if (m_bXZRebound)
    {
        ProcessXZRebound();
    }

    if (m_bYZRebound)
    {
        ProcessYZRebound();
    }

    m_bXYRebound = false;
    m_bXZRebound = false;
    m_bYZRebound = false;

Exit0:
    return;
}

void KMovableObject::CheckCollisionObj()
{
    KMAP_COLLISIONOBJINFO::iterator itMap;
    KSceneObject*                   pObj = NULL;
    
    // 检测新的碰撞物件
    for (KObjEnumerator it = m_pScene->GetObjEnumerator(); it.HasElement(); it.MoveToNext())
    {
        pObj = it.GetValue();

        if (pObj->m_dwID == m_dwID)
            continue;

        if (g_IsDoodad(pObj))
        {
            KDoodad* pDoodad = (KDoodad*)pObj;
            if (pDoodad->m_dwThrowerID == m_dwID)
                continue;
        }
        
        itMap = m_mapCollisionObjInfo.find(pObj->m_dwID);
        if (itMap != m_mapCollisionObjInfo.end())
            continue;

        if (!g_IsCollide(this, pObj))
            continue;
			
	    CollsionChecked(pObj);

        m_mapCollisionObjInfo[pObj->m_dwID] = false;
        //LogInfo("检测到碰撞:%d,%d", pObj->m_dwID,pObj->GetType());
    }

    // 检测已处理的碰撞是否离开
    itMap = m_mapCollisionObjInfo.begin();
    while (itMap != m_mapCollisionObjInfo.end())
    {
        if (!itMap->second) // 未处理
        {
            ++itMap;
            continue;
        }

        pObj = m_pScene->GetSceneObjByID(itMap->first);
        if (pObj == NULL)
        {
            m_mapCollisionObjInfo.erase(itMap++);
            continue;
        }

        if (!g_IsCollide(this, pObj)) // 已经不碰撞了
        {
            //LogInfo("移除碰撞:%d", pObj->m_dwID);

            m_pScene->m_cCollisionMgr.OnCollisionEnd(this, pObj);

            m_mapCollisionObjInfo.erase(itMap++);
            continue;
        }

        ++itMap;
    }

Exit0:
    return;
}

void KMovableObject::MoveXY(int nDeltaX, int nDeltaY)
{
    BOOL    bRetCode        = false;
    BOOL    bXDirCellMove   = true;
    int     nMoveDistance   = 0;
    int     nXFlag          = 1;
    int     nYFlag          = 1;
    int     nXStep          = 0;
    int     nYStep          = 0;

    assert(!m_bXZRebound && !m_bYZRebound);

    KG_PROCESS_ERROR(nDeltaX != 0 || nDeltaY != 0);

    nXFlag  = nDeltaX >= 0 ? 1 : -1;
    nYFlag  = nDeltaY >= 0 ? 1 : -1;
    nDeltaX = abs(nDeltaX);
    nDeltaY = abs(nDeltaY);

    if(nDeltaX < CELL_LENGTH && nDeltaY < CELL_LENGTH)
    {
        CellMove(nXFlag * nDeltaX, nYFlag * nDeltaY);
        goto Exit0;
    }

    bXDirCellMove = (nDeltaX >= nDeltaY);
    nMoveDistance = MAX(nDeltaX, nDeltaY);

    if (bXDirCellMove)
    {
        nXStep = CELL_LENGTH;
        nYStep = nDeltaY * CELL_LENGTH / nDeltaX;
    }
    else
    {
        nXStep = nDeltaX * CELL_LENGTH / nDeltaY;
        nYStep = CELL_LENGTH;
    }

    while(nMoveDistance >= CELL_LENGTH)
    {
        bRetCode = CellMove(nXFlag * nXStep, nYFlag * nYStep);
        KG_PROCESS_ERROR(bRetCode);

        nMoveDistance -= CELL_LENGTH;
        nDeltaX -= nXStep;
        nDeltaY -= nYStep;
    }

    KG_PROCESS_ERROR(nMoveDistance != 0);

    bRetCode = CellMove(nXFlag * nDeltaX, nYFlag * nDeltaY);
    KG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void KMovableObject::MoveZ(int nDeltaZ)
{
    int         nDestZ  = 0;   
    KPOSITION   cPos;

    assert(!m_bXYRebound);
    
    //KG_PROCESS_ERROR(nDeltaZ); // 偏移为0也需要检测碰撞，不然会出现贴地滑行的情况

    int nHeight = m_pScene->GetPosHeight(m_nX, m_nY);
    cPos = GetPosition();

    nDestZ = cPos.nZ + nDeltaZ;
    if (nDestZ <= nHeight) //只要有速度存在的，就需要处理与地面的反弹
    {
        nDestZ = nHeight;
        m_bXYRebound = true;
    }

    assert(nDestZ >= 0);

    SetPosition(cPos.nX, cPos.nY, nDestZ);
   
Exit0:
    return;
}

BOOL KMovableObject::CellMove(int nDeltaX, int nDeltaY)
{
    BOOL    bResult     = false;
    int     nRetCode    = false; 
    KCell*  pDestCell   = NULL;
    int     nHeight     = 0;
    int     nDestX      = 0;
    int     nDestY      = 0;
    KPOSITION cPos      = GetPosition();
    BOOL    bVerticalObstacle = false;

    KG_PROCESS_SUCCESS(nDeltaX == 0 && nDeltaY == 0);

    nDestX += cPos.nX + nDeltaX;
    nDestY += cPos.nY + nDeltaY;

    if (nDeltaX && (nDestX <= 0 || nDestX >= m_pScene->GetLength()))
    {
        m_bYZRebound = true;
        nDestX = cPos.nX;
    }

    if (nDeltaY && (nDestY <= 0 || nDestY >= m_pScene->GetWidth()))
    {
        m_bXZRebound = true;
        nDestY = cPos.nY;
    }

    KG_PROCESS_ERROR(nDestX >= 0 && nDestX < MAX_X_COORDINATE);
    KG_PROCESS_ERROR(nDestY >= 0 && nDestY < MAX_Y_COORDINATE);

    pDestCell = m_pScene->GetCell(nDestX, nDestY);
    nHeight = m_pScene->GetPosHeight(nDestX, nDestY);
    if (nHeight - cPos.nZ >= CELL_LENGTH / 4)
        bVerticalObstacle = true;

    if (pDestCell && pDestCell->byObstacleType == Obstacle_NULL && !bVerticalObstacle)
    {
        SetPosition(nDestX, nDestY, cPos.nZ);
    }
    else
    {
        nRetCode = GetCollisionResult(cPos.nX, cPos.nY, nDestX, nDestY);

        if (nRetCode == 1 || nRetCode == 3)
        {
            m_bXZRebound = true;
        }

        if (nRetCode == 2 || nRetCode == 3)
        {
            m_bYZRebound = true;
        }

        KG_PROCESS_ERROR(false);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

int KMovableObject::CalcProcessTrackMoveZmax(int nRandomSeed, const KPOSITION& StartPos, const KPOSITION& FinalPos) const
{
    int nDistanceXY         = 0;
    int nDistanceZ          = 0;
    int nOffsetX            = 0;
    int nOffsetY            = 0;
    int nTempRandomNum      = 0;
    int nSaveRandomSeed     = 0;
    int nZmax               = 0;

    nOffsetX = abs(StartPos.nX - FinalPos.nX);
    nOffsetY = abs(StartPos.nY - FinalPos.nY);
    nDistanceXY = (int)sqrt((double)(nOffsetX * nOffsetX + nOffsetY * nOffsetY));

    nDistanceZ  = abs(StartPos.nZ - FinalPos.nZ);

    nSaveRandomSeed = g_GetRandomSeed();
    g_RandomSeed(nRandomSeed);
    //KGLogPrintf(KGLOG_INFO, "seed = %d  DistanceXY = %d DistanceZ = %d OffsetX = %d OffsetY = %d\n", nRandomSeed, nDistanceXY, nDistanceZ, nOffsetX, nOffsetY);
    nTempRandomNum = g_Random(160) + 160;
    g_RandomSeed(nSaveRandomSeed);

    nZmax = nDistanceXY / 640 + nDistanceZ / 320 + nTempRandomNum;

    MakeInRange<int>(nZmax, 240, 600);

    nZmax += MAX(StartPos.nZ, FinalPos.nZ);

    //KGLogPrintf(KGLOG_INFO, "nZmax = %d, temprandom:%d\n", nZmax, nTempRandomNum);
Exit0:
    return nZmax;
}


BOOL KMovableObject::FixTo(const KPOSITION& rPos)
{
    BOOL bResult = false; 
    BOOL bRetCode = false;

    TurnToMoveType(mosFixed);
    ClearVelocity();
    SetPosition(rPos);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMovableObject::UnFix()
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(GetMoveType() == mosFixed);

    TurnToMoveType(mosFree);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMovableObject::GetStartPos(KPOSITION& rPos)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(m_eMoveType == mosTrackMove);
    KGLOG_PROCESS_ERROR(m_pPath);

    rPos = m_pPath->GetStartPos();

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMovableObject::GetFinnalPos(KPOSITION& rPos)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(m_eMoveType == mosTrackMove);
    KGLOG_PROCESS_ERROR(m_pPath);

    rPos = m_pPath->GetFinnalPos();

    bResult = true;
Exit0:
    return bResult;
}

DWORD KMovableObject::GetStandOnObjID()
{
    DWORD dwID = ERROR_ID;

    if (m_dwGuide && m_eMoveType == mosStandOn)
        dwID = m_dwGuide;

    return dwID;
}

void KMovableObject::ProcessXYRebound()
{
    m_nVelocityX = (int)(m_fXYReboundCoeX * m_nVelocityX);
    m_nVelocityY = (int)(m_fXYReboundCoeY * m_nVelocityY);

    m_nVelocityZ += GetCurrentGravity();
    m_nVelocityZ = -(int)(m_fXYReboundCoeZ * m_nVelocityZ);

    if (m_nVelocityZ < 0)
        m_nVelocityZ = 0;

    m_bXYRebound = false;
    //LogInfo("XY面碰撞后, V(%d, %d, %d), loop:%d", m_nVelocityX, m_nVelocityY, m_nVelocityZ, g_pSO3World->m_nGameLoop);
}

void KMovableObject::ProcessXZRebound()
{
    m_nVelocityY = -m_nVelocityY;
    m_bXZRebound = false;
    //LogInfo("XZ面碰撞后, V(%d, %d, %d), loop:%d", m_nVelocityX, m_nVelocityY, m_nVelocityZ, g_pSO3World->m_nGameLoop);
}

void KMovableObject::ProcessYZRebound()
{
    m_nVelocityX = -(int)(m_fYZReboundCoeX * m_nVelocityX);
    m_nVelocityY = (int)(m_fYZReboundCoeY * m_nVelocityY);
    m_nVelocityZ = (int)(m_fYZReboundCoeZ * m_nVelocityZ);
    m_bYZRebound = false;
    //LogInfo("YZ面碰撞后, V(%d, %d, %d), loop:%d", m_nVelocityX, m_nVelocityY, m_nVelocityZ, g_pSO3World->m_nGameLoop);
}

KMovableObject* KMovableObject::GetGuide()
{
    KSceneObject* pObj = m_pScene->GetSceneObjByID(m_dwGuide);
    return dynamic_cast<KMovableObject*>(pObj);
}

BOOL KMovableObject::IsStandOn(KSceneObjectType eType)
{
    KSceneObject* pObj = m_pScene->GetSceneObjByID(m_dwGuide);
    return (pObj && pObj->GetType() == eType && m_eMoveType == mosStandOn);
}

void KMovableObject::CollsionChecked(KSceneObject* pObj)
{
    assert(pObj);
    switch (pObj->GetType())
    {
    case sotLadder:
        {
            KMovableObject* pMovableObj = (KMovableObject*)pObj;
            if (pMovableObj->IsStandOn(sotBasket))
            {
                m_bSomeThingOnBasket = true;
            }
        }
        break;
    case sotCandyBag:
        {
            m_bSomeThingOnBasket = true;
        }
        break;
    default:
        break;
    }
}

void KMovableObject::ClearCollsionTempFlag()
{
    m_bSomeThingOnBasket = false;
}

void KMovableObject::ProcessAfterCollision()
{
    KSceneObject::ProcessAfterCollision();
    ClearCollsionTempFlag();
}

void KMovableObject::ClearCollision()
{
    m_mapCollisionObjInfo.clear();
}

BOOL KMovableObject::HasGuide() const
{
    return m_dwGuide != ERROR_ID;
}

KTWO_DIRECTION KMovableObject::GetAttackingDir() const
{
    if (m_nVelocityX < 0)
        return csdLeft;

    return csdRight;
}

void KMovableObject::OnDeleteFlagSetted()
{
    ClearFollower();
    RemoveGuide();
}

int KMovableObject::GetCurrentGravity()
{
    assert(m_pScene);
    return m_pScene->m_nGlobalGravity;
}

//////////////////////////////////////////////////////////////////////////
int KMovableObject::LuaClearVelocity(Lua_State* L)
{
    ClearVelocity();
    g_PlayerServer.DoSyncSceneObject(this);
    return 0;
}

BOOL KMovableObject::IsSceneObjInCollidedList(DWORD dwObjectID)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KMAP_COLLISIONOBJINFO::iterator it;
	
    it = m_mapCollisionObjInfo.find(dwObjectID);
    KG_PROCESS_ERROR(it != m_mapCollisionObjInfo.end());
    KG_PROCESS_ERROR(!it->second);

	bResult = true;
Exit0:
	return bResult;

}

BOOL KMovableObject::InsertSceneObjInCollidedList(DWORD dwObjectID)
{
    m_mapCollisionObjInfo[dwObjectID] = false;
    return true;
}
