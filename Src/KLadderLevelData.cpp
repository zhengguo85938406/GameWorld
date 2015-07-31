#include "stdafx.h"
#include "KLadderLevelData.h"


KLadderLevelData::KLadderLevelData(void)
{
    memset(m_LevelData, 0, sizeof(m_LevelData));
}


KLadderLevelData::~KLadderLevelData(void)
{
}

BOOL KLadderLevelData::Init()
{
	BOOL bResult = false;
	BOOL bRetCode = false;

    bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KLadderLevelData::UnInit()
{

}

BOOL KLadderLevelData::LoadData()
{
    BOOL        bResult         = false;
    BOOL        bRetCode        = false;
    int         nExpectedLevel  = 0;
    int         nTabValue       = 0;
    int         nTotalExp       = 0;
    ITabFile*   piTabFile       = NULL;
    KLADDER_LEVELDATA_ITEM* pLevelDataItem = NULL;

    piTabFile = g_OpenTabFile(SETTING_DIR"/LadderLevelData.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRow = 2; nRow <= piTabFile->GetHeight(); ++nRow)
    {
        ++nExpectedLevel;
        KGLOG_PROCESS_ERROR(nExpectedLevel <= MAX_LADDER_LEVEL);
        pLevelDataItem = &m_LevelData[nExpectedLevel - 1];

        bRetCode = piTabFile->GetInteger(nRow, "Level", 0, &nTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nTabValue == nExpectedLevel);
        pLevelDataItem->m_nLevel = nTabValue;

        bRetCode = piTabFile->GetInteger(nRow, "Exp", 0, &nTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nTabValue > 0);
        pLevelDataItem->m_nExp = nTabValue;

        bRetCode = piTabFile->GetInteger(nRow, "ExpCoe", 0, &nTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nTabValue > 0);
        pLevelDataItem->m_nExpCoe = nTabValue;

        bRetCode = piTabFile->GetInteger(nRow, "NoDecrease", 0, &nTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nTabValue == 0 || nTabValue == 1);
        pLevelDataItem->m_bNoDecrease = nTabValue;

        bRetCode = piTabFile->GetInteger(nRow, "MenterPointOnWin", 0, &nTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nTabValue >= 0);
        pLevelDataItem->m_nMenterPointOnWin = nTabValue;

        bRetCode = piTabFile->GetInteger(nRow, "MenterPointOnLose", 0, &nTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nTabValue >= 0);
        pLevelDataItem->m_nMenterPointOnLose = nTabValue;

        bRetCode = piTabFile->GetInteger(nRow, "MenterPointOnDraw", 0, &nTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nTabValue >= 0);
        pLevelDataItem->m_nMenterPointOnDraw = nTabValue;

        pLevelDataItem->m_nTotalExp = nTotalExp;
        nTotalExp = pLevelDataItem->m_nExp;
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

KLADDER_LEVELDATA_ITEM* KLadderLevelData::GetLadderLevelData(int nLevel)
{
    KLADDER_LEVELDATA_ITEM* pResult = NULL;

    KGLOG_PROCESS_ERROR(nLevel >= 1 && nLevel <= MAX_LADDER_LEVEL);

    pResult = &m_LevelData[nLevel - 1];
Exit0:
    return pResult;
}

BOOL KLadderLevelData::GetTotalLadderExp(int nLadderLevel, int& nTotalLadderExp)
{
	BOOL bResult = false;

	KGLOG_PROCESS_ERROR(nLadderLevel >= 1 && nLadderLevel <= countof(m_LevelData));

    nTotalLadderExp = 0;
    for (int i = 0; i < nLadderLevel - 1; ++i)
    {
        nTotalLadderExp += m_LevelData[i].m_nExp;
    }

	bResult = true;
Exit0:
	return bResult;
}
