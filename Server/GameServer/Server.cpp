#pragma once
#pragma warning(disable : 4996)
#include "Server.h"

void SESSION::init() 
{
    memset(this, 0x00, sizeof(SESSION));
    connected = false;
    isready = false;
    playing = false;
    key = -1;

    state = 0;
    f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    dx = 0;
    dy = 0;

    weapon = PlayerType::PT_BASIC;
    helmet = 0;
    shoes = 0;

    hp = 0;
    lv = 0;
    speed = 20;
}

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
    std::wcout << L"���� " << lpMsgBuf << std::endl;
    LocalFree(lpMsgBuf);
}

int Server::SetClientKey(int roomID)
{
    int cnt = 0;
    while (true){
        if (cnt == 20)
            return -1;
        if (FALSE == sessions[roomID][cnt].connected){
            return cnt;
        }
        else 
            ++cnt;
    }
}

int Server::SetroomID()
{
    int cnt = 1;
    while (1) {
        if (sessions.find(cnt) == sessions.end()) {
            sm_lock.lock();
            //std::array<SESSION, 20> s{};
            sessions.emplace(cnt, std::array<SESSION, 20>{});
            sm_lock.unlock();
            
            /*mm_lock.lock();
            maps.emplace(cnt, Map(cnt));
            maps[cnt].SetNum(cnt);
            maps[cnt].init_Map(this, m_pTimer);
            mm_lock.unlock();*/

            printf("create game room - %d\n", cnt);

            return cnt;
        }
        if (FALSE == sessions[cnt][MAX_PLAYER - 1].connected)
            return cnt;
        else {
           ++cnt;
        }
    }
}

void Server::ConnectLobby()
{
    std::array<SESSION, 20> s{};
    sessions.emplace(LOBBY_ID, s);
    sessions[LOBBY_ID][0].key = LOBBY_ID;
    sessions[LOBBY_ID][0].connected = TRUE;
    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    // socket()
    sessions[LOBBY_ID][0].sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    // connect()
    SOCKADDR_IN lobbyaddr;
    ZeroMemory(&lobbyaddr, sizeof(lobbyaddr));
    lobbyaddr.sin_family = AF_INET;
    lobbyaddr.sin_addr.s_addr = inet_addr(SERVERIP);
    lobbyaddr.sin_port = htons(LOBBYPORT);
    connect(sessions[LOBBY_ID][0].sock, (SOCKADDR*)&lobbyaddr, sizeof(lobbyaddr));

    memset(&sessions[LOBBY_ID][0].over.overlapped, 0, sizeof(sessions[LOBBY_ID][0].over.overlapped));
    sessions[LOBBY_ID][0].over.is_recv = true;
    sessions[LOBBY_ID][0].over.dataBuffer.len = BUFSIZE;
    sessions[LOBBY_ID][0].over.dataBuffer.buf =
        sessions[LOBBY_ID][0].over.messageBuffer;

    CreateIoCompletionPort((HANDLE)sessions[LOBBY_ID][0].sock, hcp, LOBBY_ID, 0);

    do_recv(LOBBY_ID, 0);
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

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen = sizeof(SOCKADDR_IN);
    DWORD flags;

    printf("ready\n");

    while (1) {
        accept_lock.lock();
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            printf("accept error: %d", WSAGetLastError());
            break;
        }

        int roomID = SetroomID();
        int client_key = SetClientKey(roomID);
        if (client_key == -1) {
            closesocket(client_sock);
            break;
        }

        memset(&sessions[roomID][client_key], 0x00, sizeof(SESSION));
        sessions[roomID][client_key].connected = TRUE;
        sessions[roomID][client_key].key = client_key;
        sessions[roomID][client_key].roomID = roomID;
        sessions[roomID][client_key].sock = client_sock;
        sessions[roomID][client_key].clientaddr = clientaddr;
        getpeername(client_sock, (SOCKADDR*)&sessions[roomID][client_key].clientaddr
            , &sessions[roomID][client_key].addrlen);

        sessions[roomID][client_key].over.type = 0;
        sessions[roomID][client_key].over.dataBuffer.len = BUFSIZE;
        sessions[roomID][client_key].over.dataBuffer.buf =
            sessions[roomID][client_key].over.messageBuffer;
        sessions[roomID][client_key].over.is_recv = true;
        sessions[roomID][client_key].over.roomID = roomID;
        accept_lock.unlock();
        // ���ϰ� ����� �Ϸ� ��Ʈ ����

        CreateIoCompletionPort((HANDLE)client_sock, hcp, client_key, 0);

        //printf("client_key: %d\n", client_key);
        send_player_key_packet(client_key, roomID);

        do_recv(client_key, roomID);
    }

    // closesocket()
    closesocket(listen_sock); 

    // ���� ����
    WSACleanup();
}

void Server::Disconnected(int key, int roomID)
{
    std::lock_guard<std::mutex> lock_guard(sm_lock);
    for (int i = 0; i < MAX_PLAYER; ++i) {
        //printf("%d\n", i->second);
        if (sessions[roomID][i].key == key) {
            printf("client_end: IP =%s, port=%d key = %d, Room = %d\n",
                inet_ntoa(sessions[roomID][i].clientaddr.sin_addr)
                , ntohs(sessions[roomID][i].clientaddr.sin_port), key, roomID);
            //send_disconnect_player_packet(key);
            closesocket(sessions[roomID][i].sock);
            sessions[roomID][i].connected = FALSE;
            sessions[roomID][i].key = -1;
            //printf("disconnected %d\n", gameroom[roomID].ID[i]);
            break;
        }
    }
    //sessions[roomID].clear();
}

void Server::do_recv(int key, int roomID)
{
    DWORD flags = 0;
    SOCKET client_s = sessions[roomID][key].sock;
    OVER_EX* over = &sessions[roomID][key].over;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)sessions[roomID][key].prev_size,
        &flags, &(over->overlapped), NULL)){
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING){
            printf("key: %d recv error: %d\n", key, err_no);
            Disconnected(key, sessions[roomID][key].over.roomID);
        }
    }
}

void Server::send_packet(int to, char* packet, int roomID)
{
    SOCKET client_s = sessions[roomID][to].sock;
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
            //printf("to: %d packet: %d send error: %d\n", to, packet[1], err_no);
            //Disconnected(to, sessions[roomID][to].over.roomID);
        }
    }
}

void Server::send_player_key_packet(int key, int roomID)
{
    player_key_packet p;

    p.key = key;
    p.size = sizeof(player_key_packet);
    p.type = PacketType::SC_player_key;
    p.roomid = roomID;
    send_packet(key, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_player_loginOK_packet(int key, int roomID)
{
    player_loginOK_packet p;

    p.key = key;
    p.size = sizeof(player_loginOK_packet);
    p.type = PacketType::SC_player_loginOK;
    p.roomid = roomID;
    p.Position = sessions[roomID][key].f3Position.load();
    p.dx = sessions[roomID][key].dx.load();
    p.dy = sessions[roomID][key].dy.load();
    send_packet(key, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_add_player_packet(int key, int to, int roomID)
{
    player_add_packet p;

    p.key = key;
    p.size = sizeof(player_add_packet);
    p.type = PacketType::SC_player_add;
    p.roomid = roomID;
    p.Position = sessions[roomID][key].f3Position.load();
    p.dx = sessions[roomID][key].dx.load();
    p.dy = sessions[roomID][key].dy.load();

    //printf("%d send login to %d\n",key, to);

    send_packet(to, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_remove_player_packet(int key, int roomID)
{
    player_remove_packet p;
    p.key = key;
    p.size = sizeof(player_remove_packet);
    p.type = PacketType::SC_player_remove;
    p.roomid = roomID;

    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == TRUE)
            send_packet(i, reinterpret_cast<char*>(&p), roomID);
    }
    Disconnected(key, roomID);
}

void Server::send_disconnect_player_packet(int key, int roomID)
{
    player_disconnect_packet p;
    p.key = key;
    p.size = sizeof(player_disconnect_packet);
    p.type = PacketType::SC_player_disconnect;
    p.roomid = roomID;

    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == TRUE)
            send_packet(i, reinterpret_cast<char*>(&p), roomID);
    }
    Disconnected(key, roomID);
}

void Server::send_packet_to_players(int key, char* buf, int roomID)
{
   //sm_lock.lock();
    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == TRUE)
            if (sessions[roomID][i].connected)
                if (in_VisualField(sessions[roomID][key], sessions[roomID][i], roomID)) {
                    send_packet(i, buf, roomID);
                }
    }
    //sm_lock.unlock();
}

void Server::send_packet_to_allplayers(int roomID, char* buf)
{
   //sm_lock.lock();
    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == TRUE)
            if (sessions[roomID][i].connected.load(std::memory_order_seq_cst))
                send_packet(i, buf, roomID);
    }
   //sm_lock.unlock();
}

void Server::send_map_collapse_packet(int num, int map_num)
{
    map_collapse_packet p;
    p.size = sizeof(map_collapse_packet);
    p.type = PacketType::SC_map_collapse;
    p.block_num = num;
    p.key = 0;
    p.roomid = map_num;

    send_packet_to_allplayers(map_num, reinterpret_cast<char*>(&p));
}

void Server::send_cloud_move_packet(float x, float z, int map_num)
{
    cloud_move_packet p;
    p.size = sizeof(cloud_move_packet);
    p.type = PacketType::SC_cloud_move;
    p.x = x;
    p.z = z;
    p.key = 0;
    p.roomid = map_num;

    send_packet_to_allplayers(map_num, reinterpret_cast<char*>(&p));
}

void Server::game_end(int roomnum)
{
    game_end_packet p;
    p.key = 0;
    p.size = sizeof(p);
    p.type = PacketType::SC_game_end;
    p.roomid = roomnum;

    send_packet_to_allplayers(roomnum, reinterpret_cast<char*>(&p));
    mm_lock.lock();
    maps.erase(roomnum);
    mm_lock.unlock();
    sm_lock.lock();
    sessions.erase(roomnum);
    sm_lock.unlock();
}

bool Server::in_VisualField(SESSION a, SESSION b, int roomID)
{
    float value = pow(((short)a.f3Position.load().x - (short)b.f3Position.load().x), 2)
        + pow(((short)a.f3Position.load().z - (short)b.f3Position.load().z), 2);

    if (value < 0) {
        if (sqrt(-value) <= VIEWING_DISTANCE) return true;
    }
    else {
        if (sqrt(value) <= VIEWING_DISTANCE) return true;
    }
    return false;
}

bool Server::in_VisualField(Monster a, SESSION b, int roomID)
{
    float value = pow(((short)a.f3Position.load().x - (short)b.f3Position.load().x), 2)
        + pow(((short)a.f3Position.load().z - (short)b.f3Position.load().z), 2);

    if (value < 0) {
        if (sqrt(-value) <= VIEWING_DISTANCE) return true;
    }
    else {
        if (sqrt(value) <= VIEWING_DISTANCE) return true;
    }
    return false;
}

unsigned short Server::calc_attack(int key, char attacktype)
{

    return 0;
}

void Server::process_packet(int key, char* buf, int roomID)
{
    // Ŭ���̾�Ʈ���� ���� ��Ŷ ó��
    switch (buf[1]) {
    case PacketType::CS_player_login: {
        player_login_packet* p = reinterpret_cast<player_login_packet*>(buf);

        int client_key = p->key;

        sessions[roomID][client_key].f3Position = XMFLOAT3(50.f, 150.0f, 50.f);

        strcpy_s(sessions[roomID][client_key].id, p->id);

        send_player_loginOK_packet(client_key, sessions[roomID][client_key].roomID);

        printf("client_connected: IP =%s, port=%d key = %d Room = %d\n",
            inet_ntoa(sessions[roomID][client_key].clientaddr.sin_addr)
            , ntohs(sessions[roomID][client_key].clientaddr.sin_port), client_key, roomID);

        /*for (int i = 0; i < MAX_PLAYER; ++i) {
            if (TRUE == sessions[roomID][i].connected)
                if ((i != client_key))
                    send_add_player_packet(i, client_key, roomID);
        }
        for (int i = 0; i < MAX_PLAYER; ++i) {
            if (TRUE == sessions[roomID][i].connected)
                if ((i != client_key))
                    send_add_player_packet(client_key, i, roomID);
        }*/

        for (auto& s : sessions[roomID]) {
            if ((TRUE == s.connected) && (s.key.load() != client_key)){
                send_add_player_packet(s.key.load(), client_key, roomID);
            }
        }
        for (auto& s : sessions[roomID]) {
            if ((TRUE == s.connected) && (s.key.load() != client_key))
                send_add_player_packet(client_key, s.key.load(), roomID);
        }

        break;
    }
    case PacketType::CS_game_ready: {
        game_ready_packet* p = reinterpret_cast<game_ready_packet*>(buf);
        sessions[roomID][p->key].isready = true;
        break;
    }
    case PacketType::CS_game_start: {
        start_ok_packet* p = reinterpret_cast<start_ok_packet*>(buf);
        p->size = sizeof(p);
        p->type = SC_start_ok;
        break;
    }
    case PacketType::CS_player_info:{
        player_info_packet* p = reinterpret_cast<player_info_packet*>(buf);
        p->type = SC_player_info;
        break;
    }
    case PacketType::CS_player_pos: {
        player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);
        p->type = SC_player_pos;
        sessions[roomID][p->key].f3Position = p->Position;
        float tx = fmodf(sessions[roomID][p->key].dx.load() + p->dx, 360.f);
        sessions[roomID][p->key].dx.store(fmodf(sessions[roomID][p->key].dx.load() + p->dx, 360.f));
        sessions[roomID][p->key].dy.store(fmodf(sessions[roomID][p->key].dy.load() + p->dy, 360.f));
        //printf("move %f %f\n", sessions[roomID][p->key].f3Position.load(std::memory_order_seq_cst).x, sessions[roomID][p->key].f3Position.load(std::memory_order_seq_cst).z);
        
        std::unordered_set<int> old_nm = sessions[roomID][p->key].near_monster;
        std::unordered_set<int> new_nm;

        for (auto& m : m_pBot->monsters[roomID]) {
            if (in_VisualField(m, sessions[roomID][p->key], roomID)) {
                new_nm.insert(m.key);
            }
        }

        for (auto& m : new_nm) {
            if (old_nm.count(m) == 0) {
                sessions[roomID][p->key].near_monster.insert(m);
                send_add_monster(p->key, roomID);
            }
        }
        for (auto& m : old_nm) {
            if (new_nm.count(m) == 0) {
                sessions[roomID][p->key].near_monster.erase(m);
                send_remove_monster(p->key, roomID);
            }
        }

        
        send_packet_to_players(key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        sessions[roomID][p->key].f3Position = p->Position;

        send_packet(key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_weapon_swap :{
         Weapon_swap_packet* p = reinterpret_cast<Weapon_swap_packet*>(buf);
         p->type = SC_weapon_swap;
         send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
         break;
    }
    case PacketType::CS_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        p->type = SC_player_move;
        switch (p->MoveType) {
        case PlayerMove::JUMP:{
            send_packet_to_players(key, reinterpret_cast<char*>(p), roomID);
                break;
            }
        default: {
            break;
        }
        }
        break;
    }
    case PacketType::CS_player_attack:{
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        switch (p->attack_type) {
        case BOWL: {
            // �浹ó��
            break;
        }
        case SWORD1HL: {
            // �浹ó��

            break;
        }
        }
        p->type = SC_player_attack;
        send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
        break;

    }
    case PacketType::CS_allow_shot: {
        player_shot_packet* p = reinterpret_cast<player_shot_packet*>(buf);
        p->type = SC_allow_shot;
        send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_player_stop: {
        player_stop_packet* p = reinterpret_cast<player_stop_packet*>(buf);
        p->type = SC_player_stop;
        send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    }
}

void Server::WorkerFunc()
{
    DWORD Transferred;
    SOCKET client_sock;
    ULONG key;
    WSAOVERLAPPED* over;

    while (1) {
        BOOL retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&key, (LPOVERLAPPED*)&over, INFINITE);

        //std::thread::key Thread_key = std::this_thread::get_key();
        OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);
        int roomID = over_ex->roomID;
        switch (over_ex->type) {
        case OE_session: {
            if (FALSE == retval)
            {
                //printf("error = %d\n", WSAGetLastError());
                display_error("GQCS", WSAGetLastError());
                Disconnected(key, roomID);
                continue;
            }

            if ((Transferred == 0)) {
                display_error("GQCS", WSAGetLastError());
                Disconnected(key, roomID);
                continue;
            }
            //printf("%d\n", true);
            if (over_ex->is_recv) {
                //printf("thread key: %d\n", Thread_key);
                int rest_size = Transferred;
                char* buf_ptr = over_ex->messageBuffer;
                char packet_size = 0;
                if (0 < sessions[roomID][key].prev_size)
                    packet_size = sessions[roomID][key].packet_buf[0];
                while (rest_size > 0) {
                    if (0 == packet_size) packet_size = buf_ptr[0];
                    int required = packet_size - sessions[roomID][key].prev_size;
                    if (rest_size >= required) {
                        memcpy(sessions[roomID][key].packet_buf + sessions[roomID][key].
                            prev_size, buf_ptr, required);
                        process_packet(key, sessions[roomID][key].packet_buf, roomID);
                        rest_size -= required;
                        buf_ptr += required;
                        packet_size = 0;
                    }
                    else {
                        memcpy(sessions[roomID][key].packet_buf + sessions[roomID][key].prev_size,
                            buf_ptr, rest_size);
                        rest_size = 0;
                    }
                }
                do_recv(key, roomID);
            }
            else {
                delete over_ex;
                over_ex = NULL;
                if (_heapchk() != _HEAPOK)
                    DebugBreak();
            }
            break;
        }
        case OE_gEvent: {
            if (FALSE == retval)
                continue;
            switch (over_ex->messageBuffer[1]) {
            case EventType::Mapset:{
                map_block_set* p = reinterpret_cast<map_block_set*>(over_ex->messageBuffer);
                maps[key].Set_map();
                
                game_end_packet ep;
                ep.roomid = key;
                ep.size = sizeof(ep);
                ep.type = EventType::game_end;
                m_pTimer->push_event(key, OE_gEvent, 1000 * (MAP_BREAK_TIME*9), reinterpret_cast<char*>(&ep));
                break;
            }
            case EventType::Cloud_move: {
                cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(over_ex->messageBuffer);
                send_cloud_move_packet(p->x, p->z, p->roomid);
                //printf("room: %d cloud x: %f | y: %f\n\n", p->roomid, p->x, p->z);
                maps[key].ismove = true;
                maps[key].cloud_move();
                delete over_ex;
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

    // ���� �ʱ�ȭ
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // ����� �Ϸ� ��Ʈ ����
    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    // CPU ���� Ȯ��
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    for (int i = 0; i <(int)si.dwNumberOfProcessors * 2; i++)
        working_threads.emplace_back(std::thread(&Server::WorkerFunc, this));

    //ConnectLobby();

    m_pTimer = new Timer;

    timer_thread = std::thread(&Timer::init, m_pTimer, Gethcp());
    accept_thread = std::thread(&Server::Accept, this);

    return 1;
}

void Server::Thread_join()
{
    accept_thread.join();
    timer_thread.join();

    for (auto& t : working_threads)
        t.join();

    for (auto& t : map_threads)
        t.join();

    delete m_pTimer;
    CloseHandle(hcp);
}