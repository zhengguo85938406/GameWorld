////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KSceneLoader.h
//  Version     : 1.0
//  Creator     : ZhaoChunfeng,XiaYong
//  Create Date : 2008-7-28 17:55:58
//  Comment     : 
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _KSceneLoader_H_
#define _KSceneLoader_H_
#include "KCircleList.h"

class KSceneLoader
{
public:
    KSceneLoader() : m_bQuit(false){}
    ~KSceneLoader(){}
    
public:
    BOOL Init();
    void UnInit();
    
    BOOL  PostLoadingScene(DWORD dwSceneID);
    DWORD GetCompletedScene();

private:    
    static void WorkThreadFunction(void *pvParam);
    void ThreadFunction();

private:
    BOOL                    m_bQuit;
    KThread                 m_Thread;
    KCircleList<DWORD, 640>	m_LoadingList;
    KCircleList<DWORD, 640>	m_LoadedList;
};
#endif

