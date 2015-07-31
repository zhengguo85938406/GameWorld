#include "stdafx.h"
#include "KStaticObject.h"


KStaticObject::KStaticObject(void)
{
}

KStaticObject::~KStaticObject(void)
{
}


std::string KStaticObject::ToStatusString()
{
    char szTemp[128] = "\0";
    sprintf(szTemp, "P:%d,%d,%d", m_nX, m_nY, m_nZ);
    return std::string(szTemp);
}

