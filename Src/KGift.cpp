#include "stdafx.h"
#include "KAllObjTemplate.h"
#include "KGift.h"


KGift::KGift(void)
{
    m_bCanToken = false;
}


KGift::~KGift(void)
{
}

KSceneObjectType KGift::GetType() const
{
    return sotGift;
}

BOOL KGift::HasGuide() const
{
    return false;
}
