#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#include "stdafx.h"
#include "Server.h"
#include "Map.h"
#include "Bot.h"
#include "Timer.h"

//#define Run_DB
#define Run_Bot

Server*		g_pServer = new Server;
DB*			g_pDB = new DB;
Bot*		g_pBot = new Bot;
Timer*		g_pTimer = new Timer;

int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale("korean"));

#ifdef Run_DB
	bool DB_Connected;
	DB_Connected = g_pDB->Connection_ODBC();
	if (DB_Connected) std::cout << "DB connected\n";
	else {
		g_pDB->Disconnection_ODBC();
		std::cout << "DB disconnected\n";
	}
	g_pDB->isRun = DB_Connected;
#endif
	g_pServer->Set_pTimer(g_pTimer);

#ifdef Run_Bot
	g_pServer->Set_pBot(g_pBot);
	g_pBot->Set_pServer(g_pServer);
	g_pBot->Set_pTimer(g_pTimer);
#endif

	g_pServer->Init();

#ifdef Run_DB
	g_pServer->Set_pDB(g_pDB);
#endif
	g_pServer->Thread_join();

   //mysql_close(g_pDB->connection);

	delete g_pServer;
	delete g_pDB;
	delete g_pBot;
	delete g_pTimer;
}