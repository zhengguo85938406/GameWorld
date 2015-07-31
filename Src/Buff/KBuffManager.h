// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBuffManager.h 
//  Creator 	: Xiayong  
//  Date		: 09/30/2011
//  Comment	: 
// ***************************************************************
#pragma once
#include "KBuff.h"

class KBuffManager
{
public:
    KBuffManager();
    ~KBuffManager();

    BOOL Init();
    void UnInit();

    KBuff* GetBuff(DWORD dwID);

private:
    typedef std::map<DWORD, KBuff> KBUFF_INFO_MAP;
    KBUFF_INFO_MAP m_mapBuff;

private:
    BOOL LoadBuff(const char* szTabFile);
};
