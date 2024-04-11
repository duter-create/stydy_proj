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


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的应用程序对象

CWinApp theApp;
using namespace std;
int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误 
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            //1 进度的可控性 2 对接的方便性 3 可行性评估，提早暴漏风险
            // TODO: socket,bind,listen,accept,read,write,close
            //套接字初始化
            CCommand cmd;
            CServerSocket* pserver = CServerSocket::getInstance();//服务器创建单例
            int count = 0;
            if (pserver->InitSocket() == false) {//初始化
                MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态！"), _T("网络初始化失败！"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while (CServerSocket::getInstance() != NULL) {
                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
                        MessageBox(NULL, _T("多次无法接入用户，结束程序！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(NULL, _T("无法正常接入用户，自动重试"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
                    count++;
                }
                TRACE("AcceptClient return true\r\n");
                int ret = pserver->DealCommand();
                TRACE("DealCommand ret %d\r\n", ret);
                if (ret > 0) {
                    ret = cmd.ExcuteCommand(ret);//短链接，以控制为目标而不以文件传输下载为目标
                    if (ret != 0) {
                        TRACE("执行命令失败:%d ret=%d\r\n", pserver->GetPacket().sCmd, ret);
                    }
                    pserver->CloseClient();//短链接
                    TRACE("Command has done\r\n");                }
            }
            //静态变量
            //在第一次调用的时候初始化，在程序销毁的时候被销毁
            //全局静态变量
            //在main函数之前被初始化，在整个程序结束之后被析构
        }
    }
    else
    { 
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
