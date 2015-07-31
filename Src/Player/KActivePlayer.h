// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KActivePlayer.h
//  Creator		: zhoukezhen 
//	Date		: 12/11/2012
//  Comment		: 
//	*********************************************************************

#pragma once
#include "Engine/KMemory.h"
#include "Protocol/gs2cl_protocol.h"
#include "game_define.h"
#include <vector>
#include <map>

class KPlayer;

typedef std::vector<KActiveProcess> KVEC_ACTIVE_PROC;

class KActivePlayer
{
public:
    KActivePlayer(void);
    ~KActivePlayer(void);

    BOOL Init(KPlayer* pPlayer);
    void UnInit();
    void Activate();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    void OnEvent(int nEvent);
    
    void Refresh();
    BOOL ApplyAward(DWORD dwAwardID);
    int GetOnlineTime(){return m_nOnlineFrame;}
    void SetOnlineTime(int nOnlineTime){m_nOnlineFrame = nOnlineTime;}
    void AddCostFatiguePoint(int nCostFatiguePoint);
    void AddActivePoint(DWORD dwActivePoint);
    void SyncActivePlayer();
private:
    void FinishActivePoint(DWORD dwActiveID);
    BOOL UpdateActiveValue(DWORD dwActiveID);
    void CallReportNotSign();
 
private:
    KPlayer* m_pPlayer;
    KVEC_ACTIVE_PROC m_vecProcActive;
    int m_nCurActivePoint;
    std::set<DWORD> m_setAwardAlreadyGet;
    int m_nOnlineFrame;
    int m_nCostFatiguePoint;
};

