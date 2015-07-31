// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KSkill.h 
//  Creator 	: Xiayong  
//  Date		: 09/16/2011
//  Comment	: 
// ***************************************************************
#pragma once
#include "GlobalMacro.h"
#include "GlobalStruct.h"
#include "Protocol/gs2cl_protocol.h"
#include "game_define.h"
#include "KTools.h"
#include "KGEnumConvertor.h"
#include "KAttribute.h"

#include <vector>

struct KSubSkillInfo
{
    DWORD               dwID;    // 子技能ID
    KSKILL_TRIGGER_TYPE eType;   //触发类型
    int                 nFrame; // 如果触发类型为KSKILL_TRIGGER_TYPE_TIMER, 则该项指定了Timer的触发间隔
    DWORD               dwRate;  // 子技能出发几率，单位 万分之一

};

struct KAnchor
{
    int nFrame;
    int nVelocityX;
    int nVelocityY;
    int nVelocityZ;
};

struct KSkillSegmentInfo
{
    int     nFrame;
    BOOL    bAttackState;
    int     nBeAttackType;
    DWORD   dwSelfBuffID;
    DWORD   dwTargetBuffID;
    int     nAttackIntervalFrame;
    KVEC_ATTRIBUTES   vecSelfAttributes;
    KVEC_ATTRIBUTES   vecTargetAttributes;
    KVEC_BULLETS      vecBullet;
    int               nStiffFrame;
};

typedef std::map<int, KAnchor> KMAP_ANCHOR;
typedef std::map<int, KSkillSegmentInfo> KMAP_SEGMENT;

struct KSkillBinaryData
{
    DWORD dwID;
    DWORD dwStep;
    int   nTotalFrame;
    KMAP_ANCHOR  Anchors;
    KMAP_SEGMENT Segments;
};

typedef std::map<uint64_t, KSkillBinaryData> KMAP_SKILL_DATA;

enum KTRIGGER_GRABSKILL
{
    KTRIGGER_GRABSKILL_INVALID,
    KTRIGGER_GRABSKILL_YES,
    KTRIGGER_GRABSKILL_NO,
};

class KHero;
struct KAttribute;
struct KBODY;
class KSkill
{
public:
    KSkill();
    ~KSkill();

    BOOL Init();
    void UnInit();

    BOOL IsOverTimeSkill();
    BOOL CanBeApplyOnEnemy();
    BOOL CanBeApplyOnMe();
    BOOL CanBeApplyOnUs();
    BOOL CanBeApplyOn(KRelationType eType);
    BOOL CheckStateCondition(KHeroMoveState eMoveState, int nDeltaFrame);
    BOOL IsType(KSkillType eType);
    KMAP_ANCHOR* GetAnchorData();
    void SetBinaryData(const KSkillBinaryData& rBinaryData);
    BOOL GetJudageBox(KHero* pAttcker, KBODY& rRetBody);
    
    BOOL IsAttackStateAtFrame(int nFrameIndex);
    BOOL WouldGrabAtFrame(int nFrameIndex);
    KSkillSegmentInfo* GetSegmentOfFrameBelongTo(int nFrameIndex);
    KAnchor* GetAnchorAtFrame(int nFrameIndex);
    KSkillSegmentInfo* GetSegmentAtFrame(int nFrameIndex);
    BOOL UpdateOnSkillLoaded();
    BOOL HasSubSkill(DWORD dwSkillID, DWORD dwSkillStep);
    BOOL CanPluginInSlot(int nSlotIndex);
    BOOL GetAIEventRange(KHero* pCaster, KBODY& rRetRange);

    DWORD               m_dwID;

    KSKILL_END_TYPE     m_eEndType;             // 技能结束类型
    int                 m_nEndFrame;             // 如果是按时间结束的技能，则这个属性用来指定这个时间值，其他情况下这个属性无意义

    int                 m_nInterruptLevel;      // 打断级别，级别高能打断级别低的
       
    KSubSkillInfo       m_SubSkill[MAX_SUBSKILL_COUNT];
    
    DWORD               m_dwSrcBuffID;          // 释放时给释放者加的buff
    DWORD               m_dwDestBuffID;         // 给目标加的buff
    KAttribute*         m_pSrcAttribute;        // 释放技能时给释放者应用的魔法属性,不回滚
    KAttribute*         m_pDestAttribute;       // 击中的目标需应用的属性,不回滚

    DWORD               m_dwAimType;            // 
    BOOL                m_bCanCastWithBall;     // 持球能否释放

    KSkillType          m_eSkillType;
    int                 m_nAttackIntervalFrame; // 攻击间隔

    int                 m_nTargetCostEndurance; // 目标被击时消耗的耐力
    int                 m_nTargetCostStamina;   // 目标被击时消耗的体力
    int                 m_nTargetCostEndurancePercent; // 目标被击时消耗的耐力的百分比, 1024为底
    int                 m_nTargetCostStaminaPercent;   // 目标被击时消耗的体力的百分比, 1024为底

    int                 m_nMoveState;           // 释放此技能的时候，英雄需要处于的状态， cmsInvalid（0）表示无此要求
    int                 m_nMoveStateFrame;      // 释放此技能的时候，需要人物处于指定状态的帧数小于该帧数

    DWORD               m_dwStepID;         // 技能步骤
    int                 m_nStepMinFrame;        
    int                 m_nStepMaxFrame;        // 前置技能时间，单位帧数 [m_nStepMinFrame, m_nStepMaxFrame]

    // 怒气相关
    int                 m_nSelfAddAngry;        // 释放技能时给自己加的
    int                 m_nTargetAddAngry;      // 命中时给目标加的
    int                 m_nNeedAngry;           // 需要多少怒气才能释放技能
    int                 m_nCostAngry;           // 释放技能消耗的点数
    int                 m_nCostAngryPercent;    // 释放技能消耗怒气百分比, 优先消耗点数

    DWORD               m_dwTargetBuffLimit;    // 技能只能作用在有指定buff的目标身上

    int                 m_nCanCastCountInAir;   // 在一次进入空中的过程中可以释放的次数上限, <0无次数限制, =0不能在空中释放, >0 一次上天最多可以释放的次数

    int                 m_nSpasticityFrame;     // 命中目标后的僵直帧数
    BOOL                m_bDelayApplySkillEffect; // 是否延迟应用技能效果
    BOOL                m_bInvincibleOnSpasticity; // 僵直的过程中是否无敌
    BOOL                m_bIsCommonSkill;
    int                 m_nRequireLevel;        // 释放改技能需要的等级
    int                 m_nJudgeBoxMinX; // 判定框最小X的值（以玩家宽度的百分比计算，下类同）
    int                 m_nJudgeBoxMaxX;
    int                 m_nJudgeBoxMinY;
    int                 m_nJudgeBoxMaxY;
    int                 m_nJudgeBoxMinZ;
    int                 m_nJudgeBoxMaxZ;
    BOOL                m_bCanSpecifyCastDir;
    BOOL                m_bCanSpecifyCastingDir;
    int                 m_nRestraintType;
    BOOL                m_bClearVelocityAtEnd;
    int                 m_nGravityPercent;
    int                 m_nSeries;
    BOOL                m_bFireAIEvent;
    int                 m_nAIEventMinX;
    int                 m_nAIEventMaxX;
    int                 m_nAIEventMinY;
    int                 m_nAIEventMaxY;
    int                 m_nAIEventMinZ;
    int                 m_nAIEventMaxZ;

    std::vector<int>    m_vecBelongToSlots;

    KMAP_ANCHOR         m_mapAnchors;
    KMAP_SEGMENT        m_mapSegments;
    BOOL                m_bCauseGrabSkill;

    BOOL                m_bNirvanaSkill;     // 是否是必杀技
    DWORD               m_dwScriptID;
};
