#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#include "stdafx.h"
#include "Server.h"
#include "Map.h"

Server* s = new Server;
DB* db = new DB;


int main(int argc, char* argv[])
{
	//std::wcout.imbue(std::locale("korean"));
   //db->Connection();

	s->Init();

	s->Thread_join();

   //mysql_close(db->connection);

}