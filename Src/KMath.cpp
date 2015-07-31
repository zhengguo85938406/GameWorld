#include "stdafx.h"
#include <math.h>
#include <algorithm>
#include "KMath.h"
using namespace std;

#define TAN_VALUE_COUNT (DIRECTION_COUNT / 4)
#define TAN_PRECISION   4096

// 64��Ԫ��
static int g_nTanValues[] = 
{
    50, 151, 252, 353, 454, 556, 659, 763, 867, 973, 1080, 1188, 1298, 1409, 1523, 1638, 1756, 1876, 
    1999, 2125, 2254, 2387, 2524, 2665, 2810, 2961, 3116, 3278, 3446, 3622, 3805, 3997, 4198, 4409, 
    4632, 4868, 5118, 5383, 5667, 5970, 6296, 6647, 7028, 7442, 7895, 8392, 8943, 9555, 10242, 11019, 
    11906, 12929, 14124, 15540, 17247, 19348, 22000, 25457, 30158, 36935, 47564, 66670, 111207, 333752
};

// 65��Ԫ��
static int g_nSinValues[] = 
{
    0, 101, 201, 301, 401, 501, 601, 700, 799, 897, 995, 1092, 1189, 1285, 1380, 1474, 1567, 1660, 
    1751, 1842, 1931, 2019, 2106, 2191, 2276, 2359, 2440, 2520, 2598, 2675, 2751, 2824, 2896, 2967, 
    3035, 3102, 3166, 3229, 3290, 3349, 3406, 3461, 3513, 3564, 3612, 3659, 3703, 3745, 3784, 3822, 
    3857, 3889, 3920, 3948, 3973, 3996, 4017, 4036, 4052, 4065, 4076, 4085, 4091, 4095, 4096
};

int g_GetDirection(int nX, int nY)
{
    int     nDirection  = 0;
    int     nX1         = nX;
    int     nY1         = nY;
    int*    pnPos       = NULL;
    int     nValue      = 0;
    int     nDirection1 = 0;

    if (nX == 0)
    {
        if (nY > 0)
        {
            nDirection = DIRECTION_COUNT / 4;
        }
        else if (nY < 0)
        {
            nDirection = (DIRECTION_COUNT / 4) * 3;
        }
        goto Exit0;
    }

    if (nX1 < 0)
    {
        nX1 = -nX1;
    }

    if (nY1 < 0)
    {
        nY1 = -nY1;
    }

    nValue = nY1 * TAN_PRECISION / nX1;
    if (nY1 >= INT_MAX / TAN_PRECISION)
    {
        nValue = (int)(((long long)nY1) * TAN_PRECISION / nX1);
    }

    pnPos = lower_bound(g_nTanValues, g_nTanValues + TAN_VALUE_COUNT, nValue);

    nDirection1 = (int)(pnPos - g_nTanValues);
    
    if (nX < 0)
    {
        if (nY >= 0)
        {
            nDirection = DIRECTION_COUNT / 2 - nDirection1;
        }
        else
        {
            nDirection = DIRECTION_COUNT / 2 + nDirection1;
        }
    }
    else
    {
        if (nY >= 0)
        {
            nDirection = nDirection1;
        }
        else
        {
            // ע��,0��256��ʵ��һ���Ƕ�,��������Ҫ���ص���ֵӦ����[0, 255],����0��ʱ��Ͳ�Ҫ������
            if (nDirection1 > 0)
            {
                nDirection = DIRECTION_COUNT - nDirection1;
            }
        }
    }
    
Exit0:
    assert(nDirection >= 0);
    assert(nDirection < DIRECTION_COUNT);
    return nDirection;
}


int g_Sin(int nDirection)
{
    int nResult = 0;

    nDirection = nDirection % DIRECTION_COUNT;

    if (nDirection < 0)
    {
        nDirection += DIRECTION_COUNT;
    }

    if (nDirection <= DIRECTION_COUNT / 4)
    {
        nResult = g_nSinValues[nDirection];
    }
    else if (nDirection <= DIRECTION_COUNT / 2)
    {
        nResult = g_nSinValues[DIRECTION_COUNT / 2 - nDirection];
    }
    else if (nDirection <= DIRECTION_COUNT * 3 / 4)
    {
        nResult = -g_nSinValues[nDirection - DIRECTION_COUNT / 2];
    }
    else
    {
        nResult = -g_nSinValues[DIRECTION_COUNT - nDirection];
    }

    return nResult;
}
 
//�ж������߶��Ƿ��ཻ(�н���) 
//ÿ���߶ε����㶼����һ���߶ε����Ҳ�ͬ�࣬���ܶ϶��߶��ཻ   
//��ʽ��������(x1,y1)->(x2,y2),�жϵ�(x3,y3)�����������,�ұ�,��������.   
//p=x1(y3-y2)+x2(y1-y3)+x3(y2-y1).p<0 ���, p=0 ����, p>0 �Ҳ�   

BOOL IsLineSegmentCross(POINT pFirst1, POINT pFirst2, POINT pSecond1, POINT pSecond2)  
{  
    long Linep1,Linep2;  
    //�ж�pSecond1��pSecond2�Ƿ���pFirst1->pFirst2����   
    Linep1 = pFirst1.x * (pSecond1.y - pFirst2.y) +  
        pFirst2.x * (pFirst1.y - pSecond1.y) +  
        pSecond1.x * (pFirst2.y - pFirst1.y);  
    Linep2 = pFirst1.x * (pSecond2.y - pFirst2.y) +  
        pFirst2.x * (pFirst1.y - pSecond2.y) +  
        pSecond2.x * (pFirst2.y - pFirst1.y);

    if (Linep1 > 0 && Linep2 > 0)
        return false;

    if (Linep1 < 0 && Linep2 < 0)
        return false;


    //if (((Linep1 ^ Linep2) >= 0) && !(Linep1==0 && Linep2==0))//����λ���Ϊ0:pSecond1��pSecond2��pFirst1->pFirst2ͬ��   
    //{  
    //    return false;  
    //}

    //�ж�pFirst1��pFirst2�Ƿ���pSecond1->pSecond2����   
    Linep1 = pSecond1.x * (pFirst1.y - pSecond2.y) +  
        pSecond2.x * (pSecond1.y - pFirst1.y) +  
        pFirst1.x * (pSecond2.y - pSecond1.y);  
    Linep2 = pSecond1.x * (pFirst2.y - pSecond2.y) +  
        pSecond2.x * (pSecond1.y - pFirst2.y) +  
        pFirst2.x * (pSecond2.y - pSecond1.y);

    if (Linep1 > 0 && Linep2 > 0)
        return false;

    if (Linep1 < 0 && Linep2 < 0)
        return false;
    //if ( ((Linep1 ^ Linep2) >= 0 ) && !(Linep1==0 && Linep2==0))//����λ���Ϊ0:pFirst1��pFirst2��pSecond1->pSecond2ͬ��   
    //{  
    //    return false;  
    //}  
    //������Ϊ�ཻ   
    return true;  
}  
/* 
����ֱ�߽��㣬ǰ��������ֱ�߱����н��� 
���ཻ������£�����Ӧ���������(��ֱ��ϵ����) 
*/  
POINT GetCrossPoint(POINT p1, POINT p2, POINT q1, POINT q2)  
{  
    //�����ཻ����Ĳ����߶εĽ��㣬��������ĳ������ͨ�õ�   
    assert(IsLineSegmentCross(p1,p2,q1,q2));  
    /*��������ʽ��Ϊ��׼ʽ�����������Է�����*/  
    POINT crossPoint;  
    long tempLeft,tempRight;  
    //��x����   
    tempLeft = (q2.x - q1.x) * (p1.y - p2.y) - (p2.x - p1.x) * (q1.y - q2.y);  
    tempRight = (p1.y - q1.y) * (p2.x - p1.x) * (q2.x - q1.x) + q1.x * (q2.y - q1.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (q2.x - q1.x);  
    crossPoint.x =(int)( (double)tempRight / (double)tempLeft );  
    //��y����     
    tempLeft = (p1.x - p2.x) * (q2.y - q1.y) - (p2.y - p1.y) * (q1.x - q2.x);  
    tempRight = p2.y * (p1.x - p2.x) * (q2.y - q1.y) + (q2.x- p2.x) * (q2.y - q1.y) * (p1.y - p2.y) - q2.y * (q1.x - q2.x) * (p2.y - p1.y);  
    crossPoint.y =(int)( (double)tempRight / (double)tempLeft );  
    return crossPoint;  
}  
