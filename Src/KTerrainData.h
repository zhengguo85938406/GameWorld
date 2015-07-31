// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KTerrainData.h 
//	Creator 	: Xiayong  
//  Date		: 08/06/2011
//  Comment		: 
// ***************************************************************

#pragma once
#include "KCell.h"
#include "KSceneGlobal.h"

class KTerrainScene
{
public:
    KTerrainScene();

    int AddRef();
    int Release();

    KCell* GetTerrainData();

public:
    BOOL m_bLoadFlag;

private:
    void Clean();

private:
    int      m_nRefCount;
    KCell    m_Cells[SCENE_GRID_WIDTH * SCENE_GRID_HEIGHT];
};
