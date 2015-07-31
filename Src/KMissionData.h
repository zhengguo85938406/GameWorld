// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KMissionData.h 
//  Creator 	: Xiayong  
//  Date		: 05/15/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "Engine/KMemory.h"
class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
}
struct KOneMission 
{
    int nCurrentOpenedLevel;
    int nFinishedLevel;
};

typedef std::map<uint32_t, KOneMission, std::less<int>, KMemory::KAllocator<std::pair<uint32_t, KOneMission> > >  KMISSION_DATA_MAP;

class KMissionData
{
public:
    KMissionData();
    ~KMissionData();

    BOOL    Init(KPlayer* pPlayer);
    void    UnInit();

    BOOL    LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL    SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL    OpenMission(int nType, int nStep, int nLevel);
    BOOL    FinishMission(int eType, int nStep, int nLevel);
    BOOL    IsMissionOpened(int eType, int nStep, int nLevel);

private:
    KPlayer*    m_pPlayer;
    KMISSION_DATA_MAP m_Data;
};
