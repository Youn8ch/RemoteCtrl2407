#pragma once
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"
#include "Tool.h"
#include <list>
#include "Packet.h"

typedef void(*SOCKET_CALLBACK)(void*, int, std::list<CPacket>&, CPacket&);

class CServerSocket
{

public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool Initsocket(short port = 8554) {
		m_sock = socket(PF_INET, SOCK_STREAM, 0); // TODO : 校验
		if (m_sock == -1)
		{
			return false;
		}
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);

		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
			return false;
		}// TODO

		if (listen(m_sock, 1) == -1) {
			return false;
		}
		return true;
	}

	int Run(SOCKET_CALLBACK callback, void* arg, short port = 8554) {
		m_callback = callback;
		m_arg = arg;
		bool ret = Initsocket(port);
		if (ret == false) return -1;
		std::list<CPacket> listPackets;
		int count = 0;
		while (true)
		{
			if (AcceptClient() == false) {
				if (count>=3)
				{
					return -2;
				}
				count++;
			}
			int ret2 = DealCommand();
			if (ret2>0)
			{
				m_callback(m_arg, ret2, listPackets, m_packet);
				while(listPackets.size()>0)
				{
					Send(listPackets.front());
					listPackets.pop_front();
				}
			}
			CloseClient();
		}
		return 0;
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
		while (true)
		{
			// LOGI("WAIT RECV");
			size_t len = recv(m_client, buffer + m_index, BUFFER_SIZE - m_index, 0);
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
		if (m_client == -1) return false;
		return send(m_client, pdata, size, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1) return false;
		CTool::Dump((BYTE*)pack.getData(), pack.getSize());
		return send(m_client, (const char*)pack.getData(), pack.getSize(), 0) > 0;
	}
	bool GetFilePath(std::string& path) {
		if (m_packet.sCmd == 9 ||(m_packet.sCmd >= 2 && m_packet.sCmd <= 4))
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
		if (m_client!= INVALID_SOCKET)
		{
			closesocket(m_client);
			m_client = INVALID_SOCKET;
			m_buffer.clear();
			m_index = 0;
		}
	}

private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;
	std::vector<char> m_buffer;
	int m_index;
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;
private:
	CServerSocket& operator = (const CServerSocket& ss) {}
	CServerSocket(const CServerSocket& ss) : m_sock(0), m_client(0),m_index(0) {}
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