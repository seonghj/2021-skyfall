#pragma once
#pragma warning(disable : 4996)
#include "ServerFunc.h"
#include "CPacket.h"

IOCPServer::IOCPServer()
{

}

IOCPServer::~IOCPServer()
{

}

void IOCPServer::display_error(const char* msg, int err_no)
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

int IOCPServer::SetClientId()
{
    while (true) {
        for (int i = 0; i < MAX_CLIENT; ++i)
            if (players[i].connected == false) {
                players[i].connected = true;
                return i;
            }
    }
}

void IOCPServer::Accept()
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
    serveraddr.sin_port = htons(SERVERPORT);
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
        //players[client_id] = SESSION();

        players.emplace(client_id, SESSION());
        memset(&players[client_id], 0x00, sizeof(SESSION));
        players[client_id].id = client_id;
        players[client_id].sock = client_sock;
        players[client_id].clientaddr = clientaddr;

        getpeername(client_sock, (SOCKADDR*)&players[client_id].clientaddr
            , &players[client_id].addrlen);

        printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d, key=%d\n",
            inet_ntoa(players[client_id].clientaddr.sin_addr)
            , ntohs(players[client_id].clientaddr.sin_port), client_id);

        players[client_id].over.dataBuffer.len = BUFSIZE;
        players[client_id].over.dataBuffer.buf =
            players[client_sock].over.messageBuffer;
        players[client_id].over.is_recv = true;
        flags = 0;

        // 소켓과 입출력 완료 포트 연결
        CreateIoCompletionPort((HANDLE)client_sock, hcp, client_id, 0);
        players[client_id].connected = true;

        send_ID_player_packet(client_id);

        // 로그인한 클라이언트에 다른 클라이언트 정보 전달
        for (int i = 0; i < MAX_CLIENT; ++i) {
            if (players[i].connected && i != client_id)
                send_login_player_packet(i, client_id);
        }

        // 다른 클라이언트에 로그인정보 전달
        for (int i = 0; i < MAX_CLIENT; ++i) {
            if (players[i].connected && i != client_id)
                send_login_player_packet(client_id, i);
        }
        do_recv(client_id);
    }

    // closesocket()
    closesocket(listen_sock); 

    // 윈속 종료
    WSACleanup();
}

void IOCPServer::Disconnected(int id)
{
    players[id].connected = false;
    printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d key = %d\n",
        inet_ntoa(players[id].clientaddr.sin_addr)
        , ntohs(players[id].clientaddr.sin_port), id);
    send_disconnect_player_packet(id);
}

void IOCPServer::do_recv(char id)
{
    DWORD flags = 0;

    SOCKET client_s = players[id].sock;
    OVER_EX* over = &players[id].over;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));

    if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)players[id].prev_size,
        &flags, &(over->overlapped), NULL))
    {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING)
        {
            display_error("recv error: ", err_no);
        }
    }
    printf("Recv id: %d: type:%d / size: %d\n", id, over->dataBuffer.buf[1], over->dataBuffer.buf[0]);
    //memcpy(players[id].packet_buf, over->dataBuffer.buf, over->dataBuffer.buf[0]);
}

void IOCPServer::do_send(int to, char* packet)
{
    SOCKET client_s = players[to].sock;
    OVER_EX* over = reinterpret_cast<OVER_EX*>(malloc(sizeof(OVER_EX)));

    over->dataBuffer.buf = packet;
    over->dataBuffer.len = packet[0];

    memcpy(over->messageBuffer, packet, packet[0]);

    ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
    over->is_recv = false;

    if (WSASend(client_s, &over->dataBuffer, 1, NULL,
        0, &(over->overlapped), NULL))
    {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING)
        {
            display_error("send error: ", err_no);
        }
    }
    printf("Send %d: %d/%d\n", to, over->dataBuffer.buf[1], over->dataBuffer.len);
}

void IOCPServer::send_ID_player_packet(char id)
{
    player_ID_packet p;

    p.id = id;
    p.size = sizeof(player_login_packet);
    p.type = PacketType::Type_player_ID;

    printf("%d\n", id);
    do_send(id, reinterpret_cast<char*>(&p));
}

void IOCPServer::send_login_player_packet(char id, int to)
{
    player_login_packet p;

    p.id = id;
    p.size = sizeof(player_login_packet);
    p.type = PacketType::Type_player_login;

    //printf("%d: login\n",id);

    do_send(to, reinterpret_cast<char*>(&p));
}

void IOCPServer::send_disconnect_player_packet(char id)
{
    player_remove_packet p;
    p.id = id;
    p.size = sizeof(player_remove_packet);
    p.type = PacketType::Type_player_remove;

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (players[i].connected)
            do_send(i, reinterpret_cast<char*>(&p));
    }
    closesocket(players[id].sock);
}

void IOCPServer::send_player_move_packet(char id)
{
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (players[i].connected) {
            if (calc_distance(id, i) <= VIEWING_DISTANCE) {
                do_send(i, players[id].packet_buf);
            }
        }
    }
}

void IOCPServer::send_player_attack_packet(char id, char* buf)
{
    for (int i = 0; i < MAX_CLIENT; i++){
        if (players[i].connected) {
            if (calc_distance(id, i) <= VIEWING_DISTANCE) {
                do_send(i, players[id].packet_buf);
            }
        }
    }
}

void IOCPServer::send_map_collapse_packet(int num)
{
    map_collapse_packet packet;
    packet.size = sizeof(map_collapse_packet);
    packet.type = PacketType::Type_map_collapse;
    packet.block_num = num;

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (players[i].connected)
            do_send(i, reinterpret_cast<char*>(&packet));
    }
}


void IOCPServer::send_cloud_move_packet(float x, float z)
{
    cloud_move_packet packet;
    packet.size = sizeof(cloud_move_packet);
    packet.type = PacketType::Type_cloud_move;
    packet.x = x;
    packet.z = z;

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (players[i].connected)
        {
            printf("Send %d: %f/%f\n", i, packet.x, packet.z);
            do_send(i, reinterpret_cast<char*>(&packet));
        }
    }
}

float IOCPServer::calc_distance(int a, int b)
{
    float value = pow((players[a].x.load(std::memory_order_relaxed) - players[b].x.load(std::memory_order_relaxed)), 2)
        + pow((players[a].z.load(std::memory_order_relaxed) - players[b].z.load(std::memory_order_relaxed)), 2);

    if (value < 0)
        return sqrt(-value);
    else
        return sqrt(value);
}

void IOCPServer::process_packet(char id, char* buf)
{
    // 클라이언트에서 받은 패킷 처리
    switch (buf[1]) {
    
    case PacketType::Type_player_info:{
        break;
    }
    case PacketType::Type_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);

        float degree = players[p->id].degree.load();
        float x = players[p->id].x.load();
        float y = players[p->id].y.load();
        float z = players[p->id].z.load();

        while (!players[p->id].degree.compare_exchange_strong(degree, p->degree)) {};
        while (!players[p->id].x.compare_exchange_strong(x, p->x)) { };
        while (!players[p->id].y.compare_exchange_strong(y, p->y)) {  };
        while (!players[p->id].z.compare_exchange_strong(z, p->z)) {  };

        send_player_move_packet(id);
        break;
    }
    case PacketType::Type_player_attack:{
        break;
    }
    case PacketType::Type_map_collapse:{
        break;
    }
    case PacketType::Type_cloud_move:{
        break;
    }
    }
}

void IOCPServer::WorkerFunc()
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

        //std::thread::id Thread_id = std::this_thread::get_id();

        // 비동기 입출력 결과 확인
        if (FALSE == retval)
        {
            display_error("GQCS", WSAGetLastError());
            Disconnected(client_id);
        }


        if (over_ex->is_recv) {
            //printf("thread id: %d\n", Thread_id);
            int rest_size = Transferred;
            char* buf_ptr = over_ex->messageBuffer;
            char packet_size = 0;
            if (0 < players[client_id].prev_size)
                packet_size = players[client_id].packet_buf[0];
            while (rest_size > 0) {
                if (0 == packet_size) packet_size = buf_ptr[0];
                int required = packet_size - players[client_id].prev_size;
                if (rest_size >= required) {
                    memcpy(players[client_id].packet_buf + players[client_id].
                        prev_size, buf_ptr, required);
                    process_packet(client_id, players[client_id].packet_buf);
                    rest_size -= required;
                    buf_ptr += required;
                    packet_size = 0;
                }
                else {
                    memcpy(players[client_id].packet_buf + players[client_id].prev_size,
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

bool IOCPServer::Init()
{
    for (int i = 0; i < MAX_CLIENT; ++i)
        players[i].connected = false;

    // 입출력 완료 포트 생성
    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    // CPU 개수 확인
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    // (CPU 개수 * 2)개의 작업자 스레드 생성
    for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
        working_threads.emplace_back(std::thread(&IOCPServer::WorkerFunc, this));

    accept_thread = std::thread(&IOCPServer::Accept, this);

    return 1;
}

void IOCPServer::Thread_join()
{
    accept_thread.join();
    for (auto& t : working_threads)
        t.join();

    CloseHandle(hcp);
}