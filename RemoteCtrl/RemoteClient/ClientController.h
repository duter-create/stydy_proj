#pragma once
#include "ClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>
#include "resource.h"
#include "ClassTool.h"

#define WM_SEND_PACK (WM_USER+1)//���Ͱ�����
#define WM_SEND_DATA (WM_USER+2)//��������
#define WM_SHOW_STATUS (WM_USER+3)//չʾ״̬
#define WM_SHOW_WATCH (WM_USER+4)//Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ����

//ҵ���߼������̣�����ʱ���ܷ����ı�ģ�����
//ҵ���߼������̣�����ʱ���ܷ����ı�ģ�����
//ҵ���߼������̣�����ʱ���ܷ����ı�ģ�����

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
	//��������������ĵ�ַ
	void UpdateAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}
	bool SendPacket(const CPacket& pack) {
		CClientSocket* pClient = CClientSocket::getInstance();
		if (pClient->InitSocket() == false)
			return false;
		pClient->Send(pack);
	}
	//1 ���Բ鿴���̷��� 
//2 �鿴ָ��Ŀ¼�µ��ļ�
//3 ���ļ�
//4 �����ļ�
//5 ������
//6 ������Ļ����
//7 ����
//8 ���� 
//9 ɾ���ļ�
//1981 ��������
//����ֵ������ţ����С��0���Ǵ���
	int SendCommandPacket(int nCmd,
		bool bAutoClose = true,
		BYTE* pData = NULL,
		size_t nLength = 0);
	
	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return ClassTool::Bytes2Image(image, pClient->GetPacket().strData.c_str());	
	}


	int DownFile(CString strPath);

	void StartWatchScreen();
protected:
	void threadWatchScreen();
	static void threadWatchScreen(void* arg);

	void threadDownloadFile();
	static void threadDownloadEntry(void* arg);
	CClientController():
		m_statusDlg(&m_remoteDlg),m_watchDlg(&m_remoteDlg) 
	{
		bool m_isClose = true;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_hThreadDownload = INVALID_HANDLE_VALUE;
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
		TRACE("CClientSocket has been called\r\n");
		if (m_instance != NULL) {
			delete m_instance;
			m_instance = NULL;
			TRACE("CClientController has released\r\n");
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
	HANDLE m_hThreadDownload;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//�����Ƿ�ر�
	CString m_strRemote;//�����ļ���Զ��·��
	CString m_strLocal;//�����ļ��ı��ر���·��
	unsigned m_nThreadID;//�洢�߳�ID
	static CClientController* m_instance;//ʹ�� static ��ʾ��������Ǹ��������ʵ������ģ�������ÿ��ʵ��ӵ���Լ��Ŀ�������ȷ�������۴������ٸ� CClientController ����m_instance ��ָ��ͬһ�������ַ��
	class CHelper {//�ڵ���ģʽ�й����������ڣ�ȷ��������ʵ����ȷ���ڳ������ʱ������
	public:
		CHelper() {//���� CClientController ��������������
			//CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}

	};
	static CHelper m_helper;
};