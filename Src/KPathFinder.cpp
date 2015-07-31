#include "stdafx.h"
#include "KPathFinder.h"
#include "KScene.h"


KPathFinder::KPathFinder(void)
{
    m_pScene             = NULL;
    m_pTerrain           = NULL;
    m_pAllWayFindingData = NULL;
    m_nXLength           = 0;
    m_nYLength           = 0;
    m_vecSurroundPoint.reserve(9);
}

KPathFinder::~KPathFinder(void)
{
	assert(m_pScene == NULL);
    assert(m_pTerrain == NULL);
    assert(m_pAllWayFindingData == NULL);
    assert(m_nXLength == 0);
    assert(m_nYLength == 0);
}

BOOL KPathFinder::Init(KScene* pScene)
{
    BOOL bResult  = false;
    size_t uAllWayFindingDataLength = 0;
    KCellEx* pCellEx = NULL;
    KCell* pCell = NULL;
    KWayFindingData* pWayFindingData = NULL;

    assert(pScene);

    m_pScene    = pScene;
    m_nXLength  = pScene->m_nXGridCount;
    m_nYLength  = pScene->m_nYGridCount;

    uAllWayFindingDataLength = sizeof(KWayFindingData) * m_nXLength * m_nYLength;

    m_pAllWayFindingData = new KWayFindingData[m_nXLength * m_nYLength];
    KGLOG_PROCESS_ERROR(m_pAllWayFindingData);
    memset(m_pAllWayFindingData, 0, uAllWayFindingDataLength);

    m_pTerrain = new KCellEx[m_nXLength * m_nYLength];
    KGLOG_PROCESS_ERROR(m_pTerrain);

    for (int j = 0; j < m_nYLength; ++j)
    {
        for (int i = 0; i < m_nXLength; ++i)
        {
            pCellEx         = m_pTerrain            + j * m_nXLength  + i;
            pWayFindingData = m_pAllWayFindingData  + j * m_nXLength  + i;
            pCell           = pScene->m_pCells      + j * SCENE_GRID_WIDTH + i;

            pCellEx->nX   = i;
            pCellEx->nY   = j;
            pCellEx->bObstacle = (pCell->byObstacleType != Obstacle_NULL);
            pCellEx->bInitObstacle = pCellEx->bObstacle;
            pCellEx->pWayFindingData = pWayFindingData;
        }
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        SAFE_DELETE_ARRAY(m_pTerrain);
        SAFE_DELETE_ARRAY(m_pAllWayFindingData);
    }
    return bResult;
}

void KPathFinder::UnInit()
{
    SAFE_DELETE_ARRAY(m_pTerrain);
    SAFE_DELETE_ARRAY(m_pAllWayFindingData);

    m_nXLength = 0;
    m_nYLength = 0;

    m_pScene = NULL;
}

BOOL KPathFinder::FindPath(KCellEx* pSrc, KCellEx* pDest, KLIST_AUTOPATH& lstAutoPath, BOOL bAllObstacle)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KCellEx* pCurrent = NULL;
    KCellEx* pSurround = NULL;
	
	m_setOpenPoint.clear();
    memset(m_pAllWayFindingData, 0, sizeof(KWayFindingData) * m_nXLength * m_nYLength);

    m_setOpenPoint.insert(pSrc);
    pSrc->pWayFindingData->bOpenPoint = true;

    while (m_setOpenPoint.size())
    {
        pCurrent = *m_setOpenPoint.begin();
        m_setOpenPoint.erase(m_setOpenPoint.begin());
        pCurrent->pWayFindingData->bOpenPoint = false;

        pCurrent->pWayFindingData->bClosePoint = true;

        UpdateSurroundPoints(pCurrent, bAllObstacle);
        for (KVEC_SURROUND_POINT::iterator it = m_vecSurroundPoint.begin(); it != m_vecSurroundPoint.end(); ++it)
        {
            pSurround = *it;
            if (pSurround->pWayFindingData->bOpenPoint)
            {
                FoundPoint(pCurrent, pSurround);
            }
            else
            {
                NotFoundPoint(pCurrent, pDest, pSurround);
            }
        }

        if (pDest->pWayFindingData->bOpenPoint)
        {
            MakePath(pDest, lstAutoPath);
            bResult = true;
            break;
        }
    }

	return bResult;
}

void KPathFinder::UpdateSurroundPoints(KCellEx* pCurrent, BOOL bAllObstacle)
{
    KCellEx* pSurround      = NULL;
    int nCurrentHeight = 0;
    int nSurroundHeight = 0;
    
    nCurrentHeight = m_pScene->GetPosHeight(pCurrent->nX * CELL_LENGTH, pCurrent->nY * CELL_LENGTH);
    m_vecSurroundPoint.clear();

    for (int j = pCurrent->nY - 1; j <= pCurrent->nY + 1; ++j)
    {
        if (j < 0)
            continue;

        if (j >= m_nYLength)
            continue;

        for (int i =  pCurrent->nX - 1; i <= pCurrent->nX + 1; ++i)
        {
            if (i < 0)
                continue;

            if (i >= m_nXLength)
                continue;

            pSurround = m_pTerrain + j * m_nXLength + i;

            if (pSurround->pWayFindingData->bClosePoint)
                continue;

            if (bAllObstacle)
            {
                if (pSurround->bObstacle)
                    continue;
            }
            else
            {
                if (pSurround->IsFullObstacle())
                    continue;
            }

            if (pCurrent->nX != i && pCurrent->nY != j)
            {
                KCellEx* pXSurround = m_pTerrain + j * m_nXLength + pCurrent->nX;
                if (pXSurround->bObstacle)
                    continue;

                KCellEx* pYSurround = m_pTerrain + pCurrent->nY * m_nXLength + i;
                if (pYSurround->bObstacle)
                    continue;
            }

            nSurroundHeight = m_pScene->GetPosHeight(pSurround->nX * CELL_LENGTH, pSurround->nY * CELL_LENGTH);
			if	(nCurrentHeight < nSurroundHeight)
				continue;
					
            m_vecSurroundPoint.push_back(pSurround);
        }
    }
}

void KPathFinder::FoundPoint(KCellEx* pCurrent, KCellEx* pSurround)
{
    int G = CalcG(pCurrent, pSurround);
    if (G < pSurround->pWayFindingData->G)
    {
        pSurround->pWayFindingData->pParent = pCurrent;
        pSurround->pWayFindingData->G = G;
        pSurround->pWayFindingData->UpdateF();
    }
}

void KPathFinder::NotFoundPoint(KCellEx* pCurrent, KCellEx* pDest, KCellEx* pSurround)
{
    pSurround->pWayFindingData->pParent = pCurrent;
    pSurround->pWayFindingData->G = CalcG(pCurrent, pSurround);
    pSurround->pWayFindingData->H = CalcH(pSurround, pDest);
    pSurround->pWayFindingData->UpdateF();

    m_setOpenPoint.insert(pSurround);
    pSurround->pWayFindingData->bOpenPoint = true;
}

void KPathFinder::MakePath(KCellEx* pDest , KLIST_AUTOPATH& lstAutoPath)
{
    KCellEx* pParent = NULL;
    KCellEx* pGrandParent = NULL;
    lstAutoPath.clear();

    for (KCellEx* pCurrent = pDest; pCurrent != NULL;  NULL)
    {
        pParent = pCurrent->pWayFindingData->pParent;
        if (pParent == NULL)
            break;

        pGrandParent = pParent->pWayFindingData->pParent;
        if (pGrandParent == NULL)
        {
            lstAutoPath.push_front(pCurrent);
            pCurrent = pCurrent->pWayFindingData->pParent;
            continue;
        }

        if ( (pCurrent->nX - pGrandParent->nX) * (pParent->nY - pGrandParent->nY) != (pParent->nX - pGrandParent->nX) * (pCurrent->nY - pGrandParent->nY) )
        {
            lstAutoPath.push_front(pCurrent);
            pCurrent = pCurrent->pWayFindingData->pParent;
            continue;
        }

        pCurrent->pWayFindingData->pParent = pGrandParent;
        pParent->pWayFindingData->pParent = NULL;
    }
}

int KPathFinder::CalcG(KCellEx* pSource, KCellEx* pDest)
{
    int G = 0;
    int parentG = 0;
    
    BOOL bXEqual = (pSource->nX == pDest->nX);
    BOOL bYEqual = (pSource->nY == pDest->nY);

    if (!bXEqual && !bYEqual)
    {
        G = OBLIQUE;
    }
    else if (!bXEqual || !bYEqual)
    {
        G = STEP;
    }

    if (pDest->pWayFindingData->pParent)
        parentG = pDest->pWayFindingData->pParent->pWayFindingData->G;

    return G + parentG;
}

int KPathFinder::CalcH(KCellEx* pSource, KCellEx* pDest)
{
    return STEP * (abs(pSource->nX - pDest->nX) + abs(pSource->nY - pDest->nY));
}

KCellEx* KPathFinder::GetCell(int nCellX, int nCellY)
{
    KCellEx* pResult = NULL;

    KG_PROCESS_ERROR(nCellX >= 0 && nCellX < m_nXLength);
    KG_PROCESS_ERROR(nCellY >= 0 && nCellY < m_nYLength);

    pResult = m_pTerrain + nCellY * m_nXLength + nCellX;
Exit0:
    return pResult;
}

BOOL KPathFinder::IsDirectlyReachable(KCellEx* pSource, KCellEx* pDest)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int nMinX = 0;
    int nMaxX = 0;
    int nMinY = 0;
    int nMaxY = 0;
    int nSourceX = pSource->nX;
    int nSourceY = pSource->nY;

    int nSourceX1 = pSource->nX;
    int nSourceY1 = pSource->nY + 1;

    int nSourceX2 = pSource->nX + 1;
    int nSourceY2 = pSource->nY;


	
	assert(pSource);
    assert(pDest);

    //nMinX = min(pSource->nX, pDest->nX);
    //nMaxX = max(pSource->nX, pDest->nX);
    //nMinY = min(pSource->nY, pDest->nY); 
    //nMaxY = max(pSource->nY, pDest->nY);

    //for (int x = nMinX; x <= nMaxX; ++x)
    //{
    //    for (int y = nMinY; y <= nMaxY; ++y)
    //    {
    //        if (y != ( (pDest->nY - pSource->nY) * (x - pSource->nX) + pSource->nY * (pDest->nX - pSource->nX) ) / (pDest->nX - pSource->nX) )
    //            continue;

    //    }
    //}

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPathFinder::ApplyObstacle(DWORD dwKey, KCellEx* pLeftBottom, KCellEx* pRightTop, BOOL bFullObstacle)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KCellEx* pCurrent = NULL;
	
	KGLOG_PROCESS_ERROR(pLeftBottom);
    KGLOG_PROCESS_ERROR(pRightTop);

    KGLOG_PROCESS_ERROR(pLeftBottom->nX <= pRightTop->nX);
    KGLOG_PROCESS_ERROR(pLeftBottom->nY <= pRightTop->nY);

    for (int x = pLeftBottom->nX; x <= pRightTop->nX; ++x)
    {
        for (int y = pLeftBottom->nY; y <= pRightTop->nY; ++y)
        {
            pCurrent = GetCell(x, y);
            KGLOG_PROCESS_ERROR(pCurrent);

            m_mapCauseObstacle[dwKey].insert(pCurrent);

            pCurrent->ApplyObstacle(dwKey, true, bFullObstacle);
        }
    }

	bResult = true;
Exit0:
	return bResult;
}

void KPathFinder::UnApplyObstacle(DWORD dwKey, BOOL bFullObstacle)
{
	BOOL bRetCode = false;
	KMAP_CAUSE_OBSTACLE::iterator it;

    it = m_mapCauseObstacle.find(dwKey);
    if (it == m_mapCauseObstacle.end())
        goto Exit0;

    for (KSET_CELLEX::iterator itSet = it->second.begin(); itSet != it->second.end(); ++itSet)
    {
        KCellEx* pCurrent = *itSet;
        pCurrent->ApplyObstacle(dwKey, false, bFullObstacle);
    }

Exit0:
	return;
}
