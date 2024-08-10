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

void ShowError() {
    LPWSTR lpMessageBuf = NULL;
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_CHINESE_SIMPLIFIED),
        (LPWSTR)&lpMessageBuf, 0, NULL);
    OutputDebugString(lpMessageBuf);
    LocalFree(lpMessageBuf);
    ::exit(0);
}

bool isAdmin() {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
    {
        ShowError();
        return false;
    }
    TOKEN_ELEVATION eve;
    DWORD len = 0;
    if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == false) {
        ShowError();
        return false;
    }
    CloseHandle(hToken);
    if (len == sizeof(eve))
    {
        return eve.TokenIsElevated;
    }
    printf(" length of tokenInfo is %d\r\n", len);
    return false;
}

void RunAsAdmin() {
    HANDLE hToken = NULL;
    BOOL ret = LogonUser(L"Administrator", NULL, NULL, LOGON32_LOGON_BATCH,
        LOGON32_PROVIDER_DEFAULT, &hToken);
    if (!ret)
    {
        ShowError();
        ::exit(0);
    }
    OutputDebugString(L"Logon administrator success!\r\n");
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    TCHAR sPath[MAX_PATH] = _T("");
    GetCurrentDirectory(MAX_PATH, sPath);
    CString strCmd = sPath;
    strCmd += _T("\\RemoteCtrl2407.exe");
    ret = CreateProcessWithTokenW(hToken, LOGON_WITH_PROFILE, NULL, (LPWSTR)(LPCWSTR)strCmd,
        CREATE_UNICODE_ENVIRONMENT,NULL,NULL,&si,&pi);
    CloseHandle(hToken);
    if (!ret)
    {
        ShowError();
        MessageBox(NULL,_T("进程创建失败"),_T("ERROR"),0);
        ::exit(0);
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main()
{
    if (isAdmin())
    {
        OutputDebugString(L" run as admin \r\n");
    }

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
			int ret = pserver->Run(&CCommand::RunCommand, &cmd);
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
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
