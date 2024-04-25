// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "ClientController.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_isFull = false;
	m_nObjWidth = -1;//n表示整数，后面是驼峰命名法
	m_nObjHeight = -1;
	//注意在构造函数中初始化这些成员变量时，是在给它们赋值而不是声明它们
	//如果你尝试在构造函数内使用int m_nObjWidth = -1;这种风格，编译器会认为你在尝试重新声明一个局部变量，而不是初始化类的成员变量。这会导致编译错误，
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
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
	ON_MESSAGE(WM_SEND_PACK_ACK,&CWatchDialog::OnSendPacketAck)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point, bool isScreen)
{
	//(800*450)
	//CPoint cur = point;
	CRect clientRect;
	if (!isScreen)
		ClientToScreen(&point);//转换为相对屏幕左上角的坐标（屏幕内的绝对坐标）
		m_picture.ScreenToClient(&point);//全局坐标到客户区域坐标
	TRACE("x=%d y=%d\r\n", point.x, point.y);
	//客户端坐标，到远程坐标
	m_picture.GetWindowRect(clientRect);
	//客户端坐标
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	TRACE("x=%d y=%d\r\n", clientRect.Width(), clientRect.Height());
	//远程端坐标
	int width = m_nObjWidth, height = m_nObjHeight;
	TRACE("x=%d y=%d\r\n", width,height);
	//坐标转换
	int x = point.x * width / width0;
	int y = point.y * height / height0;
	TRACE("x=%d y=%d\r\n",x, y);
	return CPoint(x, y);
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_isFull = false;
	//SetTimer(0, 40, NULL);//50ms一次，不需要回调函数

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{//在一个定时器（Timer）事件触发时被调用。定时器事件可以根据设置的时间间隔定期触发，用于执行周期性任务
	//if (nIDEvent == 0) {//判断传入的定时器ID是否为0。在MFC中，可以使用多个定时器，每个定时器有一个唯一的ID。
	//	CClientController* pParent = CClientController::getInstance();//获取父窗口，以访问public成员和函数（私有成员用公有函数当接口调用）
	//	if (m_isFull) {//父窗口缓冲区是否为空

	//		CRect rect;
	//		m_picture.GetWindowRect(rect);//获取窗口区域
	//		m_nObjWidth = m_image.GetWidth();
	//		m_nObjHeight = m_image.GetHeight();
	//		m_image.StretchBlt(
	//			m_picture.GetDC()->GetSafeHdc(), 0, 0,rect.Width(),rect.Height(), SRCCOPY);//绘图
	//		m_picture.InvalidateRect(NULL);//重新绘制，即刷新界面
	//		TRACE("更新图片完成%d %d\r\n", m_nObjWidth, m_nObjHeight);
	//		m_image.Destroy();
	//		m_isFull = false;
	//	}
	//}
	CDialog::OnTimer(nIDEvent);
}

LRESULT CWatchDialog::OnSendPacketAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || (lParam == -2)) {
	//TODO:错误处理

	}
	else if (lParam == 1) {
		//对方关闭了套接字
	}
	else {
		CPacket* pPacket = (CPacket*)wParam;
		if (pPacket != NULL) {
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (head.sCmd) {
			case 6:
			{
				ClassTool::Bytes2Image(m_image, head.strData);
				CRect rect;
				m_picture.GetWindowRect(rect);//获取窗口区域
				m_nObjWidth = m_image.GetWidth();
				m_nObjHeight = m_image.GetHeight();
				m_image.StretchBlt(
					m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);//绘图
				m_picture.InvalidateRect(NULL);//重新绘制，即刷新界面
				TRACE("更新图片完成%d %d\r\n", m_nObjWidth, m_nObjHeight);
				m_image.Destroy();
				break;
			}
			case 5:
				TRACE("远程端应答了鼠标操作\r\n");
				break;
			case 7:
			case 8:
			default:
				break;
			}
		}
	}
	return 0;
}

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		TRACE("remote:%d %d\r\n", remote.x, remote.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//双击
		//发送
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		TRACE("x=%d y=%d\r\n", point.x, point.y);
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		TRACE("x=%d y=%d\r\n", point.x, point.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//按下
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 3;//弹起
		//发送
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 1;//双击
		//发送
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 2;//按下//TODO:服务端要做对应修改
		//发送
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 3;//弹起
		//发送
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;//没有按键
		event.nAction = 0;//移动
		////发送
		//CClientSocket* pClient = CClientSocket::getInstance();
		//CPacket pack(5, (BYTE*)&event, sizeof(event));
		//pClient->Send(pack);
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CWatchDialog::OnStnClickedWatch()
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint point;
		GetCursorPos(&point);
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 0;//单击
		//发送
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
}

void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}

void CWatchDialog::OnBnClickedBtnLock()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 7);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 8);
}
