// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KRecipeData.h
//  Creator		: hanruofei
//	Date		: 8/7/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include "GlobalMacro.h"
#include "GlobalStruct.h"

struct KRecipe
{
    DWORD     dwID;
    int       nType;
    int       nRequireLevel;
    BOOL      bAutoLearn;
    int       nCostGoldCount;
    KMaterial Materials[2];
    KProduct  Product;
    int       nLowerValuePoint;
    int       nUpperValuePoint;
};

class KRecipeData
{
public:
    KRecipeData(void);
    ~KRecipeData(void);
    BOOL Init();
    void UnInit();
    KRecipe* GetRecipe(DWORD dwRecipeID);

private:
    BOOL LoadData();
    KRecipe m_Data[MAX_RECIPE_COUNT];
};

