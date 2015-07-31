// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KMissionMgr.h 
//  Creator 	: Xiayong  
//  Date		: 04/28/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include <map>
#include "KTabConfig.h"
#include "Engine/KMemory.h"



struct KMission
{
    KENUM_MISSION_TYPE  eType;
    int                 nStep;              // 关卡阶段
    int                 nLevel;             // 关卡难度
    char                szName[_NAME_LEN];  
    uint32_t            dwMapID;    
    int                 nRequireLevel;
    int                 nExpLevel;
    KAwardData          stAwardData;
    int                 nBaseExp;
    int                 nBaseMoney;
    DWORD               dwScriptID;
    BOOL                bUseAICoe;
    BOOL                bAffectAICoe;
    int                 nCostFatiguePoint;
	int					nUnenableReady;
	int					nCanPause;
    DWORD               dwRequireQuest;

    struct KNpcInfo
    {
        DWORD       dwTemplateID;
        int         nAIType;
        int         nSide;
        KPOSITION   cPos;
    } Npc[MAX_NPC_IN_MISSION];
};

typedef KMemory::KAllocator<std::pair<uint64_t, KMission> > KMISSION_TABLE_ALLOCTOR;
typedef std::map<uint64_t, KMission, std::less<uint64_t>,  KMISSION_TABLE_ALLOCTOR>  KMIOSSION_TABLE;

class KPlayer;
class KMissionMgr
{
public:
    KMissionMgr();
    ~KMissionMgr();

    BOOL        Init();
    void        UnInit();
    BOOL        CanEnterMission(KPlayer* pPlayer, int eType, int nStep, int nLevel);
    uint32_t    GetMissionMap(int eType, int nStep, int nLevel);
    KMission*   GetMission(int eType, int nStep, int nLevel);
    KMission*   GetMission(uint32_t dwMapID);
    uint64_t    GetMissionKey(int eType, int nStep, int nLevel);
    
    BOOL        IsTrainingMission(uint32_t dwMapID);

private:
    BOOL        Load();

private:
    KMIOSSION_TABLE m_mapMissions;
};
