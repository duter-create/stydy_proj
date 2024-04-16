#include "pch.h"
#include "ClientController.h"

std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;

CClientController* CClientController::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CClientController();
		//函数映射表
		struct { UINT nMsg; MSGFUNC func; }MsgFuncs[] =
		{ 
			{WM_SEND_PACK,&CClientController::OnSendPack},
			{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatch},
			{(UINT) - 1,NULL}//数组终止符
		};
		for (int i = 0; MsgFuncs[i].func != NULL; i++) {
			m_mapFunc.insert(std::pair < UINT, MSGFUNC >(MsgFuncs[i].nMsg,MsgFuncs[i].func));
		}
	}
	return nullptr;
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
	WaitForSingleObject(hEvent, -1);
	return info.result;//返回处理结果
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

LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lPARAM)
{

	return LRESULT();
}

LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lPARAM)
{
	 
	return LRESULT();
}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lPARAM)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lPARAM)
{
	return m_watchDlg.DoModal();
}
