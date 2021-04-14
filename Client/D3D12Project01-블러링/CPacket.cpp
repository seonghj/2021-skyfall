#pragma once
#include "CPacket.h"
#include <iostream>
#pragma warning(disable : 4996)

PacketFunc::PacketFunc()
{

}

PacketFunc::~PacketFunc()
{

}

// ���� �Լ� ���� ��� �� ����
void PacketFunc::err_quit(char* msg)
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

// ���� �Լ� ���� ���
void PacketFunc::err_display(char* msg)
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

void PacketFunc::RecvPacket()
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

    // ������ �ޱ�
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
        //// ���� ������ ���
        while (rest_size > 0)
        {
            if (0 == packet_size)
                packet_size = recvbytes;
            if (rest_size + saved_packet_size >= packet_size) {
                std::memcpy(buffer + saved_packet_size, buf_ptr, packet_size - saved_packet_size);
                //std::printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�. %d\r\n", recvbytes, recvbuf[1]);

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

void PacketFunc::SendPacket(char* buf)
{
    int retval = 0;

    wsabuf.len = buf[0];
    wsabuf.buf = buf;
    retval = WSASend(sock, &wsabuf, 1, &sendbytes, 0, NULL, NULL);
    if (retval == SOCKET_ERROR) {
        err_display("send()");
    }
}

void PacketFunc::Send_ready_packet()
{
    game_ready_packet p;
    p.id = client_id;
    p.size = sizeof(p);
    p.type = PacketType::Type_game_ready;
    SendPacket(reinterpret_cast<char*>(&p));
}

void PacketFunc::ProcessPacket(char* buf)
{
    switch (buf[1])
    {
    case PacketType::Type_player_ID: {
        player_ID_packet* p = reinterpret_cast<player_ID_packet*>(buf);
        client_id = buf[2];
        printf("recv id from server: %d\n", p->id);
        Send_ready_packet();
        break;
    }
    case PacketType::Type_player_login: {
        player_login_packet* p = reinterpret_cast<player_login_packet*>(buf);
        //printf("login id: %d\n", p->id);
        break;
    }
    case PacketType::Type_player_remove: {

        break;
    }
    case PacketType::Type_start_ok: {
        GameConnect();
        break;
    }
    case PacketType::Type_game_end: {
        printf("gameover\n");
        LobbyConnect();
        break;
    }
    case PacketType::Type_player_info: {

        break;
    }

    case PacketType::Type_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        //if (client_id == 1)
          //  printf("id: %d player move x = %f, y = %f, z = %f\n", p->id, p->x, p->y, p->z);
        break;
    }
    case PacketType::Type_player_pos: {
        player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);
        m_pPlayer->SetPosition(p->Position);
        m_pPlayer->Update(60.0);
        break;
    }
    case PacketType::Type_player_attack: {
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

void PacketFunc::Set_clientid(int n)
{
    client_id = n;
}

int PacketFunc::Get_clientid()
{
    return client_id;
}

void PacketFunc::Set_currentfps(unsigned long FrameRate)
{
    if (FrameRate > 0)
        currentfps = FrameRate;
    else
        currentfps = 1;
}

void PacketFunc::LobbyConnect()
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


    //gPacketFunc[num]->Set_clientid(num);

    Recv_thread = std::thread(&PacketFunc::RecvPacket, this);
    //std::thread test_Send_thread = std::thread(&PacketFunc::testSendPacket, gPacketFunc[num]);
    //test_Send_thread.join();
    Recv_thread.join();

}

void PacketFunc::GameConnect()
{
    // disconnect
    shutdown(sock, SD_RECEIVE);
    closesocket(sock);
    //cout << "lobby diconnect" << endl;

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

    std::thread Recv_thread = std::thread(&PacketFunc::RecvPacket, this);
    //std::thread test_Send_thread = std::thread(&PacketFunc::testSendPacket, this);
    //test_Send_thread.join();
    Recv_thread.join();

}