#include "stdafx.h"
#include "KRankExpSettings.h"


KRankExpSettings::KRankExpSettings(void)
    : m_vecData(cdMaxHeroLevel)
{

}


KRankExpSettings::~KRankExpSettings(void)
{
}

BOOL KRankExpSettings::Init()
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
	
	bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KRankExpSettings::UnInit()
{

}

BOOL KRankExpSettings::GetExp(int nLevel, int nScore, BOOL bWinGame, int& nExp)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;
    KRANK_EXP_SETTING_ITEM* pRankExpSettingItem = NULL;
	
	KGLOG_PROCESS_ERROR(nLevel > 0 && nLevel <= (int)m_vecData.size());
    KGLOG_PROCESS_ERROR(nScore >= 0);

    nExp = 0;
    pRankExpSettingItem = &m_vecData[nLevel - 1];
    for (int i = 0; i < MAX_RANK_EXP_RANGE_COUNT; ++i)
    {
        if (nScore < pRankExpSettingItem->RangeScore[i])
            break;

        if (i + 1 >= MAX_RANK_EXP_RANGE_COUNT || nScore < pRankExpSettingItem->RangeScore[i + 1])
        {
            nExp += (nScore - pRankExpSettingItem->RangeScore[i] + 1) * pRankExpSettingItem->RangeExpOneScore[i];
        }
        else
        {
            nExp += (pRankExpSettingItem->RangeScore[i + 1] - pRankExpSettingItem->RangeScore[i]) * pRankExpSettingItem->RangeExpOneScore[i];
        }
    }

    if (bWinGame)
        nExp = nExp * pRankExpSettingItem->nCoeOnWin / HUNDRED_NUM;

    if (nExp < 0)
        nExp = 0;

	bResult = true;
Exit0:
	return bResult;
}

BOOL KRankExpSettings::LoadData()
{
	BOOL 		bResult 	= false;
	BOOL 		bRetCode 	= false;
	int  		nHeight 	= 0;
	ITabFile* 	piTabFile 	= NULL;
    int         nLevel      = 0;
    KRANK_EXP_SETTING_ITEM* pItem = NULL;
    char        szKey[64];

	piTabFile = g_OpenTabFile(SETTING_DIR"/RankExp.tab");
	KGLOG_PROCESS_ERROR(piTabFile);

	nHeight = piTabFile->GetHeight();
	for(int nRow = 2; nRow <= nHeight; ++nRow)
	{
		bRetCode = piTabFile->GetInteger(nRow, "Level", 0, &nLevel);
        KGLOG_PROCESS_ERROR(bRetCode);

        KGLOG_PROCESS_ERROR(nLevel + 1 == nRow);

        KGLOG_PROCESS_ERROR(nLevel > 0 && nLevel <= (int)m_vecData.size());

        pItem = &m_vecData[nLevel - 1];

        bRetCode = piTabFile->GetInteger(nRow, "CoeOnWin", 0, &pItem->nCoeOnWin);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(pItem->nCoeOnWin >= HUNDRED_NUM);

        for (int i = 0; i < MAX_RANK_EXP_RANGE_COUNT; ++i)
        {
            sprintf(szKey, "Range%dScore", i + 1);
            bRetCode = piTabFile->GetInteger(nRow, szKey, 0, &pItem->RangeScore[i]);
            KGLOG_PROCESS_ERROR(bRetCode > 0);
            KGLOG_PROCESS_ERROR(pItem->RangeScore[i] > 0);

            sprintf(szKey, "Range%dExpOneScore", i + 1);
            bRetCode = piTabFile->GetInteger(nRow, szKey, 0, &pItem->RangeExpOneScore[i]);
            KGLOG_PROCESS_ERROR(bRetCode);
            KGLOG_PROCESS_ERROR(pItem->RangeExpOneScore[i] >= 0);
        }
	}

	bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}

