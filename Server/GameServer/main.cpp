#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#include "stdafx.h"
#include "Server.h"
#include "Map.h"
#include "Timer.h"

Server*		server = new Server;
DB*			db = new DB;

int main(int argc, char* argv[])
{
	//std::wcout.imbue(std::locale("korean"));
   //db->Connection();

	server->Init();
	server->Thread_join();

   //mysql_close(db->connection);

	delete server;
	delete db;
}