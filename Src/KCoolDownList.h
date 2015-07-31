#pragma once
#include <map>

#define MAX_COOL_DOWN_COUNT 512

struct KCOOL_DWON_INFO 
{
    DWORD dwID;
    int   nDuration;
    BOOL  bNeedSave;
    int   nGroupID;
};
typedef std::map<DWORD, KCOOL_DWON_INFO> KCOOL_DOWN_MAP;

class KCoolDownList
{
public:
    BOOL Init();
    void UnInit();

    int     GetCoolDownValue(DWORD dwCoolDownID); // ·µ»ØÖ¡Êý
    BOOL    IsCoolDownNeedSave(DWORD dwCoolDownID); 
    void    GetGroupCoolDown(DWORD dwCoolDownID, std::vector<DWORD>& vecCoolDown);

private:
    int     GetCoolDownGroupID(DWORD dwCoolDownID);

private:
    KCOOL_DOWN_MAP m_CoolDownTable;
};

