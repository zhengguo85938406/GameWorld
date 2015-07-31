// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KRankExpSettings.h
//  Creator		: hanruofei
//	Date		: 2/28/2013
//  Comment		: 
//	*********************************************************************
#pragma once

#include "GlobalMacro.h"
#include "game_define.h"

#include <vector>

struct KRANK_EXP_SETTING_ITEM
{
    int nCoeOnWin;
    int RangeScore[MAX_RANK_EXP_RANGE_COUNT];
    int RangeExpOneScore[MAX_RANK_EXP_RANGE_COUNT];
};

class KRankExpSettings
{
public:
    KRankExpSettings(void);
    ~KRankExpSettings(void);

    BOOL Init();
    void UnInit();

    BOOL GetExp(int nLevel, int nScore, BOOL bWinGame, int& nExp);

private:
    BOOL LoadData();

private:
    std::vector<KRANK_EXP_SETTING_ITEM> m_vecData;

};

