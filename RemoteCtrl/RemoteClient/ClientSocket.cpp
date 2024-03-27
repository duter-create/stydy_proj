#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//对静态成员函数显示的进行初始化
CClientSocket::CHelper CClientSocket::m_helper;//m_helper 是 CServerSocket 类的一个静态成员变量，它的类型是 CHelper。

CClientSocket* pclient = CClientSocket::getInstance();