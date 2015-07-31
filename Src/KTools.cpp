#include "stdafx.h"
#include "KTools.h"
#include "KSO3World.h"

#include <math.h>

KVELOCITY GetBlowupVelocity(KPOSITION& rSrc, KPOSITION& rDest, int nBlowupRange, int nBlowupVelocity)
{
    assert(nBlowupRange > 0);
    KVELOCITY result = {0};
    KPOINT3D distance3d = {rDest.nX - rSrc.nX, rDest.nY - rSrc.nY, rDest.nZ - rSrc.nZ};

    if (!distance3d.x && !distance3d.y)
    {
        result.nVelocityX = 0;
        result.nVelocityY = 0;
        result.nVelocityZ = nBlowupVelocity * (nBlowupRange - distance3d.z) / nBlowupRange;

        goto Exit0;
    }
    else
    {
        double   fDis = sqrt((double)distance3d.x * distance3d.x + distance3d.y * distance3d.y + distance3d.z * distance3d.z);
        double   fVxyz = nBlowupVelocity * (nBlowupRange - fDis) / nBlowupRange;
        double   fDisXY = sqrt((double)distance3d.x * distance3d.x + distance3d.y * distance3d.y);
        double   fVxy = fVxyz * fDisXY / fDis;
        assert(fVxyz > 0);

        result.nVelocityZ = (int)(fVxyz * distance3d.z / fDis);
        result.nVelocityX = (int)(fVxy * distance3d.x / fDisXY);
        result.nVelocityY = (int)(fVxy * distance3d.y / fDisXY);
    }

Exit0:
    return result;
}

time_t g_GetNextCycleTime(int nCycle, int nOffset)
{
    time_t  nResult         = 0;
    time_t  nGSCurrentTime  = g_pSO3World->m_nCurrentTime;

    nOffset += timezone; // 周期偏移转为UTC时间
    nResult = nGSCurrentTime - (nGSCurrentTime - nOffset) % nCycle + nCycle;

    return nResult;
}

BOOL g_StringToTime(const char* szTimeString, int& nTime)
{
	BOOL bResult = false;
    int nRetCode = 0;
    int nYear = 0;
    int nMon = 0;
    int nDay = 0;
    int nHour = 0;
    int nMin = 0;
    int nSec = 0;
    tm sTime;
	
    KGLOG_PROCESS_ERROR(szTimeString);

    nRetCode = sscanf(szTimeString, "%d-%d-%d-%d-%d-%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec);
    KGLOG_PROCESS_ERROR(nRetCode == 6);

    memset(&sTime, 0, sizeof(sTime));
    sTime.tm_year = nYear - 1900;
    sTime.tm_mon  = nMon - 1;
    sTime.tm_mday = nDay;
    sTime.tm_hour = nHour;
    sTime.tm_min  = nMin;
    sTime.tm_sec  = nSec;
    nTime = mktime(&sTime);

	bResult = true;
Exit0:
	return bResult;
}

BOOL g_StringToPosition(const char* szData, KPOSITION& cPos)
{
	BOOL    bResult = false;
	int     nRet    = 0;
	
    KGLOG_PROCESS_ERROR(szData);

    nRet = sscanf(szData, "%d,%d,%d", &cPos.nX, &cPos.nY, &cPos.nZ);
	KGLOG_PROCESS_ERROR(nRet == 3);

	bResult = true;
Exit0:
	return bResult;
}

BOOL g_StringToItemInfo(const char* szData, DWORD& dwItemType, DWORD& dwItemIndex, int& nCount)
{
    BOOL bResult = false;
    int  nRet    = 0;

    KGLOG_PROCESS_ERROR(szData);

    nRet = sscanf(szData, "%u,%u,%d", &dwItemType, &dwItemIndex, &nCount);
    KGLOG_PROCESS_ERROR(nRet == 3);

    bResult = true;
Exit0:
    return bResult;
}

BOOL g_ReadStringFromBuffer(BYTE*& pBuffer, size_t& uBufferSize, char* pszRet, size_t uStringLen)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pBuffer);
    KGLOG_PROCESS_ERROR(pszRet);
    KGLOG_PROCESS_ERROR(uBufferSize >= uStringLen);

    memcpy(pszRet, pBuffer, uStringLen);

    pBuffer     += uStringLen;
    uBufferSize -= uStringLen;

    bResult = true;
Exit0:
    return bResult;
}

BOOL g_DoBufferSkip(BYTE*& pBuffer, size_t& uBufferSize, size_t uSkipSize)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	KGLOG_PROCESS_ERROR(pBuffer);
    KGLOG_PROCESS_ERROR(uBufferSize >= uSkipSize);

    pBuffer     += uSkipSize;
    uBufferSize -= uSkipSize;

	bResult = true;
Exit0:
	return bResult;
}

BOOL g_GetTimeFromString(char* pszStr, int nStrLen, uint32_t& dwDestValue)
{
    BOOL    bResult                     = false;
    int     nRetCode                    = false;
    struct  tm TimeData;

    KGLOG_PROCESS_ERROR(pszStr);
    KGLOG_PROCESS_ERROR(nStrLen > 0 && nStrLen < _NAME_LEN);

    memset(&TimeData, 0, sizeof(TimeData));

    nRetCode = sscanf(pszStr, "%d-%d-%d-%d", &TimeData.tm_year, &TimeData.tm_mon, &TimeData.tm_mday, &TimeData.tm_hour);
    KGLOG_PROCESS_ERROR(nRetCode == 4);

    TimeData.tm_year -= 1900; 
    TimeData.tm_mon  -= 1;
    KGLOG_PROCESS_ERROR(TimeData.tm_mon >= 0);

    dwDestValue = mktime(&TimeData);
    KGLOG_PROCESS_ERROR(dwDestValue >= 0);

    bResult = true;
Exit0:
    return bResult;
}

BOOL g_TimeToDayIndexOfMonth(time_t nTime, int& nDayIndex)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	struct tm* pTime = NULL;

    pTime = localtime(&nTime);
    KGLOG_PROCESS_ERROR(pTime);
	
    nDayIndex = pTime->tm_mday;

	bResult = true;
Exit0:
	return bResult;
}

BOOL g_IsSameMonth(time_t nTimeA, time_t nTimeB)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int nYearA = 0;
    int nMonthA = 0;
    int nYearB = 0;
    int nMonthB = 0;
	struct tm* pTime = NULL;

    pTime = localtime(&nTimeA);
    KGLOG_PROCESS_ERROR(pTime);
    nYearA = pTime->tm_year;
    nMonthA = pTime->tm_mon;

    pTime = localtime(&nTimeB);
    KGLOG_PROCESS_ERROR(pTime);
    nYearB = pTime->tm_year;
    nMonthB = pTime->tm_mon;

    bResult = (nYearA == nYearB && nMonthA == nMonthB);
Exit0:
	return bResult;
}

BOOL g_IsSameWeek(time_t nTimeA, time_t nTimeB)
{
    BOOL bResult  = false;
    int nWeekDayA = 0;
    int nWeekDayB = 0;
    struct tm* pTime = NULL;

    KG_PROCESS_ERROR(abs(nTimeB - nTimeA) / SECONDS_PER_WEEK == 0);

    pTime = localtime(&nTimeA);
    KGLOG_PROCESS_ERROR(pTime);
    nWeekDayA = pTime->tm_wday;

    pTime = localtime(&nTimeB);
    KGLOG_PROCESS_ERROR(pTime);
    nWeekDayB = pTime->tm_wday;

    bResult = ((nTimeA - nTimeB) * (nWeekDayA - nWeekDayB) >= 0);    
Exit0:
    return bResult;
}

BOOL g_IsSameDay(time_t nTimeA, time_t nTimeB)
{
    BOOL bResult  = false;
    struct tm* pTime = NULL;
    struct tm tmA;
    struct tm tmB;

    KG_PROCESS_ERROR(abs(nTimeB - nTimeA) / SECONDS_PER_WEEK == 0);

    pTime = localtime(&nTimeA);
    KGLOG_PROCESS_ERROR(pTime);
    tmA = *pTime; 

    pTime = localtime(&nTimeB);
    KGLOG_PROCESS_ERROR(pTime);
    tmB = *pTime;

    KG_PROCESS_ERROR(tmA.tm_year == tmA.tm_year);
    KG_PROCESS_ERROR(tmA.tm_yday == tmA.tm_yday);

Exit0:
    return bResult;
}

int g_RandomBySeed(unsigned uSeed, int nMinValue, int nMaxValue)
{
    int nRetValue = nMinValue;
    int nDeltaValue = nMaxValue - nMinValue;
    unsigned uOldSeed = g_GetRandomSeed();

    assert(nMinValue <= nMaxValue);
    
    if (nMinValue < nMaxValue)
    {
        g_RandomSeed(uSeed);
        nRetValue += g_Random(nMaxValue - nMinValue);
        g_RandomSeed(uOldSeed);
    }

    return nRetValue;
}

BOOL g_ParseMutiInteger(char szData[], const char* cszDelim, std::vector<int>& vecResult)
{
	BOOL bResult = false;
    int nRetCode = 0;
    char* pszValue = NULL;
    int nValue = 0;

	KGLOG_PROCESS_ERROR(szData);
    KGLOG_PROCESS_ERROR(cszDelim);

    pszValue = strtok(szData, cszDelim);
    while(pszValue)
    {
        nRetCode = sscanf(pszValue, "%d", &nValue);
        KGLOG_PROCESS_ERROR(nRetCode == 1);

        vecResult.push_back(nValue);

        pszValue = strtok(NULL, cszDelim);
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL g_ReadCharFromCString(char*& pszString, char& cRet)
{
	BOOL bResult = false;

	KGLOG_PROCESS_ERROR(pszString);
    KGLOG_PROCESS_ERROR(pszString[0]);

    cRet = pszString[0];

    ++pszString;

	bResult = true;
Exit0:
	return bResult;
}

BOOL g_ReadIntegerFromCString(char*& pszString, int& nRet)
{
	BOOL bResult = false;
    unsigned uCur = 0;
    int nPlusMinus = 1;

    KGLOG_PROCESS_ERROR(pszString);
    KGLOG_PROCESS_ERROR(pszString[0]);
	
    KGLOG_PROCESS_ERROR(pszString[0] == '+' || pszString[0] == '-' || pszString[0] >= '0' && pszString[0] <= '9');

    nRet = 0;
    if (pszString[0] == '-')
    {
        nPlusMinus = -1;
        uCur = 1;
    }
    else if (pszString[0] == '+')
    {
        uCur = 1;
    }

    for (NULL; pszString[uCur]; ++uCur)
    {
        if (pszString[uCur] < '0' || pszString[uCur] > '9')
        {
            break;
        }

        nRet = nRet * 10 + pszString[uCur] - '0';
    }
    nRet *= nPlusMinus;

    pszString += uCur;

	bResult = true;
Exit0:
	return bResult;
}

BOOL g_CStringToInteger(char szData[], int& nRetValue)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    int nPlusMinus = 1;
    int nCur = 0;

	KGLOG_PROCESS_ERROR(szData);
    KGLOG_PROCESS_ERROR(szData[0]);

    if (szData[nCur] == '+')
    {
        ++nCur;
        nPlusMinus = 1;
    }
    else if (szData[nCur] == '-')
    {
        ++nCur;
        nPlusMinus = -1;
    }

    nRetValue = 0;
    for (NULL; szData[nCur]; ++nCur)
    {
        KGLOG_PROCESS_ERROR(szData[nCur] >= '0' && szData[nCur] <= '9');
        nRetValue = nRetValue * 10 + szData[nCur] - '0';
    }

    nRetValue *= nPlusMinus;

	bResult = true;
Exit0:
	return bResult;
}
