#include "stdafx.h"
#include "KBattleSetting.h"


KBattleSetting::KBattleSetting(void)
{
}


KBattleSetting::~KBattleSetting(void)
{
}

BOOL KBattleSetting::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = LoadBattleTemplate();
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = LoadBattleResult();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KBattleSetting::UnInit()
{
    m_mapBattleTemplate.clear();
    m_mapBattleResult.clear();
}

KBattleTempalte* KBattleSetting::GetBattleTemplate(DWORD dwBattleTemplateID)
{
    KBattleTempalte* pResult = NULL;
    KMAP_BATTLE_TEMPLATE::iterator it;

    KG_PROCESS_ERROR(dwBattleTemplateID);

    it = m_mapBattleTemplate.find(dwBattleTemplateID);
    KG_PROCESS_ERROR(it != m_mapBattleTemplate.end());

    pResult = &it->second;
Exit0:
    return pResult;
}

BOOL KBattleSetting::GetDeltaAICoe(int nDeltaScore, BOOL bSuccess, int& nDeltaAICoe)
{
    BOOL bResult = false;
    KMAP_BATTLE_RESULT::iterator it;

    nDeltaScore = abs(nDeltaScore);

    KGLOG_PROCESS_ERROR(m_mapBattleResult.size());

    it = m_mapBattleResult.upper_bound(nDeltaScore);
    KGLOG_PROCESS_ERROR(it != m_mapBattleResult.begin());

    --it;

    if (bSuccess)
    {
        nDeltaAICoe = it->second.nWinPveAICoe;
    }
    else
    {
        nDeltaAICoe = it->second.nLosePveAICoe;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBattleSetting::LoadBattleTemplate()
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    ITabFile*   piTabFile   = NULL;
    DWORD       dwID        = ERROR_ID;
    KBattleTempalte* pBattleTemplate = NULL;
    pair<std::map<DWORD, KBattleTempalte>::iterator, bool> InsRet;

    piTabFile = g_OpenTabFile(SETTING_DIR"/Battle.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); ++nRowIndex)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", ERROR_ID, (int*)(&dwID));
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(dwID > 0);

        InsRet = m_mapBattleTemplate.insert(std::make_pair(dwID, KBattleTempalte()));
        KGLOG_PROCESS_ERROR(InsRet.second);

        pBattleTemplate = &InsRet.first->second;

        bRetCode = piTabFile->GetInteger(nRowIndex, "BattleTime", 0, &pBattleTemplate->nTotleFrame);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        pBattleTemplate->nTotleFrame *= GAME_FPS;

        KGLOG_PROCESS_ERROR(pBattleTemplate->nTotleFrame >= 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Mode", 0, &pBattleTemplate->nMode);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "WinScore", 0, &pBattleTemplate->nWinScore);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

BOOL KBattleSetting::LoadBattleResult()
{
	BOOL        bResult         = false;
	BOOL        bRetCode        = false;
    ITabFile*   piTabFile       = NULL;
    int         nHeight         = 0;
    int         nMinDeltaScore  = 0;
    KBattleResult* pDataItem    = NULL;

    piTabFile = g_OpenTabFile(SETTING_DIR"/BattleResult.tab");
    KGLOG_PROCESS_ERROR(piTabFile);
	
    nHeight = piTabFile->GetHeight();
    for (int nRow = 2; nRow <= nHeight; ++nRow)
    {
        bRetCode = piTabFile->GetInteger(nRow, "MinDeltaScore", 0, &nMinDeltaScore);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nMinDeltaScore >= 0);

        pDataItem = &m_mapBattleResult[nMinDeltaScore];
        pDataItem->nMinDeltaScore = nMinDeltaScore;

        bRetCode = piTabFile->GetInteger(nRow, "WinPveAICoe", 0, &pDataItem->nWinPveAICoe);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRow, "LosePveAICoe", 0, &pDataItem->nLosePveAICoe);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
    }

	bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
	return bResult;
}
