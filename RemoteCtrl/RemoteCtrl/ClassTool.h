#pragma once
#include "framework.h"
#include <string>
class ClassTool
{
public:
    //
    static void Dump(BYTE* pData, size_t nSize)//将提供的字节数组（BYTE* pData）转储成一个十六进制字符串，并通过 OutputDebugStringA 输出调试信息
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

    static bool IsAdmin() {//检查当前进程是否有管理员权限
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {//尝试打开当前进程访问令牌
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;//接收令牌的提升状态信息
        DWORD len = 0;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
            //拿info，检查进程访问令牌权限是否被提升（是否拥有管理员权限）
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve)) {//当前进程是否以管理员权限运行
            return eve.TokenIsElevated;
        }
        printf("length of tokeninformation is %d\r\n", len);
        return false;
    }
    static bool RunAsAdmin() {
        //获取管理员权限，使用该权限创建进程
        //本地策略组 开启Administrator账户  禁止空密码只能登陆本地控制台
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        //GetCurrentDirectory(MAX_PATH, sPath);//获取当前目录
        GetModuleFileName(NULL, sPath, MAX_PATH);
        //尝试以Administrator身份创建一个新进程
        BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
        if (!ret) {//创建新进程失败
            ShowError();//TODO:去除调试信息
            MessageBox(NULL, sPath, _T("创建进程失败"), 0);//TODO:去除调试信息
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    static void ShowError() {//显示windows系统调用错误
        LPWSTR lpMessageBuf = NULL;//存放错误信息
        //strerror(errno);//标准c语言库
        //从系统获取错误信息，并存放在lpMessageBuf中
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);//把错误信息发送给调试器
        MessageBox(NULL, lpMessageBuf, _T("发生错误"), 0);
        LocalFree(lpMessageBuf);//释放缓冲区
    }
    /**
    * 改bug的思路
    * 0 观察现象
    * 1 先确定范围
    * 2 分析错误的可能性
    * 3 调试或者打日志，排查错误
    * 4 处理错误
    * 5 验证/长时间验证/多次验证/多条件的验证dou'shgi
    **/

     static BOOL WriteStartupDir(const CString& strPath) {
         //通过修改开机启动文件夹来实现开机启动
         TCHAR sPath[MAX_PATH] = _T("");
         GetModuleFileName(NULL, sPath, MAX_PATH);
         return CopyFile(sPath, strPath, FALSE);
         //fopen CFile system(copy) CopyFile OpenFile  

     }
    //开机启动的时候，程序的权限是跟随启动用户的
    //如果两者的权限不一致，则会导致程序启动失败
    //开机启动对环境变量有影响，如果依赖dll（动态库），则可能启动失败
    //[复制这些dll到system32下面或者sysWOW64下面]
    //system32下面，多是64位程序，syswow64下面多是32位程序
    //【使用静态库，而非动态库】
     static bool WriteRegisterTable(const CString& strPath) {
         //通过修改注册表来实现开机启动
         CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
         TCHAR sPath[MAX_PATH] = _T("");
         GetModuleFileName(NULL, sPath, MAX_PATH);
         BOOL ret = CopyFile(sPath, strPath, FALSE);
         //fopen CFile system(copy) CopyFile OpenFile  
         if (ret == FALSE) {
             MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
             return false;
         }
         HKEY hKey = NULL;
         ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
         if (ret != ERROR_SUCCESS) {
             RegCloseKey(hKey);
             MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
             return false;
         }
         ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
         if (ret != ERROR_SUCCESS) {
             RegCloseKey(hKey);
             MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
             return false;
         }
         RegCloseKey(hKey);
         return true;
     }
     static bool Init() {
         //用于带MFC项目初始化（通用）
         HMODULE hModule = ::GetModuleHandle(nullptr);
         if (hModule == nullptr) {
             wprintf(L"错误: GetModuleHandle 失败\n");
             return false;
         }
         if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
         {
             // TODO: 在此处为应用程序的行为编写代码。
             wprintf(L"错误: MFC 初始化失败\n");
             return false;
         }
         return true;
     }
};
