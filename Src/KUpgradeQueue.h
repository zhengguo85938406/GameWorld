// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KUpgradeQueue.h 
//  Creator 	: Xiayong  
//  Date		: 09/07/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include <map>
#include "game_define.h"
#include "Ksdef/TypeDef.h"

class KPlayer;
namespace T3DB
{
    class KPB_SAVE_DATA;
}
struct KUpgradeNode 
{
    KUPGRADE_TYPE eType;
    int nEndTime;
    uint32_t dwHeroTemplateID;
};

class KUpgradeQueue
{
public:
    KUpgradeQueue();
    ~KUpgradeQueue();

    BOOL    Init(KPlayer* pPlayer);
    void    UnInit();

    void    Activate();

    BOOL    LoadFromProtoBuf(T3DB::KPB_SAVE_DATA* pLoadBuf);
    BOOL    SaveToProtoBuf(T3DB::KPB_SAVE_DATA* pSaveBuf);

    BOOL    HasFreeSlot();
    BOOL    CanAddUpgradeNode(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID);
    BOOL    AddUpgradeNode(KUPGRADE_TYPE eType, int nCDTime, uint32_t dwHeroTemplateID);
    
    KUpgradeNode* GetNode(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID);

    BOOL    FreeClearCDRequest(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID);
    void    ResetFreeClearCDTimes();

    BOOL    ClearCDRequest(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID);
    BOOL    ClearCDRespond(KUPGRADE_TYPE eType, uint32_t dwHeroTemplateID, int nCostMoney);

    void    SetMaxUpgradeCount(int nNewMaxUpgradeCount);

    BOOL    GetSyncData(BYTE* pbyBuffer, size_t uBufferSize, size_t& uUsedSize);

private:
    KPlayer*    m_pPlayer;
    int         m_nMaxUpgradeCount;
    int         m_nFreeClearCDTimes;

    typedef std::multimap<int, KUpgradeNode> KMAP_UPGRADE_NODE;
    KMAP_UPGRADE_NODE m_mapUpgradeNode;
};
