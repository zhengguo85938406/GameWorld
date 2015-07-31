#ifndef KMathH
#define	KMathH

#include <math.h>

#define SO3WORLD_PI             3.1415926f

template<typename T>
inline T g_GetDistance3(T nDistance2, T nSourceZ, T nDestZ)
{
	T nDistance3 = nDistance2 + (nSourceZ - nDestZ) * (nSourceZ - nDestZ);

	return nDistance3;
}

template<typename T>
inline T g_GetDistance3(T nSourceX, T nSourceY, T nSourceZ, T nDestX, T nDestY, T nDestZ)
{
	T nDistance3 = 
        (nSourceX - nDestX) * (nSourceX - nDestX) + 
		(nSourceY - nDestY) * (nSourceY - nDestY) + 
        (nSourceZ - nDestZ) * (nSourceZ - nDestZ);

	return nDistance3;
}

template<typename T>
inline T g_GetDistance2(T nSourceX, T nSourceY, T nDestX, T nDestY)
{
	T nDistance2 = 
        (nSourceX - nDestX) * (nSourceX - nDestX) + 
        (nSourceY - nDestY) * (nSourceY - nDestY);

	return nDistance2;
}

int g_GetDirection(int nX, int nY);

template<typename T>
inline T g_GetDirection(T nSourceX, T nSourceY, T nDestX, T nDestY)
{
    return (T)g_GetDirection((int)(nDestX - nSourceX), (int)(nDestY - nSourceY));
}

inline DWORD g_Dice(void)
{
	return g_Random(DICE_MAX - DICE_MIN + 1) + DICE_MIN;
}

#define SIN_COS_NUMBER	4096

int g_Sin(int nDirection);

inline int g_Cos(int nDirection)
{
    return g_Sin(DIRECTION_COUNT / 4 - nDirection);
}

inline BOOL g_RandPercent(int nPercent)
{
	return ((int)g_Random(100) < nPercent);
}

inline BOOL g_InRange(int nXa, int nYa, int nZa, int nXb, int nYb, int nZb, int nRange)
{
    BOOL        bResult     = false;
    long long   llDistance  = 0;
    long long   llRange     = (long long)nRange * nRange;

    llDistance = g_GetDistance3(
        (long long)nXa, (long long)nYa, (long long)nZa, 
        (long long)nXb, (long long)nYb, (long long)nZb
    );

    KG_PROCESS_ERROR(llDistance <= llRange);

    bResult = true;
Exit0:
    return bResult;
}

enum IN_RANGE_RESULT
{
	irrInvalid = 0,

    irrInRange,
    irrTooClose,
    irrTooFar,

	irrTotal
};

BOOL  IsLineSegmentCross(POINT pFirst1, POINT pFirst2, POINT pSecond1, POINT pSecond2);
POINT GetCrossPoint(POINT p1, POINT p2, POINT q1, POINT q2);

template<class T>
inline void MakeInRange(T& inputValue,  const T& minValue, const T& maxValue)
{
    assert(minValue <= maxValue);

    if (inputValue < minValue)
    {
        inputValue = minValue;
    }
    else if (inputValue > maxValue)
    {
        inputValue = maxValue;
    }
}

template<class T>
inline BOOL g_IsInRange(const T& inputValue,  const T& minValue, const T& maxValue)
{
    assert(minValue <= maxValue);

    return inputValue > minValue && inputValue < maxValue;
}
#endif //KMathH
