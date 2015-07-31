#include "stdafx.h"
#include "KMissionData.h"
#include "KRoleDBDataDef.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KSO3World.h"

KMissionData::KMissionData()
{
}

KMissionData::~KMissionData()
{
}

BOOL KMissionData::Init(KPlayer* pPlayer)
{
    m_pPlayer = pPlayer;
    return true;
}

void KMissionData::UnInit()
{
}

BOOL KMissionData::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL                 bResult         = false;

    assert(pLoadBuf);

    const T3DB::KPBMissionData&   Mission        = pLoadBuf->missiondata();
    const T3DB::KPB_MISSION_DATA& MissionData    = Mission.missiondata();

    for (int i = 0; i < MissionData.missions_size(); ++i)
    {
        const T3DB::KPB_ONE_MISSION_DATA& OneMissionData = MissionData.missions(i);
        KOneMission OneMission = {OneMissionData.openedlevel(),OneMissionData.finishedlevel()};

        m_Data[MAKE_INT32(OneMissionData.type(), OneMissionData.step())] = OneMission;
    }

    if (m_pPlayer->m_eConnectType == eLogin)
    {
        g_PlayerServer.DoSyncMissionData(m_pPlayer->m_nConnIndex, &MissionData);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMissionData::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                    bResult         = false;
    T3DB::KPBMissionData*     pMission        = NULL;
    T3DB::KPB_MISSION_DATA* pMissionData    = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pMission = pSaveBuf->mutable_missiondata();
    pMissionData = pMission->mutable_missiondata();

    for (KMISSION_DATA_MAP::iterator it = m_Data.begin(); it != m_Data.end(); ++it)
    {
        T3DB::KPB_ONE_MISSION_DATA* pData = pMissionData->add_missions();
        pData->set_step(LOW_DWORD_IN_UINT32(it->first));
        pData->set_type(HIGH_DWORD_IN_UINT32(it->first));
        pData->set_finishedlevel(it->second.nFinishedLevel);
        pData->set_openedlevel(it->second.nCurrentOpenedLevel);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMissionData::OpenMission(int nType, int nStep, int nLevel)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KMission* pMission = NULL;
    KMISSION_DATA_MAP::iterator it;

    pMission = g_pSO3World->m_MissionMgr.GetMission(nType, nStep, nLevel);
    KG_PROCESS_ERROR(pMission);

    KGLOG_PROCESS_ERROR(pMission->eType >= emitNewbie && pMission->eType < emitTotal);

    it = m_Data.find(MAKE_INT32(nType, nStep));
    if (it == m_Data.end())
    {
        KOneMission OneMission = {nLevel, 0};
        m_Data[MAKE_INT32(nType, nStep)] = OneMission;
        g_PlayerServer.DoSyncMissionOpened(m_pPlayer->m_nConnIndex, nType, nStep, nLevel);
        goto Exit1;        
    }

    if (it->second.nCurrentOpenedLevel < nLevel)
    {
        it->second.nCurrentOpenedLevel = nLevel;
        g_PlayerServer.DoSyncMissionOpened(m_pPlayer->m_nConnIndex, nType, nStep, nLevel);
    }

Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL KMissionData::FinishMission(int nType, int nStep, int nLevel)
{
	BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KMission* pMission = NULL;
    KMISSION_DATA_MAP::iterator it;

    bRetCode = IsMissionOpened(nType, nStep, nLevel);
    KGLOG_PROCESS_ERROR(bRetCode);

    pMission = g_pSO3World->m_MissionMgr.GetMission(nType, nStep, nLevel);
    KGLOG_PROCESS_ERROR(pMission);

    KGLOG_PROCESS_ERROR(pMission->eType >= emitNewbie && pMission->eType < emitTotal);

    it = m_Data.find(MAKE_INT32(nType, nStep));
    KGLOG_PROCESS_ERROR(it != m_Data.end());

    KG_PROCESS_ERROR(it->second.nFinishedLevel < nLevel);

    it->second.nFinishedLevel = nLevel;

    g_PlayerServer.DoSyncMissionFinished(m_pPlayer->m_nConnIndex, nType, nStep, nLevel);

    OpenMission(nType, nStep, nLevel + 1); // 开启下一个难度

    if (nType == 2 && nStep == 1 && nLevel == 3) // 热身赛要第三关打完开下一关
        OpenMission(nType, nStep + 1, 1);
    else if (nLevel == 1) // 简单难度则打开下一关
        OpenMission(nType, nStep + 1, 1);
    
	bResult = true;
Exit0:
	return bResult;
}

BOOL KMissionData::IsMissionOpened(int nType, int nStep, int nLevel)
{
    BOOL bResult 	= false;
	BOOL bRetCode 	= false;
    KMission* pMission = NULL;
    KMISSION_DATA_MAP::iterator it;

    pMission = g_pSO3World->m_MissionMgr.GetMission(nType, nStep, nLevel);
    KGLOG_PROCESS_ERROR(pMission);

    KGLOG_PROCESS_ERROR(pMission->eType >= emitNewbie && pMission->eType < emitTotal);
    
    KG_PROCESS_SUCCESS(pMission->eType == emitPVE1ContinuousChallenge || pMission->eType == emitPVE2ContinuousChallenge);

    it = m_Data.find(MAKE_INT32(nType, nStep));
    KG_PROCESS_ERROR(it != m_Data.end());
    KG_PROCESS_ERROR(it->second.nCurrentOpenedLevel >= nLevel);

Exit1:
	bResult = true;
Exit0:
	return bResult;
}
