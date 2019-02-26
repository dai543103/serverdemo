
#include "Random.hpp"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>


const int MAX_RANDOM_NUM = 1000000;

#ifdef GET_RAND_FROM_SHM
int CRandomCalculator::m_iRandRingBuffIdx = 0;
int *CRandomCalculator::m_piRandRingBuff = NULL;
#endif

int CRandomCalculator::Initialize()
{
    srand(time(NULL));

    return 0;
}

int CRandomCalculator::GetMyRand()
{
    return rand();
}

int CRandomCalculator::GetRandomNumberInRange(int iRange)
{
	if(iRange == 0)
	{
		return 0;
	}

	return GetMyRand()%iRange;
}

int CRandomCalculator::GetRandomNumber(int iMin, int iMax)
{
    int i = GetMyRand();
    if (iMin > iMax)
    {
        iMin = iMax;
    }

    return (i % (iMax - iMin + 1)) + iMin;
}

// �������0~MAX_DROP_RATE֮���һ����
int CRandomCalculator::GetRandomInRangeHundredMillion()
{
    return GetRandomNumber(0, MAX_DROP_RATE);
}

//pluto jasonxiong �����ӷ���0 ~ 1000 ��Χ�ڵ������
int CRandomCalculator::GetRandomInRangeThousand()
{
    return GetRandomNumber(0, 1000);
}

//jasonxiong �����ӷ���0 ~ 1000 ��Χ�ڵ������
int CRandomCalculator::GetRandomInRangeTenThousand()
{
	return GetRandomNumber(0, 10000);
}

bool CRandomCalculator::GetRandomBool()
{
    int i = 0;

    i = GetMyRand();

    if (0 == (i % 2) )
    {
        return false;
    }

    return true;
}

// �Ƿ��������Χ��
bool CRandomCalculator::IsInRangeTenThousand(int iIn)
{
    if (GetRandomNumber(1, 10000) <= iIn)
    {
        return true;
    }

    return false;
}

bool CRandomCalculator::IsInRangeHundredMillion(int iIn)
{
    if (GetRandomNumber(1, MAX_DROP_RATE) <= iIn)
    {
        return true;
    }

    return false;
}

bool CRandomCalculator::IsInGivenRange(int iRange, int iIn)
{
    if (iRange <= 0 || iIn <= 0)
    {
        return false;
    }

    if (GetRandomNumber(1, iRange) <= iIn)
    {
        return true;
    }

    return false;
}

// ����[iLow, iHigh]֮������ظ���iCount�������
int CRandomCalculator::GetDistinctRandNumber(const int iLow, const int iHigh,
        const int iCount, int aiNumber[])
{
    const int iTotal = iHigh - iLow + 1;
    if (iCount > iTotal)
    {
        return -1;
    }

    int* piTemp = new int[iTotal];
    if (!piTemp)
    {
        return -1;
    }

    int iRange = iTotal;
    int iIdx;
    int i, j;

    for (i = 0; i < iTotal; i++)
    {
        piTemp[i] = i + iLow; // ��������ܵ�ȡֵ
    }

    for (j = 0; j < iCount; j++)
    {
        iIdx = GetMyRand() % iRange;
        aiNumber[j] = piTemp[iIdx];
        piTemp[iIdx] = piTemp[iRange - 1];
        iRange--;
    }

    delete [] piTemp;
    return 0;
}

// ����[iLow, iHigh]֮��Ŀ��ظ���iCount�������
int CRandomCalculator::GetManyRandNumber(const int iLow, const int iHigh,
        const int iCount, int aiNumber[])
{

    if (iHigh < iLow)
    {
        return -3;
    }

    const int iTotal = iHigh - iLow + 1;

    int iRange = iTotal;
    int j;

    for (j = 0; j < iCount; j++)
    {
        aiNumber[j] = GetMyRand() % iRange + iLow;
    }

    return 0;
}


// ����iMin��iMax֮����϶���ֲ��������R ~ B(N�� p)
// ����N=iMax-iMin����N>10, �̶�p=0.5���ֶԳ���
// ������ģ����̬���������ʱ����ΪNp
// �������ܿ��ǣ�����N����10��100֮��
// ��������ֵ�����齫����������ȷ֣����ȷ�ֵ���룬
// �˺�����ȡ��ֵ��ʶ�ȷ����䣬Ȼ�����ڸ�����ƽ�����
// ʵ���㷨��ʵ�鷽ʽ
int CRandomCalculator::GetBinoRandNum(int iMin, int iMax)
{

    int iNum = iMax - iMin;
    int iHit = 0;
    for (int i = 0; i < iNum; ++i)
    {
        if ((GetMyRand() % 100) < 50 )
        {
            ++iHit;
        }
    }

    return iMin + iHit;
}

bool CRandomCalculator::TestSuccess(int iSuccessRate)
{
    srand(time(NULL));

    if(iSuccessRate < 0)
    {
        return false;
    }

    if(iSuccessRate >= 100)
    {
        return true;
    }

    int iRadomNumber = GetMyRand() % 100;

    if(iRadomNumber < iSuccessRate)
    {
        return true;
    }
    else
    {
        return false;
    }
}






