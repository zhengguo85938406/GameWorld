// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBackboard.h
//  Creator 	: Hanruofei  
//  Date		: 12/27/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "KStaticObject.h"
#include "game_define.h"

class KBackboard :  public KStaticObject
{
public:
    KBackboard(void);
    virtual ~KBackboard(void);

    virtual KSceneObjectType GetType() const;
};

