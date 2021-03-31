#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#include "stdafx.h"
#include "Server.h"
#include "CPacket.h"
#include "Map.h"

Server* s = new Server;
DB* db = new DB;
Map* m = new Map;


int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale("korean"));
   //db->Connection();

	s->Init();

	std::thread map_thread = std::thread(&Map::init_Map, m, s);

	s->Thread_join();

   //mysql_close(db->connection);

}

// TO DO