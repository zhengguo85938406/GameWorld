#include "stdafx.h"
#include "KPlayerValue.h"
#include "KWorldSettings.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KPlayerServer.h"


KPlayerValue::KPlayerValue(void)
{
    memset(m_Data, 0, sizeof(m_Data));
    m_pPlayer = NULL;
}


KPlayerValue::~KPlayerValue(void)
{
}

BOOL KPlayerValue::Init(KPlayer* pPlayer)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pPlayer);
    KGLOG_PROCESS_ERROR(m_pPlayer == NULL);

    m_pPlayer = pPlayer;

    bResult = true;
Exit0:
    return bResult;
}

void KPlayerValue::UnInit()
{
    m_pPlayer = NULL;
}

BOOL KPlayerValue::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    size_t uMaxSize = 0;
	
	KGLOG_PROCESS_ERROR(pLoadBuf);

    {
        const T3DB::KPB_PLAYER_VALUE& PlayerValue = pLoadBuf->playervalue();
        const std::string& PlayerValueData = PlayerValue.playervaluedata();

        KGLOG_PROCESS_ERROR(PlayerValueData.size() <= sizeof(m_Data));

        memcpy(m_Data, PlayerValueData.data(), PlayerValueData.size());
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerValue::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    size_t uTotalBytes = 0;
    T3DB::KPB_PLAYER_VALUE* pPlayerValue = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pPlayerValue = pSaveBuf->mutable_playervalue();
    KGLOG_PROCESS_ERROR(pPlayerValue);

    uTotalBytes = g_pSO3World->m_Settings.m_PlayerValueInfoList.GetTotalBytes();
    KGLOG_PROCESS_ERROR(uTotalBytes <= sizeof(m_Data));

    pPlayerValue->set_playervaluedata((void*)m_Data, uTotalBytes);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerValue::GetValue(DWORD dwID, int& nValue)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    uint32_t* pCurUInt32 = NULL;
    size_t uBitOffsetInUInt32   = 0;
	KPlayerValueInfo* pPlayerValueInfo = NULL;

    pPlayerValueInfo = g_pSO3World->m_Settings.m_PlayerValueInfoList.GetPlayerValue(dwID);
    KGLOG_PROCESS_ERROR(pPlayerValueInfo);

    pCurUInt32          = m_Data + pPlayerValueInfo->uOffset / UINT32_BIT_COUNT;
    uBitOffsetInUInt32  = pPlayerValueInfo->uOffset % UINT32_BIT_COUNT;

    switch (pPlayerValueInfo->nType)
    {
    case KPLAYER_VALUE_TYPE_BOOLEAN:
        nValue = (*pCurUInt32 >> uBitOffsetInUInt32) & (uint32_t)1;
        break;
    case KPLAYER_VALUE_TYPE_INT32:
        nValue = (int)(*pCurUInt32);
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

	bResult = true;
Exit0:
	return bResult;
}

BOOL KPlayerValue::SetValue(DWORD dwID, int nNewValue)
{
    KPlayerValueInfo* pPlayerValueInfo = g_pSO3World->m_Settings.m_PlayerValueInfoList.GetPlayerValue(dwID);

    return SetValue(pPlayerValueInfo, nNewValue);
}

void KPlayerValue::Refresh(time_t nLastDailyRefreshTime)
{
    BOOL bRetCode = false;

    bRetCode = g_IsSameMonth(nLastDailyRefreshTime, g_pSO3World->m_nCurrentTime);
    if(!bRetCode)
    {
        bRetCode = MonthlyReset();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = g_IsSameWeek(nLastDailyRefreshTime, g_pSO3World->m_nCurrentTime);
    if (!bRetCode)
    {
        bRetCode = WeeklyReset();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = g_IsSameDay(nLastDailyRefreshTime, g_pSO3World->m_nCurrentTime);
    if (!bRetCode)
    {
        bRetCode = DailyReset();
        KGLOG_PROCESS_ERROR(bRetCode);
    }

Exit0:
    return;
}

BOOL KPlayerValue::DailyReset()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
    KVEC_DAILY_RESET& vecDailyReset = g_pSO3World->m_Settings.m_PlayerValueInfoList.GetDailyResetList();

    for (KVEC_DAILY_RESET::iterator it = vecDailyReset.begin(); it != vecDailyReset.end(); ++it)
    {
        bRetCode = SetValue(*it, 0);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerValue::WeeklyReset()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KVEC_WEEKLY_RESET& vecWeeklyReset = g_pSO3World->m_Settings.m_PlayerValueInfoList.GetWeeklyResetList();

    for (KVEC_WEEKLY_RESET::iterator it = vecWeeklyReset.begin(); it != vecWeeklyReset.end(); ++it)
    {
        bRetCode = SetValue(*it, 0);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KPlayerValue::MonthlyReset()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KVEC_MONTHLY_RESET& vecMonthlyReset = g_pSO3World->m_Settings.m_PlayerValueInfoList.GetMonthlyResetList();

    for (KVEC_MONTHLY_RESET::iterator it = vecMonthlyReset.begin(); it != vecMonthlyReset.end(); ++it)
    {
        bRetCode = SetValue(*it, 0);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}


BOOL KPlayerValue::SetValue(KPlayerValueInfo* pPlayerValueInfo, int nNewValue)
{
    BOOL        bResult  = false;
    uint32_t*   pCurUInt32 = NULL;
    size_t      uBitOffsetInUInt32   = 0;

    KGLOG_PROCESS_ERROR(pPlayerValueInfo);

    pCurUInt32          = m_Data + pPlayerValueInfo->uOffset / UINT32_BIT_COUNT;;
    uBitOffsetInUInt32  = pPlayerValueInfo->uOffset % UINT32_BIT_COUNT;

    switch(pPlayerValueInfo->nType)
    {
    case KPLAYER_VALUE_TYPE_BOOLEAN:      
        if (nNewValue)
            *pCurUInt32 |= (uint32_t)1 << uBitOffsetInUInt32;
        else
            *pCurUInt32 &= ~((uint32_t)1 << uBitOffsetInUInt32);
        break;
    case KPLAYER_VALUE_TYPE_INT32:
        *pCurUInt32 = (uint32_t)nNewValue;
        break;
    default:
        KGLOG_PROCESS_ERROR(false);
        break;
    }

    g_PlayerServer.DoSyncOnePlayerValue(m_pPlayer, pPlayerValueInfo->dwID, nNewValue);

    bResult = true;
Exit0:
    return bResult;
}
