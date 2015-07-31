#ifndef _KSCRIPTMANAGER_H_ 
#define _KSCRIPTMANAGER_H_ 


#include <map>
#include <string>
#include "Engine/KMemory.h"
#include "Engine/KLuaScriptEx.h"
#include "Luna.h"

#define SERVER_SCRIPTS_DIR "scripts"

class KPlayer;

class KScriptManager
{
public:
    KScriptManager();

    BOOL Init();
    void UnInit();

    void Activate();

    BOOL ExecutePlayerScript(KPlayer* pPlayer, const char* pszScript);
    BOOL ExecuteGMCScript(const char cszGmName[], const char cszScript[]);

	void SafeCallBegin(int* pIndex);
	void SafeCallEnd(int nIndex);

    BOOL GetValuesFromStack(const char * pszFormat , ...);

	template<class T>
	void Push(T value)
    {
	    m_piScript->PushValueToStack(value);
    }

	BOOL Call(DWORD dwScriptID, const char* pszFuncName, int nResults);
	BOOL Call(const char* pszScriptName, const char* pszFuncName, int nResults);

	BOOL IsFuncExist(DWORD dwScriptID, const char* pszFuncName);
	BOOL IsFuncExist(const char* pszScriptName, const char* pszFuncName);

	BOOL IsScriptExist(DWORD dwScriptID);
	BOOL IsScriptExist(const char* pszScriptName);

    ILuaScriptEx* GetScriptHolder()
    {
        return m_piScript;
    }

    void StatBegin();
    void StatEnd();
    void ReloadAll() { m_bReload = true; }
    BOOL Reload(const char* pszFileName);

private:
    BOOL IsLuaScriptFile(const char cszFileName[]);
    BOOL LoadScripts(ILuaScriptEx* piScript, const char cszDir[]);    

    BOOL Load();
    
    void CallActivateScript();

private:
    ILuaScriptEx*   m_piScript;
    BOOL            m_bReload;

private:
    BOOL UpdateStatInfo(DWORD dwScriptID, const char cszFunction[], uint64_t uCostTime);
    void OutputStatInfo();

    struct KStatInfoNode
    {
        DWORD       dwScriptID;
        DWORD       dwCount;
        uint64_t    uTime;
        kstring     sFunction;
        KStatInfoNode() : dwCount(0), uTime(0) {};
    };

    typedef KMemory::KAllocator<std::pair<kstring, KStatInfoNode> > KFUNC_STAT_INFO_ALLOCTOR;
    typedef std::map<kstring, KStatInfoNode, std::less<kstring>, KFUNC_STAT_INFO_ALLOCTOR> KFUNC_STAT_TABLE;

    KFUNC_STAT_TABLE    m_StatTable;
    BOOL                m_bStatFlag;
    DWORD               m_dwMainScriptID;
};

#endif
