#pragma once

#include "game_define.h"
#include "KMovableObstacle.h"

class KBuffBox : public KMovableObstacle
{
public:
    KBuffBox(void);
    virtual ~KBuffBox(void); 
     
    virtual KSceneObjectType GetType() const;
    virtual int GetCustomData(int nIndex);
    void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
 
    void BeAttacked();

private:
    int     m_nCanBeAttackTime;
};

