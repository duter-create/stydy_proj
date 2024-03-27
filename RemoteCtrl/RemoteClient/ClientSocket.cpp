#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//�Ծ�̬��Ա������ʾ�Ľ��г�ʼ��
CClientSocket::CHelper CClientSocket::m_helper;//m_helper �� CServerSocket ���һ����̬��Ա���������������� CHelper��

CClientSocket* pclient = CClientSocket::getInstance();


std::string CClientSocket::GetErrorInfo(int wsaErrCode)
{
	std::string ret;//�洢���մ�����Ϣ
	LPVOID lpMsgBuf = NULL;//����ָ�򻺳�����ָ��
	FormatMessage(//Ϊ������Ϣ�ı�����һ��������
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;//������Ϣ��ֵ��ret
	LocalFree(lpMsgBuf);//�ͷŻ�����
	return ret;
}
