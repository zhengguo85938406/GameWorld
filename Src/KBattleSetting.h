// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KBattleSetting.h
//  Creator		: hanruofei
//	Date		: 8/31/2012
//  Comment		: 
//	*********************************************************************

#pragma once

struct KBattleTempalte
{
    int nID;
    int nMode;
    int nTotleFrame;
    int nWinScore;
};

struct KBattleResult
{
    int nMinDeltaScore;
    int nWinPveAICoe;
    int nLosePveAICoe;
};

typedef std::map<DWORD, KBattleTempalte> KMAP_BATTLE_TEMPLATE;
typedef std::map<int, KBattleResult> KMAP_BATTLE_RESULT;

class KBattleSetting
{
public:
    KBattleSetting(void);
    ~KBattleSetting(void);
    BOOL Init();
    void UnInit();

    KBattleTempalte* GetBattleTemplate(DWORD dwBattleTemplateID);
    BOOL GetDeltaAICoe(int nDeltaScore, BOOL bSuccess, int& nDeltaAICoe);

private:
    BOOL LoadBattleTemplate();
    BOOL LoadBattleResult();

private:
    KMAP_BATTLE_TEMPLATE    m_mapBattleTemplate;
    KMAP_BATTLE_RESULT      m_mapBattleResult;
};

