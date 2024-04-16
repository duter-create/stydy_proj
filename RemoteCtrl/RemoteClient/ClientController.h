#pragma once
#include "ClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>
#include "resource.h"

#define WM_SEND_PACK (WM_USER+1)//���Ͱ�����
#define WM_SEND_DATA (WM_USER+2)//��������
#define WM_SHOW_STATUS (WM_USER+3)//չʾ״̬
#define WM_SHOW_WATCH (WM_USER+4)//Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ����

class CClientController
{
public:
	//��ȡȫ��Ψһ����
	//���� m_instance �Ǿ�̬�ģ���˲�����������ĺ���Ҳ��Ҫ�Ǿ�̬�ģ���Ϊ��̬����������û�����ʵ��������±����á�
	//��� getInstance() �� releaseInstance() ���Ǿ�̬�ģ�������Ҫ����һ�� CClientController �Ķ�����ܵ������ǣ�����Ҳ�Ͳ���ʵ�ֵ���ģʽ��Ŀ����
	static CClientController* getInstance();
	//��ʼ��������
	int	InitController();
	//����������
	int Invoke(CWnd*& pMainWnd);
	//������Ϣ
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

	//�̵߳���ڵ㣬���ڴ����߳����첽ִ��ĳЩ����
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
	//������һ���µ����� MSGFUNC ��Ϊָ�� CClientController ��Ա������ָ�룬�����Ա����������������
	//LRESULTͨ�������˺���ִ�еĽ���򷵻�״̬��
	//UINT nMsg������Ϣ�ı�ʶ������ WM_PAINT���ػ���Ϣ����WM_KEYDOWN�����̰�����Ϣ���ȡ�
	//WPARAM wParam �� LPARAM lParam ���������ݸ�����Ϣ��Ϣ�Ĳ�������������ȡ������Ϣ�����͡�
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lPARAM);
	//����key������Ϊ UINT��ֵ��value������Ϊǰ�涨��� MSGFUNC����Ա����ָ�룩
	static std::map<UINT,MSGFUNC>m_mapFunc;
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;//�洢�߳̾��
	unsigned m_nThreadID;//�洢�߳�ID
	static CClientController* m_instance;//ʹ�� static ��ʾ��������Ǹ��������ʵ������ģ�������ÿ��ʵ��ӵ���Լ��Ŀ�������ȷ�������۴������ٸ� CClientController ����m_instance ��ָ��ͬһ�������ַ��
	class CHelper {//�ڵ���ģʽ�й����������ڣ�ȷ��������ʵ����ȷ���ڳ������ʱ������
	public:
		CHelper() {//���� CClientController ��������������
			CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}

	};
	static CHelper m_helper;
};