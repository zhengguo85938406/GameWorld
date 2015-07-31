#include "stdafx.h"
#include "KAllObjTemplate.h"
#include "KTrap.h"

KTrap::KTrap(void)
{
	m_nRepresentID = 0;
}

KTrap::~KTrap(void)
{
}

KSceneObjectType KTrap::GetType() const
{
    return sotTrap;
}

BOOL KTrap::HasGuide() const
{
    return false;
}

void KTrap::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
	assert(pTemplate);

	m_nRepresentID = (int)pTemplate->m_nCustomParam[0];
}

int KTrap::GetCustomData( int nIndex )
{
	if (nIndex == 0)
		return m_nRepresentID;

	return 0;
}

