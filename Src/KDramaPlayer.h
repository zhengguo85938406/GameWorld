// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KDramaPlayer.h 
//  Creator 	: Xiayong  
//  Date		: 10/15/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KDramaManager.h"

class KScene;
class KDramaPlayer
{
public:
    KDramaPlayer();
    ~KDramaPlayer();

    BOOL Init(KScene* pScene);
    void UnInit();

    void Activate();
    BOOL StartDrama(int nDramaID);
    BOOL StopDrama();
    BOOL ResumeDrama();
    BOOL IsPlayering(){return (m_pDramaFrameInfo != NULL);};

private:
    KMAP_DRAMAFRAMEINFO* m_pDramaFrameInfo;
    int m_nNextFrame;
    uint32_t m_dwScriptID;
    BOOL m_bPause;
    int m_nLoop;
    KScene* m_pScene;
    int m_nDramaNumber; // 每次剧情的唯一编号
};
