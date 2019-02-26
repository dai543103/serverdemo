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
	char *cha = (char*)str.data();             // 将string转换成char*。
	tm tm_;                                    // 定义tm结构体。
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
	tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
	tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
	tm_.tm_mday = day;                         // 日。
	tm_.tm_hour = hour;                        // 时。
	tm_.tm_min = minute;                       // 分。
	tm_.tm_sec = second;                       // 秒。
	tm_.tm_isdst = 0;                          // 非夏令时。
	time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式。
	return t_;                                 // 返回值。 
}

std::string DatetimeToString(time_t time)
{
	tm *tm_ = localtime(&time);                // 将time_t格式转换为tm结构体
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	year = tm_->tm_year + 1900;                // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900。
	month = tm_->tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1。
	day = tm_->tm_mday;                        // 临时变量，日。
	hour = tm_->tm_hour;                       // 临时变量，时。
	minute = tm_->tm_min;                      // 临时变量，分。
	second = tm_->tm_sec;                      // 临时变量，秒。
	char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// 定义时间的各个char*变量。
	sprintf(yearStr, "%d", year);              // 年。
	sprintf(monthStr, "%d", month);            // 月。
	sprintf(dayStr, "%d", day);                // 日。
	sprintf(hourStr, "%d", hour);              // 时。
	sprintf(minuteStr, "%d", minute);          // 分。
	if (minuteStr[1] == '\0')                  // 如果分为一位，如5，则需要转换字符串为两位，如05。
	{
		minuteStr[2] = '\0';
		minuteStr[1] = minuteStr[0];
		minuteStr[0] = '0';
	}
	sprintf(secondStr, "%d", second);          // 秒。
	if (secondStr[1] == '\0')                  // 如果秒为一位，如5，则需要转换字符串为两位，如05。
	{
		secondStr[2] = '\0';
		secondStr[1] = secondStr[0];
		secondStr[0] = '0';
	}
	char s[20];                                // 定义总日期时间char*变量。
	sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// 将年月日时分秒合并。
	std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。
	return str;                                // 返回转换日期时间后的string变量。
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

	//压缩数据
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
	//解压缩数据
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
	//先序列化数据
	std::string strSerialized;
	if (!stProtoMsg.SerializeToString(&strSerialized))
	{
		return false;
	}

	return CompressData(strSerialized, strCompressedData);
};

//解压数据并反序列化到protobuf中
template <class ProtoType>
bool DecodeProtoData(const std::string& strCompressData, ProtoType& stProtoMsg)
{
	std::string strUnCompressData;
	if (!UnCompressData(strCompressData, strUnCompressData))
	{
		return false;
	}

	//反序列化protobuf
	stProtoMsg.Clear();
	if (!stProtoMsg.ParseFromString(strUnCompressData))
	{
		return false;
	}

	return true;
};

#endif
