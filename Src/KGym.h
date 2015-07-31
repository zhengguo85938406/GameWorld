// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KGym.h 
//  Creator 	: Xiayong  
//  Date		: 09/04/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "game_define.h"

class KPlayer;
class KHero;
namespace T3DB
{
    class KPB_SAVE_DATA;
}

struct KUSING_EQUIP_INFO 
{
    DWORD       dwHeroTemplateID;
    KGYM_TYPE   eType;
    time_t      nEndTime;
    int         nCount;
};

class KGym
{
public:
    KGym();
    ~KGym();

    BOOL Init(KPlayer* pOwner);
    void UnInit();

    BOOL LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    void Activate();

    BOOL UpgradeEquip();

    BOOL HeroUseEquip(DWORD dwHeroTemplateID, KGYM_TYPE eType, int nCount);
    KUSING_EQUIP_INFO* GetHeroUsingEquipInfo(DWORD dwHeroTemplateID);
    int  GetUsingEquipCount(KGYM_TYPE eType);
    BOOL HeroUseEquipFinished(DWORD dwHeroTemplateID, KGYM_TYPE eType, int nCount);

    BOOL DirectEndUseEquipRequest(DWORD dwHeroTemplateID);
    BOOL DirectEndUseEquipResult(DWORD dwHeroTemplateID, int nCostMoney);

    BOOL GetSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSiz);
    int  GetEquipLevel(){return m_nEquipLevel;};

    void OnUpgradeQueueFinished(KUPGRADE_TYPE eType, uint32_t uHeroTemplateID);

private:
    void DelUsingEquipInfo(DWORD dwHeroTemplateID);

private:
    KPlayer* m_pPlayer;
    int m_nEquipLevel;
    
    typedef std::vector<KUSING_EQUIP_INFO> KVEC_EQUIP_INFO;
    KVEC_EQUIP_INFO      m_vecEquipInfo;
};
