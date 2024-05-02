// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "Command.h"
#include "ClassTool.h"
#include <conio.h>
#include "CQueue.h"

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
        return true;
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
#define IOCP_LIST_EMPTY 0//置空
#define IOCP_LIST_PUSH 1
#define IOCP_LIST_POP 2

enum {
    IocplISTEmpty,
    IocpListPush,
    IocpListPop
};

typedef struct IocpParam {
    int nOperator;//操作
    std::string strData;//数据
    _beginthread_proc_type cbFunc;//回调
    IocpParam(int op, const char* sData, _beginthread_proc_type cb = NULL) {
        nOperator = op;
        strData = sData;
        cbFunc = cb;
    }
    IocpParam() {
        nOperator = -1;
    }
}IOCP_PARAM;

void threadmain(HANDLE hIOCP) {
    std::list<std::string> lstString;
    DWORD dwTransferred = 0;//存储传输的字节数
    ULONG_PTR CompletionKey = 0;//存储完成包的标识
    OVERLAPPED* pOverlapped;//异步（重叠）io操作
    int count = 0, count0 = 0;
    //在一个无限循环等待IOCP投递的数据
    //根据收到的指令操作一个字符串列表，其中再弹出操作中调用回调函数
    while (GetQueuedCompletionStatus(hIOCP, &dwTransferred, &CompletionKey, &pOverlapped, INFINITE)) {
        if ((dwTransferred == 0) || (CompletionKey == NULL)) {//没有更多的io操作，终止线程
            printf("thread is prepare to exit!\r\n");
            break;
        }
        IOCP_PARAM* pParam = (IOCP_PARAM*)CompletionKey;
        if (pParam->nOperator == IocpListPush) {
            lstString.push_back(pParam->strData);
            count0++;
        }
        else if (pParam->nOperator == IocpListPop) {
            std::string* pStr = NULL;
            if (lstString.size() > 0) {
                pStr = new std::string(lstString.front());
                lstString.pop_front();
            }
            if (pParam->cbFunc) {
                pParam->cbFunc(pStr);
            }
            count++;
        }
        else if (pParam->nOperator == IocplISTEmpty) {
            lstString.clear();
        }
        delete pParam;
        printf("count %d OUnt0 %d\r\n", count, count0);
    }
}

void threadQueueEntry(HANDLE hIOCP) {
    threadmain(hIOCP);
    _endthread();//代码到此为止，会导致本地对象无法调用析构，从而导致内存发生泄漏
}

void func(void* arg) {
    std::string* pstr = (std::string*)arg;
    if (pstr != NULL) {
        printf("pop from list:%s\r\n", pstr->c_str());
        delete pstr;
    }
    else {
        printf("list is empty,no data\r\n");
    }
}
int main()
{
    if (!ClassTool::Init())
        return 1;
    printf("press any key to exit ...\r\n");
    CQueue <std::string> lstStrings;
    ULONGLONG tick0 = GetTickCount64(), tick = GetTickCount64();
    while (_kbhit() == 0) {//定期检查按键是否被按下
        //完成端口 把请求和实现分离了
        if (GetTickCount64() - tick0 > 1300) {//读
            lstStrings.PushBack("hello world");
            tick0 = GetTickCount64();
        }
        if (GetTickCount64() - tick > 2000) {//写
            std::string str;
            lstStrings.PopFront(str);
            printf("pop from queue:%s\r\n", str.c_str());
        }
        Sleep(1);
    }
   
    printf("exit done! size %d\r\n",lstStrings.Size());
    lstStrings.Clear();
    printf("exit done! size %d\r\n", lstStrings.Size());
    ::exit(0);

    /*
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
    */
    return 0;
}    