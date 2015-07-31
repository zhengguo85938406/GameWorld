#include "stdafx.h"
#include "KChallengeMissionNpc.h"

BOOL KChallengeMissionNpc::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

    bRetCode = m_Data1.Load("Challenge1MissionNpc.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Data2.Load("Challenge2MissionNpc.tab");
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KChallengeMissionNpc::UnInit()
{
}

BOOL KChallengeMissionNpc::GetTwoRandomNpc(int nType, int nStep, int& nNPCID1, int& nNPCID2)
{
    BOOL    bResult = false;
    int     nRandomNum = 0;
    int     nValidNpcCount = 0;
    KChallengeMissionNpcData* pData = NULL;

    if (nType == emitPVE1ContinuousChallenge)
    {
        pData = m_Data1.GetByID(nStep);
        KGLOG_PROCESS_ERROR(pData);
    }
    else if (nType == emitPVE2ContinuousChallenge)
    {
        pData = m_Data2.GetByID(nStep);
        KGLOG_PROCESS_ERROR(pData);
    }
    else
    {
        pData = m_Data1.GetByID(nStep);
        KGLOG_PROCESS_ERROR(pData);
    }
    
    nValidNpcCount = pData->vecValidNpc.size();
    if (nValidNpcCount == 0)
    {
        nNPCID1 = 0;
        nNPCID2 = 0;
    }
    else if (nValidNpcCount == 1)
    {
        nNPCID1 = pData->vecValidNpc[0];
        nNPCID2 = 0;
    }
    else if (nValidNpcCount == 2)
    {
        nNPCID1 = pData->vecValidNpc[0];
        nNPCID2 = pData->vecValidNpc[1];
    }
    else
    {
        nRandomNum = g_Random(MILLION_NUM) % nValidNpcCount;
        nNPCID1 = pData->vecValidNpc[nRandomNum];

        nRandomNum = g_Random(MILLION_NUM) % nValidNpcCount;
        nNPCID2 = pData->vecValidNpc[nRandomNum];
    }
    
    bResult = true;
Exit0:
    return bResult;
}
