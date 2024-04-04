// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"

// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{

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
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 45, NULL);//50ms一次，不需要回调函数

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{//在一个定时器（Timer）事件触发时被调用。定时器事件可以根据设置的时间间隔定期触发，用于执行周期性任务
	if (nIDEvent == 0) {//判断传入的定时器ID是否为0。在MFC中，可以使用多个定时器，每个定时器有一个唯一的ID。
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();//获取父窗口，以访问public成员和函数（私有成员用公有函数当接口调用）
		if (pParent->isFull()) {//父窗口缓冲区是否为空
			CRect rect;
			m_picture.GetWindowRect(rect);//获取窗口区域
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			pParent->GetImage().StretchBlt(
				m_picture.GetDC()->GetSafeHdc(), 0, 0,rect.Width(),rect.Height(), SRCCOPY);//绘图
			m_picture.InvalidateRect(NULL);//重新绘制，即刷新界面
			pParent->GetImage().Destroy();
			pParent->SetImageStatus();//清理资源
		}
	}
	CDialog::OnTimer(nIDEvent);
}
