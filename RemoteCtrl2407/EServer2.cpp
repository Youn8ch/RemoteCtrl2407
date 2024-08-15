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
    if (*(LPDWORD)*m_client.get() > 0)
    {
        GetAcceptExSockaddrs(*m_client, 0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
            (sockaddr**)m_client->GetLocalAddr(), &lLength,
            (sockaddr**)m_client->GetRemoteAddr(), &rLength);

        int ret = WSARecv((SOCKET)*m_client,
            m_client->RecvWSAbuffer(), 1,
            *m_client, &m_client->flags(), *m_client, NULL);

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
int RecvOverlapped<op>::RecvWorker()
{
    return 0;
}

template <EOperator op>
int SendOverlapped<op>::SendWorker()
{
    return 0;
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
    m_send(new SENDOVERLAPPED())
{
    m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    m_buffer.resize(1024);
    memset(&m_laddr, 0, sizeof(m_laddr));
    memset(&m_raddr, 0, sizeof(m_raddr));
}

void EClient::SetOverlapped(PCLIENT& ptr)
{
    m_overlapped->m_client = ptr;
    m_recv->m_client = ptr;
    m_send->m_client = ptr;
}

EClient::operator LPOVERLAPPED()
{
    return &m_overlapped->m_overlapped;
}

LPWSABUF EClient::RecvWSAbuffer()
{
    return &m_recv->m_wsabuffer;
}

LPWSABUF EClient::SendWSAbuffer()
{
    return &m_send->m_wsabuffer;
}

int EServer::threadIocp() {

    DWORD transfer = 0;
    ULONG_PTR compKey = 0;
    OVERLAPPED* lpOver = NULL;
    if (GetQueuedCompletionStatus(m_hIOCP, &transfer, &compKey, &lpOver, INFINITE)) {
        EOverlapped* pO = CONTAINING_RECORD(lpOver, EOverlapped, m_overlapped);
        if (transfer > 0 && compKey != 0)
        {
            EOverlapped* pO = CONTAINING_RECORD(lpOver, EOverlapped, m_overlapped);
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
    return -2;

}

