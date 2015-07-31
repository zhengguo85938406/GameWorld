///////////////////////////////////////////////////////////////////////////////
//  Copyright(c)    Kingsoft
//  FileName	:   KWardrobe.h
//  Creator 	:   SunXun  
//  Date		:   9/20/2012   11:09
//  Comment		: 
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Ksdef/TypeDef.h"
#include "Engine/KMemory.h"

struct KPLAYER_FASHION_INFO 
{
    uint32_t    dwFashionID;
    uint32_t    dwOverduePoint;
    uint32_t    dwBelongtoHeroID;
};
typedef KMemory::KAllocator<std::pair<uint32_t, KPLAYER_FASHION_INFO> > KFASHION_INFO_ALLOCATOR;

typedef std::map<uint32_t, KPLAYER_FASHION_INFO, std::less<WORD>, KFASHION_INFO_ALLOCATOR> WARDROBE_INFO_TABLE;

class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
}
class KWardrobe
{
public:
    BOOL Init(KPlayer* pPlayer);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL OnBuyFashionRequest(uint32_t dwFashionIndex);
    BOOL AddFashion(uint32_t dwFashionIndex);
    BOOL OnChargeFashionRequest(uint32_t dwFashionIndex, uint32_t dwChargeType);
    BOOL OnChargeFashionSuccess(uint32_t dwFashionIndex);
    BOOL ChangeFashion(uint32_t dwHeroTemplateID, uint32_t dwTargetFashionID);
    BOOL CheckValidity();

    BOOL IsFashionExist(uint32_t dwFashionID);
    BOOL GetSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize);

private:
    BOOL IsWardrobeFull()
    {
        return (int)m_WardrobeInfoMap.size() <= m_nMaxFashionCount;
    }

private:
    WARDROBE_INFO_TABLE m_WardrobeInfoMap;

    int         m_nMaxFashionCount;
    KPlayer*    m_pPlayer;
};
