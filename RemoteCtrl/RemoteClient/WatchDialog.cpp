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
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point)
{
	//(800*450)
	//CPoint cur = point;
	CRect clientRect;
	ScreenToClient(&point);//全局坐标到客户区域坐标
	m_picture.GetWindowRect(clientRect);
	//客户端坐标
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	//远程端坐标
	int width = 1920, height = 1080;
	//坐标转换
	int x = point.x * width / width0;
	int y = point.y * height / height0;

	return CPoint(x,y);
}

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


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 2;//双击
	//发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*) & event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	////坐标转换
	//CPoint remote = UserPoint2RemoteScreenPoint(point);
	////封装
	//MOUSEEV event;
	//event.ptXY = remote;
	//event.nButton = 0;//左键
	//event.nAction = 4;//弹起
	////发送
	//CClientSocket* pClient = CClientSocket::getInstance();
	//CPacket pack(5, (BYTE*)&event, sizeof(event));
	//pClient->Send(pack);
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 2;//右键
	event.nAction = 2;//双击
	//发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//按下//TODO:服务端要做对应修改
	//发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 4;//弹起
	//发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 1;//移动
	//发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	CPoint point;
	GetCursorPos(&point);
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//按下
	//发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
}
