// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KMentorPointDecrease.h
//  Creator		: hanruofei
//	Date		: 2/28/2013
//  Comment		: 
//	*********************************************************************
#pragma once
#include <map>

class KMentorPointDecrease
{
public:
    KMentorPointDecrease(void);
    ~KMentorPointDecrease(void);

    BOOL Init();
    void UnInit();

    int GetPercent(int nNoFatigueWinCount);

private:
    BOOL LoadData();

private:
    std::map<int, int> m_mapData;
};

