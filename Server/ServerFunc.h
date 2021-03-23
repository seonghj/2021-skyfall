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

struct player_infomation {
    std::atomic<float> x = 0;
    std::atomic<float> y = 0;
    std::atomic<float> z = 0;
    std::atomic<float> degree = 0;
    std::atomic<int> weapon = 0;

    // 0 ���� / 1 ����
    std::atomic<bool> state = 0;

    std::atomic<float> hp = 0;
    std::atomic<float> speed = 10;
};

bool CASfloat(std::atomic<float>* addr, int expected, int new_val);


class IOCPServer {
public:
    IOCPServer();
    ~IOCPServer();

    void display_error(const char* msg, int err_no);

    int get_new_id();

    bool Init();
    void Thread_join();
    void Disconnect(int id);

    void do_accept();
    void WorkerFunc();

    void do_recv(char id);
    void do_send(int to, char* packet);
    void process_packet(char id, char* buf);

    void send_ID_player_packet(char id);
    void send_login_player_packet(char id, int to);
    void send_disconnect_player_packet(char id);
    void send_player_move_packet(char id);
    void send_player_attack_packet(char id, char* buf);
    void send_map_collapse_packet(int num);
    void send_cloud_move_packet(float x, float z);

    float calc_distance(int a, int b);

private:
    HANDLE hcp;

    SOCKETINFO clients[MAX_CLIENT];
    player_infomation player_info[MAX_CLIENT];

    std::vector <std::thread> working_threads;
    std::thread accept_thread;
};