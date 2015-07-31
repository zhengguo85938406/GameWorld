// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KSecretary.h
//  Creator		: hanruofei
//	Date		: 9/19/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include "game_define.h"

#include <vector>
#include <list>

typedef std::vector<BYTE> KREPORT_PARAM;

struct KREPORT_ITEM
{
    int           nReportIndex;
    int           nReportTime;
    int           nReportEvent;
    BOOL          bNewReport;
    KREPORT_PARAM Param;
};

typedef std::list<KREPORT_ITEM> KLIST_REPORT;

class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
    class KPB_REPORT_ITEM;
}

class KSecretary
{
public:
    KSecretary();
    ~KSecretary();

    BOOL Init(KPlayer* pOwner);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL AddReport(int nReportEvent, BYTE* pParamBegin, size_t uParamSize);
    BOOL OnItemRead(int nReportIndex);

    BOOL m_bLoadComplete;

private:
    BOOL LoadItemFromProtoBuf(const T3DB::KPB_REPORT_ITEM* pReportItem);
    KREPORT_ITEM* GetItem(int nReportIndex);
    void SyncAllReportItem();
    BOOL GetNextValidIndex(int& nReportIndex);

private:
    KLIST_REPORT    m_lstReport;
    KPlayer*        m_pOwner;
    BOOL            m_IndexUsed[cdMaxReportCount];
};

