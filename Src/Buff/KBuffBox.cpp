#include "stdafx.h"
#include "KBuffBox.h"
#include "KAllObjTemplate.h"
#include "KPlayerServer.h"
#include "KScene.h"
#include "KBuffObj.h"

KBuffBox::KBuffBox(void)
{
    m_bCanBeAttack = true;
}


KBuffBox::~KBuffBox(void)
{
}

KSceneObjectType KBuffBox::GetType() const
{
    return sotBuffBox;
}

int KBuffBox::GetCustomData(int nIndex)
{
    return m_nCanBeAttackTime;
}

void KBuffBox::ApplyTemplateCustomParam(KSceneObjectTemplate* pTemplate)
{
    assert(pTemplate);
    m_nCanBeAttackTime = pTemplate->m_nCustomParam[0];
}


void KBuffBox::BeAttacked()
{ 
    DWORD dwTemplateID = 0;
    KSceneObjectTemplate* pTemplate = NULL;
    KSceneObject* pObj = NULL;

    dwTemplateID = g_pSO3World->m_Settings.m_ConstList.nSceneBuffObjID;
    if (--m_nCanBeAttackTime <= 0)
    {
        m_pScene->RemoveSceneObject(this);

        pTemplate = g_pSO3World->m_Settings.m_ObjTemplate.GetTemplate(dwTemplateID);
        KGLOG_PROCESS_ERROR(pTemplate);
        KGLOG_PROCESS_ERROR(pTemplate->m_nType == sotBuffObj);
        pTemplate = NULL;

        //¶ªÒ»¸öbuff
        pObj = m_pScene->CreateSceneObject(dwTemplateID, m_nX, m_nY, m_nZ);
        KG_PROCESS_ERROR(pObj);
        KG_PROCESS_ERROR(pObj->GetType() == sotBuffObj);
    }

Exit0:
    return;
}
