#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
#include "Packet.h"

typedef void (*SOCKET_CALLBACK)(void*,int,std::list<CPacket>&,CPacket&);

class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {//静态函数没有this指针，不能直接访问成员变量
			m_instance = new CServerSocket();
		}
		return m_instance; 
	}


	int Run(SOCKET_CALLBACK callback, void* arg,short port = 9527) {
		//1 进度的可控性 2 对接的方便性 3 可行性评估，提早暴漏风险
		// TODO: socket,bind,listen,accept,read,write,close
		//套接字初始化
		bool ret = InitSocket(port);
		if (ret == false)
			return -1;
		std::list<CPacket> lstPackets;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true) {
			if (AcceptClient() == false) {
				if (count >= 3) {
					return -2;
				}
				count++;
			}
			int ret = DealCommand();
			if (ret > 0) {
				m_callback(m_arg, ret,lstPackets,m_packet);//m_callback(m_arg, ret); 会调用回调函数，并向其中传递两个参数：一个是与特定操作相关的数据（m_arg），另一个是最近一次操作的结果（ret）
				while (lstPackets.size() > 0) {
					Send(lstPackets.front());//发第一个数据包
					lstPackets.pop_front();
				}
			}
			CloseClient();
		}
		return 0;
	}
protected:
	bool InitSocket(short port) {
		if (m_sock == -1)return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);
		//绑定
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
			return false;
		}
		if (listen(m_sock, 1) == -1) {
			return false;
		}
		return true;
	}
	bool AcceptClient() {
		TRACE("enter AccessClient\r\n");
		sockaddr_in client_adr;
		//char buffer[1024];
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		TRACE("m_client = %d\r\n", m_client);
		if (m_client == -1)return false;
		return true;

	}
#define BUFFER_SIZE 4096
	int  DealCommand() {//处理接收到的网络命令
		if (m_client == -1)return -1;
		//char buffer[1024] = "";
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL) {
			TRACE("内存不足\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client, buffer+index, BUFFER_SIZE -index, 0);
			if (len <= 0) {
				delete[]buffer;
				return -1;
			}
			TRACE("recv %d\r\n", len);
			index += len;
			len = index;
			m_packet= CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len; 
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		delete[]buffer;
		return -1;
	}
	const bool Send(char* pData, int nSize) {
		if (m_client == -1)return false;
		return (send(m_client, pData, nSize, 0)) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1)return false;
		//Dump((BYTE*)pack.Data(), pack.Size());
		return (send(m_client, pack.Data(), pack.Size(), 0)) > 0;
	}
	void CloseClient() {
		if (m_client != INVALID_SOCKET) {
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}
	}
private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;//void 不能用来声明一个变量，因为它不代表任何具体的数据类型。应该使用 void*，它是一个指向 void 的指针。这通常用于泛型数据指针，可转换为任何数据类型的指针
	SOCKET m_sock;
	SOCKET m_client;	
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss) {}
	CServerSocket(const CServerSocket& ss) {
		m_client = ss.m_client;
		m_sock = ss.m_sock;
	}
	CServerSocket() {
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket() {
		closesocket(m_sock);
		WSACleanup();
	}
	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		else return TRUE;
	}
	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static CServerSocket* m_instance;

	class CHelper {//在单例模式中管理生命周期，确保单例的实例正确的在程序结束时被销毁
	public:
		CHelper(){
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}

	};
	static CHelper m_helper;
};


