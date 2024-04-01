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