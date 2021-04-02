#pragma once
#pragma warning(disable : 4996)
#include "Server.h"

Server::Server()
{

}

Server::~Server()
{

}

void Server::display_error(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::cout << msg;
    std::wcout << L"에러 " << lpMsgBuf << std::endl;
    while (true);
    LocalFree(lpMsgBuf);
}

int Server::SetClientId()
{
    int count = GAMESERVER_ID + 1;
    auto iter = sessions.begin();
    while (true) {
        if (!iter->second.connected) {
            iter->second.connected = true;
            return count;
        }
        else {
            ++count;
            ++iter;
        }
    }
}

void Server::Accept()
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    // socket()
    SOCKET listen_sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(LOBBYSERVERPORT);
    int retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
  
    // listen()
    retval = listen(listen_sock, MAX_CLIENT);

    SOCKET gameserver_sock;
    SOCKADDR_IN gameserveraddr;
    int addrlen = sizeof(SOCKADDR_IN);
    DWORD flags;
    gameserver_sock = accept(listen_sock, (struct sockaddr*)&gameserveraddr, &addrlen);
    if (gameserver_sock == INVALID_SOCKET) {
        display_error("accept error: ", WSAGetLastError());
    }

    sessions.emplace(GAMESERVER_ID, SESSION());
    memset(&sessions[GAMESERVER_ID], 0x00, sizeof(SESSION));
    sessions[GAMESERVER_ID].id = GAMESERVER_ID;
    sessions[GAMESERVER_ID].sock = gameserver_sock;
    sessions[GAMESERVER_ID].clientaddr = gameserveraddr;

    printf("gameserver connect\n");

    sessions[GAMESERVER_ID].over.dataBuffer.len = BUFSIZE;
    sessions[GAMESERVER_ID].over.dataBuffer.buf =
        sessions[GAMESERVER_ID].over.messageBuffer;
    sessions[GAMESERVER_ID].over.is_recv = true;
    flags = 0;

    // 소켓과 입출력 완료 포트 연결
    CreateIoCompletionPort((HANDLE)gameserver_sock, hcp, GAMESERVER_ID, 0);
    sessions[GAMESERVER_ID].connected = true;

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;

    printf("ready\n");

    send_game_start_packet(GAMESERVER_ID);

    while (1) {
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            display_error("accept error: ", WSAGetLastError());
            break;
        }

        int client_id = SetClientId();
        sessions.emplace(client_id, SESSION());
        memset(&sessions[client_id], 0x00, sizeof(SESSION));
        sessions[client_id].id = client_id;
        sessions[client_id].sock = client_sock;
        sessions[client_id].clientaddr = clientaddr;

        getpeername(client_sock, (SOCKADDR*)&sessions[client_id].clientaddr
            , &sessions[client_id].addrlen);

        printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d, key=%d\n",
            inet_ntoa(sessions[client_id].clientaddr.sin_addr)
            , ntohs(sessions[client_id].clientaddr.sin_port), client_id);

        sessions[client_id].over.dataBuffer.len = BUFSIZE;
        sessions[client_id].over.dataBuffer.buf =
            sessions[client_sock].over.messageBuffer;
        sessions[client_id].over.is_recv = true;
        flags = 0;

        // 소켓과 입출력 완료 포트 연결
        CreateIoCompletionPort((HANDLE)client_sock, hcp, client_id, 0);
        sessions[client_id].connected = true;

        // id전송
        send_ID_player_packet(client_id);

        do_recv(client_id);
    }

    // closesocket()
    closesocket(listen_sock); 

    // 윈속 종료
    WSACleanup();
}

void Server::Disconnected(int id)
{
    sessions[id].connected = false;
    printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d key = %d\n",
        inet_ntoa(sessions[id].clientaddr.sin_addr)
        , ntohs(sessions[id].clientaddr.sin_port), id);
    send_disconnect_player_packet(id);
}

void Server::do_recv(char id)
{
    DWORD flags = 0;

    SOCKET client_s = sessions[id].sock;
    OVER_EX* over = &sessions[id].over;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));

    if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)sessions[id].prev_size,
        &flags, &(over->overlapped), NULL)){
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING){
            display_error("recv error: ", err_no);
        }
    }
    //printf("Recv id: %d: type:%d / size: %d\n", id, over->dataBuffer.buf[1], over->dataBuffer.buf[0]);
    //memcpy(sessions[id].packet_buf, over->dataBuffer.buf, over->dataBuffer.buf[0]);
}

void Server::do_send(int to, char* packet)
{
    SOCKET client_s = sessions[to].sock;
    OVER_EX* over = reinterpret_cast<OVER_EX*>(malloc(sizeof(OVER_EX)));

    over->dataBuffer.buf = packet;
    over->dataBuffer.len = packet[0];

    memcpy(over->messageBuffer, packet, packet[0]);

    ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
    over->is_recv = false;

    if (WSASend(client_s, &over->dataBuffer, 1, NULL,
        0, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING){
            display_error("send error: ", err_no);
        }
    }
}

void Server::send_ID_player_packet(char id)
{
    player_ID_packet p;

    p.id = id;
    p.size = sizeof(player_login_packet);
    p.type = PacketType::Type_player_ID;
    do_send(id, reinterpret_cast<char*>(&p));
}

void Server::send_login_player_packet(char id, int to)
{
    player_login_packet p;

    p.id = id;
    p.size = sizeof(player_login_packet);
    p.type = PacketType::Type_player_login;

    //printf("%d: login\n",id);

    do_send(to, reinterpret_cast<char*>(&p));
}

void Server::send_disconnect_player_packet(char id)
{
    player_remove_packet p;
    p.id = id;
    p.size = sizeof(player_remove_packet);
    p.type = PacketType::Type_player_remove;

    for (auto &iter: sessions){
        if (iter.second.connected)
            do_send(iter.first, reinterpret_cast<char*>(&p));
    }
    closesocket(sessions[id].sock);
}

void Server::send_game_start_packet(char id)
{
    game_start_packet p;
    p.id = id;
    p.size = sizeof(player_remove_packet);
    p.type = PacketType::Type_game_start;

    do_send(id, reinterpret_cast<char*>(&p));
}

void Server::process_packet(char id, char* buf)
{
    // 클라이언트에서 받은 패킷 처리
    switch (buf[1]) {
    case PacketType::Type_game_ready: {
        sessions[buf[2]].isready = true;
        break;
    }
   /* case PacketType::Type_game_start: {
        start_ok_packet* sop = new start_ok_packet;
        sop->size = sizeof(sop);
        sop->type = Type_start_ok;
        for (auto i = sessions.begin(); i != sessions.end(); ++i) {
            if (!i->second.isready) {
                sop->value = false;
                break;
            }
            else
                sop->value = true;
        }
        for (auto& iter : sessions) {
            if (iter.second.connected)
                do_send(iter.first, reinterpret_cast<char*>(&sop));
        }
        break;
    }*/
    }
}

void Server::WorkerFunc()
{
    int retval = 0;

    while (1) {
        DWORD Transferred;
        SOCKET client_sock;
        ULONG client_id;
        SESSION* ptr;

        OVER_EX* over_ex;

        retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&client_id, (LPOVERLAPPED*)&over_ex, INFINITE);

        // 비동기 입출력 결과 확인
        if (FALSE == retval)
        {
            display_error("GQCS", WSAGetLastError());
            Disconnected(client_id);
        }


        if (over_ex->is_recv) {
            ////printf("thread id: %d\n", Thread_id);
            int rest_size = Transferred;
            char* buf_ptr = over_ex->messageBuffer;
            char packet_size = 0;
            if (0 < sessions[client_id].prev_size)
                packet_size = sessions[client_id].packet_buf[0];
            while (rest_size > 0) {
                if (0 == packet_size) packet_size = buf_ptr[0];
                int required = packet_size - sessions[client_id].prev_size;
                if (rest_size >= required) {
                    memcpy(sessions[client_id].packet_buf + sessions[client_id].
                        prev_size, buf_ptr, required);
                    process_packet(client_id, sessions[client_id].packet_buf);
                    rest_size -= required;
                    buf_ptr += required;
                    packet_size = 0;
                }
                else {
                    memcpy(sessions[client_id].packet_buf + sessions[client_id].prev_size,
                        buf_ptr, rest_size);
                    rest_size = 0;
                }
            }
            do_recv(client_id);
            //printf("ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ\n");
        }
        else {
            delete over_ex;
        }
    }
}

bool Server::Init()
{
    for (int i = 0; i < MAX_CLIENT; ++i)
        sessions[i].connected = false;

    // 입출력 완료 포트 생성
    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    // CPU 개수 확인
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    // (CPU 개수 * 2)개의 작업자 스레드 생성
    for (int i = 0; i < (int)si.dwNumberOfProcessors; i++)
        working_threads.emplace_back(std::thread(&Server::WorkerFunc, this));

    accept_thread = std::thread(&Server::Accept, this);

    return 1;
}

void Server::Thread_join()
{
    accept_thread.join();
    for (auto& t : working_threads)
        t.join();

    CloseHandle(hcp);
}