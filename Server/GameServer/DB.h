#pragma once
#include "stdafx.h"
#include "sql.h"
#include "sqlext.h"
#include "Server.h"

#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PW "123456789"
#define DB_NAME "skyfall"

class SESSION;

class DB
{
public:
	MYSQL* connection = NULL;
	MYSQL conn;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	char query[255];
	bool isRun = false;

	bool Connection();
	bool Send_Query(char* query);

	SQLHENV hEnv;
	SQLHDBC hDbc;
	SQLHSTMT hStmt = 0;

	SQLCHAR* name = (SQLCHAR*)"skyfall";
	SQLCHAR* user = (SQLCHAR*)"root";
	SQLCHAR* pw = (SQLCHAR*)"123456789";

	bool Connection_ODBC();
	void Disconnection_ODBC();

	bool Search_ID(char* id, char* pw);
	bool Insert_ID(char* id, char* pw);
	bool Logout_player(char* id);
	bool Send_player_record(const SESSION& player, int survival_time, int rank);
	bool Get_player_record(char* ID, SESSION& session, int* survival_time, int* rank);
};

