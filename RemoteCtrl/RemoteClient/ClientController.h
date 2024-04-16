#pragma once
#include "ClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>
#include "resource.h"

#define WM_SEND_PACK (WM_USER+1)//发送包数据
#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)//展示状态
#define WM_SHOW_WATCH (WM_USER+4)//远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000)//自定义消息处理

class CClientController
{
public:
	//获取全局唯一对象
	//由于 m_instance 是静态的，因此操作这个变量的函数也需要是静态的，因为静态函数可以在没有类的实例的情况下被调用。
	//如果 getInstance() 或 releaseInstance() 不是静态的，你则需要先有一个 CClientController 的对象才能调用它们，这样也就不能实现单例模式的目标了
	static CClientController* getInstance();
	//初始化控制器
	int	InitController();
	//启动控制器
	int Invoke(CWnd*& pMainWnd);
	//发送消息
	LRESULT SendMessage(MSG msg);
protected:
	CClientController():
		m_statusDlg(&m_remoteDlg),m_watchDlg(&m_remoteDlg) 
	{
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController() {

		WaitForSingleObject(m_hThread, 100);
	}
	void threadFunc();

	//线程的入口点，用于创建线程以异步执行某些任务
	static unsigned __stdcall threadEntry(void* arg);
	static void releaseInstance() {
		if (m_instance != NULL) {
			delete m_instance;
			m_instance = NULL;
		}
	}
	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lPARAM);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lPARAM);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lPARAM);
	LRESULT OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lPARAM);

private:
	typedef struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));

		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
				return *this;
			}
		}
	} MSGINFO;
	//定义了一个新的类型 MSGFUNC 作为指向 CClientController 成员函数的指针，这个成员函数接受三个参数
	//LRESULT通常代表了函数执行的结果或返回状态。
	//UINT nMsg代表消息的标识符，如 WM_PAINT（重绘消息），WM_KEYDOWN（键盘按键消息）等。
	//WPARAM wParam 和 LPARAM lParam 是两个传递附加消息信息的参数，具体意义取决于消息的类型。
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lPARAM);
	//键（key）类型为 UINT，值（value）类型为前面定义的 MSGFUNC（成员函数指针）
	static std::map<UINT,MSGFUNC>m_mapFunc;
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;//存储线程句柄
	unsigned m_nThreadID;//存储线程ID
	static CClientController* m_instance;//使用 static 表示这个变量是该类的所有实例共享的，而不是每个实例拥有自己的拷贝。这确保了无论创建多少个 CClientController 对象，m_instance 都指向同一个对象地址。
	class CHelper {//在单例模式中管理生命周期，确保单例的实例正确的在程序结束时被销毁
	public:
		CHelper() {//管理 CClientController 单例的生命周期
			CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}

	};
	static CHelper m_helper;
};