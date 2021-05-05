#pragma once
#include"stdafx.h"
#include "DB.h"
#include "Map.h"
#include "protocol.h"

struct OVER_EX
{
    WSAOVERLAPPED	overlapped;
    WSABUF			dataBuffer;
    char			messageBuffer[BUFSIZE];
    bool			is_recv;
    int             type;
    // 0 = session 1 = map
};

class SESSION
{
public:

    SESSION() {}
    SESSION(const int& i) { id = i; }
    ~SESSION() {}

    OVER_EX     over;
    SOCKET      sock;
    SOCKADDR_IN clientaddr;
    int         addrlen;
    char        packet_buf[BUFSIZE];

    char*       packet_start;
    char*       recv_start;

    bool        connected = false;
    bool        isready = false;
    bool        playing = false;
    int         prev_size;
    int         id;
    int         gameroom_num;

    // 0 죽음 / 1 생존
    std::atomic<bool>       state = 0;

    std::atomic<DirectX::XMFLOAT3>  f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::atomic<float>      dx = 0;
    std::atomic<float>      dy = 0;
    std::atomic<float>      dz = 0;
    
    std::atomic<short>        weapon = 0;
    std::atomic<short>        helmet = 0;
    std::atomic<short>        shoes = 0;

    std::atomic<float>      hp = 0;
    std::atomic<float>      lv = 0;
    std::atomic<float>      speed = 20;
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

class Map;

class Server {
public:
    Server();
    ~Server();

    HANDLE Gethcp() { return hcp; }

    void display_error(const char* msg, int err_no);

    int SetClientId();
    int SetGameNum();

    void ConnectLobby();

    bool Init();
    void Thread_join();
    void Disconnected(int id);

    void Accept();
    void WorkerFunc();

    void do_recv(char id);
    void send_packet(int to, char* packet);
    void process_packet(char id, char* buf);

    void send_ID_player_packet(char id);
    void send_login_player_packet(char id, int to);
    void send_disconnect_player_packet(char id);
    void send_packet_to_players(char id, char* buf);
    void send_packet_to_players(int game_num, char* buf);
    void send_map_collapse_packet(int num, int map_num);
    void send_cloud_move_packet(float x, float z, int map_num);
    
    void game_end(int game_num);

    float calc_distance(int a, int b);
    DirectX::XMFLOAT3 move_calc(DWORD dwDirection, float fDistance, int state, int id);

private:
    HANDLE hcp;
    
    std::unordered_multimap <int, int> gameroom; // <방번호, 플레이아ID>
    std::unordered_map <int, SESSION> sessions;
    std::unordered_map <int, Map> maps;

    std::vector <std::thread> working_threads;
    std::thread accept_thread;
    std::vector <std::thread> map_threads;
};