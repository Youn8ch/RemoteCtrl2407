#include "pch.h"
#include "ClientSocket.h"
#include "ClientController.h"

std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;
CClientController::CHelper CClientController::m_helper;

CClientController* CClientController::getInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new CClientController();
		TRACE(_T("CClientController SIZE = %d \r\n"), sizeof(*m_instance));
		struct { UINT nMsg; MSGFUNC func; }MsgFUNCs[]{
			//{WM_SEND_PACK,&CClientController::OnSendPack},
			//{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatcher},
			{(UINT) - 1,NULL}
		};
		for (int i = 0; MsgFUNCs[i].nMsg != -1; i++)
		{
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFUNCs[i].nMsg, MsgFUNCs[i].func));
		}
	}
	return m_instance;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0,
		&CClientController::threadEntry, this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS, &m_remoteDlg);

	return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();

}

LRESULT CClientController::SendMessage(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL) return -2;
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE,(WPARAM)&info,(LPARAM)hEvent);
	WaitForSingleObject(hEvent, INFINITE); // ����ͬ�� ���¼�֪ͨ
	CloseHandle(hEvent);
	return info.result;
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();

	_endthreadex(0);
	return 0;
}

void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
			{
				pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else
			{
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else
		{
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
			{
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}
	}
}

//LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	CPacket* pPacket = (CPacket*)wParam;
//	return pClient->Send(*pPacket);
//}

//LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	char* pBuffer = (char*)wParam;
//	return pClient->Send(pBuffer, (int)lParam);
//}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}

void CClientController::ThreadEntryWatchScreen(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->ThreadWatchScreen();
	_endthread();
}

void CClientController::ThreadWatchScreen()
{
	Sleep(50);
	while (!m_isClosed)
	{
		if (m_watchDlg.isFull() == false) {
			std::list<CPacket> lstPacks;
			int ret = SendCommandPacket(6, &lstPacks);
			if (ret == 6)
			{
				if (CTool::Bytes2Image(m_watchDlg.GetImage(), lstPacks.front().strData)==0)
				{
					m_watchDlg.SetImgStatus(true);
				}
				else
				{
					TRACE(_T(" ��ȡͼƬʧ�� ret = %d \r\n",ret));
				}
			}
		}
		Sleep(1);
	}
}

void CClientController::ThreadEntryDownLoadFile(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->ThreadDownLoadFile();
	_endthread();
}

void CClientController::ThreadDownLoadFile()
{
	FILE* pFile = fopen(m_strLocal, "wb+");
	if (pFile == NULL)
	{
		AfxMessageBox("û��Ȩ�ޱ��� �� �ļ��޷�����");
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		return;
	}
	do
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		int ret = SendCommandPacket(4, NULL,false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength());
		if (ret < 0)
		{
			AfxMessageBox("ִ�������ļ�ʧ��");
			TRACE("ִ������ʧ��: ret = %d\r\n", ret);
			break;
		}
		long long nlength = *(long long*)CClientSocket::getInstance()->GetPacket().strData.c_str();
		if (nlength == 0)
		{
			AfxMessageBox("�ļ�����Ϊ0 ���޷���ȡ");
			break;
		}
		long long nCount = 0;
		// �����̺߳���
		while (nCount < nlength)
		{
			ret = pClient->DealCommand();
			if (ret < 0)
			{
				AfxMessageBox("����ʧ��");
				TRACE("����ʧ��, ret = %d\r\n", ret);
				break;
			}
			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
			nCount += pClient->GetPacket().strData.size();
		}
		AfxMessageBox("����ɹ�");

	} while (false);
	
	fclose(pFile);
	CloseSocket();
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();

}