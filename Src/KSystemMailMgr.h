// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KSystemMail.h
//  Creator		: zhoukezhen 
//	Date		: 11/26/2012
//  Comment		: 
//	*********************************************************************

#pragma once

class KSystemMailTxt
{
public:
    int  m_nMailType;
    char m_szSendName[_NAME_LEN];
    char m_szMailTitl[64];
    char m_szMailTxt[128];
};

typedef std::map<DWORD, KSystemMailTxt> KMAP_SYSMAIL;

class KSystemMailMgr
{
public:
    KSystemMailMgr(void);
    ~KSystemMailMgr(void);
    BOOL Init();
    void UnInit();

    KSystemMailTxt* GetSystemMailTxt(DWORD dwSystemMailTxtID);

private:
    BOOL LoadSystemMailTxt();
private:
    KMAP_SYSMAIL    m_mapSystemMail;
}; 

enum KSYSTEMMAIL
{
    KMAIL_NONE= 0,
    KMAIL_CHOOSE = 1,
    KMAIL_SHOP= 2,
    KMAIL_CHALLENGE = 3,
    KMAIL_ACHIEVEMENT = 4,
    KMAIL_BATTLE_AWARD = 5,
    KMAIL_ACTIVEPLAYER = 6,
    KMAIL_QUESTIONNAIRE = 7,
};


