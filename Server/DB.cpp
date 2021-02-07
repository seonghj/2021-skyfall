#include "DB.h"

bool DB::Connection()
{
    printf("MySQL Ver. %s\n", mysql_get_client_info());

    if (mysql_init(&conn) == NULL)
        printf("mysql_init() error\n");

    connection = mysql_real_connect(&conn, DB_HOST, DB_USER
        , DB_PW, DB_NAME, 3306, (const char*)NULL, 0);

    mysql_query(connection, "set session character_set_connection=euckr;");
    mysql_query(connection, "set session character_set_results=euckr;");
    mysql_query(connection, "set session character_set_client=euckr;");

    if (connection == NULL)
    {
        printf("%d: %s\n", mysql_errno(&conn), mysql_error(&conn));
        return 0;
    }
    else
    {
        printf("DB connected\n");
        return 1;
    }
}

bool DB::Send_Query(char* buf)
{
    int state = mysql_query(connection, buf);
    if (state != 0)
    {
        printf("MySQL query error : %s\n", mysql_error(&conn));
        return 0;
    }
    else
        return 1;
}