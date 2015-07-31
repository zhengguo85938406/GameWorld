#include "stdafx.h"
#include "KPlayer.h"
#include "KUserPreferences.h"
#include "KPlayerServer.h"

KUserPreferences::KUserPreferences(void)
{
    m_pPlayer = NULL;
}

KUserPreferences::~KUserPreferences(void)
{
    m_pPlayer = NULL;
}

BOOL KUserPreferences::Init(KPlayer* pPlayer)
{
	memset(m_byUserPrefences, 0, sizeof(m_byUserPrefences));
    m_pPlayer = pPlayer;

	return true;
}

void KUserPreferences::UnInit()
{	
    m_pPlayer = NULL;
	return;
}

BOOL KUserPreferences::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL    bResult     = false;
    T3DB::KPBUserPreferences* pUserPreferences = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pUserPreferences = pSaveBuf->mutable_userpreferences();
    pUserPreferences->set_userprefences(m_byUserPrefences,MAX_USER_PREFERENCES_LEN);

    bResult = true;
Exit0:
    return bResult;
}
BOOL KUserPreferences::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL    bResult     = false;

    assert(pLoadBuf);

    const T3DB::KPBUserPreferences& UserPreferences = pLoadBuf->userpreferences();
    const std::string& strUserPreferences = UserPreferences.userprefences();

    if (strUserPreferences.size() > MAX_USER_PREFERENCES_LEN)
        memcpy(m_byUserPrefences, strUserPreferences.data(), MAX_USER_PREFERENCES_LEN);
    else
        memcpy(m_byUserPrefences,strUserPreferences.data(),strUserPreferences.size());

    if (m_pPlayer->m_eConnectType == eLogin)
        g_PlayerServer.DoSyncUserPreferences(m_pPlayer->m_nConnIndex, 0, MAX_USER_PREFERENCES_LEN, m_byUserPrefences);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KUserPreferences::SetUserPreferences(int nOffset, int nLength, BYTE* pbyData)
{
	BOOL bResult = false;

	assert(pbyData);
	KGLOG_PROCESS_ERROR(nOffset >= 0 && nOffset < MAX_USER_PREFERENCES_LEN);
    KGLOG_PROCESS_ERROR(nLength > 0 && nLength <= MAX_USER_PREFERENCES_LEN);
	KGLOG_PROCESS_ERROR(nOffset + nLength <= MAX_USER_PREFERENCES_LEN);

    memcpy(m_byUserPrefences + nOffset, pbyData, nLength);

	bResult = true;
Exit0:
	return bResult;
}

BOOL KUserPreferences::GetUserPreferences(int nOffset, int nLength, BYTE* pbyData)
{
	BOOL bResult = false;
	int nIndex = 0;

	KGLOG_PROCESS_ERROR(pbyData);
	KGLOG_PROCESS_ERROR(nOffset >= 0 && nOffset < MAX_USER_PREFERENCES_LEN);
	KGLOG_PROCESS_ERROR(nLength > 0 && (nOffset + nLength) <= MAX_USER_PREFERENCES_LEN);

	for (nIndex = 0; nIndex < nLength; nIndex++)
	{
		pbyData[nIndex] = m_byUserPrefences[nOffset + nIndex];
	}

	bResult = true;
Exit0:
	return bResult;
}

BYTE* KUserPreferences::GetUserPreferences(int nOffset)
{
	KGLOG_PROCESS_ERROR(nOffset >= 0 && nOffset < MAX_USER_PREFERENCES_LEN);

	return m_byUserPrefences + nOffset;
Exit0:
	return NULL;
}

BYTE* KUserPreferences::GetData()
{
	return m_byUserPrefences;
}
