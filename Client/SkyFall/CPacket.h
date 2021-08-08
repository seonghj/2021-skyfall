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
	void Send_ready_packet(PlayerType t);
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

private:
	int client_key = INVALIDID;
	int roomID = INVALIDID;
	char userID[50];
	unsigned long currentfps = 1;

	bool isLogin = false;

	HANDLE                         hcp;
	std::vector <std::thread>      working_threads;

	PlayerType start_weapon;

	//static DWORD WINAPI ServerConnect(LPVOID arg);
};
