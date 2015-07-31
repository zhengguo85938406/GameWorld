#include "stdafx.h"
#include "KMovableObstacle.h"
#include "KScene.h"

KMovableObstacle::KMovableObstacle()
{
    m_bObstacle = true;
}

KMovableObstacle::~KMovableObstacle()
{
}

void KMovableObstacle::GetMyAreaCells(const KPOSITION& pos, std::vector<uint64_t>& vecCells)
{
    int nX1 = pos.nX - m_nLength / 2;
    int nX2 = pos.nX + m_nLength / 2;
    int nY1 = pos.nY - m_nWidth / 2;
    int nY2 = pos.nY + m_nWidth / 2;

    for (int nX = nX1; nX < nX2; nX += CELL_LENGTH)
    {
        for (int nY = nY1; nY < nY2; nY += CELL_LENGTH)
        {
            int nCellX = nX / CELL_LENGTH;
            int nCellY = nY / CELL_LENGTH;

            vecCells.push_back(MAKE_INT64(nCellX, nCellY));
        }
    }
}

void KMovableObstacle::SetObstacleFlag(BOOL bObstacleFlag)
{
    m_bObstacle = bObstacleFlag;

    if (m_bObstacle)
    {
        KDYNAMIC_OBSTACLE_MAP::iterator it;
        std::vector<uint64_t> vecCells;

        KPOSITION Pos(m_nX, m_nY, m_nZ);

        GetMyAreaCells(Pos, vecCells);

        for (size_t i = 0; i < vecCells.size(); ++i)
            m_pScene->m_mapDynamicObstacle[vecCells[i]].insert(m_dwID);
    }
    else
    {
        KDYNAMIC_OBSTACLE_MAP::iterator it;
        std::vector<uint64_t> vecCells;

        KPOSITION Pos(m_nX, m_nY, m_nZ);

        GetMyAreaCells(Pos, vecCells);

        for (size_t i = 0; i < vecCells.size(); ++i)
        {
            it = m_pScene->m_mapDynamicObstacle.find(vecCells[i]);
            if (it != m_pScene->m_mapDynamicObstacle.end())
                it->second.erase(m_dwID);
        }
    }
}

void KMovableObstacle::Activate(int nCurLoop)
{
    if (m_bObstacle)
    {
        SetObstacleFlag(false);
        KMovableObject::Activate(nCurLoop);
        SetObstacleFlag(true);
    }
    else
    {
        KMovableObject::Activate(nCurLoop);
    }
}

void KMovableObstacle::OnPosChanged(const KPOSITION& cOld, const KPOSITION& cNew)
{
    KCell* pDestCell    = NULL;
    KCell* pSrcCell     = NULL;

    assert(m_pScene);

    KMovableObject::OnPosChanged(cOld, cNew);

    KG_PROCESS_ERROR(m_bObstacle);

    pSrcCell = m_pScene->GetCell(cOld.nX, cOld.nY);
    KGLOG_PROCESS_ERROR(pSrcCell);

    pDestCell = m_pScene->GetCell(cNew.nX, cNew.nY);
    KGLOG_PROCESS_ERROR(pDestCell);

    KG_PROCESS_ERROR(pSrcCell != pDestCell);

    m_pScene->ObstacleCellChanged(this, cOld, cNew);

Exit0:
    return;
}

void KMovableObstacle::OnDeleteFlagSetted()
{
    KDYNAMIC_OBSTACLE_MAP::iterator it;
    std::vector<uint64_t> vecCells;

    KPOSITION Pos(m_nX, m_nY, m_nZ);

    GetMyAreaCells(Pos, vecCells);

    for (size_t i = 0; i < vecCells.size(); ++i)
    {
        it = m_pScene->m_mapDynamicObstacle.find(vecCells[i]);
        if (it != m_pScene->m_mapDynamicObstacle.end())
            it->second.erase(m_dwID);
    }
}
