#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//对静态成员函数显示的进行初始化
CClientSocket::CHelper CClientSocket::m_helper;//m_helper 是 CServerSocket 类的一个静态成员变量，它的类型是 CHelper。

CClientSocket* pclient = CClientSocket::getInstance();


std::string CClientSocket::GetErrorInfo(int wsaErrCode)
{
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
