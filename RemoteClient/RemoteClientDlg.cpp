﻿
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "ClientSocket.h"
#include "WatchDialog.h"


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
	, m_Port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT2, m_Port);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILEINFO, m_List);
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BUTTON_FILE, &CRemoteClientDlg::OnBnClickedButtonFile)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILEINFO, &CRemoteClientDlg::OnNMRClickListFileinfo)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	ON_MESSAGE(WM_SEND_PACKET, &CRemoteClientDlg::OnSendPacket)
	ON_BN_CLICKED(IDC_BTN_STARTWATCH, &CRemoteClientDlg::OnBnClickedBtnStartwatch)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	// m_server_address = 0x7F000001;
	m_server_address = 0xC0A83865; // 192.168.56.101
	m_Port = _T("8554");
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DLG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);
	m_imgfull = false;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteClientDlg::OnBnClickedBtnTest()
{

	SendCommandPacket(666);

}


void CRemoteClientDlg::OnBnClickedButtonFile()
{
	int ret = SendCommandPacket(1);
	if (ret < 0)
	{
		AfxMessageBox(_T("ButtonFile命令处理失败"));
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	m_Tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',') {
			dr += ":";
			HTREEITEM htemp = m_Tree.InsertItem(dr.c_str(),TVI_ROOT,TVI_LAST);
			m_Tree.InsertItem("", htemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	dr += ":";
	HTREEITEM htemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
	m_Tree.InsertItem("", htemp, TVI_LAST);

}

int CRemoteClientDlg::SendCommandPacket(int nCmd, bool bAutoclose, BYTE* pData, size_t nLength)
{
	TRACE(">Try connect<");
	UpdateData(); // 不是一个主线程调用的话 会崩溃 用msg
	CClientSocket* pClient = CClientSocket::getInstance();
	bool ret = pClient->Initsocket(m_server_address, atoi((LPCTSTR)m_Port)); // TODO 返回值
	if (!ret)
	{
		TRACE(">NET init failed<");
		AfxMessageBox((LPCTSTR)"网络初始化失败");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	ret = pClient->Send(pack);
	TRACE("send ? = %d\r\n", ret);
	int cmd = pClient->DealCommand();
	TRACE("ACK cmd = %d\r\n", cmd);
	if (bAutoclose) pClient->CloseClient();
	return cmd;
}

void CRemoteClientDlg::threadEntryForWatchData(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadWatchData();
	_endthread();
}

void CRemoteClientDlg::threadWatchData()
{
	Sleep(50);
	CClientSocket* pClient = NULL;
	do
	{
		pClient = CClientSocket::getInstance();
	} while (pClient==NULL);
	while (!m_isClosed)
	{
		if (m_imgfull == false) {
			int ret = SendMessage(WM_SEND_PACKET, 6 << 1 | 0);
			if (ret == 6)
			{
				BYTE* pData = (BYTE*)pClient->GetPacket().strData.c_str();
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
				if (hMem == NULL)
				{
					TRACE("no memory !");
					Sleep(1);
					continue;
				}
				IStream* pStream = NULL;
				HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
				if (hRet == S_OK)
				{
					// std::lock_guard<std::mutex> lock(m_imageMutex);
					ULONG length = 0;
					pStream->Write(pData, pClient->GetPacket().strData.size(), &length);
					LARGE_INTEGER bg = { 0 };
					pStream->Seek(bg, STREAM_SEEK_SET, NULL);
					if (m_image.IsDIBSection())
					{
						m_image.Destroy();
					}
					// if ((HBITMAP)m_image!=NULL) m_image.Destroy();

					m_image.Load(pStream);
					m_imgfull = true;
				}
			}
			else
			{
				Sleep(1);
			}
		}
		else
		{
			Sleep(1);
		}

	}

}

void CRemoteClientDlg::threadEntryForDownFile(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadDownFile();
	_endthread();
}

void CRemoteClientDlg::threadDownFile()
{
	// TODO: 在此添加命令处理程序代码
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	CFileDialog dlg(FALSE, "*", strFile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() == IDOK) {

		FILE* pFile = fopen(dlg.GetPathName(), "wb+");

		if (pFile == NULL)
		{
			AfxMessageBox("没有权限保存 或 文件无法创建");
			m_dlgStatus.ShowWindow(SW_HIDE);
			EndWaitCursor();
			return;
		}
		CClientSocket* pClient = CClientSocket::getInstance();
		HTREEITEM hSelected = m_Tree.GetSelectedItem();
		strFile = GetTreePath(hSelected) + "\\" + strFile;
		TRACE("%s\r\n", LPCSTR(strFile));
		do {
			int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)strFile);
			// int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
			if (ret < 0)
			{
				AfxMessageBox("执行下载文件失败");
				TRACE("执行下载失败: ret = %d\r\n", ret);
				break;
			}
			long long nlength = *(long long*)CClientSocket::getInstance()->GetPacket().strData.c_str();
			if (nlength == 0)
			{
				AfxMessageBox("文件长度为0 或无法读取");
				break;
			}
			long long nCount = 0;
			// 添加线程函数
			while (nCount < nlength)
			{
				ret = pClient->DealCommand();
				if (ret < 0)
				{
					AfxMessageBox("传输失败");
					TRACE("传输失败, ret = %d\r\n", ret);
					break;
				}
				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
				nCount += pClient->GetPacket().strData.size();
			}
			AfxMessageBox("传输成功");
		} while (false);
		fclose(pFile);
		pClient->CloseClient();
	}
	EndWaitCursor();
	m_dlgStatus.ShowWindow(SW_HIDE);
}



void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL) return;
	if (m_Tree.GetChildItem(hTreeSelected) == NULL) return;

	DeleteTreeChildItem(hTreeSelected);
	m_List.DeleteAllItems();
	CString strPath = GetTreePath(hTreeSelected);

	int ncmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	CClientSocket* pClient = CClientSocket::getInstance();
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	while (pInfo->HasNext) {
		TRACE("[%s] is dir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (pInfo->IsDirectory)
		{
			if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..") {
				int cmd = pClient->DealCommand();
				TRACE("DIR ack : %d\r\n", cmd);
				if (cmd < 0) break;
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM htemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", htemp, TVI_LAST);
		}
		else
		{
			m_List.InsertItem(0, pInfo->szFileName);
		}
		int cmd = pClient->DealCommand();
		TRACE("DIR ack : %d\r\n", cmd);
		if (cmd < 0) break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseClient();
}

void CRemoteClientDlg::LoadFileCurInfo()
{
	HTREEITEM hTreeSelected = m_Tree.GetSelectedItem();
	CString strPath = GetTreePath(hTreeSelected);
	if (hTreeSelected == NULL) return;
	m_List.DeleteAllItems();
	int ncmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	CClientSocket* pClient = CClientSocket::getInstance();
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	while (pInfo->HasNext) {
		TRACE("[%s] is dir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (!(pInfo->IsDirectory))
		{
			m_List.InsertItem(0, pInfo->szFileName);
		}
		int cmd = pClient->DealCommand();
		TRACE("DIR ack : %d\r\n", cmd);
		if (cmd < 0) break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseClient();


}

CString CRemoteClientDlg::GetTreePath(HTREEITEM hTree) {
	CString strRet, strTmp;
	do
	{
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = m_Tree.GetParentItem(hTree);

	} while (hTree!=NULL);
	if (!strRet.IsEmpty() && strRet[strRet.GetLength() - 1] == '\\') {
		strRet = strRet.Left(strRet.GetLength() - 1); // 去除末尾的分隔符
	}
	return strRet;
}

void CRemoteClientDlg::DeleteTreeChildItem(HTREEITEM hTree) {
	
	HTREEITEM hsub = NULL;
	do
	{
		hsub = m_Tree.GetChildItem(hTree);
		if (hsub!=NULL)
		{
			m_Tree.DeleteItem(hsub);
		}
	} while (hsub!=NULL);
}


void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMRClickListFileinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0)
	{
		return;
	}
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPupup =  menu.GetSubMenu(0);
	if (pPupup!=NULL)
	{
		pPupup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,ptMouse.x,ptMouse.y,this);
	}



}


void CRemoteClientDlg::OnDownloadFile()
{
	// TODO: 在此添加命令处理程序代码
	_beginthread(CRemoteClientDlg::threadEntryForDownFile, 0, this);
	BeginWaitCursor();
	m_dlgStatus.m_info.SetWindowText(_T("执行中..."));
	m_dlgStatus.ShowWindow(SW_SHOW);
	m_dlgStatus.CenterWindow(this);
	m_dlgStatus.SetActiveWindow();
	Sleep(50);
}


void CRemoteClientDlg::OnDeleteFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetTreePath(hSelected);
	int nSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nSelected, 0);
	strFile = GetTreePath(hSelected) + "\\" + strFile;
	int ret = SendCommandPacket(9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("删除文件命令失败");
		return;
	}

	LoadFileCurInfo();
}


void CRemoteClientDlg::OnRunFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetTreePath(hSelected);
	int nSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nSelected, 0);
	strFile = GetTreePath(hSelected) + "\\" + strFile;
	int ret = SendCommandPacket(3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("打开文件命令失败");
		return;
	}


}

LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)
{
	int cmd = wParam >> 1;
	int ret = -1;
	switch (cmd)
	{
	case 4:
	{
		CString strFile = (LPCSTR)lParam;
		ret = SendCommandPacket(cmd, wParam & 1, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
		break;
	}
	case 5: // 鼠标操作
	{
		ret = SendCommandPacket(cmd, wParam & 1, (BYTE*)lParam, sizeof(MOUSEEVENT));
		break;
	}
	case 6:
	case 7:
	case 8:
	{
		ret = SendCommandPacket(cmd, wParam & 1);
		break;
	}
	default:
		break;
	}
	return ret;
}


void CRemoteClientDlg::OnBnClickedBtnStartwatch()
{
	m_isClosed = false;
	CWatchDialog dlg(this);
	HANDLE hTread = (HANDLE)_beginthread(CRemoteClientDlg::threadEntryForWatchData,0,this);
	dlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(hTread,500);
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}
