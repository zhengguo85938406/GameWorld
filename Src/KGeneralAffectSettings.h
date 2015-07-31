// 	********************************************************************
//	Copyright(c) Kingsoft
//	FileName	: KGeneralAffectSettings.h
//  Creator		: hanruofei
//	Date		: 9/5/2012
//  Comment		: 
//	*********************************************************************
#pragma once

#include "KAttribute.h"
#include <map>

struct KGeneralAffect
{
    DWORD           dwID;
    int             nCostEndurance;
    int             nCostEndurancePercent;
    int             nCostStamina;
    int             nCostStaminaPercent;
    DWORD           dwBuffID;
    KVEC_ATTRIBUTES vecAttributes;
    int             nStiffFrame;
};

typedef std::map<DWORD, KGeneralAffect> KMAP_GENERALAFFECT;

class KGeneralAffectSettings
{
public:
    KGeneralAffectSettings(void);
    ~KGeneralAffectSettings(void);

    BOOL Init();
    void UnInit();

    KGeneralAffect* GetGeneralAffect(DWORD dwID);

private:
    BOOL LoadData();

private:
    KMAP_GENERALAFFECT m_mapGeneralAffect;
};

