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
    int count = LOBBY_ID;
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

void Server::ConnectLobby()
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    // socket()
    sessions[LOBBY_ID].sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    // connect()
    SOCKADDR_IN lobbyaddr;
    ZeroMemory(&lobbyaddr, sizeof(lobbyaddr));
    lobbyaddr.sin_family = AF_INET;
    lobbyaddr.sin_addr.s_addr = inet_addr(SERVERIP);
    lobbyaddr.sin_port = htons(LOBBYPORT);
    connect(sessions[LOBBY_ID].sock, (SOCKADDR*)&lobbyaddr, sizeof(lobbyaddr));

    memset(&sessions[LOBBY_ID].over.overlapped, 0, sizeof(sessions[LOBBY_ID].over.overlapped));
    sessions[LOBBY_ID].over.is_recv = true;
    sessions[LOBBY_ID].over.dataBuffer.len = BUFSIZE;
    sessions[LOBBY_ID].over.dataBuffer.buf =
        sessions[LOBBY_ID].over.messageBuffer;
    sessions[LOBBY_ID].connected = true;

    CreateIoCompletionPort((HANDLE)sessions[LOBBY_ID].sock, hcp, LOBBY_ID, 0);

    do_recv(LOBBY_ID);
}

void Server::Accept()
{
    // socket()
    SOCKET listen_sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(GAMESERVERPORT);
    int retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
  
    // listen()
    retval = listen(listen_sock, MAX_CLIENT);

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen = sizeof(SOCKADDR_IN);
    DWORD flags;

    printf("ready\n");

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

        // 로그인한 클라이언트에 다른 클라이언트 정보 전달
        for (int i = 0; i < MAX_CLIENT; ++i) {
            if (sessions[i].connected && i != client_id)
                send_login_player_packet(i, client_id);
        }

        // 다른 클라이언트에 로그인정보 전달
        for (int i = 0; i < MAX_CLIENT; ++i) {
            if (sessions[i].connected && i != client_id)
                send_login_player_packet(client_id, i);
        }
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

void Server::send_player_move_packet(char id)
{
    for (auto& iter : sessions) {
        if (iter.second.connected){
            if (calc_distance(id, iter.first) <= VIEWING_DISTANCE) {
                do_send(iter.first, sessions[id].packet_buf);
            }
        }
    }
}

void Server::send_player_attack_packet(char id, char* buf)
{
    for (auto& iter : sessions) {
        if (iter.second.connected) {
            if (calc_distance(id, iter.first) <= VIEWING_DISTANCE) {
                do_send(iter.first, sessions[id].packet_buf);
            }
        }
    }
}

void Server::send_map_collapse_packet(int num)
{
    map_collapse_packet packet;
    packet.size = sizeof(map_collapse_packet);
    packet.type = PacketType::Type_map_collapse;
    packet.block_num = num;

    for (auto& iter : sessions) {
        if (iter.second.connected)
            do_send(iter.first, reinterpret_cast<char*>(&packet));
    }
}


void Server::send_cloud_move_packet(float x, float z)
{
    cloud_move_packet packet;
    packet.size = sizeof(cloud_move_packet);
    packet.type = PacketType::Type_cloud_move;
    packet.x = x;
    packet.z = z;


    for (auto& iter : sessions) {
        if (iter.second.connected) {
            //printf("Send %d: %f/%f\n", iter.first, packet.x, packet.z);
            do_send(iter.first, reinterpret_cast<char*>(&packet));
        }
    }
}

float Server::calc_distance(int a, int b)
{
    float value = pow((sessions[a].x.load(std::memory_order_relaxed) - sessions[b].x.load(std::memory_order_relaxed)), 2)
        + pow((sessions[a].z.load(std::memory_order_relaxed) - sessions[b].z.load(std::memory_order_relaxed)), 2);

    if (value < 0)
        return sqrt(-value);
    else
        return sqrt(value);
}

void Server::process_packet(char id, char* buf)
{
    // 클라이언트에서 받은 패킷 처리
    switch (buf[1]) {
    case PacketType::Type_game_ready: {
        sessions[buf[2]].isready = true;
        break;
    }
    case PacketType::Type_game_start: {
        start_ok_packet* sop = new start_ok_packet;
        sop->size = sizeof(sop);
        sop->type = Type_start_ok;
        /*for (auto i = sessions.begin(); i != sessions.end(); ++i) {
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
        }*/
        break;
    }
    case PacketType::Type_player_info:{

        break;
    }
    case PacketType::Type_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);

        float degree = sessions[p->id].degree.load();
        float x = sessions[p->id].x.load();
        float y = sessions[p->id].y.load();
        float z = sessions[p->id].z.load();

        while (!sessions[p->id].degree.compare_exchange_strong(degree, p->degree)) {};
        while (!sessions[p->id].x.compare_exchange_strong(x, p->x)) { };
        while (!sessions[p->id].y.compare_exchange_strong(y, p->y)) {  };
        while (!sessions[p->id].z.compare_exchange_strong(z, p->z)) {  };

        send_player_move_packet(id);
        break;
    }
    case PacketType::Type_player_attack:{
        break;
    }
    }
}

void Server::WorkerFunc()
{
    int retval = 0;

    while (1) {
        DWORD Transferred;
        SOCKET client_sock;
        ULONG id;
        SESSION* ptr;

        OVER_EX* over_ex;

        retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&id, (LPOVERLAPPED*)&over_ex, INFINITE);

        //std::thread::id Thread_id = std::this_thread::get_id();

        // 비동기 입출력 결과 확인
        if (FALSE == retval)
        {
            display_error("GQCS", WSAGetLastError());
            Disconnected(id);
        }


        if (over_ex->is_recv) {

            /*char* next_recv_ptr = sessions[id].recv_start + Transferred;
            int packet_size = *sessions[id].packet_start;
            while (packet_size <= (sessions[id].recv_start - sessions[id].packet_start)) {
                process_packet(id, sessions[id].packet_start);
                sessions[id].packet_start += packet_size;
                if (sessions[id].recv_start > sessions[id].packet_start)
                    packet_size = *sessions[id].packet_start;
                else break;
            }

            if (sessions[id].recv_start - sessions[id].packet_buf < 3) {
                memcpy(sessions[id].packet_start, sessions[id].packet_buf,
                    (next_recv_ptr - sessions[id].packet_start));
                sessions[id].packet_start = sessions[id].packet_buf;
                sessions[id].recv_start = sessions[id].packet_buf;
            }*/

            ////printf("thread id: %d\n", Thread_id);
            int rest_size = Transferred;
            char* buf_ptr = over_ex->messageBuffer;
            char packet_size = 0;
            if (0 < sessions[id].prev_size)
                packet_size = sessions[id].packet_buf[0];
            while (rest_size > 0) {
                if (0 == packet_size) packet_size = buf_ptr[0];
                int required = packet_size - sessions[id].prev_size;
                if (rest_size >= required) {
                    memcpy(sessions[id].packet_buf + sessions[id].
                        prev_size, buf_ptr, required);
                    process_packet(id, sessions[id].packet_buf);
                    rest_size -= required;
                    buf_ptr += required;
                    packet_size = 0;
                }
                else {
                    memcpy(sessions[id].packet_buf + sessions[id].prev_size,
                        buf_ptr, rest_size);
                    rest_size = 0;
                }
            }
            do_recv(id);
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
    for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
        working_threads.emplace_back(std::thread(&Server::WorkerFunc, this));

    ConnectLobby();

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