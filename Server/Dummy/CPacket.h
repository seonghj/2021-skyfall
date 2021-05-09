#pragma once
#include"stdafx.h"
#include "player.h"
#include "protocol.h"

enum OPTYPE { OP_SEND, OP_RECV, OP_DO_MOVE };

class Player;

class CPacket {
public:

	CPacket();
	~CPacket();


	float fTimeElapsed;
	float ChargeTimer;

	void err_quit(char* msg);
	void err_display(char* msg);

	void WorkerThread();

	void do_recv(int id);
	void SendPacket(int id, char* buf);
	void Send_ready_packet();
	void ProcessPacket(int id,char* buf);

	void Set_clientid(int n);
	int Get_clientid();
	void Set_currentfps(unsigned long FrameRate);

	void TestGameConnect();

	bool Init();
	void Thread_join();
	void Test_Thread();

	Player players[MAX_CLIENT];
	RECT rcTmpPlayer[MAX_CLIENT];

private:
	//int client_id;
	unsigned long currentfps = 1;
	HANDLE hcp;

	std::vector <std::thread> working_threads;

	std::thread  test_threads;

	int num_connections = 0;
	int client_id;
};