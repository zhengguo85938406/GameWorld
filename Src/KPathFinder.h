// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KPathFinderEx.h
//  Creator		: hanruofei
//	Date		: 10/10/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include "Engine/KMemory.h"

#include <vector>
#include <list>
#include <set>

typedef int BOOL;

struct KCellEx;
class KScene;

struct KWayFindingData
{
    int F;
    int G;
    int H;
    BOOL bOpenPoint;
    BOOL bClosePoint;
    KCellEx* pParent;

    void UpdateF()
    {
        F = G + H;
    }
};

typedef std::set<DWORD, std::less<DWORD>, KMemory::KAllocator<DWORD> > KSET_FULL_OBSTACLE; // 无法通行的障碍
typedef std::set<DWORD, std::less<DWORD>, KMemory::KAllocator<DWORD> > KSET_DANGEOURS_OBSTACLE; // 可以强行通过的障碍

struct KCellEx
{
    BOOL             bObstacle;
    int              nX;
    int              nY;
    BOOL             bInitObstacle;
    KSET_FULL_OBSTACLE      setFullObstacle;
    KSET_DANGEOURS_OBSTACLE setDangerousObstacle;

    BOOL IsFullObstacle()
    {
        return bInitObstacle || setFullObstacle.size();
    }

    void ApplyObstacle (DWORD dwKey, BOOL bApply, BOOL bFullObstacle)
    {
        if (bInitObstacle)
            return;

        if (bApply)
        {
            if (bFullObstacle)
            {
                setFullObstacle.insert(dwKey);
            }
            else
            {
                setDangerousObstacle.insert(dwKey);
            }
        }
        else
        {
            if (bFullObstacle)
            {
                setFullObstacle.erase(dwKey);
            }
            else
            {
                setDangerousObstacle.erase(dwKey);
            }
        }

        bObstacle = (setFullObstacle.size() || setDangerousObstacle.size());
    }
    KWayFindingData* pWayFindingData;
};

struct KCellExLess
{
    bool operator()(KCellEx* pLeft, KCellEx* pRight)
    {
        return pLeft->pWayFindingData->F < pRight->pWayFindingData->F;
    }
};

typedef std::list<KCellEx*, KMemory::KAllocator<KCellEx*> > KLIST_AUTOPATH;
typedef std::set<KCellEx*, std::less<KCellEx*>, KMemory::KAllocator<KCellEx*> > KSET_CELLEX;
typedef std::multiset<KCellEx*, KCellExLess, KMemory::KAllocator<KCellEx*> > KSET_OPEN_POINT;
typedef std::vector<KCellEx*, KMemory::KAllocator<KCellEx*> > KVEC_SURROUND_POINT;
typedef std::map<DWORD, KSET_CELLEX> KMAP_CAUSE_OBSTACLE;

class KPathFinder
{
public:
    enum
    {
        OBLIQUE = 14,
        STEP = 10,
    };
public:
    KPathFinder(void);
    ~KPathFinder(void);

    BOOL Init(KScene* pScene);
    void UnInit();

    BOOL FindPath(KCellEx* pSrc, KCellEx* pDest, KLIST_AUTOPATH& lstAutoPath, BOOL bAllObstacle);
    KCellEx* GetCell(int nCellX, int nCellY);
    BOOL ApplyObstacle(DWORD dwKey, KCellEx* pLeftBottom, KCellEx* pRightTop, BOOL bFullObstacle);
    void UnApplyObstacle(DWORD dwKey, BOOL bFullObstacle);

private:
    void UpdateSurroundPoints(KCellEx* pCurrent, BOOL bAllObstacle);
    void FoundPoint(KCellEx* pCurrent,  KCellEx* pSurround);
    void NotFoundPoint(KCellEx* pCurrent, KCellEx* pDest, KCellEx* pSurround);
    void MakePath(KCellEx* pDest , KLIST_AUTOPATH& lstAutoPath);
    int CalcG(KCellEx* pSource, KCellEx* pDest);
    int CalcH(KCellEx* pSource, KCellEx* pDest);
    BOOL IsDirectlyReachable(KCellEx* pSource, KCellEx* pDest);

private:
    KScene*             m_pScene;
    int                 m_nXLength;
    int                 m_nYLength;
    KCellEx*            m_pTerrain;
    KWayFindingData*    m_pAllWayFindingData;
    KSET_OPEN_POINT     m_setOpenPoint;
    KVEC_SURROUND_POINT m_vecSurroundPoint;
    KMAP_CAUSE_OBSTACLE m_mapCauseObstacle;
};

