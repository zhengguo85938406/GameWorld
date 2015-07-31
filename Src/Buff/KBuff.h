// ***************************************************************
//  Copyright(c) Kingsoft
//  FileName	: KBuff.h 
//  Creator 	: Xiayong  
//  Date		: 09/30/2011
//  Comment	: 
// ***************************************************************
#pragma once
struct KAttribute;
class KBuff
{
public:
    KBuff();
    ~KBuff();

    BOOL Init(DWORD dwID);
    void UnInit();

public:
    DWORD       m_dwID;
    KAttribute* m_pRollBackAttr; // 可回滚的属性
    int         m_nFrame;               // 持续帧数
    BOOL        m_bTimeLapseOffline;    // 离线是否计时
    BOOL        m_bDelOnFloor;          // buff是否在落地后消失
    BOOL        m_bDelOnObjNoAttached;  //buff是不是在没有指定的Doodad跟随角色的时候消失

    int         m_nActiveCount;         // 跳数
    KAttribute* m_pActiveAttr;          // 不可回滚
    BOOL        m_bNeedSave;
    BOOL        m_bShare;               //是否共享给友军
};
