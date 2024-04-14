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
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

int CRemoteClientDlg::SendCommandPacket(int nCmd,bool bAutoClose, BYTE* pData, size_t nLength)
{//向服务器发送一个命令数据包，然后接收回应
	UpdateData();//在MFC对话框类中用来从界面控件更新成员变量，或者更新界面控件的值
	CClientSocket* pClient = CClientSocket::getInstance();//获得单例
	//atoi 函数将端口号从字符串转换为整数；LPCTSTR把字符串对象转换成C风格字符串
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nPort));
	if (!ret) {
		AfxMessageBox("网络初始化失败");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);//构造packet包
	//CPacket pack;
	pClient->Send(pack);
	TRACE("Send ret %d\r\n", ret);
	int cmd = pClient->DealCommand();
	TRACE("ack:%d\r\n", cmd);
	if (bAutoClose) {
		pClient->CloseSocket();
	}
	return cmd;
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERV, &CRemoteClientDlg::OnIpnFieldchangedIpaddressServ)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CRemoteClientDlg::OnEnChangeEditPort)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	ON_MESSAGE(WM_SEND_PACKET,&CRemoteClientDlg::OnSendPacket)//第三步，在消息映射表里面注册消息
	//将 Windows 消息（在这里是 WM_SEND_PACKET）映射到特定消息处理函数（在这里是 CRemoteClientDlg::OnSendPacket）的一种方法。这段代码通常在一个消息映射宏列表中。
	//WM_SEND_PACKET是用户自定义的待处理的消息
	//&CRemoteClientDlg::OnSendPacket是用来处理这个消息的函数。这个函数必须是声明在接收处理消息的类（在这里是 CRemoteClientDlg）内的成员函数。
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
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
	m_server_address = 0x7F000001;//127.0.0.1
	//m_server_address = 0xC0A83B85;//192.168.59.133
	//m_server_address = 0x0A19F1D5;//10.25.241.213
	m_nPort = _T("9527");
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DLG_STATUS,this);
	m_dlgStatus.FlashWindow(SW_HIDE);
	m_isFull = false;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{

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
{//发一个1981的cmd测试
	SendCommandPacket(1981);
}


void CRemoteClientDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{//更新一个树视图控件来显示这些磁盘分区
	int ret = SendCommandPacket(1);//查看磁盘分区
	if (ret == -1) {
		AfxMessageBox(_T("命令处理失败!!!"));
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();//获取客户端socket实例
	std::string drivers = pClient->GetPacket().strData;//拿packet里面的数据
	std::string dr;//存储驱动器标识符
	m_Tree.DeleteAllItems();//将新数据添加到树视图控件 (CTreeCtrl) 之前，先删除所有已存在的项目
	for (size_t i = 0; i < drivers.size(); i++) {
		//if (drivers[i] == 'C' || drivers[i] == 'E') {
		if (isalpha(drivers[i]) && isupper(drivers[i])){
			dr += drivers[i];
			dr += ":";
			HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(),TVI_ROOT,TVI_LAST);//TVI_ROOT: 加在树视图的根节点下面,TVI_LAST: 插入在根节点子项的最后一个
			//hTemp：新插入项的句柄   dr.c_str():传递添加新项的文本标签

			m_Tree.InsertItem("", hTemp, TVI_LAST);//添加一个空文本的子项到hTemp所引用的父项下，供后续可能的内容填充
			dr.clear();
			continue;
		}	
	}
}

void CRemoteClientDlg::threadEntryForWatchData(void* arg)
{//开线程处理数据
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadWatchData();
	_endthread();
}

void CRemoteClientDlg::threadWatchData()
{//处理数据的线程实现
//	可能存在异步问题，导致程序崩溃
	Sleep(50);
	CClientSocket* pClient = NULL;

	do {
		//CClientSocket* pClient = CClientSocket::getInstance();//循环内部重新声明了一个局部变量pClient，这实际上是个错误。新声明的pClient会遮蔽函数开头声明的pClient，导致循环外部的pClient始终是NULL，循环永远不会退出。
		pClient = CClientSocket::getInstance();

	} while (pClient == NULL);
	ULONGLONG tick = GetTickCount64();
	while(!m_isClosed) {//
		if (m_isFull == false) {//更新数据到缓存
			//CPacket pack(6, NULL, 0);
			//bool ret = pClient->Send(pack);
		int ret = SendMessage(WM_SEND_PACKET, 6 << 1 | 1);//SendMessage 函数是一个Windows API，用于发送一个消息到某个窗口的消息队列，
		if (ret == 6) {
			//int cmd = pClient->DealCommand();//拿数据,已经拿了，这里不用再拿了
			//if (cmd == 6) {//发送屏幕的截图

			BYTE* pData = (BYTE*)pClient->GetPacket().strData.c_str();
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
			if (hMem == NULL) {
				TRACE("内存不足了");
				Sleep(1);
				continue;
			}
			IStream* pStream = NULL;
			HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
			if (hRet == S_OK) {
				ULONG length = 0;
				pStream->Write(pData, pClient->GetPacket().strData.size(), &length);
				LARGE_INTEGER bg = { 0 };
				pStream->Seek(bg, STREAM_SEEK_SET, NULL);
				if((HBITMAP)m_image != NULL)
					m_image.Destroy();
				m_image.Load(pStream);
				m_isFull = true;
			}
		}
		else {
			Sleep(1);
		}
	}
	else Sleep(1);//预防网络突然断掉，上面的循环把cpu拉满，留个空闲让cpu处理其他程序
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
	int nListSelected = m_List.GetSelectionMark();//从列表视图控件 m_List 中获取当前选中项的索引
	CString strFile = m_List.GetItemText(nListSelected, 0);//用刚刚拿到的索引来拿选中项的文件名

	long long nCount = 0;
	//CFileDialog 是 MFC (Microsoft Foundation Classes) 库中用于创建文件对话框的类
	//FALSE 参数用于指定对话框的模式，这里 FALSE 表示对话框是“保存文件”模式。如果是 TRUE，则表示为“打开文件”模式。
	//"*" 参数是默认的文件扩展名。在这种情况下，它被设置为一个通配符，这意味着所有文件扩展名都可以。
	//m_List.GetItemText(nListSelected, 0) 获得列表控件中当前被选中的项的文本，通常表示文件名，将用作对话框显示的默认文件名。
	//OFN_HIDEREADONLY 表示隐藏只读选项。
	//OFN_OVERWRITEPROMPT 表示在用户选择要保存到一个已经存在的文件时，提示用户确认是否覆盖原文件。

	CFileDialog dlg(FALSE, "*",
		strFile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, this);
	//以模态方式显示对话框。如果用户点击“保存”按钮（通常响应为 IDOK）
	// 则 DoModal 方法会返回 IDOK，代码将进入到 if 语句块内执行
	if (dlg.DoModal() == IDOK) {
		//开个文件把接收到的文件写到本地
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");//以二进制模式打开文件，允许读写操作
		if (pFile == NULL) {
			AfxMessageBox(_T("本地没有权限保存该文件，或者文件无法创建"));
			m_dlgStatus.ShowWindow(SW_HIDE);//将 m_dlgStatus 对话框隐藏起来
			EndWaitCursor();
			return;
		}
		HTREEITEM hSelected = m_Tree.GetSelectedItem();//获取选中项的句柄
		strFile = GetPath(hSelected) + strFile;//父项和该项的路径，构成完整的文件路径
		TRACE("%s\r\n", LPCSTR(strFile));
		CClientSocket* pClient = CClientSocket::getInstance();//获取单例模式中的实例
		do {
			//int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)strFile, strFile.GetLength());//发送下载命令到服务器
			int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)strFile);//SendMessage 函数是一个Windows API，用于发送一个消息到某个窗口的消息队列，左移一位为了移动到指定位置上
			if (ret < 0) {
				AfxMessageBox("执行下载命令失败");
				TRACE("执行下载失败：ret = %d\r\n", ret);
				break;
			}
			long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
			if (nLength == 0) {
				AfxMessageBox("文件长度为0或者无法读取文件");
				break;
			}
			while (nCount < nLength) {//已接收的数据量未达到文件总数据量
				ret = pClient->DealCommand();
				if (ret < 0) {
					AfxMessageBox("传输失败");
					TRACE("传输失败: ret = %d\r\n", ret);
					break;
				}
				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
				nCount += pClient->GetPacket().strData.size();//更新已经接收到的数据量
			}
		} while (false);
		fclose(pFile);
		pClient->CloseSocket();
	}
	m_dlgStatus.ShowWindow(SW_HIDE);//将 m_dlgStatus 对话框隐藏起来
	EndWaitCursor();
	MessageBox(_T("下载完成"), _T("完成"));
}

void CRemoteClientDlg::LoadFileCurrent()
{//从树形视图控件 m_Tree 的当前选中项目获取路径，更新当前选中目录的文件列表
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hTree);
	m_List.DeleteAllItems();//清空列表控件中所有的条目，准备添加新的文件信息。
	//CString strPath = GetPath(hTreeSelected);//获取从选中项到树根的路径
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());//发送一个命令数据包到服务器
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();//获取服务器响应数据
	CClientSocket* pClient = CClientSocket::getInstance();//定义一个指向文件信息的指针 pInfo，并指向从客户端套接字获取的数据包中的数据
	while (pInfo->HasNext == TRUE) {
		TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (!pInfo->IsDirectory) {//当前处理的是文件不是目录
			m_List.InsertItem(0, pInfo->szFileName);
		}

		int cmd = pClient->DealCommand();
		TRACE("ack:%d\r\n", cmd);//打印接收到的响应命令
		if (cmd < 0)
			break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}

	pClient->CloseSocket();
}

void CRemoteClientDlg::LoadFileInfo()
{//函数是基于鼠标点击的位置来确定操作的树节点，然后获取相应路径。此函数首先转换鼠标屏幕位置到树形视图坐标系中，然后使用 HitTest 方法判断鼠标点中了哪个树节点
	//获取鼠标当前位置，并将屏幕坐标转换为树视图控件的客户区坐标
	CPoint ptMouse;//在MFC中，CPoint 是一个表示点的类，它能存储二维坐标
	GetCursorPos(&ptMouse);//把当前鼠标光标的屏幕坐标填充到 ptMouse 中。屏幕坐标是相对于屏幕左上角的位置
	m_Tree.ScreenToClient(&ptMouse);//将 ptMouse 中的屏幕坐标转换为相对于 m_Tree 的客户区域的坐标
	//确定鼠标双击的位置在树视图中对应哪个项 hTreeSelected
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);//获取当前鼠标光标位置 ptMouse 所在的树形视图 m_Tree 的项（HTREEITEM）
	if (hTreeSelected == NULL)//单击事件发生在没有树视图项目的位置
		return;
	if (m_Tree.GetChildItem(hTreeSelected) == NULL)//是否有子项目
		return;
	DeleteTreeChildrenItem(hTreeSelected);
	m_List.DeleteAllItems();
	CString strPath = GetPath(hTreeSelected);//获取从选中项到树根的路径
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());//发送一个命令数据包到服务器
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();//获取服务器响应数据
	CClientSocket* pClient = CClientSocket::getInstance();//定义一个指向文件信息的指针 pInfo，并指向从客户端套接字获取的数据包中的数据
	int Count = 0;
	while (pInfo->HasNext == TRUE) {
		TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (pInfo->IsDirectory) {
			if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == "..")) {
				//不会对 "." 或 ".." 这两个特殊目录项目进行处理，而是跳过它们，并继续处理其他的文件或目录项
				int cmd = pClient->DealCommand();
				TRACE("ack:%d\r\n", cmd);//打印接收到的响应命令
				if (cmd < 0)
					break;
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", hTemp, TVI_LAST);
		}
		else {
			m_List.InsertItem(0, pInfo->szFileName);
		}
		Count++;
		int cmd = pClient->DealCommand();
 		TRACE("ack:%d\r\n", cmd);//打印接收到的响应命令
		if (cmd < 0)
			break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}

	pClient->CloseSocket();
	TRACE("Client: Count = %d\r\n", Count);
}

void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{//
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTree);//获取父项的第一个子项
		if (hSub != NULL)
			m_Tree.DeleteItem(hSub);//删除这个子项
	} while (hSub != NULL);
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree) {//返回值代表从树视图控件 m_Tree 中给定项 hTree 到根的路径。
	CString strRet, strTmp;
	do {
		strTmp = m_Tree.GetItemText(hTree);//获取当前节点的文本，并将其存储到 strTmp
		strRet = strTmp + '\\' + strRet;
		hTree = m_Tree.GetParentItem(hTree);//获取当前节点的父节点，并将父节点作为下一次循环的当前节点
	} while (hTree != NULL);
	return strRet;//包含了从起始节点到树的根的完整路径
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{//双击和单击效果一样
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{//单击
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}

void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)//右键单击
{//点击列表视图 m_List 控件时弹出一个上下文菜单，使用户能够对选中的列表项进行操作
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse,ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);//从屏幕坐标转换成客户端的坐标
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0)
		return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);//把菜单加载上来
	CMenu* pPupup = menu.GetSubMenu(0);//取子菜单第一个
	if (pPupup != NULL) {
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);//弹出来
	}
}

void CRemoteClientDlg::OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CRemoteClientDlg::OnEnChangeEditPort()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void CRemoteClientDlg::OnDownloadFile()
{
	////////添加线程函数
	_beginthread(CRemoteClientDlg::threadEntryForDownFile, 0, this);
	//1 作为线程入口点，这个函数将在新线程中执行
	//2 0是初始线程堆栈大小的参数。数值0表示使用默认的大小
	//3 传递给线程的参数。在这种情况下，this 指针指向当前正在执行 _beginthread 调用的类实例 CRemoteClientDlg 对象。
	BeginWaitCursor();//沙漏光标
	m_dlgStatus.m_info.SetWindowText(_T("命令正在执行中"));//SetWindowText 方法用于设置 m_info 控件的文本内容。在这里，它被设置为显示 "命令正在执行中"
	m_dlgStatus.ShowWindow(SW_SHOW);
	m_dlgStatus.CenterWindow(this);
	m_dlgStatus.SetActiveWindow();//使 m_dlgStatus 对话框成为当前活动窗口,将对话框带到屏幕的最前端
	//TODO:大文件传输需要额外的处理
}

void CRemoteClientDlg::OnDeleteFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();//获取选中项的句柄
	CString strPath = GetPath(hSelected);
	int nSelected = m_List.GetSelectionMark();//从列表视图控件 m_List 中获取当前选中项的索引
	CString strFile = m_List.GetItemText(nSelected, 0);//根据索引拿文本，0 表明我们想要获取当前选中项的第一列的内容
	strFile = strPath + strFile;
	int ret = SendCommandPacket(9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox("删除文件命令执行失败");
	}
	LoadFileCurrent();
}


void CRemoteClientDlg::OnRunFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();//获取选中项的句柄
	CString strPath = GetPath(hSelected);
	int nSelected = m_List.GetSelectionMark();//从列表视图控件 m_List 中获取当前选中项的索引
	CString strFile = m_List.GetItemText(nSelected, 0);//根据索引拿文本，0 表明我们想要获取当前选中项的第一列的内容
	strFile = strPath + strFile;
	int ret = SendCommandPacket(3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox("打开文件命令执行失败");
	}
}

LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)//第四步：实现消息响应函数
{//类 CRemoteClientDlg 中的一个消息处理函数 OnSendPacket 的定义。
//函数 OnSendPacket 响应自定义的 WM_SEND_PACKET 消息，并执行发送命令到服务器的操作。
	int ret = 0;
	int cmd = wParam >> 1;
	switch (cmd) {
	case 4:
	{
		CString strFile = (LPCSTR)lParam;
		//int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)strFile, strFile.GetLength());//发送下载命令到服务器
		//只接收两个函数的处理
		ret = SendCommandPacket(cmd, wParam & 1, (BYTE*)(LPCSTR)strFile, strFile.GetLength());//发送下载命令到服务器
	}
	break;
	case 5: {//鼠标操作
		ret = SendCommandPacket(cmd, wParam & 1, (BYTE*)lParam, sizeof(MOUSEEV));
	}
	break;
	case 6:
	case 7:
	case 8:
	{
		ret = SendCommandPacket(cmd, wParam & 1);//发送下载命令到服务器
	}
	break;
	default:
		ret = -1;
	}
	return ret;
}

/*
void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	_beginthread(CRemoteClientDlg::threadEntryForWatchData, 0, this);
	GetDlgItem(IDC_BTN_START_WATCH)->EnableWindow(FALSE);//窗口禁用,开启监视窗口的时候再打开
	CWatchDialog dlg;
	dlg.DoModal();
}
*/


void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	m_isClosed = false;//对话框开启状态
	CWatchDialog dlg(this);//创建了对话框CWatchDialog的一个实例dlg
	//this作为父窗口的指针传入，可以直接访问CRemoteClientDlg的成员变量或函数
	//父窗口关闭时，子窗口也都会关闭并且回收资源
	HANDLE hThread = (HANDLE)_beginthread(CRemoteClientDlg::threadEntryForWatchData, 0, this);
	
	dlg.DoModal();//显示对话框，并进入一个模态循环，阻塞调用线程直到对话框被关闭。
	//模态和非模态对话框的主要区别在于模态对话框在用户与其交互时会禁用其它窗口，防止用户在没有处理完当前对话框情况下进行其它操作。
	// 而非模态对话框允许用户在对话框打开的时候与应用程序的其它窗口交互。
	m_isClosed = true;//对话框关闭
	WaitForSingleObject(hThread, 500);//等待之前创建的线程停止（500ms）
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值


	CDialogEx::OnTimer(nIDEvent);
}
