// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KLandMine.h 
//  Creator 	: Xiayong  
//  Date		: 02/09/2012
//  Comment	: 
// ***************************************************************
#pragma once
#include "KDoodad.h"
#include "game_define.h"

struct KSceneObjectTemplate;

class KLandMine : public KDoodad
{
public:
    KLandMine();
    virtual ~KLandMine();
    virtual KSceneObjectType GetType() const;

    virtual void Activate(int nCurLoop);

    int     m_nActiveFrame;  // ���׼����ʱ��
    int     m_nWaitActiveFrame; // ���ú����֡����
    bool    m_bDistinguishForce; // �Ƿ����ֵ���
    int     m_nBlowupVelocity;  // ��ը�ٶ�
    int     m_nBlowupRange;     // ��ը����

    void    ActiveState();

private:
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    virtual void OnTurnedToIdle();
    virtual void OnThrowOut(KHero* pHero);
};
