#pragma once
#pragma comment(lib, "ws2_32")

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <windows.h>
#include <random>
#include <cmath>

#include <thread>
#include <map>
#include <vector>
#include <mysql.h>
#include <functional>
#include <mutex>
#include <atomic>

#pragma comment(lib, "libmySQL.lib")

#define DB_HOST "database-1.cjfrzsztpm1z.ap-northeast-2.rds.amazonaws.com"
#define DB_USER "admin"
#define DB_PW "tjdwo104"
#define DB_NAME "sys"


#define TRUE 1
#define FALSE 0

#define SERVERPORT 3500
#define BUFSIZE    1024
#define MAX_CLIENT 100
#define MAX_PLAYER 20

#define MAX_MAP_BLOCK 9
#define MAP_SIZE 3000
#define MAP_BLOCK_SIZE 1000
#define MAP_BREAK_TIME 30

#define VIEWING_DISTANCE 500

using _packet = unsigned char;

class Vector2D
{
public:
	float x, y;
};

enum GameState {
	
};

enum terrain {
	Forest,
	Desert,
	Snowy_field
};