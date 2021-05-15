#pragma once
#include"stdafx.h"

constexpr int GAMESERVERPORT = 3500;
constexpr int LOBBYSERVERPORT = 4000;
constexpr int BUFSIZE = 1024;
constexpr int MAX_CLIENT = 100;
constexpr int MAX_PLAYER = 20;

constexpr int GAMESERVER_ID = 0;

constexpr int MAX_MAP_BLOCK = 9;
constexpr int MAP_SIZE = 3000;
constexpr int MAP_BLOCK_SIZE = 1000;
constexpr int MAP_BREAK_TIME = 30;

constexpr int VIEWING_DISTANCE = 500;

enum PacketType {
	Type_player_ID,			// S->C
	Type_player_login,		// S->C
	Type_player_remove,		// S->C
	Type_game_ready,		// C->S
	Type_game_start,		// C->S
	Type_start_ok,			// S->C
	Type_game_end,			// S->C
	Type_player_info,		//	
	Type_player_move,		// C->S
	Type_player_pos,		//
	Type_player_attack,		//
	Type_map_collapse,		// S->C
	Type_cloud_move,		// S->C
	Type_bot_ID,			//	
	Type_bot_remove,
	Type_bot_info,
	Type_bot_move,
	Type_bot_pos,
	Type_bot_attack,
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

struct game_ready_packet :public Packet {
	char id;
};

struct game_start_packet :public Packet {
	char id;
};

struct start_ok_packet :public Packet {
	// 0 = no / 1 = ok
	char value;
};

struct game_end_packet :public Packet {
	char id;
};
struct player_remove_packet : public Packet {
	char id;
};

struct player_info_packet : public Packet {
	char id;
	char state;
	char weapon;
	char armor;
	char helmet;
	char shoes;
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