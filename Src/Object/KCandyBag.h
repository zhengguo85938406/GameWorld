// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KCandyBag.h
//  Creator		: hanruofei
//	Date		: 8/16/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include "KDoodad.h"
class KBall;
class KCandyBag : public KDoodad
{
public:
    KCandyBag(void);
    ~KCandyBag(void);

    KSceneObjectType GetType() const;
    BOOL CollidedByBall(KBall* pBall);

private:
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    int m_nLifePoint;
};

