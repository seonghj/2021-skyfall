#pragma once
#include "stdafx.h"
#include "player.h"
#include "Scene.h"
#include "protocol.h"
#include "GameFramework.h"

class CGameFramework;

class CPacket {
public:

	CPacket();
	~CPacket();

	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char Sendbuf[BUFSIZE];
	char Recvbuf[BUFSIZE];
	char ipaddr[50];
	DWORD recvbytes;
	DWORD sendbytes;
	WSABUF wsabuf;
	HANDLE SendEvent;

	CTerrainPlayer*		m_pPlayer = NULL;
	CGameFramework*		m_pFramework = NULL;
	CScene*				m_pScene = NULL;
	CMap*				m_pMap = NULL;

	float fTimeElapsed;
	float ChargeTimer;

	void err_quit(char* msg);
	void err_display(char* msg);

	void RecvPacket();
	void SendPacket(char* buf);
	void Send_ready_packet();
	void Send_attack_packet(int type);
	void Send_stop_packet();
	void Send_login_packet(char* id);
	void Send_swap_weapon_packet(PlayerType weapon);
	void Swap_weapon(int key, PlayerType weapon);
	void Map_set(map_block_set* p);
	void CheckCollision(CMonster* mon);
	int MonsterAttackCheck(CMonster* mon);

	void ProcessPacket(char* buf);

	void Login();

	void Set_clientkey(int n);
	int Get_clientkey();
	void Set_currentfps(unsigned long FrameRate);

	void LobbyConnect();
	void GameConnect();

	std::thread Recv_thread;

	bool canmove = false;

	void Set_UserID(char* ID) { strcpy_s(userID, ID); 
	cout << userID << endl;
	}
	void Set_IP(char* IP) { strcpy_s(ipaddr, IP); 
	cout << ipaddr << endl;
	}

private:
	int client_key = INVALIDID;
	int roomID = INVALIDID;
	char userID[50];
	unsigned long currentfps = 1;

	bool isLogin = false;

	XMFLOAT3 Mon_pos_before_error[50];

	//static DWORD WINAPI ServerConnect(LPVOID arg);
};

enum animation_1HSword{
	SH1_Idle = 0,
	SH1_Jump = 1,
	SH1_Run = 2,
	SH1_RunBack = 3,
	SH1_RunLeft = 4,
	SH1_RunRight = 5,
	SH1_Attack1 = 6,
	SH1_Attack2 = 7,
	SH1_Attack3 = 8,
	SH1_Attack4 = 9,
	SH1_TakeDamage = 10,
	SH1_Walk = 11,
	SH1_Death = 12
};

enum animation_Bow {
	B_Idle = 0,
	B_Jump = 1,
	B_Run = 2,
	B_RunBack = 3,
	B_RunLeft = 4,
	B_RunRight = 5,
	B_ShotHold = 6,
	B_ShotReady = 7,
	B_ShotRelease = 8,
	B_TakeDamage = 9,
	B_Walk = 10,
	B_Death = 11
};