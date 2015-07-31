#pragma once

#include "game_define.h"
#include "KDoodad.h"

class KGift : public KDoodad
{
public:
    KGift(void);
    virtual ~KGift(void); 

    virtual KSceneObjectType GetType() const;
    virtual BOOL HasGuide() const;
};

