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
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <list>
#include <atlimage.h>
#pragma comment(lib, "Ws2_32.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;


void Dump(BYTE* pdata, size_t length) {
	std::string strout;
	for (size_t i = 0; i < length; i++)
	{
		char buf[8] = "";
		if (i > 0)
		{
			strout += " ";
		}
		snprintf(buf, sizeof(buf), "%02X", pdata[i] & 0xFF);
		// printf("pData[%d] in hex: %s\n", i, buf);
		strout += buf;
	}
	strout += "\n";
	std::cout << "Dump:\n" << strout << std::endl;
}

int MakeDriverInfo() {
	std::string result;
	for (int i = 1; i <= 26; i++) {
		if (_chdrive(i) == 0) {
			if (result.size() > 0)
			{
				result += ',';
			}
			result += ('A' + i - 1);
		}
	}
	// CServerSocket::getInstance()->Send(CPacket(1,(const BYTE*)result.c_str(),result.size()));
	CPacket pack(1, (const BYTE*)result.c_str(), result.size());
	CServerSocket::getInstance()->Send(pack);
	// Dump((BYTE*)pack.getData(), pack.getSize());
	return 0;
}


int MakeDirectoryInfo() {
	std::string path;
	// std::list<FILEINFO> ListFileInfo;
	if (CServerSocket::getInstance()->GetFilePath(path) == false)
	{
		LOGE("> Get file path failed <");
		return -1;
	}

	//LOGI("Path to switch: %s", path.c_str());
	//char currentPath[FILENAME_MAX];
	//_getcwd(currentPath, FILENAME_MAX);
	//LOGI("Current working directory: %s", currentPath);

	if (path.length() == 2 && path[1] == ':') {
		path.append("\\");
	}

	if (_chdir(path.c_str()) != 0)
	{
		FILEINFO finfo;
		finfo.HasNext = FALSE;
		// ListFileInfo.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
		LOGE("> No permission to access the directory <");
		return -2;
	}
	//_getcwd(currentPath, FILENAME_MAX);
	//LOGI("Current working directory: %s", currentPath);

	_finddata_t fdata;
	long long hfind = 0;
	if ((hfind = _findfirst("*", &fdata)) == -1) {
		LOGE("> No file being the path <");
		return -3;
	}

	do
	{
		FILEINFO finfo;
		finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		// ListFileInfo.push_back(finfo);
		LOGI("FILE[%s]", finfo.szFileName);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
	} while (!_findnext(hfind, &fdata));
	FILEINFO finfo;
	finfo.HasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	CServerSocket::getInstance()->Send(pack);
	return 0;
}

int RunFile() {
	std::string path;
	CServerSocket::getInstance()->GetFilePath(path);
	ShellExecuteA(NULL, NULL, path.c_str(), NULL, NULL, SW_SHOWNORMAL);
	CPacket pack(3, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;
}

int DownloadFile() {
	std::string path;
	CServerSocket::getInstance()->GetFilePath(path);
	long long data = 0;
	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, path.c_str(), "rb");
	if (pFile == NULL || err != 0)
	{
		CPacket pack(4, (BYTE*)&data, 8);
		CServerSocket::getInstance()->Send(pack);
		return -1;
	}
	fseek(pFile, 0, SEEK_END);
	data = _ftelli64(pFile);
	CPacket head(4, (BYTE*)&data, 8);
	CServerSocket::getInstance()->Send(head);
	fseek(pFile, 0, SEEK_SET);
	char buffer[1024] = "";
	size_t rlen = 0;
	do {
		rlen = fread(buffer, 1, 1024, pFile);
		CPacket pack(4, (BYTE*)buffer, rlen);
		CServerSocket::getInstance()->Send(pack);
	} while (rlen >= 1024);
	CPacket pack(4, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	fclose(pFile);
	return 0;
}


int MouseEvent() {
	MOUSEEVENT mouse;
	if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
		SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		DWORD nFlag = 0;
		switch (mouse.nButton)
		{
		case 0: // 左键
			nFlag = 1;
			break;
		case 1: // 右键
			nFlag = 2;
			break;
		case 2:	// 中键
			nFlag = 4;
			break;
		case 4: // 没有按键
			nFlag = 8;
			break;
		default:
			break;
		}
		switch (mouse.nAction)
		{
		case 0: // 单击
			nFlag |= 0x10;
			break;
		case 1: // 双击
			nFlag |= 0x20;
			break;
		case 2:	// 按下
			nFlag |= 0x40;
			break;
		case 3: // 放开
			nFlag |= 0x80;
			break;
		default:
			break;
		}
		switch (nFlag)
		{
			// 低位 nButton 高位 nAction
		case 0x11: // 左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x21: // 左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41: // 左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81: // 左键放开
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;

		case 0x12: // 右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22: // 右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42: // 右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82: // 右键放开
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;

		case 0x14: // 中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24: // 中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44: // 中键按下
			nFlag |= 0x40;
			break;
		case 0x84: // 中键放开
			nFlag |= 0x80;
			break;
		case 0x08: // 鼠标移动
			break;
		default:
			break;
		}
		CPacket pack(4, NULL, 0);
		CServerSocket::getInstance()->Send(pack);
	}
	else
	{
		LOGE("> mouse operation failed <");
		return -1;
	}
	return 0;
}

int SendScreen() {
	CImage screen;
	HDC hScreen = ::GetDC(NULL);
	int nBitPerpixel = GetDeviceCaps(hScreen, BITSPIXEL);
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeigth = GetDeviceCaps(hScreen, VERTRES);
	screen.Create(nWidth, nHeigth, nBitPerpixel);
	BitBlt(screen.GetDC(), 0, 0, nWidth, nHeigth, hScreen, 0, 0, SRCCOPY);
	ReleaseDC(NULL, hScreen);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
	if (hMem == NULL) return -1;
	IStream* pStream = NULL;
	HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
	if (ret == S_OK)
	{
		screen.Save(pStream, Gdiplus::ImageFormatPNG);
		LARGE_INTEGER bg = { 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);
		PBYTE pData = (PBYTE)GlobalLock(hMem);
		SIZE_T nSize = GlobalSize(hMem);
		CPacket pack(6, pData, nSize);
		CServerSocket::getInstance()->Send(pack);
		GlobalUnlock(hMem);
	}
	//screen.Save(pStream, Gdiplus::ImageFormatPNG);
    screen.Save(_T("test123.jpeg"), Gdiplus::ImageFormatJPEG);
	// LOGI("> save done! <");
	pStream->Release();
	GlobalFree(hMem);
	screen.ReleaseDC();
	return 0;
}

#include "LockInfoDialog.h"
CLockInfoDialog dlg;
unsigned threadid = 0;


unsigned __stdcall threadLockDlg(void* arg) {

	LOGI("> Created id = %d <", GetCurrentThreadId());

	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	ShowCursor(false); // 限制鼠标功能
	CRect rect;
	dlg.GetWindowRect(rect);
	ClipCursor(rect); // 限制鼠标功能
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
	MSG msg;
	// GetMessage 只能拿到跟这个线程相关的消息
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_KEYDOWN)
		{
			TRACE("msg:%08X wparam:%08x lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
			if (msg.wParam == 0x1b)
			{
				break;
			}
		}
	}
	ShowCursor(true);
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
	dlg.DestroyWindow();
	_endthreadex(0);
	return 0;
}

int LockMachine() {
	if (dlg.m_hWnd==NULL || dlg.m_hWnd==INVALID_HANDLE_VALUE)
	{
		// _beginthread(threadLockDlg, 0, NULL);
		_beginthreadex(NULL,0,threadLockDlg, NULL, 0, &threadid);
		LOGI("> threadid = %d <", threadid);
	}
	CPacket pack(7, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;
}

int UnLockMachine() {
	
	PostThreadMessage(threadid, WM_KEYDOWN, 0x0000001b, 0);

	// ::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x0000001b, 0x00010001);
	return 0;
}

int DeleteLocalFile() {
	std::string path;
	CServerSocket::getInstance()->GetFilePath(path);
	// TCHAR sPath[MAX_PATH] = _T("");
	// mbstowcs(sPath, path.c_str(), path.size());
	DeleteFileA(path.c_str());
	CPacket pack(9, NULL, 0);
	bool ret = CServerSocket::getInstance()->Send(pack);
	return 0;
}


int TestConnect() {
	LOGI("test conn 666");
	CPacket pack(666, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;
}


int ExcuteCommand(int nCmd) {
	int ret;
	switch (nCmd)
	{
	case 1:
		ret = MakeDriverInfo(); // 查看磁盘分区
		break;
	case 2:
		ret = MakeDirectoryInfo(); // 查看指定目录下的文件
		break;
	case 3:
		ret = RunFile(); // 打开文件
		break;
	case 4:
		ret = DownloadFile();
		break;
	case 5:
		ret = MouseEvent(); // 鼠标操作
		break;
	case 6:
		ret = SendScreen(); // 发送屏幕内容 -> 发送屏幕截图
		break;
	case 7:
		ret = LockMachine();
		Sleep(50);
		// LockMachine();
		break;
	case 8:
		ret = UnLockMachine();
		break;
	case 9:
		ret = DeleteLocalFile();
		break;
	case 666:
		ret = TestConnect();
		break;
	default:
		ret = -1;
		break;
	}
	return ret;
}


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
					ret = ExcuteCommand(pserver->GetPacket().sCmd);
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
