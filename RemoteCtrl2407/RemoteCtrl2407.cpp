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
    IOCP_Param(int op, const char* data = "", _beginthread_proc_type cb = NULL) {
        nOperator = op;
        strData = data;
        cbFunc = cb;
    }
    IOCP_Param() {
        nOperator = -1;
    }
}IOCP_PARAM,*pIOCP_PARAM;



void threadmain(HANDLE hIOCP) {
    std::list<std::string> lstString;
    DWORD dwTransfer = 0;
    ULONG_PTR CompKey = 0;
    OVERLAPPED* pOverlap = NULL;
    int count = 0, count0 = 0;
    while (GetQueuedCompletionStatus(hIOCP
        , &dwTransfer, &CompKey, &pOverlap, INFINITE))
    {
        if (dwTransfer == 0 || CompKey == NULL)
        {
            printf("thread to close\r\n");
            break;
        }
        IOCP_PARAM* pParam = (IOCP_PARAM*)CompKey;
        switch (pParam->nOperator)
        {
        case IocpListPush:
        {
            lstString.push_back(pParam->strData);
            count0++;
            break;
        }
        case IocpListPop:
        {
            std::string* pStr = NULL;
            if (lstString.size() > 0)
            {
                pStr = new std::string(lstString.front());
                lstString.pop_front();
            }
            if (pParam->cbFunc)
            {
                pParam->cbFunc(pStr);
            }
            count++;
            break;
        }
        default:
        {
            printf(" T!!!!!!!!!!!! ");
            lstString.clear();
            break;
        }
        }
        delete pParam;

    }
    printf(" Thread count = %d , count0 = %d \r\n", count, count0);
    Sleep(1000);
}


void threadQueueEntry(HANDLE hIOCP)
{
    threadmain(hIOCP);
    _endthread();
}

void func(void* arg) {
    std::string* pStr = (std::string*)arg;
    if (pStr != NULL)
    {
        printf("pop -> %s \r\n", pStr->c_str());
        delete pStr;
    }
    else
    {
        printf("ERROR list is empty \r\n");
    }
}



void test() {
    CQueue<std::string> lstStrings;
    ULONGLONG tick = GetTickCount64();
    ULONGLONG tick0 = GetTickCount64();
    ULONGLONG total = GetTickCount64();
    while (GetTickCount64() - total <= 1000)
    {
        if ((GetTickCount64() - tick0) > 13)
        {
            lstStrings.Pushback("666");
            tick0 = GetTickCount64();
        }
        if ((GetTickCount64() - tick) > 20)
        {
            std::string str;
            lstStrings.Popfront(str);
            tick = GetTickCount64();
            printf(" -> %s \r\n", str.c_str());
        }
        // printf(" str size = %s \r\n", lstStrings.Size());
        Sleep(1);
    }
    printf(" str size = %d \r\n", lstStrings.Size());
    lstStrings.Clear();
    printf(" closed str size = %d \r\n", lstStrings.Size());
}

int main()
{

    if (!CTool::Init()) return 1;


    //printf("press\r\n");
    for (size_t i = 0; i < 5; i++)
    {
        test();
    }
    
    // ::exit(0);


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
