#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;

CServerSocket* CServerSocket::m_instance = NULL;//�Ծ�̬��Ա������ʾ�Ľ��г�ʼ��
CServerSocket::CHelper CServerSocket::m_helper;//m_helper �� CServerSocket ���һ����̬��Ա���������������� CHelper��