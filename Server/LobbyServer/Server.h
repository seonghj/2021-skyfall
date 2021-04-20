#pragma once
#include"stdafx.h"
#include "DB.h"
#include "CPacket.h"

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

    char* packet_start;
    char* recv_start;

    bool        connected = false;
    bool        isready = false;
    bool        playing = false;

    int         prev_size;
    int         id;
    int         game_num;

    // 0 Á×À½ / 1 »ýÁ¸
    std::atomic<bool>       state = 0;

    std::atomic<DirectX::XMFLOAT3>  f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::atomic<float>      dx = 0;
    std::atomic<float>      dy = 0;
    std::atomic<float>      dz = 0;
    /*std::atomic<float>      x = 0;
    std::atomic<float>      y = 0;
    std::atomic<float>      z = 0;*/

    std::atomic<short>        weapon = 0;
    std::atomic<short>        helmet = 0;
    std::atomic<short>        shoes = 0;

    std::atomic<float>      hp = 0;
    std::atomic<float>      lv = 0;
    std::atomic<float>      speed = 10;
}

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
    bool MatchMaking(int cnt);

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

    void send_game_start_packet(char id);

private:
    HANDLE hcp;

    bool Connected_Game_Server = false;

    std::unordered_map <int, SESSION> sessions;

    std::vector <std::thread> working_threads;
    std::thread accept_thread;
};