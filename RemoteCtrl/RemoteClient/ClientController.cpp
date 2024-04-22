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
		//����ӳ���
		struct { UINT nMsg; MSGFUNC func; }MsgFuncs[] =
		{ 
			//{WM_SEND_PACK,&CClientController::OnSendPack},
			//{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatch},
			{(UINT) - 1,NULL}//������ֹ��
		};
		for (int i = 0; MsgFuncs[i].func != NULL; i++) {
			m_mapFunc.insert(std::pair < UINT, MSGFUNC >(MsgFuncs[i].nMsg,MsgFuncs[i].func));
		}
	}
	return m_instance;
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
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);//�ȴ��¼���������Ϊ�ź�״̬�����ȴ���һ���̴߳�������Ϣ������ʹ����INFINITE��������ζ�Ż����޵ȴ�ֱ����Ϣ������
	return info.result;//���ش�����
}

int CClientController::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength, std::list<CPacket>* plstPacks)
{
	TRACE("cmd:%d %s start %lld \r\n",nCmd,__FUNCTION__,GetTickCount64());
	CClientSocket* pClient = CClientSocket::getInstance();
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//��Ӧ��ֱ�ӷ��ͣ�Ӧ��Ͷ�����
	std::list<CPacket> lstPacks;//Ӧ������
	if (plstPacks == NULL)
		plstPacks = &lstPacks;
	pClient->SendPacket(CPacket(nCmd, pData, nLength,hEvent),*plstPacks);//Ԥ����plstPacks�����������Ӧ���ݰ�������еĻ�
	CloseHandle(hEvent);//�����¼��������ֹ��Դ�ľ�
	if (plstPacks->size() > 0) {
		TRACE("%s start %lld \r\n", __FUNCTION__, GetTickCount64());
		return plstPacks->front().sCmd;
	}
	TRACE("%s start %lld \r\n", __FUNCTION__, GetTickCount64());
	return -1;
}

int CClientController::DownFile(CString strPath)
{
	CFileDialog dlg(FALSE, NULL,
		strPath,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, &m_remoteDlg);
	//��ģ̬��ʽ��ʾ�Ի�������û���������桱��ť��ͨ����ӦΪ IDOK��
	// �� DoModal �����᷵�� IDOK�����뽫���뵽 if ������ִ��
	if (dlg.DoModal() == IDOK) {
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		m_hThreadDownload = (HANDLE)_beginthread(&CClientController::threadDownloadEntry, 0, this);
		//1 ��Ϊ�߳���ڵ㣬��������������߳���ִ��
		//2 0�ǳ�ʼ�̶߳�ջ��С�Ĳ�������ֵ0��ʾʹ��Ĭ�ϵĴ�С
		//3 ���ݸ��̵߳Ĳ���������������£�this ָ��ָ��ǰ����ִ�� _beginthread ���õ���ʵ�� CRemoteClientDlg ����

		if (WaitForSingleObject(m_hThreadDownload, 0) != WAIT_TIMEOUT) {
			return -1;
		}
		m_remoteDlg.BeginWaitCursor();
		m_statusDlg.m_info.SetWindowText(_T("��������ִ����"));//SetWindowText ������������ m_info �ؼ����ı����ݡ��������������Ϊ��ʾ "��������ִ����"
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);
		m_statusDlg.SetActiveWindow();//ʹ m_dlgStatus �Ի����Ϊ��ǰ�����,���Ի��������Ļ����ǰ��
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
	WaitForSingleObject(m_hThreadWatch, 500);//ͬ�������������ȴ� _beginthread �������߳���ɡ�����߳���500�����ڽ����������᷵�أ�����߳�û����ָ��ʱ���ڽ���������Ҳ�᷵�أ����������ڵصȴ�
}

void CClientController::threadWatchScreen()
{
	Sleep(50);
	while (!m_isClosed) {
		if (m_watchDlg.isFull()==false) {
			std::list<CPacket> lstPacks;
			int ret = SendCommandPacket(6,true,NULL,0,&lstPacks);
			if (ret == 6) {
				if (ClassTool::Bytes2Image(m_watchDlg.GetImage(), lstPacks.front().strData) == 0) {
					m_watchDlg.SetImageStatus(true);
					TRACE("�ɹ�����ͼƬ\r\n");
				}
				else {
					TRACE("��ȡͼƬʧ�� ret = %d\r\n",ret);
				}
			}
		}
		Sleep(1);
	}
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
		AfxMessageBox(_T("����û��Ȩ�ޱ�����ļ��������ļ��޷�����"));
		m_statusDlg.ShowWindow(SW_HIDE);//�� m_dlgStatus �Ի�����������
		m_remoteDlg.EndWaitCursor();
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	do {
		int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)m_strRemote,
			m_strRemote.GetLength());
		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
		if (nLength == 0) {
			AfxMessageBox("�ļ�����Ϊ0�����޷���ȡ�ļ�");
			break;
		}
		long long nCount = 0;
		while (nCount < nLength) {//�ѽ��յ�������δ�ﵽ�ļ���������
			ret = pClient->DealCommand();
			if (ret < 0) {
				AfxMessageBox("����ʧ��");
				TRACE("����ʧ��: ret = %d\r\n", ret);
				break;
			}
			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
			nCount += pClient->GetPacket().strData.size();//�����Ѿ����յ���������
		}
	} while (false);
	fclose(pFile);
	pClient->CloseSocket();
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("�������"), _T("���"));
}

void CClientController::threadDownloadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadDownloadFile();
	_endthread();
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
	char* pBuffer = (char*)wParam;//Ҫ�������ݵĻ�������ָ��
	return pClient->Send(pBuffer,(int)lParam);//Ҫ���͵����ݵĴ�С�򳤶�
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
