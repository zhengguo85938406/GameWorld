#include "stdafx.h"
#include "KTerrainData.h"
#include "Engine/KMemory.h"

KTerrainScene::KTerrainScene()
{
    m_nRefCount     = 1;
    m_bLoadFlag     = false;
    memset(m_Cells, 0, sizeof(m_Cells));
}

int KTerrainScene::AddRef()
{
    return ++m_nRefCount;
}

int KTerrainScene::Release()
{
    assert(m_nRefCount > 0);

    m_nRefCount--;

    if (m_nRefCount == 0)
    {
        Clean();

        KMemory::Delete(this);
        return 0;
    }

    return m_nRefCount;
}

KCell* KTerrainScene::GetTerrainData()
{
    return &m_Cells[0];
}

void KTerrainScene::Clean()
{
    memset(m_Cells, 0, sizeof(m_Cells));
}
