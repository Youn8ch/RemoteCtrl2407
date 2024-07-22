#pragma once
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"


#pragma pack(push)
#pragma pack(1)

void Dump(BYTE* pdata, size_t length);


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
		HasNext = 1;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid; // 是否有效
	BOOL IsDirectory; // 是否是目录 0否 1是
	BOOL HasNext; // 是否还有后续 0否 1是
	char szFileName[256]; // 文件名
}FILEINFO, * PFILEINFO;










class CServerSocket
{

public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool Initsocket() {
		m_sock = socket(PF_INET, SOCK_STREAM, 0); // TODO : 校验
		if (m_sock == -1)
		{
			return false;
		}
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(8554);

		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
			return false;
		}// TODO

		if (listen(m_sock, 1) == -1) {
			return false;
		}
		return true;
	}

	bool AcceptClient() {
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1)
		{
			return false;
		}
		LOGI("ACCPET fd = %llu", m_client);
		return true;
	}

#define BUFFER_SIZE 4096

	int DealCommand() {
		if (m_client == -1) return -1;
		char* buffer = m_buffer.data();
		if (buffer == NULL)
		{
			LOGE("> Not enough mem ");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			// LOGI("WAIT RECV");
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
			{
				delete[] buffer;
				return -1;
			}
			index += len;
			m_packet = CPacket((const BYTE*)buffer, index);

			// 如果数据包处理后剩余数据，可以适当移动缓冲区内容
			if (index < BUFFER_SIZE) {
				memmove(buffer, buffer + index, BUFFER_SIZE - index);
			}
			index = 0;
			return m_packet.sCmd;
		}
	}

	bool Send(const char* pdata, int size) {
		if (m_client == -1) return false;
		return send(m_client, pdata, size, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1) return false;
		Dump((BYTE*)pack.getData(), pack.getSize());
		return send(m_client, (const char*)pack.getData(), pack.getSize(), 0) > 0;
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
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}

private:
	std::vector<char> m_buffer;
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;
private:
	CServerSocket operator = (const CServerSocket&) {}
	CServerSocket(const CServerSocket&) : m_sock(0), m_client(0) {}
	CServerSocket() {
		m_client = INVALID_SOCKET;
		m_sock = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			LOGE("初始化网络环境失败");
		}
		m_buffer.resize(BUFFER_SIZE);
	}
	~CServerSocket() {
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

	static CServerSocket* m_instance;

	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	class CHelper
	{
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};

extern CServerSocket server;