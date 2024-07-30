#pragma once
#include <map>
#include "ServerSocket.h"
#include <atlimage.h>
#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <list>
#include "LockInfoDialog.h"
#include "pch.h"
#include "framework.h"
#include "Resource.h"
#include "Log.h"
class CCommand
{

public:
	CCommand();
	~CCommand() {};
	int ExcuteCommand(int nCmd, std::list<CPacket>& listPackets,CPacket& inPacket);
	static void RunCommand(void* arg, int status, std::list<CPacket>& listPackets, CPacket& inPacket) {
		CCommand* thiz = (CCommand*)arg;
		if (status > 0) {
			int ret = thiz->ExcuteCommand(status, listPackets, inPacket);
			if (ret != 0) {
				LOGE(">Cmd Excute failed, ret = %d, cmd = %d", ret, status);
			}
		}
		else
		{
			LOGE("FAIL TO CONN");
		}
	};
protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>& listPackets, CPacket& inPacket); // 成员函数指针
	std::map<int, CMDFUNC> m_mapFunction; // 从命令号到功能的印射
	CLockInfoDialog dlg;
	unsigned threadid;

protected:
	static unsigned __stdcall threadLockDlg(void* arg) {
		CCommand* thiz = (CCommand*)arg;
		thiz->threadLockDlgMain();
		_endthreadex(0);
		return 0;
	}

	void threadLockDlgMain() {

		LOGI("> Created id = %d <", GetCurrentThreadId());

		dlg.Create(IDD_DIALOG_INFO, NULL);
		dlg.ShowWindow(SW_SHOW);

		CRect rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
		rect.bottom = LONG(rect.bottom * 1.1);
		dlg.MoveWindow(rect);
		CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
		if (pText)
		{
			CRect rtText;
			pText->GetWindowRect(rtText);
			int nWidth = (rect.right - rtText.Width()) / 2;
			int nHeight = (rect.bottom - rtText.Height()) / 2;
			pText->MoveWindow(nWidth, nHeight, rtText.Width(), rtText.Height());
		}
		dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		ShowCursor(false); // 限制鼠标功能
		dlg.GetWindowRect(rect);
		ClipCursor(rect); // 限制鼠标功能
		::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);

		rect.left = 0;
		rect.top = 0;
		rect.right = 1;
		rect.bottom = 1;
		ClipCursor(rect);

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
		ClipCursor(NULL);
		// 恢复鼠标
		ShowCursor(true);
		// 恢复任务栏
		::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
		dlg.DestroyWindow();
	}

protected:
	int MakeDriverInfo(std::list<CPacket>& listPackets, CPacket& inPacket) {
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
		listPackets.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
		return 0;
	}


	int MakeDirectoryInfo(std::list<CPacket>& listPackets, CPacket& inPacket) {
		std::string path = inPacket.strData;
		// std::list<FILEINFO> ListFileInfo;

		if (path.length() == 2 && path[1] == ':') {
			path.append("\\");
		}

		if (_chdir(path.c_str()) != 0)
		{
			FILEINFO finfo;
			finfo.HasNext = FALSE;
			listPackets.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			LOGE("> No permission to access the directory <");
			return -2;
		}
		//_getcwd(currentPath, FILENAME_MAX);
		//LOGI("Current working directory: %s", currentPath);

		_finddata_t fdata;
		long long hfind = 0;
		if ((hfind = _findfirst("*", &fdata)) == -1) {
			LOGE("> No file being the path <");
			FILEINFO finfo;
			finfo.HasNext = FALSE;
			listPackets.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			return -3;
		}

		do
		{
			FILEINFO finfo;
			finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
			memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
			// ListFileInfo.push_back(finfo);
			LOGI("FILE[%s]", finfo.szFileName);
			listPackets.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
		} while (!_findnext(hfind, &fdata));
		FILEINFO finfo;
		finfo.HasNext = FALSE;
		listPackets.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
		return 0;
	}

	int RunFile(std::list<CPacket>& listPackets, CPacket& inPacket) {
		std::string path = inPacket.strData;
		ShellExecuteA(NULL, NULL, path.c_str(), NULL, NULL, SW_SHOWNORMAL);
		listPackets.push_back(CPacket(3, NULL, 0));
		return 0;
	}

	int DownloadFile(std::list<CPacket>& listPackets, CPacket& inPacket) {
		std::string path = inPacket.strData;
		long long data = 0;
		FILE* pFile = NULL;
		errno_t err = fopen_s(&pFile, path.c_str(), "rb");
		if (pFile == NULL || err != 0)
		{
			listPackets.push_back(CPacket(4, (BYTE*)&data, 8));
			return -1;
		}
		fseek(pFile, 0, SEEK_END);
		data = _ftelli64(pFile);
		listPackets.push_back(CPacket(4, (BYTE*)&data, 8));
		fseek(pFile, 0, SEEK_SET);
		char buffer[1024] = "";
		size_t rlen = 0;
		do {
			rlen = fread(buffer, 1, 1024, pFile);
			listPackets.push_back(CPacket(4, (BYTE*)buffer, rlen));
		} while (rlen >= 1024);
		listPackets.push_back(CPacket(4, NULL, 0));
		fclose(pFile);
		return 0;
	}


	int MouseEvent(std::list<CPacket>& listPackets, CPacket& inPacket) {
		MOUSEEVENT mouse;
		memcpy(&mouse, inPacket.strData.c_str(), sizeof(MOUSEEVENT));
		
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
		};
		listPackets.push_back(CPacket(5, NULL, 0));
		
		return 0;
	}

	int SendScreen(std::list<CPacket>& listPackets, CPacket& inPacket) {
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
			listPackets.push_back(CPacket(6, pData, nSize));
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




	int LockMachine(std::list<CPacket>& listPackets, CPacket& inPacket) {
		if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE)
		{
			// _beginthread(threadLockDlg, 0, NULL);
			_beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);
			LOGI("> threadid = %d <", threadid);
		}
		listPackets.push_back(CPacket(7, NULL, 0));
		return 0;
	}

	int UnLockMachine(std::list<CPacket>& listPackets, CPacket& inPacket) {

		PostThreadMessage(threadid, WM_KEYDOWN, 0x0000001b, 0);
		listPackets.push_back(CPacket(8, NULL, 0));
		// ::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x0000001b, 0x00010001);
		return 0;
	}

	int DeleteLocalFile(std::list<CPacket>& listPackets, CPacket& inPacket) {
		std::string path = inPacket.strData;
		// TCHAR sPath[MAX_PATH] = _T("");
		// mbstowcs(sPath, path.c_str(), path.size());
		DeleteFileA(path.c_str());
		listPackets.push_back(CPacket(9, NULL, 0));
		return 0;
	}


	int TestConnect(std::list<CPacket>& listPackets, CPacket& inPacket) {
		LOGI("test conn 666");
		listPackets.push_back(CPacket(666, NULL, 0));
		return 0;
	}


};

