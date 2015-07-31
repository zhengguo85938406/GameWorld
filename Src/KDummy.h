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
    int     m_nAttackFrames;        // ��������ʱ��
    int		m_nAttackRange;         // ������Χ
    BOOL    m_bOnlyAttackPlayer;    // �Ƿ�ֻ�������

    int     m_nLeftAttackFrames;    // ʣ��Ĺ���֡��
};
