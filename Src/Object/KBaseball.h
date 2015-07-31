// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KBaseball.h
//  Creator		: hanruofei
//	Date		: 8/17/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include "KMovableObject.h"
#include "KHero.h"

class KBaseball : public KMovableObject
{
public:
    KBaseball(void);
    ~KBaseball(void);

    KSceneObjectType GetType() const;

    BOOL CollideHero(KHero* pHero);

private:
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    virtual void ProcessRebounds();
    virtual void OnTurnedToIdle();

private:
    int m_nCurAttackPoint;
};

