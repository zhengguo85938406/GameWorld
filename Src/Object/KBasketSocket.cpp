#include "stdafx.h"
#include "KBasketSocket.h"
#include "KBasket.h"
#include "KDoodad.h"
#include "KAllObjTemplate.h"

KBasketSocket::KBasketSocket()
{
    m_pBasket           = NULL;
    m_eFloor            = bfInvalid;
    m_nSide             = sidBegin;
}

KBasketSocket::~KBasketSocket()
{
    if (m_pBasket)
    {
        m_pBasket->m_pPluginSocket = NULL;
        m_pBasket = NULL;
    }
}

BOOL KBasketSocket::IsEmpty()
{
    return (m_pBasket == NULL);
}

BOOL KBasketSocket::AddBasket(KBasket* pBasket)
{
    BOOL bResult = false;
    BOOL bRetCode = false;

    KGLOG_PROCESS_ERROR(pBasket);
    KGLOG_PROCESS_ERROR(!pBasket->m_pPluginSocket);
    KGLOG_PROCESS_ERROR(!m_pBasket);

    bRetCode = pBasket->FixTo(GetPosition());
    KGLOG_PROCESS_ERROR(bRetCode);

    pBasket->m_pPluginSocket = this;
    m_pBasket = pBasket;
    

    bResult = true;
Exit0:
    return bResult;
}

BOOL KBasketSocket::DelBasket()
{
    assert(m_pBasket);

    if (m_eDir == csdLeft)
        m_pBasket->m_nVelocityX = -m_pBasket->m_nDropBasketVelocityX;
    else
        m_pBasket->m_nVelocityX = m_pBasket->m_nDropBasketVelocityX;

    m_pBasket->m_nVelocityZ = m_pBasket->m_nDropBasketVelocityZ;
       
    m_pBasket->m_pPluginSocket = NULL;
    m_pBasket->UnFix(); 
    m_pBasket = NULL;
    return true;
}

KBasket* KBasketSocket::GetBasket()
{
    return m_pBasket;
}

int KBasketSocket::GetCustomData(int nIndex)
{
    int nData = 0;
    switch(nIndex)
    {
    case 0:
        nData = m_eFloor;
        break;
    case 1:
        nData = IsAtLeftHalf();
    default:
        break;
    }
    return nData;
}

KSceneObjectType KBasketSocket::GetType() const
{
    return sotBasketSocket;
}

BOOL KBasketSocket::IsAtLeftHalf() const
{
    return m_eDir == csdRight;
}

void KBasketSocket::ApplySpecialInitInfo(const KSceneObjInitInfo& cTemplate)
{
    m_eDir = (KTWO_DIRECTION)cTemplate.m_nDir;
    m_eFloor = (KBasketSocketFloor)cTemplate.m_nFloor;

    if (m_eDir == csdLeft)
        m_nSide = sidLeft;
    else if(m_eDir == csdRight)
        m_nSide = sidRight;
    else
        assert(false);
}
