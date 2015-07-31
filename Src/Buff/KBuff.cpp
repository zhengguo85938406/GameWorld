#include "stdafx.h"
#include "KBuff.h"
#include "KAttribute.h"

KBuff::KBuff()
{
    m_dwID              = ERROR_ID;
    m_pRollBackAttr     = NULL;
    m_pActiveAttr       = NULL;
    m_nFrame            = 0;
    m_bTimeLapseOffline = false;
    m_bDelOnFloor       = false;
    m_bDelOnObjNoAttached = false;
    m_bNeedSave         = false;
}

KBuff::~KBuff()
{
    assert(m_pRollBackAttr == NULL);
    assert(m_pActiveAttr == NULL);
}

BOOL KBuff::Init(DWORD dwID)
{
    m_dwID = dwID;
    return true;
}

void KBuff::UnInit()
{
    if (m_pRollBackAttr)
    {
        FREE_ATTRIB_LIST(m_pRollBackAttr);
        m_pRollBackAttr = NULL;
    }

    if (m_pActiveAttr)
    {
        FREE_ATTRIB_LIST(m_pActiveAttr);
        m_pActiveAttr = NULL;
    }
}
