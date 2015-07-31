#include "stdafx.h"
#include "KMentorPointDecrease.h"


KMentorPointDecrease::KMentorPointDecrease(void)
{
}


KMentorPointDecrease::~KMentorPointDecrease(void)
{
}

BOOL KMentorPointDecrease::Init()
{
	BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);
	
	bResult = true;
Exit0:
	return bResult;
}

void KMentorPointDecrease::UnInit()
{
    m_mapData.clear();
}

BOOL KMentorPointDecrease::LoadData()
{
	BOOL 		bResult 	= false;
	BOOL 		bRetCode 	= false;
	int  		nHeight 	= 0;
    int         nNoFatigueWin = 0;
    int         nPercent        = 0;
	ITabFile* 	piTabFile 	= NULL;

	piTabFile = g_OpenTabFile(SETTING_DIR"/MentorPointDecrease.tab");
	KGLOG_PROCESS_ERROR(piTabFile);

	nHeight = piTabFile->GetHeight();
	for(int nRow = 2; nRow <= nHeight; ++nRow)
	{
		bRetCode = piTabFile->GetInteger(nRow, "NoFatigueWin", 0, &nNoFatigueWin);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nNoFatigueWin > 0);

        bRetCode = piTabFile->GetInteger(nRow, "Percent", 0, &nPercent);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(nPercent >= 0 && nPercent <= 100);

        m_mapData[nNoFatigueWin] = nPercent;
	}

	bResult = true;
Exit0:
	KG_COM_RELEASE(piTabFile);
	return bResult;
}

int KMentorPointDecrease::GetPercent(int nNoFatigueWinCount)
{
    int nResult = 0;
    std::map<int, int>::iterator it;
    
    it = m_mapData.find(nNoFatigueWinCount);
    if (it != m_mapData.end())
    {
        nResult = it->second;
    }
    
    return nResult;
}
