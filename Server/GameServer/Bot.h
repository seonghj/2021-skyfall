#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "Server.h"

class SESSION;

class Monster{
public:

    Monster() {}
    Monster(const Monster& m) {}
    ~Monster() {}

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

    DirectX::XMFLOAT3 GetPosition() { return f3Position.load(); }
};

class Bot {
public:
    void CheckTarget(const SESSION& player, int roomID);

    std::unordered_map <int, std::array<Monster, 100>> monsters;
};

