#pragma once
#include "stdafx.h"


struct Packet {
public:
	char size;
	char type;
};

enum PacketType {
	T_player_ID,
	T_player_login,
	T_player_remove,
	T_player_info,
	T_player_move,
	T_player_pos,
	T_player_attack,
	T_map_collapse,
	T_cloud_move
};

enum MoveType {
	FRONT,
	RIGHT,
	LEFT,
	BACK,
	STOP
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

//struct player_pos_packet : public Packet {
//	char id;
//	float x, y, z;
//	float degree;
//};

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

class PacketFunc {
public:

	PacketFunc();
	~PacketFunc();

	//WSAOVERLAPPED overlapped;
	SOCKET sock;
	char Sendbuf[BUFSIZE];
	char Recvbuf[BUFSIZE];
	DWORD recvbytes;
	DWORD sendbytes;
	WSABUF wsabuf;
	HANDLE SendEvent;

	void err_quit(char* msg);
	void err_display(char* msg);
	int recvn(SOCKET s, char* buf, int len, int flags);

	void RecvPacket();
	void SendPacket();
	void ProcessPacket(char* buf);

	void Set_clientid(int n);
	int Get_clientid();
	void Set_currentfps(unsigned long FrameRate);


private:
	int client_id;
	unsigned long currentfps = 1;
	//static DWORD WINAPI ServerConnect(LPVOID arg);
};