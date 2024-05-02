#pragma once
#include "pch.h"
#include <atomic>

template<class T>
//CQueue 是一个模板类，可以存储任何类型 T 的元素
//由于 C++ 模板需要在编译时完全可见，以便编译器可以生成所有必要的实例代码，
//所以模板类和函数的实现一般位于头文件中。这使得在程序的任何地方都可以包含这个头文件来使用模板
class CQueue
{//线程安全的队列（利用IOCP实现）
public:
	enum {
		EQNone,
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};
	typedef struct IocpParam {
		int nOperator;//操作
		T Data;//数据
		HANDLE hEvent;//pop操作需要
		IocpParam(int op, const T& data,HANDLE hEve = NULL) {
			nOperator = op;
			Data = data;
			hEvent = hEve;//存储时间句柄
		}
		IocpParam() {
			nOperator = EQNone;
		}
	}PPARAM;//Post Parameter 用于投递信息的结构体
public:
	CQueue() {
		m_lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompeletion != NULL) {
			m_hThread = (HANDLE)_beginthread(
				&CQueue<T>::threadEntry, 
				0, m_hCompeletionPort);
		}
	}
	~CQueue() {
		if (m_lock)
			return;
		m_lock = true;
		HANDLE hTemp = m_hCompeletionPort;
		PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);
		WaitForSingleObject(m_hThread, INFINITE);
		m_hCompeletionPort = NULL;//防御性编程
		CloseHandle(hTemp);
	}
	bool void PushBack(const T& data) {
		if (m_lock)
			return false;
		IocpParam* pParam = new IocpParam(EQPush, data);//创建iocpparam示例
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), 
			(ULONG_PTR)pParam,NULL);
		if (ret == false)
			delete pParam;
		return ret;
	}
	bool PopFront(T& data) {

		HANDLE hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);//用于同步等待pop操作完成
		IocpParam* pParam(EQPop, data,hEvent);
		if (ret == false) {
			CloseHandle(hEvent);
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM),
			(ULONG_PTR)&pParam, NULL);
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;//等待成功
		if (ret) {
			data = pParam.Data;
		}
		return ret;
	}
	size_t Size() {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//用于同步等待pop操作完成,初始状态为非信号态
		IocpParam* pParam(EQSize, T(), hEvent);
		if (m_lock)
			if (hEvent)CloseHandle(hEvent);
			return -1;
			//操作完成后，hEvent设置为信号态
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM),
			(ULONG_PTR)&pParam, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return -1;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;//等待成功
		if (ret) {
			return pParam.nOperator;
		}
		return ret;
	}
	void Clear() {
		if (m_lock)
			return false;
		IocpParam* pParam = new IocpParam(EQClear, T());//创建iocpparam示例,调用默认构造
		bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM),
			(ULONG_PTR)pParam, NULL);
		if (ret == false)
			delete pParam;
		return ret;
	}
private:
	static void threadEntry(void* arg) {
		CQueue<T>* thiz = (CQueue<T>*)arg;
		thiz->threadMain();
		_endthread();
	}
	void threadMain() {
		DWORD dwTransferred = 0;//存储传输的字节数
		PPARAM* pParam = NULL;
		ULONG_PTR CompletionKey = 0;//存储完成包的标识
		OVERLAPPED* pOverlapped = NULL;//异步（重叠）io操作
		while (GetQueuedCompletionStatus(
			m_hCompletionPort, &dwTransferred,
			&CompletionKey, &pOverlapped, INFINITE)) {
			if ((dwTransferred == 0) || (CompletionKey == NULL)) {//没有更多的io操作，终止线程
				printf("thread is prepare to exit!\r\n");
				break;
			}
			pParam = (PPARAM*)CompletionKey;
			switch (pParam->nOperator) {
			case EQPush:
				m_list.push_back(pParam->strData);
				delete pParam;
				break;
			case EQPop:
				if (m_lstData.size() > 0) {
					pParam->Data = lstString.front();
					m_lstData.pop_front();
				}
				if (pParam->hEvent != NULL)
					SetEvent(pParam->hEvent);
				break;
			case EQSize:
				pParam->nOperator = m_lstData.size();
				if (pParam->hEvent != NULL)
					SetEvent(pParam->hEvent);
				break;
			case EQClear:
				lstString.clear();
				delete pParam;
				break;
			default:
				OutputDebugString("unknown opreator!\r\n");
				break;
			}
		}
		CloseHandle(m_hCompletionPort);
	}
private:
	std::list<T> m_lstData;
	HANDLE m_hCompletionPort;
	HANDLE m_hThread;
	//定义了一个名为 m_lock 的原子变量，这个变量的类型为 bool
	std::atomic<bool> m_lock;//队列正在析构
};

