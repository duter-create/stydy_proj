#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//对静态成员函数显示的进行初始化
CClientSocket::CHelper CClientSocket::m_helper;//m_helper 是 CServerSocket 类的一个静态成员变量，它的类型是 CHelper。

CClientSocket* pclient = CClientSocket::getInstance();


std::string CClientSocket::GetErrorInfo(int wsaErrCode)
{//根据 Windows Sockets API (Winsock) 的错误码获取对应的错误描述文本的。
	std::string ret;//存储最终错误信息
	LPVOID lpMsgBuf = NULL;//用于指向缓冲区的指针
	FormatMessage(//为错误消息文本分配一个缓冲区
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;//错误信息赋值给ret
	LocalFree(lpMsgBuf);//释放缓冲区
	return ret;
}

bool CClientSocket::InitSocket()
{
	if (m_sock != INVALID_SOCKET)
		CloseSocket();
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == -1)return false;
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	TRACE("addr %08X nIP %08X\r\n", inet_addr("127.0.0.1"), m_nIP);
	serv_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_adr.sin_addr.s_addr = htonl(m_nIP);
	serv_adr.sin_port = htons(m_nPort);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
		AfxMessageBox("指定的IP地址，不存在");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1) {
		AfxMessageBox("连接失败!");
		TRACE("连接失败：%d%s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
		return false;
	}
	TRACE("socket init done!\r\n");
	return true;
}

bool CClientSocket::SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed,WPARAM wParam)
{
	UINT nMode = isAutoClosed ? CSM_AUTOCLOSE : 0;
	std::string strOut;
	pack.Data(strOut);
	PACKET_DATA* pData = new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam);
	bool ret =  PostThreadMessage(m_nThreadID, WM_SEND_PACK, (WPARAM)pData, (LPARAM)hWnd);
	if (ret == false) {
		delete pData;//避免post失败后的内存泄露
	}
	return ret;
}

/*
bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
{
	if (m_sock == INVALID_SOCKET && m_hThread == INVALID_HANDLE_VALUE) {
		//	if (InitSocket() == false)
		//		return false;
		m_hThread = (HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
		TRACE("start thread\r\n");
	}
	m_lock.lock();
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPacks));
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));


	m_lstSend.push_back(pack);
	m_lock.unlock();
	TRACE("cmd:%d event %08X thread id%d\r\n", pack.sCmd, pack.hEvent, GetCurrentThreadId());
	WaitForSingleObject(pack.hEvent, INFINITE);//无限等待
	TRACE("cmd:%d event %08X thread id%d\r\n", pack.sCmd, pack.hEvent, GetCurrentThreadId());
	std::map<HANDLE, std::list<CPacket>&>::iterator it;
	it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end()) {
		m_lock.lock();
		m_mapAck.erase(it);
		m_lock.unlock();
		return true;
	}
	return false;
}
*/
CClientSocket::CClientSocket(const CClientSocket& ss) {
	m_hThread = INVALID_HANDLE_VALUE;
	m_sock = ss.m_sock;
	m_nIP = ss.m_nIP;
	m_nPort = ss.m_nPort;
	m_bAutoClose = ss.m_bAutoClose;
	std::map<UINT, CClientSocket::MSGFUNC>::const_iterator it = ss.m_mapFunc.begin();
	for (; it != ss.m_mapFunc.end(); it++) {
		m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
	}
}

CClientSocket::CClientSocket() :
	m_nIP(INADDR_ANY), m_nPort(0), m_sock(INVALID_SOCKET),
	m_bAutoClose(true), m_hThread(INVALID_HANDLE_VALUE)
{
	if (InitSockEnv() == FALSE) {
		MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_eventInvoke = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
	if ((WaitForSingleObject(m_eventInvoke, 100)) == WAIT_TIMEOUT){
		TRACE("网络消息处理线程启动失败了!\r\n");
	}
	CloseHandle(m_eventInvoke);
	m_buffer.resize(BUFFER_SIZE);
	memset(m_buffer.data(), 0, BUFFER_SIZE);
	struct {
		UINT message;
		MSGFUNC func;
	}funcs[] = {
		{WM_SEND_PACK,&CClientSocket::SendPack},
		//{WM_SEND_PACK,},
		{0,NULL}
	};
	for (int i = 0; funcs[i].message != 0; i++) {
		if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false) {
			TRACE("插入失败，消息值：%d 函数值：%08X 序号：%d\r\n", funcs[i].message, funcs[i].func, i);
		}
	}
}

unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc2();
	_endthreadex(0);
	return 0;
}
/*
void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	InitSocket();
	while (m_sock != INVALID_SOCKET) {
		if (m_lstSend.size() > 0) {
			TRACE("lstSend size: %d\r\n", m_lstSend.size());
			m_lock.lock();
			CPacket& head = m_lstSend.front();
			m_lock.unlock();
			if (Send(head) == false) {
				TRACE("发送失败\r\n");
				continue;
			}
			std::map<HANDLE, std::list<CPacket>&>::iterator it;
			it = m_mapAck.find(head.hEvent);
			if (it != m_mapAck.end()) {
				std::list<CPacket> lstRecv;
				std::map<HANDLE, bool>::iterator it0 = m_mapAutoClosed.find(head.hEvent);
				do {
					int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
					if (length > 0 || index > 0) {
						index += length;
						size_t size = (size_t)index;
						CPacket pack((BYTE*)pBuffer, size);
						if (size > 0) {//TODO:对于文件夹信息获取，文件信息获取可能产生问题
							pack.hEvent = head.hEvent;
							it->second.push_back(pack);
							memmove(pBuffer, pBuffer + size, index - size);
							index -= size;
							if (it0->second) {
								SetEvent(head.hEvent);
								break;
							}
						}
					}
					else if (length <= 0 && index <= 0) {
						CloseSocket();
						SetEvent(head.hEvent);//等到服务器关闭命令之后,在通知事情完成
						if (it0 != m_mapAutoClosed.end()) {
							TRACE("SetEvent %d %d\r\n", head.sCmd, it0->second);
						}
						else {
							TRACE("异常的情况，没有对应的pair\r\n");
						}
						break;
					}
				} while (it0->second == false || index >0);
			}
			m_lock.lock();
			m_lstSend.pop_front();
			m_mapAutoClosed.erase(head.hEvent);
			m_lock.unlock();
			if (InitSocket() == false) {
				InitSocket(); 
			}
		}
		Sleep(1);
	}
	CloseSocket();
}
*/
void CClientSocket::threadFunc2()
{
	SetEvent(m_eventInvoke);
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {//::表示全局作用域
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		//检查msg.message是否存在于m_mapFunc映射中
		//如果存在，就使用存储在映射中的函数指针来调用适当的成员函数，并将消息的三个关键成分作为参数传递给该函数
		if (m_mapFunc.find(msg.message) != m_mapFunc.end()) {
			(this->*m_mapFunc[msg.message])(msg.message,msg.wParam,msg.lParam);

		}
	}
}

bool CClientSocket::Send(const CPacket& pack)
{
	TRACE("m_sock = %d\r\n", m_sock);
	if (m_sock == -1)return false;
	std::string strOut;
	pack.Data(strOut);
	return (send(m_sock, strOut.c_str(), strOut.size(), 0)) > 0;
}

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{//TODO:定义一个消息的数据结构(数据和数据长度，模式) 回调消息的数据结构（HWND）
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;//上来就复制构造，防止内存泄漏
	HWND hWnd = (HWND)lParam;
	if (InitSocket() == true) {
		int ret = send(m_sock, (char*)data.strData.c_str(),(int)data.strData.size(), 0);
		if (ret > 0) {
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while (m_sock != INVALID_SOCKET) {
				int length = recv(m_sock,pBuffer+index ,BUFFER_SIZE - index , 0);
				if (length > 0 || (index>0)) {
					index += (size_t)length;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0) {
						TRACE("ack pack %d to hWnd %08X %d %d\r\n", pack.sCmd, hWnd, index, nLen);
						TRACE("%04X\r\n", *(WORD*)(pBuffer + nLen));
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack),data.wParam);
						if (data.nMode & CSM_AUTOCLOSE) {						
							CloseSocket();
							return;
						}
					}
					index -= nLen;
					memmove(pBuffer, pBuffer + index, nLen);
				}
				else {//TODO:对方关闭了套接字或者网络设备异常
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK,NULL, 1);
				}
			}
		}
		else {
			CloseSocket();
			//网络终止
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else {
		//TODO:错误处理
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -2);
	}
}

void Dump(BYTE* pData, size_t nSize)//将提供的字节数组（BYTE* pData）转储成一个十六进制字符串，并通过 OutputDebugStringA 输出调试信息
//Dump 函数通常用于调试目的，它使得开发者可以在调试时轻松地以可读的十六进制格式查看内存数据。
{
	std::string strOut;//存转的字符串
	for (size_t i = 0; i < nSize; i++) {
		char buf[8] = "";
		if (i > 0 && (i % 16 == 0))//每16字节加一个换行符
			strOut += "\n";
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
}