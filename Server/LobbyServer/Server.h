#pragma once
#include"stdafx.h"
#include "DB.h"
#include "protocol.h"

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

    char* packet_start;
    char* recv_start;

    std::atomic<bool>        connected = false;
    bool                     isready = false;
    bool                     playing = false;
    int                      prev_size;
    std::atomic<int>         key = -1;
    std::atomic<int>         roomID = -1;
    char                     id[50];

    // 0 Á×À½ / 1 »ýÁ¸
    std::atomic<bool>       state = 0;
    std::atomic<DirectX::XMFLOAT3>  f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::atomic<float>      dx = 0;
    std::atomic<float>      dy = 0;

    std::atomic<short>      weapon1 = PlayerType::PT_BASIC;
    std::atomic<short>      weapon2 = PlayerType::PT_BASIC;
    std::atomic<short>      helmet = 0;
    std::atomic<short>      shoes = 0;
    std::atomic<short>      armor = 0;

    std::atomic<float>      hp = 0;
    std::atomic<float>      lv = 0;
    std::atomic<float>      speed = 20;

    std::atomic<short>      inventory[INVENTORY_MAX]{};

    void init();

public:
    std::unordered_set<int> near_monster;

    std::mutex               s_lock;
    std::mutex               nm_lock;
};

class DB;

class Server {
public:
    Server();
    ~Server();

    void display_error(const char* msg, int err_no);

    int SetClientKey();
    bool MatchMaking(int cnt);

    bool Init();
    void Thread_join();
    void Disconnected(int id);

    void Connect_Game_Server();
    void Accept();
    void WorkerFunc();

    void do_recv(char id);
    void send_packet(int to, char* packet);
    void process_packet(char id, char* buf);

    void send_key_player_packet(char key);
    void send_player_loginOK_packet(char key);
    void send_player_loginFail_packet(char key);
    void send_disconnect_player_packet(char id);

    void send_game_start_packet(char id);

    void Set_pDB(DB* d) { m_pDB = d; }

private:
    HANDLE hcp;
    SOCKET listen_sock;

    DB* m_pDB = NULL;

    bool Connected_Game_Server = false;

    std::unordered_map <int, SESSION> sessions;

    std::unordered_map <int, int> room_player_cnt;

    std::vector <std::thread> working_threads;
    std::thread accept_thread;

    std::mutex                     sessions_lock;
    std::mutex                     room_player_cnt_lock;
};