// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KMakingMachine.h
//  Creator		: hanruofei
//	Date		: 8/8/2012
//  Comment		: 
//	*********************************************************************
#pragma once
#include "GlobalMacro.h"

class KPlayer;
struct KRecipe;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

class KMakingMachine
{
public:
    KMakingMachine(void);
    ~KMakingMachine(void);

    BOOL Init(KPlayer* pOwner);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL LearnRecipe(DWORD dwRecipeID);
    BOOL ProduceItem(DWORD dwRecipeID);
    void GetInfo(uint32_t(& recipeState)[MAX_RECIPE_COUNT / UINT32_BIT_COUNT + 1]);
    BOOL HasLearnedRecipe(KRecipe* pRecipe);
    BOOL CanLearnRecipe(KRecipe* pRecipe, BOOL bNotifyOnFailed);

private:
    BOOL CanProduceWithRecipe(KRecipe* pRecipe, BOOL bNotifyOnFailed);
    BOOL HasEnoughRecipeMaterials(KRecipe* pRecipe);
    BOOL CostRecipeMoneyAndMaterial(KRecipe* pRecipe);
    BOOL AddProductAccordingRecipe(KRecipe* pRecipe, BOOL& bCrit);

private:
    KPlayer* m_pOwner;
    uint32_t m_RecipeState[MAX_RECIPE_COUNT / UINT32_BIT_COUNT + 1];
    int      m_nLastProduceTime;
};

