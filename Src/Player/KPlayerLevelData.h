// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KPlayerLevelData.h 
//  Creator 	: Xiayong  
//  Date		: 09/05/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include <map>

class KPlayerLevelData
{
public:
    BOOL LoadData();
    int  GetLevelUpExp(int nLevel);

private:
    std::map<int, int> m_mapLevelData;
};
