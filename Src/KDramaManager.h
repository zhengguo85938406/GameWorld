// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KDramaManager.h 
//  Creator 	: Xiayong  
//  Date		: 10/15/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KReadTab.h"

struct KDramaConfig :  public KLineData
{
    int nID;
    char szScriptName[64];
    uint32_t dwScriptID;
    int nMapID;

    KDramaConfig()
    {
        REGISTER_COL_INFO(nID, eIntCol);
        REGISTER_COL_INFO(szScriptName, eStringCol);
        REGISTER_COL_INFO(nMapID, eIntCol);
    }

    BOOL CheckData();
};

struct KDramaFrameInfo
{
    int     nFrame;
    char    szFuncName[32];
    int     nDialogID;
    bool    bPauseOnDialog;
};

typedef std::multimap<int, KDramaFrameInfo> KMAP_DRAMAFRAMEINFO;
typedef std::map<int, KMAP_DRAMAFRAMEINFO> KMAP_DRAMA;

class KDramaManager
{
public:
    KDramaManager();
    ~KDramaManager();

    BOOL LoadDrama();
    BOOL ReloadDrama();

    KDramaConfig* GetDramaConfig(int nID);
    KMAP_DRAMAFRAMEINFO* GetDrama(int nID);
    BOOL AddDramaFrameInfo(int nDramaID, int nFrame, const char szFunName[], int nDialogID, bool bPauseOnDialog);
    uint32_t GetDramaScriptID(int nID);
    int GetNextDramaNumber(){return m_nNextDramaNumber++;};

private:
    KReadTab<KDramaConfig> m_DramaConfig;
    KMAP_DRAMA m_mapDramas;
    int m_nNextDramaNumber;
};
