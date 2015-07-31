#pragma once

struct KSceneObjectTemplate
{
    DWORD m_dwID;
    int m_nLength;
    int m_nWidth;
    int m_nHeight;
    int m_nType;
    int m_nVXOnThrow;
    int m_nVYOnThrow;
    int m_nVZOnThrow;
    DWORD m_dwScriptID;
    DWORD m_dwAttackAffectID1;
    DWORD m_dwAttackAffectID2;
    int m_nCustomParam[OBJ_TEMPLATE_CUSTOM_PARAM_COUNT];
    BOOL m_bImpregnability;
    int m_nBeAttackedType;
    BOOL m_Obstacle;

    BOOL m_bFireAIEvent;
    int  m_nAIEventMinX;
    int  m_nAIEventMaxX;
    int  m_nAIEventMinY;
    int  m_nAIEventMaxY;
    int  m_nAIEventMinZ;
    int  m_nAIEventMaxZ;
};

struct KSceneObjInitInfo
{
    int m_nX;
    int m_nY;
    int m_nZ;
    int m_nDir;
    int m_nFloor;
};

class KAllObjTemplate
{
public:
    KAllObjTemplate();
    ~KAllObjTemplate(void);
    
    KSceneObjectTemplate* GetTemplate(DWORD dwID);

    BOOL LoadAll();
private:
    typedef std::map<DWORD, KSceneObjectTemplate> KMAP_SCENEOBJ_TEMPLATE;
    KMAP_SCENEOBJ_TEMPLATE m_mapData;
};

