#pragma once
#include"stdafx.h"
#include "DB.h"

#define SERVERPORT 3500
#define BUFSIZE    1024
#define MAX_CLIENT 100
#define MAX_PLAYER 20

#define MAX_MAP_BLOCK 9
#define MAP_SIZE 3000
#define MAP_BLOCK_SIZE 1000
#define MAP_BREAK_TIME 30

#define VIEWING_DISTANCE 500

struct OVER_EX
{
    WSAOVERLAPPED	overlapped;
    WSABUF			dataBuffer;
    char			messageBuffer[BUFSIZE];
    bool			is_recv;
};

class SESSION
{
public:

    SESSION() {}
    SESSION(const SESSION& s) {}
    ~SESSION() {}

    OVER_EX     over;
    SOCKET      sock;
    SOCKADDR_IN clientaddr;
    int         addrlen;
    char        packet_buf[BUFSIZE];

    char*       packet_start;
    char*       recv_start;

    bool        connected = false;
    int         prev_size;
    int         id;
    bool        isready = false;

    std::atomic<float>      x = 0;
    std::atomic<float>      y = 0;
    std::atomic<float>      z = 0;
    std::atomic<float>      degree = 0;
    std::atomic<int>        weapon = 0;

    // 0 ���� / 1 ����
    std::atomic<bool>       state = 0;

    std::atomic<float>      hp = 0;
    std::atomic<float>      lv = 0;
    std::atomic<float>      speed = 10;
};

//namespace std {
//    template<>
//    class hash<SESSION> {
//    public:
//        size_t operator() (const SESSION& s) const { 
//            return std::hash<int>()(s.id);
//        }
//    };
//}


class Server {
public:
    Server();
    ~Server();

    void display_error(const char* msg, int err_no);

    int SetClientId();

    bool Init();
    void Thread_join();
    void Disconnected(int id);

    void Accept();
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

    //SESSION players[MAX_CLIENT];

    std::unordered_map <int, SESSION> players;

    std::vector <std::thread> working_threads;
    std::thread accept_thread;
};