#pragma once
#include "CPacket.h"
#include <iostream>
#include <random>
#include "player.h"
#pragma warning(disable : 4996)

CPacket::CPacket()
{

}

CPacket::~CPacket()
{

}

// 소켓 함수 오류 출력 후 종료
void CPacket::err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void CPacket::err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

void CPacket::RecvPacket()
{

    DWORD flags = 0;
    int retval = 0;
    int saved_packet_size = 0;

    bool isRun = true;

    char recvbuf[BUFSIZE];
    char buffer[BUFSIZE];

    int rest_size = retval;
    DWORD packet_size = 0;


    WSABUF r_wsabuf;
    r_wsabuf.buf = recvbuf;
    r_wsabuf.len = BUFSIZE;

    u_long nonBlockingMode = 1;

    // 데이터 받기
    while (isRun) {
        recvbytes = 0;
        //ioctlsocket(sock, FIONBIO, &nonBlockingMode);
        retval = WSARecv(sock, &r_wsabuf, 1, &recvbytes, &flags, NULL, NULL);
        //printf("%d, %d", recvbytes, r_wsabuf.buf[1]);
        if (retval == SOCKET_ERROR) {
            int err_no = WSAGetLastError();
            if (err_no != WSA_IO_PENDING) {
                printf("recv error %d packet: %d error\n", err_no, r_wsabuf.buf[1]);
                printf("server disconnect\n");
                closesocket(sock);
                break;
            }
        }

        int rest_size = recvbytes;
        unsigned char* buf_ptr = reinterpret_cast<unsigned char*>(recvbuf);
        //// 받은 데이터 출력
        while (rest_size > 0)
        {
            if (0 == packet_size)
                packet_size = recvbytes;
            if (rest_size + saved_packet_size >= packet_size) {
                std::memcpy(buffer + saved_packet_size, buf_ptr, packet_size - saved_packet_size);
                //std::printf("[TCP 클라이언트] %d바이트를 받았습니다. %d\r\n", recvbytes, recvbuf[1]);

               /* if (buffer[1] == PacketType::SC_start_ok) {
                    printf("start\n");
                    isRun = false;

                }*/
                ProcessPacket(buffer);
                buf_ptr += packet_size - saved_packet_size;
                rest_size -= packet_size - saved_packet_size;
                packet_size = 0;
                saved_packet_size = 0;
            }
            else {
                std::memcpy(buffer + saved_packet_size, buf_ptr, rest_size);
                saved_packet_size += rest_size;
                rest_size = 0;
            }
        }
    }
}

void CPacket::SendPacket(char* buf)
{
    int retval = 0;
    wsabuf.len = buf[0];
    wsabuf.buf = buf;
    retval = WSASend(sock, &wsabuf, 1, &sendbytes, 0, NULL, NULL);
    if (retval == SOCKET_ERROR) {
        printf("%d: ", WSAGetLastError());
        err_display("send()");
    }
}

void CPacket::Send_ready_packet(PlayerType t)
{
    game_ready_packet p;
    p.key = client_key;
    p.roomid = roomID;
    p.size = sizeof(p);
    p.type = CS_game_ready;
    p.weaponType = t;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_attack_packet(int type)
{
    switch (type) {
    case PlayerAttackType::BOWL: {
        player_attack_packet p;
        p.key = client_key;
        p.size = sizeof(p);
        p.type = PacketType::CS_player_attack;
        p.attack_type = type;
        SendPacket(reinterpret_cast<char*>(&p));
        break;
    }
    case PlayerAttackType::BOWR: {
        player_attack_packet p;
        p.key = client_key;
        p.size = sizeof(p);
        p.type = PacketType::CS_player_attack;
        p.attack_type = type;
        SendPacket(reinterpret_cast<char*>(&p));
        break;
    }
    default: {
        player_attack_packet p;
        p.key = client_key;
        p.size = sizeof(p);
        p.type = PacketType::CS_player_attack;
        p.attack_type = type;
        SendPacket(reinterpret_cast<char*>(&p));
        break;
    }
    }
}

void CPacket::Send_stop_packet()
{
    player_stop_packet p;
    p.key = client_key;
    p.size = sizeof(p);
    p.type = PacketType::CS_player_stop;
    p.Position = m_pPlayer->GetPosition();
    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_login_packet(char* id, char* pw)
{
    player_login_packet p;
    p.key = client_key;
    p.size = sizeof(p);
    p.type = PacketType::CS_player_login;
    p.roomid = roomID;
    strcpy_s(p.id, id);
    strcpy_s(p.pw, pw);
    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_swap_weapon_packet(PlayerType weapon)
{
    Weapon_swap_packet p;
    p.key = client_key;
    p.size = sizeof(p);
    p.type = CS_weapon_swap;
    p.weapon = weapon;
    p.roomid = roomID;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_damage_to_player_packet(int target, int nAttack)
{
    player_damage_packet p;
    p.key = client_key;
    p.type = CS_player_damage;
    p.roomid = roomID;
    p.size = sizeof(p);
    p.damage = 0;
    p.target = target;
    p.nAttack = nAttack;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_mon_damaged_packet(int target, int nAttack)
{
    mon_damaged_packet p;
    p.key = client_key;
    p.type = CS_monster_damaged;
    p.roomid = roomID;
    p.size = sizeof(p);
    p.damage = 0;
    p.target = target;
    p.nAttack = nAttack;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Swap_weapon(int key, PlayerType weapon)
{
    if (key != client_key) {
        switch (weapon) {
        case PT_BOW: {
            float beforepitch = m_pScene->m_mPlayer[key]->GetPitch();
            float beforeyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float beforeroll = m_pScene->m_mPlayer[key]->GetRoll();
            XMFLOAT3 beforepos = m_pScene->m_mPlayer[key]->GetPosition();
            int beforplace = m_pScene->m_mPlayer[key]->GetPlace();

            //m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[key] = m_pScene->m_mBowPlayer[key];
            m_pScene->m_mPlayer[key]->m_nkey = key;

            m_pScene->m_mPlayer[key]->SetPosition(beforepos);
            m_pScene->m_mPlayer[key]->SetPlace(beforplace);

            float tpitch = m_pScene->m_mPlayer[key]->GetPitch();
            float tyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float troll = m_pScene->m_mPlayer[key]->GetRoll();

            m_pScene->m_mPlayer[key]->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            break;
        }
        case PT_SWORD1H: {
            float beforepitch = m_pScene->m_mPlayer[key]->GetPitch();
            float beforeyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float beforeroll = m_pScene->m_mPlayer[key]->GetRoll();
            XMFLOAT3 beforepos = m_pScene->m_mPlayer[key]->GetPosition();
            int beforplace = m_pScene->m_mPlayer[key]->GetPlace();

            //m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[key] = m_pScene->m_m1HswordPlayer[key];
            m_pScene->m_mPlayer[key]->m_nkey = key;

            m_pScene->m_mPlayer[key]->SetPosition(beforepos);
            m_pScene->m_mPlayer[key]->SetPlace(beforplace);

            float tpitch = m_pScene->m_mPlayer[key]->GetPitch();
            float tyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float troll = m_pScene->m_mPlayer[key]->GetRoll();

            m_pScene->m_mPlayer[key]->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            break;
        }
        case PT_SWORD2H: {
            float beforepitch = m_pScene->m_mPlayer[key]->GetPitch();
            float beforeyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float beforeroll = m_pScene->m_mPlayer[key]->GetRoll();
            XMFLOAT3 beforepos = m_pScene->m_mPlayer[key]->GetPosition();
            int beforplace = m_pScene->m_mPlayer[key]->GetPlace();

            m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[key] = m_pScene->m_m2HswordPlayer[key];
            m_pScene->m_mPlayer[key]->m_nkey = key;

            m_pScene->m_mPlayer[key]->SetPosition(beforepos);
            m_pScene->m_mPlayer[key]->SetPlace(beforplace);

            float tpitch = m_pScene->m_mPlayer[key]->GetPitch();
            float tyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float troll = m_pScene->m_mPlayer[key]->GetRoll();

            m_pScene->m_mPlayer[key]->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            break;
        }
        case PT_SPEAR2H: {
            float beforepitch = m_pScene->m_mPlayer[key]->GetPitch();
            float beforeyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float beforeroll = m_pScene->m_mPlayer[key]->GetRoll();
            XMFLOAT3 beforepos = m_pScene->m_mPlayer[key]->GetPosition();
            int beforplace = m_pScene->m_mPlayer[key]->GetPlace();

            m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[key] = m_pScene->m_m2HspearPlayer[key];
            m_pScene->m_mPlayer[key]->m_nkey = key;

            m_pScene->m_mPlayer[key]->SetPosition(beforepos);
            m_pScene->m_mPlayer[key]->SetPlace(beforplace);

            float tpitch = m_pScene->m_mPlayer[key]->GetPitch();
            float tyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float troll = m_pScene->m_mPlayer[key]->GetRoll();

            m_pScene->m_mPlayer[key]->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            break;
        }
        }
    }
    else {
        switch (weapon) {
        case PT_BOW: {
            float beforepitch = m_pPlayer->GetPitch();
            float beforeyaw = m_pPlayer->GetYaw();
            XMFLOAT3 beforepos = m_pPlayer->GetPosition();
            int beforplace = m_pPlayer->GetPlace();

            m_pPlayer->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[client_key] = m_pScene->m_mBowPlayer[client_key];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_mPlayer[client_key]->m_nkey = client_key;
            m_pScene->m_mPlayer[client_key]->m_pPacket = this;

            m_pPlayer->SetPosition(beforepos);
            m_pPlayer->SetPlace(beforplace);

            float tpitch = m_pPlayer->GetPitch();
            float tyaw = m_pPlayer->GetYaw();

            m_pPlayer->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            m_pFramework->m_pCamera = m_pPlayer->GetCamera();
            break;
        }
        case PT_SWORD1H: {
            XMFLOAT3 beforepos = m_pPlayer->GetPosition();
            float beforepitch = m_pPlayer->GetPitch();
            float beforeyaw = m_pPlayer->GetYaw();
            int beforplace = m_pPlayer->GetPlace();

            m_pPlayer->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[client_key] = m_pScene->m_m1HswordPlayer[client_key];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_mPlayer[client_key]->m_nkey = client_key;
            m_pScene->m_mPlayer[client_key]->m_pPacket = this;

            m_pPlayer->SetPosition(beforepos);
            m_pPlayer->SetPlace(beforplace);

            float tpitch = m_pPlayer->GetPitch();
            float tyaw = m_pPlayer->GetYaw();

            m_pPlayer->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            m_pFramework->m_pCamera = m_pPlayer->GetCamera();
            break;
        }
        case PT_SWORD2H: {
            float beforepitch = m_pScene->m_mPlayer[key]->GetPitch();
            float beforeyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float beforeroll = m_pScene->m_mPlayer[key]->GetRoll();
            XMFLOAT3 beforepos = m_pScene->m_mPlayer[key]->GetPosition();

            m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[client_key] = m_pScene->m_m2HswordPlayer[client_key];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_mPlayer[client_key]->m_nkey = client_key;
            m_pScene->m_mPlayer[client_key]->m_pPacket = this;

            m_pScene->m_mPlayer[key]->SetPosition(beforepos);

            float tpitch = m_pScene->m_mPlayer[key]->GetPitch();
            float tyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float troll = m_pScene->m_mPlayer[key]->GetRoll();

            m_pScene->m_mPlayer[key]->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            m_pFramework->m_pCamera = m_pPlayer->GetCamera();
            break;
        }
        case PT_SPEAR2H: {
            float beforepitch = m_pScene->m_mPlayer[key]->GetPitch();
            float beforeyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float beforeroll = m_pScene->m_mPlayer[key]->GetRoll();
            XMFLOAT3 beforepos = m_pScene->m_mPlayer[key]->GetPosition();

            m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[client_key] = m_pScene->m_m2HspearPlayer[client_key];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_mPlayer[client_key]->m_nkey = client_key;
            m_pScene->m_mPlayer[client_key]->m_pPacket = this;

            m_pScene->m_mPlayer[key]->SetPosition(beforepos);

            float tpitch = m_pScene->m_mPlayer[key]->GetPitch();
            float tyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float troll = m_pScene->m_mPlayer[key]->GetRoll();

            m_pScene->m_mPlayer[key]->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            m_pFramework->m_pCamera = m_pPlayer->GetCamera();
            break;
        }
        case PT_BASIC: {

            break;
        }
        }
    }
}

void CPacket::Map_set(map_block_set* p)
{
    int Desert_count = 0;
    int Forest_count = 3;
    int Snowy_count = 6;

    vector<vector<int>> m_vMapArrange = { { -1, -1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1} };

    for (int i = 0; i < MAX_MAP_BLOCK; i++){
        switch (p->block_type[i]) {
        case Desert: {
            //printf("%d\n", Desert_count);
            m_pScene->GetTerrain(Desert_count)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            if (Desert_count >= 6)  m_pScene->GetTerrain(Desert_count)->MoveUp(125.f);
            m_pMap->GetMap(Desert_count * 3)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            m_pMap->GetMap((Desert_count * 3) + 1)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            m_pMap->GetMap((Desert_count * 3) + 2)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            ++Desert_count;
            break;
        }
        case Forest: {
            //printf("%d\n", Forest_count);
            m_pScene->GetTerrain(Forest_count)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            if (Forest_count >= 6)  m_pScene->GetTerrain(Forest_count)->MoveUp(125.f);
            m_pMap->GetMap(Forest_count * 3)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            m_pMap->GetMap((Forest_count * 3) + 1)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            m_pMap->GetMap((Forest_count * 3) + 2)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            ++Forest_count;
            break;
        }
        case Snowy_field: {
           // printf("%d\n", Snowy_count);
            m_pScene->GetTerrain(Snowy_count)->SetPosition(2048.0f * m_vMapArrange[i][0], 0.0f, 2048.0f * m_vMapArrange[i][1]);
            if (Snowy_count >= 6)  m_pScene->GetTerrain(Snowy_count)->MoveUp(125.f);
            m_pMap->GetMap((Snowy_count * 3))->SetPosition(2048.0f * m_vMapArrange[i][0], 60.0f, 2048.0f * m_vMapArrange[i][1]);
            m_pMap->GetMap((Snowy_count * 3) + 1)->SetPosition(2048.0f * m_vMapArrange[i][0], 60.0f, 2048.0f * m_vMapArrange[i][1]);
            m_pMap->GetMap((Snowy_count * 3) + 2)->SetPosition(2048.0f * m_vMapArrange[i][0], 60.0f, 2048.0f * m_vMapArrange[i][1]);
            ++Snowy_count;
            break;
        }
        }
    }
}

void CPacket::CheckCollision(CMonster * mon)
{
    CGameObject* pObject = pObject = m_pMap->GetMap(3 * mon->GetPlace())->FindFrame("RootNode")->m_pChild->m_pChild;
    while (true) {

        if (mon->isCollide(pObject)) {

            XMFLOAT3 d = Vector3::Subtract(mon->GetPosition(), pObject->GetPosition());
            mon->Move(Vector3::ScalarProduct(d, 20.f, true), true);
            //cout << "monster Map Collision - " << pObject->m_pstrFrameName << endl;

            return;
        }
        if (pObject->m_pSibling)
            pObject = pObject->m_pSibling;
        else
            return;
    }
}

int CPacket::MonsterAttackCheck(CMonster* mon)
{
    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (m_pScene->m_mPlayer[i]->GetHp() > 0) {
            if (mon->isCollide(m_pScene->m_mPlayer[i]) == true)
                return i;
        }
    }
    return -1;
}

void CPacket::ProcessPacket(char* buf)
{
    switch (buf[1])
    {
    case PacketType::SC_player_key: {
        player_key_packet* p = reinterpret_cast<player_key_packet*>(buf);
        int key = p->key;

        if (key < 0 || key > 20) break;

        client_key = key;
        roomID = p->roomid;
        printf("recv key from server: %d\n", key);

        //m_pPlayer = m_pFramework->m_pPlayer = m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key] = m_pScene->m_m1HswordPlayer[client_key];
        m_pPlayer = m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key] = m_pFramework->m_pPlayer;
        //m_pPlayer->SetPosition(XMFLOAT3(0, -500, 0));

        m_pFramework->m_pCamera = m_pPlayer->GetCamera();
        break;
    }
    case PacketType::SC_player_loginFail: {
        printf("Login fail\n");
        break;
    }
    case PacketType::SC_player_loginOK: {
        player_loginOK_packet* p = reinterpret_cast<player_loginOK_packet*>(buf);
        int key = p->key;
        if (isLogin == TRUE) break;
        if (key != -1) {
            client_key = key;
            roomID = p->roomid;
            /* m_pPlayer->SetPosition(p->Position);
             m_pPlayer->Rotate(p->dx, p->dy, 0);*/

            printf("Login game\n");
            canmove = TRUE;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_iState = SCENE::LOBBY;
            m_pFramework->MouseHold(true);
        }
        break;
    }
    case PacketType::SC_player_add: {
        player_add_packet* p = reinterpret_cast<player_add_packet*>(buf);
        int key = p->key;
        printf("login key: %d\n", p->key);
        if (key != client_key) {
            Swap_weapon(p->key, p->WeaponType);
            m_pScene->MovePlayer(key, p->Position);
            m_pScene->m_mPlayer[key]->Rotate(p->dx, p->dy, 0);
            m_pScene->AnimatePlayer(key, 2);
            m_pScene->AnimatePlayer(key, 0);
        }
        else {
            Swap_weapon(p->key, p->WeaponType);
            m_pPlayer->SetPosition(p->Position);
            m_pPlayer->Rotate(p->dx, p->dy, 0);
        }
        break;
    }
    case PacketType::SC_player_remove: {

        break;
    }
    case PacketType::SC_start_ok: {
        //GameConnect();
        game_start_packet* p = reinterpret_cast<game_start_packet*>(buf);
        Swap_weapon(p->key, p->weaponType);
        m_pScene->m_iState = INGAME;
        break;
    }
    case PacketType::SC_game_end: {
        printf("gameover\n");
        //LobbyConnect();
        break;
    }
    case PacketType::SC_player_info: {

        break;
    }

    case PacketType::SC_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        int key = p->key;
        if (p->key == client_key) {
            switch (p->MoveType) {
            case PlayerMove::JUMP: {
                m_pPlayer->SetJump(TRUE);
                break;
            }
            }
        }
        else {
            switch (p->MoveType) {
            case PlayerMove::JUMP: {
                m_pScene->m_mPlayer[key]->SetJump(true);
                m_pScene->AnimatePlayer(key, 2);
                break;
            }
            }
        }

        //if (client_key == 1)
          //  printf("key: %d player move x = %f, y = %f, z = %f\n", p->key, p->x, p->y, p->z);
        break;
    }
    case PacketType::SC_player_pos: {
        player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);
        int key = p->key;
        if (p->key == client_key) {
            //m_pPlayer->SetPosition(p->Position);
            m_pPlayer->Rotate(p->dx - m_pPlayer->GetPitch()
                , p->dy - m_pPlayer->GetYaw(), 0);
            switch (p->MoveType) {
            case PlayerMove::RUNNING:
                //m_pPlayer->SetRunning(true);
                break;
            }
        }
        else {
            switch (p->MoveType) {
            case PlayerMove::WAKING:
                m_pScene->AnimatePlayer(key, 3);
                break;
            case PlayerMove::RUNNING:
                if (p->dir & DIR_FORWARD)
                    m_pScene->AnimatePlayer(key, 4);
                else if (p->dir & DIR_BACKWARD)
                    m_pScene->AnimatePlayer(key, 5);
                else if (p->dir & DIR_RIGHT)
                    m_pScene->AnimatePlayer(key, 6);
                else if (p->dir & DIR_LEFT)
                    m_pScene->AnimatePlayer(key, 7);
                //m_pScene->AnimatePlayer(key, 4);
                break;
            case PlayerMove::STAND:
                m_pScene->AnimatePlayer(key, 0);
                break;
            default:
                break;
            }
            m_pScene->MovePlayer(key, p->Position);
            m_pScene->m_mPlayer[key]->Rotate(p->dx - m_pScene->m_mPlayer[key]->GetPitch()
                , p->dy - m_pScene->m_mPlayer[key]->GetYaw(), 0);
        }
        break;
    }
    case PacketType::SC_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        int key = p->key;
        if (p->key == client_key) {
            m_pPlayer->SetPosition(p->Position);
        }
        else {
            m_pScene->m_mPlayer[key]->SetPosition(p->Position);
        }
        break;
    }
    case PacketType::SC_weapon_swap: {
        Weapon_swap_packet* p = reinterpret_cast<Weapon_swap_packet*>(buf);
        int key = p->key;
        Swap_weapon(key, p->weapon);
        break;
    }
    case PacketType::SC_player_attack: {
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        int key = p->key;
        if (p->key == client_key) {
            switch (p->attack_type) {
            case SWORD1HL1: {
                m_pPlayer->LButtonDown();
                break;
            }
            case SWORD1HR: {
                m_pPlayer->RButtonDown();
                break;
            }
            case BOWL: {
                m_pPlayer->LButtonDown();
                break;
            }
            case BOWR: {
                m_pPlayer->RButtonDown();
                break;
            }
            }
        }
        else {
            switch (p->attack_type) {
            case SWORD1HL1: {
                m_pScene->AnimatePlayer(key, 9);
                break;
            }
            case SWORD1HL2: {
                m_pScene->AnimatePlayer(key, 10);
                break;
            }
            case SWORD1HR: {
                m_pScene->AnimatePlayer(key, 12);
                break;
            }
            case BOWL: {
                m_pScene->AnimatePlayer(key, 10);
                break;
            }
            case BOWR: {
                m_pScene->AnimatePlayer(key, 10);
                break;
            }
            }
        }

        break;
    }
    case PacketType::SC_allow_shot: {
        player_shot_packet* p = reinterpret_cast<player_shot_packet*>(buf);
        int key = p->key;
        if (p->key == client_key) {
            m_pPlayer->Shot(p->fTimeElapsed, p->ChargeTimer * 100.f, p->Look);
            m_pPlayer->SetAttack(false);
            m_pPlayer->SetCharging(false);
            m_pScene->AnimatePlayer(key, 11);
        }
        else {
            m_pScene->m_mPlayer[key]->Shot(p->fTimeElapsed, p->ChargeTimer * 100.f, p->Look);
            m_pScene->m_mPlayer[key]->SetAttack(false);
            m_pScene->AnimatePlayer(key, 11);
        }
        break;
    }
    case PacketType::SC_player_stop: {
        player_stop_packet* p = reinterpret_cast<player_stop_packet*>(buf);
        int key = p->key;
        if (p->key != client_key) {
            m_pScene->AnimatePlayer(key, 0);
            if (m_pScene->m_mPlayer[key]->GetJump() == TRUE) {
                m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->SetTrackPosition(2, 0);
                m_pScene->m_mPlayer[key]->SetJump(FALSE);
            }
            m_pScene->m_mPlayer[key]->SetPosition(p->Position);
        }
        else
            m_pPlayer->SetPosition(p->Position);
        m_pScene->m_mPlayer[key]->SetGround(true);
        break;
    }

    case PacketType::SC_map_collapse: {
        map_collapse_packet* p = reinterpret_cast<map_collapse_packet*>(buf);
        m_pScene->m_ppTerrain[p->block_num]->Falling();
        
        printf("break map: %d\n", p->block_num);
        break;
    }

    case PacketType::SC_cloud_move: {
        cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(buf);
        //m_pFramework->SetCloud(p->x, p->z);
        m_pFramework->SetCloud(5000,1000);
        //printf("key: %d cloud move x = %f, z = %f\n",p->roomid, p->x, p->z);
        break;
    }
    case PacketType::SC_map_set: {
        map_block_set* p = reinterpret_cast<map_block_set*>(buf);
        //Map_set(p);
        m_pPlayer->SetPosition(XMFLOAT3(m_pPlayer->GetPosition().x, 200, m_pPlayer->GetPosition().z));
        break;
    }
    case PacketType::SC_bot_add: {
        mon_pos_packet* p = reinterpret_cast<mon_pos_packet*>(buf);
        int key = p->key;
        //m_pScene->m_ppGameObjects[key]->SetPosition(p->Position);
        break;
    }
    case PacketType::SC_monster_pos: {
        mon_pos_packet* p = reinterpret_cast<mon_pos_packet*>(buf);
        int key = p->key;
        {
            CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pScene->m_ppTerrain[m_pScene->m_ppGameObjects[key]->GetPlace()];
            XMFLOAT3 xmf3MonsterPosition = m_pScene->m_ppGameObjects[key]->GetPosition();
            XMFLOAT3 xmf3TerrainPosition = pTerrain->GetPosition();
            XMFLOAT3 xmf3Scale = pTerrain->GetScale();
            int z = (int)(xmf3MonsterPosition.z / xmf3Scale.z);
            bool bReverseQuad = ((z % 2) != 0);
            float fHeight = pTerrain->GetHeight(xmf3MonsterPosition.x - xmf3TerrainPosition.x, xmf3MonsterPosition.z - xmf3TerrainPosition.z, bReverseQuad) + xmf3TerrainPosition.y;

            p->Position.y = (fHeight + m_pScene->m_ppGameObjects[key]->m_fHeight);
            if (p->MonsterType == MonsterType::Wolf)
                p->Position.y = p->Position.y + 50;
            else if (p->MonsterType == MonsterType::Metalon)
                p->Position.y = p->Position.y + 20;
            m_pScene->m_ppGameObjects[key]->SetPosition(p->Position);
            m_pScene->m_ppGameObjects[key]->Rotate(0, 0, p->degree - m_pScene->m_ppGameObjects[key]->m_fRotateDegree);
            m_pScene->m_ppGameObjects[key]->m_fRotateDegree = p->degree;

            XMFLOAT3 pos = m_pScene->m_ppGameObjects[key]->GetPosition();
            int nPlace = m_pScene->m_ppGameObjects[key]->GetPlace();
            if (pos.x < m_vMapArrange[nPlace][0] * 2048 && nPlace % 3>0) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace - 1);
            }
            else if (pos.x > (m_vMapArrange[nPlace][0] + 1) * 2048 && nPlace % 3 < 2) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace + 1);
            }

            if (pos.z < m_vMapArrange[nPlace][1] * 2048 && nPlace>2) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace - 3);
            }
            else if (pos.z > (m_vMapArrange[nPlace][1] + 1) * 2048 && nPlace < 6) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace + 3);
            }
        }

        CheckCollision(m_pScene->m_ppGameObjects[key]);

        p->type = CS_monster_pos;
        p->Position = m_pScene->m_ppGameObjects[key]->GetPosition();

        //printf("%f, %f, %f\n", p->Position.x, p->Position.y, p->Position.z);

        SendPacket(reinterpret_cast<char*>(p));
        break;
    }
    case PacketType::SC_monster_attack: {
        mon_attack_packet* p = reinterpret_cast<mon_attack_packet*>(buf);
        int key = p->key;
        //if (m_pScene->m_ppGameObjects[key]->m_iReady == FALSE) break;
        m_pScene->m_ppGameObjects[key]->Rotate(0, 0, p->degree - m_pScene->m_ppGameObjects[key]->m_fRotateDegree);
        m_pScene->m_ppGameObjects[key]->m_fRotateDegree = p->degree;
        m_pScene->m_ppGameObjects[key]->Attack();

        m_pScene->AnimatePlayer(p->target, 8);
        if (p->target == client_key) {
            m_pPlayer->SetHp(p->PlayerLeftHp);
            //m_pScene->m_ppUIObjects[0]->SetvPercent(p->PlayerLeftHp / m_pPlayer->m_iMaxHp);
            cout << key << ": attack to " << p->target << " leftHP: " << p->PlayerLeftHp << endl;
        }

        break;
    }
    case PacketType::SC_monster_add: {
        mon_add_packet* p = reinterpret_cast<mon_add_packet*>(buf);
        int key = p->key;
        {
            CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pScene->m_ppTerrain[m_pScene->m_ppGameObjects[key]->GetPlace()];
            XMFLOAT3 xmf3MonsterPosition = m_pScene->m_ppGameObjects[key]->GetPosition();
            XMFLOAT3 xmf3TerrainPosition = pTerrain->GetPosition();
            XMFLOAT3 xmf3Scale = pTerrain->GetScale();
            int z = (int)(xmf3MonsterPosition.z / xmf3Scale.z);
            bool bReverseQuad = ((z % 2) != 0);
            float fHeight = pTerrain->GetHeight(xmf3MonsterPosition.x - xmf3TerrainPosition.x, xmf3MonsterPosition.z - xmf3TerrainPosition.z, bReverseQuad) + xmf3TerrainPosition.y;

            p->Position.y = (fHeight + m_pScene->m_ppGameObjects[key]->m_fHeight);
            if (p->MonsterType == MonsterType::Wolf)
                p->Position.y = p->Position.y + 50;
            else if (p->MonsterType == MonsterType::Metalon)
                p->Position.y = p->Position.y + 20;
            m_pScene->m_ppGameObjects[key]->SetPosition(p->Position);
            m_pScene->m_ppGameObjects[key]->Rotate(0, 0, p->dz - m_pScene->m_ppGameObjects[key]->m_fRotateDegree);
            m_pScene->m_ppGameObjects[key]->m_fRotateDegree = p->dz;

            XMFLOAT3 pos = m_pScene->m_ppGameObjects[key]->GetPosition();
            int nPlace = m_pScene->m_ppGameObjects[key]->GetPlace();
            if (pos.x < m_vMapArrange[nPlace][0] * 2048 && nPlace % 3>0) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace - 1);
            }
            else if (pos.x > (m_vMapArrange[nPlace][0] + 1) * 2048 && nPlace % 3 < 2) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace + 1);
            }

            if (pos.z < m_vMapArrange[nPlace][1] * 2048 && nPlace>2) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace - 3);
            }
            else if (pos.z > (m_vMapArrange[nPlace][1] + 1) * 2048 && nPlace < 6) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace + 3);
            }
        }

        CheckCollision(m_pScene->m_ppGameObjects[key]);

        m_pScene->m_ppGameObjects[key]->m_iReady = TRUE;
        //printf("key: %d y : %f\n", key, m_pScene->m_ppGameObjects[key]->GetPosition().y);

        break;
    }
    case PacketType::SC_monster_damaged:{
        mon_damaged_packet* p = reinterpret_cast<mon_damaged_packet*>(buf);
        m_pScene->m_ppGameObjects[p->target]->SetHp(p->leftHp);
        cout << "Monster: " << p->target << " hp: " << m_pScene->m_ppGameObjects[p->target]->GetHp() << endl;
        m_pScene->m_ppGameObjects[p->target]->FindFrame("HpBar")->SetHp(m_pScene->m_ppGameObjects[p->target]->GetHp());
        if (m_pScene->m_ppGameObjects[p->target]->m_iHp > 0)
            m_pScene->m_ppGameObjects[p->target]->ChangeState(MonsterState::TakeDamage);
        else {
            m_pScene->m_ppGameObjects[p->target]->ChangeState(MonsterState::Die);
        }
        m_pScene->m_ppGameObjects[p->target]->m_pSkinnedAnimationController->SetAllTrackDisable();
        m_pScene->m_ppGameObjects[p->target]->m_pSkinnedAnimationController->SetTrackPosition(
            m_pScene->m_ppGameObjects[p->target]->GetState(), 0);
        m_pScene->m_ppGameObjects[p->target]->m_pSkinnedAnimationController->SetTrackEnable(
            m_pScene->m_ppGameObjects[p->target]->GetState(), true);
        break;
    }
    case PacketType::SC_monster_respawn: {
        mon_respawn_packet* p = reinterpret_cast<mon_respawn_packet*>(buf);
        int key = p->key;
        m_pScene->m_ppGameObjects[key]->ChangeState(MonsterState::Idle);
        m_pScene->m_ppGameObjects[key]->m_pSkinnedAnimationController->SetAllTrackDisable();
        m_pScene->m_ppGameObjects[key]->m_pSkinnedAnimationController->SetTrackPosition(
            m_pScene->m_ppGameObjects[key]->GetState(), 0);
        m_pScene->m_ppGameObjects[key]->m_pSkinnedAnimationController->SetTrackEnable(
            m_pScene->m_ppGameObjects[key]->GetState(), true);
        m_pScene->m_ppGameObjects[key]->SetIdle();
        m_pScene->m_ppGameObjects[key]->m_iHp = m_pScene->m_ppGameObjects[key]->m_iMaxHp;
        switch (p->MonsterType) {
        case MonsterType::Dragon:
            m_pScene->m_ppGameObjects[key]->SetActive("Polygonal_Dragon", true);
            break;
        case MonsterType::Wolf:
            m_pScene->m_ppGameObjects[key]->SetActive("Polygonal_Wolf", true);
            break;
        case MonsterType::Metalon:
            m_pScene->m_ppGameObjects[key]->SetActive("Polygonal_Metalon", true);
            break;
        }
        m_pScene->m_ppGameObjects[key]->SetActive("BoundingBox", true);
        //m_pScene->m_ppGameObjects[key]->SetBehaviorActivate(true);
        {
            CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pScene->m_ppTerrain[m_pScene->m_ppGameObjects[key]->GetPlace()];
            XMFLOAT3 xmf3MonsterPosition = m_pScene->m_ppGameObjects[key]->GetPosition();
            XMFLOAT3 xmf3TerrainPosition = pTerrain->GetPosition();
            XMFLOAT3 xmf3Scale = pTerrain->GetScale();
            int z = (int)(xmf3MonsterPosition.z / xmf3Scale.z);
            bool bReverseQuad = ((z % 2) != 0);
            float fHeight = pTerrain->GetHeight(xmf3MonsterPosition.x - xmf3TerrainPosition.x, xmf3MonsterPosition.z - xmf3TerrainPosition.z, bReverseQuad) + xmf3TerrainPosition.y;

            p->Position.y = (fHeight + m_pScene->m_ppGameObjects[key]->m_fHeight);
            if (p->MonsterType == MonsterType::Wolf)
                p->Position.y = p->Position.y + 50;
            else if (p->MonsterType == MonsterType::Metalon)
                p->Position.y = p->Position.y + 20;
            m_pScene->m_ppGameObjects[key]->SetPosition(p->Position);
            m_pScene->m_ppGameObjects[key]->Rotate(0, 0, p->dz - m_pScene->m_ppGameObjects[key]->m_fRotateDegree);
            m_pScene->m_ppGameObjects[key]->m_fRotateDegree = p->dz;

            XMFLOAT3 pos = m_pScene->m_ppGameObjects[key]->GetPosition();
            int nPlace = m_pScene->m_ppGameObjects[key]->GetPlace();
            if (pos.x < m_vMapArrange[nPlace][0] * 2048 && nPlace % 3>0) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace - 1);
            }
            else if (pos.x > (m_vMapArrange[nPlace][0] + 1) * 2048 && nPlace % 3 < 2) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace + 1);
            }

            if (pos.z < m_vMapArrange[nPlace][1] * 2048 && nPlace>2) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace - 3);
            }
            else if (pos.z > (m_vMapArrange[nPlace][1] + 1) * 2048 && nPlace < 6) {
                m_pScene->m_ppGameObjects[key]->SetPlace(nPlace + 3);
            }
        }
        CheckCollision(m_pScene->m_ppGameObjects[key]);
        break;
    }
    case PacketType::SC_player_damage: {
        player_damage_packet* p = reinterpret_cast<player_damage_packet*>(buf);
        m_pScene->m_mPlayer[p->target]->SetHp(p->leftHp);
        cout << "player: " << p->target << " hp: " << m_pScene->m_mPlayer[p->target]->GetHp() << endl;
        //m_pScene->m_ppGameObjects[p->target]->FindFrame("HpBar")->SetHp(m_pScene->m_ppGameObjects[p->target]->GetHp());

        if (p->target == client_key) {
            m_pPlayer->SetDamaged(true);
            m_pScene->AnimatePlayer(client_key, 8);
        }
        else
            m_pScene->AnimatePlayer(p->target, 8);

        break;
    }
    }
}

void CPacket::Set_clientkey(int n)
{
    client_key = n;
}

int CPacket::Get_clientkey()
{
    return client_key;
}

void CPacket::Set_currentfps(unsigned long FrameRate)
{
    if (FrameRate > 0)
        currentfps = FrameRate;
    else
        currentfps = 1;
}

void CPacket::Login()
{
    int retval = 0;

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(0, 5000);

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
    serveraddr.sin_port = htons(GAMESERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    memset(&overlapped, 0, sizeof(overlapped));

    Recv_thread = std::thread(&CPacket::RecvPacket, this);
}

void CPacket::LobbyConnect()
{
    // disconnect
    shutdown(sock, SD_RECEIVE);
    closesocket(sock);
    //cout << "game diconnect" << endl;

    int retval;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return;

    // socket()
    sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET)err_quit("socket()");

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(LOBBYPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    memset(&overlapped, 0, sizeof(overlapped));

    //gCPacket[num]->Set_clientkey(num);

    Recv_thread = std::thread(&CPacket::RecvPacket, this);
    //std::thread test_Send_thread = std::thread(&CPacket::testSendPacket, gCPacket[num]);
    //test_Send_thread.join();
    Recv_thread.join();

}

void CPacket::GameConnect()
{
    //// disconnect
    //shutdown(sock, SD_RECEIVE);
    //closesocket(sock);
    ////cout << "lobby diconnect" << endl;

    int retval = 0;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // socket()
    sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET)err_quit("socket()");

    //cout << "game connect" << endl;
    //std::thread test_Send_thread = std::thread(&CPacket::testSendPacket, this);
    //test_Send_thread.join();
   
    //InsertID();

    Login();
}