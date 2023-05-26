#pragma once
#include "stdafx.h"
#include "DB.h"
#include "Map.h"
#include "protocol.h"
#include "Timer.h"
#include "Bot.h"

constexpr int INVALUED_ID = -1;
constexpr int SERVER_ID = 0;

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
    int                      prev_size = 0;

    int                      key = -1;

    std::atomic<int>         roomID = -1;
    char                     id[50];

    int                      InGamekey = -1;

    std::atomic<bool>       state = 0;
    std::atomic<DirectX::XMFLOAT3>  f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::atomic<float>      m_fPitch = 0;
    std::atomic<float>      m_fYaw = 0;
    XMFLOAT3			    m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
    XMFLOAT3				m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3				m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
    
    std::atomic<PlayerType>      weapon1 = PlayerType::PT_BASIC;
    std::atomic<PlayerType>      weapon2 = PlayerType::PT_BASIC;
    std::atomic<short>      helmet = 0;
    std::atomic<short>      shoes = 0;
    std::atomic<short>      armor = 0;

    std::atomic<float>      hp = 100;
    std::atomic<float>      def = 0;
    std::atomic<float>      lv = 0;
    std::atomic<float>      att = 10;
    std::atomic<float>      speed = 20;
    std::atomic<float>      proficiency = 0.0f;

    PlayerType              using_weapon = PlayerType::PT_BASIC;

    std::atomic<short>      inventory[INVENTORY_MAX]{};

    int moveframe = -1;

    void init();

    DirectX::XMFLOAT3 GetPosition() { return f3Position; }

    void TakeDamage(int iDamage) { hp -= iDamage * (100 - def) / 100; };
    void AddProficiency() { proficiency += 0.06; }
    float GetAtkDamage() const { return(att + att * (proficiency)); }
    void Reset();

    void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity, bool isRun);
    void Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity, bool isRun);

public:
    std::unordered_set<int> near_monster;

    std::mutex               s_lock;
    std::mutex               nm_lock;
};

class Map;
class DB;
class Bot;
class Monster;

class GameRoom {
public:
    bool        isMade = false;
    int         pkeys[MAX_PLAYER];
    char        name[20];
    bool        CanJoin = false;
    int         TotalPlayer = 0;
    int         master = INVALIDID;
    Map*        m_pMap;
    std::mutex  r_lock;
};

class Server {
public:
    Server();
    ~Server();

    HANDLE Gethcp() { return hcp; }
    Timer* Get_pTimer() { return m_pTimer; }

    void display_error(const char* msg, int err_no);

    int SetInGameKey(int roomID);
    int SetroomID();
    int SetLobbyKey();

    int CreateRoom(int* key, char* name);



    void Set_pTimer(Timer* t) { m_pTimer = t; }
    void Set_pBot(Bot* b) { m_pBot = b; }
    void Set_pDB(DB* d) { m_pDB = d; }


    bool Init();
    void Thread_join();
    void Disconnect(int key);

    void Accept();
    void WorkerFunc();

    void do_recv(int key, int roomID);
    void send_packet(int to, char* packet, int roomID);
    void process_packet(int key, char* buf, int roomID);
    void ProcessEvent(OVER_EX* over_ex, int roomID, int key);

    void send_Lobby_key_packet(int key);
    void send_Lobby_loginOK_packet(int key);

    void send_room_list_packet(int key);

    void send_player_InGamekey_packet(int key, int roomID);
    void send_player_loginOK_packet(int key, int roomID);
    void send_player_loginFail_packet(int key, int roomID);

    void send_add_player_packet(int key, int to, int roomID);
    void send_remove_player_packet(int key, int roomID);

    void send_disconnect_player_packet(int key,int roomID);

    void send_packet_to_players(int key, char* buf, int roomID);
    void send_packet_to_allplayers(int roomnum, char* buf);

    void send_start_packet(int to, int roomID);
    void send_game_end_packet(int key, int roomID);
    void send_player_dead_packet(int key, int roomID);

    void player_go_lobby(int key, int roomID);

    void send_map_collapse_packet(int num, int roomID);
    void send_cloud_move_packet(float x, float z, int roomID);

    void send_add_monster(int key, int roomID, int to);
    void send_remove_monster(int key, int roomID, int to);
    void send_monster_pos(Monster& mon, XMFLOAT3 direction, int target);
    void send_monster_move(Monster& mon, XMFLOAT3 direction, int target);
    void send_monster_attack(Monster& mon, XMFLOAT3 direction, int target);
    void send_monster_stop(int key, int roomID);

    void send_player_record(int key, int roomID, const SESSION& s, int time, int rank);
    void send_map_packet(int to, int roomID);

    void game_end(int roomnum);

    void Delete_room(int roomID);

    bool in_VisualField(SESSION& a, SESSION& b, int roomID);
    bool in_VisualField(Monster& a, SESSION& b, int roomID);
    unsigned short calc_attack(int key, char attacktype);

    float CalcDamageToMon(int att, int def) { return (att * (100 - def) / 100); }

    void player_move(int key, int roomID, DirectX::XMFLOAT3 pos, float dx, float dy);

    std::array <GameRoom, 1000> GameRooms;
    std::array <SESSION, 1000> sessions;

private:
    SOCKET                         listenSocket;
    HANDLE                         hcp;
    Timer*                         m_pTimer = NULL;
    Bot*                           m_pBot = NULL;
    DB*                            m_pDB = NULL;


    std::vector <std::thread>      working_threads;
    std::thread                    accept_thread;
    std::thread                    timer_thread;

    std::mutex                     sessions_lock;
    std::mutex                     GameRooms_lock;
    std::mutex                     maps_lock;
};