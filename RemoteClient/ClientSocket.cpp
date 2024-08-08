#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;

CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();

std::string NEWGetErrorInfo(int wsaErrcode)
{

	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;

}

unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc2();
	_endthreadex(0);
	return 0;
}

//void CClientSocket::threadFunc()
//{
//	
//	std::string strBuffer;
//	strBuffer.resize(BUFFER_SIZE);
//	char* pBuffer = (char*)strBuffer.c_str();
//	int index = 0;
//	int len = 0;
//	size_t size = 0;
//	while (m_lstSend.size() > 0)
//	{
//		if (Initsocket() == false)
//		{
//			return;
//		}
//		m_lock.lock();
//		CPacket& head = m_lstSend.front();
//		m_lock.unlock();
//		if (Send(head) == false)
//		{
//			TRACE(_T("发送失败\r\n"));
//			Initsocket();
//			break;
//		}
//		std::map<HANDLE, std::list<CPacket>&>::iterator it;
//		it = m_mapAck.find(head.hEvent);
//		std::map<HANDLE, bool>::iterator itclosed;
//		itclosed = m_mapAutoClosed.find(head.hEvent);
//		if (it == m_mapAck.end() || itclosed == m_mapAutoClosed.end()) {
//			TRACE(" ACK ERROR \r\n");
//			break;
//		}
//		do
//		{
//			len = recv(m_sock, pBuffer+index, BUFFER_SIZE - index, 0);
//			if (len > 0 || index > 0)
//			{
//				TRACE(" len = %d \r\n", len);
//				index += len;
//				size = index;
//				CPacket pack((BYTE*)pBuffer, size);
//				TRACE(" index1 = %d \r\n", index);
//				if (size > 0)
//				{
//					if (index < BUFFER_SIZE) {
//						memmove(pBuffer, pBuffer + size, index - size);
//					}
//					index -= size;
//					// TODO 对于文件夹信息获取有问题
//					pack.hEvent = head.hEvent;
//					it->second.push_back(pack);
//					// SetEvent(head.hEvent);
//				}
//				TRACE(" index2 = %d \r\n", index);
//			}
//			else
//			{
//				TRACE(" !!!!!!!!!!! \r\n", index);
//				break;
//			}
//		} while (itclosed->second == false || index > 0);
//		SetEvent(head.hEvent);
//		m_lock.lock();
//		if (itclosed!=m_mapAutoClosed.end()) m_mapAutoClosed.erase(itclosed);
//		m_lstSend.pop_front();
//		m_lock.unlock();
//		CloseClient();
//	}
//	Sleep(1);
//	m_hThread = INVALID_HANDLE_VALUE;
//}

void CClientSocket::threadFunc2()
{
	MSG msg;
	while (::GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (m_mapFunc.find(msg.message)!= m_mapFunc.end())
		{
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{

	// TODO 定义一个消息的数据结构 回调消息的数据结构  数据 数据长度 模式
	// 回调消息的 数据结果
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	if (Initsocket() == true)
	{
		int ret = send(m_sock, (char*)data.strData.c_str(), (int)data.strData.size(), 0);
		if (ret>0)
		{
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while (m_sock != INVALID_SOCKET)
			{
				int length = recv(m_sock,pBuffer+index,BUFFER_SIZE-index,0);
				if (length>0 || index>0)
				{
					index += (size_t)length;
					size_t nlen = index;
					CPacket pack((BYTE*)pBuffer, nlen);
					if (nlen>0)
					{
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack), data.wParam);
						if (data.nMode & CSM_AUTOCLOSE)
						{
							CloseClient();
							return;
						}
					}
					memmove(pBuffer, pBuffer + nlen, index - nlen);
					index -= nlen;
				}
				else
				{
					// TODO 异常处理 或对方关闭套接字
					CloseClient();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, 1);
				}
			}
		
		}
		else
		{
			CloseClient(); // 网络终止处理
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else
	{
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -2);
	}
}

bool CClientSocket::Send(const char* pdata, int size)
{
	if (m_sock == -1) return false;
	return send(m_sock, pdata, size, 0) > 0;
}

bool CClientSocket::Send(const CPacket& pack)
{
	if (m_sock == -1) return false;
	std::string strOut;
	pack.getData(strOut);
	return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
}
