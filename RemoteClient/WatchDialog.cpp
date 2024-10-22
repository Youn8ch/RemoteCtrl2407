﻿// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "ClientSocket.h"

// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_WATCH, pParent)
{
	m_nObjHeight = -1;
	m_nObjWidth = -1;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{
	CPoint cur = point;
	CRect clientRect;
	if (isScreen) ScreenToClient(&point); // 全局坐标到客户区域坐标
	m_picture.GetWindowRect(clientRect);
	return CPoint(point.x * m_nObjWidth / clientRect.Width(), point.y * m_nObjHeight / clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 30, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull())
		{	
			// std::lock_guard<std::mutex> lock(pParent->m_imageMutex);

			CRect rect;
			m_picture.GetWindowRect(rect);
			pParent->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			if (m_nObjWidth == -1)
			{
				m_nObjWidth = pParent->GetImage().GetWidth();
			}
			if (m_nObjHeight == -1)
			{
				m_nObjHeight = pParent->GetImage().GetHeight();
			}
			// pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			m_picture.InvalidateRect(NULL);
			pParent->GetImage().Destroy();
			pParent->SetImgStatus();
			//
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!(m_nObjHeight == -1 || m_nObjWidth == -1))
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 0; // 左键
		event.nAction = 2; // 双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!(m_nObjHeight == -1 || m_nObjWidth == -1))
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 0; // 左键
		event.nAction = 2; // 按下
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event); 
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!(m_nObjHeight == -1 || m_nObjWidth == -1))
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 0; // 左键
		event.nAction = 3; // 弹起
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!(m_nObjHeight == -1 || m_nObjWidth == -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 1; // 右键
		event.nAction = 1; // 双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}



	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!(m_nObjHeight == -1 || m_nObjWidth == -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);

		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 1; // 右键
		event.nAction = 2; // 按下 
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!(m_nObjHeight == -1 || m_nObjWidth == -1))
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		CPoint remote = UserPoint2RemoteScreenPoint(point);

		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 1; // 右键
		event.nAction = 3; // 弹起
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}

	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!(m_nObjHeight == -1 || m_nObjWidth == -1))
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);

		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 4; // 没有按键
		event.nAction = 0; // 移动
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent(); // TODO 存在设计缺陷 网络通信和对话框有耦合
		// pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}

	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	if (!(m_nObjHeight == -1 || m_nObjWidth == -1))
	{
		// TODO: 在此添加控件通知处理程序代码
		CPoint point;
		GetCursorPos(&point);
		// 坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		// 封装

		MOUSEEVENT event;
		event.ptXY = remote;
		event.nButton = 0; // 左键
		event.nAction = 0; // 单击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
}



void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	// CDialog::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);
}
