
#ifndef __RANDOM_HPP__
#define __RANDOM_HPP__

#include <algorithm>


const int MAX_DROP_RATE = 100000000;
const int MAX_RANDOM_NUMBER = 32767;
const int CONST_INT_TEN_THOUSAND = 10000;

typedef struct
{
    int iIdx;
    int iWeight;
} IdxWeight;

class CRandomCalculator
{

public:
    static int Initialize();

    static int GetMyRand();

    // 随机返回0~MAX_DROP_RATE之间的一个数
    static int  GetRandomInRangeHundredMillion();

    //返回0 ~ 1000 范围内的随机数
    static int GetRandomInRangeThousand();

	//返回0 ~ 10000 范围内的随机数
	static int GetRandomInRangeTenThousand();

	//返回[0,range) 区间内的一个数
	static int GetRandomNumberInRange(int iRange);

    // 随机返回一个布尔值
    static bool GetRandomBool();

    // 随机返回一个[min, max]区间数
    static int  GetRandomNumber(int iMin, int iMax);

    // 随机返回一个符合p=0.5的二项分布的[min, max]区间数
    static int  GetBinoRandNum(int iMin, int iMax);

    // 随机是否在壹万范围内
    static bool IsInRangeTenThousand(int iIn);

    // 随机是否在壹亿范围内
    static bool IsInRangeHundredMillion(int iIn);

    // 随机是否在指定范围
    static bool IsInGivenRange(int iRange, int iIn);

    // 生成[iLow, iHigh]之间的无重复的iCount个随机数
    static int GetDistinctRandNumber(const int iLow, const int iHigh,
                                     const int iCount, int aiNumber[]);

    // 生成[iLow, iHigh]之间的可重复的iCount个随机数
    static int GetManyRandNumber(const int iLow, const int iHigh,
                                 const int iCount, int aiNumber[]);


    //测试是否会成功
    static bool TestSuccess(int iSuccessRate);

    //洗牌算法
    template<typename T>
    static void Shuffle(T *array, int iNum)
    {
        for (int i = 0; i < iNum; i++)
        {
            int iRanPos = GetRandomNumber(i, iNum - 1);
            std::swap(array[i], array[iRanPos]);
        }
    }

};


#endif
