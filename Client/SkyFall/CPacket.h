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
	void Send_stop_packet();
	void Send_login_packet(char* id);
	void ProcessPacket(char* buf);

	void Login();

	void Set_clientkey(int n);
	int Get_clientkey();
	void Set_currentfps(unsigned long FrameRate);

	void LobbyConnect();
	void GameConnect();

	std::thread Recv_thread;

private:
	int client_key = INVALIDID;
	int roomID = INVALIDID;
	char userID[50];
	unsigned long currentfps = 1;
	//static DWORD WINAPI ServerConnect(LPVOID arg);
};