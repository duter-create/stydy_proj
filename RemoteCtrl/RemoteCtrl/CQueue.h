#pragma once
#include "pch.h"
#include <atomic>

template<class T>
//CQueue ��һ��ģ���࣬���Դ洢�κ����� T ��Ԫ��
//���� C++ ģ����Ҫ�ڱ���ʱ��ȫ�ɼ����Ա�����������������б�Ҫ��ʵ�����룬
//����ģ����ͺ�����ʵ��һ��λ��ͷ�ļ��С���ʹ���ڳ�����κεط������԰������ͷ�ļ���ʹ��ģ��
class CQueue
{//�̰߳�ȫ�Ķ��У�����IOCPʵ�֣�
public:
	enum {
		EQNone,
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};
	typedef struct IocpParam {
		int nOperator;//����
		T Data;//����
		HANDLE hEvent;//pop������Ҫ
		IocpParam(int op, const T& data,HANDLE hEve = NULL) {
			nOperator = op;
			Data = data;
			hEvent = hEve;//�洢ʱ����
		}
		IocpParam() {
			nOperator = EQNone;
		}
	}PPARAM;//Post Parameter ����Ͷ����Ϣ�Ľṹ��
public:
	CQueue() {
		m_lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompeletionPort != NULL) {
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
		m_hCompeletionPort = NULL;//�����Ա��
		CloseHandle(hTemp);
	}
	bool PushBack(const T& data) {
		IocpParam* pParam = new IocpParam(EQPush, data);//����iocpparamʾ��
		if (m_lock) {
			delete pParam;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), 
			(ULONG_PTR)pParam,NULL);
		if (ret == false)
			delete pParam;
		return ret;
	}
	bool PopFront(T& data) {

		HANDLE hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);//����ͬ���ȴ�pop�������
		IocpParam pParam(EQPop, data,hEvent);
		if (m_lock) {
			if(hEvent)CloseHandle(hEvent);
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM),
			(ULONG_PTR)&pParam, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return false;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;//�ȴ��ɹ�
		if (ret) {
			data = pParam.Data;
		}
		return ret;
	}
	size_t Size() {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//����ͬ���ȴ�pop�������,��ʼ״̬Ϊ���ź�̬
		IocpParam pParam(EQSize, T(), hEvent);
		if (m_lock)
			if (hEvent)CloseHandle(hEvent);
			return -1;
			//������ɺ�hEvent����Ϊ�ź�̬
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM),
			(ULONG_PTR)&pParam, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return -1;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;//�ȴ��ɹ�
		if (ret) {
			return pParam.nOperator;
		}
		return ret;
	}
	bool Clear() {
		if (m_lock)
			return false;
		IocpParam* pParam = new IocpParam(EQClear, T());//����iocpparamʾ��,����Ĭ�Ϲ���
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM),
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
	void DealParam(PPARAM* pParam) {
		switch (pParam->nOperator) {
		case EQPush:
			m_lstData.push_back(pParam->Data);
			delete pParam;
			break;
		case EQPop:
			if (m_lstData.size() > 0) {
				pParam->Data = m_lstData.front();
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
			m_lstData.clear();
			delete pParam;
			break;
		default:
			OutputDebugStringA("unknown opreator!\r\n");
			break;
		}
	}
	void threadMain() {
		DWORD dwTransferred = 0;//�洢������ֽ���
		PPARAM* pParam = NULL;
		ULONG_PTR CompletionKey = 0;//�洢��ɰ��ı�ʶ
		OVERLAPPED* pOverlapped = NULL;//�첽���ص���io����
		while (GetQueuedCompletionStatus(
			m_hCompeletionPort, &dwTransferred,
			&CompletionKey, &pOverlapped, INFINITE)) {
			if ((dwTransferred == 0) || (CompletionKey == NULL)) {//û�и����io��������ֹ�߳�
				printf("thread is prepare to exit!\r\n");
				break;
			}
			pParam = (PPARAM*)CompletionKey;
			DealParam(pParam);
		}
		while (GetQueuedCompletionStatus(
			m_hCompeletionPort,
			&dwTransferred,
			&CompletionKey,
			&pOverlapped, 0)) {
			if ((dwTransferred == 0) || (CompletionKey == NULL)) {//û�и����io��������ֹ�߳�
				printf("thread is prepare to exit!\r\n");
				continue;
			}
			pParam = (PPARAM*)CompletionKey;
			DealParam(pParam);
		}
		CloseHandle(m_hCompeletionPort);
	}
private:
	std::list<T> m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;
	//������һ����Ϊ m_lock ��ԭ�ӱ������������������Ϊ bool
	std::atomic<bool> m_lock;//������������
};

