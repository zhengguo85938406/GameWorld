#include "stdafx.h"
#include "KMath.h"
#include "SO3GlobalDef.h"
#include "KSceneObject.h"
#include "KScene.h"
#include "KHero.h"
#include "KAISearchTactics.h"
#include "KDoodad.h"

#define MAX_ALARM_RANGE_REVISE 1000

bool g_CompByDistance(const KSearchNode& left, const KSearchNode& right)
{
	return left.m_nDistance2 < right.m_nDistance2;
}

int g_GetLevelByTargetLife(int nLifeConversionLevel, double fPrecentLife)
{
    int nDeltaLevel = 0;

    if (fPrecentLife < 0.5)
        nDeltaLevel += nLifeConversionLevel;
    if (fPrecentLife < 0.1)
        nDeltaLevel += nLifeConversionLevel;

    return nDeltaLevel;
}

double g_GetRangeByLevelDiff(int nLevelDiff)
{
    double fRangePercent = 1.0;

    if (nLevelDiff > 8)
        nLevelDiff = 8;
    else if (nLevelDiff < -8)
        nLevelDiff = -8;

    fRangePercent -= 0.1 * (8 - nLevelDiff) / 2;

//Exit0:
    return fRangePercent;
}


