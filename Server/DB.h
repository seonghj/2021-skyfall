#pragma once
#include "stdafx.h"

class DB
{
public:
	MYSQL* connection = NULL;
	MYSQL conn;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	char query[255];

	bool Connection();
	bool Send_Query(char* buf);
};

