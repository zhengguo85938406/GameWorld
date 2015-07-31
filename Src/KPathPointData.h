// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KPathPointData.h
//  Creator		: tongxuehu 
//	Date		: 01/16/2013
//  Comment		: 
//	*********************************************************************
#pragma once

#include "game_define.h"

struct KPathPointInfo
{
    DWORD dwPointID;
    KPATH_POINT_TYPE ePointType;
    int nX;
    int nY;
    int nZ;
    KFLY_PATH_TYPE ePathType;
    int nVelocity;
    int nGravity;
    int nHeightOffset;
};

typedef std::map<DWORD, KPathPointInfo> KPATH_POINT_INFO_TABLE;

class KPathPointData
{
public:
    KPathPointData(void);
    virtual ~KPathPointData(void);

    BOOL Init();
    void UnInit();

    KPathPointInfo* GetPointInfo(DWORD dwPointID);

private:
    KPATH_POINT_INFO_TABLE m_InfoTable;
};

