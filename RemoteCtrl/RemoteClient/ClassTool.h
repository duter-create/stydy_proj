#pragma once
#include <Windows.h>
#include <string>
#include <atlimage.h>

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
    static int Bytes2Image(CImage& image, const std::string& strBuffer) {
		BYTE* pData = (BYTE*)strBuffer.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL) {
			TRACE("�ڴ治����\r\n");
			Sleep(1);
			return -1;
		}
		IStream* pStream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
		if (hRet == S_OK) {
			ULONG length = 0;
			pStream->Write(pData, strBuffer.size(), &length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			if ((HBITMAP)image != NULL)
				image.Destroy();
			image.Load(pStream);
		}
		return hRet;
    }

};

