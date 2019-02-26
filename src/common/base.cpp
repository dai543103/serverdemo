#ifndef _BASE_CPP_
#define _BASE_CPP_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>

#include "base.h"



void TrimStr( char *strInput )
{
    char *pb;
    char *pe;
    int iTempLength;

    if( strInput == NULL )
    {
        return;
    }

    iTempLength = strlen(strInput);
    if( iTempLength == 0 )
    {
        return;
    }

    pb = strInput;

    while (((*pb == ' ') || (*pb == '\t') || (*pb == '\n') || (*pb == '\r')) && (*pb != 0))
    {
        pb ++;
    }

    pe = &strInput[iTempLength-1];
    while ((pe >= pb) && ((*pe == ' ') || (*pe == '\t') || (*pe == '\n') || (*pe == '\r')))
    {
        pe --;
    }

    *(pe+1) = '\0';

    strcpy( strInput, pb );
    return;
}
time_t StringToDatetime(std::string str)
{
	char *cha = (char*)str.data();             // ��stringת����char*��
	tm tm_;                                    // ����tm�ṹ�塣
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// ��string�洢������ʱ�䣬ת��Ϊint��ʱ������
	tm_.tm_year = year - 1900;                 // �꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬����tm_yearΪint��ʱ������ȥ1900��
	tm_.tm_mon = month - 1;                    // �£�����tm�ṹ����·ݴ洢��ΧΪ0-11������tm_monΪint��ʱ������ȥ1��
	tm_.tm_mday = day;                         // �ա�
	tm_.tm_hour = hour;                        // ʱ��
	tm_.tm_min = minute;                       // �֡�
	tm_.tm_sec = second;                       // �롣
	tm_.tm_isdst = 0;                          // ������ʱ��
	time_t t_ = mktime(&tm_);                  // ��tm�ṹ��ת����time_t��ʽ��
	return t_;                                 // ����ֵ�� 
}

std::string DatetimeToString(time_t time)
{
	tm *tm_ = localtime(&time);                // ��time_t��ʽת��Ϊtm�ṹ��
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	year = tm_->tm_year + 1900;                // ��ʱ�������꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬������ʱ����intΪtm_year����1900��
	month = tm_->tm_mon + 1;                   // ��ʱ�������£�����tm�ṹ����·ݴ洢��ΧΪ0-11��������ʱ����intΪtm_mon����1��
	day = tm_->tm_mday;                        // ��ʱ�������ա�
	hour = tm_->tm_hour;                       // ��ʱ������ʱ��
	minute = tm_->tm_min;                      // ��ʱ�������֡�
	second = tm_->tm_sec;                      // ��ʱ�������롣
	char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// ����ʱ��ĸ���char*������
	sprintf(yearStr, "%d", year);              // �ꡣ
	sprintf(monthStr, "%d", month);            // �¡�
	sprintf(dayStr, "%d", day);                // �ա�
	sprintf(hourStr, "%d", hour);              // ʱ��
	sprintf(minuteStr, "%d", minute);          // �֡�
	if (minuteStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		minuteStr[2] = '\0';
		minuteStr[1] = minuteStr[0];
		minuteStr[0] = '0';
	}
	sprintf(secondStr, "%d", second);          // �롣
	if (secondStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		secondStr[2] = '\0';
		secondStr[1] = secondStr[0];
		secondStr[0] = '0';
	}
	char s[20];                                // ����������ʱ��char*������
	sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// ��������ʱ����ϲ���
	std::string str(s);                             // ����string����������������ʱ��char*������Ϊ���캯���Ĳ������롣
	return str;                                // ����ת������ʱ����string������
}


void SplitString2Int(const std::string&s, std::vector<uint32_t>&v, const std::string& c)
{
	if (s.empty())
	{
		return;
	}

	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;

	while (std::string::npos != pos2)
	{
		auto temp = s.substr(pos1, pos2 - pos1);
		v.emplace_back(std::stoi(temp));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}

	if (pos1 != s.length())
	{
		auto temp = s.substr(pos1);
		v.emplace_back(std::stoi(temp));
	}
}

void PieceInt2String(std::vector<uint32_t>&v, std::string&s, const std::string&c)
{
	if (v.size() > 0)
	{
		s.clear();
		s = std::to_string(v[0]);
		for (int i = 1; i < v.size(); i++)
		{
			s += c;
			s += std::to_string(v[i]);
		}
	}
}

bool CompressData(const std::string& strSource, std::string& strDest)
{
	if (LZ4_compressBound(strSource.size()) > LZ4_COMPRESS_BUFF_LEN)
	{
		return false;
	}

	//ѹ������
	static char szCompressedBuff[LZ4_COMPRESS_BUFF_LEN];
	int iCompressedLen = LZ4_compress(strSource.c_str(), szCompressedBuff, strSource.size());
	if (iCompressedLen == 0)
	{
		return false;
	}

	strDest.assign(szCompressedBuff, iCompressedLen);

	return true;
}

bool UnCompressData(const std::string& strSource, std::string& strDest)
{
	//��ѹ������
	static char szUnCompressedBuff[LZ4_COMPRESS_BUFF_LEN];
	int iUnCompressedLen = LZ4_decompress_safe(strSource.c_str(), szUnCompressedBuff, strSource.size(), sizeof(szUnCompressedBuff) - 1);
	if (iUnCompressedLen < 0)
	{
		return false;
	}

	strDest.assign(szUnCompressedBuff, iUnCompressedLen);

	return true;
}

template <class ProtoType>
bool EncodeProtoData(const ProtoType& stProtoMsg, std::string& strCompressedData)
{
	//�����л�����
	std::string strSerialized;
	if (!stProtoMsg.SerializeToString(&strSerialized))
	{
		return false;
	}

	return CompressData(strSerialized, strCompressedData);
};

//��ѹ���ݲ������л���protobuf��
template <class ProtoType>
bool DecodeProtoData(const std::string& strCompressData, ProtoType& stProtoMsg)
{
	std::string strUnCompressData;
	if (!UnCompressData(strCompressData, strUnCompressData))
	{
		return false;
	}

	//�����л�protobuf
	stProtoMsg.Clear();
	if (!stProtoMsg.ParseFromString(strUnCompressData))
	{
		return false;
	}

	return true;
};

#endif
