#include "pch.h"
#include "ClientController.h"

std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;

CClientController* CClientController::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CClientController();
		//����ӳ���
		struct { UINT nMsg; MSGFUNC func; }MsgFuncs[] =
		{ 
			{WM_SEND_PACK,&CClientController::OnSendPack},
			{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatch},
			{(UINT) - 1,NULL}//������ֹ��
		};
		for (int i = 0; MsgFuncs[i].func != NULL; i++) {
			m_mapFunc.insert(std::pair < UINT, MSGFUNC >(MsgFuncs[i].nMsg,MsgFuncs[i].func));
		}
	}
	return nullptr;
}

int CClientController::InitController()
{
	//�����߳�
	m_hThread = (HANDLE)_beginthreadex(//�̰߳�ȫ���ԡ�ջ��С���̺߳������̲߳���������ѡ����߳�IDָ��
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
	return m_remoteDlg.DoModal();//��������ʾһ��ģ̬�Ի���
}

LRESULT CClientController::SendMessage(MSG msg)
{//���� m_nThreadID ��ʶ���̷߳���һ����Ϣ
	/*
	UINT message����Ϣ�ı�ʶ�������� WM_COMMAND��
	WPARAM wParam����Ϣ�ĵ�һ��������ͨ������������Ϣ�ض�����Ϣ����˵���� ID ��ؼ��� ID��
	LPARAM lParam����Ϣ�ĵڶ���������ͨ������Я�����������Ϣ�������������λ�á�
	*/
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)
		return -2;
	MSGINFO info(msg);
	//��һ����Ϣ���͵�������һ���̵߳���Ϣ������
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, -1);
	return info.result;//���ش�����
}

void CClientController::threadFunc()
{//ʵ����Ϣѭ��
	MSG msg;//�����Ϣ�����е���Ϣ
	while (::GetMessage(&msg, NULL, 0, 0)) {//�ӵ�ǰ�̵߳���Ϣ�����м�����Ϣ
		TranslateMessage(&msg);//ת��������Ϣ
		DispatchMessage(&msg);//����Ϣ���ݸ���Ӧ�Ĵ��ڽ���
		if (msg.message == WM_SEND_MESSAGE) {
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(pmsg->msg.message);
			if (it != m_mapFunc.end()) {
				//(this->*)it->second
				pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);//�����ҵ�����Ϣ������
			}
			else {
				pmsg->result = -1;
			}
			SetEvent(hEvent);//ͨ�������¼�����֪ͨ��Ϣ�����ߣ�����Ϣ�����Ѿ����
		}
		else {
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {
				//(this->*)it->second
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);//�����ҵ�����Ϣ������
			}
		}
	}
}
unsigned __stdcall CClientController::threadEntry(void* arg)
{//�߳���ں���
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();//ִ����Ϣѭ��
	_endthreadex(0);//�����߳�
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
