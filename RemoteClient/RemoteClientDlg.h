﻿
// RemoteClientDlg.h: 头文件
//

#pragma once


// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_nPort;
	CString m_Port;
	afx_msg void OnBnClickedButtonFile();

private:
	void LoadFileInfo();
	CString GetTreePath(HTREEITEM hTree);
	void DeleteTreeChildItem(HTREEITEM hTree);
	// 1 查看磁盘分区
	// 2 查看指定目录下文件
	// 3 打开文件
	// 4 查看文件
	// 返回值是命令号，小于0则错误
	int SendCommandPacket(int nCmd,bool bAutoclose = true, BYTE* pData = NULL,size_t nLength=0);

public:
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFileinfo(NMHDR* pNMHDR, LRESULT* pResult);
};
