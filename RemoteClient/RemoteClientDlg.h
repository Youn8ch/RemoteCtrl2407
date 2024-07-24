
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "StatusDlg.h"

#define WM_SEND_PACKET (WM_USER + 1)

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
	CStatusDlg m_dlgStatus;
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


public:
	bool isFull() const {
		return m_imgfull;
	}
	CImage& GetImage() {
		return m_image;
	}
	void SetImgStatus(bool isfull = false) {
		m_imgfull = isfull;
	}

private:
	CImage m_image; // 缓存
	bool m_imgfull; // true 有， false 无

private:
	static void threadEntryForWatchData(void* arg); // 转接
	void threadWatchData();

	static void threadEntryForDownFile(void* arg);
	void threadDownFile();
	void LoadFileInfo();
	void LoadFileCurInfo();
	// 后缀没带"\\"
	CString GetTreePath(HTREEITEM hTree);
	void DeleteTreeChildItem(HTREEITEM hTree);
	// 1 查看磁盘分区
	// 2 查看指定目录下文件
	// 3 打开文件
	// 4 查看文件
	// 9 删除文件
	// 5 鼠标操作
	// 6 屏幕内容
	// 7 锁机
	// 8 解锁
	// 666 测试连接
	// 返回值是命令号，小于0则错误
	int SendCommandPacket(int nCmd,bool bAutoclose = true, BYTE* pData = NULL,size_t nLength=0);

public:
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFileinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	afx_msg LRESULT OnSendPacket(WPARAM wParam,LPARAM lParam);
	afx_msg void OnBnClickedBtnStartwatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
