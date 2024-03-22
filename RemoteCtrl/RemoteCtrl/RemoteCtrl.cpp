// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
void Dump(BYTE* pData, size_t nSize)//将提供的字节数组（BYTE* pData）转储成一个十六进制字符串，并通过 OutputDebugStringA 输出调试信息
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

int MakeDriverInfo() {//创建裆前系统可用的磁盘分区信息,1代表A盘，2代表B盘...26代表Z盘
    std::string result;//存储结果字符串
    for (int i = 1; i <= 26; i++) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0)
                result += ',';
            result += 'A' + i - 1;
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());//创建CPacket实例打包用的
    Dump((BYTE*)pack.Data(), pack.Size() + 6);//输出pack的数据内容
    //CServerSocket::getInstance()->Send(pack);
    return 0;
}
#include <stdio.h>
#include <io.h>
#include <list>

typedef struct file_info{
    file_info() {
        IsInvalid = FALSE;
        IsDirectory = -1;
        HasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    }
    BOOL IsInvalid;//是否有效
    char szFileName[256];//文件名
    BOOL HasNext;//是否还有后续，0没有1有
    BOOL IsDirectory;//是否为目录，0否1是

}FILEINFO,*PFILEINFO;

int MakeDirectoryInfo() {//用来收集特定路径下的文件和目录信息，并在发生错误的时候输出调试信息
    std::string strPath;
    //std::initializer_list<FILEINFO> lstFileInfos;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前的命令，不是获取文件列表，命令解析错误"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
        FILEINFO finfo;
        finfo.IsInvalid = TRUE;
        finfo.IsDirectory = TRUE;
        finfo.HasNext = FALSE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
        //lstFileInfos.pushback(finfo);
        CPacket pack(2, (BYTE*) & finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限访问目录"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件"));
        return -3;
    }
    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
    } while (!_findnext(hfind, &fdata));
    //发送信息到客户端 
    FILEINFO finfo;
    finfo.HasNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

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
            ////1 进度的可控性 2 对接的方便性 3 可行性评估，提早暴漏风险
            //// TODO: socket,bind,listen,accept,read,write,close
            ////套接字初始化
            //CServerSocket* pserver = CServerSocket::getInstance();//服务器创建单例
            //int count = 0;
            //if (pserver->InitSocker() == false) {//初始化
            //    MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态！"), _T("网络初始化失败！"), MB_OK | MB_ICONERROR);
            //    exit(0);
            //}
            //while (CServerSocket::getInstance() != NULL) {
            //    if (pserver->AcceptClient() == false) {
            //        if (count >= 3) {
            //            MessageBox(NULL, _T("多次无法接入用户，结束程序！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
            //            exit(0);
            //        }
            //        MessageBox(NULL, _T("无法正常接入用户，自动重试"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
            //        count++;
            //    }
            //    int ret = pserver->DealCommand();
            //    //TODO:
            //}
            ////静态变量
            ////在第一次调用的时候初始化，在程序销毁的时候被销毁
            ////全局静态变量
            ////在main函数之前被初始化，在整个程序结束之后被析构
            int nCmd = 1;
            switch (nCmd) {
            case 1://查看磁盘分区
                MakeDriverInfo();
                break;
            case 2://查看指定目录下的文件
                MakeDirectoryInfo();
                break;
            }

        }
    }
    else
    { 
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
