#pragma once
#include "stdafx.h"
#include "Server.h"
#include "protocol.h"

//struct OVER_EX;
class Server;

class Map{
public:
	Map() { }
	Map(const int& num) { game_num = num; }
	~Map() {}

	OVER_EX     over;

	Vector2D Cloud;

	int Map_num[MAX_MAP_BLOCK];

	float atm[9] = { 0 };
	float wind[12] = { 0 };

	bool isMap_block[9];

	void SetNum(int n) { game_num = n; }
	void init_Map(Server* s);
	void Set_wind();
	void Set_cloudpos();
	void print_Map();
	float calc_windpower(float a, float b);
	void cloud_move();
	void Map_collapse();

private:
	int game_time;
	Server* m_pServer = NULL;
	int game_num = -1;
};