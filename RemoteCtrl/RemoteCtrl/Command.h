#pragma once
#include "Resource.h"
#include <map>
#include "ServerSocket.h"
#include <atlimage.h>
#include <direct.h>
#include "ClassTool.h"
#include <stdio.h>
#include <io.h>
#include <list>
#include "LockDialog.h"
#include "Packet.h"
#pragma warning(disable:4966) //fopen sprintf strcpy strstr

class CCommand
{
public:
	CCommand();
	~CCommand() {};
	int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket,CPacket& inPacket);
    static void RunCommand(void* arg, int status,std::list<CPacket>& lstPacket, CPacket& inPacket) {
        CCommand* thiz = (CCommand*)arg;
        if (status > 0) {
            int ret = thiz->ExcuteCommand(status,lstPacket,inPacket);
            if (ret != 0) {
                TRACE("ִ������ʧ��: %d ret=%d\r\n",status,ret);
            }
        }
        else {
            MessageBox(NULL, _T("�޷����������û����Զ�����"), _T("�����û�ʧ�ܣ�"), MB_OK | MB_ICONERROR);
        }

    }
protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket& inPacket);//��Ա����ָ��
	std::map<int, CMDFUNC> m_mapFunction;//����ӳ���������ŵ����ܵ�ӳ�䣬ȡ��switch
    CLockDialog dlg;
    unsigned threadid;
protected:
    static unsigned __stdcall threadLockDlg(void* arg) {
        CCommand* thiz = (CCommand*)arg;
        thiz->threadLockDlgMain();
        _endthreadex(0);
        return 0;
    }

    void threadLockDlgMain() {
        TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
        dlg.Create(IDD_DIALOG_INFO, NULL);//����һ���� IDD_DIALOG_INFO ��ʶ�ķ�ģ̬�Ի���ʵ������������Ի�����һ���������ڣ���Ϊ��û��ָ��������
        dlg.ShowWindow(SW_SHOW);//��dlg�Ի����ڿɼ�״̬
        //�����ö�

        CRect rect;

        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
        rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
        rect.bottom = LONG(rect.bottom * 1.10);
        TRACE("right = %d bottom = %d\r\n", rect.right, rect.bottom);
        dlg.MoveWindow(rect);
        CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
        if (pText) {
            CRect rtText;
            pText->GetWindowRect(rtText);
            int nWidth = rtText.Width();//w0
            int x = (rect.right - nWidth) / 2;
            int nHeight = rtText.Height();
            int y = (rect.bottom - nHeight) / 2;
            pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
        }
        //�����ö�
        dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//�� dlg �Ի��򱣳������д��ڵ����ϲ㣬���ǲ��ı����С��λ�á�
        //������깦��
        ShowCursor(false);//���ع��
        //����������
        ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);//Ѱ������Ϊ Shell_TrayWnd���� Windows ���������Ĵ��ڣ�Ȼ��ͨ�� ShowWindow ������������Ϊ��ʾ״̬����ͨ���ᵼ��ԭ�����ص�����������ʾ������
        //���������Χ
        dlg.GetWindowRect(rect);//GetWindowRect ������ȡ�� dlg ��������Ļ�ϵ�λ�úͳߴ磬�ٸ�ֵ�� rect
        rect.left = 0;
        rect.top = 0;
        rect.right = 1;
        rect.bottom = 1;
        //������귶Χ
        ClipCursor(rect);
        MSG msg;
        /*��Ϣѭ��
        1 ��Ϣ�������ӵ�ǰ�̵߳���Ϣ�����м�����Ϣ���������Ϊ�գ�GetMessage ����ȴ���ֱ������Ϣ������
        2 ��Ϣ���룬�������������Ϣ���簴����Ϣת��Ϊ�ַ���Ϣ�������Ϳ����ڴ��ڹ�������ȷ�����������
        3 ��Ϣ���ɣ�����Ϣ�ַ�����Ӧ�Ĵ��ڹ��̣��������ʵ���ϵ��õ��Ǵ��ڹ����Ļص�����������ͨ��������Ϣ��ִ����Ӧ�Ĳ�������������
        */

        while (GetMessage(&msg, NULL, 0, 0)) {//�ӳ������Ϣ������ȡ���û�������������¼�,���������������ֱ������Ϣ��ȡ
            TranslateMessage(&msg);//������������緽��������ܼ��ȣ����ַ���Ϣ��
            DispatchMessage(&msg);//����Ϣ�ַ������ڳ���Ĵ��ڹ��̺�����������Ϣ����
            if (msg.message == WM_KEYDOWN) {
                TRACE("msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
                if (msg.wParam == 0x41) {//��ESC�˳�
                    break;
                }
            }
        }
        //�ָ���귶Χ
        ClipCursor(NULL);
        //�ָ����
        ShowCursor(true);
        //�ָ�������
        ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);//Ѱ������Ϊ Shell_TrayWnd���� Windows ���������Ĵ��ڣ�Ȼ��ͨ�� ShowWindow ������������Ϊ��ʾ״̬����ͨ���ᵼ��ԭ�����ص�����������ʾ������
        dlg.DestroyWindow();
    }
    int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket) {//������ǰϵͳ���õĴ��̷�����Ϣ,1����A�̣�2����B��...26����Z��
        std::string result;//�洢����ַ���
        for (int i = 1; i <= 26; i++) {
            int ret = _chdrive(i);
            if (ret == 0) {//�ɹ��ı䵱ǰ���������
                if (result.size() > 0)
                    result += ',';
                result += 'A' + i - 1;
            }
        }
        lstPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        return 0;
    }

    int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket) {//�����ռ��ض�·���µ��ļ���Ŀ¼��Ϣ�����ڷ��������ʱ�����������Ϣ
        std::string strPath = inPacket.strData;
        //std::initializer_list<FILEINFO> lstFileInfos;
        if (_chdir(strPath.c_str()) != 0) {//���ĵ�ǰ����Ŀ¼Ϊstrpathָ���·��
            //_chdir:���ĵ�ǰ�Ĺ���Ŀ¼
            FILEINFO finfo;//��Ŀ¼����Ȩ�޲����޷��л�ʱ
            finfo.HasNext = FALSE;
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            OutputDebugString(_T("û��Ȩ�޷���Ŀ¼"));
            return -2;
        }
        _finddata_t fdata;//�洢�ļ�������Ϣ
        int hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1) {
            OutputDebugString(_T("û���ҵ��κ��ļ�"));
            FILEINFO finfo;
            finfo.HasNext = FALSE;
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            return -3;
        }
        int count = 0;
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;//�жϵ�ǰ������ļ����ǲ���Ŀ¼
            memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
            TRACE("%s\r\n", finfo.szFileName);
            lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
            count++;
        } while (!_findnext(hfind, &fdata));//��ȡ��һ���ļ�����Ϣ
        TRACE("server: count = %d\r\n", count);
        //������Ϣ���ͻ��� 
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
        return 0;
    }

    int RunFile(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        std::string strPath = inPacket.strData;
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);//�����û��Ļ���������һ��������һ���ļ���������Ϊȡ�����ļ����͵Ĺ������򡣱��磬���·����һ����ִ���ļ���������иó������·�����ĵ��ļ���������֮������Ӧ�ó���鿴���ĵ���
        /*
        ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);������ Win32 API ���� ShellExecuteA ��ִ�л�� strPath �����е��ļ����˺����Ĳ����������£�
        NULL����ʾ��������Ҫʹ�ô��ھ����
        NULL��������Ϊ NULL ��ʾִ���ļ���Ĭ�ϲ���ͨ���ǡ��򿪡���
        strPath.c_str()��ָ��Ҫ���л�򿪵��ļ���·����
        NULL��û��Ҫ���ݸ�Ҫִ�еĳ���Ĳ�����
        NULL��Ĭ��Ŀ¼����Ϊ NULL������ִ���ļ�ʱ��ʹ������Ĭ��Ŀ¼��
        SW_SHOWNORMAL��ָ�����ڵ���ʾ��ʽ��SW_SHOWNORMAL Ϊ��ͨ���ڴ�С��
        */
        lstPacket.push_back(CPacket(3, NULL, 0));
        return 0;
    }


    int DownloadFile(std::list<CPacket>& lstPacket, CPacket& inPacket) {//�����ļ��������ļ��ӷ���˷��͵��ͻ���
        std::string strPath = inPacket.strData;
        long long data = 0;
        FILE* pFile = NULL;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");//rb:�����ƶ�ȡģʽ���ļ�ָ��pfile
        if (err != 0) {//���ļ�ʧ��
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            return -1;
        }
        if (pFile != NULL) {
            fseek(pFile, 0, SEEK_END);//��fseek�������ļ�ָ��pfile�ƶ����ļ�ĩβ
            data = _ftelli64(pFile);//_ftelli64����ȡ�ļ���С
            lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            fseek(pFile, 0, SEEK_SET);//��fseek�������ļ�ָ��pfile��λ���ļ���ʼλ��
            char buffer[1024] = "";
            size_t rlen = 0;//fread����ֵ
            do {
                rlen = fread(buffer, 1, 1024, pFile);//��buffer�����һ�ζ�1�ֽڣ���1024��
                lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
            } while (rlen >= 1024);
            fclose(pFile);
        }
        lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
        return 0;
    }

    int MouseEvent(std::list<CPacket>& lstPacket, CPacket& inPacket) {//��������¼�
        MOUSEEV mouse;
        memcpy(&mouse, inPacket.strData.c_str(), sizeof(MOUSEEV));
        DWORD nFlags = 0;//�������İ�ť�Ͳ������ñ�־λ

        switch (mouse.nButton) {
        case 0://���
            nFlags = 1;
            break;
        case 1://�Ҽ�
            nFlags = 2;
            break;
        case 2://�м�
            nFlags = 4;
            break;
        case 4://û�а���
            nFlags = 8;
            break;
        }
        if (nFlags != 8)
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//������ƶ�������¼�ָ������Ļ����
        switch (mouse.nAction)
        {
            //nFlags ֮ǰ�洢���ĸ���갴ť�����µ���Ϣ��ͨ��|=�ټ�����궯������Ϣ�������Ƹ�λ��λ��
        case 0://����
            nFlags |= 0x10;
            break;
        case 1://˫��
            nFlags |= 0x20;
            break;
        case 2://����
            nFlags |= 0x40;
            break;
        case 3://�ſ�
            nFlags |= 0x80;
            break;
        default:
            break;
        }
        TRACE("mouse event : %08X x %d y %d \r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
        switch (nFlags)
        {
        case 0x21://���˫��
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://�������
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://�������
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://����ſ�
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://�Ҽ�˫��
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12://�Ҽ�����
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://�Ҽ�����
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://�Ҽ��ſ�
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24://�м�˫��
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14://�м�����
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;

        case 0x44://�м�����
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://�м��ſ�
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08://����������ƶ�
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        }
        lstPacket.push_back(CPacket(5, NULL, 0));
        return 0;
    }

    int SendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        CImage screen;//GDI(ȫ���豸�ӿ�)
        HDC hScreen = ::GetDC(NULL);//��ȡ�豸�����ģ�������Ļ�������HDC(Handle to device context
        //::��ʾ��ȫ�ֺ���������Ҫ��������Ϳ���ֱ�ӵ��øú������Ҹú���������ȫ�������ռ��С�
        //��ȡÿ�����ص�λ������Ļ�߶ȺͿ��
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
        int nWidth = GetDeviceCaps(hScreen, HORZRES);
        int nHeight = GetDeviceCaps(hScreen, VERTRES);
        screen.Create(nWidth, nHeight, nBitPerPixel);//������Ĳ���������screenͼƬ����
        //BitBlt(screen.GetDC(), 0, 0, 2560, 1440, hScreen, 0, 0, SRCCOPY); //ʹ�� BitBlt ����������Ļ��ͼ���Ƶ� screen ������
        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY); //ʹ�� BitBlt ����������Ļ��ͼ���Ƶ� screen ������
        ReleaseDC(NULL, hScreen);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//����ȫ���ڴ����׼��ʹ�ú����ڴ���
        if (hMem == NULL)//ȫ���ڴ����ʧ��
            return -1;
        IStream* pStream = NULL;//�ڷ����ȫ���ڴ��ϴ���һ�� IStream �ӿ�
        HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
        if (ret == S_OK) {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);//��һ����Ļ��ͼ����ΪPNG��ʽ��ͼƬ��һ����������
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, NULL);//���������ڲ�λ��ָ�뵽���Ŀ�ʼ��
            PBYTE pData = (BYTE*)GlobalLock(hMem);//��һ��ȫ���ڴ�����������������ȡ��ָ���ڴ�������ݵ�ָ�롣
            SIZE_T nSize = GlobalSize(hMem);
            lstPacket.push_back(CPacket(6, pData, nSize));
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
    int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {//dlg�ľ��Ϊ�ջ���Ч
            //_beginthread �������߳�Ӧ�����н���������ڸ��߳̿�ʼ��LockMachine �������������������ȴ����̵߳Ľ���
            //_beginthread(threadLockDlg, 0, NULL);//_beginthread�������̣߳��ú���ָ��threadLockDlgָ���߳�Ӧ��ִ�еĺ���
            _beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);
            TRACE("threadid=%d\r\n", threadid);
        }
        lstPacket.push_back(CPacket(7, NULL,0));
        return 0;
    }

    int UnlockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        //dlg.SendMessage(WM_KEYDOWN,0x41, LPARAM(001E0001));
        //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x41, 0x01E0001);
        PostThreadMessage(threadid, WM_KEYDOWN, 0x41, 0);//��һ����Ϣ(����)Ͷ�ݵ�ָ���߳�(threadid)������Ϣ������
        //��Ϣ�Ǹ����߳����ģ����Ǹ��ŶԻ��򣬾������
        lstPacket.push_back(CPacket(8, NULL, 0));
        return 0;
    }

    int TestConnect(std::list<CPacket>& lstPacket, CPacket& inPacket) {
        lstPacket.push_back(CPacket(1981, NULL, 0));
        return 0;
    }

    int DeleteLocalFile(std::list<CPacket>& lstPacket, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        TCHAR sPath[MAX_PATH] = _T("");//ʹ��ϵͳ����� MAX_PATH��ͨ��Ϊ 260������һ�� TCHAR ���͵����� sPath�����ڴ洢ת��Ϊ���ַ���ʽ����ļ�·����
        //_T("") ��һ���꣬�����ڸ�����Ŀ�Ƿ�֧�� Unicode �������ʵ�������ַ�������
        //mbstowcs(sPath, strPath.c_str(), strPath.size());������
        //ʹ�� mbstowcs ������ strPath �����д洢�Ķ��ֽ��ַ��ַ���ת���ɿ��ַ��ַ�������������洢�� sPath ���顣���Ǳ�Ҫ�ģ���Ϊ Windows API ���� DeleteFile ������Ҫһ�����ַ��ַ�����Ϊ��������ȡ���������Ŀ���á�
        MultiByteToWideChar(
            CP_ACP, 0, strPath.c_str(), strPath.size(), sPath,
            sizeof(sPath) / sizeof(TCHAR));
        DeleteFileA(strPath.c_str());
        //���� Windows API ���� DeleteFile ɾ����Ϊ sPath ���ļ�����������ɹ����ļ��������Ϊɾ�����������һ������ر�ʱɾ����
        lstPacket.push_back(CPacket(9, NULL, 0));
        return 0;
    }
};

