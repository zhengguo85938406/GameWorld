#include "stdafx.h"
#include "KSceneObject.h"
#include "KMath.h"
#include "KSceneGlobal.h"
#include "GlobalStruct.h"
#include "KAllObjTemplate.h"
#include "KSO3World.h"
#include "KScene.h"
#include "KPlayerServer.h"
#include "KMovableObstacle.h"

static KSceneObjectType g_aAllDoodadType[] = {
    sotChest, sotBall, sotLadder, 
    sotBasket, sotLandMine, sotBrick, sotClip, sotTyre,
    sotGift, sotGold, sotBuffObj
};

static KSceneObjectType g_aAllObstacleType[] = {
    sotHoist, sotDummy, sotBarrier, sotBuffBox
};

KSceneObject::KSceneObject()
{
    m_nX                = 0;
    m_nY                = 0;
    m_nZ                = 0;
    m_nWidth            = CELL_LENGTH;
    m_nLength           = CELL_LENGTH;
    m_nHeight           = CELL_LENGTH * 2;
    m_pScene            = NULL;
    m_bToBeDelete       = false;
    m_dwScriptID        = ERROR_ID;
    m_pTemplate         = NULL;
    m_bCanBeAttack      = false;
    m_bAttackState      = false;
    m_bFullObstacle     = true;
}

KSceneObject::~KSceneObject()
{
}


BOOL KSceneObject::Init()
{
    return true;
}

void KSceneObject::UnInit()
{

}

void KSceneObject::ApplyTemplateInfo(KSceneObjectTemplate* pTemplate)
{
    m_nLength       = pTemplate->m_nLength;
    m_nWidth        = pTemplate->m_nWidth;
    m_nHeight       = pTemplate->m_nHeight;
    m_dwScriptID    = pTemplate->m_dwScriptID;
    m_pTemplate     = pTemplate;

    if (g_IsObstacle(GetType()))
    {
        KMovableObstacle* pObstacle = (KMovableObstacle*)this;
        pObstacle->SetObstacleFlag(pTemplate->m_Obstacle);
    }

    ApplyTemplateCustomParam(pTemplate);
}

void KSceneObject::ApplyInitInfo(const KSceneObjInitInfo& cInitInfo)
{
    m_nX        = cInitInfo.m_nX;
    m_nY        = cInitInfo.m_nY;
    m_nZ        = cInitInfo.m_nZ;

    int nHeight = m_pScene->GetPosHeight(m_nX, m_nY);
    if (m_nZ < nHeight)
        m_nZ = nHeight;

    KPOSITION cOld;
    KPOSITION cNew(m_nX, m_nY, m_nZ);

    OnPosChanged(cOld, cNew);

    ApplySpecialInitInfo(cInitInfo);
}

BOOL KSceneObject::IsOnGround() const
{
    int nHeight = m_pScene->GetPosHeight(m_nX, m_nY);
    return (m_nZ == nHeight);   
}

BOOL KSceneObject::IsInAir() const
{
    int nHeight = m_pScene->GetPosHeight(m_nX, m_nY);
    return (m_nZ > nHeight);
}

KBODY KSceneObject::GetBody() const
{
    return KBODY(m_nX, m_nY, m_nZ, m_nLength, m_nWidth, m_nHeight);
}

KCell* KSceneObject::GetCell() const
{
    assert(m_pScene);
    return m_pScene->GetCell(m_nX, m_nY);
}

KPOSITION KSceneObject::GetPosition() const
{
    return KPOSITION(m_nX, m_nY, m_nZ);
}

KPOSITION KSceneObject::GetTopCenter() const
{
    return KPOSITION(m_nX, m_nY, m_nZ + m_nHeight);
}

void KSceneObject::SetPosition(const KPOSITION& cPos)
{
    SetPosition(cPos.nX, cPos.nY, cPos.nZ);
}

void KSceneObject::SetPosition(int nX, int nY, int nZ)
{  
    KPOSITION cOld(m_nX, m_nY, m_nZ);
    int nHeight = 0;

    KGLOG_PROCESS_ERROR(m_pScene);
    KG_PROCESS_ERROR(nX >= 0 && nX < m_pScene->GetLength());
    KG_PROCESS_ERROR(nY >= 0 && nY < m_pScene->GetWidth());

    m_nX = nX;
    m_nY = nY;
    m_nZ = nZ;

    nHeight = m_pScene->GetPosHeight(m_nX, m_nY);
    if (m_nZ < nHeight)
        m_nZ = nHeight;

    {
        KPOSITION cNew(m_nX, m_nY, m_nZ);
        OnPosChanged(cOld, cNew);
    }

Exit0:
    return;
}

void KSceneObject::SetDeleteFlag()
{
    if (!m_bToBeDelete)
    {
        m_bToBeDelete = true; 
        m_pScene->m_pSceneObjMgr->DelayDelObj(m_dwID);
        m_pScene->ApplyObstacle(this, false);
        OnDeleteFlagSetted();
    }
}

void KSceneObject::SetX(int nX)
{
    assert(m_pScene);
    assert(nX >= 0 && nX < m_pScene->GetLength());

    m_nX = nX;

    int nHeight = m_pScene->GetPosHeight(m_nX, m_nY);
    if (m_nZ < nHeight)
        m_nZ = nHeight;
}

void KSceneObject::SetY(int nY)
{
    assert(m_pScene);
    assert(nY >= 0 && nY < m_pScene->GetWidth());
    m_nY = nY;
    int nHeight = m_pScene->GetPosHeight(m_nX, m_nY);
    if (m_nZ < nHeight)
        m_nZ = nHeight;
}

void KSceneObject::SetZ(int nZ)
{
    m_nZ = nZ;
    int nHeight = m_pScene->GetPosHeight(m_nX, m_nY);
    if (m_nZ < nHeight)
        m_nZ = nHeight;
}

int KSceneObject::GetLocationZ()
{
    return m_nZ + m_nHeight;
}

BOOL g_IsDoodad(const KSceneObject* pObj)
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(pObj);

    for (int i = 0; i < countof(g_aAllDoodadType); ++i)
    {
        if (pObj->GetType() == g_aAllDoodadType[i])
        {
            bResult = true;
            break;
        }
    }

Exit0:
    return bResult;
}

BOOL g_IsDoodad(KSceneObjectType eType)
{
    BOOL bResult = false;

    for (int i = 0; i < countof(g_aAllDoodadType); ++i)
    {
        if (eType == g_aAllDoodadType[i])
        {
            bResult = true;
            break;
        }
    }

Exit0:
    return bResult;
}


BOOL g_IsObstacle(const KSceneObject* pObj)
{
    BOOL bResult = false;

    KG_PROCESS_ERROR(pObj);

    for (int i = 0; i < countof(g_aAllObstacleType); ++i)
    {
        if (pObj->GetType() == g_aAllObstacleType[i])
        {
            bResult = true;
            break;
        }
    }

Exit0:
    return bResult;
}

BOOL g_IsObstacle(KSceneObjectType eType)
{
    BOOL bResult = false;

    for (int i = 0; i < countof(g_aAllObstacleType); ++i)
    {
        if (eType == g_aAllObstacleType[i])
        {
            bResult = true;
            break;
        }
    }

Exit0:
    return bResult;
}

int g_GetDistance(KSceneObject* pSrc, KSceneObject* pDst)
{
    return g_GetDistance3(pSrc->m_nX, pSrc->m_nY, pSrc->m_nZ, pDst->m_nX, pDst->m_nY, pDst->m_nZ);
}


//////////////////////////////////////////////////////////////////////////
// lua function

int KSceneObject::LuaGetPosition(Lua_State* L)
{
    int         nResult        = 0;
    int         nTopIndex      = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 0);

    lua_pushinteger(L, m_nX);
    lua_pushinteger(L, m_nY);
    lua_pushinteger(L, m_nZ);

    nResult = 3;
Exit0:
    return nResult;
}

int KSceneObject::LuaSetPosition(Lua_State* L)
{
    int         nResult         = 0;
    int         nTopIndex       = 0;
    int         nX              = 0;
    int         nY              = 0;
    int         nZ              = 0;

    nTopIndex = lua_gettop(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nX = (int)lua_tonumber(L, 1);
    nY = (int)lua_tonumber(L, 2);
    nZ = (int)lua_tonumber(L, 3);

    SetPosition(nX, nY, nZ);

    g_PlayerServer.DoSyncSceneObjPosition(this);

    nResult = 3;
Exit0:
    return nResult;
}

BOOL KSceneObject::Is(KSceneObjectType eType) const
{
    return GetType() == eType;
}

KPOSITION KSceneObject::GetBodyCenter() const
{
    return KPOSITION(m_nX, m_nY, m_nZ + m_nHeight / 2);
}

void KSceneObject::OnDeleteFlagSetted()
{
}

BOOL KSceneObject::GetAttackRange(int& nLeftBottomX, int& nLeftBottomY, int& nRightTopX, int& nRightTopY)
{
    nLeftBottomX = m_nX - m_nLength / 2 - CELL_LENGTH;
    nLeftBottomY = m_nY - m_nWidth / 2  - CELL_LENGTH;

    nRightTopX = m_nX + m_nLength / 2 + CELL_LENGTH;
    nRightTopY = m_nY + m_nWidth / 2  + CELL_LENGTH;

    return true;
}









