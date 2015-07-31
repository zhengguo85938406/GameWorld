#include "stdafx.h"
#include <math.h>
#include <algorithm>
#include "KMath.h"
using namespace std;

#define TAN_VALUE_COUNT (DIRECTION_COUNT / 4)
#define TAN_PRECISION   4096

// 64个元素
static int g_nTanValues[] = 
{
    50, 151, 252, 353, 454, 556, 659, 763, 867, 973, 1080, 1188, 1298, 1409, 1523, 1638, 1756, 1876, 
    1999, 2125, 2254, 2387, 2524, 2665, 2810, 2961, 3116, 3278, 3446, 3622, 3805, 3997, 4198, 4409, 
    4632, 4868, 5118, 5383, 5667, 5970, 6296, 6647, 7028, 7442, 7895, 8392, 8943, 9555, 10242, 11019, 
    11906, 12929, 14124, 15540, 17247, 19348, 22000, 25457, 30158, 36935, 47564, 66670, 111207, 333752
};

// 65个元素
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
            // 注意,0和256其实是一个角度,但是我们要返回的数值应该是[0, 255],所以0的时候就不要修正了
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
 
//判断两条线段是否相交(有交点) 
//每个线段的两点都在另一个线段的左右不同侧，则能断定线段相交   
//公式对于向量(x1,y1)->(x2,y2),判断点(x3,y3)在向量的左边,右边,还是线上.   
//p=x1(y3-y2)+x2(y1-y3)+x3(y2-y1).p<0 左侧, p=0 线上, p>0 右侧   

BOOL IsLineSegmentCross(POINT pFirst1, POINT pFirst2, POINT pSecond1, POINT pSecond2)  
{  
    long Linep1,Linep2;  
    //判断pSecond1和pSecond2是否在pFirst1->pFirst2两侧   
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


    //if (((Linep1 ^ Linep2) >= 0) && !(Linep1==0 && Linep2==0))//符号位异或为0:pSecond1和pSecond2在pFirst1->pFirst2同侧   
    //{  
    //    return false;  
    //}

    //判断pFirst1和pFirst2是否在pSecond1->pSecond2两侧   
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
    //if ( ((Linep1 ^ Linep2) >= 0 ) && !(Linep1==0 && Linep2==0))//符号位异或为0:pFirst1和pFirst2在pSecond1->pSecond2同侧   
    //{  
    //    return false;  
    //}  
    //否则判为相交   
    return true;  
}  
/* 
求两直线交点，前提是两条直线必须有交点 
在相交的情况下，可以应付各种情况(垂直、系数等) 
*/  
POINT GetCrossPoint(POINT p1, POINT p2, POINT q1, POINT q2)  
{  
    //必须相交求出的才是线段的交点，但是下面的程序段是通用的   
    assert(IsLineSegmentCross(p1,p2,q1,q2));  
    /*根据两点式化为标准式，进而求线性方程组*/  
    POINT crossPoint;  
    long tempLeft,tempRight;  
    //求x坐标   
    tempLeft = (q2.x - q1.x) * (p1.y - p2.y) - (p2.x - p1.x) * (q1.y - q2.y);  
    tempRight = (p1.y - q1.y) * (p2.x - p1.x) * (q2.x - q1.x) + q1.x * (q2.y - q1.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (q2.x - q1.x);  
    crossPoint.x =(int)( (double)tempRight / (double)tempLeft );  
    //求y坐标     
    tempLeft = (p1.x - p2.x) * (q2.y - q1.y) - (p2.y - p1.y) * (q1.x - q2.x);  
    tempRight = p2.y * (p1.x - p2.x) * (q2.y - q1.y) + (q2.x- p2.x) * (q2.y - q1.y) * (p1.y - p2.y) - q2.y * (q1.x - q2.x) * (p2.y - p1.y);  
    crossPoint.y =(int)( (double)tempRight / (double)tempLeft );  
    return crossPoint;  
}  
