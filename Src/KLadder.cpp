#include "stdafx.h"
#include "KSO3World.h"
#include "KLadder.h"
#include "KBasketSocket.h"
#include "KBasket.h"
#include "KPlayerServer.h"
#include "KHero.h"
#include <algorithm>

KLadder::KLadder(void)
{
    m_bCanStood = true;
}

KLadder::~KLadder(void)
{
}

KSceneObjectType KLadder::GetType() const
{
    return sotLadder;
}

void KLadder::ApplySpecialInitInfo(const KSceneObjInitInfo& cTemplate)
{

}

BOOL KLadder::BeSpecialUsedBy(KHero* pHero)
{
	BOOL bResult  = false;
	BOOL bRetCode = false;

	KGLOG_PROCESS_ERROR(pHero);

    bRetCode = pHero->ShootDoodad(this, true);
    KGLOG_PROCESS_ERROR(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}
