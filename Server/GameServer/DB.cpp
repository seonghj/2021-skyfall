#include "DB.h"

//#define Test_DB

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

bool DB::Send_Query(char* query)
{
    int state = mysql_query(connection, query);
    if (state != 0)
    {
        printf("MySQL query error : %s\n", mysql_error(&conn));
        return 0;
    }
    else
        return 1;
}


bool DB::Connection_ODBC()
{
    // 환경 구성
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv)
        != SQL_SUCCESS)
        return false;
    // 버전 정보 설정
    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, SQL_IS_INTEGER)
        != SQL_SUCCESS)
        return false;
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc)
        != SQL_SUCCESS)
        return false;
    // 접속
    SQLSetConnectAttr(hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
    if (SQLConnect(hDbc, (SQLWCHAR*)L"skyfall", SQL_NTS
        , (SQLWCHAR*)L"root", SQL_NTS
        , (SQLWCHAR*)L"123456789", SQL_NTS)
        != SQL_SUCCESS)
        return false;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    return true;
}

void DB::Disconnection_ODBC()
{
    if(hStmt)
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if(hDbc)
        SQLDisconnect(hDbc);
    if(hDbc)
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    if(hEnv)
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool DB::Search_ID(char* id, char* pw)
{
    wchar_t query[512] = L"SELECT PassWord, isLogin FROM mychat.userinfo WHERE ID = '";
    wchar_t wcID[20];
    wchar_t wcPW[20];

    char PW[25];
    SQLLEN len = 0;
    bool isLogin;

    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));

    wcscat_s(query, wcID);
    wcscat_s(query, L"'");

#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif 

    // ID 검색
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Query invaild\n");
        return false;
    }
    SQLBindCol(hStmt, 1, SQL_C_CHAR, PW, sizeof(PW), &len);
    SQLBindCol(hStmt, 2, SQL_C_TINYINT, &isLogin, sizeof(bool), &len);
    if (SQLFetch(hStmt) == SQL_NO_DATA) return false;
    if (hStmt) SQLCloseCursor(hStmt);

    if (isLogin == true) return false;
    if (strcmp(PW, pw) == 0) return true;

    return false;
}

bool DB::Insert_ID(char* id, char* pw)
{
    wchar_t query[512] = L"insert into skyfall.userinfo VALUES ('";
    wchar_t wcID[20];
    wchar_t wcPW[20];

    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));
    MultiByteToWideChar(CP_ACP, 0, pw, -1, wcPW, sizeof(pw));

    wcscat_s(query, wcID);
    wcscat_s(query, L"', 0, '");
    wcscat_s(query, wcPW);
    wcscat_s(query, L"')");

#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Query invaild\n");
        return false;
    }

    if (hStmt) SQLCloseCursor(hStmt);

    return true;
}

bool DB::Logout_player(char* id)
{
    wchar_t query[512] = L"UPDATE skyfall.UserInfo SET isLogin = 0 WHERE ID = '";
    wchar_t wcID[20];

    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));
    wcscat_s(query, wcID);
    wcscat_s(query, L"'");

#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Query invaild\n");
        return false;
    }

    if (hStmt) SQLCloseCursor(hStmt);

    return true;
}

bool DB::Send_player_record(const SESSION& player, int survival_time, int rank)
{
    wchar_t query[512] = L"insert into skyfall.UserRecord VALUES (";

    char player_info[512];
    wchar_t wc_player_info[512];
    sprintf_s(player_info, sizeof(player_info)
        , "'%s', %d, %d, %d, %d, %d, %d, %d)"
        , player.id, survival_time, rank, player.weapon1.load(), player.weapon2.load()
        , player.helmet.load(), player.shoes.load(), player.armor.load());
    MultiByteToWideChar(CP_ACP, 0, player_info, -1, wc_player_info, sizeof(player_info));
    wcscat_s(query, wc_player_info);

#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        //printf("Query invaild\n");
        return false;
    }
    if (hStmt) SQLCloseCursor(hStmt);

    return true;
}

bool DB::Get_player_record(char* ID, SESSION& session, int* survival_time, int* rank)
{
    wchar_t query[512] = L"SELECT * FROM skyfall.UserRecord WHERE User_ID = '";
    wchar_t wcID[20];
    SQLLEN len = 0;

    MultiByteToWideChar(CP_ACP, 0, ID, -1, wcID, sizeof(ID));
    wcscat_s(query, wcID);
    wcscat_s(query, L"'");

#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Query invaild\n");
        return false;
    }
    SQLBindCol(hStmt, 1, SQL_C_CHAR, session.id, sizeof(session.id), &len);
    SQLBindCol(hStmt, 2, SQL_INTEGER, survival_time, sizeof(int), &len);
    SQLBindCol(hStmt, 3, SQL_INTEGER, rank, sizeof(int), &len);
    SQLBindCol(hStmt, 4, SQL_INTEGER, &session.weapon1, sizeof(int), &len);
    SQLBindCol(hStmt, 5, SQL_INTEGER, &session.weapon2, sizeof(int), &len);
    SQLBindCol(hStmt, 6, SQL_INTEGER, &session.helmet, sizeof(int), &len);
    SQLBindCol(hStmt, 7, SQL_INTEGER, &session.shoes, sizeof(int), &len);
    SQLBindCol(hStmt, 8, SQL_INTEGER, &session.armor, sizeof(int), &len);
    if (SQLFetch(hStmt) == SQL_NO_DATA) return false;
    if (hStmt) SQLCloseCursor(hStmt);

#ifdef Test_DB 
    printf("%s, %d, %d, %d, %d, %d, %d, %d\n", session.id, *survival_time, *rank
        , session.weapon1.load(), session.weapon2.load(), session.helmet.load()
        , session.shoes.load(), session.armor.load());
#endif

    return true;
}