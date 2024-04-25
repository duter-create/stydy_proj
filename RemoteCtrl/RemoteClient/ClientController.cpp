#include "pch.h"
#include "ClientController.h"
#include "ClientSocket.h"

std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;
CClientController::CHelper CClientController::m_helper;

CClientController* CClientController::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CClientController();
		TRACE("CClientController size is %d\r\n", sizeof(*m_instance));
		//函数映射表
		struct { UINT nMsg; MSGFUNC func; }MsgFuncs[] =
		{ 
			//{WM_SEND_PACK,&CClientController::OnSendPack},
			//{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatch},
			{(UINT) - 1,NULL}//数组终止符
		};
		for (int i = 0; MsgFuncs[i].func != NULL; i++) {
			m_mapFunc.insert(std::pair < UINT, MSGFUNC >(MsgFuncs[i].nMsg,MsgFuncs[i].func));
		}
	}
	return m_instance;
}

int CClientController::InitController()
{
	//创建线程
	m_hThread = (HANDLE)_beginthreadex(//线程安全属性、栈大小、线程函数、线程参数、创建选项和线程ID指针
		NULL, 0,
		&CClientController::threadEntry,
		this, 0,
		&m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS,&m_remoteDlg);
	return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();//创建并显示一个模态对话框
}

LRESULT CClientController::SendMessage(MSG msg)
{//向由 m_nThreadID 标识的线程发送一个消息
	/*
	UINT message：消息的标识符，例如 WM_COMMAND。
	WPARAM wParam：消息的第一个参数，通常用来传递消息特定的信息，如菜单项的 ID 或控件的 ID。
	LPARAM lParam：消息的第二个参数，通常用来携带更具体的信息，例如鼠标点击的位置。
	*/
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)
		return -2;
	MSGINFO info(msg);
	//将一个消息发送到属于另一个线程的消息队列中
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);//等待事件对象被设置为信号状态，即等待另一个线程处理完消息。这里使用了INFINITE参数，意味着会无限等待直到消息被处理。
	return info.result;//返回处理结果
}

bool CClientController::SendCommandPacket
(HWND hWnd,int nCmd, bool bAutoClose, BYTE* pData, size_t nLength, WPARAM wParam)
{
	TRACE("cmd:%d %s start %lld \r\n",nCmd,__FUNCTION__,GetTickCount64());
	CClientSocket* pClient = CClientSocket::getInstance();
	bool ret = pClient->SendPacket(hWnd,CPacket(nCmd, pData, nLength), bAutoClose,wParam);
	return ret;
}

void CClientController::DownloadEnd()
{
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成"), _T("完成"));
}

int CClientController::DownFile(CString strPath)
{
	CFileDialog dlg(FALSE, NULL,
		strPath,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, &m_remoteDlg);
	//以模态方式显示对话框。如果用户点击“保存”按钮（通常响应为 IDOK）
	// 则 DoModal 方法会返回 IDOK，代码将进入到 if 语句块内执行
	if (dlg.DoModal() == IDOK) {
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		FILE* pFile = fopen(m_strLocal, "wb+");
		if (pFile == NULL) {
			AfxMessageBox(_T("本地没有权限保存该文件，或者文件无法创建"));
			return -1;
		}
		SendCommandPacket(m_remoteDlg, 4, false, (BYTE*)(LPCSTR)m_strRemote,
			m_strRemote.GetLength(), (WPARAM)pFile);
		//m_hThreadDownload = (HANDLE)_beginthread(&CClientController::threadDownloadEntry, 0, this);
		//1 作为线程入口点，这个函数将在新线程中执行
		//2 0是初始线程堆栈大小的参数。数值0表示使用默认的大小
		//3 传递给线程的参数。在这种情况下，this 指针指向当前正在执行 _beginthread 调用的类实例 CRemoteClientDlg 对象。

		/*if (WaitForSingleObject(m_hThreadDownload, 0) != WAIT_TIMEOUT) {
			return -1;
		}*/
		m_remoteDlg.BeginWaitCursor();
		m_statusDlg.m_info.SetWindowText(_T("命令正在执行中"));//SetWindowText 方法用于设置 m_info 控件的文本内容。在这里，它被设置为显示 "命令正在执行中"
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);
		m_statusDlg.SetActiveWindow();//使 m_dlgStatus 对话框成为当前活动窗口,将对话框带到屏幕的最前端
	}
	return 0;
}

void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	//m_watchDlg.SetParent(&m_remoteDlg);
	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadWatchScreen, 0, this);
	m_watchDlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);//同步函数，用来等待 _beginthread 创建的线程完成。如果线程在500毫秒内结束，函数会返回；如果线程没有在指定时间内结束，函数也会返回，不会无限期地等待
}

void CClientController::threadWatchScreen()
{
	Sleep(50);
	ULONGLONG nTick = GetTickCount64();//记录当前的系统运行时间。GetTickCount64返回自系统启动以来的毫秒数。
	while (!m_isClosed) {
		if (m_watchDlg.isFull()==false) {
			if (GetTickCount64() - nTick < 200) {
				Sleep(200 - DWORD(GetTickCount64() - nTick));
			}
			nTick = GetTickCount64();
			int ret = SendCommandPacket(m_watchDlg.GetSafeHwnd(), 6, true, NULL, 0);
			//TODO:添加消息响应函数WM_SEND_PACK_ACK
			//TODO：控制发送频率
			if (ret == 1) {
				TRACE("成功发送请求图片命令\r\n");				
			}
			else {
				TRACE("获取图片失败 ret = %d\r\n",ret);
			}
		}
		Sleep(1);
	}
	TRACE("thread end %d\r\n", m_isClosed);
}

void CClientController::threadWatchScreen(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchScreen();
	_endthread();
}

void CClientController::threadDownloadFile()
{
	FILE* pFile = fopen(m_strLocal, "wb+");
	if (pFile == NULL) {
		AfxMessageBox(_T("本地没有权限保存该文件，或者文件无法创建"));
		m_statusDlg.ShowWindow(SW_HIDE);//将 m_dlgStatus 对话框隐藏起来
		m_remoteDlg.EndWaitCursor();
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	do {
		int ret = SendCommandPacket(m_remoteDlg,4, false, (BYTE*)(LPCSTR)m_strRemote,
			m_strRemote.GetLength(),(WPARAM)pFile);
		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
		if (nLength == 0) {
			AfxMessageBox("文件长度为0或者无法读取文件");
			break;
		}
		long long nCount = 0;
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
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成"), _T("完成"));
	m_remoteDlg.LoadFileInfo();
}

void CClientController::threadDownloadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadDownloadFile();
	_endthread();
}

void CClientController::threadFunc()
{//实现消息循环
	MSG msg;//存放消息队列中的消息
	while (::GetMessage(&msg, NULL, 0, 0)) {//从当前线程的消息队列中检索消息
		TranslateMessage(&msg);//转换键盘消息
		DispatchMessage(&msg);//把消息传递给相应的窗口进程
		if (msg.message == WM_SEND_MESSAGE) {
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(pmsg->msg.message);
			if (it != m_mapFunc.end()) {
				//(this->*)it->second
				pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);//调用找到的消息处理函数
			}
			else {
				pmsg->result = -1;
			}
			SetEvent(hEvent);//通过设置事件对象通知消息发送者，该消息处理已经完成
		}
		else {
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {
				//(this->*)it->second
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);//调用找到的消息处理函数
			}
		}
	}
}
unsigned __stdcall CClientController::threadEntry(void* arg)
{//线程入口函数
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();//执行消息循环
	_endthreadex(0);//结束线程
	return 0;
}

/*
LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lPARAM)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket* pPacket = (CPacket*)wParam;
	return pClient->Send(*pPacket);
}
*/

/*
LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{ 
	CClientSocket* pClient = CClientSocket::getInstance();
	char* pBuffer = (char*)wParam;//要发送数据的缓冲区的指针
	return pClient->Send(pBuffer,(int)lParam);//要发送的数据的大小或长度
}
*/

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lPARAM)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lPARAM)
{
	return m_watchDlg.DoModal();
}
