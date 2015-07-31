#pragma once

#include "KDoodad.h"
class KHero;

class KLadder : public KDoodad
{
public:
    KLadder(void);
    virtual ~KLadder(void);

    virtual KSceneObjectType GetType() const;

private:
    virtual BOOL BeSpecialUsedBy(KHero* pHero);

    virtual void ApplySpecialInitInfo(const KSceneObjInitInfo& cTemplate);
};

