#include "stdafx.h"
#include "KPathPointData.h"
#include "KSceneGlobal.h"


KPathPointData::KPathPointData(void)
{
}


KPathPointData::~KPathPointData(void)
{
}

BOOL KPathPointData::Init()
{
    BOOL            bResult     = false;
    BOOL            bRetCode    = false;

    KPathPointInfo  PointInfo;
    ITabFile*	    piTabFile   = NULL;

    piTabFile = g_OpenTabFile(SETTING_DIR"/"FLY_PATH_DIR"/"PATH_POINT_LIST_FILE_NAME);
    KGLOG_PROCESS_ERROR(piTabFile);

    piTabFile->SetErrorLog(false);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        memset(&PointInfo, 0, sizeof(PointInfo));

        bRetCode = piTabFile->GetInteger(nRowIndex, "PointID", 0, (int*)&PointInfo.dwPointID);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(PointInfo.dwPointID > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "PointType", KPATH_POINT_TYPE_ERROR, (int*)&PointInfo.ePointType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "X", 0, &PointInfo.nX);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Y", 0, &PointInfo.nY);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Z", 0, &PointInfo.nZ);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "PathType", KFLY_PATH_TYPE_ERROR, (int*)&PointInfo.ePathType);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Velocity", 0, &PointInfo.nVelocity);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Gravity", 0, &PointInfo.nGravity);
        KGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = piTabFile->GetInteger(nRowIndex, "HeightOffset", 0, &PointInfo.nHeightOffset);
        KGLOG_PROCESS_ERROR(bRetCode);
        KGLOG_PROCESS_ERROR(PointInfo.nHeightOffset >= 0);
        PointInfo.nHeightOffset += CELL_LENGTH;

        if (PointInfo.ePointType == KPATH_POINT_TYPE_PERCENT)
        {
            KGLOG_PROCESS_ERROR(PointInfo.nX <= HUNDRED_NUM);
            KGLOG_PROCESS_ERROR(PointInfo.nY <= HUNDRED_NUM);
        }

        switch (PointInfo.ePathType)
        {
        case KFLY_PATH_TYPE_LINE:
            KGLOG_PROCESS_ERROR(PointInfo.nVelocity > 0);
            break;
        case KFLY_PATH_TYPE_PARABOLA:
            KGLOG_PROCESS_ERROR(PointInfo.nGravity > 0);
            break;
        }

        m_InfoTable[PointInfo.dwPointID] = PointInfo;
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}

void KPathPointData::UnInit()
{
    m_InfoTable.clear();
}

KPathPointInfo* KPathPointData::GetPointInfo(DWORD dwPointID)
{
    KPathPointInfo* pPointInfo = NULL;
    KPATH_POINT_INFO_TABLE::iterator itTable = m_InfoTable.find(dwPointID);
    if (itTable != m_InfoTable.end())
    {
        pPointInfo = &itTable->second;
    }
    return pPointInfo;
}
