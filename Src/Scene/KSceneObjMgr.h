// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KSceneObjMgr.h
//  Creator 	: Hanruofei  
//  Date		: 12/19/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "KHero.h"
#include "KAllObjTemplate.h"
#include "Engine/KMemory.h"
class KSceneObject;
class KBall;
class KChest;
class KLadder;
class KBasketSocket;
class KBackboard;
class KBasket;
class KHero;
class KDoodad;
class KScene;

class KObjEnumerator
{
protected:
    typedef std::map<DWORD, KSceneObject*>::const_iterator MAP_CONST_IT;
public:
    KObjEnumerator(MAP_CONST_IT curIt, MAP_CONST_IT endIt)
        : m_curIt(curIt), m_endIt(endIt)
    {
    }

    KSceneObject* GetValue()
    {
        assert(HasElement());
        return m_curIt->second;
    }
    bool HasElement()
    {
        return m_curIt != m_endIt;
    }
    void MoveToNext()
    {
        ++ m_curIt;
    }

    KObjEnumerator(const KObjEnumerator& rOther)
    {
        m_curIt = rOther.m_curIt;
        m_endIt = rOther.m_endIt;
    }

private:
    MAP_CONST_IT m_curIt;
    MAP_CONST_IT m_endIt;
};

class KSceneObjMgr
{
public:
    KSceneObjMgr(void);
    ~KSceneObjMgr(void);

    BOOL Init(KScene* pScene);
    void UnInit();

    KSceneObject*   AddByType(KSceneObjectType eType);
    KSceneObject*   AddByTemplate(DWORD dwTemplateID);
    BOOL            RemoveByTemplate(DWORD dwTemplateID);

    KSceneObject*   Get(DWORD dwID) const;
    void            Delete(DWORD dwID);

    void Activate(int nCurLoop);
    void SpecialActivate(int nCurLoop);
    void ActivateHeroVirtualFrame();

    void ProcessCollision();

    KObjEnumerator GetEnumerator() const;
    void ClearAll();

    void DelayDelObj(DWORD dwID);

private:
    KSceneObject* CreateObj(KSceneObjectType eType) const;

    DWORD   m_dwCurID;
    KScene* m_pScene;

    std::map<DWORD, KSceneObject*>  m_mapAllSceneObject;
    std::vector<DWORD> m_vecNeedDelObj;
};


