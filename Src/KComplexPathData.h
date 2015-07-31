// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KComplexPathData.h
//  Creator		: tongxuehu 
//	Date		: 01/16/2013
//  Comment		: 
//	*********************************************************************

#pragma once

#include "KComplexPathData.h"

typedef std::vector<int> KFLY_PATH_POINTS;

struct KComplexPathInfo
{
    DWORD  dwPathID;
    int    nOffsetX;
    int    nOffsetY;
    int    nOffsetZ;

    KBasketSocketFloor  eTargetFloor;
    KFLY_PATH_POINTS    cPoints;
    KFLY_PATH_TYPE      eEndPathType;
};

typedef std::map<DWORD, KComplexPathInfo> KCOMPLEX_PATH_TABLE;

class KComplexPathData
{
public:
    KComplexPathData(void);
    ~KComplexPathData(void);

    BOOL Init();
    void UnInit();

    KComplexPathInfo* GetPath(DWORD dwPathID);

private:
    KCOMPLEX_PATH_TABLE m_cPathTable;
};

