#include "stdafx.h"
#include "KSecretary.h"
#include "KRoleDBDataDef.h"
#include "KPlayer.h"
#include "KPlayerServer.h"
#include "KSO3World.h"

static size_t gs_AllSize[KREPORT_EVENT_END] = {0};

KSecretary::KSecretary()
{
    m_pOwner = NULL;
    m_bLoadComplete = false;
    memset(m_IndexUsed, 0, sizeof(m_IndexUsed));

    gs_AllSize[KREPORT_EVENT_HERO_LEVELUP] = sizeof(KReport_Hero_LevelUP);
    gs_AllSize[KREPORT_EVENT_TEAM_LEVELUP] = sizeof(KReport_Team_LevelUp);
    gs_AllSize[KREPORT_EVENT_VIP_LEVELUP] = sizeof(KReport_VIP_LevelUp);
    gs_AllSize[KREPORT_EVENT_HERO_FINISH_BASKETBALL_TRAINING] = sizeof(KReport_Hero_Finish_Basketball_Training);
    gs_AllSize[KREPORT_EVENT_HERO_FINISH_WEIGHTLIFTING_TRAINING] = sizeof(KReport_Hero_Finish_WeightLifting_Training);
    gs_AllSize[KREPORT_EVENT_HERO_FINISH_RUNNING_TRAINING] = sizeof(KReport_Hero_Finish_Running_Training);
    gs_AllSize[KREPORT_EVENT_HERO_FINISH_BOXING_TRAINING] = sizeof(KReport_Hero_Finish_Boxing_Training);
    gs_AllSize[KREPORT_EVENT_AUTOREPAIR_FAILED] = 0;
    gs_AllSize[KREPORT_EVENT_MONEY_OUT_OF_LIMIT] = 0;
    gs_AllSize[KREPORT_EVENT_ZERO_FATIGUE_POINT] = 0;
    gs_AllSize[KREPORT_EVENT_AUTOCOLLECT_PROFITS] = sizeof(KReport_Autocollect_Profits);
    gs_AllSize[KREPORT_EVENT_NEW_HERO] = sizeof(KReport_New_Hero);
    gs_AllSize[KREPORT_EVENT_RECEIVE_PRESENTATION_VIP] = sizeof(KReport_Receive_Presentation_VIP);
    gs_AllSize[KREPORT_EVENT_OPEN_OR_CONTINUE_VIP] = sizeof(KReport_Open_Or_Continue_VIP);
    gs_AllSize[KREPORT_EVENT_RECHARGE] =  sizeof(KReport_Recharge);
    gs_AllSize[KREPORT_EVENT_ENTER_CLUB] = sizeof(KReport_Enter_Club);
    gs_AllSize[KREPORT_EVENT_LEAVE_CLUB] = 0;
    gs_AllSize[KREPORT_EVENT_VIP_TIMEOUT] = 0;
    gs_AllSize[KREPORT_EVENT_CHARGE_VIP_TIME] = sizeof(KReport_Charge_VIP_Time);
    gs_AllSize[KREPORT_EVENT_CANCOLLECT_PROFITS] = 0;
    gs_AllSize[KREPORT_EVENT_BEADDED_FRIEND]     = sizeof(KReport_BeAdded_Friend);
    gs_AllSize[KREPORT_EVNET_ACTIVEPLAYER_AWARD] = 0;
    gs_AllSize[KREPORT_EVNET_CAN_GET_SIGN_AWARD] = 0;
    gs_AllSize[KREPORT_EVNET_NOT_SIGN] = 0;
    gs_AllSize[KREPORT_EVENT_LEVELUP_AWARD] = sizeof(KReport_LevelUp_Award);
    gs_AllSize[KREPORT_EVENT_ONLINE_AWARD] = 0;
    gs_AllSize[KREPORT_EVENT_FASHION_TIMEOUT] = sizeof(KReport_Fasion_TimeOut);
    gs_AllSize[KREPORT_EVENT_CHEERLEADING_TIMEOUT] = sizeof(KReport_Cheerleading_TimeOut);
    gs_AllSize[KREPORT_EVENT_HERO_UNLOCK] = sizeof(KReport_Hero_UnLock);
    gs_AllSize[KREPORT_EVENT_BUFF_END] = sizeof(KReport_Buff_End);
    gs_AllSize[KREPORT_EVENT_NEW_MAIL] = 0;
    gs_AllSize[KREPORT_EVENT_NEW_GIFT] = sizeof(KReport_New_Gift);
    gs_AllSize[KREPORT_EVENT_MAIL_TOO_MANY] = 0;
}

KSecretary::~KSecretary()
{
    assert(!m_pOwner);
}


BOOL KSecretary::Init(KPlayer* pOwner)
{
    BOOL bResult = false;

    KGLOG_PROCESS_ERROR(pOwner && !m_pOwner);

    m_pOwner = pOwner;

    bResult = true;
Exit0:
    return bResult;
}

void KSecretary::UnInit()
{
    m_lstReport.clear();
    m_pOwner = NULL;
}

BOOL KSecretary::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    assert(pLoadBuf);

    const T3DB::KPBSecretary&  Secretary  = pLoadBuf->secretary();

    for (int i = 0; i < Secretary.reportitem_size(); ++i)
    {
        if (m_lstReport.size() > cdMaxReportCount)
            break;

        const T3DB::KPB_REPORT_ITEM&  ReportItem = Secretary.reportitem(i);
        bRetCode = LoadItemFromProtoBuf(&ReportItem);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    if (m_pOwner->m_eConnectType == eLogin)
    {
        SyncAllReportItem();
    }

    m_bLoadComplete = true;

    bResult = true;
Exit0:
    return bResult;   
}

BOOL KSecretary::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL                bResult         = false;
    T3DB::KPBSecretary*   pSecretary      = NULL;
    T3DB::KPB_REPORT_ITEM*  pReportItem = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pSecretary = pSaveBuf->mutable_secretary();
    
    for (KLIST_REPORT::iterator it = m_lstReport.begin(); it != m_lstReport.end(); ++it)
    {
        pReportItem = pSecretary->add_reportitem();
        pReportItem->set_reportevent(it->nReportEvent);
        pReportItem->set_isnewreport(it->bNewReport);
        pReportItem->set_reportindex(it->nReportIndex);
        pReportItem->set_reporttime(it->nReportTime);
        pReportItem->set_reportparam(it->Param.data(), it->Param.size());
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KSecretary::LoadItemFromProtoBuf(const T3DB::KPB_REPORT_ITEM* pPBReportItem)
{
    BOOL    bResult     = false;
    KREPORT_ITEM*       pReportItem = NULL;
    BYTE* pbyData       = NULL;

    assert(pPBReportItem);

    const std::string& strParam = pPBReportItem->reportparam();
    pbyData = (BYTE*)strParam.data();

    KGLOG_PROCESS_ERROR(m_lstReport.size() <= cdMaxReportCount);

    if (m_lstReport.size() == cdMaxReportCount)
        m_lstReport.pop_front();

    m_lstReport.resize(m_lstReport.size() + 1);
    pReportItem = &m_lstReport.back();

    pReportItem->nReportIndex   = pPBReportItem->reportindex();
    pReportItem->nReportEvent   = pPBReportItem->reportevent();
    pReportItem->nReportTime    = pPBReportItem->reporttime();
    pReportItem->bNewReport     = pPBReportItem->isnewreport();

    pReportItem->Param.insert(pReportItem->Param.begin(), pbyData, pbyData + strParam.size());

    m_IndexUsed[pReportItem->nReportIndex] = true;

    bResult = true;
Exit0:
    return bResult;
}

BOOL KSecretary::AddReport(int nReportEvent, BYTE* pParamBegin, size_t uParamSize)
{
    BOOL bResult        = false;
    BOOL bRetCode       = false;
    int  nReportIndex   = 0;
    KREPORT_ITEM* pReportItem = NULL;

    KG_PROCESS_ERROR(m_bLoadComplete);
    KGLOG_PROCESS_ERROR(nReportEvent > KREPORT_EVENT_BEGIN && nReportEvent <= KREPORT_EVENT_END);
    KGLOG_PROCESS_ERROR(pParamBegin || uParamSize == 0);
    KGLOG_PROCESS_ERROR(uParamSize == gs_AllSize[nReportEvent]);
    KGLOG_PROCESS_ERROR(m_lstReport.size() <= cdMaxReportCount);

    if (m_lstReport.size() == cdMaxReportCount)
    {
        KREPORT_ITEM* pOldestReport = &m_lstReport.front();
        g_PlayerServer.DoRemoveReportItem(m_pOwner, pOldestReport);
        m_IndexUsed[pOldestReport->nReportIndex] = false;

        m_lstReport.pop_front();
    }

    bRetCode = GetNextValidIndex(nReportIndex);
    KGLOG_PROCESS_ERROR(bRetCode);

    m_lstReport.resize(m_lstReport.size() + 1);
    pReportItem = &m_lstReport.back();

    pReportItem->nReportIndex   = nReportIndex;
    pReportItem->nReportEvent   = nReportEvent;
    pReportItem->nReportTime    = g_pSO3World->m_nCurrentTime;
    pReportItem->bNewReport     = true;

    m_IndexUsed[nReportIndex] = true;

    if (pParamBegin && uParamSize)
        pReportItem->Param.insert(pReportItem->Param.begin(), pParamBegin, pParamBegin + uParamSize);

    g_PlayerServer.DoSyncReportItem(m_pOwner, pReportItem);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KSecretary::OnItemRead(int nReportIndex)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    KREPORT_ITEM* pReportItem = NULL;

    pReportItem = GetItem(nReportIndex);
    KGLOG_PROCESS_ERROR(pReportItem);

    if (pReportItem->bNewReport)
    {
        pReportItem->bNewReport = false;
        m_pOwner->OnEvent(peViewReport, pReportItem->nReportEvent);
    }


    bResult = true;
Exit0:
    return bResult;
}

KREPORT_ITEM* KSecretary::GetItem(int nReportIndex)
{
    KREPORT_ITEM* pReportItem = NULL;

    for (KLIST_REPORT::iterator it = m_lstReport.begin(); it != m_lstReport.end(); ++it)
    {
        if (nReportIndex == it->nReportIndex)
        {
            pReportItem = &(*it);
            break;
        }
    }

    return pReportItem;
}

void KSecretary::SyncAllReportItem()
{
    for (KLIST_REPORT::iterator it = m_lstReport.begin(); it != m_lstReport.end(); ++it)
    {
        g_PlayerServer.DoSyncReportItem(m_pOwner, &(*it));
    }
}

BOOL KSecretary::GetNextValidIndex(int& nReportIndex)
{
    BOOL bResult = false;

    for (size_t i = 0; i < countof(m_IndexUsed); ++i)
    {
        if (!m_IndexUsed[i])
        {
            nReportIndex = i;
            bResult = true;
            break;
        }
    }

    return bResult;
}


