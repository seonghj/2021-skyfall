#pragma once
#include"stdafx.h"

constexpr int GAMESERVERPORT = 3500;
constexpr int LOBBYPORT = 4000;
constexpr int BUFSIZE = 1024;
constexpr int MAX_CLIENT = 3000;
constexpr int MAX_PLAYER = 20;

constexpr int LOBBY_ID = 0;
constexpr int GAMESERVER_ID = 0;

constexpr int MAX_MAP_BLOCK = 9;
constexpr int MAP_SIZE = 99999;
constexpr int MAP_BLOCK_SIZE = 33333;
constexpr int MAP_BREAK_TIME = 30;

constexpr float VIEWING_DISTANCE = 16666.f;
// 1 = 3cm

#define SERVERIP   "127.0.0.1"

struct OVER_EX
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuffer;
	char			messageBuffer[BUFSIZE];
	bool			is_recv;
	int             type;
	int				roomID;
	// 0 = session 1 = map
};

enum OVER_EX_Type {
	OE_session,
	OE_map
};

enum PacketType {
	Type_player_ID,			// S->C
	Type_player_login,		// S->C
	Type_player_remove,		// S->C
	Type_game_ready,		// C->S
	Type_game_start,		// C->S
	Type_start_ok,			// S->C
	Type_game_end,			// S->C
	Type_player_info,		//	
	Type_weapon_swap,
	Type_player_move,		// C->S
	Type_player_pos,
	Type_start_pos,
	Type_player_attack,		//
	Type_allow_shot,
	Type_player_Damage,
	Type_map_set,
	Type_map_collapse,		// S->C
	Type_cloud_move,		// S->C
	Type_bot_ID,			//	
	Type_bot_remove,
	Type_bot_info,
	Type_bot_move,
	Type_bot_pos,
	Type_bot_attack,
};

enum EventType {
	Mapset,
	Cloud_move
};

enum PlayerState {
	DEAD,
	ALIVE,
};

enum PlayerMove {
	WAKING,
	RUNNING,
	JUMP
};

enum PlayerAttackType {
	SWORD1H,
	BOW
};

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

#pragma pack(push, 1)

// 0: size // 1: type // 2: id;

struct Packet {
public:
	char size;
	char type;
	unsigned short id;
};

struct player_ID_packet :public Packet {
};

struct player_login_packet : public Packet {
	DirectX::XMFLOAT3 Position;
};

struct game_ready_packet :public Packet {
};

struct game_start_packet :public Packet {
};

struct start_ok_packet :public Packet {
	// 0 = no / 1 = ok
	char value;
};

struct game_end_packet :public Packet {
};

struct player_remove_packet : public Packet {
};

struct player_info_packet : public Packet {
	char state;
	DirectX::XMFLOAT3 Position;
	float dx, dy, dz;
	char weapon;
	char armor;
	char helmet;
	char shoes;
	float hp;
	float lv;
	float speed;
};

struct player_pos_packet : public Packet {
	char state;
	DirectX::XMFLOAT3 Position;
	float dx, dy, dz;
};

struct player_start_pos : public Packet {
	DirectX::XMFLOAT3 Position;
};

struct player_move_packet : public Packet {
	char state;
	DWORD MoveType;
	float dx, dy, dz;
};

struct player_status_packet : public Packet {
	char state;
};

struct player_stat_packet : public Packet {
	float hp;
	float lv;
	float speed;
};

struct Weapon_swap_packet : public Packet {
	char weapon;
};

struct player_equipment_packet : public Packet {
	char armor;
	char helmet;
	char shoes;
};

struct player_attack_packet : public Packet {
	char attack_type;
};

struct player_allow_packet : public Packet {
	char attack_type;
	float fSpeed;
};

struct player_damage_packet : public Packet {
	unsigned short damage;
};

struct map_block_set : public Packet {
	char block_num[9];
};

struct map_collapse_packet : public Packet {
	char block_num;
};

struct cloud_move_packet : public Packet {
	float x, z;
};

#pragma pack(pop)