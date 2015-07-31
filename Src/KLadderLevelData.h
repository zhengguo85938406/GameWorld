// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KLadderLevelData.h
//  Creator 	: hanruofei 
//  Date		: 2012-5-18
//  Comment	: 
// ***************************************************************
#pragma once
#include "GlobalMacro.h"

struct KLADDER_LEVELDATA_ITEM
{
    int m_nLevel;
    int m_nExp;
    int m_nTotalExp;
    int m_nExpCoe;
    BOOL m_bNoDecrease;
    int m_nMenterPointOnWin;
    int m_nMenterPointOnLose;
    int m_nMenterPointOnDraw;
};

class KLadderLevelData
{
public:
    KLadderLevelData(void);
    ~KLadderLevelData(void);
    BOOL Init();
    void UnInit();
    KLADDER_LEVELDATA_ITEM* GetLadderLevelData(int nLevel);
    BOOL GetTotalLadderExp(int nLadderLevel, int& nTotalLadderExp);

private:
    BOOL LoadData();
    KLADDER_LEVELDATA_ITEM m_LevelData[MAX_LADDER_LEVEL];
};

