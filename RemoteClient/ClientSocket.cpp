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
	if (Initsocket() == false)
	{
		return;
	}
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	while (m_sock != INVALID_SOCKET)
	{
		if (m_lstSend.size()>0)
		{
			CPacket& head = m_lstSend.front();
			do
			{
				if (Send(head) == false)
				{
					TRACE(_T("发送失败\r\n"));
					break;
				}
				auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));
				int len = recv(m_sock, pBuffer, BUFFER_SIZE - index, 0);
				if (len > 0 || index > 0)
				{
					index += len;
					size_t size = index;
					CPacket pack((BYTE*)pBuffer, size);
					if (size >= 0)
					{
						// TODO 对于文件夹信息获取有问题
						pack.hEvent = head.hEvent;
						pr.first->second.push_back(pack);
						SetEvent(head.hEvent);
					}
					break;
				}
				else if (len <= 0 && index <= 0)
				{
					CloseClient();
				}
			} while (false);
			m_lstSend.pop_front();
			break;
		}
	}
	CloseClient();
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
