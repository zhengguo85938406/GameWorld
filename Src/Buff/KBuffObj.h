#pragma once

#include "game_define.h"
#include "KDoodad.h"

class KBuffObj : public KDoodad
{
public:
    KBuffObj(void);
    virtual ~KBuffObj(void); 

    virtual KSceneObjectType GetType() const;
    virtual BOOL HasGuide() const;
    virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    virtual int GetCustomData(int nIndex);
private:
    int m_nBuffID;
};

