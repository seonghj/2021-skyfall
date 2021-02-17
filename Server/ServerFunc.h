#pragma once
#include"stdafx.h"
#include "DB.h"

struct OVER_EX
{
    WSAOVERLAPPED	overlapped;
    WSABUF			dataBuffer;
    char			messageBuffer[BUFSIZE];
    bool			is_recv;
};

struct SOCKETINFO
{
    bool	connected;
    OVER_EX over;
    SOCKET sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char packet_buf[BUFSIZE];
    int prev_size;
};

int recvn(SOCKET s, char* buf, int len, int flags);
void err_quit(char* msg);
void err_display(char* msg);

class IOCPServer {
public:
    IOCPServer();
    ~IOCPServer();

    int get_new_id();

    bool Init();
    void Run();
    void Disconnect(int id);

    void do_accept();
    void WorkerFunc();

    void do_recv(char id);
    void do_send(int to, char* packet);
    void process_packet(char id, char* buf);

    void send_login_player_packet(char to, char id);
    void send_disconnect_player_packet(char to, char id);
    void send_player_pos_packet(char id);
    void send_player_attack_packet(char id);
    void send_map_collapse_packet(int num);
    void send_cloud_move_packet(int x, int y);

private:
    HANDLE hcp;
    SOCKETINFO clients[MAX_CLIENT];

    std::vector <std::thread> working_threads;
    std::thread accept_thread;
};