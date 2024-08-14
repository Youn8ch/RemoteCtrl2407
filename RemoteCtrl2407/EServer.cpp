#include "pch.h"
#include"EServer.h"

template <EOperator op>
AcceptOverlapped<op>::AcceptOverlapped()
{
    m_operator = EAccept;
    m_worker = ThreadWorker(this, (FUNCTYPE) &AcceptOverlapped<op>::AcceptWorker);
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
            (sockaddr**)&m_client->m_laddr, &lLength,
            (sockaddr**)&m_client->m_raddr, &rLength);
        if (!m_server->NewAccept())
        {
            return -2;
        }
    }
    return -1;

}

EClient::EClient() :isBusy(false) {
    m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    m_buffer.resize(1024);
    memset(&m_laddr, 0, sizeof(m_laddr));
    memset(&m_raddr, 0, sizeof(m_raddr));
}