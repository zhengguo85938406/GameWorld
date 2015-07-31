#include "stdafx.h"
#include "KSO3World.h"
#include "KChest.h"


KChest::KChest(void)
{
    m_bCanAttach = true;
}

KChest::~KChest(void)
{
}


KSceneObjectType KChest::GetType() const
{
    return sotChest;
}

void KChest::ApplySpecialInitInfo(const KSceneObjInitInfo& cTemplate)
{

}

void KChest::ProcessAfterCollision()
{
    KMoveType eMoveType = mosInvalid;
    KDoodad::ProcessAfterCollision();
    eMoveType = GetMoveType();
    if (eMoveType != mosFree && eMoveType != mosIdle && eMoveType != mosTrackMove)
    {
        ClearCollision();
    }
}
