// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBullet.h 
//  Creator 	: Xiayong  
//  Date		: 01/11/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "Ksdef.h"
#include "KMovableObject.h"
#include "KBulletInfoMgr.h"

class KSkill;
class KHero;
struct KBulletInfo;

class KBullet : public KMovableObject
{
public:
    KBullet();
    virtual ~KBullet();

    void Activate(int nCurLoop);

    KSceneObjectType GetType() const;

    virtual void ProcessRebounds();
    virtual int  GetCurrentGravity();
    void ApplyBulletInfo(KBulletInfo* pInfo);

    BOOL CollideHero(KHero* pHero);

    DWORD m_dwCasterID;
    KSkill* m_pSkill;
    int   m_nStartFrame;
    int   m_nLifeTime;
    int   m_nGravity;
    int   m_nSide;
    KBulletInfo* m_pBulletTemplate;

private:
    void OnDeleteFlagSetted();
    BOOL Blast();
    BOOL IsInAttackInterval(DWORD dwHeroID);
    BOOL RecordAttackedHero(DWORD dwHeroID);

private:
    BOOL m_bBlastedOnCurFrame;
    KATTACKED_OBJ_LIST  m_AttackedObjList;
};
