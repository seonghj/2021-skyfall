#pragma once
#include"stdafx.h"

struct Packet {
public:
	char size;
	char type;
};

enum PacketType{
	T_player_login,
	T_player_remove,
	T_player_pos,
	T_player_attack,
	T_map_collapse,
	T_cloud_move
};

struct player_login : public Packet{
	char id;
};

struct player_remove : public Packet {
	char id;
};

struct player_pos : public Packet {
	char id;
	char x, y, z;
	char degree;
};

struct player_attack : public Packet {
	char id;
	char attack_type;
	char damage;
};

struct map_collapse : public Packet {
	char block_num;
};

struct cloud_move : public Packet {
	char x, z;
};