#pragma once
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"
#include "Tool.h"
#include <list>
#pragma pack(push)
#pragma pack(1)




class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(WORD nCmd, const BYTE* pdata, size_t nsize) {
		sHead = 0xFEFF;
		nLength = (unsigned long)(nsize + 4);
		sCmd = nCmd;
		if (nsize > 0)
		{
			strData.resize(nsize);
			memcpy(&strData[0], pdata, nsize);
		}
		else
		{
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pdata, size_t& nsize) : sHead(0), nLength(0), sCmd(0), sSum(0) {
		size_t i = 0;
		for (; i < nsize; i++)
		{
			if (*(WORD*)(pdata + i) == 0xFEFF)
			{
				sHead = *(WORD*)(pdata + i);
				i += 2;
				break;
			}
		}
		if (i + 8 > nsize)
		{
			nsize = 0;
			return;
			// �����ݿ��ܲ�ȫ�����ͷδ���ܵ�
		}
		nLength = *(DWORD*)(pdata + i); i += 4;
		if (nLength + i > nsize)
		{
			nsize = 0; return;
			// �����ݲ�ȫ
		}
		sCmd = *(WORD*)(pdata + i); i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pdata + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pdata + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum = sSum)
		{
			nsize = i; // head 2 length 4 data
			return;
		}
		nsize = 0;

	}
	CPacket operator=(const CPacket& pack) {
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
	~CPacket() {}

public:
	int getSize() {
		return nLength + 6;
	}
	const char* getData() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

public:
	WORD sHead;	// �̶�λ FE FF
	DWORD nLength; // ������ �ӿ������ʼ������У�����
	WORD sCmd; // ��������
	std::string strData; // ������
	WORD sSum; // ��У��
	std::string strOut; // ������������
};

#pragma pack(pop)


typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction; // ������ƶ���˫��
	WORD nButton; // ������Ҽ����м�
	POINT ptXY; // ����
}MOUSEEVENT, * PMOUSEEVENT;



typedef struct file_info {
	file_info() {
		IsInvalid = 0;
		IsDirectory = -1;
		HasNext = 1;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid; // �Ƿ���Ч
	BOOL IsDirectory; // �Ƿ���Ŀ¼ 0�� 1��
	BOOL HasNext; // �Ƿ��к��� 0�� 1��
	char szFileName[256]; // �ļ���
}FILEINFO, * PFILEINFO;