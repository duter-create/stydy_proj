#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//�Ծ�̬��Ա������ʾ�Ľ��г�ʼ��
CClientSocket::CHelper CClientSocket::m_helper;//m_helper �� CServerSocket ���һ����̬��Ա���������������� CHelper��

CClientSocket* pclient = CClientSocket::getInstance();