#ifndef _BASE__H_
#define _BASE__H_

#include "lz4.hpp"
#include <string>
#include <vector>

#define LZ4_COMPRESS_BUFF_LEN 50*1024     //ѹ�����ݵĻ�������СΪ50K
#define ABS(a,b)			(((unsigned int) (a) > (unsigned int)(b)) ? ((a) - (b)) : ((b) - (a)))

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif


//�ַ���������
void TrimStr(char *strInput);
void SplitString2Int(const std::string&s, std::vector<uint32_t>&v, const std::string& c);
void PieceInt2String(std::vector<uint32_t>&v, std::string&s, const std::string&c);
time_t StringToDatetime(std::string str);
std::string DatetimeToString(time_t time);

//lz4ѹ������
bool CompressData(const std::string& strSource, std::string& strDest);
bool UnCompressData(const std::string& strSource, std::string& strDest);

//���л�protobuf���� ѹ��/��ѹ
template <class ProtoType>
bool EncodeProtoData(const ProtoType& stProtoMsg, std::string& strCompressedData);

template <class ProtoType>
bool DecodeProtoData(const std::string& strCompressData, ProtoType& stProtoMsg);



#endif
