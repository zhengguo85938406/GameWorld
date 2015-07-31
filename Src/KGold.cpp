#include "stdafx.h"
#include "KGold.h"
#include "KAllObjTemplate.h"
#include "KSO3World.h"

KGold::KGold(void)
{
    m_fXYReboundCoeX    = 0.65;
    m_fXYReboundCoeY    = 0.65;
    m_fXYReboundCoeZ    = 0.65;

    m_fYZReboundCoeX    = 0.5;	             
    m_fYZReboundCoeY    = 0.5;            
    m_fYZReboundCoeZ    = 0.5;

    m_nMoney            = 1;
    m_bCanToken         = false;

}

KGold::~KGold(void)
{
}



KSceneObjectType KGold::GetType() const
{
    return sotGold;
}

BOOL KGold::HasGuide() const
{
    return false;
}

int KGold::GetCustomData( int nIndex )
{
   return m_nMoney;
}

void KGold::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    assert(pTemplate);
    
    m_nVelocityX = pTemplate->m_nVXOnThrow - g_Random(pTemplate->m_nVXOnThrow/2);
    m_nVelocityY = pTemplate->m_nVYOnThrow - g_Random(pTemplate->m_nVYOnThrow/2);
    m_nVelocityZ = pTemplate->m_nVZOnThrow - g_Random(pTemplate->m_nVZOnThrow/2);
    if (g_Random(2) > 0)
        m_nVelocityX = -m_nVelocityX;
    if (g_Random(2) > 0)
        m_nVelocityY = -m_nVelocityY;
}
