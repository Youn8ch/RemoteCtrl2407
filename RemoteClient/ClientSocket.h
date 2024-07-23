#pragma once
#include "pch.h"
#include "framework.h"
#include <WS2tcpip.h>
#include "Log.h"
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
			// 包数据可能不全，或包头未接受到
		}
		nLength = *(DWORD*)(pdata + i); i += 4;
		if (nLength + i > nsize)
		{
			nsize = 0; return;
			// 包数据不全
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
	WORD sHead;	// 固定位 FE FF
	DWORD nLength; // 包长度 从控制命令开始，到和校验结束
	WORD sCmd; // 控制命令
	std::string strData; // 包数据
	WORD sSum; // 和校验
	std::string strOut; // 整个包的数据
};

#pragma pack(pop)

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction; // 点击、移动、双击
	WORD nButton; // 左键、右键、中键
	POINT ptXY; // 坐标
}MOUSEEVENT, * PMOUSEEVENT;



typedef struct file_info {
	file_info() {
		IsInvalid = 0;
		IsDirectory = -1;
		HasNext = 0;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid; // 是否有效
	BOOL IsDirectory; // 是否是目录 0否 1是
	BOOL HasNext; // 是否还有后续 0否 1是
	char szFileName[256]; // 文件名
}FILEINFO, * PFILEINFO;

















std::string NEWGetErrorInfo(int wsaErrcode);

class CClientSocket
{

public:
	static CClientSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}
	bool Initsocket(int nIp,int nPort) {
		m_sock = socket(PF_INET, SOCK_STREAM, 0); // TODO : 校验
		if (m_sock == -1)
		{
			return false;
		}
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = htonl(nIp);
		serv_adr.sin_port = htons(nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			LOGE("connect addr error");
			return false;
		}
		if (connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
			LOGE(">connect server failed %d%s<",WSAGetLastError(), NEWGetErrorInfo(WSAGetLastError()).c_str());
			return false;
		}// TODO
		LOGI(">connect server done!<");
		return true;
	}

#define BUFFER_SIZE 4096

	int DealCommand() {
		if (m_sock == -1) return -1;
		char* buffer = m_buffer.data();
		if (buffer == NULL)
		{
			LOGE("> Not enough mem ");
			return -2;
		}
		// memset(buffer, 0, BUFFER_SIZE);
		while (true)
		{
			// LOGI("WAIT RECV");
			size_t len = recv(m_sock, buffer + m_index, BUFFER_SIZE - m_index, 0);
			m_index += len;
			if (m_index <= 0)
			{
				return -1;
			}
			size_t index = m_index;
			m_packet = CPacket((const BYTE*)buffer, index);

			// 如果数据包处理后剩余数据，可以适当移动缓冲区内容
			if (index < BUFFER_SIZE) {
				memmove(buffer, buffer + index, BUFFER_SIZE - index);
			}
			m_index -= index;
			return m_packet.sCmd;
		}
	}


	bool Send(const char* pdata, int size) {
		if (m_sock == -1) return false;
		return send(m_sock, pdata, size, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_sock == -1) return false;
		return send(m_sock, (const char*)pack.getData(), pack.getSize(), 0) > 0;
	}
	bool GetFilePath(std::string& path) {
		if (m_packet.sCmd >= 2 && m_packet.sCmd <= 4)
		{
			path = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEVENT& mouse) {
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEVENT));
			return true;
		}
		return false;
	}

	CPacket& GetPacket() {
		return m_packet;
	}

	void CloseClient() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		m_buffer.clear();
		m_index = 0;
	}


private:
	std::vector<char> m_buffer;
	int m_index;
	SOCKET m_sock;
	CPacket m_packet;
private:
	CClientSocket operator = (const CClientSocket&) {}
	CClientSocket(const CClientSocket&) : m_sock(INVALID_SOCKET) {}
	CClientSocket() {
		m_sock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			LOGE("初始化网络环境失败");
		}
		m_buffer.resize(BUFFER_SIZE);
	}
	~CClientSocket() {
		closesocket(m_sock);
		WSACleanup();
	}

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

	static CClientSocket* m_instance;

	static void releaseInstance() {
		if (m_instance != NULL) {
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	class CHelper
	{
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};

extern CClientSocket server;