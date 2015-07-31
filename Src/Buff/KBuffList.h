// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBufferList.h 
//  Creator 	: Xiayong  
//  Date		: 09/30/2011
//  Comment	: 
// ***************************************************************
#pragma once
#include "Engine/KMemory.h"
#include "KHeroDataList.h"

class KHero;
class KBuff;

struct KBuffInfo 
{
    KBuff* pBuff;
    int    m_nEndFrame;
    int    m_nActiveCount;

    KBuffInfo()
    {
        pBuff           = NULL;
        m_nEndFrame     = -1;
        m_nActiveCount  = 0;
    }
};

typedef std::vector<KBuffInfo*> KVEC_BUFF;

typedef KMemory::KAllocator<std::pair<DWORD, KBuffInfo> > KBuffTableAlloctor;
typedef std::map<DWORD, KBuffInfo, std::less<DWORD>,  KBuffTableAlloctor>  KBuffTable;

class KBuffList
{
public:
    KBuffList();
    ~KBuffList();

    BOOL Init(KHero* pHero);
    void UnInit();

    BOOL AddBuff(DWORD dwBuffID);
    BOOL DelBuff(DWORD dwBuffID);
    BOOL GetAllBuff(KVEC_BUFF& vecRetObj);
    BOOL SetBuffParam(DWORD dwBuffID, int nEndFrame, int nActiveCount);

    BOOL HasBuff(DWORD dwBuffID) const;

    void Activate();

    BOOL Save(KBUFF_NODE_MAP& rmapBuffNode);
    BOOL Load(KBUFF_NODE_MAP& rmapBuffNode);

private:
    KHero* m_pSelf;
    KBuffTable  m_BuffTable;
};
