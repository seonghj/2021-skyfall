#pragma once
#include"stdafx.h"
#include "player.h"
#include "Scene.h"
#include "protocol.h"


constexpr int GAMESERVERPORT = 3500;
constexpr int LOBBYPORT = 4000;
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

	CPacket();
	~CPacket();

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