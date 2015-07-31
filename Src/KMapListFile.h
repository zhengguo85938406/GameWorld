#ifndef _KMAP_LIST_FILE_H_
#define _KMAP_LIST_FILE_H_

struct KMapParams
{
    DWORD   dwMapID;
	char	szMapName[_NAME_LEN];    
    int     nType;
    char	szResourceFilePath[MAX_PATH];
    DWORD   dwBattleID;
    int     nServerResourceID;
    int     nScriptDir;
    DWORD   dwBuffAwardID;
    DWORD   dwItemAwardID;
};

typedef std::map<DWORD, KMapParams> KMAP_PARAM_TABLE;

class KMapListFile
{
public:
	BOOL Init();
    void UnInit(){};

	KMapParams*	GetMapParamByID(DWORD dwMapID);
    KMapParams*	GetMapParamByName(const char cszName[]);

    const KMAP_PARAM_TABLE& GetMapList() { return m_MapParamTable; }

private:
	KMAP_PARAM_TABLE m_MapParamTable;
};

#endif	//_KMAP_LIST_FILE_H_

