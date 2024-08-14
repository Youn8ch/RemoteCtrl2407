#pragma once
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

	static bool isAdmin() {
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;
        DWORD len = 0;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == false) {
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve))
        {
            return eve.TokenIsElevated;
        }
        printf(" length of tokenInfo is %d\r\n", len);
        return false;
    }

    static bool RunAsAdmin() {
        // ���ز����� ����Administrator�˻� ��ֹ������ֻ�ܵ�¼���ؿ���̨
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL,
            NULL, LOGON_WITH_PROFILE, NULL,
            sPath,
            CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
        if (!ret)
        {
            ShowError();
            MessageBox(NULL, _T("���̴���ʧ��"), _T("ERROR"), 0);
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    static void ShowError() {
        LPWSTR lpMessageBuf = NULL;
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_CHINESE_SIMPLIFIED),
            (LPWSTR)&lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);
        LocalFree(lpMessageBuf);
        ::exit(0);
    }


    static bool Init() {
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr)
        {
            wprintf(L"����: ʧ��\n");
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


