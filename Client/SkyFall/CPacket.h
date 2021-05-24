#pragma once
#include"stdafx.h"
#include "player.h"
#include "Scene.h"
#include "protocol.h"

class CPacket {
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
	void Send_attack_packet(int type);
	void Send_animation_stop_packet();
	void ProcessPacket(char* buf);

	void Set_clientid(int n);
	int Get_clientid();
	void Set_currentfps(unsigned long FrameRate);

	void LobbyConnect();
	void GameConnect();


private:
	int client_id = INVALIDID;
	int roomID = INVALIDID;
	unsigned long currentfps = 1;

	std::thread Recv_thread;
	//static DWORD WINAPI ServerConnect(LPVOID arg);
};