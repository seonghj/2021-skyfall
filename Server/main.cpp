#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#include "stdafx.h"
#include "ServerFunc.h"
#include "CPacket.h"
#include "Map.h"

IOCPServer* s = new IOCPServer;
DB* db = new DB;
Map* m = new Map;


int main(int argc, char* argv[])
{
   //db->Connection();

   s->Init();
   s->Run();

   //mysql_close(db->connection);

   /*m->init_Map();
   m->print_Map();

   Sleep(1000);
   m->cloud_move();*/

   return 0;
}