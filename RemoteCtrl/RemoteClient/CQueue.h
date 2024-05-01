#pragma once
template<class T>
class CQueue
{//�̰߳�ȫ�Ķ��У�����IOCPʵ�֣�
public:
	CQueue();
	~CQueue() {}
	bool void PushBack(const T& data);
	bool void PopFrom(T& data);
	size_t Size();
	void Clear();
private:
	static void threadEntry(void* arg);
	void threadMain();
private:
	std::list<T> m_lstData;
	HANDLE m_hCompletionPort;
	HANDLE m_hThread;
public:
	typedef struct IocpParam {
		int nOperator;//����
		T strData;//����
		HANDLE hEvent;//pop������Ҫ
		IocpParam(int op, const char* sData, _beginthread_proc_type cb = NULL) {
			nOperator = op;
			strData = sData;
		}
		IocpParam() {
			nOperator = -1;
		}
	}PPARAM;//Post Parameter ����Ͷ����Ϣ�Ľṹ��
	enum {
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};
};


