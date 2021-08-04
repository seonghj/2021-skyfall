#pragma once
#include "CPacket.h"
#include <iostream>
#include <random>
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

void CPacket::Send_ready_packet()
{
    game_ready_packet p;
    p.key = client_key;
    p.size = sizeof(p);
    p.type = PacketType::CS_game_ready;
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

void CPacket::Send_login_packet(char* id)
{
    player_login_packet p;
    p.key = client_key;
    p.size = sizeof(p);
    p.type = PacketType::CS_player_login;
    p.roomid = roomID;
    strcpy_s(p.id, id);
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

void CPacket::Swap_weapon(int key, PlayerType weapon)
{
    if (key != client_key) {
        switch (weapon) {
        case PT_BOW: {
            float beforepitch = m_pScene->m_mPlayer[key]->GetPitch();
            float beforeyaw = m_pScene->m_mPlayer[key]->GetYaw();
            float beforeroll = m_pScene->m_mPlayer[key]->GetRoll();
            XMFLOAT3 beforepos = m_pScene->m_mPlayer[key]->GetPosition();

            m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[key] = m_pScene->m_mBowPlayer[key];

            m_pScene->m_mPlayer[key]->SetPosition(beforepos);

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

            m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[key] = m_pScene->m_m1HswordPlayer[key];

            m_pScene->m_mPlayer[key]->SetPosition(beforepos);

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

            m_pPlayer->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[client_key] = m_pScene->m_mBowPlayer[client_key];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;

            m_pPlayer->SetPosition(beforepos);

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

            m_pPlayer->SetPosition(XMFLOAT3(0, -500, 0));
            m_pScene->m_mPlayer[client_key] = m_pScene->m_m1HswordPlayer[client_key];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;

            m_pPlayer->SetPosition(beforepos);

            float tpitch = m_pPlayer->GetPitch();
            float tyaw = m_pPlayer->GetYaw();

            m_pPlayer->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            m_pFramework->m_pCamera = m_pPlayer->GetCamera();
            break;
        }
        }
    }
}

void CPacket::Map_set(int type, int pos)
{
    switch (pos) {
    case 0: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    case 1: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    case 2: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    case 3: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    case 4: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    case 5: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    case 6: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    case 7: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    case 8: {
        switch (type) {
        case terrain::Forest: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Desert: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        case terrain::Snowy_field: {
            m_pScene->m_pTerrain->SetPosition(2048.0f * -1, 0.0f, 2048.0f * -1);
            break;
        }
        }
        break;
    }
    }
}

void CPacket::CheckCollision(CMonster * mon)
{
    CGameObject* pObject;
    for (int i = 0; i < 3; ++i) {
        pObject = m_pMap->GetMap(3 * i)->FindFrame("RootNode")->m_pChild->m_pChild;
        while (true) {

            if (mon->isCollide(pObject)) {
              
                XMFLOAT3 d = Vector3::Subtract(mon->GetPosition(), pObject->GetPosition());
                mon->Move(Vector3::ScalarProduct(d, 50.25f, true), true);
                cout << "monster Map Collision - " << pObject->m_pstrFrameName << endl;
                
                return;
            }
            if (pObject->m_pSibling)
                pObject = pObject->m_pSibling;
            else
                return;
        }
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

        m_pPlayer->SetPosition(XMFLOAT3(0, -500, 0));
        m_pScene->m_mPlayer[client_key] = m_pScene->m_m1HswordPlayer[client_key];
        m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key];
        m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
        m_pPlayer = m_pFramework->m_pPlayer;
        m_pFramework->m_pCamera = m_pPlayer->GetCamera();

        Send_login_packet(userID);
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
        }
        break;
    }
    case PacketType::SC_player_add: {
        player_add_packet* p = reinterpret_cast<player_add_packet*>(buf);
        int key = p->key;
        printf("login key: %d\n", p->key);
        if (key != client_key) {
            m_pScene->MovePlayer(key, p->Position);
            m_pScene->m_mPlayer[key]->Rotate(p->dx, p->dy, 0);
            m_pScene->AnimatePlayer(key, 0);
        }
        else {
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
                m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->SetTrackPosition(1, 0);
                printf("key %d jump\n", key);
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
            m_pPlayer->Rotate(p->dx, p->dy, 0);
            switch (p->MoveType) {
            case PlayerMove::RUNNING:
                //m_pPlayer->SetRunning(true);
                break;
            }
        }
        else {
            switch (p->MoveType) {
            case PlayerMove::WAKING:
                if (!strcmp(m_pScene->m_mPlayer[key]->m_pstrFrameName, "Player_Bow")) {
                    m_pScene->AnimatePlayer(key, animation_Bow::B_Walk);
                }
                else if (!strcmp(m_pScene->m_mPlayer[key]->m_pstrFrameName, "Player_1Hsword"))
                    m_pScene->AnimatePlayer(key, animation_1HSword::SH1_Walk); // 11
                break;
            case PlayerMove::RUNNING:
                m_pScene->AnimatePlayer(key, 2); // 2
                break;
            case PlayerMove::JUMP:
                m_pScene->AnimatePlayer(key, 1);
                break;
            default:
                break;
            }
            m_pScene->MovePlayer(key, p->Position);
            m_pScene->m_mPlayer[key]->Rotate(p->dx, p->dy, 0);
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
        Swap_weapon(key, (PlayerType)p->weapon);
        break;
    }
    case PacketType::SC_player_attack: {
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        int key = p->key;
        if (p->key == client_key) {
            switch (p->attack_type) {
            case SWORD1HL: {
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
            case SWORD1HL: {
                m_pScene->AnimatePlayer(key, animation_1HSword::SH1_Attack1);
                printf("key: %d SWORD1HL attack\n", p->key);
                break;
            }
            case SWORD1HR: {
                m_pScene->AnimatePlayer(key, animation_1HSword::SH1_Attack4);
                printf("key: %d SWORD1HR attack\n", p->key);
                break;
            }
            case BOWL: {
                m_pScene->AnimatePlayer(key, animation_Bow::B_ShotReady);
                printf("key: %d BOWL attack\n", p->key);
                break;
            }
            case BOWR: {
                m_pScene->AnimatePlayer(key, animation_Bow::B_ShotReady);
                printf("key: %d BOWR attack\n", p->key);
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
            m_pScene->AnimatePlayer(key, animation_Bow::B_ShotRelease);
        }
        else {
            m_pScene->m_mPlayer[key]->Shot(p->fTimeElapsed, p->ChargeTimer * 100.f, p->Look);
            m_pScene->m_mPlayer[key]->SetAttack(false);
            m_pScene->AnimatePlayer(key, animation_Bow::B_ShotRelease);
        }
        break;
    }
    case PacketType::SC_player_stop: {
        player_stop_packet* p = reinterpret_cast<player_stop_packet*>(buf);
        int key = p->key;

        if (p->key != client_key) {
            m_pScene->AnimatePlayer(key, 0);
            m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->SetTrackPosition(1, 0);
            m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->SetTrackPosition(6, 0);
            m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->SetTrackPosition(9, 0);
            m_pScene->m_mPlayer[key]->SetPosition(p->Position);
        }
        else
            m_pPlayer->SetPosition(p->Position);
        break;
    }

    case PacketType::SC_map_collapse: {
        map_collapse_packet* p = reinterpret_cast<map_collapse_packet*>(buf);
        //printf("break map: %d\n", p->block_num);
        break;
    }

    case PacketType::SC_cloud_move: {
        cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(buf);
        //printf("key: %d cloud move x = %f, z = %f\n",p->roomid, p->x, p->z);
        break;
    }
    case PacketType::SC_map_set: {
        map_block_set* p = reinterpret_cast<map_block_set*>(buf);
        /*for (int i = 0; i < MAX_MAP_BLOCK; i++)
            Map_set(p->block_type[i], i);*/
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

        XMFLOAT3 BeforePos = m_pScene->m_ppGameObjects[key]->GetPosition();

        m_pScene->m_ppGameObjects[key]->Rotate(0, 0, p->degree);
        CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pScene->m_pTerrain;
        p->Position.y = pTerrain->GetHeight(300.f, 300.f) + m_pScene->m_ppGameObjects[key]->m_AABBExtentsY - m_pScene->m_ppGameObjects[key]->GetPosition().y;
        m_pScene->m_ppGameObjects[key]->Move(p->Position, 1.f);
        //CheckCollision(m_pScene->m_ppGameObjects[key]);

        p->type = CS_monster_pos;
        p->Position = m_pScene->m_ppGameObjects[key]->GetPosition();

        SendPacket(reinterpret_cast<char*>(p));
        /*printf("%f, %f, %f\n", m_pScene->m_ppGameObjects[key]->GetPosition().x
            , m_pScene->m_ppGameObjects[key]->GetPosition().y
            , m_pScene->m_ppGameObjects[key]->GetPosition().z);*/
        break;
    }
    case PacketType::SC_monster_attack: {
        mon_attack_packet* p = reinterpret_cast<mon_attack_packet*>(buf);
        int key = p->key;

        if (m_pScene->m_ppGameObjects[key]->m_iReady == FALSE) break;

        m_pScene->m_ppGameObjects[key]->Attack();

        int target = MonsterAttackCheck(m_pScene->m_ppGameObjects[key]);

        if (target != -1) {
            p->type = CS_monster_attack;
            p->target = target;
            SendPacket(reinterpret_cast<char*>(p));
        }
        break;
    }
    case PacketType::SC_monster_add: {
        mon_add_packet* p = reinterpret_cast<mon_add_packet*>(buf);
        int key = p->key;

        m_pScene->m_ppGameObjects[key]->SetPosition(p->Position);
        m_pScene->m_ppGameObjects[key]->Rotate(0, 0, p->dz);

        m_pScene->m_ppGameObjects[key]->m_iReady = TRUE;

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

    /*cout << "IP를 입력하시오: ";
    cin >> ipaddr;

    char ID[50];

    cout << "ID를 입력하시오: ";
    cin >> ID;*/
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(0, 5000);
    sprintf_s(userID, "TEST%d", dis(gen));
    //strcpy(userID, "TEST");

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
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