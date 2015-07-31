// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBulletInfoMgr.h 
//  Creator 	: Xiayong  
//  Date		: 01/11/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "Ksdef.h"
#include "game_define.h"
#include <map>



struct KBulletInfo 
{
    DWORD dwTemplateID;
    int nVelocityX;
    int nVelocityY;
    int nVelocityZ;
    int nOffsetX;
    int nOffsetY;
    int nOffsetZ;
    int nLengthX;
    int nLengthY;
    int nLengthZ;
    int nLifeTime;
    BOOL bPenetrate;
    int nGravity;
	int nBeAttackType;
    BOOL bAttackOnFly;
    DWORD dwCollideAffectID;
    DWORD dwBlastAffectID;
    BOOL bBlastOnCollide;
    BOOL bBlastOnDisappear;
    int nBlastRadiusX;
    int nBlastRadiusY;
    int nBlastRadiusZ;
    int nAttackIntervalFrame;
};

typedef std::map<DWORD, KBulletInfo> KMAP_BULLETINFO;

class KBulletInfoMgr
{
public:
    KBulletInfoMgr();
    ~KBulletInfoMgr();

    BOOL Init();
    void UnInit();

    KBulletInfo* GetBulletInfo(DWORD dwBulletTemplateID);

private:
    BOOL LoadData();

private:
    KMAP_BULLETINFO m_mapBulletInfo;
};
