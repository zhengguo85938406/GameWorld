#include "stdafx.h"
#include "KMakingMachine.h"
#include "KSO3World.h"
#include "KPlayerServer.h"
#include "KPlayer.h"

KMakingMachine::KMakingMachine(void)
{
    m_pOwner            = NULL;
    m_nLastProduceTime  = 0;
    memset(m_RecipeState, 0, sizeof(m_RecipeState));
}

KMakingMachine::~KMakingMachine(void)
{
}

BOOL KMakingMachine::Init(KPlayer* pOwner)
{
    assert(pOwner);
    m_pOwner = pOwner;
    return true;
}

void KMakingMachine::UnInit()
{
    m_pOwner = NULL;
}

BOOL KMakingMachine::LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf)
{
    BOOL   bResult   = false;
    int    nMaxLevel = 0;

    assert(pLoadBuf);

    const T3DB::KPBMakingMachine& MakingMachine = pLoadBuf->makingmachine();
    const T3DB::KPB_MAKING_MACHINE_DATA& MakingMachineData = MakingMachine.makingmachinedata();
    const std::string& strRecipeState = MakingMachineData.recipestate();

    memcpy(m_RecipeState, strRecipeState.data(), MIN(sizeof(m_RecipeState),strRecipeState.size()));

    if (m_pOwner->m_eConnectType == eLogin)
        g_PlayerServer.DoSyncMakingMachineInfo(m_pOwner);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMakingMachine::SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf)
{
    BOOL bResult  = false;
    T3DB::KPBMakingMachine* pMakingMachine = NULL;
    T3DB::KPB_MAKING_MACHINE_DATA* pMakingMachineData = NULL;

    KGLOG_PROCESS_ERROR(pSaveBuf);

    pMakingMachine = pSaveBuf->mutable_makingmachine();
    pMakingMachineData = pMakingMachine->mutable_makingmachinedata();

    pMakingMachineData->set_recipestate(m_RecipeState,sizeof(m_RecipeState));

    bResult = true;
Exit0:
    return bResult;
}
BOOL KMakingMachine::LearnRecipe(DWORD dwRecipeID)
{
    BOOL bResult = false;
    BOOL bRetCode = false;
    unsigned uUInt32Index = 0;
    unsigned uBitIndex = 0;
    KRecipe* pRecipe = NULL;

    KGLOG_PROCESS_ERROR(dwRecipeID);

    pRecipe = g_pSO3World->m_Settings.m_RecipeData.GetRecipe(dwRecipeID);
    KGLOG_PROCESS_ERROR(pRecipe);
    
    bRetCode = CanLearnRecipe(pRecipe, true);
    KGLOG_PROCESS_ERROR(bRetCode);   

    uUInt32Index = (dwRecipeID - 1) / UINT32_BIT_COUNT;
    uBitIndex  = (dwRecipeID - 1) % UINT32_BIT_COUNT;

    KGLOG_PROCESS_ERROR(uUInt32Index < countof(m_RecipeState));

    m_RecipeState[uUInt32Index] |= (1 << uBitIndex);

    g_PlayerServer.DoDownwardNotify(m_pOwner, KMESSAGE_RECIPE_LEARNED_SUCCES);
    g_PlayerServer.DoLearnRecipeNotify(m_pOwner, dwRecipeID);

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMakingMachine::HasLearnedRecipe(KRecipe* pRecipe)
{
    BOOL     bResult    = false;
    unsigned uUInt32Index = 0;
    unsigned uBitIndex  = 0;
    DWORD    dwRecipeID = ERROR_ID;

    KGLOG_PROCESS_ERROR(pRecipe);

    if (pRecipe->bAutoLearn)
    {
        bResult = true;
        goto Exit0;
    }

    dwRecipeID = pRecipe->dwID;
    KGLOG_PROCESS_ERROR(dwRecipeID);

    uUInt32Index = (dwRecipeID - 1) / UINT32_BIT_COUNT;
    uBitIndex  = (dwRecipeID - 1) % UINT32_BIT_COUNT;

    KGLOG_PROCESS_ERROR(uUInt32Index < countof(m_RecipeState));

    bResult = (m_RecipeState[uUInt32Index] >> uBitIndex) & 1;
Exit0:
    return bResult;
}

BOOL KMakingMachine::ProduceItem(DWORD dwRecipeID)
{
    BOOL     bResult  = false;
    BOOL     bRetCode = false;
    BOOL     bCrit    = false;
    KRecipe* pRecipe  = NULL;

    pRecipe = g_pSO3World->m_Settings.m_RecipeData.GetRecipe(dwRecipeID);
    KGLOG_PROCESS_ERROR(pRecipe);

    bRetCode = CanProduceWithRecipe(pRecipe, true);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = CostRecipeMoneyAndMaterial(pRecipe);
    KGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = AddProductAccordingRecipe(pRecipe, bCrit);
    KGLOG_PROCESS_ERROR(bRetCode);
    
    m_nLastProduceTime = g_pSO3World->m_nCurrentTime;

    if (pRecipe->nType == KRECIPE_TYPE_EQUIP)
    {
        m_pOwner->OnEvent(peMakeEquipment, pRecipe->Product.dwItemType, pRecipe->Product.dwItemIndex);
    }

    bResult = true;
Exit0:
    g_PlayerServer.DoProduceItemRespond(m_pOwner, bResult, bCrit);
   	return bResult;
}

BOOL KMakingMachine::CostRecipeMoneyAndMaterial(KRecipe* pRecipe)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    bRetCode = m_pOwner->m_MoneyMgr.AddMoney(emotMoney, -pRecipe->nCostGoldCount);
    KGLOG_PROCESS_ERROR(bRetCode);

    for (int i = 0; i < countof(pRecipe->Materials); ++i)
    {
        if (!pRecipe->Materials[i].dwItemType)
            continue;

        bRetCode = m_pOwner->m_ItemList.CostPlayerItem(pRecipe->Materials[i].dwItemType, pRecipe->Materials[i].dwItemIndex, pRecipe->Materials[i].nCount);
        KGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMakingMachine::CanProduceWithRecipe(KRecipe* pRecipe, BOOL bNotifyOnFailed)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pRecipe);
    
    bRetCode = HasLearnedRecipe(pRecipe);
    if (!bRetCode)
    {
        if (bNotifyOnFailed)
            g_PlayerServer.DoDownwardNotify(m_pOwner, KMESSAGE_NOT_LEARN_THIS_RECIPE);

        goto Exit0;
    }

    bRetCode = m_pOwner->m_MoneyMgr.CanAddMoney(emotMoney, -pRecipe->nCostGoldCount);
    if (!bRetCode)
    {
        if (bNotifyOnFailed)
            g_PlayerServer.DoDownwardNotify(m_pOwner, KMESSAGE_NOT_ENOUGH_GOLD_COIN);

        goto Exit0;
    }

    bRetCode = HasEnoughRecipeMaterials(pRecipe);
    if (!bRetCode)
    {
        if (bNotifyOnFailed)
            g_PlayerServer.DoDownwardNotify(m_pOwner, KMESSAGE_NOT_ENOUGH_MATERIALS);

        goto Exit0;
    }

    bRetCode = m_pOwner->HasFreePosAddItem(pRecipe->Product.dwItemType, pRecipe->Product.dwItemIndex);
    if (!bRetCode)
    {
        if (bNotifyOnFailed)
            g_PlayerServer.DoDownwardNotify(m_pOwner, KMESSAGE_NOT_ENOUGH_BAG);

        goto Exit0;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMakingMachine::AddProductAccordingRecipe(KRecipe* pRecipe, BOOL& bCrit)
{
	BOOL    bResult   = false;
	BOOL    bRetCode  = false;
    int     nCritRate = 0;
    int     nCritValuePointPercent = 0;
	IItem*  pItem     = NULL;
    int     nValuePoint = 0;
    int     nProduceItemCount = 0;
    KVIPConfig* pVIPInfo = NULL;
    int nVIPLevel = 0;
    const KItemProperty*    pProperty   = NULL;
    KGItemInfo*             pItemInfo   = NULL;
    int     nCount    = 0;

    assert(pRecipe);

    nCritRate = g_pSO3World->m_Settings.m_ConstList.nMKCritRate;
    nCritValuePointPercent = g_pSO3World->m_Settings.m_ConstList.nMKCritValuePointPercent;

    if (m_pOwner->IsVIP())
        nVIPLevel = m_pOwner->m_nVIPLevel;

    pVIPInfo = g_pSO3World->m_Settings.m_VIPConfig.GetByID(nVIPLevel);
    KGLOG_PROCESS_ERROR(pVIPInfo);
    nCritRate += pVIPInfo->nGoodMakeItemRate;

    bCrit = ((int)g_Random(1000) <= nCritRate);
    if (pRecipe->nType == KRECIPE_TYPE_CONSUME)
    {
        nProduceItemCount = pRecipe->Product.nCount;
        if (bCrit)
            nProduceItemCount *= 2;

        pItem = m_pOwner->AddItem(pRecipe->Product.dwItemType, pRecipe->Product.dwItemIndex, nProduceItemCount, 0);
        KGLOG_PROCESS_ERROR(pItem);
    }
    else
    {
        int nLowerValuePoint = pRecipe->nLowerValuePoint;
        int nUpperValuePoint = pRecipe->nUpperValuePoint;

        if (bCrit)
        {
            nLowerValuePoint += (nUpperValuePoint - nLowerValuePoint) * nCritValuePointPercent / HUNDRED_NUM;
        }

        nValuePoint = nLowerValuePoint + g_Random(nUpperValuePoint - nLowerValuePoint);
        pItem = m_pOwner->AddItem(pRecipe->Product.dwItemType, pRecipe->Product.dwItemIndex, 1, nValuePoint);
        KGLOG_PROCESS_ERROR(pItem);
    }

    assert(pItem);
    pProperty = pItem->GetProperty();
    assert(pProperty);
    pItemInfo = pProperty->pItemInfo;
    assert(pItemInfo);
    nCount = pItem->GetStackNum();

    g_LogClient.DoFlowRecord(
        frmtMakingAndStrengthen, masfrMakeEquip, 
        "%s,%d,%u,%u,%d,%u,%u,%d,%d,%s", 
        m_pOwner->m_szName,
        pRecipe->nCostGoldCount,
        pRecipe->Materials[0].dwItemType,
        pRecipe->Materials[0].dwItemIndex,
        pRecipe->Materials[0].nCount,
        pRecipe->Materials[1].dwItemType,
		pRecipe->Materials[1].dwItemIndex,
		pRecipe->Materials[1].nCount,
        nCount,
        pItemInfo->szName
    );

	bResult = true;
Exit0:
	return bResult;
}

void KMakingMachine::GetInfo(uint32_t(& recipeState)[MAX_RECIPE_COUNT / UINT32_BIT_COUNT + 1])
{
    memcpy(recipeState, m_RecipeState, sizeof(recipeState));
}

BOOL KMakingMachine::HasEnoughRecipeMaterials(KRecipe* pRecipe)
{
    BOOL bResult    = false;
    int  nItemCount = 0;

    assert(pRecipe);
    for (int i = 0; i < countof(pRecipe->Materials); ++i)
    {
        if (!pRecipe->Materials[i].dwItemType)
            continue;

        nItemCount = m_pOwner->m_ItemList.GetPlayerItemCount(pRecipe->Materials[i].dwItemType, pRecipe->Materials[i].dwItemIndex);
        KGLOG_PROCESS_ERROR(nItemCount >= pRecipe->Materials[i].nCount);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL KMakingMachine::CanLearnRecipe(KRecipe* pRecipe, BOOL bNotifyOnFailed)
{
    BOOL bResult  = false;
    int  nResult  = false;
    int  nRetCode = false;

    KG_PROCESS_ERROR_RET_CODE(pRecipe, KMESSAGE_RECIPE_NOT_EXISTED);

    nRetCode = HasLearnedRecipe(pRecipe);
    KG_PROCESS_ERROR_RET_CODE(!nRetCode, KMESSAGE_RECIPE_ALREADY_LEARNED);

    KG_PROCESS_ERROR_RET_CODE(m_pOwner->m_nLevel >= pRecipe->nRequireLevel, KMESSAGE_RECIPE_MAKINGMACHINE_LEVEL_NOT_ENOUGH);
    
    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bNotifyOnFailed)
            g_PlayerServer.DoDownwardNotify(m_pOwner, nResult);
    }
    return bResult;
}

