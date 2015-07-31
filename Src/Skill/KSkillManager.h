// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KSkillManager.h 
//  Creator 	: Xiayong  
//  Date		: 09/16/2011
//  Comment	: 
// ***************************************************************
#pragma once
#include "KSkill.h"
#include "Engine/KMemory.h"
#include "KSpecialSkill.h"
#include "KTools.h"

class KSkillManager
{
public:
    KSkillManager();
    ~KSkillManager();

    BOOL Init();
    void UnInit();

    KSkill* GetSkill(DWORD dwSkillID, DWORD dwStepID = 0U);
    KSpecialSkillInfo* GetSpecialSkill(KSPECIAL_SKILL eID);

private:
    BOOL LoadSkillTable();    
	BOOL LoadSkillBinaryData(KMAP_SKILL_DATA& allBinarySkillData);
    BOOL LoadAnchors(BYTE*& pBuffer, size_t& uBufferSize, KMAP_ANCHOR& rMapAnchors);
    BOOL LoadSegments(BYTE*& pBuffer, size_t& uBufferSize, KMAP_SEGMENT& rMapSegments);
    BOOL LoadAttributes(BYTE*& pBuffer, size_t& uBufferSize, std::vector<KAttribData>& rVecAttributes);
    BOOL LoadBullets(BYTE*& pBuffer, size_t& uBufferSize, std::vector<DWORD>& rVecBullets);

private:
    typedef KMemory::KAllocator<std::pair<uint64_t, KSkill> > KSkillTableAlloctor;
    typedef std::map<uint64_t, KSkill, std::less<uint64_t>,  KSkillTableAlloctor>  KSkillTable;
    KSkillTable  m_SkillTable;
    KSpecialSkill m_SpecSkill;
};

