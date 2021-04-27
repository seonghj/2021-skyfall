#pragma once
#include"stdafx.h"
#include "player.h"
#include "Scene.h"

constexpr int GAMESERVERPORT = 3500;
constexpr int LOBBYPORT = 4000;
constexpr int BUFSIZE = 1024;
constexpr int MAX_CLIENT = 100;
constexpr int MAX_PLAYER = 20;

constexpr int LOBBY_ID = 0;

constexpr int MAX_MAP_BLOCK = 9;
constexpr int MAP_SIZE = 3000;
constexpr int MAP_BLOCK_SIZE = 1000;
constexpr int MAP_BREAK_TIME = 30;

constexpr float VIEWING_DISTANCE = 500.f;

#define SERVERIP   "127.0.0.1"


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
	Type_player_pos,
	Type_start_pos,
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

enum PlayerState {
	DEAD,
	ALIVE,
};

enum PlayerMove {
	WAKING,
	RUNNING,
	JUMP
};

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

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
	char id;
	char state;
	DirectX::XMFLOAT3 Position;
	float dx, dy, dz;
};

struct player_start_pos : public Packet {
	char id;
	DirectX::XMFLOAT3 Position;
};

struct player_move_packet : public Packet {
	char id;
	char state;
	DWORD MoveType;
	float dx, dy, dz;
};

struct player_status_packet : public Packet {
	char id;
	char state;
};

struct player_stat_packet : public Packet {
	char id;
	float hp;
	float lv;
	float speed;
};

struct player_weapon_packet : public Packet {
	char id;
	char weapon;
};

struct player_equipment_packet : public Packet {
	char id;
	char armor;
	char helmet;
	char shoes;
};

struct player_attack_packet : public Packet {
	char id;
	char attack_type;
	DirectX::XMFLOAT3 Position;
	float damage;
};

struct map_collapse_packet : public Packet {
	char block_num;
};

struct cloud_move_packet : public Packet {
	float x, z;
};


#pragma pack(pop)

class PacketFunc {
public:

	PacketFunc();
	~PacketFunc();

	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char Sendbuf[BUFSIZE];
	char Recvbuf[BUFSIZE];
	DWORD recvbytes;
	DWORD sendbytes;
	WSABUF wsabuf;
	HANDLE SendEvent;

	CPlayer* m_pPlayer = NULL;
	CScene* m_pScene = NULL;

	float fTimeElapsed;
	float ChargeTimer;

	void err_quit(char* msg);
	void err_display(char* msg);

	void RecvPacket();
	void SendPacket(char* buf);
	void Send_ready_packet();
	void ProcessPacket(char* buf);

	void Set_clientid(int n);
	int Get_clientid();
	void Set_currentfps(unsigned long FrameRate);

	void LobbyConnect();
	void GameConnect();


private:
	int client_id;
	unsigned long currentfps = 1;

	std::thread Recv_thread;
	//static DWORD WINAPI ServerConnect(LPVOID arg);
};