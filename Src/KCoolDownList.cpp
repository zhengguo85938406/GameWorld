#include "stdafx.h"
#include "KCoolDownList.h"

#define COOL_DOWN_TAB_FILE  "CoolDownList.tab"

BOOL KCoolDownList::Init()
{
    BOOL        bResult                 = false;
    int         nRetCode                = false;
    ITabFile*   piTabFile               = NULL;
    int         nItemCount              = 0;
    char        szFileName[MAX_PATH];
    KCOOL_DWON_INFO Info;

    nRetCode = snprintf(szFileName, sizeof(szFileName), "%s/%s", SETTING_DIR, COOL_DOWN_TAB_FILE);
    szFileName[sizeof(szFileName) - 1] = '\0';
    KGLOG_PROCESS_ERROR(nRetCode > 0);

	piTabFile = g_OpenTabFile(szFileName);
	KGLOG_PROCESS_ERROR(piTabFile);

    // 第一行是名称(ID, Duration)
    nItemCount = piTabFile->GetHeight() - 1; 
    KGLOG_PROCESS_ERROR(nItemCount >= 0);
    KGLOG_PROCESS_ERROR(nItemCount < MAX_COOL_DOWN_COUNT);

    for (int i = 0; i < nItemCount; ++i)
    {
        float fDuration = 0.0f;

		nRetCode = piTabFile->GetInteger(2 + i, "ID", 0, (int*)&Info.dwID);
		KGLOG_PROCESS_ERROR(nRetCode);
        KGLOG_PROCESS_ERROR(Info.dwID > 0 && Info.dwID < USHRT_MAX);

        // 策划填写数值时填秒数即可，这里转换为游戏帧数
		nRetCode = piTabFile->GetFloat(2 + i, "Duration", 0, &fDuration);
		KGLOG_PROCESS_ERROR(nRetCode);
        KGLOG_PROCESS_ERROR(fDuration > 0);

        Info.nDuration = (int)(fDuration * GAME_FPS);

        nRetCode = piTabFile->GetInteger(2 + i, "NeedSave", 0, (int*)&Info.bNeedSave);
		KGLOG_PROCESS_ERROR(nRetCode);

        nRetCode = piTabFile->GetInteger(2 + i, "GroupID", 0, (int*)&Info.nGroupID);
		KGLOG_PROCESS_ERROR(nRetCode);

        m_CoolDownTable[Info.dwID] = Info;
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_CoolDownTable.clear();
    }
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

void KCoolDownList::UnInit()
{
    m_CoolDownTable.clear();
}

int KCoolDownList::GetCoolDownValue(DWORD dwCoolDownID)
{
    int                      nResult = 0;
    KCOOL_DOWN_MAP::iterator it;

    KG_PROCESS_ERROR(dwCoolDownID != 0);

    it = m_CoolDownTable.find(dwCoolDownID);
    if (it == m_CoolDownTable.end())
    {
        KGLogPrintf(KGLOG_DEBUG, "GetCoolDownValue %u Faild!", dwCoolDownID);
        goto Exit0;
    }

    nResult = it->second.nDuration;
Exit0:
    return nResult;
}

BOOL KCoolDownList::IsCoolDownNeedSave(DWORD dwCoolDownID)
{
    BOOL bResult 	= false;
    BOOL bRetCode 	= false;
    KCOOL_DOWN_MAP::iterator it;

    it = m_CoolDownTable.find(dwCoolDownID);
    if (it == m_CoolDownTable.end())
    {
        KGLogPrintf(KGLOG_DEBUG, "GetCoolDownValue %u Faild!", dwCoolDownID);
        goto Exit0;
    }

    KG_PROCESS_ERROR(it->second.bNeedSave);
    
    bResult = true;
Exit0:
    return bResult;
}

void KCoolDownList::GetGroupCoolDown(DWORD dwCoolDownID, std::vector<DWORD>& vecCoolDown)
{
    int nGroupID = GetCoolDownGroupID(dwCoolDownID);

    if (nGroupID == 0)
    {
        vecCoolDown.push_back(dwCoolDownID);
        goto Exit0;
    }

    for (KCOOL_DOWN_MAP::iterator it = m_CoolDownTable.begin(); it != m_CoolDownTable.end(); ++it)
    {
        if (it->second.nGroupID == nGroupID)
            vecCoolDown.push_back(it->first);
    }

Exit0:
    return;
}

int KCoolDownList::GetCoolDownGroupID(DWORD dwCoolDownID)
{
    int nResult = 0;
    KCOOL_DOWN_MAP::iterator it = m_CoolDownTable.find(dwCoolDownID);
    KG_PROCESS_ERROR(it != m_CoolDownTable.end());

    nResult = it->second.nGroupID;
Exit0:
    return nResult;
}
