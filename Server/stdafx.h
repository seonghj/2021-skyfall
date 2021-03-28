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
#include <unordered_map>
#include <vector>
#include <mysql.h>
#include <functional>
#include <mutex>
#include <atomic>

#pragma comment(lib, "libmySQL.lib")


#define TRUE 1
#define FALSE 0

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