// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KChest.h
//  Creator 	: Hanruofei  
//  Date		: 12/28/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "KDoodad.h"
class KChest : public KDoodad
{
public:
    KChest(void);
    virtual ~KChest(void);

    virtual KSceneObjectType GetType() const;

    virtual void ProcessAfterCollision();

private:
    virtual void ApplySpecialInitInfo(const KSceneObjInitInfo& cTemplate);
};

