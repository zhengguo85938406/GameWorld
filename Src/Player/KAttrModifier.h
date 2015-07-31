// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KAttrModifier.h 
//  Creator 	: Xiayong  
//  Date		: 09/20/2011
//  Comment	: 
// ***************************************************************
#pragma once

#include "KAttribute.h"

class KHero;

void InitAttributeFunctions();

typedef BOOL (*ATTR_PROCESS_FUN)(KHero *pDstHero,BOOL bApply, int nValue1, int nValue2);

extern ATTR_PROCESS_FUN g_AttributeProcessFunctions[];
