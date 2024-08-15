#pragma once
#include "Thread.h"
#include <map>
#include "Log.h"
#include "Queue.h"
#include <MSWSock.h>

enum EOperator
{
    ENone,
    EAccept,
    ERecv,
    ESend,
    EError
};

class EServer;
class EClient;
typedef std::shared_ptr<EClient> PCLIENT;

class EOverlapped : public ThreadFuncBase
{
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;
    std::vector<char> m_buffer;
    ThreadWorker m_worker; // 处理函数
    EServer* m_server; // 服务器对象
    PCLIENT m_client; // 对应客户端
    WSABUF m_wsabuffer;
    EOverlapped() {
        m_operator = 0;
        memset(&m_overlapped, 0, sizeof(OVERLAPPED));
        memset(&m_buffer, 0, sizeof(m_buffer));
    }

};

template <EOperator>class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;
template <EOperator>class RecvOverlapped;
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;
template <EOperator>class SendOverlapped;
typedef SendOverlapped<ESend> SENDOVERLAPPED;

class EClient : public ThreadFuncBase
{
public:
    EClient();
    ~EClient() { closesocket(m_sock);
}

    void SetOverlapped(PCLIENT& ptr);

    operator SOCKET() {
        return m_sock;
    }
    operator PVOID() {
        return &m_buffer[0];
    }
    operator LPOVERLAPPED();
    operator LPDWORD() {
        return &m_received;
    }
    int Recv() {
        int ret = recv(m_sock, m_buffer.data(), m_buffer.size(), 0);
        if (ret <= 0) return -1;
        m_used += (size_t)ret;
        // 解析数据
        return 0;
    }
    size_t GetBufferSize() const {
        return m_buffer.size();
    }
    DWORD& flags() {
        return m_flags;
    }
    LPWSABUF RecvWSAbuffer();
    LPWSABUF SendWSAbuffer();
    sockaddr_in* GetLocalAddr() { return &m_laddr; }
    sockaddr_in* GetRemoteAddr() { return &m_raddr; }
public:
    SOCKET m_sock;
    DWORD m_received;
    DWORD m_flags;
    std::shared_ptr<ACCEPTOVERLAPPED> m_overlapped;
    std::shared_ptr<RECVOVERLAPPED> m_recv;
    std::shared_ptr<SENDOVERLAPPED> m_send;
    std::vector<char> m_buffer;
    size_t m_used; // 已经使用的缓存区大小
    sockaddr_in m_laddr;
    sockaddr_in m_raddr;
    bool isBusy;
};





// EAccept
template <EOperator>
class AcceptOverlapped :public EOverlapped
{
public:
    AcceptOverlapped();
    int AcceptWorker();
};

// ERecv
template <EOperator>
class RecvOverlapped :public EOverlapped
{
public:
    RecvOverlapped();
    int RecvWorker();
};


// ESend
template <EOperator>
class SendOverlapped :public EOverlapped
{
public:
    SendOverlapped();
    int SendWorker();

};

//template <EOperator>
//class ErrorOverlapped :public EOverlapped, ThreadFuncBase
//{
//public:
//    ErrorOverlapped() :
//        m_operator(EAccept),
//        m_worker(this, &ErrorOverlapped::ErrorWorker)
//    {
//        memset(&m_overlapped, 0, sizeof(m_overlapped));
//        m_buffer.resize(1024);
//    }
//    int ErrorWorker() {
//        // TODO
//    }
//private:
//
//};
//typedef ErrorOverlapped<EError> ERROROVERLAPPED;


// ============================================================================







// ============================================================================



class EServer :
    public ThreadFuncBase
{

public:



    EServer(const std::string& ip = "0.0.0.0", const short& port = 8554) : m_pool(10) {
        m_hIOCP = INVALID_HANDLE_VALUE;
        m_sock = INVALID_SOCKET;
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    bool StartServer() {
        CreateSocket();
        if (bind(m_sock, (sockaddr*)&m_addr, sizeof(m_addr)) == false) {
            LOGE("BIND ERROR");
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return false;
        }
        if (listen(m_sock, 3) == -1)
        {
            LOGE("listen ERROR");
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return false;
        }
        m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
        if (m_hIOCP == NULL || m_hIOCP == INVALID_HANDLE_VALUE)
        {
            LOGE("m_hIOCP ERROR");
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            m_hIOCP = INVALID_HANDLE_VALUE;
            return false;
        }
        CreateIoCompletionPort((HANDLE)m_sock, m_hIOCP, (ULONG_PTR)this, 0);
        m_pool.Invoke();
        m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&EServer::threadIocp));
        return NewAccept();
    }

    ~EServer() {

    }
public:
    void CreateSocket() {
        m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        int opt = 1;
        setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    }

    bool NewAccept() {
        PCLIENT pClient(new EClient());
        pClient->SetOverlapped(pClient);
        m_client.insert(std::pair <SOCKET, PCLIENT>(*pClient, pClient));

        if (!AcceptEx(m_sock, *pClient, *pClient, 0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, *pClient, *pClient))
        {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            m_hIOCP = INVALID_HANDLE_VALUE;
            return false;
        }
        return true;
    }
    int threadIocp();
public:
    ThreadPool m_pool;
    HANDLE m_hIOCP;
    SOCKET m_sock;
    std::map<SOCKET, PCLIENT> m_client;
    CQueue<EClient> m_lstClient;
    sockaddr_in m_addr;
};

