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
#pragma comment(lib, "Ws2_32.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;


int main()
{


    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
			CCommand cmd;
			CServerSocket* pserver = CServerSocket::getInstance();
			int count = 0;
			if (pserver->Initsocket()==false)
			{
				LOGE(">server socket init failed<"); exit(0);
			}
			LOGI(">server socket init done!<");
			while (CServerSocket::getInstance()!=NULL)
			{
				if (pserver->AcceptClient() == false) {
					if (count >= 3)
					{
						LOGE(">failed conn 3<"); exit(0);
					}
					LOGE(">failed conn Retry<");
					count++;
				}
				int ret = pserver->DealCommand();
				LOGI("Dealcommand ret = %d", ret);
				if (ret > 0)
				{
					ret = cmd.ExcuteCommand(pserver->GetPacket().sCmd);
					if (ret!=0)
					{
						LOGE(">Cmd Excute failed, ret = %d, cmd = %d",ret, pserver->GetPacket().sCmd);
					}
					pserver->CloseClient();
				}
			}
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
