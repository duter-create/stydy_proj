
// RemoteClientDlg.h: 头文件
//


#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"
#ifndef WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER+2) //发送包数据应答
#endif

//#define WM_SEND_PACKET (WM_USER + 1) //第一步，自定义消息的ID
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
public:
	void LoadFileInfo();
private:

	bool m_isClosed;//监视是否关闭
private:
	void DealCommand(WORD nCmd,const std::string& strData, LPARAM lParam);
	void InitUIData();
	void LoadFileCurrent();
	void Str2Tree(const std::string& driver,CTreeCtrl& tree);
	void UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent);
	void UpdateDownloadFile(const std::string& strData,FILE* pFile);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	CString GetPath(HTREEITEM hTree);

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
	afx_msg void OnEnChangeEdit1();
	DWORD m_server_address;
	CString m_nPort;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditPort();
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	//第二步，定义自定义消息响应函数
	//afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);//在类的声明中用来定义一个消息处理函数。这个函数是为了响应自定义或已定义的 Windows 消息
	//OnSendPacket: 这是函数的名称。按照MFC的命名惯例，消息处理函数的名称通常以 "On" 开头，后跟消息的名称。
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnSendPacketAck(WPARAM wParam, LPARAM lParam);
};


