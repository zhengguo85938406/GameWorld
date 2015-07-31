// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KDummy.h 
//  Creator 	: Xiayong  
//  Date		: 08/29/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KMovableObstacle.h"
class KHero;
class KBall;
struct KSceneObjectTemplate;

class KDummy : public KMovableObstacle
{
public:
    KDummy();
    ~KDummy();

    virtual void Activate(int nCurLoop); 
    virtual int GetCustomData(int nIndex);

    KSceneObjectType GetType() const;
    void BeAttacked();

    virtual BOOL GetAttackRange(int& nLeftBottomX, int& nLeftBottomY, int& nRightTopX, int& nRightTopY);

protected:
	BOOL ProcessDummyAttack();
    void AttackBall(KBall* pBall);

private:
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);

private:
    int     m_nAttackFrames;        // ¹¥»÷³ÖÐøÊ±¼ä
    int		m_nAttackRange;         // ¹¥»÷·¶Î§
    BOOL    m_bOnlyAttackPlayer;    // ÊÇ·ñÖ»¹¥»÷Íæ¼Ò

    int     m_nLeftAttackFrames;    // Ê£ÓàµÄ¹¥»÷Ö¡Êý
};
