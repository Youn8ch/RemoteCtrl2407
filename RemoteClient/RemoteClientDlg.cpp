
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "ClientController.h"
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
	ON_EN_CHANGE(IDC_EDIT_PORT2, &CRemoteClientDlg::OnEnChangeEditPort2)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERV, &CRemoteClientDlg::OnIpnFieldchangedIpaddressServ)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CRemoteClientDlg::OnSendPacketACK)
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
	 m_server_address = 0x7F000001;
    //m_server_address = 0xC0A83865; // 192.168.56.101
	m_Port = _T("8554");
	CClientController::getInstance()->UpdateAddress(m_server_address, atoi((LPCTSTR)m_Port));
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DLG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);
	

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

	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 666);

}


void CRemoteClientDlg::OnBnClickedButtonFile()
{
	std::list<CPacket> lstPackets;
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 1);
	if (ret == 0)
	{
		AfxMessageBox(_T("ButtonFile命令处理失败"));
		return;
	}
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
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),
		2,false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength(),(WPARAM)hTreeSelected);

}

void CRemoteClientDlg::LoadFileCurInfo()
{
	HTREEITEM hTreeSelected = m_Tree.GetSelectedItem();
	CString strPath = GetTreePath(hTreeSelected);
	if (hTreeSelected == NULL) return;
	m_List.DeleteAllItems();
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());

}

void CRemoteClientDlg::Str2Tree(const std::string& drivers, CTreeCtrl& tree)
{
	std::string dr;
	tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',') {
			dr += ":";
			HTREEITEM htemp = tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			tree.InsertItem("", htemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	dr += ":";
	HTREEITEM htemp = tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
	tree.InsertItem("", htemp, TVI_LAST);
}

void CRemoteClientDlg::UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent)
{
	if (finfo.HasNext == FALSE) return;
	if (finfo.IsDirectory) {
		if (CString(finfo.szFileName) == "." || CString(finfo.szFileName) == "..") {
			return;
		}
		HTREEITEM htemp = m_Tree.InsertItem(finfo.szFileName, hParent, TVI_LAST);
		m_Tree.InsertItem("", htemp, TVI_LAST);
	}
	else
	{
		m_List.InsertItem(0, finfo.szFileName);
	}
	m_Tree.Expand(hParent, TVE_EXPAND);
}



//void CRemoteClientDlg::UpdataFileInfo(const FILEINFO& finfo, HTREEITEM hParent)
//{
//	if (finfo.HasNext == FALSE) return;
//	if (finfo.IsDirectory) {
//		if (CString(finfo.szFileName) == "." || CString(finfo.szFileName) == "..") {
//			return;
//		}
//		HTREEITEM htemp = m_Tree.InsertItem(finfo.szFileName, hParent, TVI_LAST);
//		m_Tree.InsertItem("", htemp, TVI_LAST);
//	}
//	else
//	{
//		m_List.InsertItem(0, finfo.szFileName);
//	}
//	m_Tree.Expand(hParent, TVE_EXPAND);
//}

void CRemoteClientDlg::UpdataDownloadFile(const std::string& strData, FILE* pFILE)
{
	static LONGLONG length = 0, index = 0;
	if (length == 0)
	{
		length = *(long long*)strData.c_str();
		if (length == 0)
		{
			AfxMessageBox("文件长度为0 或无法读取");
			CClientController::getInstance()->DownFileEnd();
		}
	}
	else if (length > 0 && index >= length)
	{
		fclose(pFILE);
		length = 0;
		index = 0;
		CClientController::getInstance()->DownFileEnd();
	}
	else
	{
		fwrite(strData.c_str(), 1, strData.size(), pFILE);
		index += strData.size();
		if (index >= length) {
			fclose(pFILE);
			length = 0;
			index = 0;
			CClientController::getInstance()->DownFileEnd();
		}
	}

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
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	strFile = GetTreePath(hSelected) + "\\" + strFile;
	int ret = CClientController::getInstance()->DownFile(strFile);
	if (ret < 0)
	{
		MessageBox(_T("文件下载失败！"));
		TRACE("FILE DOWN FAILED ret = %d \r\n", ret);
	}
}


void CRemoteClientDlg::OnDeleteFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetTreePath(hSelected);
	int nSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nSelected, 0);
	strFile = GetTreePath(hSelected) + "\\" + strFile;
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
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
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
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
		ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), cmd, wParam & 1, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
		break;
	}
	case 5: // 鼠标操作
	{
		ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), cmd,wParam & 1, (BYTE*)lParam, sizeof(MOUSEEVENT));
		break;
	}
	case 6:
	case 7:
	case 8:
	{
		ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), cmd,wParam & 1);
		break;
	}
	default:
		break;
	}
	return ret;
}


void CRemoteClientDlg::OnBnClickedBtnStartwatch()
{
	CClientController::getInstance()->StartWatchScreen();
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}


void CRemoteClientDlg::OnEnChangeEditPort2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData();
	CClientController::getInstance()->UpdateAddress(
		m_server_address, atoi((LPCTSTR)m_nPort));
}


void CRemoteClientDlg::OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	UpdateData();
	CClientController::getInstance()->UpdateAddress(
		m_server_address, atoi((LPCTSTR)m_nPort));
}

LRESULT CRemoteClientDlg::OnSendPacketACK(WPARAM wParam, LPARAM lParam)
{
	CPacket* pPacket = (CPacket*)wParam;
	if (lParam == -1 || lParam == -2 || lParam == 1)
	{
		// TODO 错误处理
	}
	if (pPacket != NULL)
	{
		switch (pPacket->sCmd) {
		case 1: // 获取驱动信息
		{
			Str2Tree(pPacket->strData, m_Tree);
			break;
		}
		case 2: // 获取文件信息
		{
			PFILEINFO Info = (PFILEINFO)pPacket->strData.c_str();
			UpdateFileInfo(*Info,(HTREEITEM)lParam);

			break;
		}
		case 3:
		{
			TRACE(" Run File Done! \r\n");
			break;
		}
		case 4:
		{
			UpdataDownloadFile(pPacket->strData.c_str(), (FILE*)lParam);
			break;
		}
		case 9:
		{
			TRACE(" Delete File Done! \r\n");
			break;
		}
		case 666:
		{
			MessageBox("连接测试成功", "连接成功", MB_ICONINFORMATION);
			break;
		}
		default:
			TRACE(" Unknown cmd = %d Received! \r\n", pPacket->sCmd);
			break;
		}
	}
	if (pPacket != NULL) delete pPacket;
	return LRESULT();
}
