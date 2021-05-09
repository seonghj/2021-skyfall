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

void CPacket::do_recv(int id)
{
    DWORD flags = 0;

    SOCKET client_s = players[id].sock;
    OVER_EX* over = &players[id].over;
    players[id].event_type = OP_RECV;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));

    if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)players[id].prev_size,
        &flags, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING) {
            printf("recv error: %d", err_no);
        }
    }
}

void CPacket::WorkerThread()
{

    while (1) {
        DWORD Transferred;
        SOCKET client_sock;
        ULONG id;
        //SESSION* ptr;

        OVER_EX* over_ex;

        BOOL retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&id, (LPOVERLAPPED*)&over_ex, INFINITE);

        //std::thread::id Thread_id = std::this_thread::get_id();

        // 비동기 입출력 결과 확인
        if (FALSE == retval)
        {
            //printf("error = %d\n", WSAGetLastError());
            closesocket(players[id].sock);
            num_connections--;
            continue;
        }

        if ((Transferred == 0)) {
            closesocket(players[id].sock);
            num_connections--;
            continue;
        }


        if (OP_RECV == players[id].event_type) {
            //printf("thread id: %d\n", Thread_id);
            int rest_size = Transferred;
            char* buf_ptr = over_ex->messageBuffer;
            char packet_size = 0;
            if (0 < players[id].prev_size)
                packet_size = players[id].packet_buf[0];
            /*while (rest_size > 0) {
                if (0 == packet_size) packet_size = buf_ptr[0];
                int required = packet_size - players[id].prev_size;
                if (rest_size >= required) {
                    memcpy(players[id].packet_buf + players[id].
                        prev_size, buf_ptr, required);
                    ProcessPacket(id, players[id].packet_buf);
                    rest_size -= required;
                    buf_ptr += required;
                    packet_size = 0;
                }
                else {
                    memcpy(players[id].packet_buf + players[id].prev_size,
                        buf_ptr, rest_size);
                    rest_size = 0;
                }
            }*/
            //printf("%d\n", over_ex->messageBuffer[1]);
            ProcessPacket(id, over_ex->messageBuffer);
            do_recv(id);
        }
       /* else {
            delete over_ex;
        }*/

    }
}

void CPacket::SendPacket(int id, char* packet)
{
    SOCKET client_s = players[id].sock;
    OVER_EX* over = reinterpret_cast<OVER_EX*>(malloc(sizeof(OVER_EX)));

    players[id].event_type = OP_SEND;
    over->dataBuffer.buf = packet;
    over->dataBuffer.len = packet[0];

    memcpy(over->messageBuffer, packet, packet[0]);

    ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
    over->is_recv = false;

    if (WSASend(client_s, &over->dataBuffer, 1, NULL,
        0, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING) {
            printf("send error: %d", err_no);
        }
    }
    //printf("%d send to server%d\n", id, packet[1]);
}

void CPacket::Send_ready_packet()
{
    game_ready_packet p;
    p.id = client_id;
    p.size = sizeof(p);
    p.type = PacketType::Type_game_ready;
    SendPacket(p.id, reinterpret_cast<char*>(&p));
}

void CPacket::ProcessPacket(int id, char* buf)
{
    switch (buf[1])
    {
    case PacketType::Type_player_ID: {
        player_ID_packet* p = reinterpret_cast<player_ID_packet*>(buf);
        players[p->id].id = p->id;
        //printf("recv id from server: %d\n", p->id);
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

        switch (p->MoveType){
        case PlayerMove::JUMP:
            //m_pPlayer->SetJump(TRUE);
            break;
        }
        break;
    }
    case PacketType::Type_player_pos: {
        //printf("왕\n");
        player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);
        players[p->id].pos.x = p->Position.x;
        players[p->id].pos.y = p->Position.z;
        //printf("id %d : move %d %d\n", p->id, players[p->id].pos.x, players[p->id].pos.y);
        
        break;
    }
    case PacketType::Type_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        players[p->id].pos.x = p->Position.x;
        players[p->id].pos.y = p->Position.z;
        break;
    }
    case PacketType::Type_player_attack: {
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);

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

void CPacket::TestGameConnect()
{
   
    int retval;

    if (num_connections < MAX_CLIENT){
        num_connections++;

        // socket()
        players[num_connections].sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (players[num_connections].sock == INVALID_SOCKET)err_quit("socket()");

        // connect()
        SOCKADDR_IN serveraddr;
        ZeroMemory(&serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
        serveraddr.sin_port = htons(GAMESERVERPORT);
        retval = connect(players[num_connections].sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR) err_quit("connect()");

        memset(&players[num_connections].over.overlapped, 0, sizeof(players[num_connections].over.overlapped));
        players[num_connections].connected = true;

        players[num_connections].pos.x = rand() % 1000;
        players[num_connections].pos.y = rand() % 1000;

        player_start_pos p;
        p.size = sizeof(p);
        p.type = Type_start_pos;
        p.Position.x = players[num_connections].pos.x;
        p.Position.z = players[num_connections].pos.y;
        p.id = num_connections;
        p.Position.y = 0;
        SendPacket(num_connections, reinterpret_cast<char*>(&p));
        

        CreateIoCompletionPort(reinterpret_cast<HANDLE>(players[num_connections].sock), hcp, num_connections, 0);

        printf("%d is connected\n", num_connections);
        do_recv(num_connections);

    }
}

void CPacket::Test_Thread()
{
    while (true) {
        //Sleep(max(20, global_delay));
        TestGameConnect();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for (int i = 0; i < num_connections; ++i) {
            if (false == players[i].connected) continue;
            player_pos_packet p;
            p.size = sizeof(p);
            p.type = Type_player_pos;
            switch (rand() % 4) {
            case 0: p.Position.x -= 20; break;
            case 1: p.Position.x += 20; break;
            case 2: p.Position.z -= 20; break;
            case 3: p.Position.z += 20; break;
            }
            p.id = i;
            p.dx = 0;
            p.dy = 0;
            p.dz = 0;
            p.Position.y = 0;
            p.state = 1;
            SendPacket(i, reinterpret_cast<char*>(&p));
        }

        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool CPacket::Init()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    for (auto& cl : players) {
        cl.connected = false;
        cl.id = 0;
    }

    num_connections = 0;

    // 입출력 완료 포트 생성
    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    // CPU 개수 확인
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    // (CPU 개수 * 2)개의 작업자 스레드 생성
    for (int i = 0; i < 20; i++)
        working_threads.emplace_back(std::thread(&CPacket::WorkerThread, this));

    test_threads = thread{ std::thread(&CPacket::Test_Thread, this) };


    return 1;
}

void CPacket::Thread_join()
{
    for (auto& t : working_threads)
        t.join();

    CloseHandle(hcp);
}