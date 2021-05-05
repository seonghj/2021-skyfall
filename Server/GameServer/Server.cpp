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
    LocalFree(lpMsgBuf);
}

int Server::SetClientId()
{
    int count = LOBBY_ID+1;
    while (true){
        if (sessions.count(count) == 0)
            return count;
        else 
            ++count;
    }
}

int Server::SetGameNum()
{
    int count = 1;
    while (1) {
        if (gameroom.count(count) >= 20)
            count++;
        else
            return count;
    }
}

void Server::ConnectLobby()
{
    sessions.emplace(LOBBY_ID, SESSION());
    sessions[LOBBY_ID].id = LOBBY_ID;

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
        //printf("만들기 전 세션 갯수: %d\n", sessions.size());
        sessions.emplace(client_id, SESSION());
        //printf("만든 후 세션 갯수: %d\n", sessions.size());
        memset(&sessions[client_id], 0x00, sizeof(SESSION));
        sessions[client_id].id = client_id;
        sessions[client_id].sock = client_sock;
        sessions[client_id].clientaddr = clientaddr;

        getpeername(client_sock, (SOCKADDR*)&sessions[client_id].clientaddr
            , &sessions[client_id].addrlen);

        printf("client_connected: IP =%s, port=%d key = %d\n",
            inet_ntoa(sessions[client_id].clientaddr.sin_addr)
            , ntohs(sessions[client_id].clientaddr.sin_port), client_id);

        sessions[client_id].over.dataBuffer.len = BUFSIZE;
        sessions[client_id].over.dataBuffer.buf =
            sessions[client_sock].over.messageBuffer;
        sessions[client_id].over.is_recv = true;
        sessions[client_id].over.type = 0;
        flags = 0;

        // 소켓과 입출력 완료 포트 연결
        CreateIoCompletionPort((HANDLE)client_sock, hcp, client_id, 0);

        int gameroom_num = SetGameNum();
        //int gameroom_num = client_id % 2;

        sessions[client_id].gameroom_num = gameroom_num;

        gameroom.emplace(gameroom_num, client_id);
        //printf("만든 후 방 갯수: %d\n", gameroom.size());

        // id전송
        send_ID_player_packet(client_id);

        sessions[client_id].connected = true;

        // 로그인한 클라이언트에 다른 클라이언트 정보 전달
        auto iter = gameroom.equal_range(gameroom_num);
        for (auto it = iter.first; it != iter.second; ++it) {
            if (sessions[it->second].connected && (it->second != client_id))
                send_login_player_packet(it->second, client_id);
        }

        // 다른 클라이언트에 로그인정보 전달
        for (auto it = iter.first; it != iter.second; ++it){
            if (sessions[it->second].connected && (it->second != client_id) )
                send_login_player_packet(client_id, it->second);
        }

        /*if (20 == gameroom.count(gameroom_num))
        {
            maps.emplace(gameroom_num, Map(gameroom_num));
            maps[gameroom_num].init_Map(this);
        }*/


        do_recv(client_id);
    }

    // closesocket()
    closesocket(listen_sock); 

    // 윈속 종료
    WSACleanup();
}

void Server::Disconnected(int id)
{
    printf("client_end: IP =%s, port=%d key = %d\n",
        inet_ntoa(sessions[id].clientaddr.sin_addr)
        , ntohs(sessions[id].clientaddr.sin_port), id);
    //send_disconnect_player_packet(id);
    closesocket(sessions[id].sock);
    sessions.erase(id);
    //sessions.clear();
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

void Server::send_packet(int to, char* packet)
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
    send_packet(id, reinterpret_cast<char*>(&p));
}

void Server::send_login_player_packet(char id, int to)
{
    player_login_packet p;

    p.id = id;
    p.size = sizeof(player_login_packet);
    p.type = PacketType::Type_player_login;

    printf("%d: login to %d\n",id, to);

    send_packet(to, reinterpret_cast<char*>(&p));
}

void Server::send_disconnect_player_packet(char id)
{
    player_remove_packet p;
    p.id = id;
    p.size = sizeof(player_remove_packet);
    p.type = PacketType::Type_player_remove;

    auto iter = gameroom.equal_range(sessions[id].gameroom_num);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected)
            send_packet(it->second, reinterpret_cast<char*>(&p));

    }
    /*for (auto &iter: sessions){
        if (iter.second.connected &&
            iter.second.gameroom_num == sessions[id].gameroom_num)
            send_packet(iter.first, reinterpret_cast<char*>(&p));
    }*/
    Disconnected(id);
}

void Server::send_packet_to_players(char id, char* buf)
{
    auto iter = gameroom.equal_range(sessions[id].gameroom_num);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected)
            if (calc_distance(id, it->second) <= VIEWING_DISTANCE)
                send_packet(it->second, buf);
    }

    /*for (auto& iter : sessions) {
        if (iter.second.connected && 
            (iter.second.gameroom_num == sessions[id].gameroom_num)){
            if (calc_distance(id, iter.first) <= VIEWING_DISTANCE) {
                send_packet(iter.first, buf);
            }
        }
    }*/
}

void Server::send_packet_to_players(int game_num, char* buf)
{
    auto iter = gameroom.equal_range(game_num);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected)
            send_packet(it->second, buf);
    }
   /* for (auto& iter : sessions) {
        if (iter.second.connected &&
            iter.second.gameroom_num == game_num) {
            send_packet(iter.first, buf);
        }
    };*/
}

void Server::send_map_collapse_packet(int num, int map_num)
{
    map_collapse_packet packet;
    packet.size = sizeof(map_collapse_packet);
    packet.type = PacketType::Type_map_collapse;
    packet.block_num = num;

    auto iter = gameroom.equal_range(map_num);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected)
            send_packet(it->second, reinterpret_cast<char*>(&packet));
    }
    /*for (auto& iter : sessions) {
        if (iter.second.connected &&
            iter.second.gameroom_num == map_num) {
            send_packet(iter.first, reinterpret_cast<char*>(&packet));
        }
    }*/
}

void Server::send_cloud_move_packet(float x, float z, int map_num)
{
    cloud_move_packet packet;
    packet.size = sizeof(cloud_move_packet);
    packet.type = PacketType::Type_cloud_move;
    packet.x = x;
    packet.z = z;

    auto iter = gameroom.equal_range(map_num);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected)
            send_packet(it->second, reinterpret_cast<char*>(&packet));
    }
    //for (auto& iter : sessions) {
    //    if (iter.second.connected &&
    //        iter.second.gameroom_num == map_num) {
    //        //printf("Send %d: %f/%f\n", iter.first, packet.x, packet.z);
    //        send_packet(iter.first, reinterpret_cast<char*>(&packet));
    //    }
    //}
}

void Server::game_end(int game_num)
{
    game_end_packet packet;
    packet.id = 0;
    packet.size = sizeof(packet);
    packet.type = PacketType::Type_game_end;

    auto iter = gameroom.equal_range(game_num);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected)
            send_packet(it->second, reinterpret_cast<char*>(&packet));
    }

    /*for (auto& iter : sessions) {
        if (iter.second.connected) {
            send_packet(iter.first, reinterpret_cast<char*>(&packet));
        }
    }*/
}

float Server::calc_distance(int a, int b)
{
    float value = pow((sessions[a].f3Position.load(std::memory_order_relaxed).x - sessions[b].f3Position.load(std::memory_order_relaxed).x), 2)
        + pow((sessions[a].f3Position.load(std::memory_order_relaxed).z - sessions[b].f3Position.load(std::memory_order_relaxed).z), 2);

    if (value < 0)
        return sqrt(-value);
    else
        return sqrt(value);
}

DirectX::XMFLOAT3 Server::move_calc(DWORD dwDirection, float fDistance, int state, int id)
{
    XMFLOAT3 xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
    XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

    XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

    switch (dwDirection) {
    case DIR_FORWARD: {
        xmf3Shift = Add(xmf3Shift, xmf3Look, fDistance);
        break;
    }
    case DIR_BACKWARD: {
        xmf3Shift = Add(xmf3Shift, xmf3Look, -fDistance);
        break;
    }
    case DIR_LEFT: {
        xmf3Shift = Add(xmf3Shift, xmf3Right, -fDistance);
        break;
    }
    case DIR_RIGHT: {
        xmf3Shift = Add(xmf3Shift, xmf3Right, fDistance);
        break;
    }
    }

    XMFLOAT3 xmf3Velocity;

    xmf3Velocity.x = 0;
    xmf3Velocity.y = 0;
    xmf3Velocity.z = 0;

    xmf3Velocity = Add(xmf3Velocity, xmf3Shift);
    if (state == PlayerMove::RUNNING)
    {
        xmf3Velocity.x *= 3.3;
        xmf3Velocity.z *= 3.3;
    }

    xmf3Shift = Add(sessions[id].f3Position.load(), xmf3Shift);

    return xmf3Shift;
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
        start_ok_packet* p = new start_ok_packet;
        p->size = sizeof(p);
        p->type = Type_start_ok;
        break;
    }
    case PacketType::Type_player_info:{
        player_info_packet* p = reinterpret_cast<player_info_packet*>(buf);
        sessions[p->id].f3Position.store(p->Position);

        break;
    }
    case PacketType::Type_player_pos: {
        player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);

        sessions[p->id].f3Position.store(p->Position);
        sessions[p->id].dx.store(p->dx);
        sessions[p->id].dy.store(p->dy);
        sessions[p->id].dz.store(p->dz);
        //printf("move %f %f\n", sessions[p->id].f3Position.load().x, sessions[p->id].f3Position.load().z);

        send_packet_to_players(id, reinterpret_cast<char*>(p));

        break;
    }
    case PacketType::Type_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        sessions[p->id].f3Position.store(p->Position);

        send_packet(id, reinterpret_cast<char*>(p));
        break;
    }

    case PacketType::Type_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);


        switch (p->MoveType) {
        case PlayerMove::JUMP:{
            send_packet_to_players(id, reinterpret_cast<char*>(p));
                break;
            }
        default: {
            break;
        }
        }
        {
            //DirectX::XMFLOAT3 Position = move_calc(p->MoveType, sessions[p->id].speed,p->state, p->id);
            //float dx = p->dx;
            //float dy = p->dy;
            //float dz = p->dz;
            //
            ///*float dx = sessions[p->id].dx.load();
            //float dy = sessions[p->id].dy.load();
            //float dz = sessions[p->id].dz.load();
            //
            //while (!sessions[p->id].dx.compare_exchange_strong(dx, p->dx)) {};
            //while (!sessions[p->id].dy.compare_exchange_strong(dy, p->dy)) {};
            //while (!sessions[p->id].dz.compare_exchange_strong(dz, p->dz)) {};*/
            //sessions[p->id].f3Position.store(Position);

            //player_pos_packet* pp = new player_pos_packet;
            //pp->size = sizeof(player_pos_packet);
            //pp->type = Type_player_pos;
            //pp->id = p->id;
            //pp->Position = Position;
            //pp->dx = dx;
            //pp->dy = dy;
            //pp->dy = dz;
            //pp->state = p->state;

            //send_player_move_packet(id, reinterpret_cast<char*>(pp));
        }
        break;
    }
    case PacketType::Type_player_attack:{
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        send_packet_to_players(p->id, reinterpret_cast<char*>(p));
        break;
    }
    }
}

void Server::WorkerFunc()
{

    while (1) {
        DWORD Transferred;
        SOCKET client_sock;
        ULONG id;
        SESSION* ptr;

        OVER_EX* over_ex;

        BOOL retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&id, (LPOVERLAPPED*)&over_ex, INFINITE);

        //std::thread::id Thread_id = std::this_thread::get_id();

        switch (over_ex->type) {
        case 0: {
            // 비동기 입출력 결과 확인
            if (FALSE == retval)
            {
                //printf("error = %d\n", WSAGetLastError());
                display_error("GQCS", WSAGetLastError());
                Disconnected(id);
                continue;
            }

            if ((Transferred == 0)) {
                Disconnected(id);
                continue;
            }

            if (over_ex->is_recv) {
                //printf("thread id: %d\n", Thread_id);
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
            break;
        }

        case 1: {
            if (FALSE == retval)
                continue;
            cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(over_ex->messageBuffer);
            send_cloud_move_packet(p->x, p->z, p->id);
            printf("cloud x: %f | y: %f\n\n", p->x, p->z);
            break;
        }
        }
    }
}

bool Server::Init()
{
    sessions.clear();

    // 입출력 완료 포트 생성
    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    // CPU 개수 확인
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    // (CPU 개수 * 2)개의 작업자 스레드 생성
    for (int i = 0; i < (int)si.dwNumberOfProcessors; i++)
        working_threads.emplace_back(std::thread(&Server::WorkerFunc, this));

    //ConnectLobby();

    accept_thread = std::thread(&Server::Accept, this);

    return 1;
}

void Server::Thread_join()
{
    accept_thread.join();
    for (auto& t : working_threads)
        t.join();

    for (auto& t : map_threads)
        t.join();

    CloseHandle(hcp);
}