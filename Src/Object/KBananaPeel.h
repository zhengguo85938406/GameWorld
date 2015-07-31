// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KBananaPeel.h
//  Creator		: hanruofei
//	Date		: 8/17/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include "KMovableObject.h"
#include "KHero.h"

class KBananaPeel :  public KMovableObject
{
public:
    KBananaPeel(void);
    ~KBananaPeel(void);
    KSceneObjectType GetType() const;

    BOOL CollidedByHero(KHero* pHero);

private:
	virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
	virtual void OnTurnedToIdle();

private:
    BOOL  m_bOnlyAttackPlayer;
};

