#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//�Ծ�̬��Ա������ʾ�Ľ��г�ʼ��
CClientSocket::CHelper CClientSocket::m_helper;//m_helper �� CServerSocket ���һ����̬��Ա���������������� CHelper��

CClientSocket* pclient = CClientSocket::getInstance();


std::string CClientSocket::GetErrorInfo(int wsaErrCode)
{//���� Windows Sockets API (Winsock) �Ĵ������ȡ��Ӧ�Ĵ��������ı��ġ�
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

void Dump(BYTE* pData, size_t nSize)//���ṩ���ֽ����飨BYTE* pData��ת����һ��ʮ�������ַ�������ͨ�� OutputDebugStringA ���������Ϣ
//Dump ����ͨ�����ڵ���Ŀ�ģ���ʹ�ÿ����߿����ڵ���ʱ���ɵ��Կɶ���ʮ�����Ƹ�ʽ�鿴�ڴ����ݡ�
{
	std::string strOut;//��ת���ַ���
	for (size_t i = 0; i < nSize; i++) {
		char buf[8] = "";
		if (i > 0 && (i % 16 == 0))//ÿ16�ֽڼ�һ�����з�
			strOut += "\n";
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
}