#pragma once
#include "stdafx.h"
#include "DB.h"
#include "Map.h"
#include "protocol.h"

//struct OVER_EX
//{
//    WSAOVERLAPPED	overlapped;
//    WSABUF			dataBuffer;
//    char			messageBuffer[BUFSIZE];
//    bool			is_recv;
//    int             type;
//    // 0 = session 1 = map
//};

class Arrow {
public:
    DirectX::XMFLOAT3       f3Position;

};



class SESSION
{
public:

    SESSION() {}
    SESSION(const SESSION& session) {}
    SESSION(BOOL b) : connected(b) {}
    ~SESSION() {}

    OVER_EX                  over;
    SOCKET                   sock;
    SOCKADDR_IN              clientaddr;
    int                      addrlen;
    char                     packet_buf[BUFSIZE];
        
    char*                    packet_start;
    char*                    recv_start;

    std::atomic<bool>        connected = false;
    bool                     isready = false;
    bool                     playing = false;
    int                      prev_size;
    int                      id = -1;

    // 0 ���� / 1 ����
    std::atomic<bool>       state = 0;
    DirectX::XMFLOAT3       f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::atomic<float>      dx = 0;
    std::atomic<float>      dy = 0;
    
    std::atomic<short>      weapon = PlayerType::PT_BASIC;
    std::atomic<short>      helmet = 0;
    std::atomic<short>      shoes = 0;

    std::atomic<float>      hp = 0;
    std::atomic<float>      lv = 0;
    std::atomic<float>      speed = 20;

    void init();
    void Setf3Posion(DirectX::XMFLOAT3 f) { f3Position = f; }
};

class Map;

class Server {
public:
    Server();
    ~Server();

    HANDLE Gethcp() { return hcp; }

    void display_error(const char* msg, int err_no);

    int SetClientId(int roomID);
    int SetroomID();

    void ConnectLobby();

    bool Init();
    void Thread_join();
    void Disconnected(int id, int roomID);

    void Accept();
    void WorkerFunc();

    void do_recv(int id, int roomID);
    void send_packet(int to, char* packet, int roomID);
    void process_packet(int id, char* buf, int roomID);

    void send_ID_player_packet(int id, int roomID);
    void send_player_loginOK_packet(int id, int roomID);
    void send_add_player_packet(int id, int to, int roomID);
    void send_disconnect_player_packet(int id,int roomID);
    void send_packet_to_players(int id, char* buf, int roomID);
    void send_packet_to_allplayers(int roomnum, char* buf);
    void send_map_collapse_packet(int num, int roomID);
    void send_cloud_move_packet(float x, float z, int roomID);
    
    void game_end(int roomnum);

    float calc_distance(int a, int b, int roomID);
    unsigned short calc_attack(int id, char attacktype);

    std::unordered_map <int, std::array<SESSION, 20>> sessions; // ��ID, Player�迭

private:
    HANDLE hcp;
    
    std::unordered_map <int, Map> maps;

    std::vector <std::thread> working_threads;
    std::thread accept_thread;
    std::vector <std::thread> map_threads;

    std::mutex accept_lock;
    std::mutex sm_lock;
    std::mutex mm_lock;
};