// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>
#include <atlimage.h>
#include <winbase.h>

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
    if (_chdir(strPath.c_str()) != 0) {//更改当前工作目录为strpath指向的路径
        FILEINFO finfo;//当目录由于权限不足无法切换时
        finfo.IsInvalid = TRUE;//文件信息无效
        finfo.IsDirectory = TRUE;//是目录
        finfo.HasNext = FALSE;//没有更多的信息要发送
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());// 将 strPath 中的字符串复制到 finfo.szFileName 中，作为无法访问的路径名
        //lstFileInfos.pushback(finfo);
        CPacket pack(2, (BYTE*) & finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限访问目录"));
        return -2;
    }
    _finddata_t fdata;//存储文件查找信息
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件"));
        return -3;
    }
    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;//判断当前处理的文件项是不是目录
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
    } while (!_findnext(hfind, &fdata));//获取下一个文件项信息
    //发送信息到客户端 
    FILEINFO finfo;
    finfo.HasNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int RunFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);//将在用户的机器上运行一个程序或打开一个文件，具体行为取决于文件类型的关联程序。比如，如果路径是一个可执行文件，则会运行该程序；如果路径是文档文件，则会打开与之关联的应用程序查看该文档。
    /*
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);：调用 Win32 API 函数 ShellExecuteA 来执行或打开 strPath 变量中的文件。此函数的参数解释如下：
    NULL：表示函数不需要使用窗口句柄。
    NULL：操作设为 NULL 表示执行文件，默认操作通常是“打开”。
    strPath.c_str()：指定要运行或打开的文件的路径。
    NULL：没有要传递给要执行的程序的参数。
    NULL：默认目录设置为 NULL，所以执行文件时会使用它的默认目录。
    SW_SHOWNORMAL：指定窗口的显示方式，SW_SHOWNORMAL 为普通窗口大小。
    */
    CPacket pack(3, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

//#pragma warning(disable:4966)
int DownloadFile() {//下载文件，即把文件从服务端发送到客户端
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    long long data = 0;
    FILE* pFile = NULL;
    errno_t err = fopen_s(&pFile,strPath.c_str(),"rb");//rb:二进制读取模式，文件指针pfile
    if (err != 0) {//打开文件失败
        CPacket pack(4, (BYTE*)&data,8);
        CServerSocket::getInstance()->Send(pack);
        return -1;
    }
    if (pFile != NULL) {
        fseek(pFile, 0, SEEK_END);//用fseek函数将文件指针pfile移动到文件末尾
        data = _ftelli64(pFile);//_ftelli64：获取文件大小
        CPacket head(4, (BYTE*)&data, 8);
        fseek(pFile, 0, SEEK_SET);//用fseek函数将文件指针pfile定位到文件起始位置
        char buffer[1024] = "";
        size_t rlen = 0;//fread返回值
        do {
            rlen = fread(buffer, 1, 1024, pFile);//在buffer里读，一次读1字节，读1024次
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        fclose(pFile);
    }
    CPacket pack(4, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
} 

int MouseEvent() {//处理鼠标事件
    MOUSEEV mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
        DWORD nFlags = 0;//根据鼠标的按钮和操作设置标志位

        switch (mouse.nButton) {
        case 0://左键
            nFlags = 1;
            break;
        case 1://右键
            nFlags = 2;
            break;
        case 2://中键
            nFlags = 4;
            break;
        case 4://没有按键
            nFlags = 8;
            break;
        }
        if (nFlags != 8)
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//将光标移动到鼠标事件指定的屏幕坐标
        switch (mouse.nAction)
        {
            //nFlags 之前存储了哪个鼠标按钮被按下的信息，通过|=再加上鼠标动作的信息（二进制高位低位）
        case 0://单击
            nFlags |= 0x10;
            break;
        case 1://双击
            nFlags |= 0x20;
            break;
        case 2://按下
            nFlags |= 0x40;
            break;
        case 3://放开
            nFlags |= 0x80;
            break;
        default:
            break;
        }
        switch (nFlags)
        {
        case 0x21://左键双击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://左键单击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0,GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP,  0, 0, 0,GetMessageExtraInfo());
            break;
        case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://左键放开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://右键双击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12://右键单击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://右键按下
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://右键放开
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24://中键双击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14://中键单击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;

        case 0x44://中键按下
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://中键放开
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08://单纯的鼠标移动
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        }
        CPacket pack(4, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
    }
    else {
        OutputDebugString (_T("获取鼠标参数失败"));
        return -1;
            
    }

    return 0;
}

int SendScreen() {
    CImage screen;//GDI(全局设备接口)
    HDC hScreen = ::GetDC(NULL);
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, 1920, 1020, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == NULL)
        return -1;
    IStream* pStream = NULL;
    HRESULT ret = CreateStreamOnHGlobal(hMem,TRUE,&pStream);
    if (ret == S_OK) {
        screen.Save(pStream, Gdiplus::ImageFormatPNG);
        LARGE_INTEGER bg = { 0 };
        pStream->Seek(bg, STREAM_SEEK_SET, NULL);
        PBYTE pData = (BYTE*)GlobalLock(hMem);
        SIZE_T nSize = GlobalSize(hMem);
        CPacket pack(6, pData,nSize);
        CServerSocket::getInstance()->Send(pack);
        GlobalUnlock(hMem);
    }

    //screen.Save(_T("test2023.png"),Gdiplus::ImageFormatPNG);
    /*
    TRACE("png %d\r\n", GetTickCount64() - tickpng);
    DWORD tickjpg = GetTickCount64();
    screen.Save(_T("test2023.jpg"),Gdiplus::ImageFormatPNG);
    TRACE("jpg %d\r\n", GetTickCount64() - tickjpg);
    */
    pStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();
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
            int nCmd = 6;
            switch (nCmd) {
            case 1://查看磁盘分区
                MakeDriverInfo();
                break;
            case 2://查看指定目录下的文件
                MakeDirectoryInfo();
                break;
            case 3://打开文件
                RunFile();
                break;
            case 4:// 下载文件
                DownloadFile();
                break;
            case 5://鼠标操作
                MouseEvent();
                break;
            case 6://发送屏幕内容，即发送屏幕的截图
                SendScreen();
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
