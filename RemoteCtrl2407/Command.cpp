#include "pch.h"
#include "Command.h"
#include "Log.h"

CCommand::CCommand():threadid(0)
{
	struct 
	{
		int nCmd;
		CMDFUNC func;
	} data[] = {
		{1,&CCommand::MakeDriverInfo},
		{2,&CCommand::MakeDirectoryInfo},
		{3,&CCommand::RunFile},
		{4,&CCommand::DownloadFile},
		{5,&CCommand::MouseEvent},
		{6,&CCommand::SendScreen},
		{7,&CCommand::LockMachine},
		{8,&CCommand::UnLockMachine},
		{9,&CCommand::DeleteLocalFile},
		{666,&CCommand::TestConnect},
		{-1,NULL},
	};
	for (int i = 0; data[i].nCmd != -1; i++)
	{
		m_mapFunction.insert(std::pair<int,CMDFUNC>(data[i].nCmd,data[i].func));
	}
}

int CCommand::ExcuteCommand(int nCmd)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);
	if (it == m_mapFunction.end())
	{
		LOGE("ExcuteCommand Failed!");
		return -1;
	}
	return (this->*it->second)();
}
