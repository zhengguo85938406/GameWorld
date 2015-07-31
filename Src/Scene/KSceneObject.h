// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KSceneObject.h 
//	Creator 	: Xiayong  
//  Date		: 08/07/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "KObject.h"
#include "KCell.h"
#include "KMath.h"
#include "KSceneGlobal.h"
#include "GlobalEnum.h"
#include "Engine/KMemory.h"
#include "Luna.h"

class KScene;
struct KPOSITION;
struct KSceneObjInitInfo;
struct KSceneObjectTemplate;

class KSceneObject : public KObject
{
public:
	KSceneObject();
	virtual ~KSceneObject();

    virtual KSceneObjectType GetType() const = 0;

    virtual BOOL Init();
    virtual void UnInit();

    void ApplyTemplateInfo(KSceneObjectTemplate* pTemplate);
    void ApplyInitInfo(const KSceneObjInitInfo& cInitInfo);
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate){};
    virtual int GetCustomData(int nIndex){return 0;};
    virtual int GetVelocityX(){return 0;};
    virtual int GetVelocityY(){return 0;};
    virtual int GetVelocityZ(){return 0;};
    virtual int GetCurrentGravity(){return 0;};
    virtual KMoveType GetMoveType(){return mosInvalid;};
    virtual DWORD GetGuideID(){return ERROR_ID;};

    virtual void Activate(int nCurLoop) {};
    virtual void SpecialActivate(int nCurLoop){};
    virtual void ProcessCollision(){};
    virtual void ProcessAfterCollision(){};
	
	virtual BOOL    IsOnGround() const;
	virtual BOOL    IsInAir() const;

    KBODY   GetBody() const;
    KCell*  GetCell() const;
    KPOSITION GetPosition() const;
    KPOSITION GetTopCenter() const;
    KPOSITION GetBodyCenter() const;

    void    SetPosition(const KPOSITION& cPos);
    void    SetPosition(int nX, int nY, int nZ);

    void    SetX(int nX);
    void    SetY(int nY);
    void    SetZ(int nZ);

    int     GetLocationZ();

    void    SetDeleteFlag();
    BOOL    IsToBeDelete(){return m_bToBeDelete;};

    virtual BOOL HasGuide() const = 0;

    BOOL Is(KSceneObjectType eType) const;

    virtual BOOL GetAttackRange(int& nLeftBottomX, int& nLeftBottomY, int& nRightTopX, int& nRightTopY);

public:
    KScene*		m_pScene;
    DWORD       m_dwScriptID;

    KSceneObjectTemplate* m_pTemplate;

    BOOL        m_bCanBeAttack;
    BOOL        m_bFullObstacle;

protected:
    virtual void OnPosChanged(const KPOSITION& cOld, const KPOSITION& cNew){};
    virtual void ApplySpecialInitInfo(const KSceneObjInitInfo& cInitInfo) {};
    virtual void OnDeleteFlagSetted();;

public:
	int			m_nX;
	int			m_nY;
	int			m_nZ;
    int			m_nLength;
    int			m_nWidth;
    int			m_nHeight;
    BOOL        m_bToBeDelete;
    BOOL        m_bAttackState;

public:
    DECLARE_LUA_CLASS(KSceneObject);
    DECLARE_LUA_INTEGER(X);
    DECLARE_LUA_INTEGER(Y);
    DECLARE_LUA_INTEGER(Z);
    DECLARE_LUA_INTEGER(Length);
    DECLARE_LUA_INTEGER(Width);
    DECLARE_LUA_INTEGER(Height);

    int LuaGetPosition(Lua_State* L);
    int LuaSetPosition(Lua_State* L);
};

BOOL g_IsDoodad(const KSceneObject* pObj);
BOOL g_IsDoodad(KSceneObjectType eType);

BOOL g_IsObstacle(const KSceneObject* pObj);
BOOL g_IsObstacle(KSceneObjectType eType);

int g_GetDistance(KSceneObject* pSrc, KSceneObject* pDst);

