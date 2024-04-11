#include "pch.h"
#include "Command.h"

CCommand::CCommand():threadid(0)
{
	struct {
		int nCmd;
		CMDFUNC func;
	}data[] = {
		{1,&CCommand::MakeDriverInfo},
		{2,&CCommand::MakeDirectoryInfo},
		{3,&CCommand::RunFile},
		{4,&CCommand::DownloadFile},
		{5,&CCommand::MouseEvent},
		{6,&CCommand::SendScreen},
		{7,&CCommand::LockMachine},
		{8,&CCommand::UnlockMachine},
		{9,&CCommand::DeleteLocalFile},
		{1981,&CCommand::TestConnect},
		{-1,NULL }
	};
	for (int i = 0; data[i].nCmd != -1; i++) {//将data中的每个命令号和函数指针对插入到m_mapFunction这个map中去
		m_mapFunction.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func)); 
	}
}

int CCommand::ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPacket)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);
	if (it == m_mapFunction.end()) {
		return -1; 
	}
	return (this->*it->second)(lstPacket, inPacket);//实际调用该成员函数，并传递 lstPacket 作为参数
}
