#ifndef _KAISEARCH_TACTICS_H_
#define _KAISEARCH_TACTICS_H_

#include <list>
#include <vector>

#include "KSceneObject.h"
#include "KScene.h"

struct KVirtualMove 
{
    int m_nSrcX;
    int m_nSrcY;
    int m_nSrcZ;

    int m_nDstX;
    int m_nDstY;
    int m_nDstZ;

    int m_nDistance;

    int m_State;
    // ... ...


}; 

struct KSearchNode
{
	KHero* m_pCharacter;
	int m_nDistance2;
};

bool    g_CompByDistance(const KSearchNode& left, const KSearchNode& right);
double  g_GetRangeByLevelDiff(int nLevelDiff);

template <typename T>
void AISearchPlayer(T& rTactic)
{
    KSceneObject* pCenterObj = (KSceneObject*)rTactic.m_pSelf;
    KScene*       pScene     = NULL;

    assert(pCenterObj);

    pScene = pCenterObj->m_pScene;
    KGLOG_PROCESS_ERROR(pScene);

    //pScene->TraverseRangePlayer(pCenterObj->m_pRegion, rTactic, pScene->m_nBroadcastRegion);

Exit0:
    return;
}

template<typename TSelfType>
struct KSearchTactic
{
    KSearchTactic() : m_pSelf(NULL){}
    TSelfType* m_pSelf;
};

//��ʱ���룬TAB�ã�����Ƕ����ƣ����ؾ�����Ϣ
//TODO:�����Ƿ��ж��ϰ���ѡ��
struct KSearchForCharacter : KSearchTactic<KHero>
{
	KSearchForCharacter() : m_nDistance(INT_MAX)
        , m_nAngle(DIRECTION_COUNT)
        , m_nRelation(0)
        , m_nLifeConversionLevel(0)
        , m_bAdjustByVisible(true)
        , m_bAdjustByAttribute(false)
        , m_bAdjustRangeByLevelDiff(false)
        , m_bAdjustRangeByTargetLife(false)
        , m_b3DSearch(true)
        { m_Result.reserve(16);}
	int m_nDistance;
	int m_nAngle;
	int m_nRelation;
    int m_nLifeConversionLevel;
    BOOL m_bAdjustByVisible;            //3D�ϰ�
	BOOL m_bAdjustByAttribute;		    //�������Ե������ȼ�
	BOOL m_bAdjustRangeByLevelDiff;     //���ݵȼ����������ҷ�Χ
    BOOL m_bAdjustRangeByTargetLife;    //����Ŀ��Ѫ���������ҷ�Χ
    BOOL m_b3DSearch;                   //�Ƿ�ʹ������Բ��(3D)����,�ߺͰ뾶���. ��ֵΪfalse��ʾ��2D����,������Բ��
    typedef std::vector<KSearchNode, KMemory::KAllocator<KSearchNode> > KResultVec;
    KResultVec m_Result;
	BOOL operator()(KHero* pCharacter);
};
#endif //_KAISEARCH_TACTICS_H_
