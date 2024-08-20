#include "pch.h"
#include"EServer2.h"
#include "Tool.h"

template <EOperator op>
AcceptOverlapped<op>::AcceptOverlapped()
{
    m_operator = EAccept;
    m_worker = ThreadWorker(this, (FUNCTYPE)&AcceptOverlapped<op>::AcceptWorker);
    m_server = NULL;
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024);
}

template <EOperator op>
int AcceptOverlapped<op>::AcceptWorker()
{
    INT lLength = 0, rLength = 0;
    if (m_client->GetBufferSize() > 0)
    {
        LPSOCKADDR pLocalAddr, pRemoteAddr;
        GetAcceptExSockaddrs(*m_client, 0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
            (sockaddr**)&pLocalAddr, &lLength,
            (sockaddr**)&pRemoteAddr, &rLength);

        memcpy(m_client->GetLocalAddr(), pLocalAddr, sizeof(sockaddr_in));
        memcpy(m_client->GetRemoteAddr(), pRemoteAddr, sizeof(sockaddr_in));
        m_server->BindNewSocket(*m_client, (ULONG_PTR)m_client);

        int ret = WSARecv((SOCKET)*m_client,
            m_client->RecvWSAbuffer(), 1,
            *m_client, &m_client->flags(), m_client->RecvOverlapped(), NULL);

        if (ret == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING))
        {
            // TODO 
        }

        if (!m_server->NewAccept())
        {
            return -2;
        }
    }
    return -1;

}





template <EOperator op>
RecvOverlapped<op>::RecvOverlapped()
{
    m_operator = op;
    m_worker = ThreadWorker(this, (FUNCTYPE)&RecvOverlapped<op>::RecvWorker);
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024 * 256);
}

template <EOperator op>
SendOverlapped<op>::SendOverlapped()
{
    m_operator = op;
    m_worker = ThreadWorker(this, (FUNCTYPE)&SendOverlapped<op>::SendWorker);
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024 * 256);
}


EClient::EClient() :
    isBusy(false),
    m_flags(0),
    m_overlapped(new ACCEPTOVERLAPPED()),
    m_recv(new RECVOVERLAPPED()),
    m_send(new SENDOVERLAPPED()),
    m_vecSend(this,(SENDCALLBACK)& EClient::SendData)
{
    m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    m_buffer.resize(1024);
    memset(&m_laddr, 0, sizeof(m_laddr));
    memset(&m_raddr, 0, sizeof(m_raddr));
}



void EClient::SetOverlapped(EClient* ptr)
{
    m_overlapped->m_client = ptr;
    m_recv->m_client = ptr;
    m_send->m_client = ptr;
}

EClient::operator LPOVERLAPPED()
{
    return &m_overlapped->m_overlapped;
}

int EClient::Recv()
{
    int ret = recv(m_sock, m_buffer.data(), m_buffer.size(), 0);
    if (ret <= 0) return -1;
    m_used += (size_t)ret;
    // ½âÎöÊý¾Ý
    return 0;
}

int EClient::Send(void* buffer, size_t nSize)
{
    std::vector<char> data(nSize);
    memcpy(data.data(), buffer, nSize);
    return (m_vecSend.Pushback(data)) ? 0 : -1;
    return 0;
}

int EClient::SendData(std::vector<char>& data)
{
    if (m_vecSend.Size()>0)
    {
        int ret = WSASend(m_sock, SendWSAbuffer(), 1, &m_received, m_flags, &m_send->m_overlapped, NULL);
        if (ret!=0 && WSAGetLastError()!=WSA_IO_PENDING)
        {
            CTool::ShowError();
            return -1;
        }
    }
    return 0;
}

LPWSABUF EClient::RecvWSAbuffer()
{
    return &m_recv->m_wsabuffer;
}

LPOVERLAPPED EClient::RecvOverlapped()
{
    return &m_recv->m_overlapped;
}

LPWSABUF EClient::SendWSAbuffer()
{
    return &m_send->m_wsabuffer;
}

LPOVERLAPPED EClient::SendOverlapped()
{
    return &m_send->m_overlapped;
}

int EServer::threadIocp() {

    DWORD transfer = 0;
    ULONG_PTR compKey = 0;
    OVERLAPPED* lpOver = NULL;
    if (GetQueuedCompletionStatus(m_hIOCP, &transfer, &compKey, &lpOver, INFINITE)) {
        EOverlapped* pO = CONTAINING_RECORD(lpOver, EOverlapped, m_overlapped);
        if ( compKey != 0)
        {
            EOverlapped* pO = CONTAINING_RECORD(lpOver, EOverlapped, m_overlapped);
            LOGI("pO->m_operator = %d", pO->m_operator);
            pO->m_server = this;
            switch (pO->m_operator)
            {
            case EAccept:
            {
                ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pO;
                m_pool.DispatchWorker(pOver->m_worker);
                break;
            }
            case ERecv:
            {
                RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)pO;
                m_pool.DispatchWorker(pOver->m_worker);
                break;
            }
            case ESend:
            {

                SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)pO;
                m_pool.DispatchWorker(pOver->m_worker);
                break;
            }
            //case EError:
            //{
            //    ERROROVERLAPPED* pOver = (ERROROVERLAPPED*)pO;
            //    m_pool.DispatchWorker(pOver->m_worker);
            //    break;
            //}
            default:
                LOGE("123");
                break;
            }
        }
        else
        {
            return -1;
        }

    }
    return 0;

}

