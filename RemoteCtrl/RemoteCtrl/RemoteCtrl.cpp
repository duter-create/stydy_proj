// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>
#include <atlimage.h>
#include <winbase.h>
#include "LockDialog.h"
#include <stdio.h>
#include <io.h>
#include <list>
#include "Command.h"
#include "ClassTool.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#define INVOKE_PATH _T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe")
#define INVOKE_PATH _T("C:\\Users\\a6267\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")

CWinApp theApp;
using namespace std;
//业务和通用
bool ChooseAutoInvoke(const CString& strPath) {
    TCHAR wcsSystem[MAX_PATH] = _T("");//MAX_PATH表示Windows中表示系统最长路径限制
    // CString strPath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));//指向RemoteCtrl.exe可执行文件的路径字符串
    if (PathFileExists(strPath)) {//检查strPath指定的文件是否存在，如果存在则直接退出函数
        return;
    }
    //定义一个注册表子键路径（一般用于设置程序开机自启动项）
    CString strInfo = _T("该程序只允许用于合法的用途！\r\n");
    strInfo += _T("继续运行该程序，将使得这台机器处于被监控状态\r\n");
    strInfo += _T("如果你不希望这样，请按“取消”按钮退出程序\r\n");
    strInfo += _T("按下“是”按钮，该程序将被复制到你的机器上，并随系统启动而自动运行\r\n");
    strInfo += _T("按下“否”按钮，程序只运行一次，不会再系统内留下任何东西\r\n");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret == IDYES) {
        //WriteRegisterTable(strPath);
        if (!ClassTool::WriteStartupDir(strPath)) {
            MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
    }
    else if (ret == IDCANCEL) {
        return false;
    }
    return true;
}

int main()
{
    if (ClassTool::IsAdmin()) {
        if (!ClassTool::Init())
            return 1;
        if (ChooseAutoInvoke(INVOKE_PATH) == false) {
            CCommand cmd;
            int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);//服务器创建单例
            switch (ret) {
            case -1:
                MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态！"), _T("网络初始化失败！"), MB_OK | MB_ICONERROR);
                break;
            case -2:
                MessageBox(NULL, _T("多次无法接入用户，结束程序！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
                break;
            }
        }
    else {
        if (ClassTool::RunAsAdmin() == false) {
            ClassTool::ShowError();
            return 1;
        }
    }
    return 0;
}    