#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "Server.h"
#include "Timer.h"

class SESSION;
class Server;
class Timer;

class Monster{
public:

    Monster() {}
    Monster(const Monster& m) {}
    ~Monster() {}

    XMFLOAT4X4				 m_xmf4x4ToParent = Matrix4x4::Identity();
    XMFLOAT4X4				 m_xmf4x4World = Matrix4x4::Identity();

    std::mutex               Mon_lock;

    int                     key = -1;
    std::atomic<int>         roomID = -1;
    char                     id[50];

    // 0 Á×À½ / 1 »ýÁ¸
    std::atomic<bool>       state = 0;
    MonsterType             type = MonsterType::None;
    std::atomic<DirectX::XMFLOAT3>       f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::atomic<DirectX::XMFLOAT3>       SpawnPos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::atomic<float>      m_fPitch = 0;
    std::atomic<float>      m_fYaw = 0;
    std::atomic<float>      m_fRoll = 0;

    std::atomic<float>      hp = 100;
    std::atomic<float>      def = 0;
    std::atomic<float>      lv = 0;
    std::atomic<float>      att = 10;
    std::atomic<float>      speed = 20;

    std::atomic<DirectX::XMFLOAT3>      before_dir = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

    bool                    CanAttack = TRUE;
    short                   TraceTarget = INVALIDID;

    void init();
    void SetPosition(float x, float y, float z);
    void Move(const XMFLOAT3& vDirection, float fSpeed);
    void Rotate(float fPitch, float fYaw, float fRoll);
    void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent);

    void TakeDamage(int iDamage) { hp -= iDamage * (100 - def) / 100; };

    bool recv_pos = TRUE;

    DirectX::XMFLOAT3 GetUp();

    DirectX::XMFLOAT3 GetPosition() { return f3Position; }
};

class Bot {
public:
    void Set_pServer(Server* s) { m_pServer = s; };
    void Set_pTimer(Timer* t) { m_pTimer = t; };

    void Init(int roomID);

    void CheckBehavior(int roomID);

    void RunBot(int roomID);

    void push_mon_attack_event(int monsterKey, int target, int roomID, DirectX::XMFLOAT3 direction);
    void push_mon_attack_cooltime_event(int monsterKey, int roomID, MonsterType type);
    void push_mon_move_event(int monsterKey, int target, int roomID
        , DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 subtract);
    void push_mon_stop_event(int monsterKey, int roomID);
    void push_mon_behavior_event(int roomID);

    std::unordered_map <int, std::array<Monster, MAX_MONSTER>> monsters;

    std::unordered_map <int, bool> monsterRun;

    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;

    std::mutex               monsters_lock;

    std::unordered_map <int, std::chrono::system_clock::time_point> StartTime;

private:
    Server* m_pServer = NULL;
    Timer* m_pTimer = NULL;
};

