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
    //if (SC_NONE >= buf[1] || buf[1] >= CS_NONE) return;
    int retval = 0;
    wsabuf.len = buf[0];
    wsabuf.buf = buf;
    //printf("%d\n",wsabuf.len);
    retval = WSASend(sock, &wsabuf, 1, &sendbytes, 0, NULL, NULL);
    if (retval == SOCKET_ERROR) {
        printf("%d: ", WSAGetLastError());
        err_display("send()");
    }
}

void CPacket::Send_start_packet(PlayerType t)
{
    game_start_packet p;
    p.key = client_key;
    p.roomid = roomID;
    p.size = sizeof(p);
    p.type = CS_game_start;
    p.weaponType = t;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_attack_packet(int type)
{
    if (InGamekey == -1) return;
    switch (type) {
    case PlayerAttackType::BOWL: {
        player_attack_packet p;
        p.key = Get_clientkey();
        p.ingamekey = InGamekey;
        p.size = sizeof(p);
        p.type = PacketType::CS_player_attack;
        p.attack_type = type;
        SendPacket(reinterpret_cast<char*>(&p));
        break;
    }
    case PlayerAttackType::BOWR: {
        player_attack_packet p;
        p.key = Get_clientkey();
        p.ingamekey = InGamekey;
        p.size = sizeof(p);
        p.type = PacketType::CS_player_attack;
        p.attack_type = type;
        SendPacket(reinterpret_cast<char*>(&p));
        break;
    }
    default: {
        player_attack_packet p;
        p.key = Get_clientkey();
        p.ingamekey = InGamekey;
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
    if (m_pScene->GetState() != SCENE::INGAME) return;
    //printf("stop\n");
    player_stop_packet p;
    p.key = client_key;
    p.ingamekey = InGamekey;
    p.roomid = roomID;
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
    p.type = PacketType::CS_player_Lobbylogin;
    p.roomid = roomID;
    strcpy_s(p.id, id);
    strcpy_s(p.pw, pw);
    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_swap_weapon_packet(PlayerType weapon)
{
    Weapon_swap_packet p;
    p.key = client_key;
    p.ingamekey = InGamekey;
    p.size = sizeof(p);
    p.type = CS_weapon_swap;
    p.weapon = weapon;
    p.roomid = roomID;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_damage_to_player_packet(int target, int nAttack)
{
    if (nAttack == 3)
        m_pFramework->TurnOnSE(SE::Arrow_Hit);
    else
        m_pFramework->TurnOnSE(SE::Knife_Attack);
    player_damage_packet p;
    p.key = client_key;
    p.ingamekey = InGamekey;
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
    if (nAttack == 3)
        m_pFramework->TurnOnSE(SE::Arrow_Hit);
    else
        m_pFramework->TurnOnSE(SE::Monster_Hit);
    mon_damaged_packet p;
    p.key = client_key;
    p.ingamekey = InGamekey;
    p.type = CS_monster_damaged;
    p.roomid = roomID;
    p.size = sizeof(p);
    p.damage = 0;
    p.target = target;
    p.nAttack = nAttack;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_room_create_packet(const char* name)
{
    room_create_packet p;

    p.key = client_key;
    p.roomid = -1;
    p.size = sizeof(p);
    p.type = CS_create_room;
    strncpy_s(p.name, name, 20);

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_room_select_packet(int room)
{
    room_select_packet p;

    p.key = client_key;
    p.room = room;
    p.roomid = -1;
    p.size = sizeof(p);
    p.type = CS_room_select;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_return_lobby_packet()
{
    return_lobby_packet p;

    p.key = InGamekey;
    p.roomid = roomID;
    p.size = sizeof(p);
    p.type = CS_return_lobby;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_refresh_room_packet()
{
    refresh_lobby_packet p;
    p.key = client_key;
    p.roomid = roomID;
    p.size = sizeof(p);
    p.type = CS_refresh_lobby;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_create_account_packet(char* id, char* pw)
{
    create_account_packet p;
    p.key = client_key;
    p.size = sizeof(p);
    p.type = PacketType::CS_create_account;
    p.roomid = roomID;
    strcpy_s(p.id, id);
    strcpy_s(p.pw, pw);
    p.canmake = 0;
    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::OtherPlayerMove(float fTimeElapsed)
{
    if (m_pScene->GetState() != SCENE::INGAME) return;
    for (int i = 0; i < MAX_PLAYER; i++) {
        if (isMove[i] == true && i != InGamekey) {
            m_pScene->m_mPlayer[i]->Move(MoveDir[i], 50.25f, true);
            m_pScene->m_mPlayer[i]->SetStamina(10);
        }
    }
}

void CPacket::Send_Rotate(float pitch, float yaw)
{
    player_pos_packet p;
    p.key = Get_clientkey();
    p.ingamekey = InGamekey;
    p.roomid = roomID;
    p.Position = m_pPlayer->GetPosition();
    p.dx = m_pPlayer->GetPitch();
    p.dy = m_pPlayer->GetYaw();
    p.dir = 0;
    p.size = sizeof(player_pos_packet);
    p.state = 1;
    p.type = CS_player_pos;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Swap_weapon(int key, PlayerType weapon)
{
    if (key != InGamekey) {
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

            m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -5000, 0));
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

            m_pScene->m_mPlayer[key]->SetPosition(XMFLOAT3(0, -5000, 0));
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
        case PT_SWORD1H: {
            float beforepitch = m_pPlayer->GetPitch();
            float beforeyaw = m_pPlayer->GetYaw();
            XMFLOAT3 beforepos = m_pPlayer->GetPosition();
            int beforplace = m_pPlayer->GetPlace();

            m_pPlayer->SetPosition(XMFLOAT3(0, -5000, 0));
            m_pScene->m_mPlayer[InGamekey] = m_pScene->m_m1HswordPlayer[InGamekey];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[InGamekey];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_mPlayer[InGamekey]->m_nkey = InGamekey;
            m_pScene->m_mPlayer[InGamekey]->m_pPacket = this;

            m_pPlayer->SetPosition(beforepos);
            m_pPlayer->SetPlace(beforplace);

            float tpitch = m_pPlayer->GetPitch();
            float tyaw = m_pPlayer->GetYaw();

            m_pPlayer->Rotate(-tpitch + beforepitch, -tyaw + beforeyaw, 0);
            m_pFramework->m_pCamera = m_pPlayer->GetCamera();
            break;
        }
        case PT_BOW: {
            XMFLOAT3 beforepos = m_pPlayer->GetPosition();
            float beforepitch = m_pPlayer->GetPitch();
            float beforeyaw = m_pPlayer->GetYaw();
            int beforplace = m_pPlayer->GetPlace();

            m_pPlayer->SetPosition(XMFLOAT3(0, -5000, 0));
            m_pScene->m_mPlayer[InGamekey] = m_pScene->m_mBowPlayer[InGamekey];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[InGamekey];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_mPlayer[InGamekey]->m_nkey = InGamekey;
            m_pScene->m_mPlayer[InGamekey]->m_pPacket = this;

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

            m_pPlayer->SetPosition(XMFLOAT3(0, -5000, 0));
            m_pScene->m_mPlayer[InGamekey] = m_pScene->m_m2HswordPlayer[InGamekey];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[InGamekey];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_mPlayer[InGamekey]->m_nkey = InGamekey;
            m_pScene->m_mPlayer[InGamekey]->m_pPacket = this;

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

            m_pPlayer->SetPosition(XMFLOAT3(0, -5000, 0));
            m_pScene->m_mPlayer[InGamekey] = m_pScene->m_m2HspearPlayer[InGamekey];
            m_pScene->m_pPlayer = m_pScene->m_mPlayer[InGamekey];
            m_pFramework->m_pPlayer = m_pScene->m_pPlayer;
            m_pPlayer = m_pFramework->m_pPlayer;
            m_pPlayer->m_pPacket = this;
            m_pScene->m_mPlayer[InGamekey]->m_nkey = InGamekey;
            m_pScene->m_mPlayer[InGamekey]->m_pPacket = this;

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

bool CPacket::CheckCollision(CMonster * mon)
{
    CGameObject* pObject = pObject = m_pMap->GetMap(2 * mon->GetPlace())->FindFrame("RootNode")->m_pChild->m_pChild;
    while (true) {

        if (mon->isCollide(pObject)) {

            XMFLOAT3 d = Vector3::Subtract(mon->GetPosition(), pObject->GetPosition());
            
            mon->Move(Vector3::ScalarProduct(d, 20.25f, true), true);
            //cout << "monster Map Collision - " << pObject->m_pstrFrameName << endl;
            return true;
        }
        if (pObject->m_pSibling)
            pObject = pObject->m_pSibling;
        else
            return false;
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
    //printf("%d\n", buf[1]);
    if (buf[1] <= SC_NONE || buf[1] >= CS_NONE) return;
    switch (buf[1])
    {
    case PacketType::SC_create_account: {
        if (m_pScene->GetState() == SCENE::INGAME) break;
        create_account_packet* p = reinterpret_cast<create_account_packet*>(buf);
        if (p->canmake == false) {
            m_pFramework->SetbError(true);
            m_pFramework->SetErrorMsg("Creation account fail");
            printf("Creation account fail\n");
        }
        else {
            m_pFramework->SetbShowAccountWindow(false);
            m_pFramework->SetbError(false);
        }
        break;
    }
    case PacketType::SC_player_Lobbykey: {
        if (m_pScene->GetState() == SCENE::INGAME) break;
        player_key_packet* p = reinterpret_cast<player_key_packet*>(buf);
        int key = p->key;

        client_key = key;
        roomID = -1;
        printf("recv Lobby key from server: %d\n", key);
        m_pPlayer->m_pPacket = this;
        break;
    }
    case PacketType::SC_player_InGamekey: {
        if (m_pScene->GetState() == SCENE::INGAME) break;
        player_key_packet* p = reinterpret_cast<player_key_packet*>(buf);
        int key = p->key;

        if (key <= 1000) break;

        InGamekey = key;
        roomID = p->roomid;
        printf("recv InGamekey from server: %d / room = %d\n", key, roomID);

        //m_pPlayer = m_pFramework->m_pPlayer = m_pScene->m_pPlayer = m_pScene->m_mPlayer[client_key] = m_pScene->m_m1HswordPlayer[client_key];
        m_pPlayer = m_pScene->m_pPlayer = m_pScene->m_mPlayer[InGamekey] = m_pFramework->m_pPlayer;
        //m_pPlayer->SetPosition(XMFLOAT3(0, -500, 0));

        m_pFramework->m_pCamera = m_pPlayer->GetCamera();
        canmove = TRUE;
        m_pPlayer->m_pPacket = this;
        //Send_login_packet("test", "test");

        break;
    }
    case PacketType::SC_player_loginFail: {
        if (m_pScene->GetState() == SCENE::INGAME) break;
        printf("Login fail\n");
        m_pFramework->SetbError(true);
        m_pFramework->SetErrorMsg("Login fail");
        break;
    }
    case PacketType::SC_player_loginOK: {
        if (m_pScene->GetState() == SCENE::INGAME) break;
        player_loginOK_packet* p = reinterpret_cast<player_loginOK_packet*>(buf);
        int key = p->key;
        if (isLogin == TRUE) break;
        if (key != -1) {
            client_key = key;
            roomID = p->roomid;
            /* m_pPlayer->SetPosition(p->Position);
             m_pPlayer->Rotate(p->dx, p->dy, 0);*/

            printf("Login Lobby\n");
            canmove = TRUE;
            m_pPlayer->m_pPacket = this;
            //m_pScene->m_iState = SCENE::LOBBY;
            m_pScene->SetState(SCENE::LOBBY);
        }
        break;
    }
    case PacketType::SC_player_add: {
        player_add_packet* p = reinterpret_cast<player_add_packet*>(buf);
        int key = p->key;
        if (key < 0) break;
        if (key >= MAX_PLAYER) break;
        printf("login key: %d weapon: %d\n", p->key, p->WeaponType);
        if (key != InGamekey) {
            Swap_weapon(p->key, p->WeaponType);
            m_pScene->MovePlayer(key, p->Position);
            m_pScene->m_mPlayer[key]->Rotate(p->dx, p->dy, 0);
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
        printf("game start\n");
        //InGamekey = p->ingamekey;
        //Send_swap_weapon_packet(start_weapon);
        Swap_weapon(InGamekey, start_weapon);
        isfalling = false;

        for (int i = 0; i < MAX_PLAYER; i++)
            isMove[i] = false;

        m_pFramework->TrunOnBGM(BGM::The_Battle_of_1066);
        m_pFramework->StartGame();
        break;
    }
    case PacketType::SC_game_end: {
        game_end_packet* p = reinterpret_cast<game_end_packet*>(buf);
        printf("GameOver\n");
        m_pScene->SetState(SCENE::ENDGAME);
        m_pFramework->TrunOnBGM(BGM::Poisoned_Rose);
        m_pPlayer->SetPosition(XMFLOAT3(5366, 136, 1480));
        m_pPlayer->SetRate(m_pScene->rate--);
        if (m_pPlayer->GetRate() == 1)
        {
            m_pScene->m_ppUIObjects[3]->SetAlpha(1.0f);
        }
        else if (m_pPlayer->GetRate() > 1)
        {
            m_pScene->m_ppUIObjects[2]->SetAlpha(1.0f);
        }

        break;
    }
    case PacketType::SC_player_info: {

        break;
    }

    case PacketType::SC_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        int key = p->ingamekey;
        //if (key >= MAX_PLAYER) break;
        //printf("player %d move\n", key);
        if (key == InGamekey) {
            switch (p->MoveType) {
            case PlayerMove::JUMP: {
                //m_pPlayer->SetJump(TRUE);
                break;
            }
            }
        }
        else {
            m_pScene->m_mPlayer[key]->SetPosition(p->Position);
            isMove[key] = true;
            switch (p->MoveType) {
            case PlayerMove::JUMP: {
                m_pScene->m_mPlayer[key]->SetJump(true);
                //m_pScene->AnimatePlayer(key, 2);
                break;
            }
            case PlayerMove::RUNNING: {
                MoveDir[key] = p->direction;
                m_pScene->m_mPlayer[key]->SetRunning(true);
                break;
            }
            case PlayerMove::WALKING: {
                MoveDir[key] = p->direction;
                m_pScene->m_mPlayer[key]->SetRunning(false);
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
        int key = p->ingamekey;
        if (key >= MAX_PLAYER) break;
        if (key == InGamekey) {
            m_pPlayer->SetPosition(p->Position);
            m_pPlayer->Rotate(p->dx - m_pPlayer->GetPitch()
               , p->dy - m_pPlayer->GetYaw(), 0);
        }
        else {
            m_pScene->MovePlayer(key, p->Position);
            m_pScene->m_mPlayer[key]->Rotate(p->dx - m_pScene->m_mPlayer[key]->GetPitch()
                , p->dy - m_pScene->m_mPlayer[key]->GetYaw(), 0);

            if (m_pScene->m_mPlayer[key]->GetType() != p->playertype) {
                Swap_weapon(key, p->playertype);
            }
        }
        m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->SetTrackPosition(PlayerState::Take_Damage, 0);
        break;
    }
    case PacketType::SC_select_room: {
        if (m_pScene->GetState() == SCENE::INGAME) break;
        room_select_packet* p = reinterpret_cast<room_select_packet*>(buf);
        InGamekey = p->ingamekey;
        roomID = p->room;
        printf("recv InGamekey from server: %d / room = %d\n", InGamekey, roomID);
        m_pScene->SetState(SCENE::INROOM);
        m_pFramework->TrunOnBGM(BGM::Poisoned_Rose);
        m_pPlayer->SetJump(TRUE);
        break;
    }
    case PacketType::SC_room_list: {
        if (m_pScene->GetState() == SCENE::INGAME) break;
        room_list_packet* p = reinterpret_cast<room_list_packet*>(buf);
        if (p->idx >= 20) break;
        if (p->idx == INVALIDID) {
            m_pFramework->SetbError(true);
            m_pFramework->SetErrorMsg("Room name already exists");
            Send_refresh_room_packet();
            break;
        }
        else {
            m_pFramework->SetbError(false);
        }
        m_pFramework->m_vRooms.insert(m_pFramework->m_vRooms.begin()
            ,make_pair(p->idx, p->name));
        break;
    }
    case PacketType::SC_return_lobby: {
        return_lobby_packet* p = reinterpret_cast<return_lobby_packet*>(buf);
        if (p->key != InGamekey) break;
        if (p->roomid != roomID) break;
        InGamekey = -1;
        roomID = -1;
        m_pScene->SetState(SCENE::LOBBY);
        break;
    }
    case PacketType::SC_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        int key = p->key;
        if (key >= MAX_PLAYER) break;
        if (key == InGamekey) {
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
        if (key >= MAX_PLAYER) break;
        Swap_weapon(key, p->weapon);
        break;
    }
    case PacketType::SC_player_attack: {
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        int key = p->ingamekey;
        if (key >= MAX_PLAYER) break;
        if (key == InGamekey) {
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
                isMove[key] = false;
                m_pScene->AnimatePlayer(key, 9);
                break;
            }
            case SWORD1HL2: {
                isMove[key] = false;
                m_pScene->AnimatePlayer(key, 10);
                break;
            }
            case SWORD1HR: {
                isMove[key] = false;
                m_pScene->AnimatePlayer(key, 12);
                break;
            }
            case BOWL: {
                isMove[key] = false;
                m_pScene->AnimatePlayer(key, 10);
                break;
            }
            case BOWR: {
                isMove[key] = false;
                m_pScene->AnimatePlayer(key, 10);
                break;
            }
            }
        }

        break;
    }
    case PacketType::SC_allow_shot: {
        player_shot_packet* p = reinterpret_cast<player_shot_packet*>(buf);
        int key = p->ingamekey;
        if (key >= MAX_PLAYER) break;
        if (key == InGamekey) {
            m_pPlayer->Shot(p->fTimeElapsed, p->ChargeTimer * 100.f, p->Look);
            m_pPlayer->SetAttack(false);
            m_pPlayer->SetCharging(false);
            //m_pScene->AnimatePlayer(key, 11);
            //printf("%d player shot allow\n", key);
        }
        else {
            m_pScene->m_mPlayer[key]->Shot(p->fTimeElapsed, p->ChargeTimer * 100.f, p->Look);
            m_pScene->m_mPlayer[key]->SetAttack(false);
            m_pScene->AnimatePlayer(key, 11);
            //printf("%d player shot allow\n", key);
        }
        break;
    }
    case PacketType::SC_player_stop: {
        player_stop_packet* p = reinterpret_cast<player_stop_packet*>(buf);
        int key = p->ingamekey;
        if (key >= MAX_PLAYER) break;
        m_pScene->m_mPlayer[key]->SetGround(true);
        if (m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->GetTrackPosition(PlayerState::Take_Damage) == 0
            || m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->IsTrackFinish(PlayerState::Take_Damage))
            m_pScene->AnimatePlayer(key, PlayerState::Idle);
        if (key != InGamekey) {
            if (m_pScene->m_mPlayer[key]->GetJump() == TRUE) {
                m_pScene->m_mPlayer[key]->m_pSkinnedAnimationController->SetTrackPosition(PlayerState::Jump, 0);
                m_pScene->m_mPlayer[key]->SetJump(FALSE);
            }
        }
        m_pScene->m_mPlayer[key]->SetPosition(p->Position);
        isMove[key] = false;
        MoveDir[key] = 0;
       /* printf("stop key: %d player move x = %f, y = %f, z = %f\n", p->key
            , m_pScene->m_mPlayer[key]->GetPosition().x
            , m_pScene->m_mPlayer[key]->GetPosition().y
            , m_pScene->m_mPlayer[key]->GetPosition().z);*/
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
        m_pFramework->SetCloud(p->x, p->z);
        break;
    }
    case PacketType::SC_map_set: {
        map_block_set* p = reinterpret_cast<map_block_set*>(buf);
        break;
    }
    case PacketType::SC_monster_pos: {
        mon_pos_packet* p = reinterpret_cast<mon_pos_packet*>(buf);
        int key = p->key;
        if (key >= 15) break;
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
                p->Position.y = p->Position.y + 15;
            m_pScene->m_ppGameObjects[key]->SetPosition(p->Position);
            m_pScene->m_ppGameObjects[key]->Rotate(0, 0, p->degree - m_pScene->m_ppGameObjects[key]->m_fRotateDegree);
            m_pScene->m_ppGameObjects[key]->m_fRotateDegree = p->degree;
            if (pTerrain->IsFalling()) {
                m_pScene->m_ppGameObjects[key]->OnUpdateCallback();
            }
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

            if (p->MonsterType == MonsterType::Wolf) {
                if (m_pScene->m_ppGameObjects[key]->GetState() != MonsterState::m_Walk) {
                    if (m_pScene->m_ppGameObjects[key]->GetState() == MonsterState::m_Take_Damage) {
                        if (m_pScene->m_ppGameObjects[key]->m_pSkinnedAnimationController->IsTrackFinish(MonsterState::m_Take_Damage) == true)
                            m_pScene->m_ppGameObjects[key]->ChangeState(MonsterState::m_Walk);
                    }
                    else {
                        m_pScene->m_ppGameObjects[key]->ChangeState(MonsterState::m_Walk);
                    }
                }
            }
            else {
                if (m_pScene->m_ppGameObjects[key]->GetState() != MonsterState::m_Idle) {
                    if (m_pScene->m_ppGameObjects[key]->GetState() == MonsterState::m_Take_Damage) {
                        if (m_pScene->m_ppGameObjects[key]->m_pSkinnedAnimationController->IsTrackFinish(MonsterState::m_Take_Damage) == true)
                            m_pScene->m_ppGameObjects[key]->ChangeState(MonsterState::m_Idle);
                    }
                    else {
                        m_pScene->m_ppGameObjects[key]->ChangeState(MonsterState::m_Idle);
                    }
                }
            }
        }

        bool b = CheckCollision(m_pScene->m_ppGameObjects[key]);
        if (b == true && p->target == client_key) {
            p->type = CS_monster_pos;
            p->Position = m_pScene->m_ppGameObjects[key]->GetPosition();

            //printf("%f, %f, %f\n", p->Position.x, p->Position.y, p->Position.z);
            SendPacket(reinterpret_cast<char*>(p));
        }
        break;
    }
    case PacketType::SC_monster_attack: {
        mon_attack_packet* p = reinterpret_cast<mon_attack_packet*>(buf);
        int key = p->key;
        if (key >= 15) break;
        if (p->target >= MAX_PLAYER) break;
        //if (m_pScene->m_ppGameObjects[key]->m_iReady == FALSE) break;
        m_pScene->m_ppGameObjects[key]->Rotate(0, 0, p->degree - m_pScene->m_ppGameObjects[key]->m_fRotateDegree);
        m_pScene->m_ppGameObjects[key]->m_fRotateDegree = p->degree;
        m_pScene->m_ppGameObjects[key]->Attack();

        XMFLOAT3 subtract = Vector3::Subtract(m_pScene->m_mPlayer[p->target]->GetPosition()
            , m_pScene->m_ppGameObjects[key]->GetPosition());
        subtract.y = 0;
        float range = Vector3::Length(subtract);

        if (range <= p->attack_dis) {
            if (p->target == InGamekey) {
                p->type = CS_monster_attack;
                SendPacket(reinterpret_cast<char*>(p));
                m_pPlayer->SetHp(p->PlayerLeftHp);
                m_pScene->m_ppUIObjects[0]->SethPercent(p->PlayerLeftHp / m_pPlayer->m_iMaxHp);
                cout << key << ": attack to " << p->target << " leftHP: " << p->PlayerLeftHp << endl;
                m_pPlayer->SetDamaged(true);
                m_pScene->TakeDamage(true);
            }
            m_pScene->AnimatePlayer(p->target, PlayerState::Take_Damage);
        }
        break;
    }
    case PacketType::SC_monster_add: {
        mon_add_packet* p = reinterpret_cast<mon_add_packet*>(buf);
        int key = p->key;
        if (key >= 15) break;
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
                p->Position.y = p->Position.y + 15;
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
        printf("Monster key: %d add \ pos (%f, %f, %f)\n"
            , key, m_pScene->m_ppGameObjects[key]->GetPosition().x
            , m_pScene->m_ppGameObjects[key]->GetPosition().y
            , m_pScene->m_ppGameObjects[key]->GetPosition().z);

        break;
    }
    case PacketType::SC_monster_damaged:{
        mon_damaged_packet* p = reinterpret_cast<mon_damaged_packet*>(buf);
        if (p->target >= 15) break;
        m_pScene->m_ppGameObjects[p->target]->SetHp(p->leftHp);
        cout << "Monster: " << p->target << " hp: " << m_pScene->m_ppGameObjects[p->target]->GetHp() << endl;
        m_pScene->m_ppGameObjects[p->target]->FindFrame("HpBar")->SetHp(m_pScene->m_ppGameObjects[p->target]->GetHp());
        m_pFramework->TurnOnSE(SE::Monster_Hit);
        if (m_pScene->m_ppGameObjects[p->target]->m_iHp > 0)
            m_pScene->m_ppGameObjects[p->target]->ChangeState(MonsterState::m_Take_Damage);
        else {
            m_pScene->m_ppGameObjects[p->target]->Move(XMFLOAT3(0, -20, 0), 1.f);
            m_pScene->m_ppGameObjects[p->target]->ChangeState(MonsterState::m_Die);
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
        if (key >= 15) break;
        m_pScene->m_ppGameObjects[key]->ChangeState(0);
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
    case PacketType::SC_monster_stop: {
        mon_stop_packet* p = reinterpret_cast<mon_stop_packet*>(buf);

        int key = p->key;
        if (key >= 15) break;
        m_pScene->m_ppGameObjects[key]->ChangeState(MonsterState::m_Idle);
        //printf("%d mon stop\n", p->key);
        break;
    }
    case PacketType::SC_player_damage: {
        player_damage_packet* p = reinterpret_cast<player_damage_packet*>(buf);
        if (p->ingamekey >= MAX_PLAYER) break;
        if (p->target >= MAX_PLAYER) break;
        m_pScene->m_mPlayer[p->target]->SetHp(p->leftHp);
        if (p->target == InGamekey)
            m_pScene->m_ppUIObjects[0]->SethPercent(p->leftHp / m_pPlayer->m_iMaxHp);
        cout << "player: " << p->target << " hp: " << m_pScene->m_mPlayer[p->target]->GetHp() << endl;
        //m_pScene->m_ppGameObjects[p->target]->FindFrame("HpBar")->SetHp(m_pScene->m_ppGameObjects[p->target]->GetHp());

        if (p->target == InGamekey) {
            m_pPlayer->SetStanding(true);
            m_pScene->m_ppUIObjects[0]->SethPercent(p->leftHp / m_pPlayer->m_iMaxHp);
            //Send_stop_packet();
            m_pPlayer->SetDamaged(true);
            m_pScene->TakeDamage(true);
        }
        m_pScene->AnimatePlayer(p->target, PlayerState::Take_Damage);

       /* if (m_pScene->m_mPlayer[p->target]->GetHp() < 0)
            m_pScene->AnimatePlayer(p->target, PlayerState::Death);*/

        break;
    }
    case PacketType::SC_player_dead: {
        player_dead_packet* p = reinterpret_cast<player_dead_packet*>(buf);
        if (p->ingamekey >= MAX_PLAYER) break;
        printf("player dead key: %d\n", p->key);
        m_pScene->AnimatePlayer(p->ingamekey, PlayerState::Death);
        if (p->ingamekey != InGamekey)
        {
            m_pScene->m_mPlayer[p->ingamekey]->SetPosition(XMFLOAT3(-6000, 0, -6000));
            m_pScene->m_mPlayer[p->ingamekey]->SetRate(m_pScene->rate--);
        }
        if (p->ingamekey == InGamekey)
        {
            InGamekey = -1;
            m_pScene->m_iState = SCENE::ENDGAME;
            m_pFramework->TrunOnBGM(BGM::Poisoned_Rose);
            m_pPlayer->SetRate(m_pScene->rate--);
            m_pPlayer->SetPosition(XMFLOAT3(5366, 136, 1480));
            if (m_pPlayer->GetRate() == 1)
            {
                m_pScene->m_ppUIObjects[3]->SetAlpha(1.0f);
            }
            else if (m_pPlayer->GetRate() > 1)
            {
                m_pScene->m_ppUIObjects[2]->SetAlpha(1.0f);
            }
        }
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
    serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
    serveraddr.sin_port = htons(LOBBYPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    memset(&overlapped, 0, sizeof(overlapped));

    Recv_thread = std::thread(&CPacket::RecvPacket, this);

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

    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
    serveraddr.sin_port = htons(GAMESERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    memset(&overlapped, 0, sizeof(overlapped));

    Recv_thread = std::thread(&CPacket::RecvPacket, this);

    Recv_thread.join();
}