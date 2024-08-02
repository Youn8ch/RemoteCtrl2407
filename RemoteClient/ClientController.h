#pragma once
#include "ClientSocket.h"
#include "WatchDialog.h"
#include "StatusDlg.h"
#include "RemoteClientDlg.h"
#include <map>
#include "resource.h"
#include "Tool.h"
#define WM_SEND_PACK (WM_USER+1)  // 发送包数据
#define WM_SEND_DATA (WM_USER+2)  // 发送数据
#define WM_SHOW_STATUS (WM_USER+3) // 展示状态
#define WM_SHOW_WATCH (WM_USER+4) // 远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000) // 自定义

class CClientController
{

public:
	static CClientController* getInstance();
	int InitController();
	int Invoke(CWnd*& pMainWnd);
	LRESULT SendMessage(MSG msg);
	void UpdateAddress(int ip, int port) {
		CClientSocket::getInstance()->UpdateAddress(ip, port);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseClient();
	}
	bool SendPacket(const CPacket& pack) {
		CClientSocket* pClient = CClientSocket::getInstance();
		if (pClient->Initsocket() == false) return false;
		pClient->Send(pack);
	}
	int SendCommandPacket(
		int nCmd, 
		bool bAutoclose = true, 
		BYTE* pData = NULL, 
		size_t nLength = 0) {
		SendPacket(CPacket(nCmd, pData, nLength));
		int cmd = DealCommand();
		TRACE("ACK cmd = %d\r\n", cmd);
		if (bAutoclose) CloseSocket();
		return cmd;
	}

	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return CTool::Bytes2Image(image, pClient->GetPacket().strData);
	}


protected:

	CClientController():
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg)
	{
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	}
	static void releaseInstance() {
		if (m_instance != NULL)
		{
			m_instance = NULL;
		}
	}
	static unsigned __stdcall threadEntry(void* arg);
	void threadFunc();

	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);

private:

	typedef struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &(m.msg), sizeof(MSG));
		}
		MsgInfo& operator=(const const MsgInfo& m) {
			if (this != &m)
			{
				result = m.result;
				memcpy(&msg, &(m.msg), sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;

	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	static CClientController* m_instance;
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	unsigned int m_nThreadID;

	class CHelper
	{
	public:
		CHelper() {
			CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;


};

