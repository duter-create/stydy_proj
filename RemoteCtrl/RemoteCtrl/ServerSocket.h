#pragma once
#include "pch.h"
#include "framework.h"

#pragma pack(push)
#pragma pack(1)//告诉编译器将每个成员变量的对齐设为1字节

class CPacket
{
public:
	CPacket():sHead(0),nLength(0),sCmd(0),sSum(0) {}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {//根据给定的参数来构建CPacket对象
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		strData.resize(nSize);
		memcpy((void*)strData.c_str(), pData, nSize);
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}

	}
	CPacket(const CPacket& pack) {//拷贝构造
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {//找包头
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if ((i+4+2+2) > nSize) {//加上4字节长度字段，命令字段（2字节），校验和字段（2字节）包数据可能不全，或者包头未能全部接收到
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//包未完全接收到，就返回，解析失败
			nSize = 0;
			return;
		}

		sCmd = *(WORD*)(pData + i); i += 2;//pData是一个指针，(WORD*)是将一个指针转换成一个指向WORD类型的指针，再加*是解引用
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;//head,length,data...
			return;
		}
		else
			nSize = 0;
	}
	~CPacket() {}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		else
			return *this;
	}
	int Size() {//包数据的大小
		return nLength + 6;
	}
	const char* Data() {//将CPacket对象的各个部分组合起来，并返回指向序列开头的const char*指针
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();//对pData操作影响strOut内容
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

public:
	WORD sHead;//包头，固定位FE FF
	DWORD nLength;//包长度（从控制命令开始，到和校验结束）
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
	std::string strOut;//整个包的数据
};

class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {//静态函数没有this指针，不能直接访问成员变量
			m_instance = new CServerSocket();
		}
		return m_instance; 
	}
	bool InitSocket() {
		
		if (m_sock == -1)return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
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
		sockaddr_in client_adr;
		//char buffer[1024];
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1)return false;
		return true;

	}
#define BUFFER_SIZE 4096
	int  DealCommand() {//处理接收到的网络命令
		if (m_client == -1)return -1;
		//char buffer[1024] = "";
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client, buffer+index, BUFFER_SIZE -index, 0);
			if (len <= 0) {
				return -1;
			}
			index += len;
			len = index;
			m_packet= CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len; 
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	const bool Send(char* pData, int nSize) {
		if (m_client == -1)return false;
		return (send(m_client, pData, nSize, 0)) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1)return false;
		return (send(m_client, pack.Data(), pack.Size(), 0)) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if (m_packet.sCmd == 2) {//获取文件列表
			strPath = m_packet.strData;
			return true;
		}
		else
			return false;
	}
private:
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


