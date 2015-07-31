#pragma once

#include "game_define.h"
#include "KSceneObject.h"

class KTrap : public KSceneObject
{
public:
    KTrap(void);
    virtual ~KTrap(void);

    virtual KSceneObjectType GetType() const;

    virtual BOOL HasGuide() const;
	virtual int GetCustomData(int nIndex);
private:
	virtual void ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate);

private:
	int m_nRepresentID;
};

