#include "stdafx.h"
#include "KComplexPathData.h"


KComplexPathData::KComplexPathData(void)
{
}


KComplexPathData::~KComplexPathData(void)
{
}


BOOL KComplexPathData::Init()
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;
    
    KComplexPathInfo PathData;
    ITabFile*	    piTabFile   = NULL;
    char            szPoints[256];

    piTabFile = g_OpenTabFile(SETTING_DIR"/"FLY_PATH_DIR"/"COMPLEX_PATH_LIST_FILE_NAME);
    KGLOG_PROCESS_ERROR(piTabFile);

    piTabFile->SetErrorLog(false);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        memset(&PathData, 0, sizeof(PathData));

        bRetCode = piTabFile->GetInteger(nRowIndex, "PathID", 0, (int*)&PathData.dwPathID);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(PathData.dwPathID > 0);
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "OffsetX", 0, &PathData.nOffsetX);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "OffsetY", 0, &PathData.nOffsetY);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "OffsetZ", 0, &PathData.nOffsetZ);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "TargetFloor", bfFifth, (int*)&PathData.eTargetFloor);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetString(nRowIndex, "Points", "", szPoints, sizeof(szPoints));
        KGLOG_PROCESS_ERROR(bRetCode);
        szPoints[sizeof(szPoints) - 1] = '\0';

        bRetCode = piTabFile->GetInteger(nRowIndex, "EndPathType", KFLY_PATH_TYPE_ERROR, (int*)&PathData.eEndPathType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = g_ParseMutiInteger(szPoints, ";", PathData.cPoints);
        KGLOG_PROCESS_ERROR(bRetCode);

        m_cPathTable[PathData.dwPathID] = PathData;
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

void KComplexPathData::UnInit()
{
    m_cPathTable.clear();
}

KComplexPathInfo* KComplexPathData::GetPath(DWORD dwPathID)
{
    KComplexPathInfo* pPathData = NULL;
    KCOMPLEX_PATH_TABLE::iterator itTable = m_cPathTable.find(dwPathID);
    if (itTable != m_cPathTable.end())
    {
        pPathData = &itTable->second;
    }
    return pPathData;

}