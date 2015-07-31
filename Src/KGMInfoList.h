#ifndef _KGMINFO_LIST_H_
#define _KGMINFO_LIST_H_

struct GMAccountInfo
{
    char szGMAccount[_NAME_LEN];
};

class KGMInfoList
{
public:
    KGMInfoList();
    ~KGMInfoList();

    BOOL Init();
    void UnInit();

    BOOL CheckIsPermission(char* pszAccount);

private:
    std::vector<GMAccountInfo> m_GMAccountInfoList;
};

#endif//_KGMINFO_LIST_H_
