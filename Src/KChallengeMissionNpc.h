// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KChallengeMissionNpc.h 
//  Creator 	: Xiayong  
//  Date		: 11/14/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KReadTab.h"

#define NPC_COUNT 16

struct KChallengeMissionNpcData : public KLineData
{
    int nStep;
    int nNpcID[NPC_COUNT];
    std::vector<int> vecValidNpc;

    KChallengeMissionNpcData()
    {
        REGISTER_COL_INFO(nStep, eIntCol);

        for (int i = 0; i < NPC_COUNT; ++i)
        {
            REGISTER_COL_INFO(nNpcID[i], eIntCol);
        }
    }

    BOOL CheckData()
    {
    	BOOL bResult  = false;
        
        for (int i = 0; i < NPC_COUNT; ++i)
        {
            if (nNpcID[i] != 0)
                vecValidNpc.push_back(nNpcID[i]);
        }

        KGLOG_PROCESS_ERROR(vecValidNpc.size() > 0);

    	bResult = true;
    Exit0:
    	return bResult;
    }
};


class KChallengeMissionNpc
{
public:
    BOOL Init();
    void UnInit();

    int GetTwoRandomNpc(int nType, int nStep, int& nNPCID1, int& nNPCID2);

private:
    KReadTab<KChallengeMissionNpcData> m_Data1;
    KReadTab<KChallengeMissionNpcData> m_Data2;
};
