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

//临时代码，TAB用，加入角度限制，返回距离信息
//TODO:增加是否判断障碍的选项
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
    BOOL m_bAdjustByVisible;            //3D障碍
	BOOL m_bAdjustByAttribute;		    //根据属性调整掉等级
	BOOL m_bAdjustRangeByLevelDiff;     //根据等级差修正查找范围
    BOOL m_bAdjustRangeByTargetLife;    //根据目标血量修正查找范围
    BOOL m_b3DSearch;                   //是否使用有限圆柱(3D)搜索,高和半径相等. 此值为false表示是2D搜索,即无限圆柱
    typedef std::vector<KSearchNode, KMemory::KAllocator<KSearchNode> > KResultVec;
    KResultVec m_Result;
	BOOL operator()(KHero* pCharacter);
};
#endif //_KAISEARCH_TACTICS_H_
