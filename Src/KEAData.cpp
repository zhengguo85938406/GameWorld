#include "stdafx.h"
#include "KEAData.h"


KEAData::KEAData(void)
{
}


KEAData::~KEAData(void)
{
}

BOOL KEAData::Init()
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	
    bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}

void KEAData::UnInit()
{
    m_vecEAData.clear();
}

BOOL KEAData::LoadData()
{
	BOOL            bResult     = false;
	BOOL            bRetCode    = false;
	int             nDeltaEol   = -1;
    int             nTabValue   = 0;
    float           fTabValue   = 0.0;
    int             nHeight     = 0;
    ITabFile*       piTabFile   = NULL;
    KEADATA_ITEM*   pEADataItem = NULL;

    piTabFile = g_OpenTabFile(SETTING_DIR"/EAData.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    nHeight = piTabFile->GetHeight();
    KGLOG_PROCESS_ERROR(nHeight > 1);

    m_vecEAData.resize(nHeight - 1);
    for (int nRow = 2; nRow <= nHeight; ++nRow)
    {
        bRetCode = piTabFile->GetInteger(nRow, "DeltaEol", 0, &nTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(nTabValue > nDeltaEol);

        pEADataItem = &m_vecEAData[nRow - 2];
        pEADataItem->nLowerDeltaEol = nTabValue;
        
        bRetCode = piTabFile->GetFloat(nRow, "HigherEa", 0.0f, &fTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(fTabValue >= 0);
        pEADataItem->dHigherEA = fTabValue / HUNDRED_NUM;

        bRetCode = piTabFile->GetFloat(nRow, "LowerEa", 0.0f, &fTabValue);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(fTabValue >= 0);
        pEADataItem->dLowerEA = fTabValue / HUNDRED_NUM;

        nDeltaEol = pEADataItem->nLowerDeltaEol;
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

void KEAData::GetEA(const int(& nEol)[sidTotal], double(& rEA)[sidTotal])
{
    int nDeltaEol = 0;
    BOOL bShouldSwap = false;
    int nHigherIndex = sidLeft;
    int nLowerIndex = sidRight;

    nDeltaEol = nEol[sidLeft] - nEol[sidRight];
    if (nDeltaEol < 0)
    {
        nHigherIndex = sidRight;
        nLowerIndex = sidLeft;
        nDeltaEol = -nDeltaEol;
    }

    for (size_t uIndex = 0U; uIndex < m_vecEAData.size(); ++uIndex)
    {
        if (uIndex + 1 == m_vecEAData.size() || 
            nDeltaEol >= m_vecEAData[uIndex].nLowerDeltaEol && nDeltaEol < m_vecEAData[uIndex + 1].nLowerDeltaEol)
        {
            rEA[nHigherIndex] = m_vecEAData[uIndex].dHigherEA;
            rEA[nLowerIndex] = m_vecEAData[uIndex].dLowerEA;
            break;
        }
    }
}
