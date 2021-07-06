#pragma once
#include <WinSock2.h>
#include <DirectXMath.h>

constexpr int GAMESERVERPORT = 3500;
constexpr int LOBBYPORT = 4000;
constexpr int BUFSIZE = 128;
constexpr int MAX_CLIENT = 3000;
constexpr int MAX_PLAYER = 20;
constexpr int INVALIDID = -1;
constexpr int LOBBY_ID = 0;
constexpr int GAMESERVER_ID = 0;
constexpr int AI_ID = 5000;

constexpr int MAX_MAP_BLOCK = 9;
constexpr int MAP_SIZE = 99999;
constexpr int MAP_BLOCK_SIZE = 33333;
constexpr int MAP_BREAK_TIME = 30;

constexpr float VIEWING_DISTANCE = 16666.f;

// 1 = 3cm

#define SERVERIP   "127.0.0.1"
//#define SERVERIP   "39.120.192.92"
//#define SERVERIP   "192.168.0.118"

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
	OE_gEvent
};

enum PacketType {
	SC_player_key,
	SC_player_loginOK,
	SC_player_loginFail,
	SC_player_add,
	SC_player_remove,
	SC_player_disconnect,
	SC_start_ok,
	SC_game_end,
	SC_player_info,
	SC_weapon_swap,
	SC_player_pos,
	SC_player_move,
	SC_start_pos,
	SC_player_attack,
	SC_allow_shot,
	SC_player_damage,
	SC_player_stop,
	SC_map_set,
	SC_map_collapse,
	SC_cloud_move,
	SC_bot_add,
	SC_bot_remove,
	SC_bot_info,
	SC_bot_move,
	SC_bot_pos,
	SC_bot_attack,
	SC_bot_damaged,
	SC_monster_add,
	SC_monster_remove,
	SC_monster_info,
	SC_monster_move,
	SC_monster_pos,
	SC_monster_attack,
	SC_monster_damaged,


	CS_player_login,
	CS_game_ready,
	CS_game_start,
	CS_player_info,
	CS_weapon_swap,
	CS_player_move,
	CS_player_pos,
	CS_start_pos,
	CS_player_attack,
	CS_player_stop,
	CS_allow_shot,
};

enum EventType {
	Mapset,
	Cloud_move,
	game_end
};

enum PlayerState {
	DEAD,
	ALIVE,
};

enum PlayerMove {
	WAKING,
	RUNNING,
	JUMP,
	STAND
};

enum PlayerAttackType {
	SWORD1HL,
	SWORD1HR,
	BOWL,
	BOWR,
};

enum PlayerType {
	PT_BASIC,
	PT_SWORD1H,
	PT_BOW
};

enum MonsterType {
	LOW,
	MIDDLE,
	BOSS
};

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

#pragma pack(push, 1)

// 0: size // 1: type // 2: key;

struct Packet {
public:
	char size;
	char type;
	unsigned int key;
	unsigned int roomid;
};

struct player_key_packet :public Packet {
};

struct player_login_packet :public Packet {
	char id[50];
};

struct player_loginOK_packet :public Packet {
	DirectX::XMFLOAT3 Position;
	float dx, dy;
	short PlayerType;
};

struct player_loginFail_packet :public Packet {
};


struct player_add_packet : public Packet {
	DirectX::XMFLOAT3 Position;
	float dx, dy;
	short PlayerType;
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

struct player_disconnect_packet : public Packet {
};

struct player_info_packet : public Packet {
	char state;
	float dx, dy;
	char weapon;
	char armor;
	char helmet;
	char shoes;
	short hp;
	short lv;
	short speed;
};

struct player_pos_packet : public Packet {
	char state;
	DirectX::XMFLOAT3 Position;
	float dx, dy;
	DWORD MoveType;
};

struct player_start_pos : public Packet {
	DirectX::XMFLOAT3 Position;
};

struct player_move_packet : public Packet {
	char state;
	DWORD MoveType;
	float dx, dy;
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

struct player_shot_packet : public Packet {
	DirectX::XMFLOAT3 Look;
	float fTimeElapsed;
	float ChargeTimer;
};

struct player_arrow_packet : public Packet {
	char attack_type;
	float fSpeed;
};

struct player_damage_packet : public Packet {
	unsigned short damage;
};

struct player_stop_packet : public Packet {
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

struct mon_add_packet : public Packet {
	DirectX::XMFLOAT3 Position;
	float dx, dy;
	short MonsterType;
};

struct mon_remove_packet : public Packet {
};
#pragma pack(pop)