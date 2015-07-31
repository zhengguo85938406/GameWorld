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
    KAttribute* m_pRollBackAttr; // �ɻع�������
    int         m_nFrame;               // ����֡��
    BOOL        m_bTimeLapseOffline;    // �����Ƿ��ʱ
    BOOL        m_bDelOnFloor;          // buff�Ƿ�����غ���ʧ
    BOOL        m_bDelOnObjNoAttached;  //buff�ǲ�����û��ָ����Doodad�����ɫ��ʱ����ʧ

    int         m_nActiveCount;         // ����
    KAttribute* m_pActiveAttr;          // ���ɻع�
    BOOL        m_bNeedSave;
    BOOL        m_bShare;               //�Ƿ�����Ѿ�
};
