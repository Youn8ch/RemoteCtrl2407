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
};

