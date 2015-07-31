// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBasketSocket.h 
//	Creator 	: Xiayong  
//  Date		: 09/13/2011
//  Comment		: 
// ***************************************************************
#pragma once
#include "KStaticObject.h"
#include "GlobalEnum.h"

class KBasket;
class KBasketSocket : public KStaticObject
{
public:
    KBasketSocket();
    ~KBasketSocket();

    virtual KSceneObjectType GetType() const;

    BOOL IsEmpty();

    BOOL AddBasket(KBasket*  pBasket);
    KBasket* GetBasket();
    BOOL DelBasket();
    BOOL IsAtLeftHalf() const;

    int GetCustomData(int nIndex);

    int                 m_nSide;
    KBasketSocketFloor  m_eFloor;
    KBasket*            m_pBasket;
    KTWO_DIRECTION      m_eDir;

private:
    void ApplySpecialInitInfo(const KSceneObjInitInfo& cTemplate);
};
