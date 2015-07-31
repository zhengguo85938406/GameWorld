// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KEAData.h
//  Creator 	: hanruofei 
//  Date		: 2012-5-19
//  Comment	: 
// ***************************************************************
#pragma once
#include <vector>

struct KEADATA_ITEM
{
    int     nLowerDeltaEol;
    double  dHigherEA;
    double  dLowerEA;
};

typedef std::vector<KEADATA_ITEM> KVEC_EADATA;

class KEAData
{
public:
    KEAData(void);
    ~KEAData(void);
      
    BOOL Init();
    void UnInit();
    void GetEA(const int(& nEol)[sidTotal], double(& rEA)[sidTotal]);

private:
    BOOL LoadData();
    KVEC_EADATA m_vecEAData;
};

