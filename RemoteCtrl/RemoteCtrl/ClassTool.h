#pragma once
#include "framework.h"
#include <string>
class ClassTool
{
public:
    //
    static void Dump(BYTE* pData, size_t nSize)//���ṩ���ֽ����飨BYTE* pData��ת����һ��ʮ�������ַ�������ͨ�� OutputDebugStringA ���������Ϣ
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

    static bool IsAdmin() {//��鵱ǰ�����Ƿ��й���ԱȨ��
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {//���Դ򿪵�ǰ���̷�������
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;//�������Ƶ�����״̬��Ϣ
        DWORD len = 0;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
            //��info�������̷�������Ȩ���Ƿ��������Ƿ�ӵ�й���ԱȨ�ޣ�
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve)) {//��ǰ�����Ƿ��Թ���ԱȨ������
            return eve.TokenIsElevated;
        }
        printf("length of tokeninformation is %d\r\n", len);
        return false;
    }
    static bool RunAsAdmin() {
        //��ȡ����ԱȨ�ޣ�ʹ�ø�Ȩ�޴�������
        //���ز����� ����Administrator�˻�  ��ֹ������ֻ�ܵ�½���ؿ���̨
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        //GetCurrentDirectory(MAX_PATH, sPath);//��ȡ��ǰĿ¼
        GetModuleFileName(NULL, sPath, MAX_PATH);
        //������Administrator��ݴ���һ���½���
        BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
        if (!ret) {//�����½���ʧ��
            ShowError();//TODO:ȥ��������Ϣ
            MessageBox(NULL, sPath, _T("��������ʧ��"), 0);//TODO:ȥ��������Ϣ
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    static void ShowError() {//��ʾwindowsϵͳ���ô���
        LPWSTR lpMessageBuf = NULL;//��Ŵ�����Ϣ
        //strerror(errno);//��׼c���Կ�
        //��ϵͳ��ȡ������Ϣ���������lpMessageBuf��
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);//�Ѵ�����Ϣ���͸�������
        MessageBox(NULL, lpMessageBuf, _T("��������"), 0);
        LocalFree(lpMessageBuf);//�ͷŻ�����
    }
    /**
    * ��bug��˼·
    * 0 �۲�����
    * 1 ��ȷ����Χ
    * 2 ��������Ŀ�����
    * 3 ���Ի��ߴ���־���Ų����
    * 4 �������
    * 5 ��֤/��ʱ����֤/�����֤/����������֤dou'shgi
    **/

     static BOOL WriteStartupDir(const CString& strPath) {
         //ͨ���޸Ŀ��������ļ�����ʵ�ֿ�������
         TCHAR sPath[MAX_PATH] = _T("");
         GetModuleFileName(NULL, sPath, MAX_PATH);
         return CopyFile(sPath, strPath, FALSE);
         //fopen CFile system(copy) CopyFile OpenFile  

     }
    //����������ʱ�򣬳����Ȩ���Ǹ��������û���
    //������ߵ�Ȩ�޲�һ�£���ᵼ�³�������ʧ��
    //���������Ի���������Ӱ�죬�������dll����̬�⣩�����������ʧ��
    //[������Щdll��system32�������sysWOW64����]
    //system32���棬����64λ����syswow64�������32λ����
    //��ʹ�þ�̬�⣬���Ƕ�̬�⡿
     static bool WriteRegisterTable(const CString& strPath) {
         //ͨ���޸�ע�����ʵ�ֿ�������
         CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
         TCHAR sPath[MAX_PATH] = _T("");
         GetModuleFileName(NULL, sPath, MAX_PATH);
         BOOL ret = CopyFile(sPath, strPath, FALSE);
         //fopen CFile system(copy) CopyFile OpenFile  
         if (ret == FALSE) {
             MessageBox(NULL, _T("�����ļ�ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n"), _T("����"), MB_ICONERROR | MB_TOPMOST);
             return false;
         }
         HKEY hKey = NULL;
         ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
         if (ret != ERROR_SUCCESS) {
             RegCloseKey(hKey);
             MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n��������ʧ�ܣ�"), _T("����"), MB_ICONERROR | MB_TOPMOST);
             return false;
         }
         ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
         if (ret != ERROR_SUCCESS) {
             RegCloseKey(hKey);
             MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n��������ʧ�ܣ�"), _T("����"), MB_ICONERROR | MB_TOPMOST);
             return false;
         }
         RegCloseKey(hKey);
         return true;
     }
     static bool Init() {
         //���ڴ�MFC��Ŀ��ʼ����ͨ�ã�
         HMODULE hModule = ::GetModuleHandle(nullptr);
         if (hModule == nullptr) {
             wprintf(L"����: GetModuleHandle ʧ��\n");
             return false;
         }
         if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
         {
             // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
             wprintf(L"����: MFC ��ʼ��ʧ��\n");
             return false;
         }
         return true;
     }
};
