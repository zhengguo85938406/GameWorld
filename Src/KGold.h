#pragma once

#include "game_define.h"
#include "KDoodad.h"

class KGold : public KDoodad
{
public:
    KGold(void);
    virtual ~KGold(void); 

    virtual KSceneObjectType GetType() const;
    virtual BOOL HasGuide() const;
    virtual int GetCustomData(int nIndex);
    void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);
    
    void SetMoney(int nMoney){m_nMoney = nMoney;}
    int GetMoney(){return m_nMoney;}
private:
    int m_nMoney;
};

