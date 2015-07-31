#include "stdafx.h"
#include "KBulletInfoMgr.h"
#include "KGEnumConvertor.h"

KBulletInfoMgr::KBulletInfoMgr()
{
}

KBulletInfoMgr::~KBulletInfoMgr()
{
}

BOOL KBulletInfoMgr::Init()
{
	BOOL bResult    = false;
    BOOL bRetCode   = false;

    bRetCode = LoadData();
    KGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void KBulletInfoMgr::UnInit()
{
    m_mapBulletInfo.clear();
}

KBulletInfo* KBulletInfoMgr::GetBulletInfo(DWORD dwBulletTemplateID)
{
    KMAP_BULLETINFO::iterator it = m_mapBulletInfo.find(dwBulletTemplateID);
    KG_PROCESS_ERROR(it != m_mapBulletInfo.end());

    return &it->second;
Exit0:
    return NULL;
}

BOOL KBulletInfoMgr::LoadData()
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    ITabFile*   piTabFile   = NULL;
    const int   MAX_ATTRIBUTE_COUNT = 8;
    KBulletInfo* pBulletInfo    = NULL;         
    DWORD       dwBulletInfoID  = ERROR_ID;
    pair<KMAP_BULLETINFO::iterator, bool> InsRet;

    piTabFile = g_OpenTabFile(SETTING_DIR"/bullet.tab");
    KGLOG_PROCESS_ERROR(piTabFile);

    for (int nRowIndex = 2; nRowIndex <= piTabFile->GetHeight(); nRowIndex++)
    {
        bRetCode = piTabFile->GetInteger(nRowIndex, "ID", 0, (int*)(&dwBulletInfoID));
        KGLOG_PROCESS_ERROR(dwBulletInfoID > 0);

        InsRet = m_mapBulletInfo.insert(make_pair(dwBulletInfoID, KBulletInfo()));
        KGLOG_PROCESS_ERROR(InsRet.second);
        pBulletInfo = &(InsRet.first->second);

        pBulletInfo->dwTemplateID = dwBulletInfoID;
        
        bRetCode = piTabFile->GetInteger(nRowIndex, "VelocityX", 0, (int*)&pBulletInfo->nVelocityX);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "VelocityY", 0, (int*)&pBulletInfo->nVelocityY);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "VelocityZ", 0, (int*)&pBulletInfo->nVelocityZ);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "OffsetX", 0, (int*)&pBulletInfo->nOffsetX);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "OffsetY", 0, (int*)&pBulletInfo->nOffsetY);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "OffsetZ", 0, (int*)&pBulletInfo->nOffsetZ);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "LengthX", 0, (int*)&pBulletInfo->nLengthX);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "LengthY", 0, (int*)&pBulletInfo->nLengthY);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "LengthZ", 0, (int*)&pBulletInfo->nLengthZ);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "LifeTime", 0, (int*)&pBulletInfo->nLifeTime);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        pBulletInfo->nLifeTime = pBulletInfo->nLifeTime * GAME_FPS / cdTimeBase;

        bRetCode = piTabFile->GetInteger(nRowIndex, "Penetrate", 0, (int*)&pBulletInfo->bPenetrate);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "Gravity", 0, (int*)&pBulletInfo->nGravity);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

		bRetCode = piTabFile->GetInteger(nRowIndex, "BeAttackType", 0, &pBulletInfo->nBeAttackType);
		KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AttackOnFly", 0, (int*)&pBulletInfo->bAttackOnFly);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "CollideAffectID", ERROR_ID,  (int*)&pBulletInfo->dwCollideAffectID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BlastAffectID", ERROR_ID, (int*)&pBulletInfo->dwBlastAffectID);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BlastOnCollide", 0, (int*)&pBulletInfo->bBlastOnCollide);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BlastOnDisappear", 0, (int*)&pBulletInfo->bBlastOnDisappear);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BlastRadiusX", 0, &pBulletInfo->nBlastRadiusX);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BlastRadiusY", 0, &pBulletInfo->nBlastRadiusY);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "BlastRadiusZ", 0, &pBulletInfo->nBlastRadiusZ);
        KGLOG_PROCESS_ERROR(bRetCode > 0);

        bRetCode = piTabFile->GetInteger(nRowIndex, "AttackIntervalFrame", 0, &pBulletInfo->nAttackIntervalFrame);
        KGLOG_PROCESS_ERROR(bRetCode > 0);
        KGLOG_PROCESS_ERROR(pBulletInfo->nAttackIntervalFrame >= 0);
    }

    bResult = true;
Exit0:
    KG_COM_RELEASE(piTabFile);
    return bResult;
}
