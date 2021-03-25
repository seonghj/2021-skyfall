#pragma once
#include "CPacket.h"

#pragma once

PacketFunc::PacketFunc()
{

}

PacketFunc::~PacketFunc()
{

}

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

    WSABUF r_wsabuf;

    char recv_buffer[BUFSIZE];
    char buffer[BUFSIZE];

    r_wsabuf.buf = buffer;
    r_wsabuf.len = BUFSIZE;

    DWORD packet_size = 0;

    // 데이터 받기
    while (1) {
        retval = WSARecv(sock, &wsabuf, 1, &recvbytes, &flags, NULL, NULL);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            printf("server disconnect\n");
            break;
        }
        int rest_size = sizeof(recv_buffer);
        BYTE* buf_ptr = reinterpret_cast<BYTE*>(recv_buffer);

        // 받은 데이터 출력
        while (rest_size > 0)
        {
            if (0 == packet_size)
                packet_size = retval;
            if (rest_size + saved_packet_size >= packet_size) {
                std::memcpy(buffer + saved_packet_size, buf_ptr, packet_size - saved_packet_size);
                //std::printf("[TCP 클라이언트] %d바이트를 받았습니다. %d\r\n", retval, recvbuf[1]);
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

void PacketFunc::ProcessPacket(char* buf)
{
    switch (buf[1])
    {
    case PacketType::T_player_ID: {
        player_ID_packet* p = reinterpret_cast<player_ID_packet*>(buf);
        client_id = buf[2];
        printf("recv id from server: %d\n", p->id);
        break;
    }
    case PacketType::T_player_login: {
        player_login_packet* p = reinterpret_cast<player_login_packet*>(buf);
        printf("login id: %d\n", p->id);
        break;
    }
    case PacketType::T_player_remove: {
        player_remove_packet* p = reinterpret_cast<player_remove_packet*>(buf);
        printf("%d client logout\n", buf[2]);
        break;
    }
    case PacketType::T_player_info: {

        break;
    }

    case PacketType::T_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        //printf("id: %d player move x = %f, y = %f, z = %f\n",p->id, p->x, p->y, p->z);
        break;
    }
    case PacketType::T_player_pos: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        //printf("id: %d player move x = %f, y = %f, z = %f\n",p->id, p->x, p->y, p->z);
        break;
    }
    case PacketType::T_player_attack: {
        break;
    }

    case PacketType::T_map_collapse: {
        map_collapse_packet* p = reinterpret_cast<map_collapse_packet*>(buf);
        //printf("break map: %d\n", p->block_num);
        break;
    }

    case PacketType::T_cloud_move: {
        cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(buf);
        //printf("cloud move x = %f, z = %f\n", p->x, p->z);
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

