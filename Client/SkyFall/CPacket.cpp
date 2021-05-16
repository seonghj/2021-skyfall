#pragma once
#include "CPacket.h"
#include <iostream>
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

    // 데이터 받기
    while (isRun) {
        retval = WSARecv(sock, &r_wsabuf, 1, &recvbytes, &flags, NULL, NULL);
        //retval = recvn(sock, recvbuf, BUFSIZE, 0);

        //printf("%d, %d", recvbytes, r_wsabuf.buf[1]);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            printf("server disconnect\n");
            break;
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

                if (buffer[1] == PacketType::Type_start_ok) {
                    printf("start\n");
                    isRun = false;

                }
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
    p.id = client_id;
    p.size = sizeof(p);
    p.type = PacketType::Type_game_ready;
    SendPacket(reinterpret_cast<char*>(&p));
}

void CPacket::Send_attack_packet(int type)
{
    player_attack_packet p;
    p.id = client_id;
    p.size = sizeof(p);
    p.type = PacketType::Type_player_attack;
    p.attack_type = type;
    SendPacket(reinterpret_cast<char*>(&p));
    printf("tlqkf\n");
}

void CPacket::Send_animation_stop_packet()
{
    player_stop_packet sp;
    sp.id = client_id;
    sp.size = sizeof(sp);
    sp.type = PacketType::Type_player_stop;
    SendPacket(reinterpret_cast<char*>(&sp));
}

void CPacket::ProcessPacket(char* buf)
{
    switch (buf[1])
    {
    case PacketType::Type_player_ID: {
        player_ID_packet* p = reinterpret_cast<player_ID_packet*>(buf);
        client_id = buf[2];
        printf("recv id from server: %d\n", p->id);

        /*player_info_packet pp;
        pp.id = Get_clientid();
        pp.Position = m_pPlayer->GetPosition();
        pp.size = sizeof(pp);
        pp.state = 0;
        pp.type = Type_player_info;
        SendPacket(reinterpret_cast<char*>(&pp));*/

        //Send_ready_packet();
        break;
    }
    case PacketType::Type_player_login: {
        player_login_packet* p = reinterpret_cast<player_login_packet*>(buf);
        printf("login id: %d\n", p->id);
        if (p->id != client_id) {
            for (int i = 0; i < MAX_PLAYER; ++i) {
                if (m_pScene->PlayerIDs[i] == 0) {
                    m_pScene->PlayerIDs[i] = p->id;
                    m_pScene->MovePlayer(i, p->Position);
                    m_pScene->m_mPlayer[i]->Rotate(p->dx, p->dy, p->dz);
                    m_pScene->AnimatePlayer(i, 0);
                    //printf("id: %d x: %f, z: %f\n", p->id, p->Position.x, p->Position.z);
                    break;
                }
            }
        }
        else {
            m_pScene->m_pPlayer->SetPosition(p->Position);
            m_pScene->m_pPlayer->Rotate(p->dx, p->dy, p->dz);
        }
        break;
    }
    case PacketType::Type_player_remove: {

        break;
    }
    case PacketType::Type_start_ok: {
        //GameConnect();
        break;
    }
    case PacketType::Type_game_end: {
        printf("gameover\n");
        //LobbyConnect();
        break;
    }
    case PacketType::Type_player_info: {
        break;
    }

    case PacketType::Type_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);

        if (p->id == client_id) {
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
                for (int i = 0; i < MAX_PLAYER; ++i) {
                    if (m_pScene->PlayerIDs[i] == p->id) {
                        //m_pScene->m_mPlayer[i]->SetJump(true);
                        m_pScene->m_mPlayer[i]->m_pSkinnedAnimationController->SetTrackPosition(1, 0);
                        printf("id %d jump\n", p->id);
                        break;
                    }
                }
                break;
            }
            }
        }

        break;
    }
    case PacketType::Type_player_pos: {
        player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);
        if (p->id == client_id) {
            m_pPlayer->SetPosition(p->Position);
            m_pPlayer->Rotate(p->dx, p->dy, p->dz);
            switch (p->MoveType) {
            case PlayerMove::RUNNING:
                //m_pPlayer->SetRunning(true);
                break;
            }
        }
        else {
            for (int i = 0; i < MAX_PLAYER; ++i) {
                if (m_pScene->PlayerIDs[i] == p->id) {
                    switch (p->MoveType) {
                    case PlayerMove::WAKING:
                        m_pScene->AnimatePlayer(i, 11); // 11
                        break;
                    case PlayerMove::RUNNING:
                        m_pScene->AnimatePlayer(i, 2); // 2
                        break;
                    case PlayerMove::JUMP:
                        m_pScene->AnimatePlayer(i, 1);
                        break;
                    default:
                        m_pScene->AnimatePlayer(i, 0);
                        break;
                    }
                    m_pScene->MovePlayer(i, p->Position);
                    m_pScene->m_mPlayer[i]->Rotate(p->dx, p->dy, p->dz);
                    //printf("id %d move (%f, %f)\n", p->id, p->Position.x, p->Position.z);
                    break;
                }
            }
        }
        //m_pCamera->Rotate(p->dx, p->dy, p->dz);
        /* m_pPlayer->Update(fTimeElapsed);
         m_pScene->Update(fTimeElapsed);*/
        break;
    }
    case PacketType::Type_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        if (p->id == client_id) {
            m_pPlayer->SetPosition(p->Position);
        }
        else {
            for (int i = 0; i < MAX_PLAYER; ++i) {
                if (m_pScene->PlayerIDs[i] == p->id) {
                    m_pScene->m_mPlayer[i]->SetPosition(p->Position);
                    break;
                }
            }
        }
        break;
    }
    case PacketType::Type_player_attack: {
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        if (p->id == client_id) {
            switch (p->attack_type) {
            case SWORD1HL: {
                m_pPlayer->LButtonDown();
                break;
            }
            case SWORD1HR: {
                m_pPlayer->RButtonDown();
                break;
            }
            }
        }
        else {
            for (int i = 0; i < MAX_PLAYER; ++i) {
                if (m_pScene->PlayerIDs[i] == p->id) {
                    switch (p->attack_type) {
                    case SWORD1HL: {
                        m_pScene->AnimatePlayer(i, 6);
                        printf("id: %d SWORD1HL attack\n", p->id);
                        break;
                    }
                    case SWORD1HR: {
                        m_pScene->AnimatePlayer(i, 9);
                        printf("id: %d SWORD1HR attack\n", p->id);
                        break;
                    }
                    }
                    break;
                }
            }
        }

        break;
    }

    case PacketType::Type_player_stop: {
        player_stop_packet* p = reinterpret_cast<player_stop_packet*>(buf);
        if (p->id != client_id) {
            for (int i = 0; i < MAX_PLAYER; ++i) {
                if (m_pScene->PlayerIDs[i] == p->id) {
                    m_pScene->AnimatePlayer(i, 0);
                    m_pScene->m_mPlayer[i]->m_pSkinnedAnimationController->SetTrackPosition(6, 0);
                    m_pScene->m_mPlayer[i]->m_pSkinnedAnimationController->SetTrackPosition(9, 0);
                }
            }
        }
        break;
    }

    case PacketType::Type_map_collapse: {
        map_collapse_packet* p = reinterpret_cast<map_collapse_packet*>(buf);
        //printf("break map: %d\n", p->block_num);
        break;
    }

    case PacketType::Type_cloud_move: {
        cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(buf);
        printf("id: %d cloud move x = %f, z = %f\n", client_id, p->x, p->z);
        break;
    }

    }
}

void CPacket::Set_clientid(int n)
{
    client_id = n;
}

int CPacket::Get_clientid()
{
    return client_id;
}

void CPacket::Set_currentfps(unsigned long FrameRate)
{
    if (FrameRate > 0)
        currentfps = FrameRate;
    else
        currentfps = 1;
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

    //gCPacket[num]->Set_clientid(num);

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
    serveraddr.sin_port = htons(GAMESERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    memset(&overlapped, 0, sizeof(overlapped));

    //cout << "game connect" << endl;



    std::thread Recv_thread = std::thread(&CPacket::RecvPacket, this);
    //std::thread test_Send_thread = std::thread(&CPacket::testSendPacket, this);
    //test_Send_thread.join();
    Recv_thread.join();

}