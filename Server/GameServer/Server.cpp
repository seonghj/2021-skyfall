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
        if (!sessions.find(count)->second.connected.load(std::memory_order_seq_cst)) {
            return count;
        }
        else 
            ++count;
    }
}

int Server::SetroomID()
{
    int count = 1;
    while (1) {
        if (gameroom.count(count) >= 20)
            ++count;
        else {
            return count;
        }
    }
}

void Server::ConnectLobby()
{
    sessions.emplace(LOBBY_ID, SESSION());
    sessions[LOBBY_ID].id = LOBBY_ID;
    sessions[LOBBY_ID].connected.store(TRUE);
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

        //accept_lock.lock();
        int client_id = SetClientId();
        sessions.emplace(client_id, SESSION());
        memset(&sessions[client_id], 0x00, sizeof(SESSION));
        sessions[client_id].connected.store(TRUE);
        sessions[client_id].id = client_id;
        sessions[client_id].sock = client_sock;
        sessions[client_id].clientaddr = clientaddr;
        getpeername(client_sock, (SOCKADDR*)&sessions[client_id].clientaddr
            , &sessions[client_id].addrlen);

        int roomID = SetroomID();
        gameroom.emplace(roomID, client_id);
        auto iter = gameroom.equal_range(roomID);

        printf("client_connected: IP =%s, port=%d key = %d\n",
            inet_ntoa(sessions[client_id].clientaddr.sin_addr)
            , ntohs(sessions[client_id].clientaddr.sin_port), client_id);

        sessions[client_id].over.type = 0;
        sessions[client_id].over.dataBuffer.len = BUFSIZE;
        sessions[client_id].over.dataBuffer.buf =
            sessions[client_sock].over.messageBuffer;
        sessions[client_id].over.is_recv = true;
        sessions[client_id].over.roomID = roomID;
        flags = 0;

        // 소켓과 입출력 완료 포트 연결
        CreateIoCompletionPort((HANDLE)client_sock, hcp, client_id, 0);


        //printf("create game room - %d\n", roomID);

        // id전송
        send_ID_player_packet(client_id, roomID);

        sessions[client_id].f3Position.store(XMFLOAT3(0 , 8.0f, 0));


        // 로그인한 클라이언트에 다른 클라이언트 정보 전달
        for (auto it = iter.first; it != iter.second; ++it) {
            if (sessions[it->second].connected.load(std::memory_order_seq_cst))
                send_login_player_packet(it->second, client_id, roomID);
        }

        // 다른 클라이언트에 로그인정보 전달
        for (auto it = iter.first; it != iter.second; ++it){
            if (sessions[it->second].connected.load(std::memory_order_seq_cst) && (it->second != client_id))
                send_login_player_packet(client_id, it->second, roomID);
        }

        /*if (20 == gameroom.count(roomID))
        {
            if (maps.find(roomID) == maps.end()) {
                maps.emplace(roomID, Map(roomID));
                maps[roomID].SetNum(roomID);
                maps[roomID].init_Map(this);
            }
        }*/
        //accept_lock.unlock();

        do_recv(client_id);
    }

    // closesocket()
    closesocket(listen_sock); 

    // 윈속 종료
    WSACleanup();
}

void Server::Disconnected(int id, int roomID)
{
    printf("client_end: IP =%s, port=%d key = %d\n",
        inet_ntoa(sessions[id].clientaddr.sin_addr)
        , ntohs(sessions[id].clientaddr.sin_port), id);
    //send_disconnect_player_packet(id);
    closesocket(sessions[id].sock);
    sessions[id].connected.store(FALSE);
    sessions.erase(id);
    //sessions.clear();
}

void Server::do_recv(int id)
{
    DWORD flags = 0;


    SOCKET client_s = sessions[id].sock;
    OVER_EX* over = &sessions[id].over;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)sessions[id].prev_size,
        &flags, &(over->overlapped), NULL)){
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING){
            printf("id: %d recv error: %d\n", id, err_no);
            Disconnected(id, sessions[id].over.roomID);
        }
    }
}

void Server::send_packet(int to, char* packet)
{
    SOCKET client_s = sessions[to].sock;
    OVER_EX* over = new OVER_EX;
    ZeroMemory(over, sizeof(OVER_EX));
    over->dataBuffer.buf = packet;
    over->dataBuffer.len = packet[0];

    memcpy(over->messageBuffer, packet, packet[0]);
    over->is_recv = false;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSASend(client_s, &over->dataBuffer, 1, NULL,
        0, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING){
            printf("to: %d packet: %d send error: %d\n", to, packet[1], err_no);
            //Disconnected(to, sessions[to].over.roomID);
        }
    }
}

void Server::send_ID_player_packet(int id, int roomID)
{
    player_ID_packet p;

    p.id = id;
    p.size = sizeof(player_login_packet);
    p.type = PacketType::Type_player_ID;
    send_packet(id, reinterpret_cast<char*>(&p));
}

void Server::send_login_player_packet(int id, int to, int roomID)
{
    player_login_packet p;

    p.id = id;
    p.size = sizeof(player_login_packet);
    p.type = PacketType::Type_player_login;
    p.Position = sessions[id].f3Position;

    printf("%d: login to %d\n",id, to);

    send_packet(to, reinterpret_cast<char*>(&p));
}

void Server::send_disconnect_player_packet(int id, int roomID)
{
    player_remove_packet p;
    p.id = id;
    p.size = sizeof(player_remove_packet);
    p.type = PacketType::Type_player_remove;

    auto iter = gameroom.equal_range(roomID);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected.load(std::memory_order_seq_cst))
            send_packet(it->second, reinterpret_cast<char*>(&p));

    }
    Disconnected(id, roomID);
}

void Server::send_packet_to_players(int id, char* buf, int roomID)
{
    auto iter = gameroom.equal_range(roomID);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected.load(std::memory_order_seq_cst))
            if (calc_distance(id, it->second) <= VIEWING_DISTANCE) {
                send_packet(it->second, buf);
            }
    }
}

void Server::send_packet_to_allplayers(int roomnum, char* buf)
{
    auto iter = gameroom.equal_range(roomnum);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected.load(std::memory_order_seq_cst))
            send_packet(it->second, buf);
    }
}

void Server::send_map_collapse_packet(int num, int map_num)
{
    map_collapse_packet packet;
    packet.size = sizeof(map_collapse_packet);
    packet.type = PacketType::Type_map_collapse;
    packet.block_num = num;
    packet.id = 0;

    send_packet_to_allplayers(map_num, reinterpret_cast<char*>(&packet));
}

void Server::send_cloud_move_packet(float x, float z, int map_num)
{
    cloud_move_packet packet;
    packet.size = sizeof(cloud_move_packet);
    packet.type = PacketType::Type_cloud_move;
    packet.x = x;
    packet.z = z;
    packet.id = 0;

    send_packet_to_allplayers(map_num, reinterpret_cast<char*>(&packet));
}

void Server::game_end(int roomnum)
{
    game_end_packet packet;
    packet.id = 0;
    packet.size = sizeof(packet);
    packet.type = PacketType::Type_game_end;

    auto iter = gameroom.equal_range(roomnum);
    for (auto it = iter.first; it != iter.second; ++it) {
        if (sessions[it->second].connected.load(std::memory_order_seq_cst))
            send_packet(it->second, reinterpret_cast<char*>(&packet));
    }
    maps.erase(roomnum);
}

float Server::calc_distance(int a, int b)
{
    float value = pow(((short)sessions[a].f3Position.load(std::memory_order_seq_cst).x - (short)sessions[b].f3Position.load(std::memory_order_seq_cst).x), 2)
        + pow(((short)sessions[a].f3Position.load(std::memory_order_seq_cst).z - (short)sessions[b].f3Position.load(std::memory_order_seq_cst).z), 2);

    if (value < 0)
        return sqrt(-value);
    else
        return sqrt(value);
}

unsigned short Server::calc_attack(int id, char attacktype)
{
    return 0;
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

    xmf3Shift = Add(sessions[id].f3Position.load(std::memory_order_seq_cst), xmf3Shift);

    return xmf3Shift;
}

void Server::process_packet(int id, char* buf, int roomID)
{
    // 클라이언트에서 받은 패킷 처리
    switch (buf[1]) {
    case PacketType::Type_game_ready: {
        game_ready_packet* p = reinterpret_cast<game_ready_packet*>(buf);
        sessions[p->id].isready = true;
        break;
    }
    case PacketType::Type_game_start: {
        start_ok_packet* p = reinterpret_cast<start_ok_packet*>(buf);
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
        //printf("move %f %f\n", sessions[p->id].f3Position.load(std::memory_order_seq_cst).x, sessions[p->id].f3Position.load(std::memory_order_seq_cst).z);
        send_packet_to_players(id, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::Type_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        sessions[p->id].f3Position.store(p->Position);

        send_packet(id, reinterpret_cast<char*>(p));
        break;
    }
    case PacketType::Type_weapon_swap :{
         Weapon_swap_packet* p = reinterpret_cast<Weapon_swap_packet*>(buf);
         send_packet_to_allplayers(roomID, reinterpret_cast<char*>(p));
         break;
    }
    case PacketType::Type_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        switch (p->MoveType) {
        case PlayerMove::JUMP:{
            send_packet_to_players(id, reinterpret_cast<char*>(p), roomID);
                break;
            }
        default: {
            break;
        }
        }
        break;
    }
    case PacketType::Type_player_attack:{
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        send_packet_to_players(p->id, reinterpret_cast<char*>(p), roomID);
        switch (p->attack_type) {
        case BOW: {
            // 충돌처리
            break;
        }
        case SWORD1H: {
            // 충돌처리

            break;
        }
        }
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
        ULONG roomID;
        OVER_EX* over_ex;

        BOOL retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&id, (LPOVERLAPPED*)&over_ex, INFINITE);

        //std::thread::id Thread_id = std::this_thread::get_id();
        roomID = over_ex->roomID;
        switch (over_ex->type) {
        case OE_session: {
            if (FALSE == retval)
            {
                //printf("error = %d\n", WSAGetLastError());
                display_error("GQCS", WSAGetLastError());
                Disconnected(id, roomID);
                continue;
            }

            if ((Transferred == 0)) {
                display_error("GQCS", WSAGetLastError());
                Disconnected(id, roomID);
                continue;
            }
            //printf("%d\n", true);
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
                        process_packet(id, sessions[id].packet_buf, roomID);
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
                over_ex = NULL;
                if (_heapchk() != _HEAPOK)
                    DebugBreak();
            }
            break;
        }
        case OE_map: {
            if (FALSE == retval)
                continue;
            switch (over_ex->messageBuffer[1]) {
            case EventType::Mapset: {
                map_block_set* p = reinterpret_cast<map_block_set*>(over_ex->messageBuffer);
                maps[roomID].Set_map();
                break;
            }
            case EventType::Cloud_move: {
                cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(over_ex->messageBuffer);
                send_cloud_move_packet(p->x, p->z, p->id);
                //printf("cloud x: %f | y: %f\n\n", p->x, p->z);
                maps[p->id].cloud_move();
                break;
            }
            }
        }
        break;
        }

    }
}

bool Server::Init()
{
    sessions.clear();

    // 윈속 초기화
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // 입출력 완료 포트 생성
    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    // CPU 개수 확인
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
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