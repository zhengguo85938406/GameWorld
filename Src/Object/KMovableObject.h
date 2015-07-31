// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KMovableObject.h
//  Creator 	: Hanruofei  
//  Date		: 12/14/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "KSceneObject.h"
#include "IPath.h"
#include "game_define.h"

enum KMoveActionType
{
    mtNormalShoot,
    mtSkillShoot,
    mtNormalSlam,
    mtSkillSlam,
    mtPassball,
};

enum KTrackMoveState
{
    tmsNone,
    tmsMoving,
    tmsFinished,
};

typedef KMemory::KAllocator<std::pair<DWORD, int> > KATTACKED_OBJ_PAIR;
typedef std::map<DWORD, int, std::less<DWORD>, KATTACKED_OBJ_PAIR> KATTACKED_OBJ_LIST;

class KMovableObject : public KSceneObject
{
public:
    KMovableObject();
    virtual ~KMovableObject(void);

    virtual BOOL Init();
    virtual void UnInit();

    virtual void Activate(int nCurLoop);

    void    AddFollower(KMovableObject* pFollower);
    void    RemoveFollower(KMovableObject* pFollower);
    void    ClearFollower();

    void    TurnToMoveType(KMoveType eMoveType);

    int     CalcProcessTrackMoveZmax(int nRandomSeed, const KPOSITION& StartPos, const KPOSITION& FinalPos) const;

    virtual DWORD   GetGuideID();

    virtual BOOL    IsOnGround() const;
	virtual BOOL    IsInAir() const;

    BOOL    IsAncestorOf(const KMovableObject* pOther) const;
    BOOL    HasFollowRelationWith(const KMovableObject* pOther) const;

    BOOL    IsTokenBy(KMovableObject* pHost) const;
    BOOL    IsAttachedTo(KMovableObject* pHost) const;
    BOOL    IsStandOnObj() const;

    BOOL    IsFollowing(const KMovableObject* pObject) const;
    BOOL    IsFollowing(DWORD dwID) const;
    BOOL    IsFollowingSomeone() const;
    BOOL    IsFollowedBy(KMovableObject* pObject) const;
    BOOL    HasFollower() const;
    BOOL    HasFollowerWithState(KMoveType eState) const;
    BOOL    HasFollowerWithType(KSceneObjectType eType) const;
    BOOL    HasFollowerWithStateAndType(KMoveType eState, KSceneObjectType eType) const;

    KMovableObject* GetFirstFollowerWithType(KSceneObjectType eType) const;
    KMovableObject* GetFirstFollowerWithState(KMoveType eState) const;

    virtual KMoveType GetMoveType();

    virtual KTWO_DIRECTION GetAttackingDir() const;

    void    ClearVelocity();
    void    ProcessCollision();

    std::string ToStatusString() const;

    void    DoTrackMove(const IPath* pPath);

    BOOL FixTo(const KPOSITION& rPos);
    BOOL UnFix();

    BOOL GetStartPos(KPOSITION& rPos);
    BOOL GetFinnalPos(KPOSITION& rPos);

    DWORD GetStandOnObjID();

    KMovableObject* GetGuide();

    BOOL IsStandOn(KSceneObjectType eType);
    void ProcessAfterCollision();
    BOOL HasGuide() const;

    void OnDeleteFlagSetted();

    virtual int GetVelocityX(){return m_nVelocityX;};
    virtual int GetVelocityY(){return m_nVelocityY;};
    virtual int GetVelocityZ(){return m_nVelocityZ;};

    virtual int GetCurrentGravity();

public:

    int         m_nVelocityX;
    int         m_nVelocityY;
    int         m_nVelocityZ;
    
    double      m_fXYReboundCoeX;             // 撞XY平面(地面)时X方向反弹系数
    double      m_fXYReboundCoeY;             // 撞XY平面(地面)时Y方向反弹系数
    double      m_fXYReboundCoeZ;             // 撞XY平面(地面)时Z方向反弹系数

    double      m_fYZReboundCoeX;             // 撞YZ平面时X方向反弹系数
    double      m_fYZReboundCoeY;             // 撞YZ平面时Y方向反弹系数
    double      m_fYZReboundCoeZ;             // 撞YZ平面时Z方向反弹系数

    double      m_fXZReboundCoeX;              // 撞XZ平面时X方向反弹系数
    double      m_fXZReboundCoeY;              // 撞XZ平面时Y方向反弹系数
    double      m_fXZReboundCoeZ;              // 撞XZ平面时Z方向反弹系数

    BOOL        m_bSomeThingOnBasket;           // 是否有物件在篮筐上，有的话不能判定进球

    DWORD       m_dwObstacleID;                 // 所站立的障碍物件id
    KPOSITION   m_moveOffset;

protected:
    void    ProcessMove();

    BOOL    CheckGameLoop(int nCurLoop);

    virtual void OnPosChanged(const KPOSITION& cOld, const KPOSITION& cNew);
    virtual void OnGuidePosChanged(const KPOSITION& cOld, const KPOSITION& cNew);

    virtual void ProcessRebounds();
  
    virtual void AfterProcessMoveZ();

    virtual void OnTrackMoveFinished();
    virtual void OnTrackMoveInterrupted();

    void    CheckCollisionObj();

    void    ClearCollision();

    void    RemoveGuide();

    int     m_nGameLoop;

    BOOL    IsSceneObjInCollidedList(DWORD dwObjectID);
    BOOL    InsertSceneObjInCollidedList(DWORD dwObjectID);

protected:
    virtual void OnTrackMove(int nCurLoop);
private:
    void    OnFree(int nCurLoop);
    void    OnIdle(int nCurLoop);

    void    ClearPath();

    int     GetCollisionResult(int nSrcX, int nSrcY, int nDestX, int nDestY);

    void    CollsionChecked(KSceneObject* pObj);
    void    ClearCollsionTempFlag();


    void    MoveXY(int nDeltaX, int nDeltaY);
    void    MoveZ(int nDeltaZ);
    BOOL    CellMove(int nDeltaX, int nDeltaY);

    virtual void OnTurnedToIdle() {};

    virtual void ProcessXYRebound();
    virtual void ProcessXZRebound();
    virtual void ProcessYZRebound();

    std::vector<DWORD>  m_vecFollower;
    DWORD               m_dwGuide;
    IPath*              m_pPath;

    KMoveType           m_eMoveType;

    typedef std::map<DWORD, bool> KMAP_COLLISIONOBJINFO;
    KMAP_COLLISIONOBJINFO m_mapCollisionObjInfo;

    KTrackMoveState     m_eTrackMoveState;
protected:
    BOOL                m_bXZRebound;                // X方向发生反弹
    BOOL                m_bYZRebound;                // Y方向发生反弹
    BOOL                m_bXYRebound;                // Z方向发生反弹


public:
    int LuaClearVelocity(Lua_State* L);
};
