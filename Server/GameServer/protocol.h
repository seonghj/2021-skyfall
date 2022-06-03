#pragma once
#include <WinSock2.h>
#include <DirectXMath.h>

constexpr int GAMESERVERPORT = 3500;
constexpr int LOBBYPORT = 4000;
constexpr int BUFSIZE = 128;
constexpr int MAX_CLIENT = 3000;
constexpr int MAX_PLAYER = 5;
constexpr int INVALIDID = -1;
constexpr int LOBBY_ID = 0;
constexpr int GAMESERVER_ID = 0;
constexpr int AI_ID = 5000;
constexpr int MAX_STAMINA = 100;
constexpr int MAX_MONSTER = 15;

constexpr int MAX_MAP_BLOCK = 9;
constexpr int MAP_SIZE = 6135;
constexpr int MAP_BLOCK_SIZE = 2045;
constexpr int MAP_BREAK_TIME = 90000;

constexpr int MON_SPAWN_TIME = 10000;

constexpr float VIEWING_DISTANCE = 2000.f;

constexpr int INVENTORY_MAX = 20;

constexpr int MAX_ROOM = 20;

constexpr float Atack_Distance_Dragon = 80.f;
constexpr float Atack_Distance_Wolf = 55.f;
constexpr float Atack_Distance_Metalon = 97.f;

constexpr float ARROW_SPEED_WEIGHT = 300.f;


#define SERVERIP   "127.0.0.1"
//#define SERVERIP   "39.120.192.92"

enum OVER_EX_Type {
	OE_accept,
	OE_send,
	OE_recv,
	OE_gEvent
};

struct OVER_EX
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuffer;
	char			messageBuffer[BUFSIZE];
	SOCKET			csocket;
	OVER_EX_Type    type;
	int				roomID;
	// 0 = session 1 = map
};

enum terrain {
	Forest,
	Desert,
	Snowy_field
};


enum PacketType {
	SC_NONE,
	SC_create_account,
	SC_player_Lobbykey,
	/*SC_player_LobbyloginOK,
	SC_player_LobbyloginFail,*/
	SC_create_room,
	SC_room_list,
	SC_select_room,
	SC_return_lobby,
	SC_player_InGamekey,
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
	SC_player_rotate,
	SC_start_pos,
	SC_player_attack,
	SC_allow_shot,
	SC_player_damage,
	SC_player_stop,
	SC_player_dead,
	SC_map_set,
	SC_map_collapse,
	SC_cloud_move,
	SC_monster_add,
	SC_monster_remove,
	SC_monster_info,
	SC_monster_move,
	SC_monster_pos,
	SC_monster_attack,
	SC_monster_damaged,
	SC_monster_respawn,
	SC_monster_stop,
	SC_player_record,
	SC_player_getitem,
	SC_weapon_select,


	CS_create_account,
	CS_room_select,
	CS_create_room,
	CS_refresh_lobby,
	CS_player_Lobbylogin,
	CS_player_login,
	CS_game_ready,
	CS_game_start,
	CS_player_info,
	CS_weapon_swap,
	CS_player_move,
	CS_player_pos,
	CS_player_dead,
	CS_start_pos,
	CS_player_attack,
	CS_player_damage,
	CS_player_stop,
	CS_allow_shot,
	CS_player_getitem,
	CS_monster_pos,
	CS_monster_attack,
	CS_monster_damaged,
	CS_return_lobby,
	CS_weapon_select,
	CS_NONE,
};

enum EventType {
	Mapset,
	Cloud_move,
	game_end,
	Mon_behavior,
	Mon_move_to_player,
	Mon_attack_cooltime,
	Mon_respawn,
	Mon_attack,
	Mon_stop,
	MapBreak,
};

enum PlayerMove {
	WALKING,
	RUNNING,
	JUMP,
	STAND
};

enum PlayerAttackType {
	SWORD1HL1,
	SWORD1HL2,
	SWORD1HR,
	SWORD1HR2,
	BOWL,
	BOWR,
};

enum PlayerType {
	PT_BASIC,
	PT_SWORD1H,
	PT_BOW,
	PT_SWORD2H,
	PT_SPEAR2H
};

enum MonsterType {
	Metalon,
	Wolf,
	Dragon
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
	char id[20];
	char pw[20];
};

struct player_loginOK_packet :public Packet {
	DirectX::XMFLOAT3 Position;
	float dx, dy;
	short PlayerType;
};

struct player_loginFail_packet :public Packet {
};

struct room_create_packet :public Packet {
	char name[20];
};

struct room_select_packet :public Packet {
	short room;
	short ingamekey;
	bool isMaster;
};

struct room_list_packet :public Packet {
	char name[20];
	short idx;
};

struct refresh_lobby_packet : public Packet {

};

struct player_add_packet : public Packet {
	DirectX::XMFLOAT3 Position;
	float dx, dy;
	PlayerType WeaponType;
};

struct game_ready_packet :public Packet {
	PlayerType weaponType;
};

struct game_start_packet :public Packet {
	PlayerType weaponType;
	short ingamekey;
	XMFLOAT3 pos;
	short leftplayer;
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
	short ingamekey;
	char state;
	DirectX::XMFLOAT3 Position;
	float dx, dy;
	DWORD MoveType;
	DWORD dir;
	PlayerType playertype;
};

struct player_start_pos : public Packet {
	DirectX::XMFLOAT3 Position;
};

struct player_move_packet : public Packet {
	short ingamekey;
	char state;
	DirectX::XMFLOAT3 Position;
	DWORD MoveType;
	DWORD direction;
	float dx, dy;
};

struct player_rotate_packet : public Packet {
	short ingamekey;
	float dx, dy;
};

struct player_status_packet : public Packet {
	short ingamekey;
	char state;
};

struct player_stat_packet : public Packet {
	short ingamekey;
	float hp;
	float lv;
	float speed;
};

struct Weapon_swap_packet : public Packet {
	short ingamekey;
	PlayerType weapon;
};

struct Weapon_select_packet : public Packet {
	short ingamekey;
	PlayerType weapon;
};

struct player_equipment_packet : public Packet {
	short ingamekey;
	char armor;
	char helmet;
	char shoes;
};

struct player_attack_packet : public Packet {
	short ingamekey;
	char attack_type;
	float dx, dy;
};

struct player_shot_packet : public Packet {
	short ingamekey;
	DirectX::XMFLOAT3 Look;
	float fTimeElapsed;
	float ChargeTimer;
	float dx, dy;
};

struct player_arrow_packet : public Packet {
	short ingamekey;
	char attack_type;
	float fSpeed;
};

struct player_damage_packet : public Packet {
	short ingamekey;
	unsigned short damage;
	short target;
	short nAttack;
	float leftHp;
};

struct player_stop_packet : public Packet {
	short ingamekey;
	DirectX::XMFLOAT3 Position;
	float dx, dy;
	PlayerType playertype;
};

struct player_dead_packet : public Packet {
	short ingamekey;
};

struct map_block_set : public Packet {
	char block_type[9];
	std::chrono::system_clock::time_point GameStartTime;
};

struct map_collapse_packet : public Packet {
	char block_num;
	short index[2];
};

struct cloud_move_packet : public Packet {
	float x, z;
	std::chrono::system_clock::time_point GameStartTime;
};

struct mon_add_packet : public Packet {
	DirectX::XMFLOAT3 Position;
	float dx, dy, dz;
	short MonsterType;
};

struct mon_remove_packet : public Packet {
};

struct mon_pos_packet : public Packet {
	char state;
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 direction;
	float degree;
	DWORD MoveType;
	short MonsterType;
	short target;
};

struct mon_move_packet : public Packet {
	char state;
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 direction;
	float degree;
	DWORD MoveType;
	short MonsterType;
	short target;
};

struct mon_attack_packet : public Packet {
	DirectX::XMFLOAT3 direction;
	float degree;
	DWORD MoveType;
	int target;
	float PlayerLeftHp;
	float attack_dis;
};

struct mon_damaged_packet : public Packet {
	short ingamekey;
	unsigned short damage;
	short target;
	short nAttack;
	float leftHp;
};

struct mon_respawn_packet : public Packet {
	DirectX::XMFLOAT3 Position;
	float dx, dy, dz;
	short MonsterType;
};

struct mon_stop_packet : public Packet {
	DirectX::XMFLOAT3 Position;
};

struct player_record_packet : public Packet {
	char id[50];
	short survivalTime;
	short rank;
	short weapon1;
	short weapon2;
	short helmet;
	short shoes;
	short armor;
};

struct player_getitem_packet :public Packet {
	short item;
};

struct return_lobby_packet :public Packet {
};

struct create_account_packet :public Packet {
	char id[20];
	char pw[20];
	bool canmake;
};

struct mon_behavior_event : public Packet {
	std::chrono::system_clock::time_point GameStartTime;
};

struct mon_move_event : public Packet {
	XMFLOAT3 subtract;
	XMFLOAT3 direction;
	short target;
	std::chrono::system_clock::time_point GameStartTime;
};

struct mon_attack_cooltime_event : public Packet {
	std::chrono::system_clock::time_point GameStartTime;
};

struct mon_respawn_event : public Packet {
	std::chrono::system_clock::time_point GameStartTime;
};

struct mon_attack_event : public Packet {
	XMFLOAT3 direction;
	short target;
	std::chrono::system_clock::time_point GameStartTime;
};

struct mon_stop_event : public Packet {
	std::chrono::system_clock::time_point GameStartTime;
};

struct Mapbreak_event : public Packet {
	std::chrono::system_clock::time_point GameStartTime;
};

struct game_end_event : public Packet {
	std::chrono::system_clock::time_point GameStartTime;
};
#pragma pack(pop)