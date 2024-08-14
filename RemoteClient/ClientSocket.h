#pragma once
#include "pch.h"
#include "framework.h"
#include <WS2tcpip.h>
#include "Log.h"
#include <map>
#include <list>
#include <mutex>

#define WM_SEND_PACK (WM_USER+1)  // 发送包数据
#define WM_SEND_PACK_ACK (WM_USER+2)  // 发送包数据应答



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
	CPacket(const BYTE* pdata, size_t& nsize) : sHead(0),
		nLength(0),
		sCmd(0),
		sSum(0)
	{
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
			nsize = 0; 
			return;
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
		if (sum == sSum)
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
	const char* getData(std::string & strOut) const {
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
}FILEINFO, *PFILEINFO;













enum MyEnum
{
	CSM_AUTOCLOSE = 1,// CSM Client Socket Mode 自动关闭模式
};

typedef struct PacketData {
	std::string strData;
	UINT nMode;
	WPARAM wParam;
	PacketData(const char* pData, size_t nLen, UINT mode, WPARAM nParam=0) {
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
		wParam = nParam;
	}
	PacketData(const PacketData& data) {
		strData = data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PacketData& operator=(const PacketData& data) {
		if (this != &data)
		{
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this;
	}
}PACKET_DATA;

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
	bool Initsocket() {
		if (m_sock!=INVALID_SOCKET)
		{
			CloseClient();
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0); // TODO : 校验
		if (m_sock == -1)
		{
			return false;
		}
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = htonl(m_nIp);
		serv_adr.sin_port = htons(m_nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			TRACE("connect addr error\r\n");
			return false;
		}
		if (connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
			TRACE(">connect server failed %d%s \r\n<", WSAGetLastError(), NEWGetErrorInfo(WSAGetLastError()).c_str());
			TRACE(" m_sock = %d\r\n", m_sock);
			return false;
		}// TODO

		return true;
	}

#define BUFFER_SIZE 2048000

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
			int len = recv(m_sock, buffer + m_index, BUFFER_SIZE - m_index, 0);
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
			m_index -= (int)index;
			return m_packet.sCmd;
		}
	}


	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed = true,WPARAM wParam = 0) {
		UINT nMode = isAutoClosed ? CSM_AUTOCLOSE : 0;
		std::string strOut;
		pack.getData(strOut);
		PACKET_DATA* pData = new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam);
		bool ret = PostThreadMessage(m_Threadid, WM_SEND_PACK,
			(WPARAM)pData, (LPARAM)hWnd);
		if (!ret)
		{
			delete pData;
		}
		return ret;
	}

	//bool SendPacket (const CPacket& pack,std::list<CPacket>& lstpacks,bool isAutoClosed = true) {
	//	m_lock.lock();
	//	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent,lstpacks));
	//	m_mapAutoClosed.insert(std::pair < HANDLE, bool>(pack.hEvent, isAutoClosed));
	//	m_lstSend.push_back(pack);
	//	m_lock.unlock();
	//	while (!(m_sock == INVALID_SOCKET && m_hThread == INVALID_HANDLE_VALUE))
	//	{
	//		Sleep(1);
	//	} 
	//	m_hThread = (HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
	//	TRACE(" Thread : %d \r\n", m_hThread);


	//	WaitForSingleObject(pack.hEvent, INFINITE);
	//	// CloseHandle(pack.hEvent); // 回收事件句柄
	//	std::map<HANDLE, std::list<CPacket>&>::iterator it;
	//	m_lock.lock();
	//	it = m_mapAck.find(pack.hEvent);
	//	if (it != m_mapAck.end())
	//	{
	//		m_mapAck.erase(it);
	//		m_lock.unlock();
	//		return true;
	//	}
	//	m_lock.unlock();
	//	return false;
	//}

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
		if (m_sock != INVALID_SOCKET)
		{
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			Clearbuffer();
		}
		// WSACleanup();
	}

	void UpdateAddress(int nip, int nport) {
		if (m_nIp != nip  || m_nPort != nport)
		{
			m_nIp = nip;
			m_nPort = nport;
		}
	}

	void Clearbuffer() {
		m_buffer.clear();
		m_index = 0;
	}

public:
	static unsigned __stdcall threadEntry(void* arg);
protected:
	// void threadFunc();

public:
	void threadFunc2();
	unsigned m_Threadid;

	// wParam 缓冲区的值
	// lParam 缓冲区长度
	void SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);

private:

	// wParam 缓冲区的值
	// lParam 缓冲区长度
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam /*缓冲区的值*/, LPARAM lParam /*缓冲区长度*/);
	std::map<UINT, MSGFUNC> m_mapFunc;

private:
	HANDLE m_eventInvoke; // 启动事件
	HANDLE m_hThread;
	std::mutex m_lock;
	std::list<CPacket> m_lstSend;
	std::map<HANDLE, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;

	std::vector<char> m_buffer;
	int m_index;
	SOCKET m_sock;
	CPacket m_packet;

private:
	int m_nIp;
	int m_nPort;

private:
	bool Send(const char* pdata, int size);
	bool Send(const CPacket& pack);
	CClientSocket operator = (const CClientSocket&) {}
	CClientSocket(const CClientSocket& ss){
		m_nIp = ss.m_nIp;
		m_nPort = ss.m_nPort;
		m_sock = ss.m_sock;
		m_index = ss.m_index;
		memcpy(&m_buffer, &ss.m_buffer, m_index);
		std::map<UINT, CClientSocket::MSGFUNC>::const_iterator it = ss.m_mapFunc.begin();
		for (; it != ss.m_mapFunc.end(); it++)
		{
			m_mapFunc.insert(std::pair<UINT,MSGFUNC>(it->first,it->second));
		}
	}
	CClientSocket() :
		m_sock(INVALID_SOCKET),
		m_index(0),
		m_nIp(INADDR_ANY),
		m_nPort(0),
		m_hThread(INVALID_HANDLE_VALUE){
		if (InitSockEnv() == FALSE) {
			LOGE("初始化网络环境失败");
		}
		m_buffer.resize(BUFFER_SIZE);
		m_eventInvoke = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientSocket::threadEntry, this, 0, &m_Threadid);
		if (m_eventInvoke == 0) return;
		if (WaitForSingleObject(m_eventInvoke,100) == WAIT_TIMEOUT)
		{
			TRACE("网络消息处理线程启动失败 \r\n");
		}
		CloseHandle(m_eventInvoke);

		struct
		{
			UINT message;
			MSGFUNC func;
		}funcs[] = {
			{WM_SEND_PACK,&CClientSocket::SendPack},
			{0,NULL}
		};
		for (int i = 0; funcs[i].message != 0; i++)
		{
			if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false) {
				TRACE(" SOCKET MAPFUNC insert failed \r\n");
			}
		}

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