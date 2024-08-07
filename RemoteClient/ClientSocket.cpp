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

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();

}

void CClientSocket::threadFunc()
{
	
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	int len = 0;
	size_t size = 0;
	while (m_lstSend.size() > 0)
	{
		if (Initsocket() == false)
		{
			return;
		}
		m_lock.lock();
		CPacket& head = m_lstSend.front();
		m_lock.unlock();
		if (Send(head) == false)
		{
			TRACE(_T("发送失败\r\n"));
			Initsocket();
			break;
		}
		std::map<HANDLE, std::list<CPacket>&>::iterator it;
		it = m_mapAck.find(head.hEvent);
		std::map<HANDLE, bool>::iterator itclosed;
		itclosed = m_mapAutoClosed.find(head.hEvent);
		if (it == m_mapAck.end() || itclosed == m_mapAutoClosed.end()) {
			TRACE(" ACK ERROR \r\n");
			break;
		}
		do
		{
			len = recv(m_sock, pBuffer+index, BUFFER_SIZE - index, 0);
			if (len > 0 || index > 0)
			{
				TRACE(" len = %d \r\n", len);
				index += len;
				size = index;
				CPacket pack((BYTE*)pBuffer, size);
				TRACE(" index1 = %d \r\n", index);
				if (size > 0)
				{
					if (index < BUFFER_SIZE) {
						memmove(pBuffer, pBuffer + size, index - size);
					}
					index -= size;
					// TODO 对于文件夹信息获取有问题
					pack.hEvent = head.hEvent;
					it->second.push_back(pack);
					// SetEvent(head.hEvent);
				}
				TRACE(" index2 = %d \r\n", index);
			}
			else
			{
				TRACE(" !!!!!!!!!!! \r\n", index);
				break;
			}
		} while (itclosed->second == false || index > 0);
		SetEvent(head.hEvent);
		m_lock.lock();
		if (itclosed!=m_mapAutoClosed.end()) m_mapAutoClosed.erase(itclosed);
		m_lstSend.pop_front();
		m_lock.unlock();
		CloseClient();
	}
	Sleep(1);
	m_hThread = INVALID_HANDLE_VALUE;
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
