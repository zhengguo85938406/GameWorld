// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KClip.h
//  Creator 	: hanruofei 
//  Date		: 2012-2-13
//  Comment	: 
// ***************************************************************

#pragma once
#include "KDoodad.h"
class KClip : public KDoodad
{
public:
    KClip(void);
    virtual ~KClip(void);

    void Stampede(KHero* pOwner);
    void Activate(int nCurLoop);
    
    KSceneObjectType GetType() const;

    virtual void OnDeleteFlagSetted();

private:
    virtual void OnTurnedToIdle();
    virtual void OnThrowOut(KHero* pHero);
    void ProcessStampede(int nCurLoop);
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    void FinishStampe();

    DWORD m_dwStampeID;
    int m_nFrameStart;
    int m_nCurTotalFrameCount;
    int m_nTotalFrameCount; // 被夹住的时间
};

