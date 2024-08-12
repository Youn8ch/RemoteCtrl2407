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
#pragma comment(lib, "Ws2_32.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;


enum IOCP_LIST
{
    IocpListEmpty,
    IocpListPush,
    IocpListPop
};

typedef struct IOCP_Param
{
    int nOperator;
    std::string strData;
    _beginthread_proc_type cbFunc;
    IOCP_Param(int op, const char* data, _beginthread_proc_type cb = NULL) {
        nOperator = op;
        strData = data;
        cbFunc = cb;
    }
    IOCP_Param() {
        nOperator = -1;
    }
}IOCP_PARAM;

void threadQueueEntry(HANDLE hIOCP)
{
    std::list<std::string> lstString;
    DWORD dwTransfer = 0;
    ULONG_PTR CompKey = 0;
    OVERLAPPED* pOverlap = NULL;
    while (GetQueuedCompletionStatus(hIOCP
        , &dwTransfer, &CompKey, &pOverlap, INFINITE)) 
    {
        if (dwTransfer ==0 || CompKey==NULL)
        {
            printf("thread to close\r\n");
        }
        IOCP_PARAM* pParam = (IOCP_PARAM*)CompKey;
        switch (pParam->nOperator)
        {
        case IocpListPush:
        {
            lstString.push_back(pParam->strData);
            break;
        }
        case IocpListPop:
        {
            std::string* pStr = NULL;
            if (lstString.size()>0)
            {
                pStr = new std::string(lstString.front());
                lstString.pop_front();
            }
            if (pParam->cbFunc)
            {
                pParam->cbFunc(pStr);
            }
        }
        default:
        {
            lstString.clear();
            break;
        }
        }
        delete pParam;
    }
    _endthread();
}

void func(void* arg) {
    std::string* pStr = (std::string*)arg;
    if (pStr != NULL)
    {
        printf("pop -> %s \r\n", arg);
        delete pStr;
    }
    else
    {
        printf("ERROR list is empty \r\n");
    }
}

int main()
{

    if (!CTool::Init()) return 1;
    HANDLE hIOCP = INVALID_HANDLE_VALUE;  // IO Completion Port
    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,1);
    // epoll 区别1 IOCP可以多线程
    HANDLE hThread = (HANDLE)_beginthread(threadQueueEntry, 0, hIOCP);
    
    ULONGLONG tick = GetTickCount64();
    while (_kbhit()!=0)
    {
        if ((GetTickCount64() - tick) > 1300)
        {
            PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)new IOCP_PARAM(IocpListPop, "666", func), NULL);
        }
        if ((GetTickCount64()-tick)>2000)
        {
            PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)new IOCP_PARAM(IocpListPush,"666", func), NULL);
        }
        tick = GetTickCount64();
        Sleep(1);
    }

    if (hIOCP != NULL)
    {
        PostQueuedCompletionStatus(hIOCP, 0, NULL, NULL);
        WaitForSingleObject(hIOCP,INFINITE);
    }
    printf("123123\r\n");
    ::exit(0);


    /*if (CTool::isAdmin())
    {
        if (!CTool::Init()) return 1;
        OutputDebugString(L" run as admin \r\n");
        CCommand cmd;
        int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
        switch (ret)
        {
        case -1:
            LOGE(">server socket init failed<"); exit(0);
            break;
        case -2:
            LOGE(">failed conn 3<"); exit(0);
            break;
        default:
            break;
        }
    }
    else
    {
        if (CTool::RunAsAdmin()==false) CTool::ShowError();
    }
    return 0;*/
}
