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
    DWORD               dwID;    // �Ӽ���ID
    KSKILL_TRIGGER_TYPE eType;   //��������
    int                 nFrame; // �����������ΪKSKILL_TRIGGER_TYPE_TIMER, �����ָ����Timer�Ĵ������
    DWORD               dwRate;  // �Ӽ��ܳ������ʣ���λ ���֮һ

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

    KSKILL_END_TYPE     m_eEndType;             // ���ܽ�������
    int                 m_nEndFrame;             // ����ǰ�ʱ������ļ��ܣ��������������ָ�����ʱ��ֵ������������������������

    int                 m_nInterruptLevel;      // ��ϼ��𣬼�����ܴ�ϼ���͵�
       
    KSubSkillInfo       m_SubSkill[MAX_SUBSKILL_COUNT];
    
    DWORD               m_dwSrcBuffID;          // �ͷ�ʱ���ͷ��߼ӵ�buff
    DWORD               m_dwDestBuffID;         // ��Ŀ��ӵ�buff
    KAttribute*         m_pSrcAttribute;        // �ͷż���ʱ���ͷ���Ӧ�õ�ħ������,���ع�
    KAttribute*         m_pDestAttribute;       // ���е�Ŀ����Ӧ�õ�����,���ع�

    DWORD               m_dwAimType;            // 
    BOOL                m_bCanCastWithBall;     // �����ܷ��ͷ�

    KSkillType          m_eSkillType;
    int                 m_nAttackIntervalFrame; // �������

    int                 m_nTargetCostEndurance; // Ŀ�걻��ʱ���ĵ�����
    int                 m_nTargetCostStamina;   // Ŀ�걻��ʱ���ĵ�����
    int                 m_nTargetCostEndurancePercent; // Ŀ�걻��ʱ���ĵ������İٷֱ�, 1024Ϊ��
    int                 m_nTargetCostStaminaPercent;   // Ŀ�걻��ʱ���ĵ������İٷֱ�, 1024Ϊ��

    int                 m_nMoveState;           // �ͷŴ˼��ܵ�ʱ��Ӣ����Ҫ���ڵ�״̬�� cmsInvalid��0����ʾ�޴�Ҫ��
    int                 m_nMoveStateFrame;      // �ͷŴ˼��ܵ�ʱ����Ҫ���ﴦ��ָ��״̬��֡��С�ڸ�֡��

    DWORD               m_dwStepID;         // ���ܲ���
    int                 m_nStepMinFrame;        
    int                 m_nStepMaxFrame;        // ǰ�ü���ʱ�䣬��λ֡�� [m_nStepMinFrame, m_nStepMaxFrame]

    // ŭ�����
    int                 m_nSelfAddAngry;        // �ͷż���ʱ���Լ��ӵ�
    int                 m_nTargetAddAngry;      // ����ʱ��Ŀ��ӵ�
    int                 m_nNeedAngry;           // ��Ҫ����ŭ�������ͷż���
    int                 m_nCostAngry;           // �ͷż������ĵĵ���
    int                 m_nCostAngryPercent;    // �ͷż�������ŭ���ٷֱ�, �������ĵ���

    DWORD               m_dwTargetBuffLimit;    // ����ֻ����������ָ��buff��Ŀ������

    int                 m_nCanCastCountInAir;   // ��һ�ν�����еĹ����п����ͷŵĴ�������, <0�޴�������, =0�����ڿ����ͷ�, >0 һ�������������ͷŵĴ���

    int                 m_nSpasticityFrame;     // ����Ŀ���Ľ�ֱ֡��
    BOOL                m_bDelayApplySkillEffect; // �Ƿ��ӳ�Ӧ�ü���Ч��
    BOOL                m_bInvincibleOnSpasticity; // ��ֱ�Ĺ������Ƿ��޵�
    BOOL                m_bIsCommonSkill;
    int                 m_nRequireLevel;        // �ͷŸļ�����Ҫ�ĵȼ�
    int                 m_nJudgeBoxMinX; // �ж�����СX��ֵ������ҿ�ȵİٷֱȼ��㣬����ͬ��
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

    BOOL                m_bNirvanaSkill;     // �Ƿ��Ǳ�ɱ��
    DWORD               m_dwScriptID;
};
