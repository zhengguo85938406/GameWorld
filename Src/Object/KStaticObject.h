// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KStaticObject.h
//  Creator 	: Hanruofei  
//  Date		: 12/28/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "KSceneObject.h"
class KStaticObject : public KSceneObject
{
public:
    KStaticObject(void);
    virtual ~KStaticObject(void);

    std::string ToStatusString();

    BOOL HasGuide() const {return false;} ;
};

