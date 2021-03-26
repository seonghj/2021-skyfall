#pragma once
#include"stdafx.h"

enum PacketType {
	Type_player_ID,
	Type_player_login,
	Type_player_remove,
	Type_player_info,
	Type_player_move,
	Type_player_pos,
	Type_player_attack,
	Type_map_collapse,
	Type_cloud_move
};

enum MoveType {
	FRONT,
	RIGHT,
	LEFT,
	BACK,
	STOP
};

#pragma pack(push, 1)

// 0: size // 1: type // 2: id

struct Packet {
public:
	char size;
	char type;
};

struct player_ID_packet :public Packet {
	char id;
};

struct player_login_packet : public Packet {
	char id;
};

struct player_remove_packet : public Packet {
	char id;
};

struct player_info_packet : public Packet {
	char id;
	char state;
	char weapon;
	float hp;
	float speed;
};

struct player_move_packet : public Packet {
	char id;
	char MoveType;
	float x, y, z;
	float degree;
};

struct player_attack_packet : public Packet {
	char id;
	char attack_type;
	float damage;
};

struct map_collapse_packet : public Packet {
	char block_num;
};

struct cloud_move_packet : public Packet {
	float x, z;
};

#pragma pack(pop)