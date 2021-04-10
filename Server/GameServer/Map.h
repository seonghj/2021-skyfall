#pragma once
#include "stdafx.h"
#include "Server.h"
#include "CPacket.h"

class Server;

class Map{
public:
	Map() { }
	Map(const Map& m) {}
	~Map() {}

	Vector2D Cloud;

	int Map_num[MAX_MAP_BLOCK];

	float atm[9] = { 0 };
	float wind[12] = { 0 };
	int collapse_count = 0;

	bool isMap_block[9];

	void init_Map(Server* s);
	void Set_wind();
	void Set_cloudpos();
	void print_Map();
	float calc_windpower(float a, float b);
	void cloud_move();

	void Map_collapse();

private:
	Server* m_pServer = NULL;
};