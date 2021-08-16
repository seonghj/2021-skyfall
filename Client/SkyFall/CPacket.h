#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "protocol.h"
#include "GameFramework.h"

class CGameFramework;
class player;

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
	void Send_start_packet(PlayerType t);
	void Send_attack_packet(int type);
	void Send_stop_packet();
	void Send_login_packet(char* id, char* pw);
	void Send_swap_weapon_packet(PlayerType weapon);
	void Send_damage_to_player_packet(int target, int nAttack);
	void Send_mon_damaged_packet(int target, int nAttack);
	void Send_room_create_packet(const char* name);
	void Send_room_select_packet(int room);
	void Send_return_lobby_packet();
	void Send_refresh_room_packet();
	void Send_create_account_packet(char* id, char* pw);

	void Swap_weapon(int key, PlayerType weapon);
	void Map_set(map_block_set* p);
	void CheckCollision(CMonster* mon);
	int MonsterAttackCheck(CMonster* mon);

	void ProcessPacket(char* buf);

	void Login();

	void Set_clientkey(int n);
	int Get_clientkey();

	void Set_StartWeapon(PlayerType t) { start_weapon = t; }
	PlayerType Get_StartWeapon() { return start_weapon; }

	void Set_currentfps(unsigned long FrameRate);

	void LobbyConnect();
	void GameConnect();

	std::thread Recv_thread;

	bool canmove = false;

	void Set_UserID(char* ID) { strcpy_s(userID, ID); 
	cout << "ID: " << userID << endl;
	}
	void Set_IP(char* IP) { strcpy_s(ipaddr, IP); 
	cout << "IP: " << ipaddr << endl;
	}

	int roomID = INVALIDID;
	int InGamekey = INVALIDID;
private:
	int client_key = INVALIDID;
	char userID[50];
	unsigned long currentfps = 1;

	bool isLogin = false;

	HANDLE                         hcp;
	std::vector <std::thread>      working_threads;

	PlayerType start_weapon = PlayerType::PT_SWORD1H;

	vector<vector<int>> m_vMapArrange = { { 0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0, 2}, {1, 2}, {2, 2} };

	//static DWORD WINAPI ServerConnect(LPVOID arg);

	enum PlayerState {
		Idle = 0,
		Death,
		Jump,
		Walk,
		Run,
		RunBack,
		RunRight,
		RunLeft,
		Take_Damage,
	};

	enum MonsterState {
		m_Idle,
		m_Die,
		m_Take_Damage,
		m_Walk,
		m_Run,
	};

	int TotalPlayer = 0;
};
