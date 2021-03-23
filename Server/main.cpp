#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#include "stdafx.h"
#include "ServerFunc.h"
#include "CPacket.h"
#include "Map.h"

IOCPServer* s = new IOCPServer;
DB* db = new DB;
Map* m = new Map;


int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale("korean"));
   //db->Connection();

	s->Init();

	//std::thread map_thread = std::thread(&Map::init_Map, m, s);

	s->Thread_join();

   //mysql_close(db->connection);


	return 0;
}