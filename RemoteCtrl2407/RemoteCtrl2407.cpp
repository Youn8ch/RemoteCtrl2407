// RemoteCtrl2407.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl2407.h"

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"
#include "Command.h"
#include <conio.h>
#include "Queue.h"
#include "MSWSock.h"
#include "EServer2.h"
#pragma comment(lib, "Ws2_32.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

class COverlapped
{
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;
    char m_buffer[4096];
    COverlapped() {
        m_operator = 0;
        memset(&m_overlapped, 0, sizeof(OVERLAPPED));
        memset(&m_buffer, 0, sizeof(m_buffer));
    }

};

void iocp() {
    EServer server;
    server.StartServer();
    getchar();












    //SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // TCP
    //sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    //if (sock == INVALID_SOCKET)
    //{
    //    CTool::ShowError();
    //    return;
    //}
    //HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, sock, 4);
    //SOCKET client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    //CreateIoCompletionPort((HANDLE)sock, hIOCP, 0, 0);

    //sockaddr_in addr;
    //addr.sin_family = PF_INET;
    //addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    //addr.sin_port = htons(8554);
    //bind(sock, (sockaddr*)&addr, sizeof(addr));
    //listen(sock, 5);

    //COverlapped overlapped;
    //overlapped.m_operator = 1;
    //memset(&overlapped, 0, sizeof(COverlapped));
    //DWORD received = 0;
    //if (AcceptEx(sock, client, overlapped.m_buffer, 0,
    //    sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &received, &overlapped.m_overlapped) == FALSE) {
    //    CTool::ShowError();
    //}



    //while (true)
    //{
    //    LPOVERLAPPED pOverlapped = NULL;
    //    DWORD transferred = 0;
    //    unsigned __int64 key = 0;
    //    if (GetQueuedCompletionStatus(hIOCP, &transferred, &key, &pOverlapped, INFINITY)) {
    //        COverlapped* p0 = CONTAINING_RECORD(pOverlapped, COverlapped, m_overlapped);
    //        switch (p0->m_operator)
    //        {
    //        default:
    //            break;
    //        }
    //    }
    //}

 


}

int main()
{
    if (!CTool::Init()) return 1;
    iocp();


    //if (!CTool::Init()) return 1;
    //OutputDebugString(L" run as admin \r\n");
    //CCommand cmd;
    //int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
    //switch (ret)
    //{
    //case -1:
    //    LOGE(">server socket init failed<"); exit(0);
    //    break;
    //case -2:
    //    LOGE(">failed conn 3<"); exit(0);
    //    break;
    //default:
    //    break;
    //}


    return 0;
}
