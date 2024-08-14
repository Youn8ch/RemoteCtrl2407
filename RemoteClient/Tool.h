#pragma once
#include <string>
#include <Windows.h>
#include <iostream>
#include <atlimage.h>

class CTool
{

public:
	static void Dump(BYTE* pdata, size_t nSize) {
		std::string strout;
		for (size_t i = 0; i < nSize; i++)
		{
			char buf[8] = "";
			if (i > 0)
			{
				strout += " ";
			}
			snprintf(buf, sizeof(buf), "%02X", pdata[i] & 0xFF);
			// printf("pData[%d] in hex: %s\n", i, buf);
			strout += buf;
		}
		strout += "\n";
		std::cout << "Dump:\n" << strout << std::endl;
	}

	static int Bytes2Image(CImage& image,std::string& strBuffer) {
		BYTE* pData = (BYTE*)strBuffer.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL)
		{
			TRACE("no memory !\r\n");
			Sleep(1);
			return -1;
		}
		IStream* pStream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
		if (hRet == S_OK)
		{
			ULONG length = 0;
			pStream->Write(pData, (ULONG)strBuffer.size(), &length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			if (image.IsDIBSection())
			{
				image.Destroy();
			}
			// if ((HBITMAP)m_image!=NULL) m_image.Destroy();
			image.Load(pStream);
		}
		return hRet;
	}

};

