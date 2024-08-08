#pragma once
#include "ClientSocket.h"
#include "WatchDialog.h"
#include "StatusDlg.h"
#include "RemoteClientDlg.h"
#include <map>
#include "resource.h"
#include "Tool.h"

#define WM_SHOW_STATUS (WM_USER+3) // չʾ״̬
#define WM_SHOW_WATCH (WM_USER+4) // Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000) // �Զ���

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

	// 1 �鿴���̷���
	// 2 �鿴ָ��Ŀ¼���ļ�
	// 3 ���ļ�
	// 4 �鿴�ļ�
	// 9 ɾ���ļ�
	// 5 ������
	// 6 ��Ļ����
	// 7 ����
	// 8 ����
	// 666 ��������
	// ����ֵ ״̬
	bool SendCommandPacket(
		HWND hWnd, // ���ݰ��յ�����ҪӦ��Ĵ���
		int nCmd,
		bool bAutoclose = true, 
		BYTE* pData = NULL, 
		size_t nLength = 0) 
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		return pClient->SendPacket(hWnd, CPacket(nCmd, pData, nLength), bAutoclose);
	}

	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return CTool::Bytes2Image(image, pClient->GetPacket().strData);
	}

	int DownFile(CString path) {

		CFileDialog dlg(FALSE, "*", path,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, &m_remoteDlg);
		if (dlg.DoModal() == IDOK) {
			m_strRemote = path;
			m_strLocal = dlg.GetPathName();

			m_hThreadDownload = (HANDLE)_beginthread(&CClientController::ThreadEntryDownLoadFile, 0, this);
			if (WaitForSingleObject(m_hThreadDownload,0) != WAIT_TIMEOUT) return -1;


			m_remoteDlg.BeginWaitCursor();
			m_statusDlg.m_info.SetWindowText(_T("ִ����..."));
			m_statusDlg.ShowWindow(SW_SHOW);
			m_statusDlg.CenterWindow(&m_remoteDlg);
			m_statusDlg.SetActiveWindow();


		}
		return 0;
	}

	void StartWatchScreen() {
		m_isClosed = false;
		// m_watchDlg.SetParent(&m_remoteDlg);
		HANDLE hTread = (HANDLE)_beginthread(CClientController::ThreadEntryWatchScreen, 0, this);
		m_watchDlg.DoModal();
		m_isClosed = true;
		WaitForSingleObject(hTread, 500);
	}

protected:

	CClientController():
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg)
	{
		m_isClosed = true;
		m_hThread = INVALID_HANDLE_VALUE;
		m_hThreadDownload = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	}
	static void releaseInstance() {
		if (m_instance != NULL)
		{
			CClientController* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static unsigned __stdcall threadEntry(void* arg);
	void threadFunc();

//	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
//	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);

protected:
	static void ThreadEntryWatchScreen(void* arg);
	void ThreadWatchScreen();

	static void ThreadEntryDownLoadFile(void* arg);
	void ThreadDownLoadFile();

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
	CString m_strRemote; // �����ļ�Զ��·��
	CString m_strLocal; // �����ļ��ı��ر���·��
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	HANDLE m_hThreadDownload;
	unsigned int m_nThreadID;
	bool m_isClosed; // �����Ƿ�ر�

	class CHelper
	{
	public:
		CHelper() {
		    // CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;


};

