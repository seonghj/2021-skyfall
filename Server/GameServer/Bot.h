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

    std::atomic<int>         key = -1;
    std::atomic<int>         roomID = -1;
    char                     id[50];

    // 0 Á×À½ / 1 »ýÁ¸
    std::atomic<bool>       state = 0;
    std::atomic<int>        type = 0;
    std::atomic<DirectX::XMFLOAT3>       f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::atomic<float>      dx = 0;
    std::atomic<float>      dy = 0;

    std::atomic<float>      hp = 0;
    std::atomic<float>      lv = 0;
    std::atomic<float>      speed = 20;

    void init();
    void SetPosition(float x, float y, float z);
    void Move(const XMFLOAT3& vDirection, float fSpeed);

    DirectX::XMFLOAT3 GetPosition() { return f3Position; }
};

class Bot {
public:
    void Set_pServer(Server* s) { m_pServer = s; };
    void Set_pTimer(Timer* t) { m_pTimer = t; };

    void CheckTarget(std::array<SESSION, 20> sessions, int roomID);

    void RunBot(std::array<SESSION, 20> sessions, int roomID);

    std::unordered_map <int, std::array<Monster, 100>> monsters;

    bool monsterRun = false;

private:
    Server* m_pServer = NULL;
    Timer* m_pTimer = NULL;
};

