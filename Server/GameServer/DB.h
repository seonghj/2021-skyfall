#pragma once
#include "stdafx.h"
#include "sql.h"
#include "sqlext.h"

#define DB_HOST "sky-fall.cj14ovuewlov.us-east-2.rds.amazonaws.com"
#define DB_USER "admin"
#define DB_PW "tjdwo1034"
#define DB_NAME "skyfall"

class DB
{
public:
	MYSQL* connection = NULL;
	MYSQL conn;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	char query[255];

	bool Connection();
	bool Send_Query(char* query);

	SQLHENV hEnv;
	SQLHDBC hDbc;
	SQLHSTMT hStmt = 0;

	SQLCHAR* name = (SQLCHAR*)"skyfall";
	SQLCHAR* user = (SQLCHAR*)"admin";
	SQLCHAR* pw = (SQLCHAR*)"tjdwo1034";

	bool Connection_ODBC();
	void Disconnection_ODBC();

	bool Search_ID(char* id);
	bool Insert_ID(char* id);
};

