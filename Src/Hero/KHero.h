// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KHero.h 
//	Creator 	: Xiayong  
//  Date		: 08/07/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include <list>
#include <bitset>
#include "SO3Result.h"
#include "KMovableObject.h"
#include "KSceneGlobal.h"
#include "GlobalEnum.h"
#include "KBuffList.h"
#include "game_define.h"
#include "KBall.h"
#include "KTraceableArg.h"
#include "Luna.h"
#include "KAIVM.h"
#include "KHeroTemplate.h"
#include "KSkill.h"
#include "KPassiveSkill.h"
#include "KPathFinder.h"
#include <map>
#include <functional>
#include <string>

struct KAttribute;
struct KPOSITION;
class KDoodad;
class KBasketSocket;
class KBall;
class KFix2PParabola;
class KChest;
class KPlayer;
struct KMove_Critical_Param;
class KHero;
class KSkill;
class KBullet;
struct KCharacterInfo;
struct KNpcTemplate;
class KDummy;
class KBuffBox;
struct KGeneralAffect;
class KBarrier;
struct KSlamBallSkill;

struct KGAME_STATISTICS 
{
    int      m_nUseItemCount;
    int      m_nDieTimes;
    int      m_nScore;
    int      m_nKnockDownOther;
    int      m_nNormalShoot;
    int      m_nSlamBall;
    int      m_nPassBall;
    int      m_nSkillShoot;
    int      m_nUseSceneDoodad;
    int      m_nPluginBasket;
};

typedef std::map<std::string, int, std::less<std::string>, KMemory::KAllocator<std::pair<std::string, int> > > KMAP_AI_SELFDEF_DATA;

class KHero : public KMovableObject
{
public:
	KHero();
	virtual ~KHero();

	BOOL Init();
	void UnInit();

	void Activate(int nCurLoop);

    virtual KSceneObjectType GetType() const;

    // 会影响其它人移动状态的操作放这里,
    // 与正常Activate是因为希望对他人产生的移动变化在下一帧才生效
    void SpecialActivate(int nCurLoop);

    void TryTriggerGrabSkill();

    BOOL IsVip();
    int  GetVIPLevel();
    int  GetNormalLevel();
    int  GetLadderLevel();
    virtual int GetCurrentGravity();

public:
    BOOL UpdateMoveParameter();
    BOOL Stand(BOOL bSyncSelf);
    BOOL Jump(BOOL bMove, int nDirection, BOOL bSyncSelf);
    BOOL TakeBall(BOOL bNotifySelf);
	void PassBallTo(DWORD dwID);
    BOOL AimAt(BOOL bAimAt, int nDirection, DWORD dwHoldingObjID, BOOL bSyncSelf);
    BOOL ShootBasket(DWORD dwBasketID);

    BOOL CanCastSpecialSkill(KSPECIAL_SKILL eSkillID);
    void OnCastSpecialSkill(KSPECIAL_SKILL eSkillID);

    BOOL OwnerOperateAITeammate(int nOPIdx);

    BOOL ApplyGeneralAffect(DWORD dwCasterID, KMovableObject* pAttackerObj, DWORD dwAffectID);
    BOOL ApplyPassiveSkill();

    BOOL CastSlamBallSkill(DWORD dwSlamBallSkillID, DWORD dwTargetSocketID, DWORD dwInterferenceID);

    int GetDecreasedHitRate(DWORD dwInterferenceID);

private:
    void SetMoveState(int nState);
    void InvalidateMoveState(int nDelayFrame, BOOL bSyncSelf);
    BOOL WalkTo(int nDirection, BOOL bSyncSelf);   
    BOOL RunTo(int nDirection, BOOL bSyncSelf);
    BOOL SlamBall(KBall* pBall, KBasketSocket* pBasketSocket);

    void Sit();
    void Halt();

    BOOL EnsureUntakeBall();
    BOOL EnsureNoFollows();
    BOOL EnsureNoGuide();
    BOOL FixAutoTakeBallBug();

    BOOL DetachObject(DWORD dwID);
    
    void StopLoseControlByCounter();

    void StartStiff(int nStiffFrame);
    void StopStiff();
    
    BOOL CanCastSkill(KSkill* pSkill, KTWO_DIRECTION eFaceDir, BOOL bSubSkill);

    // 处理角色的状态机
    void RunStateMachine();

    int GetRandomNum(int nMin, int nMax) const;
    KTWO_DIRECTION GetNextFaceDir(int nDirection);

    void CheckObjStandOn(const KPOSITION& rPos);

    BOOL IsForbitCastSkill() const;
    BOOL IsForbitPick() const;
    BOOL IsAlive() const;

    BOOL CanBeAttached(KChest* pObject);

    void    AttackHero(KSkill* pSkill, KHero* pObj);
    void    AttackDummy(KSkill* pSkill, KDummy* pObj);
    void    AttackBarrier(KSkill* pSkill, KBarrier* pObj);
    void    AttackBuffBox(KSkill* pSkill, KBuffBox* pObj);

    int     GetAutoMoveDir(KAutoMoveDirection eType);

    BOOL    HasBuff(DWORD dwBuffID) const;

    BOOL CanCastSlamBallSkill(KSlamBallSkill* pSkill, KBasketSocket* pSocket);

public:
    void BeAttacked(KMovableObject* pAttacker);
    void BeAttackedByHero(KHero* pAttacker, KSkill* pSkill, KSkillSegmentInfo* pSegment = NULL);
    void KnockedOff(int nVelocityXY, int nVelocityZ);
    void KnockedOff(int nVelocityX, int nVelocityY, int nVelocityZ);
    void WeakKnockDown(int nVelocityXY, int nVelocityZ);
    void KnockedDown();
    
    int  ProcessEnduranceRemission(int nDamage, KHero* pAttacker);
    int  ProcessStaminaRemission(int nDamage, KHero* pAttacker);

    BOOL AddAngry(int nShouldAddAngry);

    BOOL AddBuff(DWORD dwBuffID);

    BOOL EnsureNoAttached(); 
    BOOL TakeObject(DWORD dwID);

    KDoodad* GetHoldingDoodad();
    DWORD GetHoldingObjID();
    BOOL DropObject(DWORD dwID);

    void EnsureNotStandOnObject();

    BOOL AutoMove(int nVelocity, KAutoMoveDirection eDirType);

    BOOL LoseControlByCounter(int nFrame);

    BOOL DoSkillCost(KSkill* pSkill);
    BOOL CastSkill(DWORD dwSkillID, DWORD dwStep, KTWO_DIRECTION eFaceDir, BOOL bSubSkill = false, BOOL bSyncSelf = false);
    BOOL CanCastGrabSkill(KSkill* pSkill, KHero* pTargetHero);
    BOOL CastGrabSkill(DWORD dwSkillID, DWORD dwStep, DWORD dwTargetHeroID);
    BOOL TrackMoveByCastingSkill(DWORD dwPathID);

    BOOL HasAttachedByDoodad(DWORD dwID);

    BOOL HasSkill(DWORD dwSkillID);
    BOOL HasSlamBallSkill(DWORD dwSkillID);
    BOOL HasOtherActiveSkill(DWORD dwSkillID);

    void OnLoseControlByCounter();
    void OnStiff();
    void OnSit();
    void OnHalt();
    void OnKnockedOff();
    void OnKnockedDown();

    BOOL UpdateVelocityByAnchors();

    void UpdateFaceDir(int nDirection);
    KTWO_DIRECTION GetFaceDir() const;
    void SetFaceDir(KTWO_DIRECTION eNewDir, BOOL bSyncSelf);

    KRelationType GetRelationTypeWith(KHero* pOtherOne) const;

    BOOL ShootDoodad(DWORD dwID);

    void LoseEndurance(int nEndurance);
    void LoseStamina(int nStamina);
    void OnEnduranceDownToZero();
	void onStaminaDownToZero();

    void ApplyCannotSkill();
    void UnApplyCannotSkill();

    void ApplyCannotPick();
    void UnApplyCannotPick();

    void CalcHitRateParams(const KPOSITION& cSrc, const KPOSITION& cDst, int& nNowAttenuationTimes, int& nNowHitRate, int nInterference) const;

    void ApplyMoveAction(BYTE byMoveState, BYTE byDirection);

    BOOL CheckMoveAdjust(int nClientFrame, const KMove_Critical_Param& crParam, BOOL& rbReverseFailed);
    BOOL AdjustVirtualFrame(int nClientFrame, BOOL& rbReverseFailed);
    void SetRunMode(bool isRunMode);

    BOOL RandomHitted(int nNowHitRate) const;

    void Detach();
    void AttachAsFightingHero(KPlayer* pOwner);
    void AttachAsAITeammate(KPlayer* pOwner);

    void BeThrownChest(KChest* pObject);
   
    void ProcessAI();

    void OnAttacked(BOOL bIgnoreBaTi=false);

    BOOL UseDoodad(DWORD dwID, unsigned char direction);

    void SetLevel(int nNewLevel);
    void DoCycleRevise();
    void DoCycleSynchronous();

    BOOL FreezeByFrame(int nFrameCount);
    BOOL UnFreeze();

    BOOL TurnOnAI();
    BOOL TurnOffAI();

    void OnEnterScene(KScene* pScene);

    void StopMove(BOOL bSyncSelf);
    BOOL NormalShootBall(int nAimLoop, DWORD dwInterferenceHeroID);
    BOOL SkillShootBall(DWORD dwTargetBasketSocketID);
    BOOL SkillSlamBall(DWORD dwTargetBasketSocketID);
    BOOL ShootBallBySkill(DWORD dwPathID, BOOL bFollowBall);
    BOOL NormalSlamBall();

    BOOL DirectlyMoveToDest();
    void StopAutoMove();

    BOOL GetDirectionByType(int nDirectionType, int& rDirection, int nParam);
    BOOL NormalSlamBallEx(DWORD dwSocketID, DWORD dwInterferenceID);
    BOOL IsShootBallHitted(DWORD dwInterferenceID, int nAddtionalHitRate);
    BOOL SpecialUseDoodad();

    BOOL FireCastSkillAIEvent(KSkill* pSkill);
    BOOL FireUseDoodadAIEvent(KDoodad* pDoodad);
    void MoveTriggerPassive();

public:
	char						m_szName[_NAME_LEN];		// 角色名
	KGENDER	                    m_eGender;
	int							m_nLevel;					// 角色等级

    ////////////////////////////////////////////////////////////////////////// 基础属性

    int							m_nCurrentEndurance;		// 耐力

    int                         m_nMaxEnduranceBase;
    int                         m_nAddMaxEndurancePercent;
    int							m_nMaxEndurance;

    int                         m_nEnduranceReplenish;
    int                         m_nEnduranceReplenishPercent;

    int                         m_nCurrentStamina;          // 体力
    int                         m_nMaxStaminaBase;
    int                         m_nAddMaxStaminaPercent;
    int                         m_nMaxStamina;          
    int							m_nStaminaReplenish;			
    int							m_nStaminaReplenishPercent;	

    int							m_nCurrentAngry;	        // 怒气			
    int							m_nMaxAngry;
    int                         m_nAdditionalAddAngryPercent;      // 怒气额外回复, 是回复数值的百分比

    int                         m_nWillPowerBase;
    int                         m_nAddWillPowerPercent;
    int                         m_nWillPower;               // 意志力, 单位是千分之一

    int                         m_nInterferenceBase;
    int                         m_nAddInterferencePercent;
    int                         m_nInterference;            // 干扰力, 单位是千分之一
    int                         m_nInterferenceRange;
    int                         m_nInterferenceRangeBase;
    int                         m_nAddInterferenceRangePercent;

    int                         m_nAttackPoint;             // 攻击点数，1024为底
    int                         m_nAttackPointPercent;      // 攻击力影响百分比，100为底
    int                         m_nAgility;                 // 暴击概率，1024为底
    int                         m_nCritPoint;               // 暴击点数，按照公式映射到暴击率
    int                         m_nCritRate;                // 暴击倍率，100为底
    int                         m_nDefense;                 // 防御点数
    int                         m_nDefensePointPercent;     // 防御点数发挥出来的百分比
    int                         m_nDefensePercent;          // 防御力影响百分比，100为底

    int                         m_nMoveSpeed;
    int                         m_nMoveSpeedBase;
    int                         m_nAddMoveSpeedBasePercent;

    int                         m_nJumpSpeedBase;               
    int                         m_nAddJumpSpeedBasePercent;
    int                         m_nJumpCount;               
    int                         m_nJumpSpeed;               // 跳跃速度

    int                         m_nHitRate;                 // 投篮命中率,百分比
    int                         m_nHitRateAttenuation;      // 命中率衰减:每距m_fHitRateAttenuation个点衰减1%命中
    int                         m_nMaxHitOffset;            // 投篮允许偏离篮筐的最大偏移量

    int                         m_nNormalShootForce;
    int                         m_nSkillShootForce;
    int                         m_nSlamDunkForce;

    ////////////////////////////////////////////////////////////////////////// 基础属性结束

    // 同步用变量
    int                         m_nLastSyncCurrentEndurance;
    int                         m_nLastSyncCurrentStamina;
    int                         m_nLastSyncCurrentAngry;

    int                         m_nNextReplenishFrame;

    DWORD                       m_dwTemplateID;             // 主角英雄的模版id

    int                         m_nLastAimTime;             //瞄准时帧
    int                         m_nShootTime;               //投篮出手时帧

    int                         m_nAddGravityBase;
    int			                m_nGravityPercent;

    DWORD                       m_dwAttachedDoodadID;
    KHeroMoveState 		        m_eLastMoveState;   		// 上一次移动状态
    KHeroMoveState 		        m_eMoveState;				// 移动状态
    int							m_nMoveFrameCounter;

    int                         m_nMoveSyncFrame;
    BOOL                        m_bAimAt;

    KSkill*                     m_pCastingSkill;
    int                         m_nAttackSkillTime;         // 持续性技能的开始时间
    DWORD                       m_dwAttackerID;
    KTWO_DIRECTION              m_eBeAttackedToDir;         // 被往某方向攻击
    BOOL                        m_bCastingSkillTrackMoved;
    BOOL                        m_bWeakKnockDown;

    KBuffList                   m_BuffList;

    KTWO_DIRECTION              m_eFaceDir;
    int                         m_nVelocityDir; // 速度方向

    int                         m_nVirtualFrame;
    int                         m_nCreateTime;

    int                         m_nCopyIndex;

    BOOL                        m_bInvincible;

    bool                        m_bIsRunMode;
    int                         m_MoveCount;
    int                         m_nSide;
    int                         m_nPos;
    BOOL                        m_bAiMode;
    BOOL                        m_bAiTurnOffFlag;
    KAIVM                       m_AIVM;
    KNpcTemplate*               m_pNpcTemplate;     // npc英雄的模版id
    int                         m_nProtectEndFrame; // 起身保护结束帧
    DWORD                       m_dwGrabHeroID;// 释放抓投技的时候抓住的英雄
    BOOL                        m_bBaTi;        // 是否是霸体状态

    struct KHeroAIData 
    {
        int                 nAIType;

        KShootType          eShootType;

        KTARGET             eTargetType;
        DWORD               dwTargetObjID;
        int                 nTargetX;
        int                 nTargetY;
        int                 nTargetZ;

    }   m_AIData;

    KGAME_STATISTICS        m_GameStat;
    int                     m_bForbitAutoAction;
    WORD                    m_wFashionID;
    KPassiveSkill           m_PassiveSkillUsed[cdPassiveSkillSlotCount];

    DWORD                   m_ActiveSkill[KACTIVE_SLOT_TYPE_TOTAL];
    DWORD                   m_PassiveSkill[cdPassiveSkillSlotCount];
    DWORD                   m_NirvanaSkill[cdNirvanaSkillSlotCount];
    DWORD                   m_SlamBallSkill[cdSlamBallSkillSlotCount];
    KVEC_DWORD              m_vecAllOtherActiveSkill;

public:
    void    ApplyAttribute(KAttribute* pAttr);
    void    UnApplyAttribute(KAttribute* pAttr);

    BOOL    CallAttributeFunction(int nKey, BOOL bApply, int nValue1, int nValue2);
    BOOL    CanBeAttacked();

    KPlayer* GetOwner() const;
    int     GetConnIndex() const;
    virtual KTWO_DIRECTION GetAttackingDir() const;

    void ForbitAutoTakeBall(int nFrames);

    void    AutoTakeBall();

    BOOL    SetUpAVM();
    
    void    ApplySkillEffect(KSkill* pSkill);

    BOOL    UpdateAutoMoveParam(int nAutoMoveAroundX, int nAutoMoveAroundY, BOOL bWalk);
    BOOL    ShootDoodad(KDoodad* pDoodad, BOOL bAdjust);

protected:
    virtual void ProcessRebounds();
    virtual void AfterProcessMoveZ();
    virtual void OnTrackMove(int nCurLoop);
private:
    virtual void OnTrackMoveFinished();
    virtual void OnTrackMoveInterrupted();

    void RestoreStatus(const KPOSITION_RECORD& rRecord);
    BOOL ReverseFrame(int nFrame);
    BOOL ForwardFrame(int nFrame);
    
    BOOL CanTakeDoodad() const;
    BOOL CanDropDoodad() const;
    BOOL CanThrowDoodad() const;
    BOOL CanShootDoodad() const;

    KSceneObject* GetAttacker();

    KBasketSocket* GetShootSocket();

    BOOL TakeDoodad(KDoodad * pObject, BOOL bAdjust);
    BOOL UseDoodad(KDoodad* pDoodad, unsigned char direction, BOOL bAdjust);
    BOOL DropDoodad(KDoodad* pDoodad, BOOL bAdjust);
    BOOL PassBallTo(KHero* pReceiver, BOOL bAdjust);

    void RecordStatus();
    void UpdateVelocity(int nSpeed, int nDirection);

    void TryFinishCastingSkill();
    BOOL EnsureNotCastingSkill();
    void TriggerOnSkillFinished(KSkill* pSkill);

    void OverTimeSkillProcess();

    KSkill*     GetWillCastSkill(DWORD dwSkillID);
    KBullet*    FireBullet(DWORD dwBulletTemplateID);
    BOOL        FireBullets(const KVEC_BULLETS& rvecBullets);
    BOOL        PrepareToCastSkill();
    BOOL        ProcessSkillSegment();
    BOOL        ApplyBuffAndAttributes(DWORD dwBuffID, const KVEC_ATTRIBUTES& rvecAttributes);
    BOOL        ApplyBuffAndAttributes(DWORD dwBuffID, const KAttribute* pListHead);
    BOOL        ProcessSkillAttack();
    BOOL        DoRestraintJudge(KHero* pTarget, BOOL& bMeAttackTarget, BOOL& bTargetAttackMe);

    void        DoDelayAction();

    void        DelayApplySkillEffect();
    void        OnFreeze();
    int         m_nForbitAutoTakeBall; // 禁止捡球的帧数
    BOOL        m_bSkillSlaming;
    KSlamBallSkill* m_pCastingSlamBallSkill;
    BOOL        m_bSkillSlamingHit;
    BOOL        m_bForbitSkill;
    BOOL        m_bForbitPick;
    KPlayer*    m_pOwner;
    KTraceableArg<KPOSITION_RECORD, MAX_POSITION_RECORD> m_cRecordedStatus;
    KATTACKED_OBJ_LIST  m_AttackObjList;
    
private:
    int     m_nLastCalcStaminaPercent;
    int     m_nMoveStateStartFrame;
    
    KSkill* m_pPreSkill;
    int     m_nPreSkillCastTime;
    int     m_nPreSkillEndTime;
    DWORD   m_dwPerSkillTargetID;
    int     m_nTriggerDepth;
    int     m_nFreezeFrame;
    BOOL    m_bExhausted;


    typedef KMemory::KAllocator<std::pair<uint64_t, int> > KCAST_SKILL_COUNT_INAIR_PAIR;
    typedef std::map<uint64_t, int, std::less<uint64_t>, KCAST_SKILL_COUNT_INAIR_PAIR> KCAST_SKILL_COUNT_INAIR_LIST;
    KCAST_SKILL_COUNT_INAIR_LIST m_mapCastSkillCountInAir;

    BOOL ProcessCastSkillInAir();

    int     m_nFrameToApplySkillEffect;
    KSkill* m_pSkillDelayApply;

    enum KAUTOMOVE_STATUS
    {
        KAUTOMOVE_STATUS_NONE,
        KAUTOMOVE_STATUS_AUTOMOVING,
        KAUTOMOVE_STATUS_WAITING_PATH,
        KAUTOMOVE_STATUS_DIRECTMOVING,
    };

    enum
    {
        KAUTOMOVE_MAX_DELTAX = CELL_LENGTH * 2,
        KAUTOMOVE_MAX_DELTAY = CELL_LENGTH * 2,
    };

    int              m_nDestX;
    int              m_nDestY;
    
    int              m_nAutoMoveAroundX;
    int              m_nAutoMoveAroundY;
    BOOL             m_bWalkAutoMove;
    KAUTOMOVE_STATUS m_eAutoMoveStatus;
    KLIST_AUTOPATH   m_lstAutoMovePath;

    // 僵直后的击飞速度
    int              m_nKnockoffVelocityX;
    int              m_nKnockoffVelocityY;
    int              m_nKnockoffVelocityZ;
    int              m_nAttackCountOnStiff;

    void ProcessAutoMove();
    void FinishAutoMove();
    BOOL MoveToNextCell(KCellEx* pCurrent);
    BOOL CanCellAtNextFrameReachable();
    BOOL MoveTo(int nDestX, int nDestY, BOOL bWalk);
    BOOL IsAroundOfAutoMovingTarget();
    BOOL IsReachedNextCell();
    BOOL IsTargetMoveFarway();
    BOOL StartAutoMove();

// AI 相关

private:
    KMAP_AI_SELFDEF_DATA m_dictAISelfDefData;

public:
    void FireAIEvent(KAI_EVENT eAIEvent, DWORD dwSrcID, int nEventParam);
public:
    KAI_BRANCH AISetMoveTarget(int nParams[]);
    KAI_BRANCH AIMoveToTarget(BOOL bWalk);
    KAI_BRANCH AITakeBall();
    KAI_BRANCH AICheckNearTargetA(int nDelta);
    KAI_BRANCH AIAimAt(BOOL bSelf);
    KAI_BRANCH AICheckAimFloor(int nFloor);
    KAI_BRANCH AIShootBall();
    KAI_BRANCH AICheckFollower(int nObjType, int nMoveType);
    KAI_BRANCH AICheckBallTaker(int nType);
    KAI_BRANCH AICastSkill(int nSlotIndex);
    KAI_BRANCH AIJump(BOOL bMove, int nDirectionType, int nParam);
    KAI_BRANCH AICheckHeroState(int nParams[]);
    KAI_BRANCH AITakeDoodad(int nDoodadType);
    KAI_BRANCH AIUseDoodad();
    KAI_BRANCH AIShootDoodad();
    KAI_BRANCH AIDropDoodad();
    KAI_BRANCH AIPassBall();
    KAI_BRANCH AIChangeDirection(int nDirectionType, int nParam);
    KAI_BRANCH AICheckBallMoveType(int nParam[]);

// AI 用辅助函数
private:
    typedef BOOL (KHero::*FUNC_SETTARGET)(int nParams[]);

public:
    BOOL GetAITargetPos(int& nX, int& nY, int& nZ);
    KMovableObject* GetAITarget();
    KHero* GetFarerEnemy(BOOL bExcludeCannotBeAttacked);
    KHero* GetNearerEnemy(BOOL bExcludeCannotBeAttacked);
    KHero* GetTeammate();
    BOOL   IsMainHero();
    BOOL   IsAssistHero();
    BOOL   IsNpc();
    void   OnHpDecrease();
    BOOL   TriggerPassiveSkill(int nEvent, int nOldValue = 0, int nNewValue = 0);
    int    GetBallTakerRelation();
    DWORD  GetInterferenceID();

private:
    void InitAIFunctions();
    int  GetAIAimAtDirection(BOOL bSelf);
    BOOL IsValidAimFloor(int nFloor);

    BOOL SetTarget_Position(int nParams[]);
    BOOL SetTarget_Hero(int nParams[]);
    BOOL SetTarget_SceneObj(int nParams[]);
    BOOL SetTarget_Ball(int nParams[]);

    KHero* GetHeroByFilter(int nParams[]);
    KHero* GetBallTaker();
    KHero* GetRandomEnemy(BOOL bExcludeCannotBeAttacked);

    KSceneObject* GetSceneObjByFilter(int nParams[]);
    KSceneObject* GetFarerSceneObj(int nObjType);
    KSceneObject* GetNearerSceneObj(int nObjType);

    KHero* GetNearerTeammate();
    KBasketSocket* GetOneSelfBaksetSocket();
    KBasketSocket* GetOneEnemyBaksetSocket();

    KHero*  GetOneTeammate();
    void DropGold(int nOldStaminaPercent, int nNewStaminaPercent);

private:
    std::map<int, FUNC_SETTARGET> m_SetTargetFuncs;

public:
    int LuaJump(Lua_State* L);
    int LuaSetMoveTarget(Lua_State* L);
    int LuaMoveToTarget(Lua_State* L);
    int LuaSwitchAIMode(Lua_State* L);
    int LuaGetScene(Lua_State* L);
    int LuaSetAIDebug(Lua_State* L);
    int LuaSetAIMode(Lua_State* L);
    int LuaAddBuff(Lua_State* L);
    int LuaDelBuff(Lua_State* L);
    int LuaIsMainHero(Lua_State* L);
    int LuaIsAssistHero(Lua_State* L);
    int LuaIsNpc(Lua_State* L);
    int LuaGetOwner(Lua_State* L);
    int LuaIsTakingBall(Lua_State* L);
    int LuaTakeBall(Lua_State* L);
    int LuaUnTakeBall(Lua_State* L);
    int LuaPassBall(Lua_State* L);
    int LuaAimAt(Lua_State* L);
    int LuaShootBall(Lua_State* L);
    int LuaSlamBall(Lua_State* L);
    int LuaLoseStamina(Lua_State* L);
    int LuaReleaseRelationWithOther(Lua_State* L);
    int LuaStand(Lua_State* L);
    int LuaSit(Lua_State* L);
    int LuaFireBullet(Lua_State* L);
    int LuaAISay(Lua_State* L);
    int LuaAutoMove(Lua_State* L);
    int LuaCastSkill(Lua_State* L);
    int LuaChangeFaceDir(Lua_State* L);
    int LuaSetAISelfDefData(Lua_State* L);
    int LuaCheckAISelfDefData(Lua_State* L);
    int LuaForbitAutoTakeBall(Lua_State* L);
    int LuaLoseEndurance(Lua_State* L);
    int LuaFinishCastingSkill(Lua_State* L);
    int LuaSpecialUseDoodad(Lua_State* L);
    int LuaChangeToDefenseAI(Lua_State* L);
    int LuaChangeToAttackAI(Lua_State* L);

public:
    DECLARE_LUA_CLASS(KHero);
    DECLARE_LUA_STRING(Name, sizeof(m_szName));
    DECLARE_LUA_INTEGER(Level);
    DECLARE_LUA_INTEGER(Side);
    DECLARE_LUA_INTEGER(Pos);
    DECLARE_LUA_BOOL(AiMode);
    DECLARE_LUA_INTEGER(CurrentEndurance);
    DECLARE_LUA_INTEGER(MaxEndurance);
    DECLARE_LUA_INTEGER(CurrentStamina);
    DECLARE_LUA_INTEGER(MaxStamina);
    DECLARE_LUA_INTEGER(CurrentAngry);
    DECLARE_LUA_INTEGER(MaxAngry);
    DECLARE_LUA_INTEGER(HitRate);
};


